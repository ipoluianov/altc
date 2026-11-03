/* AltC <stdalign.h> - alignment macros.  */

#ifndef _STDALIGN_H
#define _STDALIGN_H 1

#if !defined __STDC_VERSION__ || __STDC_VERSION__ < 202311L
/* In C23 alignas and alignof are keywords.  */
# define alignas _Alignas
# define alignof _Alignof
#endif
#define __alignas_is_defined 1
#define __alignof_is_defined 1

#endif /* stdalign.h */
