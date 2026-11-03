/* AltC <signal.h> - signal handling (glibc-compatible, x86-64 Linux).  */

#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <features.h>
#include <_altc/types.h>
#include <_altc/sigset_t.h>
#include <_altc/time_types.h>

__BEGIN_DECLS

/* Signal numbers (<bits/signum-generic.h>, <bits/signum-arch.h>).  */
#define SIG_ERR ((__sighandler_t) -1)
#define SIG_DFL ((__sighandler_t) 0)
#define SIG_IGN ((__sighandler_t) 1)
#define SIG_HOLD ((__sighandler_t) 2)

#define SIGINT 2
#define SIGILL 4
#define SIGABRT 6
#define SIGFPE 8
#define SIGSEGV 11
#define SIGTERM 15
#define SIGHUP 1
#define SIGQUIT 3
#define SIGTRAP 5
#define SIGKILL 9
#define SIGPIPE 13
#define SIGALRM 14
#define SIGIOT SIGABRT
#define SIGSTKFLT 16
#define SIGPWR 30
#define SIGBUS 7
#define SIGSYS 31
#define SIGURG 23
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGCONT 18
#define SIGCHLD 17
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGPOLL 29
#define SIGXFSZ 25
#define SIGXCPU 24
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGUSR1 10
#define SIGUSR2 12
#define SIGWINCH 28
#define SIGIO SIGPOLL
#define SIGCLD SIGCHLD

#define __SIGRTMIN 32
#define __SIGRTMAX 64
#define _NSIG (__SIGRTMAX + 1)
#define NSIG _NSIG

#define SIGRTMIN (__libc_current_sigrtmin ())
#define SIGRTMAX (__libc_current_sigrtmax ())
extern int __libc_current_sigrtmin (void);
extern int __libc_current_sigrtmax (void);

#ifndef __sig_atomic_t_defined
#define __sig_atomic_t_defined 1
typedef __sig_atomic_t sig_atomic_t;
#endif

#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif
#ifndef __uid_t_defined
typedef __uid_t uid_t;
# define __uid_t_defined
#endif

/* union sigval / __sigval_t */
#ifndef ____sigval_t_defined
#define ____sigval_t_defined
union sigval
{
  int sival_int;
  void *sival_ptr;
};
typedef union sigval __sigval_t;
#endif

#ifndef __sigval_t_defined
#define __sigval_t_defined
typedef __sigval_t sigval_t;
#endif

/* siginfo_t */
#ifndef __siginfo_t_defined
#define __siginfo_t_defined 1
#define __SI_MAX_SIZE 128
#define __SI_PAD_SIZE ((__SI_MAX_SIZE / sizeof (int)) - 4)
typedef struct
  {
    int si_signo;
    int si_errno;
    int si_code;
    int __pad0;
    union
      {
        int _pad[__SI_PAD_SIZE];
        struct
          {
            __pid_t si_pid;
            __uid_t si_uid;
          } _kill;
        struct
          {
            int si_tid;
            int si_overrun;
            __sigval_t si_sigval;
          } _timer;
        struct
          {
            __pid_t si_pid;
            __uid_t si_uid;
            __sigval_t si_sigval;
          } _rt;
        struct
          {
            __pid_t si_pid;
            __uid_t si_uid;
            int si_status;
            __clock_t si_utime;
            __clock_t si_stime;
          } _sigchld;
        struct
          {
            void *si_addr;
            short int si_addr_lsb;
            union
              {
                struct
                  {
                    void *_lower;
                    void *_upper;
                  } _addr_bnd;
                __uint32_t _pkey;
              } _bounds;
          } _sigfault;
        struct
          {
            long int si_band;
            int si_fd;
          } _sigpoll;
        struct
          {
            void *_call_addr;
            int _syscall;
            unsigned int _arch;
          } _sigsys;
      } _sifields;
  } siginfo_t;

#define si_pid _sifields._kill.si_pid
#define si_uid _sifields._kill.si_uid
#define si_timerid _sifields._timer.si_tid
#define si_overrun _sifields._timer.si_overrun
#define si_status _sifields._sigchld.si_status
#define si_utime _sifields._sigchld.si_utime
#define si_stime _sifields._sigchld.si_stime
#define si_value _sifields._rt.si_sigval
#define si_int _sifields._rt.si_sigval.sival_int
#define si_ptr _sifields._rt.si_sigval.sival_ptr
#define si_addr _sifields._sigfault.si_addr
#define si_addr_lsb _sifields._sigfault.si_addr_lsb
#define si_lower _sifields._sigfault._bounds._addr_bnd._lower
#define si_upper _sifields._sigfault._bounds._addr_bnd._upper
#define si_pkey _sifields._sigfault._bounds._pkey
#define si_band _sifields._sigpoll.si_band
#define si_fd _sifields._sigpoll.si_fd
#define si_call_addr _sifields._sigsys._call_addr
#define si_syscall _sifields._sigsys._syscall
#define si_arch _sifields._sigsys._arch
#endif /* __siginfo_t_defined */

