#include "check.h"
#include <alt/sync.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

/* a counter protected by a mutex, incremented by tasks of three
   schedulers and by plain threads at the same time */
static alt_mutex mu = ALT_MUTEX_INIT;
static long counter;
static alt_waitgroup wg = ALT_WAITGROUP_INIT;
enum { ITER = 20000 };

static void inc_task(void *arg) {
	(void)arg;
	for (int i = 0; i < ITER; i++) {
		alt_mutex_lock(&mu);
		long v = counter;
		if (i % 100 == 0)
			alt_yield(); /* holding the mutex across a switch */
		counter = v + 1;
		alt_mutex_unlock(&mu);
	}
	alt_waitgroup_done(&wg);
}

static void *inc_thread(void *arg) {
	inc_task(arg);
	return 0;
}

static void mutexes(void) {
	counter = 0;
	alt_sched *s[3];
	for (int i = 0; i < 3; i++)
		CHECK_OK(alt_sched_start(&s[i]));
	alt_waitgroup_add(&wg, 3 * 4 + 2);
	for (int i = 0; i < 12; i++)
		CHECK_OK(alt_go_on(s[i % 3], inc_task, 0, 0));
	pthread_t th[2];
	for (int i = 0; i < 2; i++)
		pthread_create(&th[i], 0, inc_thread, 0);
	alt_waitgroup_wait(&wg);
	for (int i = 0; i < 2; i++)
		pthread_join(th[i], 0);
	CHECK_INT(counter, 14L * ITER);
	for (int i = 0; i < 3; i++)
		CHECK_OK(alt_sched_stop(s[i]));
	/* trylock and timeouts */
	alt_mutex m = ALT_MUTEX_INIT;
	CHECK(alt_mutex_trylock(&m));
	CHECK(!alt_mutex_trylock(&m));
	alt_time t0 = alt_now();
	CHECK_ERR(alt_mutex_lock_until(&m, alt_deadline(10 * ALT_MS)), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 10 * ALT_MS);
	CHECK_ERR(alt_mutex_lock_until(&m, ALT_NOWAIT), -EAGAIN);
	alt_mutex_unlock(&m);
	CHECK_OK(alt_mutex_lock_until(&m, ALT_NOWAIT));
	alt_mutex_unlock(&m);
}

/* readers and writers: a writer must never see readers inside */
static alt_rwmutex rw = ALT_RWMUTEX_INIT;
static _Atomic int inside_readers, inside_writers, bad;
static long shared_value;

static void reader(void *arg) {
	(void)arg;
	for (int i = 0; i < 2000; i++) {
		alt_rwmutex_rlock(&rw);
		atomic_fetch_add(&inside_readers, 1);
		if (atomic_load(&inside_writers))
			atomic_fetch_add(&bad, 1);
		long v = shared_value;
		if (i % 50 == 0)
			alt_yield();
		if (v != shared_value)
			atomic_fetch_add(&bad, 1);
		atomic_fetch_sub(&inside_readers, 1);
		alt_rwmutex_runlock(&rw);
	}
	alt_waitgroup_done(&wg);
}

static void writer(void *arg) {
	(void)arg;
	for (int i = 0; i < 500; i++) {
		alt_rwmutex_lock(&rw);
		if (atomic_fetch_add(&inside_writers, 1) || atomic_load(&inside_readers))
			atomic_fetch_add(&bad, 1);
		shared_value++;
		if (i % 20 == 0)
			alt_yield();
		atomic_fetch_sub(&inside_writers, 1);
		alt_rwmutex_unlock(&rw);
	}
	alt_waitgroup_done(&wg);
}

static void *reader_thread(void *a) {
	reader(a);
	return 0;
}

