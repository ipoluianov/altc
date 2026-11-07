/* Contexts: the tree is changed under one global lock (creation, freeing,
   cancellation are rare); each context's list of cancel waiters has its
   own lock. Lock order: tree, then a context. */
#include "rt.h"
#include <alt/context.h>
#include <alt/chan.h>
#include <alt/errors.h>
#include <alt/timer.h>
#include <errno.h>
#include <stdlib.h>

struct alt_ctx {
	alt_ctx *parent, *children, *prev, *next;
	alt_time deadline;
	const void *key;
	void *value;
	bool has_value;
	alt__lock lock;
	_Atomic int err;
	alt__ctx_watch *watch;
	struct alt_chan *done;
	alt_timer *timer; /* cancels at the deadline once done was asked for */
};

static alt__lock tree = ALT__LOCK_INIT;

static int child(alt_ctx **out, alt_ctx *parent, alt_time deadline) {
	alt_ctx *c = calloc(1, sizeof *c);
	if (!c)
		return -ENOMEM;
	c->deadline = deadline;
	alt__lock_lock(&tree);
	if (parent) {
		if (parent->deadline < c->deadline)
			c->deadline = parent->deadline;
		c->parent = parent;
		c->next = parent->children;
		if (parent->children)
			parent->children->prev = c;
		parent->children = c;
		atomic_store(&c->err, atomic_load(&parent->err));
	}
	alt__lock_unlock(&tree);
	*out = c;
	return 0;
}

int alt_ctx_new(alt_ctx **out, alt_ctx *parent) { return child(out, parent, ALT_FOREVER); }
int alt_ctx_with_deadline(alt_ctx **out, alt_ctx *parent, alt_time deadline) { return child(out, parent, deadline); }
int alt_ctx_with_timeout(alt_ctx **out, alt_ctx *parent, alt_dur d) { return child(out, parent, alt_deadline(d)); }

int alt_ctx_with_value(alt_ctx **out, alt_ctx *parent, const void *key, void *value) {
	int rc = child(out, parent, ALT_FOREVER);
	if (rc == 0) {
		(*out)->key = key;
		(*out)->value = value;
		(*out)->has_value = true;
	}
	return rc;
}

void alt__ctx_close_done(struct alt_chan *ch); /* chan.c */

/* marks c and its descendants done (tree locked) */
static void cancel_locked(alt_ctx *c, int err) {
	alt__lock_lock(&c->lock);
	int expected = 0;
	bool first = atomic_compare_exchange_strong(&c->err, &expected, err);
	/* each watcher is unlinked before it is woken, under the lock: the
	   woken side's alt__ctx_watch_remove then finds it unlisted */
	while (c->watch) {
		alt__ctx_watch *w = c->watch;
		c->watch = w->next;
		if (c->watch)
			c->watch->prev = 0;
		w->prev = w->next = 0;
		alt__waiter_cancel(w->w, err);
	}
	struct alt_chan *done = first ? c->done : 0;
	alt__lock_unlock(&c->lock);
	if (done)
		alt__ctx_close_done(done);
	for (alt_ctx *k = c->children; k; k = k->next)
		cancel_locked(k, err);
}

void alt_ctx_cancel(alt_ctx *c) {
	if (!c)
		return;
	alt__lock_lock(&tree);
	cancel_locked(c, -ECANCELED);
	alt__lock_unlock(&tree);
}

void alt__chan_free_done(struct alt_chan *ch); /* chan.c */

void alt_ctx_free(alt_ctx *c) {
	if (!c)
		return;
	/* before the tree lock: a firing timer takes it */
	if (c->timer)
		alt_timer_free(c->timer);
	alt__lock_lock(&tree);
	cancel_locked(c, -ECANCELED);
	if (c->parent) {
		if (c->prev)
			c->prev->next = c->next;
		else
			c->parent->children = c->next;
		if (c->next)
			c->next->prev = c->prev;
	}
	for (alt_ctx *k = c->children; k;) {
		alt_ctx *n = k->next;
		k->parent = 0;
		k->prev = k->next = 0;
		k = n;
	}
	alt__lock_unlock(&tree);
	if (c->done)
		alt__chan_free_done(c->done);
	free(c);
}

