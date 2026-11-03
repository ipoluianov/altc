#include <stdio.h>
#include <string.h>

struct pt { int x, y; };
struct line { struct pt a, b; int color; };
struct mixed { char c; int arr[3]; struct pt p; const char *s; double d; };
union un { int i; char c[8]; double d; };

int gi = 5;
int garr[5] = {1, 2};
int *gp = &gi;
int *gpa = &garr[3];
int *gpb = garr + 1;
char gstr[] = "global";
char gstr2[10] = "abc";
char gstr3[3] = "abc";
const char *gstrs[] = {"one", "two", "three", NULL};
char *gs = "lit" + 1;
struct pt gpt = {.y = 9};
struct line gline = {{1, 2}, {3, 4}, 5};
struct line gline2 = {1, 2, 3, 4, 5};
struct line gline3 = {.b.y = 7, .a = {8}, .color = 1};
int gmat[3][3] = {[0][0] = 1, [1][1] = 1, [2][2] = 1};
int grange[10] = {[2 ... 5] = 7, [8] = 1};
static int sint = 3;
static int *sptr = &sint;
long gaddr_diff = (long)(sizeof gstr);
static struct pt *gptp = &gpt;
static int *gelem = &gmat[1][1];
int (*gfn)(const char *) = puts;
static int add1(int x) { return x + 1; }
static int dbl(int x) { return x * 2; }
static int (*ftab[])(int) = {add1, dbl, add1};
static struct { const char *name; int (*fn)(int); } ops[] = {{"add1", add1}, {"dbl", dbl}};
struct pt *gcl = &(struct pt){10, 20};
int *gcla = (int[]){3, 2, 1};
union un gun = {.d = 1.5};
union un gun2 = {65};
char *gptrs[3] = {gstr, gstr2 + 1, 0};
struct mixed gm = {'m', {1, 2, 3}, {4, 5}, "str", 6.5};
struct mixed gm2 = {'n', 1, 2, 3, 4, 5, "s2", 7.5};
_Bool gb = 2;
float gf = 1 / 3.0;
double gd = 'A';
long long gll = 1ULL << 63 >> 1;
unsigned char guc = 300;
int gsum = sizeof(int) * 3 + (2 > 1) + (5 ? 3 : 4);
const int gci = 10;
char gnest[2][3][4] = {{"ab", "cd"}, {"ef"}};
struct { int a; struct { int b; int c[2]; } in; } gnested = {1, {2, {3, 4}}};
struct { int n; int data[]; } gflex = {3, {7, 8, 9}};
int gtent;
int gtent;

