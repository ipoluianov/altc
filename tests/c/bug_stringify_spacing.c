// whitespace from an empty macro expansion between tokens is lost when stringifying:
// gcc gives "A_ ()", altc "A_()"
#include <stdio.h>
#define VSTR(...) #__VA_ARGS__
#define XSTR(...) VSTR(__VA_ARGS__)
#define EMPTY
#define DEFER(m) m EMPTY
int main(void) {
    puts(XSTR(DEFER(A_)())); /* expect: A_ () */
    return 0;
}
