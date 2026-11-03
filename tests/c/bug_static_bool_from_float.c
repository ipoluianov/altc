// static _Bool initialized from a floating constant is truncated instead of compared with 0
#include <stdio.h>
static _Bool sb = 0.1;
static _Bool sb2 = 0.5f;
int main(void) {
    printf("%d %d\n", sb, sb2); /* expect: 1 1 */
    return 0;
}
