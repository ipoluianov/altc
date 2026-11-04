// ifunc attribute and libgcc's processor model (__builtin_cpu_*)
#include <stdio.h>

static int add_plain(int x) { return x + 1; }
static int add_fast(int x) { return x + 100; }

static void *resolve_add(void) {
    __builtin_cpu_init();
    return __builtin_cpu_supports("sse2") ? (void *)add_fast : (void *)add_plain;
}

int add(int) __attribute__((ifunc("resolve_add")));

static long twice_a(long x) { return 2 * x; }
static long (*resolve_twice(void))(long) { return twice_a; }
long twice(long) __attribute__((ifunc("resolve_twice")));

int main(void) {
    int (*p)(int) = add;
    printf("%d %d %ld\n", add(1), p(2), twice(21));
    printf("%d %d %d %d\n", __builtin_cpu_supports("sse2"), __builtin_cpu_supports("x86-64"),
           __builtin_cpu_supports("cmov"), __builtin_cpu_supports("x86-64-v2"));
    printf("%d %d %d\n", !!__builtin_cpu_supports("avx2"), !!__builtin_cpu_supports("sha"),
           __builtin_cpu_is("intel") + 2 * __builtin_cpu_is("amd"));
    return p == add ? 0 : 3;
}