/* si_code values (<bits/siginfo-consts.h>).  */
#ifndef _BITS_SIGINFO_CONSTS_H
#define _BITS_SIGINFO_CONSTS_H 1
#define SI_ASYNCNL (-60)
#define SI_DETHREAD (-7)
#define SI_TKILL (-6)
#define SI_SIGIO (-5)
#define SI_ASYNCIO (-4)
#define SI_MESGQ (-3)
#define SI_TIMER (-2)
#define SI_QUEUE (-1)
#define SI_USER 0
#define SI_KERNEL 0x80

#define ILL_ILLOPC 1
#define ILL_ILLOPN 2
#define ILL_ILLADR 3
#define ILL_ILLTRP 4
#define ILL_PRVOPC 5
#define ILL_PRVREG 6
#define ILL_COPROC 7
#define ILL_BADSTK 8
#define ILL_BADIADDR 9

#define FPE_INTDIV 1
#define FPE_INTOVF 2
#define FPE_FLTDIV 3
#define FPE_FLTOVF 4
#define FPE_FLTUND 5
#define FPE_FLTRES 6
#define FPE_FLTINV 7
#define FPE_FLTSUB 8
#define FPE_FLTUNK 14
#define FPE_CONDTRAP 15

#define SEGV_MAPERR 1
#define SEGV_ACCERR 2
#define SEGV_BNDERR 3
#define SEGV_PKUERR 4
#define SEGV_ACCADI 5
#define SEGV_ADIDERR 6
#define SEGV_ADIPERR 7
#define SEGV_MTEAERR 8
#define SEGV_MTESERR 9
#define SEGV_CPERR 10

#define BUS_ADRALN 1
#define BUS_ADRERR 2
#define BUS_OBJERR 3
#define BUS_MCEERR_AR 4
#define BUS_MCEERR_AO 5

#define TRAP_BRKPT 1
#define TRAP_TRACE 2
#define TRAP_BRANCH 3
#define TRAP_HWBKPT 4
#define TRAP_UNK 5

#define CLD_EXITED 1
#define CLD_KILLED 2
#define CLD_DUMPED 3
#define CLD_TRAPPED 4
#define CLD_STOPPED 5
#define CLD_CONTINUED 6

#define POLL_IN 1
#define POLL_OUT 2
#define POLL_MSG 3
#define POLL_ERR 4
#define POLL_PRI 5
#define POLL_HUP 6

#endif /* _BITS_SIGINFO_CONSTS_H */

#include <_altc/sigevent.h>

#ifndef _BITS_SIGEVENT_CONSTS_H
#define _BITS_SIGEVENT_CONSTS_H 1
#define SIGEV_SIGNAL 0
#define SIGEV_NONE 1
#define SIGEV_THREAD 2
#define SIGEV_THREAD_ID 4
#endif

typedef void (*__sighandler_t) (int);

extern __sighandler_t __sysv_signal (int __sig, __sighandler_t __handler);
#ifdef __USE_GNU
extern __sighandler_t sysv_signal (int __sig, __sighandler_t __handler);
#endif
extern __sighandler_t signal (int __sig, __sighandler_t __handler);

extern int kill (__pid_t __pid, int __sig);
extern int killpg (__pid_t __pgrp, int __sig);
extern int raise (int __sig);

extern __sighandler_t ssignal (int __sig, __sighandler_t __handler);
extern int gsignal (int __sig);

extern void psignal (int __sig, const char *__s);
extern void psiginfo (const siginfo_t *__pinfo, const char *__s);

extern int __sigpause (int __sig_or_mask, int __is_sig);
#define sigpause(sig) __sigpause ((sig), 1)

extern int sigblock (int __mask) __attribute__ ((__deprecated__));
extern int sigsetmask (int __mask) __attribute__ ((__deprecated__));
extern int siggetmask (void) __attribute__ ((__deprecated__));
#define sigmask(sig) ((int)(1u << ((sig) - 1)))

