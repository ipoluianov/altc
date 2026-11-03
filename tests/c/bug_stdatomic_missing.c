// <stdatomic.h> is not available ("stdatomic.h: No such file or directory"); widely included by real code
#include <stdio.h>
#include <stdatomic.h>
int main(void) {
    atomic_int v = 1;
    atomic_fetch_add(&v, 2);
    _Atomic long w = 5; w += 1;
    printf("%d %ld\n", atomic_load(&v), (long)w); /* expect: 3 6 */
    return 0;
}
