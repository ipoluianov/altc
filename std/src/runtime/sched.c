/* Tasks and schedulers */
#define _GNU_SOURCE
#include "sched.h"
#include <alt/errors.h>
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

void alt__swap(void **save, void *load);
void alt__task_start(void);
void alt__task_main(alt_task *t) __attribute__((noreturn, used));

static __thread alt_sched *cur;

/* sanitizers (in builds with gcc or clang -fsanitize=...) learn about
   the switches between stacks */
#if defined(__SANITIZE_ADDRESS__)
void __sanitizer_start_switch_fiber(void **fake_stack_save, const void *bottom, size_t size);
void __sanitizer_finish_switch_fiber(void *fake_stack_save, const void **bottom_old, size_t *size_old);
#define SAN_START(save, bottom, size) __sanitizer_start_switch_fiber(save, bottom, size)
#define SAN_FINISH(fake, bottom, size) __sanitizer_finish_switch_fiber(fake, bottom, size)
#else
#define SAN_START(save, bottom, size) ((void)0)
#define SAN_FINISH(fake, bottom, size) ((void)0)
#endif
#if defined(__SANITIZE_THREAD__)
void *__tsan_get_current_fiber(void);
void *__tsan_create_fiber(unsigned flags);
void __tsan_destroy_fiber(void *fiber);
void __tsan_switch_to_fiber(void *fiber, unsigned flags);
#define TSAN_SWITCH(f) __tsan_switch_to_fiber(f, 0)
#else
#define TSAN_SWITCH(f) ((void)0)
#endif

alt_sched *alt__sched(void) { return cur; }
alt_sched *alt_sched_current(void) { return cur; }
bool alt_in_task(void) { return cur && cur->current; }
alt_task *alt_task_current(void) { return cur ? cur->current : 0; }
const char *alt_task_name(const alt_task *t) { return t && t->name ? t->name : ""; }
size_t alt_sched_tasks(const alt_sched *s) { return atomic_load(&((alt_sched *)s)->ntasks); }

/* ---- stacks: mappings with a guard page below, a few kept for reuse ---- */

enum { DEFAULT_STACK = 256 * 1024, GUARD = 4096, CACHED_STACKS = 64 };

static alt__lock stack_lock = ALT__LOCK_INIT;
static unsigned char *stack_cache[CACHED_STACKS];
static int nstack_cache;

static unsigned char *stack_alloc(size_t size) {
	if (size == DEFAULT_STACK) {
		alt__lock_lock(&stack_lock);
		unsigned char *p = nstack_cache ? stack_cache[--nstack_cache] : 0;
		alt__lock_unlock(&stack_lock);
		if (p)
			return p;
	}
	unsigned char *p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_STACK, -1, 0);
	if (p == MAP_FAILED)
		return 0;
	if (mprotect(p, GUARD, PROT_NONE) != 0) {
		munmap(p, size);
		return 0;
	}
	return p;
}

static void stack_free(unsigned char *p, size_t size) {
	if (size == DEFAULT_STACK) {
		alt__lock_lock(&stack_lock);
		if (nstack_cache < CACHED_STACKS) {
			stack_cache[nstack_cache++] = p;
			p = 0;
		}
		alt__lock_unlock(&stack_lock);
		if (!p)
			return;
	}
	munmap(p, size);
}

/* ---- timers: a binary heap of tasks ordered by deadline ---- */

static void heap_swap(alt_sched *s, size_t i, size_t j) {
	alt_task *a = s->heap[i], *b = s->heap[j];
	s->heap[i] = b, b->heap_index = i;
	s->heap[j] = a, a->heap_index = j;
}

static void heap_up(alt_sched *s, size_t i) {
	while (i > 0) {
		size_t p = (i - 1) / 2;
		if (s->heap[p]->when <= s->heap[i]->when)
			break;
		heap_swap(s, i, p);
		i = p;
	}
}

static void heap_down(alt_sched *s, size_t i) {
	for (;;) {
		size_t l = 2 * i + 1, r = l + 1, m = i;
		if (l < s->nheap && s->heap[l]->when < s->heap[m]->when)
			m = l;
		if (r < s->nheap && s->heap[r]->when < s->heap[m]->when)
			m = r;
		if (m == i)
			break;
		heap_swap(s, i, m);
		i = m;
	}
}

static bool heap_push(alt_sched *s, alt_task *t) {
	if (s->nheap == s->heapcap) {
		size_t cap = s->heapcap ? s->heapcap * 2 : 64;
		alt_task **h = realloc(s->heap, cap * sizeof *h);
		if (!h)
			return false;
		s->heap = h;
		s->heapcap = cap;
	}
	t->heap_index = s->nheap;
	s->heap[s->nheap++] = t;
	heap_up(s, t->heap_index);
	return true;
}

