/* AltC <stdarg.h> - variable arguments.

   Honours __need___va_list (used by glibc headers): in that case only the
   type __gnuc_va_list is defined.  */

#ifndef __need___va_list
# ifndef _STDARG_H
#  define _STDARG_H 1
#  define _ANSI_STDARG_H_ 1
#  define __altc_stdarg_all 1
# endif
#endif
#undef __need___va_list

#ifndef __GNUC_VA_LIST
# define __GNUC_VA_LIST 1
typedef __builtin_va_list __gnuc_va_list;
#endif

#ifdef __altc_stdarg_all
# undef __altc_stdarg_all

# define va_start(v, l) __builtin_va_start (v, l)
# define va_end(v) __builtin_va_end (v)
# define va_arg(v, l) __builtin_va_arg (v, l)
# if !defined __STRICT_ANSI__ \
     || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
#  define va_copy(d, s) __builtin_va_copy (d, s)
# endif
# define __va_copy(d, s) __builtin_va_copy (d, s)

# if !defined _VA_LIST_ && !defined _VA_LIST && !defined _VA_LIST_DEFINED \
     && !defined _VA_LIST_T_H && !defined __va_list__
typedef __gnuc_va_list va_list;
#  define _VA_LIST_ 1
#  define _VA_LIST 1
#  define _VA_LIST_DEFINED 1
#  define _VA_LIST_T_H 1
#  define __va_list__ 1
# endif

# if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
#  define __STDC_VERSION_STDARG_H__ 202311L
# endif
#endif
