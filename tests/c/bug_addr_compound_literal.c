// taking the address of a compound literal at block scope is rejected:
// "error: lvalue required as unary '&' operand"
#include <stdio.h>
struct pt { int x, y; };
int main(void) {
    int *p = &(int){5};
    struct pt *q = &(struct pt){.y = 4};
    printf("%d %d\n", *p, q->y); /* expect: 5 4 */
    return 0;
}
