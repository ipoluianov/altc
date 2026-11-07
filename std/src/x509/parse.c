/* Certificate parsing, translated from Go's crypto/x509/parser.go
   (BSD-3-Clause, see LICENSE-Go): the same checks in the same order,
   with Go's error messages. */
#include <alt/crypto/x509.h>
#include <alt/net.h>
#include <alt/net/url.h>
#include <alt/strconv.h>
#include <alt/unicode/utf8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	alt_arena *a;
	const char *why;
	uint32_t pk_oid_last; /* the last arc of the public key algorithm */
} ctx;

#define FAIL(msg) \
	do { \
		c->why = (msg); \
		return false; \
	} while (0)

/* growable arena arrays */
#define PUSH(c, arr, n, val) \
	do { \
		void *p_ = alt_arena_realloc((c)->a, (arr), (n) * sizeof *(arr), ((n) + 1) * sizeof *(arr)); \
		if (!p_) \
			FAIL("out of memory"); \
		(arr) = p_; \
		(arr)[(n)++] = (val); \
	} while (0)

static const unsigned CTX0 = ALT_ASN1_CONTEXT, CTX0C = ALT_ASN1_CONTEXT | ALT_ASN1_CONSTRUCTED;

static bool is_printable(uint8_t b) {
	return ('a' <= b && b <= 'z') || ('A' <= b && b <= 'Z') || ('0' <= b && b <= '9') || ('\'' <= b && b <= ')') ||
	       ('+' <= b && b <= '/') || b == ' ' || b == ':' || b == '=' || b == '?' || b == '*' || b == '&';
}

static bool is_ia5(alt_der v) {
	for (size_t i = 0; i < v.n; i++)
		if (v.p[i] >= 0x80)
			return false;
	return true;
}

static alt_str view(alt_der d) { return alt_str_make(d.p, d.n); }

/* a message with a value quoted as Go's %q does */
static bool failq(ctx *c, const char *prefix, alt_str v, const char *suffix) {
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, prefix);
	alt_quote(&b, v);
	alt_buf_append_cstr(&b, suffix);
	c->why = alt_arena_strdup(c->a, alt_buf_cstr(&b));
	alt_buf_free(&b);
	return false;
}

/* parseASN1String: the value as UTF-8 */
static bool asn1_string(ctx *c, unsigned tag, alt_der v, alt_str *out) {
	alt_buf b = ALT_BUF_INIT;
	switch (tag) {
	case ALT_ASN1_T61_STRING:
		for (size_t i = 0; i < v.n; i++) {
			char u[4];
			alt_buf_append(&b, u, alt_utf8_encode(u, v.p[i]));
		}
		break;
	case ALT_ASN1_PRINTABLE_STRING:
		for (size_t i = 0; i < v.n; i++)
			if (!is_printable(v.p[i]))
				FAIL("invalid PrintableString");
		*out = view(v);
		return true;
	case ALT_ASN1_UTF8_STRING:
		if (!alt_utf8_valid((const char *)v.p, v.n))
			FAIL("invalid UTF-8 string");
		*out = view(v);
		return true;
	case ALT_ASN1_BMP_STRING: {
		if (v.n % 2)
			FAIL("invalid BMPString");
		size_t n = v.n;
		if (n >= 2 && v.p[n - 1] == 0 && v.p[n - 2] == 0)
			n -= 2;
		for (size_t i = 0; i < n; i += 2) {
			unsigned pt = (unsigned)v.p[i] << 8 | v.p[i + 1];
			if (pt == 0xfffe || pt == 0xffff || (pt >= 0xfdd0 && pt <= 0xfdef) || (pt >= 0xd800 && pt <= 0xdfff)) {
				alt_buf_free(&b);
				FAIL("invalid BMPString");
			}
			char u[4];
			alt_buf_append(&b, u, alt_utf8_encode(u, (int32_t)pt));
		}
		break;
	}
	case ALT_ASN1_IA5_STRING:
		if (!is_ia5(v))
			FAIL("invalid IA5String");
		*out = view(v);
		return true;
	case 18: /* NumericString */
		for (size_t i = 0; i < v.n; i++)
			if (!(('0' <= v.p[i] && v.p[i] <= '9') || v.p[i] == ' '))
				FAIL("invalid NumericString");
		*out = view(v);
		return true;
	default:
		FAIL("unsupported string type");
	}
	*out = alt_arena_str(c->a, alt_buf_str(&b));
	alt_buf_free(&b);
	return true;
}

/* readASN1Any for attribute values: strings decoded, others checked */
static bool asn1_any(ctx *c, alt_der *der, unsigned *tag, alt_str *value) {
	alt_der full, inner;
	unsigned t;
	if (!alt_der_read_any_element(der, &full, &t))
		FAIL("invalid ASN.1 element");
	*tag = t;
	switch (t) {
	case ALT_ASN1_T61_STRING:
	case ALT_ASN1_PRINTABLE_STRING:
	case ALT_ASN1_UTF8_STRING:
	case ALT_ASN1_BMP_STRING:
	case ALT_ASN1_IA5_STRING:
	case 18:
		if (!alt_der_read(&full, &inner, t))
			FAIL("invalid ASN.1 element");
		return asn1_string(c, t, inner, value);
	case ALT_ASN1_INTEGER: {
		int64_t i;
		alt_der f2 = full;
		if (!alt_der_read_int64(&f2, &i))
			FAIL("invalid ASN.1 integer");
		break;
	}
	case ALT_ASN1_BIT_STRING: {
		size_t bits;
		alt_der f2 = full;
		if (!alt_der_read_bit_string(&f2, &inner, &bits))
			FAIL("invalid ASN.1 BIT STRING");
		break;
	}
	case ALT_ASN1_OID: {
		alt_oid o;
		alt_der f2 = full;
		if (!alt_der_read_oid(&f2, &o))
			FAIL("invalid ASN.1 OBJECT IDENTIFIER");
		break;
	}
	case ALT_ASN1_UTC_TIME:
	case ALT_ASN1_GENERALIZED_TIME: {
		int64_t tm;
		alt_der f2 = full;
		if (t == ALT_ASN1_UTC_TIME ? !alt_der_read_utc_time(&f2, &tm) : !alt_der_read_generalized_time(&f2, &tm))
			FAIL(t == ALT_ASN1_UTC_TIME ? "x509: malformed UTCTime" : "x509: malformed GeneralizedTime");
		break;
	}
	case ALT_ASN1_BOOLEAN: {
		bool b;
		alt_der f2 = full;
		if (!alt_der_read_bool(&f2, &b))
			FAIL("invalid ASN.1 BOOLEAN");
		break;
	}
	default:
		break;
	}
	alt_der f2 = full;
	unsigned t2;
	if (!alt_der_read_any(&f2, &inner, &t2))
		FAIL("invalid ASN.1 element");
	*value = view(inner);
	return true;
}

