// std: c11
#include <stdio.h>
#include <stdalign.h>
#include <stdnoreturn.h>
#include <stdlib.h>
#include <stddef.h>

_Static_assert(sizeof(int) == 4, "int is 4 bytes");
_Static_assert(alignof(double) == 8, "double alignment");
struct anon { int tag; union { int i; float f; }; struct { short a, b; }; };
struct al { char c; _Alignas(16) int x; };
_Alignas(32) static char aligned_buf[64];
static _Noreturn void die(int code) { printf("dying %d\n", code); fflush(stdout); exit(code); }
#define kind(x) _Generic((x), int: 1, long: 2, double: 3, char *: 4, default: 0)
static _Thread_local int tl = 3;

int main(void) {
    struct anon a = {.tag = 1, .i = 5, .a = 7, .b = 8}; printf("%d %d %d %d\n", a.tag, a.i, a.a, a.b);
    a.f = 2.0f; printf("%g\n", a.f);
    printf("%zu %zu %zu\n", sizeof(struct al), offsetof(struct al, x), alignof(struct al));
    printf("%d\n", (int)((size_t)aligned_buf % 32 == 0));
    printf("%d %d %d %d %d\n", kind(1), kind(1L), kind(1.0), kind((char *)0), kind('c'));
    _Static_assert(1, "block scope");
    printf("%d\n", tl);
    alignas(8) int x = 5; printf("%d %zu\n", x, alignof(max_align_t) >= 16);
    char16_like: ;
    printf("%zu %zu\n", sizeof(u"ab"), sizeof(U"ab"));
    printf("%s\n", u8"utf8");
    die(0);
}
