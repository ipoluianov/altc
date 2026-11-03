#include <stdio.h>
#include <stddef.h>

int sum2d(int rows, int (*a)[4]) { int s = 0; for (int i = 0; i < rows; i++) for (int j = 0; j < 4; j++) s += a[i][j] * (i + 1); return s; }
int sum_ptrs(int **pp, int n) { int s = 0; for (int i = 0; i < n; i++) s += *pp[i]; return s; }
static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }
typedef int (*binop)(int, int);
binop pick(int k) { static binop t[] = {add, sub, mul}; return t[k % 3]; }
int apply(int (*f)(int, int), int x, int y) { return f(x, y); }
int (*getarr(void))[3] { static int arr[2][3] = {{1, 2, 3}, {4, 5, 6}}; return arr; }
void fill(char *p, size_t n, char c) { while (n--) *p++ = c++; }

int main(void) {
    int a[10]; for (int i = 0; i < 10; i++) a[i] = i * i;
    int *p = a, *q = &a[9];
    printf("%td %d %d %d\n", q - p, *(p + 3), p[4], 5[a]);
    printf("%d %d %d %d\n", p < q, p >= q, p == a, q != a + 9);
    p += 2; q -= 2; printf("%d %d %td\n", *p, *q, q - p);
    { int v1 = *p++; int v2 = *++p; printf("%d %d\n", v1, v2); }
    { int v1 = *p--; int v2 = *--p; printf("%d %d\n", v1, v2); }
    int m[3][4]; for (int i = 0; i < 3; i++) for (int j = 0; j < 4; j++) m[i][j] = i * 10 + j;
    printf("%d %d %d\n", m[2][3], *(*(m + 1) + 2), (*m)[3]);
    printf("%zu %zu %zu\n", sizeof m, sizeof m[0], sizeof m[0][0]);
    printf("%d\n", sum2d(3, m));
    int (*pm)[4] = m; pm++; printf("%d %d\n", (*pm)[0], pm[1][1]);
    printf("%td\n", (char *)&m[2][0] - (char *)&m[0][0]);
    int x = 1, y = 2, z = 3; int *ps[] = {&x, &y, &z}; printf("%d\n", sum_ptrs(ps, 3));
    int **pp = ps; **pp = 100; *pp[2] = 300; printf("%d %d %d\n", x, y, z);
    for (int k = 0; k < 5; k++) printf("%d ", pick(k)(7, 3)); printf("\n");
    printf("%d %d\n", apply(add, 1, 2), apply(&mul, 4, 5));
    binop fp = sub; printf("%d %d\n", fp(9, 4), (*fp)(9, 4));
    int (*arr)[3] = getarr(); printf("%d %d\n", arr[1][2], arr[0][1]);
    char buf[8]; fill(buf, 7, 'a'); buf[7] = 0; printf("%s\n", buf);
    char *s = "hello"; printf("%c %c %s\n", *s, s[4], s + 2);
    const char *names[] = {"zero", "one", "two", "three"}; printf("%s %c %zu\n", names[2], names[3][1], sizeof names);
    char grid[3][6] = {"abc", "defgh", "ij"}; printf("%s %s %s %d\n", grid[0], grid[1], grid[2], grid[2][4]);
    int cube[2][3][4]; int n = 0; for (int i = 0; i < 2; i++) for (int j = 0; j < 3; j++) for (int k = 0; k < 4; k++) cube[i][j][k] = n++;
    printf("%d %d %zu\n", cube[1][2][3], cube[1][0][2], sizeof cube[1]);
    int (*pc)[3][4] = cube; printf("%d\n", pc[1][1][1]);
    void *vp = a; int *ip = vp; printf("%d\n", ip[3]);
    unsigned char *bp = (unsigned char *)&a[1]; printf("%d %d\n", bp[0], bp[1]);
    int *np = NULL; printf("%d %d\n", np == 0, !np);
    ptrdiff_t pd = &a[7] - &a[2]; printf("%td\n", pd);
    struct { int a; char b[5]; double c; } st[3]; printf("%td\n", &st[2].c - &st[0].c);
    printf("%td\n", (char*)&st[1] - (char*)&st[0]);
    int *ap[3][2]; printf("%zu\n", sizeof ap);
    int (*fa[2])(int, int) = {add, sub}; printf("%d\n", fa[1](10, 1));
    long long lla[4] = {1, 2, 3, 4}; long long *lp = lla + 4; printf("%lld\n", lp[-1] + *(lp - 4));
    const int ca[] = {5, 6, 7}; const int *cp = ca; printf("%d\n", cp[1]);
    int idx = 2; { int v1 = a[idx++]; printf("%d %d\n", v1, a[idx]); }
    char str[] = "abcdef"; char *e = str + 6; while (e > str) putchar(*--e); putchar('\n');
    short sa[5] = {1, 2, 3, 4, 5}; short *sp = sa; sp += 3; printf("%d %td\n", *sp, sp - sa);
    double da[3] = {1.5, 2.5, 3.5}; double *dp = &da[2]; printf("%f %f\n", *dp, dp[-2]);
    return 0;
}