static bool parse_name(ctx *c, alt_der raw, alt_x509_name *out) {
	out->attrs = 0;
	out->n = 0;
	if (!alt_der_read(&raw, &raw, ALT_ASN1_SEQUENCE))
		FAIL("x509: invalid RDNSequence");
	for (int set_i = 0; !alt_der_empty(raw); set_i++) {
		alt_der set;
		if (!alt_der_read(&raw, &set, ALT_ASN1_SET))
			FAIL("x509: invalid RDNSequence");
		while (!alt_der_empty(set)) {
			alt_der atav;
			if (!alt_der_read(&set, &atav, ALT_ASN1_SEQUENCE))
				FAIL("x509: invalid RDNSequence: invalid attribute");
			alt_x509_attr at = {.set = set_i};
			if (!alt_der_read_oid(&atav, &at.type))
				FAIL("x509: invalid RDNSequence: invalid attribute type");
			if (!asn1_any(c, &atav, &at.tag, &at.value)) {
				c->why = alt_arena_printf(c->a, "x509: invalid RDNSequence: invalid attribute value: %s", c->why);
				return false;
			}
			PUSH(c, out->attrs, out->n, at);
		}
	}
	return true;
}

/* parseAI: the algorithm OID and the parameters element (empty when absent) */
static bool parse_ai(ctx *c, alt_der der, alt_oid *alg, alt_der *params, unsigned *ptag) {
	if (!alt_der_read_oid(&der, alg))
		FAIL("x509: malformed OID");
	*params = alt_der_make(0, 0);
	*ptag = 0;
	if (alt_der_empty(der))
		return true;
	if (!alt_der_read_any_element(&der, params, ptag))
		FAIL("x509: malformed parameters");
	return true;
}

static bool read_time(ctx *c, alt_der *der, int64_t *t) {
	if (alt_der_peek(*der, ALT_ASN1_UTC_TIME)) {
		if (!alt_der_read_utc_time(der, t))
			FAIL("x509: malformed UTCTime");
	} else if (alt_der_peek(*der, ALT_ASN1_GENERALIZED_TIME)) {
		if (!alt_der_read_generalized_time(der, t))
			FAIL("x509: malformed GeneralizedTime");
	} else
		FAIL("x509: unsupported time format");
	return true;
}

#define OIDEQ(o, ...) alt_oid_eq((o), &ALT_OID(__VA_ARGS__))

static const struct {
	alt_x509_sig_alg alg;
	alt_oid oid;
} sig_oids[] = {
	{ALT_X509_MD5_RSA, ALT_OID(1, 2, 840, 113549, 1, 1, 4)},
	{ALT_X509_SHA1_RSA, ALT_OID(1, 2, 840, 113549, 1, 1, 5)},
	{ALT_X509_SHA1_RSA, ALT_OID(1, 3, 14, 3, 2, 29)},
	{ALT_X509_SHA256_RSA, ALT_OID(1, 2, 840, 113549, 1, 1, 11)},
	{ALT_X509_SHA384_RSA, ALT_OID(1, 2, 840, 113549, 1, 1, 12)},
	{ALT_X509_SHA512_RSA, ALT_OID(1, 2, 840, 113549, 1, 1, 13)},
	{ALT_X509_DSA_SHA1, ALT_OID(1, 2, 840, 10040, 4, 3)},
	{ALT_X509_DSA_SHA256, ALT_OID(2, 16, 840, 1, 101, 3, 4, 3, 2)},
	{ALT_X509_ECDSA_SHA1, ALT_OID(1, 2, 840, 10045, 4, 1)},
	{ALT_X509_ECDSA_SHA256, ALT_OID(1, 2, 840, 10045, 4, 3, 2)},
	{ALT_X509_ECDSA_SHA384, ALT_OID(1, 2, 840, 10045, 4, 3, 3)},
	{ALT_X509_ECDSA_SHA512, ALT_OID(1, 2, 840, 10045, 4, 3, 4)},
	{ALT_X509_ED25519, ALT_OID(1, 3, 101, 112)},
};

/* an AlgorithmIdentifier inside an EXPLICIT tag (pssParameters fields) */
static bool explicit_ai(alt_der *s, unsigned tag, alt_oid *alg, alt_der *params) {
	alt_der e, seq;
	if (!alt_der_read(s, &e, tag | ALT_ASN1_CONTEXT | ALT_ASN1_CONSTRUCTED) || !alt_der_read(&e, &seq, ALT_ASN1_SEQUENCE) ||
		!alt_der_empty(e) || !alt_der_read_oid(&seq, alg))
		return false;
	*params = alt_der_make(0, 0);
	if (!alt_der_empty(seq)) {
		unsigned t;
		if (!alt_der_read_any_element(&seq, params, &t) || !alt_der_empty(seq))
			return false;
	}
	return true;
}

