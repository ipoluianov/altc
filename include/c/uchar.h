/* AltC <uchar.h> - Unicode utilities (glibc-compatible).  */

#ifndef _UCHAR_H
#define _UCHAR_H 1

#include <features.h>

#define __need_size_t
#include <stddef.h>

#include <_altc/types.h>
#include <_altc/mbstate_t.h>

#if __GLIBC_USE (ISOC23)
typedef unsigned char char8_t;
#endif
typedef __uint_least16_t char16_t;
typedef __uint_least32_t char32_t;

__BEGIN_DECLS

#if __GLIBC_USE (ISOC23)
extern size_t mbrtoc8 (char8_t *__restrict __pc8,
                       const char *__restrict __s, size_t __n,
                       mbstate_t *__restrict __p);
extern size_t c8rtomb (char *__restrict __s, char8_t __c8,
                       mbstate_t *__restrict __ps);
#endif
#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_UCHAR_H__ 202311L
#endif

extern size_t mbrtoc16 (char16_t *__restrict __pc16,
                        const char *__restrict __s, size_t __n,
                        mbstate_t *__restrict __p);
extern size_t c16rtomb (char *__restrict __s, char16_t __c16,
                        mbstate_t *__restrict __ps);
extern size_t mbrtoc32 (char32_t *__restrict __pc32,
                        const char *__restrict __s, size_t __n,
                        mbstate_t *__restrict __p);
extern size_t c32rtomb (char *__restrict __s, char32_t __c32,
                        mbstate_t *__restrict __ps);

__END_DECLS

#endif /* uchar.h */
