/* AltC standard library: arbitrary-precision integers (Go's math/big Int).

   An alt_big is a sign and a magnitude of 64-bit words; it owns its
   memory (alt_big_free). Results go to the first argument, which may be
   any of the operands, as in Go:

	alt_big a = ALT_BIG_INIT, b = ALT_BIG_INIT;
	alt_big_set_string(&a, ALT_STR("123456789012345678901234567890"), 10);
	alt_big_mul(&b, &a, &a);
	alt_big_exp(&b, &a, &b, &m);

   Functions that allocate return 0 or -ENOMEM; the semantics (truncated
   and Euclidean division, Exp with negative exponents, GCD's Bézout
   coefficients, two's complement bit operations, string forms) are
   Go's. */
#ifndef ALT_MATH_BIG_H
#define ALT_MATH_BIG_H

#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint64_t *w;      /* the magnitude, least significant word first */
	size_t n, cap;    /* n: no leading zero words (0 for zero) */
	bool neg;         /* never for zero */
} alt_big;

#define ALT_BIG_INIT {0, 0, 0, false}

void alt_big_init(alt_big *z);
void alt_big_free(alt_big *z);

int alt_big_set(alt_big *z, const alt_big *x);
int alt_big_set_int64(alt_big *z, int64_t v);
int alt_big_set_uint64(alt_big *z, uint64_t v);
/* the magnitude from big-endian bytes (positive) */
int alt_big_set_bytes(alt_big *z, const void *b, size_t n);
/* the magnitude as big-endian bytes: the count (alt_big_bytes_len) */
size_t alt_big_bytes_len(const alt_big *x);
void alt_big_bytes(const alt_big *x, uint8_t *out);
/* the magnitude right-aligned in n bytes (zero-padded); -ERANGE if it does not fit */
int alt_big_fill_bytes(const alt_big *x, uint8_t *out, size_t n);

int alt_big_sign(const alt_big *x);
int alt_big_cmp(const alt_big *x, const alt_big *y);
int alt_big_cmp_abs(const alt_big *x, const alt_big *y);
bool alt_big_is_int64(const alt_big *x);
bool alt_big_is_uint64(const alt_big *x);
/* the low 64 bits (two's complement for int64), as Go's Int64/Uint64 */
int64_t alt_big_int64(const alt_big *x);
uint64_t alt_big_uint64(const alt_big *x);
size_t alt_big_bit_len(const alt_big *x);
size_t alt_big_trailing_zero_bits(const alt_big *x);
/* bit i of x in two's complement */
unsigned alt_big_bit(const alt_big *x, size_t i);
int alt_big_set_bit(alt_big *z, const alt_big *x, size_t i, unsigned b);

int alt_big_neg(alt_big *z, const alt_big *x);
int alt_big_abs(alt_big *z, const alt_big *x);
int alt_big_add(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_sub(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_mul(alt_big *z, const alt_big *x, const alt_big *y);
/* truncated division (Go's Quo, Rem, QuoRem): -EDOM when y is 0 */
int alt_big_quo(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_rem(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_quo_rem(alt_big *q, alt_big *r, const alt_big *x, const alt_big *y);
/* Euclidean division (Go's Div, Mod, DivMod): 0 <= m < |y| */
int alt_big_div(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_mod(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_div_mod(alt_big *q, alt_big *m, const alt_big *x, const alt_big *y);
int alt_big_lsh(alt_big *z, const alt_big *x, size_t n);
/* an arithmetic shift (rounds toward minus infinity) */
int alt_big_rsh(alt_big *z, const alt_big *x, size_t n);
int alt_big_and(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_or(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_xor(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_and_not(alt_big *z, const alt_big *x, const alt_big *y);
int alt_big_not(alt_big *z, const alt_big *x);

/* x**y mod |m| (m NULL or 0: x**y; y <= 0 without a modulus: 1); a
   negative y uses the inverse of x: -EDOM when there is none (z unchanged) */
int alt_big_exp(alt_big *z, const alt_big *x, const alt_big *y, const alt_big *m);
/* z = gcd(a, b) >= 0 and, when x or y are not NULL, a*x + b*y = z */
int alt_big_gcd(alt_big *z, alt_big *x, alt_big *y, const alt_big *a, const alt_big *b);
/* the inverse of g modulo n: -EDOM when g and n are not coprime (z unchanged) */
int alt_big_mod_inverse(alt_big *z, const alt_big *g, const alt_big *n);
/* the floor square root: -EDOM for negative x */
int alt_big_sqrt(alt_big *z, const alt_big *x);
/* the Jacobi symbol (x/y), y odd */
int alt_big_jacobi(const alt_big *x, const alt_big *y);
/* Miller-Rabin with n random bases plus Baillie-PSW (Go's ProbablyPrime) */
bool alt_big_probably_prime(const alt_big *x, int n);
/* a random prime of bits bits (top two bits set, as Go's crypto/rand.Prime) */
int alt_big_random_prime(alt_big *z, size_t bits);

/* the number in base 2..62 ("-1a"; digits, then a-z, then A-Z) */
int alt_big_append_text(alt_buf *b, const alt_big *x, int base);
/* Go's SetString: base 0 takes a prefix (0b, 0o, 0, 0x) and underscores;
   the whole string must be a number: -EINVAL otherwise (z unchanged) */
int alt_big_set_string(alt_big *z, alt_str s, int base);

#ifdef __cplusplus
}
#endif

#endif
