/* AltC <strings.h> - BSD string functions.  */

#ifndef _STRINGS_H
#define _STRINGS_H 1

#include <features.h>

#define __need_size_t
#include <stddef.h>

#include <_altc/locale_t.h>

__BEGIN_DECLS

extern int bcmp (const void *__s1, const void *__s2, size_t __n);
extern void bcopy (const void *__src, void *__dest, size_t __n);
extern void bzero (void *__s, size_t __n);
extern char *index (const char *__s, int __c);
extern char *rindex (const char *__s, int __c);

extern int ffs (int __i) __attribute__ ((__const__));
extern int ffsl (long int __l) __attribute__ ((__const__));
extern int ffsll (long long int __ll) __attribute__ ((__const__));

extern int strcasecmp (const char *__s1, const char *__s2);
extern int strncasecmp (const char *__s1, const char *__s2, size_t __n);
extern int strcasecmp_l (const char *__s1, const char *__s2, locale_t __loc);
extern int strncasecmp_l (const char *__s1, const char *__s2,
                          size_t __n, locale_t __loc);

__END_DECLS

#endif /* strings.h */
