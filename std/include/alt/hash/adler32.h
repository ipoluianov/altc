/* AltC standard library: Adler-32 (Go's hash/adler32, RFC 1950).
   A checksum starts at 1: a = alt_adler32_update(1, p, n). */
#ifndef ALT_HASH_ADLER32_H
#define ALT_HASH_ADLER32_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t alt_adler32_update(uint32_t adler, const void *p, size_t n);
static inline uint32_t alt_adler32_checksum(const void *p, size_t n) { return alt_adler32_update(1, p, n); }

#ifdef __cplusplus
}
#endif

#endif
