// None of the predefined macros from cc/pp.go definePredefined() are defined:
// __STDC__, __STDC_VERSION__, __x86_64__, __CHAR_BIT__, __SIZEOF_LONG__, linux ... (altc -E leaves them unexpanded)
#include <stdio.h>
int main(void) {
#ifdef __STDC__
    puts("defined");
#endif
    printf("%d %d %d\n", __STDC__, __x86_64__, __CHAR_BIT__); /* expect: defined / 1 1 8 */
    return 0;
}
