/* AltC <stdlib.h> - general utilities (glibc-compatible, x86-64).  */

#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <features.h>

#define __need_size_t
#define __need_wchar_t
#define __need_NULL
#include <stddef.h>

__BEGIN_DECLS

#include <_altc/wait.h>

typedef struct
  {
    int quot;
    int rem;
  } div_t;

#ifndef __ldiv_t_defined
typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;
# define __ldiv_t_defined 1
#endif

#ifndef __lldiv_t_defined
typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
# define __lldiv_t_defined 1
#endif

#define RAND_MAX 2147483647

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define MB_CUR_MAX (__ctype_get_mb_cur_max ())
extern size_t __ctype_get_mb_cur_max (void);

extern double atof (const char *__nptr);
extern int atoi (const char *__nptr);
extern long int atol (const char *__nptr);
extern long long int atoll (const char *__nptr);

extern double strtod (const char *__restrict __nptr,
                      char **__restrict __endptr);
extern float strtof (const char *__restrict __nptr,
                     char **__restrict __endptr);
extern long double strtold (const char *__restrict __nptr,
                            char **__restrict __endptr);

extern long int strtol (const char *__restrict __nptr,
                        char **__restrict __endptr, int __base);
extern unsigned long int strtoul (const char *__restrict __nptr,
                                  char **__restrict __endptr, int __base);
extern long long int strtoq (const char *__restrict __nptr,
                             char **__restrict __endptr, int __base);
extern unsigned long long int strtouq (const char *__restrict __nptr,
                                       char **__restrict __endptr,
                                       int __base);
extern long long int strtoll (const char *__restrict __nptr,
                              char **__restrict __endptr, int __base);
extern unsigned long long int strtoull (const char *__restrict __nptr,
                                        char **__restrict __endptr,
                                        int __base);

#if __GLIBC_USE (C23_STRTOL)
extern long int __isoc23_strtol (const char *__restrict __nptr,
                                 char **__restrict __endptr, int __base);
extern unsigned long int __isoc23_strtoul (const char *__restrict __nptr,
                                           char **__restrict __endptr,
                                           int __base);
extern long long int __isoc23_strtoll (const char *__restrict __nptr,
                                       char **__restrict __endptr,
                                       int __base);
extern unsigned long long int __isoc23_strtoull (const char *__restrict __nptr,
                                                 char **__restrict __endptr,
                                                 int __base);
# define strtol __isoc23_strtol
# define strtoul __isoc23_strtoul
# define strtoll __isoc23_strtoll
# define strtoull __isoc23_strtoull
#endif

#if defined __USE_GNU \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)
extern int strfromd (char *__dest, size_t __size, const char *__format,
                     double __f);
extern int strfromf (char *__dest, size_t __size, const char *__format,
                     float __f);
extern int strfroml (char *__dest, size_t __size, const char *__format,
                     long double __f);
#endif

#ifdef __USE_GNU
# include <_altc/locale_t.h>
extern long int strtol_l (const char *__restrict __nptr,
                          char **__restrict __endptr, int __base,
                          locale_t __loc);
extern unsigned long int strtoul_l (const char *__restrict __nptr,
                                    char **__restrict __endptr,
                                    int __base, locale_t __loc);
extern long long int strtoll_l (const char *__restrict __nptr,
                                char **__restrict __endptr, int __base,
                                locale_t __loc);
extern unsigned long long int strtoull_l (const char *__restrict __nptr,
                                          char **__restrict __endptr,
                                          int __base, locale_t __loc);
extern double strtod_l (const char *__restrict __nptr,
                        char **__restrict __endptr, locale_t __loc);
extern float strtof_l (const char *__restrict __nptr,
                       char **__restrict __endptr, locale_t __loc);
extern long double strtold_l (const char *__restrict __nptr,
                              char **__restrict __endptr, locale_t __loc);
