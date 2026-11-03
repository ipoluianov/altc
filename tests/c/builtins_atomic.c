#include <stdio.h>
#include <stdint.h>

int main(void) {
    long lv = 10;
    printf("%ld ", __atomic_fetch_add(&lv, 5, __ATOMIC_SEQ_CST)); printf("%ld\n", lv);
    printf("%ld ", __atomic_add_fetch(&lv, 5, __ATOMIC_RELAXED)); printf("%ld\n", lv);
    printf("%ld ", __atomic_sub_fetch(&lv, 1, __ATOMIC_ACQ_REL)); printf("%ld\n", lv);
    printf("%ld\n", __atomic_load_n(&lv, __ATOMIC_ACQUIRE));
    __atomic_store_n(&lv, 77, __ATOMIC_RELEASE); printf("%ld\n", lv);
    printf("%ld ", __atomic_exchange_n(&lv, 88, __ATOMIC_SEQ_CST)); printf("%ld\n", lv);
    long expected = 88; printf("%d ", __atomic_compare_exchange_n(&lv, &expected, 99, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)); printf("%ld %ld\n", lv, expected);
    expected = 1; printf("%d ", __atomic_compare_exchange_n(&lv, &expected, 5, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)); printf("%ld %ld\n", lv, expected);
    long out; __atomic_load(&lv, &out, __ATOMIC_SEQ_CST); printf("%ld\n", out);
    long nv = 3; __atomic_store(&lv, &nv, __ATOMIC_SEQ_CST); printf("%ld\n", lv);
    printf("%ld ", __atomic_fetch_or(&lv, 8, __ATOMIC_SEQ_CST)); printf("%ld\n", lv);
    unsigned char flag = 0; { int t1 = __atomic_test_and_set(&flag, __ATOMIC_SEQ_CST); int t2 = __atomic_test_and_set(&flag, __ATOMIC_SEQ_CST); printf("%d %d\n", t1, t2); }
    __atomic_clear(&flag, __ATOMIC_SEQ_CST); printf("%d\n", flag);
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    short sv = 1; __atomic_fetch_add(&sv, 1, __ATOMIC_SEQ_CST); char cv = 1; __sync_fetch_and_add(&cv, 2); printf("%d %d\n", sv, cv);
    return 0;
}
