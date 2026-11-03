// GNU extended inline asm with a read-write register operand
#include <stdio.h>
int main(void) {
    int x = 5;
    __asm__("addl $1, %0" : "+r"(x));
    printf("%d\n", x); /* expect: 6 */
    return 0;
}
