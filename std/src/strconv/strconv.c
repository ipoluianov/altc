/* Number formatting and parsing, after Go's strconv (ftoa.go fmtEFG;
   BSD-3-Clause, see LICENSE-Go). The digits come from the C library's
   exact conversions (correctly rounded, ties to even, as Go's decimal
   rounding); the shortest digits are the shortest that read back. */
#include <alt/strconv.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the digits and exponent of a %.*e conversion of |f| with p digits */
static int edigits(double f, int p, char *d, int *exp) {
	char tmp[64];
	snprintf(tmp, sizeof tmp, "%.*e", p - 1, fabs(f));
	int n = 0;
	char *q = tmp;
	for (; *q && *q != 'e'; q++)
		if (*q >= '0' && *q <= '9')
			d[n++] = *q;
	d[n] = 0;
	*exp = atoi(q + 1) + 1;
	return n;
}

/* adds delta (+1 or -1) to the last digit of d[0:n]; false when the
   number of digits would change */
static bool bump(char *d, int n, int delta) {
	int i = n - 1;
	char carry = delta > 0 ? '9' : '0', wrap = delta > 0 ? '0' : '9';
	while (i >= 0 && d[i] == carry)
		d[i--] = wrap;
	if (i < 0)
		return false;
	d[i] = (char)(d[i] + delta);
	return d[0] != '0';
}

static double value(const char *d, int n, int exp) {
	char tmp[64];
	snprintf(tmp, sizeof tmp, "0.%.*se%d", n, d, exp);
	return strtod(tmp, 0);
}

int alt_shortest_digits(double f, char buf[32], int *exp) {
	f = fabs(f);
	for (int p = 1; p <= 17; p++) {
		char d[32];
		int e, n = edigits(f, p, d, &e);
		/* the closest p digits; when they do not read back, a neighbor may
		   (the rounding interval of a power of two is not symmetric) */
		bool ok = value(d, n, e) == f;
		for (int k = -1; !ok && k <= 1; k += 2) {
			char c[32];
			memcpy(c, d, (size_t)n + 1);
			if (bump(c, n, k) && value(c, n, e) == f) {
				memcpy(d, c, (size_t)n + 1);
				ok = true;
			}
		}
		if (ok) {
			while (n > 1 && d[n - 1] == '0')
				n--;
			memcpy(buf, d, (size_t)n);
			buf[n] = 0;
			*exp = e;
			return n;
		}
	}
	int n = edigits(f, 17, buf, exp);
	while (n > 1 && buf[n - 1] == '0')
		n--;
	buf[n] = 0;
	return n;
}

/* Go's fmtEFG */
static void fmt_efg(alt_buf *b, bool neg, const char *s, int dp, int nd, int prec, char fmt, bool shortest) {
	if (fmt == 'g' || fmt == 'G') {
		int eprec = prec;
		if (eprec > nd && nd >= dp)
			eprec = nd;
		if (shortest)
			eprec = 6;
		int exp = dp - 1;
		if (exp < -4 || exp >= eprec) {
			if (prec > nd)
				prec = nd;
			prec--;
			fmt = (char)(fmt + 'e' - 'g');
		} else {
			if (prec > dp)
				prec = nd;
			prec = prec - dp > 0 ? prec - dp : 0;
			fmt = 'f';
		}
	}
	if (fmt == 'e' || fmt == 'E') {
		if (neg)
			alt_buf_append_byte(b, '-');
		alt_buf_append_byte(b, nd ? s[0] : '0');
		if (prec > 0) {
			alt_buf_append_byte(b, '.');
			int i = 1, m = nd < prec + 1 ? nd : prec + 1;
			if (i < m) {
				alt_buf_append(b, s + i, (size_t)(m - i));
				i = m;
			}
			for (; i < prec + 1; i++)
				alt_buf_append_byte(b, '0');
		}
		alt_buf_append_byte(b, fmt);
		int exp = nd ? dp - 1 : 0;
		alt_buf_append_byte(b, exp < 0 ? '-' : '+');
		if (exp < 0)
			exp = -exp;
		if (exp < 100)
			alt_buf_printf(b, "%02d", exp);
		else
			alt_buf_printf(b, "%03d", exp);
		return;
	}
	/* 'f' */
	if (neg)
		alt_buf_append_byte(b, '-');
	if (dp > 0) {
		int m = nd < dp ? nd : dp;
		alt_buf_append(b, s, (size_t)m);
		for (int i = m; i < dp; i++)
			alt_buf_append_byte(b, '0');
	} else
		alt_buf_append_byte(b, '0');
	if (prec > 0) {
		alt_buf_append_byte(b, '.');
		int lz = -dp > 0 ? -dp : 0;
		if (lz > prec)
			lz = prec;
		int off = dp + lz;
		int m = nd - off > 0 ? nd - off : 0;
		if (m > prec - lz)
			m = prec - lz;
		int tz = prec - lz - m > 0 ? prec - lz - m : 0;
		for (int i = 0; i < lz; i++)
			alt_buf_append_byte(b, '0');
		alt_buf_append(b, s + off, (size_t)m);
		for (int i = 0; i < tz; i++)
			alt_buf_append_byte(b, '0');
	}
}

