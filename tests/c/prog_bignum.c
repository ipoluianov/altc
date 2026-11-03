#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAXD 400
typedef struct { int n; uint32_t d[MAXD]; } Big; /* base 1e9, little endian */

static void big_set(Big *b, uint64_t v) { b->n = 0; do { b->d[b->n++] = v % 1000000000u; v /= 1000000000u; } while (v); }
static void big_mul(Big *r, const Big *a, const Big *b) {
    uint64_t t[2 * MAXD] = {0};
    for (int i = 0; i < a->n; i++) { uint64_t carry = 0; for (int j = 0; j < b->n || carry; j++) { uint64_t cur = t[i + j] + carry + (j < b->n ? (uint64_t)a->d[i] * b->d[j] : 0); t[i + j] = cur % 1000000000u; carry = cur / 1000000000u; } }
    int n = a->n + b->n; while (n > 1 && t[n - 1] == 0) n--;
    r->n = n; for (int i = 0; i < n; i++) r->d[i] = (uint32_t)t[i];
}
static void big_mul_small(Big *a, uint32_t m) { uint64_t c = 0; for (int i = 0; i < a->n; i++) { uint64_t cur = (uint64_t)a->d[i] * m + c; a->d[i] = cur % 1000000000u; c = cur / 1000000000u; } while (c) { a->d[a->n++] = c % 1000000000u; c /= 1000000000u; } }
static void big_add(Big *a, const Big *b) { uint32_t c = 0; int n = a->n > b->n ? a->n : b->n; for (int i = 0; i < n; i++) { uint64_t s = (uint64_t)(i < a->n ? a->d[i] : 0) + (i < b->n ? b->d[i] : 0) + c; a->d[i] = s % 1000000000u; c = s >= 1000000000u; } a->n = n; if (c) a->d[a->n++] = c; }
static void big_print(const char *label, const Big *b) { printf("%s", label); printf("%u", b->d[b->n - 1]); for (int i = b->n - 2; i >= 0; i--) printf("%09u", b->d[i]); printf("\n"); }
static int digit_sum(const Big *b) { int s = 0; for (int i = 0; i < b->n; i++) { uint32_t v = b->d[i]; while (v) { s += v % 10; v /= 10; } } return s; }

int main(void) {
    static Big f, a, b, r, x, y;
    big_set(&f, 1); for (uint32_t i = 2; i <= 100; i++) big_mul_small(&f, i);
    big_print("100! = ", &f); printf("digits sum %d\n", digit_sum(&f));
    big_set(&a, 18446744073709551615ULL); big_set(&b, 18446744073709551615ULL); big_mul(&r, &a, &b); big_print("(2^64-1)^2 = ", &r);
    big_set(&x, 2); big_set(&y, 1); for (int i = 0; i < 10; i++) { big_mul(&r, &x, &x); x = r; } big_print("2^1024 = ", &x);
    big_set(&a, 0); big_set(&b, 1); for (int i = 0; i < 300; i++) { Big t = b; big_add(&b, &a); a = t; } big_print("fib(300) = ", &a);
    return 0;
}
