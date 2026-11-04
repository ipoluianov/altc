/* C99 complex arithmetic <complex.h> (functions from libm) */
#ifndef _COMPLEX_H
#define _COMPLEX_H 1

#define complex _Complex
#define _Complex_I (__builtin_complex(0.0f, 1.0f))
#undef I
#define I _Complex_I

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define CMPLX(x, y) __builtin_complex((double)(x), (double)(y))
#define CMPLXF(x, y) __builtin_complex((float)(x), (float)(y))
#define CMPLXL(x, y) __builtin_complex((long double)(x), (long double)(y))
#endif

#define __ALTC_CDECL(T, S) \
	extern T _Complex cacos##S(T _Complex); extern T _Complex casin##S(T _Complex); \
	extern T _Complex catan##S(T _Complex); extern T _Complex ccos##S(T _Complex); \
	extern T _Complex csin##S(T _Complex); extern T _Complex ctan##S(T _Complex); \
	extern T _Complex cacosh##S(T _Complex); extern T _Complex casinh##S(T _Complex); \
	extern T _Complex catanh##S(T _Complex); extern T _Complex ccosh##S(T _Complex); \
	extern T _Complex csinh##S(T _Complex); extern T _Complex ctanh##S(T _Complex); \
	extern T _Complex cexp##S(T _Complex); extern T _Complex clog##S(T _Complex); \
	extern T cabs##S(T _Complex); extern T _Complex cpow##S(T _Complex, T _Complex); \
	extern T _Complex csqrt##S(T _Complex); extern T carg##S(T _Complex); \
	extern T cimag##S(T _Complex); extern T _Complex conj##S(T _Complex); \
	extern T _Complex cproj##S(T _Complex); extern T creal##S(T _Complex);

__ALTC_CDECL(double, )
__ALTC_CDECL(float, f)
__ALTC_CDECL(long double, l)
#undef __ALTC_CDECL

#if defined(_GNU_SOURCE)
extern double _Complex clog10(double _Complex);
extern float _Complex clog10f(float _Complex);
extern long double _Complex clog10l(long double _Complex);
#endif


#endif
