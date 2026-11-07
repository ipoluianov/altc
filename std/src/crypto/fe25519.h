/* Arithmetic modulo 2^255 - 19 with five 51-bit limbs, translated from
   Go's crypto/internal/fips140/edwards25519/field (BSD-3-Clause, see
   LICENSE-Go). Constant time. */
#ifndef ALT_CRYPTO_FE25519_H
#define ALT_CRYPTO_FE25519_H

#include <stdint.h>
#include <string.h>

typedef struct {
	uint64_t l[5];
} fe;

typedef unsigned __int128 u128;
#define M51 ((1ull << 51) - 1)

static const fe fe_zero = {{0}}, fe_one = {{1}};

static inline uint64_t fe_le64(const uint8_t *p) {
	uint64_t v = 0;
	for (int i = 7; i >= 0; i--)
		v = v << 8 | p[i];
	return v;
}

static inline void fe_carry(fe *v) {
	uint64_t l0 = v->l[0];
	v->l[0] = (v->l[0] & M51) + 19 * (v->l[4] >> 51);
	v->l[4] = (v->l[4] & M51) + (v->l[3] >> 51);
	v->l[3] = (v->l[3] & M51) + (v->l[2] >> 51);
	v->l[2] = (v->l[2] & M51) + (v->l[1] >> 51);
	v->l[1] = (v->l[1] & M51) + (l0 >> 51);
}

static inline void fe_reduce(fe *v) {
	fe_carry(v);
	uint64_t c = (v->l[0] + 19) >> 51;
	c = (v->l[1] + c) >> 51;
	c = (v->l[2] + c) >> 51;
	c = (v->l[3] + c) >> 51;
	c = (v->l[4] + c) >> 51;
	v->l[0] += 19 * c;
	v->l[1] += v->l[0] >> 51;
	v->l[0] &= M51;
	v->l[2] += v->l[1] >> 51;
	v->l[1] &= M51;
	v->l[3] += v->l[2] >> 51;
	v->l[2] &= M51;
	v->l[4] += v->l[3] >> 51;
	v->l[3] &= M51;
	v->l[4] &= M51;
}

static inline void fe_add(fe *v, const fe *a, const fe *b) {
	for (int i = 0; i < 5; i++)
		v->l[i] = a->l[i] + b->l[i];
	fe_carry(v);
}

static inline void fe_sub(fe *v, const fe *a, const fe *b) {
	v->l[0] = (a->l[0] + 0xFFFFFFFFFFFDAull) - b->l[0];
	for (int i = 1; i < 5; i++)
		v->l[i] = (a->l[i] + 0xFFFFFFFFFFFFEull) - b->l[i];
	fe_carry(v);
}

static inline void fe_neg(fe *v, const fe *a) { fe_sub(v, &fe_zero, a); }

/* the 128-bit products reduced to limbs (Go's feMulGeneric tail) */
static inline void fe_finish(fe *v, u128 r0, u128 r1, u128 r2, u128 r3, u128 r4) {
	uint64_t c0 = (uint64_t)(r0 >> 51), c1 = (uint64_t)(r1 >> 51), c2 = (uint64_t)(r2 >> 51), c3 = (uint64_t)(r3 >> 51),
		 c4 = (uint64_t)(r4 >> 51);
	uint64_t rr0 = ((uint64_t)r0 & M51) + 19 * c4, rr1 = ((uint64_t)r1 & M51) + c0, rr2 = ((uint64_t)r2 & M51) + c1,
		 rr3 = ((uint64_t)r3 & M51) + c2, rr4 = ((uint64_t)r4 & M51) + c3;
	v->l[0] = (rr0 & M51) + 19 * (rr4 >> 51);
	v->l[1] = (rr1 & M51) + (rr0 >> 51);
	v->l[2] = (rr2 & M51) + (rr1 >> 51);
	v->l[3] = (rr3 & M51) + (rr2 >> 51);
	v->l[4] = (rr4 & M51) + (rr3 >> 51);
}

