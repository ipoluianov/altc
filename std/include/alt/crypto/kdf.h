/* AltC standard library: key derivation: HKDF (RFC 5869; Go's
   crypto/hkdf) and PBKDF2 (RFC 8018; crypto/pbkdf2) over the hashes of
   <alt/crypto/hash.h> */
#ifndef ALT_CRYPTO_KDF_H
#define ALT_CRYPTO_KDF_H

#include <alt/crypto/hash.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the pseudorandom key (alt_hash_size bytes) */
void alt_hkdf_extract(alt_hash_kind kind, const void *salt, size_t nsalt, const void *ikm, size_t nikm, uint8_t *prk);
/* n bytes of key: 0, or -EINVAL beyond 255 hash lengths */
int alt_hkdf_expand(alt_hash_kind kind, const void *prk, size_t nprk, const void *info, size_t ninfo, void *out, size_t n);
int alt_hkdf(alt_hash_kind kind, const void *salt, size_t nsalt, const void *ikm, size_t nikm, const void *info, size_t ninfo,
	void *out, size_t n);
/* PBKDF2 with HMAC: n bytes of key */
int alt_pbkdf2(alt_hash_kind kind, const void *password, size_t npass, const void *salt, size_t nsalt, unsigned iter, void *out,
	size_t n);

#ifdef __cplusplus
}
#endif

#endif
