// std: c2x
#include <stdio.h>
#include <stddef.h>

static_assert(sizeof(long) == 8);
constexpr int CN = 10;
enum small : unsigned char { S0, S1 = 200 };
[[nodiscard]] static int twice(int x) { return x * 2; }
[[maybe_unused]] static int unused_fn(void) { return 0; }
[[deprecated]] static int old(void) { return 1; }
static int f(int [[maybe_unused]] x) { return 1; }

int main(void) {
    bool b = true; printf("%d %d %d\n", b, false, (int)sizeof(true));
    int *p = nullptr; printf("%d\n", p == nullptr);
    typeof(1.5) d = 2.5; typeof_unqual(const int) u = 3; u++; printf("%g %d\n", d, u);
    int arr[CN]; printf("%zu\n", sizeof arr / sizeof arr[0]);
    long big = 1'000'000; unsigned bin = 0b1010'1010; printf("%ld %u\n", big, bin);
    struct { int a, b; } s = {}; int z[4] = {}; printf("%d %d %d\n", s.a, s.b, z[3]);
    auto inferred = 3.5f; printf("%zu\n", sizeof inferred);
    printf("%d\n", twice(4) + old() + f(1));
    char8_t_like: ;
    unsigned char c8 = u8'a'; printf("%d\n", c8);
    printf("%zu %d\n", sizeof(enum small), S1);
#ifdef __STDC_VERSION__
    printf("has version\n");
#endif
#define FOO 1
#ifdef BAR
    printf("bar\n");
#elifdef FOO
    printf("elifdef works\n");
#endif

    constexpr double cd = 1.5; printf("%g\n", cd * 2);
    static_assert(CN == 10, "msg");
    nullptr_t np = nullptr; printf("%d\n", np == nullptr);
    return 0;
}