static int trim(char *d, int n) {
	while (n > 0 && d[n - 1] == '0')
		n--;
	d[n] = 0;
	return n;
}

int alt_format_float(alt_buf *b, double f, char fmt, int prec) {
	if (isnan(f))
		return alt_buf_append_cstr(b, "NaN");
	if (isinf(f))
		return alt_buf_append_cstr(b, f < 0 ? "-Inf" : "+Inf");
	bool neg = signbit(f);
	if (fmt != 'e' && fmt != 'E' && fmt != 'f' && fmt != 'g' && fmt != 'G') {
		alt_buf_append_byte(b, '%');
		return alt_buf_append_byte(b, fmt);
	}
	char d[400];
	int dp = 0, nd = 0;
	if (f == 0) {
		fmt_efg(b, neg, "", 0, 0, prec, fmt, prec < 0);
		return b->err;
	}
	if (prec < 0) {
		char s[32];
		nd = alt_shortest_digits(f, s, &dp);
		memcpy(d, s, (size_t)nd + 1);
		switch (fmt) {
		case 'e':
		case 'E': prec = nd - 1 > 0 ? nd - 1 : 0; break;
		case 'f': prec = nd - dp > 0 ? nd - dp : 0; break;
		default: prec = nd;
		}
		fmt_efg(b, neg, d, dp, nd, prec, fmt, true);
		return b->err;
	}
	if (fmt == 'f') {
		/* the digits of %.*f */
		char *tmp = malloc((size_t)prec + 330);
		if (!tmp)
			return -ENOMEM;
		snprintf(tmp, (size_t)prec + 330, "%.*f", prec, fabs(f));
		char *dot = strchr(tmp, '.');
		int intlen = dot ? (int)(dot - tmp) : (int)strlen(tmp);
		char *all = malloc(strlen(tmp) + 1);
		if (!all) {
			free(tmp);
			return -ENOMEM;
		}
		int n = 0;
		for (char *q = tmp; *q; q++)
			if (*q >= '0' && *q <= '9')
				all[n++] = *q;
		all[n] = 0;
		int lead = 0;
		while (lead < n && all[lead] == '0')
			lead++;
		dp = intlen - lead;
		nd = trim(all + lead, n - lead);
		if (nd == 0)
			dp = 0;
		fmt_efg(b, neg, all + lead, dp, nd, prec, fmt, false);
		free(all);
		free(tmp);
		return b->err;
	}
	int digits = fmt == 'g' || fmt == 'G' ? (prec ? prec : 1) : prec + 1;
	if (fmt == 'g' || fmt == 'G')
		prec = digits;
	if (digits > 390)
		digits = 390; /* beyond that the digits are exact zeros */
	nd = trim(d, edigits(f, digits, d, &dp));
	fmt_efg(b, neg, d, dp, nd, prec, fmt, false);
	return b->err;
}

/* ---- parsing ---- */

