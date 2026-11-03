// label-address differences (&&a - &&b) in a static initializer are rejected:
// "error: label address is not a constant here"
#include <stdio.h>
static int run(int k) {
    static const int offs[] = {&&L0 - &&L0, &&L1 - &&L0};
    goto *(&&L0 + offs[k]);
L0: return 10;
L1: return 20;
}
int main(void) { printf("%d %d\n", run(0), run(1)); return 0; } /* expect: 10 20 */
