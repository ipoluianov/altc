// constant expressions in static long double initializers are folded in double precision
#include <stdio.h>
static long double sld = 1.0L / 7;
int main(void) {
    printf("%.20Lf\n", sld); /* expect: 0.14285714285714285714 (got ...84921) */
    return 0;
}
