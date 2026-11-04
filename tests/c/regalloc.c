// Locals kept in registers: widths, conversions, calls, setjmp
#include <stdio.h>
#include <setjmp.h>
#include <string.h>

static unsigned long long shifts(void) {
    unsigned long long big = 0;
    for (int i = 0; i < 64; i += 3)
        big |= 1ULL << i;
    long s = 0;
    for (short k = 0; k < 40; k += 7)
        s += 1L << k;
    return big ^ (unsigned long long)s;
}

static int narrow(void) {
    signed char c = 120;
    unsigned char u = 250;
    short sh = 32000;
    unsigned short us = 65530;
    _Bool b = 0;
    for (int i = 0; i < 10; i++) {
        c++;
        u += 3;
        sh += 1000;
        us += 7;
        b = !b;
    }
    return c * 1000000 + u * 10000 + sh + us + b;
}

static long many(int a, int b, int c, int d, int e, int f, int g, long h, unsigned char i) {
    long r = 0;
    for (int k = 0; k < 3; k++)
        r = r * 7 + a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9;
    return r;
}

static long sum_ptr(const int *p, int n) {
    long s = 0;
    for (int i = 0; i < n; i++)
        s += p[i] * (i & 3) + p[n - 1 - i];
    const int *q = p + n;
    while (q != p)
        s -= *--q;
    return s;
}

static int six_vars(int x) {
    int a = x, b = x + 1, c = x + 2, d = x + 3, e = x + 4, f = x + 5, g = x + 6;
    for (int i = 0; i < 4; i++) {
        a += b; b ^= c; c -= d; d *= 3; e |= f; f &= g; g += a;
    }
    return a + b + c + d + e + f + g;
}

static void bump(int *p) { (*p)++; }

static int addr_taken(void) {
    int x = 5, y = 6;
    bump(&x);
    for (int i = 0; i < 3; i++)
        y += x;
    return x * 100 + y;
}

static jmp_buf jb;
static void jump(int v) { longjmp(jb, v); }

static int with_setjmp(void) {
    volatile int count = 0;
    int r = setjmp(jb);
    count++;
    if (r < 3)
        jump(r + 1);
    return r * 10 + count;
}

struct P { int x; unsigned char tag; long y; double d; };

static double structs(struct P *p, int n) {
    double s = 0;
    for (int i = 0; i < n; i++) {
        p[i].x = i * 3;
        p[i].tag = 200 + i;
        p[i].y = -i;
        p[i].d = i * 0.5;
    }
    for (int i = 0; i < n; i++)
        s += p[i].x + p[i].tag + p[i].y + p[i].d * 2.0;
    return s;
}

static double fargs(double a, float b, double c) { return a * 2 + b + c / 4; }

static double floats(double *v, int n) {
    double acc = 0, k = 1.5;
    for (int i = 0; i < n; i++)
        acc += fargs(v[i], (float)i, k) * v[i] - k;
    return acc;
}

static unsigned mixed(unsigned u, int s, long l, unsigned long ul) {
    unsigned r = 0;
    for (int i = 0; i < 5; i++) {
        r += u >> i;
        r ^= (unsigned)(s >> i);
        l -= s * i;
        ul += (unsigned long)l * u;
        if (s < l)
            r++;
        if (u > ul)
            r += 2;
    }
    return r + (unsigned)ul;
}

__attribute__((noinline)) static double scale(double v) { return v * 1.25; }

// float and double variables kept in xmm registers across calls
static double xmm_vars(float a, double b, float c) {
    float x = a, y = c;
    double z = b, w = 0;
    for (int i = 0; i < 20; i++) {
        w += x * y + z;
        x = (float)scale(x) - 1;
        z = z * 0.5 + scale(w) / 100;
        y += 0.25f;
    }
    return w + x + y + z;
}

static const double table[4] = {0.5, 1.5, 2.5, 3.5};
static double consts(int n) {
    double s = 0;
    float f = 3;
    for (int i = 0; i < n; i++)
        s += table[i & 3] * 2 + (float)7 + f;
    return s + (double)18446744073709551615ULL;
}

int main(void) {
    printf("shifts %llx\n", shifts());
    printf("narrow %d\n", narrow());
    printf("many %ld\n", many(1, -2, 3, -4, 5, -6, 7, -8, 250));
    int arr[10];
    for (int i = 0; i < 10; i++)
        arr[i] = i * i - 20;
    printf("ptr %ld\n", sum_ptr(arr, 10));
    printf("six %d\n", six_vars(3));
    printf("addr %d\n", addr_taken());
    printf("setjmp %d\n", with_setjmp());
    struct P ps[4];
    memset(ps, 0, sizeof ps);
    printf("structs %.2f\n", structs(ps, 4));
    double v[5] = {1, -2, 3.5, 4, 0.25};
    printf("floats %.4f\n", floats(v, 5));
    printf("mixed %u %u\n", mixed(0xdeadbeefu, -12345, 99, 7), mixed(3, 77, -5, 1ul << 40));
    printf("xmm %.6f\n", xmm_vars(1.5f, -2.0, 0.75f));
    printf("consts %.1f\n", consts(10));
    return 0;
}
