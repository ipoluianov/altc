/* AltC <wchar.h> - wide character handling (glibc-compatible).
   wchar_t is a 32-bit int holding ISO 10646 code points.  */

#ifndef _WCHAR_H
#define _WCHAR_H 1

#include <features.h>

#define __need_size_t
#define __need_wchar_t
#define __need_NULL
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>

#include <_altc/types.h>
#include <_altc/wchar.h>
#include <_altc/mbstate_t.h>
#include <_altc/FILE.h>
#include <_altc/locale_t.h>

#ifndef __wint_t_defined
# define __wint_t_defined 1
# ifndef _WINT_T
#  define _WINT_T 1
typedef unsigned int wint_t;
# endif
#endif

struct tm;

__BEGIN_DECLS

#if !defined _VA_LIST_DEFINED && !defined _VA_LIST_ && !defined _VA_LIST \
    && !defined _VA_LIST_T_H && !defined __va_list__
typedef __gnuc_va_list va_list;
# define _VA_LIST_DEFINED
#endif

#ifndef WCHAR_MIN
# define WCHAR_MIN __WCHAR_MIN
# define WCHAR_MAX __WCHAR_MAX
#endif

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif

extern wchar_t *wcscpy (wchar_t *__restrict __dest,
                        const wchar_t *__restrict __src);
extern wchar_t *wcsncpy (wchar_t *__restrict __dest,
                         const wchar_t *__restrict __src, size_t __n);
extern size_t wcslcpy (wchar_t *__restrict __dest,
                       const wchar_t *__restrict __src, size_t __n);
extern size_t wcslcat (wchar_t *__restrict __dest,
                       const wchar_t *__restrict __src, size_t __n);
extern wchar_t *wcscat (wchar_t *__restrict __dest,
                        const wchar_t *__restrict __src);
extern wchar_t *wcsncat (wchar_t *__restrict __dest,
                         const wchar_t *__restrict __src, size_t __n);
extern int wcscmp (const wchar_t *__s1, const wchar_t *__s2);
extern int wcsncmp (const wchar_t *__s1, const wchar_t *__s2, size_t __n);
extern int wcscasecmp (const wchar_t *__s1, const wchar_t *__s2);
extern int wcsncasecmp (const wchar_t *__s1, const wchar_t *__s2,
                        size_t __n);
extern int wcscasecmp_l (const wchar_t *__s1, const wchar_t *__s2,
                         locale_t __loc);
extern int wcsncasecmp_l (const wchar_t *__s1, const wchar_t *__s2,
                          size_t __n, locale_t __loc);
extern int wcscoll (const wchar_t *__s1, const wchar_t *__s2);
extern size_t wcsxfrm (wchar_t *__restrict __s1,
                       const wchar_t *__restrict __s2, size_t __n);
extern int wcscoll_l (const wchar_t *__s1, const wchar_t *__s2,
                      locale_t __loc);
extern size_t wcsxfrm_l (wchar_t *__s1, const wchar_t *__s2,
                         size_t __n, locale_t __loc);
extern wchar_t *wcsdup (const wchar_t *__s);
extern wchar_t *wcschr (const wchar_t *__wcs, wchar_t __wc);
extern wchar_t *wcsrchr (const wchar_t *__wcs, wchar_t __wc);
#ifdef __USE_GNU
extern wchar_t *wcschrnul (const wchar_t *__s, wchar_t __wc);
#endif
extern size_t wcscspn (const wchar_t *__wcs, const wchar_t *__reject);
extern size_t wcsspn (const wchar_t *__wcs, const wchar_t *__accept);
extern wchar_t *wcspbrk (const wchar_t *__wcs, const wchar_t *__accept);
extern wchar_t *wcsstr (const wchar_t *__haystack, const wchar_t *__needle);
extern wchar_t *wcstok (wchar_t *__restrict __s,
                        const wchar_t *__restrict __delim,
                        wchar_t **__restrict __ptr);
extern size_t wcslen (const wchar_t *__s);
extern wchar_t *wcswcs (const wchar_t *__haystack, const wchar_t *__needle);
extern size_t wcsnlen (const wchar_t *__s, size_t __maxlen);

extern wchar_t *wmemchr (const wchar_t *__s, wchar_t __c, size_t __n);
extern int wmemcmp (const wchar_t *__s1, const wchar_t *__s2, size_t __n);
extern wchar_t *wmemcpy (wchar_t *__restrict __s1,
                         const wchar_t *__restrict __s2, size_t __n);
extern wchar_t *wmemmove (wchar_t *__s1, const wchar_t *__s2, size_t __n);
extern wchar_t *wmemset (wchar_t *__s, wchar_t __c, size_t __n);
#ifdef __USE_GNU
extern wchar_t *wmempcpy (wchar_t *__restrict __s1,
                          const wchar_t *__restrict __s2, size_t __n);
#endif

extern wint_t btowc (int __c);
extern int wctob (wint_t __c);
extern int mbsinit (const mbstate_t *__ps);
extern size_t mbrtowc (wchar_t *__restrict __pwc,
                       const char *__restrict __s, size_t __n,
                       mbstate_t *__restrict __p);
