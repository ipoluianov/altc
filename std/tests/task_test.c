#include "check.h"
#include <alt/chan.h>
#include <alt/context.h>
#include <alt/task.h>
#include <alt/timer.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

/* ---- tasks ---- */

static int order[16], norder;
static void step(void *arg) {
	int id = (int)(intptr_t)arg;
	order[norder++] = id;
	alt_yield();
	order[norder++] = id + 10;
}

static void first(void *arg) {
	(void)arg;
	CHECK(alt_in_task());
	CHECK(alt_sched_current() != 0);
	alt_go(step, (void *)1);
	alt_go(step, (void *)2);
	order[norder++] = 0;
}

static void tasks(void) {
	CHECK(!alt_in_task());
	CHECK(alt_sched_current() == 0);
	norder = 0;
	CHECK_OK(alt_run(first, 0));
	/* FIFO: the first task, then 1 and 2, each yielding once */
	int want[] = {0, 1, 2, 11, 12};
	CHECK_INT(norder, 5);
	for (int i = 0; i < 5; i++)
		CHECK_INT(order[i], want[i]);
	CHECK(alt_sched_current() == 0);
	CHECK_ERR(alt_go(step, 0), -ALT_ENOTASK);
}

static _Atomic int spawned;
static void leaf(void *arg) {
	(void)arg;
	atomic_fetch_add(&spawned, 1);
	alt_yield();
}
static void many(void *arg) {
	(void)arg;
	for (int i = 0; i < 20000; i++)
		CHECK_OK(alt_go(leaf, 0));
}

static void many_tasks(void) {
	atomic_store(&spawned, 0);
	CHECK_OK(alt_run(many, 0));
	CHECK_INT(atomic_load(&spawned), 20000);
}

static int depth(int n) {
	volatile char buf[1024];
	buf[0] = 1;
	buf[1023] = 1;
	return n ? depth(n - 1) + buf[0] : 0;
}
static void deep(void *arg) { *(int *)arg = depth(150); /* 150 KiB of the 256 KiB stack */ }

static void stacks(void) {
	int r = 0;
	CHECK_OK(alt_run(deep, &r));
	CHECK_INT(r, 150);
}

static void errno_task(void *arg) {
	errno = (int)(intptr_t)arg;
	alt_sleep(ALT_MS);
	CHECK_INT(errno, (int)(intptr_t)arg);
}
static void errno_main(void *arg) {
	(void)arg;
	for (int i = 1; i <= 10; i++)
		alt_go(errno_task, (void *)(intptr_t)i);
}

static void errno_saved(void) { CHECK_OK(alt_run(errno_main, 0)); }

/* ---- sleeping ---- */

static void sleeper(void *arg) {
	alt_time t0 = alt_now();
	alt_sleep(20 * ALT_MS);
	*(alt_dur *)arg = alt_since(t0);
}
static void sleeps_main(void *arg) {
	alt_dur *d = arg;
	for (int i = 0; i < 3; i++)
		alt_go(sleeper, &d[i]);
}

static void sleeping(void) {
	alt_dur d[3] = {0};
	alt_time t0 = alt_now();
	CHECK_OK(alt_run(sleeps_main, d));
	alt_dur total = alt_since(t0);
	for (int i = 0; i < 3; i++)
		CHECK(d[i] >= 20 * ALT_MS && d[i] < 200 * ALT_MS);
	/* concurrently, not one after the other */
	CHECK(total < 55 * ALT_MS);
	/* in a plain thread */
	t0 = alt_now();
	alt_sleep(5 * ALT_MS);
	CHECK(alt_since(t0) >= 5 * ALT_MS);
}

/* ---- channels ---- */

typedef struct {
	alt_chan *in, *out;
	int n;
} pipe_arg;

static void producer(void *arg) {
	pipe_arg *p = arg;
	for (int i = 0; i < p->n; i++)
		CHECK_OK(alt_chan_send(p->in, &i, ALT_FOREVER));
	alt_chan_close(p->in);
}

static void squarer(void *arg) {
	pipe_arg *p = arg;
	int v;
	while (alt_chan_recv(p->in, &v, ALT_FOREVER) == 0) {
		long long sq = (long long)v * v;
		alt_chan_send(p->out, &sq, ALT_FOREVER);
	}
	alt_chan_close(p->out);
}

