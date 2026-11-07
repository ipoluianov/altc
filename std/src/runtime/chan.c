/* Channels and select, after Go's runtime/chan.go and select.go (BSD-3-Clause,
   see LICENSE-Go): a waiting sender or receiver is a queued sudog that
   points to its waiter; whoever dequeues a sudog must win its waiter
   (alt__waiter_claim) to use it, so a select waiting on several channels
   is completed exactly once. Select locks all its channels in address
   order, and the locks are released only once the selecting side has
   stopped running. */
#include "rt.h"
#include <alt/chan.h>
#include <alt/errors.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct sudog {
	struct sudog *prev, *next;
	alt__waiter *w;
	void *elem;
	int index;  /* the select case */
	bool queued;
	bool ok;    /* set by the completing side */
} sudog;

typedef struct {
	sudog *head, *tail;
} waitq;

struct alt_chan {
	alt__lock lock;
	size_t esize, cap, count, head;
	unsigned char *buf;
	bool closed;
	waitq sendq, recvq;
};

static void wq_push(waitq *q, sudog *s) {
	s->next = 0;
	s->prev = q->tail;
	if (q->tail)
		q->tail->next = s;
	else
		q->head = s;
	q->tail = s;
	s->queued = true;
}

static void wq_remove(waitq *q, sudog *s) {
	if (!s->queued)
		return;
	if (s->prev)
		s->prev->next = s->next;
	else
		q->head = s->next;
	if (s->next)
		s->next->prev = s->prev;
	else
		q->tail = s->prev;
	s->prev = s->next = 0;
	s->queued = false;
}

/* the first sudog whose waiter this side wins; stale ones are dropped */
static sudog *wq_dequeue(waitq *q) {
	while (q->head) {
		sudog *s = q->head;
		wq_remove(q, s);
		if (alt__waiter_claim(s->w))
			return s;
	}
	return 0;
}

int alt_chan_new(alt_chan **out, size_t elem_size, size_t cap) {
	alt_chan *c = calloc(1, sizeof *c);
	if (!c)
		return -ENOMEM;
	c->esize = elem_size;
	c->cap = cap;
	if (cap && elem_size) {
		if (elem_size > SIZE_MAX / cap) {
			free(c);
			return -ENOMEM;
		}
		c->buf = malloc(elem_size * cap);
		if (!c->buf) {
			free(c);
			return -ENOMEM;
		}
	}
	*out = c;
	return 0;
}

void alt_chan_free(alt_chan *c) {
	if (!c)
		return;
	free(c->buf);
	free(c);
}

static void copy(alt_chan *c, void *dst, const void *src) {
	if (c->esize && dst && src)
		memcpy(dst, src, c->esize);
}

static void zero(alt_chan *c, void *dst) {
	if (c->esize && dst)
		memset(dst, 0, c->esize);
}

static unsigned char *slot(alt_chan *c, size_t i) { return c->buf + ((c->head + i) % c->cap) * c->esize; }

/* the immediate send (channel locked): 0, -ALT_ECLOSED, -EAGAIN; a woken
   receiver is returned in *wake to be completed after unlocking */
static int try_send(alt_chan *c, const void *elem, sudog **wake) {
	*wake = 0;
	if (c->closed)
		return -ALT_ECLOSED;
	sudog *r = wq_dequeue(&c->recvq);
	if (r) {
		copy(c, r->elem, elem);
		r->ok = true;
		r->w->index = r->index;
		*wake = r;
		return 0;
	}
	if (c->count < c->cap) {
		if (c->esize)
			memcpy(slot(c, c->count), elem, c->esize);
		c->count++;
		return 0;
	}
	return -EAGAIN;
}

static int try_recv(alt_chan *c, void *elem, bool *ok, sudog **wake) {
	*wake = 0;
	sudog *s = wq_dequeue(&c->sendq);
	if (s) {
		if (c->cap) {
			/* the buffer is full: take its head, the sender's element
			   goes to the tail */
			if (c->esize) {
				copy(c, elem, slot(c, 0));
				memcpy(slot(c, 0), s->elem, c->esize);
			}
			c->head = (c->head + 1) % c->cap;
		} else
			copy(c, elem, s->elem);
		s->ok = true;
		s->w->index = s->index;
		*wake = s;
		*ok = true;
		return 0;
	}
	if (c->count) {
		copy(c, elem, slot(c, 0));
		c->head = (c->head + 1) % c->cap;
		c->count--;
		*ok = true;
		return 0;
	}
	if (c->closed) {
		zero(c, elem);
		*ok = false;
		return -ALT_ECLOSED;
	}
	return -EAGAIN;
}

