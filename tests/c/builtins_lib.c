#include <stdio.h>
#include <stdint.h>

int main(void) {
    unsigned long ul; printf("%d ", __builtin_umull_overflow(1UL << 63, 2, &ul)); printf("%lu\n", ul);
    printf("%g %g %g\n", __builtin_fabs(-2.5), __builtin_sqrt(16.0), __builtin_copysign(1.0, -3.0));
    char buf[16]; __builtin_memcpy(buf, "builtin", 8); printf("%s %zu %d\n", buf, __builtin_strlen(buf), __builtin_strcmp(buf, "builtin"));
    __builtin_memset(buf, 'z', 3); printf("%.5s\n", buf);
    printf("%d\n", __builtin_abs(-9));
    printf("%lld\n", (long long)__builtin_llabs(-5LL));
    return 0;
}
