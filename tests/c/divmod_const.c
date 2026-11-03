#include <stdio.h>
#include <limits.h>
#include <stdint.h>

static int vals[] = {0, 1, -1, 7, -7, 8, -8, 9, -9, 100, -100, 1023, -1023, 1024, -1024, INT_MAX, INT_MIN + 1, INT_MIN, 123456789, -123456789};
static long long lvals[] = {0, 1, -1, 7, -7, 1LL << 40, -(1LL << 40), LLONG_MAX, LLONG_MIN + 1, LLONG_MIN, 999999999999LL, -999999999999LL};

int main(void) {
    for (unsigned i = 0; i < sizeof vals / sizeof *vals; i++) {
        int x = vals[i]; unsigned u = (unsigned)x;
        printf("%d: %d %d %d %d %d %d %d %d | %d %d %d %d %d %d\n", x, x / 2, x / 4, x / 8, x / 3, x / 7, x / 10, x / 1000, x / 1, x % 2, x % 4, x % 8, x % 3, x % 7, x % 1000);
        if (x != INT_MIN) printf("   %d %d %d\n", x / -2, x % -2, x / -1);
        printf("   %u %u %u %u %u %u %u\n", u / 2, u / 16, u / 3, u / 10, u % 16, u % 10, u / 0x80000000u);
        printf("   %d %d %u %u\n", x >> 1, x >> 31, u >> 1, u >> 31);
        int sh = (int)(i % 32); printf("   %d %u %d\n", (int)(u << (sh / 2)) >> (sh / 2), u >> sh, sh);
    }
    for (unsigned i = 0; i < sizeof lvals / sizeof *lvals; i++) {
        long long x = lvals[i]; unsigned long long u = (unsigned long long)x;
        printf("%lld: %lld %lld %lld %lld %lld | %lld %lld %lld\n", x, x / 2, x / 3, x / 16, x / 1000000007LL, x / 10, x % 2, x % 16, x % 1000000007LL);
        printf("   %llu %llu %llu %llu\n", u / 3, u / 10, u % 1000, u / 0x8000000000000000ULL);
        for (int s = 0; s < 64; s += 21) printf("   %lld %llu", x >> s, u >> s);
        printf("\n");
    }
    unsigned char uc = 200; signed char sc = -100; unsigned short us = 50000; short ss = -20000;
    printf("%d %d %d %d\n", uc / 3, sc / 3, us / 7, ss / 7);
    printf("%d %d %d %d\n", uc % 3, sc % 3, us % 7, ss % 7);
    printf("%d %d %d %d\n", uc << 4, (int)((unsigned)sc << 2), uc >> 3, sc >> 3);
    int n = 5; printf("%d %d\n", 1 << n, (int)(~0u << n));
    uint64_t one = 1; for (int s = 30; s < 36; s++) printf("%llx ", (unsigned long long)(one << s)); printf("\n");
    return 0;
}
