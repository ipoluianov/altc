#include <stdio.h>
#include <stdbool.h>
static bool to_bool(int x) { return x; }
static bool to_bool_d(double d) { return d; }
static bool to_bool_p(void *p) { return p; }
static int take_bool(bool b) { return b * 10; }
static int take_uc(unsigned char c) { return c; }
static int take_sc(signed char c) { return c; }
static int take_us(unsigned short s) { return s; }
static double take_f(float f) { return f; }
static long double ret_ld(int k) { return k / 3.0L; }
static float cond_ret(int c) { return c ? 1 : 2.5f; }
static char ret_char(int x) { return x; }
static unsigned ret_u_from_d(double d) { return d; }
static long long ret_ll_from_f(float f) { return f; }
static float ret_f_from_ll(long long v) { return v; }
static double ret_d_from_u(unsigned long long v) { return v; }
static int (*fp_bool)(bool) = take_bool;
int main(void) {
    int big = 256; double half = 0.5; int x;
    printf("%d %d %d %d\n", to_bool(big), to_bool(0), to_bool_d(half), to_bool_p(&x));
    printf("%d %d %d\n", take_bool(big), take_bool(0.1), fp_bool(512));
    printf("%d %d %d\n", take_uc(300), take_sc(200), take_us(-1));
    printf("%g\n", take_f(16777217));
    printf("%.15Lf %.15Lf\n", ret_ld(1), ret_ld(2) + ret_ld(4));
    printf("%g %g\n", cond_ret(1), cond_ret(0));
    printf("%d %d\n", ret_char(200) < 0, ret_char(65));
    printf("%u %lld %g %g\n", ret_u_from_d(4e9), ret_ll_from_f(-1e10f), ret_f_from_ll(123456789123LL), ret_d_from_u(18446744073709551615ULL));
    bool flags[3] = {7, 0, -1}; printf("%d %d %d\n", flags[0], flags[1], flags[2]);
    bool b = 1; b += 5; b = b + 1; printf("%d\n", b);
    bool bb = 2 == 2; bool bc = (bool)0.0001; bool bd = !!3; printf("%d %d %d\n", bb, bc, bd);
    return 0;
}
