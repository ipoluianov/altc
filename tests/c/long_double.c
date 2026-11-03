#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
typedef long double LD;
static LD sum_many(LD a, LD b, LD c, LD d, LD e, LD f, LD g, LD h, LD i, LD j) { return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10; }
static LD mix(int a, LD b, double c, LD d, float e, long f, LD g) { return a + b + c + d + e + f + g; }
static LD vsum(int n, ...) { va_list ap; va_start(ap, n); LD s = 0; for (int i = 0; i < n; i++) { s += va_arg(ap, double); s += va_arg(ap, LD); } va_end(ap); return s; }
static LD (*ldfp)(LD, LD, LD, LD, LD, LD, LD, LD, LD, LD) = sum_many;
struct LS { char c; LD v; short s; };
static struct LS mkls(LD v) { struct LS r = {'q', v, 7}; return r; }
int main(void) {
    LD x = 1.0L / 3.0L, y = 2.0L / 3.0L;
    printf("%.20Lg %.20Lg %.20Lg %.20Lg\n", x + y, x * y, x - y, x / y);
    printf("%d %d %d %d %d %d\n", x < y, x > y, x == y, x != y, x + x == y, x * 3 == 1);
    printf("%.20Lg\n", sum_many(1, 2, 3, 4, 5, 6, 7, 8, 9, 10.5L));
    printf("%.20Lg\n", ldfp(0.1L, 0.2L, 0.3L, 0.4L, 0.5L, 0.6L, 0.7L, 0.8L, 0.9L, 1.0L));
    printf("%.20Lg\n", mix(1, 2.5L, 3.25, 4.125L, 5.5f, 6, 7.0625L));
    printf("%.20Lg\n", vsum(10, 1.0, 1.0L, 2.0, 2.0L, 3.0, 3.0L, 4.0, 4.0L, 5.0, 5.0L, 6.0, 6.0L, 7.0, 7.0L, 8.0, 8.0L, 9.0, 9.0L, 10.0, 10.0L));
    struct LS ls = mkls(x); printf("%c %.20Lg %d %zu\n", ls.c, ls.v, ls.s, sizeof ls);
    LD arr[4] = {1.5L, -2.5L, LDBL_MAX, LDBL_MIN}; LD m = 0; for (int i = 0; i < 4; i++) if (arr[i] > m) m = arr[i]; printf("%Lg\n", m);
    unsigned long long u = 0x8000000000000000ULL; LD lu = u; printf("%.1Lf %llu\n", lu, (unsigned long long)lu);
    u = 0xffffffffffffffffULL; lu = u; printf("%.1Lf %llu\n", lu, (unsigned long long)lu);
    long long sl = -9223372036854775807LL - 1; LD ls2 = sl; printf("%.1Lf %lld\n", ls2, (long long)ls2);
    LD nz = -0.0L; printf("%d %Lg\n", signbit(nz) != 0, nz);
    LD nn = NAN; printf("%d %d %d\n", nn == nn, nn != nn, !!isnan(nn));
    LD big = 1e4000L; printf("%Lg %Lg\n", big, 1 / big);
    printf("%.25Lf\n", 0.1L);
    printf("%La\n", 1.0L / 3);
    LD acc = 0; for (int i = 1; i <= 100; i++) acc += 1.0L / i; printf("%.20Lg\n", acc);
    printf("%.20Lg %.20Lg\n", sqrtl(2.0L), powl(2.0L, 0.5L));
    int iv = 7; LD r = iv / 2.0L; printf("%Lg %d %ld\n", r, (int)r, (long)(r * -1));
    LD t = 3; t++; t--; ++t; printf("%Lg\n", t);
    LD cmp_a = 1e-4930L, cmp_b = 0; printf("%d\n", cmp_a > cmp_b);
    return 0;
}
