// NaN signs follow gcc's folding of a - -b and a + -b
#include <stdio.h>
#include <math.h>
__attribute__((noinline)) double id(double x) { return x; }
int main(void) {
    double inf = id(INFINITY), n = inf - inf, z = id(0);
    printf("%f %f %f %f %f %f\n", n, -n, z - n, z - (-n), z + -n, -n + z);
    double s = 0;
    s -= -(inf - inf);
    float f = (float)n;
    printf("%f %f %f\n", s, (double)(f - -f), (double)(-f + 1.0f));
    return 0;
}
