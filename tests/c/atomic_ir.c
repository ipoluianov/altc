// atomic operations through the IR backend: every size and signedness,
// old and new values, compare-and-swap forms, threads
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

static signed char sc = -100;
static unsigned char uc = 250;
static short ss = -30000;
static unsigned short us = 65000;
static int si = -5;
static unsigned ui = 4000000000u;
static long sl = -1;
static unsigned long ul = 1;
static int arr[4] = {10, 20, 30, 40};
static int *ptr = arr;
static _Atomic int counter;
static _Atomic long lcounter;
static int plain;
static _Atomic int flag;

static void *worker(void *arg) {
    long id = (long)arg;
    for (int i = 0; i < 10000; i++) {
        counter += 1;
        atomic_fetch_add(&lcounter, id);
        __sync_fetch_and_add(&plain, 1);
        int old;
        do
            old = __atomic_load_n(&si, __ATOMIC_RELAXED);
        while (!__sync_bool_compare_and_swap(&si, old, old + 1));
    }
    atomic_store(&flag, 1);
    return 0;
}

int main(void) {
    // fetch-and-op returning old and new values (wrapping)
    { long long x1 = __sync_fetch_and_add(&sc, 50); long long x2 = __sync_add_and_fetch(&sc, 50); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __sync_fetch_and_add(&uc, 10); long long x2 = __sync_sub_and_fetch(&uc, 20); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __sync_fetch_and_sub(&ss, 5000); long long x2 = __sync_and_and_fetch(&ss, 0x7ff0); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __sync_fetch_and_or(&us, 0xff); long long x2 = __sync_xor_and_fetch(&us, 0xffff); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __sync_fetch_and_nand(&si, 6); long long x2 = __sync_nand_and_fetch(&si, 3); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __sync_fetch_and_add(&ui, 500000000u); long long x2 = __sync_or_and_fetch(&ui, 1); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __sync_fetch_and_xor(&sl, 0x55); long long x2 = __sync_fetch_and_and(&sl, 0xf0f0); printf("%lld %lld\n", x1, x2); }
    { long long x1 = __atomic_fetch_add(&ul, 41, __ATOMIC_SEQ_CST); long long x2 = __atomic_add_fetch(&ul, 1, __ATOMIC_SEQ_CST); printf("%lld %lld\n", x1, x2); }
    int *p = __sync_fetch_and_add(&ptr, 2);
    printf("%d %d\n", *p, *ptr);
    p = __atomic_sub_fetch(&ptr, 1, __ATOMIC_SEQ_CST);
    printf("%d\n", *p);

    // compare and swap
    sc = 7;
    { int x1 = __sync_bool_compare_and_swap(&sc, 7, -8); int x2 = __sync_bool_compare_and_swap(&sc, 7, 9); printf("%d %d %d\n", x1, x2, sc); }
    { long long x1 = __sync_val_compare_and_swap(&us, 65535, 3); long long x2 = __sync_val_compare_and_swap(&us, 1, 4); printf("%lld %lld\n", x1, x2); }
    long expected = 12345;
    sl = 12345;
    _Bool ok = __atomic_compare_exchange_n(&sl, &expected, 99, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    printf("%d %ld %ld\n", ok, sl, expected);
    ok = __atomic_compare_exchange_n(&sl, &expected, 77, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    printf("%d %ld %ld\n", ok, sl, expected);
    unsigned char ue = 1;
    uc = 200;
    ok = atomic_compare_exchange_strong((_Atomic unsigned char *)&uc, &ue, 5);
    printf("%d %d %d\n", ok, uc, ue);

    // exchange
    { int x1 = __atomic_exchange_n(&ss, -2, __ATOMIC_SEQ_CST); printf("%d %d\n", x1, ss); }
    { int x1 = __sync_lock_test_and_set(&uc, 1); printf("%d %d\n", x1, uc); }
    __sync_lock_release(&uc);
    printf("%d\n", uc);

    // _Atomic compound assignments
    _Atomic short as = 32767;
    as += 1;
    printf("%d\n", (int)as);
    _Atomic unsigned au = 0;
    au -= 1;
    au &= 0xff00ff;
    au |= 2;
    au ^= 3;
    printf("%u\n", (unsigned)au);

    // pointers: the builtins add bytes, _Atomic pointer arithmetic scales
    {
        int *bp = arr;
        _Atomic(int *) ap = arr;
        __sync_fetch_and_add(&bp, 4);
        atomic_fetch_add(&ap, 4);
        printf("%ld %ld\n", (long)((char *)bp - (char *)arr), (long)((char *)ap - (char *)arr));
        ap += 1;
        ap++;
        ap -= 1;
        printf("%ld %d\n", (long)((char *)ap - (char *)arr), *ap);
    }

    // threads
    si = 0;
    pthread_t t[4];
    for (long i = 0; i < 4; i++)
        pthread_create(&t[i], 0, worker, (void *)(i + 1));
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], 0);
    printf("%d %ld %d %d %d\n", (int)counter, (long)lcounter, plain, si, (int)flag);
    return 0;
}
