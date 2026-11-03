// &array has type "pointer to array" (int (*)[N]), not "int *": pointer arithmetic and
// dereference on it are wrong in altc (it behaves like &array[0]).
// Also affects &m[i] (row of a 2D array), &s.a (array member), *&arr, (*&arr)[i].
#include <stdio.h>
struct S { int a[4]; };
int main(void) {
    int arr[5] = {0};
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    struct S s;
    printf("%td\n", (char *)(&arr + 1) - (char *)arr);      /* expect: 20 (altc: 4) */
    printf("%td\n", (char *)(&m[0] + 1) - (char *)m);       /* expect: 12 (altc: 4) */
    printf("%td\n", (char *)(&s.a + 1) - (char *)&s);       /* expect: 16 (altc: 4) */
    printf("%zu %zu\n", sizeof(*&arr), sizeof(*&m[0]));      /* expect: 20 12 (altc: 4 4) */
    int *p = *&arr;
    printf("%d\n", p == arr);                                /* expect: 1 (altc: 0) */
    return 0;
}
