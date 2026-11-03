#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N 200
static unsigned seed = 42;
static unsigned rnd(void) { seed = seed * 1664525u + 1013904223u; return seed >> 8; }
static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }
static void bubble(int *a, int n) { for (int i = 0; i < n; i++) for (int j = 0; j + 1 < n - i; j++) if (a[j] > a[j + 1]) swap(&a[j], &a[j + 1]); }
static void insertion(int *a, int n) { for (int i = 1; i < n; i++) { int v = a[i], j = i - 1; while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; j--; } a[j + 1] = v; } }
static void quick(int *a, int lo, int hi) { if (lo >= hi) return; int p = a[(lo + hi) / 2], i = lo, j = hi; while (i <= j) { while (a[i] < p) i++; while (a[j] > p) j--; if (i <= j) swap(&a[i++], &a[j--]); } quick(a, lo, j); quick(a, i, hi); }
static void merge_sort(int *a, int *tmp, int n) { if (n < 2) return; int m = n / 2; merge_sort(a, tmp, m); merge_sort(a + m, tmp, n - m); int i = 0, j = m, k = 0; while (i < m && j < n) tmp[k++] = a[i] <= a[j] ? a[i++] : a[j++]; while (i < m) tmp[k++] = a[i++]; while (j < n) tmp[k++] = a[j++]; memcpy(a, tmp, n * sizeof *a); }
static void sift(int *a, int n, int i) { for (;;) { int l = 2 * i + 1, r = l + 1, m = i; if (l < n && a[l] > a[m]) m = l; if (r < n && a[r] > a[m]) m = r; if (m == i) return; swap(&a[i], &a[m]); i = m; } }
static void heap(int *a, int n) { for (int i = n / 2 - 1; i >= 0; i--) sift(a, n, i); for (int i = n - 1; i > 0; i--) { swap(&a[0], &a[i]); sift(a, i, 0); } }
static void shell(int *a, int n) { for (int g = n / 2; g > 0; g /= 2) for (int i = g; i < n; i++) { int v = a[i], j = i; while (j >= g && a[j - g] > v) { a[j] = a[j - g]; j -= g; } a[j] = v; } }
static void counting(int *a, int n) { static int cnt[1000]; memset(cnt, 0, sizeof cnt); for (int i = 0; i < n; i++) cnt[a[i]]++; int k = 0; for (int v = 0; v < 1000; v++) while (cnt[v]--) a[k++] = v; }
static unsigned checksum(const int *a, int n) { unsigned h = 0; for (int i = 0; i < n; i++) h = h * 31 + a[i]; return h; }
static int sorted(const int *a, int n) { for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0; return 1; }
static void sort_strings(char **s, int n) { for (int i = 1; i < n; i++) { char *v = s[i]; int j = i - 1; while (j >= 0 && strcmp(s[j], v) > 0) { s[j + 1] = s[j]; j--; } s[j + 1] = v; } }
static void sort_doubles(double *d, int n) { for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (d[j] < d[i]) { double t = d[i]; d[i] = d[j]; d[j] = t; } }

int main(void) {
    int orig[N], a[N], tmp[N];
    for (int i = 0; i < N; i++) orig[i] = rnd() % 1000;
    const char *names[] = {"bubble", "insertion", "quick", "merge", "heap", "shell", "counting"};
    for (int alg = 0; alg < 7; alg++) {
        memcpy(a, orig, sizeof a);
        switch (alg) { case 0: bubble(a, N); break; case 1: insertion(a, N); break; case 2: quick(a, 0, N - 1); break;
            case 3: merge_sort(a, tmp, N); break; case 4: heap(a, N); break; case 5: shell(a, N); break; case 6: counting(a, N); break; }
        printf("%-10s sorted=%d sum=%u first=%d last=%d\n", names[alg], sorted(a, N), checksum(a, N), a[0], a[N - 1]);
    }
    char w[][8] = {"delta", "alpha", "echo", "charlie", "bravo", "golf", "foxtrot"}; char *ps[7]; for (int i = 0; i < 7; i++) ps[i] = w[i];
    sort_strings(ps, 7); for (int i = 0; i < 7; i++) printf("%s ", ps[i]); printf("\n");
    double d[8]; for (int i = 0; i < 8; i++) d[i] = (double)(rnd() % 10000) / 100.0 - 50; sort_doubles(d, 8);
    for (int i = 0; i < 8; i++) printf("%.2f ", d[i]); printf("\n");
    return 0;
}