# if __GLIBC_USE (C23_STRTOL)
extern long int __isoc23_strtol_l (const char *__restrict __nptr,
                                   char **__restrict __endptr, int __base,
                                   locale_t __loc);
extern unsigned long int __isoc23_strtoul_l (const char *__restrict __nptr,
                                             char **__restrict __endptr,
                                             int __base, locale_t __loc);
extern long long int __isoc23_strtoll_l (const char *__restrict __nptr,
                                         char **__restrict __endptr,
                                         int __base, locale_t __loc);
extern unsigned long long int __isoc23_strtoull_l (const char *__restrict __nptr,
                                                   char **__restrict __endptr,
                                                   int __base, locale_t __loc);
#  define strtol_l __isoc23_strtol_l
#  define strtoul_l __isoc23_strtoul_l
#  define strtoll_l __isoc23_strtoll_l
#  define strtoull_l __isoc23_strtoull_l
# endif
#endif

extern char *l64a (long int __n);
extern long int a64l (const char *__s);

#include <sys/types.h>

extern long int random (void);
extern void srandom (unsigned int __seed);
extern char *initstate (unsigned int __seed, char *__statebuf,
                        size_t __statelen);
extern char *setstate (char *__statebuf);

struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
                     int32_t *__restrict __result);
extern int srandom_r (unsigned int __seed, struct random_data *__buf);
extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
                        size_t __statelen,
                        struct random_data *__restrict __buf);
extern int setstate_r (char *__restrict __statebuf,
                       struct random_data *__restrict __buf);

extern int rand (void);
extern void srand (unsigned int __seed);
extern int rand_r (unsigned int *__seed);

extern double drand48 (void);
extern double erand48 (unsigned short int __xsubi[3]);
extern long int lrand48 (void);
extern long int nrand48 (unsigned short int __xsubi[3]);
extern long int mrand48 (void);
extern long int jrand48 (unsigned short int __xsubi[3]);
extern void srand48 (long int __seedval);
extern unsigned short int *seed48 (unsigned short int __seed16v[3]);
extern void lcong48 (unsigned short int __param[7]);

struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };

extern int drand48_r (struct drand48_data *__restrict __buffer,
                      double *__restrict __result);
extern int erand48_r (unsigned short int __xsubi[3],
                      struct drand48_data *__restrict __buffer,
                      double *__restrict __result);
extern int lrand48_r (struct drand48_data *__restrict __buffer,
                      long int *__restrict __result);
extern int nrand48_r (unsigned short int __xsubi[3],
                      struct drand48_data *__restrict __buffer,
                      long int *__restrict __result);
extern int mrand48_r (struct drand48_data *__restrict __buffer,
                      long int *__restrict __result);
extern int jrand48_r (unsigned short int __xsubi[3],
                      struct drand48_data *__restrict __buffer,
                      long int *__restrict __result);
extern int srand48_r (long int __seedval, struct drand48_data *__buffer);
extern int seed48_r (unsigned short int __seed16v[3],
                     struct drand48_data *__buffer);
extern int lcong48_r (unsigned short int __param[7],
                      struct drand48_data *__buffer);

extern __uint32_t arc4random (void);
extern void arc4random_buf (void *__buf, size_t __size);
extern __uint32_t arc4random_uniform (__uint32_t __upper_bound);

extern void *malloc (size_t __size);
extern void *calloc (size_t __nmemb, size_t __size);
extern void *realloc (void *__ptr, size_t __size);
extern void free (void *__ptr);
extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size);

#include <alloca.h>

extern void *valloc (size_t __size);
extern int posix_memalign (void **__memptr, size_t __alignment,
                           size_t __size);
extern void *aligned_alloc (size_t __alignment, size_t __size);

extern void abort (void) __attribute__ ((__noreturn__));
extern int atexit (void (*__func) (void));
extern int at_quick_exit (void (*__func) (void));
extern int on_exit (void (*__func) (int __status, void *__arg),
                    void *__arg);
