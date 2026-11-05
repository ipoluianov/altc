// 128-bit integers through the IR backend: arithmetic, shifts by constant
// and variable counts, comparisons, conversions, calls (registers, stack,
// varargs), structs, loops carrying values, switch
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

typedef __int128 i128;
typedef unsigned __int128 u128;

static void put(const char *name, u128 v) {
    printf("%s = %016llx%016llx\n", name, (unsigned long long)(v >> 64), (unsigned long long)v);
}

static void putd(const char *name, i128 v) {
    char buf[64], *p = buf + sizeof buf - 1;
    int neg = v < 0;
    u128 u = neg ? -(u128)v : (u128)v;
    *p = 0;
    do
        *--p = '0' + (int)(u % 10);
    while (u /= 10);
    if (neg)
        *--p = '-';
    printf("%s = %s\n", name, p);
}

i128 gs = -12345678901234567LL;
u128 gu = ~(u128)0 / 3;
struct rec { char c; i128 v; int tag; u128 w; } grec = {1, 42, 7, 99};

static const u128 k100 = (u128)1 << 100;
i128 garr[] = {-1, (i128)(~(u128)0 >> 1), (i128)((u128)1 << 127), 0x7fffffffffffffffLL * (i128)0x100000001LL,
               (i128)18446744073709551615ULL * 3 / 7, -(i128)1e30, (i128)2.5e37L, (u128)-1 % 1000000007};

static i128 add3(i128 a, i128 b, i128 c) { return a + b + c; }

// the 128-bit arguments that no longer fit in registers go to the stack
static i128 many(int a, i128 b, long c, i128 d, i128 e, int f, i128 g) {
    return a + b * 3 - c + d - e * f + g;
}

static u128 sum_va(int n, ...) {
    va_list ap;
    va_start(ap, n);
    u128 s = 0;
    for (int i = 0; i < n; i++)
        s += i % 2 ? va_arg(ap, u128) : (u128)va_arg(ap, long);
    va_end(ap);
    return s;
}

static inline u128 mul64(uint64_t a, uint64_t b) { return (u128)a * b; }

__attribute__((noinline)) static u128 shl(u128 v, int n) { return v << n; }
__attribute__((noinline)) static u128 shr(u128 v, int n) { return v >> n; }
__attribute__((noinline)) static i128 sar(i128 v, int n) { return v >> n; }
__attribute__((noinline)) static i128 ident(i128 v) { return v; }

static int classify(i128 v) {
    switch (v) {
    case 0:
        return 0;
    case -1:
        return 1;
    case 1000000000000LL:
        return 2;
    default:
        return v < 0 ? 3 : 4;
    }
}

static i128 (*fp)(i128, i128, i128) = add3;

// division of random values of every magnitude (the runtime's divq paths)
static uint64_t rs = 88172645463325252ULL;
static uint64_t rnd(void) {
    rs ^= rs << 13;
    rs ^= rs >> 7;
    rs ^= rs << 17;
    return rs;
}

static u128 r128(void) {
    uint64_t r1 = rnd(), r2 = rnd(), r3 = rnd(), r4 = rnd();
    u128 v = ((u128)r2 << 64) | r3;
    switch (r1 % 6) {
    case 0:
        return v >> (r4 % 128);
    case 1:
        return r2;
    case 2:
        return r2 >> (r4 % 64);
    case 3:
        return v;
    case 4:
        return ~(u128)0 - (r4 % 3);
    }
    return ((u128)1 << (r4 % 128)) + (r2 % 3) - 1;
}

static void divisions(void) {
    u128 h = 0;
    int bad = 0;
    for (int i = 0; i < 200000; i++) {
        u128 a = r128();
        u128 b = r128();
        if (b == 0)
            b = 1;
        u128 q = a / b, r = a % b;
        if (q * b + r != a || r >= b)
            bad++;
        h = h * 31 + q + (r << 1);
        i128 sa = (i128)a, sb = (i128)b;
        if (sb == -1 && sa == (i128)((u128)1 << 127))
            continue;
        h ^= (u128)(sa / sb) + (u128)(sa % sb);
    }
    printf("divisions: %d bad, ", bad);
    put("hash", h);
}

