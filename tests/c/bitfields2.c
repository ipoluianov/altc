#include <stdio.h>
#include <string.h>
enum Color { RED, GREEN, BLUE, WHITE = 7 };
struct B1 { enum Color c : 3; signed char s : 3; unsigned char u : 5; long long ll : 50; unsigned long long ull : 64; };
union UB { struct { unsigned lo : 16, hi : 16; } p; unsigned all; };
struct B2 { volatile unsigned v : 4; const int k : 4; };
struct B3 { unsigned a : 1; unsigned : 0; unsigned b : 1; unsigned : 3; unsigned c : 2; };
struct B4 { char a; int b : 4; char c; int d : 20; short e : 3; };
struct B5 { unsigned long long x : 1; unsigned long long y : 63; };
struct B6 { int a : 31; int b : 2; int c : 31; };
struct B7 { unsigned short a : 9, b : 9, c : 9, d : 9; };
int main(void) {
    struct B1 b = {BLUE, -4, 31, -(1LL << 48), 0xfedcba9876543210ULL};
    printf("%d %d %d %lld %llx\n", b.c, b.s, b.u, (long long)b.ll, (unsigned long long)b.ull);
    b.c = WHITE; b.s = 3; b.u = 33; b.ll = (1LL << 49) - 1; b.ull >>= 4;
    printf("%d %d %d %lld %llx\n", b.c, b.s, b.u, (long long)b.ll, (unsigned long long)b.ull);
    b.ll += 1; printf("%lld\n", (long long)b.ll);
    union UB ub; ub.all = 0x12345678; printf("%x %x\n", ub.p.lo, ub.p.hi); ub.p.hi = 0xabcd; printf("%x\n", ub.all);
    struct B2 b2 = {9, -3}; b2.v++; printf("%u %d\n", b2.v, b2.k);
    printf("%zu %zu %zu %zu %zu %zu\n", sizeof(struct B1), sizeof(struct B3), sizeof(struct B4), sizeof(struct B5), sizeof(struct B6), sizeof(struct B7));
    struct B3 b3 = {1, 1, 3}; unsigned raw[2]; memcpy(raw, &b3, sizeof b3); printf("%x %x\n", raw[0], sizeof b3 > 4 ? raw[1] : 0);
    struct B4 b4 = {'a', -8, 'c', -500000, 3}; printf("%c %d %c %d %d\n", b4.a, b4.b, b4.c, b4.d, b4.e);
    struct B5 b5 = {1, 0x7fffffffffffffffULL}; printf("%llu %llx\n", (unsigned long long)b5.x, (unsigned long long)b5.y);
    b5.y++; printf("%llx\n", (unsigned long long)b5.y);
    struct B6 b6 = {-1073741824, -2, 1073741823}; printf("%d %d %d\n", b6.a, b6.b, b6.c);
    struct B7 b7 = {511, 256, 1, 300}; printf("%d %d %d %d\n", b7.a, b7.b, b7.c, b7.d);
    int arr[8] = {0, 10, 20, 30, 40, 50, 60, 70}; struct { unsigned i : 3; } idx = {5}; printf("%d\n", arr[idx.i]);
    for (idx.i = 0; idx.i < 7; idx.i++) printf("%d", idx.i); printf("\n");
    struct { unsigned n : 2; } wrap = {0}; for (int i = 0; i < 6; i++) wrap.n++; printf("%u\n", wrap.n);
    struct { unsigned n : 4; } neg = {0}; neg.n = -1; printf("%u\n", neg.n);
    struct { int s : 4; } sg; sg.s = 15; printf("%d\n", sg.s);
    struct B4 *pb = &b4; pb->d += 1000000; printf("%d\n", pb->d);
    unsigned long long xs = b5.y << 1; printf("%llx\n", xs);
    return 0;
}