static void consumer(void *arg) {
	pipe_arg *p = arg;
	long long v, sum = 0;
	int count = 0;
	while (alt_chan_recv(p->out, &v, ALT_FOREVER) == 0)
		sum += v, count++;
	CHECK_INT(count, p->n);
	CHECK_INT(sum, (long long)(p->n - 1) * p->n * (2 * p->n - 1) / 6);
	CHECK_ERR(alt_chan_recv(p->out, &v, ALT_FOREVER), -ALT_ECLOSED);
	CHECK_INT(v, 0);
}

static void pipeline_main(void *arg) {
	pipe_arg *p = arg;
	alt_go(producer, p);
	alt_go(squarer, p);
	alt_go(consumer, p);
}

static void channels(void) {
	for (int cap = 0; cap <= 3; cap += 3) {
		pipe_arg p = {0, 0, 1000};
		CHECK_OK(alt_chan_new(&p.in, sizeof(int), (size_t)cap));
		CHECK_OK(alt_chan_new(&p.out, sizeof(long long), (size_t)cap));
		CHECK_OK(alt_run(pipeline_main, &p));
		alt_chan_free(p.in);
		alt_chan_free(p.out);
	}
	/* non-blocking and deadlines outside tasks */
	alt_chan *c;
	CHECK_OK(alt_chan_new(&c, sizeof(int), 2));
	int v = 7;
	CHECK_OK(alt_chan_send(c, &v, ALT_NOWAIT));
	v = 8;
	CHECK_OK(alt_chan_send(c, &v, ALT_NOWAIT));
	CHECK_ERR(alt_chan_send(c, &v, ALT_NOWAIT), -EAGAIN);
	alt_time t0 = alt_now();
	CHECK_ERR(alt_chan_send(c, &v, alt_deadline(10 * ALT_MS)), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 10 * ALT_MS);
	CHECK_INT(alt_chan_len(c), 2);
	CHECK_INT(alt_chan_cap(c), 2);
	CHECK_OK(alt_chan_recv(c, &v, ALT_NOWAIT));
	CHECK_INT(v, 7);
	CHECK_OK(alt_chan_close(c));
	CHECK_ERR(alt_chan_close(c), -ALT_ECLOSED);
	CHECK_ERR(alt_chan_send(c, &v, ALT_NOWAIT), -ALT_ECLOSED);
	CHECK_OK(alt_chan_recv(c, &v, ALT_NOWAIT)); /* buffered values survive the close */
	CHECK_INT(v, 8);
	CHECK_ERR(alt_chan_recv(c, &v, ALT_FOREVER), -ALT_ECLOSED);
	CHECK(alt_chan_closed(c));
	alt_chan_free(c);
}

/* threads and tasks on the same channel */
typedef struct {
	alt_chan *c;
	int base;
} thr_arg;

static void *thread_sender(void *arg) {
	thr_arg *a = arg;
	for (int i = 0; i < 5000; i++) {
		int v = a->base + i;
		alt_chan_send(a->c, &v, ALT_FOREVER);
	}
	return 0;
}

static void task_receiver(void *arg) {
	alt_chan *c = arg;
	long long sum = 0;
	int v;
	for (int i = 0; i < 20000; i++) {
		CHECK_OK(alt_chan_recv(c, &v, ALT_FOREVER));
		sum += v;
	}
	long long want = 0;
	for (int t = 0; t < 4; t++)
		for (int i = 0; i < 5000; i++)
			want += t * 100000 + i;
	CHECK_INT(sum, want);
}

static void threads_and_tasks(void) {
	for (int cap = 0; cap <= 16; cap += 16) {
		alt_chan *c;
		CHECK_OK(alt_chan_new(&c, sizeof(int), (size_t)cap));
		pthread_t th[4];
		thr_arg a[4];
		for (int t = 0; t < 4; t++) {
			a[t] = (thr_arg){c, t * 100000};
			pthread_create(&th[t], 0, thread_sender, &a[t]);
		}
		CHECK_OK(alt_run(task_receiver, c));
		for (int t = 0; t < 4; t++)
			pthread_join(th[t], 0);
		alt_chan_free(c);
	}
}

/* schedulers on threads of their own */
static void remote_work(void *arg) {
	alt_chan *c = arg;
	alt_sleep(ALT_MS);
	int v = 1;
	alt_chan_send(c, &v, ALT_FOREVER);
}

static void schedulers(void) {
	alt_sched *s[3];
	for (int i = 0; i < 3; i++)
		CHECK_OK(alt_sched_start(&s[i]));
	alt_chan *c;
	alt_chan_new(&c, sizeof(int), 0);
	for (int i = 0; i < 300; i++)
		CHECK_OK(alt_go_on(s[i % 3], remote_work, c, 0));
	int sum = 0, v;
	for (int i = 0; i < 300; i++) {
		CHECK_OK(alt_chan_recv(c, &v, alt_deadline(5 * ALT_SEC)));
		sum += v;
	}
	CHECK_INT(sum, 300);
	for (int i = 0; i < 3; i++)
		CHECK_OK(alt_sched_stop(s[i]));
	alt_chan_free(c);
}

