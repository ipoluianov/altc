/* The scheduler's internals, shared by sched.c and the poller */
#ifndef ALT_RT_SCHED_H
#define ALT_RT_SCHED_H

#include "rt.h"
#include <pthread.h>

/* a task's park state: wakeups that arrive before the task has stopped
   running are remembered (PENDING) */
enum { PS_RUN, PS_PARKED, PS_READY, PS_PENDING };
/* what a task asks of the scheduler when it switches out */
enum { T_PARK, T_YIELD, T_EXIT };

struct alt_task {
	void *sp;
	unsigned char *stack;
	size_t stack_size;
	alt_sched *sched;
	alt_task_fn fn;
	void *arg;
	const char *name;
	int saved_errno;
	int why;             /* T_PARK, T_YIELD, T_EXIT */
	_Atomic int ps;      /* park state */
	alt_task *next;      /* run queues */
	/* the deadline of the current wait */
	alt_time when;
	size_t heap_index;   /* SIZE_MAX: not in the heap */
	alt__waiter *timer_w;
	void *san_fake;      /* AddressSanitizer: the fake stack while switched out */
	void *san_fiber;     /* ThreadSanitizer: the fiber */
};

struct alt_sched {
	alt_task *current;
	void *loop_sp;
	void (*unlock)(void *);
	void *unlock_arg;
	alt_task *rq_head, *rq_tail;   /* ready tasks (owner thread only) */
	alt__lock rlock;               /* remote queue */
	alt_task *rem_head, *rem_tail;
	bool sleeping;                 /* in the poller with an infinite or long wait */
	_Atomic bool stop;
	bool until_empty;              /* alt_run: return when no task is left */
	_Atomic size_t ntasks;
	alt_task **heap;               /* timers: tasks by deadline */
	size_t nheap, heapcap;
	int epfd, evfd;
	_Atomic bool dead;
	_Atomic size_t registered;     /* poll descriptors registered with epfd */
	_Atomic unsigned gen;          /* changes when the structure is reused */
	struct alt_sched *next_free;   /* the pool of structures */
	pthread_t thread;
	void *san_fake;              /* sanitizers: the loop's context */
	const void *san_bottom;
	size_t san_size;
	void *san_fiber;
};

/* poller (poll_linux.c) */
int alt__poll_init(alt_sched *s);
void alt__poll_free(alt_sched *s);
void alt__poll(alt_sched *s, alt_time deadline);
void alt__poll_wakeup(alt_sched *s);

#endif
