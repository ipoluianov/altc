// #x applied to a string literal or char constant: backslashes/quotes are escaped twice
#include <stdio.h>
#define S(x) #x
int main(void) {
    puts(S("a\n\"b"));   /* expect: "a\n\"b"   */
    puts(S('\xff'));     /* expect: '\xff'     */
    return 0;
}
