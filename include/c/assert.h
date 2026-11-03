/* AltC <assert.h> - diagnostics.  Like glibc's, this header may be included
   several times with different settings of NDEBUG.  */

#ifdef _ASSERT_H
# undef _ASSERT_H
# undef assert
# undef __ASSERT_VOID_CAST
# undef assert_perror
#endif

#define _ASSERT_H 1
#include <features.h>

#define __ASSERT_VOID_CAST (void)

#ifdef NDEBUG

# define assert(expr) (__ASSERT_VOID_CAST (0))
# ifdef __USE_GNU
#  define assert_perror(errnum) (__ASSERT_VOID_CAST (0))
# endif

#else

#ifndef __altc_assert_decls
#define __altc_assert_decls 1
__BEGIN_DECLS
extern void __assert_fail (const char *__assertion, const char *__file,
                           unsigned int __line, const char *__function)
     __attribute__ ((__noreturn__));
extern void __assert_perror_fail (int __errnum, const char *__file,
                                  unsigned int __line,
                                  const char *__function)
     __attribute__ ((__noreturn__));
extern void __assert (const char *__assertion, const char *__file, int __line)
     __attribute__ ((__noreturn__));
__END_DECLS
#endif

# if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#  define __ASSERT_FUNCTION __func__
# else
#  define __ASSERT_FUNCTION ((const char *) 0)
# endif

/* C23 allows assert to take an expression containing unparenthesized
   commas, hence the variadic form there.  */
# if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
#  define assert(...) \
  ((__VA_ARGS__) ? __ASSERT_VOID_CAST (0) \
   : __assert_fail (#__VA_ARGS__, __FILE__, __LINE__, __ASSERT_FUNCTION))
# else
#  define assert(expr) \
  ((expr) ? __ASSERT_VOID_CAST (0) \
   : __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))
# endif

# ifdef __USE_GNU
#  define assert_perror(errnum) \
  (!(errnum) ? __ASSERT_VOID_CAST (0) \
   : __assert_perror_fail ((errnum), __FILE__, __LINE__, __ASSERT_FUNCTION))
# endif

#endif /* NDEBUG */

#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L \
     && __STDC_VERSION__ < 202311L && !defined __cplusplus)
# undef static_assert
# define static_assert _Static_assert
#endif
