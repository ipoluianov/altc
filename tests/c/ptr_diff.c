#include <stdio.h>
#include <stddef.h>
struct S12 { int a, b, c; };
struct S24 { double d; int x[4]; };
struct S40 { char c[40]; };
struct S7 { char c[7]; };
int main(void) {
    struct S12 a12[10]; struct S24 a24[10]; struct S40 a40[10]; struct S7 a7[10];
    printf("%td %td %td %td\n", &a12[7] - &a12[2], &a24[1] - &a24[9], &a40[9] - &a40[0], &a7[3] - &a7[8]);
    struct S12 *p = a12 + 5; signed char sc = -2; short sh = -3; long long ll = -4; unsigned char uc = 2; int neg = -1;
    printf("%td %td %td %td %td\n", &p[sc] - a12, &p[sh] - a12, &p[ll] - a12, &p[uc] - a12, &p[neg] - a12);
    long arr[100]; long *q = arr + 50; unsigned u = 10; long long big = 40;
    printf("%td %td %td\n", (q + u) - arr, (q - u) - arr, (q - big) - arr);
    char *c1 = (char *)arr, *c2 = (char *)&arr[99]; printf("%td %td\n", c2 - c1, c1 - c2);
    ptrdiff_t d = &a7[0] - &a7[9]; printf("%td %zu\n", d, (size_t)(&a7[9] - &a7[0]) * sizeof(struct S7));
    int i = 3; printf("%td\n", &a12[i] - &a12[i * 2]);
    printf("%d %d\n", &a40[3] > &a40[2], &a7[1] <= &a7[0]);
    return 0;
}
