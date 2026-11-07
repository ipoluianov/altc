/* AltC standard library: RSA (RFC 8017; Go's crypto/rsa): signatures
   (PKCS #1 v1.5, PSS), encryption (OAEP, PKCS #1 v1.5), key generation. Keys are big-endian byte strings; the private
   operation uses the CRT parameters, in constant time, and is checked
   against the public key (as Go does). Keys below 1024 bits are
   refused, as Go does. */
#ifndef ALT_CRYPTO_RSA_H
#define ALT_CRYPTO_RSA_H

#include <alt/crypto/hash.h>
#include <alt/errors.h>
#include <alt/str.h>
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

/* encryption: ct gets alt_rsa_size bytes; -EMSGSIZE for messages too long
   (Go's ErrMessageTooLong) */
int alt_rsa_encrypt_oaep(const alt_rsa_public *k, alt_hash_kind h, const void *msg, size_t n, const void *label, size_t nlabel,
	uint8_t *ct);
int alt_rsa_encrypt_pkcs1v15(const alt_rsa_public *k, const void *msg, size_t n, uint8_t *ct);
/* decryption in constant time: out gets up to alt_rsa_size bytes; every
   failure is -ALT_EAUTH (Go's ErrDecryption) */
int alt_rsa_decrypt_oaep(const alt_rsa_private *k, alt_hash_kind h, const uint8_t *ct, size_t nct, const void *label, size_t nlabel,
	uint8_t *out, size_t *nout);
int alt_rsa_decrypt_pkcs1v15(const alt_rsa_private *k, const uint8_t *ct, size_t nct, uint8_t *out, size_t *nout);

/* a generated key: the parts live in the struct, k points to them */
typedef struct {
	alt_rsa_private k;
	uint8_t n[ALT_RSA_MAX_BYTES], d[ALT_RSA_MAX_BYTES], p[ALT_RSA_MAX_BYTES / 2], q[ALT_RSA_MAX_BYTES / 2],
		dp[ALT_RSA_MAX_BYTES / 2], dq[ALT_RSA_MAX_BYTES / 2], qinv[ALT_RSA_MAX_BYTES / 2];
} alt_rsa_key;

/* GenerateKey: e = 65537, bits from 1024 to 4096 (FIPS 186-5's checks) */
int alt_rsa_generate(alt_rsa_key *out, int bits);
/* the PKCS #1 RSAPrivateKey and the PKCS #8 PrivateKeyInfo (DER) appended to b
   (x509.MarshalPKCS1PrivateKey, MarshalPKCS8PrivateKey) */
int alt_rsa_marshal_pkcs1(const alt_rsa_private *k, alt_buf *b);
int alt_rsa_marshal_pkcs8(const alt_rsa_private *k, alt_buf *b);
/* the PKCS #1 RSAPublicKey */
int alt_rsa_marshal_public_pkcs1(const alt_rsa_public *k, alt_buf *b);

#ifdef __cplusplus
}
#endif

#endif
