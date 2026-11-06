// The C library: memory and string functions over all alignments and
// small lengths, wide and multibyte strings, conversions
#define _GNU_SOURCE
#include <ctype.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include <wctype.h>

static unsigned hash;
static void mix(uintptr_t v) { hash = hash * 2654435761u + (unsigned)v + 1; }
static void mixbuf(const unsigned char *p, size_t n) {
	for (size_t i = 0; i < n; i++)
		mix(p[i]);
}

static void memory(void) {
	static unsigned char src[256], dst[256];
	for (int i = 0; i < 256; i++)
		src[i] = (unsigned char)(i * 37 + 11);
	for (int sa = 0; sa < 16; sa++)
		for (int da = 0; da < 16; da++)
			for (size_t n = 0; n < 80; n += (n < 20 ? 1 : 7)) {
				memset(dst, 0xaa, sizeof dst);
				memcpy(dst + da, src + sa, n);
				mixbuf(dst, 128);
				memmove(dst + da + 3, dst + da, n);
				mixbuf(dst, 128);
				memmove(dst + da, dst + da + 2, n);
				mixbuf(dst, 128);
				memset(dst + sa, sa * 7, n);
				mixbuf(dst, 128);
				int c = memcmp(dst + da, src + sa, n);
				mix(c < 0 ? 1 : c > 0 ? 2 : 0);
				mix((uintptr_t)((unsigned char *)memchr(src + sa, src[sa + n / 2], n + 1) - src));
			}
	printf("memory %08x\n", hash);
}

