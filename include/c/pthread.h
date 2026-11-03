/* AltC <pthread.h> - POSIX threads (glibc-compatible, x86-64).
   Since glibc 2.34 all functions live in libc.so.6.  */

#ifndef _PTHREAD_H
#define _PTHREAD_H 1

#include <features.h>
#include <sched.h>
#include <time.h>
#include <_altc/types.h>
#include <_altc/pthreadtypes.h>
#include <_altc/jmp_buf_tag.h>
#include <_altc/pthread_stack_min.h>

#define __need_NULL
#include <stddef.h>

enum
{
  PTHREAD_CREATE_JOINABLE,
#define PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_JOINABLE
  PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_DETACHED PTHREAD_CREATE_DETACHED
};

enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
#ifdef __USE_GNU
  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP
#endif
};

enum
{
  PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_STALLED_NP = PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_ROBUST,
  PTHREAD_MUTEX_ROBUST_NP = PTHREAD_MUTEX_ROBUST
};

enum
{
  PTHREAD_PRIO_NONE,
  PTHREAD_PRIO_INHERIT,
  PTHREAD_PRIO_PROTECT
};

#define PTHREAD_MUTEX_INITIALIZER \
 { { __PTHREAD_MUTEX_INITIALIZER (PTHREAD_MUTEX_TIMED_NP) } }
#ifdef __USE_GNU
# define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
 { { __PTHREAD_MUTEX_INITIALIZER (PTHREAD_MUTEX_RECURSIVE_NP) } }
# define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP \
 { { __PTHREAD_MUTEX_INITIALIZER (PTHREAD_MUTEX_ERRORCHECK_NP) } }
# define PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP \
 { { __PTHREAD_MUTEX_INITIALIZER (PTHREAD_MUTEX_ADAPTIVE_NP) } }
#endif

enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP
};

#define PTHREAD_RWLOCK_INITIALIZER \
  { { __PTHREAD_RWLOCK_INITIALIZER (PTHREAD_RWLOCK_DEFAULT_NP) } }
#ifdef __USE_GNU
# define PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP \
  { { __PTHREAD_RWLOCK_INITIALIZER \
      (PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP) } }
#endif

enum
{
  PTHREAD_INHERIT_SCHED,
#define PTHREAD_INHERIT_SCHED PTHREAD_INHERIT_SCHED
  PTHREAD_EXPLICIT_SCHED
#define PTHREAD_EXPLICIT_SCHED PTHREAD_EXPLICIT_SCHED
};

enum
{
  PTHREAD_SCOPE_SYSTEM,
#define PTHREAD_SCOPE_SYSTEM PTHREAD_SCOPE_SYSTEM
  PTHREAD_SCOPE_PROCESS
#define PTHREAD_SCOPE_PROCESS PTHREAD_SCOPE_PROCESS
};

enum
{
  PTHREAD_PROCESS_PRIVATE,
#define PTHREAD_PROCESS_PRIVATE PTHREAD_PROCESS_PRIVATE
  PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED PTHREAD_PROCESS_SHARED
};

#define PTHREAD_COND_INITIALIZER { { {0}, {0}, {0, 0}, 0, 0, {0, 0}, 0, 0 } }

struct _pthread_cleanup_buffer
{
  void (*__routine) (void *);
  void *__arg;
  int __canceltype;
  struct _pthread_cleanup_buffer *__prev;
};

enum
{
  PTHREAD_CANCEL_ENABLE,
#define PTHREAD_CANCEL_ENABLE PTHREAD_CANCEL_ENABLE
  PTHREAD_CANCEL_DISABLE
#define PTHREAD_CANCEL_DISABLE PTHREAD_CANCEL_DISABLE
};
enum
{
  PTHREAD_CANCEL_DEFERRED,
#define PTHREAD_CANCEL_DEFERRED PTHREAD_CANCEL_DEFERRED
  PTHREAD_CANCEL_ASYNCHRONOUS
#define PTHREAD_CANCEL_ASYNCHRONOUS PTHREAD_CANCEL_ASYNCHRONOUS
};
#define PTHREAD_CANCELED ((void *) -1)

#define PTHREAD_ONCE_INIT 0

#define PTHREAD_BARRIER_SERIAL_THREAD -1

#ifdef __USE_GNU
# define PTHREAD_ATTR_NO_SIGMASK_NP (-1)
#endif

