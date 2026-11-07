/* Mutexes, condition variables, once, wait groups, semaphores: an
   internal lock protects the state and a FIFO queue of waiting nodes;
   the side that releases hands the resource directly to the first node it
   wins (a node that timed out has been claimed already and is skipped). */
#define _GNU_SOURCE
#include "rt.h"
#include <alt/errors.h>
#include <alt/sync.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

struct alt__sync_node {
	struct alt__sync_node *prev, *next;
	alt__waiter w;
	long n;     /* semaphores: the weight */
	bool queued;
};
typedef struct alt__sync_node node;

#define LK(p) ((alt__lock *)(p))

static void q_push(alt__sync_queue *q, node *n) {
	n->next = 0;
	n->prev = q->tail;
	if (q->tail)
		q->tail->next = n;
	else
		q->head = n;
	q->tail = n;
	n->queued = true;
}

static void q_remove(alt__sync_queue *q, node *n) {
	if (!n->queued)
		return;
	if (n->prev)
		n->prev->next = n->next;
	else
		q->head = n->next;
	if (n->next)
		n->next->prev = n->prev;
	else
		q->tail = n->prev;
	n->prev = n->next = 0;
	n->queued = false;
}

/* the first node this side wins */
static node *q_take(alt__sync_queue *q) {
	while (q->head) {
		node *n = q->head;
		q_remove(q, n);
		if (alt__waiter_claim(&n->w))
			return n;
	}
	return 0;
}

static void unlock_lk(void *arg) { alt__lock_unlock(arg); }

/* waits on node n queued in q under lk (locked); on a timeout the node
   is taken out again. Returns the waiter's result. */
static int wait_node(_Atomic int *lk, alt__sync_queue *q, node *n, alt_time deadline) {
	int rc = alt__wait(&n->w, deadline, unlock_lk, LK(lk));
	if (rc == -ETIMEDOUT) {
		alt__lock_lock(LK(lk));
		q_remove(q, n);
		alt__lock_unlock(LK(lk));
	}
	return rc;
}

static int expired(alt_time deadline) { return deadline == ALT_NOWAIT ? -EAGAIN : -ETIMEDOUT; }

/* ---- mutex ---- */

int alt_mutex_lock_until(alt_mutex *m, alt_time deadline) {
	alt__lock_lock(LK(&m->lk));
	if (!m->locked) {
		m->locked = true;
		alt__lock_unlock(LK(&m->lk));
		return 0;
	}
	if (deadline != ALT_FOREVER && alt_until(deadline) <= 0) {
		alt__lock_unlock(LK(&m->lk));
		return expired(deadline);
	}
	node n;
	alt__waiter_init(&n.w);
	q_push(&m->q, &n);
	return wait_node(&m->lk, &m->q, &n, deadline); /* 0: handed over */
}

void alt_mutex_lock(alt_mutex *m) { alt_mutex_lock_until(m, ALT_FOREVER); }

bool alt_mutex_trylock(alt_mutex *m) { return alt_mutex_lock_until(m, ALT_NOWAIT) == 0; }

void alt_mutex_unlock(alt_mutex *m) {
	alt__lock_lock(LK(&m->lk));
	if (!m->locked) {
		alt__lock_unlock(LK(&m->lk));
		fprintf(stderr, "alt_mutex_unlock: the mutex is not locked\n");
		abort();
	}
	node *n = q_take(&m->q);
	if (!n)
		m->locked = false;
	alt__lock_unlock(LK(&m->lk));
	if (n)
		alt__waiter_complete(&n->w, 0);
}

/* ---- rwmutex ---- */

void alt_rwmutex_rlock(alt_rwmutex *m) {
	alt__lock_lock(LK(&m->lk));
	if (!m->writer && m->waiting_writers == 0) {
		m->readers++;
		alt__lock_unlock(LK(&m->lk));
		return;
	}
	node n;
	alt__waiter_init(&n.w);
	q_push(&m->rq, &n);
	wait_node(&m->lk, &m->rq, &n, ALT_FOREVER); /* the waker counted us */
}

bool alt_rwmutex_tryrlock(alt_rwmutex *m) {
	alt__lock_lock(LK(&m->lk));
	bool ok = !m->writer && m->waiting_writers == 0;
	if (ok)
		m->readers++;
	alt__lock_unlock(LK(&m->lk));
	return ok;
}

