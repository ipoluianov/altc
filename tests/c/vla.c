#include <stdio.h>
#include <alloca.h>
#include <string.h>

static int sum_vla(int n, int a[n]) { int s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
static int trace(int n, int m, int a[n][m]) { int s = 0; for (int i = 0; i < n && i < m; i++) s += a[i][i]; return s; }
static void fill2d(int r, int c, double (*m)[c]) { for (int i = 0; i < r; i++) for (int j = 0; j < c; j++) m[i][j] = i * c + j; }
static size_t vla_size(int n) { char buf[n * 3 + 1]; return sizeof buf; }
static int nested_scope(int n) {
    int total = 0;
    for (int k = 1; k <= n; k++) { int tmp[k]; for (int i = 0; i < k; i++) tmp[i] = i; for (int i = 0; i < k; i++) total += tmp[i]; }
    return total;
}
static int with_alloca(int n) { int *p = alloca(n * sizeof(int)); for (int i = 0; i < n; i++) p[i] = i * i; int s = 0; for (int i = 0; i < n; i++) s += p[i]; return s; }

int main(void) {
    int n = 5;
    int a[n]; for (int i = 0; i < n; i++) a[i] = i + 1;
    printf("%zu %d\n", sizeof a, sum_vla(n, a));
    int r = 3, c = 4;
    int m[r][c]; for (int i = 0; i < r; i++) for (int j = 0; j < c; j++) m[i][j] = i * 10 + j;
    printf("%zu %zu %d\n", sizeof m, sizeof m[0], trace(r, c, m));
    double dm[r][c]; fill2d(r, c, dm); printf("%g %g\n", dm[2][3], dm[1][1]);
    printf("%zu %zu\n", vla_size(1), vla_size(10));
    printf("%d\n", nested_scope(6));
    printf("%d\n", with_alloca(10));
    typedef int row_t[c]; row_t *rows = (row_t *)m; printf("%d %zu\n", rows[1][2], sizeof(row_t));
    int (*pm)[c] = m; pm++; printf("%d\n", (*pm)[3]);
    char s[n + 10]; strcpy(s, "vla string"); printf("%s %zu\n", s, strlen(s));
    for (int k = 1; k < 4; k++) { long big[k * 100]; memset(big, 0, sizeof big); big[k * 100 - 1] = k; printf("%ld ", big[k * 100 - 1] + big[0]); }
    printf("\n");
    int x = 3; int cube[x][x][x]; int cnt = 0; for (int i = 0; i < x; i++) for (int j = 0; j < x; j++) for (int k = 0; k < x; k++) cube[i][j][k] = cnt++;
    printf("%d %zu\n", cube[2][1][0], sizeof cube[1]);
    int sz_side = 0; int arr2[(sz_side++, 4)]; printf("%d %zu\n", sz_side, sizeof arr2);
    return 0;
}
