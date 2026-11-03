// std: gnu89
// In -std=gnu89/c89 a K&R definition that follows an unprototyped declaration "int sum();"
// is rejected: "error: too many arguments to function" (works with -std=gnu17)
#include <stdio.h>
int sum();
int main(void) { printf("%d\n", sum(1, 2, 3)); return 0; } /* expect: 6 */
int sum(a, b, c) int a, b, c; { return a + b + c; }
