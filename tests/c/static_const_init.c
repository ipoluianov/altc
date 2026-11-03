// GNU extension: const objects with constant initializers in static initializers
#include <stdio.h>
#include <stdint.h>
static const uint64_t three = 0xc0000000;
static const int base = 10;
static const short neg = -5;
struct u128 { uint64_t hi, lo; };
static const struct u128 g = {.hi = three << 32, .lo = base + 1};
static const int table[] = {base, base * 2, neg};
int main(void) {
    static const uint64_t local = 7;
    static const struct u128 l = {.hi = local * 3, .lo = three >> 4};
    static int arr[3] = {base, neg, (int)local};
    printf("%llx %llu %d %d %d\n", (unsigned long long)g.hi, (unsigned long long)g.lo, table[0], table[1], table[2]);
    printf("%llu %llx %d %d %d\n", (unsigned long long)l.hi, (unsigned long long)l.lo, arr[0], arr[1], arr[2]);
    return 0;
}
