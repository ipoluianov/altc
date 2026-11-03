// __ATOMIC_RELAXED/__ATOMIC_SEQ_CST/... memory-order macros are not predefined (error: '__ATOMIC_SEQ_CST' undeclared)
#include <stdio.h>
int main(void) {
    long v = 1;
    __atomic_fetch_add(&v, 2, __ATOMIC_SEQ_CST);
    printf("%ld %d\n", v, __ATOMIC_RELAXED); /* expect: 3 0 */
    return 0;
}
