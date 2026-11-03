// __attribute__((cleanup(fn))) is silently ignored: the cleanup function is never called at scope exit
#include <stdio.h>
static void done(int *p) { printf("cleanup %d\n", *p); }
int main(void) {
    {
        __attribute__((cleanup(done))) int guard = 42;
        printf("in scope\n");
    }
    for (int i = 0; i < 2; i++) { int v __attribute__((cleanup(done))) = i; if (i == 1) break; }
    printf("after\n"); /* expect: in scope / cleanup 42 / cleanup 0 / cleanup 1 / after */
    return 0;
}