__BEGIN_DECLS

/* Cancellation cleanup buffers (C version: setjmp-based).  */
typedef struct
{
  struct
  {
    __jmp_buf __cancel_jmp_buf;
    int __mask_was_saved;
  } __cancel_jmp_buf[1];
  void *__pad[4];
} __pthread_unwind_buf_t __attribute__ ((__aligned__ (16)));

struct __pthread_cleanup_frame
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;
};

extern int pthread_create (pthread_t *__restrict __newthread,
                           const pthread_attr_t *__restrict __attr,
                           void *(*__start_routine) (void *),
                           void *__restrict __arg);
extern void pthread_exit (void *__retval) __attribute__ ((__noreturn__));
extern int pthread_join (pthread_t __th, void **__thread_return);
extern int pthread_detach (pthread_t __th);
extern pthread_t pthread_self (void);
extern int pthread_equal (pthread_t __thread1, pthread_t __thread2);
extern int pthread_attr_init (pthread_attr_t *__attr);
extern int pthread_attr_destroy (pthread_attr_t *__attr);
extern int pthread_attr_getdetachstate (const pthread_attr_t *__attr,
                                        int *__detachstate);
extern int pthread_attr_setdetachstate (pthread_attr_t *__attr,
                                        int __detachstate);
extern int pthread_attr_getguardsize (const pthread_attr_t *__attr,
                                      size_t *__guardsize);
extern int pthread_attr_setguardsize (pthread_attr_t *__attr,
                                      size_t __guardsize);
extern int pthread_attr_getschedparam (const pthread_attr_t *__restrict __attr,
                                       struct sched_param *__restrict __param);
extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr,
                                       const struct sched_param *__restrict __param);
extern int pthread_attr_getschedpolicy (const pthread_attr_t *__restrict __attr,
                                        int *__restrict __policy);
extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy);
extern int pthread_attr_getinheritsched (const pthread_attr_t *__restrict __attr,
                                         int *__restrict __inherit);
extern int pthread_attr_setinheritsched (pthread_attr_t *__attr,
                                         int __inherit);
extern int pthread_attr_getscope (const pthread_attr_t *__restrict __attr,
                                  int *__restrict __scope);
extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope);
extern int pthread_attr_getstackaddr (const pthread_attr_t *__restrict __attr,
                                      void **__restrict __stackaddr);
extern int pthread_attr_setstackaddr (pthread_attr_t *__attr,
                                      void *__stackaddr);
extern int pthread_attr_getstacksize (const pthread_attr_t *__restrict __attr,
                                      size_t *__restrict __stacksize);
extern int pthread_attr_setstacksize (pthread_attr_t *__attr,
                                      size_t __stacksize);
extern int pthread_attr_getstack (const pthread_attr_t *__restrict __attr,
                                  void **__restrict __stackaddr,
                                  size_t *__restrict __stacksize);
extern int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
                                  size_t __stacksize);
extern int pthread_setschedparam (pthread_t __target_thread, int __policy,
                                  const struct sched_param *__param);
extern int pthread_getschedparam (pthread_t __target_thread,
                                  int *__restrict __policy,
                                  struct sched_param *__restrict __param);
extern int pthread_setschedprio (pthread_t __target_thread, int __prio);
extern int pthread_once (pthread_once_t *__once_control,
                         void (*__init_routine) (void));
extern int pthread_setcancelstate (int __state, int *__oldstate);
extern int pthread_setcanceltype (int __type, int *__oldtype);
extern int pthread_cancel (pthread_t __th);
extern void pthread_testcancel (void);
extern void __pthread_register_cancel (__pthread_unwind_buf_t *__buf);
extern void __pthread_unregister_cancel (__pthread_unwind_buf_t *__buf);
extern void __pthread_unwind_next (__pthread_unwind_buf_t *__buf)
     __attribute__ ((__noreturn__));
extern int pthread_mutex_init (pthread_mutex_t *__mutex,
                               const pthread_mutexattr_t *__mutexattr);
extern int pthread_mutex_destroy (pthread_mutex_t *__mutex);
extern int pthread_mutex_trylock (pthread_mutex_t *__mutex);
extern int pthread_mutex_lock (pthread_mutex_t *__mutex);
extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,
                                    const struct timespec *__restrict __abstime);
