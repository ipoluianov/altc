#include <stdio.h>
static size_t sz_arr(int a[10]) { return sizeof a; }
static size_t sz_2d(int a[3][4]) { return sizeof a + sizeof a[0] * 100; }
static int call_fn(int f(int), int v) { return f(v) + (int)sizeof f; }
static int dbl(int x) { return 2 * x; }
static size_t sz_const(const char s[const]) { return sizeof s; }
static int mod_param(int a[3]) { a++; return a[0]; }
static int sum(int n, int a[static 1]) { int s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
typedef int fn_t(int);
static int via_typedef(fn_t f) { return f(21); }
static fn_t *ret_fn(void) { return dbl; }
int main(void) {
    int arr[10] = {1, 2, 3}; int m[3][4];
    printf("%zu %zu %d %zu %d %d\n", sz_arr(arr), sz_2d(m), call_fn(dbl, 5), sz_const("x"), mod_param(arr), sum(3, arr));
    printf("%d %d\n", via_typedef(dbl), ret_fn()(4));
    fn_t *fp = dbl; printf("%d %zu\n", fp(1), sizeof fp);
    return 0;
}
