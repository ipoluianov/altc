#include <unistd.h>
__attribute__((noinline)) static void step(volatile long *n) { *n += 1; usleep(1000); }
void inner_loop(void) { volatile long n = 0; for (;;) step(&n); }
