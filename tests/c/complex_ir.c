// float and double _Complex in the IR backend: values in registers, the
// calling convention (registers and stack), memory, conversions
#include <complex.h>
#include <stdarg.h>
#include <stdio.h>

typedef double _Complex cd;
typedef float _Complex cf;

static void show(const char *what, cd z) { printf("%s %g %g\n", what, creal(z), cimag(z)); }
static void showf(const char *what, cf z) { printf("%s %g %g\n", what, (double)crealf(z), (double)cimagf(z)); }

__attribute__((noinline)) static cd addd(cd a, cd b) { return a + b; }
__attribute__((noinline)) static cf addf(cf a, cf b) { return a + b; }
__attribute__((noinline)) static cd muld(cd a, cd b) { return a * b; }
__attribute__((noinline)) static cf divf(cf a, cf b) { return a / b; }
static cd scale(cd a, double k) { return a * k; } // inlined

// more complex arguments than registers: the rest go on the stack
__attribute__((noinline)) static cd many(cd a, cd b, cd c, cd d, cd e, cf f, cd g, double h, cf i) {
	return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + h + 9 * i;
}

__attribute__((noinline)) static cf manyf(cf a, cf b, cf c, cf d, cf e, cf f, cf g, cf h, cf i, cf j, int k) {
	return a - b + c - d + e - f + g - h + i - j + k;
}

// variadic complex arguments
static cd sum(int n, ...) {
	va_list ap;
	va_start(ap, n);
	cd s = 0;
	for (int i = 0; i < n; i++) {
		s += va_arg(ap, cd);
		s += va_arg(ap, double);
	}
	va_end(ap);
	return s;
}

static cf sumf(int n, ...) {
	va_list ap;
	va_start(ap, n);
	cf s = 0;
	for (int i = 0; i < n; i++)
		s += va_arg(ap, cf);
	va_end(ap);
	return s;
}

struct pt {
	int tag;
	cf f;
	cd d;
};

__attribute__((noinline)) static cd fromStruct(struct pt *p) { return p->d * p->f + p->tag; }

static cd garr[4] = {1, 2 * I, 3 + 3 * I, -1};
static volatile cd vz = 5 + 6 * I;

// a loop with complex accumulators: z = z^2 + c
__attribute__((noinline)) static int mandel(cd c) {
	cd z = 0;
	int n = 0;
	while (n < 100 && creal(z) * creal(z) + cimag(z) * cimag(z) <= 4) {
		z = z * z + c;
		n++;
	}
	return n;
}

__attribute__((noinline)) static cd poly(cd x) {
	cd r = 1;
	for (int i = 0; i < 5; i++)
		r = r * x + i;
	return r;
}

__attribute__((noinline)) static cd parts(cd z) {
	__real__ z = __imag__ z * 2;
	__imag__ z += 1;
	return z;
}

__attribute__((noinline)) static int truth(cd z, cf w) {
	int r = 0;
	if (z)
		r |= 1;
	if (!w)
		r |= 2;
	r |= (z == w) << 2;
	r |= (z != 0) << 3;
	return r && z ? r : -r;
}

__attribute__((noinline)) static long conv(cd z) {
	int i = z;      // the real part
	_Bool b = z;    // both parts
	float f = z;
	long double l = z;
	return i * 1000 + b * 100 + (long)(f * 10) + (long)l;
}

__attribute__((noinline)) static cd pick(int c, cd a, cf b) { return c > 0 ? a : c < 0 ? b : 1.5; }

static cd (*fp)(cd, cd) = addd;

// long double _Complex: parts in memory, returned in st(0) and st(1)
typedef long double _Complex cl;

static void showl(const char *what, cl z) { printf("%s %.12Lg %.12Lg\n", what, creall(z), cimagl(z)); }

__attribute__((noinline)) static cl addl(cl a, cl b) { return a + b; }
__attribute__((noinline)) static cl mull(cl a, cl b) { return a * b; }
__attribute__((noinline)) static cl divl(cl a, cl b) { return a / b; }
__attribute__((noinline)) static cl swapl(cl z) {
	z = __builtin_complex(__imag__ z, __real__ z);
	return z;
}
__attribute__((noinline)) static cl mixl(int n, cl a, double x, cd b, cf c, cl d, long double e) {
	cl s = 0;
	for (int i = 0; i < n; i++)
		s = s * a + x + b - c + d * e;
	return n > 2 ? s : conjl(s);
}
static cl suml(int n, ...) {
	va_list ap;
	va_start(ap, n);
	cl s = 0;
	for (int i = 0; i < n; i++)
		s += va_arg(ap, cl);
	va_end(ap);
	return s;
}
__attribute__((noinline)) static int truthl(cl z) { return !z + 2 * (z != 1) + 4 * (z ? 1 : 0) + 8 * ((z ?: 1) == 1); }
static cl gl = 1.25L + 2.5Li;

int main(void) {
	cd a = 1 + 2 * I, b = 3 - 4 * I;
	cf x = 1.5f + 2.5f * I, y = -0.5f + 4 * I;
	show("add", addd(a, b));
	showf("addf", addf(x, y));
	show("mul", muld(a, b));
	showf("divf", divf(x, y));
	show("scale", scale(a, 3));
	show("many", many(a, b, a, b, a, x, b, 0.25, y));
	showf("manyf", manyf(x, y, x, y, x, y, x, y, x, 3 * y, 7));
	show("sum", sum(3, a, 1.0, b, 2.0, a * b, 3.0));
	showf("sumf", sumf(4, x, y, x, y));
	struct pt p = {3, x, b};
	show("struct", fromStruct(&p));
	cd t = 0;
	for (int i = 0; i < 4; i++)
		t += garr[i] * (i + 1);
	show("arr", t);
	cd v = vz;
	vz = v * I;
	show("volatile", vz);
	printf("mandel %d %d %d\n", mandel(0.25 + 0.5 * I), mandel(-1), mandel(1 + I));
	show("poly", poly(0.5 - I));
	show("parts", parts(a));
	printf("truth %d %d %d\n", truth(a, 0), truth(0, x), truth(1.5f + 2.5f * I, x));
	printf("conv %ld %ld\n", conv(3.75 + I), conv(-2.5 * I));
	show("pick", pick(1, a, x) + pick(-1, a, x) + pick(0, a, x));
	show("fp", fp(a, b));
	cd *pz = &garr[2];
	*pz *= 2;
	__imag__ *pz = 9;
	show("ptr", garr[2]);
	cf w = a;
	w /= 2;
	showf("narrow", w);
	cl la = 1.5L + 0.25Li, lb = -3 + 4.5Li;
	showl("addl", addl(la, lb));
	showl("mull", mull(la, lb));
	showl("divl", divl(la, lb));
	showl("swapl", swapl(la));
	showl("mixl", mixl(3, la, 0.5, b, x, lb, 2) + mixl(2, la, 1, a, y, gl, -1));
	showl("suml", suml(3, la, lb, gl));
	printf("truthl %d %d %d\n", truthl(0), truthl(1), truthl(la));
	cd fromL = la * 2;
	cl toL = a;
	long double re = lb;
	show("fromL", fromL);
	showl("toL", toL + re);
	struct { char c; cl z; } sl = {1, la};
	sl.z *= sl.z;
	__real__ sl.z += 1;
	showl("structl", sl.z);
	cd c = conj(a) * cexp(I * 0.5) + csqrt(b);
	printf("libm %.6f %.6f\n", creal(c), cimag(c));
	return 0;
}