/* ---- select ---- */

static void select_sender(void *arg) {
	alt_chan **cs = arg;
	for (int i = 0; i < 300; i++) {
		int v = i;
		alt_chan_send(cs[i % 3], &v, ALT_FOREVER);
	}
	for (int i = 0; i < 3; i++)
		alt_chan_close(cs[i]);
}

static void select_main(void *arg) {
	alt_chan **cs = arg;
	alt_go(select_sender, cs);
	int got[3] = {0}, v[3], open = 3;
	long long sum = 0;
	while (open) {
		alt_case cases[3] = {{ALT_RECV, cs[0], &v[0], 0}, {ALT_RECV, cs[1], &v[1], 0}, {ALT_RECV, cs[2], &v[2], 0}};
		int i = alt_select(cases, 3, alt_deadline(5 * ALT_SEC));
		CHECK(i >= 0 && i < 3);
		if (i < 0)
			break;
		if (!cases[i].ok) {
			cs[i] = 0; /* a closed channel: disable its case */
			open--;
			continue;
		}
		got[i]++;
		sum += v[i];
		CHECK_INT(v[i] % 3, i);
	}
	CHECK_INT(got[0] + got[1] + got[2], 300);
	CHECK_INT(sum, 299 * 300 / 2);
}

static void selecting(void) {
	alt_chan *cs[3], *keep[3];
	for (int i = 0; i < 3; i++) {
		alt_chan_new(&cs[i], sizeof(int), (size_t)i);
		keep[i] = cs[i];
	}
	CHECK_OK(alt_run(select_main, cs));
	for (int i = 0; i < 3; i++)
		alt_chan_free(keep[i]);

	/* ready cases, sends, timeouts, NOWAIT */
	alt_chan *a, *b;
	alt_chan_new(&a, sizeof(int), 1);
	alt_chan_new(&b, sizeof(int), 1);
	int x = 5, y = 0;
	alt_case c1[] = {{ALT_SEND, a, &x, 0}, {ALT_RECV, b, &y, 0}};
	CHECK_INT(alt_select(c1, 2, ALT_NOWAIT), 0);
	CHECK(c1[0].ok);
	alt_case c2[] = {{ALT_SEND, a, &x, 0}, {ALT_RECV, b, &y, 0}};
	CHECK_ERR(alt_select(c2, 2, ALT_NOWAIT), -EAGAIN);
	alt_time t0 = alt_now();
	CHECK_ERR(alt_select(c2, 2, alt_deadline(15 * ALT_MS)), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 15 * ALT_MS);
	alt_case c3[] = {{ALT_RECV, a, &y, 0}, {0, 0, 0, 0}};
	CHECK_INT(alt_select(c3, 2, ALT_FOREVER), 0);
	CHECK_INT(y, 5);
	/* fairness: both ready cases get chosen */
	int count[2] = {0};
	for (int i = 0; i < 1000; i++) {
		x = 1;
		alt_chan_send(a, &x, ALT_NOWAIT);
		alt_chan_send(b, &x, ALT_NOWAIT);
		alt_case c4[] = {{ALT_RECV, a, &y, 0}, {ALT_RECV, b, &y, 0}};
		int k = alt_select(c4, 2, ALT_NOWAIT);
		if (k >= 0)
			count[k]++;
		alt_chan_recv(a, &y, ALT_NOWAIT);
		alt_chan_recv(b, &y, ALT_NOWAIT);
	}
	CHECK(count[0] > 300 && count[1] > 300);
	alt_chan_free(a);
	alt_chan_free(b);
}

/* a select waiting on many channels, completed by one of several threads */
typedef struct {
	alt_chan *cs[10];
	_Atomic int won;
} race_arg;

static void *racer(void *arg) {
	race_arg *r = arg;
	int v = 1;
	for (int i = 0; i < 10 && !atomic_load(&r->won); i++)
		if (alt_chan_send(r->cs[i], &v, alt_deadline(20 * ALT_MS)) == 0)
			atomic_fetch_add(&r->won, 1);
	return 0;
}

