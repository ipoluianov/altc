// typeof(*fnptr) must be the function type, so typeof(*fnptr) *p is a function pointer:
// altc: "error: called object is not a function or function pointer"
#include <stdio.h>
static int two(void) { return 2; }
int (*fptr)(void) = two;
int main(void) {
  __typeof__(*fptr) *a = two;
  printf("%d\n", a()); /* expect: 2 */
  return 0;
}
