/* math/big by identities (division, GCD, exponentiation, strings) and
   RSA key generation with encryption round trips (also run under the
   sanitizers; tests/stdbig_test.go compares with Go) */
#include "check.h"
#include <alt/crypto/rsa.h>
#include <alt/math/big.h>

static uint64_t seed = 88172645463325252ull;
static uint64_t rnd(void) {
	seed ^= seed << 13, seed ^= seed >> 7, seed ^= seed << 17;
	return seed;
}

static void random_big(alt_big *x, size_t maxwords) {
	size_t n = rnd() % (maxwords + 1);
	uint8_t b[8 * 300];
	for (size_t i = 0; i < 8 * n; i++)
		b[i] = (uint8_t)rnd();
	alt_big_set_bytes(x, b, 8 * n);
	if (rnd() & 1)
		alt_big_neg(x, x);
}

static void identities(void) {
	alt_big a = ALT_BIG_INIT, b = ALT_BIG_INIT, q = ALT_BIG_INIT, r = ALT_BIG_INIT, t = ALT_BIG_INIT, g = ALT_BIG_INIT,
		x = ALT_BIG_INIT, y = ALT_BIG_INIT, u = ALT_BIG_INIT;
	for (int i = 0; i < 400; i++) {
		random_big(&a, i % 5 == 0 ? 250 : 20);
		random_big(&b, i % 7 == 0 ? 200 : 20);
		if (!alt_big_sign(&b))
			alt_big_set_int64(&b, 7);
		/* a = q b + r, |r| < |b|, sign(r) = sign(a) */
		CHECK_OK(alt_big_quo_rem(&q, &r, &a, &b));
		alt_big_mul(&t, &q, &b);
		alt_big_add(&t, &t, &r);
		CHECK(alt_big_cmp(&t, &a) == 0);
		CHECK(alt_big_cmp_abs(&r, &b) < 0);
		/* Euclidean: 0 <= m < |b| */
		CHECK_OK(alt_big_div_mod(&q, &r, &a, &b));
		CHECK(alt_big_sign(&r) >= 0 && alt_big_cmp_abs(&r, &b) < 0);
		/* (a b) / b = a, also in place */
		alt_big_mul(&t, &a, &b);
		alt_big_quo(&t, &t, &b);
		CHECK(alt_big_cmp(&t, &a) == 0);
		/* Bézout */
		alt_big_gcd(&g, &x, &y, &a, &b);
		alt_big_mul(&t, &a, &x);
		alt_big_mul(&u, &b, &y);
		alt_big_add(&t, &t, &u);
		CHECK(alt_big_cmp(&t, &g) == 0);
		/* strings round trip */
		alt_buf s = ALT_BUF_INIT;
		int base = 2 + (int)(rnd() % 61);
		alt_big_append_text(&s, &a, base);
		CHECK_OK(alt_big_set_string(&t, alt_buf_str(&s), base));
		CHECK(alt_big_cmp(&t, &a) == 0);
		alt_buf_free(&s);
		/* shifts: (a << k) >> k = a */
		size_t k = rnd() % 200;
		alt_big_lsh(&t, &a, k);
		alt_big_rsh(&t, &t, k);
		CHECK(alt_big_cmp(&t, &a) == 0);
	}
	/* exponentiation: Montgomery against repeated multiplication */
	for (int i = 0; i < 60; i++) {
		random_big(&a, 12);
		random_big(&b, 12);
		alt_big_abs(&b, &b);
		alt_big_set_bit(&b, &b, 0, 1);
		if (alt_big_bit_len(&b) < 70)
			alt_big_lsh(&b, &b, 70), alt_big_set_bit(&b, &b, 0, 1);
		unsigned e = (unsigned)(rnd() % 40);
		alt_big_set_uint64(&q, e);
		CHECK_OK(alt_big_exp(&t, &a, &q, &b));
		alt_big_set_int64(&r, 1);
		for (unsigned j = 0; j < e; j++) {
			alt_big_mul(&r, &r, &a);
			alt_big_mod(&r, &r, &b);
		}
		CHECK(alt_big_cmp(&t, &r) == 0);
	}
	alt_big *all[] = {&a, &b, &q, &r, &t, &g, &x, &y, &u};
	for (size_t i = 0; i < 9; i++)
		alt_big_free(all[i]);
}

static void primes(void) {
	alt_big p = ALT_BIG_INIT;
	CHECK_OK(alt_big_set_string(&p, ALT_STR("170141183460469231731687303715884105727"), 10));
	CHECK(alt_big_probably_prime(&p, 10));
	CHECK_OK(alt_big_set_string(&p, ALT_STR("3825123056546413051"), 10)); /* a strong pseudoprime to several bases */
	CHECK(!alt_big_probably_prime(&p, 10));
	CHECK_OK(alt_big_random_prime(&p, 256));
	CHECK_INT(alt_big_bit_len(&p), 256);
	CHECK(alt_big_probably_prime(&p, 20));
	alt_big_free(&p);
}

static void rsa_keys(void) {
	static alt_rsa_key k;
	CHECK_OK(alt_rsa_generate(&k, 1024));
	CHECK_INT(k.k.pub.nlen, 128);
	uint8_t msg[40], ct[ALT_RSA_MAX_BYTES], back[ALT_RSA_MAX_BYTES], sig[ALT_RSA_MAX_BYTES];
	for (size_t i = 0; i < sizeof msg; i++)
		msg[i] = (uint8_t)i;
	size_t n;
	CHECK_OK(alt_rsa_encrypt_oaep(&k.k.pub, ALT_SHA256, msg, sizeof msg, "l", 1, ct));
	CHECK_OK(alt_rsa_decrypt_oaep(&k.k, ALT_SHA256, ct, 128, "l", 1, back, &n));
	CHECK(n == sizeof msg && memcmp(back, msg, n) == 0);
	CHECK_ERR(alt_rsa_decrypt_oaep(&k.k, ALT_SHA256, ct, 128, "m", 1, back, &n), -ALT_EAUTH);
	CHECK_OK(alt_rsa_encrypt_pkcs1v15(&k.k.pub, msg, sizeof msg, ct));
	CHECK_OK(alt_rsa_decrypt_pkcs1v15(&k.k, ct, 128, back, &n));
	CHECK(n == sizeof msg && memcmp(back, msg, n) == 0);
	ct[5] ^= 1;
	CHECK_ERR(alt_rsa_decrypt_pkcs1v15(&k.k, ct, 128, back, &n), -ALT_EAUTH);
	uint8_t h[32] = {1, 2, 3};
	CHECK_OK(alt_rsa_sign_pss(&k.k, ALT_SHA256, h, 32, sig, -1));
	CHECK_OK(alt_rsa_verify_pss(&k.k.pub, ALT_SHA256, h, 32, sig, 128, -1));
	CHECK_ERR(alt_rsa_encrypt_pkcs1v15(&k.k.pub, back, 118, ct), -EMSGSIZE);
}

int main(void) {
	RUN(identities);
	RUN(primes);
	RUN(rsa_keys);
	return check_done();
}
