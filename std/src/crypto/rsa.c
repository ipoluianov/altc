/* RSA after RFC 8017 and Go's crypto/internal/fips140/rsa (checks,
   encodings): Montgomery arithmetic over up to 128 limbs, constant-time
   exponentiation with a fixed 4-bit window, CRT for the private key,
   reductions by shifting in one bit at a time (as Go's bigmod). */
#include <alt/encoding/asn1.h>
#include <alt/math/big.h>
#include <alt/crypto/rsa.h>
#include <stdbool.h>
#include <string.h>

enum { N = 64 }; /* 4096-bit moduli */
typedef unsigned __int128 u128;

typedef struct {
	int n;
	uint64_t m[N], minv, rr[N], one[N];
} mod;

static uint64_t addc(uint64_t a, uint64_t b, uint64_t *c) {
	u128 s = (u128)a + b + *c;
	*c = (uint64_t)(s >> 64);
	return (uint64_t)s;
}

static uint64_t subb(uint64_t a, uint64_t b, uint64_t *bw) {
	u128 d = (u128)a - b - *bw;
	*bw = (uint64_t)(d >> 64) & 1;
	return (uint64_t)d;
}

static void sel(uint64_t *r, const uint64_t *a, const uint64_t *b, uint64_t cond, int n) {
	uint64_t m = 0 - cond;
	for (int i = 0; i < n; i++)
		r[i] = (a[i] & m) | (b[i] & ~m);
}

/* x = 2x + bit mod m, for x < m */
static void shift_in_bit(const mod *m, uint64_t *x, uint64_t bit) {
	uint64_t t[N], u[N], c = bit, bw = 0;
	for (int i = 0; i < m->n; i++) {
		uint64_t v = x[i];
		t[i] = v << 1 | c;
		c = v >> 63;
	}
	for (int i = 0; i < m->n; i++)
		u[i] = subb(t[i], m->m[i], &bw);
	sel(x, u, t, c | (bw ^ 1), m->n);
}

/* big-endian bytes of any length, reduced mod m (normal form) */
static void reduce_bytes(const mod *m, uint64_t *x, const uint8_t *b, size_t n) {
	memset(x, 0, (size_t)m->n * 8);
	for (size_t i = 0; i < n; i++)
		for (int k = 7; k >= 0; k--)
			shift_in_bit(m, x, b[i] >> k & 1);
}

static void mont_mul(const mod *m, uint64_t *r, const uint64_t *a, const uint64_t *b) {
	int n = m->n;
	uint64_t t[N + 2] = {0};
	for (int i = 0; i < n; i++) {
		uint64_t c = 0;
		for (int j = 0; j < n; j++) {
			u128 x = (u128)a[j] * b[i] + t[j] + c;
			t[j] = (uint64_t)x;
			c = (uint64_t)(x >> 64);
		}
		u128 x = (u128)t[n] + c;
		t[n] = (uint64_t)x;
		t[n + 1] = (uint64_t)(x >> 64);
		uint64_t q = t[0] * m->minv;
		x = (u128)q * m->m[0] + t[0];
		c = (uint64_t)(x >> 64);
		for (int j = 1; j < n; j++) {
			x = (u128)q * m->m[j] + t[j] + c;
			t[j - 1] = (uint64_t)x;
			c = (uint64_t)(x >> 64);
		}
		x = (u128)t[n] + c;
		t[n - 1] = (uint64_t)x;
		t[n] = t[n + 1] + (uint64_t)(x >> 64);
	}
	uint64_t u[N], bw = 0;
	for (int i = 0; i < n; i++)
		u[i] = subb(t[i], m->m[i], &bw);
	(void)subb(t[n], 0, &bw);
	sel(r, t, u, bw, n);
}

