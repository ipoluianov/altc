#include <stdio.h>
void thrower(int);
__attribute__((noinline)) int mid(int x) {
    long a[64];
    for (int i = 0; i < 64; i++) a[i] = i * x;
    register long keep = a[x & 63] + 7;
    thrower(x);
    printf("not reached %ld\n", keep);
    return (int)keep;
}
__attribute__((noinline)) int mid2(int x) { int r = mid(x + 1) + 1; return r * 2; }