extern size_t wcrtomb (char *__restrict __s, wchar_t __wc,
                       mbstate_t *__restrict __ps);
extern size_t __mbrlen (const char *__restrict __s, size_t __n,
                        mbstate_t *__restrict __ps);
extern size_t mbrlen (const char *__restrict __s, size_t __n,
                      mbstate_t *__restrict __ps);
extern size_t mbsrtowcs (wchar_t *__restrict __dst,
                         const char **__restrict __src, size_t __len,
                         mbstate_t *__restrict __ps);
extern size_t wcsrtombs (char *__restrict __dst,
                         const wchar_t **__restrict __src, size_t __len,
                         mbstate_t *__restrict __ps);
extern size_t mbsnrtowcs (wchar_t *__restrict __dst,
                          const char **__restrict __src, size_t __nmc,
                          size_t __len, mbstate_t *__restrict __ps);
extern size_t wcsnrtombs (char *__restrict __dst,
                          const wchar_t **__restrict __src,
                          size_t __nwc, size_t __len,
                          mbstate_t *__restrict __ps);

extern int wcwidth (wchar_t __c);
extern int wcswidth (const wchar_t *__s, size_t __n);

extern double wcstod (const wchar_t *__restrict __nptr,
                      wchar_t **__restrict __endptr);
extern float wcstof (const wchar_t *__restrict __nptr,
                     wchar_t **__restrict __endptr);
extern long double wcstold (const wchar_t *__restrict __nptr,
                            wchar_t **__restrict __endptr);
extern long int wcstol (const wchar_t *__restrict __nptr,
                        wchar_t **__restrict __endptr, int __base);
extern unsigned long int wcstoul (const wchar_t *__restrict __nptr,
                                  wchar_t **__restrict __endptr, int __base);
extern long long int wcstoll (const wchar_t *__restrict __nptr,
                              wchar_t **__restrict __endptr, int __base);
extern unsigned long long int wcstoull (const wchar_t *__restrict __nptr,
                                        wchar_t **__restrict __endptr,
                                        int __base);
extern long long int wcstoq (const wchar_t *__restrict __nptr,
                             wchar_t **__restrict __endptr, int __base);
extern unsigned long long int wcstouq (const wchar_t *__restrict __nptr,
                                       wchar_t **__restrict __endptr,
                                       int __base);

#if __GLIBC_USE (C23_STRTOL)
extern long int __isoc23_wcstol (const wchar_t *__restrict __nptr,
                                 wchar_t **__restrict __endptr, int __base);
extern unsigned long int __isoc23_wcstoul (const wchar_t *__restrict __nptr,
                                           wchar_t **__restrict __endptr,
                                           int __base);
extern long long int __isoc23_wcstoll (const wchar_t *__restrict __nptr,
                                       wchar_t **__restrict __endptr,
                                       int __base);
extern unsigned long long int __isoc23_wcstoull (const wchar_t *__restrict __nptr,
                                                 wchar_t **__restrict __endptr,
                                                 int __base);
# define wcstol __isoc23_wcstol
# define wcstoul __isoc23_wcstoul
# define wcstoll __isoc23_wcstoll
# define wcstoull __isoc23_wcstoull
# define wcstoq __isoc23_wcstoll
# define wcstouq __isoc23_wcstoull
#endif

#ifdef __USE_GNU
extern long int wcstol_l (const wchar_t *__restrict __nptr,
                          wchar_t **__restrict __endptr, int __base,
                          locale_t __loc);
extern unsigned long int wcstoul_l (const wchar_t *__restrict __nptr,
                                    wchar_t **__restrict __endptr,
                                    int __base, locale_t __loc);
extern long long int wcstoll_l (const wchar_t *__restrict __nptr,
                                wchar_t **__restrict __endptr,
                                int __base, locale_t __loc);
extern unsigned long long int wcstoull_l (const wchar_t *__restrict __nptr,
                                          wchar_t **__restrict __endptr,
                                          int __base, locale_t __loc);
extern double wcstod_l (const wchar_t *__restrict __nptr,
                        wchar_t **__restrict __endptr, locale_t __loc);
extern float wcstof_l (const wchar_t *__restrict __nptr,
                       wchar_t **__restrict __endptr, locale_t __loc);
extern long double wcstold_l (const wchar_t *__restrict __nptr,
                              wchar_t **__restrict __endptr,
                              locale_t __loc);
#endif

extern wchar_t *wcpcpy (wchar_t *__restrict __dest,
                        const wchar_t *__restrict __src);
extern wchar_t *wcpncpy (wchar_t *__restrict __dest,
                         const wchar_t *__restrict __src, size_t __n);

extern __FILE *open_wmemstream (wchar_t **__bufloc, size_t *__sizeloc);

extern int fwide (__FILE *__fp, int __mode);

extern int fwprintf (__FILE *__restrict __stream,
                     const wchar_t *__restrict __format, ...);
extern int wprintf (const wchar_t *__restrict __format, ...);
extern int swprintf (wchar_t *__restrict __s, size_t __n,
                     const wchar_t *__restrict __format, ...);
