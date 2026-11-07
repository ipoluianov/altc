/* The NIST prime curves y^2 = x^3 - 3x + b with Montgomery arithmetic
   over 64-bit limbs (any odd modulus: the field and the group order),
   projective points with the complete addition formulas of Renes,
   Costello and Batina (2015, algorithm 4), scalar multiplication that
   adds at every bit. Verification follows Go's crypto/ecdsa. */
#include <alt/crypto/ecdsa.h>
#include <alt/crypto/hash.h>
#include <alt/encoding/asn1.h>
#include <alt/sync.h>
#include <string.h>

enum { L = 9 }; /* limbs of the largest modulus (P-521) */
typedef unsigned __int128 u128;

typedef struct {
	int n;                 /* limbs */
	int bits, bytes;
	uint64_t m[L], minv, rr[L], one[L];
} mod;

typedef struct {
	const char *name;
	mod p, q;              /* the field and the group order */
	uint64_t b[L], gx[L], gy[L];
} curve;

typedef struct {
	uint64_t x[L], y[L], z[L];
} point;

/* ---- multi-precision modular arithmetic (constant time) ---- */

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

static void mod_add(const mod *m, uint64_t *r, const uint64_t *a, const uint64_t *b) {
	uint64_t t[L], u[L], c = 0, bw = 0;
	for (int i = 0; i < m->n; i++)
		t[i] = addc(a[i], b[i], &c);
	for (int i = 0; i < m->n; i++)
		u[i] = subb(t[i], m->m[i], &bw);
	sel(r, u, t, c | (bw ^ 1), m->n);
}

static void mod_sub(const mod *m, uint64_t *r, const uint64_t *a, const uint64_t *b) {
	uint64_t t[L], u[L], c = 0, bw = 0;
	for (int i = 0; i < m->n; i++)
		t[i] = subb(a[i], b[i], &bw);
	for (int i = 0; i < m->n; i++)
		u[i] = addc(t[i], m->m[i], &c);
	sel(r, u, t, bw, m->n);
}

/* r = a b R^-1 mod m (CIOS) */
static void mont_mul(const mod *m, uint64_t *r, const uint64_t *a, const uint64_t *b) {
	int n = m->n;
	uint64_t t[L + 2] = {0};
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
	uint64_t u[L], bw = 0;
	for (int i = 0; i < n; i++)
		u[i] = subb(t[i], m->m[i], &bw);
	(void)subb(t[n], 0, &bw);
	sel(r, t, u, bw, n);
}

static bool is_zero(const uint64_t *a, int n) {
	uint64_t x = 0;
	for (int i = 0; i < n; i++)
		x |= a[i];
	return x == 0;
}

/* big-endian bytes (exactly m->bytes of them) into Montgomery form:
   false when the value is not below m */
static bool from_bytes(const mod *m, uint64_t *r, const uint8_t *b) {
	uint64_t x[L] = {0};
	for (int i = 0; i < m->bytes; i++) {
		int bit = (m->bytes - 1 - i) * 8;
		x[bit / 64] |= (uint64_t)b[i] << (bit % 64);
	}
	uint64_t bw = 0;
	for (int i = 0; i < m->n; i++)
		(void)subb(x[i], m->m[i], &bw);
	if (!bw)
		return false;
	mont_mul(m, r, x, m->rr);
	return true;
}

static void to_bytes(const mod *m, uint8_t *b, const uint64_t *a) {
	uint64_t one[L] = {1}, x[L];
	mont_mul(m, x, a, one);
	for (int i = 0; i < m->bytes; i++) {
		int bit = (m->bytes - 1 - i) * 8;
		b[i] = (uint8_t)(x[bit / 64] >> (bit % 64));
	}
}

/* r = a^e (e in normal form, all its limbs scanned) */
static void mod_pow(const mod *m, uint64_t *r, const uint64_t *a, const uint64_t *e) {
	uint64_t acc[L], t[L];
	memcpy(acc, m->one, sizeof acc);
	for (int i = m->n * 64 - 1; i >= 0; i--) {
		mont_mul(m, acc, acc, acc);
		mont_mul(m, t, acc, a);
		sel(acc, t, acc, e[i / 64] >> (i % 64) & 1, m->n);
	}
	memcpy(r, acc, (size_t)m->n * 8);
}

