// long double through the IR backend: arithmetic, conversions, compares,
// calls, varargs, loops carrying values, structs holding long doubles
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct { long double v; int tag; } tagged;
typedef struct { int a; long double x, y; } pair;

long double gl = 1.25L;
long double arr[4] = {1.0L, -2.5L, 3.0e-10L, 1e300L};

static long double add3(long double a, long double b, long double c) { return a + b + c; }

static long double mixed(int i, double d, long double x, float f, long long ll, long double y, int j, long double z) {
    return i + d + x * f - ll + y / j + z;
}

static long double sum_va(int n, ...) {
    va_list ap;
    va_start(ap, n);
    long double s = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2)
            s += va_arg(ap, long double);
        else
            s += va_arg(ap, double);
    }
    va_end(ap);
    return s;
}

static long double by_struct(tagged t, pair p) { return t.v * t.tag + p.x - p.y * p.a; }

static long double modify_param(long double x) {
    x *= 2;
    x -= 1;
    return x;
}

static long double fact(int n) { return n <= 1 ? 1.0L : n * fact(n - 1); }

static long double (*fp)(long double) = modify_param;

__attribute__((noinline)) static int classify(long double x) {
    return (isnan(x) ? 1 : 0) | (isinf(x) ? 2 : 0) | (signbit(x) ? 4 : 0) | (isfinite(x) ? 8 : 0);
}

static void print(const char *name, long double v) { printf("%s = %.18Lg\n", name, v); }

int main(void) {
    long double a = 1.0L / 3, b = -7.75L, c;
    print("a", a);
    print("a+b", a + b);
    print("a-b", a - b);
    print("a*b", a * b);
    print("a/b", a / b);
    print("-a", -a);
    c = a;
    c += b;
    c *= 3;
    c /= 2;
    c -= 0.5;
    print("c", c);
    print("c++", c++);
    print("++c", ++c);
    print("c--", c--);
    print("cond", a > b ? a : b);
    print("gl", gl += 0.125L);
    for (int i = 0; i < 4; i++)
        print("arr", arr[i] * i);

    // conversions
    int8_t i8 = -5;
    uint16_t u16 = 65535;
    int i32 = -123456789;
    unsigned u32 = 4000000000u;
    long long i64 = -9000000000000000000LL;
    unsigned long long u64 = 18000000000000000000ULL;
    print("i8", i8);
    print("u16", u16);
    print("i32", i32);
    print("u32", u32);
    print("i64", i64);
    print("u64", u64);
    long double big = 17000000000000000000.0L, neg = -3.99L, frac = 2.99L;
    printf("%llu %llu %lld %d %d %u\n", (unsigned long long)big, (unsigned long long)frac, (long long)neg, (int)neg,
           (int)frac, (unsigned)frac);
    printf("%d %d %d\n", (signed char)neg, (short)-30000.5L, (_Bool)0.0L + (_Bool)0.1L);
    printf("%.17g %.9g\n", (double)a, (float)a);
    float f = 1.1f;
    double d = 2.2;
    print("f", f);
    print("d", d);
    print("f+d+ld", f + d + a);

    // comparisons, NaN
    long double nan = NAN, inf = INFINITY, zero = 0.0L, nzero = -0.0L;
    printf("%d %d %d %d %d %d\n", a < b, a <= b, a > b, a >= b, a == b, a != b);
    printf("%d %d %d %d %d %d\n", nan < a, nan <= a, nan > a, nan >= a, nan == nan, nan != nan);
    printf("%d %d %d %d\n", zero == nzero, !zero, !a, isunordered(nan, a));
    printf("%d %d %d %d %d\n", classify(nan), classify(inf), classify(-inf), classify(nzero), classify(a));
    if (a)
        puts("a true");
    if (!zero && (a || zero))
        puts("logic ok");

    // calls
    print("add3", add3(a, b, 1));
    print("mixed", mixed(1, 2.5, a, 0.5f, 3, b, 4, 1e-5L));
    print("sum_va", sum_va(5, 1.5, 2.25L, 3.0, 4.125L, 5.0));
    tagged t = {2.5L, 3};
    pair p = {2, 10.0L, 0.25L};
    print("by_struct", by_struct(t, p));
    long double keep = 5.5L;
    print("modify_param", modify_param(keep));
    print("keep", keep);
    print("fp", fp(1.5L));
    print("fact", fact(25));
    printf("%Lf %Le %La\n", a, b, 1.0L);

    // loops carrying long double values
    long double s = 0, prev = 1, cur = 1;
    for (int i = 1; i <= 50; i++) {
        s += 1.0L / i;
        long double next = prev + cur;
        prev = cur;
        cur = next;
    }
    print("harmonic", s);
    print("fib", cur);
    long double x = 2;
    for (int i = 0; i < 6; i++)
        x = x - (x * x - 2) / (2 * x); // Newton for sqrt(2)
    print("sqrt2", x);
    long double m = arr[0];
    for (int i = 1; i < 4; i++)
        if (arr[i] > m)
            m = arr[i];
    print("max", m);
    long double w = ({ long double q = a; q * q; });
    print("stmtexpr", w);
    long double buf[3];
    memset(buf, 0, sizeof buf);
    buf[1] = a;
    buf[2] = buf[1] + buf[0];
    print("buf", buf[2]);
    return 0;
}
