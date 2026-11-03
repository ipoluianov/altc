/* AltC <math.h> - mathematics (glibc-compatible, x86-64).

   Functions live in libm.so.6 (link with -lm).  The classification macros
   dispatch on the argument type to glibc's exported helpers
   (__fpclassify*, __isnan*, __isinf*, __finite*, __signbit*, ...), using
   _Generic in C11 and later and sizeof otherwise.  fpclassify, isnormal,
   issubnormal and iszero need libm; isnan, isinf, isfinite and signbit are
   also provided by libc.  */

#ifndef _MATH_H
#define _MATH_H 1

#include <features.h>

__BEGIN_DECLS

#define HUGE_VAL (__builtin_huge_val ())
#define HUGE_VALF (__builtin_huge_valf ())
#define HUGE_VALL (__builtin_huge_vall ())
#define INFINITY (__builtin_inff ())
#define NAN (__builtin_nanf (""))

/* FLT_EVAL_METHOD is 0 on x86-64.  */
typedef float float_t;
typedef double double_t;

#define FP_ILOGB0 (-2147483647 - 1)
#define FP_ILOGBNAN (-2147483647 - 1)
#if defined __USE_GNU \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)
# define __FP_LONG_MAX 0x7fffffffffffffffL
# define FP_LLOGB0 (-__FP_LONG_MAX - 1)
# define FP_LLOGBNAN (-__FP_LONG_MAX - 1)
#endif

#define FP_NAN 0
#define FP_INFINITE 1
#define FP_ZERO 2
#define FP_SUBNORMAL 3
#define FP_NORMAL 4

#if defined __USE_GNU \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)
# define FP_INT_UPWARD 0
# define FP_INT_DOWNWARD 1
# define FP_INT_TOWARDZERO 2
# define FP_INT_TONEARESTFROMZERO 3
# define FP_INT_TONEAREST 4
#endif

#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2
#define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)