static bool null_or_absent(alt_der p) { return p.n == 0 || (p.n == 2 && p.p[0] == 5 && p.p[1] == 0); }

/* getSignatureAlgorithmFromAI */
static alt_x509_sig_alg sig_alg(const alt_oid *alg, alt_der params) {
	if (OIDEQ(alg, 1, 3, 101, 112) && params.n)
		return ALT_X509_SIG_UNKNOWN;
	if (!OIDEQ(alg, 1, 2, 840, 113549, 1, 1, 10)) {
		for (size_t i = 0; i < sizeof sig_oids / sizeof *sig_oids; i++)
			if (alt_oid_eq(alg, &sig_oids[i].oid))
				return sig_oids[i].alg;
		return ALT_X509_SIG_UNKNOWN;
	}
	/* RSASSA-PSS-params: hash [0], mgf [1], salt length [2], trailer [3] OPTIONAL (default 1) */
	alt_der seq, e;
	alt_oid hash, mgf, mgf_hash;
	alt_der hp, mp, mhp;
	int64_t salt, trailer = 1;
	if (!alt_der_read(&params, &seq, ALT_ASN1_SEQUENCE) || !explicit_ai(&seq, 0, &hash, &hp) || !explicit_ai(&seq, 1, &mgf, &mp) ||
		!alt_der_read(&seq, &e, 2 | ALT_ASN1_CONTEXT | ALT_ASN1_CONSTRUCTED) || !alt_der_read_int64(&e, &salt) || !alt_der_empty(e))
		return ALT_X509_SIG_UNKNOWN;
	if (alt_der_peek(seq, 3 | ALT_ASN1_CONTEXT | ALT_ASN1_CONSTRUCTED)) {
		if (!alt_der_read(&seq, &e, 3 | ALT_ASN1_CONTEXT | ALT_ASN1_CONSTRUCTED) || !alt_der_read_int64(&e, &trailer) ||
			!alt_der_empty(e))
			return ALT_X509_SIG_UNKNOWN;
	}
	/* the MGF's parameters: an AlgorithmIdentifier */
	alt_der ms;
	if (!alt_der_read(&mp, &ms, ALT_ASN1_SEQUENCE) || !alt_der_read_oid(&ms, &mgf_hash))
		return ALT_X509_SIG_UNKNOWN;
	mhp = ms;
	if (!null_or_absent(hp) || !OIDEQ(&mgf, 1, 2, 840, 113549, 1, 1, 8) || !alt_oid_eq(&mgf_hash, &hash) || !null_or_absent(mhp) ||
		trailer != 1)
		return ALT_X509_SIG_UNKNOWN;
	if (OIDEQ(&hash, 2, 16, 840, 1, 101, 3, 4, 2, 1) && salt == 32)
		return ALT_X509_SHA256_RSAPSS;
	if (OIDEQ(&hash, 2, 16, 840, 1, 101, 3, 4, 2, 2) && salt == 48)
		return ALT_X509_SHA384_RSAPSS;
	if (OIDEQ(&hash, 2, 16, 840, 1, 101, 3, 4, 2, 3) && salt == 64)
		return ALT_X509_SHA512_RSAPSS;
	return ALT_X509_SIG_UNKNOWN;
}

static alt_x509_pk_alg pk_alg(const alt_oid *o) {
	if (OIDEQ(o, 1, 2, 840, 113549, 1, 1, 1))
		return ALT_X509_PK_RSA;
	if (OIDEQ(o, 1, 2, 840, 10040, 4, 1))
		return ALT_X509_PK_DSA;
	if (OIDEQ(o, 1, 2, 840, 10045, 2, 1))
		return ALT_X509_PK_ECDSA;
	if (OIDEQ(o, 1, 3, 101, 112))
		return ALT_X509_PK_ED25519;
	if (OIDEQ(o, 2, 16, 840, 1, 101, 3, 4, 3, 17) || OIDEQ(o, 2, 16, 840, 1, 101, 3, 4, 3, 18) ||
		OIDEQ(o, 2, 16, 840, 1, 101, 3, 4, 3, 19))
		return ALT_X509_PK_MLDSA;
	return ALT_X509_PK_UNKNOWN;
}

const char *alt__ec_check_point(int curve, const uint8_t *b, size_t n);

