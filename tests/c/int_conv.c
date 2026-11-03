#include <stdio.h>
#include <stdint.h>

#define T(type, val) printf("%-20s %-30s -> %lld / %llu\n", #type, #val, (long long)(type)(val), (unsigned long long)(type)(val))

static long long vals[] = {0, 1, -1, 127, 128, 255, 256, -128, -129, 32767, 32768, 65535, 65536,
    -32768, -32769, 2147483647LL, 2147483648LL, 4294967295LL, 4294967296LL, -2147483648LL,
    -2147483649LL, 0x123456789abcdefLL, -0x123456789abcdefLL, 0x7fffffffffffffffLL};

int main(void) {
    for (unsigned k = 0; k < sizeof vals / sizeof vals[0]; k++) {
        long long v = vals[k];
        signed char a = v; unsigned char b = v; short c = v; unsigned short d = v;
        int e = v; unsigned f = v; long g = v; unsigned long h = v;
        printf("%lld: %d %d %d %d %d %u %ld %lu\n", v, a, b, c, d, e, f, g, h);
        /* chain through variables of other types */
        int fromc = a; long froms = c; unsigned long fromu = f; long long fromd = d;
        printf("  %d %ld %lu %lld\n", fromc, froms, fromu, fromd);
        unsigned long long uv = (unsigned long long)v;
        printf("  %u %d %hhd %hhu %hd %hu\n", (unsigned)uv, (int)uv, (signed char)uv, (unsigned char)uv, (short)uv, (unsigned short)uv);
    }
    T(signed char, 300);
    T(unsigned char, -3);
    T(short, 100000);
    T(unsigned short, -100000);
    T(int, 0xfffffffffLL);
    T(unsigned, -1);
    T(long, -1);
    T(unsigned long, -1);
    T(_Bool, 0x100000000LL);
    T(_Bool, 0);
    T(int8_t, 0x1ff);
    T(uint8_t, 0x1ff);
    T(int16_t, 0x18000);
    T(uint64_t, -2);
    /* conversion in function args and returns */
    return 0;
}
