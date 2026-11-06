/* C23 <stdbit.h> functions (musl 1.2.5 does not have them) */
#include <stdbit.h>

#define W(T) (8 * (unsigned)sizeof(T))
/* leading zeros of x, a value of W bits, computed in 64 bits */
#define LZ(x, w) ((x) ? (unsigned)__builtin_clzll(x) - (64 - (w)) : (w))
#define TZ(x, w) ((x) ? (unsigned)__builtin_ctzll(x) : (w))

#define FUNCS(T, S) \
	unsigned int stdc_leading_zeros_##S(T x) { return LZ((unsigned long long)x, W(T)); } \
	unsigned int stdc_leading_ones_##S(T x) { return LZ((unsigned long long)(T)~x, W(T)); } \
	unsigned int stdc_trailing_zeros_##S(T x) { return TZ((unsigned long long)x, W(T)); } \
	unsigned int stdc_trailing_ones_##S(T x) { return TZ((unsigned long long)(T)~x, W(T)); } \
	unsigned int stdc_first_leading_zero_##S(T x) { \
		unsigned n = stdc_leading_ones_##S(x); \
		return n == W(T) ? 0 : n + 1; \
	} \
	unsigned int stdc_first_leading_one_##S(T x) { return x ? stdc_leading_zeros_##S(x) + 1 : 0; } \
	unsigned int stdc_first_trailing_zero_##S(T x) { \
		unsigned n = stdc_trailing_ones_##S(x); \
		return n == W(T) ? 0 : n + 1; \
	} \
	unsigned int stdc_first_trailing_one_##S(T x) { return x ? stdc_trailing_zeros_##S(x) + 1 : 0; } \
	unsigned int stdc_count_ones_##S(T x) { return __builtin_popcountll(x); } \
	unsigned int stdc_count_zeros_##S(T x) { return W(T) - __builtin_popcountll(x); } \
	_Bool stdc_has_single_bit_##S(T x) { return __builtin_popcountll(x) == 1; } \
	unsigned int stdc_bit_width_##S(T x) { return W(T) - stdc_leading_zeros_##S(x); } \
	T stdc_bit_floor_##S(T x) { return x ? (T)((T)1 << (stdc_bit_width_##S(x) - 1)) : 0; } \
	T stdc_bit_ceil_##S(T x) { \
		if (x <= 1) \
			return 1; \
		unsigned n = stdc_bit_width_##S((T)(x - 1)); \
		return n >= W(T) ? 0 : (T)((T)1 << n); \
	}

FUNCS(unsigned char, uc)
FUNCS(unsigned short, us)
FUNCS(unsigned int, ui)
FUNCS(unsigned long, ul)
FUNCS(unsigned long long, ull)
