#include <stdio.h>
#include <math.h>

int main(void) {
    double xs[] = {0.25, 1.0, 2.5, 10.0, 100.0};
    for (int i = 0; i < 5; i++) {
        double x = xs[i];
        printf("%.12f %.12f %.12f %.12f %.12f %.12f\n", sqrt(x), log(x), exp(x / 10), sin(x), cos(x), atan(x));
        printf("%.12f %.12f %.12f %.12f\n", pow(x, 1.5), log10(x), log2(x), cbrt(x));
        printf("%.6f %.6f %.6f\n", sqrtf((float)x), (double)expf((float)x / 10), (double)sinf((float)x));
        printf("%.15Lf %.15Lf\n", sqrtl((long double)x), logl((long double)x));
    }
    printf("%f %f %f %f\n", floor(-2.5), ceil(-2.5), round(-2.5), trunc(-2.5));
    printf("%f %f %f\n", fabs(-3.0), fmod(10.5, 3.0), remainder(10.5, 3.0));
    printf("%f %f\n", atan2(1.0, -1.0), hypot(3.0, 4.0));
    printf("%f %f %f\n", fmin(1.0, 2.0), fmax(1.0, 2.0), copysign(3.0, -0.0));
    int e; double m = frexp(48.0, &e); printf("%f %d %f\n", m, e, ldexp(0.75, 6));
    double ip; double fp = modf(-3.75, &ip); printf("%f %f\n", fp, ip);
    printf("%ld %lld\n", lround(2.5), llround(-2.5));
    printf("%f %f\n", nearbyint(2.5), rint(3.5));
    printf("%f %f %f\n", tanh(0.5), sinh(1.0), cosh(1.0));
    printf("%f %f %f\n", asin(0.5), acos(0.5), tan(0.5));
    printf("%f %f\n", expm1(1e-5) * 1e5, log1p(1e-5) * 1e5);
    printf("%d %d\n", ilogb(1024.0), (int)logb(1000.0));
    printf("%f %f\n", fdim(5.0, 3.0), fma(2.0, 3.0, 4.0));
    printf("%f\n", erf(0.5));
    printf("%f\n", tgamma(5.0));
    printf("%f %f\n", (double)floorf(2.7f), (double)fabsf(-1.5f));
    printf("%Lf %Lf\n", floorl(-2.5L), fabsl(-7.25L));
    printf("%d %d %d %d\n", fpclassify(0.0) == FP_ZERO, fpclassify(1.0) == FP_NORMAL, fpclassify(1e-310) == FP_SUBNORMAL, fpclassify(NAN) == FP_NAN);
    printf("%d %d\n", isgreater(2.0, 1.0), isunordered(NAN, 1.0));
    printf("%.17g\n", M_PI);
    return 0;
}
