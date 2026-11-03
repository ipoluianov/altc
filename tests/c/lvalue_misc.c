#include <stdio.h>
struct S { int a[3]; int x; };
int main(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    (*&arr)[3] = 40; printf("%d\n", arr[3]);
    int *q = &*arr; printf("%d\n", *q);
    struct S s = {{1, 2, 3}, 4}; (*&s).x = 9; (&s)->a[1] = 8; printf("%d %d\n", s.x, s.a[1]);
    int x = 1; *&*&x = 7; printf("%d\n", x);
    struct S *ps = &s; (*ps).a[2]++; printf("%d\n", ps->a[2]);
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}}; printf("%d %d\n", (*&m)[1][2], *(*(&m[0] + 1) + 1));
    int (*pm)[3] = *&m; printf("%d\n", pm[1][0]);
    char str[] = "hello"; printf("%c %s\n", (*&str)[1], *&str);
    printf("%d\n", &(*&arr)[2] == &arr[2]);
    return 0;
}
