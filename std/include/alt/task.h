/* AltC standard library: tasks and schedulers.

   A task is a lightweight coroutine with a stack of its own. Tasks run on
   schedulers, and a scheduler runs in one OS thread: a task always stays
   in the thread that runs it (its __thread variables, errno and the
   pthread mutexes it holds behave as in any thread). Any thread can run a
   scheduler: alt_run turns the calling thread into one, alt_sched_start
   creates a thread for a new one.

   Blocking operations of the library (channels, sync, sleep, network)
   work in tasks and in plain threads: a task that waits lets the other
   tasks of its thread run, a thread that waits sleeps. Plain blocking
   calls of the C library (read on a blocking descriptor, getaddrinfo)
   block the whole scheduler thread; run them on the thread pool
   (alt_pool_run) instead. */
#ifndef ALT_TASK_H
#define ALT_TASK_H

#include <alt/errors.h>
#include <alt/time.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_sched alt_sched;
typedef struct alt_task alt_task;
typedef void (*alt_task_fn)(void *arg);

typedef struct {
	size_t stack_size; /* 0: the default, 256 KiB (memory is used as touched) */
	const char *name;  /* for debugging, may be NULL */
} alt_task_attr;

/* turns this thread into a scheduler: runs fn(arg) as its first task and
   returns when no task is left on the scheduler; -EBUSY when the thread
   already runs one */
int alt_run(alt_task_fn fn, void *arg);

/* starts a task on the scheduler of the calling thread; -ALT_ENOTASK
   outside schedulers */
int alt_go(alt_task_fn fn, void *arg);
/* starts a task on scheduler s (from any thread); attr may be NULL */
int alt_go_on(alt_sched *s, alt_task_fn fn, void *arg, const alt_task_attr *attr);

/* the scheduler of the calling thread, NULL in plain threads */
alt_sched *alt_sched_current(void);
/* whether the caller is a task */
bool alt_in_task(void);
/* the running task, NULL outside tasks */
alt_task *alt_task_current(void);
const char *alt_task_name(const alt_task *t);

/* lets the other ready tasks run (sched_yield in a plain thread) */
void alt_yield(void);
/* 0, or -ECANCELED... never fails in practice */
int alt_sleep(alt_dur d);
int alt_sleep_until(alt_time t);

/* a scheduler on a thread of its own, waiting for tasks (alt_go_on) */
int alt_sched_start(alt_sched **out);
/* asks the thread to finish once it has no tasks, waits for it and
   frees the scheduler */
int alt_sched_stop(alt_sched *s);

/* the number of tasks on a scheduler (running, ready or waiting) */
size_t alt_sched_tasks(const alt_sched *s);

/* waits until fd is ready for reading (ALT_POLL_IN) or writing
   (ALT_POLL_OUT): 0, -ETIMEDOUT at the deadline; descriptors used with
   this must be non-blocking */
enum { ALT_POLL_IN = 1, ALT_POLL_OUT = 2 };
int alt_wait_fd(int fd, int events, alt_time deadline);

#ifdef __cplusplus
}
#endif

#endif
