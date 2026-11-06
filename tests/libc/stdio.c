// The C library: formatted output and input, number conversions, streams
#define _GNU_SOURCE
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static const double dvals[] = {0.0, -0.0, 1.0, -1.0, 0.1, 0.5, 1.5, 2.5, 3.14159265358979, 1e-300, 1e300, 123456789.0,
	0.000123456, 9.999999, 99.5, 1e15, 1e16, 1e22, 1e23, 5e-324, 2.2250738585072014e-308, DBL_MAX, 1.0 / 3,
	2.0 / 3, 0.30000000000000004, 1e21, 999999.5, 0.05, 0.15, 0.25, 0.35, 1234.5678};
static const char *dfmts[] = {"%f", "%.0f", "%.1f", "%.3f", "%.17f", "%e", "%.0e", "%.3e", "%.17e", "%g", "%.0g", "%.3g",
	"%.17g", "%a", "%.0a", "%.3a", "%A", "%#g", "%#.0f", "%#x", "%+f", "% e", "%-12.4g|", "%012.3f", "%+.2e", "%#.3g"};

static void doubles(void) {
	for (size_t f = 0; f < sizeof dfmts / sizeof *dfmts; f++) {
		if (strchr(dfmts[f], 'x'))
			continue;
		printf("%s:", dfmts[f]);
		for (size_t i = 0; i < sizeof dvals / sizeof *dvals; i++) {
			char b[512];
			int n = snprintf(b, sizeof b, dfmts[f], dvals[i]);
			printf(" %d:%s", n, b);
		}
		putchar('\n');
	}
	printf("%f %f %e %g %a %F %E %G %A\n", INFINITY, -INFINITY, NAN, -NAN, INFINITY, NAN, -INFINITY, INFINITY, NAN);
	printf("%10.3f|%-10.3f|%+10.3f|%010.3f\n", -3.25, -3.25, 3.25, -3.25);
	printf("%*.*f %.*e\n", 12, 4, 2.718281828, 2, 1e-5);
}

static void longdoubles(void) {
	long double v[] = {0.0L, 1.0L / 3, 1e4000L, -1e-4000L, LDBL_MAX, LDBL_MIN, LDBL_EPSILON, 3.141592653589793238462643L,
		123456789012345678.0L, 0.1L};
	for (size_t i = 0; i < sizeof v / sizeof *v; i++)
		printf("%Lf %.20Le %Lg %.25Lg %La\n", v[i], v[i], v[i], v[i], v[i]);
}

static void integers(void) {
	long long iv[] = {0, 1, -1, 42, -42, 127, 255, 32767, -32768, 65535, INT_MAX, INT_MIN, UINT_MAX, LLONG_MAX, LLONG_MIN,
		1000000007, 0x7f00ff00ff00ffLL};
	const char *fmts[] = {"%d", "%5d", "%-5d|", "%05d", "%+d", "% d", "%x", "%#x", "%X", "%#o", "%o", "%u", "%.3d", "%.0d",
		"%8.4x", "%#.0o", "%hhd", "%hd", "%hhx"};
	for (size_t f = 0; f < sizeof fmts / sizeof *fmts; f++) {
		printf("%s:", fmts[f]);
		for (size_t i = 0; i < sizeof iv / sizeof *iv; i++)
			printf(fmts[f], (int)iv[i]), putchar(' ');
		putchar('\n');
	}
	for (size_t i = 0; i < sizeof iv / sizeof *iv; i++)
		printf("%lld %llx %llo %llu %" PRId64 " %jd %zu %td\n", iv[i], iv[i], iv[i], (unsigned long long)iv[i],
			(int64_t)iv[i], (intmax_t)iv[i], (size_t)iv[i], (ptrdiff_t)iv[i]);
	printf("%c%c%c %5c|%-3c| %%\n", 'a', 'b', 'c', 'x', 'y');
	printf("%s|%10s|%-10s|%.2s|%*s|%-*.*s|\n", "str", "right", "left", "truncate", 6, "w", 6, 2, "prec");
	printf("%p %p\n", (void *)0, (void *)0x1234);
	int n1, n2;
	printf("abc%ndef%n\n", &n1, &n2);
	printf("%d %d\n", n1, n2);
	printf("%ls %lc %5ls|\n", L"wide", (wint_t)L'w', L"ab");
	printf("%1$d %2$s %1$x\n", 255, "pos");
}