static void heap_remove(alt_sched *s, alt_task *t) {
	size_t i = t->heap_index;
	if (i == SIZE_MAX)
		return;
	t->heap_index = SIZE_MAX;
	s->nheap--;
	if (i == s->nheap)
		return;
	s->heap[i] = s->heap[s->nheap];
	s->heap[i]->heap_index = i;
	heap_down(s, i);
	heap_up(s, i);
}

static void fire_timers(alt_sched *s) {
	if (!s->nheap)
		return;
	alt_time now = alt_now();
	while (s->nheap && s->heap[0]->when <= now) {
		alt_task *t = s->heap[0];
		heap_remove(s, t);
		alt__waiter *w = t->timer_w;
		t->timer_w = 0;
		if (w)
			alt__waiter_cancel(w, -ETIMEDOUT);
	}
}

/* ---- run queues ---- */

static void rq_push(alt_sched *s, alt_task *t) {
	t->next = 0;
	if (s->rq_tail)
		s->rq_tail->next = t;
	else
		s->rq_head = t;
	s->rq_tail = t;
}

static alt_task *rq_pop(alt_sched *s) {
	alt_task *t = s->rq_head;
	if (t) {
		s->rq_head = t->next;
		if (!s->rq_head)
			s->rq_tail = 0;
	}
	return t;
}

static void enqueue(alt_task *t) {
	alt_sched *s = t->sched;
	if (s == cur) {
		rq_push(s, t);
		return;
	}
	t->next = 0;
	alt__lock_lock(&s->rlock);
	if (s->rem_tail)
		s->rem_tail->next = t;
	else
		s->rem_head = t;
	s->rem_tail = t;
	bool wake = s->sleeping;
	s->sleeping = false;
	alt__lock_unlock(&s->rlock);
	if (wake)
		alt__poll_wakeup(s);
}

/* moves the remote queue to the run queue */
static void take_remote(alt_sched *s) {
	alt__lock_lock(&s->rlock);
	alt_task *h = s->rem_head;
	s->rem_head = s->rem_tail = 0;
	alt__lock_unlock(&s->rlock);
	while (h) {
		alt_task *n = h->next;
		rq_push(s, h);
		h = n;
	}
}

void alt__ready(alt_task *t) {
	for (;;) {
		int st = atomic_load_explicit(&t->ps, memory_order_acquire);
		if (st == PS_PARKED) {
			if (atomic_compare_exchange_weak(&t->ps, &st, PS_READY)) {
				enqueue(t);
				return;
			}
		} else if (st == PS_RUN) {
			if (atomic_compare_exchange_weak(&t->ps, &st, PS_PENDING))
				return; /* the task sees it when it parks */
		} else
			return; /* a wakeup is already on its way */
	}
}

/* ---- switching ---- */

static void switch_out(alt_task *t, int why) {
	alt_sched *s = t->sched;
	t->why = why;
	SAN_START(why == T_EXIT ? 0 : &t->san_fake, s->san_bottom, s->san_size);
	TSAN_SWITCH(s->san_fiber);
	alt__swap(&t->sp, s->loop_sp);
	SAN_FINISH(t->san_fake, &s->san_bottom, &s->san_size);
}

void alt__park(void (*unlock)(void *), void *arg) {
	alt_sched *s = cur;
	s->unlock = unlock;
	s->unlock_arg = arg;
	switch_out(s->current, T_PARK);
}

void alt__task_main(alt_task *t) {
	SAN_FINISH(0, &t->sched->san_bottom, &t->sched->san_size);
	t->fn(t->arg);
	switch_out(t, T_EXIT);
	__builtin_unreachable();
}

static void task_free(alt_task *t) {
#if defined(__SANITIZE_THREAD__)
	__tsan_destroy_fiber(t->san_fiber);
#endif
	stack_free(t->stack, t->stack_size);
	free(t);
}