static void unlock_chan(void *arg) { alt__lock_unlock(&((alt_chan *)arg)->lock); }

static int no_wait(alt_time deadline) { return deadline == ALT_NOWAIT ? -EAGAIN : -ETIMEDOUT; }

int alt_chan_send(alt_chan *c, const void *elem, alt_time deadline) {
	sudog *wake;
	alt__lock_lock(&c->lock);
	int rc = try_send(c, elem, &wake);
	if (rc != -EAGAIN || (deadline != ALT_FOREVER && alt_until(deadline) <= 0)) {
		alt__lock_unlock(&c->lock);
		if (wake)
			alt__waiter_complete(wake->w, 0);
		return rc == -EAGAIN ? no_wait(deadline) : rc;
	}
	alt__waiter w;
	alt__waiter_init(&w);
	sudog me = {0};
	me.w = &w;
	me.elem = (void *)elem;
	me.index = 0;
	wq_push(&c->sendq, &me);
	rc = alt__wait(&w, deadline, unlock_chan, c);
	if (rc == -ETIMEDOUT) {
		alt__lock_lock(&c->lock);
		wq_remove(&c->sendq, &me);
		alt__lock_unlock(&c->lock);
	}
	return rc;
}

int alt_chan_recv(alt_chan *c, void *elem, alt_time deadline) {
	sudog *wake;
	bool ok;
	alt__lock_lock(&c->lock);
	int rc = try_recv(c, elem, &ok, &wake);
	if (rc != -EAGAIN || (deadline != ALT_FOREVER && alt_until(deadline) <= 0)) {
		alt__lock_unlock(&c->lock);
		if (wake)
			alt__waiter_complete(wake->w, 0);
		return rc == -EAGAIN ? no_wait(deadline) : rc;
	}
	alt__waiter w;
	alt__waiter_init(&w);
	sudog me = {0};
	me.w = &w;
	me.elem = elem;
	wq_push(&c->recvq, &me);
	rc = alt__wait(&w, deadline, unlock_chan, c);
	if (rc == -ETIMEDOUT) {
		alt__lock_lock(&c->lock);
		wq_remove(&c->recvq, &me);
		alt__lock_unlock(&c->lock);
	}
	return rc;
}

int alt_chan_close(alt_chan *c) {
	alt__lock_lock(&c->lock);
	if (c->closed) {
		alt__lock_unlock(&c->lock);
		return -ALT_ECLOSED;
	}
	c->closed = true;
	/* the waiters won here are completed after unlocking: chained
	   through prev (their queue links are free now) */
	sudog *woken = 0, *s;
	while ((s = wq_dequeue(&c->recvq))) {
		zero(c, s->elem);
		s->ok = false;
		s->w->index = s->index;
		s->prev = woken;
		woken = s;
	}
	while ((s = wq_dequeue(&c->sendq))) {
		s->ok = false;
		s->w->index = s->index;
		s->prev = woken;
		woken = s;
	}
	alt__lock_unlock(&c->lock);
	while (woken) {
		sudog *n = woken->prev;
		alt__waiter_complete(woken->w, -ALT_ECLOSED);
		woken = n;
	}
	return 0;
}

size_t alt_chan_len(alt_chan *c) {
	alt__lock_lock(&c->lock);
	size_t n = c->count;
	alt__lock_unlock(&c->lock);
	return n;
}

size_t alt_chan_cap(const alt_chan *c) { return c->cap; }

bool alt_chan_closed(alt_chan *c) {
	alt__lock_lock(&c->lock);
	bool b = c->closed;
	alt__lock_unlock(&c->lock);
	return b;
}

/* ---- select ---- */

typedef struct {
	alt_chan **locks; /* distinct channels in address order */
	int n;
} lockset;

static void lock_all(lockset *ls) {
	for (int i = 0; i < ls->n; i++)
		alt__lock_lock(&ls->locks[i]->lock);
}

static void unlock_all(void *arg) {
	lockset *ls = arg;
	for (int i = ls->n; i-- > 0;)
		alt__lock_unlock(&ls->locks[i]->lock);
}

