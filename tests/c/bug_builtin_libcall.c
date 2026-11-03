// __builtin_<libc function> calls (strlen, memcpy, memcmp, abs, fabs, sqrt, floor, ...) fail to parse:
// "error: expected ',' before ';'" (cc/builtins.go libCall path)
#include <stdio.h>
int main(void) {
    double d = __builtin_fabs(-2.5);
    unsigned long n = __builtin_strlen("abc");
    printf("%g %lu\n", d, n); /* expect: 2.5 3 */
    return 0;
}
