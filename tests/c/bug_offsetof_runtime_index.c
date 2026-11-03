// offsetof with a non-constant array index (GCC extension, common in FAM allocation:
// malloc(offsetof(struct buf, data[n]))) is rejected: "expression is not a compile-time constant"
#include <stdio.h>
#include <stddef.h>
struct buf { int len; char data[]; };
struct S { char c; int arr[10]; };
int main(void) {
    int n = 5, i = 3;
    printf("%zu %zu\n", offsetof(struct buf, data[n]), offsetof(struct S, arr[i])); /* expect: 9 16 */
    return 0;
}
