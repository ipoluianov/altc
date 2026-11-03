#include <stdio.h>
#include <math.h>

#define CMP(T, a, b) do { T x = (a), y = (b); \
    printf("%-6s %-5s %-5s: < %d <= %d > %d >= %d == %d != %d | !< %d !> %d | br:", #T, #a, #b, x < y, x <= y, x > y, x >= y, x == y, x != y, !(x < y), !(x > y)); \
    if (x < y) printf("L"); if (x <= y) printf("l"); if (x > y) printf("G"); if (x >= y) printf("g"); if (x == y) printf("E"); if (x != y) printf("N"); \
    if (!(x < y)) printf("a"); if (!(x == y)) printf("b"); if (!(x != y)) printf("c"); if (!(x >= y)) printf("d"); \
    printf(" t:%d%d%d", x < y ? 1 : 0, x == y ? 1 : 0, x != y ? 1 : 0); \
    int w = 0; while (x == y && w < 1) w++; printf(" w%d", w); \
    printf(" && %d || %d", x == y && 1, x != y || 0); \
    printf("\n"); } while (0)

static int truthy_d(double d) { return d ? 1 : 0; }
static int truthy_f(float f) { if (f) return 1; return 0; }
static int not_d(double d) { return !d; }

int main(void) {
    double nan = NAN, inf = INFINITY;
    CMP(double, 1.0, 2.0); CMP(double, 2.0, 1.0); CMP(double, 1.0, 1.0); CMP(double, nan, 1.0); CMP(double, 1.0, nan); CMP(double, nan, nan);
    CMP(double, -0.0, 0.0); CMP(double, inf, inf); CMP(double, -inf, inf);
    CMP(float, 1.0f, 2.0f); CMP(float, (float)nan, 1.0f); CMP(float, (float)nan, (float)nan); CMP(float, 3.0f, 3.0f);
    CMP(long double, 1.0L, 2.0L); CMP(long double, (long double)nan, 1.0L); CMP(long double, 2.0L, 2.0L); CMP(long double, (long double)nan, (long double)nan);
    printf("%d %d %d %d\n", truthy_d(nan), truthy_d(0.0), truthy_d(-0.0), truthy_d(1e-300));
    printf("%d %d %d\n", truthy_f((float)nan), truthy_f(0.0f), truthy_f(-0.0f));
    printf("%d %d %d\n", not_d(nan), not_d(0.0), not_d(2.0));
    _Bool b1 = nan, b2 = -0.0, b3 = (long double)nan; printf("%d %d %d\n", b1, b2, b3);
    printf("%d %d\n", nan && 1, 0.0 || nan);
    long double lnan = nan; printf("%d %d\n", lnan ? 1 : 0, !lnan);
    double vals[] = {3, nan, -1, 2, nan, 5}; double mx = -inf; int cnt = 0;
    for (int i = 0; i < 6; i++) { if (vals[i] > mx) mx = vals[i]; if (vals[i] != vals[i]) cnt++; }
    printf("%g %d\n", mx, cnt);
    printf("%d %d %d\n", isless(nan, 1.0), islessequal(1.0, 1.0), islessgreater(1.0, 2.0));
    return 0;
}
