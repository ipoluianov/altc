// loop vectorization: element-wise loops, integer reductions, overlapping arrays
#include <stdio.h>
__attribute__((noinline)) void saxpy(float *y, const float *x, float a, int n) { for (int i = 0; i < n; i++) y[i] = a * x[i] + y[i]; }
__attribute__((noinline)) void vadd(double *c, const double *a, const double *b, long n) { for (long i = 0; i < n; i++) c[i] = a[i] + b[i] * 2.0; }
__attribute__((noinline)) long isum(const int *a, int n) { int s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
__attribute__((noinline)) long lsum(const long *a, unsigned long n) { long s = 7; for (unsigned long i = 0; i < n; i++) s += a[i] ^ 3; return s; }
__attribute__((noinline)) void imap(int *c, const int *a, int k, int n) { for (int i = 0; i < n; i++) c[i] = (a[i] * k + 1) >> 2; }
__attribute__((noinline)) unsigned xr(const unsigned *a, int n) { unsigned x = 0; for (int i = 0; i < n; i++) x ^= a[i] << 1; return x; }
__attribute__((noinline)) void inplace(float *a, int n) { for (int i = 0; i < n; i++) a[i] = -a[i] * 0.5f; }
__attribute__((noinline)) void cvt(float *f, const int *a, int n) { for (int i = 0; i < n; i++) f[i] = (float)a[i] / 3.0f; }
__attribute__((noinline)) int range(int *c, const int *a, int s, int n) { int t = 0; for (int i = s; i < n; i++) { c[i] = a[i] + i; t += a[i]; } return t; }
int main(void) {
    static float x[1003], y[1003]; static double a[517], b[517], c[517]; static int ia[1001], ic[1001]; static long la[333]; static unsigned ua[77];
    for (int i = 0; i < 1003; i++) { x[i] = i * 0.25f; y[i] = 1000 - i; }
    for (int i = 0; i < 517; i++) { a[i] = i; b[i] = i * 0.5; }
    for (int i = 0; i < 1001; i++) ia[i] = i * 3 - 500;
    for (int i = 0; i < 333; i++) la[i] = (long)i * 1000003;
    for (int i = 0; i < 77; i++) ua[i] = i * 2654435761u;
    double h = 0;
    for (int n = 0; n < 40; n++) {
        saxpy(y, x, 0.5f, 1003 - n); vadd(c, a, b, 517 - n);
        h += isum(ia, 1001 - n) + lsum(la, 333 - n) + xr(ua, 77 - n % 20);
        imap(ic, ia, 3, 1001 - n);
        inplace(x, 100 + n);
        cvt(y, ia, 50 + n);
    }
    for (int i = 0; i < 1003; i += 7) h += y[i] + x[i];
    for (int i = 0; i < 517; i += 5) h += c[i];
    for (int i = 0; i < 1001; i += 3) h += ic[i];
    /* overlapping arrays: the run-time check must fall back */
    for (int i = 0; i < 1003; i++) x[i] = i;
    saxpy(x + 1, x, 1.0f, 500);
    for (int i = 0; i < 600; i += 9) h += x[i];
    /* misaligned arrays and short loops: the peeled iterations */
    static int pa[200], pc[200];
    for (int off = 0; off < 9; off++)
        for (int n = 0; n < 45; n++) {
            for (int i = 0; i < 200; i++) { x[i] = i * 0.5f; y[i] = 3 - i; pa[i] = i * 7 - 300; }
            saxpy(y + off, x + (off * 3) % 8, 2.0f, n);
            h += isum(pa + off, n) + xr(ua + off % 5, n % 70);
            imap(pc + (off + 1) % 9, pa + off, 5, n);
            h += range(pc, pa, off, n + off);
            for (int i = 0; i < 60; i++) h += y[i] * (i + 1) + pc[i] * 0.001;
        }
    printf("%.6f\n", h);
    return 0;
}