static void mod_inv(const mod *m, uint64_t *r, const uint64_t *a) {
	uint64_t e[L] = {0}, two[L] = {2}, bw = 0;
	for (int i = 0; i < m->n; i++)
		e[i] = subb(m->m[i], two[i], &bw);
	mod_pow(m, r, a, e);
}

static int hexv(char c) { return c <= '9' ? c - '0' : (c | 32) - 'a' + 10; }

/* a big-endian hex constant into limbs */
static void hex_limbs(uint64_t *x, const char *h) {
	memset(x, 0, L * 8);
	size_t n = strlen(h);
	for (size_t i = 0; i < n; i++) {
		size_t bit = (n - 1 - i) * 4;
		x[bit / 64] |= (uint64_t)hexv(h[i]) << (bit % 64);
	}
}

static void mod_init(mod *m, const char *hex, int bits) {
	hex_limbs(m->m, hex);
	m->bits = bits;
	m->bytes = (bits + 7) / 8;
	m->n = (bits + 63) / 64;
	uint64_t inv = 1;
	for (int i = 0; i < 7; i++)
		inv *= 2 - m->m[0] * inv;
	m->minv = 0 - inv;
	/* R^2 = 2^(128 n) mod m by doubling 1 */
	uint64_t x[L] = {1};
	for (int i = 0; i < 128 * m->n; i++)
		mod_add(m, x, x, x);
	memcpy(m->rr, x, sizeof x);
	uint64_t one[L] = {1};
	mont_mul(m, m->one, one, m->rr);
}

/* ---- the curves ---- */

static curve curves[3];
static alt_once curves_once = ALT_ONCE_INIT;

static void to_mont(const mod *m, uint64_t *r, const char *hex) {
	uint64_t x[L];
	hex_limbs(x, hex);
	mont_mul(m, r, x, m->rr);
}

static void init_curve(curve *c, const char *name, int bits, const char *p, const char *q, const char *b, const char *gx,
	const char *gy) {
	c->name = name;
	mod_init(&c->p, p, bits);
	mod_init(&c->q, q, bits);
	to_mont(&c->p, c->b, b);
	to_mont(&c->p, c->gx, gx);
	to_mont(&c->p, c->gy, gy);
}

static void init_curves(void *arg) {
	(void)arg;
	init_curve(&curves[0], "P256", 256, "ffffffff00000001000000000000000000000000ffffffffffffffffffffffff",
		"ffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",
		"5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
		"6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
		"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5");
	init_curve(&curves[1], "P384", 384,
		"fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffff0000000000000000ffffffff",
		"ffffffffffffffffffffffffffffffffffffffffffffffffc7634d81f4372ddf581a0db248b0a77aecec196accc52973",
		"b3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875ac656398d8a2ed19d2a85c8edd3ec2aef",
		"aa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a385502f25dbf55296c3a545e3872760ab7",
		"3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f");
	init_curve(&curves[2], "P521", 521,
		"1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
		"fffffffffffffffffffffffffffffff",
		"1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa51868783bf2f966b7fcc0148f709a5d03"
		"bb5c9b8899c47aebb6fb71e91386409",
		"51953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf0735"
		"73df883d2c34f1ef451fd46b503f00",
		"c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de33"
		"48b3c1856a429bf97e7e31c2e5bd66",
		"11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad07613"
		"53c7086a272c24088be94769fd16650");
}

static const curve *get(alt_ec_curve c) {
	alt_once_do(&curves_once, init_curves, 0);
	return c >= ALT_P256 && c <= ALT_P521 ? &curves[c - 1] : 0;
}

size_t alt_ec_scalar_size(alt_ec_curve c) { return c == ALT_P256 ? 32 : c == ALT_P384 ? 48 : c == ALT_P521 ? 66 : 0; }
size_t alt_ec_point_size(alt_ec_curve c) { return 1 + 2 * alt_ec_scalar_size(c); }

/* ---- points ---- */

static void pt_identity(const curve *c, point *p) {
	memset(p, 0, sizeof *p);
	memcpy(p->y, c->p.one, sizeof p->y);
}

