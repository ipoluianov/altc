/* AltC standard library: synchronization (Go's sync) and the thread pool.

   The primitives work between tasks and threads in any combination: a
   task that waits lets its scheduler run other tasks, a thread sleeps.
   All of them are initialized statically with their ALT_*_INIT or by
   zeroing, need no destruction, and must not be moved or copied while in
   use. A mutex is handed directly to the next waiter in FIFO order. */
#ifndef ALT_SYNC_H
#define ALT_SYNC_H

#include <alt/errors.h>
#include <alt/time.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct alt__sync_node;
typedef struct {
	struct alt__sync_node *head, *tail;
} alt__sync_queue;

typedef struct {
	_Atomic int lk;     /* internal */
	bool locked;
	alt__sync_queue q;
} alt_mutex;
#define ALT_MUTEX_INIT {0, false, {0, 0}}

void alt_mutex_lock(alt_mutex *m);
/* 0, or -ETIMEDOUT at the deadline (-EAGAIN with ALT_NOWAIT) */
int alt_mutex_lock_until(alt_mutex *m, alt_time deadline);
bool alt_mutex_trylock(alt_mutex *m);
void alt_mutex_unlock(alt_mutex *m);

/* readers share, a writer is alone; waiting writers block new readers */
typedef struct {
	_Atomic int lk;
	int readers;        /* active readers */
	bool writer;        /* a writer holds it */
	int waiting_writers;
	alt__sync_queue rq, wq;
} alt_rwmutex;
#define ALT_RWMUTEX_INIT {0, 0, false, 0, {0, 0}, {0, 0}}

void alt_rwmutex_rlock(alt_rwmutex *m);
void alt_rwmutex_runlock(alt_rwmutex *m);
void alt_rwmutex_lock(alt_rwmutex *m);
void alt_rwmutex_unlock(alt_rwmutex *m);
bool alt_rwmutex_tryrlock(alt_rwmutex *m);
bool alt_rwmutex_trylock(alt_rwmutex *m);

typedef struct {
	_Atomic int lk;
	alt__sync_queue q;
} alt_cond;
#define ALT_COND_INIT {0, {0, 0}}

/* releases m, waits for a signal, locks m again */
void alt_cond_wait(alt_cond *c, alt_mutex *m);
/* 0, or -ETIMEDOUT (m is locked again either way) */
int alt_cond_wait_until(alt_cond *c, alt_mutex *m, alt_time deadline);
void alt_cond_signal(alt_cond *c);
void alt_cond_broadcast(alt_cond *c);

typedef struct {
	_Atomic int state;  /* 0, 1 running, 2 done */
	alt_mutex m;
} alt_once;
#define ALT_ONCE_INIT {0, ALT_MUTEX_INIT}

/* runs fn(arg) once; other callers wait until it has returned */
void alt_once_do(alt_once *o, void (*fn)(void *), void *arg);

typedef struct {
	_Atomic int lk;
	long count;
	alt__sync_queue q;
} alt_waitgroup;
#define ALT_WAITGROUP_INIT {0, 0, {0, 0}}

/* a negative count aborts the program */
void alt_waitgroup_add(alt_waitgroup *wg, long n);
static inline void alt_waitgroup_done(alt_waitgroup *wg) { alt_waitgroup_add(wg, -1); }
void alt_waitgroup_wait(alt_waitgroup *wg);
int alt_waitgroup_wait_until(alt_waitgroup *wg, alt_time deadline);

/* a counting semaphore (golang.org/x/sync/semaphore: weighted, FIFO) */
typedef struct {
	_Atomic int lk;
	long avail;
	alt__sync_queue q;
} alt_sem;
#define ALT_SEM_INIT(n) {0, (n), {0, 0}}

static inline void alt_sem_init(alt_sem *s, long n) {
	*s = (alt_sem)ALT_SEM_INIT(n);
}
/* 0, -ETIMEDOUT, -EAGAIN (ALT_NOWAIT) */
int alt_sem_acquire(alt_sem *s, long n, alt_time deadline);
void alt_sem_release(alt_sem *s, long n);

/* ---- the thread pool ----

   Jobs run on threads of the pool, which grows up to its limit when no
   thread is idle and shrinks when threads stay idle: a pool fits both
   computation and blocking calls (the default pool, which the library
   uses for blocking calls, allows 256 threads). */

typedef struct alt_pool alt_pool;

/* max_threads 0: the number of processors */
int alt_pool_new(alt_pool **out, int max_threads);
/* waits for the queued jobs, then frees */
void alt_pool_free(alt_pool *p);
/* queues fn(arg) */
int alt_pool_go(alt_pool *p, void (*fn)(void *), void *arg);
/* runs fn(arg) on the pool and waits for it (a task keeps its scheduler
   running meanwhile): the way to make a blocking call from a task */
int alt_pool_run(alt_pool *p, void (*fn)(void *), void *arg);
/* the shared pool */
alt_pool *alt_pool_default(void);
/* the number of processors available to the program */
int alt_num_cpu(void);

#ifdef __cplusplus
}
#endif

#endif