/* hands the lock to the next writer or to all waiting readers (m->lk
   locked; it is released) */
static void rw_release(alt_rwmutex *m) {
	node *woken[64];
	int nw = 0;
	if (m->waiting_writers > 0 && m->readers == 0 && !m->writer) {
		node *w = q_take(&m->wq);
		if (w) {
			m->waiting_writers--;
			m->writer = true;
			alt__lock_unlock(LK(&m->lk));
			alt__waiter_complete(&w->w, 0);
			return;
		}
	}
	if (!m->writer && m->waiting_writers == 0) {
		for (;;) {
			while (nw < 64) {
				node *r = q_take(&m->rq);
				if (!r)
					break;
				m->readers++;
				woken[nw++] = r;
			}
			bool more = nw == 64;
			alt__lock_unlock(LK(&m->lk));
			for (int i = 0; i < nw; i++)
				alt__waiter_complete(&woken[i]->w, 0);
			if (!more)
				return;
			nw = 0;
			alt__lock_lock(LK(&m->lk));
			if (m->writer || m->waiting_writers) {
				alt__lock_unlock(LK(&m->lk));
				return;
			}
		}
	}
	alt__lock_unlock(LK(&m->lk));
}

void alt_rwmutex_runlock(alt_rwmutex *m) {
	alt__lock_lock(LK(&m->lk));
	if (m->readers <= 0) {
		alt__lock_unlock(LK(&m->lk));
		fprintf(stderr, "alt_rwmutex_runlock: not read-locked\n");
		abort();
	}
	m->readers--;
	rw_release(m);
}

void alt_rwmutex_lock(alt_rwmutex *m) {
	alt__lock_lock(LK(&m->lk));
	if (!m->writer && m->readers == 0) {
		m->writer = true;
		alt__lock_unlock(LK(&m->lk));
		return;
	}
	node n;
	alt__waiter_init(&n.w);
	m->waiting_writers++;
	q_push(&m->wq, &n);
	wait_node(&m->lk, &m->wq, &n, ALT_FOREVER);
}

bool alt_rwmutex_trylock(alt_rwmutex *m) {
	alt__lock_lock(LK(&m->lk));
	bool ok = !m->writer && m->readers == 0;
	if (ok)
		m->writer = true;
	alt__lock_unlock(LK(&m->lk));
	return ok;
}

void alt_rwmutex_unlock(alt_rwmutex *m) {
	alt__lock_lock(LK(&m->lk));
	if (!m->writer) {
		alt__lock_unlock(LK(&m->lk));
		fprintf(stderr, "alt_rwmutex_unlock: not write-locked\n");
		abort();
	}
	m->writer = false;
	/* readers that waited while this writer held the lock go first, then
	   the next writer (so neither side starves) */
	if (m->rq.head) {
		int saved = m->waiting_writers;
		m->waiting_writers = 0;
		node *woken[64];
		int nw = 0;
		node *r;
		while (nw < 64 && (r = q_take(&m->rq))) {
			m->readers++;
			woken[nw++] = r;
		}
		m->waiting_writers = saved;
		alt__lock_unlock(LK(&m->lk));
		for (int i = 0; i < nw; i++)
			alt__waiter_complete(&woken[i]->w, 0);
		if (nw)
			return;
		alt__lock_lock(LK(&m->lk));
	}
	rw_release(m);
}

/* ---- condition variables ---- */

int alt_cond_wait_until(alt_cond *c, alt_mutex *m, alt_time deadline) {
	node n;
	alt__waiter_init(&n.w);
	alt__lock_lock(LK(&c->lk));
	q_push(&c->q, &n);
	alt_mutex_unlock(m);
	int rc = wait_node(&c->lk, &c->q, &n, deadline);
	alt_mutex_lock(m);
	return rc == -ETIMEDOUT ? -ETIMEDOUT : 0;
}

void alt_cond_wait(alt_cond *c, alt_mutex *m) { alt_cond_wait_until(c, m, ALT_FOREVER); }

void alt_cond_signal(alt_cond *c) {
	alt__lock_lock(LK(&c->lk));
	node *n = q_take(&c->q);
	alt__lock_unlock(LK(&c->lk));
	if (n)
		alt__waiter_complete(&n->w, 0);
}

