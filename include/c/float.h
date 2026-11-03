/* AltC <float.h> - characteristics of floating types (x86-64:
   float = IEEE binary32, double = IEEE binary64,
   long double = x87 80-bit extended precision).  */

#ifndef _FLOAT_H___
#define _FLOAT_H___ 1

#define FLT_RADIX 2
#define FLT_ROUNDS 1
#define FLT_EVAL_METHOD 0

#define FLT_MANT_DIG 24
#define DBL_MANT_DIG 53
#define LDBL_MANT_DIG 64

#define FLT_DIG 6
#define DBL_DIG 15
#define LDBL_DIG 18

#define FLT_MIN_EXP (-125)
#define DBL_MIN_EXP (-1021)
#define LDBL_MIN_EXP (-16381)

#define FLT_MIN_10_EXP (-37)
#define DBL_MIN_10_EXP (-307)
#define LDBL_MIN_10_EXP (-4931)

#define FLT_MAX_EXP 128
#define DBL_MAX_EXP 1024
#define LDBL_MAX_EXP 16384

#define FLT_MAX_10_EXP 38
#define DBL_MAX_10_EXP 308
#define LDBL_MAX_10_EXP 4932

/* Hexadecimal floating constants are not valid in C89/C90; the decimal
   forms (GCC's) denote exactly the same values.  */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define FLT_MAX 0x1.fffffep+127F
# define DBL_MAX ((double)0x1.fffffffffffffp+1023)
# define LDBL_MAX 0x1.fffffffffffffffep+16383L
# define FLT_EPSILON 0x1p-23F
# define DBL_EPSILON ((double)0x1p-52)
# define LDBL_EPSILON 0x1p-63L
# define FLT_MIN 0x1p-126F
# define DBL_MIN ((double)0x1p-1022)
# define LDBL_MIN 0x1p-16382L
#else
# define FLT_MAX 3.40282346638528859811704183484516925e+38F
# define DBL_MAX ((double)1.79769313486231570814527423731704357e+308L)
# define LDBL_MAX 1.18973149535723176502126385303097021e+4932L
# define FLT_EPSILON 1.19209289550781250000000000000000000e-7F
# define DBL_EPSILON ((double)2.22044604925031308084726333618164062e-16L)
# define LDBL_EPSILON 1.08420217248550443400745280086994171e-19L
# define FLT_MIN 1.17549435082228750796873653722224568e-38F
# define DBL_MIN ((double)2.22507385850720138309023271733240406e-308L)
# define LDBL_MIN 3.36210314311209350626267781732175260e-4932L
#endif

#define DECIMAL_DIG 21

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
# define FLT_DECIMAL_DIG 9
# define DBL_DECIMAL_DIG 17
# define LDBL_DECIMAL_DIG 21
# define FLT_HAS_SUBNORM 1
# define DBL_HAS_SUBNORM 1
# define LDBL_HAS_SUBNORM 1
# define FLT_TRUE_MIN 0x1p-149F
# define DBL_TRUE_MIN ((double)0x1p-1074)
# define LDBL_TRUE_MIN 0x1p-16445L
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define FLT_IS_IEC_60559 1
# define DBL_IS_IEC_60559 1
# define LDBL_IS_IEC_60559 1
# define FLT_NORM_MAX FLT_MAX
# define DBL_NORM_MAX DBL_MAX
# define LDBL_NORM_MAX LDBL_MAX
# define INFINITY (__builtin_inff ())
# define NAN (__builtin_nanf (""))
# define __STDC_VERSION_FLOAT_H__ 202311L
#endif

#endif /* float.h */
