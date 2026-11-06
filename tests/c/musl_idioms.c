// Idioms of musl's sources and headers: an array initialized by adjacent
// string literals in braces, __extension__ before a parenthesized
// expression (complex.h's _Complex_I), <tgmath.h>
#include <complex.h>
#include <stdio.h>
#include <tgmath.h>

static const unsigned char protos[] = {"\000ip\0"
				       "\001icmp\0"
				       "\002igmp\0"};
static const char messages[] = {"No error\0"
				"No match\0"};

int main(void) {
	printf("%d %d %s %s %d\n", (int)sizeof protos, protos[5], (const char *)protos + 6, messages + 9, (int)sizeof messages);
	int a = (__extension__(1 + 2));
	int b = (__extension__ 4);
	int c = (__extension__(int) 5.5);
	double complex z = 1.5 + 2 * I;
	printf("%d %d %d %g %g\n", a, b, c, creal(z), cimag(z));
	float f = 2.0f;
	double d = 2.0;
	long double l = 2.0L;
	printf("%d %d %d\n", (int)sizeof sqrt(f), (int)sizeof sqrt(d), (int)sizeof sqrt(l));
	printf("%.6f %.6f %.6Lf\n", (double)sqrt(f), sqrt(d), sqrt(l));
	printf("%d %d\n", (int)sizeof sqrt(z), (int)sizeof pow(f, d));
	double complex r = sqrt(z);
	printf("%.6f %.6f %.6f\n", creal(r), cimag(r), (double)fabs(-f));
	return 0;
}
