#include <alt/errors.h>
#include <alt/str.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

alt_str alt_str_slice(alt_str s, size_t i, size_t j) {
	if (j > s.n)
		j = s.n;
	if (i > j)
		i = j;
	return (alt_str){s.p + i, j - i};
}

bool alt_str_eq(alt_str a, alt_str b) { return a.n == b.n && (a.n == 0 || memcmp(a.p, b.p, a.n) == 0); }

bool alt_str_eq_cstr(alt_str a, const char *b) { return alt_str_eq(a, alt_str_cstr(b)); }

static inline int lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

bool alt_str_eq_fold(alt_str a, alt_str b) {
	if (a.n != b.n)
		return false;
	for (size_t i = 0; i < a.n; i++)
		if (lower((unsigned char)a.p[i]) != lower((unsigned char)b.p[i]))
			return false;
	return true;
}

int alt_str_cmp(alt_str a, alt_str b) {
	size_t n = a.n < b.n ? a.n : b.n;
	int c = n ? memcmp(a.p, b.p, n) : 0;
	if (c)
		return c < 0 ? -1 : 1;
	return a.n < b.n ? -1 : a.n > b.n;
}

bool alt_str_has_prefix(alt_str s, alt_str prefix) { return s.n >= prefix.n && (prefix.n == 0 || memcmp(s.p, prefix.p, prefix.n) == 0); }

bool alt_str_has_suffix(alt_str s, alt_str suffix) {
	return s.n >= suffix.n && (suffix.n == 0 || memcmp(s.p + s.n - suffix.n, suffix.p, suffix.n) == 0);
}

bool alt_str_has_prefix_fold(alt_str s, alt_str prefix) { return s.n >= prefix.n && alt_str_eq_fold(alt_str_slice(s, 0, prefix.n), prefix); }

ssize_t alt_str_index_byte(alt_str s, int c) {
	if (s.n == 0)
		return -1;
	const char *q = memchr(s.p, c, s.n);
	return q ? q - s.p : -1;
}

ssize_t alt_str_last_index_byte(alt_str s, int c) {
	for (size_t i = s.n; i > 0; i--)
		if ((unsigned char)s.p[i - 1] == (unsigned char)c)
			return (ssize_t)i - 1;
	return -1;
}

ssize_t alt_str_index(alt_str s, alt_str sub) {
	if (sub.n == 0)
		return 0;
	if (sub.n > s.n)
		return -1;
	const char *end = s.p + s.n - sub.n + 1;
	for (const char *q = s.p; q < end;) {
		q = memchr(q, sub.p[0], end - q);
		if (!q)
			return -1;
		if (memcmp(q, sub.p, sub.n) == 0)
			return q - s.p;
		q++;
	}
	return -1;
}

ssize_t alt_str_last_index(alt_str s, alt_str sub) {
	if (sub.n == 0)
		return (ssize_t)s.n;
	if (sub.n > s.n)
		return -1;
	for (size_t i = s.n - sub.n + 1; i > 0; i--)
		if (memcmp(s.p + i - 1, sub.p, sub.n) == 0)
			return (ssize_t)i - 1;
	return -1;
}

ssize_t alt_str_index_any(alt_str s, const char *chars) {
	for (size_t i = 0; i < s.n; i++)
		if (s.p[i] && strchr(chars, s.p[i]))
			return (ssize_t)i;
	return -1;
}

bool alt_str_contains(alt_str s, alt_str sub) { return alt_str_index(s, sub) >= 0; }

size_t alt_str_count(alt_str s, alt_str sub) {
	if (sub.n == 0)
		return s.n + 1;
	size_t k = 0;
	ssize_t i;
	while ((i = alt_str_index(s, sub)) >= 0) {
		k++;
		s = alt_str_from(s, (size_t)i + sub.n);
	}
	return k;
}

static bool space(int c) { return c == ' ' || (c >= '\t' && c <= '\r'); }

alt_str alt_str_trim_space(alt_str s) {
	while (s.n && space((unsigned char)s.p[0]))
		s.p++, s.n--;
	while (s.n && space((unsigned char)s.p[s.n - 1]))
		s.n--;
	return s;
}

static bool in(const char *set, char c) { return c && strchr(set, c); }

alt_str alt_str_trim_left(alt_str s, const char *cutset) {
	while (s.n && in(cutset, s.p[0]))
		s.p++, s.n--;
	return s;
}

