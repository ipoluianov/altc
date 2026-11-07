/* Ed25519 (RFC 8032) on the twisted Edwards curve -x^2 + y^2 = 1 + d x^2 y^2
   in extended coordinates. Scalar multiplication adds at every bit
   (constant time); scalars are reduced modulo L in radix 2^8. Encoding
   and verification follow Go's crypto/ed25519: non-canonical public
   keys are accepted, S must be below L, R is compared by its encoding. */
#include "fe25519.h"
#include <alt/crypto/ed25519.h>
#include <alt/crypto/hash.h>

typedef struct {
	fe x, y, z, t;
} ge;

static const uint8_t d_bytes[32] = {0xa3, 0x78, 0x59, 0x13, 0xca, 0x4d, 0xeb, 0x75, 0xab, 0xd8, 0x41, 0x41, 0x4d, 0x0a, 0x70, 0x00,
	0x98, 0xe8, 0x79, 0x77, 0x79, 0x40, 0xc7, 0x8c, 0x73, 0xfe, 0x6f, 0x2b, 0xee, 0x6c, 0x03, 0x52};
static const uint8_t base_bytes[32] = {0x58, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66};
static const uint8_t L[32] = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10};

static void ge_identity(ge *p) {
	p->x = fe_zero, p->y = fe_one, p->z = fe_one, p->t = fe_zero;
}

static void ge_add(ge *r, const ge *p, const ge *q) {
	fe a, b, c, d, e, f, g, h, t, d2;
	fe_from_bytes(&d2, d_bytes);
	fe_add(&d2, &d2, &d2);
	fe_sub(&a, &p->y, &p->x);
	fe_sub(&t, &q->y, &q->x);
	fe_mul(&a, &a, &t);
	fe_add(&b, &p->y, &p->x);
	fe_add(&t, &q->y, &q->x);
	fe_mul(&b, &b, &t);
	fe_mul(&c, &p->t, &q->t);
	fe_mul(&c, &c, &d2);
	fe_mul(&d, &p->z, &q->z);
	fe_add(&d, &d, &d);
	fe_sub(&e, &b, &a);
	fe_sub(&f, &d, &c);
	fe_add(&g, &d, &c);
	fe_add(&h, &b, &a);
	fe_mul(&r->x, &e, &f);
	fe_mul(&r->y, &g, &h);
	fe_mul(&r->t, &e, &h);
	fe_mul(&r->z, &f, &g);
}

static void ge_double(ge *r, const ge *p) {
	fe a, b, c, e, f, g, h, t;
	fe_sq(&a, &p->x);
	fe_sq(&b, &p->y);
	fe_sq(&c, &p->z);
	fe_add(&c, &c, &c);
	fe_add(&h, &a, &b);
	fe_add(&t, &p->x, &p->y);
	fe_sq(&t, &t);
	fe_sub(&e, &h, &t);
	fe_sub(&g, &a, &b);
	fe_add(&f, &c, &g);
	fe_mul(&r->x, &e, &f);
	fe_mul(&r->y, &g, &h);
	fe_mul(&r->t, &e, &h);
	fe_mul(&r->z, &f, &g);
}

static void ge_select(ge *r, const ge *a, const ge *b, int cond) {
	fe_select(&r->x, &a->x, &b->x, cond);
	fe_select(&r->y, &a->y, &b->y, cond);
	fe_select(&r->z, &a->z, &b->z, cond);
	fe_select(&r->t, &a->t, &b->t, cond);
}

/* [s]p for a 256-bit little-endian s: an addition at every bit */
static void ge_mul(ge *r, const ge *p, const uint8_t s[32]) {
	ge acc, sum;
	ge_identity(&acc);
	for (int i = 255; i >= 0; i--) {
		ge_double(&acc, &acc);
		ge_add(&sum, &acc, p);
		ge_select(&acc, &sum, &acc, s[i / 8] >> (i & 7) & 1);
	}
	*r = acc;
}

static void ge_to_bytes(uint8_t out[32], const ge *p) {
	fe zi, x, y;
	fe_invert(&zi, &p->z);
	fe_mul(&x, &p->x, &zi);
	fe_mul(&y, &p->y, &zi);
	fe_to_bytes(out, &y);
	out[31] |= (uint8_t)(fe_is_negative(&x) << 7);
}

/* Go's Point.SetBytes */
static int ge_from_bytes(ge *p, const uint8_t in[32]) {
	fe y, y2, u, v, x, xn, d;
	fe_from_bytes(&y, in);
	fe_from_bytes(&d, d_bytes);
	fe_sq(&y2, &y);
	fe_sub(&u, &y2, &fe_one);
	fe_mul(&v, &y2, &d);
	fe_add(&v, &v, &fe_one);
	if (!fe_sqrt_ratio(&x, &u, &v))
		return -ALT_EKEY;
	fe_neg(&xn, &x);
	fe_select(&x, &xn, &x, in[31] >> 7);
	p->x = x, p->y = y, p->z = fe_one;
	fe_mul(&p->t, &x, &y);
	return 0;
}

static void ge_base(ge *b) { ge_from_bytes(b, base_bytes); }

/* ---- scalars modulo L ---- */

