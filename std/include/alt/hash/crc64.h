/* AltC standard library: CRC-64 (Go's hash/crc64): ISO and ECMA
   polynomials in reversed form */
#ifndef ALT_HASH_CRC64_H
#define ALT_HASH_CRC64_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALT_CRC64_ISO 0xD800000000000000ull
#define ALT_CRC64_ECMA 0xC96C5795D7870F42ull

typedef struct alt_crc64_table alt_crc64_table;

const alt_crc64_table *alt_crc64_make_table(uint64_t poly);
uint64_t alt_crc64_update(uint64_t crc, const alt_crc64_table *tab, const void *p, size_t n);
uint64_t alt_crc64_checksum(const void *p, size_t n, const alt_crc64_table *tab);

#ifdef __cplusplus
}
#endif

#endif
