#include <stdio.h>
#include <string.h>

typedef void (*handler_t)(int);
static int last;
static void h1(int x) { last = x; }
static void h2(int x) { last = -x; }
static handler_t current = h1;
static handler_t set_handler(int which, handler_t h) { handler_t old = current; current = which ? h : h1; return old; }
static void (*set_handler_raw(int sig, void (*fn)(int)))(int) { (void)sig; void (*old)(int) = current; current = fn; return old; }
static int arr3[3] = {1, 2, 3};
static int (*get_arr(void))[3] { return &arr3; }
static int (*(*fp_ret_arr)(void))[3] = get_arr;
static int add(int a, int b) { return a + b; }
static int (*choose(char op))(int, int) { return op == '+' ? add : NULL; }
static int (*(*choose_ptr)(char))(int, int) = choose;
typedef int matrix[2][2];
static int trace(const matrix m) { return m[0][0] + m[1][1]; }
typedef struct { int v; } Wrap, *WrapPtr;
static const char *const days[] = {"mon", "tue"};
static char *(*str_fn)(char *, const char *) = strcpy;
int (*const const_fp)(int, int) = add;
static int sum_array(int n, const int a[]) { int s = 0; while (n--) s += a[n]; return s; }
static int apply2(int (*f[2])(int, int), int x) { return f[0](x, x) + f[1](x, 1); }
static int mulf(int a, int b) { return a * b; }

int main(void) {
    current(5); printf("%d\n", last);
    handler_t old = set_handler(1, h2); current(5); printf("%d %d\n", last, old == h1);
    void (*o2)(int) = set_handler_raw(0, h1); o2(7); printf("%d\n", last);
    printf("%d\n", (*fp_ret_arr())[2]);
    printf("%d\n", choose('+')(3, 4));
    printf("%d %d\n", (*choose_ptr)('+')(10, 20), choose('-') == NULL);
    matrix m = {{1, 2}, {3, 4}}; printf("%d %zu\n", trace(m), sizeof(matrix));
    Wrap w = {9}; WrapPtr wp = &w; printf("%d\n", wp->v);
    printf("%s %c\n", days[1], days[0][2]);
    char buf[16]; str_fn(buf, "copied"); printf("%s\n", buf);
    printf("%d\n", const_fp(2, 2));
    int data[] = {1, 2, 3, 4}; printf("%d\n", sum_array(4, data));
    int (*fs[2])(int, int) = {add, mulf}; printf("%d\n", apply2(fs, 6));
    int *ptrs[3], **pp = ptrs, x = 1, y = 2, z = 3; ptrs[0] = &x; ptrs[1] = &y; ptrs[2] = &z; printf("%d\n", **(pp + 2));
    int (*pa)[4] = &data; printf("%d %zu\n", (*pa)[3], sizeof *pa);
    char *(*names_p)[2] = (char *(*)[2])&days; printf("%s\n", (*names_p)[0]);
    const volatile int cvi = 5; printf("%d\n", cvi);
    int const *const icc = &x; printf("%d\n", *icc);
    unsigned typedef_test; typedef unsigned long ulong_t; ulong_t big = 1UL << 40; printf("%lu\n", big);
    int a2, *b2, c2[2], (*d2)(void) = 0; a2 = 1; b2 = &a2; c2[1] = *b2; printf("%d %d\n", c2[1], d2 == 0);
    struct S { int x; } s1 = {1}, *s2 = &s1, s3[2] = {{2}, {3}}; printf("%d %d %d\n", s1.x, s2->x, s3[1].x);
    enum E { A = -5, B, C = 1u << 31 > 0 ? 100 : 0, D } e = D; printf("%d %d %d %d\n", A, B, C, e);
    enum Big { BIGV = 0x7fffffff }; printf("%d %zu\n", BIGV, sizeof(enum Big));
    enum Huge { HUGEV = 0x100000000LL }; printf("%lld %zu\n", (long long)HUGEV, sizeof(enum Huge));
    enum Neg { NEGV = -1 }; enum Neg nv = NEGV; printf("%d\n", nv < 0);
    return 0;
}
