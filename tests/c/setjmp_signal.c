#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>

static jmp_buf env;
static int depth;
static void deep(int n) { depth = n; if (n == 5) longjmp(env, n * 10); deep(n + 1); }
static volatile sig_atomic_t got;
static void handler(int sig) { got = sig; }
static void bye1(void) { printf("atexit 1\n"); }
static void bye2(void) { printf("atexit 2\n"); }

int main(void) {
    int r = setjmp(env);
    if (r == 0) { deep(0); printf("not reached\n"); }
    else printf("longjmp returned %d depth %d\n", r, depth);
    volatile int count = 0;
    jmp_buf e2;
    if (setjmp(e2) < 3) { count++; if (count < 3) longjmp(e2, count); }
    printf("count %d\n", count);
    jmp_buf e3; if (!setjmp(e3)) longjmp(e3, 0); else printf("longjmp 0 -> 1\n");
    signal(SIGUSR1, handler); raise(SIGUSR1); printf("got %d\n", got == SIGUSR1);
    signal(SIGUSR1, SIG_IGN); raise(SIGUSR1); printf("ignored\n");
    errno = 0; double v = sqrt(-1.0); printf("%d %d\n", isnan(v), errno == EDOM);
    errno = 0; v = log(0.0); printf("%d %d\n", isinf(v), errno == ERANGE);
    errno = 0; FILE *f = fopen("/nonexistent/dir/file", "r"); printf("%d %d\n", f == NULL, errno == ENOENT);
    atexit(bye1); atexit(bye2);
    printf("main done\n");
    return 0;
}