int alt_ctx_err(alt_ctx *c) {
	if (!c)
		return 0;
	int e = atomic_load(&c->err);
	if (e)
		return e;
	if (c->deadline != ALT_FOREVER && alt_until(c->deadline) <= 0)
		return -ETIMEDOUT;
	return 0;
}

int alt__ctx_err(struct alt_ctx *c) { return alt_ctx_err(c); }

alt_time alt_ctx_deadline(const alt_ctx *c) { return c ? c->deadline : ALT_FOREVER; }
alt_time alt__ctx_deadline(const struct alt_ctx *c) { return alt_ctx_deadline(c); }

void *alt_ctx_value(const alt_ctx *c, const void *key) {
	alt__lock_lock(&tree);
	void *v = 0;
	for (; c; c = c->parent)
		if (c->has_value && c->key == key) {
			v = c->value;
			break;
		}
	alt__lock_unlock(&tree);
	return v;
}

bool alt__ctx_watch_add(struct alt_ctx *c, alt__ctx_watch *cw, alt__waiter *w) {
	if (!c)
		return true;
	alt__lock_lock(&c->lock);
	if (atomic_load(&c->err) || (c->deadline != ALT_FOREVER && alt_until(c->deadline) <= 0)) {
		alt__lock_unlock(&c->lock);
		return false;
	}
	cw->w = w;
	cw->prev = 0;
	cw->next = c->watch;
	if (c->watch)
		c->watch->prev = cw;
	c->watch = cw;
	alt__lock_unlock(&c->lock);
	return true;
}

void alt__ctx_watch_remove(struct alt_ctx *c, alt__ctx_watch *cw) {
	if (!c)
		return;
	alt__lock_lock(&c->lock);
	/* after a cancellation the list was taken: cw is in it no more */
	bool listed = c->watch == cw || cw->prev;
	if (listed) {
		if (cw->prev)
			cw->prev->next = cw->next;
		else
			c->watch = cw->next;
		if (cw->next)
			cw->next->prev = cw->prev;
	}
	cw->prev = cw->next = 0;
	alt__lock_unlock(&c->lock);
}

int alt_ctx_wait(alt_ctx *c, alt_time deadline) {
	if (!c)
		return alt_sleep_until(deadline) < 0 ? 0 : 0;
	int e = alt_ctx_err(c);
	if (e)
		return e;
	alt__waiter w;
	alt__waiter_init(&w);
	alt__ctx_watch cw;
	if (!alt__ctx_watch_add(c, &cw, &w))
		return alt_ctx_err(c);
	alt_time dl = alt_time_min(deadline, c->deadline);
	int rc = alt__wait(&w, dl, 0, 0);
	alt__ctx_watch_remove(c, &cw);
	if (rc == -ETIMEDOUT)
		return alt_ctx_err(c); /* the context's deadline, or 0 for ours */
	return rc;
}

static void expire(void *arg) {
	alt__lock_lock(&tree);
	cancel_locked(arg, -ETIMEDOUT);
	alt__lock_unlock(&tree);
}

struct alt_chan *alt_ctx_done(alt_ctx *c) {
	if (!c)
		return 0;
	alt__lock_lock(&c->lock);
	if (!c->done) {
		if (alt_chan_new(&c->done, 0, 0) < 0) {
			alt__lock_unlock(&c->lock);
			return 0;
		}
		if (atomic_load(&c->err))
			alt_chan_close(c->done);
		else if (c->deadline != ALT_FOREVER)
			alt_timer_after_func(&c->timer, alt_until(c->deadline), expire, c);
	}
	struct alt_chan *ch = c->done;
	alt__lock_unlock(&c->lock);
	return ch;
}
