#include <stdio.h>

static const char *classify(int c) {
    switch (c) {
    case 'a' ... 'z': return "lower";
    case 'A' ... 'Z': return "upper";
    case '0' ... '9': return "digit";
    case ' ': case '\t': case '\n': return "space";
    default: return "other";
    }
}
static int fallthrough(int n) {
    int r = 0;
    switch (n) {
    case 0: r += 1;
    case 1: r += 10;
    case 2: r += 100; break;
    case 3: r += 1000;
    default: r += 10000;
    case 4: r += 100000;
    }
    return r;
}
static int duff(int count) {
    int n = (count + 7) / 8, ops = 0;
    if (!count) return 0;
    switch (count % 8) {
    case 0: do { ops++;
    case 7: ops++;
    case 6: ops++;
    case 5: ops++;
    case 4: ops++;
    case 3: ops++;
    case 2: ops++;
    case 1: ops++;
            } while (--n > 0);
    }
    return ops;
}
static int computed(int x) {
    static void *tbl[] = {&&L0, &&L1, &&L2};
    int r = 0;
    goto *tbl[x % 3];
L0: r += 1;
L1: r += 2;
L2: r += 4;
    return r;
}
static long long switch64(long long v) {
    switch (v) {
    case 0x100000000LL: return 1;
    case -1: return 2;
    case 9223372036854775807LL: return 3;
    default: return 0;
    }
}
static int sw_char(unsigned char c) { switch (c) { case 200: return 1; case 255: return 2; } return 0; }
static int sw_dense(int x) {
    switch (x) { case 0: return 10; case 1: return 11; case 2: return 12; case 3: return 13; case 4: return 14;
                 case 5: return 15; case 6: return 16; case 7: return 17; case 8: return 18; case 9: return 19;
                 case 10: return 20; case 11: return 21; case -3: return 7; }
    return -1;
}

int main(void) {
    const char *s = "aZ5 !\n";
    for (const char *p = s; *p; p++) printf("%s ", classify(*p)); printf("\n");
    for (int i = 0; i < 7; i++) printf("%d ", fallthrough(i)); printf("\n");
    for (int i = 0; i < 20; i += 3) printf("%d ", duff(i)); printf("\n");
    for (int i = 0; i < 4; i++) printf("%d ", computed(i)); printf("\n");
    printf("%lld %lld %lld %lld\n", switch64(0x100000000LL), switch64(-1), switch64(9223372036854775807LL), switch64(5));
    printf("%d %d %d\n", sw_char(200), sw_char(255), sw_char(0));
    for (int i = -4; i < 13; i++) printf("%d ", sw_dense(i)); printf("\n");
    int i, j, cnt = 0;
    for (i = 0; i < 10; i++) { if (i == 2) continue; if (i == 8) break; for (j = 0; j < 10; j++) { if (j > i) break; if ((i + j) % 3 == 0) continue; cnt++; } }
    printf("%d %d %d\n", i, j, cnt);
    int w = 0; while (1) { if (++w > 5) break; } printf("%d\n", w);
    int d = 10; do { d -= 3; } while (d > 0); printf("%d\n", d);
    int dd = 0; do dd++; while (0); printf("%d\n", dd);
    int g = 0;
again:
    g++;
    if (g < 5) goto again;
    printf("%d\n", g);
    for (int a = 0; a < 3; a++) for (int b = 0; b < 3; b++) { if (a * b == 2) goto out; printf("%d%d ", a, b); }
out:
    printf("\n");
    int sum = 0; for (int a = 0, b = 10; a < b; a++, b--) sum += a * b; printf("%d\n", sum);
    for (;;) { sum--; if (sum < 40) break; } printf("%d\n", sum);
    int x = 3;
    switch (x) { int y; case 3: y = 7; printf("y=%d\n", y); break; }
    switch (x) { default: printf("only default\n"); }
    switch (x) case 3: printf("bare case\n");
    int nest = 0;
    switch (x) { case 3: switch (nest) { case 0: nest = 5; break; default: nest = 9; } nest++; break; case 4: nest = -1; }
    printf("%d\n", nest);
    int k = 0; while (k < 100) { k++; if (k % 2) continue; if (k > 10) break; } printf("%d\n", k);
    if (x == 3) if (k == 0) printf("no\n"); else printf("dangling else\n");
    int lbl = 0; { void *pp = &&end; lbl = 1; goto *pp; lbl = 2; end: ; } printf("%d\n", lbl);
    return 0;
}
