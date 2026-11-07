/* AltC standard library: CRC-32 (Go's hash/crc32).

   The polynomials are in reversed form, as in Go: IEEE (gzip, zip, PNG,
   Ethernet), Castagnoli (iSCSI, ext4, SCTP), Koopman. alt_crc32_update
   continues a checksum: crc = alt_crc32_update(crc, tab, p, n) from 0. */
#ifndef ALT_HASH_CRC32_H
#define ALT_HASH_CRC32_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_CRC32_SIZE = 4 };
#define ALT_CRC32_IEEE 0xedb88320u
#define ALT_CRC32_CASTAGNOLI 0x82f63b78u
#define ALT_CRC32_KOOPMAN 0xeb31d82eu

typedef struct alt_crc32_table alt_crc32_table;

/* the table of a polynomial (made once, never freed); NULL without memory */
const alt_crc32_table *alt_crc32_make_table(uint32_t poly);
uint32_t alt_crc32_update(uint32_t crc, const alt_crc32_table *tab, const void *p, size_t n);
uint32_t alt_crc32_checksum(const void *p, size_t n, const alt_crc32_table *tab);
/* with the IEEE polynomial */
uint32_t alt_crc32_ieee(const void *p, size_t n);
uint32_t alt_crc32_ieee_update(uint32_t crc, const void *p, size_t n);

#ifdef __cplusplus
}
#endif

#endif
