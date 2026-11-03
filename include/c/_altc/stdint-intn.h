/* AltC private header: exact-width integer types.
   Same include guards as glibc's <bits/stdint-intn.h> and
   <bits/stdint-uintn.h>.  */

#include <_altc/types.h>

#ifndef _BITS_STDINT_INTN_H
#define _BITS_STDINT_INTN_H 1
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
#endif

#if defined __altc_need_uintn && !defined _BITS_STDINT_UINTN_H
#define _BITS_STDINT_UINTN_H 1
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
#endif
#undef __altc_need_uintn
