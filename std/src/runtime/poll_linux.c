/* The poller of a scheduler (epoll, edge-triggered) and poll descriptors */
#define _GNU_SOURCE
#include "sched.h"
#include <alt/errors.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

struct alt__pd {
	alt__lock lock;
	int fd;
	alt_sched *sched;        /* whose epoll has the descriptor, or NULL */
	unsigned sched_gen;      /* the generation of sched when registered */
	bool rready, wready;     /* an edge arrived while nobody waited */
	bool closing;
	alt__waiter *rw, *ww;    /* the waiting reader and writer */
	alt__pd *next_free;
};

int alt__poll_init(alt_sched *s) {
	s->epfd = epoll_create1(EPOLL_CLOEXEC);
	if (s->epfd < 0)
		return -errno;
	s->evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (s->evfd < 0) {
		int e = errno;
		close(s->epfd);
		return -e;
	}
	struct epoll_event ev = {.events = EPOLLIN, .data.ptr = 0};
	if (epoll_ctl(s->epfd, EPOLL_CTL_ADD, s->evfd, &ev) < 0) {
		int e = errno;
		close(s->evfd);
		close(s->epfd);
		return -e;
	}
	return 0;
}

void alt__poll_free(alt_sched *s) {
	close(s->evfd);
	close(s->epfd);
	s->evfd = s->epfd = -1;
}

void alt__poll_wakeup(alt_sched *s) {
	uint64_t one = 1;
	ssize_t r = write(s->evfd, &one, sizeof one);
	(void)r;
}

/* whether the scheduler pd is registered with still exists (pd locked) */
static bool sched_alive(const alt__pd *pd) {
	return pd->sched && !atomic_load(&pd->sched->dead) && atomic_load(&pd->sched->gen) == pd->sched_gen;
}

static void pd_event(alt__pd *pd, uint32_t ev) {
	alt__waiter *r = 0, *w = 0;
	alt__lock_lock(&pd->lock);
	if (ev & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
		if (pd->rw && alt__waiter_claim(pd->rw)) {
			r = pd->rw;
			pd->rw = 0;
		} else
			pd->rready = true;
	}
	if (ev & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
		if (pd->ww && alt__waiter_claim(pd->ww)) {
			w = pd->ww;
			pd->ww = 0;
		} else
			pd->wready = true;
	}
	alt__lock_unlock(&pd->lock);
	if (r)
		alt__waiter_complete(r, 0);
	if (w)
		alt__waiter_complete(w, 0);
}

void alt__poll(alt_sched *s, alt_time deadline) {
	int timeout;
	if (deadline == ALT_FOREVER)
		timeout = -1;
	else if (deadline == ALT_NOWAIT)
		timeout = 0;
	else {
		alt_dur left = alt_until(deadline);
		if (left <= 0)
			timeout = 0;
		else if (left > (alt_dur)INT32_MAX * ALT_MS)
			timeout = INT32_MAX;
		else
			timeout = (int)((left + ALT_MS - 1) / ALT_MS);
	}
	struct epoll_event evs[128];
	int n = epoll_wait(s->epfd, evs, 128, timeout);
	for (int i = 0; i < n; i++) {
		if (!evs[i].data.ptr)
			alt__drain_evfd(s->evfd);
		else
			pd_event(evs[i].data.ptr, evs[i].events);
	}
}

/* ---- the pool of poll descriptors ---- */

static alt__lock pool_lock = ALT__LOCK_INIT;
static alt__pd *pool_free;

alt__pd *alt__pd_open(int fd) {
	alt__lock_lock(&pool_lock);
	if (!pool_free) {
		enum { BLOCK = 128 };
		alt__pd *b = calloc(BLOCK, sizeof *b);
		if (!b) {
			alt__lock_unlock(&pool_lock);
			return 0;
		}
		for (int i = 0; i < BLOCK; i++) {
			b[i].next_free = pool_free;
			pool_free = &b[i];
		}
	}
	alt__pd *pd = pool_free;
	pool_free = pd->next_free;
	alt__lock_unlock(&pool_lock);
	alt__lock_lock(&pd->lock);
	pd->fd = fd;
	pd->sched = 0;
	pd->rready = pd->wready = false;
	pd->closing = false;
	pd->rw = pd->ww = 0;
	pd->next_free = 0;
	alt__lock_unlock(&pd->lock);
	return pd;
}

void alt__pd_close(alt__pd *pd) {
	alt__waiter *r = 0, *w = 0;
	alt__lock_lock(&pd->lock);
	pd->closing = true;
	if (pd->rw && alt__waiter_claim(pd->rw))
		r = pd->rw;
	if (pd->ww && alt__waiter_claim(pd->ww))
		w = pd->ww;
	pd->rw = pd->ww = 0;
	alt_sched *s = pd->sched;
	if (sched_alive(pd)) {
		epoll_ctl(s->epfd, EPOLL_CTL_DEL, pd->fd, 0);
		atomic_fetch_sub(&s->registered, 1);
	}
	pd->sched = 0;
	alt__lock_unlock(&pd->lock);
	if (r)
		alt__waiter_complete(r, -ALT_ECLOSED);
	if (w)
		alt__waiter_complete(w, -ALT_ECLOSED);
}

void alt__pd_free(alt__pd *pd) {
	alt__lock_lock(&pool_lock);
	pd->next_free = pool_free;
	pool_free = pd;
	alt__lock_unlock(&pool_lock);
}

static void unlock_pd(void *arg) { alt__lock_unlock(&((alt__pd *)arg)->lock); }

