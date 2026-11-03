// static array of label addresses (classic computed-goto dispatch table) is rejected:
// "error: label address in a static initializer is not supported"
#include <stdio.h>
static int run(int x) {
    static void *tbl[] = {&&L0, &&L1};
    goto *tbl[x];
L0: return 10;
L1: return 20;
}
int main(void) { printf("%d %d\n", run(0), run(1)); return 0; } /* expect: 10 20 */
