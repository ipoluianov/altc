// blank lines right after a #line directive are not counted: __LINE__ is too small
#include <stdio.h>
#line 60


int main(void) {
    printf("%d\n", __LINE__); /* expect: 63 */
    return 0;
}