extern double acos (double __x);
extern float acosf (float __x);
extern long double acosl (long double __x);
extern double asin (double __x);
extern float asinf (float __x);
extern long double asinl (long double __x);
extern double atan (double __x);
extern float atanf (float __x);
extern long double atanl (long double __x);
extern double cos (double __x);
extern float cosf (float __x);
extern long double cosl (long double __x);
extern double sin (double __x);
extern float sinf (float __x);
extern long double sinl (long double __x);
extern double tan (double __x);
extern float tanf (float __x);
extern long double tanl (long double __x);
extern double cosh (double __x);
extern float coshf (float __x);
extern long double coshl (long double __x);
extern double sinh (double __x);
extern float sinhf (float __x);
extern long double sinhl (long double __x);
extern double tanh (double __x);
extern float tanhf (float __x);
extern long double tanhl (long double __x);
extern double exp (double __x);
extern float expf (float __x);
extern long double expl (long double __x);
extern double log (double __x);
extern float logf (float __x);
extern long double logl (long double __x);
extern double log10 (double __x);
extern float log10f (float __x);
extern long double log10l (long double __x);
extern double sqrt (double __x);
extern float sqrtf (float __x);
extern long double sqrtl (long double __x);
extern double ceil (double __x);
extern float ceilf (float __x);
extern long double ceill (long double __x);
extern double fabs (double __x);
extern float fabsf (float __x);
extern long double fabsl (long double __x);
extern double floor (double __x);
extern float floorf (float __x);
extern long double floorl (long double __x);
extern double atan2 (double __x, double __y);
extern float atan2f (float __x, float __y);
extern long double atan2l (long double __x, long double __y);
extern double pow (double __x, double __y);
extern float powf (float __x, float __y);
extern long double powl (long double __x, long double __y);
extern double fmod (double __x, double __y);
extern float fmodf (float __x, float __y);
extern long double fmodl (long double __x, long double __y);
extern double frexp (double __x, int *__exponent);
extern float frexpf (float __x, int *__exponent);
extern long double frexpl (long double __x, int *__exponent);
extern double ldexp (double __x, int __exponent);
extern float ldexpf (float __x, int __exponent);
extern long double ldexpl (long double __x, int __exponent);
extern double modf (double __x, double *__iptr);
extern float modff (float __x, float *__iptr);
extern long double modfl (long double __x, long double *__iptr);
extern double acosh (double __x);
extern float acoshf (float __x);
extern long double acoshl (long double __x);
extern double asinh (double __x);
extern float asinhf (float __x);
extern long double asinhl (long double __x);
extern double atanh (double __x);
extern float atanhf (float __x);
extern long double atanhl (long double __x);
extern double expm1 (double __x);
extern float expm1f (float __x);
extern long double expm1l (long double __x);
extern double log1p (double __x);
extern float log1pf (float __x);
extern long double log1pl (long double __x);
extern double logb (double __x);
extern float logbf (float __x);
extern long double logbl (long double __x);
extern double exp2 (double __x);
extern float exp2f (float __x);
extern long double exp2l (long double __x);
extern double log2 (double __x);
extern float log2f (float __x);
extern long double log2l (long double __x);
extern double cbrt (double __x);
extern float cbrtf (float __x);
extern long double cbrtl (long double __x);
extern double erf (double __x);
extern float erff (float __x);
extern long double erfl (long double __x);
extern double erfc (double __x);
extern float erfcf (float __x);
extern long double erfcl (long double __x);
extern double lgamma (double __x);
extern float lgammaf (float __x);
extern long double lgammal (long double __x);
extern double tgamma (double __x);
extern float tgammaf (float __x);
extern long double tgammal (long double __x);
extern double rint (double __x);
extern float rintf (float __x);
extern long double rintl (long double __x);
extern double nearbyint (double __x);
extern float nearbyintf (float __x);
extern long double nearbyintl (long double __x);
extern double round (double __x);
extern float roundf (float __x);
extern long double roundl (long double __x);
extern double trunc (double __x);
extern float truncf (float __x);
extern long double truncl (long double __x);
extern double hypot (double __x, double __y);
extern float hypotf (float __x, float __y);
extern long double hypotl (long double __x, long double __y);
extern double copysign (double __x, double __y);
extern float copysignf (float __x, float __y);
extern long double copysignl (long double __x, long double __y);
extern double nextafter (double __x, double __y);
extern float nextafterf (float __x, float __y);
extern long double nextafterl (long double __x, long double __y);
extern double remainder (double __x, double __y);
extern float remainderf (float __x, float __y);
extern long double remainderl (long double __x, long double __y);
extern double fdim (double __x, double __y);
extern float fdimf (float __x, float __y);
extern long double fdiml (long double __x, long double __y);
extern double fmax (double __x, double __y);
extern float fmaxf (float __x, float __y);
extern long double fmaxl (long double __x, long double __y);
extern double fmin (double __x, double __y);
extern float fminf (float __x, float __y);
extern long double fminl (long double __x, long double __y);
extern double nan (const char *__tagb);
extern float nanf (const char *__tagb);
extern long double nanl (const char *__tagb);
extern double scalbn (double __x, int __n);
extern float scalbnf (float __x, int __n);
extern long double scalbnl (long double __x, int __n);
extern double scalbln (double __x, long int __n);
extern float scalblnf (float __x, long int __n);
extern long double scalblnl (long double __x, long int __n);
extern int ilogb (double __x);
extern int ilogbf (float __x);
extern int ilogbl (long double __x);
extern long int lrint (double __x);
extern long int lrintf (float __x);
extern long int lrintl (long double __x);
extern long int lround (double __x);
extern long int lroundf (float __x);
extern long int lroundl (long double __x);
extern long long int llrint (double __x);
extern long long int llrintf (float __x);
extern long long int llrintl (long double __x);
extern long long int llround (double __x);
extern long long int llroundf (float __x);
extern long long int llroundl (long double __x);
extern double remquo (double __x, double __y, int *__quo);
extern float remquof (float __x, float __y, int *__quo);
extern long double remquol (long double __x, long double __y, int *__quo);
extern double fma (double __x, double __y, double __z);
extern float fmaf (float __x, float __y, float __z);
extern long double fmal (long double __x, long double __y, long double __z);
extern double nexttoward (double __x, long double __y);
extern float nexttowardf (float __x, long double __y);
extern long double nexttowardl (long double __x, long double __y);

