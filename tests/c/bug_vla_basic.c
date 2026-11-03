// any VLA declaration crashes the code generator (nil Type in x64 genAssign via cc.(*Type).IsScalar);
// driver.CompileSource then returns a nil *Unit (recover without named result) and the CLI/harness segfault
#include <stdio.h>
int main(void) {
    int n = 3;
    int a[n];
    a[0] = 1;
    printf("%d %zu\n", a[0], sizeof a); /* expect: 1 12 */
    return 0;
}