/* complete addition for a = -3 (RCB algorithm 4); also doubles */
static void pt_add(const curve *cv, point *r, const point *p1, const point *p2) {
	const mod *m = &cv->p;
	uint64_t t0[L], t1[L], t2[L], t3[L], t4[L], X3[L], Y3[L], Z3[L];
	const uint64_t *X1 = p1->x, *Y1 = p1->y, *Z1 = p1->z, *X2 = p2->x, *Y2 = p2->y, *Z2 = p2->z;
#define MUL(r, a, b) mont_mul(m, r, a, b)
#define ADD(r, a, b) mod_add(m, r, a, b)
#define SUB(r, a, b) mod_sub(m, r, a, b)
	MUL(t0, X1, X2);
	MUL(t1, Y1, Y2);
	MUL(t2, Z1, Z2);
	ADD(t3, X1, Y1);
	ADD(t4, X2, Y2);
	MUL(t3, t3, t4);
	ADD(t4, t0, t1);
	SUB(t3, t3, t4);
	ADD(t4, Y1, Z1);
	ADD(X3, Y2, Z2);
	MUL(t4, t4, X3);
	ADD(X3, t1, t2);
	SUB(t4, t4, X3);
	ADD(X3, X1, Z1);
	ADD(Y3, X2, Z2);
	MUL(X3, X3, Y3);
	ADD(Y3, t0, t2);
	SUB(Y3, X3, Y3);
	MUL(Z3, cv->b, t2);
	SUB(X3, Y3, Z3);
	ADD(Z3, X3, X3);
	ADD(X3, X3, Z3);
	SUB(Z3, t1, X3);
	ADD(X3, t1, X3);
	MUL(Y3, cv->b, Y3);
	ADD(t1, t2, t2);
	ADD(t2, t1, t2);
	SUB(Y3, Y3, t2);
	SUB(Y3, Y3, t0);
	ADD(t1, Y3, Y3);
	ADD(Y3, t1, Y3);
	ADD(t1, t0, t0);
	ADD(t0, t1, t0);
	SUB(t0, t0, t2);
	MUL(t1, t4, Y3);
	MUL(t2, t0, Y3);
	MUL(Y3, X3, Z3);
	ADD(Y3, Y3, t2);
	MUL(X3, t3, X3);
	SUB(X3, X3, t1);
	MUL(Z3, t4, Z3);
	MUL(t1, t3, t0);
	ADD(Z3, Z3, t1);
#undef MUL
#undef ADD
#undef SUB
	memcpy(r->x, X3, sizeof X3);
	memcpy(r->y, Y3, sizeof Y3);
	memcpy(r->z, Z3, sizeof Z3);
}

/* [k]p, k as big-endian bytes of the scalar size */
static void pt_mul(const curve *c, point *r, const point *p, const uint8_t *k) {
	point acc, sum;
	pt_identity(c, &acc);
	int nb = c->q.bytes;
	for (int i = 0; i < nb * 8; i++) {
		int bit = k[i / 8] >> (7 - i % 8) & 1;
		pt_add(c, &acc, &acc, &acc);
		pt_add(c, &sum, &acc, p);
		sel(acc.x, sum.x, acc.x, (uint64_t)bit, c->p.n);
		sel(acc.y, sum.y, acc.y, (uint64_t)bit, c->p.n);
		sel(acc.z, sum.z, acc.z, (uint64_t)bit, c->p.n);
	}
	*r = acc;
}

static void pt_base(const curve *c, point *g) {
	memcpy(g->x, c->gx, sizeof g->x);
	memcpy(g->y, c->gy, sizeof g->y);
	memcpy(g->z, c->p.one, sizeof g->z);
}

/* the affine coordinates as bytes: false at infinity */
static bool pt_affine(const curve *c, const point *p, uint8_t *x, uint8_t *y) {
	if (is_zero(p->z, c->p.n))
		return false;
	uint64_t zi[L], t[L];
	mod_inv(&c->p, zi, p->z);
	mont_mul(&c->p, t, p->x, zi);
	to_bytes(&c->p, x, t);
	if (y) {
		mont_mul(&c->p, t, p->y, zi);
		to_bytes(&c->p, y, t);
	}
	return true;
}

