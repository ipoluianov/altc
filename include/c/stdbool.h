/* AltC <stdbool.h> - boolean type and values.  */

#ifndef _STDBOOL_H
#define _STDBOOL_H 1

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 202311L
/* bool, true and false are keywords in C23.  */
#elif !defined __cplusplus
# define bool _Bool
# define true 1
# define false 0
#endif

#define __bool_true_false_are_defined 1

#endif /* stdbool.h */
