#include <stdio.h>
#include <stdlib.h>
static void last(void) { printf("atexit runs, exiting with 3\n"); }
static void deep(int n) { if (n == 0) { fflush(stdout); exit(3); } deep(n - 1); }
int main(int argc, char **argv) {
    printf("argc=%d argv0_nonnull=%d argv[argc]=%p\n", argc, argv[0] != NULL, (void *)argv[argc]);
    atexit(last);
    deep(10);
    return 0;
}
