/* AltC <ctype.h> - character classification (glibc-compatible).  */

#ifndef _CTYPE_H
#define _CTYPE_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/locale_t.h>

__BEGIN_DECLS

/* Bit masks in glibc's classification table (little-endian layout).  */
#ifndef _ISbit
# define _ISbit(bit) ((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))
enum
{
  _ISupper = _ISbit (0),
  _ISlower = _ISbit (1),
  _ISalpha = _ISbit (2),
  _ISdigit = _ISbit (3),
  _ISxdigit = _ISbit (4),
  _ISspace = _ISbit (5),
  _ISprint = _ISbit (6),
  _ISgraph = _ISbit (7),
  _ISblank = _ISbit (8),
  _IScntrl = _ISbit (9),
  _ISpunct = _ISbit (10),
  _ISalnum = _ISbit (11)
};
#endif

extern const unsigned short int **__ctype_b_loc (void)
     __attribute__ ((__const__));
extern const __int32_t **__ctype_tolower_loc (void)
     __attribute__ ((__const__));
extern const __int32_t **__ctype_toupper_loc (void)
     __attribute__ ((__const__));

#define __isctype(c, type) \
  ((*__ctype_b_loc ())[(int) (c)] & (unsigned short int) type)

#define __isascii(c) (((c) & ~0x7f) == 0)
#define __toascii(c) ((c) & 0x7f)

extern int isalnum (int);
extern int isalpha (int);
extern int iscntrl (int);
extern int isdigit (int);
extern int islower (int);
extern int isgraph (int);
extern int isprint (int);
extern int ispunct (int);
extern int isspace (int);
extern int isupper (int);
extern int isxdigit (int);
extern int isblank (int);
extern int tolower (int __c);
extern int toupper (int __c);

extern int isascii (int __c);
extern int toascii (int __c);
extern int _toupper (int);
extern int _tolower (int);

#ifndef __NO_CTYPE
# define isalnum(c) __isctype((c), _ISalnum)
# define isalpha(c) __isctype((c), _ISalpha)
# define iscntrl(c) __isctype((c), _IScntrl)
# define isdigit(c) __isctype((c), _ISdigit)
# define islower(c) __isctype((c), _ISlower)
# define isgraph(c) __isctype((c), _ISgraph)
# define isprint(c) __isctype((c), _ISprint)
# define ispunct(c) __isctype((c), _ISpunct)
# define isspace(c) __isctype((c), _ISspace)
# define isupper(c) __isctype((c), _ISupper)
# define isxdigit(c) __isctype((c), _ISxdigit)
# define isblank(c) __isctype((c), _ISblank)
# define isascii(c) __isascii (c)
# define toascii(c) __toascii (c)
#endif

#define __isctype_l(c, type, locale) \
  ((locale)->__ctype_b[(int) (c)] & (unsigned short int) type)

extern int isalnum_l (int, locale_t);
extern int isalpha_l (int, locale_t);
extern int iscntrl_l (int, locale_t);
extern int isdigit_l (int, locale_t);
extern int islower_l (int, locale_t);
extern int isgraph_l (int, locale_t);
extern int isprint_l (int, locale_t);
extern int ispunct_l (int, locale_t);
extern int isspace_l (int, locale_t);
extern int isupper_l (int, locale_t);
extern int isxdigit_l (int, locale_t);
extern int isblank_l (int, locale_t);
#ifdef __USE_GNU
extern int isctype (int __c, int __mask);
#endif

extern int __tolower_l (int __c, locale_t __l);
extern int tolower_l (int __c, locale_t __l);
extern int __toupper_l (int __c, locale_t __l);
extern int toupper_l (int __c, locale_t __l);

__END_DECLS

#endif /* ctype.h */
