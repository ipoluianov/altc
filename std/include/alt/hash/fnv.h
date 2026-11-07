/* AltC standard library: FNV-1 and FNV-1a hashes (Go's hash/fnv), 32
   and 64 bits. Start from the offset basis and update:
   h = alt_fnv1a_64(ALT_FNV64_OFFSET, p, n). */
#ifndef ALT_HASH_FNV_H
#define ALT_HASH_FNV_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALT_FNV32_OFFSET 2166136261u
#define ALT_FNV64_OFFSET 14695981039346656037ull
#define ALT_FNV32_PRIME 16777619u
#define ALT_FNV64_PRIME 1099511628211ull

static inline uint32_t alt_fnv1_32(uint32_t h, const void *p, size_t n) {
	const unsigned char *s = (const unsigned char *)p;
	for (size_t i = 0; i < n; i++)
		h = (h * ALT_FNV32_PRIME) ^ s[i];
	return h;
}
static inline uint32_t alt_fnv1a_32(uint32_t h, const void *p, size_t n) {
	const unsigned char *s = (const unsigned char *)p;
	for (size_t i = 0; i < n; i++)
		h = (h ^ s[i]) * ALT_FNV32_PRIME;
	return h;
}
static inline uint64_t alt_fnv1_64(uint64_t h, const void *p, size_t n) {
	const unsigned char *s = (const unsigned char *)p;
	for (size_t i = 0; i < n; i++)
		h = (h * ALT_FNV64_PRIME) ^ s[i];
	return h;
}
static inline uint64_t alt_fnv1a_64(uint64_t h, const void *p, size_t n) {
	const unsigned char *s = (const unsigned char *)p;
	for (size_t i = 0; i < n; i++)
		h = (h ^ s[i]) * ALT_FNV64_PRIME;
	return h;
}

#ifdef __cplusplus
}
#endif

#endif
