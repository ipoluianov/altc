// std: c99
// In strict -std=c89/c99/c11/c17, "typeof", "asm", "inline"(c89) and "restrict"(c89) are plain identifiers.
// altc: "variable name omitted" / "expected '(' before '='"
#include <stdio.h>
int main(void) {
    int typeof = 3, asm = 4;
    printf("%d %d\n", typeof, asm); /* expect: 3 4 */
    return 0;
}
