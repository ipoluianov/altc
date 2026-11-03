/* AltC <stddef.h> - common definitions (x86-64 LP64).

   Like GCC's <stddef.h>, this header supports partial inclusion: if any of
   __need_size_t, __need_ptrdiff_t, __need_wchar_t, __need_NULL,
   __need_wint_t, __need_max_align_t or __need_offsetof is defined, only the
   requested definitions are provided (glibc headers rely on this).  */

#if !defined __need_size_t && !defined __need_ptrdiff_t \
    && !defined __need_wchar_t && !defined __need_NULL \
    && !defined __need_wint_t && !defined __need_max_align_t \
    && !defined __need_offsetof
# ifndef _STDDEF_H
#  define _STDDEF_H 1
#  define _STDDEF_H_ 1
#  define _ANSI_STDDEF_H 1
#  define __STDDEF_H__ 1
#  define __altc_stddef_all 1
# endif
#endif

#if defined __altc_stddef_all || defined __need_ptrdiff_t
# if !defined _PTRDIFF_T && !defined _GCC_PTRDIFF_T
#  define _PTRDIFF_T 1
#  define _GCC_PTRDIFF_T 1
#  define _PTRDIFF_T_ 1
#  define _BSD_PTRDIFF_T_ 1
#  define ___int_ptrdiff_t_h 1
typedef long int ptrdiff_t;
# endif
#endif
#undef __need_ptrdiff_t

#if defined __altc_stddef_all || defined __need_size_t
# if !defined _SIZE_T && !defined _GCC_SIZE_T
#  define _SIZE_T 1
#  define _GCC_SIZE_T 1
#  define __SIZE_T 1
#  define _SIZE_T_ 1
#  define _SIZE_T_DEFINED 1
#  define _SIZE_T_DEFINED_ 1
#  define _BSD_SIZE_T_ 1
#  define _BSD_SIZE_T_DEFINED_ 1
#  define _SIZE_T_DECLARED 1
#  define ___int_size_t_h 1
#  define _SIZET_ 1
#  define __size_t 1
typedef unsigned long int size_t;
# endif
#endif
#undef __need_size_t

#if defined __altc_stddef_all || defined __need_wchar_t
# if !defined _WCHAR_T && !defined _GCC_WCHAR_T && !defined __cplusplus
#  define _WCHAR_T 1
#  define _GCC_WCHAR_T 1
#  define __WCHAR_T 1
#  define _WCHAR_T_ 1
#  define _WCHAR_T_DEFINED 1
#  define _WCHAR_T_DEFINED_ 1
#  define _BSD_WCHAR_T_ 1
#  define _WCHAR_T_DECLARED 1
#  define ___int_wchar_t_h 1
#  define __INT_WCHAR_T_H 1
typedef int wchar_t;
# endif
#endif
#undef __need_wchar_t

#if defined __need_wint_t
# ifndef _WINT_T
#  define _WINT_T 1
typedef unsigned int wint_t;
# endif
#endif
#undef __need_wint_t

#if defined __altc_stddef_all || defined __need_NULL
# undef NULL
# ifdef __cplusplus
#  define NULL __null
# else
#  define NULL ((void *)0)
# endif
#endif
#undef __need_NULL

#if defined __altc_stddef_all || defined __need_offsetof
# ifndef offsetof
#  define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
# endif
#endif
#undef __need_offsetof

#if (defined __altc_stddef_all \
     && defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L) \
    || defined __need_max_align_t
# ifndef _GCC_MAX_ALIGN_T
#  define _GCC_MAX_ALIGN_T 1
#  define __CLANG_MAX_ALIGN_T_DEFINED 1
/* size 32, alignment 16 - identical to GCC's definition.  */
typedef struct {
  long long __max_align_ll;
  long double __max_align_ld;
} max_align_t;
# endif
#endif
#undef __need_max_align_t

#if defined __altc_stddef_all \
    && defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# ifndef __STDC_VERSION_STDDEF_H__
#  define __STDC_VERSION_STDDEF_H__ 202311L
# endif
#endif

#if defined __altc_stddef_all \
    && defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
/* C23 (GCC 14 reports 202000L for -std=c2x, hence "> 201710L").  */
# ifndef _GCC_NULLPTR_T
#  define _GCC_NULLPTR_T 1
typedef typeof (nullptr) nullptr_t;
# endif
# ifndef unreachable
#  define unreachable() (__builtin_unreachable ())
# endif
#endif

#undef __altc_stddef_all