alt_str alt_str_trim_right(alt_str s, const char *cutset) {
	while (s.n && in(cutset, s.p[s.n - 1]))
		s.n--;
	return s;
}

alt_str alt_str_trim(alt_str s, const char *cutset) { return alt_str_trim_right(alt_str_trim_left(s, cutset), cutset); }

alt_str alt_str_trim_prefix(alt_str s, alt_str prefix) { return alt_str_has_prefix(s, prefix) ? alt_str_from(s, prefix.n) : s; }

alt_str alt_str_trim_suffix(alt_str s, alt_str suffix) {
	return alt_str_has_suffix(s, suffix) ? alt_str_slice(s, 0, s.n - suffix.n) : s;
}

bool alt_str_cut(alt_str s, alt_str sep, alt_str *before, alt_str *after) {
	ssize_t i = alt_str_index(s, sep);
	if (i < 0) {
		if (before)
			*before = s;
		if (after)
			*after = (alt_str){s.p + s.n, 0};
		return false;
	}
	if (before)
		*before = alt_str_slice(s, 0, (size_t)i);
	if (after)
		*after = alt_str_from(s, (size_t)i + sep.n);
	return true;
}

bool alt_str_cut_byte(alt_str s, int sep, alt_str *before, alt_str *after) {
	char c = (char)sep;
	return alt_str_cut(s, (alt_str){&c, 1}, before, after);
}

bool alt_str_split_next(alt_str *rest, alt_str sep, alt_str *field) {
	if (!rest->p)
		return false;
	if (!alt_str_cut(*rest, sep, field, rest))
		rest->p = 0; /* the last field */
	return true;
}

bool alt_str_fields_next(alt_str *rest, alt_str *field) {
	size_t i = 0;
	while (i < rest->n && space((unsigned char)rest->p[i]))
		i++;
	if (i == rest->n) {
		*rest = alt_str_from(*rest, i);
		return false;
	}
	size_t j = i;
	while (j < rest->n && !space((unsigned char)rest->p[j]))
		j++;
	*field = alt_str_slice(*rest, i, j);
	*rest = alt_str_from(*rest, j);
	return true;
}

static int digit(int c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	c = lower(c);
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	return 99;
}

int alt_str_to_uint(alt_str s, int base, unsigned long long *out) {
	*out = 0;
	if (base == 0) {
		base = 10;
		if (s.n >= 2 && s.p[0] == '0') {
			int c = lower((unsigned char)s.p[1]);
			if (c == 'x')
				base = 16, s = alt_str_from(s, 2);
			else if (c == 'o')
				base = 8, s = alt_str_from(s, 2);
			else if (c == 'b')
				base = 2, s = alt_str_from(s, 2);
			else
				base = 8, s = alt_str_from(s, 1);
		}
	}
	if (base < 2 || base > 36 || s.n == 0)
		return -ALT_ESYNTAX;
	unsigned long long v = 0;
	for (size_t i = 0; i < s.n; i++) {
		int d = digit((unsigned char)s.p[i]);
		if (d >= base)
			return -ALT_ESYNTAX;
		if (v > (ULLONG_MAX - (unsigned)d) / (unsigned)base) {
			*out = ULLONG_MAX;
			/* the rest must still be digits */
			for (i++; i < s.n; i++)
				if (digit((unsigned char)s.p[i]) >= base)
					return -ALT_ESYNTAX;
			return -ALT_ERANGE;
		}
		v = v * (unsigned)base + (unsigned)d;
	}
	*out = v;
	return 0;
}

int alt_str_to_int(alt_str s, int base, long long *out) {
	*out = 0;
	bool neg = false;
	if (s.n && (s.p[0] == '+' || s.p[0] == '-')) {
		neg = s.p[0] == '-';
		s = alt_str_from(s, 1);
	}
	unsigned long long u;
	int rc = alt_str_to_uint(s, base, &u);
	if (rc == -ALT_ESYNTAX)
		return rc;
	if (rc == -ALT_ERANGE || (!neg && u > (unsigned long long)LLONG_MAX) || (neg && u > (unsigned long long)LLONG_MAX + 1)) {
		*out = neg ? LLONG_MIN : LLONG_MAX;
		return -ALT_ERANGE;
	}
	*out = neg ? (long long)(0 - u) : (long long)u;
	return 0;
}

