#include <stdio.h>
#include <stddef.h>

typedef int T;
static int one(void) { return 1; }
static int two(void) { return 2; }
static int (*(*tbl_ptr)[2])(void);
static int (*tbl[2])(void) = {one, two};
union U { int i; float f; double d; };
struct WithT { T T; int x; };
enum Attr { AA __attribute__((deprecated)) = 1, AB [[deprecated]] = 2 };
static int attr_param(int a __attribute__((unused)), int b) { return b; }
__attribute__((noinline)) int static_after_attr(void);
int __attribute__((noinline)) static_after_attr(void) { return 3; }
int attr_after_decl(void) __attribute__((noinline));
int attr_after_decl(void) { return 4; }

int main(void) {
    tbl_ptr = &tbl; printf("%d %d\n", (*tbl_ptr)[0](), (*tbl_ptr)[1]());
    int arr[3] = {1, 2, 3}; void *vp = arr;
    printf("%d\n", ((int (*)[3])vp)[0][2]);
    void (*nullfn)(int) = (void (*)(int))0; printf("%d\n", nullfn == 0);
    printf("%zu %zu\n", sizeof(int (*(*)[3])(void)), sizeof(int (*)[3]));
    _Atomic(int) ai = 5; _Atomic int ai2 = 6; ai += 1; ai2++; printf("%d %d\n", ai, ai2);
    int x = 1; int *restrict rp = &x; int *restrict *restrict rpp = &rp; printf("%d\n", **rpp);
    int __attribute__((unused)) a1 = 1, a2 __attribute__((aligned(8))) = 2; printf("%d\n", a1 + a2);
    struct __attribute__((packed)) { char c; int i; } __attribute__((aligned(1))) pk = {1, 2}; printf("%zu\n", sizeof pk);
    lbl: __attribute__((unused)); printf("%d %d\n", AA, AB);
    printf("%d %d %d\n", attr_param(0, 5), static_after_attr(), attr_after_decl());
    __extension__ long long ell = 1; struct { __extension__ int ext_field; } es = {3}; printf("%lld %d\n", ell, es.ext_field);
    typeof(int[3]) ta = {4, 5, 6}; printf("%zu %d\n", sizeof ta, ta[2]);
    typeof(one) *fp2 = one; printf("%d\n", fp2());
    __typeof__(two) *fp3 = two; printf("%d\n", fp3());
    printf("%zu\n", sizeof (int[]){1, 2, 3, 4});
    printf("%d %d %d\n", _Generic(arr, int *: 1, default: 0), _Generic("abc", char *: 1, default: 0), _Generic(one, int (*)(void): 1, default: 0));
    printf("%d\n", _Generic((const int)5, int: 1, const int: 2, default: 0));
    struct WithT wt = {1, 2}; printf("%d %d\n", wt.T, wt.x);
    switch ('b') { case (int)'a': puts("a"); break; case (char)98: puts("b"); break; }
    int *pp = arr; printf("%zu %zu %zu\n", sizeof *pp, sizeof -1, sizeof pp[1]);
    struct E {} empty = {}; printf("%zu\n", sizeof empty);
    int zero_init[3] = {}; printf("%d\n", zero_init[1]);
    enum { IDX = 2 }; struct { int a[4]; } di = {.a[IDX] = 9, .a[IDX + 1] = 8}; printf("%d %d\n", di.a[2], di.a[3]);
    printf("%d\n", (int)offsetof(struct WithT, x));
    return 0;
}