/* x (64 signed radix-2^8 digits) mod L into r */
static void mod_l(uint8_t r[32], int64_t x[64]) {
	for (int i = 63; i >= 32; i--) {
		int64_t carry = 0;
		int j;
		for (j = i - 32; j < i - 12; j++) {
			x[j] += carry - 16 * x[i] * L[j - (i - 32)];
			carry = (x[j] + 128) >> 8;
			x[j] -= carry * 256;
		}
		x[j] += carry;
		x[i] = 0;
	}
	int64_t carry = 0;
	for (int j = 0; j < 32; j++) {
		x[j] += carry - (x[31] >> 4) * L[j];
		carry = x[j] >> 8;
		x[j] &= 255;
	}
	for (int j = 0; j < 32; j++)
		x[j] -= carry * L[j];
	for (int i = 0; i < 32; i++) {
		x[i + 1] += x[i] >> 8;
		r[i] = (uint8_t)(x[i] & 255);
	}
}

static void reduce64(uint8_t r[32], const uint8_t h[64]) {
	int64_t x[64];
	for (int i = 0; i < 64; i++)
		x[i] = h[i];
	mod_l(r, x);
}

/* (a * b + c) mod L */
static void mul_add(uint8_t r[32], const uint8_t a[32], const uint8_t b[32], const uint8_t c[32]) {
	int64_t x[64] = {0};
	for (int i = 0; i < 32; i++)
		x[i] = c[i];
	for (int i = 0; i < 32; i++)
		for (int j = 0; j < 32; j++)
			x[i + j] += (int64_t)a[i] * b[j];
	mod_l(r, x);
}

static int below_l(const uint8_t s[32]) {
	for (int i = 31; i >= 0; i--) {
		if (s[i] < L[i])
			return 1;
		if (s[i] > L[i])
			return 0;
	}
	return 0;
}

/* ---- the scheme ---- */

int alt_ed25519_keygen(uint8_t pub[32], uint8_t priv[64], const uint8_t *seed) {
	uint8_t s[32];
	if (seed)
		memcpy(s, seed, 32);
	else {
		int rc = alt_crypto_rand(s, 32);
		if (rc < 0)
			return rc;
	}
	uint8_t h[64];
	alt_hash_data(ALT_SHA512, s, 32, h);
	h[0] &= 248;
	h[31] &= 127;
	h[31] |= 64;
	ge b, a;
	ge_base(&b);
	ge_mul(&a, &b, h);
	ge_to_bytes(pub, &a);
	memcpy(priv, s, 32);
	memcpy(priv + 32, pub, 32);
	memset(h, 0, sizeof h);
	memset(s, 0, sizeof s);
	return 0;
}

void alt_ed25519_sign(uint8_t sig[64], const uint8_t priv[64], const void *msg, size_t n) {
	uint8_t h[64], r[32], k[32], rh[64];
	alt_hash_data(ALT_SHA512, priv, 32, h);
	h[0] &= 248;
	h[31] &= 127;
	h[31] |= 64;
	alt_hash hs;
	alt_hash_init(&hs, ALT_SHA512);
	alt_hash_write(&hs, h + 32, 32);
	alt_hash_write(&hs, msg, n);
	alt_hash_sum(&hs, rh);
	reduce64(r, rh);
	ge b, R;
	ge_base(&b);
	ge_mul(&R, &b, r);
	ge_to_bytes(sig, &R);
	alt_hash_init(&hs, ALT_SHA512);
	alt_hash_write(&hs, sig, 32);
	alt_hash_write(&hs, priv + 32, 32);
	alt_hash_write(&hs, msg, n);
	alt_hash_sum(&hs, rh);
	reduce64(k, rh);
	/* a mod L: the clamped scalar is up to 2^255 */
	uint8_t a[32], zero[32] = {0}, one[32] = {1};
	mul_add(a, h, one, zero);
	mul_add(sig + 32, k, a, r);
	memset(h, 0, sizeof h);
	memset(r, 0, sizeof r);
	memset(a, 0, sizeof a);
}

int alt_ed25519_verify(const uint8_t pub[32], const void *msg, size_t n, const uint8_t sig[64]) {
	if (sig[63] & 224)
		return -ALT_EVERIFY;
	ge A;
	if (ge_from_bytes(&A, pub) < 0)
		return -ALT_EKEY;
	uint8_t kh[64], k[32];
	alt_hash hs;
	alt_hash_init(&hs, ALT_SHA512);
	alt_hash_write(&hs, sig, 32);
	alt_hash_write(&hs, pub, 32);
	alt_hash_write(&hs, msg, n);
	alt_hash_sum(&hs, kh);
	reduce64(k, kh);
	if (!below_l(sig + 32))
		return -ALT_EVERIFY;
	/* [S]B - [k]A must be R */
	fe_neg(&A.x, &A.x);
	fe_neg(&A.t, &A.t);
	ge b, sb, ka, r;
	ge_base(&b);
	ge_mul(&sb, &b, sig + 32);
	ge_mul(&ka, &A, k);
	ge_add(&r, &sb, &ka);
	uint8_t rb[32];
	ge_to_bytes(rb, &r);
	return memcmp(rb, sig, 32) == 0 ? 0 : -ALT_EVERIFY;
}