char *alt_str_dup(alt_str s) {
	char *p = malloc(s.n + 1);
	if (!p)
		return 0;
	if (s.n)
		memcpy(p, s.p, s.n);
	p[s.n] = 0;
	return p;
}

void alt_ascii_lower(char *p, size_t n) {
	for (size_t i = 0; i < n; i++)
		p[i] = (char)lower((unsigned char)p[i]);
}

void alt_ascii_upper(char *p, size_t n) {
	for (size_t i = 0; i < n; i++)
		if (p[i] >= 'a' && p[i] <= 'z')
			p[i] -= 32;
}

/* ---- alt_buf ---- */

void alt_buf_free(alt_buf *b) {
	free(b->p);
	*b = (alt_buf)ALT_BUF_INIT;
}

int alt_buf_reserve(alt_buf *b, size_t extra) {
	if (b->err)
		return b->err;
	if (extra > SIZE_MAX - b->n - 1)
		return b->err = -ENOMEM;
	size_t need = b->n + extra + 1;
	if (need <= b->cap)
		return 0;
	size_t cap = b->cap ? b->cap : 32;
	while (cap < need)
		cap = cap > SIZE_MAX / 2 ? need : cap * 2;
	char *p = realloc(b->p, cap);
	if (!p)
		return b->err = -ENOMEM;
	b->p = p;
	b->cap = cap;
	return 0;
}

int alt_buf_append(alt_buf *b, const void *p, size_t n) {
	/* p may point into the buffer, which may move */
	const char *src = p;
	bool inside = b->p && src >= b->p && src < b->p + b->cap;
	size_t off = inside ? (size_t)(src - b->p) : 0;
	int rc = alt_buf_reserve(b, n);
	if (rc)
		return rc;
	if (inside)
		src = b->p + off;
	if (n)
		memmove(b->p + b->n, src, n);
	b->n += n;
	b->p[b->n] = 0;
	return 0;
}

int alt_buf_append_byte(alt_buf *b, int c) {
	char ch = (char)c;
	return alt_buf_append(b, &ch, 1);
}

int alt_buf_vprintf(alt_buf *b, const char *fmt, va_list ap) {
	va_list ap2;
	va_copy(ap2, ap);
	char small[256];
	int k = vsnprintf(small, sizeof small, fmt, ap2);
	va_end(ap2);
	if (k < 0)
		return -EINVAL;
	if ((size_t)k < sizeof small)
		return alt_buf_append(b, small, (size_t)k);
	int rc = alt_buf_reserve(b, (size_t)k);
	if (rc)
		return rc;
	vsnprintf(b->p + b->n, (size_t)k + 1, fmt, ap);
	b->n += (size_t)k;
	return 0;
}

int alt_buf_printf(alt_buf *b, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int rc = alt_buf_vprintf(b, fmt, ap);
	va_end(ap);
	return rc;
}

int alt_buf_append_uint(alt_buf *b, unsigned long long v) {
	char tmp[24];
	int i = sizeof tmp;
	do {
		tmp[--i] = (char)('0' + v % 10);
		v /= 10;
	} while (v);
	return alt_buf_append(b, tmp + i, sizeof tmp - (size_t)i);
}

int alt_buf_append_int(alt_buf *b, long long v) {
	if (v < 0) {
		int rc = alt_buf_append_byte(b, '-');
		if (rc)
			return rc;
		return alt_buf_append_uint(b, 0 - (unsigned long long)v);
	}
	return alt_buf_append_uint(b, (unsigned long long)v);
}

void alt_buf_truncate(alt_buf *b, size_t n) {
	if (!b->p)
		return;
	if (n >= b->cap)
		n = b->cap - 1;
	b->n = n;
	b->p[n] = 0;
}

void alt_buf_consume(alt_buf *b, size_t n) {
	if (n >= b->n) {
		alt_buf_reset(b);
		return;
	}
	memmove(b->p, b->p + n, b->n - n);
	b->n -= n;
	b->p[b->n] = 0;
}

char *alt_buf_detach(alt_buf *b, size_t *n) {
	char *p = b->p;
	if (n)
		*n = b->n;
	if (!p)
		p = calloc(1, 1);
	*b = (alt_buf)ALT_BUF_INIT;
	return p;
}
