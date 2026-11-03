#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static int nq_count(int n, int row, unsigned cols, unsigned d1, unsigned d2) {
    if (row == n) return 1; int c = 0;
    for (int i = 0; i < n; i++) { unsigned a = 1u << i, b = 1u << (row + i), e = 1u << (row - i + n); if (!(cols & a) && !(d1 & b) && !(d2 & e)) c += nq_count(n, row + 1, cols | a, d1 | b, d2 | e); }
    return c;
}
static int grid[9][9];
static int ok(int r, int c, int v) { for (int i = 0; i < 9; i++) if (grid[r][i] == v || grid[i][c] == v || grid[r / 3 * 3 + i / 3][c / 3 * 3 + i % 3] == v) return 0; return 1; }
static int solve(void) { for (int r = 0; r < 9; r++) for (int c = 0; c < 9; c++) if (!grid[r][c]) { for (int v = 1; v <= 9; v++) if (ok(r, c, v)) { grid[r][c] = v; if (solve()) return 1; grid[r][c] = 0; } return 0; } return 1; }
static void life(char g[10][10], int steps) {
    for (int s = 0; s < steps; s++) { char n[10][10];
        for (int i = 0; i < 10; i++) for (int j = 0; j < 10; j++) { int k = 0; for (int di = -1; di <= 1; di++) for (int dj = -1; dj <= 1; dj++) if ((di || dj) && g[(i + di + 10) % 10][(j + dj + 10) % 10]) k++; n[i][j] = g[i][j] ? (k == 2 || k == 3) : k == 3; }
        memcpy(g, n, sizeof n); }
}
#define V 6
static void dijkstra(int w[V][V], int src, int dist[V]) {
    int done[V] = {0}; for (int i = 0; i < V; i++) dist[i] = INT_MAX; dist[src] = 0;
    for (int it = 0; it < V; it++) { int u = -1; for (int i = 0; i < V; i++) if (!done[i] && (u < 0 || dist[i] < dist[u])) u = i; if (dist[u] == INT_MAX) break; done[u] = 1;
        for (int v = 0; v < V; v++) if (w[u][v] && dist[u] + w[u][v] < dist[v]) dist[v] = dist[u] + w[u][v]; }
}
static long long lcs(const char *a, const char *b) { int n = strlen(a), m = strlen(b); long long dp[40][40] = {{0}}; for (int i = 1; i <= n; i++) for (int j = 1; j <= m; j++) dp[i][j] = a[i - 1] == b[j - 1] ? dp[i - 1][j - 1] + 1 : (dp[i - 1][j] > dp[i][j - 1] ? dp[i - 1][j] : dp[i][j - 1]); return dp[n][m]; }
static int knap(int n, const int *wt, const int *val, int cap) { int dp[101] = {0}; for (int i = 0; i < n; i++) for (int c = cap; c >= wt[i]; c--) if (dp[c - wt[i]] + val[i] > dp[c]) dp[c] = dp[c - wt[i]] + val[i]; return dp[cap]; }
static void mandel(void) {
    for (int y = -6; y <= 6; y++) { for (int x = -20; x <= 8; x++) { double cr = x / 10.0, ci = y / 6.0, zr = 0, zi = 0; int k = 0; while (k < 50 && zr * zr + zi * zi < 4) { double t = zr * zr - zi * zi + cr; zi = 2 * zr * zi + ci; zr = t; k++; } putchar(" .:-=+*#%@"[k % 10]); } putchar('\n'); }
}
static void primes(int n) { char *s = calloc(n + 1, 1); int c = 0, last = 0; for (int i = 2; i <= n; i++) if (!s[i]) { c++; last = i; for (long j = (long)i * i; j <= n; j += i) s[j] = 1; } printf("primes<=%d: %d last=%d\n", n, c, last); free(s); }
static void hanoi(int n, char a, char b, char c, int *moves) { if (!n) return; hanoi(n - 1, a, c, b, moves); (*moves)++; hanoi(n - 1, c, b, a, moves); }
static void perms(char *s, int k, int n, int *cnt) { if (k == n) { if (*cnt < 6) printf("%s ", s); (*cnt)++; return; } for (int i = k; i < n; i++) { char t = s[k]; s[k] = s[i]; s[i] = t; perms(s, k + 1, n, cnt); t = s[k]; s[k] = s[i]; s[i] = t; } }

int main(void) {
    for (int n = 4; n <= 9; n++) printf("%d ", nq_count(n, 0, 0, 0, 0)); printf("\n");
    const char *puz = "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    for (int i = 0; i < 81; i++) grid[i / 9][i % 9] = puz[i] - '0';
    printf("solved=%d\n", solve()); for (int r = 0; r < 9; r++) { for (int c = 0; c < 9; c++) printf("%d", grid[r][c]); printf("\n"); }
    char g[10][10] = {{0}}; g[1][2] = g[2][3] = g[3][1] = g[3][2] = g[3][3] = 1; life(g, 8);
    for (int i = 0; i < 10; i++) { for (int j = 0; j < 10; j++) putchar(g[i][j] ? '#' : '.'); putchar('\n'); }
    int w[V][V] = {{0, 7, 9, 0, 0, 14}, {7, 0, 10, 15, 0, 0}, {9, 10, 0, 11, 0, 2}, {0, 15, 11, 0, 6, 0}, {0, 0, 0, 6, 0, 9}, {14, 0, 2, 0, 9, 0}};
    int dist[V]; dijkstra(w, 0, dist); for (int i = 0; i < V; i++) printf("%d ", dist[i]); printf("\n");
    printf("%lld %lld\n", lcs("AGGTABQWERTY", "GXTXAYBWRT"), lcs("abcdefghij", "acegi"));
    int wt[] = {10, 20, 30, 15, 25}, val[] = {60, 100, 120, 70, 110}; printf("%d\n", knap(5, wt, val, 50));
    mandel();
    primes(100000);
    int moves = 0; hanoi(16, 'a', 'b', 'c', &moves); printf("hanoi %d\n", moves);
    char ps[] = "abcd"; int pc = 0; perms(ps, 0, 4, &pc); printf("| %d\n", pc);
    return 0;
}
