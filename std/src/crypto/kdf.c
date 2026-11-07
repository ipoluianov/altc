/* HKDF (RFC 5869) and PBKDF2 (RFC 8018) */
#include <alt/crypto/kdf.h>
#include <string.h>

void alt_hkdf_extract(alt_hash_kind kind, const void *salt, size_t nsalt, const void *ikm, size_t nikm, uint8_t *prk) {
	uint8_t zero[ALT_HASH_MAX_SIZE] = {0};
	if (!nsalt) {
		salt = zero;
		nsalt = alt_hash_size(kind);
	}
	alt_hmac_data(kind, salt, nsalt, ikm, nikm, prk);
}

int alt_hkdf_expand(alt_hash_kind kind, const void *prk, size_t nprk, const void *info, size_t ninfo, void *out, size_t n) {
	size_t hl = alt_hash_size(kind);
	if (n > 255 * hl)
		return -EINVAL;
	uint8_t t[ALT_HASH_MAX_SIZE];
	size_t tl = 0, done = 0;
	for (uint8_t i = 1; done < n; i++) {
		alt_hmac m;
		alt_hmac_init(&m, kind, prk, nprk);
		alt_hmac_write(&m, t, tl);
		alt_hmac_write(&m, info, ninfo);
		alt_hmac_write(&m, &i, 1);
		alt_hmac_sum(&m, t);
		tl = hl;
		size_t k = n - done < hl ? n - done : hl;
		memcpy((uint8_t *)out + done, t, k);
		done += k;
	}
	return 0;
}

int alt_hkdf(alt_hash_kind kind, const void *salt, size_t nsalt, const void *ikm, size_t nikm, const void *info, size_t ninfo,
	void *out, size_t n) {
	uint8_t prk[ALT_HASH_MAX_SIZE];
	alt_hkdf_extract(kind, salt, nsalt, ikm, nikm, prk);
	return alt_hkdf_expand(kind, prk, alt_hash_size(kind), info, ninfo, out, n);
}

int alt_pbkdf2(alt_hash_kind kind, const void *password, size_t npass, const void *salt, size_t nsalt, unsigned iter, void *out,
	size_t n) {
	if (iter == 0)
		return -EINVAL;
	size_t hl = alt_hash_size(kind), done = 0;
	alt_hmac base;
	alt_hmac_init(&base, kind, password, npass);
	for (uint32_t block = 1; done < n; block++) {
		uint8_t be[4] = {(uint8_t)(block >> 24), (uint8_t)(block >> 16), (uint8_t)(block >> 8), (uint8_t)block};
		alt_hmac m = base;
		alt_hmac_write(&m, salt, nsalt);
		alt_hmac_write(&m, be, 4);
		uint8_t u[ALT_HASH_MAX_SIZE], t[ALT_HASH_MAX_SIZE];
		alt_hmac_sum(&m, u);
		memcpy(t, u, hl);
		for (unsigned i = 1; i < iter; i++) {
			m = base;
			alt_hmac_write(&m, u, hl);
			alt_hmac_sum(&m, u);
			for (size_t k = 0; k < hl; k++)
				t[k] ^= u[k];
		}
		size_t k = n - done < hl ? n - done : hl;
		memcpy((uint8_t *)out + done, t, k);
		done += k;
	}
	return 0;
}
