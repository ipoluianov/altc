// regression: a struct returned in registers by a function with inlined calls
#include <stdio.h>
#include <stdint.h>
typedef uint64_t u64;
typedef struct { u64 low64, high64; } h128_t;
static u64 xs(u64 v, int s) { return v ^ (v >> s); }
static u64 aval(u64 h) { h = xs(h, 37); h *= 0x165667919E3779F9ULL; h = xs(h, 32); return h; }
__attribute__((noinline)) h128_t f(u64 len, u64 seed) {
    h128_t acc;
    acc.low64 = len * 11400714785074694791ULL;
    acc.high64 = 0;
    if (len > 32) acc.high64 = len * 3;
    {
        h128_t h;
        h.low64 = acc.low64 + acc.high64;
        h.high64 = (acc.low64 * 3) + (acc.high64 * 5) + ((len - seed) * 7);
        h.low64 = aval(h.low64);
        h.high64 = (u64)0 - aval(h.high64);
        return h;
    }
}
int main(void) {
    u64 a = 0;
    for (u64 n = 17; n < 60; n++) { h128_t r = f(n, n * 3); a = a * 31 + r.low64 + r.high64; }
    printf("%llx\n", (unsigned long long)a);
    return 0;
}