static bool digit(char c) { return c >= '0' && c <= '9'; }
static bool hexdigit(char c) { return digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

/* Go's underscoreOK: underscores only between digits (or after a base
   prefix) */
static bool underscore_ok(alt_str s) {
	char saw = '^';
	size_t i = 0;
	if (s.n && (s.p[0] == '-' || s.p[0] == '+'))
		s = alt_str_from(s, 1);
	bool hex = false;
	if (s.n >= 2 && s.p[0] == '0' && strchr("bBoOxX", s.p[1])) {
		i = 2;
		saw = '0';
		hex = s.p[1] == 'x' || s.p[1] == 'X';
	}
	for (; i < s.n; i++) {
		char c = s.p[i];
		if (digit(c) || (hex && hexdigit(c))) {
			saw = '0';
			continue;
		}
		if (c == '_') {
			if (saw != '0')
				return false;
			saw = '_';
			continue;
		}
		if (saw == '_')
			return false;
		saw = '!';
	}
	return saw != '_';
}

int alt_parse_float(alt_str s, double *out) {
	*out = 0;
	size_t i = 0;
	bool neg = false;
	if (i < s.n && (s.p[i] == '+' || s.p[i] == '-'))
		neg = s.p[i++] == '-';
	alt_str rest = alt_str_from(s, i);
	if (alt_str_eq_fold(rest, ALT_STR("inf")) || alt_str_eq_fold(rest, ALT_STR("infinity"))) {
		*out = neg ? -INFINITY : INFINITY;
		return 0;
	}
	if (alt_str_eq_fold(rest, ALT_STR("nan")) && i == 0) {
		*out = NAN;
		return 0;
	}
	/* the grammar of Go's readFloat; underscores only with a prefix */
	char buf[512];
	size_t n = 0;
	bool hex = rest.n > 2 && rest.p[0] == '0' && (rest.p[1] == 'x' || rest.p[1] == 'X');
	size_t j = hex ? 2 : 0;
	int ndig = 0;
	bool dot = false, underscores = false;
	char lastc = 0;
	for (; j < rest.n; j++) {
		char c = rest.p[j];
		if (c == '_') {
			underscores = true;
			lastc = c;
			continue;
		}
		if (c == '.') {
			if (dot)
				return -ALT_ESYNTAX;
			dot = true;
		} else if (hex ? hexdigit(c) : digit(c))
			ndig++;
		else
			break;
		lastc = c;
	}
	(void)lastc;
	if (ndig == 0)
		return -ALT_ESYNTAX;
	if (j < rest.n) {
		char c = rest.p[j];
		if ((!hex && (c == 'e' || c == 'E')) || (hex && (c == 'p' || c == 'P'))) {
			j++;
			if (j < rest.n && (rest.p[j] == '+' || rest.p[j] == '-'))
				j++;
			size_t k = j;
			while (j < rest.n && (digit(rest.p[j]) || rest.p[j] == '_')) {
				underscores |= rest.p[j] == '_';
				j++;
			}
			if (j == k)
				return -ALT_ESYNTAX;
		} else
			return -ALT_ESYNTAX;
	} else if (hex)
		return -ALT_ESYNTAX; /* hexadecimal floats need an exponent */
	if (j != rest.n)
		return -ALT_ESYNTAX;
	if (underscores && !underscore_ok(s))
		return -ALT_ESYNTAX;
	/* the C library converts (without the underscores) */
	if (s.n >= sizeof buf)
		return -ALT_ERANGE;
	for (size_t k = 0; k < s.n; k++)
		if (s.p[k] != '_')
			buf[n++] = s.p[k];
	buf[n] = 0;
	errno = 0;
	double v = strtod(buf, 0);
	*out = v;
	if (isinf(v))
		return -ALT_ERANGE;
	return 0;
}

/* ---- integers (Go's ParseUint and ParseInt) ---- */

static int lower(int c) { return c | 0x20; }

int alt_parse_uint(alt_str s, int base, int bits, unsigned long long *out) {
	*out = 0;
	if (s.n == 0)
		return -ALT_ESYNTAX;
	bool base0 = base == 0;
	alt_str s0 = s;
	if (base == 0) {
		base = 10;
		if (s.p[0] == '0') {
			if (s.n >= 3 && lower(s.p[1]) == 'b')
				base = 2, s = alt_str_slice(s, 2, s.n);
			else if (s.n >= 3 && lower(s.p[1]) == 'o')
				base = 8, s = alt_str_slice(s, 2, s.n);
			else if (s.n >= 3 && lower(s.p[1]) == 'x')
				base = 16, s = alt_str_slice(s, 2, s.n);
			else
				base = 8, s = alt_str_slice(s, 1, s.n);
		}
	} else if (base < 2 || base > 36)
		return -EINVAL;
	if (bits == 0)
		bits = 64;
	else if (bits < 0 || bits > 64)
		return -EINVAL;
	unsigned long long cutoff = ~0ULL / (unsigned)base + 1;
	unsigned long long max = bits == 64 ? ~0ULL : (1ULL << bits) - 1;
	bool underscores = false;
	unsigned long long n = 0;
	for (size_t i = 0; i < s.n; i++) {
		int c = (unsigned char)s.p[i], d;
		if (c == '_' && base0) {
			underscores = true;
			continue;
		} else if (c >= '0' && c <= '9')
			d = c - '0';
		else if (lower(c) >= 'a' && lower(c) <= 'z')
			d = lower(c) - 'a' + 10;
		else
			return -ALT_ESYNTAX;
		if (d >= base)
			return -ALT_ESYNTAX;
		if (n >= cutoff) {
			*out = max;
			return -ALT_ERANGE;
		}
		n *= (unsigned)base;
		unsigned long long n1 = n + (unsigned)d;
		if (n1 < n || n1 > max) {
			*out = max;
			return -ALT_ERANGE;
		}
		n = n1;
	}
	if (underscores && !underscore_ok(s0))
		return -ALT_ESYNTAX;
	*out = n;
	return 0;
}

int alt_parse_int(alt_str s, int base, int bits, long long *out) {
	*out = 0;
	if (s.n == 0)
		return -ALT_ESYNTAX;
	bool neg = false;
	if (s.p[0] == '+' || s.p[0] == '-') {
		neg = s.p[0] == '-';
		s = alt_str_slice(s, 1, s.n);
	}
	unsigned long long un;
	int rc = alt_parse_uint(s, base, bits, &un);
	if (rc < 0 && rc != -ALT_ERANGE)
		return rc;
	if (bits == 0)
		bits = 64;
	unsigned long long cutoff = 1ULL << (bits - 1);
	if (!neg && un >= cutoff) {
		*out = (long long)(cutoff - 1);
		return -ALT_ERANGE;
	}
	if (neg && un > cutoff) {
		*out = -(long long)(cutoff - 1) - 1;
		return -ALT_ERANGE;
	}
	*out = neg ? (long long)(0 - un) : (long long)un;
	return 0;
}

/* ---- quoting ---- */

enum { Q_PRINT, Q_ASCII, Q_GRAPHIC };

static int quote(alt_buf *b, alt_str s, int mode) {
	static const char hex[] = "0123456789abcdef";
	alt_buf_append_byte(b, '"');
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (r == ALT_UTF8_RUNE_ERROR && w == 1) {
			unsigned char c = (unsigned char)s.p[i];
			alt_buf_printf(b, "\\x%c%c", hex[c >> 4], hex[c & 15]);
			i++;
			continue;
		}
		i += w;
		switch (r) {
		case '\a': alt_buf_append_cstr(b, "\\a"); continue;
		case '\b': alt_buf_append_cstr(b, "\\b"); continue;
		case '\f': alt_buf_append_cstr(b, "\\f"); continue;
		case '\n': alt_buf_append_cstr(b, "\\n"); continue;
		case '\r': alt_buf_append_cstr(b, "\\r"); continue;
		case '\t': alt_buf_append_cstr(b, "\\t"); continue;
		case '\v': alt_buf_append_cstr(b, "\\v"); continue;
		case '\\': alt_buf_append_cstr(b, "\\\\"); continue;
		case '"': alt_buf_append_cstr(b, "\\\""); continue;
		}
		bool keep = mode == Q_ASCII ? r >= ' ' && r < 0x7f : mode == Q_PRINT ? alt_unicode_is_print(r) : alt_unicode_is_graphic(r);
		if (keep)
			alt_buf_append(b, s.p + i - w, w);
		else if (r < ' ' || r == 0x7f)
			alt_buf_printf(b, "\\x%c%c", hex[r >> 4], hex[r & 15]);
		else if (r < 0x10000)
			alt_buf_printf(b, "\\u%04x", (unsigned)r);
		else
			alt_buf_printf(b, "\\U%08x", (unsigned)r);
	}
	return alt_buf_append_byte(b, '"');
}

