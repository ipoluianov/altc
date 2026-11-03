// std: c2x
#include <stdio.h>
#include <stddef.h>
#include <stdckdint.h>
#include <limits.h>

static int unnamed_param(int, int b) { return b; }
static int lbl_end(int x) { if (x) goto end; x = 5; end: }
static int lbl_decl(void) { goto L; L: int v = 3; return v; }
thread_local int tl = 4;
alignas(16) static char abuf[16];
_Static_assert(1);
static int fallthrough(int x) { int r = 0; switch (x) { case 0: r += 1; [[fallthrough]]; case 1: r += 2; break; } return r; }
struct [[deprecated]] Old { int a; };
enum E : long { BIGE = 1L << 40 };

int main(void) {
    printf("%d %d %d\n", unnamed_param(1, 2), lbl_decl(), fallthrough(0));
    (void)lbl_end;
    printf("%d %d\n", tl, (int)((size_t)abuf % 16));
    int r; bool o = ckd_add(&r, INT_MAX, 1); printf("%d %d\n", o, r);
    o = ckd_mul(&r, 1000, 1000); printf("%d %d\n", o, r);
    unsigned ur; o = ckd_sub(&ur, 0u, 1u); printf("%d %u\n", o, ur);
#if true && 0b11 == 3 && 1'000 == 1000
    puts("pp c23 literals");
#endif
#if __has_c_attribute(fallthrough)
    puts("has fallthrough attr");
#endif
    printf("%ld %zu\n", (long)BIGE, sizeof(enum E));
    int café = 7; printf("%d\n", café);
    auto x = 5LL; printf("%zu\n", sizeof x);
    constexpr long lim = 100; static_assert(lim == 100);
    int arr[] = {[0] = 1, [2] = 3}; printf("%zu\n", sizeof arr);
    printf("%d\n", (int)alignof(max_align_t) >= 16);
    typeof_unqual(const volatile int) q = 2; q++; printf("%d\n", q);
    if (tl > 100) unreachable();
    return 0;
}
