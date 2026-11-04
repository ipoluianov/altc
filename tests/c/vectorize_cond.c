// if-converted loop vectorization: ?:, if/else, && and ||, conditional reductions
#include <stdio.h>
__attribute__((noinline)) void relu(float *c, const float *a, int n) { for (int i = 0; i < n; i++) c[i] = a[i] > 0 ? a[i] : 0; }
__attribute__((noinline)) void clampi(int *c, const int *a, int n) { for (int i = 0; i < n; i++) { int x = a[i]; if (x > 100) x = 100; else if (x < -50) x = -50; c[i] = x; } }
__attribute__((noinline)) int cnt(const int *a, int n) { int s = 0; for (int i = 0; i < n; i++) if (a[i] > 3 && a[i] != 10) s += a[i]; return s; }
__attribute__((noinline)) unsigned umax(unsigned *c, const unsigned *a, const unsigned *b, int n) { unsigned x = 0; for (int i = 0; i < n; i++) { c[i] = a[i] < b[i] ? b[i] : a[i]; if (a[i] >= 5u || b[i] == 2) x ^= a[i]; } return x; }
__attribute__((noinline)) void sel64(long *c, const long *a, long k, int n) { for (int i = 0; i < n; i++) c[i] = a[i] <= k ? a[i] * 2 : k - a[i]; }
__attribute__((noinline)) double dsel(double *c, const double *a, int n) { double s = 0; for (int i = 0; i < n; i++) { double v = a[i]; c[i] = (v < 1.5 || v >= 7) ? -v : v * 0.5; } return s; }
int main(void) {
    static float f[300], g[300]; static int ia[300], ic[300]; static unsigned ua[300], ub[300], uc[300]; static long la[300], lc[300]; static double da[300], dc[300];
    for (int i = 0; i < 300; i++) { f[i] = (i % 7) - 3; ia[i] = (i * 37) % 300 - 100; ua[i] = i * 2654435761u; ub[i] = (i % 9) * 0x30000000u; la[i] = (long)(i % 13) * 1000000007L - 6000000000L; da[i] = (i % 10) * 0.75; }
    long h = 0;
    for (int n = 0; n < 60; n += 7) {
        relu(g, f + n % 3, 200 + n); clampi(ic, ia + n % 5, 250 - n); h += cnt(ia, 200 + n); h += umax(uc, ua + 1, ub, 230 + n);
        sel64(lc, la + n % 2, 5000000000L, 220); dsel(dc, da + 1, 210 + n);
        for (int i = 0; i < 300; i++) h += (long)(g[i] * 4) + ic[i] * 3 + uc[i] % 1000 + lc[i] % 100000 + (long)(dc[i] * 8);
    }
    printf("%ld\n", h);
    return 0;
}

#include <stdio.h>
