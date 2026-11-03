#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

static int aligned16(void *p) { return ((uintptr_t)p & 15) == 0; }
static void after_vla(int n) { char v[n]; memset(v, 'a', n); double d = n * 0.5; long double ld = n; printf("vla %d %c %.2f %.2Lf\n", n, v[n - 1], d, ld); }
static void after_alloca(int n) { char *p = alloca(n); memset(p, 'b', n); printf("alloca %d %c %g %d\n", n, p[n - 1], n / 3.0, aligned16(alloca(1))); }
static void odd_locals(char a, short b, char c) { char x = a; short y = b; char z = c; printf("odd %c %d %c %f\n", x, y, z, (double)y); }
static double var_avg(int n, ...) { va_list ap; va_start(ap, n); double s = 0; for (int i = 0; i < n; i++) s += va_arg(ap, double); va_end(ap); char buf[64]; snprintf(buf, sizeof buf, "%.3f", s / n); printf("avg %s\n", buf); return s / n; }
static void recurse(int depth, int odd) { char pad[odd]; memset(pad, 0, odd); if (depth) { recurse(depth - 1, odd + 1); return; } printf("deep %d %.1f %Lg\n", odd, 1.5, 2.5L); }
struct big { long double x; char c; };
static struct big mkbig(int k) { struct big b = {k * 1.5L, (char)('a' + k)}; return b; }
static void stack_args(int a, int b, int c, int d, int e, int f, int g, double h, struct big i, char j) {
    long double alignedv = i.x; printf("stack %d %d %g %Lg %c %c %d\n", a + b + c + d + e + f, g, h, alignedv, i.c, j, aligned16(&alignedv));
}
_Alignas(32) static char g32[3];
int main(void) {
    for (int n = 1; n < 6; n++) after_vla(n);
    for (int n = 1; n < 6; n++) after_alloca(n * 7);
    odd_locals('x', -3, 'z');
    var_avg(3, 1.0, 2.0, 4.0);
    var_avg(9, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.5);
    for (int o = 1; o < 4; o++) recurse(5, o);
    struct big b = mkbig(3); printf("%Lg %c\n", b.x, b.c);
    stack_args(1, 2, 3, 4, 5, 6, 7, 8.5, mkbig(2), 'q');
    long double lds[3] = {1, 2, 3}; printf("%d %d\n", aligned16(&lds[0]), aligned16(&lds[1]));
    _Alignas(16) char c16 = 1; _Alignas(64) int i64 = 2; printf("%d %d %d\n", aligned16(&c16), ((uintptr_t)&i64 & 63) == 0, ((uintptr_t)g32 & 31) == 0);
    return 0;
}
