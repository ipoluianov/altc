#include <stdio.h>
inline int c99_inline(int x) { return x + 1; }
extern int c99_inline(int x);
static inline int st_inline(int x) { return x * 2; }
__attribute__((always_inline)) static inline int ai(int x) { return x - 1; }
static inline int recursive_inline(int n) { return n <= 1 ? 1 : n * recursive_inline(n - 1); }
int main(void) {
    int (*fp)(int) = c99_inline;
    return 0;
}
