/* The runtime's internal interface: locks, waiters, parking, timers and
   poll descriptors, shared by the packages that block (chan, sync,
   context, net). */
#ifndef ALT_RT_H
#define ALT_RT_H

#include <alt/task.h>
#include <alt/time.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ---- locks: short critical sections only (never held across a wait);
   a waiting thread sleeps on a futex ---- */

typedef struct {
	_Atomic int v; /* 0 free, 1 locked, 2 locked with waiters */
} alt__lock;

#define ALT__LOCK_INIT {0}
void alt__lock_slow(alt__lock *l);
void alt__unlock_slow(alt__lock *l);

static inline void alt__lock_lock(alt__lock *l) {
	int c = 0;
	if (!atomic_compare_exchange_strong_explicit(&l->v, &c, 1, memory_order_acquire, memory_order_relaxed))
		alt__lock_slow(l);
}

static inline void alt__lock_unlock(alt__lock *l) {
	if (atomic_exchange_explicit(&l->v, 0, memory_order_release) == 2)
		alt__unlock_slow(l);
}

/* futexes */
int alt__futex_wait(_Atomic int *addr, int val, alt_time deadline);
void alt__futex_wake(_Atomic int *addr, int n);

/* ---- waiters: how one task or thread waits for one event ----

   A waiter starts WAITING. Exactly one party wins it with
   alt__waiter_claim (an event, a timeout, a cancellation), does its work
   (copying data into the waiter's buffers) and then calls
   alt__waiter_complete with the result, which wakes the waiting side.

   A waiter is published (put in some queue) only under a lock that the
   waiting side releases through the unlock function of alt__wait, after
   it has stopped running: whoever finds the waiter under that lock can
   wake it safely. */

enum { ALT__W_WAITING, ALT__W_CLAIMED, ALT__W_DONE };

typedef struct alt__waiter {
	_Atomic int state;
	int result;
	alt_task *task; /* NULL: a plain thread */
	int evfd;       /* a thread's event descriptor to signal, or -1 */
	void *data;     /* for the owner of the queue */
	int index;      /* for select: the case that completed */
} alt__waiter;

void alt__waiter_init(alt__waiter *w);
static inline bool alt__waiter_claim(alt__waiter *w) {
	int c = ALT__W_WAITING;
	return atomic_compare_exchange_strong_explicit(&w->state, &c, ALT__W_CLAIMED, memory_order_acq_rel, memory_order_acquire);
}
void alt__waiter_complete(alt__waiter *w, int result);
/* claim and complete at once; false when already claimed */
bool alt__waiter_cancel(alt__waiter *w, int result);

/* waits until the waiter is completed and returns its result; unlock(arg)
   is called once the caller is no longer running (a task) or right away
   (a thread). At the deadline the waiter is cancelled with -ETIMEDOUT
   (unless something claimed it first, which then completes it). */
int alt__wait(alt__waiter *w, alt_time deadline, void (*unlock)(void *), void *arg);

/* ---- tasks and schedulers ---- */

alt_sched *alt__sched(void);         /* of this thread, NULL in plain threads */
void alt__ready(alt_task *t);        /* makes a parked task runnable (any thread) */
/* parks the current task; unlock(arg) runs after the switch */
void alt__park(void (*unlock)(void *), void *arg);

/* the thread's event descriptor (for waking a plain thread in poll) */
int alt__thread_evfd(void);
void alt__drain_evfd(int fd);

/* ---- poll descriptors: a file descriptor registered with the poller of
   a scheduler (edge-triggered); tasks of any scheduler may wait on it.
   Descriptors come from a pool and are never freed: a stale event for a
   reused descriptor only wakes a waiter spuriously. ---- */

typedef struct alt__pd alt__pd;

alt__pd *alt__pd_open(int fd);
/* wakes waiters with -ALT_ECLOSED and unregisters; later waits get
   -ALT_ECLOSED. The caller closes fd. */
void alt__pd_close(alt__pd *pd);
/* returns a closed descriptor to the pool, once nothing uses it */
void alt__pd_free(alt__pd *pd);
/* waits for readiness (ALT_POLL_IN or ALT_POLL_OUT): 0, -ETIMEDOUT,
   -ECANCELED (by the cancel waiter list of ctx), -ALT_ECLOSED, -EBUSY
   when another waiter of the same direction is waiting */
struct alt_ctx;
int alt__pd_wait(alt__pd *pd, int mode, alt_time deadline, struct alt_ctx *ctx);
/* wakes a waiter of the direction with -ECANCELED (deadline changes) */
void alt__pd_interrupt(alt__pd *pd, int mode, int result);

/* ---- context cancellation hooks (context.c) ---- */
typedef struct alt__ctx_watch {
	struct alt__ctx_watch *prev, *next;
	alt__waiter *w;
} alt__ctx_watch;
/* registers w to be cancelled (-ECANCELED, or the context's error) when
   ctx is; false (and nothing registered) when ctx is already done */
bool alt__ctx_watch_add(struct alt_ctx *ctx, alt__ctx_watch *cw, alt__waiter *w);
void alt__ctx_watch_remove(struct alt_ctx *ctx, alt__ctx_watch *cw);
/* the deadline of ctx (ALT_FOREVER for none or NULL) */
alt_time alt__ctx_deadline(const struct alt_ctx *ctx);
/* 0, or the error of a done context */
int alt__ctx_err(struct alt_ctx *ctx);

#endif
