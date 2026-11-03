#include <stdio.h>
#include <stdbool.h>

struct bf { unsigned a : 4; int b : 6; unsigned long long c : 40; };

int main(void) {
    char c = 100; c += 1.5; printf("%d\n", c);
    c = 120; c += 10; printf("%d\n", c);
    unsigned char uc = 250; uc += 10; printf("%d\n", uc);
    uc = 3; uc -= 5; printf("%d\n", uc);
    uc = 0x81; uc <<= 1; printf("%d\n", uc);
    uc = 0x81; uc >>= 1; printf("%d\n", uc);
    signed char sc = -128; sc >>= 1; printf("%d\n", sc);
    sc = -1; sc /= 2; printf("%d\n", sc);
    short s = 32767; s += 1; printf("%d\n", s);
    short s2 = -7; s2 %= 3; printf("%d\n", s2);
    unsigned short us = 1; us -= 2; printf("%d\n", us);
    us = 65535; us *= us; printf("%d\n", us);
    int i = 7; i *= 2.5; printf("%d\n", i);
    i = 10; i /= 0.3; printf("%d\n", i);
    i = -7; i >>= 1; printf("%d\n", i);
    i = 5; i <<= 3; printf("%d\n", i);
    unsigned u = 5; u -= 10; printf("%u\n", u);
    u = 100; u /= -1; printf("%u\n", u);
    long l = 1; l <<= 62; printf("%ld\n", l);
    unsigned long long ull = 1; ull <<= 63; ull >>= 3; printf("%llu\n", ull);
    float f = 1.0f; f += 1; f *= 3; f -= 0.5; f /= 2; printf("%g\n", f);
    f = 16777216.0f; f++; printf("%g\n", f);
    f = 0.5f; f++; ++f; f--; printf("%g\n", f);
    double d = 1.5; d++; ++d; d--; --d; printf("%g\n", d);
    d = 10; d *= d; printf("%g\n", d);
    long double ld = 2.5L; ld++; ld *= 2; ld -= 0.25; printf("%Lg\n", ld);
    ld = 1; ld /= 3; printf("%.18Lf\n", ld);
    bool b = false; b++; printf("%d\n", b); b++; printf("%d\n", b); b--; printf("%d\n", b); b--; printf("%d\n", b);
    b = 5; b += 1; printf("%d\n", b); b -= 1; printf("%d\n", b); b *= 0.1; printf("%d\n", b);
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; int *p = arr; p += 3; printf("%d\n", *p);
    p -= 1; printf("%d\n", *p); p++; ++p; printf("%d\n", *p); long off = -2; p += off; printf("%d\n", *p);
    unsigned uoff = 4; p += uoff; printf("%d\n", *p);
    char *cp = (char *)arr; cp += sizeof(int) * 2; printf("%d\n", *(int *)cp);
    double *dp = 0; dp++; printf("%td\n", (char *)dp - (char *)0);
    struct bf x = {15, -32, 0xffffffffffULL};
    x.a++; printf("%u\n", x.a);
    x.a--; printf("%u\n", x.a);
    x.b--; printf("%d\n", x.b);
    x.b += 70; printf("%d\n", x.b);
    x.c++; printf("%llu\n", (unsigned long long)x.c);
    x.c--; x.c >>= 4; printf("%llx\n", (unsigned long long)x.c);
    x.a = 6; x.a *= 3; printf("%u\n", x.a);
    x.a = 9; x.a %= 4; printf("%u\n", x.a);
    x.a = 5; x.a |= 10; printf("%u\n", x.a);
    x.b = 31; x.b ^= -1; printf("%d\n", x.b);
    int pre = 5, post = 5; int r1 = ++pre * 2, r2 = post++ * 2; printf("%d %d %d %d\n", r1, r2, pre, post);
    int chain_a, chain_b, chain_c; chain_a = chain_b = chain_c = 7; printf("%d %d %d\n", chain_a, chain_b, chain_c);
    char cc; int ii = (cc = 300); printf("%d %d\n", ii, cc);
    unsigned char ucc; int jj = (ucc = -1); printf("%d\n", jj);
    double dd; int kk = (dd = 2.75); printf("%d\n", kk);
    float ff; double dd2 = (ff = 0.1); printf("%.17g\n", dd2);
    long long lla = 5; lla += 0x100000000LL * 3; printf("%lld\n", lla);
    return 0;
}
