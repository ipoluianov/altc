#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
static sigjmp_buf jb;
static volatile sig_atomic_t which;
static void on_sig(int s) { which = s; siglongjmp(jb, s); }
static int divide(volatile int a, volatile int b) { return a / b; }
int main(void) {
    struct sigaction sa; memset(&sa, 0, sizeof sa); sa.sa_handler = on_sig; sigemptyset(&sa.sa_mask);
    sigaction(SIGFPE, &sa, NULL); sigaction(SIGSEGV, &sa, NULL);
    if (sigsetjmp(jb, 1) == 0) { printf("%d\n", divide(1, 0)); } else printf("caught SIGFPE %d\n", which == SIGFPE);
    if (sigsetjmp(jb, 1) == 0) { volatile int *p = 0; printf("%d\n", *p); } else printf("caught SIGSEGV %d\n", which == SIGSEGV);
    if (sigsetjmp(jb, 1) == 0) { raise(SIGFPE); } else printf("caught raise %d\n", which);
    sigset_t set; sigemptyset(&set); sigaddset(&set, SIGUSR1); sigprocmask(SIG_BLOCK, &set, NULL); raise(SIGUSR1);
    sigset_t pend; sigpending(&pend); printf("pending %d\n", sigismember(&pend, SIGUSR1));
    return 0;
}
