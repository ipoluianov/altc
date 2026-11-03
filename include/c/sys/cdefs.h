/* AltC <sys/cdefs.h> - helper macros used by the C library headers.

   This file replaces glibc's <sys/cdefs.h>.  It defines every macro that
   glibc 2.41's headers use, in the form appropriate for a compiler that is
   not GCC: no asm-label redirection (__REDIRECT is left undefined, so glibc
   headers fall back to plain names or #define aliases), no extern inline
   functions and no _FORTIFY_SOURCE.  __attribute__ is kept (AltC parses it).
   It also provides the definitions of <bits/wordsize.h> and
   <bits/long-double.h> with token-identical replacement lists, so those
   glibc files may be included later without conflicts.  */

#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H 1

#ifndef _FEATURES_H
# include <features.h>
#endif

/* <bits/wordsize.h> */
#define __WORDSIZE	64
#define __WORDSIZE_TIME64_COMPAT32 1
#define __SYSCALL_WORDSIZE		64

/* <bits/long-double.h>: long double is the x87 80-bit type.  */
#define __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI 0

#define __glibc_has_attribute(attr) 0
#define __glibc_has_builtin(name) 0
#define __glibc_has_extension(ext) 0

#undef __P
#undef __PMT
#define __P(args) args
#define __PMT(args) args

#define __LEAF
#define __LEAF_ATTR
#define __THROW
#define __THROWNL
#define __NTH(fct) fct
#define __NTHNL(fct) fct
#define __COLD

#define __CONCAT(x, y) x ## y
#define __STRING(x) #x
#define __ptr_t void *

#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

/* Object size checking is not supported (fortify level is always 0).  */
#define __bos(ptr) ((unsigned long) -1)
#define __bos0(ptr) ((unsigned long) -1)
#define __glibc_objsize0(__o) ((unsigned long) -1)
#define __glibc_objsize(__o) ((unsigned long) -1)

#define __errordecl(name, msg) extern void name (void)
#define __warndecl(name, msg) extern void name (void)
#define __warnattr(msg)

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define __flexarr []
# define __glibc_c99_flexarr_available 1
#else
# define __flexarr [1]
# define __glibc_c99_flexarr_available 0
#endif

/* No asm-label redirection: __REDIRECT, __REDIRECT_NTH, __REDIRECT_NTHNL
   and __ASMNAME are deliberately left undefined.  */

#define __attribute_malloc__
#define __attribute_alloc_size__(params)
#define __attribute_alloc_align__(param)
#define __attribute_pure__
#define __attribute_const__
#define __attribute_maybe_unused__
#define __attribute_used__
#define __attribute_noinline__
#define __attribute_deprecated__
#define __attribute_deprecated_msg__(msg)
#define __attribute_format_arg__(x)
#define __attribute_format_strfmon__(a, b)
#define __attribute_nonnull__(params)
#define __nonnull(params)
#define __returns_nonnull
#define __attribute_warn_unused_result__
#define __wur
#define __attribute_artificial__
#define __attribute_returns_twice__
#define __attribute_nonstring__
#define __attribute_copy__(arg)
#define __attribute_struct_may_alias__
#define __attribute_overloadable__
#define __attr_access(x)
#define __fortified_attr_access(a, o, s)
#define __attr_access_none(argno)
#define __attr_dealloc(dealloc, argno)
#define __attr_dealloc_free

#undef __always_inline
#define __always_inline __inline

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define __inline inline
#else
# define __inline
#endif

#define __extension__

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define __restrict restrict
# define __restrict_arr restrict
#else
# define __restrict
# define __restrict_arr
#endif

#define __glibc_unlikely(cond) (cond)
#define __glibc_likely(cond) (cond)

#if !defined __STDC_VERSION__ || __STDC_VERSION__ < 201112L
# define _Noreturn __attribute__ ((__noreturn__))
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
# define __HAVE_GENERIC_SELECTION 1
#else
# define __HAVE_GENERIC_SELECTION 0
#endif

#define __glibc_macro_warning(message)

/* long double is not redirected to another ABI.  */
#define __LDBL_REDIR1(name, proto, alias) name proto
#define __LDBL_REDIR(name, proto) name proto
#define __LDBL_REDIR1_NTH(name, proto, alias) name proto __THROW
#define __LDBL_REDIR_NTH(name, proto) name proto __THROW
#define __LDBL_REDIR2_DECL(name)
#define __LDBL_REDIR_DECL(name)

#endif /* sys/cdefs.h */
