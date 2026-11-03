/* AltC <sys/resource.h> - resource limits and usage (x86-64 Linux).  */

#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/time_types.h>
#include <_altc/rusage.h>

__BEGIN_DECLS

enum __rlimit_resource
{
  RLIMIT_CPU = 0,
#define RLIMIT_CPU RLIMIT_CPU
  RLIMIT_FSIZE = 1,
#define RLIMIT_FSIZE RLIMIT_FSIZE
  RLIMIT_DATA = 2,
#define RLIMIT_DATA RLIMIT_DATA
  RLIMIT_STACK = 3,
#define RLIMIT_STACK RLIMIT_STACK
  RLIMIT_CORE = 4,
#define RLIMIT_CORE RLIMIT_CORE
  __RLIMIT_RSS = 5,
#define RLIMIT_RSS __RLIMIT_RSS
  RLIMIT_NOFILE = 7,
#define RLIMIT_NOFILE RLIMIT_NOFILE
  __RLIMIT_OFILE = RLIMIT_NOFILE,
#define RLIMIT_OFILE __RLIMIT_OFILE
  RLIMIT_AS = 9,
#define RLIMIT_AS RLIMIT_AS
  __RLIMIT_NPROC = 6,
#define RLIMIT_NPROC __RLIMIT_NPROC
  __RLIMIT_MEMLOCK = 8,
#define RLIMIT_MEMLOCK __RLIMIT_MEMLOCK
  __RLIMIT_LOCKS = 10,
#define RLIMIT_LOCKS __RLIMIT_LOCKS
  __RLIMIT_SIGPENDING = 11,
#define RLIMIT_SIGPENDING __RLIMIT_SIGPENDING
  __RLIMIT_MSGQUEUE = 12,
#define RLIMIT_MSGQUEUE __RLIMIT_MSGQUEUE
  __RLIMIT_NICE = 13,
#define RLIMIT_NICE __RLIMIT_NICE
  __RLIMIT_RTPRIO = 14,
#define RLIMIT_RTPRIO __RLIMIT_RTPRIO
  __RLIMIT_RTTIME = 15,
#define RLIMIT_RTTIME __RLIMIT_RTTIME
  __RLIMIT_NLIMITS = 16,
#define RLIMIT_NLIMITS __RLIMIT_NLIMITS
  __RLIM_NLIMITS = __RLIMIT_NLIMITS
#define RLIM_NLIMITS __RLIM_NLIMITS
};

#define RLIM_INFINITY ((__rlim_t) -1)
#ifdef __USE_LARGEFILE64
# define RLIM64_INFINITY 0xffffffffffffffffuLL
#endif
#define RLIM_SAVED_MAX RLIM_INFINITY
#define RLIM_SAVED_CUR RLIM_INFINITY

#ifndef __id_t_defined
typedef __id_t id_t;
# define __id_t_defined
#endif

typedef __rlim_t rlim_t;
#ifdef __USE_LARGEFILE64
typedef __rlim64_t rlim64_t;
#endif

struct rlimit
  {
    rlim_t rlim_cur;
    rlim_t rlim_max;
  };

#ifdef __USE_LARGEFILE64
struct rlimit64
  {
    rlim64_t rlim_cur;
    rlim64_t rlim_max;
  };
#endif

enum __rusage_who
{
  RUSAGE_SELF = 0,
#define RUSAGE_SELF RUSAGE_SELF
  RUSAGE_CHILDREN = -1
#define RUSAGE_CHILDREN RUSAGE_CHILDREN
#ifdef __USE_GNU
  ,
  RUSAGE_THREAD = 1
# define RUSAGE_THREAD RUSAGE_THREAD
# define RUSAGE_LWP RUSAGE_THREAD
#endif
};

#define PRIO_MIN -20
#define PRIO_MAX 20

enum __priority_which
{
  PRIO_PROCESS = 0,
#define PRIO_PROCESS PRIO_PROCESS
  PRIO_PGRP = 1,
#define PRIO_PGRP PRIO_PGRP
  PRIO_USER = 2
#define PRIO_USER PRIO_USER
};

/* glibc uses the enum types with _GNU_SOURCE and int otherwise; both have
   the same ABI.  */
#ifdef __USE_GNU
typedef enum __rlimit_resource __rlimit_resource_t;
typedef enum __rusage_who __rusage_who_t;
typedef enum __priority_which __priority_which_t;
#else
typedef int __rlimit_resource_t;
typedef int __rusage_who_t;
typedef int __priority_which_t;
#endif

extern int getrlimit (__rlimit_resource_t __resource,
                      struct rlimit *__rlimits);
extern int setrlimit (__rlimit_resource_t __resource,
                      const struct rlimit *__rlimits);
#ifdef __USE_LARGEFILE64
extern int getrlimit64 (__rlimit_resource_t __resource,
                        struct rlimit64 *__rlimits);
extern int setrlimit64 (__rlimit_resource_t __resource,
                        const struct rlimit64 *__rlimits);
#endif
extern int getrusage (__rusage_who_t __who, struct rusage *__usage);
extern int getpriority (__priority_which_t __which, id_t __who);
extern int setpriority (__priority_which_t __which, id_t __who, int __prio);

#ifdef __USE_GNU
extern int prlimit (__pid_t __pid, enum __rlimit_resource __resource,
                    const struct rlimit *__new_limit,
                    struct rlimit *__old_limit);
extern int prlimit64 (__pid_t __pid, enum __rlimit_resource __resource,
                      const struct rlimit64 *__new_limit,
                      struct rlimit64 *__old_limit);
#endif

__END_DECLS

#endif /* sys/resource.h */