extern int vfwprintf (__FILE *__restrict __s,
                      const wchar_t *__restrict __format,
                      __gnuc_va_list __arg);
extern int vwprintf (const wchar_t *__restrict __format,
                     __gnuc_va_list __arg);
extern int vswprintf (wchar_t *__restrict __s, size_t __n,
                      const wchar_t *__restrict __format,
                      __gnuc_va_list __arg);

extern int fwscanf (__FILE *__restrict __stream,
                    const wchar_t *__restrict __format, ...);
extern int wscanf (const wchar_t *__restrict __format, ...);
extern int swscanf (const wchar_t *__restrict __s,
                    const wchar_t *__restrict __format, ...);
extern int vfwscanf (__FILE *__restrict __s,
                     const wchar_t *__restrict __format,
                     __gnuc_va_list __arg);
extern int vwscanf (const wchar_t *__restrict __format,
                    __gnuc_va_list __arg);
extern int vswscanf (const wchar_t *__restrict __s,
                     const wchar_t *__restrict __format,
                     __gnuc_va_list __arg);

#if !__GLIBC_USE (DEPRECATED_SCANF)
# if __GLIBC_USE (C23_STRTOL)
extern int __isoc23_fwscanf (__FILE *__restrict __stream,
                             const wchar_t *__restrict __format, ...);
extern int __isoc23_wscanf (const wchar_t *__restrict __format, ...);
extern int __isoc23_swscanf (const wchar_t *__restrict __s,
                             const wchar_t *__restrict __format, ...);
extern int __isoc23_vfwscanf (__FILE *__restrict __s,
                              const wchar_t *__restrict __format,
                              __gnuc_va_list __arg);
extern int __isoc23_vwscanf (const wchar_t *__restrict __format,
                             __gnuc_va_list __arg);
extern int __isoc23_vswscanf (const wchar_t *__restrict __s,
                              const wchar_t *__restrict __format,
                              __gnuc_va_list __arg);
#  define fwscanf __isoc23_fwscanf
#  define wscanf __isoc23_wscanf
#  define swscanf __isoc23_swscanf
#  define vfwscanf __isoc23_vfwscanf
#  define vwscanf __isoc23_vwscanf
#  define vswscanf __isoc23_vswscanf
# else
extern int __isoc99_fwscanf (__FILE *__restrict __stream,
                             const wchar_t *__restrict __format, ...);
extern int __isoc99_wscanf (const wchar_t *__restrict __format, ...);
extern int __isoc99_swscanf (const wchar_t *__restrict __s,
                             const wchar_t *__restrict __format, ...);
extern int __isoc99_vfwscanf (__FILE *__restrict __s,
                              const wchar_t *__restrict __format,
                              __gnuc_va_list __arg);
extern int __isoc99_vwscanf (const wchar_t *__restrict __format,
                             __gnuc_va_list __arg);
extern int __isoc99_vswscanf (const wchar_t *__restrict __s,
                              const wchar_t *__restrict __format,
                              __gnuc_va_list __arg);
#  define fwscanf __isoc99_fwscanf
#  define wscanf __isoc99_wscanf
#  define swscanf __isoc99_swscanf
#  define vfwscanf __isoc99_vfwscanf
#  define vwscanf __isoc99_vwscanf
#  define vswscanf __isoc99_vswscanf
# endif
#endif

extern wint_t fgetwc (__FILE *__stream);
extern wint_t getwc (__FILE *__stream);
extern wint_t getwchar (void);
extern wint_t fputwc (wchar_t __wc, __FILE *__stream);
extern wint_t putwc (wchar_t __wc, __FILE *__stream);
extern wint_t putwchar (wchar_t __wc);
extern wchar_t *fgetws (wchar_t *__restrict __ws, int __n,
                        __FILE *__restrict __stream);
extern int fputws (const wchar_t *__restrict __ws,
                   __FILE *__restrict __stream);
extern wint_t ungetwc (wint_t __wc, __FILE *__stream);

#ifdef __USE_GNU
extern wint_t getwc_unlocked (__FILE *__stream);
extern wint_t getwchar_unlocked (void);
extern wint_t fgetwc_unlocked (__FILE *__stream);
extern wint_t fputwc_unlocked (wchar_t __wc, __FILE *__stream);
extern wint_t putwc_unlocked (wchar_t __wc, __FILE *__stream);
extern wint_t putwchar_unlocked (wchar_t __wc);
extern wchar_t *fgetws_unlocked (wchar_t *__restrict __ws, int __n,
                                 __FILE *__restrict __stream);
extern int fputws_unlocked (const wchar_t *__restrict __ws,
                            __FILE *__restrict __stream);
#endif

extern size_t wcsftime (wchar_t *__restrict __s, size_t __maxsize,
                        const wchar_t *__restrict __format,
                        const struct tm *__restrict __tp);
#ifdef __USE_GNU
extern size_t wcsftime_l (wchar_t *__restrict __s, size_t __maxsize,
                          const wchar_t *__restrict __format,
                          const struct tm *__restrict __tp,
                          locale_t __loc);
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_WCHAR_H__ 202311L
#endif

__END_DECLS

#endif /* wchar.h */
