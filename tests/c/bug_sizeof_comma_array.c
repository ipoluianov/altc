// the result of the comma operator is not an lvalue, so an array operand decays: sizeof(0, arr) == sizeof(int *)
#include <stdio.h>
int main(void) {
    int arr[10];
    printf("%zu\n", sizeof(0, arr)); /* expect: 8 (altc: 40) */
    return 0;
}
