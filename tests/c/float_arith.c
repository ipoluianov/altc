#include <stdio.h>
#include <float.h>
#include <math.h>

int main(void) {
    float f = 1.5f, g = -2.25f; double d = 3.125, e = -0.1; long double ld = 1.0L / 3.0L;
    printf("%f %f %f %f\n", f + g, f * g, f / g, f - g);
    printf("%.17g %.17g %.17g\n", d * e, d / e, d + e);
    printf("%.20Lg %.20Lg\n", ld, ld * 3.0L - 1.0L);
    printf("%Lf %Le %La\n", ld, ld, 1.0L);
    printf("%.10e %.3E %g %G %a %A\n", d, e, 1e-5, 1e20, 1.0, 0.5);
    printf("%g %g %g %g\n", 100000.0, 1000000.0, 0.0001, 0.00001);
    printf("%10.3f|%-10.3f|%+f|% f|%#.0f|%010.2f\n", d, d, d, d, d, -d);
    printf("%.0f %.0f %.0f %.0f\n", 0.5, 1.5, 2.5, -0.5);
    printf("%a %a %a\n", 0x1.8p3, 0x1p-1074, -0x1.fffffffffffffp1023);
    printf("%e %e %e\n", DBL_MAX, DBL_MIN, DBL_EPSILON);
    printf("%e %e %e\n", FLT_MAX, FLT_MIN, FLT_EPSILON);
    printf("%Le %Le %Le\n", LDBL_MAX, LDBL_MIN, LDBL_EPSILON);
    printf("%d %d %d %d %d\n", FLT_DIG, DBL_DIG, LDBL_DIG, DBL_MANT_DIG, LDBL_MANT_DIG);
    double nan = NAN, inf = INFINITY, z = 0.0;
    printf("%f %f %f %F\n", nan, inf, -inf, inf);
    printf("%d %d %d %d %d %d\n", nan == nan, nan != nan, nan < 1, nan > 1, nan <= nan, !(nan >= 0));
    printf("%d %d %d %d\n", inf > DBL_MAX, -inf < -DBL_MAX, inf == inf, inf - inf != inf - inf);
    printf("%d %d %d\n", isnan(nan), isinf(-inf), isfinite(1.0));
    printf("%f %f\n", 1.0 / inf, -1.0 / inf);
    printf("%d %d\n", !!signbit(-z), !!signbit(z));
    printf("%d\n", 0.1 + 0.2 == 0.3);
    printf("%d\n", 0.1f + 0.2f == 0.3f);
    printf("%d\n", (float)0.1 == 0.1);
    printf("%.20f\n", 0.1f);
    printf("%.20f\n", (double)0.1f);
    float fx = 16777216.0f; fx += 1.0f; printf("%.1f\n", fx);
    double dx = 9007199254740992.0; dx += 1.0; printf("%.1f\n", dx);
    long double lx = 9007199254740992.0L; lx += 1.0L; printf("%.1Lf\n", lx);
    printf("%f %f %f\n", -f, -d, (double)-ld);
    f += 1; f *= 2; f /= 4; f -= 0.125f; printf("%f\n", f);
    d += 1; d *= 2; d /= 4; d -= 0.125; printf("%f\n", d);
    ld += 1; ld *= 2; ld /= 4; ld -= 0.125L; printf("%.15Lf\n", ld);
    int i = 7; i += 0.9; printf("%d\n", i);
    i *= 1.5f; printf("%d\n", i);
    printf("%f %f\n", 1e308 * 10, -1e308 * 10);
    printf("%g %g\n", 1e-320, 4.9406564584124654e-324);
    printf("%.3f %.3f\n", 2.0f / 3.0f, (double)(2.0f / 3.0f));
    float ff[5] = {1.25f, -3.5f, 0.0f, 1e10f, 1e-10f};
    double sum = 0; for (int k = 0; k < 5; k++) sum += ff[k] * k; printf("%g\n", sum);
    printf("%d %d %d\n", 1.0 < 2.0f, 1.0L > 0.5, -0.0 == 0.0);
    printf("%d %d\n", d ? 1 : 0, 0.0 ? 1 : 0);
    printf("%d %d\n", !0.0, !nan);
    double hex = 0x1.2p+3; float hf = 0x.8p1f; long double hl = 0x1p-16382L;
    printf("%g %g %Lg\n", hex, hf, hl);
    printf("%g %g\n", 1.e2, .5e-1);
    printf("%g %g %g\n", 1e+2f, 2E3, 3.L);
    return 0;
}
