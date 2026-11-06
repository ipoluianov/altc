/* AltC <stdbit.h> - C23 bit and byte utilities (the functions are in the
   built-in C library, libc/altc/stdbit.c; the type-generic macros need
   _Generic, i.e. C11 or later).  */

#ifndef _STDBIT_H
#define _STDBIT_H 1

#include <stdbool.h>

#define __STDC_VERSION_STDBIT_H__ 202311L

#define __STDC_ENDIAN_LITTLE__ 1234
#define __STDC_ENDIAN_BIG__ 4321
#define __STDC_ENDIAN_NATIVE__ __STDC_ENDIAN_LITTLE__

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define __altc_stdbit_bool _Bool
#else
# define __altc_stdbit_bool unsigned char
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int stdc_leading_zeros_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_zeros_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_zeros_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_zeros_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_zeros_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_ones_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_ones_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_ones_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_ones_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_leading_ones_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_zeros_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_zeros_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_zeros_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_zeros_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_zeros_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_ones_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_ones_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_ones_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_ones_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_trailing_ones_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_zero_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_zero_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_zero_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_zero_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_zero_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_one_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_one_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_one_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_one_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_leading_one_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_zero_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_zero_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_zero_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_zero_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_zero_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_one_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_one_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_one_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_one_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_first_trailing_one_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_count_zeros_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_count_zeros_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_count_zeros_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_count_zeros_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_count_zeros_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_count_ones_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_count_ones_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_count_ones_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_count_ones_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_count_ones_ull (unsigned long long int __x) __attribute__ ((__const__));
extern __altc_stdbit_bool stdc_has_single_bit_uc (unsigned char __x) __attribute__ ((__const__));
extern __altc_stdbit_bool stdc_has_single_bit_us (unsigned short __x) __attribute__ ((__const__));
extern __altc_stdbit_bool stdc_has_single_bit_ui (unsigned int __x) __attribute__ ((__const__));
extern __altc_stdbit_bool stdc_has_single_bit_ul (unsigned long int __x) __attribute__ ((__const__));
extern __altc_stdbit_bool stdc_has_single_bit_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_width_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_width_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_width_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_width_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_width_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned char stdc_bit_floor_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned short stdc_bit_floor_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_floor_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned long int stdc_bit_floor_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned long long int stdc_bit_floor_ull (unsigned long long int __x) __attribute__ ((__const__));
extern unsigned char stdc_bit_ceil_uc (unsigned char __x) __attribute__ ((__const__));
extern unsigned short stdc_bit_ceil_us (unsigned short __x) __attribute__ ((__const__));
extern unsigned int stdc_bit_ceil_ui (unsigned int __x) __attribute__ ((__const__));
extern unsigned long int stdc_bit_ceil_ul (unsigned long int __x) __attribute__ ((__const__));
extern unsigned long long int stdc_bit_ceil_ull (unsigned long long int __x) __attribute__ ((__const__));

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
#define stdc_leading_zeros(x) \
  _Generic ((x), \
            unsigned char: stdc_leading_zeros_uc, \
            unsigned short: stdc_leading_zeros_us, \
            unsigned int: stdc_leading_zeros_ui, \
            unsigned long int: stdc_leading_zeros_ul, \
            unsigned long long int: stdc_leading_zeros_ull) (x)
#define stdc_leading_ones(x) \
  _Generic ((x), \
            unsigned char: stdc_leading_ones_uc, \
            unsigned short: stdc_leading_ones_us, \
            unsigned int: stdc_leading_ones_ui, \
            unsigned long int: stdc_leading_ones_ul, \
            unsigned long long int: stdc_leading_ones_ull) (x)
#define stdc_trailing_zeros(x) \
  _Generic ((x), \
            unsigned char: stdc_trailing_zeros_uc, \
            unsigned short: stdc_trailing_zeros_us, \
            unsigned int: stdc_trailing_zeros_ui, \
            unsigned long int: stdc_trailing_zeros_ul, \
            unsigned long long int: stdc_trailing_zeros_ull) (x)
