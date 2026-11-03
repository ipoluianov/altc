#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
struct S { char c; int i; double d[3]; struct { short s; char t[5]; } in; };
enum { E1 = (int)1.9, E2 = (int)-2.7, E3 = sizeof(struct S), E4 = offsetof(struct S, in.t[3]), E5 = (char)300, E6 = (unsigned char)-1,
       E7 = 10 / 3 * 3, E8 = -10 % 3, E9 = (1 ? 5 : 6) + (0 ? 7 : 8), E10 = !0 + !5 + ~0, E11 = (int)(0.5 + 0.5) , E12 = (short)0x18000 , E13 = 1.0 < 2.0, E14 = 'z' - 'a',
       E15 = (int)(unsigned char)(-1) << 2, E16 = sizeof(int[3][4]) / sizeof(int), E17 = (int)(long long)1e10 % 7, E18 = (_Bool)0.5, E19 = (int)sizeof("abc" "de"), E20 = INT_MIN / -INT_MAX };
static int arr_size[(int)2.5 + E1];
_Static_assert(offsetof(struct S, d) == 8, "off");
_Static_assert(sizeof(struct S) == 40, "size");
_Static_assert((int)3.7 == 3 && (unsigned)-1 == UINT_MAX && (signed char)200 == -56, "casts");
_Static_assert(1.5 > 1, "float cmp");
_Static_assert((uintptr_t)0 == 0, "ptr");
_Static_assert(__builtin_offsetof(struct S, in.s) == 32, "builtin off");
static const int ci = 5;
static char from_cond[1 ? 3 : 5];
static long long_consts[] = {LONG_MAX, LONG_MIN, -LONG_MAX, (long)ULONG_MAX, (long)(unsigned)-1};
static unsigned long long ull_consts[] = {-1, (unsigned long long)-1LL >> 1, 1ULL << 63 | 1, (unsigned long long)(int)-5};
static int shift_consts[] = {(int)(1u << 31) >> 31, -1 >> 1, (int)(0xffffffffu >> 1)};
static double d_consts[] = {1 / 2, 1 / 2.0, (double)(1 << 30) * 4, 1e308 * 1e-308, -0.0 * 1, (float)0.1};
static int ptr_diff_const = (int)(&((struct S *)0)->in.t[4] - &((struct S *)0)->in.t[0]);
int main(void) {
    printf("%d %d %d %d %d %d %d %d %d %d\n", E1, E2, E3, E4, E5, E6, E7, E8, E9, E10);
    printf("%d %d %d %d %d %d %d %d %d %d\n", E11, E12, E13, E14, E15, E16, E17, E18, E19, E20);
    printf("%zu %zu %d\n", sizeof arr_size, sizeof from_cond, ci);
    for (int i = 0; i < 5; i++) printf("%ld ", long_consts[i]); printf("\n");
    for (int i = 0; i < 4; i++) printf("%llx ", ull_consts[i]); printf("\n");
    for (int i = 0; i < 3; i++) printf("%d ", shift_consts[i]); printf("\n");
    for (int i = 0; i < 6; i++) printf("%.17g ", d_consts[i]); printf("\n");
    printf("%d\n", ptr_diff_const);
    return 0;
}