static inline void fe_mul(fe *v, const fe *a, const fe *b) {
	uint64_t a0 = a->l[0], a1 = a->l[1], a2 = a->l[2], a3 = a->l[3], a4 = a->l[4];
	uint64_t b0 = b->l[0], b1 = b->l[1], b2 = b->l[2], b3 = b->l[3], b4 = b->l[4];
	uint64_t a1_19 = 19 * a1, a2_19 = 19 * a2, a3_19 = 19 * a3, a4_19 = 19 * a4;
	u128 r0 = (u128)a0 * b0 + (u128)a1_19 * b4 + (u128)a2_19 * b3 + (u128)a3_19 * b2 + (u128)a4_19 * b1;
	u128 r1 = (u128)a0 * b1 + (u128)a1 * b0 + (u128)a2_19 * b4 + (u128)a3_19 * b3 + (u128)a4_19 * b2;
	u128 r2 = (u128)a0 * b2 + (u128)a1 * b1 + (u128)a2 * b0 + (u128)a3_19 * b4 + (u128)a4_19 * b3;
	u128 r3 = (u128)a0 * b3 + (u128)a1 * b2 + (u128)a2 * b1 + (u128)a3 * b0 + (u128)a4_19 * b4;
	u128 r4 = (u128)a0 * b4 + (u128)a1 * b3 + (u128)a2 * b2 + (u128)a3 * b1 + (u128)a4 * b0;
	fe_finish(v, r0, r1, r2, r3, r4);
}

static inline void fe_sq(fe *v, const fe *a) {
	uint64_t l0 = a->l[0], l1 = a->l[1], l2 = a->l[2], l3 = a->l[3], l4 = a->l[4];
	u128 r0 = (u128)l0 * l0 + (u128)(38 * l1) * l4 + (u128)(38 * l2) * l3;
	u128 r1 = (u128)(2 * l0) * l1 + (u128)(38 * l2) * l4 + (u128)(19 * l3) * l3;
	u128 r2 = (u128)(2 * l0) * l2 + (u128)l1 * l1 + (u128)(38 * l3) * l4;
	u128 r3 = (u128)(2 * l0) * l3 + (u128)(2 * l1) * l2 + (u128)(19 * l4) * l4;
	u128 r4 = (u128)(2 * l0) * l4 + (u128)(2 * l1) * l3 + (u128)l2 * l2;
	fe_finish(v, r0, r1, r2, r3, r4);
}

static inline void fe_sqn(fe *v, const fe *a, int n) {
	*v = *a;
	while (n--)
		fe_sq(v, v);
}

/* v * y for y < 2^32 */
static inline void fe_mul32(fe *v, const fe *x, uint32_t y) {
	uint64_t lo[5], hi[5];
	for (int i = 0; i < 5; i++) {
		u128 m = (u128)x->l[i] * y;
		lo[i] = (uint64_t)m & M51;
		hi[i] = (uint64_t)(m >> 51);
	}
	v->l[0] = lo[0] + 19 * hi[4];
	v->l[1] = lo[1] + hi[0];
	v->l[2] = lo[2] + hi[1];
	v->l[3] = lo[3] + hi[2];
	v->l[4] = lo[4] + hi[3];
}

static inline void fe_invert(fe *v, const fe *z) {
	fe z11, t0, t1, t2, t;
	fe_sq(&t1, z);
	fe_sq(&t, &t1);
	fe_sq(&t, &t);
	fe_mul(&t2, &t, z);
	fe_mul(&z11, &t2, &t1);
	fe_sq(&t, &z11);
	fe_mul(&t0, &t, &t2);
	fe_sqn(&t, &t0, 5);
	fe_mul(&t2, &t, &t0);
	fe_sqn(&t, &t2, 10);
	fe_mul(&t0, &t, &t2);
	fe_sqn(&t, &t0, 20);
	fe_mul(&t1, &t, &t0);
	fe_sqn(&t, &t1, 10);
	fe_mul(&t0, &t, &t2);
	fe_sqn(&t, &t0, 50);
	fe_mul(&t1, &t, &t0);
	fe_sqn(&t, &t1, 100);
	fe_mul(&t, &t, &t1);
	fe_sqn(&t, &t, 50);
	fe_mul(&t, &t, &t0);
	fe_sqn(&t, &t, 5);
	fe_mul(v, &t, &z11);
}