/* decodes 04 || X || Y: 0, or a message as Go's (invalid encoding, not on the curve) */
static const char *pt_decode(const curve *c, point *p, const uint8_t *b, size_t n) {
	size_t sz = (size_t)c->p.bytes;
	if (n != 1 + 2 * sz || b[0] != 4)
		return "invalid point encoding";
	if (!from_bytes(&c->p, p->x, b + 1) || !from_bytes(&c->p, p->y, b + 1 + sz))
		return "invalid element encoding";
	/* y^2 = x^3 - 3x + b */
	uint64_t l[L], r[L], t[L];
	mont_mul(&c->p, l, p->y, p->y);
	mont_mul(&c->p, r, p->x, p->x);
	mont_mul(&c->p, r, r, p->x);
	mod_add(&c->p, t, p->x, p->x);
	mod_add(&c->p, t, t, p->x);
	mod_sub(&c->p, r, r, t);
	mod_add(&c->p, r, r, c->b);
	uint64_t d = 0;
	for (int i = 0; i < c->p.n; i++)
		d |= l[i] ^ r[i];
	if (d)
		return "point not on curve";
	memcpy(p->z, c->p.one, sizeof p->z);
	return 0;
}

/* for crypto/x509: Go's messages ("invalid P256Element encoding", "P256 point not on curve") */
const char *alt__ec_check_point(int cv, const uint8_t *b, size_t n) {
	const curve *c = get((alt_ec_curve)cv);
	point p;
	const char *why = pt_decode(c, &p, b, n);
	if (!why)
		return 0;
	static const char *msgs[3][3] = {{"invalid P256Element encoding", "P256 point not on curve", "invalid P256 point encoding"},
		{"invalid P384Element encoding", "P384 point not on curve", "invalid P384 point encoding"},
		{"invalid P521Element encoding", "P521 point not on curve", "invalid P521 point encoding"}};
	if (why[0] == 'p')
		return msgs[cv - 1][1];
	return msgs[cv - 1][why[8] == 'e' ? 0 : 2];
}

bool alt_ec_point_valid(alt_ec_curve cv, const uint8_t *pub, size_t n) {
	const curve *c = get(cv);
	point p;
	return c && !pt_decode(c, &p, pub, n);
}

/* a scalar in [1, q-1] from bytes */
static bool scalar_ok(const curve *c, const uint8_t *k) {
	uint64_t x[L];
	return from_bytes(&c->q, x, k) && !is_zero(x, c->q.n);
}

int alt_ec_public(alt_ec_curve cv, const uint8_t *priv, uint8_t *pub) {
	const curve *c = get(cv);
	if (!c || !scalar_ok(c, priv))
		return -ALT_EKEY;
	point g, q;
	pt_base(c, &g);
	pt_mul(c, &q, &g, priv);
	pub[0] = 4;
	pt_affine(c, &q, pub + 1, pub + 1 + c->p.bytes);
	return 0;
}

/* a random scalar in [1, q-1] (rejection sampling) */
static int random_scalar(const curve *c, uint8_t *k) {
	for (;;) {
		int rc = alt_crypto_rand(k, (size_t)c->q.bytes);
		if (rc < 0)
			return rc;
		int excess = c->q.bytes * 8 - c->q.bits;
		k[0] &= (uint8_t)(0xff >> excess);
		if (scalar_ok(c, k))
			return 0;
	}
}

int alt_ec_keygen(alt_ec_curve cv, uint8_t *priv, uint8_t *pub) {
	const curve *c = get(cv);
	if (!c)
		return -EINVAL;
	int rc = random_scalar(c, priv);
	return rc < 0 ? rc : alt_ec_public(cv, priv, pub);
}

int alt_ecdh(alt_ec_curve cv, const uint8_t *priv, const uint8_t *peer, size_t npeer, uint8_t *secret) {
	const curve *c = get(cv);
	point p, s;
	if (!c || !scalar_ok(c, priv) || pt_decode(c, &p, peer, npeer))
		return -ALT_EKEY;
	pt_mul(c, &s, &p, priv);
	return pt_affine(c, &s, secret, 0) ? 0 : -ALT_EKEY;
}

