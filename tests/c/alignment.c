#include <stdio.h>
#include <stddef.h>
#include <stdalign.h>
struct S { char c; int arr[10]; double d; };
struct Q { char c; __int128 q; };
struct LDA { char c; long double ld; };
struct ALN { char c; alignas(32) char x; };
struct NestedAl { char c; struct ALN in; };
int main(void) {
    struct S s; int i = 3;
    printf("%zu %zu %zu\n", __alignof__(s.c), __alignof__(s.d), _Alignof(s.arr));
    printf("%zu %zu\n", offsetof(struct S, arr[3]), __builtin_offsetof(struct S, arr[4]));
    printf("%zu %zu %zu %zu\n", sizeof(struct Q), alignof(struct Q), sizeof(struct LDA), alignof(struct LDA));
    printf("%zu %zu %zu %zu\n", sizeof(struct ALN), alignof(struct ALN), sizeof(struct NestedAl), offsetof(struct NestedAl, in));
    printf("%zu %zu %zu\n", alignof(__int128), alignof(long double), alignof(max_align_t));
    struct ALN arr[2]; printf("%td\n", (char *)&arr[1] - (char *)&arr[0]);
    printf("%zu\n", __alignof__(int[3]));
    return 0;
}
