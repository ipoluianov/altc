/* AltC standard library: AES (FIPS 197) and its modes: CTR, CBC
   (SP 800-38A) and GCM (SP 800-38D), as Go's crypto/aes and
   crypto/cipher.

   AES uses the AES-NI instructions when the processor has them (constant
   time), else tables (as Go's generic code). GCM's GHASH is constant
   time. */
#ifndef ALT_CRYPTO_AES_H
#define ALT_CRYPTO_AES_H

#include <alt/errors.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_AES_BLOCK = 16, ALT_GCM_NONCE = 12, ALT_GCM_TAG = 16 };

typedef struct {
	uint32_t enc[60], dec[60]; /* round keys, big-endian words */
	int rounds;
	bool ni;
} alt_aes;

/* a 16, 24 or 32-byte key: 0 or -EINVAL */
int alt_aes_init(alt_aes *c, const void *key, size_t n);
void alt_aes_encrypt(const alt_aes *c, void *dst, const void *src);
void alt_aes_decrypt(const alt_aes *c, void *dst, const void *src);

/* CTR mode: the counter is the whole block, big-endian (as Go's NewCTR) */
typedef struct {
	const alt_aes *c;
	uint8_t ctr[16], ks[16];
	size_t used;               /* of the key stream block ks */
} alt_aes_ctr;
void alt_aes_ctr_init(alt_aes_ctr *s, const alt_aes *c, const uint8_t iv[16]);
/* dst = src ^ key stream (dst may be src) */
void alt_aes_ctr_xor(alt_aes_ctr *s, void *dst, const void *src, size_t n);

/* CBC: n a multiple of 16 (else -EINVAL); iv is updated for the next call */
int alt_aes_cbc_encrypt(const alt_aes *c, uint8_t iv[16], void *dst, const void *src, size_t n);
int alt_aes_cbc_decrypt(const alt_aes *c, uint8_t iv[16], void *dst, const void *src, size_t n);

typedef struct {
	alt_aes aes;
	uint8_t h[16];             /* the hash key E(K, 0) */
} alt_gcm;

int alt_gcm_init(alt_gcm *g, const void *key, size_t n);
/* writes n bytes of ciphertext and the 16-byte tag to dst (which may be
   pt); a nonce of any non-zero length, 12 bytes being the standard */
void alt_gcm_seal(const alt_gcm *g, void *dst, const void *nonce, size_t nnonce, const void *pt, size_t n, const void *ad,
	size_t nad);
/* ct is n bytes including the tag: writes n - 16 bytes to dst (which may
   be ct): 0, or -ALT_EAUTH (dst is then cleared) */
int alt_gcm_open(const alt_gcm *g, void *dst, const void *nonce, size_t nnonce, const void *ct, size_t n, const void *ad,
	size_t nad);

#ifdef __cplusplus
}
#endif

#endif
