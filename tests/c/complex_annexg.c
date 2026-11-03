// complex multiplication and division with infinities (C11 Annex G, libgcc helpers)
#include <stdio.h>
#include <math.h>
#include <complex.h>
static void show(const char *what, double complex z) {
    printf("%s: %s%s %s%s\n", what, isnan(creal(z)) ? "nan" : "", isinf(creal(z)) ? (creal(z) > 0 ? "inf" : "-inf") : "",
           isnan(cimag(z)) ? "nan" : "", isinf(cimag(z)) ? (cimag(z) > 0 ? "inf" : "-inf") : "");
}
int main(void) {
    volatile double inf = INFINITY, nan_ = NAN;
    double complex a = CMPLX(inf, inf), b = CMPLX(2.0, 3.0), e = CMPLX(nan_, inf);
    double complex c = CMPLX(inf, nan_), d = CMPLX(1.0, 1.0);
    show("inf*fin", a * b);
    show("infnan*fin", c * d);
    show("fin/0", b / 0.0);
    show("naninf*fin", e * b);
    show("inf*inf", a * a);
    float complex fa = CMPLXF(INFINITY, 1.0f), fb = CMPLXF(NAN, NAN);
    float complex fp = fa * fb;
    printf("f: %d %d\n", isinf(crealf(fp)) || isnan(crealf(fp)), isinf(cimagf(fp)) || isnan(cimagf(fp)));
    double complex p = (1.5 + 2.0 * I) * (3.0 - 1.0 * I);
    printf("%.2f %.2f\n", creal(p), cimag(p));
    long double complex lp = (1.5L + 2.0L * I) * (3.0L - 1.0L * I);
    printf("%.2Lf %.2Lf\n", creall(lp), cimagl(lp));
    return 0;
}