extern void exit (int __status) __attribute__ ((__noreturn__));
extern void quick_exit (int __status) __attribute__ ((__noreturn__));
extern void _Exit (int __status) __attribute__ ((__noreturn__));

extern char *getenv (const char *__name);
#ifdef __USE_GNU
extern char *secure_getenv (const char *__name);
#endif
extern int putenv (char *__string);
extern int setenv (const char *__name, const char *__value, int __replace);
extern int unsetenv (const char *__name);
extern int clearenv (void);

extern char *mktemp (char *__template);
extern int mkstemp (char *__template);
extern int mkstemps (char *__template, int __suffixlen);
extern char *mkdtemp (char *__template);
#ifdef __USE_GNU
extern int mkostemp (char *__template, int __flags);
extern int mkostemps (char *__template, int __suffixlen, int __flags);
#endif
#ifdef __USE_LARGEFILE64
extern int mkstemp64 (char *__template);
extern int mkstemps64 (char *__template, int __suffixlen);
# ifdef __USE_GNU
extern int mkostemp64 (char *__template, int __flags);
extern int mkostemps64 (char *__template, int __suffixlen, int __flags);
# endif
#endif

extern int system (const char *__command);

#ifdef __USE_GNU
extern char *canonicalize_file_name (const char *__name);
#endif
extern char *realpath (const char *__restrict __name,
                       char *__restrict __resolved);

#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) (const void *, const void *);
# ifdef __USE_GNU
typedef __compar_fn_t comparison_fn_t;
# endif
#endif
#ifdef __USE_GNU
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);
#endif

extern void *bsearch (const void *__key, const void *__base,
                      size_t __nmemb, size_t __size, __compar_fn_t __compar);
extern void qsort (void *__base, size_t __nmemb, size_t __size,
                   __compar_fn_t __compar);
#ifdef __USE_GNU
extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
                     __compar_d_fn_t __compar, void *__arg);
#endif

extern int abs (int __x) __attribute__ ((__const__));
extern long int labs (long int __x) __attribute__ ((__const__));
extern long long int llabs (long long int __x) __attribute__ ((__const__));

extern div_t div (int __numer, int __denom) __attribute__ ((__const__));
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__const__));
extern lldiv_t lldiv (long long int __numer, long long int __denom)
     __attribute__ ((__const__));

extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign);
extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign);
extern char *gcvt (double __value, int __ndigit, char *__buf);
extern char *qecvt (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign);
extern char *qfcvt (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign);
extern char *qgcvt (long double __value, int __ndigit, char *__buf);
extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign, char *__restrict __buf,
                   size_t __len);
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign, char *__restrict __buf,
                   size_t __len);
extern int qecvt_r (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign,
                    char *__restrict __buf, size_t __len);
extern int qfcvt_r (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign,
                    char *__restrict __buf, size_t __len);

extern int mblen (const char *__s, size_t __n);
extern int mbtowc (wchar_t *__restrict __pwc,
                   const char *__restrict __s, size_t __n);
extern int wctomb (char *__s, wchar_t __wchar);
extern size_t mbstowcs (wchar_t *__restrict __pwcs,
                        const char *__restrict __s, size_t __n);
extern size_t wcstombs (char *__restrict __s,
                        const wchar_t *__restrict __pwcs, size_t __n);

extern int rpmatch (const char *__response);
extern int getsubopt (char **__restrict __optionp,
                      char *const *__restrict __tokens,
                      char **__restrict __valuep);

extern int posix_openpt (int __oflag);
extern int grantpt (int __fd);
extern int unlockpt (int __fd);
extern char *ptsname (int __fd);
#ifdef __USE_GNU
extern int ptsname_r (int __fd, char *__buf, size_t __buflen);
extern int getpt (void);
#endif

extern int getloadavg (double __loadavg[], int __nelem);

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_STDLIB_H__ 202311L
#endif

__END_DECLS

#endif /* stdlib.h */