int main(void) {
    i128 a = ident((i128)0x123456789abcdefLL << 40), b = ident(-987654321987654321LL);
    u128 ua = (u128)a, ub = (u128)b;
    put("a", a);
    put("b", b);
    put("a+b", a + b);
    put("a-b", a - b);
    put("b-a", b - a);
    put("a*b", a * b);
    put("ua*ub", ua * ub);
    put("a/b", a / b);
    put("a%b", a % b);
    put("ua/ub", ua / ub);
    put("ua%ub", ua % ub);
    put("a&b", a & b);
    put("a|b", a | b);
    put("a^b", a ^ b);
    put("~a", ~a);
    put("-a", -a);
    put("-b", -b);
    putd("a", a);
    putd("b", b);
    putd("a*b/7", a * b / 7);
    printf("%d %d %d %d %d %d\n", a < b, a <= b, a > b, a >= b, a == b, a != b);
    printf("%d %d %d %d %d %d\n", ua < ub, ua <= ub, ua > ub, ua >= ub, ua == ub, ua != ub);
    printf("%d %d %d\n", !a, !(a - a), a && b);
    if (a > b)
        puts("a > b");
    if (ua < ub)
        puts("ua < ub");
    // carries through the halves
    u128 m = ~(u128)0 >> 64;
    put("m+1", m + 1);
    put("0-1", (u128)0 - 1);
    put("carry", (u128)0xffffffffffffffffULL + 1);
    put("borrow", ((u128)1 << 64) - 1);
    // shifts
    for (int n = 0; n < 128; n += 9) {
        printf("%3d: ", n);
        put("", shl(gu, n) ^ shr(gu, n) ^ (u128)sar(b, n));
    }
    int counts[] = {0, 1, 63, 64, 65, 127};
    for (int i = 0; i < 6; i++) {
        int n = counts[i];
        put("shl", shl(ua, n));
        put("shr", shr(ua, n));
        put("sar", sar(b, n));
    }
    put("c<<1", a << 1);
    put("c<<63", a << 63);
    put("c<<64", a << 64);
    put("c<<100", a << 100);
    put("c>>3", a >> 3);
    put("c>>64", b >> 64);
    put("c>>70", b >> 70);
    put("u>>70", ub >> 70);
    put("u>>127", ub >> 127);
    // conversions
    signed char sc = -5;
    unsigned short us = 65535;
    int si = -123456789;
    unsigned ui = 4000000000u;
    long long sl = -9000000000000000000LL;
    unsigned long long ul = 18000000000000000000ULL;
    put("sc", sc);
    put("us", us);
    put("si", si);
    put("ui", ui);
    put("sl", sl);
    put("ul", ul);
    i128 big = (i128)ul * 1000 + 7;
    printf("%d %u %lld %llu %d\n", (int)big, (unsigned)big, (long long)big, (unsigned long long)(big >> 3), (signed char)big);
    printf("%d %d\n", (_Bool)big, (_Bool)(big - big));
    double d = (double)big;
    float f = (float)b;
    long double ld = (long double)ub;
    printf("%.17g %.9g %.21Lg\n", d, f, ld);
    printf("%.17g %.21Lg\n", (double)ub, (long double)a);
    put("from double", (i128)-1.5e30);
    put("from float", (u128)3.0e25f);
    put("from ld", (i128)-12345678901234567890.0L);
    put("from uld", (u128)1.7e38L);
    // calls
    putd("add3", add3(a, b, 1));
    putd("fp", fp(a, -a, 5));
    putd("many", many(1, a, 2, b, 3, 4, a));
    put("sum_va", sum_va(5, 1L, ~(u128)0, 3L, (u128)1 << 100, -1L));
    put("mul64", mul64(0xfedcba9876543210ULL, 0x123456789abcdefULL));
    // memory
    i128 arr[5];
    for (int i = 0; i < 5; i++)
        arr[i] = (i128)i << (30 * i);
    put("arr", arr[0] + arr[1] + arr[2] + arr[3] + arr[4]);
    i128 *p = &arr[2];
    *p += 1;
    p[1] -= p[2];
    put("arr2", arr[2] ^ arr[3]);
    grec.v *= -3;
    grec.w = grec.v + grec.tag;
    putd("grec", grec.v + grec.w);
    gs++;
    gu--;
    putd("gs", gs);
    put("gu", gu);
    volatile i128 vv = 5;
    vv += vv;
    putd("vv", vv);
    // loops carrying values
    u128 fib0 = 0, fib1 = 1;
    for (int i = 0; i < 180; i++) {
        u128 t = fib0 + fib1;
        fib0 = fib1;
        fib1 = t;
    }
    put("fib", fib1);
    i128 fact = 1;
    for (int i = 1; i <= 33; i++)
        fact *= i;
    putd("33!", fact);
    u128 h = 14695981039346656037ULL;
    for (int i = 0; i < 100; i++)
        h = (h ^ (u128)i) * (((u128)1 << 88) + 0x13b);
    put("hash", h);
    i128 x = 1;
    int steps = 0;
    while (x < ((i128)1 << 120)) {
        x = x * 3 + (x >> 2);
        steps++;
    }
    printf("steps %d\n", steps);
    putd("x", x);
    // conditional values and switch
    i128 c = a > 0 ? a : -a;
    putd("cond", c);
    i128 vals[] = {0, -1, 1000000000000LL, -5, 5, (i128)1 << 64};
    for (int i = 0; i < 6; i++)
        printf("%d", classify(vals[i]));
    printf("\n");
    i128 w = ({ i128 q = b; q * q; });
    putd("stmtexpr", w);
    divisions();
    return 0;
}
