/* Private keys after Go's crypto/x509 pkcs8.go, pkcs1.go and sec1.go
   (BSD-3-Clause, see LICENSE-Go) */
#include <alt/crypto/ecdsa.h>
#include <alt/crypto/ed25519.h>
#include <alt/crypto/x509.h>
#include <string.h>

#define OIDEQ(o, ...) alt_oid_eq((o), &ALT_OID(__VA_ARGS__))

static int curve_of(const alt_oid *o) {
	if (OIDEQ(o, 1, 2, 840, 10045, 3, 1, 7))
		return ALT_X509_P256;
	if (OIDEQ(o, 1, 3, 132, 0, 34))
		return ALT_X509_P384;
	if (OIDEQ(o, 1, 3, 132, 0, 35))
		return ALT_X509_P521;
	return 0;
}

/* PKCS #1 RSAPrivateKey (two primes) */
static const char *pkcs1(alt_der der, alt_x509_private_key *k) {
	alt_der s, v[8];
	int64_t version;
	if (!alt_der_read(&der, &s, ALT_ASN1_SEQUENCE) || !alt_der_read_int64(&s, &version))
		return "x509: failed to parse private key";
	if (version > 1)
		return "x509: unsupported private key version";
	for (int i = 0; i < 8; i++)
		if (i == 1 ? !alt_der_read_int64(&s, &k->rsa.e) : !alt_der_read_uint_bytes(&s, &v[i]))
			return "x509: invalid RSA private key";
	if (!alt_der_empty(s))
		return "x509: multi-prime RSA keys are not supported";
	k->alg = ALT_X509_PK_RSA;
	k->rsa.n = v[0], k->rsa.d = v[2], k->rsa.p = v[3], k->rsa.q = v[4], k->rsa.dp = v[5], k->rsa.dq = v[6], k->rsa.qinv = v[7];
	if (k->rsa.e <= 0)
		return "x509: private key contains zero or negative value";
	return 0;
}

/* SEC 1 ECPrivateKey */
static const char *sec1(alt_der der, const alt_oid *named, alt_x509_private_key *k) {
	alt_der s, priv, params;
	int64_t version;
	bool has_params;
	if (!alt_der_read(&der, &s, ALT_ASN1_SEQUENCE) || !alt_der_read_int64(&s, &version) ||
		!alt_der_read(&s, &priv, ALT_ASN1_OCTET_STRING) ||
		!alt_der_read_optional(&s, &params, &has_params, 0 | ALT_ASN1_CONTEXT | ALT_ASN1_CONSTRUCTED))
		return "x509: failed to parse EC private key";
	if (version != 1)
		return "x509: unknown EC private key version";
	alt_oid curve;
	int cv = 0;
	if (named)
		cv = curve_of(named);
	else if (has_params && alt_der_read_oid(&params, &curve))
		cv = curve_of(&curve);
	if (!cv)
		return "x509: unknown elliptic curve";
	size_t size = alt_ec_scalar_size((alt_ec_curve)cv);
	while (priv.n > size) {
		if (priv.p[0] != 0)
			return "x509: invalid private key length";
		priv.p++, priv.n--;
	}
	memset(k->ec_priv, 0, sizeof k->ec_priv);
	memcpy(k->ec_priv + size - priv.n, priv.p, priv.n);
	if (alt_ec_public((alt_ec_curve)cv, k->ec_priv, k->ec_pub) < 0)
		return "ecdsa: invalid private key";
	k->alg = ALT_X509_PK_ECDSA;
	k->curve = cv;
	return 0;
}

static const char *pkcs8(alt_der der, alt_x509_private_key *k) {
	alt_der s, ai, key;
	int64_t version;
	alt_oid alg;
	if (!alt_der_read(&der, &s, ALT_ASN1_SEQUENCE) || !alt_der_read_int64(&s, &version) || !alt_der_read(&s, &ai, ALT_ASN1_SEQUENCE) ||
		!alt_der_read_oid(&ai, &alg) || !alt_der_read(&s, &key, ALT_ASN1_OCTET_STRING))
		return "x509: failed to parse private key";
	if (OIDEQ(&alg, 1, 2, 840, 113549, 1, 1, 1))
		return pkcs1(key, k);
	if (OIDEQ(&alg, 1, 2, 840, 10045, 2, 1)) {
		alt_oid named;
		bool has = alt_der_read_oid(&ai, &named);
		return sec1(key, has ? &named : 0, k);
	}
	if (OIDEQ(&alg, 1, 3, 101, 112)) {
		if (ai.n)
			return "x509: invalid Ed25519 private key parameters";
		alt_der seed;
		if (!alt_der_read(&key, &seed, ALT_ASN1_OCTET_STRING))
			return "x509: invalid Ed25519 private key";
		if (seed.n != 32)
			return "x509: invalid Ed25519 private key length";
		uint8_t pub[32];
		alt_ed25519_keygen(pub, k->ed25519, seed.p);
		k->alg = ALT_X509_PK_ED25519;
		return 0;
	}
	return "x509: PKCS#8 wrapping contained private key with unknown algorithm";
}

int alt_x509_parse_private_key(alt_arena *a, const void *der, size_t n, alt_x509_private_key *out, const char **why) {
	(void)a;
	alt_der d = alt_der_make(der, n);
	memset(out, 0, sizeof *out);
	const char *e8 = pkcs8(d, out);
	if (!e8)
		return 0;
	memset(out, 0, sizeof *out);
	if (!pkcs1(d, out))
		return 0;
	memset(out, 0, sizeof *out);
	if (!sec1(d, 0, out))
		return 0;
	if (why)
		*why = e8;
	return -ALT_EX509;
}
