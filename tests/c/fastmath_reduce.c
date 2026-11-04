// flags: -ffast-math
// floating-point reductions vectorized with -ffast-math (exact data), subtraction reductions
#include <stdio.h>
__attribute__((noinline)) float fsum(const float *a, int n) { float s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
__attribute__((noinline)) double dot(const double *a, const double *b, long n) { double s = 1.5; for (long i = 0; i < n; i++) s += a[i] * b[i]; return s; }
__attribute__((noinline)) float fprod(const float *a, int n) { float p = 2; for (int i = 0; i < n; i++) p *= a[i]; return p; }
__attribute__((noinline)) double dsub(const double *a, int n) { double s = 100; for (int i = 0; i < n; i++) s -= a[i]; return s; }
__attribute__((noinline)) int isub(const int *a, int n) { int s = 7; for (int i = 0; i < n; i++) s -= a[i]; return s; }
int main(void) {
    static float f[1000]; static double d[1000], e[1000]; static int k[1000];
    for (int i = 0; i < 1000; i++) { f[i] = (i % 17) * 0.25f; d[i] = i * 0.5; e[i] = (i % 5) - 2; k[i] = i * 3 - 7; }
    static float g[64]; for (int i = 0; i < 64; i++) g[i] = (i % 3 == 0) ? 2.0f : (i % 3 == 1) ? 0.5f : 1.0f;
    for (int n = 0; n < 40; n += 3)
        printf("%d %.3f %.3f %.5g %.3f %d\n", n, fsum(f + n % 5, 900 + n), dot(d + 1, e, 700 + n), fprod(g + n % 4, 20 + n % 30), dsub(d + n % 3, 500 + n), isub(k + 1, 900 + n));
    return 0;
}