static void rwmutexes(void) {
	alt_sched *s[2];
	for (int i = 0; i < 2; i++)
		alt_sched_start(&s[i]);
	alt_waitgroup_add(&wg, 8 + 3 + 2);
	for (int i = 0; i < 8; i++)
		alt_go_on(s[i % 2], reader, 0, 0);
	for (int i = 0; i < 3; i++)
		alt_go_on(s[i % 2], writer, 0, 0);
	pthread_t th[2];
	for (int i = 0; i < 2; i++)
		pthread_create(&th[i], 0, reader_thread, 0);
	alt_waitgroup_wait(&wg);
	for (int i = 0; i < 2; i++)
		pthread_join(th[i], 0);
	CHECK_INT(atomic_load(&bad), 0);
	CHECK_INT(shared_value, 1500);
	for (int i = 0; i < 2; i++)
		alt_sched_stop(s[i]);
	alt_rwmutex r = ALT_RWMUTEX_INIT;
	CHECK(alt_rwmutex_tryrlock(&r));
	CHECK(alt_rwmutex_tryrlock(&r));
	CHECK(!alt_rwmutex_trylock(&r));
	alt_rwmutex_runlock(&r);
	alt_rwmutex_runlock(&r);
	CHECK(alt_rwmutex_trylock(&r));
	CHECK(!alt_rwmutex_tryrlock(&r));
	alt_rwmutex_unlock(&r);
}

/* a bounded queue with two condition variables */
static alt_mutex qm = ALT_MUTEX_INIT;
static alt_cond not_empty = ALT_COND_INIT, not_full = ALT_COND_INIT;
static int queue[8], qhead, qlen;
static long consumed_sum;
static int done_producers;

static void produce(void *arg) {
	int base = (int)(intptr_t)arg;
	for (int i = 0; i < 3000; i++) {
		alt_mutex_lock(&qm);
		while (qlen == 8)
			alt_cond_wait(&not_full, &qm);
		queue[(qhead + qlen++) % 8] = base + i;
		alt_cond_signal(&not_empty);
		alt_mutex_unlock(&qm);
	}
	alt_mutex_lock(&qm);
	done_producers++;
	alt_cond_broadcast(&not_empty);
	alt_mutex_unlock(&qm);
	alt_waitgroup_done(&wg);
}

static void consume(void *arg) {
	(void)arg;
	for (;;) {
		alt_mutex_lock(&qm);
		while (qlen == 0 && done_producers < 4)
			alt_cond_wait(&not_empty, &qm);
		if (qlen == 0) {
			alt_mutex_unlock(&qm);
			break;
		}
		int v = queue[qhead];
		qhead = (qhead + 1) % 8;
		qlen--;
		consumed_sum += v;
		alt_cond_signal(&not_full);
		alt_mutex_unlock(&qm);
	}
	alt_waitgroup_done(&wg);
}

static void *consume_thread(void *a) {
	consume(a);
	return 0;
}

static void cond_main(void *arg) {
	(void)arg;
	for (int i = 0; i < 4; i++)
		alt_go(produce, (void *)(intptr_t)(i * 10000));
	alt_go(consume, 0);
}

static void conds(void) {
	alt_waitgroup_add(&wg, 4 + 1 + 1);
	pthread_t th;
	pthread_create(&th, 0, consume_thread, 0);
	CHECK_OK(alt_run(cond_main, 0));
	alt_waitgroup_wait(&wg);
	pthread_join(th, 0);
	long want = 0;
	for (int p = 0; p < 4; p++)
		for (int i = 0; i < 3000; i++)
			want += p * 10000 + i;
	CHECK_INT(consumed_sum, want);
	/* a timed wait */
	alt_mutex m = ALT_MUTEX_INIT;
	alt_cond c = ALT_COND_INIT;
	alt_mutex_lock(&m);
	alt_time t0 = alt_now();
	CHECK_ERR(alt_cond_wait_until(&c, &m, alt_deadline(10 * ALT_MS)), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 10 * ALT_MS);
	CHECK(!alt_mutex_trylock(&m)); /* locked again */
	alt_mutex_unlock(&m);
	alt_cond_signal(&c); /* nobody waits */
	alt_cond_broadcast(&c);
}

static alt_once once = ALT_ONCE_INIT;
static _Atomic int once_runs;
static void init_once(void *arg) {
	(void)arg;
	alt_sleep(5 * ALT_MS);
	atomic_fetch_add(&once_runs, 1);
}
static void once_task(void *arg) {
	(void)arg;
	alt_once_do(&once, init_once, 0);
	CHECK_INT(atomic_load(&once_runs), 1);
}
static void once_main(void *arg) {
	(void)arg;
	for (int i = 0; i < 10; i++)
		alt_go(once_task, 0);
}

static void onces(void) {
	CHECK_OK(alt_run(once_main, 0));
	alt_once_do(&once, init_once, 0);
	CHECK_INT(atomic_load(&once_runs), 1);
}

