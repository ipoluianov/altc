#include <stdio.h>
#include <complex.h>
static double complex g = 1.0 + 2.0 * I;
static float complex gf = 3.0f - 1.0if;
double complex mul(double complex a, double complex b) { return a * b; }
long double complex ldf(long double complex a) { return a * 2 + 1; }
int main(void) {
    double complex z = 3.0 + 4.0 * I;
    double complex w = mul(z, g);
    printf("%.3f %.3f\n", creal(w), cimag(w));
    printf("abs=%.3f arg=%.4f\n", cabs(z), carg(z));
    double complex q = z / g;
    printf("div %.6f %.6f\n", creal(q), cimag(q));
    double complex e = cexp(I * 3.14159265358979323846);
    printf("exp %.6f %.6f\n", creal(e), cimag(e));
    float complex f = gf * 2;
    printf("f %.2f %.2f %zu %zu\n", crealf(f), cimagf(f), sizeof f, sizeof z);
    long double complex l = ldf(1.5L + 2.5L * I);
    printf("ld %.2Lf %.2Lf\n", creall(l), cimagl(l));
    z += 1; z *= I; printf("%.1f %.1f %d %d\n", creal(z), cimag(z), z == z, z != 0);
    double complex c = conj(z); printf("%.1f %.1f\n", creal(c), __imag__ c);
    double complex s = csqrt(-4.0); printf("sqrt %.1f %.1f\n", creal(s), cimag(s));
    if (z) puts("nonzero");
    return 0;
}
