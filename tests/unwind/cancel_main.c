#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
void inner_loop(void);
static void cleanup(void *p) { printf("cleanup %s\n", (char *)p); }
static void *thr(void *arg) {
    pthread_cleanup_push(cleanup, "outer");
    inner_loop();
    pthread_cleanup_pop(0);
    return 0;
}
int main(void) {
    pthread_t t; pthread_create(&t, 0, thr, 0);
    usleep(100000); pthread_cancel(t);
    void *r; pthread_join(t, &r);
    printf("joined canceled=%d\n", r == PTHREAD_CANCELED);
    return 0;
}
