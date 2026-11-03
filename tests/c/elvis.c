#include <stdio.h>
static int calls;
static int f(int v) { calls++; return v; }
static int *pf(int *p) { calls++; return p; }
int main(void) {
    calls = 0; int r = f(5) ?: f(7); printf("%d %d\n", r, calls);
    calls = 0; r = f(0) ?: f(7); printf("%d %d\n", r, calls);
    int x = 3; calls = 0; int *p = pf(&x) ?: pf(0); printf("%d %d\n", *p, calls);
    calls = 0; double d = f(0) ?: 2.5; printf("%g %d\n", d, calls);
    int arr[3] = {0}; int i = 0; arr[i++] ?: (arr[i] = 9); printf("%d %d %d\n", i, arr[0], arr[1]);
    calls = 0; long l = (long)f(3) ?: 0L; printf("%ld %d\n", l, calls);
    return 0;
}
