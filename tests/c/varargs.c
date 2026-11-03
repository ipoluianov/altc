#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct { int a, b; } P2;
typedef struct { double x, y; } V2;
typedef struct { long a, b, c; } Big;
typedef struct { float f; int i; } FI;

long sum_ints(int n, ...) { va_list ap; va_start(ap, n); long s = 0; for (int i = 0; i < n; i++) s += va_arg(ap, int); va_end(ap); return s; }
double sum_d(int n, ...) { va_list ap; va_start(ap, n); double s = 0; for (int i = 0; i < n; i++) s += va_arg(ap, double); va_end(ap); return s; }
long double sum_ld(int n, ...) { va_list ap; va_start(ap, n); long double s = 0; for (int i = 0; i < n; i++) s += va_arg(ap, long double); va_end(ap); return s; }
void typed(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    for (const char *p = fmt; *p; p++) {
        switch (*p) {
        case 'i': printf("i=%d ", va_arg(ap, int)); break;
        case 'l': printf("l=%ld ", va_arg(ap, long)); break;
        case 'u': printf("u=%llu ", va_arg(ap, unsigned long long)); break;
        case 'd': printf("d=%g ", va_arg(ap, double)); break;
        case 'L': printf("L=%Lg ", va_arg(ap, long double)); break;
        case 's': printf("s=%s ", va_arg(ap, char *)); break;
        case 'p': { P2 v = va_arg(ap, P2); printf("P2=%d,%d ", v.a, v.b); break; }
        case 'v': { V2 v = va_arg(ap, V2); printf("V2=%g,%g ", v.x, v.y); break; }
        case 'b': { Big v = va_arg(ap, Big); printf("Big=%ld,%ld,%ld ", v.a, v.b, v.c); break; }
        case 'f': { FI v = va_arg(ap, FI); printf("FI=%g,%d ", v.f, v.i); break; }
        case 'c': printf("c=%c ", va_arg(ap, int)); break;
        case '*': { int *ip = va_arg(ap, int *); printf("*=%d ", *ip); break; }
        }
    }
    va_end(ap); printf("\n");
}
void twice(int n, ...) {
    va_list ap, ap2; va_start(ap, n); va_copy(ap2, ap);
    long s1 = 0; for (int i = 0; i < n; i++) s1 += va_arg(ap, long);
    double s2 = 0; for (int i = 0; i < n; i++) s2 += va_arg(ap2, long) * 0.5;
    va_end(ap); va_end(ap2); printf("%ld %g\n", s1, s2);
}
int my_printf(const char *fmt, ...) { va_list ap; va_start(ap, fmt); int r = vprintf(fmt, ap); va_end(ap); return r; }
char *fmt_buf(char *buf, size_t n, const char *fmt, ...) { va_list ap; va_start(ap, fmt); vsnprintf(buf, n, fmt, ap); va_end(ap); return buf; }
static void vinner(int depth, va_list ap) { int x = va_arg(ap, int); double d = va_arg(ap, double); printf("inner %d %d %g\n", depth, x, d); }
static void outer(int depth, ...) { va_list ap; va_start(ap, depth); vinner(depth, ap); int y = va_arg(ap, int); va_end(ap); printf("after %d\n", y); }
static int count_until_zero(int first, ...) { va_list ap; int c = 0; va_start(ap, first); for (int v = first; v; v = va_arg(ap, int)) c++; va_end(ap); return c; }
static double mixed(int a, double b, ...) { va_list ap; va_start(ap, b); double s = a + b; for (int i = 0; i < 10; i++) { s += va_arg(ap, int); s += va_arg(ap, double); } va_end(ap); return s; }
static float ret_float_var(int n, ...) { va_list ap; va_start(ap, n); double d = va_arg(ap, double); va_end(ap); return (float)d * n; }

int main(void) {
    printf("%ld\n", sum_ints(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    printf("%ld\n", sum_ints(3, -1, (char)-2, (short)-3));
    printf("%g\n", sum_d(12, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 0.5f));
    printf("%Lg\n", sum_ld(3, 1.0L, 2.5L, 1e10L));
    int k = 77;
    typed("ilud", 1, -2L, 18446744073709551615ULL, 3.5);
    typed("Lsdi", 2.25L, "str", 1e-3, 'x');
    typed("pvbf", (P2){3, 4}, (V2){1.5, 2.5}, (Big){7, 8, 9}, (FI){0.5f, 6});
    typed("iiiiiiddddddddddpvbfL*", 1, 2, 3, 4, 5, 6, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
          (P2){3, 4}, (V2){1.5, 2.5}, (Big){7, 8, 9}, (FI){0.5f, 6}, 3.0L, &k);
    twice(4, 10L, 20L, 30L, 40L);
    twice(8, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L);
    my_printf("%d-%s-%.2f-%c-%lld\n", 5, "five", 5.5, 'F', 5555555555LL);
    char buf[64]; printf("[%s]\n", fmt_buf(buf, sizeof buf, "%05d|%-6s|%x", 42, "ab", 255));
    printf("[%s]\n", fmt_buf(buf, 8, "%s", "truncated string"));
    outer(1, 10, 2.5, 20);
    printf("%d\n", count_until_zero(1, 2, 3, 4, 5, 6, 7, 8, 9, 0));
    printf("%g\n", mixed(1, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5));
    printf("%g\n", ret_float_var(3, 1.5));
    char out[100]; int n = snprintf(out, sizeof out, "%s %d %g %Lg", "x", 1, 2.0, 3.0L); printf("%d %s\n", n, out);
    printf("%d\n", snprintf(NULL, 0, "%d", 123456));
    fprintf(stderr, "to stderr %d\n", 1); fflush(stderr);
    return 0;
}
