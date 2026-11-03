/* AltC <sched.h> - execution scheduling (glibc-compatible, x86-64 Linux).  */

#ifndef _SCHED_H
#define _SCHED_H 1

#include <features.h>
#include <_altc/types.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#include <_altc/time_types.h>

#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif

/* <bits/sched.h> */
#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2
#ifdef __USE_GNU
# define SCHED_NORMAL SCHED_OTHER
# define SCHED_BATCH 3
# define SCHED_ISO 4
# define SCHED_IDLE 5
# define SCHED_DEADLINE 6
# define SCHED_EXT 7
# define SCHED_RESET_ON_FORK 0x40000000

# define CSIGNAL 0x000000ff
# define CLONE_VM 0x00000100
# define CLONE_FS 0x00000200
# define CLONE_FILES 0x00000400
# define CLONE_SIGHAND 0x00000800
# define CLONE_PIDFD 0x00001000
# define CLONE_PTRACE 0x00002000
# define CLONE_VFORK 0x00004000
# define CLONE_PARENT 0x00008000
# define CLONE_THREAD 0x00010000
# define CLONE_NEWNS 0x00020000
# define CLONE_SYSVSEM 0x00040000
# define CLONE_SETTLS 0x00080000
# define CLONE_PARENT_SETTID 0x00100000
# define CLONE_CHILD_CLEARTID 0x00200000
# define CLONE_DETACHED 0x00400000
# define CLONE_UNTRACED 0x00800000
# define CLONE_CHILD_SETTID 0x01000000
# define CLONE_NEWCGROUP 0x02000000
# define CLONE_NEWUTS 0x04000000
# define CLONE_NEWIPC 0x08000000
# define CLONE_NEWUSER 0x10000000
# define CLONE_NEWPID 0x20000000
# define CLONE_NEWNET 0x40000000
# define CLONE_IO 0x80000000
# define CLONE_NEWTIME 0x00000080
#endif

#ifndef _BITS_TYPES_STRUCT_SCHED_PARAM
#define _BITS_TYPES_STRUCT_SCHED_PARAM 1
struct sched_param
{
  int sched_priority;
};
#endif
#define sched_priority sched_priority

__BEGIN_DECLS

#ifdef __USE_GNU
extern int clone (int (*__fn) (void *__arg), void *__child_stack,
                  int __flags, void *__arg, ...);
extern int unshare (int __flags);
extern int sched_getcpu (void);
extern int getcpu (unsigned int *, unsigned int *);
extern int setns (int __fd, int __nstype);
#endif

/* <bits/cpu-set.h>.  The macros below avoid GNU statement expressions, so
   the CPU argument of CPU_SET/CPU_CLR/CPU_ISSET may be evaluated twice.  */
#ifndef _BITS_CPU_SET_H
#define _BITS_CPU_SET_H 1

#define __CPU_SETSIZE 1024
#define __NCPUBITS (8 * sizeof (__cpu_mask))

typedef __CPU_MASK_TYPE __cpu_mask;

#define __CPUELT(cpu) ((cpu) / __NCPUBITS)
#define __CPUMASK(cpu) ((__cpu_mask) 1 << ((cpu) % __NCPUBITS))

typedef struct
{
  __cpu_mask __bits[__CPU_SETSIZE / __NCPUBITS];
} cpu_set_t;

#define __CPU_ZERO_S(setsize, cpusetp) \
  do { \
    size_t __i; \
    size_t __imax = (setsize) / sizeof (__cpu_mask); \
    __cpu_mask *__bits = (cpusetp)->__bits; \
    for (__i = 0; __i < __imax; ++__i) \
      __bits[__i] = 0; \
  } while (0)
#define __CPU_SET_S(cpu, setsize, cpusetp) \
  ((size_t) (cpu) / 8 < (setsize) \
   ? (((__cpu_mask *) ((cpusetp)->__bits))[__CPUELT ((size_t) (cpu))] \
      |= __CPUMASK ((size_t) (cpu))) \
   : 0)
#define __CPU_CLR_S(cpu, setsize, cpusetp) \
  ((size_t) (cpu) / 8 < (setsize) \
   ? (((__cpu_mask *) ((cpusetp)->__bits))[__CPUELT ((size_t) (cpu))] \
      &= ~__CPUMASK ((size_t) (cpu))) \
   : 0)
#define __CPU_ISSET_S(cpu, setsize, cpusetp) \
  ((size_t) (cpu) / 8 < (setsize) \
   ? ((((const __cpu_mask *) ((cpusetp)->__bits))[__CPUELT ((size_t) (cpu))] \
       & __CPUMASK ((size_t) (cpu)))) != 0 \
   : 0)
#define __CPU_COUNT_S(setsize, cpusetp) \
  __sched_cpucount (setsize, cpusetp)
#define __CPU_EQUAL_S(setsize, cpusetp1, cpusetp2) \
  (__altc_cpu_equal ((setsize), (cpusetp1), (cpusetp2)))
