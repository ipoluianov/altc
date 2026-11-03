/* AltC private header: wait status macros and waitpid flags, shared by
   <stdlib.h> and <sys/wait.h> (token-identical to glibc's
   <bits/waitstatus.h> and <bits/waitflags.h>, which have no guards).  */

#ifndef __altc_wait_h
#define __altc_wait_h 1

#define WNOHANG 1
#define WUNTRACED 2
#define WSTOPPED 2
#define WEXITED 4
#define WCONTINUED 8
#define WNOWAIT 0x01000000
#define __WNOTHREAD 0x20000000
#define __WALL 0x40000000
#define __WCLONE 0x80000000

#define __WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#define __WTERMSIG(status) ((status) & 0x7f)
#define __WSTOPSIG(status) __WEXITSTATUS(status)
#define __WIFEXITED(status) (__WTERMSIG(status) == 0)
#define __WIFSIGNALED(status) \
  (((signed char) (((status) & 0x7f) + 1) >> 1) > 0)
#define __WIFSTOPPED(status) (((status) & 0xff) == 0x7f)
#define __WIFCONTINUED(status) ((status) == __W_CONTINUED)
#define __WCOREDUMP(status) ((status) & __WCOREFLAG)
#define __W_EXITCODE(ret, sig) ((ret) << 8 | (sig))
#define __W_STOPCODE(sig) ((sig) << 8 | 0x7f)
#define __W_CONTINUED 0xffff
#define __WCOREFLAG 0x80

#define WEXITSTATUS(status) __WEXITSTATUS (status)
#define WTERMSIG(status) __WTERMSIG (status)
#define WSTOPSIG(status) __WSTOPSIG (status)
#define WIFEXITED(status) __WIFEXITED (status)
#define WIFSIGNALED(status) __WIFSIGNALED (status)
#define WIFSTOPPED(status) __WIFSTOPPED (status)
#define WIFCONTINUED(status) __WIFCONTINUED (status)

#endif
