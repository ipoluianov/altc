/* AltC <endian.h> - byte order (x86-64 is little-endian).  */

#ifndef _ENDIAN_H
#define _ENDIAN_H 1

#include <features.h>

/* <bits/endian.h> and <bits/endianness.h> */
#ifndef _BITS_ENDIAN_H
#define _BITS_ENDIAN_H 1
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __PDP_ENDIAN 3412
#define __FLOAT_WORD_ORDER __BYTE_ORDER
#define __LONG_LONG_PAIR(HI, LO) LO, HI
#endif
#ifndef _BITS_ENDIANNESS_H
#define _BITS_ENDIANNESS_H 1
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#define PDP_ENDIAN __PDP_ENDIAN
#define BYTE_ORDER __BYTE_ORDER

#include <_altc/byteswap.h>

#define htobe16(x) __bswap_16 (x)
#define htole16(x) __uint16_identity (x)
#define be16toh(x) __bswap_16 (x)
#define le16toh(x) __uint16_identity (x)

#define htobe32(x) __bswap_32 (x)
#define htole32(x) __uint32_identity (x)
#define be32toh(x) __bswap_32 (x)
#define le32toh(x) __uint32_identity (x)

#define htobe64(x) __bswap_64 (x)
#define htole64(x) __uint64_identity (x)
#define be64toh(x) __bswap_64 (x)
#define le64toh(x) __uint64_identity (x)

#endif /* endian.h */
