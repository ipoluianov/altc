#include <stdio.h>

typedef struct { char c; } S1;
typedef struct { char c[3]; } S3;
typedef struct { short a; char b; } S4;
typedef struct { int a; } I1;
typedef struct { int a, b; } I2;
typedef struct { int a, b, c; } I3;
typedef struct { int a, b, c, d; } I4;
typedef struct { int a, b, c, d, e; } I5;
typedef struct { long a, b; } L2;
typedef struct { long a, b, c; } L3;
typedef struct { float f; } F1;
typedef struct { float f, g; } F2;
typedef struct { float f, g, h; } F3;
typedef struct { float f, g, h, i; } F4;
typedef struct { double d; } D1;
typedef struct { double d, e; } D2;
typedef struct { double d, e, f; } D3;
typedef struct { float f; int i; } FI;
typedef struct { int i; float f; } IF;
typedef struct { double d; long l; } DL;
typedef struct { long l; double d; } LD;
typedef struct { char c; double d; } CD;
typedef struct { float a, b; double c; } FFD;
typedef struct { long double x; } X1;
typedef struct { char s[13]; } C13;
typedef struct { char s[40]; } C40;
typedef struct { short s[3]; } SH3;
typedef struct { unsigned char a; float f; } UF;
typedef struct { double d[2]; } DA2;
typedef struct { float f[3]; int i; } F3I;

#define MK(T) static T id_##T(T x) { return x; }
MK(S1) MK(S3) MK(S4) MK(I1) MK(I2) MK(I3) MK(I4) MK(I5) MK(L2) MK(L3) MK(F1) MK(F2) MK(F3) MK(F4)
MK(D1) MK(D2) MK(D3) MK(FI) MK(IF) MK(DL) MK(LD) MK(CD) MK(FFD) MK(X1) MK(C13) MK(C40) MK(SH3) MK(UF) MK(DA2) MK(F3I)

static S3 mk_s3(int k) { S3 s = {{'a' + k, 'b' + k, 'c' + k}}; return s; }
static I3 mk_i3(int k) { I3 s = {k, k * 2, k * 3}; return s; }
static F3 mk_f3(float k) { F3 s = {k, k * 2, k * 3}; return s; }
static DL mk_dl(double d, long l) { DL s = {d, l}; return s; }
static FFD mk_ffd(float a) { FFD s = {a, a + 1, a + 2}; return s; }
static C40 mk_c40(char c) { C40 s; for (int i = 0; i < 39; i++) s.s[i] = c + i % 26; s.s[39] = 0; return s; }

/* many args: some structs spill to the stack */
static double many(I2 a, D2 b, L3 c, F2 d, I4 e, DL f, FI g, X1 h, IF i, S3 j, D1 k, F3 l) {
    return a.a + a.b + b.d + b.e + c.a + c.b + c.c + d.f + d.g + e.a + e.b + e.c + e.d + f.d + f.l +
           g.f + g.i + (double)h.x + i.i + i.f + j.c[0] + j.c[2] + k.d + l.f + l.g + l.h;
}
static long ints(long a, long b, long c, long d, long e, long f, I2 g, long h, L2 i, int j) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g.a * 7 + g.b * 8 + h * 9 + i.a * 10 + i.b * 11 + j * 12;
}
static double fl(double a, double b, double c, double d, double e, double f, double g, D2 h, double i, F2 j, float k) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h.d * 8 + h.e * 9 + i * 10 + j.f * 11 + j.g * 12 + k * 13;
}
static void mixed(int a, double b, I5 big, float c, L2 l, D3 d, long double x, int e, F4 f4, C13 c13) {
    printf("mixed: %d %g %d %d %g %ld %ld %g %g %Lg %d %g %g %s\n", a, b, big.a, big.e, c, l.a, l.b, d.d, d.f, x, e, f4.f, f4.i, c13.s);
}

