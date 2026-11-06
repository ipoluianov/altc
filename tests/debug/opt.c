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
__attribute__((noinline)) double cplx(double _Complex a, float _Complex b);

int main(void) {
    int a[5] = {1, 2, 3, 4, 5};
    int r = work(5, a) + (int)wide(5) - 320 + (int)cplx(1.5 + 2.5i, 0.5f - 1i);
    return r == 60 ? 0 : 1;
}

// a 128-bit variable: its halves in two places
__attribute__((noinline)) unsigned long wide(unsigned long x) {
    unsigned __int128 big = (unsigned __int128)x << 70;
    printf("%lu\n", (unsigned long)(big >> 70));
    return (unsigned long)(big >> 64);
}

// complex variables: the parts in two registers (float parts packed in
// one for the argument)
__attribute__((noinline)) double cplx(double _Complex a, float _Complex b) {
    double _Complex z = a * 2 + b;
    float _Complex w = b * 3;
    printf("%g %g\n", __real__ z, __imag__ w);
    return __real__ z + __imag__ z + __real__ w + __imag__ w - 6; // 0
}