static void run_task(alt_sched *s, alt_task *t) {
	s->current = t;
	atomic_store_explicit(&t->ps, PS_RUN, memory_order_relaxed);
	errno = t->saved_errno;
	SAN_START(&s->san_fake, t->stack, t->stack_size);
	TSAN_SWITCH(t->san_fiber);
	alt__swap(&s->loop_sp, t->sp);
	SAN_FINISH(s->san_fake, 0, 0);
	t->saved_errno = errno;
	s->current = 0;
	switch (t->why) {
	case T_EXIT:
		task_free(t);
		atomic_fetch_sub(&s->ntasks, 1);
		return;
	case T_YIELD:
		atomic_store(&t->ps, PS_READY);
		rq_push(s, t);
		return;
	}
	if (s->unlock) {
		void (*u)(void *) = s->unlock;
		s->unlock = 0;
		u(s->unlock_arg);
	}
	int st = PS_RUN;
	if (!atomic_compare_exchange_strong(&t->ps, &st, PS_PARKED)) {
		/* woken while switching out */
		atomic_store(&t->ps, PS_READY);
		rq_push(s, t);
	}
}

static void sched_loop(alt_sched *s) {
#if defined(__SANITIZE_THREAD__)
	s->san_fiber = __tsan_get_current_fiber();
#endif
	for (;;) {
		take_remote(s);
		for (int n = 0; n < 1024 && s->rq_head; n++)
			run_task(s, rq_pop(s));
		fire_timers(s);
		if (s->rq_head)
			alt__poll(s, ALT_NOWAIT); /* I/O between batches of tasks */
		else {
			if (atomic_load(&s->ntasks) == 0 && (s->until_empty || atomic_load(&s->stop)))
				break;
			alt_time dl = s->nheap ? s->heap[0]->when : ALT_FOREVER;
			alt__lock_lock(&s->rlock);
			bool idle = !s->rem_head;
			s->sleeping = idle;
			alt__lock_unlock(&s->rlock);
			alt__poll(s, idle ? dl : ALT_NOWAIT);
			alt__lock_lock(&s->rlock);
			s->sleeping = false;
			alt__lock_unlock(&s->rlock);
		}
		fire_timers(s);
	}
}

/* ---- creating ---- */

/* scheduler structures are reused, never freed: poll descriptors may
   still name a scheduler that is gone, and know it by its generation */
static alt__lock sched_pool_lock = ALT__LOCK_INIT;
static alt_sched *sched_pool;

static alt_sched *sched_new(void) {
	alt__lock_lock(&sched_pool_lock);
	alt_sched *s = sched_pool;
	if (s)
		sched_pool = s->next_free;
	alt__lock_unlock(&sched_pool_lock);
	unsigned gen = 0;
	if (s) {
		gen = atomic_load(&s->gen) + 1;
		memset(s, 0, sizeof *s);
	} else if (!(s = calloc(1, sizeof *s)))
		return 0;
	atomic_store(&s->gen, gen);
	if (alt__poll_init(s) < 0) {
		atomic_store(&s->dead, true);
		alt__lock_lock(&sched_pool_lock);
		s->next_free = sched_pool;
		sched_pool = s;
		alt__lock_unlock(&sched_pool_lock);
		return 0;
	}
	return s;
}

static void sched_free(alt_sched *s) {
	atomic_store(&s->dead, true);
	alt__poll_free(s);
	free(s->heap);
	s->heap = 0;
	alt__lock_lock(&sched_pool_lock);
	s->next_free = sched_pool;
	sched_pool = s;
	alt__lock_unlock(&sched_pool_lock);
}

int alt_go_on(alt_sched *s, alt_task_fn fn, void *arg, const alt_task_attr *attr) {
	if (!s)
		return -ALT_ENOTASK;
	size_t size = attr && attr->stack_size ? attr->stack_size : DEFAULT_STACK;
	size = (size + 4095) & ~(size_t)4095;
	if (size < 4 * GUARD)
		size = 4 * GUARD;
	alt_task *t = calloc(1, sizeof *t);
	if (!t)
		return -ENOMEM;
	t->stack = stack_alloc(size);
	if (!t->stack) {
		free(t);
		return -ENOMEM;
	}
	t->stack_size = size;
	t->sched = s;
	t->fn = fn;
	t->arg = arg;
	t->name = attr ? attr->name : 0;
	t->heap_index = SIZE_MAX;
#if defined(__SANITIZE_THREAD__)
	t->san_fiber = __tsan_create_fiber(0);
#endif
	/* the frame alt__swap pops: MXCSR and the control word, r15..r12,
	   rbx, rbp, the return address */
	uintptr_t a = ((uintptr_t)(t->stack + size) - 16) & ~(uintptr_t)15;
	uint64_t *f = (uint64_t *)(a - 64);
	memset(f, 0, 64);
	((uint32_t *)f)[0] = 0x1F80;
	((uint16_t *)f)[2] = 0x037F;
	f[4] = (uint64_t)(uintptr_t)t; /* r12 */
	f[7] = (uint64_t)(uintptr_t)alt__task_start;
	t->sp = f;
	atomic_store(&t->ps, PS_READY);
	atomic_fetch_add(&s->ntasks, 1);
	enqueue(t);
	return 0;
}

