/* Helpers of the differential test programs: they read requests from
   stdin, one per line, fields separated by tabs, and answer one line each;
   tests/stddiff_test.go compares the answers with Go's packages. */
#ifndef ALT_TESTS_DIFF_H
#define ALT_TESTS_DIFF_H

#include <alt/str.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the next line without its newline; NULL at the end */
static char *diff_line(void) {
	static char *line;
	static size_t cap;
	ssize_t n = getline(&line, &cap, stdin);
	if (n < 0)
		return 0;
	if (n && line[n - 1] == '\n')
		line[--n] = 0;
	return line;
}

/* splits line at tabs into at most max fields */
static int diff_fields(char *line, char **f, int max) {
	int n = 0;
	while (n < max) {
		f[n++] = line;
		char *t = strchr(line, '\t');
		if (!t || n == max)
			break;
		*t = 0;
		line = t + 1;
	}
	return n;
}

static int hexval(int c) { return c <= '9' ? c - '0' : (c | 32) - 'a' + 10; }

/* decodes hexadecimal in place; returns the length */
static size_t diff_unhex(char *s) {
	size_t n = strlen(s) / 2;
	for (size_t i = 0; i < n; i++)
		s[i] = (char)(hexval((unsigned char)s[2 * i]) << 4 | hexval((unsigned char)s[2 * i + 1]));
	return n;
}

static void diff_puthex(const char *p, size_t n) {
	for (size_t i = 0; i < n; i++)
		printf("%02x", (unsigned char)p[i]);
}

#endif
