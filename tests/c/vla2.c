#include <stdio.h>
#include <string.h>
static void proto_star(int n, int a[*]);
static void proto_star(int n, int a[n]) { printf("%d\n", a[n - 1]); }
static int loop_vla(int iters) {
    long total = 0;
    for (int i = 0; i < iters; i++) { char buf[1000 + i % 7]; memset(buf, i & 0x7f, sizeof buf); total += buf[999]; }
    return (int)(total % 100000);
}
static int goto_vla(int iters) {
    int count = 0, i = 0;
again:
    if (i < iters) { int v[256 + i % 3]; v[0] = i; count += v[0] & 1; i++; goto again; }
    return count;
}
static int break_vla(int iters) { int s = 0; while (iters--) { double d[500]; int vl[iters % 50 + 1]; vl[0] = 1; d[0] = 1; s += vl[0] + (int)d[0]; if (iters == 3) break; } return s; }
int main(void) {
    int arr[4] = {1, 2, 3, 4}; proto_star(4, arr);
    int n = 3; size_t sz = sizeof(int[n++]); printf("%zu %d\n", sz, n);
    typedef int vt[n]; n = 100; vt x; printf("%zu\n", sizeof x);
    printf("%d\n", loop_vla(200000));
    printf("%d\n", goto_vla(300000));
    printf("%d\n", break_vla(100000));
    int m = 2; int (*p)[m] = (int (*)[m])arr; printf("%d %zu\n", p[1][1], sizeof *p);
    int k = 5; char (*q)[k] = 0; q++; printf("%td\n", (char *)q - (char *)0);
    return 0;
}
