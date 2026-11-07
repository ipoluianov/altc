#include "check.h"
#include <alt/crypto/aes.h>
#include <alt/crypto/chacha20poly1305.h>
#include <alt/crypto/hash.h>
#include <alt/crypto/kdf.h>
#include <alt/encoding/hex.h>

static size_t unhex(const char *s, uint8_t *out) {
	alt_buf b = ALT_BUF_INIT;
	alt_hex_decode(&b, alt_str_cstr(s));
	memcpy(out, b.p, b.n);
	size_t n = b.n;
	alt_buf_free(&b);
	return n;
}

#define CHECK_HEX(p, n, want) \
	do { \
		alt_buf check_h = ALT_BUF_INIT; \
		alt_hex_encode(&check_h, (p), (n)); \
		CHECK_STR(alt_buf_str(&check_h), want); \
		alt_buf_free(&check_h); \
	} while (0)

static void hashes(void) {
	uint8_t d[64];
	alt_hash_data(ALT_SHA256, "abc", 3, d);
	CHECK_HEX(d, 32, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	alt_hash_data(ALT_SHA512, "abc", 3, d);
	CHECK_HEX(d, 64, "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd"
			 "454d4423643ce80e2a9ac94fa54ca49f");
	alt_hash_data(ALT_SHA1, "abc", 3, d);
	CHECK_HEX(d, 20, "a9993e364706816aba3e25717850c26c9cd0d89d");
	alt_hash_data(ALT_MD5, "", 0, d);
	CHECK_HEX(d, 16, "d41d8cd98f00b204e9800998ecf8427e");
	/* a million a's, written in odd pieces, summing in between */
	alt_hash h;
	alt_hash_init(&h, ALT_SHA256);
	char a[1000];
	memset(a, 'a', sizeof a);
	for (int i = 0; i < 1000; i++) {
		alt_hash_write(&h, a, 333);
		alt_hash_write(&h, a, 667);
		if (i % 100 == 0)
			alt_hash_sum(&h, d);
	}
	alt_hash_sum(&h, d);
	CHECK_HEX(d, 32, "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
	/* RFC 4231 case 2 */
	alt_hmac_data(ALT_SHA256, "Jefe", 4, "what do ya want for nothing?", 28, d);
	CHECK_HEX(d, 32, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
	/* RFC 5869 case 1 */
	uint8_t ikm[22], salt[13], info[10], okm[42];
	memset(ikm, 0x0b, sizeof ikm);
	for (int i = 0; i < 13; i++)
		salt[i] = (uint8_t)i;
	for (int i = 0; i < 10; i++)
		info[i] = (uint8_t)(0xf0 + i);
	CHECK_OK(alt_hkdf(ALT_SHA256, salt, 13, ikm, 22, info, 10, okm, 42));
	CHECK_HEX(okm, 42, "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865");
	/* RFC 6070 */
	CHECK_OK(alt_pbkdf2(ALT_SHA1, "password", 8, "salt", 4, 4096, okm, 20));
	CHECK_HEX(okm, 20, "4b007901b765489abead49d926f721d065a429c1");
}

static void rand_and_eq(void) {
	uint8_t a[64] = {0}, b[64] = {0};
	CHECK_OK(alt_crypto_rand(a, sizeof a));
	CHECK_OK(alt_crypto_rand(b, sizeof b));
	CHECK(memcmp(a, b, 64) != 0);
	CHECK(alt_ct_eq(a, a, 64));
	CHECK(!alt_ct_eq(a, b, 64));
	CHECK(alt_ct_eq(a, b, 0));
}

static void aes_vectors(void) {
	/* FIPS 197 appendix C */
	uint8_t key[32], pt[16], ct[16], back[16];
	unhex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", key);
	unhex("00112233445566778899aabbccddeeff", pt);
	const char *want[3] = {"69c4e0d86a7b0430d8cdb78070b4c55a", "dda97ca4864cdfe06eaf70a0ec0d7191", "8ea2b7ca516745bfeafc49904b496089"};
	for (int i = 0; i < 3; i++) {
		alt_aes c;
		CHECK_OK(alt_aes_init(&c, key, 16 + 8 * (size_t)i));
		alt_aes_encrypt(&c, ct, pt);
		CHECK_HEX(ct, 16, want[i]);
		alt_aes_decrypt(&c, back, ct);
		CHECK(memcmp(back, pt, 16) == 0);
	}
	CHECK_INT(alt_aes_init(&(alt_aes){0}, key, 15), -EINVAL);
	/* GCM spec test case 4 */
	uint8_t k[16], iv[12], p[60], ad[20], out[76];
	unhex("feffe9928665731c6d6a8f9467308308", k);
	unhex("cafebabefacedbaddecaf888", iv);
	unhex("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a721c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657"
	      "ba637b39",
		p);
	unhex("feedfacedeadbeeffeedfacedeadbeefabaddad2", ad);
	alt_gcm g;
	CHECK_OK(alt_gcm_init(&g, k, 16));
	alt_gcm_seal(&g, out, iv, 12, p, 60, ad, 20);
	CHECK_HEX(out, 76, "42831ec2217774244b7221b784d0d49ce3aa212f2c02a4e035c17e2329aca12e21d514b25466931c7d8f6a5aac84aa051ba30b39"
			   "6a0aac973d58e0915bc94fbc3221a5db94fae95ae7121a47");
	uint8_t dec[60];
	CHECK_OK(alt_gcm_open(&g, dec, iv, 12, out, 76, ad, 20));
	CHECK(memcmp(dec, p, 60) == 0);
	out[0] ^= 1;
	CHECK_INT(alt_gcm_open(&g, dec, iv, 12, out, 76, ad, 20), -ALT_EAUTH);
	CHECK_INT(alt_gcm_open(&g, dec, iv, 12, out, 15, ad, 20), -ALT_EAUTH);
	/* in place */
	memcpy(out, p, 60);
	alt_gcm_seal(&g, out, iv, 12, out, 60, ad, 20);
	CHECK_OK(alt_gcm_open(&g, out, iv, 12, out, 76, ad, 20));
	CHECK(memcmp(out, p, 60) == 0);
}

static void chacha_vectors(void) {
	/* RFC 8439 2.5.2 */
	uint8_t key[32], tag[16];
	unhex("85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b", key);
	alt_poly1305 p;
	alt_poly1305_init(&p, key);
	const char *msg = "Cryptographic Forum Research Group";
	alt_poly1305_write(&p, msg, 10);
	alt_poly1305_write(&p, msg + 10, strlen(msg) - 10);
	alt_poly1305_sum(&p, tag);
	CHECK_HEX(tag, 16, "a8061dc1305136c6c22b8baf0c0127a9");
	/* RFC 8439 2.4.2 */
	uint8_t nonce[12], out[200];
	unhex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", key);
	unhex("000000000000004a00000000", nonce);
	const char *sun = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
	alt_chacha20 c;
	alt_chacha20_init(&c, key, nonce, 1);
	alt_chacha20_xor(&c, out, sun, strlen(sun));
	CHECK_HEX(out, 16, "6e2e359a2568f98041ba0728dd0d6981");
	CHECK_HEX(out + 96, 18, "5af90bbf74a35be6b40b8eedf2785e42874d");
	/* RFC 8439 2.8.2 */
	uint8_t k2[32], n2[12], ad[12];
	unhex("808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f", k2);
	unhex("070000004041424344454647", n2);
	unhex("50515253c0c1c2c3c4c5c6c7", ad);
	alt_chacha20poly1305_seal(k2, n2, out, sun, strlen(sun), ad, 12);
	CHECK_HEX(out, 16, "d31a8d34648e60db7b86afbc53ef7ec2");
	CHECK_HEX(out + strlen(sun), 16, "1ae10b594f09e26a7e902ecbd0600691");
	uint8_t dec[200];
	CHECK_OK(alt_chacha20poly1305_open(k2, n2, dec, out, strlen(sun) + 16, ad, 12));
	CHECK(memcmp(dec, sun, strlen(sun)) == 0);
	ad[0] ^= 1;
	CHECK_INT(alt_chacha20poly1305_open(k2, n2, dec, out, strlen(sun) + 16, ad, 12), -ALT_EAUTH);
	/* draft-irtf-cfrg-xchacha A.3.1 */
	uint8_t xn[24];
	unhex("404142434445464748494a4b4c4d4e4f5051525354555657", xn);
	ad[0] ^= 1;
	alt_xchacha20poly1305_seal(k2, xn, out, sun, strlen(sun), ad, 12);
	CHECK_HEX(out, 16, "bd6d179d3e83d43b9576579493c0e939");
	CHECK_HEX(out + strlen(sun), 16, "c0875924c1c7987947deafd8780acf49");
	CHECK_OK(alt_xchacha20poly1305_open(k2, xn, dec, out, strlen(sun) + 16, ad, 12));
	/* the Poly1305 limbs at their maximum: the key's r of all ones (clamped) and s of all ones */
	memset(key, 0xff, 32);
	uint8_t ff[64];
	memset(ff, 0xff, 64);
	alt_poly1305_init(&p, key);
	alt_poly1305_write(&p, ff, 64);
	alt_poly1305_sum(&p, tag);
	CHECK_HEX(tag, 16, "900fe32bc15fa8d7bca8efe4c7e37eb1"); /* python cryptography */
}

int main(void) {
	RUN(hashes);
	RUN(rand_and_eq);
	RUN(aes_vectors);
	RUN(chacha_vectors);
	return check_done();
}
