#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef union { float f; uint32_t u; } FU;
typedef union { double d; uint64_t u; struct { uint32_t lo, hi; } w; } DU;
typedef union { struct { uint8_t a, b, c, d; } bytes; uint32_t word; } BW;
typedef struct { int kind; union { int i; double d; char s[8]; } v; } Tagged;
typedef union { int i; double d; } UD;
static UD ud_id(UD x) { return x; }
static Tagged tag_mk(int k) { Tagged t; t.kind = k; if (k == 0) t.v.i = 42; else if (k == 1) t.v.d = 2.5; else strcpy(t.v.s, "str"); return t; }
static void tag_print(Tagged t) { if (t.kind == 0) printf("int %d\n", t.v.i); else if (t.kind == 1) printf("dbl %g\n", t.v.d); else printf("str %s\n", t.v.s); }
union U3 { char c; short s; long long ll; };
union Nested { struct { int x, y; } p; long l; };
struct WithUArr { union { int a[2]; long b; }; int tail; };

int main(void) {
    FU fu = {.f = -1.5f}; printf("%08x\n", fu.u);
    fu.u = 0x3f800000; printf("%g\n", fu.f);
    DU du = {.d = 1.0}; printf("%016llx %08x %08x\n", (unsigned long long)du.u, du.w.hi, du.w.lo);
    du.u = 0x7ff0000000000000ULL; printf("%g\n", du.d);
    BW bw = {.word = 0x11223344}; printf("%x %x %x %x\n", bw.bytes.a, bw.bytes.b, bw.bytes.c, bw.bytes.d);
    bw.bytes.a = 0xff; printf("%x\n", bw.word);
    for (int k = 0; k < 3; k++) tag_print(tag_mk(k));
    UD u = {.d = 3.75}; UD r = ud_id(u); printf("%g\n", r.d);
    u.i = 7; r = ud_id(u); printf("%d\n", r.i);
    printf("%zu %zu %zu\n", sizeof(union U3), sizeof(union Nested), sizeof(struct WithUArr));
    union Nested n = {{1, 2}}; printf("%d %d %lx\n", n.p.x, n.p.y, n.l);
    struct WithUArr w = {{{1, 2}}, 3}; printf("%d %d %d\n", w.a[0], w.a[1], w.tail);
    union U3 u3; memset(&u3, 0, sizeof u3); u3.c = -1; printf("%lld %d\n", u3.ll, u3.s);
    float fv = 3.0f; uint32_t bits; memcpy(&bits, &fv, 4); printf("%08x\n", bits);
    double dv = -0.0; uint64_t db; memcpy(&db, &dv, 8); printf("%016llx\n", (unsigned long long)db);
    union { int i; char c[4]; } endian = {1}; printf("%s endian\n", endian.c[0] ? "little" : "big");
    UD arr[3] = {{1}, {.d = 2.0}, {3}}; printf("%d %g %d\n", arr[0].i, arr[1].d, arr[2].i);
    UD *up = &arr[1]; up->d *= 4; printf("%g\n", arr[1].d);
    return 0;
}
