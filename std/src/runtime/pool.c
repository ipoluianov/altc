/* The thread pool: a queue of jobs; a job arriving with no idle thread
   starts one (up to the limit); threads idle for 30 seconds exit */
#define _GNU_SOURCE
#include "rt.h"
#include <alt/errors.h>
#include <alt/sync.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct job {
	struct job *next;
	void (*fn)(void *);
	void *arg;
	alt__waiter *done; /* alt_pool_run waits for it */
} job;

struct alt_pool {
	alt_mutex m;
	alt_cond work, idle_all;
	job *head, *tail;
	int threads, idle, max;
	int running;       /* jobs being run */
	bool closing;
};

enum { IDLE_EXIT = 30 };

int alt_num_cpu(void) {
	cpu_set_t set;
	if (sched_getaffinity(0, sizeof set, &set) == 0) {
		int n = CPU_COUNT(&set);
		if (n > 0)
			return n;
	}
	long n = sysconf(_SC_NPROCESSORS_ONLN);
	return n > 0 ? (int)n : 1;
}

int alt_pool_new(alt_pool **out, int max_threads) {
	alt_pool *p = calloc(1, sizeof *p);
	if (!p)
		return -ENOMEM;
	p->max = max_threads > 0 ? max_threads : alt_num_cpu();
	*out = p;
	return 0;
}

static void *worker(void *arg) {
	alt_pool *p = arg;
	alt_mutex_lock(&p->m);
	for (;;) {
		while (!p->head && !p->closing) {
			p->idle++;
			int rc = alt_cond_wait_until(&p->work, &p->m, alt_deadline(IDLE_EXIT * ALT_SEC));
			p->idle--;
			if (rc == -ETIMEDOUT && !p->head)
				goto out;
		}
		if (!p->head)
			break; /* closing and nothing left */
		job *j = p->head;
		p->head = j->next;
		if (!p->head)
			p->tail = 0;
		p->running++;
		alt_mutex_unlock(&p->m);
		j->fn(j->arg);
		if (j->done)
			alt__waiter_complete(j->done, 0); /* the job lives on the waiting stack */
		else
			free(j);
		alt_mutex_lock(&p->m);
		p->running--;
	}
out:
	p->threads--;
	if (p->threads == 0)
		alt_cond_broadcast(&p->idle_all);
	alt_mutex_unlock(&p->m);
	return 0;
}

static int submit(alt_pool *p, job *j) {
	alt_mutex_lock(&p->m);
	if (p->closing) {
		alt_mutex_unlock(&p->m);
		return -ALT_ECLOSED;
	}
	j->next = 0;
	if (p->tail)
		p->tail->next = j;
	else
		p->head = j;
	p->tail = j;
	int rc = 0;
	if (p->idle == 0 && p->threads < p->max) {
		pthread_t th;
		pthread_attr_t a;
		pthread_attr_init(&a);
		pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
		int e = pthread_create(&th, &a, worker, p);
		pthread_attr_destroy(&a);
		if (e == 0)
			p->threads++;
		else if (p->threads == 0) {
			/* no thread to run it at all */
			p->head = j->next;
			if (!p->head)
				p->tail = 0;
			rc = -e;
		}
	} else
		alt_cond_signal(&p->work);
	alt_mutex_unlock(&p->m);
	return rc;
}

int alt_pool_go(alt_pool *p, void (*fn)(void *), void *arg) {
	job *j = malloc(sizeof *j);
	if (!j)
		return -ENOMEM;
	j->fn = fn;
	j->arg = arg;
	j->done = 0;
	int rc = submit(p, j);
	if (rc)
		free(j);
	return rc;
}

int alt_pool_run(alt_pool *p, void (*fn)(void *), void *arg) {
	alt__waiter w;
	alt__waiter_init(&w);
	alt__waiter_claim(&w); /* nobody else completes it: the worker does */
	job j = {0, fn, arg, &w};
	int rc = submit(p, &j);
	if (rc)
		return rc;
	return alt__wait(&w, ALT_FOREVER, 0, 0);
}

void alt_pool_free(alt_pool *p) {
	if (!p)
		return;
	alt_mutex_lock(&p->m);
	p->closing = true;
	alt_cond_broadcast(&p->work);
	while (p->threads > 0)
		alt_cond_wait(&p->idle_all, &p->m);
	alt_mutex_unlock(&p->m);
	free(p);
}

static alt_once default_once = ALT_ONCE_INIT;
static alt_pool *default_pool;

static void make_default(void *arg) {
	(void)arg;
	alt_pool_new(&default_pool, 256);
}

alt_pool *alt_pool_default(void) {
	alt_once_do(&default_once, make_default, 0);
	return default_pool;
}
