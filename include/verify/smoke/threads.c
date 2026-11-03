/* Smoke test: <pthread.h>, <threads.h>, <semaphore.h>.  */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <threads.h>

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t key;
static int counter, ready, once_count, cleanup_ran;
static sem_t sem;
static mtx_t cmtx;
static once_flag cflag = ONCE_FLAG_INIT;
static tss_t tkey;

static void
init_once (void)
{
  once_count++;
}

static void
cleanup (void *arg)
{
  cleanup_ran = *(int *) arg;
}

static void *
worker (void *arg)
{
  int i, v = 5;
  pthread_once (&once, init_once);
  pthread_setspecific (key, arg);
  for (i = 0; i < 1000; i++)
    {
      pthread_mutex_lock (&mu);
      counter++;
      pthread_mutex_unlock (&mu);
    }
  pthread_rwlock_rdlock (&rw);
  pthread_rwlock_unlock (&rw);
  pthread_cleanup_push (cleanup, &v);
  pthread_cleanup_pop (1);
  pthread_mutex_lock (&mu);
  ready++;
  pthread_cond_signal (&cv);
  pthread_mutex_unlock (&mu);
  sem_post (&sem);
  return pthread_getspecific (key);
}

static void
c_once (void)
{
  once_count += 100;
}

static int
c_worker (void *arg)
{
  int i;
  call_once (&cflag, c_once);
  tss_set (tkey, arg);
  for (i = 0; i < 1000; i++)
    {
      mtx_lock (&cmtx);
      counter++;
      mtx_unlock (&cmtx);
    }
  return *(int *) tss_get (tkey);
}

int
main (void)
{
  pthread_t th[4];
  thrd_t ct[2];
  int ids[4] = { 1, 2, 3, 4 }, i, res;
  void *ret;
  long sum = 0;
  pthread_attr_t attr;
  size_t ss;

  pthread_key_create (&key, NULL);
  sem_init (&sem, 0, 0);
  pthread_attr_init (&attr);
  pthread_attr_setstacksize (&attr, 1 << 20);
  pthread_attr_getstacksize (&attr, &ss);
  for (i = 0; i < 4; i++)
    pthread_create (&th[i], &attr, worker, &ids[i]);
  pthread_mutex_lock (&mu);
  while (ready < 4)
    pthread_cond_wait (&cv, &mu);
  pthread_mutex_unlock (&mu);
  for (i = 0; i < 4; i++)
    {
      pthread_join (th[i], &ret);
      sum += *(int *) ret;
      sem_wait (&sem);
    }
  printf ("pthread counter %d sum %ld once %d cleanup %d stack %zu\n",
          counter, sum, once_count, cleanup_ran, ss);
  printf ("equal %d\n", pthread_equal (pthread_self (), pthread_self ()));

  mtx_init (&cmtx, mtx_plain);
  tss_create (&tkey, NULL);
  for (i = 0; i < 2; i++)
    thrd_create (&ct[i], c_worker, &ids[i]);
  sum = 0;
  for (i = 0; i < 2; i++)
    {
      thrd_join (ct[i], &res);
      sum += res;
    }
  printf ("c11 counter %d sum %ld once %d\n", counter, sum, once_count);
  mtx_destroy (&cmtx);
  sem_destroy (&sem);
  pthread_attr_destroy (&attr);
  return 0;
}
