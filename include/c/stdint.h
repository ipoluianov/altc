/* AltC <stdint.h> - integer types (glibc-compatible, x86-64 LP64).  */

#ifndef _STDINT_H
#define _STDINT_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/wchar.h>

#define __altc_need_uintn 1
#include <_altc/stdint-intn.h>

#ifndef _BITS_STDINT_LEAST_H
#define _BITS_STDINT_LEAST_H 1
typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;
typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;
#endif

typedef signed char int_fast8_t;
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned char uint_fast8_t;
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;

#ifndef __intptr_t_defined
typedef long int intptr_t;
# define __intptr_t_defined
#endif
typedef unsigned long int uintptr_t;

typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;

#define __INT64_C(c)	c ## L
#define __UINT64_C(c)	c ## UL

#define INT8_MIN (-128)
#define INT16_MIN (-32767-1)
#define INT32_MIN (-2147483647-1)
#define INT64_MIN (-__INT64_C(9223372036854775807)-1)
#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (__INT64_C(9223372036854775807))
#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (__UINT64_C(18446744073709551615))

#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32767-1)
#define INT_LEAST32_MIN (-2147483647-1)
#define INT_LEAST64_MIN (-__INT64_C(9223372036854775807)-1)
#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (__INT64_C(9223372036854775807))
#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295U)
#define UINT_LEAST64_MAX (__UINT64_C(18446744073709551615))

#define INT_FAST8_MIN (-128)
#define INT_FAST16_MIN (-9223372036854775807L-1)
#define INT_FAST32_MIN (-9223372036854775807L-1)
#define INT_FAST64_MIN (-__INT64_C(9223372036854775807)-1)
#define INT_FAST8_MAX (127)
#define INT_FAST16_MAX (9223372036854775807L)
#define INT_FAST32_MAX (9223372036854775807L)
#define INT_FAST64_MAX (__INT64_C(9223372036854775807))
#define UINT_FAST8_MAX (255)
#define UINT_FAST16_MAX (18446744073709551615UL)
#define UINT_FAST32_MAX (18446744073709551615UL)
#define UINT_FAST64_MAX (__UINT64_C(18446744073709551615))

#define INTPTR_MIN (-9223372036854775807L-1)
#define INTPTR_MAX (9223372036854775807L)
#define UINTPTR_MAX (18446744073709551615UL)

#define INTMAX_MIN (-__INT64_C(9223372036854775807)-1)
#define INTMAX_MAX (__INT64_C(9223372036854775807))
#define UINTMAX_MAX (__UINT64_C(18446744073709551615))

#define PTRDIFF_MIN (-9223372036854775807L-1)
#define PTRDIFF_MAX (9223372036854775807L)
#define SIG_ATOMIC_MIN (-2147483647-1)
#define SIG_ATOMIC_MAX (2147483647)
#define SIZE_MAX (18446744073709551615UL)
#define WCHAR_MIN __WCHAR_MIN
#define WCHAR_MAX __WCHAR_MAX
#define WINT_MIN (0u)
#define WINT_MAX (4294967295u)

#define INT8_C(c) c
#define INT16_C(c) c
#define INT32_C(c) c
#define INT64_C(c) c ## L
#define UINT8_C(c) c
#define UINT16_C(c) c
#define UINT32_C(c) c ## U
#define UINT64_C(c) c ## UL
#define INTMAX_C(c) c ## L
#define UINTMAX_C(c) c ## UL

#if __GLIBC_USE (ISOC23)
# define INT8_WIDTH 8
# define UINT8_WIDTH 8
# define INT16_WIDTH 16
# define UINT16_WIDTH 16
# define INT32_WIDTH 32
# define UINT32_WIDTH 32
# define INT64_WIDTH 64
# define UINT64_WIDTH 64
# define INT_LEAST8_WIDTH 8
# define UINT_LEAST8_WIDTH 8
# define INT_LEAST16_WIDTH 16
# define UINT_LEAST16_WIDTH 16
# define INT_LEAST32_WIDTH 32
# define UINT_LEAST32_WIDTH 32
# define INT_LEAST64_WIDTH 64
# define UINT_LEAST64_WIDTH 64
# define INT_FAST8_WIDTH 8
# define UINT_FAST8_WIDTH 8
# define INT_FAST16_WIDTH 64
# define UINT_FAST16_WIDTH 64
# define INT_FAST32_WIDTH 64
# define UINT_FAST32_WIDTH 64
# define INT_FAST64_WIDTH 64
# define UINT_FAST64_WIDTH 64
# define INTPTR_WIDTH 64
# define UINTPTR_WIDTH 64
# define INTMAX_WIDTH 64
# define UINTMAX_WIDTH 64
# define PTRDIFF_WIDTH 64
# define SIG_ATOMIC_WIDTH 32
# define SIZE_WIDTH 64
# define WCHAR_WIDTH 32
# define WINT_WIDTH 32
#endif
#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_STDINT_H__ 202311L
#endif

#endif /* stdint.h */
