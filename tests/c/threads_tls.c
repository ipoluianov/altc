#include <stdio.h>
#include <pthread.h>
#include <string.h>

static _Thread_local int tl_counter = 100;
static __thread char tl_name[16] = "main";
static _Thread_local long tl_arr[4];
static int shared_results[4];
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static long total;

static int *tl_addr(void) { return &tl_counter; }

static void *worker(void *arg) {
    int id = (int)(long)arg;
    snprintf(tl_name, sizeof tl_name, "worker%d", id);
    for (int i = 0; i < 1000; i++) { tl_counter++; tl_arr[i % 4] += id; }
    *tl_addr() += id;
    shared_results[id] = tl_counter;
    pthread_mutex_lock(&mu); total += tl_arr[0] + tl_arr[3]; pthread_mutex_unlock(&mu);
    return (void *)(long)(strlen(tl_name) + tl_counter);
}

int main(void) {
    pthread_t th[4];
    for (long i = 0; i < 4; i++) pthread_create(&th[i], NULL, worker, (void *)i);
    long rets[4];
    for (int i = 0; i < 4; i++) { void *r; pthread_join(th[i], &r); rets[i] = (long)r; }
    for (int i = 0; i < 4; i++) printf("%d %ld\n", shared_results[i], rets[i]);
    printf("main: %d %s %ld total=%ld\n", tl_counter, tl_name, tl_arr[0], total);
    return 0;
}
