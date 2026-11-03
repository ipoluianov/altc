#include <stdio.h>
#include <string.h>

typedef struct { double x, y; } V2;
typedef struct { float r, g, b, a; } Color;
typedef struct { long a; int b; } LI;
typedef struct { char c[20]; } Name;

static int f8(int a, int b, int c, int d, int e, int f, int g, int h) { return a - b + c - d + e - f + g - h; }
static double d10(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j) { return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10; }
static float fl10(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j) { return a - b + c - d + e - f + g - h + i - j; }
static long double ld3(long double a, int x, long double b, double y, long double c) { return a + x + b * y + c; }
static double alt(int a, double b, int c, double d, int e, double f, int g, double h, int i, double j, int k, double l, int m, double n, int o, double p, int q, double r, int s, double t) {
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t;
}
static V2 vadd(V2 a, V2 b) { return (V2){a.x + b.x, a.y + b.y}; }
static V2 vscale(V2 a, double k) { return (V2){a.x * k, a.y * k}; }
static Color blend(Color a, Color b, float t) { return (Color){a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t}; }
static LI li_mk(long a, int b) { return (LI){a, b}; }
static Name name_mk(const char *s) { Name n; memset(&n, 0, sizeof n); strncpy(n.c, s, 19); return n; }
static int nlen(Name n) { return (int)strlen(n.c); }
static int sq(int x) { return x * x; }
static int add3(int a, int b, int c) { return a + b + c; }
typedef V2 (*vop)(V2, V2);
static V2 apply_op(vop op, V2 a, V2 b) { return op(a, b); }
static int rec_struct(Name n, int depth) { if (!depth) return nlen(n); n.c[depth % 19] = 'x'; return rec_struct(n, depth - 1) + 1; }
static double many_structs(V2 a, V2 b, V2 c, V2 d, V2 e, V2 f) { return a.x + b.y + c.x + d.y + e.x + f.y; }
static long many_li(LI a, LI b, LI c, LI d, LI e) { return a.a + b.b + c.a + d.b + e.a; }

int main(void) {
    printf("%d\n", f8(1, 2, 3, 4, 5, 6, 7, 8));
    printf("%d\n", f8(sq(1), sq(2), sq(3), sq(4), sq(5), sq(6), sq(7), sq(8)));
    printf("%d\n", f8(f8(1, 1, 1, 1, 1, 1, 1, 9), add3(1, 2, 3), sq(add3(1, 1, 1)), 4, 5, f8(8, 7, 6, 5, 4, 3, 2, 1), 7, 8));
    printf("%g\n", d10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    printf("%g\n", fl10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10.5f));
    printf("%.10Lg\n", ld3(1.5L, 2, 2.5L, 3.0, 1e-10L));
    printf("%g\n", alt(1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5));
    V2 a = {1, 2}, b = {3, 4};
    V2 c = vadd(vscale(vadd(a, b), 0.5), vscale(a, -1)); printf("%g %g\n", c.x, c.y);
    V2 d = apply_op(vadd, a, apply_op(vadd, b, b)); printf("%g %g\n", d.x, d.y);
    Color col = blend((Color){0, 0, 0, 1}, (Color){1, 0.5f, 0.25f, 0}, 0.5f); printf("%g %g %g %g\n", col.r, col.g, col.b, col.a);
    LI li = li_mk(1L << 40, -3); printf("%ld %d\n", li.a, li.b);
    Name n = name_mk("structure-name-long!"); printf("%s %d\n", n.c, nlen(n));
    printf("%d\n", rec_struct(name_mk("abc"), 10));
    printf("%g\n", many_structs((V2){1, 2}, (V2){3, 4}, (V2){5, 6}, (V2){7, 8}, (V2){9, 10}, (V2){11, 12}));
    printf("%ld\n", many_li((LI){1, 2}, (LI){3, 4}, (LI){5, 6}, (LI){7, 8}, (LI){9, 10}));
    int x = 3, y = 4, z = 5;
    int e = (x * y + z) * (x - y * z) / (x + 1) - (y << 2) + (z ^ x) % 3 + ((x & y) | (z << 1)) * (x > y ? z : -z);
    printf("%d\n", e);
    double dd = (x + 0.5) * (y - 0.25) / (z * 1.5) + (x * y * 1.0) / (z - x) - (double)(x << y) / 7.0;
    printf("%.12f\n", dd);
    int deep = sq(sq(sq(2))) + add3(sq(1), add3(1, add3(1, 1, 1), sq(2)), sq(add3(sq(1), 1, 1)));
    printf("%d\n", deep);
    long acc = 0; for (int i = 0; i < 100; i++) acc += f8(i, i + 1, i * 2, i * 3, -i, i % 7, i / 3, i ^ 5); printf("%ld\n", acc);
    char big[100000]; memset(big, 1, sizeof big); long bs = 0; for (int i = 0; i < 100000; i += 1000) bs += big[i]; printf("%ld\n", bs);
    return 0;
}
