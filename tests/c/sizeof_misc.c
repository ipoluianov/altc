#include <stdio.h>
static int fn(void) { return 0; }
int main(void) {
    int arr[10]; char carr[3]; struct { char c[5]; } s;
    printf("%zu %zu %zu\n", sizeof(arr + 0), sizeof(&arr), sizeof(&arr[0]));
    printf("%zu %zu\n", sizeof(1 ? arr : arr), sizeof(1 ? carr : 0));
    printf("%zu %zu %zu\n", sizeof fn, sizeof &fn, sizeof(fn()));
    printf("%zu %zu %zu\n", sizeof(s.c), sizeof((0, s).c), sizeof(+carr[0]));
    printf("%zu %zu %zu\n", sizeof(carr[0] + carr[1]), sizeof(-carr[0]), sizeof(!arr));
    printf("%zu %zu\n", sizeof((char)1 + (char)1), sizeof((short)1 << 1L));
    printf("%zu %zu %zu\n", sizeof('a'), sizeof(L"ab"), sizeof(u8"ab"));
    printf("%zu %zu\n", sizeof(_Generic(1, int: arr, default: carr)), sizeof((int[]){1, 2}));
    printf("%zu %zu\n", sizeof(arr) / sizeof(arr[0]), sizeof arr[0] * 2);
    printf("%zu\n", sizeof(struct { int a : 3; char b; }));
    printf("%zu %zu\n", sizeof(int (*)[]), sizeof(char[2][3][4]));
    long double ld; printf("%zu %zu\n", sizeof(ld + 1), sizeof(1.0f + 1));
    _Bool b; printf("%zu %zu\n", sizeof b, sizeof(b + b));
    printf("%zu\n", sizeof(s = s));
    int i = 0; printf("%zu %zu %d\n", sizeof(i = 5), sizeof(i++), i);
    printf("%zu\n", sizeof((void)0, 'x'));
    return 0;
}
