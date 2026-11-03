// unsigned __int128 arithmetic: 64x64->128 multiply, shifts by 64, carries
#include <stdio.h>
int main(void) {
    unsigned long long x = 0xffffffffffffffffULL;
    unsigned __int128 w = (unsigned __int128)x * x;
    printf("%llx %llx\n", (unsigned long long)(w >> 64), (unsigned long long)w); /* fffffffffffffffe 1 */
    unsigned __int128 z = x; z = z + 1;
    printf("%llx %llx\n", (unsigned long long)(z >> 64), (unsigned long long)z); /* 1 0 */
    return 0;
}