extern int pthread_mutex_unlock (pthread_mutex_t *__mutex);
extern int pthread_mutex_getprioceiling (const pthread_mutex_t * __restrict __mutex,
                                         int *__restrict __prioceiling);
extern int pthread_mutex_setprioceiling (pthread_mutex_t *__restrict __mutex,
                                         int __prioceiling,
                                         int *__restrict __old_ceiling);
extern int pthread_mutex_consistent (pthread_mutex_t *__mutex);
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr);
extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr);
extern int pthread_mutexattr_getpshared (const pthread_mutexattr_t * __restrict __attr,
                                         int *__restrict __pshared);
extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
                                         int __pshared);
extern int pthread_mutexattr_gettype (const pthread_mutexattr_t *__restrict __attr,
                                      int *__restrict __kind);
extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);
extern int pthread_mutexattr_getprotocol (const pthread_mutexattr_t * __restrict __attr,
                                          int *__restrict __protocol);
extern int pthread_mutexattr_setprotocol (pthread_mutexattr_t *__attr,
                                          int __protocol);
extern int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t * __restrict __attr,
                                             int *__restrict __prioceiling);
extern int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *__attr,
                                             int __prioceiling);
extern int pthread_mutexattr_getrobust (const pthread_mutexattr_t *__attr,
                                        int *__robustness);
extern int pthread_mutexattr_setrobust (pthread_mutexattr_t *__attr,
                                        int __robustness);
extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
                                const pthread_rwlockattr_t *__restrict __attr);
extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock);
extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock);
extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock);
extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
                                       const struct timespec *__restrict __abstime);
extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock);
extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock);
extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
                                       const struct timespec *__restrict __abstime);
extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock);
extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr);
extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr);
extern int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t * __restrict __attr,
                                          int *__restrict __pshared);
extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr,
                                          int __pshared);
extern int pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t * __restrict __attr,
                                          int *__restrict __pref);
extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr,
                                          int __pref);
extern int pthread_cond_init (pthread_cond_t *__restrict __cond,
                              const pthread_condattr_t *__restrict __cond_attr);
extern int pthread_cond_destroy (pthread_cond_t *__cond);
extern int pthread_cond_signal (pthread_cond_t *__cond);
extern int pthread_cond_broadcast (pthread_cond_t *__cond);
extern int pthread_cond_wait (pthread_cond_t *__restrict __cond,
                              pthread_mutex_t *__restrict __mutex);
extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
                                   pthread_mutex_t *__restrict __mutex,
                                   const struct timespec *__restrict __abstime);
extern int pthread_condattr_init (pthread_condattr_t *__attr);
extern int pthread_condattr_destroy (pthread_condattr_t *__attr);
extern int pthread_condattr_getpshared (const pthread_condattr_t * __restrict __attr,
                                        int *__restrict __pshared);
extern int pthread_condattr_setpshared (pthread_condattr_t *__attr,
                                        int __pshared);
extern int pthread_condattr_getclock (const pthread_condattr_t * __restrict __attr,
                                      __clockid_t *__restrict __clock_id);
extern int pthread_condattr_setclock (pthread_condattr_t *__attr,
                                      __clockid_t __clock_id);
extern int pthread_spin_init (pthread_spinlock_t *__lock, int __pshared);
extern int pthread_spin_destroy (pthread_spinlock_t *__lock);
extern int pthread_spin_lock (pthread_spinlock_t *__lock);
extern int pthread_spin_trylock (pthread_spinlock_t *__lock);
extern int pthread_spin_unlock (pthread_spinlock_t *__lock);
extern int pthread_barrier_init (pthread_barrier_t *__restrict __barrier,
                                 const pthread_barrierattr_t *__restrict __attr,
                                 unsigned int __count);
extern int pthread_barrier_destroy (pthread_barrier_t *__barrier);
extern int pthread_barrier_wait (pthread_barrier_t *__barrier);
extern int pthread_barrierattr_init (pthread_barrierattr_t *__attr);
extern int pthread_barrierattr_destroy (pthread_barrierattr_t *__attr);
extern int pthread_barrierattr_getpshared (const pthread_barrierattr_t * __restrict __attr,
                                           int *__restrict __pshared);
