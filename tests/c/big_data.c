#include <stdio.h>
#include <string.h>
static char big_bss[64 << 20];
static long big_bss2[1 << 20];
#define R1(x) x, x + 1, x + 2, x + 3
#define R2(x) R1(x), R1(x + 4), R1(x + 8), R1(x + 12)
#define R3(x) R2(x), R2(x + 16), R2(x + 32), R2(x + 48)
#define R4(x) R3(x), R3(x + 64), R3(x + 128), R3(x + 192)
#define R5(x) R4(x), R4(x + 256), R4(x + 512), R4(x + 768)
#define R6(x) R5(x), R5(x + 1024), R5(x + 2048), R5(x + 3072)
static const int table[] = {R6(0), R6(4096), R6(8192)};
static double dtable[] = {R5(0.5)};
int main(void) {
    big_bss[(64 << 20) - 1] = 7; big_bss2[(1 << 20) - 1] = 9;
    printf("%d %ld\n", big_bss[(64 << 20) - 1] + big_bss[0], big_bss2[(1 << 20) - 1]);
    long s = 0; for (size_t i = 0; i < sizeof table / sizeof *table; i++) s += table[i];
    printf("%zu %ld %d\n", sizeof table / sizeof *table, s, table[12287]);
    double ds = 0; for (size_t i = 0; i < sizeof dtable / sizeof *dtable; i++) ds += dtable[i]; printf("%g\n", ds);
    char local_big[1 << 20]; memset(local_big, 3, sizeof local_big); printf("%d\n", local_big[12345] + local_big[(1 << 20) - 1]);
    return 0;
}
