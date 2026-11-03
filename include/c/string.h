/* AltC <string.h> - string handling (glibc-compatible).  */

#ifndef _STRING_H
#define _STRING_H 1

#include <features.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#include <_altc/locale_t.h>

__BEGIN_DECLS

extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
                     size_t __n);
extern void *memmove (void *__dest, const void *__src, size_t __n);
extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
                      int __c, size_t __n);
extern void *memset (void *__s, int __c, size_t __n);
extern int memcmp (const void *__s1, const void *__s2, size_t __n);
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n);
extern void *memchr (const void *__s, int __c, size_t __n);
#ifdef __USE_GNU
extern void *rawmemchr (const void *__s, int __c);
extern void *memrchr (const void *__s, int __c, size_t __n);
#endif

extern char *strcpy (char *__restrict __dest, const char *__restrict __src);
extern char *strncpy (char *__restrict __dest,
                      const char *__restrict __src, size_t __n);
extern char *strcat (char *__restrict __dest, const char *__restrict __src);
extern char *strncat (char *__restrict __dest, const char *__restrict __src,
                      size_t __n);
extern int strcmp (const char *__s1, const char *__s2);
extern int strncmp (const char *__s1, const char *__s2, size_t __n);
extern int strcoll (const char *__s1, const char *__s2);
extern size_t strxfrm (char *__restrict __dest,
                       const char *__restrict __src, size_t __n);
extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l);
extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
                         locale_t __l);

extern char *strdup (const char *__s);
extern char *strndup (const char *__string, size_t __n);

extern char *strchr (const char *__s, int __c);
extern char *strrchr (const char *__s, int __c);
extern char *strchrnul (const char *__s, int __c);
extern size_t strcspn (const char *__s, const char *__reject);
extern size_t strspn (const char *__s, const char *__accept);
extern char *strpbrk (const char *__s, const char *__accept);
extern char *strstr (const char *__haystack, const char *__needle);
extern char *strtok (char *__restrict __s, const char *__restrict __delim);
extern char *__strtok_r (char *__restrict __s,
                         const char *__restrict __delim,
                         char **__restrict __save_ptr);
extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
                       char **__restrict __save_ptr);
extern char *strcasestr (const char *__haystack, const char *__needle);
extern void *memmem (const void *__haystack, size_t __haystacklen,
                     const void *__needle, size_t __needlelen);
extern void *__mempcpy (void *__restrict __dest,
                        const void *__restrict __src, size_t __n);
extern void *mempcpy (void *__restrict __dest,
                      const void *__restrict __src, size_t __n);

extern size_t strlen (const char *__s);
extern size_t strnlen (const char *__string, size_t __maxlen);

extern char *strerror (int __errnum);
#ifdef __USE_GNU
/* GNU version: may return a pointer to a static string.  */
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen);
extern const char *strerrordesc_np (int __err);
extern const char *strerrorname_np (int __err);
#else
/* POSIX (XSI) version, exported by glibc as __xpg_strerror_r.  */
extern int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen);
# define strerror_r __xpg_strerror_r
#endif
extern char *strerror_l (int __errnum, locale_t __l);

#include <strings.h>

extern void explicit_bzero (void *__s, size_t __n);
extern char *strsep (char **__restrict __stringp,
                     const char *__restrict __delim);
extern char *strsignal (int __sig);
#ifdef __USE_GNU
extern const char *sigabbrev_np (int __sig);
extern const char *sigdescr_np (int __sig);
#endif

extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src);
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src);
extern char *__stpncpy (char *__restrict __dest,
                        const char *__restrict __src, size_t __n);
extern char *stpncpy (char *__restrict __dest,
                      const char *__restrict __src, size_t __n);

extern size_t strlcpy (char *__restrict __dest,
                       const char *__restrict __src, size_t __n);
extern size_t strlcat (char *__restrict __dest,
                       const char *__restrict __src, size_t __n);

#ifdef __USE_GNU
extern int strverscmp (const char *__s1, const char *__s2);
extern char *strfry (char *__string);
extern void *memfrob (void *__s, size_t __n);
/* GNU basename: does not modify its argument.  <libgen.h> provides the
   POSIX version.  */
# ifndef basename
extern char *basename (const char *__filename);
# endif
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_STRING_H__ 202311L
#endif

__END_DECLS

#endif /* string.h */
