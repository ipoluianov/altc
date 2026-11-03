/* AltC <wctype.h> - wide character classification (glibc-compatible).  */

#ifndef _WCTYPE_H
#define _WCTYPE_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/locale_t.h>

#ifndef __wint_t_defined
# define __wint_t_defined 1
# ifndef _WINT_T
#  define _WINT_T 1
typedef unsigned int wint_t;
# endif
#endif

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif

__BEGIN_DECLS

#ifndef _BITS_WCTYPE_WCHAR_H
#define _BITS_WCTYPE_WCHAR_H 1
typedef unsigned long int wctype_t;
#endif

typedef const __int32_t *wctrans_t;

extern int iswalnum (wint_t __wc);
extern int iswalpha (wint_t __wc);
extern int iswcntrl (wint_t __wc);
extern int iswdigit (wint_t __wc);
extern int iswgraph (wint_t __wc);
extern int iswlower (wint_t __wc);
extern int iswprint (wint_t __wc);
extern int iswpunct (wint_t __wc);
extern int iswspace (wint_t __wc);
extern int iswupper (wint_t __wc);
extern int iswxdigit (wint_t __wc);
extern int iswblank (wint_t __wc);
extern wctype_t wctype (const char *__property);
extern int iswctype (wint_t __wc, wctype_t __desc);
extern wint_t towlower (wint_t __wc);
extern wint_t towupper (wint_t __wc);

extern wctrans_t wctrans (const char *__property);
extern wint_t towctrans (wint_t __wc, wctrans_t __desc);

extern int iswalnum_l (wint_t __wc, locale_t __locale);
extern int iswalpha_l (wint_t __wc, locale_t __locale);
extern int iswcntrl_l (wint_t __wc, locale_t __locale);
extern int iswdigit_l (wint_t __wc, locale_t __locale);
extern int iswgraph_l (wint_t __wc, locale_t __locale);
extern int iswlower_l (wint_t __wc, locale_t __locale);
extern int iswprint_l (wint_t __wc, locale_t __locale);
extern int iswpunct_l (wint_t __wc, locale_t __locale);
extern int iswspace_l (wint_t __wc, locale_t __locale);
extern int iswupper_l (wint_t __wc, locale_t __locale);
extern int iswxdigit_l (wint_t __wc, locale_t __locale);
extern int iswblank_l (wint_t __wc, locale_t __locale);
extern wctype_t wctype_l (const char *__property, locale_t __locale);
extern int iswctype_l (wint_t __wc, wctype_t __desc, locale_t __locale);
extern wint_t towlower_l (wint_t __wc, locale_t __locale);
extern wint_t towupper_l (wint_t __wc, locale_t __locale);
extern wctrans_t wctrans_l (const char *__property, locale_t __locale);
extern wint_t towctrans_l (wint_t __wc, wctrans_t __desc, locale_t __locale);

__END_DECLS

#endif /* wctype.h */