/* registers pd with the poller of s (pd locked) */
static int pd_register(alt__pd *pd, alt_sched *s) {
	if (sched_alive(pd))
		return 0; /* some scheduler polls it: its events wake waiters anywhere */
	struct epoll_event ev = {.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data.ptr = pd};
	if (epoll_ctl(s->epfd, EPOLL_CTL_ADD, pd->fd, &ev) < 0 && errno != EEXIST)
		return -errno;
	pd->sched = s;
	pd->sched_gen = atomic_load(&s->gen);
	atomic_fetch_add(&s->registered, 1);
	return 0;
}

int alt__pd_wait(alt__pd *pd, int mode, alt_time deadline, struct alt_ctx *ctx) {
	alt_time cdl = alt__ctx_deadline(ctx);
	if (cdl < deadline)
		deadline = cdl;
	alt__lock_lock(&pd->lock);
	if (pd->closing) {
		alt__lock_unlock(&pd->lock);
		return -ALT_ECLOSED;
	}
	bool *ready = mode == ALT_POLL_IN ? &pd->rready : &pd->wready;
	alt__waiter **slot = mode == ALT_POLL_IN ? &pd->rw : &pd->ww;
	if (*ready) {
		*ready = false;
		alt__lock_unlock(&pd->lock);
		return 0;
	}
	if (*slot) {
		alt__lock_unlock(&pd->lock);
		return -EBUSY;
	}
	if (deadline != ALT_FOREVER && alt_until(deadline) <= 0) {
		alt__lock_unlock(&pd->lock);
		return -ETIMEDOUT;
	}
	alt_sched *s = alt__sched();
	alt__waiter w;
	alt__waiter_init(&w);
	alt__ctx_watch cw;
	bool task = w.task != 0;
	if (task) {
		int rc = pd_register(pd, s);
		if (rc < 0) {
			alt__lock_unlock(&pd->lock);
			return rc;
		}
	} else
		w.evfd = alt__thread_evfd();
	*slot = &w;
	if (ctx && !alt__ctx_watch_add(ctx, &cw, &w)) {
		*slot = 0;
		alt__lock_unlock(&pd->lock);
		return alt__ctx_err(ctx);
	}
	int rc;
	if (task)
		rc = alt__wait(&w, deadline, unlock_pd, pd);
	else {
		/* a plain thread polls the descriptor itself; events, closing and
		   cancellation also complete the waiter and signal its evfd */
		int fd = pd->fd;
		alt__lock_unlock(&pd->lock);
		for (;;) {
			if (atomic_load(&w.state) != ALT__W_WAITING)
				break;
			struct pollfd p[2] = {{fd, mode == ALT_POLL_IN ? POLLIN : POLLOUT, 0}, {w.evfd, POLLIN, 0}};
			int timeout = -1;
			if (deadline != ALT_FOREVER) {
				alt_dur left = alt_until(deadline);
				if (left <= 0) {
					alt__waiter_cancel(&w, -ETIMEDOUT);
					break;
				}
				timeout = left > (alt_dur)INT32_MAX * ALT_MS ? INT32_MAX : (int)((left + ALT_MS - 1) / ALT_MS);
			}
			int n = poll(p, 2, timeout);
			if (n > 0 && p[0].revents) {
				alt__waiter_cancel(&w, 0); /* ready (unless something else won) */
				break;
			}
			if (n > 0 && p[1].revents)
				alt__drain_evfd(w.evfd);
		}
		rc = alt__wait(&w, ALT_FOREVER, 0, 0);
		alt__drain_evfd(w.evfd);
		alt__lock_lock(&pd->lock);
	}
	if (task)
		alt__lock_lock(&pd->lock);
	if (*slot == &w)
		*slot = 0;
	alt__lock_unlock(&pd->lock);
	if (ctx)
		alt__ctx_watch_remove(ctx, &cw);
	return rc;
}

void alt__pd_interrupt(alt__pd *pd, int mode, int result) {
	alt__lock_lock(&pd->lock);
	alt__waiter **slot = mode == ALT_POLL_IN ? &pd->rw : &pd->ww;
	alt__waiter *w = 0;
	if (*slot && alt__waiter_claim(*slot)) {
		w = *slot;
		*slot = 0;
	}
	alt__lock_unlock(&pd->lock);
	if (w)
		alt__waiter_complete(w, result);
}

int alt_wait_fd(int fd, int events, alt_time deadline) {
	if (events != ALT_POLL_IN && events != ALT_POLL_OUT)
		return -EINVAL;
	if (!alt_in_task()) {
		for (;;) {
			struct pollfd p = {fd, events == ALT_POLL_IN ? POLLIN : POLLOUT, 0};
			int timeout = -1;
			if (deadline != ALT_FOREVER) {
				alt_dur left = alt_until(deadline);
				if (left <= 0)
					return -ETIMEDOUT;
				timeout = left > (alt_dur)INT32_MAX * ALT_MS ? INT32_MAX : (int)((left + ALT_MS - 1) / ALT_MS);
			}
			int n = poll(&p, 1, timeout);
			if (n > 0)
				return 0;
			if (n < 0 && errno != EINTR)
				return -errno;
		}
	}
	alt__pd *pd = alt__pd_open(fd);
	if (!pd)
		return -ENOMEM;
	/* a level check first: the edge may be long gone */
	struct pollfd p = {fd, events == ALT_POLL_IN ? POLLIN : POLLOUT, 0};
	int rc = poll(&p, 1, 0) > 0 ? 0 : alt__pd_wait(pd, events, deadline, 0);
	if (rc == -EPERM)
		rc = 0; /* regular files are always ready */
	alt__pd_close(pd);
	alt__pd_free(pd);
	return rc;
}
