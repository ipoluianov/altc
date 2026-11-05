// optimized code (-O2 -g): locations of variables and an inlined call
#include <stdio.h>

static int sq(int x) {
    int y = x * x;
    return y + 1;
}

__attribute__((noinline)) int work(int n, int *a) {
    int sum = 0;
    double scale = 1.5;
    for (int i = 0; i < n; i++) {
        int t = sq(a[i]);
        sum += t;
        scale *= 1.25;
    }
    printf("%d %g\n", sum, scale);
    return sum;
}

__attribute__((noinline)) unsigned long wide(unsigned long x);

int main(void) {
    int a[5] = {1, 2, 3, 4, 5};
    int r = work(5, a) + (int)wide(5) - 320;
    return r == 60 ? 0 : 1;
}

// a 128-bit variable: its halves in two places
__attribute__((noinline)) unsigned long wide(unsigned long x) {
    unsigned __int128 big = (unsigned __int128)x << 70;
    printf("%lu\n", (unsigned long)(big >> 70));
    return (unsigned long)(big >> 64);
}