/* BSD / SVID / X/Open extensions.  */
extern int isinf (double __value);
extern int isinff (float __value);
extern int isinfl (long double __value);
extern int finite (double __value);
extern int finitef (float __value);
extern int finitel (long double __value);
extern int isnan (double __value);
extern int isnanf (float __value);
extern int isnanl (long double __value);
extern double significand (double __x);
extern float significandf (float __x);
extern long double significandl (long double __x);
extern double drem (double __x, double __n);
extern float dremf (float __x, float __n);
extern long double dreml (long double __x, long double __n);
extern double scalb (double __x, double __n);
extern float scalbf (float __x, float __n);
extern long double scalbl (long double __x, long double __n);
extern double j0 (double __x);
extern float j0f (float __x);
extern long double j0l (long double __x);
extern double j1 (double __x);
extern float j1f (float __x);
extern long double j1l (long double __x);
extern double y0 (double __x);
extern float y0f (float __x);
extern long double y0l (long double __x);
extern double y1 (double __x);
extern float y1f (float __x);
extern long double y1l (long double __x);
extern double gamma (double __x);
extern float gammaf (float __x);
extern long double gammal (long double __x);
extern double jn (int __n, double __x);
extern float jnf (int __n, float __x);
extern long double jnl (int __n, long double __x);
extern double yn (int __n, double __x);
extern float ynf (int __n, float __x);
extern long double ynl (int __n, long double __x);
extern double lgamma_r (double __x, int *__signgamp);
extern float lgammaf_r (float __x, int *__signgamp);
extern long double lgammal_r (long double __x, int *__signgamp);

extern int signgam;

#ifdef __USE_GNU
extern double exp10 (double __x);
extern float exp10f (float __x);
extern long double exp10l (long double __x);
extern void sincos (double __x, double *__sinx, double *__cosx);
extern void sincosf (float __x, float *__sinx, float *__cosx);
extern void sincosl (long double __x, long double *__sinx, long double *__cosx);
#endif

