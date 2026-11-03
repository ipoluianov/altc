// the size of a VLA typedef must be evaluated when the typedef is declared, not at each use
#include <stdio.h>
int main(void) {
    int n = 4;
    typedef int vt[n];
    n = 100;
    vt x;
    printf("%zu\n", sizeof x); /* expect: 16 */
    return 0;
}