static bool parse_public_key(ctx *c, alt_x509_cert *cert, alt_der params, alt_der data) {
	switch (cert->pk_alg) {
	case ALT_X509_PK_RSA: {
		if (!(params.n == 2 && params.p[0] == 5 && params.p[1] == 0))
			FAIL("x509: RSA key missing NULL parameters");
		alt_der der = data, n;
		if (!alt_der_read(&der, &der, ALT_ASN1_SEQUENCE))
			FAIL("x509: invalid RSA public key");
		if (!alt_der_read_int_bytes(&der, &n))
			FAIL("x509: invalid RSA modulus");
		if (!alt_der_read_int64(&der, &cert->pub.e))
			FAIL("x509: invalid RSA public exponent");
		if ((n.p[0] & 0x80) || (n.n == 1 && n.p[0] == 0))
			FAIL("x509: RSA modulus is not a positive number");
		if (cert->pub.e <= 0)
			FAIL("x509: RSA public exponent is not a positive number");
		while (n.n > 1 && n.p[0] == 0)
			n.p++, n.n--;
		cert->pub.n = n;
		return true;
	}
	case ALT_X509_PK_ECDSA: {
		alt_der pd = params;
		alt_oid curve;
		if (!alt_der_read_oid(&pd, &curve))
			FAIL("x509: invalid ECDSA parameters");
		size_t size;
		if (OIDEQ(&curve, 1, 2, 840, 10045, 3, 1, 7))
			cert->pub.curve = ALT_X509_P256, size = 32;
		else if (OIDEQ(&curve, 1, 3, 132, 0, 34))
			cert->pub.curve = ALT_X509_P384, size = 48;
		else if (OIDEQ(&curve, 1, 3, 132, 0, 35))
			cert->pub.curve = ALT_X509_P521, size = 66;
		else
			FAIL("x509: unsupported elliptic curve");
		if (data.n < 1 || data.p[0] != 4)
			FAIL("ecdsa: invalid uncompressed public key");
		(void)size;
		const char *bad = alt__ec_check_point(cert->pub.curve, data.p, data.n);
		if (bad)
			FAIL(bad);
		cert->pub.point = data;
		return true;
	}
	case ALT_X509_PK_ED25519:
		if (params.n)
			FAIL("x509: Ed25519 key encoded with illegal parameters");
		if (data.n != 32)
			FAIL("x509: wrong Ed25519 public key size");
		cert->pub.point = data;
		return true;
	case ALT_X509_PK_MLDSA: {
		if (params.n)
			FAIL("x509: ML-DSA key encoded with illegal parameters");
		/* the sizes of ML-DSA-44, -65 and -87 keys (FIPS 204) */
		static const size_t sizes[3] = {1312, 1952, 2592};
		if (data.n != sizes[c->pk_oid_last - 17])
			FAIL("mldsa: invalid public key length");
		cert->pub.point = data;
		return true;
	}
	case ALT_X509_PK_DSA: {
		alt_der der = data, y, pp = params, p, q, g;
		if (!alt_der_read_int_bytes(&der, &y))
			FAIL("x509: invalid DSA public key");
		if (!alt_der_read(&pp, &pp, ALT_ASN1_SEQUENCE) || !alt_der_read_int_bytes(&pp, &p) || !alt_der_read_int_bytes(&pp, &q) ||
			!alt_der_read_int_bytes(&pp, &g))
			FAIL("x509: invalid DSA parameters");
		alt_der all[4] = {y, p, q, g};
		for (int i = 0; i < 4; i++)
			if ((all[i].p[0] & 0x80) || (all[i].n == 1 && all[i].p[0] == 0))
				FAIL("x509: zero or negative DSA parameter");
		cert->pub.point = y;
		return true;
	}
	default:
		FAIL("x509: unknown public key algorithm");
	}
}

/* Go's domainNameValid */
static bool domain_valid(alt_str s, bool constraint) {
	if (s.n == 0)
		return true;
	if (s.p[s.n - 1] == '.')
		return false;
	if (constraint && s.p[0] == '.')
		s = alt_str_slice(s, 1, s.n);
	long last = -1;
	for (size_t i = 0; i <= s.n; i++) {
		if (i < s.n && ((unsigned char)s.p[i] < 33 || (unsigned char)s.p[i] > 126))
			return false;
		if (i == s.n || s.p[i] == '.') {
			long len = (long)i - (last >= 0 ? last + 1 : 0);
			if (len == 0)
				return false;
			last = (long)i;
		}
	}
	return true;
}

