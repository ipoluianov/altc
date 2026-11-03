// std: c2x
// C23 header <stdckdint.h> (ckd_add/ckd_sub/ckd_mul) is missing: "stdckdint.h: No such file or directory"
#include <stdio.h>
#include <stdckdint.h>
#include <limits.h>
int main(void) {
    int r;
    bool o = ckd_add(&r, INT_MAX, 1);
    printf("%d %d\n", o, r); /* expect: 1 -2147483648 */
    return 0;
}