extern int pthread_barrierattr_setpshared (pthread_barrierattr_t *__attr,
                                           int __pshared);
extern int pthread_key_create (pthread_key_t *__key,
                               void (*__destr_function) (void *));
extern int pthread_key_delete (pthread_key_t __key);
extern void *pthread_getspecific (pthread_key_t __key);
extern int pthread_setspecific (pthread_key_t __key, const void *__pointer);
extern int pthread_getcpuclockid (pthread_t __thread_id,
                                  __clockid_t *__clock_id);
extern int pthread_atfork (void (*__prepare) (void), void (*__parent) (void),
                           void (*__child) (void));

#ifdef __USE_GNU
extern int pthread_tryjoin_np (pthread_t __th, void **__thread_return);
extern int pthread_timedjoin_np (pthread_t __th, void **__thread_return,
                                 const struct timespec *__abstime);
extern int pthread_clockjoin_np (pthread_t __th, void **__thread_return,
                                 clockid_t __clockid,
                                 const struct timespec *__abstime);
extern int pthread_attr_setaffinity_np (pthread_attr_t *__attr,
                                        size_t __cpusetsize,
                                        const cpu_set_t *__cpuset);
extern int pthread_attr_getaffinity_np (const pthread_attr_t *__attr,
                                        size_t __cpusetsize,
                                        cpu_set_t *__cpuset);
extern int pthread_getattr_default_np (pthread_attr_t *__attr);
extern int pthread_attr_setsigmask_np (pthread_attr_t *__attr,
                                       const __sigset_t *sigmask);
extern int pthread_attr_getsigmask_np (const pthread_attr_t *__attr,
                                       __sigset_t *sigmask);
extern int pthread_setattr_default_np (const pthread_attr_t *__attr);
extern int pthread_getattr_np (pthread_t __th, pthread_attr_t *__attr);
extern int pthread_getname_np (pthread_t __target_thread, char *__buf,
                               size_t __buflen);
extern int pthread_setname_np (pthread_t __target_thread, const char *__name);
extern int pthread_getconcurrency (void);
extern int pthread_setconcurrency (int __level);
extern int pthread_setaffinity_np (pthread_t __th, size_t __cpusetsize,
                                   const cpu_set_t *__cpuset);
extern int pthread_getaffinity_np (pthread_t __th, size_t __cpusetsize,
                                   cpu_set_t *__cpuset);
extern int pthread_mutex_clocklock (pthread_mutex_t *__restrict __mutex,
                                    clockid_t __clockid,
                                    const struct timespec *__restrict __abstime);
extern int pthread_rwlock_clockrdlock (pthread_rwlock_t *__restrict __rwlock,
                                       clockid_t __clockid,
                                       const struct timespec *__restrict __abstime);
extern int pthread_rwlock_clockwrlock (pthread_rwlock_t *__restrict __rwlock,
                                       clockid_t __clockid,
                                       const struct timespec *__restrict __abstime);
extern int pthread_cond_clockwait (pthread_cond_t *__restrict __cond,
                                   pthread_mutex_t *__restrict __mutex,
                                   __clockid_t __clock_id,
                                   const struct timespec *__restrict __abstime);
#endif

#define __sigsetjmp_cancel(env, savemask) \
  __sigsetjmp ((struct __jmp_buf_tag *) (void *) (env), (savemask))
extern int __sigsetjmp (struct __jmp_buf_tag __env[1], int __savemask)
     __attribute__ ((__returns_twice__));

#define pthread_cleanup_push(routine, arg) \
  do { \
    __pthread_unwind_buf_t __cancel_buf; \
    void (*__cancel_routine) (void *) = (routine); \
    void *__cancel_arg = (arg); \
    int __not_first_call = __sigsetjmp_cancel (__cancel_buf.__cancel_jmp_buf, \
                                               0); \
    if (__not_first_call) \
      { \
        __cancel_routine (__cancel_arg); \
        __pthread_unwind_next (&__cancel_buf); \
      } \
    __pthread_register_cancel (&__cancel_buf); \
    do {

#define pthread_cleanup_pop(execute) \
      do { } while (0); \
    } while (0); \
    __pthread_unregister_cancel (&__cancel_buf); \
    if (execute) \
      __cancel_routine (__cancel_arg); \
  } while (0)

__END_DECLS

#endif /* pthread.h */
