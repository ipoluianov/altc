#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int sw(long long x) {
    switch (x) {
    case -5000000000LL ... -4000000000LL: return 1;
    case 'a' ... 'z': return 2;
    case 1000: case 1001: return 3;
    case 0x7fffffffffffffffLL: return 4;
    case -1: return 5;
    }
    return 0;
}
static int sw_u(unsigned u) { switch (u) { case 0xffffffffu: return 1; case 0x80000000u: return 2; case 0: return 3; default: return 4; } }
static int sw_sc(signed char c) { switch (c) { case -128: return 1; case -1: return 2; case 127: return 3; } return 0; }
static const char *sw_str(int k) { switch (k) { case 0: { static const char s[] = "zero"; return s; } case 1: return "one"; } return "many"; }
static int sw_goto(int k) { int r = 0; switch (k) { case 0: goto inside; case 1: r = 10; inside: r += 1; break; } return r; }
static int sw_loop(int n) { int r = 0; for (int i = 0; i < n; i++) switch (i % 3) { case 0: continue; case 1: r += i; break; default: if (i > 6) goto out; r -= 1; } out: return r; }
static int sw_bool(_Bool b) { switch (b) { case 0: return 5; case 1: return 6; } return 7; }
enum E { EA = 1, EB = 1 << 20, EC = -3 };
static int sw_enum(enum E e) { switch (e) { case EA: return 1; case EB: return 2; case EC: return 3; } return 0; }
static int sw_sparse(int x) { switch (x) { case 1: return 1; case 100: return 2; case 10000: return 3; case 1000000: return 4; case -1000000: return 5; case 7: return 6; case 8: return 7; case 9: return 8; case 10: return 9; } return 0; }

int main(void) {
    long long ins[] = {-4500000000LL, 'q', 1000, 1001, 0x7fffffffffffffffLL, -1, 5, -4000000001LL, -5000000001LL};
    for (int i = 0; i < 9; i++) printf("%d", sw(ins[i])); printf("\n");
    printf("%d%d%d%d\n", sw_u(0xffffffffu), sw_u(0x80000000u), sw_u(0), sw_u(5));
    printf("%d%d%d%d\n", sw_sc(-128), sw_sc(-1), sw_sc(127), sw_sc(0));
    printf("%s %s %s\n", sw_str(0), sw_str(1), sw_str(2));
    printf("%d %d %d\n", sw_goto(0), sw_goto(1), sw_goto(2));
    printf("%d %d\n", sw_loop(5), sw_loop(20));
    printf("%d %d\n", sw_bool(0), sw_bool(1));
    printf("%d %d %d %d\n", sw_enum(EA), sw_enum(EB), sw_enum(EC), sw_enum((enum E)99));
    int xs[] = {1, 100, 10000, 1000000, -1000000, 7, 8, 9, 10, 11, 0, -1}; for (int i = 0; i < 12; i++) printf("%d", sw_sparse(xs[i])); printf("\n");
    return 0;
}
