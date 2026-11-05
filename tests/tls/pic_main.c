/* compiled by gcc -fPIC: general and local dynamic TLS accesses that the
   linker relaxes */
#include <pthread.h>
#include <stdio.h>

__thread int counter = 5;
static __thread int hidden[4] = {1, 2, 3, 4};
extern __thread int libvar; /* from the shared library (or the AltC object) */
int altc_work(int);          /* compiled by AltC */

__attribute__((noinline)) int *counter_addr(void) { return &counter; }
__attribute__((noinline)) int sum_hidden(void) {
    int s = 0;
    for (int i = 0; i < 4; i++)
        s += hidden[i]++;
    return s;
}

static void *thread(void *arg) {
    (void)arg;
    *counter_addr() += 100;
    libvar += 7;
    return (void *)(long)(*counter_addr() * 1000 + sum_hidden() * 10 + libvar);
}

int main(void) {
    pthread_t t;
    void *r;
    pthread_create(&t, 0, thread, 0);
    pthread_join(t, &r);
    printf("thread %ld\n", (long)r);
    printf("main %d %d %d %d\n", *counter_addr(), sum_hidden(), sum_hidden(), libvar);
    printf("altc %d\n", altc_work(3));
    return 0;
}
