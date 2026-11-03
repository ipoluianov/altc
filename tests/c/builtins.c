#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

struct S { int a; char b[3]; double c; };

int main(void) {
    unsigned xs[] = {1, 2, 3, 0x80000000u, 0xffffffffu, 0x00f0f000u, 12345};
    for (int i = 0; i < 7; i++) {
        unsigned x = xs[i];
        printf("%u: clz=%d ctz=%d pop=%d parity=%d ffs=%d\n", x, __builtin_clz(x), __builtin_ctz(x), __builtin_popcount(x), __builtin_parity(x), __builtin_ffs(x));
    }
    unsigned long long ys[] = {1, 0x8000000000000000ULL, 0xff00000000ULL, 0xffffffffffffffffULL, 0x123456789ULL};
    for (int i = 0; i < 5; i++) {
        unsigned long long y = ys[i];
        printf("%llx: clzll=%d ctzll=%d popll=%d clzl=%d ctzl=%d popl=%d ffsll=%d\n", y, __builtin_clzll(y), __builtin_ctzll(y), __builtin_popcountll(y),
               __builtin_clzl(y), __builtin_ctzl(y), __builtin_popcountl(y), __builtin_ffsll(y));
    }
    printf("%d %d\n", __builtin_ffs(0), __builtin_clrsb(-1));
    printf("%x %x %llx\n", __builtin_bswap16(0x1234), __builtin_bswap32(0x12345678), (unsigned long long)__builtin_bswap64(0x0102030405060708ULL));
    int r; printf("%d ", __builtin_add_overflow(INT_MAX, 1, &r)); printf("%d\n", r);
    printf("%d ", __builtin_add_overflow(5, 6, &r)); printf("%d\n", r);
    printf("%d ", __builtin_sub_overflow(INT_MIN, 1, &r)); printf("%d\n", r);
    printf("%d ", __builtin_mul_overflow(65536, 65536, &r)); printf("%d\n", r);
    long long lr; printf("%d ", __builtin_mul_overflow(65536, 65536, &lr)); printf("%lld\n", lr);
    unsigned ur; printf("%d ", __builtin_sub_overflow(1u, 2u, &ur)); printf("%u\n", ur);
    printf("%d ", __builtin_add_overflow(-1, 0u, &ur)); printf("%u\n", ur);
    unsigned char ucr; printf("%d ", __builtin_add_overflow(200, 100, &ucr)); printf("%d\n", ucr);
    printf("%d ", __builtin_sadd_overflow(1, 2, &r)); printf("%d\n", r);
    printf("%d\n", __builtin_expect(5, 1) + __builtin_expect_with_probability(1, 1, 0.9));
    if (__builtin_expect(r > 0, 1)) printf("likely\n");
    printf("%zu %zu\n", __builtin_offsetof(struct S, c), __builtin_offsetof(struct S, b[2]));
    printf("%d %d %d\n", __builtin_types_compatible_p(int, int), __builtin_types_compatible_p(int, unsigned), __builtin_types_compatible_p(char *, typeof("x"+0)));
    printf("%d\n", __builtin_constant_p(10 * 2));
    printf("%d\n", __builtin_choose_expr(1, 10, 20.0));
    printf("%zu\n", sizeof(__builtin_choose_expr(0, 10, 20.0)));
    printf("%g %g %d\n", __builtin_inf(), __builtin_huge_val(), __builtin_isnan(__builtin_nan("")));
    int v = 5;
    printf("%d ", __sync_fetch_and_add(&v, 3)); printf("%d\n", v);
    printf("%d ", __sync_add_and_fetch(&v, 2)); printf("%d\n", v);
    printf("%d ", __sync_fetch_and_sub(&v, 1)); printf("%d\n", v);
    printf("%d ", __sync_fetch_and_or(&v, 0x10)); printf("%d\n", v);
    printf("%d ", __sync_fetch_and_and(&v, 0x1c)); printf("%d\n", v);
    printf("%d ", __sync_fetch_and_xor(&v, 0xff)); printf("%d\n", v);
    printf("%d ", __sync_bool_compare_and_swap(&v, 0xe3, 7)); printf("%d\n", v);
    printf("%d ", __sync_val_compare_and_swap(&v, 1, 9)); printf("%d\n", v);
    printf("%d ", __sync_lock_test_and_set(&v, 42)); printf("%d\n", v);
    __sync_lock_release(&v); printf("%d\n", v);
    __sync_synchronize();
    printf("%d\n", __atomic_always_lock_free(sizeof(long), 0));
    void *ra = __builtin_return_address(0); printf("%d\n", ra != 0);
    printf("%d\n", __builtin_classify_type(1.0));
    if (v == 12345) __builtin_unreachable();
    int arr[10]; __builtin_prefetch(arr);
    return 0;
}
