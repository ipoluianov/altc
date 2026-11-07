/* AltC standard library: the NIST curves P-256, P-384 and P-521 (FIPS
   186-5, SP 800-186): ECDSA signatures (Go's crypto/ecdsa, ASN.1
   signatures) and ECDH (crypto/ecdh). Constant time for secrets.

   Private keys are big-endian scalars of the field size (32, 48, 66
   bytes); public keys are uncompressed points (0x04 || X || Y). */
#ifndef ALT_CRYPTO_ECDSA_H
#define ALT_CRYPTO_ECDSA_H

#include <alt/errors.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { ALT_P256 = 1, ALT_P384, ALT_P521 } alt_ec_curve;

enum { ALT_EC_MAX_SCALAR = 66, ALT_EC_MAX_POINT = 133, ALT_ECDSA_MAX_SIG = 139 };

/* the size of a scalar (and coordinate), of a public key */
size_t alt_ec_scalar_size(alt_ec_curve c);
size_t alt_ec_point_size(alt_ec_curve c);

/* a random key pair: 0 or the error of the random source */
int alt_ec_keygen(alt_ec_curve c, uint8_t *priv, uint8_t *pub);
/* the public key of priv: 0 or -ALT_EKEY (zero or not below the order) */
int alt_ec_public(alt_ec_curve c, const uint8_t *priv, uint8_t *pub);
/* whether pub is a valid uncompressed point on the curve */
bool alt_ec_point_valid(alt_ec_curve c, const uint8_t *pub, size_t n);

/* signs a hash (any length: truncated to the order's size as FIPS
   186 says): an ASN.1 signature in sig (ALT_ECDSA_MAX_SIG bytes), its
   length, or an error. The nonce is derived from the key, the hash and
   random bytes (hedged, as Go). */
int alt_ecdsa_sign(alt_ec_curve c, const uint8_t *priv, const void *hash, size_t nhash, uint8_t *sig);
/* 0, -ALT_EVERIFY, or -ALT_EKEY for an invalid public key */
int alt_ecdsa_verify(alt_ec_curve c, const uint8_t *pub, size_t npub, const void *hash, size_t nhash, const uint8_t *sig,
	size_t nsig);

/* the shared secret (the X coordinate, scalar size bytes): 0 or -ALT_EKEY */
int alt_ecdh(alt_ec_curve c, const uint8_t *priv, const uint8_t *peer, size_t npeer, uint8_t *secret);

#ifdef __cplusplus
}
#endif

#endif