static int cmp_ptr(const void *a, const void *b) {
	uintptr_t x = (uintptr_t) * (alt_chan *const *)a, y = (uintptr_t) * (alt_chan *const *)b;
	return x < y ? -1 : x > y;
}

static unsigned long long rng_state = 0x9E3779B97F4A7C15ULL;

static unsigned rnd(void) {
	/* a shared xorshift is fine for picking an order */
	unsigned long long x = __atomic_load_n(&rng_state, __ATOMIC_RELAXED);
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	__atomic_store_n(&rng_state, x, __ATOMIC_RELAXED);
	return (unsigned)(x >> 32);
}

int alt_select(alt_case *cases, int n, alt_time deadline) {
	enum { SMALL = 8 };
	int order_small[SMALL];
	alt_chan *locks_small[SMALL];
	sudog sg_small[SMALL];
	int *order = order_small;
	alt_chan **locks = locks_small;
	sudog *sg = sg_small;
	if (n < 0)
		return -EINVAL;
	if (n > SMALL) {
		order = malloc((size_t)n * sizeof *order);
		locks = malloc((size_t)n * sizeof *locks);
		sg = calloc((size_t)n, sizeof *sg);
		if (!order || !locks || !sg) {
			free(order);
			free(locks);
			free(sg);
			return -ENOMEM;
		}
	}
	/* the active cases in random order, their channels sorted */
	int m = 0, nl = 0;
	for (int i = 0; i < n; i++) {
		cases[i].ok = false;
		if (!cases[i].ch || (cases[i].op != ALT_SEND && cases[i].op != ALT_RECV))
			continue;
		int j = (int)(rnd() % (unsigned)(m + 1));
		order[m] = order[j];
		order[j] = i;
		m++;
		locks[nl++] = cases[i].ch;
	}
	qsort(locks, (size_t)nl, sizeof *locks, cmp_ptr);
	int k = 0;
	for (int i = 0; i < nl; i++)
		if (k == 0 || locks[k - 1] != locks[i])
			locks[k++] = locks[i];
	lockset ls = {locks, k};
	int rc;
	lock_all(&ls);
	/* pass 1: a case that can proceed now */
	for (int x = 0; x < m; x++) {
		int i = order[x];
		alt_case *cs = &cases[i];
		sudog *wake;
		int r;
		if (cs->op == ALT_SEND)
			r = try_send(cs->ch, cs->elem, &wake);
		else
			r = try_recv(cs->ch, cs->elem, &cs->ok, &wake);
		if (r == -EAGAIN)
			continue;
		if (cs->op == ALT_SEND)
			cs->ok = r == 0;
		unlock_all(&ls);
		if (wake)
			alt__waiter_complete(wake->w, 0);
		rc = i;
		goto out;
	}
	if (m == 0 && deadline == ALT_FOREVER) {
		/* nothing can ever happen: block forever, as Go's empty select */
		unlock_all(&ls);
		for (;;)
			alt_sleep(ALT_HOUR);
	}
	if (deadline != ALT_FOREVER && alt_until(deadline) <= 0) {
		unlock_all(&ls);
		rc = no_wait(deadline);
		goto out;
	}
	/* pass 2: wait on all of them */
	alt__waiter w;
	alt__waiter_init(&w);
	for (int x = 0; x < m; x++) {
		int i = order[x];
		sg[i] = (sudog){0};
		sg[i].w = &w;
		sg[i].elem = cases[i].elem;
		sg[i].index = i;
		wq_push(cases[i].op == ALT_SEND ? &cases[i].ch->sendq : &cases[i].ch->recvq, &sg[i]);
	}
	int wr = alt__wait(&w, deadline, unlock_all, &ls);
	/* pass 3: dequeue from the other channels */
	lock_all(&ls);
	for (int x = 0; x < m; x++) {
		int i = order[x];
		wq_remove(cases[i].op == ALT_SEND ? &cases[i].ch->sendq : &cases[i].ch->recvq, &sg[i]);
	}
	unlock_all(&ls);
	if (wr == -ETIMEDOUT && w.index < 0)
		rc = -ETIMEDOUT;
	else {
		rc = w.index;
		cases[rc].ok = sg[rc].ok;
	}
out:
	if (n > SMALL) {
		free(order);
		free(locks);
		free(sg);
	}
	return rc;
}

/* ---- the done channels of contexts ---- */

void alt__ctx_close_done(alt_chan *c) { alt_chan_close(c); }
void alt__chan_free_done(alt_chan *c) { alt_chan_free(c); }
