#include <stdio.h>

signed char ret_sc(int x) { return x; }
unsigned char ret_uc(int x) { return x; }
short ret_s(long x) { return x; }
unsigned short ret_us(long x) { return x; }
int ret_i(long long x) { return x; }
unsigned ret_u(long long x) { return x; }
_Bool ret_b(long x) { return x; }

long take(signed char a, unsigned char b, short c, unsigned short d, int e, unsigned f, long g, unsigned long h) {
    return a + b + c + d + e + (long)f + g + (long)h;
}
long take_many(char a, short b, int c, long d, char e, short f, int g, long h, char i, short j) {
    return a * 1 + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}
unsigned long long mix(unsigned char a, signed char b, unsigned short c, short d, unsigned e, int f, unsigned char g, signed char h) {
    return (unsigned long long)a << 56 ^ (unsigned long long)(b & 0xff) << 48 ^ (unsigned long long)c << 32 ^ (unsigned)(d & 0xffff) << 16 ^ e ^ f ^ g ^ h;
}

int fact(int n) { return n <= 1 ? 1 : n * fact(n - 1); }
long long fib(int n) { long long a = 0, b = 1; while (n--) { long long t = a + b; a = b; b = t; } return a; }
unsigned gcd(unsigned a, unsigned b) { while (b) { unsigned t = a % b; a = b; b = t; } return a; }
int isqrt(int n) { int r = 0; while ((r + 1) * (r + 1) <= n) r++; return r; }
unsigned long long powmod(unsigned long long b, unsigned long long e, unsigned long long m) {
    unsigned long long r = 1; b %= m;
    while (e) { if (e & 1) r = (unsigned __int128)r * b % m; b = (unsigned __int128)b * b % m; e >>= 1; }
    return r;
}
int collatz(long long n) { int s = 0; while (n != 1) { n = n % 2 ? 3 * n + 1 : n / 2; s++; } return s; }

int main(void) {
    printf("%d %d %d %d\n", ret_sc(200), ret_uc(-1), ret_s(70000), ret_us(-1));
    printf("%d %u %d %d\n", ret_i(0x100000005LL), ret_u(-5), ret_b(0x100000000L), ret_b(0));
    printf("%ld\n", take(-1, 255, -300, 60000, -70000, 4000000000u, -5000000000L, 6000000000UL));
    printf("%ld\n", take(300, 300, 70000, 70000, 1, 1, 1, 1));
    printf("%ld\n", take_many(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    printf("%ld\n", take_many(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10));
    printf("%llx\n", mix(0xab, -2, 0xcdef, -3, 0xdeadbeef, -12345, 200, -100));
    for (int i = 0; i < 13; i++) printf("%d ", fact(i));
    printf("\n");
    printf("%lld %lld\n", fib(50), fib(90));
    printf("%u %u\n", gcd(1071, 462), gcd(4000000000u, 3000000000u));
    printf("%d %d\n", isqrt(1000000), isqrt(99));
    printf("%llu\n", powmod(2, 1000000007ULL - 2, 1000000007ULL));
    printf("%llu\n", powmod(123456789, 987654321, 18446744073709551557ULL));
    printf("%d\n", collatz(837799));
    unsigned __int128 w = (unsigned __int128)0xffffffffffffffffULL * 0xffffffffffffffffULL;
    printf("%llx %llx\n", (unsigned long long)(w >> 64), (unsigned long long)w);
    __int128 sw = -(__int128)1 << 100;
    printf("%lld %d\n", (long long)(sw >> 90), sw < 0);
    return 0;
}