int alt_quote(alt_buf *b, alt_str s) { return quote(b, s, Q_PRINT); }
int alt_quote_ascii(alt_buf *b, alt_str s) { return quote(b, s, Q_ASCII); }
int alt_quote_graphic(alt_buf *b, alt_str s) { return quote(b, s, Q_GRAPHIC); }

static int hexval(char c) { return digit(c) ? c - '0' : (c | 32) - 'a' + 10; }

int alt_unquote(alt_buf *b, alt_str s) {
	if (s.n < 2)
		return -ALT_ESYNTAX;
	char q = s.p[0];
	if (q != s.p[s.n - 1] || (q != '"' && q != '`' && q != '\''))
		return -ALT_ESYNTAX;
	alt_str in = alt_str_slice(s, 1, s.n - 1);
	if (q == '`') {
		if (alt_str_index_byte(in, '`') >= 0)
			return -ALT_ESYNTAX;
		/* raw strings drop carriage returns */
		for (size_t i = 0; i < in.n; i++)
			if (in.p[i] != '\r')
				alt_buf_append_byte(b, in.p[i]);
		return b->err;
	}
	size_t start = b->n;
	for (size_t i = 0; i < in.n;) {
		char c = in.p[i];
		if (c == q || c == '\n')
			return -ALT_ESYNTAX;
		if (c != '\\') {
			alt_buf_append_byte(b, c);
			i++;
			continue;
		}
		if (++i >= in.n)
			return -ALT_ESYNTAX;
		c = in.p[i++];
		int n = 0;
		switch (c) {
		case 'a': alt_buf_append_byte(b, '\a'); break;
		case 'b': alt_buf_append_byte(b, '\b'); break;
		case 'f': alt_buf_append_byte(b, '\f'); break;
		case 'n': alt_buf_append_byte(b, '\n'); break;
		case 'r': alt_buf_append_byte(b, '\r'); break;
		case 't': alt_buf_append_byte(b, '\t'); break;
		case 'v': alt_buf_append_byte(b, '\v'); break;
		case '\\': alt_buf_append_byte(b, '\\'); break;
		case '\'':
		case '"':
			if (c != q)
				return -ALT_ESYNTAX;
			alt_buf_append_byte(b, c);
			break;
		case 'x': n = 2; break;
		case 'u': n = 4; break;
		case 'U': n = 8; break;
		default:
			if (c >= '0' && c <= '7') {
				if (i + 2 > in.n)
					return -ALT_ESYNTAX;
				int v = c - '0';
				for (int k = 0; k < 2; k++) {
					char o = in.p[i++];
					if (o < '0' || o > '7')
						return -ALT_ESYNTAX;
					v = v * 8 + (o - '0');
				}
				if (v > 255)
					return -ALT_ESYNTAX;
				alt_buf_append_byte(b, v);
				break;
			}
			return -ALT_ESYNTAX;
		}
		if (n) {
			if (i + (size_t)n > in.n)
				return -ALT_ESYNTAX;
			uint32_t v = 0;
			for (int k = 0; k < n; k++) {
				if (!hexdigit(in.p[i + (size_t)k]))
					return -ALT_ESYNTAX;
				v = v << 4 | (uint32_t)hexval(in.p[i + (size_t)k]);
			}
			i += (size_t)n;
			if (c == 'x')
				alt_buf_append_byte(b, (int)v);
			else {
				if (!alt_utf8_valid_rune((int32_t)v))
					return -ALT_ESYNTAX;
				char u[4];
				alt_buf_append(b, u, alt_utf8_encode(u, (int32_t)v));
			}
		}
	}
	if (q == '\'') {
		/* one character */
		alt_str got = alt_str_from(alt_buf_str(b), start);
		int32_t r;
		if (got.n == 0 || alt_utf8_decode(got.p, got.n, &r) != got.n)
			return -ALT_ESYNTAX;
	}
	return b->err;
}

int alt_format_uint(alt_buf *b, unsigned long long v, int base) {
	if (base < 2 || base > 36)
		return -EINVAL;
	char tmp[72];
	int i = sizeof tmp;
	do {
		tmp[--i] = "0123456789abcdefghijklmnopqrstuvwxyz"[v % (unsigned)base];
		v /= (unsigned)base;
	} while (v);
	return alt_buf_append(b, tmp + i, sizeof tmp - (size_t)i);
}

int alt_format_int(alt_buf *b, long long v, int base) {
	if (v < 0) {
		alt_buf_append_byte(b, '-');
		return alt_format_uint(b, 0 - (unsigned long long)v, base);
	}
	return alt_format_uint(b, (unsigned long long)v, base);
}

int alt_parse_bool(alt_str s, bool *out) {
	static const char *t[] = {"1", "t", "T", "TRUE", "true", "True"}, *f[] = {"0", "f", "F", "FALSE", "false", "False"};
	for (int i = 0; i < 6; i++) {
		if (alt_str_eq_cstr(s, t[i])) {
			*out = true;
			return 0;
		}
		if (alt_str_eq_cstr(s, f[i])) {
			*out = false;
			return 0;
		}
	}
	return -ALT_ESYNTAX;
}
