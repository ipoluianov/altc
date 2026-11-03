#include <stdio.h>
#include <wchar.h>
#include <uchar.h>
#include <string.h>
struct A { int a[3]; int b; };
struct B { struct A x; struct A y[2]; char s[6]; };
union V { char c[4]; int i; struct { short lo, hi; } p; };
struct F { int n; char tail[]; };
static struct A sa = {.a = {1, 2, 3}, .a[1] = 9};
static struct B sb = {.x.a = {1}, .y[1].b = 5, .y[0] = {.a[2] = 7}, .s = "hi", .x.b = 4};
static union V su = {.p.hi = 0x1234};
static union V su_arr[3] = {[1].i = 0x41424344, [2].c = "xy"};
static struct F sf = {3, "abc"};
static signed char ssc[] = "\xff\x80";
static unsigned char suc[4] = "\xfe";
static wchar_t sw[5] = L"ab";
static char16_t s16[] = u"hé";
static char32_t s32[] = U"z\U0001F600";
static char noterm[3] = "abc";
static struct { char a; char b[3]; int c; } elide = {'x', 'y', 'z', 'w', 99};
static int nested_elide[2][2][2] = {1, 2, 3, 4, 5, 6, 7};
static struct A arr_of_struct[] = {1, 2, 3, 4, [2].b = 8, 5};
int main(void) {
    printf("%d %d %d %d\n", sa.a[0], sa.a[1], sa.a[2], sa.b);
    printf("%d %d %d %d %d %d %s\n", sb.x.a[0], sb.x.b, sb.y[0].a[2], sb.y[0].b, sb.y[1].b, sb.y[1].a[0], sb.s);
    printf("%x %x %d\n", su.p.hi, su.p.lo, su.c[0]);
    printf("%x %s %d\n", su_arr[1].i, su_arr[2].c, su_arr[0].i);
    printf("%d %s\n", sf.n, sf.tail);
    printf("%d %d %zu | %d %d %zu\n", ssc[0], ssc[1], sizeof ssc, suc[0], suc[3], sizeof suc);
    printf("%ls %zu %d\n", sw, sizeof sw / sizeof *sw, (int)sw[4]);
    printf("%d %d %d %zu\n", s16[0], s16[1], s16[2], sizeof s16);
    printf("%d %x %zu\n", (int)s32[0], (unsigned)s32[1], sizeof s32 / sizeof *s32);
    printf("%.3s %zu\n", noterm, sizeof noterm);
    printf("%c %c%c%c %d\n", elide.a, elide.b[0], elide.b[1], elide.b[2], elide.c);
    for (int i = 0; i < 8; i++) printf("%d", ((int *)nested_elide)[i]); printf("\n");
    printf("%zu %d %d %d %d\n", sizeof arr_of_struct / sizeof *arr_of_struct, arr_of_struct[0].b, arr_of_struct[1].a[0], arr_of_struct[2].b, arr_of_struct[3].a[0]);
    struct A la = {.a = {1, 2, 3}, .a[1] = 9}; printf("%d %d %d\n", la.a[0], la.a[1], la.a[2]);
    struct B lb = {.x.a = {1}, .y[1].b = 5, .y[0] = {.a[2] = 7}, .s = "hi", .x.b = 4}; printf("%d %d %d %d %s\n", lb.x.a[0], lb.x.b, lb.y[0].a[2], lb.y[1].b, lb.s);
    union V lu = {.p.hi = 0x1234}; printf("%x %x\n", lu.p.hi, lu.p.lo);
    signed char lsc[] = "\xff"; unsigned char luc[] = "\xff"; printf("%d %d\n", lsc[0], luc[0]);
    char16_t l16[] = u"x"; printf("%zu\n", sizeof l16);
    struct { char a; char b[3]; int c; } lel = {'x', 'y', 'z', 'w', 99}; printf("%c %d\n", lel.b[2], lel.c);
    int lne[2][2][2] = {1, 2, 3, 4, 5, 6, 7}; printf("%d %d\n", lne[1][1][0], lne[1][1][1]);
    struct A override = {.a[0] = 1, .a = {5}}; printf("%d %d\n", override.a[0], override.a[1]);
    return 0;
}
