// The C library: every libm function in float, double and long double on a
// grid of ordinary and special arguments, printed bit-exactly; rounding
// modes and floating-point exceptions
#include <complex.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

static const double args[] = {0.0, -0.0, 0.5, -0.5, 1.0, -1.0, 2.0, 0.75, 1.5, 3.0, 10.0, -7.25, 100.5, 1e-10, 1e-300, 1e10,
	700.0, -745.0, 1e300, 3.141592653589793, 1.5707963267948966, 0.1, 123.456, 5e-324, INFINITY, -INFINITY, NAN, 0.9999999,
	1.0000001, 22.5, -3.5, 2.5, 1e22, 6.02e23};
#define N (sizeof args / sizeof *args)

static void pd(double x) { x != x ? printf(" nan") : printf(" %a", x); }
static void pf(float x) { x != x ? printf(" nan") : printf(" %a", (double)x); }
static void pl(long double x) { x != x ? printf(" nan") : printf(" %La", x); }

#define UNARY(F) \
	do { \
		printf(#F ":"); \
		for (size_t i = 0; i < N; i++) \
			pd(F(args[i])); \
		printf("\n" #F "f:"); \
		for (size_t i = 0; i < N; i++) \
			pf(F##f((float)args[i])); \
		printf("\n" #F "l:"); \
		for (size_t i = 0; i < N; i++) \
			pl(F##l((long double)args[i] * 1.0000000000000000001L)); \
		printf("\n"); \
	} while (0)

#define BINARY(F) \
	do { \
		printf(#F ":"); \
		for (size_t i = 0; i < N; i += 2) \
			for (size_t j = 1; j < N; j += 3) \
				pd(F(args[i], args[j])); \
		printf("\n" #F "f:"); \
		for (size_t i = 0; i < N; i += 2) \
			for (size_t j = 1; j < N; j += 3) \
				pf(F##f((float)args[i], (float)args[j])); \
		printf("\n" #F "l:"); \
		for (size_t i = 0; i < N; i += 2) \
			for (size_t j = 1; j < N; j += 3) \
				pl(F##l((long double)args[i], (long double)args[j])); \
		printf("\n"); \
	} while (0)

static void unary(void) {
	UNARY(sin);
	UNARY(cos);
	UNARY(tan);
	UNARY(asin);
	UNARY(acos);
	UNARY(atan);
	UNARY(sinh);
	UNARY(cosh);
	UNARY(tanh);
	UNARY(asinh);
	UNARY(acosh);
	UNARY(atanh);
	UNARY(exp);
	UNARY(exp2);
	UNARY(expm1);
	UNARY(log);
	UNARY(log2);
	UNARY(log10);
	UNARY(log1p);
	UNARY(logb);
	UNARY(sqrt);
	UNARY(cbrt);
	UNARY(erf);
	UNARY(erfc);
	UNARY(tgamma);
	UNARY(lgamma);
	UNARY(floor);
	UNARY(ceil);
	UNARY(round);
	UNARY(trunc);
	UNARY(rint);
	UNARY(nearbyint);
	UNARY(fabs);
}

static void binary(void) {
	BINARY(pow);
	BINARY(atan2);
	BINARY(hypot);
	BINARY(fmod);
	BINARY(remainder);
	BINARY(copysign);
	BINARY(fmin);
	BINARY(fmax);
	BINARY(fdim);
	BINARY(nextafter);
}

static void others(void) {
	for (size_t i = 0; i < N; i++) {
		int e, q;
		double ip;
		printf("%zu:", i);
		pd(frexp(args[i], &e));
		printf(" %d", e);
		pd(modf(args[i], &ip));
		pd(ip);
		pd(ldexp(args[i], 7));
		pd(scalbn(args[i], -1070));
		pd(remquo(args[i], 0.75, &q));
		printf(" %d", q & 7);
		pd(fma(args[i], 1.25, -0.5));
		pl(fmal(args[i], 3.0L, 1e-20L));
		if (fabs(args[i]) < 1e18)
			printf(" %ld %ld %lld", lround(args[i]), lrint(args[i]), llround(args[i]));
		printf(" %d %d %d %d %d", ilogb(args[i] == 0 ? 1 : args[i]), fpclassify(args[i]), isnormal(args[i]),
			signbit(args[i]) != 0, isinf(args[i]));
		pd(nexttoward(args[i], 0.0L));
		printf("\n");
	}
	int s;
	printf("lgamma_r");
	for (size_t i = 0; i < N; i++) {
		pd(lgamma_r(args[i], &s));
		printf(" %d", s);
	}
	printf("\n");
	printf("%a %a %a\n", nan(""), (double)nanf("0x1"), (double)nanl(""));
}

static void rounding(void) {
	const int modes[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
	const char *names[] = {"nearest", "up", "down", "zero"};
	for (int m = 0; m < 4; m++) {
		fesetround(modes[m]);
		/* volatile: a compiler may evaluate calls with constant arguments
		   assuming the default rounding (gcc does without -frounding-math) */
		volatile double a = 1.0, b = 3.0, h = 2.5, mh = -2.5, h3 = 3.5, l5 = 1.5, two = 2.0, e = 1.0, f2 = 2.0005, f5 = 0.5;
		volatile float fa = 2.0f, fb = 7.0f;
		volatile long double l2 = 2.0L;
		printf("%s %d:", names[m], fegetround() == modes[m]);
		pd(a / b);
		pf(fa / fb);
		pd(rint(h));
		pd(rint(mh));
		pd(nearbyint(h3));
		printf(" %ld %ld", lrint(-l5), lrint(l5));
		pd(sqrt(two));
		pd(exp(e));
		pd(sin(e));
		pl(sqrtl(l2));
		char buf[64];
		snprintf(buf, sizeof buf, "%.3f %.0f %.0f", f2, f5, l5);
		printf(" [%s]\n", buf);
	}
	fesetround(FE_TONEAREST);
	feclearexcept(FE_ALL_EXCEPT);
	volatile double z = 0.0, one = 1.0, big = 1e308, tiny = 1e-308;
	volatile double r = one / z;
	printf("divbyzero %d", fetestexcept(FE_DIVBYZERO) != 0);
	feclearexcept(FE_ALL_EXCEPT);
	r = big * big;
	printf(" overflow %d", fetestexcept(FE_OVERFLOW) != 0);
	feclearexcept(FE_ALL_EXCEPT);
	r = tiny * tiny;
	printf(" underflow %d", fetestexcept(FE_UNDERFLOW) != 0);
	feclearexcept(FE_ALL_EXCEPT);
	r = z / z;
	printf(" invalid %d", fetestexcept(FE_INVALID) != 0);
	feclearexcept(FE_ALL_EXCEPT);
	r = one / 3;
	printf(" inexact %d", fetestexcept(FE_INEXACT) != 0);
	feclearexcept(FE_ALL_EXCEPT);
	r = sqrt(-one);
	printf(" sqrt-invalid %d", fetestexcept(FE_INVALID) != 0);
	feclearexcept(FE_ALL_EXCEPT);
	r = log(z);
	printf(" log0 %d\n", fetestexcept(FE_DIVBYZERO) != 0);
	(void)r;
}

static void complexes(void) {
	double complex zs[] = {1 + 2 * I, -0.5 + 0.25 * I, 3 - 4 * I, 0, I, -2, 1e-5 + 1e5 * I};
	for (size_t i = 0; i < sizeof zs / sizeof *zs; i++) {
		double complex z = zs[i], r[] = {csqrt(z), cexp(z), clog(z + 1), csin(z), ccos(z), ctan(z), casin(z), cacos(z),
			catan(z), csinh(z), ccosh(z), ctanh(z), cpow(z, 0.5 + I), cproj(z)};
		printf("%zu: %a", i, cabs(z));
		pd(carg(z));
		for (size_t k = 0; k < sizeof r / sizeof *r; k++) {
			pd(creal(r[k]));
			pd(cimag(r[k]));
		}
		long double complex lz = z, lr = csqrtl(lz) + cexpl(lz) * clogl(lz + 2);
		float complex fz = z, fr = csqrtf(fz) + cexpf(fz) * clogf(fz + 2);
		pl(creall(lr));
		pl(cimagl(lr));
		pf(crealf(fr));
		pf(cimagf(fr));
		printf("\n");
	}
}

int main(void) {
	unary();
	binary();
	others();
	rounding();
	complexes();
	return 0;
}
