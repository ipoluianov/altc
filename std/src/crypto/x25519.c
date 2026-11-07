/* X25519 (RFC 7748): the Montgomery ladder over GF(2^255 - 19) */
#include "fe25519.h"
#include <alt/crypto/ed25519.h>

static void ladder(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]) {
	uint8_t e[32];
	memcpy(e, scalar, 32);
	e[0] &= 248;
	e[31] &= 127;
	e[31] |= 64;
	fe x1, x2 = fe_one, z2 = fe_zero, x3, z3 = fe_one, t0, t1;
	fe_from_bytes(&x1, point);
	x3 = x1;
	int swap = 0;
	for (int pos = 254; pos >= 0; pos--) {
		int b = e[pos / 8] >> (pos & 7) & 1;
		swap ^= b;
		fe_swap(&x2, &x3, swap);
		fe_swap(&z2, &z3, swap);
		swap = b;
		fe_sub(&t0, &x3, &z3);
		fe_sub(&t1, &x2, &z2);
		fe_add(&x2, &x2, &z2);
		fe_add(&z2, &x3, &z3);
		fe_mul(&z3, &t0, &x2);
		fe_mul(&z2, &z2, &t1);
		fe_sq(&t0, &t1);
		fe_sq(&t1, &x2);
		fe_add(&x3, &z3, &z2);
		fe_sub(&z2, &z3, &z2);
		fe_mul(&x2, &t1, &t0);
		fe_sub(&t1, &t1, &t0);
		fe_sq(&z2, &z2);
		fe_mul32(&z3, &t1, 121666);
		fe_sq(&x3, &x3);
		fe_add(&t0, &t0, &z3);
		fe_mul(&z3, &x1, &z2);
		fe_mul(&z2, &t1, &t0);
	}
	fe_swap(&x2, &x3, swap);
	fe_swap(&z2, &z3, swap);
	fe_invert(&z2, &z2);
	fe_mul(&x2, &x2, &z2);
	fe_to_bytes(out, &x2);
	memset(e, 0, sizeof e);
}

int alt_x25519(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]) {
	ladder(out, scalar, point);
	uint8_t z = 0;
	for (int i = 0; i < 32; i++)
		z |= out[i];
	return z ? 0 : -ALT_EKEY;
}

void alt_x25519_base(uint8_t pub[32], const uint8_t priv[32]) {
	static const uint8_t nine[32] = {9};
	ladder(pub, priv, nine);
}