#define __CPU_OP_S(setsize, destset, srcset1, srcset2, op) \
  do { \
    cpu_set_t *__dest = (destset); \
    const __cpu_mask *__arr1 = (srcset1)->__bits; \
    const __cpu_mask *__arr2 = (srcset2)->__bits; \
    size_t __imax = (setsize) / sizeof (__cpu_mask); \
    size_t __i; \
    for (__i = 0; __i < __imax; ++__i) \
      ((__cpu_mask *) __dest->__bits)[__i] = __arr1[__i] op __arr2[__i]; \
  } while (0)
#define __CPU_ALLOC_SIZE(count) \
  ((((count) + __NCPUBITS - 1) / __NCPUBITS) * sizeof (__cpu_mask))
#define __CPU_ALLOC(count) __sched_cpualloc (count)
#define __CPU_FREE(cpuset) __sched_cpufree (cpuset)

extern int __sched_cpucount (size_t __setsize, const cpu_set_t *__setp);
extern cpu_set_t *__sched_cpualloc (size_t __count);
extern void __sched_cpufree (cpu_set_t *__set);

static __inline __attribute__ ((__unused__)) int
__altc_cpu_equal (size_t __setsize, const cpu_set_t *__s1,
                  const cpu_set_t *__s2)
{
  size_t __i;
  for (__i = 0; __i < __setsize / sizeof (__cpu_mask); ++__i)
    if (__s1->__bits[__i] != __s2->__bits[__i])
      return 0;
  return 1;
}

#endif /* _BITS_CPU_SET_H */

#ifdef __USE_GNU
# define CPU_SETSIZE __CPU_SETSIZE
# define CPU_SET(cpu, cpusetp) __CPU_SET_S (cpu, sizeof (cpu_set_t), cpusetp)
# define CPU_CLR(cpu, cpusetp) __CPU_CLR_S (cpu, sizeof (cpu_set_t), cpusetp)
# define CPU_ISSET(cpu, cpusetp) __CPU_ISSET_S (cpu, sizeof (cpu_set_t), \
                                                cpusetp)
# define CPU_ZERO(cpusetp) __CPU_ZERO_S (sizeof (cpu_set_t), cpusetp)
# define CPU_COUNT(cpusetp) __CPU_COUNT_S (sizeof (cpu_set_t), cpusetp)

# define CPU_SET_S(cpu, setsize, cpusetp) __CPU_SET_S (cpu, setsize, cpusetp)
# define CPU_CLR_S(cpu, setsize, cpusetp) __CPU_CLR_S (cpu, setsize, cpusetp)
# define CPU_ISSET_S(cpu, setsize, cpusetp) __CPU_ISSET_S (cpu, setsize, \
                                                           cpusetp)
# define CPU_ZERO_S(setsize, cpusetp) __CPU_ZERO_S (setsize, cpusetp)
# define CPU_COUNT_S(setsize, cpusetp) __CPU_COUNT_S (setsize, cpusetp)

# define CPU_EQUAL(cpusetp1, cpusetp2) \
  __CPU_EQUAL_S (sizeof (cpu_set_t), cpusetp1, cpusetp2)
# define CPU_EQUAL_S(setsize, cpusetp1, cpusetp2) \
  __CPU_EQUAL_S (setsize, cpusetp1, cpusetp2)

# define CPU_AND(destset, srcset1, srcset2) \
  __CPU_OP_S (sizeof (cpu_set_t), destset, srcset1, srcset2, &)
# define CPU_OR(destset, srcset1, srcset2) \
  __CPU_OP_S (sizeof (cpu_set_t), destset, srcset1, srcset2, |)
# define CPU_XOR(destset, srcset1, srcset2) \
  __CPU_OP_S (sizeof (cpu_set_t), destset, srcset1, srcset2, ^)
# define CPU_AND_S(setsize, destset, srcset1, srcset2) \
  __CPU_OP_S (setsize, destset, srcset1, srcset2, &)
# define CPU_OR_S(setsize, destset, srcset1, srcset2) \
  __CPU_OP_S (setsize, destset, srcset1, srcset2, |)
# define CPU_XOR_S(setsize, destset, srcset1, srcset2) \
  __CPU_OP_S (setsize, destset, srcset1, srcset2, ^)

# define CPU_ALLOC_SIZE(count) __CPU_ALLOC_SIZE (count)
# define CPU_ALLOC(count) __CPU_ALLOC (count)
# define CPU_FREE(cpuset) __CPU_FREE (cpuset)
#endif

extern int sched_setparam (__pid_t __pid, const struct sched_param *__param);
extern int sched_getparam (__pid_t __pid, struct sched_param *__param);
extern int sched_setscheduler (__pid_t __pid, int __policy,
                               const struct sched_param *__param);
extern int sched_getscheduler (__pid_t __pid);
extern int sched_yield (void);
extern int sched_get_priority_max (int __algorithm);
extern int sched_get_priority_min (int __algorithm);
extern int sched_rr_get_interval (__pid_t __pid, struct timespec *__t);

#ifdef __USE_GNU
extern int sched_setaffinity (__pid_t __pid, size_t __cpusetsize,
                              const cpu_set_t *__cpuset);
extern int sched_getaffinity (__pid_t __pid, size_t __cpusetsize,
                              cpu_set_t *__cpuset);
#endif

__END_DECLS

#endif /* sched.h */
