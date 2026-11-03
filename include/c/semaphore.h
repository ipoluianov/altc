/* AltC <semaphore.h> - POSIX semaphores (glibc-compatible, x86-64).  */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H 1

#include <features.h>
#include <_altc/types.h>
#include <sys/types.h>
#include <_altc/time_types.h>

#define __SIZEOF_SEM_T 32

typedef union
{
  char __size[__SIZEOF_SEM_T];
  long int __align;
} sem_t;

#define SEM_FAILED ((sem_t *) 0)

__BEGIN_DECLS

extern int sem_init (sem_t *__sem, int __pshared, unsigned int __value);
extern int sem_destroy (sem_t *__sem);
extern sem_t *sem_open (const char *__name, int __oflag, ...);
extern int sem_close (sem_t *__sem);
extern int sem_unlink (const char *__name);
extern int sem_wait (sem_t *__sem);
extern int sem_timedwait (sem_t *__restrict __sem,
                          const struct timespec *__restrict __abstime);
#ifdef __USE_GNU
extern int sem_clockwait (sem_t *__restrict __sem, clockid_t __clock,
                          const struct timespec *__restrict __abstime);
#endif
extern int sem_trywait (sem_t *__sem);
extern int sem_post (sem_t *__sem);
extern int sem_getvalue (sem_t *__restrict __sem, int *__restrict __sval);

__END_DECLS

#endif /* semaphore.h */