int alt_go(alt_task_fn fn, void *arg) { return alt_go_on(cur, fn, arg, 0); }

int alt_run(alt_task_fn fn, void *arg) {
	if (cur)
		return -EBUSY;
	alt_sched *s = sched_new();
	if (!s)
		return -ENOMEM;
	s->until_empty = true;
	s->thread = pthread_self();
	cur = s;
	int rc = alt_go_on(s, fn, arg, 0);
	if (rc == 0)
		sched_loop(s);
	cur = 0;
	sched_free(s);
	return rc;
}

static void *sched_thread(void *arg) {
	alt_sched *s = arg;
	cur = s;
	sched_loop(s);
	cur = 0;
	return 0;
}

int alt_sched_start(alt_sched **out) {
	alt_sched *s = sched_new();
	if (!s)
		return -ENOMEM;
	int e = pthread_create(&s->thread, 0, sched_thread, s);
	if (e) {
		sched_free(s);
		return -e;
	}
	*out = s;
	return 0;
}

int alt_sched_stop(alt_sched *s) {
	if (s == cur)
		return -EDEADLK;
	atomic_store(&s->stop, true);
	alt__poll_wakeup(s);
	pthread_join(s->thread, 0);
	sched_free(s);
	return 0;
}

/* ---- waiting ---- */

void alt__waiter_init(alt__waiter *w) {
	atomic_store_explicit(&w->state, ALT__W_WAITING, memory_order_relaxed);
	w->result = 0;
	w->task = cur ? cur->current : 0;
	w->evfd = -1;
	w->data = 0;
	w->index = -1;
}

void alt__waiter_complete(alt__waiter *w, int result) {
	alt_task *t = w->task;
	int evfd = w->evfd;
	w->result = result;
	atomic_store_explicit(&w->state, ALT__W_DONE, memory_order_release);
	if (t)
		alt__ready(t);
	else {
		alt__futex_wake(&w->state, 1);
		if (evfd >= 0) {
			uint64_t one = 1;
			ssize_t r = write(evfd, &one, sizeof one);
			(void)r;
		}
	}
}

bool alt__waiter_cancel(alt__waiter *w, int result) {
	if (!alt__waiter_claim(w))
		return false;
	alt__waiter_complete(w, result);
	return true;
}

int alt__wait(alt__waiter *w, alt_time deadline, void (*unlock)(void *), void *arg) {
	alt_sched *s = cur;
	if (s && s->current && w->task == s->current) {
		alt_task *t = s->current;
		if (deadline != ALT_FOREVER) {
			t->when = deadline;
			t->timer_w = w;
			if (!heap_push(s, t))
				t->timer_w = 0; /* out of memory: no timeout */
		}
		alt__park(unlock, arg);
		while (atomic_load_explicit(&w->state, memory_order_acquire) != ALT__W_DONE)
			alt__park(0, 0); /* a wakeup for another reason */
		heap_remove(s, t);
		t->timer_w = 0;
		return w->result;
	}
	if (unlock)
		unlock(arg);
	for (;;) {
		int st = atomic_load_explicit(&w->state, memory_order_acquire);
		if (st == ALT__W_DONE)
			return w->result;
		if (st == ALT__W_WAITING && deadline != ALT_FOREVER && alt_until(deadline) <= 0) {
			if (alt__waiter_cancel(w, -ETIMEDOUT))
				return -ETIMEDOUT;
			continue;
		}
		alt__futex_wait(&w->state, st, st == ALT__W_WAITING ? deadline : ALT_FOREVER);
	}
}

/* ---- sleeping, yielding ---- */

int alt_sleep_until(alt_time t) {
	if (alt_in_task()) {
		alt__waiter w;
		alt__waiter_init(&w);
		int rc = alt__wait(&w, t, 0, 0);
		return rc == -ETIMEDOUT ? 0 : rc;
	}
	for (;;) {
		alt_dur left = alt_until(t);
		if (left <= 0)
			return 0;
		struct timespec ts = {left / ALT_SEC, left % ALT_SEC};
		nanosleep(&ts, 0);
	}
}

int alt_sleep(alt_dur d) { return alt_sleep_until(alt_deadline(d)); }

void alt_yield(void) {
	if (alt_in_task())
		switch_out(cur->current, T_YIELD);
	else
		sched_yield();
}