/* the hash as a scalar: its leftmost bits, reduced (Go's hashToNat) */
static void hash_scalar(const curve *c, uint64_t *e, const uint8_t *hash, size_t n) {
	uint8_t h[ALT_EC_MAX_SCALAR] = {0};
	size_t sz = (size_t)c->q.bytes;
	if (n >= sz) {
		memcpy(h, hash, sz);
		int excess = (int)sz * 8 - c->q.bits;
		if (excess > 0)
			for (int i = (int)sz - 1; i >= 0; i--) {
				h[i] >>= excess;
				if (i > 0)
					h[i] |= (uint8_t)(h[i - 1] << (8 - excess));
			}
	} else
		memcpy(h + sz - n, hash, n);
	/* below 2^bits < 2q: one subtraction reduces it */
	uint64_t x[L] = {0}, u[L], bw = 0;
	for (size_t i = 0; i < sz; i++) {
		size_t bit = (sz - 1 - i) * 8;
		x[bit / 64] |= (uint64_t)h[i] << (bit % 64);
	}
	for (int i = 0; i < c->q.n; i++)
		u[i] = subb(x[i], c->q.m[i], &bw);
	sel(x, x, u, bw, c->q.n);
	mont_mul(&c->q, e, x, c->q.rr);
}

/* an INTEGER's magnitude (as cryptobyte reads []byte) into a scalar
   below q: false when out of range */
static bool sig_scalar(const curve *c, alt_der v, uint64_t *out) {
	if (v.n > (size_t)c->q.bytes)
		return false;
	uint8_t b[ALT_EC_MAX_SCALAR] = {0};
	memcpy(b + c->q.bytes - v.n, v.p, v.n);
	return from_bytes(&c->q, out, b);
}

int alt_ecdsa_verify(alt_ec_curve cv, const uint8_t *pub, size_t npub, const void *hash, size_t nhash, const uint8_t *sig,
	size_t nsig) {
	const curve *c = get(cv);
	point q;
	if (!c || pt_decode(c, &q, pub, npub))
		return -ALT_EKEY;
	alt_der in = alt_der_make(sig, nsig), inner, rb, sb;
	if (!alt_der_read(&in, &inner, ALT_ASN1_SEQUENCE) || in.n || !alt_der_read_uint_bytes(&inner, &rb) ||
		!alt_der_read_uint_bytes(&inner, &sb) || inner.n)
		return -ALT_EVERIFY;
	uint64_t r[L], s[L], e[L], w[L], u1[L], u2[L];
	if (!sig_scalar(c, rb, r) || is_zero(r, c->q.n) || !sig_scalar(c, sb, s) || is_zero(s, c->q.n))
		return -ALT_EVERIFY;
	hash_scalar(c, e, hash, nhash);
	mod_inv(&c->q, w, s);
	mont_mul(&c->q, u1, e, w);
	mont_mul(&c->q, u2, r, w);
	uint8_t b1[ALT_EC_MAX_SCALAR], b2[ALT_EC_MAX_SCALAR];
	to_bytes(&c->q, b1, u1);
	to_bytes(&c->q, b2, u2);
	point g, p1, p2, sum;
	pt_base(c, &g);
	pt_mul(c, &p1, &g, b1);
	pt_mul(c, &p2, &q, b2);
	pt_add(c, &sum, &p1, &p2);
	uint8_t x[ALT_EC_MAX_SCALAR];
	if (!pt_affine(c, &sum, x, 0))
		return -ALT_EVERIFY;
	/* x mod q (x < p < 2q) */
	uint64_t v[L];
	alt_der xd = alt_der_make(x, (size_t)c->p.bytes);
	uint64_t xl[L] = {0}, u[L], bw = 0;
	for (size_t i = 0; i < xd.n; i++) {
		size_t bit = (xd.n - 1 - i) * 8;
		xl[bit / 64] |= (uint64_t)x[i] << (bit % 64);
	}
	for (int i = 0; i < c->q.n; i++)
		u[i] = subb(xl[i], c->q.m[i], &bw);
	sel(v, xl, u, bw, c->q.n);
	uint64_t rn[L], one[L] = {1};
	mont_mul(&c->q, rn, r, one); /* r out of Montgomery form */
	uint64_t d = 0;
	for (int i = 0; i < c->q.n; i++)
		d |= rn[i] ^ v[i];
	return d ? -ALT_EVERIFY : 0;
}