/* m from big-endian bytes (odd, no leading zeros): false otherwise */
static bool mod_init(mod *m, const uint8_t *b, size_t n) {
	if (n == 0 || n > N * 8 || b[0] == 0 || !(b[n - 1] & 1))
		return false;
	m->n = (int)((n + 7) / 8);
	memset(m->m, 0, sizeof m->m);
	for (size_t i = 0; i < n; i++) {
		size_t bit = (n - 1 - i) * 8;
		m->m[bit / 64] |= (uint64_t)b[i] << (bit % 64);
	}
	uint64_t inv = 1;
	for (int i = 0; i < 7; i++)
		inv *= 2 - m->m[0] * inv;
	m->minv = 0 - inv;
	uint64_t x[N] = {0};
	x[0] = 1;
	for (int i = 0; i < 128 * m->n; i++)
		shift_in_bit(m, x, 0);
	memcpy(m->rr, x, sizeof x);
	uint64_t one[N] = {1};
	mont_mul(m, m->one, one, m->rr);
	return true;
}

/* r = a^e mod m (a in Montgomery form, e big-endian bytes): constant
   time in a and e, for the given length of e */
static void mod_exp(const mod *m, uint64_t *r, const uint64_t *a, const uint8_t *e, size_t ne) {
	uint64_t tab[16][N];
	memcpy(tab[0], m->one, sizeof tab[0]);
	memcpy(tab[1], a, sizeof tab[1]);
	for (int i = 2; i < 16; i++)
		mont_mul(m, tab[i], tab[i - 1], a);
	uint64_t acc[N], pick[N];
	memcpy(acc, m->one, sizeof acc);
	for (size_t i = 0; i < ne; i++)
		for (int half = 1; half >= 0; half--) {
			for (int k = 0; k < 4; k++)
				mont_mul(m, acc, acc, acc);
			unsigned w = e[i] >> (4 * half) & 15;
			memset(pick, 0, sizeof pick);
			for (unsigned j = 0; j < 16; j++) {
				uint64_t eq = (uint64_t)((((j ^ w) - 1u) >> 31) & 1);
				uint64_t msk = 0 - eq;
				for (int l = 0; l < m->n; l++)
					pick[l] |= tab[j][l] & msk;
			}
			mont_mul(m, acc, acc, pick);
		}
	memcpy(r, acc, (size_t)m->n * 8);
	memset(tab, 0, sizeof tab);
}

static void to_bytes(const mod *m, uint8_t *b, size_t n, const uint64_t *x) {
	uint64_t one[N] = {1}, y[N];
	mont_mul(m, y, x, one);
	for (size_t i = 0; i < n; i++) {
		size_t bit = (n - 1 - i) * 8;
		b[i] = bit / 64 < (size_t)m->n ? (uint8_t)(y[bit / 64] >> (bit % 64)) : 0;
	}
}

static int bitlen(const uint8_t *b, size_t n) {
	if (!n)
		return 0;
	return (int)(n - 1) * 8 + (32 - __builtin_clz((unsigned)b[0]));
}

/* Go's checkPublicKey and the 1024-bit minimum */
static int check_public(const alt_rsa_public *k, mod *m) {
	if (!mod_init(m, k->n, k->nlen))
		return -ALT_EKEY;
	if (bitlen(k->n, k->nlen) < 1024 || k->e < 2 || !(k->e & 1) || k->e > 0x7fffffff)
		return -ALT_EKEY;
	return 0;
}

size_t alt_rsa_size(const alt_rsa_public *k) { return k->nlen; }

/* s^e mod n into em (k bytes): false when s >= n */
static bool encrypt(const alt_rsa_public *k, const mod *m, const uint8_t *s, uint8_t *em) {
	/* s < n: compare as numbers (same length) */
	int cmp = memcmp(s, k->n, k->nlen);
	if (cmp >= 0)
		return false;
	uint64_t x[N], xm[N];
	reduce_bytes(m, x, s, k->nlen);
	mont_mul(m, xm, x, m->rr);
	uint8_t e[8];
	for (int i = 0; i < 8; i++)
		e[i] = (uint8_t)((uint64_t)k->e >> (56 - 8 * i));
	uint64_t r[N];
	mod_exp(m, r, xm, e, 8);
	to_bytes(m, em, k->nlen, r);
	return true;
}

