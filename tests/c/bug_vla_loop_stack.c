// VLA storage is not released at the end of each loop iteration (or on a backward goto):
// a VLA inside a long loop exhausts the stack and the program segfaults
#include <stdio.h>
#include <string.h>
int main(void) {
    long total = 0;
    for (int i = 0; i < 200000; i++) { char buf[1000 + i % 7]; memset(buf, i & 0x7f, sizeof buf); total += buf[999]; }
    printf("%ld\n", total); /* expect: 12697952 */
    return 0;
}