/* the nonce: HMAC-SHA-512 over the key, random bytes and the hash,
   reduced by rejection (hedged as Go: secure without a good random
   source, varied with one) */
static int nonce(const curve *c, const uint8_t *priv, const uint8_t *hash, size_t nhash, uint8_t *k) {
	uint8_t rnd[32];
	int rc = alt_crypto_rand(rnd, sizeof rnd);
	if (rc < 0)
		return rc;
	for (uint32_t ctr = 0;; ctr++) {
		uint8_t out[128];
		size_t got = 0;
		for (uint8_t blk = 0; got < (size_t)c->q.bytes; blk++) {
			alt_hmac m;
			alt_hmac_init(&m, ALT_SHA512, priv, (size_t)c->q.bytes);
			alt_hmac_write(&m, rnd, sizeof rnd);
			alt_hmac_write(&m, hash, nhash);
			alt_hmac_write(&m, &ctr, sizeof ctr);
			alt_hmac_write(&m, &blk, 1);
			alt_hmac_sum(&m, out + got);
			got += 64;
		}
		memcpy(k, out, (size_t)c->q.bytes);
		int excess = c->q.bytes * 8 - c->q.bits;
		k[0] &= (uint8_t)(0xff >> excess);
		if (scalar_ok(c, k))
			return 0;
	}
}

int alt_ecdsa_sign(alt_ec_curve cv, const uint8_t *priv, const void *hash, size_t nhash, uint8_t *sig) {
	const curve *c = get(cv);
	if (!c || !scalar_ok(c, priv))
		return -ALT_EKEY;
	uint64_t d[L], e[L], r[L], s[L], kk[L], ki[L], t[L];
	from_bytes(&c->q, d, priv);
	hash_scalar(c, e, hash, nhash);
	for (;;) {
		uint8_t k[ALT_EC_MAX_SCALAR], x[ALT_EC_MAX_SCALAR];
		int rc = nonce(c, priv, hash, nhash, k);
		if (rc < 0)
			return rc;
		point g, R;
		pt_base(c, &g);
		pt_mul(c, &R, &g, k);
		pt_affine(c, &R, x, 0);
		/* r = x mod q */
		uint64_t xl[L] = {0}, u[L], bw = 0, one[L] = {1};
		for (int i = 0; i < c->p.bytes; i++) {
			int bit = (c->p.bytes - 1 - i) * 8;
			xl[bit / 64] |= (uint64_t)x[i] << (bit % 64);
		}
		for (int i = 0; i < c->q.n; i++)
			u[i] = subb(xl[i], c->q.m[i], &bw);
		sel(xl, xl, u, bw, c->q.n);
		if (is_zero(xl, c->q.n))
			continue;
		mont_mul(&c->q, r, xl, c->q.rr);
		/* s = k^-1 (e + r d) */
		from_bytes(&c->q, kk, k);
		mod_inv(&c->q, ki, kk);
		mont_mul(&c->q, t, r, d);
		mod_add(&c->q, t, t, e);
		mont_mul(&c->q, s, ki, t);
		if (is_zero(s, c->q.n))
			continue;
		uint8_t rb[ALT_EC_MAX_SCALAR], sb[ALT_EC_MAX_SCALAR];
		to_bytes(&c->q, rb, r);
		to_bytes(&c->q, sb, s);
		(void)one;
		alt_buf b = ALT_BUF_INIT;
		alt_der_builder db;
		alt_der_builder_init(&db, &b);
		alt_der_begin(&db, ALT_ASN1_SEQUENCE);
		alt_der_add_uint_bytes(&db, rb, (size_t)c->q.bytes);
		alt_der_add_uint_bytes(&db, sb, (size_t)c->q.bytes);
		alt_der_end(&db);
		rc = alt_der_builder_err(&db);
		if (rc == 0)
			memcpy(sig, b.p, b.n);
		size_t n = b.n;
		alt_buf_free(&b);
		return rc < 0 ? rc : (int)n;
	}
}