static void wait_groups(void) {
	alt_waitgroup g = ALT_WAITGROUP_INIT;
	CHECK_OK(alt_waitgroup_wait_until(&g, ALT_NOWAIT));
	alt_waitgroup_add(&g, 2);
	CHECK_ERR(alt_waitgroup_wait_until(&g, ALT_NOWAIT), -EAGAIN);
	CHECK_ERR(alt_waitgroup_wait_until(&g, alt_deadline(5 * ALT_MS)), -ETIMEDOUT);
	alt_waitgroup_done(&g);
	alt_waitgroup_done(&g);
	CHECK_OK(alt_waitgroup_wait_until(&g, ALT_NOWAIT));
}

/* at most 3 tasks at once inside */
static alt_sem sem = ALT_SEM_INIT(3);
static _Atomic int in_sem, max_in_sem;
static void sem_task(void *arg) {
	(void)arg;
	for (int i = 0; i < 20; i++) {
		alt_sem_acquire(&sem, 1, ALT_FOREVER);
		int n = atomic_fetch_add(&in_sem, 1) + 1;
		int m = atomic_load(&max_in_sem);
		while (n > m && !atomic_compare_exchange_weak(&max_in_sem, &m, n)) {
		}
		alt_sleep(ALT_MS / 10);
		atomic_fetch_sub(&in_sem, 1);
		alt_sem_release(&sem, 1);
	}
}
static void sem_main(void *arg) {
	(void)arg;
	for (int i = 0; i < 10; i++)
		alt_go(sem_task, 0);
}

static void semaphores(void) {
	CHECK_OK(alt_run(sem_main, 0));
	CHECK_INT(atomic_load(&max_in_sem), 3);
	alt_sem s;
	alt_sem_init(&s, 5);
	CHECK_OK(alt_sem_acquire(&s, 4, ALT_NOWAIT));
	CHECK_ERR(alt_sem_acquire(&s, 2, ALT_NOWAIT), -EAGAIN);
	CHECK_ERR(alt_sem_acquire(&s, 2, alt_deadline(5 * ALT_MS)), -ETIMEDOUT);
	CHECK_OK(alt_sem_acquire(&s, 1, ALT_NOWAIT));
	alt_sem_release(&s, 5);
	CHECK_OK(alt_sem_acquire(&s, 5, ALT_NOWAIT));
}

/* blocking calls from a task through the pool do not stop the scheduler */
static _Atomic int ticks;
static void blocking_call(void *arg) {
	(void)arg;
	usleep(50000); /* a plain blocking call */
}
static void ticker_task(void *arg) {
	(void)arg;
	for (int i = 0; i < 10; i++) {
		alt_sleep(2 * ALT_MS);
		atomic_fetch_add(&ticks, 1);
	}
}
static void pool_main(void *arg) {
	(void)arg;
	alt_go(ticker_task, 0);
	alt_time t0 = alt_now();
	CHECK_OK(alt_pool_run(alt_pool_default(), blocking_call, 0));
	CHECK(alt_since(t0) >= 50 * ALT_MS);
	/* the other task kept running meanwhile */
	CHECK(atomic_load(&ticks) >= 5);
}

static _Atomic long pool_sum;
static void add_job(void *arg) { atomic_fetch_add(&pool_sum, (long)(intptr_t)arg); }

static void pools(void) {
	CHECK_OK(alt_run(pool_main, 0));
	alt_pool *p;
	CHECK_OK(alt_pool_new(&p, 4));
	for (int i = 1; i <= 1000; i++)
		CHECK_OK(alt_pool_go(p, add_job, (void *)(intptr_t)i));
	alt_pool_free(p); /* waits for the jobs */
	CHECK_INT(atomic_load(&pool_sum), 500500);
	CHECK(alt_num_cpu() >= 1);
	CHECK_OK(alt_pool_run(alt_pool_default(), add_job, (void *)1));
	CHECK_INT(atomic_load(&pool_sum), 500501);
}

int main(void) {
	RUN(mutexes);
	RUN(rwmutexes);
	RUN(conds);
	RUN(onces);
	RUN(wait_groups);
	RUN(semaphores);
	RUN(pools);
	return check_done();
}
