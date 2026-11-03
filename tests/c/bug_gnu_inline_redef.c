// "extern inline __attribute__((gnu_inline))" provides an inline-only body; a later out-of-line
// definition of the same function is valid GNU C. altc: "error: redefinition of 'f'"
#include <stdio.h>
extern inline __attribute__((gnu_inline)) int f(int x) { return x * 100; }
int f(int x) { return x * 3; }
int main(void) {
    int (*fp)(int) = f;
    printf("%d\n", fp(2)); /* expect: 6 */
    return 0;
}
