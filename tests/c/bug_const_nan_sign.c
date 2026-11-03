// constant folding of inf*0 yields a negative NaN (gcc folds to +nan)
#include <stdio.h>
static float f = (float)1e40 * 0;
int main(void) {
    double d = 1e308 * 10 * 0;
    printf("%f %f\n", f, d); /* expect: nan nan */
    return 0;
}
