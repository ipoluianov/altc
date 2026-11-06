/* AltC <stddef.h>: the C library's, with the C23 additions that musl
   1.2.5 lacks (compilers provide them, as gcc's <stddef.h> does) */
#include_next <stddef.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 201710L
#ifndef __ALTC_STDDEF_C23
#define __ALTC_STDDEF_C23
#ifndef unreachable
#define unreachable() (__builtin_unreachable())
#endif
typedef typeof(nullptr) nullptr_t;
#define __STDC_VERSION_STDDEF_H__ 202311L
#endif
#endif
