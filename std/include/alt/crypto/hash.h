/* AltC standard library: cryptographic hashes and HMAC (Go's crypto/md5,
   crypto/sha1, crypto/sha256, crypto/sha512, crypto/hmac), random bytes
   (crypto/rand) and constant-time comparison (crypto/subtle).

   A hash is used as in Go: init, write any number of times, sum (which
   does not change the state, so writing may go on). */
#ifndef ALT_CRYPTO_HASH_H
#define ALT_CRYPTO_HASH_H

#include <alt/errors.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ALT_MD5,
	ALT_SHA1,
	ALT_SHA224,
	ALT_SHA256,
	ALT_SHA384,
	ALT_SHA512,
	ALT_SHA512_256,
} alt_hash_kind;

enum { ALT_HASH_MAX_SIZE = 64, ALT_HASH_MAX_BLOCK = 128 };

typedef struct {
	alt_hash_kind kind;
	union {
		uint32_t h32[8];
		uint64_t h64[8];
	};
	uint8_t buf[128];
	size_t nbuf;
	uint64_t len; /* bytes written (2^64 is enough: 2^61 bytes is the SHA-1 limit) */
} alt_hash;

void alt_hash_init(alt_hash *h, alt_hash_kind kind);
void alt_hash_write(alt_hash *h, const void *p, size_t n);
/* the digest (alt_hash_size bytes) of what was written so far */
void alt_hash_sum(const alt_hash *h, uint8_t *out);
size_t alt_hash_size(alt_hash_kind kind);
size_t alt_hash_block_size(alt_hash_kind kind);
/* the digest of one buffer */
void alt_hash_data(alt_hash_kind kind, const void *p, size_t n, uint8_t *out);

/* HMAC (RFC 2104) */
typedef struct {
	alt_hash inner, outer;
} alt_hmac;

void alt_hmac_init(alt_hmac *m, alt_hash_kind kind, const void *key, size_t keylen);
void alt_hmac_write(alt_hmac *m, const void *p, size_t n);
void alt_hmac_sum(const alt_hmac *m, uint8_t *out);
void alt_hmac_data(alt_hash_kind kind, const void *key, size_t keylen, const void *p, size_t n, uint8_t *out);

/* n random bytes from the kernel: 0 or -errno */
int alt_crypto_rand(void *buf, size_t n);
/* 1 when a and b are equal, in time independent of their contents */
int alt_ct_eq(const void *a, const void *b, size_t n);

#ifdef __cplusplus
}
#endif

#endif
