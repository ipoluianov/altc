// "a ## __VA_OPT__(_tail)" must paste a with the first token of the __VA_OPT__ replacement
#include <stdio.h>
#define VSTR(...) #__VA_ARGS__
#define XSTR(...) VSTR(__VA_ARGS__)
#define CAT(a, ...) a ## __VA_OPT__(_tail)
int main(void) {
    puts(XSTR(CAT(x, y))); /* expect: x_tail  (got: x _tail) */
    return 0;
}