#if defined __USE_GNU \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)
extern double exp10m1 (double __x);
extern float exp10m1f (float __x);
extern long double exp10m1l (long double __x);
extern double exp2m1 (double __x);
extern float exp2m1f (float __x);
extern long double exp2m1l (long double __x);
extern double log10p1 (double __x);
extern float log10p1f (float __x);
extern long double log10p1l (long double __x);
extern double log2p1 (double __x);
extern float log2p1f (float __x);
extern long double log2p1l (long double __x);
extern double logp1 (double __x);
extern float logp1f (float __x);
extern long double logp1l (long double __x);
extern double acospi (double __x);
extern float acospif (float __x);
extern long double acospil (long double __x);
extern double asinpi (double __x);
extern float asinpif (float __x);
extern long double asinpil (long double __x);
extern double atanpi (double __x);
extern float atanpif (float __x);
extern long double atanpil (long double __x);
extern double cospi (double __x);
extern float cospif (float __x);
extern long double cospil (long double __x);
extern double sinpi (double __x);
extern float sinpif (float __x);
extern long double sinpil (long double __x);
extern double tanpi (double __x);
extern float tanpif (float __x);
extern long double tanpil (long double __x);
extern double roundeven (double __x);
extern float roundevenf (float __x);
extern long double roundevenl (long double __x);
extern double nextup (double __x);
extern float nextupf (float __x);
extern long double nextupl (long double __x);
extern double nextdown (double __x);
extern float nextdownf (float __x);
extern long double nextdownl (long double __x);
extern double atan2pi (double __x, double __y);
extern float atan2pif (float __x, float __y);
extern long double atan2pil (long double __x, long double __y);
extern double fmaximum (double __x, double __y);
extern float fmaximumf (float __x, float __y);
extern long double fmaximuml (long double __x, long double __y);
extern double fminimum (double __x, double __y);
extern float fminimumf (float __x, float __y);
extern long double fminimuml (long double __x, long double __y);
extern double fmaximum_num (double __x, double __y);
extern float fmaximum_numf (float __x, float __y);
extern long double fmaximum_numl (long double __x, long double __y);
extern double fminimum_num (double __x, double __y);
extern float fminimum_numf (float __x, float __y);
extern long double fminimum_numl (long double __x, long double __y);
extern double fmaximum_mag (double __x, double __y);
extern float fmaximum_magf (float __x, float __y);
extern long double fmaximum_magl (long double __x, long double __y);
extern double fminimum_mag (double __x, double __y);
extern float fminimum_magf (float __x, float __y);
extern long double fminimum_magl (long double __x, long double __y);
extern double fmaximum_mag_num (double __x, double __y);
extern float fmaximum_mag_numf (float __x, float __y);
extern long double fmaximum_mag_numl (long double __x, long double __y);
extern double fminimum_mag_num (double __x, double __y);
extern float fminimum_mag_numf (float __x, float __y);
extern long double fminimum_mag_numl (long double __x, long double __y);
extern double fmaxmag (double __x, double __y);
extern float fmaxmagf (float __x, float __y);
extern long double fmaxmagl (long double __x, long double __y);
extern double fminmag (double __x, double __y);
extern float fminmagf (float __x, float __y);
extern long double fminmagl (long double __x, long double __y);
extern long int llogb (double __x);
extern long int llogbf (float __x);
extern long int llogbl (long double __x);
extern int totalorder (const double *__x, const double *__y);
extern int totalorderf (const float *__x, const float *__y);
extern int totalorderl (const long double *__x, const long double *__y);
extern int totalordermag (const double *__x, const double *__y);
extern int totalordermagf (const float *__x, const float *__y);
extern int totalordermagl (const long double *__x, const long double *__y);
extern double getpayload (const double *__x);
extern float getpayloadf (const float *__x);
extern long double getpayloadl (const long double *__x);
extern int setpayload (double *__x, double __payload);
extern int setpayloadf (float *__x, float __payload);
extern int setpayloadl (long double *__x, long double __payload);
extern int setpayloadsig (double *__x, double __payload);
extern int setpayloadsigf (float *__x, float __payload);
extern int setpayloadsigl (long double *__x, long double __payload);
extern int canonicalize (double *__cx, const double *__x);
extern int canonicalizef (float *__cx, const float *__x);
extern int canonicalizel (long double *__cx, const long double *__x);
extern float fadd (double __x, double __y);
extern float faddl (long double __x, long double __y);
extern double daddl (long double __x, long double __y);
extern float fsub (double __x, double __y);
extern float fsubl (long double __x, long double __y);
extern double dsubl (long double __x, long double __y);
extern float fmul (double __x, double __y);
extern float fmull (long double __x, long double __y);
extern double dmull (long double __x, long double __y);
extern float fdiv (double __x, double __y);
extern float fdivl (long double __x, long double __y);
extern double ddivl (long double __x, long double __y);
extern float ffma (double __x, double __y, double __z);
extern float ffmal (long double __x, long double __y, long double __z);
extern double dfmal (long double __x, long double __y, long double __z);
extern float fsqrt (double __x);
extern float fsqrtl (long double __x);
extern double dsqrtl (long double __x);
#endif

/* Helpers used by the classification macros.  */
extern int __fpclassify (double __value) __attribute__ ((__const__));
extern int __fpclassifyf (float __value) __attribute__ ((__const__));
extern int __fpclassifyl (long double __value) __attribute__ ((__const__));
extern int __signbit (double __value) __attribute__ ((__const__));
extern int __signbitf (float __value) __attribute__ ((__const__));
extern int __signbitl (long double __value) __attribute__ ((__const__));
extern int __isinf (double __value) __attribute__ ((__const__));
extern int __isinff (float __value) __attribute__ ((__const__));
extern int __isinfl (long double __value) __attribute__ ((__const__));
extern int __finite (double __value) __attribute__ ((__const__));
extern int __finitef (float __value) __attribute__ ((__const__));
extern int __finitel (long double __value) __attribute__ ((__const__));
extern int __isnan (double __value) __attribute__ ((__const__));
extern int __isnanf (float __value) __attribute__ ((__const__));
extern int __isnanl (long double __value) __attribute__ ((__const__));
extern int __iseqsig (double __x, double __y);
extern int __iseqsigf (float __x, float __y);
extern int __iseqsigl (long double __x, long double __y);
extern int __issignaling (double __value) __attribute__ ((__const__));
extern int __issignalingf (float __value) __attribute__ ((__const__));
extern int __issignalingl (long double __value) __attribute__ ((__const__));

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
# define __MATH_TG(TG_ARG, FUNC, ARGS) \
  _Generic ((TG_ARG), \
            float: FUNC ## f ARGS, \
            long double: FUNC ## l ARGS, \
            default: FUNC ARGS)
