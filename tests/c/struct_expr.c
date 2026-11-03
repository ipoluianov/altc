#include <stdio.h>
#include <string.h>

typedef struct { int a[4]; char name[8]; } Arr;
typedef struct { unsigned x : 5; int y : 11; unsigned z : 16; } BF;
typedef struct { double d; int i; } DI;
typedef struct { char c; } Tiny;
typedef struct { long double v; } LDS;
static Arr mk(int k) { Arr r; for (int i = 0; i < 4; i++) r.a[i] = k * 10 + i; snprintf(r.name, sizeof r.name, "arr%d", k); return r; }
static BF bf_mk(int x, int y, int z) { BF b = {x, y, z}; return b; }
static int bf_sum(BF b) { return b.x + b.y + b.z; }
static BF bf_inc(BF b) { b.x++; b.y--; b.z += 2; return b; }
static DI di_pick(int c, DI a, DI b) { return c ? a : b; }
static Tiny tiny(char c) { return (Tiny){c}; }
static LDS lds_add(LDS a, LDS b) { return (LDS){a.v + b.v}; }

int main(void) {
    printf("%d %s\n", mk(3).a[2], mk(4).name);
    int v = mk(5).a[3] + mk(6).a[0]; printf("%d\n", v);
    Arr x = mk(1), y = mk(2), z; z = x = y; printf("%d %d %s\n", x.a[0], z.a[1], z.name);
    Arr *px = &x; *px = (v > 0) ? mk(7) : mk(8); printf("%d\n", px->a[3]);
    Arr w = (mk(9), mk(10)); printf("%d\n", w.a[0]);
    Arr st = ({ Arr t = mk(11); t.a[0] = -1; t; }); printf("%d %d\n", st.a[0], st.a[1]);
    Arr list[3] = {mk(1), mk(2), mk(3)}; printf("%d\n", list[2].a[2]);
    BF b = bf_mk(31, -1024, 65535); printf("%u %d %u %d\n", b.x, b.y, b.z, bf_sum(b));
    BF b2 = bf_inc(b); printf("%u %d %u\n", b2.x, b2.y, b2.z);
    printf("%d\n", bf_sum(bf_inc(bf_inc(bf_mk(1, 2, 3)))));
    printf("%d %d\n", b.x - 32 < 0, b.z - 65536 < 0);
    printf("%zu %zu\n", sizeof(b.x + 0), sizeof(b.z + 0u));
    printf("%d\n", (b.y < 0 ? b.x : b.z));
    DI d1 = {1.5, 1}, d2 = {2.5, 2}; DI r = di_pick(0, d1, d2); printf("%g %d\n", r.d, r.i);
    printf("%g\n", di_pick(1, d1, d2).d);
    printf("%c%c\n", tiny('o').c, tiny('k').c);
    LDS l = lds_add((LDS){1.25L}, lds_add((LDS){2.5L}, (LDS){0.125L})); printf("%Lg\n", l.v);
    Tiny ts[5]; for (int i = 0; i < 5; i++) ts[i] = tiny('a' + i); printf("%c%c\n", ts[0].c, ts[4].c);
    struct { Arr inner; int n; } outer = {mk(12), 5}; outer.inner = mk(13); printf("%d %d\n", outer.inner.a[1], outer.n);
    const Arr ca = mk(14); Arr cb = ca; cb.a[0]++; printf("%d %d\n", ca.a[0], cb.a[0]);
    Arr arr_cmp1 = mk(1), arr_cmp2 = mk(1); printf("%d\n", memcmp(arr_cmp1.a, arr_cmp2.a, sizeof arr_cmp1.a) == 0);
    return 0;
}