static void select_race(void) {
	for (int round = 0; round < 50; round++) {
		race_arg r = {{0}, 0};
		for (int i = 0; i < 10; i++)
			alt_chan_new(&r.cs[i], sizeof(int), 0);
		pthread_t th[3];
		for (int t = 0; t < 3; t++)
			pthread_create(&th[t], 0, racer, &r);
		int v;
		alt_case cases[10];
		for (int i = 0; i < 10; i++)
			cases[i] = (alt_case){ALT_RECV, r.cs[i], &v, 0};
		int k = alt_select(cases, 10, alt_deadline(ALT_SEC));
		CHECK(k >= 0);
		for (int t = 0; t < 3; t++)
			pthread_join(th[t], 0);
		/* exactly one send succeeded: the one select received */
		CHECK_INT(atomic_load(&r.won), 1);
		for (int i = 0; i < 10; i++)
			alt_chan_free(r.cs[i]);
	}
}

/* ---- contexts ---- */

static void ctx_waiter(void *arg) {
	alt_ctx *c = arg;
	alt_time t0 = alt_now();
	CHECK_ERR(alt_ctx_wait(c, ALT_FOREVER), -ECANCELED);
	CHECK(alt_since(t0) >= 10 * ALT_MS);
}

static void ctx_main(void *arg) {
	alt_ctx *c = arg;
	alt_go(ctx_waiter, c);
	alt_go(ctx_waiter, c);
	alt_sleep(10 * ALT_MS);
	alt_ctx_cancel(c);
}

static int key1, key2;

static void contexts(void) {
	alt_ctx *root, *child, *grand, *val;
	CHECK_OK(alt_ctx_new(&root, 0));
	CHECK_OK(alt_ctx_with_timeout(&child, root, ALT_HOUR));
	CHECK_OK(alt_ctx_with_value(&val, child, &key1, (void *)"one"));
	CHECK_OK(alt_ctx_with_deadline(&grand, val, alt_deadline(ALT_SEC)));
	CHECK(alt_ctx_deadline(grand) <= alt_deadline(ALT_SEC));
	CHECK(alt_ctx_deadline(val) == alt_ctx_deadline(child));
	CHECK(alt_ctx_deadline(root) == ALT_FOREVER);
	CHECK(alt_ctx_value(grand, &key1) == (void *)"one");
	CHECK(alt_ctx_value(grand, &key2) == 0);
	CHECK(alt_ctx_value(child, &key1) == 0);
	CHECK_INT(alt_ctx_err(grand), 0);
	alt_ctx_cancel(root);
	CHECK_ERR(alt_ctx_err(grand), -ECANCELED);
	CHECK_ERR(alt_ctx_err(root), -ECANCELED);
	CHECK_ERR(alt_ctx_wait(grand, ALT_FOREVER), -ECANCELED);
	alt_ctx_free(grand);
	alt_ctx_free(root); /* before its children: they are detached */
	CHECK_ERR(alt_ctx_err(child), -ECANCELED);
	alt_ctx_free(val);
	alt_ctx_free(child);
	CHECK_INT(alt_ctx_err(0), 0);

	/* deadlines */
	alt_ctx *t;
	CHECK_OK(alt_ctx_with_timeout(&t, 0, 15 * ALT_MS));
	alt_time t0 = alt_now();
	CHECK_ERR(alt_ctx_wait(t, ALT_FOREVER), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 15 * ALT_MS);
	CHECK_ERR(alt_ctx_err(t), -ETIMEDOUT);
	alt_ctx_free(t);
	CHECK_OK(alt_ctx_new(&t, 0));
	CHECK_INT(alt_ctx_wait(t, alt_deadline(5 * ALT_MS)), 0);
	alt_ctx_free(t);

	/* cancellation wakes waiting tasks */
	alt_ctx *c;
	CHECK_OK(alt_ctx_new(&c, 0));
	CHECK_OK(alt_run(ctx_main, c));
	alt_ctx_free(c);

	/* done channels in select (the time counts from before the context) */
	t0 = alt_now();
	CHECK_OK(alt_ctx_with_timeout(&t, 0, 20 * ALT_MS));
	alt_chan *done = alt_ctx_done(t);
	CHECK(done != 0);
	CHECK(alt_ctx_done(t) == done);
	alt_case cs[] = {{ALT_RECV, done, 0, 0}};
	CHECK_INT(alt_select(cs, 1, alt_deadline(ALT_SEC)), 0);
	CHECK(!cs[0].ok);
	CHECK(alt_since(t0) >= 20 * ALT_MS && alt_since(t0) < 900 * ALT_MS);
	CHECK_ERR(alt_ctx_err(t), -ETIMEDOUT);
	alt_ctx_free(t);
	CHECK_OK(alt_ctx_new(&t, 0));
	alt_ctx_cancel(t);
	alt_case cs2[] = {{ALT_RECV, alt_ctx_done(t), 0, 0}};
	CHECK_INT(alt_select(cs2, 1, ALT_NOWAIT), 0);
	alt_ctx_free(t);
	CHECK(alt_ctx_done(0) == 0);
}