#else
# define __MATH_TG(TG_ARG, FUNC, ARGS) \
  (sizeof (TG_ARG) == sizeof (float) ? FUNC ## f ARGS \
   : sizeof (TG_ARG) == sizeof (double) ? FUNC ARGS \
   : FUNC ## l ARGS)
#endif

#define fpclassify(x) __MATH_TG ((x), __fpclassify, (x))
#define signbit(x) __builtin_signbit (x)
#define isnormal(x) (fpclassify (x) == FP_NORMAL)
#define isfinite(x) __MATH_TG ((x), __finite, (x))
#define isnan(x) __builtin_isnan (x)
#define isinf(x) __MATH_TG ((x), __isinf, (x))

#if defined __USE_GNU \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L)
# define issignaling(x) __MATH_TG ((x), __issignaling, (x))
# define issubnormal(x) (fpclassify (x) == FP_SUBNORMAL)
# define iszero(x) (fpclassify (x) == FP_ZERO)
# define iscanonical(x) ((void) (x), 1)
# define iseqsig(x, y) __MATH_TG ((x) + (y), __iseqsig, ((x), (y)))
#endif

/* Quiet comparisons.  The arguments may be evaluated more than once.  */
#define isunordered(x, y) (isnan (x) || isnan (y))
#define isgreater(x, y) (!isunordered ((x), (y)) && (x) > (y))
#define isgreaterequal(x, y) (!isunordered ((x), (y)) && (x) >= (y))
#define isless(x, y) (!isunordered ((x), (y)) && (x) < (y))
#define islessequal(x, y) (!isunordered ((x), (y)) && (x) <= (y))
#define islessgreater(x, y) \
  (!isunordered ((x), (y)) && ((x) < (y) || (x) > (y)))

#ifdef __USE_XOPEN
# define MAXFLOAT 3.40282347e+38F
#endif

/* Mathematical constants (BSD/XOPEN).  */
#define M_E 2.7182818284590452354
#define M_LOG2E 1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440

#ifdef __USE_GNU
#define M_Ef 2.7182818284590452354f
#define M_LOG2Ef 1.4426950408889634074f
#define M_LOG10Ef 0.43429448190325182765f
#define M_LN2f 0.69314718055994530942f
#define M_LN10f 2.30258509299404568402f
#define M_PIf 3.14159265358979323846f
#define M_PI_2f 1.57079632679489661923f
#define M_PI_4f 0.78539816339744830962f
#define M_1_PIf 0.31830988618379067154f
#define M_2_PIf 0.63661977236758134308f
#define M_2_SQRTPIf 1.12837916709551257390f
#define M_SQRT2f 1.41421356237309504880f
#define M_SQRT1_2f 0.70710678118654752440f
#define M_El 2.718281828459045235360287471352662498L
#define M_LOG2El 1.442695040888963407359924681001892137L
#define M_LOG10El 0.434294481903251827651128918916605082L
#define M_LN2l 0.693147180559945309417232121458176568L
#define M_LN10l 2.302585092994045684017991454684364208L
#define M_PIl 3.141592653589793238462643383279502884L
#define M_PI_2l 1.570796326794896619231321691639751442L
#define M_PI_4l 0.785398163397448309615660845819875721L
#define M_1_PIl 0.318309886183790671537767526745028724L
#define M_2_PIl 0.636619772367581343075535053490057448L
#define M_2_SQRTPIl 1.128379167095512573896158903121545172L
#define M_SQRT2l 1.414213562373095048801688724209698079L
#define M_SQRT1_2l 0.707106781186547524400844362104849039L
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_MATH_H__ 202311L
#endif

__END_DECLS

#endif /* math.h */
