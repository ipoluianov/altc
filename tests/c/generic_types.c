#include <stdio.h>
#define T(x) _Generic((x), \
    int: "int", unsigned: "unsigned", long: "long", char: "char", const int: "const int", \
    int *: "int*", const int *: "const int*", void *: "void*", const void *: "const void*", \
    int (*)[3]: "int(*)[3]", char (*)[3]: "char(*)[3]", const char *: "const char*", char *: "char*", \
    int (*)(void): "fn ptr", default: "other")
enum E { EA, EB };
enum N { NA = -1 };
static int f(void) { return 0; }
int main(void) {
    const int ci = 1; int arr[3]; int *ip = arr; const int *cip = &ci; void *vp = ip; const void *cvp = cip;
    puts(T(ci)); puts(T(+ci)); puts(T(arr)); puts(T(&arr)); puts(T(&"ab")); puts(T("ab"));
    puts(T(1 ? ip : vp)); puts(T(1 ? ip : cip)); puts(T(1 ? vp : cvp)); puts(T(1 ? ip : 0));
    puts(T(EA)); puts(T((enum E)0)); puts(T(f)); puts(T(&f)); puts(T(*f));
    enum E e = EB; puts(T(e)); puts(T(e + 0));
    printf("%d %d\n", (int)sizeof(enum E), (enum N)NA < 0);
    enum E ue = (enum E)-1; printf("%d\n", ue > 0);
    puts(T((char)1)); puts(T((char)1 + 1)); puts(T(ci ? 'a' : 'b'));
    int *restrict rp = ip; puts(T(rp));
    volatile int vi = 0; puts(T(vi));
    const int *const *cpp = &cip; puts(T(*cpp));
    return 0;
}