static void strings(void) {
	static char buf[256], b2[256];
	const char *words[] = {"", "a", "ab", "hello", "hello world", "The quick brown fox jumps over the lazy dog",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxy", "\x80\xff high bytes"};
	for (size_t w = 0; w < sizeof words / sizeof *words; w++)
		for (int a = 0; a < 16; a++) {
			strcpy(buf + a, words[w]);
			mix(strlen(buf + a));
			mix(strnlen(buf + a, 5));
			mix((uintptr_t)(strchr(buf + a, 'o') ? strchr(buf + a, 'o') - buf : 0));
			mix((uintptr_t)(strrchr(buf + a, 'o') ? strrchr(buf + a, 'o') - buf : 0));
			mix((uintptr_t)(strchrnul(buf + a, 'z') - buf));
			mix((uintptr_t)(strstr(buf + a, "ow") ? strstr(buf + a, "ow") - buf : 0));
			mix(strspn(buf + a, "helo "));
			mix(strcspn(buf + a, "qwz"));
			mix((uintptr_t)(strpbrk(buf + a, "kd") ? strpbrk(buf + a, "kd") - buf : 0));
			int c = strcmp(buf + a, "hello");
			mix(c < 0 ? 1 : c > 0 ? 2 : 0);
			c = strncmp(buf + a, "hello there", 7);
			mix(c < 0 ? 1 : c > 0 ? 2 : 0);
			c = strcasecmp(buf + a, "HELLO");
			mix(c < 0 ? 1 : c > 0 ? 2 : 0);
			strcpy(b2, "prefix:");
			strncat(b2, buf + a, 9);
			mixbuf((unsigned char *)b2, strlen(b2));
			mix(strlcpy(b2, buf + a, 6));
			mixbuf((unsigned char *)b2, 6);
			memset(b2, 'q', 32);
			strncpy(b2, buf + a, 20);
			mixbuf((unsigned char *)b2, 32);
			mix((uintptr_t)(stpcpy(b2, buf + a) - b2));
			char *d = strdup(buf + a);
			mix(strcmp(d, buf + a) == 0);
			free(d);
		}
	printf("strings %08x\n", hash);
	char text[] = "a,b,,c;d e";
	for (char *t = strtok(text, ",; "); t; t = strtok(0, ",; "))
		printf("[%s]", t);
	char text2[] = "x:y::z", *save, *s2 = text2;
	for (char *t; (t = strsep(&s2, ":"));)
		printf("<%s>", t);
	(void)save;
	printf("\n%s|%s|%s\n", strerror(0), strerror(2), strerror(22));
	printf("%d %d %d %d\n", ffs(0), ffs(8), ffsl(1L << 40), ffsll(0));
	for (int c = 0; c < 256; c++)
		mix(isalpha(c) | isdigit(c) << 1 | isspace(c) << 2 | ispunct(c) << 3 | isupper(c) << 4 | isxdigit(c) << 5 |
			iscntrl(c) << 6 | isprint(c) << 7 | (toupper(c) << 8) | (tolower(c) << 16));
	printf("ctype %08x\n", hash);
}

static void wide(void) {
	setlocale(LC_ALL, "C.UTF-8");
	const char *mb = "h\xc3\xa9llo \xe2\x82\xac \xf0\x9f\x98\x80 \xd0\xbf\xd1\x80\xd0\xb8";
	wchar_t ws[64];
	size_t n = mbstowcs(ws, mb, 64);
	printf("mbstowcs %zu:", n);
	for (size_t i = 0; i < n; i++)
		printf(" %x", (unsigned)ws[i]);
	char back[128];
	printf("\nwcstombs %zu %s\n", wcstombs(back, ws, sizeof back), back);
	mbstate_t st = {0};
	const char *p = mb;
	size_t k;
	wchar_t wc;
	printf("mbrtowc");
	while ((k = mbrtowc(&wc, p, 8, &st)) != 0 && k != (size_t)-1 && k != (size_t)-2) {
		printf(" %zu:%x:%d", k, (unsigned)wc, wcwidth(wc));
		p += k;
	}
	printf("\n%zu %zu %zu\n", mbrtowc(&wc, "\xc3", 1, &st), mbrtowc(&wc, "\xff", 1, &st), mblen("\xe2\x82\xac", 3));
	printf("wcs %zu %d %ls %ls\n", wcslen(ws), wcscmp(ws, L"h"), wcschr(ws, L'l'), wcsstr(ws, L"\x20ac"));
	printf("towupper %x %x %x %d %d\n", (unsigned)towupper(L'\xe9'), (unsigned)towlower(L'\x41f'), (unsigned)towupper(L'z'),
		iswalpha(L'\x430') != 0, iswspace(L'\x3000') != 0);
	wchar_t *we;
	long wl = wcstol(L"  -123xyz", &we, 10);
	printf("wcstol %ld %ls wcstod %a\n", wl, we, wcstod(L"2.5e3", 0));
	char tbuf[16];
	printf("wctomb %d %d\n", wctomb(tbuf, 0x1f600), wctomb(tbuf, 0x110000));
	setlocale(LC_ALL, "C");
}

static void numbers(void) {
	const char *in[] = {"0", "123", "-123", "  +77", "0x7fffffff", "0777", "z", "zz", "1y2", "4294967296",
		"-2147483649", "9999999999999999999999"};
	for (size_t i = 0; i < sizeof in / sizeof *in; i++)
		for (int base = 0; base <= 36; base += (base < 2 ? 2 : base < 16 ? 6 : 10)) {
			char *e;
			long v = strtol(in[i], &e, base);
			unsigned long long u = strtoull(in[i], 0, base);
			mix((uintptr_t)v);
			mix((uintptr_t)u);
			mix((uintptr_t)(e - in[i]));
		}
	printf("strtol %08x %d %ld %lld\n", hash, atoi("  42abc"), atol("-9"), atoll("123456789012"));
	div_t d = div(-7, 2);
	ldiv_t ld = ldiv(7, -2);
	printf("div %d %d %ld %ld %d %ld\n", d.quot, d.rem, ld.quot, ld.rem, abs(-5), labs(-6L));
	char *g = gcvt(1234.5678, 6, (char[32]){0});
	printf("gcvt %s\n", g);
}

int main(void) {
	memory();
	strings();
	wide();
	numbers();
	return 0;
}