/* x^((p-5)/8) = x^(2^252 - 3) */
static inline void fe_pow22523(fe *v, const fe *x) {
	fe t0, t1, t2;
	fe_sq(&t0, x);
	fe_mul(&t1, x, &t0);
	fe_sq(&t0, &t1);
	fe_sq(&t0, &t0);
	fe_mul(&t0, &t1, &t0);
	fe_sq(&t0, &t0);
	fe_mul(&t0, x, &t0);
	fe_sqn(&t1, &t0, 5);
	fe_mul(&t1, &t1, &t0);
	fe_sqn(&t2, &t1, 5);
	fe_mul(&t0, &t2, &t0);
	fe_sqn(&t2, &t0, 15);
	fe_mul(&t2, &t2, &t0);
	fe_sqn(&t0, &t2, 30);
	fe_mul(&t0, &t0, &t2);
	fe_sqn(&t2, &t0, 60);
	fe_mul(&t2, &t2, &t0);
	fe_sqn(&t0, &t2, 120);
	fe_mul(&t0, &t0, &t2);
	fe_sqn(&t0, &t0, 10);
	fe_mul(&t0, &t0, &t1);
	fe_sqn(&t0, &t0, 2);
	fe_mul(v, &t0, x);
}

/* 255 bits, little endian; the top bit is ignored; non-canonical values are accepted */
static inline void fe_from_bytes(fe *v, const uint8_t *x) {
	v->l[0] = fe_le64(x) & M51;
	v->l[1] = (fe_le64(x + 6) >> 3) & M51;
	v->l[2] = (fe_le64(x + 12) >> 6) & M51;
	v->l[3] = (fe_le64(x + 19) >> 1) & M51;
	v->l[4] = (fe_le64(x + 24) >> 12) & M51;
}

static inline void fe_to_bytes(uint8_t *out, const fe *v) {
	fe t = *v;
	fe_reduce(&t);
	uint64_t u[4] = {t.l[1] << 51 | t.l[0], t.l[2] << 38 | t.l[1] >> 13, t.l[3] << 25 | t.l[2] >> 26, t.l[4] << 12 | t.l[3] >> 39};
	for (int i = 0; i < 4; i++)
		for (int k = 0; k < 8; k++)
			out[8 * i + k] = (uint8_t)(u[i] >> 8 * k);
}

/* 1 when equal (constant time) */
static inline int fe_eq(const fe *a, const fe *b) {
	uint8_t x[32], y[32], d = 0;
	fe_to_bytes(x, a);
	fe_to_bytes(y, b);
	for (int i = 0; i < 32; i++)
		d |= x[i] ^ y[i];
	return ((d - 1) >> 8) & 1;
}

static inline int fe_is_negative(const fe *v) {
	uint8_t b[32];
	fe_to_bytes(b, v);
	return b[0] & 1;
}

/* v = cond ? a : b */
static inline void fe_select(fe *v, const fe *a, const fe *b, int cond) {
	uint64_t m = ~((uint64_t)cond - 1);
	for (int i = 0; i < 5; i++)
		v->l[i] = (m & a->l[i]) | (~m & b->l[i]);
}

static inline void fe_swap(fe *a, fe *b, int cond) {
	uint64_t m = ~((uint64_t)cond - 1);
	for (int i = 0; i < 5; i++) {
		uint64_t t = m & (a->l[i] ^ b->l[i]);
		a->l[i] ^= t;
		b->l[i] ^= t;
	}
}

static inline void fe_abs(fe *v, const fe *u) {
	fe n;
	fe_neg(&n, u);
	fe_select(v, &n, u, fe_is_negative(u));
}

static const fe fe_sqrt_m1 = {{1718705420411056ull, 234908883556509ull, 2233514472574048ull, 2117202627021982ull, 765476049583133ull}};

/* the non-negative square root of u/v when it exists: returns whether
   u/v was a square (Go's SqrtRatio) */
static inline int fe_sqrt_ratio(fe *r, const fe *u, const fe *v) {
	fe t0, v2, uv3, uv7, rr, check, uneg, rp;
	fe_sq(&v2, v);
	fe_mul(&t0, &v2, v);
	fe_mul(&uv3, u, &t0);
	fe_sq(&t0, &v2);
	fe_mul(&uv7, &uv3, &t0);
	fe_pow22523(&t0, &uv7);
	fe_mul(&rr, &uv3, &t0);
	fe_sq(&t0, &rr);
	fe_mul(&check, v, &t0);
	fe_neg(&uneg, u);
	int correct = fe_eq(&check, u), flipped = fe_eq(&check, &uneg);
	fe_mul(&t0, &uneg, &fe_sqrt_m1);
	int flipped_i = fe_eq(&check, &t0);
	fe_mul(&rp, &rr, &fe_sqrt_m1);
	fe_select(&rr, &rp, &rr, flipped | flipped_i);
	fe_abs(r, &rr);
	return correct | flipped;
}

#endif