/* ---- timers ---- */

static _Atomic int fired;
static void on_fire(void *arg) { atomic_fetch_add(&fired, (int)(intptr_t)arg); }

static void timers(void) {
	alt_timer *t;
	alt_time when, t0 = alt_now();
	CHECK_OK(alt_timer_new(&t, 10 * ALT_MS));
	CHECK_OK(alt_chan_recv(alt_timer_chan(t), &when, alt_deadline(ALT_SEC)));
	CHECK(when - t0 >= 10 * ALT_MS);
	CHECK(!alt_timer_stop(t));
	CHECK_OK(alt_timer_reset(t, ALT_HOUR));
	CHECK(alt_timer_stop(t));
	alt_timer_free(t);

	alt_timer *tk;
	CHECK_OK(alt_ticker_new(&tk, 5 * ALT_MS));
	alt_time prev = alt_now();
	for (int i = 0; i < 5; i++) {
		CHECK_OK(alt_chan_recv(alt_timer_chan(tk), &when, alt_deadline(ALT_SEC)));
		CHECK(when >= prev);
		prev = when;
	}
	alt_timer_free(tk);

	atomic_store(&fired, 0);
	alt_timer *f1, *f2;
	CHECK_OK(alt_timer_after_func(&f1, 5 * ALT_MS, on_fire, (void *)1));
	CHECK_OK(alt_timer_after_func(&f2, ALT_HOUR, on_fire, (void *)100));
	alt_sleep(30 * ALT_MS);
	CHECK_INT(atomic_load(&fired), 1);
	CHECK(alt_timer_stop(f2));
	alt_timer_free(f1);
	alt_timer_free(f2);
	CHECK_INT(atomic_load(&fired), 1);

	/* many timers in order: deadlines fixed in advance (made in reverse
	   order), so slow creation (sanitizers) does not reorder them */
	alt_timer *ts[50];
	alt_time base = alt_now() + 100 * ALT_MS;
	for (int i = 0; i < 50; i++)
		alt_timer_new(&ts[i], base + (alt_dur)(50 - i) * ALT_MS - alt_now());
	alt_time last = 0;
	for (int i = 49; i >= 0; i--) {
		CHECK_OK(alt_chan_recv(alt_timer_chan(ts[i]), &when, alt_deadline(ALT_SEC)));
		CHECK(when >= last);
		last = when;
	}
	for (int i = 0; i < 50; i++)
		alt_timer_free(ts[i]);
}

/* ---- waiting for descriptors ---- */

static void fd_reader(void *arg) {
	int *fds = arg;
	char buf[8];
	CHECK_ERR(alt_wait_fd(fds[0], ALT_POLL_IN, alt_deadline(10 * ALT_MS)), -ETIMEDOUT);
	CHECK_OK(alt_wait_fd(fds[0], ALT_POLL_IN, alt_deadline(ALT_SEC)));
	CHECK_INT(read(fds[0], buf, sizeof buf), 2);
}

static void fd_writer(void *arg) {
	int *fds = arg;
	alt_sleep(30 * ALT_MS);
	CHECK_INT(write(fds[1], "hi", 2), 2);
}

static void fd_main(void *arg) {
	alt_go(fd_reader, arg);
	alt_go(fd_writer, arg);
}

static void descriptors(void) {
	int fds[2];
	CHECK(pipe(fds) == 0);
	CHECK_OK(alt_run(fd_main, fds));
	CHECK_ERR(alt_wait_fd(fds[0], ALT_POLL_IN, alt_deadline(5 * ALT_MS)), -ETIMEDOUT);
	CHECK_OK(alt_wait_fd(fds[1], ALT_POLL_OUT, alt_deadline(5 * ALT_MS)));
	close(fds[0]);
	close(fds[1]);
}

int main(void) {
	RUN(tasks);
	RUN(many_tasks);
	RUN(stacks);
	RUN(errno_saved);
	RUN(sleeping);
	RUN(channels);
	RUN(threads_and_tasks);
	RUN(schedulers);
	RUN(selecting);
	RUN(select_race);
	RUN(contexts);
	RUN(timers);
	RUN(descriptors);
	return check_done();
}
