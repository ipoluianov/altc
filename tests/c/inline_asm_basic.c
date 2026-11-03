#include <stdio.h>
extern int renamed_puts(const char *) __asm__("puts");
int main(void) {
    __asm__ __volatile__("" ::: "memory");
    asm volatile("nop");
    renamed_puts("asm label");

    return 0;
}
