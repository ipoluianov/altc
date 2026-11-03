/* AltC private header: byte swapping helpers.  Same include guards as
   glibc's <bits/byteswap.h> and <bits/uintn-identity.h>; implemented in
   plain C (no compiler builtins).  */

#include <_altc/types.h>

#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

#define __bswap_constant_16(x) \
  ((__uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

#define __bswap_constant_32(x) \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8) \
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))

#define __bswap_constant_64(x) \
  ((((x) & 0xff00000000000000ul) >> 56) \
   | (((x) & 0x00ff000000000000ul) >> 40) \
   | (((x) & 0x0000ff0000000000ul) >> 24) \
   | (((x) & 0x000000ff00000000ul) >> 8) \
   | (((x) & 0x00000000ff000000ul) << 8) \
   | (((x) & 0x0000000000ff0000ul) << 24) \
   | (((x) & 0x000000000000ff00ul) << 40) \
   | (((x) & 0x00000000000000fful) << 56))

static __inline __attribute__ ((__unused__)) __uint16_t
__bswap_16 (__uint16_t __bsx)
{
  return __bswap_constant_16 (__bsx);
}

static __inline __attribute__ ((__unused__)) __uint32_t
__bswap_32 (__uint32_t __bsx)
{
  return __bswap_constant_32 (__bsx);
}

static __inline __attribute__ ((__unused__)) __uint64_t
__bswap_64 (__uint64_t __bsx)
{
  return __bswap_constant_64 (__bsx);
}

#endif /* _BITS_BYTESWAP_H */

#ifndef _BITS_UINTN_IDENTITY_H
#define _BITS_UINTN_IDENTITY_H 1

static __inline __attribute__ ((__unused__)) __uint16_t
__uint16_identity (__uint16_t __x)
{
  return __x;
}

static __inline __attribute__ ((__unused__)) __uint32_t
__uint32_identity (__uint32_t __x)
{
  return __x;
}

static __inline __attribute__ ((__unused__)) __uint64_t
__uint64_identity (__uint64_t __x)
{
  return __x;
}

#endif /* _BITS_UINTN_IDENTITY_H */
