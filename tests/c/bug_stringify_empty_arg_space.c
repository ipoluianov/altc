// C11 6.10.3.5p5: an empty macro argument must not leave a space in the stringified result:
// "p() i[q()]" must stringify as "int i[]" (altc: "int i[ ]")
#include <stdio.h>
#define VSTR(...) #__VA_ARGS__
#define XSTR(...) VSTR(__VA_ARGS__)
#define p() int
#define q(x) x
int main(void) {
    puts(XSTR(p() i[q()] = { q(1) };)); /* expect: int i[] = { 1 }; */
    return 0;
}
