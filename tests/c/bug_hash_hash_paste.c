// C11 6.10.3.3p4 example: "# ## #" in an object-like macro must paste into a single "##" token
#include <stdio.h>
#define hash_hash # ## #
#define mkstr(a) # a
#define in_between(a) mkstr(a)
#define join(c, d) in_between(c hash_hash d)
int main(void) {
    puts(join(x, y)); /* expect: x ## y   (got: x # ## # y) */
    return 0;
}
