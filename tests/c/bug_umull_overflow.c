// typed overflow builtins (__builtin_umull_overflow, __builtin_uaddl_overflow, ...) are not supported:
// "error: builtin mu_overflow is not supported"
#include <stdio.h>
int main(void) {
    unsigned long r;
    int o = __builtin_umull_overflow(1UL << 63, 2, &r);
    printf("%d %lu\n", o, r); /* expect: 1 0 */
    return 0;
}
