// minor: isnan() on a long double NaN returns 65535 (gcc: 1). Any nonzero value is allowed by
// the standard, but programs that print the result differ from gcc.
#include <stdio.h>
#include <math.h>
int main(void) {
    volatile long double ln = NAN;
    printf("%d\n", isnan(ln)); /* expect: 1 */
    return 0;
}
