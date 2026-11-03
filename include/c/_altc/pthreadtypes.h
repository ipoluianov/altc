/* AltC private header: POSIX thread types.

   Mirrors glibc's <bits/pthreadtypes.h>, <bits/thread-shared-types.h>,
   <bits/pthreadtypes-arch.h>, <bits/struct_mutex.h>,
   <bits/struct_rwlock.h> and <bits/atomic_wide_counter.h> for x86-64,
   using the same include guards and token-identical macros.  */

#ifndef _BITS_PTHREADTYPES_ARCH_H
#define _BITS_PTHREADTYPES_ARCH_H 1
#define __SIZEOF_PTHREAD_MUTEX_T 40
#define __SIZEOF_PTHREAD_ATTR_T 56
#define __SIZEOF_PTHREAD_RWLOCK_T 56
#define __SIZEOF_PTHREAD_BARRIER_T 32
#define __SIZEOF_PTHREAD_MUTEXATTR_T 4
#define __SIZEOF_PTHREAD_COND_T 48
#define __SIZEOF_PTHREAD_CONDATTR_T 4
#define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
#define __SIZEOF_PTHREAD_BARRIERATTR_T 4
#define __LOCK_ALIGNMENT
#define __ONCE_ALIGNMENT
#endif

#ifndef _BITS_ATOMIC_WIDE_COUNTER_H
#define _BITS_ATOMIC_WIDE_COUNTER_H
typedef union
{
  unsigned long long int __value64;
  struct
  {
    unsigned int __low;
    unsigned int __high;
  } __value32;
} __atomic_wide_counter;
#endif

#ifndef _THREAD_SHARED_TYPES_H
#define _THREAD_SHARED_TYPES_H 1

typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;

# ifndef _THREAD_MUTEX_INTERNAL_H
# define _THREAD_MUTEX_INTERNAL_H 1
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;
  unsigned int __nusers;
  int __kind;
  short __spins;
  short __elision;
  __pthread_list_t __list;
# define __PTHREAD_MUTEX_HAVE_PREV 1
};
# define __PTHREAD_MUTEX_INITIALIZER(__kind) \
  0, 0, 0, 0, __kind, 0, 0, { NULL, NULL }
# endif

# ifndef _RWLOCK_INTERNAL_H
# define _RWLOCK_INTERNAL_H
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;
  int __cur_writer;
  int __shared;
  signed char __rwelision;
  unsigned char __pad1[7];
# define __PTHREAD_RWLOCK_ELISION_EXTRA 0, { 0, 0, 0, 0, 0, 0, 0 }
  unsigned long int __pad2;
  unsigned int __flags;
};
# define __PTHREAD_RWLOCK_INITIALIZER(__flags) \
  0, 0, 0, 0, 0, 0, 0, 0, __PTHREAD_RWLOCK_ELISION_EXTRA, 0, __flags
# endif

struct __pthread_cond_s
{
  __atomic_wide_counter __wseq;
  __atomic_wide_counter __g1_start;
  unsigned int __g_size[2] __LOCK_ALIGNMENT;
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
  unsigned int __unused_initialized_1;
  unsigned int __unused_initialized_2;
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct
{
  int __data __ONCE_ALIGNMENT;
} __once_flag;

#define __ONCE_FLAG_INIT { 0 }

#endif /* _THREAD_SHARED_TYPES_H */

#ifndef _BITS_PTHREADTYPES_COMMON_H
#define _BITS_PTHREADTYPES_COMMON_H 1

typedef unsigned long int pthread_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  int __align;
} pthread_mutexattr_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_CONDATTR_T];
  int __align;
} pthread_condattr_t;

typedef unsigned int pthread_key_t;

typedef int __ONCE_ALIGNMENT pthread_once_t;

union pthread_attr_t
{
  char __size[__SIZEOF_PTHREAD_ATTR_T];
  long int __align;
};
#ifndef __have_pthread_attr_t
typedef union pthread_attr_t pthread_attr_t;
# define __have_pthread_attr_t 1
#endif

typedef union
{
  struct __pthread_mutex_s __data;
  char __size[__SIZEOF_PTHREAD_MUTEX_T];
  long int __align;
} pthread_mutex_t;

typedef union
{
  struct __pthread_cond_s __data;
  char __size[__SIZEOF_PTHREAD_COND_T];
  long long int __align;
} pthread_cond_t;

typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[__SIZEOF_PTHREAD_RWLOCK_T];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_RWLOCKATTR_T];
  long int __align;
} pthread_rwlockattr_t;

typedef volatile int pthread_spinlock_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_BARRIER_T];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_BARRIERATTR_T];
  int __align;
} pthread_barrierattr_t;

#endif /* _BITS_PTHREADTYPES_COMMON_H */
