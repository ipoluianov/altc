// local variables with alignment > 16 (_Alignas(64), aligned(32)) are not aligned on the stack
#include <stdio.h>
#include <stdint.h>
static int check(void) {
    char pad = 1; (void)pad;
    _Alignas(64) int a = 2;
    int b __attribute__((aligned(32))) = 3;
    return (((uintptr_t)&a & 63) == 0) + (((uintptr_t)&b & 31) == 0);
}
int main(void) {
    int ok = 0;
    for (int i = 0; i < 4; i++) ok += check();
    printf("%d\n", ok); /* expect: 8 */
    return 0;
}
