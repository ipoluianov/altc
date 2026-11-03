#include <stdio.h>

static double gd(double x) { return x * 1.5; }
static float gf(float x) { return x + 0.25f; }
static int gi(int x) { return x * 3; }
static long double gl(long double x) { return x / 2; }
static double mix8(double a, int b, double c, int d, float e, long f, double g, long double h) { return a + b + c + d + e + f + g + (double)h; }
static double sum16(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j, double k, double l, double m, double n, double o, double p) {
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h + 9 * i + 10 * j + 11 * k + 12 * l + 13 * m + 14 * n + 15 * o + 16 * p;
}
static long isum12(long a, long b, long c, long d, long e, long f, long g, long h, long i, long j, long k, long l) { return a - b + c - d + e - f + g - h + i - j + k - l; }

int main(void) {
    printf("%g\n", mix8(gd(1), gi(2), gd(gd(3)), gi(gi(4)), gf(5), (long)gi(6), gd(7) + gf(8), gl(9)));
    printf("%g\n", sum16(gd(1), gd(2), gd(3), gd(4), gd(5), gd(6), gd(7), gd(8), gd(9), gd(10), gd(11), gd(12), gd(13), gd(14), gd(15), gd(16)));
    printf("%g\n", sum16(1, gd(2), 3, gd(4), 5, gd(6), 7, gd(8), 9, gd(10), 11, gd(12), 13, gd(14), 15, sum16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1)));
    printf("%ld\n", isum12(gi(1), gi(2), gi(3), gi(4), gi(5), gi(6), gi(7), gi(8), gi(9), gi(10), gi(11), isum12(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)));
    double a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16, q = 17, r = 18;
    double big = ((a + b) * (c + d) - (e + f) * (g + h)) / ((i - j) * (k - l) + (m * n - o * p)) + (q / r) * ((a * b + c * d) / (e * f + g * h) - (i * j + k * l) / (m * n + o * p)) + gd(a + gd(b + gd(c + gd(d))));
    printf("%.15g\n", big);
    long x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5, x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10, x11 = 11, x12 = 12, x13 = 13, x14 = 14, x15 = 15, x16 = 16;
    long ib = ((x1 + x2) * (x3 + x4) - (x5 + x6) * (x7 + x8)) * ((x9 - x10) * (x11 - x12) + (x13 * x14 - x15 * x16)) + gi((int)(x1 + gi((int)(x2 + gi((int)x3)))));
    printf("%ld\n", ib);
    long acc = (x1 ? x2 : x3) + (x4 > x5 ? x6 : x7) * (x8 < x9 ? (x10 ? x11 : x12) : x13) - (x14 && x15 || x16);
    printf("%ld\n", acc);
    float fa[8]; for (int t = 0; t < 8; t++) fa[t] = gf((float)t);
    printf("%g\n", (double)(fa[0] * fa[1] + fa[2] * fa[3] - fa[4] / fa[5] + fa[6] * gf(fa[7])));
    long double la = gl(10) + gl(gl(20)) * gl(3) - (long double)gd(2) / gl(8); printf("%.12Lg\n", la);
    return 0;
}
