// skip: extended inline assembly is not supported yet
// GNU extended inline asm with operands is rejected: "error: inline assembly is not supported"
// (only empty / barrier asm statements are accepted)
#include <stdio.h>
int main(void) {
    int x = 5;
    __asm__("addl $1, %0" : "+r"(x));
    printf("%d\n", x); /* expect: 6 */
    return 0;
}
