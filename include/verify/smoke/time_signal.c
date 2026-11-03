/* Smoke test: <time.h>, <sys/time.h>, <setjmp.h>, <signal.h>, <assert.h>,
   <errno.h>.  */
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static jmp_buf jb;
static sigjmp_buf sjb;
static volatile sig_atomic_t got;
static volatile int info_code = -99;

static void
handler (int sig)
{
  got = sig;
}

static void
info_handler (int sig, siginfo_t *si, void *ctx)
{
  (void) ctx;
  got = sig;
  info_code = si->si_code;
}

static void
jumper (int v)
{
  longjmp (jb, v);
}

int
main (void)
{
  struct tm tm, *ptm;
  time_t t = 86400 * 365;
  char buf[128];
  struct timespec ts, req;
  struct timeval tv;
  volatile int count = 0;
  int count2;
  struct sigaction sa;
  sigset_t set, old;
  clock_t c;

  ptm = gmtime_r (&t, &tm);
  strftime (buf, sizeof buf, "%Y-%m-%d %H:%M:%S %j %a", ptm);
  printf ("gmtime %s yday %d wday %d\n", buf, tm.tm_yday, tm.tm_wday);
  tm.tm_mday += 40;
  t = timegm (&tm);
  printf ("timegm %ld mon %d mday %d\n", (long) t, tm.tm_mon, tm.tm_mday);
  printf ("asctime %s", asctime (&tm));
  printf ("difftime %.0f\n", difftime ((time_t) 100, (time_t) 40));
  printf ("time ok %d\n", time (NULL) > 1600000000);
  printf ("clock_gettime %d\n", clock_gettime (CLOCK_MONOTONIC, &ts) == 0);
  printf ("gettimeofday %d\n", gettimeofday (&tv, NULL) == 0);
  req.tv_sec = 0;
  req.tv_nsec = 1000000;
  printf ("nanosleep %d\n", nanosleep (&req, NULL));
  c = clock ();
  printf ("clock ok %d CLOCKS_PER_SEC %ld\n", c != (clock_t) -1,
          (long) CLOCKS_PER_SEC);
  setenv ("TZ", "UTC", 1);
  tzset ();
  t = 0;
  localtime_r (&t, &tm);
  printf ("localtime year %d gmtoff %ld zone %s\n", tm.tm_year,
          tm.tm_gmtoff, tm.tm_zone);
  printf ("mktime %ld\n", (long) mktime (&tm));

  if (setjmp (jb) == 0)
    {
      count++;
      jumper (7);
    }
  else
    count += 10;
  printf ("setjmp count %d\n", count);
  if (sigsetjmp (sjb, 1) == 0)
    siglongjmp (sjb, 1);
  else
    printf ("siglongjmp ok\n");

  signal (SIGUSR1, handler);
  raise (SIGUSR1);
  printf ("signal got %d\n", got == SIGUSR1);

  memset (&sa, 0, sizeof sa);
  sa.sa_sigaction = info_handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset (&sa.sa_mask);
  sigaction (SIGUSR2, &sa, NULL);
  got = 0;
  kill (getpid (), SIGUSR2);
  printf ("sigaction got %d code SI_USER %d\n", got == SIGUSR2,
          info_code == SI_USER);

  sigemptyset (&set);
  sigaddset (&set, SIGUSR1);
  sigprocmask (SIG_BLOCK, &set, &old);
  got = 0;
  raise (SIGUSR1);
  printf ("blocked %d member %d\n", got, sigismember (&set, SIGUSR1));
  sigprocmask (SIG_SETMASK, &old, NULL);
  printf ("unblocked %d\n", got == SIGUSR1);
  printf ("SIG_IGN prev handler is ours %d\n",
          signal (SIGUSR1, SIG_IGN) == handler);
  printf ("NSIG %d SIGRTMIN>=32 %d\n", NSIG, SIGRTMIN >= 32);

  errno = 0;
  count2 = close (-1);
  printf ("close(-1) %d errno EBADF %d\n", count2, errno == EBADF);
  assert (count == 11);
  printf ("assert passed\n");
  return 0;
}