#ifdef __USE_GNU
typedef __sighandler_t sighandler_t;
#endif
typedef __sighandler_t sig_t;

extern int sigemptyset (sigset_t *__set);
extern int sigfillset (sigset_t *__set);
extern int sigaddset (sigset_t *__set, int __signo);
extern int sigdelset (sigset_t *__set, int __signo);
extern int sigismember (const sigset_t *__set, int __signo);
#ifdef __USE_GNU
extern int sigisemptyset (const sigset_t *__set);
extern int sigandset (sigset_t *__set, const sigset_t *__left,
                      const sigset_t *__right);
extern int sigorset (sigset_t *__set, const sigset_t *__left,
                     const sigset_t *__right);
#endif

/* <bits/sigaction.h> */
#ifndef _BITS_SIGACTION_H
#define _BITS_SIGACTION_H 1
struct sigaction
  {
    union
      {
        __sighandler_t sa_handler;
        void (*sa_sigaction) (int, siginfo_t *, void *);
      }
    __sigaction_handler;
#define sa_handler __sigaction_handler.sa_handler
#define sa_sigaction __sigaction_handler.sa_sigaction
    __sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer) (void);
  };

#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO 4
#define SA_ONSTACK 0x08000000
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000
#define SA_INTERRUPT 0x20000000
#define SA_NOMASK SA_NODEFER
#define SA_ONESHOT SA_RESETHAND
#define SA_STACK SA_ONSTACK

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
#endif /* _BITS_SIGACTION_H */

extern int sigprocmask (int __how, const sigset_t *__restrict __set,
                        sigset_t *__restrict __oset);
extern int sigsuspend (const sigset_t *__set);
extern int sigaction (int __sig, const struct sigaction *__restrict __act,
                      struct sigaction *__restrict __oact);
extern int sigpending (sigset_t *__set);
extern int sigwait (const sigset_t *__restrict __set, int *__restrict __sig);
extern int sigwaitinfo (const sigset_t *__restrict __set,
                        siginfo_t *__restrict __info);
extern int sigtimedwait (const sigset_t *__restrict __set,
                         siginfo_t *__restrict __info,
                         const struct timespec *__restrict __timeout);
extern int sigqueue (__pid_t __pid, int __sig, const union sigval __val);

struct sigcontext;
extern int sigreturn (struct sigcontext *__scp);

extern int siginterrupt (int __sig, int __interrupt);

/* Alternate signal stacks (<bits/sigstack.h>, <bits/ss_flags.h>).  */
#include <_altc/stack_t.h>
#include <sys/ucontext.h>

#ifndef _BITS_SIGSTACK_H
#define _BITS_SIGSTACK_H 1
#define MINSIGSTKSZ 2048
#define SIGSTKSZ 8192
#endif

/* With _DYNAMIC_STACK_SIZE_SOURCE (implied by _GNU_SOURCE) the values
   come from the kernel at run time (<bits/sigstksz.h>).  */
#if defined __USE_DYNAMIC_STACK_SIZE && __USE_DYNAMIC_STACK_SIZE
# include <unistd.h>
# undef SIGSTKSZ
# define SIGSTKSZ sysconf (_SC_SIGSTKSZ)
# undef MINSIGSTKSZ
# define MINSIGSTKSZ SIGSTKSZ
#endif

#ifndef _BITS_SS_FLAGS_H
#define _BITS_SS_FLAGS_H 1
#define SS_ONSTACK 1
#define SS_DISABLE 2
#endif

#ifndef __sigstack_defined
#define __sigstack_defined 1
struct sigstack
  {
    void *ss_sp;
    int ss_onstack;
  };
#endif

extern int sigstack (struct sigstack *__ss, struct sigstack *__oss)
     __attribute__ ((__deprecated__));
extern int sigaltstack (const stack_t *__restrict __ss,
                        stack_t *__restrict __oss);

extern int sighold (int __sig);
extern int sigrelse (int __sig);
extern int sigignore (int __sig);
extern __sighandler_t sigset (int __sig, __sighandler_t __disp);

/* <bits/sigthread.h> */
extern int pthread_sigmask (int __how, const __sigset_t *__restrict __newmask,
                            __sigset_t *__restrict __oldmask);
extern int pthread_kill (pthread_t __threadid, int __signo);
#ifdef __USE_GNU
extern int pthread_sigqueue (pthread_t __threadid, int __signo,
                             const union sigval __value);
extern int tgkill (__pid_t __tgid, __pid_t __tid, int __signal);
#endif

__END_DECLS

#endif /* signal.h */