int main(void) {
    S1 s1 = id_S1((S1){'q'}); printf("%c\n", s1.c);
    S3 s3 = id_S3(mk_s3(1)); printf("%c%c%c\n", s3.c[0], s3.c[1], s3.c[2]);
    S4 s4 = id_S4((S4){-300, 'z'}); printf("%d %c\n", s4.a, s4.b);
    printf("%d\n", id_I1((I1){42}).a);
    I2 i2 = id_I2((I2){-1, 2}); printf("%d %d\n", i2.a, i2.b);
    I3 i3 = id_I3(mk_i3(5)); printf("%d %d %d\n", i3.a, i3.b, i3.c);
    I4 i4 = id_I4((I4){1, 2, 3, 4}); printf("%d %d %d %d\n", i4.a, i4.b, i4.c, i4.d);
    I5 i5 = id_I5((I5){1, 2, 3, 4, 5}); printf("%d %d\n", i5.a, i5.e);
    L2 l2 = id_L2((L2){1L << 40, -7}); printf("%ld %ld\n", l2.a, l2.b);
    L3 l3 = id_L3((L3){1, 2, 3}); printf("%ld %ld %ld\n", l3.a, l3.b, l3.c);
    printf("%g\n", id_F1((F1){1.5f}).f);
    F2 f2 = id_F2((F2){1.25f, -2.5f}); printf("%g %g\n", f2.f, f2.g);
    F3 f3 = id_F3(mk_f3(1.5f)); printf("%g %g %g\n", f3.f, f3.g, f3.h);
    F4 f4 = id_F4((F4){1, 2, 3, 4}); printf("%g %g %g %g\n", f4.f, f4.g, f4.h, f4.i);
    printf("%g\n", id_D1((D1){3.25}).d);
    D2 d2 = id_D2((D2){1e100, -1e-100}); printf("%g %g\n", d2.d, d2.e);
    D3 d3 = id_D3((D3){1, 2, 3}); printf("%g %g %g\n", d3.d, d3.e, d3.f);
    FI fi = id_FI((FI){2.5f, -9}); printf("%g %d\n", fi.f, fi.i);
    IF iff = id_IF((IF){-9, 2.5f}); printf("%d %g\n", iff.i, iff.f);
    DL dl = id_DL(mk_dl(6.5, -123456789012L)); printf("%g %ld\n", dl.d, dl.l);
    LD ld = id_LD((LD){77, 0.125}); printf("%ld %g\n", ld.l, ld.d);
    CD cd = id_CD((CD){'k', 9.75}); printf("%c %g\n", cd.c, cd.d);
    FFD ffd = id_FFD(mk_ffd(10)); printf("%g %g %g\n", ffd.a, ffd.b, ffd.c);
    X1 x1 = id_X1((X1){1.0L / 3}); printf("%.18Lf\n", x1.x);
    C13 c13 = id_C13((C13){"twelve chars"}); printf("%s\n", c13.s);
    C40 c40 = id_C40(mk_c40('A')); printf("%s\n", c40.s);
    SH3 sh3 = id_SH3((SH3){{-1, 2, -3}}); printf("%d %d %d\n", sh3.s[0], sh3.s[1], sh3.s[2]);
    UF uf = id_UF((UF){200, 0.5f}); printf("%d %g\n", uf.a, uf.f);
    DA2 da2 = id_DA2((DA2){{1.5, 2.5}}); printf("%g %g\n", da2.d[0], da2.d[1]);
    F3I f3i = id_F3I((F3I){{1, 2, 3}, 4}); printf("%g %g %g %d\n", f3i.f[0], f3i.f[1], f3i.f[2], f3i.i);
    printf("%g\n", many((I2){1, 2}, (D2){3, 4}, (L3){5, 6, 7}, (F2){8, 9}, (I4){10, 11, 12, 13}, (DL){14, 15},
                        (FI){16, 17}, (X1){18}, (IF){19, 20}, (S3){{1, 2, 3}}, (D1){22}, (F3){23, 24, 25}));
    printf("%ld\n", ints(1, 2, 3, 4, 5, 6, (I2){7, 8}, 9, (L2){10, 11}, 12));
    printf("%g\n", fl(1, 2, 3, 4, 5, 6, 7, (D2){8, 9}, 10, (F2){11, 12}, 13));
    mixed(1, 2.5, (I5){10, 0, 0, 0, 50}, 3.5f, (L2){100, 200}, (D3){7, 8, 9}, 1.25L, 99, (F4){1, 2, 3, 4}, (C13){"hello"});
    /* struct returned and immediately member-accessed / passed on */
    printf("%d\n", id_I3(id_I3(mk_i3(2))).c);
    printf("%g\n", id_D2(id_D2((D2){5, 6})).e);
    return 0;
}
