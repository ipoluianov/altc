/* AltC standard library: Ed25519 signatures (RFC 8032; Go's
   crypto/ed25519) and X25519 key agreement (RFC 7748; crypto/ecdh's
   X25519). Private Ed25519 keys are 64 bytes as in Go: the 32-byte seed
   followed by the public key. */
#ifndef ALT_CRYPTO_ED25519_H
#define ALT_CRYPTO_ED25519_H

#include <alt/errors.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_ED25519_PUBLIC = 32, ALT_ED25519_PRIVATE = 64, ALT_ED25519_SEED = 32, ALT_ED25519_SIG = 64, ALT_X25519_SIZE = 32 };

/* the key pair of a seed (random when seed is NULL): 0 or the error of
   the random source */
int alt_ed25519_keygen(uint8_t pub[32], uint8_t priv[64], const uint8_t *seed);
void alt_ed25519_sign(uint8_t sig[64], const uint8_t priv[64], const void *msg, size_t n);
/* 0, -ALT_EVERIFY, or -ALT_EKEY for a public key that is no point */
int alt_ed25519_verify(const uint8_t pub[32], const void *msg, size_t n, const uint8_t sig[64]);

/* X25519(scalar, point): -ALT_EKEY when the result is all zeros (a
   low-order point), as Go */
int alt_x25519(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]);
/* the public key of a private one */
void alt_x25519_base(uint8_t pub[32], const uint8_t priv[32]);

#ifdef __cplusplus
}
#endif

#endif
