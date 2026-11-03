// std: c99
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

static inline int sq(int x) { return x * x; }
struct pt { int x, y; };
static int sum(const int *restrict a, int n) { int s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
static void arr_param(int a[static 3]) { printf("%d\n", a[2]); }
static void qual_param(int a[const restrict 2]) { printf("%d\n", a[1]); }

int main(void) {
    for (int i = 0; i < 3; i++) printf("%d ", sq(i));
    printf("\n");
    bool b = true; printf("%d\n", b);
    long long ll = 1LL << 50; printf("%lld\n", ll);
    struct pt p = {.y = 2, .x = 1}; printf("%d %d\n", p.x, p.y);
    int a[] = {1, 2, 3, 4}; printf("%d\n", sum(a, 4));
    arr_param(a); qual_param(a);
    printf("%s\n", __func__);
    // line comment
    int64_t i64 = INT64_C(-5); uint32_t u32 = UINT32_C(7); printf("%lld %u\n", (long long)i64, u32);
    printf("%d\n", (int)sizeof((struct pt){1, 2}));
    _Bool bb = 3; printf("%d\n", bb);
    printf("%.1f %.1f\n", 0x1p4, 0x.8p0);
    return 0;
}
