// Division and remainder by constants (replaced by multiplications)
#include <stdio.h>
#include <limits.h>

#define T(c) s = s * 31 + x / (c) + (x % (c)) * 7; u = u * 31 + ux / (c##u) + (ux % (c##u)) * 7;
#define P(c) s = s * 31 + x / (c) + (x % (c)) * 3;

static long run(int x) {
    long s = 0;
    unsigned long u = 0;
    unsigned ux = (unsigned)x;
    T(3) T(5) T(6) T(7) T(9) T(10) T(11) T(12) T(13) T(25) T(100) T(641) T(1000) T(65537) T(1000000007) T(2147483647)
    P(2) P(4) P(8) P(16) P(1024) P(1073741824)
    long long l = x;
    s += l / 3 + l % 7 + l / -5 + x / -3 + x % -7;
    return s ^ (long)u;
}

int main(void) {
    int vals[] = {0, 1, -1, 2, -2, 3, -3, 7, -7, 9, -9, 10, -10, 99, -99, 100, -100, 12345, -12345,
                  INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1, 1000000006, -1000000007, 2147483646, 65536, -65537};
    long h = 0;
    for (unsigned i = 0; i < sizeof vals / sizeof vals[0]; i++)
        h = h * 1000003 + run(vals[i]);
    for (int x = -3000; x <= 3000; x += 7)
        h = h * 13 + run(x);
    for (unsigned k = 0; k < 4000000000u; k += 99999989)
        h = h * 17 + run((int)k);
    printf("%ld\n", h);
    return 0;
}