static void scanning(void) {
	const char *in[] = {"42", "-17 abc", "0x1f", "077", "3.25e2", "  -0.5", "inf", "nan", "1e400", "0x1.8p1", "12abc",
		"abc", "", "9223372036854775807", "18446744073709551615", "-9223372036854775809", "1.7976931348623157e308",
		"2.4703282292062328e-324", "0.1e-1", "1,234", "+5", "- 5", "1e", "1e+", ".5", "5.", "0x", "0xg"};
	for (size_t i = 0; i < sizeof in / sizeof *in; i++) {
		int d = -1, k;
		long long ll = -1;
		unsigned long long ull = 0;
		double x = -1;
		float fl = -1;
		char s[16] = "-", c = '-';
		int r1 = sscanf(in[i], "%d", &d);
		int r2 = sscanf(in[i], "%lli%n", &ll, &k);
		int r3 = sscanf(in[i], "%lg", &x);
		int r4 = sscanf(in[i], "%f %15s", &fl, s);
		int r5 = sscanf(in[i], "%llu %c", &ull, &c);
		printf("[%s] %d:%d %d:%lld %d:%a %d:%a:%s %d:%llu:%c\n", in[i], r1, d, r2, ll, r3, x, r4, (double)fl, s, r5, ull, c);
		char *e;
		errno = 0;
		double sd = strtod(in[i], &e);
		int de = errno;
		errno = 0;
		long sl = strtol(in[i], &e, 0);
		int le = errno;
		errno = 0;
		unsigned long su = strtoul(in[i], &e, 10);
		int ue = errno;
		float sf = strtof(in[i], 0);
		long double sld = strtold(in[i], 0);
		printf("  strtod %a %d strtol %ld %d strtoul %lu %d rest [%s] strtof %a strtold %La\n", sd, de, sl, le, su, ue, e,
			(double)sf, sld);
	}
	char a[10], b[10];
	int x, y;
	printf("%d ", sscanf("key=value;", "%9[^=]=%9[^;];", a, b));
	printf("%s %s\n", a, b);
	printf("%d ", sscanf("12:34", "%d:%d", &x, &y));
	printf("%d %d\n", x, y);
	printf("%d\n", sscanf("  x", " %c", a));
	// decimal conversions on halfway cases and long inputs
	const char *dec[] = {"2.5", "0.1000000000000000055511151231257827", "9007199254740993", "9007199254740993.0000001",
		"1.00000000000000011102230246251565404236316680908203125", "7.2057594037927933e16",
		"123456789012345678901234567890e-10", "4.9406564584124654e-324", "2.4703282292062327e-324",
		"1.4012984643248170709e-45", "3.4028235677973366e38"};
	for (size_t i = 0; i < sizeof dec / sizeof *dec; i++)
		printf("%s -> %a %a %La\n", dec[i], strtod(dec[i], 0), (double)strtof(dec[i], 0), strtold(dec[i], 0));
}

static void streams(void) {
	char *buf = 0;
	size_t len = 0;
	FILE *f = open_memstream(&buf, &len);
	for (int i = 0; i < 1000; i++)
		fprintf(f, "%d,", i * i);
	fclose(f);
	unsigned h = 0;
	for (size_t i = 0; i < len; i++)
		h = h * 31 + (unsigned char)buf[i];
	printf("memstream %zu %08x\n", len, h);
	free(buf);
	char mem[64] = "line one\nline two\nlast";
	f = fmemopen(mem, strlen(mem), "r");
	char *line = 0;
	size_t cap = 0;
	ssize_t n;
	while ((n = getline(&line, &cap, f)) > 0)
		printf("getline %zd [%s]\n", n, line);
	free(line);
	fclose(f);
	f = tmpfile();
	for (int i = 0; i < 300; i++)
		fwrite(&i, sizeof i, 1, f);
	fseek(f, 400, SEEK_SET);
	int v;
	fread(&v, sizeof v, 1, f);
	printf("tmpfile %d %ld\n", v, ftell(f));
	rewind(f);
	int cnt = 0;
	while (fread(&v, sizeof v, 1, f) == 1)
		cnt++;
	printf("count %d eof %d\n", cnt, feof(f));
	fclose(f);
	char s[8];
	printf("snprintf %d [%s]\n", snprintf(s, sizeof s, "%s", "truncated text"), s);
	wchar_t ws[32];
	printf("swprintf %d %ls\n", swprintf(ws, 32, L"%d-%ls-%.2f", 7, L"wide", 1.25), ws);
}

int main(void) {
	doubles();
	longdoubles();
	integers();
	scanning();
	streams();
	return 0;
}
