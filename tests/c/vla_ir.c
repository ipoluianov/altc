// variable-length arrays and alloca through the IR backend: VLAs released
// on every loop iteration and when leaving their block (also by goto),
// multi-dimensional VLAs and pointers to them, calls with stack arguments
// after a stack allocation
#include <alloca.h>
#include <stdio.h>
#include <string.h>

static long sum8(long a, long b, long c, long d, long e, long f, long g, long h) {
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h;
}

static int sum_matrix(int n, int m, int a[n][m]) {
    int s = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            s += a[i][j] * (i + 1);
    return s;
}

static long fill(int n) {
    char buf[n];
    memset(buf, 1, sizeof buf);
    long s = 0;
    for (int i = 0; i < n; i++)
        s += buf[i] * i;
    // stack arguments are written below the VLA
    return s + sum8(1, 2, 3, 4, 5, 6, 7, n);
}

static int with_goto(int n) {
    int tries = 0;
again:
    {
        int v[n + tries];
        for (int i = 0; i < n + tries; i++)
            v[i] = i;
        tries++;
        if (tries < 1000)
            goto again; // leaves the VLA scope: its space is released
        return v[n + tries - 2] + (int)(sizeof v / sizeof v[0]);
    }
}

static int nested(int n) {
    int total = 0;
    for (int i = 1; i <= n; i++) {
        double a[i];
        for (int j = 0; j < i; j++)
            a[j] = j * 0.5;
        {
            int b[i][i + 1];
            for (int j = 0; j < i; j++)
                for (int k = 0; k <= i; k++)
                    b[j][k] = j * k;
            total += b[i - 1][i] + (int)a[i - 1] + (int)sizeof b;
        }
        if (i % 7 == 0)
            continue; // the loop releases the VLA
        total++;
    }
    return total;
}

static long alloca_loop(int n) {
    long s = 0;
    for (int i = 0; i < n; i++) {
        int *p = alloca(16 * sizeof(int)); // lives until the function returns
        for (int j = 0; j < 16; j++)
            p[j] = i + j;
        s += p[i % 16];
    }
    return s;
}

static int big_loop(void) {
    // 200000 iterations of 4 KiB would overflow the stack if VLAs leaked
    int r = 0;
    for (int it = 0; it < 200000; it++) {
        char tmp[4096 + it % 3];
        tmp[0] = (char)it;
        tmp[sizeof tmp - 1] = 1;
        r += tmp[0] + tmp[sizeof tmp - 1];
    }
    int k = 0;
    do {
        long w[k + 512];
        w[k] = k;
        r += (int)w[k];
    } while (++k < 100000);
    return r;
}

int main(void) {
    int n = 5, m = 7;
    int a[n][m];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            a[i][j] = i * 10 + j;
    printf("%d %zu %zu\n", sum_matrix(n, m, a), sizeof a, sizeof a[0]);
    int (*row)[m] = a;
    row++;
    printf("%d %d\n", (*row)[3], row[2][6]);
    printf("%ld %ld\n", fill(100), fill(3000));
    printf("%d\n", with_goto(10));
    printf("%d\n", nested(40));
    printf("%ld\n", alloca_loop(1000));
    printf("%d\n", big_loop());
    char *p = __builtin_alloca(n * 3);
    strcpy(p, "alloca!");
    printf("%s %d\n", p, (int)(((unsigned long)p) % 16));
    int *v = &a[0][0];
    printf("%d\n", v[n * m - 1]);
    return 0;
}