static const uint8_t *digest_info(alt_hash_kind h, size_t *n) {
	static const uint8_t md5[] = {0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00, 0x04, 0x10};
	static const uint8_t sha1[] = {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
	static const uint8_t sha224[] = {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
	static const uint8_t sha256[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
	static const uint8_t sha384[] = {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30};
	static const uint8_t sha512[] = {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};
	static const uint8_t sha512_256[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x06, 0x05, 0x00, 0x04, 0x20};
#define P(x) (*n = sizeof x, x)
	switch (h) {
	case ALT_MD5: return P(md5);
	case ALT_SHA1: return P(sha1);
	case ALT_SHA224: return P(sha224);
	case ALT_SHA256: return P(sha256);
	case ALT_SHA384: return P(sha384);
	case ALT_SHA512: return P(sha512);
	case ALT_SHA512_256: return P(sha512_256);
	}
#undef P
	return 0;
}

/* pkcs1v15ConstructEM */
static bool pkcs1_em(size_t k, alt_hash_kind h, const uint8_t *hashed, size_t nh, uint8_t *em) {
	size_t np;
	const uint8_t *prefix = digest_info(h, &np);
	if (!prefix || nh != alt_hash_size(h) || k < np + nh + 2 + 8 + 1)
		return false;
	memset(em, 0, k);
	em[1] = 1;
	for (size_t i = 2; i < k - np - nh - 1; i++)
		em[i] = 0xff;
	memcpy(em + k - np - nh, prefix, np);
	memcpy(em + k - nh, hashed, nh);
	return true;
}

int alt_rsa_verify_pkcs1v15(const alt_rsa_public *k, alt_hash_kind h, const void *hashed, size_t nh, const uint8_t *sig,
	size_t nsig) {
	mod m;
	int rc = check_public(k, &m);
	if (rc < 0)
		return rc;
	if (nsig != k->nlen)
		return -ALT_EVERIFY;
	uint8_t em[ALT_RSA_MAX_BYTES], want[ALT_RSA_MAX_BYTES];
	if (!encrypt(k, &m, sig, em) || !pkcs1_em(k->nlen, h, hashed, nh, want))
		return -ALT_EVERIFY;
	return memcmp(em, want, k->nlen) == 0 ? 0 : -ALT_EVERIFY;
}

/* db ^= MGF1(seed) */
static void mgf1_xor(uint8_t *db, size_t n, alt_hash_kind h, const uint8_t *seed, size_t ns) {
	size_t hl = alt_hash_size(h);
	uint8_t out[ALT_HASH_MAX_SIZE];
	for (uint32_t ctr = 0, done = 0; done < n; ctr++) {
		uint8_t c[4] = {(uint8_t)(ctr >> 24), (uint8_t)(ctr >> 16), (uint8_t)(ctr >> 8), (uint8_t)ctr};
		alt_hash x;
		alt_hash_init(&x, h);
		alt_hash_write(&x, seed, ns);
		alt_hash_write(&x, c, 4);
		alt_hash_sum(&x, out);
		for (size_t i = 0; i < hl && done < n; i++)
			db[done++] ^= out[i];
	}
}

int alt_rsa_verify_pss(const alt_rsa_public *k, alt_hash_kind h, const void *hashed, size_t nh, const uint8_t *sig, size_t nsig,
	int salt_len) {
	mod m;
	int rc = check_public(k, &m);
	if (rc < 0)
		return rc;
	if (nsig != k->nlen)
		return -ALT_EVERIFY;
	int em_bits = bitlen(k->n, k->nlen) - 1;
	size_t em_len = (size_t)(em_bits + 7) / 8;
	uint8_t buf[ALT_RSA_MAX_BYTES];
	if (!encrypt(k, &m, sig, buf))
		return -ALT_EVERIFY;
	uint8_t *em = buf;
	size_t len = k->nlen;
	while (len > em_len && len > 0) {
		if (em[0] != 0)
			return -ALT_EVERIFY;
		em++, len--;
	}
	/* emsaPSSVerify */
	size_t hl = alt_hash_size(h);
	if (hl != nh)
		return -ALT_EVERIFY;
	if (salt_len >= 0 && em_len < hl + (size_t)salt_len + 2)
		return -ALT_EVERIFY;
	if (em_len < hl + 2 || em[em_len - 1] != 0xbc)
		return -ALT_EVERIFY;
	uint8_t *db = em;
	size_t dbn = em_len - hl - 1;
	const uint8_t *hh = em + dbn;
	uint8_t mask = (uint8_t)(0xff >> (8 * em_len - (size_t)em_bits));
	if (em[0] & ~mask)
		return -ALT_EVERIFY;
	mgf1_xor(db, dbn, h, hh, hl);
	db[0] &= mask;
	size_t slen;
	if (salt_len < 0) {
		const uint8_t *one = memchr(db, 1, dbn);
		if (!one)
			return -ALT_EVERIFY;
		slen = dbn - (size_t)(one - db) - 1;
	} else
		slen = (size_t)salt_len;
	if (em_len < hl + slen + 2)
		return -ALT_EVERIFY;
	size_t ps = em_len - hl - slen - 2;
	for (size_t i = 0; i < ps; i++)
		if (db[i])
			return -ALT_EVERIFY;
	if (db[ps] != 1)
		return -ALT_EVERIFY;
	alt_hash x;
	alt_hash_init(&x, h);
	static const uint8_t zero8[8];
	alt_hash_write(&x, zero8, 8);
	alt_hash_write(&x, hashed, nh);
	alt_hash_write(&x, db + dbn - slen, slen);
	uint8_t h0[ALT_HASH_MAX_SIZE];
	alt_hash_sum(&x, h0);
	return memcmp(h0, hh, hl) == 0 ? 0 : -ALT_EVERIFY;
}

/* em^d mod n by the CRT, checked with the public exponent */
static int private_op(const alt_rsa_private *k, const uint8_t *em, uint8_t *out) {
	mod mn, mp, mq;
	int rc = check_public(&k->pub, &mn);
	if (rc < 0)
		return rc;
	if (!mod_init(&mp, k->p, k->plen) || !mod_init(&mq, k->q, k->qlen))
		return -ALT_EKEY;
	size_t kn = k->pub.nlen;
	if (memcmp(em, k->pub.n, kn) >= 0)
		return -EINVAL;
	uint64_t cp[N], cq[N], m1[N], m2[N], h[N], t[N], qi[N];
	/* c mod p, c mod q in Montgomery form */
	reduce_bytes(&mp, t, em, kn);
	mont_mul(&mp, cp, t, mp.rr);
	reduce_bytes(&mq, t, em, kn);
	mont_mul(&mq, cq, t, mq.rr);
	mod_exp(&mp, m1, cp, k->dp, k->dplen);
	mod_exp(&mq, m2, cq, k->dq, k->dqlen);
	/* h = qinv (m1 - m2) mod p: m2 (mod q, normal form) reduced mod p */
	uint8_t m2b[ALT_RSA_MAX_BYTES];
	to_bytes(&mq, m2b, k->qlen, m2);
	reduce_bytes(&mp, t, m2b, k->qlen);
	uint64_t m2p[N], one[N] = {1}, m1n[N], d[N], u[N], bw = 0, c = 0;
	memcpy(m2p, t, sizeof t);
	mont_mul(&mp, m1n, m1, one); /* out of Montgomery form */
	for (int i = 0; i < mp.n; i++)
		d[i] = subb(m1n[i], m2p[i], &bw);
	for (int i = 0; i < mp.n; i++)
		u[i] = addc(d[i], mp.m[i], &c);
	sel(d, u, d, bw, mp.n);
	reduce_bytes(&mp, t, k->qinv, k->qinvlen);
	mont_mul(&mp, qi, t, mp.rr);
	mont_mul(&mp, h, d, qi); /* qinv (m1 - m2), normal form */
	/* m = m2 + h q (computed in mod n, where it is below n) */
	uint8_t hb[ALT_RSA_MAX_BYTES];
	uint64_t hn[N], qn[N], mm[N], m2n[N];
	for (size_t i = 0; i < k->plen; i++) {
		size_t bit = (k->plen - 1 - i) * 8;
		hb[i] = (uint8_t)(h[bit / 64] >> (bit % 64));
	}
	reduce_bytes(&mn, t, hb, k->plen);
	mont_mul(&mn, hn, t, mn.rr);
	reduce_bytes(&mn, qn, k->q, k->qlen);
	mont_mul(&mn, mm, hn, qn); /* h q, normal form */
	reduce_bytes(&mn, m2n, m2b, k->qlen);
	c = 0;
	bw = 0;
	for (int i = 0; i < mn.n; i++)
		t[i] = addc(mm[i], m2n[i], &c);
	for (int i = 0; i < mn.n; i++)
		u[i] = subb(t[i], mn.m[i], &bw);
	sel(t, u, t, c | (bw ^ 1), mn.n);
	for (size_t i = 0; i < kn; i++) {
		size_t bit = (kn - 1 - i) * 8;
		out[i] = (uint8_t)(t[bit / 64] >> (bit % 64));
	}
	/* the check: out^e must be em */
	uint8_t back[ALT_RSA_MAX_BYTES];
	if (!encrypt(&k->pub, &mn, out, back) || memcmp(back, em, kn) != 0) {
		memset(out, 0, kn);
		return -ALT_EKEY;
	}
	return 0;
}

int alt_rsa_sign_pkcs1v15(const alt_rsa_private *k, alt_hash_kind h, const void *hashed, size_t nh, uint8_t *sig) {
	uint8_t em[ALT_RSA_MAX_BYTES];
	if (k->pub.nlen > ALT_RSA_MAX_BYTES || !pkcs1_em(k->pub.nlen, h, hashed, nh, em))
		return -EINVAL;
	return private_op(k, em, sig);
}

int alt_rsa_sign_pss(const alt_rsa_private *k, alt_hash_kind h, const void *hashed, size_t nh, uint8_t *sig, int salt_len) {
	size_t hl = alt_hash_size(h), kn = k->pub.nlen;
	if (nh != hl || kn > ALT_RSA_MAX_BYTES)
		return -EINVAL;
	int em_bits = bitlen(k->pub.n, kn) - 1;
	size_t em_len = (size_t)(em_bits + 7) / 8, slen = salt_len < 0 ? hl : (size_t)salt_len;
	if (em_len < hl + slen + 2)
		return -EINVAL;
	uint8_t salt[ALT_RSA_MAX_BYTES], em[ALT_RSA_MAX_BYTES] = {0};
	int rc = alt_crypto_rand(salt, slen);
	if (rc < 0)
		return rc;
	/* M' = 0^8 || mHash || salt; H = Hash(M') */
	uint8_t H[ALT_HASH_MAX_SIZE];
	alt_hash x;
	alt_hash_init(&x, h);
	static const uint8_t zero8[8];
	alt_hash_write(&x, zero8, 8);
	alt_hash_write(&x, hashed, nh);
	alt_hash_write(&x, salt, slen);
	alt_hash_sum(&x, H);
	/* EM = maskedDB || H || 0xbc, in the last em_len bytes of k */
	uint8_t *e = em + (kn - em_len);
	size_t dbn = em_len - hl - 1, ps = em_len - slen - hl - 2;
	e[ps] = 1;
	memcpy(e + ps + 1, salt, slen);
	mgf1_xor(e, dbn, h, H, hl);
	e[0] &= (uint8_t)(0xff >> (8 * em_len - (size_t)em_bits));
	memcpy(e + dbn, H, hl);
	e[em_len - 1] = 0xbc;
	return private_op(k, em, sig);
}

/* ---- encryption ---- */

/* x^e mod n of a k-byte big-endian block below n */
static int public_op(const alt_rsa_public *k, const uint8_t *in, uint8_t *out) {
	mod m;
	int rc = check_public(k, &m);
	if (rc < 0)
		return rc;
	if (memcmp(in, k->n, k->nlen) >= 0)
		return -EINVAL;
	return encrypt(k, &m, in, out) ? 0 : -ALT_EKEY;
}

int alt_rsa_encrypt_oaep(const alt_rsa_public *k, alt_hash_kind h, const void *msg, size_t n, const void *label, size_t nlabel,
	uint8_t *ct) {
	size_t kn = k->nlen, hl = alt_hash_size(h);
	if (kn > ALT_RSA_MAX_BYTES)
		return -ALT_EKEY;
	if (kn < 2 * hl + 2 || n > kn - 2 * hl - 2)
		return -EMSGSIZE;
	uint8_t em[ALT_RSA_MAX_BYTES];
	/* EM = 0 || seed || DB; DB = lHash || PS || 1 || M */
	em[0] = 0;
	uint8_t *seed = em + 1, *db = em + 1 + hl;
	size_t dbn = kn - hl - 1;
	alt_hash_data(h, label, nlabel, db);
	memset(db + hl, 0, dbn - hl - n - 1);
	db[dbn - n - 1] = 1;
	memcpy(db + dbn - n, msg, n);
	int rc = alt_crypto_rand(seed, hl);
	if (rc < 0)
		return rc;
	mgf1_xor(db, dbn, h, seed, hl);
	mgf1_xor(seed, hl, h, db, dbn);
	rc = public_op(k, em, ct);
	memset(em, 0, sizeof em);
	return rc;
}

int alt_rsa_encrypt_pkcs1v15(const alt_rsa_public *k, const void *msg, size_t n, uint8_t *ct) {
	size_t kn = k->nlen;
	if (kn > ALT_RSA_MAX_BYTES)
		return -ALT_EKEY;
	if (kn < 11 || n > kn - 11)
		return -EMSGSIZE;
	/* EM = 0 || 2 || PS (nonzero random) || 0 || M */
	uint8_t em[ALT_RSA_MAX_BYTES];
	em[0] = 0, em[1] = 2;
	size_t ps = kn - n - 3;
	int rc = alt_crypto_rand(em + 2, ps);
	for (size_t i = 0; rc == 0 && i < ps; i++)
		while (em[2 + i] == 0 && (rc = alt_crypto_rand(em + 2 + i, 1)) == 0) {
		}
	if (rc < 0)
		return rc;
	em[2 + ps] = 0;
	memcpy(em + 3 + ps, msg, n);
	rc = public_op(k, em, ct);
	memset(em, 0, sizeof em);
	return rc;
}

/* constant-time helpers: all ones when the condition holds */
static uint64_t ct_is_zero(uint64_t x) { return ((~x & (x - 1)) >> 63) * UINT64_MAX; }
static uint64_t ct_eq(uint64_t a, uint64_t b) { return ct_is_zero(a ^ b); }

int alt_rsa_decrypt_oaep(const alt_rsa_private *k, alt_hash_kind h, const uint8_t *ct, size_t nct, const void *label, size_t nlabel,
	uint8_t *out, size_t *nout) {
	size_t kn = k->pub.nlen, hl = alt_hash_size(h);
	if (kn > ALT_RSA_MAX_BYTES || nct > kn || kn < 2 * hl + 2)
		return -ALT_EAUTH;
	uint8_t c[ALT_RSA_MAX_BYTES] = {0}, em[ALT_RSA_MAX_BYTES];
	memcpy(c + kn - nct, ct, nct);
	if (private_op(k, c, em) < 0)
		return -ALT_EAUTH;
	uint8_t lhash[ALT_HASH_MAX_SIZE];
	alt_hash_data(h, label, nlabel, lhash);
	uint8_t *seed = em + 1, *db = em + 1 + hl;
	size_t dbn = kn - hl - 1;
	mgf1_xor(seed, hl, h, db, dbn);
	mgf1_xor(db, dbn, h, seed, hl);
	/* everything checked without branches on the secret (Go's decryptOAEP) */
	uint64_t good = ct_is_zero(em[0]) & (alt_ct_eq(db, lhash, hl) ? UINT64_MAX : 0);
	uint64_t looking = UINT64_MAX, index = 0, invalid = 0;
	for (size_t i = hl; i < dbn; i++) {
		uint64_t eq0 = ct_is_zero(db[i]), eq1 = ct_eq(db[i], 1);
		index = (looking & eq1 & i) | (~(looking & eq1) & index);
		looking &= ~eq1;
		invalid |= looking & ~eq0;
	}
	good &= ~invalid & ~looking;
	if (!good) {
		memset(em, 0, sizeof em);
		return -ALT_EAUTH;
	}
	size_t mlen = dbn - (size_t)index - 1;
	memcpy(out, db + index + 1, mlen);
	*nout = mlen;
	memset(em, 0, sizeof em);
	return 0;
}

int alt_rsa_decrypt_pkcs1v15(const alt_rsa_private *k, const uint8_t *ct, size_t nct, uint8_t *out, size_t *nout) {
	size_t kn = k->pub.nlen;
	if (kn > ALT_RSA_MAX_BYTES || nct > kn || kn < 11)
		return -ALT_EAUTH;
	uint8_t c[ALT_RSA_MAX_BYTES] = {0}, em[ALT_RSA_MAX_BYTES];
	memcpy(c + kn - nct, ct, nct);
	if (private_op(k, c, em) < 0)
		return -ALT_EAUTH;
	uint64_t good = ct_is_zero(em[0]) & ct_eq(em[1], 2), looking = UINT64_MAX, index = 0;
	for (size_t i = 2; i < kn; i++) {
		uint64_t eq0 = ct_is_zero(em[i]);
		index = (looking & eq0 & i) | (~(looking & eq0) & index);
		looking &= ~eq0;
	}
	/* at least 8 bytes of padding (Go's decryptPKCS1v15) */
	good &= ~looking & ((((index - 10) >> 63) ^ 1) * UINT64_MAX);
	if (!good) {
		memset(em, 0, sizeof em);
		return -ALT_EAUTH;
	}
	size_t mlen = kn - (size_t)index - 1;
	memcpy(out, em + index + 1, mlen);
	*nout = mlen;
	memset(em, 0, sizeof em);
	return 0;
}

/* ---- key generation ---- */

static int to_be(const alt_big *x, uint8_t *out, size_t cap, size_t *n) {
	*n = alt_big_bytes_len(x);
	if (*n > cap)
		return -ERANGE;
	alt_big_bytes(x, out);
	return 0;
}

int alt_rsa_generate(alt_rsa_key *out, int bits) {
	if (bits < 1024 || bits > 8 * ALT_RSA_MAX_BYTES)
		return -EINVAL;
	alt_big p = ALT_BIG_INIT, q = ALT_BIG_INIT, n = ALT_BIG_INIT, e = ALT_BIG_INIT, d = ALT_BIG_INIT, p1 = ALT_BIG_INIT,
		q1 = ALT_BIG_INIT, g = ALT_BIG_INIT, l = ALT_BIG_INIT, t = ALT_BIG_INIT, dp = ALT_BIG_INIT, dq = ALT_BIG_INIT,
		qi = ALT_BIG_INIT, one = ALT_BIG_INIT;
	int rc = alt_big_set_int64(&e, 65537);
	if (rc == 0)
		rc = alt_big_set_int64(&one, 1);
	while (rc == 0) {
		if ((rc = alt_big_random_prime(&p, (size_t)(bits + 1) / 2)) < 0 || (rc = alt_big_random_prime(&q, (size_t)bits / 2)) < 0)
			break;
		if ((rc = alt_big_mul(&n, &p, &q)) < 0)
			break;
		if (alt_big_bit_len(&n) != (size_t)bits)
			continue;
		/* |p - q| > 2^(bits/2 - 100) (FIPS 186-5 A.1.3) */
		if ((rc = alt_big_sub(&t, &p, &q)) < 0)
			break;
		if (alt_big_bit_len(&t) <= (size_t)(bits / 2 - 100))
			continue;
		/* d = e^-1 mod lcm(p-1, q-1) */
		alt_big_sub(&p1, &p, &one);
		alt_big_sub(&q1, &q, &one);
		if ((rc = alt_big_gcd(&g, 0, 0, &p1, &q1)) < 0 || (rc = alt_big_mul(&l, &p1, &q1)) < 0 || (rc = alt_big_quo(&l, &l, &g)) < 0)
			break;
		rc = alt_big_mod_inverse(&d, &e, &l);
		if (rc == -EDOM) {
			rc = 0;
			continue; /* e divides p-1 or q-1 */
		}
		if (rc < 0)
			break;
		/* d > 2^(bits/2) */
		if (alt_big_bit_len(&d) <= (size_t)(bits / 2))
			continue;
		if ((rc = alt_big_mod(&dp, &d, &p1)) < 0 || (rc = alt_big_mod(&dq, &d, &q1)) < 0 || (rc = alt_big_mod_inverse(&qi, &q, &p)) < 0)
			break;
		memset(out, 0, sizeof *out);
		alt_rsa_private *k = &out->k;
		size_t ln;
		if ((rc = to_be(&n, out->n, sizeof out->n, &k->pub.nlen)) < 0 || (rc = to_be(&d, out->d, sizeof out->d, &k->dlen)) < 0 ||
			(rc = to_be(&p, out->p, sizeof out->p, &k->plen)) < 0 || (rc = to_be(&q, out->q, sizeof out->q, &k->qlen)) < 0 ||
			(rc = to_be(&dp, out->dp, sizeof out->dp, &k->dplen)) < 0 || (rc = to_be(&dq, out->dq, sizeof out->dq, &k->dqlen)) < 0 ||
			(rc = to_be(&qi, out->qinv, sizeof out->qinv, &ln)) < 0)
			break;
		k->qinvlen = ln;
		k->pub.n = out->n, k->pub.e = 65537;
		k->d = out->d, k->p = out->p, k->q = out->q, k->dp = out->dp, k->dq = out->dq, k->qinv = out->qinv;
		break;
	}
	alt_big *all[] = {&p, &q, &n, &e, &d, &p1, &q1, &g, &l, &t, &dp, &dq, &qi, &one};
	for (size_t i = 0; i < sizeof all / sizeof *all; i++) {
		if (all[i]->w)
			memset(all[i]->w, 0, all[i]->cap * sizeof(uint64_t));
		alt_big_free(all[i]);
	}
	return rc;
}

int alt_rsa_marshal_public_pkcs1(const alt_rsa_public *k, alt_buf *b) {
	alt_der_builder d;
	alt_der_builder_init(&d, b);
	alt_der_begin(&d, ALT_ASN1_SEQUENCE);
	alt_der_add_uint_bytes(&d, k->n, k->nlen);
	alt_der_add_int64(&d, k->e);
	alt_der_end(&d);
	return alt_der_builder_err(&d);
}

int alt_rsa_marshal_pkcs1(const alt_rsa_private *k, alt_buf *b) {
	alt_der_builder d;
	alt_der_builder_init(&d, b);
	alt_der_begin(&d, ALT_ASN1_SEQUENCE);
	alt_der_add_int64(&d, 0);
	alt_der_add_uint_bytes(&d, k->pub.n, k->pub.nlen);
	alt_der_add_int64(&d, k->pub.e);
	alt_der_add_uint_bytes(&d, k->d, k->dlen);
	alt_der_add_uint_bytes(&d, k->p, k->plen);
	alt_der_add_uint_bytes(&d, k->q, k->qlen);
	alt_der_add_uint_bytes(&d, k->dp, k->dplen);
	alt_der_add_uint_bytes(&d, k->dq, k->dqlen);
	alt_der_add_uint_bytes(&d, k->qinv, k->qinvlen);
	alt_der_end(&d);
	return alt_der_builder_err(&d);
}

int alt_rsa_marshal_pkcs8(const alt_rsa_private *k, alt_buf *b) {
	alt_buf inner = ALT_BUF_INIT;
	int rc = alt_rsa_marshal_pkcs1(k, &inner);
	if (rc == 0) {
		alt_der_builder d;
		alt_der_builder_init(&d, b);
		alt_der_begin(&d, ALT_ASN1_SEQUENCE);
		alt_der_add_int64(&d, 0);
		alt_der_begin(&d, ALT_ASN1_SEQUENCE);
		alt_der_add_oid(&d, &ALT_OID(1, 2, 840, 113549, 1, 1, 1));
		alt_der_add_null(&d);
		alt_der_end(&d);
		alt_der_add(&d, ALT_ASN1_OCTET_STRING, inner.p, inner.n);
		alt_der_end(&d);
		rc = alt_der_builder_err(&d);
	}
	if (inner.p)
		memset(inner.p, 0, inner.n);
	alt_buf_free(&inner);
	return rc;
}
