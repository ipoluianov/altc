#include <stdio.h>
#include <stdint.h>

static double ds[] = {0.0, -0.0, 0.5, 0.99, 1.0, -1.5, 127.9, 255.5, -128.7, 65535.9, 2147483647.0,
    -2147483648.0, 4294967295.0, 1e15, -1e15, 9.2233720368547e18, 1.8446744073709e19, 123456.789};

int main(void) {
    for (unsigned k = 0; k < sizeof ds / sizeof ds[0]; k++) {
        double d = ds[k];
        long long ll = (d < 9.2e18 && d > -9.2e18) ? (long long)d : 0;
        unsigned long long ull = (d >= 0 && d < 1.8446744073709552e19) ? (unsigned long long)d : 0;
        unsigned u = (d >= 0 && d < 4294967296.0) ? (unsigned)d : 0;
        int i = (d > -2147483649.0 && d < 2147483648.0) ? (int)d : 0;
        short s = (d > -32769 && d < 32768) ? (short)d : 0;
        unsigned short us = (d > -1 && d < 65536) ? (unsigned short)d : 0;
        signed char sc = (d > -129 && d < 128) ? (signed char)d : 0;
        unsigned char uc = (d > -1 && d < 256) ? (unsigned char)d : 0;
        float f = d; long double ld = d;
        printf("%g: %lld %llu %u %d %d %u %d %u | %.9g %.21Lg\n", d, ll, ull, u, i, s, us, sc, uc, f, ld);
        float f2 = (float)d;
        unsigned long long uf = (f2 >= 0 && f2 < 1.8e19f) ? (unsigned long long)f2 : 0;
        long long lf = (f2 > -9.2e18f && f2 < 9.2e18f) ? (long long)f2 : 0;
        unsigned long long ul = (ld >= 0 && ld < 1.8446744073709552e19L) ? (unsigned long long)ld : 0;
        long long lld = (ld > -9.2e18L && ld < 9.2e18L) ? (long long)ld : 0;
        printf("   %llu %lld %llu %lld\n", uf, lf, ul, lld);
    }
    static unsigned long long us[] = {0, 1, 0x7fffffffffffffffULL, 0x8000000000000000ULL, 0x8000000000000001ULL,
        0xffffffffffffffffULL, 0xfffffffffffff800ULL, 0xfffffffffffffbffULL, 12345678901234567890ULL, 1ULL << 53, (1ULL << 53) + 1};
    for (unsigned k = 0; k < sizeof us / sizeof us[0]; k++) {
        unsigned long long v = us[k];
        double d = v; float f = v; long double ld = v;
        printf("%llu -> %.17g %.9g %.21Lg\n", v, d, f, ld);
        long long sv = (long long)v;
        d = sv; f = sv; ld = sv;
        printf("%lld -> %.17g %.9g %.21Lg\n", sv, d, f, ld);
    }
    unsigned u = 4294967295u; printf("%f %f %Lf\n", (double)u, (float)u, (long double)u);
    int i = -2147483647 - 1; printf("%f %f %Lf\n", (double)i, (float)i, (long double)i);
    unsigned char uc = 200; signed char sc = -100; unsigned short ush = 60000; short sh = -30000;
    printf("%f %f %f %f\n", (double)uc, (float)sc, (double)ush, (float)sh);
    printf("%Lf %Lf %Lf %Lf\n", (long double)uc, (long double)sc, (long double)ush, (long double)sh);
    _Bool b = 1; printf("%f\n", (double)b);
    /* long double <-> float <-> double */
    long double big = 1e300L * 1e100L; printf("%Lg %g\n", big, (double)(big / 1e200L));
    float fl = (float)1e-50; printf("%g\n", fl);
    double rd = 1.0000000000000002; float rf = rd; printf("%.10f\n", rf);
    printf("%d %d %d\n", (int)3.99f, (int)-3.99, (int)(long double)-3.99L);
    printf("%u\n", (unsigned)3000000000.0f);
    printf("%llu\n", (unsigned long long)1e19f);
    printf("%llu\n", (unsigned long long)1e19L);
    printf("%lld\n", (long long)-9.2e18L);
    return 0;
}