void alt_cond_broadcast(alt_cond *c) {
	for (;;) {
		node *woken[64];
		int nw = 0;
		alt__lock_lock(LK(&c->lk));
		node *n;
		while (nw < 64 && (n = q_take(&c->q)))
			woken[nw++] = n;
		alt__lock_unlock(LK(&c->lk));
		for (int i = 0; i < nw; i++)
			alt__waiter_complete(&woken[i]->w, 0);
		if (nw < 64)
			return;
	}
}

/* ---- once ---- */

void alt_once_do(alt_once *o, void (*fn)(void *), void *arg) {
	if (atomic_load_explicit(&o->state, memory_order_acquire) == 2)
		return;
	alt_mutex_lock(&o->m);
	if (atomic_load_explicit(&o->state, memory_order_relaxed) == 0) {
		atomic_store_explicit(&o->state, 1, memory_order_relaxed);
		fn(arg);
		atomic_store_explicit(&o->state, 2, memory_order_release);
	}
	alt_mutex_unlock(&o->m);
}

/* ---- wait groups ---- */

void alt_waitgroup_add(alt_waitgroup *wg, long n) {
	alt__lock_lock(LK(&wg->lk));
	wg->count += n;
	if (wg->count < 0) {
		alt__lock_unlock(LK(&wg->lk));
		fprintf(stderr, "alt_waitgroup_add: negative counter\n");
		abort();
	}
	if (wg->count > 0) {
		alt__lock_unlock(LK(&wg->lk));
		return;
	}
	for (;;) {
		node *woken[64];
		int nw = 0;
		node *x;
		while (nw < 64 && (x = q_take(&wg->q)))
			woken[nw++] = x;
		alt__lock_unlock(LK(&wg->lk));
		for (int i = 0; i < nw; i++)
			alt__waiter_complete(&woken[i]->w, 0);
		if (nw < 64)
			return;
		alt__lock_lock(LK(&wg->lk));
	}
}

int alt_waitgroup_wait_until(alt_waitgroup *wg, alt_time deadline) {
	alt__lock_lock(LK(&wg->lk));
	if (wg->count == 0) {
		alt__lock_unlock(LK(&wg->lk));
		return 0;
	}
	if (deadline != ALT_FOREVER && alt_until(deadline) <= 0) {
		alt__lock_unlock(LK(&wg->lk));
		return expired(deadline);
	}
	node n;
	alt__waiter_init(&n.w);
	q_push(&wg->q, &n);
	return wait_node(&wg->lk, &wg->q, &n, deadline);
}

void alt_waitgroup_wait(alt_waitgroup *wg) { alt_waitgroup_wait_until(wg, ALT_FOREVER); }

/* ---- semaphores ---- */

int alt_sem_acquire(alt_sem *s, long n, alt_time deadline) {
	alt__lock_lock(LK(&s->lk));
	/* FIFO: nobody overtakes the waiting ones */
	if (!s->q.head && s->avail >= n) {
		s->avail -= n;
		alt__lock_unlock(LK(&s->lk));
		return 0;
	}
	if (deadline != ALT_FOREVER && alt_until(deadline) <= 0) {
		alt__lock_unlock(LK(&s->lk));
		return expired(deadline);
	}
	node x;
	alt__waiter_init(&x.w);
	x.n = n;
	q_push(&s->q, &x);
	int rc = alt__wait(&x.w, deadline, unlock_lk, LK(&s->lk));
	if (rc == -ETIMEDOUT) {
		alt__lock_lock(LK(&s->lk));
		bool head = s->q.head == &x;
		q_remove(&s->q, &x);
		alt__lock_unlock(LK(&s->lk));
		/* the next waiters may fit now */
		if (head)
			alt_sem_release(s, 0);
	}
	return rc;
}

void alt_sem_release(alt_sem *s, long n) {
	alt__lock_lock(LK(&s->lk));
	s->avail += n;
	for (;;) {
		node *x = s->q.head;
		if (!x || x->n > s->avail)
			break;
		q_remove(&s->q, x);
		if (!alt__waiter_claim(&x->w))
			continue; /* timed out */
		s->avail -= x->n;
		alt__lock_unlock(LK(&s->lk));
		alt__waiter_complete(&x->w, 0);
		alt__lock_lock(LK(&s->lk));
	}
	alt__lock_unlock(LK(&s->lk));
}