#define stdc_trailing_ones(x) \
  _Generic ((x), \
            unsigned char: stdc_trailing_ones_uc, \
            unsigned short: stdc_trailing_ones_us, \
            unsigned int: stdc_trailing_ones_ui, \
            unsigned long int: stdc_trailing_ones_ul, \
            unsigned long long int: stdc_trailing_ones_ull) (x)
#define stdc_first_leading_zero(x) \
  _Generic ((x), \
            unsigned char: stdc_first_leading_zero_uc, \
            unsigned short: stdc_first_leading_zero_us, \
            unsigned int: stdc_first_leading_zero_ui, \
            unsigned long int: stdc_first_leading_zero_ul, \
            unsigned long long int: stdc_first_leading_zero_ull) (x)
#define stdc_first_leading_one(x) \
  _Generic ((x), \
            unsigned char: stdc_first_leading_one_uc, \
            unsigned short: stdc_first_leading_one_us, \
            unsigned int: stdc_first_leading_one_ui, \
            unsigned long int: stdc_first_leading_one_ul, \
            unsigned long long int: stdc_first_leading_one_ull) (x)
#define stdc_first_trailing_zero(x) \
  _Generic ((x), \
            unsigned char: stdc_first_trailing_zero_uc, \
            unsigned short: stdc_first_trailing_zero_us, \
            unsigned int: stdc_first_trailing_zero_ui, \
            unsigned long int: stdc_first_trailing_zero_ul, \
            unsigned long long int: stdc_first_trailing_zero_ull) (x)
#define stdc_first_trailing_one(x) \
  _Generic ((x), \
            unsigned char: stdc_first_trailing_one_uc, \
            unsigned short: stdc_first_trailing_one_us, \
            unsigned int: stdc_first_trailing_one_ui, \
            unsigned long int: stdc_first_trailing_one_ul, \
            unsigned long long int: stdc_first_trailing_one_ull) (x)
#define stdc_count_zeros(x) \
  _Generic ((x), \
            unsigned char: stdc_count_zeros_uc, \
            unsigned short: stdc_count_zeros_us, \
            unsigned int: stdc_count_zeros_ui, \
            unsigned long int: stdc_count_zeros_ul, \
            unsigned long long int: stdc_count_zeros_ull) (x)
#define stdc_count_ones(x) \
  _Generic ((x), \
            unsigned char: stdc_count_ones_uc, \
            unsigned short: stdc_count_ones_us, \
            unsigned int: stdc_count_ones_ui, \
            unsigned long int: stdc_count_ones_ul, \
            unsigned long long int: stdc_count_ones_ull) (x)
#define stdc_has_single_bit(x) \
  _Generic ((x), \
            unsigned char: stdc_has_single_bit_uc, \
            unsigned short: stdc_has_single_bit_us, \
            unsigned int: stdc_has_single_bit_ui, \
            unsigned long int: stdc_has_single_bit_ul, \
            unsigned long long int: stdc_has_single_bit_ull) (x)
#define stdc_bit_width(x) \
  _Generic ((x), \
            unsigned char: stdc_bit_width_uc, \
            unsigned short: stdc_bit_width_us, \
            unsigned int: stdc_bit_width_ui, \
            unsigned long int: stdc_bit_width_ul, \
            unsigned long long int: stdc_bit_width_ull) (x)
#define stdc_bit_floor(x) \
  _Generic ((x), \
            unsigned char: stdc_bit_floor_uc, \
            unsigned short: stdc_bit_floor_us, \
            unsigned int: stdc_bit_floor_ui, \
            unsigned long int: stdc_bit_floor_ul, \
            unsigned long long int: stdc_bit_floor_ull) (x)
#define stdc_bit_ceil(x) \
  _Generic ((x), \
            unsigned char: stdc_bit_ceil_uc, \
            unsigned short: stdc_bit_ceil_us, \
            unsigned int: stdc_bit_ceil_ui, \
            unsigned long int: stdc_bit_ceil_ul, \
            unsigned long long int: stdc_bit_ceil_ull) (x)
#endif

#ifdef __cplusplus
}
#endif

#endif /* stdbit.h */