int main(void) {
    printf("%d %d %d %d %d\n", gi, garr[0], garr[1], garr[2], garr[4]);
    printf("%d %d %d\n", *gp, *gpa, *gpb);
    printf("%s %zu %s %zu %.3s %s\n", gstr, sizeof gstr, gstr2, sizeof gstr2, gstr3, gs);
    for (const char **p = gstrs; *p; p++) printf("%s ", *p); printf("\n");
    printf("%d %d | %d %d %d %d %d | %d %d %d %d %d | %d %d %d\n", gpt.x, gpt.y, gline.a.x, gline.a.y, gline.b.x, gline.b.y, gline.color,
           gline2.a.x, gline2.a.y, gline2.b.x, gline2.b.y, gline2.color, gline3.a.x, gline3.b.y, gline3.color);
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) printf("%d", gmat[i][j]); printf("\n");
    for (int i = 0; i < 10; i++) printf("%d", grange[i]); printf("\n");
    printf("%d %ld %d %d\n", *sptr, gaddr_diff, gptp->y, *gelem);
    gfn("via fnptr");
    printf("%d %d %d\n", ftab[0](1), ftab[1](5), ftab[2](9));
    for (int i = 0; i < 2; i++) printf("%s=%d ", ops[i].name, ops[i].fn(10)); printf("\n");
    printf("%d %d %d %d\n", gcl->x, gcl->y, gcla[0], gcla[2]);
    printf("%g %d\n", gun.d, gun2.c[0]);
    printf("%s %s %p\n", gptrs[0], gptrs[1], (void *)gptrs[2]);
    printf("%c %d %d %d %s %g\n", gm.c, gm.arr[2], gm.p.x, gm.p.y, gm.s, gm.d);
    printf("%c %d %d %d %s %g\n", gm2.c, gm2.arr[2], gm2.p.x, gm2.p.y, gm2.s, gm2.d);
    printf("%d %.6f %g %lld %d %d %d\n", gb, gf, gd, gll, guc, gsum, gci);
    printf("%s %s %s %d\n", gnest[0][0], gnest[0][1], gnest[1][0], gnest[1][2][0]);
    printf("%d %d %d %d\n", gnested.a, gnested.in.b, gnested.in.c[0], gnested.in.c[1]);
    printf("%d %d %d\n", gflex.n, gflex.data[0], gflex.data[2]);
    printf("%d\n", gtent);

    /* locals */
    int la[6] = {1, [4] = 5, 6};
    for (int i = 0; i < 6; i++) printf("%d ", la[i]); printf("\n");
    int lb[] = {[3] = 1, [1] = 2}; printf("%zu %d %d\n", sizeof lb / sizeof *lb, lb[0], lb[1]);
    char lc[10] = "hi"; printf("%s %d %d\n", lc, lc[2], lc[9]);
    char ld[] = {"braced"}; printf("%s %zu\n", ld, sizeof ld);
    char le[2][5] = {"abcd", "efg"}; printf("%s %s\n", le[0], le[1]);
    struct pt lp[] = {1, 2, 3, 4, 5}; printf("%zu %d %d\n", sizeof lp / sizeof *lp, lp[2].x, lp[2].y);
    struct line ll = {.b = {.x = 3}, .a.y = 2}; printf("%d %d %d %d %d\n", ll.a.x, ll.a.y, ll.b.x, ll.b.y, ll.color);
    struct mixed lm = {.s = "x", .arr[1] = 4, .c = 'z'}; printf("%c %d %d %d %s %g\n", lm.c, lm.arr[0], lm.arr[1], lm.p.x, lm.s, lm.d);
    struct mixed zero = {0}; printf("%d %d %p %g\n", zero.c, zero.arr[2], (void *)zero.s, zero.d);
    int lz[100] = {0}; int nz = 0; for (int i = 0; i < 100; i++) nz += lz[i] != 0; printf("%d\n", nz);
    int lr[8] = {[1 ... 3] = 9, [6 ... 7] = 2}; for (int i = 0; i < 8; i++) printf("%d", lr[i]); printf("\n");
    union un lu = {.c = {'a', 'b'}}; printf("%s\n", lu.c);
    struct pt *cl = &(struct pt){.y = 4}; cl->x = 3; printf("%d %d\n", cl->x, cl->y);
    int *ca = (int[5]){1, 2, 3}; printf("%d %d\n", ca[2], ca[4]);
    const char *cs = (const char[]){"compound"}; printf("%s\n", cs);
    int v = 10; int dyn[3] = {v, v * 2, v * 3}; printf("%d %d\n", dyn[1], dyn[2]);
    struct pt dp = {v + 1, v - 1}; printf("%d %d\n", dp.x, dp.y);
    struct line over = {.a.x = 1, .a.x = 2}; printf("%d\n", over.a.x);
    struct { int a, b, c; } partial = {.b = 5, 6}; printf("%d %d %d\n", partial.a, partial.b, partial.c);
    int arr2[2][3] = {1, 2, 3, 4}; printf("%d %d %d\n", arr2[1][0], arr2[1][1], arr2[1][2]);
    int arr3[2][3] = {{1}, {4}}; printf("%d %d %d\n", arr3[0][1], arr3[1][0], arr3[1][2]);
    static int st_local = 5; static int *st_lp = &st_local; static const char *st_s = "static"; printf("%d %s\n", *st_lp, st_s);
    static int st_arr[3] = {[2] = 1}; printf("%d\n", st_arr[2]);
    wchar_t_like: ;
    char big[300] = "x"; printf("%d %d\n", big[0], big[299]);
    double dd[3] = {1, 2.5f, 'c'}; printf("%g %g %g\n", dd[0], dd[1], dd[2]);
    struct pt nested_cl[2] = {(struct pt){1, 2}, (struct pt){3, 4}}; printf("%d\n", nested_cl[1].x);
    struct pt cp_init = gpt; printf("%d\n", cp_init.y);
    return 0;
}
