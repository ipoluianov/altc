/* AltC <sys/wait.h> - waiting for process termination.  */

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H 1

#include <features.h>
#include <_altc/types.h>
#include <signal.h>
#include <_altc/rusage.h>
#include <_altc/wait.h>

__BEGIN_DECLS

#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif
#ifndef __id_t_defined
typedef __id_t id_t;
# define __id_t_defined
#endif

#define WCOREFLAG __WCOREFLAG
#define WCOREDUMP(status) __WCOREDUMP (status)
#define W_EXITCODE(ret, sig) __W_EXITCODE (ret, sig)
#define W_STOPCODE(sig) __W_STOPCODE (sig)

#define WAIT_ANY (-1)
#define WAIT_MYPGRP 0

#ifndef __idtype_t_defined
#define __idtype_t_defined
typedef enum
{
  P_ALL,
  P_PID,
  P_PGID,
  P_PIDFD
} idtype_t;
#endif

extern __pid_t wait (int *__stat_loc);
extern __pid_t waitpid (__pid_t __pid, int *__stat_loc, int __options);
extern int waitid (idtype_t __idtype, __id_t __id, siginfo_t *__infop,
                   int __options);

struct rusage;
extern __pid_t wait3 (int *__stat_loc, int __options,
                      struct rusage *__usage);
extern __pid_t wait4 (__pid_t __pid, int *__stat_loc, int __options,
                      struct rusage *__usage);

__END_DECLS

#endif /* sys/wait.h */
