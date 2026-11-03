/* Smoke test: <math.h>, <fenv.h>, <float.h>.  Link with -lm.  */
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

int
main (void)
{
  double x = 0.5, ip;
  float xf = 2.0f;
  long double xl = 3.0L;
  int e, q;
  volatile double zero = 0.0;

  printf ("%.6f %.6f %.6f %.6f\n", sin (x), cos (x), tan (x), atan2 (1, 1));
  printf ("%.6f %.6f %.6f %.6f\n", exp (1.0), log (10.0), log10 (1000.0),
          pow (2.0, 10.0));
  printf ("%.6f %.6f %.6Lf %.6f\n", sqrt (2.0), (double) sqrtf (xf),
          sqrtl (xl), cbrt (27.0));
  printf ("%.1f %.1f %.1f %.1f %.1f\n", floor (-1.5), ceil (-1.5),
          round (2.5), trunc (-2.7), rint (3.5));
  x = remquo (10.0, 3.0, &q);
  printf ("%.6f %.6f %d\n", fmod (10.0, 3.0), x, q);
  x = frexp (8.0, &e);
  printf ("frexp %.3f %d ldexp %.1f\n", x, e, ldexp (1.0, 5));
  x = modf (3.75, &ip);
  printf ("modf %.2f %.0f\n", x, ip);
  printf ("fma %.1f hypot %.1f fmax %.1f fmin %.1f fdim %.1f\n",
          fma (2.0, 3.0, 1.0), hypot (3.0, 4.0), fmax (1.0, 2.0),
          fmin (1.0, 2.0), fdim (5.0, 3.0));
  printf ("lrint %ld llround %lld nextafter %d tgamma %.1f lgamma %.4f\n",
          lrint (2.5), llround (-2.5), nextafter (1.0, 2.0) > 1.0,
          tgamma (5.0), lgamma (10.0));
  printf ("expm1 %.6f log1p %.6f exp2 %.1f log2 %.1f erf %.6f\n",
          expm1 (1e-5), log1p (1e-5), exp2 (8.0), log2 (1024.0), erf (0.5));
  printf ("copysign %.1f nan %d scalbn %.1f ilogb %d logb %.1f\n",
          copysign (3.0, -1.0), isnan (nan ("")) != 0, scalbn (1.0, 3),
          ilogb (1024.0), logb (8.0));
  printf ("sinf %.4f powl %.4Lf\n", (double) sinf (1.0f), powl (2.0L, 0.5L));

  printf ("classify d %d %d %d %d %d\n", fpclassify (1.0), fpclassify (0.0),
          fpclassify (HUGE_VAL), fpclassify (NAN), fpclassify (DBL_MIN / 4));
  printf ("classify f %d %d %d\n", fpclassify (1.0f), fpclassify (FLT_MIN / 4),
          fpclassify (INFINITY));
  printf ("classify l %d %d %d\n", fpclassify (1.0L),
          fpclassify (LDBL_MIN / 4), fpclassify ((long double) NAN));
  printf ("isnan %d %d %d isinf %d %d %d isfinite %d %d isnormal %d %d\n",
          isnan (NAN) != 0, isnan (1.0f) != 0, isnan ((long double) NAN) != 0,
          isinf (-INFINITY) != 0, isinf (HUGE_VALF) != 0,
          isinf (HUGE_VALL) != 0, isfinite (1.0) != 0, isfinite (1 / zero) != 0,
          isnormal (1.0) != 0, isnormal (DBL_MIN / 2) != 0);
  printf ("signbit %d %d %d %d\n", signbit (-0.0) != 0, signbit (0.0) != 0,
          signbit (-1.0f) != 0, signbit (-1.0L) != 0);
  printf ("isgreater %d isless %d islessgreater %d isunordered %d %d\n",
          isgreater (2.0, 1.0), isless (2.0, 1.0), islessgreater (1.0, 2.0),
          isunordered (NAN, 1.0), isunordered (1.0, 2.0));
  printf ("DBL_DIG %d LDBL_MANT_DIG %d FLT_EVAL_METHOD %d DECIMAL_DIG %d\n",
          DBL_DIG, LDBL_MANT_DIG, FLT_EVAL_METHOD, DECIMAL_DIG);
  printf ("M_PI %.15f M_E %.15f M_SQRT2 %.15f\n", M_PI, M_E, M_SQRT2);
  printf ("ldbl %La %Lg\n", LDBL_EPSILON, LDBL_MAX);

  feclearexcept (FE_ALL_EXCEPT);
  x = 1.0 / zero;
  printf ("divbyzero %d\n", fetestexcept (FE_DIVBYZERO) != 0);
  printf ("round mode %d\n", fegetround () == FE_TONEAREST);
  fesetround (FE_UPWARD);
  printf ("rint up %.1f\n", rint (2.1));
  fesetround (FE_TONEAREST);
  {
    fenv_t env;
    fegetenv (&env);
    fesetenv (FE_DFL_ENV);
    fesetenv (&env);
  }
  printf ("x %g\n", x);
  return 0;
}
