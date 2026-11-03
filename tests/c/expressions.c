#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

static int calls;
static int side(int v) { calls++; return v; }
#define TYPENAME(x) _Generic((x), char: "char", signed char: "schar", unsigned char: "uchar", short: "short", \
    unsigned short: "ushort", int: "int", unsigned: "uint", long: "long", unsigned long: "ulong", long long: "llong", \
    unsigned long long: "ullong", float: "float", double: "double", long double: "ldouble", char *: "char*", \
    const char *: "const char*", int *: "int*", void *: "void*", _Bool: "bool", default: "other")
struct S { char c; double d; int arr[5]; };

int main(void) {
    int a = (side(1), side(2), side(3)); printf("%d %d\n", a, calls);
    calls = 0; int r = side(0) && side(1); printf("%d %d\n", r, calls);
    calls = 0; r = side(1) || side(1); printf("%d %d\n", r, calls);
    calls = 0; r = side(1) && side(0) || side(5); printf("%d %d\n", r, calls);
    int t = 5; long l = 7; double d = 0.5; unsigned u = 1;
    printf("%s %s %s %s\n", TYPENAME(t ? t : l), TYPENAME(t ? d : t), TYPENAME(t ? u : -1), TYPENAME(1 ? (char)1 : (short)2));
    printf("%s %s %s %s %s\n", TYPENAME('a'), TYPENAME((char)'a'), TYPENAME(1.0f), TYPENAME(1.0L), TYPENAME("str"));
    printf("%s %s %s %s\n", TYPENAME(1u + 1L), TYPENAME(1ul + 1ll), TYPENAME((short)1 + (short)1), TYPENAME(1 == 1));
    printf("%s %s %s\n", TYPENAME((void *)0), TYPENAME(&t), TYPENAME((_Bool)1));
    printf("%s %s\n", TYPENAME(-1 > 0u ? 1 : 2), TYPENAME(t ? (void*)0 : &t));
    printf("%d\n", t > 3 ? t < 10 ? 1 : 2 : 3);
    printf("%g\n", t ? 1 : 2.5);
    int *pnull = 0; int *pp = t ? pnull : &t; printf("%d\n", pp == 0);
    printf("%zu %zu %zu %zu %zu %zu\n", sizeof(char), sizeof(short), sizeof(int), sizeof(long), sizeof(long long), sizeof(void *));
    printf("%zu %zu %zu %zu\n", sizeof(float), sizeof(double), sizeof(long double), sizeof(_Bool));
    printf("%zu %zu %zu %zu\n", _Alignof(char), _Alignof(double), _Alignof(long double), _Alignof(struct S));
    printf("%zu %zu %zu\n", sizeof(struct S), sizeof((struct S *)0)->arr, sizeof(((struct S *)0)->arr[0]));
    printf("%zu %zu %zu\n", sizeof "hello", sizeof(int[10]), sizeof(int (*)[10]));
    int arr[7]; printf("%zu %zu\n", sizeof arr, sizeof &arr);
    calls = 0; size_t sz = sizeof(side(1)); printf("%zu %d\n", sz, calls);
    printf("%zu\n", offsetof(struct S, arr[2]));
    int se = ({ int q = 10; q * 2; }); printf("%d\n", se);
    int mx = ({ int _a = 3, _b = 9; _a > _b ? _a : _b; }); printf("%d\n", mx);
    typeof(d) td = 1.25; __typeof__(arr) ta; printf("%g %zu\n", td, sizeof ta);
    typeof(int *) tp = &t; printf("%d\n", *tp);
    printf("%d\n", (a = 4, a += 2, a * 10));
    int b = 5; b = b++ + 1; /* not UB in this order? avoid */ b = 5;
    int c = (b += 3) * 2; printf("%d %d\n", b, c);
    int x = 10, y = 3; printf("%d %d %d %d %d\n", x / y, x % y, -x / y, -x % y, x / -y);
    printf("%d %d %d\n", x << 2 >> 1, x & y | 4, x ^ y & 1);
    printf("%d %d %d\n", x > y == 1, x < y != 0, !x == 0);
    printf("%d\n", 1 + 2 * 3 - 4 / 2 % 3);
    printf("%d\n", -~x);
    printf("%d\n", ~-x);
    unsigned char uc = 0xf0; printf("%d %d\n", ~uc, (unsigned char)~uc);
    int cnt = 0; for (int i = 0; i < 10; ++i) cnt += i & 1 ? i : -i; printf("%d\n", cnt);
    int arr2[3] = {1, 2, 3}; int *ap = arr2; *ap++ += 10; printf("%d %d\n", arr2[0], *ap);
    struct S s = {.c = 'q'}; struct S *sp = &s; printf("%c %c\n", sp->c, (*sp).c);
    printf("%d\n", (int)(char)(x * 100));
    long long big = (long long)x << 40; printf("%lld\n", big);
    printf("%d\n", (x > 5) + (y > 5) * 2);
    double dv = 7; dv /= 2; printf("%g\n", dv);
    printf("%d\n", (int)sizeof(x++)); printf("%d\n", x);
    void *vp = &x; printf("%d\n", *(int *)vp);
    printf("%d\n", __builtin_types_compatible_p(int, signed) + __builtin_types_compatible_p(int, long) * 2);
    printf("%d %d\n", __builtin_constant_p(5), __builtin_constant_p(x));
    int compound = 0; compound += compound++ ? 0 : 0; compound = 1;
    enum { E0, E1 = 5, E2, E3 = E1 + 10 } ev = E2; printf("%d %d %zu\n", ev, E3, sizeof ev);
    printf("%d\n", (int)(uintptr_t)(void*)0);
    const int ci = 42; const int *cip = &ci; printf("%d\n", *cip);
    printf("%d\n", "abc"[1]);
    printf("%c\n", *("xyz" + 2));
    return 0;
}
