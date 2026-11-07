/* AltC standard library: RSA signatures (RFC 8017: PKCS #1 v1.5 and
   PSS; Go's crypto/rsa). Keys are big-endian byte strings; the private
   operation uses the CRT parameters, in constant time, and is checked
   against the public key (as Go does). Keys below 1024 bits are
   refused, as Go does. */
#ifndef ALT_CRYPTO_RSA_H
#define ALT_CRYPTO_RSA_H

#include <alt/crypto/hash.h>
#include <alt/errors.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_RSA_MAX_BYTES = 512 }; /* 4096-bit moduli */

typedef struct {
	const uint8_t *n;
	size_t nlen;           /* without leading zeros */
	int64_t e;
} alt_rsa_public;

typedef struct {
	alt_rsa_public pub;
	const uint8_t *d, *p, *q, *dp, *dq, *qinv;
	size_t dlen, plen, qlen, dplen, dqlen, qinvlen;
} alt_rsa_private;

/* the signature size (the modulus in bytes) */
size_t alt_rsa_size(const alt_rsa_public *k);
/* 0, -ALT_EVERIFY, -ALT_EKEY (Go's key checks: odd modulus of 1024 bits
   or more, odd exponent from 3 to 2^31 - 1) */
int alt_rsa_verify_pkcs1v15(const alt_rsa_public *k, alt_hash_kind h, const void *hashed, size_t nh, const uint8_t *sig,
	size_t nsig);
/* salt_len -1: any (autodetect), else the exact length */
int alt_rsa_verify_pss(const alt_rsa_public *k, alt_hash_kind h, const void *hashed, size_t nh, const uint8_t *sig, size_t nsig,
	int salt_len);
/* sig gets alt_rsa_size bytes */
int alt_rsa_sign_pkcs1v15(const alt_rsa_private *k, alt_hash_kind h, const void *hashed, size_t nh, uint8_t *sig);
/* salt_len -1: the hash length (as TLS uses) */
int alt_rsa_sign_pss(const alt_rsa_private *k, alt_hash_kind h, const void *hashed, size_t nh, uint8_t *sig, int salt_len);

#ifdef __cplusplus
}
#endif

#endif
