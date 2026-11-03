// signed char / short converted to a 64-bit type must be sign-extended to 64 bits
#include <stdio.h>
long f4(long x, signed char h) { return x + h; }
int main(void) {
    short c = -1; long a = c; signed char sc = -2; long long e = sc;
    printf("%ld %lld %ld\n", a, e, f4(0, -2)); /* expect: -1 -2 -2 */
    return 0;
}
