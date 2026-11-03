#include <stdio.h>

int tentative;
int tentative = 7;
int tent_arr[];
extern int ext_later;
static int counter(void) { static int n; return ++n; }
static int counter2(void) { static int n = 100; n += 10; return n; }
_Thread_local int tls = 5;
static _Thread_local int tls_arr[4] = {1, 2, 3, 4};
__thread long gtls;
const int cglob = 11;
static const char *const names[] = {"a", "b"};
extern int fwd(int);
int ext_later = 33;
volatile int vol = 3;
static int recursion_depth(int n) { static int maxd; if (n > maxd) maxd = n; if (n < 10) recursion_depth(n + 1); return maxd; }
int *get_static_ptr(void) { static int x = 42; return &x; }
int fwd(int x) { return x * 3; }
int tent_arr[3];

int main(void) {
    for (int i = 0; i < 3; i++) printf("%d ", counter()); printf("\n");
    for (int i = 0; i < 3; i++) printf("%d ", counter2()); printf("\n");
    printf("%d %d\n", tentative, ext_later);
    printf("%d %d\n", tls, tls_arr[2]);
    tls += 10; tls_arr[3] *= 5; gtls = -1; printf("%d %d %ld\n", tls, tls_arr[3], gtls);
    int *tp = &tls; *tp = 1; printf("%d\n", tls);
    printf("%d %s %d\n", cglob, names[1], fwd(4));
    register int r = 9; register int r2 = r * 2; printf("%d\n", r + r2);
    vol++; printf("%d\n", vol);
    printf("%d\n", recursion_depth(0));
    int *sp = get_static_ptr(); (*sp)++; printf("%d\n", *get_static_ptr());
    { int shadow = 1; { int shadow = 2; printf("%d ", shadow); } printf("%d\n", shadow); }
    for (int i = 0; i < 2; i++) { static int s; int a = 0; s++; a++; printf("%d %d ", s, a); } printf("\n");
    extern int tentative; printf("%d\n", tentative);
    auto int au = 4; printf("%d\n", au);
    tent_arr[2] = 4; printf("%zu %d\n", sizeof tent_arr, tent_arr[2]);
    static long long big_static[1000]; big_static[999] = 1; printf("%lld\n", big_static[999] + big_static[0]);
    static char zeroed[4096]; int nz = 0; for (int i = 0; i < 4096; i++) nz += zeroed[i]; printf("%d\n", nz);
    return 0;
}
