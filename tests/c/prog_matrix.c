#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct { int r, c; double *d; } Mat;
static Mat mat_new(int r, int c) { Mat m = {r, c, calloc((size_t)r * c, sizeof(double))}; return m; }
static double *at(Mat m, int i, int j) { return &m.d[i * m.c + j]; }
static Mat mat_mul(Mat a, Mat b) { Mat m = mat_new(a.r, b.c); for (int i = 0; i < a.r; i++) for (int j = 0; j < b.c; j++) { double s = 0; for (int k = 0; k < a.c; k++) s += *at(a, i, k) * *at(b, k, j); *at(m, i, j) = s; } return m; }
static Mat mat_t(Mat a) { Mat m = mat_new(a.c, a.r); for (int i = 0; i < a.r; i++) for (int j = 0; j < a.c; j++) *at(m, j, i) = *at(a, i, j); return m; }
static void mat_print(const char *name, Mat m) { printf("%s =\n", name); for (int i = 0; i < m.r; i++) { for (int j = 0; j < m.c; j++) printf(" %10.4f", *at(m, i, j)); printf("\n"); } }
static double det(Mat a) {
    int n = a.r; Mat m = mat_new(n, n); for (int i = 0; i < n * n; i++) m.d[i] = a.d[i];
    double d = 1;
    for (int c = 0; c < n; c++) {
        int p = c; for (int r = c + 1; r < n; r++) if (fabs(*at(m, r, c)) > fabs(*at(m, p, c))) p = r;
        if (*at(m, p, c) == 0) { free(m.d); return 0; }
        if (p != c) { for (int j = 0; j < n; j++) { double t = *at(m, c, j); *at(m, c, j) = *at(m, p, j); *at(m, p, j) = t; } d = -d; }
        d *= *at(m, c, c);
        for (int r = c + 1; r < n; r++) { double f = *at(m, r, c) / *at(m, c, c); for (int j = c; j < n; j++) *at(m, r, j) -= f * *at(m, c, j); }
    }
    free(m.d); return d;
}
static Mat inverse(Mat a) {
    int n = a.r; Mat m = mat_new(n, 2 * n);
    for (int i = 0; i < n; i++) { for (int j = 0; j < n; j++) *at(m, i, j) = *at(a, i, j); *at(m, i, n + i) = 1; }
    for (int c = 0; c < n; c++) {
        int p = c; for (int r = c + 1; r < n; r++) if (fabs(*at(m, r, c)) > fabs(*at(m, p, c))) p = r;
        for (int j = 0; j < 2 * n; j++) { double t = *at(m, c, j); *at(m, c, j) = *at(m, p, j); *at(m, p, j) = t; }
        double pv = *at(m, c, c); for (int j = 0; j < 2 * n; j++) *at(m, c, j) /= pv;
        for (int r = 0; r < n; r++) if (r != c) { double f = *at(m, r, c); for (int j = 0; j < 2 * n; j++) *at(m, r, j) -= f * *at(m, c, j); }
    }
    Mat inv = mat_new(n, n); for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) *at(inv, i, j) = *at(m, i, n + j);
    free(m.d); return inv;
}

int main(void) {
    Mat a = mat_new(3, 3); double av[] = {2, -1, 0, -1, 2, -1, 0, -1, 2}; for (int i = 0; i < 9; i++) a.d[i] = av[i];
    Mat b = mat_new(3, 2); for (int i = 0; i < 6; i++) b.d[i] = i * 0.5 - 1;
    mat_print("a", a); mat_print("b", b);
    Mat ab = mat_mul(a, b); mat_print("ab", ab);
    mat_print("bt", mat_t(b));
    printf("det(a) = %.6f\n", det(a));
    Mat ia = inverse(a); mat_print("inv(a)", ia);
    Mat id = mat_mul(a, ia); for (int i = 0; i < 9; i++) id.d[i] = round(id.d[i] * 1e9) / 1e9 + 0.0; mat_print("a*inv(a)", id);
    Mat h = mat_new(5, 5); for (int i = 0; i < 5; i++) for (int j = 0; j < 5; j++) *at(h, i, j) = 1.0 / (i + j + 1);
    printf("det(hilbert5) = %.6e\n", det(h));
    double v[4] = {1, 2, 3, 4}, w[4] = {4, 3, 2, 1}, dot = 0; for (int i = 0; i < 4; i++) dot += v[i] * w[i];
    printf("dot = %g norm = %.10f\n", dot, sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]));
    return 0;
}