/* Go's parseRFC2821Mailbox (only whether it parses) */
static bool rfc2821_mailbox(alt_str in) {
	if (in.n == 0)
		return false;
	size_t i = 0, local = 0;
	char prev = 0;
	bool dots = false, first_dot = false;
	char last = 0;
	if (in.p[0] == '"') {
		i = 1;
		for (;;) {
			if (i >= in.n)
				return false;
			unsigned char ch = (unsigned char)in.p[i++];
			if (ch == '"')
				break;
			if (ch == '\\') {
				if (i >= in.n)
					return false;
				unsigned char n = (unsigned char)in.p[i];
				if (n == 11 || n == 12 || (1 <= n && n <= 9) || (14 <= n && n <= 127))
					i++;
				else
					return false;
			} else if (!(ch == 11 || ch == 12 || ch == 32 || ch == 33 || ch == 127 || (1 <= ch && ch <= 8) || (14 <= ch && ch <= 31) ||
					   (35 <= ch && ch <= 91) || (93 <= ch && ch <= 126)))
				return false;
		}
	} else {
		while (i < in.n) {
			unsigned char ch = (unsigned char)in.p[i];
			if (ch == '\\') {
				i++;
				if (i >= in.n)
					return false;
				ch = (unsigned char)in.p[i]; /* the escaped byte, kept as it is */
				goto take;
			}
			if (!(('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || strchr("!#$%&'*+-/=?^_`{|}~.", ch)) ||
				ch == 0)
				break;
		take:
			if (local == 0)
				first_dot = ch == '.';
			if (ch == '.' && prev == '.')
				dots = true;
			prev = (char)ch;
			last = (char)ch;
			local++;
			i++;
		}
		if (local == 0 || first_dot || last == '.' || dots)
			return false;
	}
	if (i >= in.n || in.p[i] != '@')
		return false;
	alt_str dom = alt_str_slice(in, i + 1, in.n);
	return domain_valid(dom, false) && alt_str_index_byte(dom, '@') < 0;
}

static bool push_str(ctx *c, alt_x509_strs *l, alt_str s) {
	PUSH(c, l->items, l->n, s);
	return true;
}

static bool parse_san(ctx *c, alt_x509_cert *cert, alt_der der) {
	if (!alt_der_read(&der, &der, ALT_ASN1_SEQUENCE))
		FAIL("x509: invalid subject alternative names");
	while (!alt_der_empty(der)) {
		alt_der san;
		unsigned tag;
		if (!alt_der_read_any(&der, &san, &tag))
			FAIL("x509: invalid subject alternative name");
		switch (tag ^ 0x80) {
		case 1:
			if (!is_ia5(san))
				FAIL("x509: SAN rfc822Name is malformed");
			if (!push_str(c, &cert->emails, view(san)))
				return false;
			break;
		case 2:
			if (!is_ia5(san))
				FAIL("x509: SAN dNSName is malformed");
			if (!push_str(c, &cert->dns_names, view(san)))
				return false;
			break;
		case 6: {
			if (!is_ia5(san))
				FAIL("x509: SAN uniformResourceIdentifier is malformed");
			alt_url u;
			if (alt_url_parse(c->a, view(san), &u) < 0) {
				return failq(c, "x509: cannot parse URI ", view(san), "");
			}
			if (u.host.n > 0 && !domain_valid(u.host, false)) {
				return failq(c, "x509: cannot parse URI ", view(san), ": invalid domain");
			}
			if (!push_str(c, &cert->uris, view(san)))
				return false;
			break;
		}
		case 7: {
			if (san.n == 16) {
				static const uint8_t v4[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
				if (memcmp(san.p, v4, 12) == 0)
					FAIL("x509: SAN iPAddress contains IPv4-mapped IPv6 address");
			} else if (san.n != 4) {
				c->why = alt_arena_printf(c->a, "x509: cannot parse IP address of length %zu", san.n);
				return false;
			}
			void *p = alt_arena_realloc(c->a, cert->ips, cert->nips * sizeof *cert->ips, (cert->nips + 1) * sizeof *cert->ips);
			if (!p)
				FAIL("out of memory");
			cert->ips = p;
			memcpy(cert->ips[cert->nips].b, san.p, san.n);
			cert->ips[cert->nips++].len = (int)san.n;
			break;
		}
		}
	}
	return true;
}

static bool valid_mask(const uint8_t *m, int n) {
	bool zero = false;
	for (int i = 0; i < n; i++) {
		if (zero) {
			if (m[i])
				return false;
			continue;
		}
		switch (m[i]) {
		case 0x00: case 0x80: case 0xc0: case 0xe0: case 0xf0: case 0xf8: case 0xfc: case 0xfe: zero = true; break;
		case 0xff: break;
		default: return false;
		}
	}
	return true;
}

static bool subtrees(ctx *c, alt_der s, alt_x509_strs *dns, alt_x509_ipnet **ips, size_t *nips, alt_x509_strs *emails,
	alt_x509_strs *uris, bool *unhandled) {
	while (!alt_der_empty(s)) {
		alt_der seq, value;
		unsigned tag;
		if (!alt_der_read(&s, &seq, ALT_ASN1_SEQUENCE) || !alt_der_read_any(&seq, &value, &tag))
			FAIL("x509: invalid NameConstraints extension");
		alt_str v = view(value);
		if (tag == (2 | CTX0)) {
			if (!is_ia5(value)) {
				c->why = alt_arena_printf(c->a, "x509: invalid constraint value: x509: %s cannot be encoded as an IA5String", "");
				return false;
			}
			if (!domain_valid(v, true)) {
				return failq(c, "x509: failed to parse dnsName constraint ", v, "");
			}
			if (!push_str(c, dns, v))
				return false;
		} else if (tag == (7 | CTX0)) {
			if (value.n != 8 && value.n != 32) {
				c->why = alt_arena_printf(c->a, "x509: IP constraint contained value of length %zu", value.n);
				return false;
			}
			int l = (int)value.n / 2;
			alt_x509_ipnet net = {.len = l};
			memcpy(net.ip, value.p, (size_t)l);
			memcpy(net.mask, value.p + l, (size_t)l);
			if (!valid_mask(net.mask, l)) {
				c->why = "x509: IP constraint contained invalid mask";
				return false;
			}
			static const uint8_t v4[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
			if (l == 16 && memcmp(net.ip, v4, 12) == 0)
				FAIL("x509: IP constraint contained IPv4-mapped IPv6 address");
			PUSH(c, *ips, *nips, net);
		} else if (tag == (1 | CTX0)) {
			if (!is_ia5(value)) {
				c->why = "x509: invalid constraint value";
				return false;
			}
			bool ok = alt_str_index_byte(v, '@') >= 0 ? rfc2821_mailbox(v) : domain_valid(v, true);
			if (!ok) {
				return failq(c, "x509: failed to parse rfc822Name constraint ", v, "");
			}
			if (!push_str(c, emails, v))
				return false;
		} else if (tag == (6 | CTX0)) {
			if (!is_ia5(value)) {
				c->why = "x509: invalid constraint value";
				return false;
			}
			alt_ip ip;
			if (alt_ip_parse(v, &ip) == 0 && !ip.zone[0]) {
				return failq(c, "x509: failed to parse URI constraint ", v, ": cannot be IP address");
			}
			if (!domain_valid(v, true)) {
				return failq(c, "x509: failed to parse URI constraint ", v, "");
			}
			if (!push_str(c, uris, v))
				return false;
		} else
			*unhandled = true;
	}
	return true;
}

static bool parse_name_constraints(ctx *c, alt_x509_cert *cert, const alt_x509_ext *e, bool *unhandled) {
	alt_der outer = e->value, top, permitted, excluded;
	bool hp, he;
	if (!alt_der_read(&outer, &top, ALT_ASN1_SEQUENCE) || !alt_der_empty(outer) ||
		!alt_der_read_optional(&top, &permitted, &hp, 0 | CTX0C) || !alt_der_read_optional(&top, &excluded, &he, 1 | CTX0C) ||
		!alt_der_empty(top))
		FAIL("x509: invalid NameConstraints extension");
	if ((!hp && !he) || (permitted.n == 0 && excluded.n == 0))
		FAIL("x509: empty name constraints extension");
	if (!subtrees(c, permitted, &cert->permitted_dns, &cert->permitted_ips, &cert->npermitted_ips, &cert->permitted_emails,
		    &cert->permitted_uris, unhandled) ||
		!subtrees(c, excluded, &cert->excluded_dns, &cert->excluded_ips, &cert->nexcluded_ips, &cert->excluded_emails,
			&cert->excluded_uris, unhandled))
		return false;
	cert->permitted_dns_critical = e->critical;
	return true;
}

static const alt_oid eku_oids[] = {
	ALT_OID(2, 5, 29, 37, 0),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 1),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 2),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 3),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 4),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 5),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 6),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 7),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 8),
	ALT_OID(1, 3, 6, 1, 5, 5, 7, 3, 9),
	ALT_OID(1, 3, 6, 1, 4, 1, 311, 10, 3, 3),
	ALT_OID(2, 16, 840, 1, 113730, 4, 1),
	ALT_OID(1, 3, 6, 1, 4, 1, 311, 2, 1, 22),
	ALT_OID(1, 3, 6, 1, 4, 1, 311, 61, 1, 1),
};

/* an OID's DER contents as Go's newOIDFromDER accepts them (arcs of any size) */
static bool oid_der_valid(alt_der b) {
	if (b.n == 0 || (b.p[b.n - 1] & 0x80))
		return false;
	size_t start = 0;
	for (size_t i = 0; i < b.n; i++) {
		if (i == start && b.p[i] == 0x80)
			return false;
		if (!(b.p[i] & 0x80))
			start = i + 1;
	}
	return true;
}

static bool process_extensions(ctx *c, alt_x509_cert *out) {
	for (size_t k = 0; k < out->nexts; k++) {
		const alt_x509_ext *e = &out->exts[k];
		bool unhandled = false;
		const alt_oid *id = &e->id;
		if (id->n == 4 && id->arc[0] == 2 && id->arc[1] == 5 && id->arc[2] == 29) {
			alt_der val = e->value;
			switch (id->arc[3]) {
			case 15: {
				alt_der bits;
				size_t nbits;
				if (!alt_der_read_bit_string(&val, &bits, &nbits))
					FAIL("x509: invalid key usage");
				int usage = 0;
				for (size_t i = 0; i < 9; i++)
					if (i < nbits && (bits.p[i / 8] >> (7 - i % 8) & 1))
						usage |= 1 << i;
				out->key_usage = usage;
				break;
			}
			case 19: {
				bool ca = false;
				if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid basic constraints");
				if (alt_der_peek(val, ALT_ASN1_BOOLEAN) && !alt_der_read_bool(&val, &ca))
					FAIL("x509: invalid basic constraints");
				int mpl = -1;
				if (alt_der_peek(val, ALT_ASN1_INTEGER)) {
					uint64_t v;
					if (!alt_der_read_uint64(&val, &v) || v > (uint64_t)INT64_MAX)
						FAIL("x509: invalid basic constraints");
					mpl = v > 0x7fffffff ? 0x7fffffff : (int)v;
				}
				out->is_ca = ca;
				out->max_path_len = mpl;
				out->basic_constraints_valid = true;
				out->max_path_len_zero = mpl == 0;
				break;
			}
			case 17: {
				size_t before = out->dns_names.n + out->emails.n + out->nips + out->uris.n;
				if (!parse_san(c, out, val))
					return false;
				if (out->dns_names.n + out->emails.n + out->nips + out->uris.n == before && before == 0)
					unhandled = true;
				break;
			}
			case 30:
				if (!parse_name_constraints(c, out, e, &unhandled))
					return false;
				break;
			case 31: {
				if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid CRL distribution points");
				while (!alt_der_empty(val)) {
					alt_der dp, name;
					bool present;
					if (!alt_der_read(&val, &dp, ALT_ASN1_SEQUENCE))
						FAIL("x509: invalid CRL distribution point");
					if (!alt_der_read_optional(&dp, &name, &present, 0 | CTX0C))
						FAIL("x509: invalid CRL distribution point");
					if (!present)
						continue;
					if (!alt_der_read(&name, &name, 0 | CTX0C))
						FAIL("x509: invalid CRL distribution point");
					while (!alt_der_empty(name)) {
						if (!alt_der_peek(name, 6 | CTX0))
							break;
						alt_der uri;
						if (!alt_der_read(&name, &uri, 6 | CTX0))
							FAIL("x509: invalid CRL distribution point");
						if (!push_str(c, &out->crl_dps, view(uri)))
							return false;
					}
				}
				break;
			}
			case 35: {
				if (e->critical)
					FAIL("x509: authority key identifier incorrectly marked critical");
				alt_der akid;
				if (!alt_der_read(&val, &akid, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid authority key identifier");
				if (alt_der_peek(akid, 0 | CTX0)) {
					if (!alt_der_read(&akid, &akid, 0 | CTX0))
						FAIL("x509: invalid authority key identifier");
					out->authority_key_id = akid;
				}
				break;
			}
			case 36: {
				if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid policy constraints extension");
				for (unsigned t = 0; t < 2; t++)
					if (alt_der_peek(val, t | CTX0)) {
						alt_der b;
						int64_t v;
						alt_der save = val;
						if (!alt_der_read(&val, &b, t | CTX0) || b.n == 0) {
							val = save;
							FAIL("x509: invalid policy constraints extension");
						}
						/* ReadASN1Int64WithTag: an INTEGER's contents under the tag */
						alt_der wrapped = b;
						uint8_t tmp[16];
						if (b.n > 14)
							FAIL("x509: invalid policy constraints extension");
						tmp[0] = ALT_ASN1_INTEGER;
						tmp[1] = (uint8_t)b.n;
						memcpy(tmp + 2, b.p, b.n);
						wrapped = alt_der_make(tmp, b.n + 2);
						if (!alt_der_read_int64(&wrapped, &v))
							FAIL("x509: invalid policy constraints extension");
					}
				break;
			}
			case 37: {
				if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid extended key usages");
				while (!alt_der_empty(val)) {
					alt_oid o;
					if (!alt_der_read_oid(&val, &o))
						FAIL("x509: invalid extended key usages");
					int found = -1;
					for (int i = 0; i < (int)(sizeof eku_oids / sizeof *eku_oids); i++)
						if (alt_oid_eq(&o, &eku_oids[i]))
							found = i;
					if (found >= 0)
						PUSH(c, out->ext_key_usage, out->neku, found);
					else
						PUSH(c, out->unknown_eku, out->nunknown_eku, o);
				}
				break;
			}
			case 14: {
				if (e->critical)
					FAIL("x509: subject key identifier incorrectly marked critical");
				alt_der skid;
				if (!alt_der_read(&val, &skid, ALT_ASN1_OCTET_STRING))
					FAIL("x509: invalid subject key identifier");
				out->subject_key_id = skid;
				break;
			}
			case 32: {
				if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid certificate policies");
				alt_der *seen = 0;
				size_t nseen = 0;
				while (!alt_der_empty(val)) {
					alt_der cp, ob;
					if (!alt_der_read(&val, &cp, ALT_ASN1_SEQUENCE) || !alt_der_read(&cp, &ob, ALT_ASN1_OID))
						FAIL("x509: invalid certificate policies");
					for (size_t i = 0; i < nseen; i++)
						if (seen[i].n == ob.n && memcmp(seen[i].p, ob.p, ob.n) == 0)
							FAIL("x509: invalid certificate policies");
					PUSH(c, seen, nseen, ob);
					if (!oid_der_valid(ob))
						FAIL("x509: invalid certificate policies");
					/* as an alt_oid when it fits (Go keeps all of them as OID) */
					uint8_t tmp[300];
					alt_oid o = {0};
					if (ob.n + 3 <= sizeof tmp) {
						size_t hl = ob.n < 128 ? 2 : 3;
						tmp[0] = ALT_ASN1_OID;
						if (hl == 2)
							tmp[1] = (uint8_t)ob.n;
						else
							tmp[1] = 0x81, tmp[2] = (uint8_t)ob.n;
						memcpy(tmp + hl, ob.p, ob.n);
						alt_der w = alt_der_make(tmp, ob.n + hl);
						alt_der_read_oid(&w, &o);
					}
					PUSH(c, out->policies, out->npolicies, o);
				}
				break;
			}
			case 33: {
				if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid policy mappings extension");
				while (!alt_der_empty(val)) {
					alt_der s, a, b;
					if (!alt_der_read(&val, &s, ALT_ASN1_SEQUENCE) || !alt_der_read(&s, &a, ALT_ASN1_OID) ||
						!alt_der_read(&s, &b, ALT_ASN1_OID))
						FAIL("x509: invalid policy mappings extension");
				}
				break;
			}
			case 54: {
				int64_t v;
				if (!alt_der_read_int64(&val, &v))
					FAIL("x509: invalid inhibit any policy extension");
				break;
			}
			default:
				unhandled = true;
			}
		} else if (OIDEQ(id, 1, 3, 6, 1, 5, 5, 7, 1, 1)) {
			if (e->critical)
				FAIL("x509: authority info access incorrectly marked critical");
			alt_der val = e->value;
			if (!alt_der_read(&val, &val, ALT_ASN1_SEQUENCE))
				FAIL("x509: invalid authority info access");
			while (!alt_der_empty(val)) {
				alt_der aia;
				alt_oid method;
				if (!alt_der_read(&val, &aia, ALT_ASN1_SEQUENCE))
					FAIL("x509: invalid authority info access");
				if (!alt_der_read_oid(&aia, &method))
					FAIL("x509: invalid authority info access");
				if (!alt_der_peek(aia, 6 | CTX0))
					continue;
				if (!alt_der_read(&aia, &aia, 6 | CTX0))
					FAIL("x509: invalid authority info access");
				if (OIDEQ(&method, 1, 3, 6, 1, 5, 5, 7, 48, 1)) {
					if (!push_str(c, &out->ocsp, view(aia)))
						return false;
				} else if (OIDEQ(&method, 1, 3, 6, 1, 5, 5, 7, 48, 2)) {
					if (!push_str(c, &out->issuing_urls, view(aia)))
						return false;
				}
			}
		} else
			unhandled = true;
		if (e->critical && unhandled)
			PUSH(c, out->unhandled_critical, out->nunhandled_critical, e->id);
	}
	return true;
}

static bool parse(ctx *c, const uint8_t *der, size_t n, alt_x509_cert *cert) {
	alt_der input = alt_der_make(der, n), tbs;
	if (!alt_der_read_element(&input, &input, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed certificate");
	cert->raw = input;
	if (!alt_der_read(&input, &input, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed certificate");
	if (!alt_der_read_element(&input, &tbs, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed tbs certificate");
	cert->raw_tbs = tbs;
	if (!alt_der_read(&tbs, &tbs, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed tbs certificate");
	int64_t version;
	if (!alt_der_read_optional_int64(&tbs, &version, 0 | CTX0C, 0))
		FAIL("x509: malformed version");
	if (version < 0)
		FAIL("x509: malformed version");
	if (version + 1 > 3)
		FAIL("x509: invalid version");
	cert->version = (int)version + 1;
	if (!alt_der_read_int_bytes(&tbs, &cert->serial))
		FAIL("x509: malformed serial number");
	if (cert->serial.p[0] & 0x80)
		FAIL("x509: negative serial number");
	alt_der sigai;
	if (!alt_der_read_element(&tbs, &sigai, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed signature algorithm identifier");
	cert->raw_sig_alg = sigai;
	if (!alt_der_read(&sigai, &sigai, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed signature algorithm identifier");
	alt_der outer;
	if (!alt_der_read(&input, &outer, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed algorithm identifier");
	if (outer.n != sigai.n || memcmp(outer.p, sigai.p, sigai.n) != 0)
		FAIL("x509: inner and outer signature algorithm identifiers don't match");
	alt_oid alg;
	alt_der params;
	unsigned ptag;
	if (!parse_ai(c, sigai, &alg, &params, &ptag))
		return false;
	cert->sig_alg = sig_alg(&alg, params);
	alt_der issuer;
	if (!alt_der_read_element(&tbs, &issuer, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed issuer");
	cert->raw_issuer = issuer;
	if (!parse_name(c, issuer, &cert->issuer))
		return false;
	alt_der validity;
	if (!alt_der_read(&tbs, &validity, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed validity");
	if (!read_time(c, &validity, &cert->not_before) || !read_time(c, &validity, &cert->not_after))
		return false;
	alt_der subject;
	if (!alt_der_read_element(&tbs, &subject, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed issuer");
	cert->raw_subject = subject;
	if (!parse_name(c, subject, &cert->subject))
		return false;
	alt_der spki;
	if (!alt_der_read_element(&tbs, &spki, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed spki");
	cert->raw_spki = spki;
	if (!alt_der_read(&spki, &spki, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed spki");
	alt_der pkai;
	if (!alt_der_read(&spki, &pkai, ALT_ASN1_SEQUENCE))
		FAIL("x509: malformed public key algorithm identifier");
	alt_oid pkalg;
	if (!parse_ai(c, pkai, &pkalg, &params, &ptag))
		return false;
	cert->pk_alg = pk_alg(&pkalg);
	c->pk_oid_last = pkalg.arc[pkalg.n - 1];
	alt_der spk;
	size_t spk_bits;
	if (!alt_der_read_bit_string(&spki, &spk, &spk_bits))
		FAIL("x509: malformed subjectPublicKey");
	/* BitString.RightAlign: the bytes are whole when the padding is 0;
	   otherwise shifted right */
	alt_der data = spk;
	if (spk_bits % 8) {
		size_t shift = 8 - spk_bits % 8, nb = (spk_bits + 7) / 8;
		uint8_t *a = alt_arena_alloc(c->a, nb ? nb : 1);
		if (!a)
			FAIL("out of memory");
		a[0] = spk.p[0] >> shift;
		for (size_t i = 1; i < nb; i++)
			a[i] = (uint8_t)(spk.p[i - 1] << (8 - shift) | spk.p[i] >> shift);
		data = alt_der_make(a, nb);
	}
	if (cert->pk_alg != ALT_X509_PK_UNKNOWN && !parse_public_key(c, cert, params, data))
		return false;
	cert->max_path_len = -1;
	if (cert->version > 1) {
		if (!alt_der_skip_optional(&tbs, 1 | CTX0))
			FAIL("x509: malformed issuerUniqueID");
		if (!alt_der_skip_optional(&tbs, 2 | CTX0))
			FAIL("x509: malformed subjectUniqueID");
		if (cert->version == 3) {
			alt_der exts;
			bool present;
			if (!alt_der_read_optional(&tbs, &exts, &present, 3 | CTX0C))
				FAIL("x509: malformed extensions");
			if (present) {
				if (!alt_der_read(&exts, &exts, ALT_ASN1_SEQUENCE))
					FAIL("x509: malformed extensions");
				while (!alt_der_empty(exts)) {
					alt_der ext;
					if (!alt_der_read(&exts, &ext, ALT_ASN1_SEQUENCE))
						FAIL("x509: malformed extension");
					alt_x509_ext x = {0};
					if (!alt_der_read_oid_bytes(&ext, &x.raw_id))
						FAIL("x509: malformed extension OID field");
					if (!alt_oid_from_bytes(x.raw_id, &x.id))
						x.id.n = 0;
					if (alt_der_peek(ext, ALT_ASN1_BOOLEAN) && !alt_der_read_bool(&ext, &x.critical))
						FAIL("x509: malformed extension critical field");
					if (!alt_der_read(&ext, &x.value, ALT_ASN1_OCTET_STRING))
						FAIL("x509: malformed extension value field");
					for (size_t i = 0; i < cert->nexts; i++)
						if (cert->exts[i].raw_id.n == x.raw_id.n && memcmp(cert->exts[i].raw_id.p, x.raw_id.p, x.raw_id.n) == 0) {
							alt_buf ob = ALT_BUF_INIT;
							alt_oid_format(&ob, &x.id);
							c->why = alt_arena_printf(c->a, "x509: certificate contains duplicate extension with OID \"%s\"",
								alt_buf_cstr(&ob));
							alt_buf_free(&ob);
							return false;
						}
					PUSH(c, cert->exts, cert->nexts, x);
				}
				if (!process_extensions(c, cert))
					return false;
			}
		}
	}
	alt_der sig;
	size_t sig_bits;
	if (!alt_der_read_bit_string(&input, &sig, &sig_bits))
		FAIL("x509: malformed signature");
	cert->signature = sig;
	if (sig_bits % 8) {
		size_t shift = 8 - sig_bits % 8, nb = (sig_bits + 7) / 8;
		uint8_t *a = alt_arena_alloc(c->a, nb ? nb : 1);
		if (!a)
			FAIL("out of memory");
		a[0] = sig.p[0] >> shift;
		for (size_t i = 1; i < nb; i++)
			a[i] = (uint8_t)(sig.p[i - 1] << (8 - shift) | sig.p[i] >> shift);
		cert->signature = alt_der_make(a, nb);
	}
	return true;
}

int alt_x509_parse(alt_arena *a, const void *der, size_t n, alt_x509_cert **out, const char **why) {
	ctx c = {a, 0, 0};
	alt_x509_cert *cert = alt_arena_zalloc(a, sizeof *cert);
	if (!cert)
		return -ENOMEM;
	if (!parse(&c, der, n, cert)) {
		if (why)
			*why = c.why;
		return -ALT_EX509;
	}
	if (cert->raw.n != n) {
		if (why)
			*why = "x509: trailing data";
		return -ALT_EX509;
	}
	*out = cert;
	return 0;
}

alt_str alt_x509_name_get(const alt_x509_name *n, const alt_oid *type) {
	for (size_t i = 0; i < n->n; i++)
		if (alt_oid_eq(&n->attrs[i].type, type))
			return n->attrs[i].value;
	return alt_str_make("", 0);
}
