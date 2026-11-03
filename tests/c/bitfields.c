#include <stdio.h>
#include <string.h>

struct bf1 { unsigned a : 3; unsigned b : 5; unsigned c : 24; };
struct bf2 { int a : 3; int b : 5; signed int c : 1; unsigned d : 1; };
struct bf3 { char x; unsigned a : 4; unsigned b : 4; char y; };
struct bf4 { unsigned long long a : 40; unsigned long long b : 24; };
struct bf5 { unsigned a : 30; unsigned b : 4; };   /* b straddles -> new unit */
struct bf6 { int a : 7; int : 0; int b : 7; };
struct bf7 { unsigned char a : 1, b : 2, c : 3; };
struct bf8 { short s : 9; short t : 9; };
struct bf9 { long long v : 33; int w : 31; };
struct bf10 { _Bool f1 : 1; _Bool f2 : 1; unsigned rest : 6; };
struct bf11 { int x; unsigned : 4; unsigned y : 4; };

int main(void) {
    printf("%zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n", sizeof(struct bf1), sizeof(struct bf2), sizeof(struct bf3),
           sizeof(struct bf4), sizeof(struct bf5), sizeof(struct bf6), sizeof(struct bf7), sizeof(struct bf8),
           sizeof(struct bf9), sizeof(struct bf10), sizeof(struct bf11));
    struct bf1 s1 = {5, 17, 0xabcdef};
    printf("%u %u %x\n", s1.a, s1.b, s1.c);
    unsigned raw; memcpy(&raw, &s1, 4); printf("%08x\n", raw);
    s1.a = 9; s1.b = 40; s1.c = 0x1234567; printf("%u %u %x\n", s1.a, s1.b, s1.c);
    s1.a++; s1.b += 30; s1.c *= 2; printf("%u %u %x\n", s1.a, s1.b, s1.c);
    s1.a = 7; s1.a += 1; printf("%u\n", s1.a);
    struct bf2 s2 = {-3, 15, -1, 1};
    printf("%d %d %d %u\n", s2.a, s2.b, s2.c, s2.d);
    s2.a = 3; s2.a++; printf("%d\n", s2.a);
    s2.b = -16; s2.b--; printf("%d\n", s2.b);
    s2.c = 1; printf("%d\n", s2.c);
    int assigned = (s2.a = 5); printf("%d %d\n", assigned, s2.a);
    unsigned ua = (s1.a = 12); printf("%u\n", ua);
    struct bf3 s3 = {'x', 10, 5, 'y'}; printf("%c %u %u %c %zu\n", s3.x, s3.a, s3.b, s3.y, sizeof s3);
    unsigned char bytes[4]; memcpy(bytes, &s3, sizeof s3); printf("%02x %02x %02x\n", bytes[0], bytes[1], bytes[2]);
    struct bf4 s4 = {0xffffffffffULL, 0xabcdef}; printf("%llx %llx\n", (unsigned long long)s4.a, (unsigned long long)s4.b);
    s4.a += 1; printf("%llx\n", (unsigned long long)s4.a);
    s4.b <<= 4; printf("%llx\n", (unsigned long long)s4.b);
    struct bf5 s5 = {0x3fffffff, 9}; printf("%x %u\n", s5.a, s5.b);
    struct bf6 s6 = {-64, 63}; printf("%d %d\n", s6.a, s6.b);
    struct bf7 s7 = {1, 3, 7}; printf("%d %d %d\n", s7.a, s7.b, s7.c);
    unsigned char b7; memcpy(&b7, &s7, 1); printf("%02x\n", b7);
    struct bf8 s8 = {-256, 255}; printf("%d %d\n", s8.s, s8.t);
    struct bf9 s9 = {-4294967296LL, -1073741824}; printf("%lld %d\n", (long long)s9.v, s9.w);
    s9.v = 4294967295LL; printf("%lld\n", (long long)s9.v);
    struct bf10 s10 = {1, 0, 63}; printf("%d %d %u\n", s10.f1, s10.f2, s10.rest);
    s10.f2 = 5; printf("%d\n", s10.f2);
    struct bf11 s11 = {1, 15}; printf("%d %u\n", s11.x, s11.y);
    /* promotions: unsigned bitfield < int width promotes to int */
    struct bf1 p1 = {0, 0, 0}; printf("%d\n", p1.a - 1 < 0);
    printf("%d\n", s1.c - 0x2000000 < 0);
    /* bitfield in expressions */
    struct bf2 e2 = {1, 2, 0, 1}; int tot = e2.a * 100 + e2.b * 10 + e2.d; printf("%d\n", tot);
    printf("%d\n", e2.a ? e2.b : e2.d);
    struct bf1 arr[3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int sum = 0; for (int i = 0; i < 3; i++) sum += arr[i].a + arr[i].b + arr[i].c; printf("%d\n", sum);
    struct bf1 *pb = &arr[1]; pb->b |= 0x10; pb->a ^= 7; printf("%u %u\n", pb->a, pb->b);
    unsigned long long big = s4.a * 2; printf("%llx\n", big);
    return 0;
}
