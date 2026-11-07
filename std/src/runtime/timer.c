/* The timer thread: one heap of timers ordered by deadline, a thread that
   sleeps until the earliest and fires it */
#define _GNU_SOURCE
#include "rt.h"
#include <alt/errors.h>
#include <alt/timer.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

struct alt_timer {
	alt_time when;
	alt_dur period;          /* tickers */
	size_t index;            /* in the heap, SIZE_MAX when not */
	void (*fn)(void *);
	void *arg;
	alt_chan *ch;
	bool running;            /* fn is being called */
};

static alt__lock lock = ALT__LOCK_INIT;
static alt_timer **heap;
static size_t nheap, heapcap;
static _Atomic int wake_word;    /* futex: changes when the earliest timer changes */
static _Atomic int run_word;     /* futex: changes when a function returns */
static pthread_once_t started = PTHREAD_ONCE_INIT;
static int start_error;

static void hswap(size_t i, size_t j) {
	alt_timer *a = heap[i], *b = heap[j];
	heap[i] = b, b->index = i;
	heap[j] = a, a->index = j;
}

static void hup(size_t i) {
	while (i > 0 && heap[(i - 1) / 2]->when > heap[i]->when) {
		hswap(i, (i - 1) / 2);
		i = (i - 1) / 2;
	}
}

static void hdown(size_t i) {
	for (;;) {
		size_t l = 2 * i + 1, r = l + 1, m = i;
		if (l < nheap && heap[l]->when < heap[m]->when)
			m = l;
		if (r < nheap && heap[r]->when < heap[m]->when)
			m = r;
		if (m == i)
			return;
		hswap(i, m);
		i = m;
	}
}

static bool hremove(alt_timer *t) {
	size_t i = t->index;
	if (i == SIZE_MAX)
		return false;
	t->index = SIZE_MAX;
	nheap--;
	if (i != nheap) {
		heap[i] = heap[nheap];
		heap[i]->index = i;
		hdown(i);
		hup(i);
	}
	return true;
}

static int hpush(alt_timer *t) {
	if (nheap == heapcap) {
		size_t cap = heapcap ? heapcap * 2 : 64;
		alt_timer **h = realloc(heap, cap * sizeof *h);
		if (!h)
			return -ENOMEM;
		heap = h;
		heapcap = cap;
	}
	t->index = nheap;
	heap[nheap++] = t;
	hup(t->index);
	if (t->index == 0) {
		atomic_fetch_add(&wake_word, 1);
		alt__futex_wake(&wake_word, 1);
	}
	return 0;
}

static void *timer_thread(void *arg) {
	(void)arg;
	alt__lock_lock(&lock);
	for (;;) {
		alt_time now = alt_now();
		if (nheap && heap[0]->when <= now) {
			alt_timer *t = heap[0];
			hremove(t);
			if (t->period > 0) {
				t->when += t->period;
				if (t->when <= now)
					t->when = now + t->period; /* missed ticks are dropped */
				hpush(t);
			}
			if (t->ch) {
				/* a full channel drops the value */
				alt_chan_send(t->ch, &now, ALT_NOWAIT);
				continue;
			}
			t->running = true;
			alt__lock_unlock(&lock);
			t->fn(t->arg);
			alt__lock_lock(&lock);
			t->running = false;
			atomic_fetch_add(&run_word, 1);
			alt__futex_wake(&run_word, 0x7fffffff);
			continue;
		}
		int w = atomic_load(&wake_word);
		alt_time dl = nheap ? heap[0]->when : ALT_FOREVER;
		alt__lock_unlock(&lock);
		alt__futex_wait(&wake_word, w, dl);
		alt__lock_lock(&lock);
	}
	return 0;
}

static void start(void) {
	pthread_t th;
	pthread_attr_t a;
	pthread_attr_init(&a);
	pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
	start_error = pthread_create(&th, &a, timer_thread, 0);
	pthread_attr_destroy(&a);
}

static int timer_new(alt_timer **out, alt_dur d, alt_dur period, void (*fn)(void *), void *arg, bool with_chan) {
	pthread_once(&started, start);
	if (start_error)
		return -start_error;
	alt_timer *t = calloc(1, sizeof *t);
	if (!t)
		return -ENOMEM;
	t->index = SIZE_MAX;
	t->fn = fn;
	t->arg = arg;
	t->period = period;
	if (with_chan) {
		int rc = alt_chan_new(&t->ch, sizeof(alt_time), 1);
		if (rc) {
			free(t);
			return rc;
		}
	}
	t->when = alt_deadline(d);
	alt__lock_lock(&lock);
	int rc = hpush(t);
	alt__lock_unlock(&lock);
	if (rc) {
		alt_chan_free(t->ch);
		free(t);
		return rc;
	}
	*out = t;
	return 0;
}

int alt_timer_new(alt_timer **out, alt_dur d) { return timer_new(out, d, 0, 0, 0, true); }

int alt_ticker_new(alt_timer **out, alt_dur period) {
	if (period <= 0)
		return -EINVAL;
	return timer_new(out, period, period, 0, 0, true);
}

int alt_timer_after_func(alt_timer **out, alt_dur d, void (*fn)(void *), void *arg) { return timer_new(out, d, 0, fn, arg, false); }

alt_chan *alt_timer_chan(alt_timer *t) { return t->ch; }

bool alt_timer_stop(alt_timer *t) {
	alt__lock_lock(&lock);
	bool pending = hremove(t);
	while (t->running) {
		int w = atomic_load(&run_word);
		alt__lock_unlock(&lock);
		alt__futex_wait(&run_word, w, ALT_FOREVER);
		alt__lock_lock(&lock);
	}
	alt__lock_unlock(&lock);
	return pending;
}

int alt_timer_reset(alt_timer *t, alt_dur d) {
	alt_timer_stop(t);
	if (t->ch) {
		/* a value of the old timer is not kept (Go 1.23 semantics) */
		alt_time old;
		alt_chan_recv(t->ch, &old, ALT_NOWAIT);
	}
	alt__lock_lock(&lock);
	if (t->period > 0)
		t->period = d;
	t->when = alt_deadline(d);
	int rc = hpush(t);
	alt__lock_unlock(&lock);
	return rc;
}

void alt_timer_free(alt_timer *t) {
	if (!t)
		return;
	alt_timer_stop(t);
	alt_chan_free(t->ch);
	free(t);
}
