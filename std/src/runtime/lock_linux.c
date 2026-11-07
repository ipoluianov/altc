/* Futexes and the short-section lock (Drepper, "Futexes Are Tricky",
   mutex 3), the thread's event descriptor */
#include "rt.h"
#include <errno.h>
#include <sched.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

/* <linux/futex.h> (the C library has no kernel headers) */
#define FUTEX_WAIT_PRIVATE 128
#define FUTEX_WAKE_PRIVATE 129

int alt__futex_wait(_Atomic int *addr, int val, alt_time deadline) {
	struct timespec ts, *tp = 0;
	if (deadline != ALT_FOREVER) {
		alt_dur left = alt_until(deadline);
		if (left <= 0)
			return -ETIMEDOUT;
		ts.tv_sec = left / ALT_SEC;
		ts.tv_nsec = left % ALT_SEC;
		tp = &ts;
	}
	long r = syscall(SYS_futex, (int *)addr, FUTEX_WAIT_PRIVATE, val, tp, 0, 0);
	return r < 0 ? -errno : 0;
}

void alt__futex_wake(_Atomic int *addr, int n) { syscall(SYS_futex, (int *)addr, FUTEX_WAKE_PRIVATE, n, 0, 0, 0); }

void alt__lock_slow(alt__lock *l) {
	/* a short spin: the sections are short */
	for (int i = 0; i < 100; i++) {
		int c = 0;
		if (atomic_compare_exchange_weak_explicit(&l->v, &c, 1, memory_order_acquire, memory_order_relaxed))
			return;
		__asm__ volatile("pause");
	}
	int c = atomic_exchange_explicit(&l->v, 2, memory_order_acquire);
	while (c != 0) {
		alt__futex_wait(&l->v, 2, ALT_FOREVER);
		c = atomic_exchange_explicit(&l->v, 2, memory_order_acquire);
	}
}

void alt__unlock_slow(alt__lock *l) { alt__futex_wake(&l->v, 1); }

static __thread int thread_evfd = -1;

int alt__thread_evfd(void) {
	if (thread_evfd < 0)
		thread_evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	return thread_evfd;
}

void alt__drain_evfd(int fd) {
	uint64_t v;
	while (read(fd, &v, sizeof v) > 0) {
	}
}
