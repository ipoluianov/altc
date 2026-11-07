/* AltC standard library: ChaCha20, Poly1305 and their AEAD (RFC 8439),
   with XChaCha20-Poly1305 (24-byte nonces), as golang.org/x/crypto's
   chacha20poly1305 */
#ifndef ALT_CRYPTO_CHACHA20POLY1305_H
#define ALT_CRYPTO_CHACHA20POLY1305_H

#include <alt/errors.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_CHACHA20_KEY = 32, ALT_CHACHA20_NONCE = 12, ALT_XCHACHA20_NONCE = 24, ALT_POLY1305_TAG = 16 };

typedef struct {
	uint32_t s[16];
	uint8_t ks[64];
	size_t used;
} alt_chacha20;

/* key 32 bytes, nonce 12 bytes, the block counter to start from */
void alt_chacha20_init(alt_chacha20 *c, const uint8_t key[32], const uint8_t nonce[12], uint32_t counter);
void alt_chacha20_xor(alt_chacha20 *c, void *dst, const void *src, size_t n);
/* HChaCha20: the subkey of XChaCha20 */
void alt_hchacha20(uint8_t out[32], const uint8_t key[32], const uint8_t nonce[16]);

typedef struct {
	uint64_t r[3], h[3], pad[2];
	uint8_t buf[16];
	size_t nbuf;
} alt_poly1305;

void alt_poly1305_init(alt_poly1305 *p, const uint8_t key[32]);
void alt_poly1305_write(alt_poly1305 *p, const void *m, size_t n);
void alt_poly1305_sum(alt_poly1305 *p, uint8_t tag[16]);

/* nonce of 12 bytes (or 24 with the x variants); dst gets n + 16 bytes */
void alt_chacha20poly1305_seal(const uint8_t key[32], const uint8_t *nonce, void *dst, const void *pt, size_t n, const void *ad,
	size_t nad);
/* ct holds n bytes including the tag: 0 or -ALT_EAUTH */
int alt_chacha20poly1305_open(const uint8_t key[32], const uint8_t *nonce, void *dst, const void *ct, size_t n, const void *ad,
	size_t nad);
void alt_xchacha20poly1305_seal(const uint8_t key[32], const uint8_t nonce[24], void *dst, const void *pt, size_t n,
	const void *ad, size_t nad);
int alt_xchacha20poly1305_open(const uint8_t key[32], const uint8_t nonce[24], void *dst, const void *ct, size_t n,
	const void *ad, size_t nad);

#ifdef __cplusplus
}
#endif

#endif
