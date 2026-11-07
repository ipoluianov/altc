/* Tar header fields, PAX records and times, following Go's archive/tar
   (strconv.go, format.go; BSD-3-Clause, see LICENSE-Go). */
#include "tar_internal.h"
#include <stdio.h>

/* ---- strings ---- */

bool alt__tar_has_nul(alt_str s) { return s.n && memchr(s.p, 0, s.n) != 0; }

/* Go ranges over runes: a byte >= 0x80 never decodes to an ASCII rune */
bool alt__tar_is_ascii(alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		if ((unsigned char)s.p[i] >= 0x80 || s.p[i] == 0)
			return false;
	return true;
}

alt_str alt__tar_to_ascii(alt_str s, alt_buf *b) {
	if (alt__tar_is_ascii(s))
		return s;
	alt_buf_reset(b);
	for (size_t i = 0; i < s.n; i++)
		if ((unsigned char)s.p[i] < 0x80 && s.p[i] != 0)
			alt_buf_append_byte(b, s.p[i]);
	return alt_buf_str(b);
}

alt_str alt__tar_parse_string(const uint8_t *b, size_t n) {
	const uint8_t *z = memchr(b, 0, n);
	return alt_str_make(b, z ? (size_t)(z - b) : n);
}

void alt__tar_format_string(uint8_t *b, size_t n, alt_str s, int *err) {
	if (s.n > n)
		*err = -ALT_ETAR_FIELD;
	if (s.n)
		memcpy(b, s.p, s.n < n ? s.n : n);
	if (s.n < n)
		b[s.n] = 0;
	/* GNU and BSD tar check for a trailing slash in a truncated name:
	   replace it (and the slashes before it) with NUL */
	if (s.n > n && b[n - 1] == '/') {
		size_t k = n - 1;
		while (k > 0 && s.p[k - 1] == '/')
			k--;
		b[k] = 0;
	}
}

/* ---- numbers ---- */

bool alt__tar_fits_base256(size_t n, int64_t x) {
	if (n >= 9)
		return true;
	unsigned bits = (unsigned)(n - 1) * 8;
	return x >= -((int64_t)1 << bits) && x < ((int64_t)1 << bits);
}

bool alt__tar_fits_octal(size_t n, int64_t x) {
	unsigned bits = (unsigned)(n - 1) * 3;
	return x >= 0 && (n >= 22 || x < ((int64_t)1 << bits));
}

int64_t alt__tar_parse_octal(const uint8_t *b, size_t n, int *err) {
	size_t i = 0, j = n;
	while (i < j && (b[i] == ' ' || b[i] == 0))
		i++;
	while (j > i && (b[j - 1] == ' ' || b[j - 1] == 0))
		j--;
	if (i == j)
		return 0;
	alt_str s = alt__tar_parse_string(b + i, j - i);
	unsigned long long x = 0;
	if (alt_parse_uint(s, 8, 64, &x) < 0)
		*err = -ALT_ETAR_HEADER;
	return (int64_t)x;
}

int64_t alt__tar_parse_numeric(const uint8_t *b, size_t n, int *err) {
	/* base-256 (binary) encoding: the high bit of the first byte is set,
	   the next bit is the sign (two's complement) */
	if (n > 0 && b[0] & 0x80) {
		uint8_t inv = b[0] & 0x40 ? 0xff : 0;
		uint64_t x = 0;
		for (size_t i = 0; i < n; i++) {
			uint8_t c = b[i] ^ inv;
			if (i == 0)
				c &= 0x7f;
			if (x >> 56) {
				*err = -ALT_ETAR_HEADER;
				return 0;
			}
			x = x << 8 | c;
		}
		if (x >> 63) {
			*err = -ALT_ETAR_HEADER;
			return 0;
		}
		return inv ? (int64_t)~x : (int64_t)x;
	}
	return alt__tar_parse_octal(b, n, err);
}

void alt__tar_format_octal(uint8_t *b, size_t n, int64_t x, int *err) {
	if (!alt__tar_fits_octal(n, x)) {
		x = 0;
		*err = -ALT_ETAR_FIELD;
	}
	char d[32], s[32];
	int nd = 0;
	uint64_t u = (uint64_t)x;
	do
		d[nd++] = (char)('0' + (u & 7));
	while (u >>= 3);
	int ns = 0;
	for (int k = (int)n - nd - 1; k > 0; k--)
		s[ns++] = '0';
	while (nd)
		s[ns++] = d[--nd];
	alt__tar_format_string(b, n, alt_str_make(s, (size_t)ns), err);
}

void alt__tar_format_numeric(uint8_t *b, size_t n, int64_t x, int *err) {
	if (alt__tar_fits_octal(n, x)) {
		alt__tar_format_octal(b, n, x, err);
		return;
	}
	if (alt__tar_fits_base256(n, x)) {
		uint64_t u = (uint64_t)x;
		for (size_t i = n; i-- > 0;) {
			b[i] = (uint8_t)u;
			u = (uint64_t)((int64_t)u >> 8);
		}
		b[0] |= 0x80;
		return;
	}
	alt__tar_format_octal(b, n, 0, err);
	*err = -ALT_ETAR_FIELD;
}

/* ---- blocks ---- */

static void checksums(const uint8_t *b, int64_t *unsig, int64_t *sig) {
	int64_t u = 0, s = 0;
	for (int i = 0; i < TAR_BLOCK; i++) {
		uint8_t c = i >= 148 && i < 156 ? ' ' : b[i];
		u += c;
		s += (int8_t)c;
	}
	*unsig = u;
	*sig = s;
}

static bool field_is(const uint8_t *b, size_t n, const char *s) { return memcmp(b, s, n) == 0; }

int alt__tar_get_format(const uint8_t *blk) {
	int err = 0;
	int64_t v = alt__tar_parse_octal(FLD(blk, F_CHKSUM), &err), c1, c2;
	checksums(blk, &c1, &c2);
	if (err || (v != c1 && v != c2))
		return ALT_TAR_FORMAT_UNKNOWN;
	bool ustar = field_is(FLD(blk, F_MAGIC), "ustar\0");
	if (ustar && field_is(FLD(blk, F_STAR_TRAILER), "tar\0"))
		return ALT_TAR_FORMAT_STAR;
	if (ustar)
		return ALT_TAR_FORMAT_USTAR | ALT_TAR_FORMAT_PAX;
	if (field_is(FLD(blk, F_MAGIC), "ustar ") && field_is(FLD(blk, F_VERSION), " \0"))
		return ALT_TAR_FORMAT_GNU;
	return ALT_TAR_FORMAT_V7;
}

void alt__tar_set_format(uint8_t *blk, int format) {
	if (format & ALT_TAR_FORMAT_V7) {
	} else if (format & ALT_TAR_FORMAT_GNU) {
		memcpy(blk + 257, "ustar ", 6);
		memcpy(blk + 263, " \0", 2);
	} else if (format & ALT_TAR_FORMAT_STAR) {
		memcpy(blk + 257, "ustar\0", 6);
		memcpy(blk + 263, "00", 2);
		memcpy(blk + 508, "tar\0", 4);
	} else {
		memcpy(blk + 257, "ustar\0", 6);
		memcpy(blk + 263, "00", 2);
	}
	int64_t c, c2;
	checksums(blk, &c, &c2);
	int err = 0;
	alt__tar_format_octal(blk + 148, 7, c, &err); /* 256..128776 always fits */
	blk[155] = ' ';
}

/* ---- PAX records ---- */

int alt__tar_atoi(alt_str s, int64_t *v) {
	long long x;
	int rc = alt_parse_int(s, 10, 64, &x);
	*v = rc < 0 ? 0 : x;
	return rc;
}

bool alt__tar_valid_pax_record(alt_str k, alt_str v) {
	if (k.n == 0 || memchr(k.p, '=', k.n))
		return false;
	if (alt_str_eq_cstr(k, "path") || alt_str_eq_cstr(k, "linkpath") || alt_str_eq_cstr(k, "uname") ||
		alt_str_eq_cstr(k, "gname"))
		return !alt__tar_has_nul(v);
	return !alt__tar_has_nul(k);
}

/* "%d %s=%s\n": the length counts itself */
int alt__tar_parse_pax_record(alt_str s, alt_str *k, alt_str *v, alt_str *rest) {
	ssize_t sp = alt_str_index_byte(s, ' ');
	if (sp < 0)
		return -ALT_ETAR_HEADER;
	int64_t n;
	if (alt__tar_atoi(alt_str_make(s.p, (size_t)sp), &n) < 0 || n < 5 || n > (int64_t)s.n)
		return -ALT_ETAR_HEADER;
	n -= sp + 1;
	if (n <= 0)
		return -ALT_ETAR_HEADER;
	alt_str r = alt_str_from(s, (size_t)sp + 1);
	if (r.p[n - 1] != '\n')
		return -ALT_ETAR_HEADER;
	alt_str rec = alt_str_make(r.p, (size_t)n - 1);
	ssize_t eq = alt_str_index_byte(rec, '=');
	if (eq < 0)
		return -ALT_ETAR_HEADER;
	*k = alt_str_make(rec.p, (size_t)eq);
	*v = alt_str_from(rec, (size_t)eq + 1);
	if (!alt__tar_valid_pax_record(*k, *v))
		return -ALT_ETAR_HEADER;
	*rest = alt_str_from(r, (size_t)n);
	return 0;
}

static size_t ndigits(size_t x) {
	size_t n = 1;
	while (x >= 10) {
		x /= 10;
		n++;
	}
	return n;
}

int alt__tar_format_pax_record(alt_buf *b, alt_str k, alt_str v) {
	if (!alt__tar_valid_pax_record(k, v))
		return -ALT_ETAR_HEADER;
	size_t size = k.n + v.n + 3;
	size += ndigits(size);
	size_t rec = ndigits(size) + 1 + k.n + 1 + v.n + 1;
	if (rec != size)
		size = rec;
	alt_buf_append_uint(b, size);
	alt_buf_append_byte(b, ' ');
	alt_buf_append_str(b, k);
	alt_buf_append_byte(b, '=');
	alt_buf_append_str(b, v);
	return alt_buf_append_byte(b, '\n');
}

/* ---- times ---- */

alt_tar_time alt_tar_unix(int64_t sec, int64_t nsec) {
	if (nsec < 0 || nsec >= 1000000000) {
		int64_t q = nsec / 1000000000;
		sec = (int64_t)((uint64_t)sec + (uint64_t)q);
		nsec -= q * 1000000000;
		if (nsec < 0) {
			nsec += 1000000000;
			sec = (int64_t)((uint64_t)sec - 1);
		}
	}
	return (alt_tar_time){sec, (int32_t)nsec, true};
}

bool alt_tar_time_is_zero(alt_tar_time t) { return !t.set || (t.sec == TAR_ZERO_SEC && t.nsec == 0); }

alt_tar_time alt__tar_round_second(alt_tar_time t) {
	if (!t.set || t.nsec < 500000000) {
		t.nsec = 0;
		return t;
	}
	/* Go keeps seconds since year 1 and saturates at its limits */
	int64_t internal = (int64_t)((uint64_t)t.sec - (uint64_t)TAR_ZERO_SEC);
	if (internal != INT64_MAX)
		internal++;
	t.sec = (int64_t)((uint64_t)internal + (uint64_t)TAR_ZERO_SEC);
	t.nsec = 0;
	return t;
}

int alt__tar_parse_pax_time(alt_str s, alt_tar_time *t) {
	ssize_t dot = alt_str_index_byte(s, '.');
	alt_str ss = dot < 0 ? s : alt_str_make(s.p, (size_t)dot), sn = dot < 0 ? ALT_STR("") : alt_str_from(s, (size_t)dot + 1);
	int64_t secs;
	if (alt__tar_atoi(ss, &secs) < 0)
		return -ALT_ETAR_HEADER;
	if (sn.n == 0) {
		*t = alt_tar_unix(secs, 0);
		return 0;
	}
	int64_t nsecs = 0;
	for (size_t i = 0; i < sn.n; i++) {
		char c = sn.p[i];
		if (c < '0' || c > '9')
			return -ALT_ETAR_HEADER;
		if (i < 9)
			nsecs = nsecs * 10 + (c - '0');
	}
	for (size_t i = sn.n; i < 9; i++)
		nsecs *= 10;
	*t = alt_tar_unix(secs, ss.n && ss.p[0] == '-' ? -nsecs : nsecs);
	return 0;
}

void alt__tar_format_pax_time(alt_buf *b, alt_tar_time t) {
	int64_t secs = t.sec, nsecs = t.nsec;
	if (nsecs == 0) {
		alt_buf_append_int(b, secs);
		return;
	}
	if (secs < 0) {
		alt_buf_append_byte(b, '-');
		secs = -(secs + 1);
		nsecs = 1000000000 - nsecs;
	}
	char frac[16];
	snprintf(frac, sizeof frac, "%09lld", (long long)nsecs);
	size_t n = strlen(frac);
	while (n && frac[n - 1] == '0')
		n--;
	alt_buf_printf(b, "%lld.%.*s", (long long)secs, (int)n, frac);
}

static void append_int_width(alt_buf *b, int64_t x, int width) {
	uint64_t u = (uint64_t)x;
	if (x < 0) {
		alt_buf_append_byte(b, '-');
		u = -u;
	}
	char d[24];
	int n = 0;
	do
		d[n++] = (char)('0' + u % 10);
	while (u /= 10);
	for (int k = width - n; k > 0; k--)
		alt_buf_append_byte(b, '0');
	while (n)
		alt_buf_append_byte(b, d[--n]);
}

void alt__tar_format_go_time(alt_buf *b, alt_tar_time t) {
	if (!t.set)
		t = (alt_tar_time){TAR_ZERO_SEC, 0, true};
	/* Go counts seconds from year 1 in an int64 (wrapping like time.Unix) */
	int64_t internal = (int64_t)((uint64_t)t.sec - (uint64_t)TAR_ZERO_SEC);
	int64_t days = internal / 86400, rem = internal % 86400;
	if (rem < 0) {
		rem += 86400;
		days--;
	}
	/* civil date from days since 0001-01-01 (proleptic Gregorian, H. Hinnant's algorithm on days since 0000-03-01) */
	int64_t z = days + 306;
	int64_t era = (z >= 0 ? z : z - 146096) / 146097;
	int64_t doe = z - era * 146097;
	int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	int64_t y = yoe + era * 400;
	int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	int64_t mp = (5 * doy + 2) / 153;
	int64_t d = doy - (153 * mp + 2) / 5 + 1;
	int64_t m = mp < 10 ? mp + 3 : mp - 9;
	if (m <= 2)
		y++;
	append_int_width(b, y, 4);
	alt_buf_printf(b, "-%02d-%02d %02d:%02d:%02d", (int)m, (int)d, (int)(rem / 3600), (int)(rem / 60 % 60), (int)(rem % 60));
	if (t.nsec) {
		char frac[16];
		snprintf(frac, sizeof frac, "%09d", (int)t.nsec);
		size_t n = strlen(frac);
		while (n && frac[n - 1] == '0')
			n--;
		alt_buf_printf(b, ".%.*s", (int)n, frac);
	}
	alt_buf_append_cstr(b, " +0000 UTC");
}

/* ---- record sets ---- */

static void merge_sort(alt_tar_record *r, alt_tar_record *tmp, size_t n) {
	if (n < 2)
		return;
	size_t h = n / 2;
	merge_sort(r, tmp, h);
	merge_sort(r + h, tmp, n - h);
	size_t i = 0, j = h, k = 0;
	while (i < h && j < n)
		tmp[k++] = alt_str_cmp(r[j].key, r[i].key) < 0 ? r[j++] : r[i++];
	while (i < h)
		tmp[k++] = r[i++];
	while (j < n)
		tmp[k++] = r[j++];
	memcpy(r, tmp, n * sizeof *r);
}

size_t alt__tar_sort_records(alt_tar_record *r, size_t n) {
	if (n < 2)
		return n;
	alt_tar_record *tmp = malloc(n * sizeof *tmp);
	if (!tmp) {
		/* insertion sort, stable */
		for (size_t i = 1; i < n; i++) {
			alt_tar_record x = r[i];
			size_t j = i;
			for (; j > 0 && alt_str_cmp(x.key, r[j - 1].key) < 0; j--)
				r[j] = r[j - 1];
			r[j] = x;
		}
	} else {
		merge_sort(r, tmp, n);
		free(tmp);
	}
	size_t m = 0;
	for (size_t i = 0; i < n; i++) {
		if (m && alt_str_eq(r[m - 1].key, r[i].key))
			r[m - 1] = r[i];
		else
			r[m++] = r[i];
	}
	return m;
}

const alt_tar_record *alt__tar_find_record(const alt_tar_record *r, size_t n, alt_str key) {
	size_t lo = 0, hi = n;
	while (lo < hi) {
		size_t mid = lo + (hi - lo) / 2;
		int c = alt_str_cmp(r[mid].key, key);
		if (c == 0)
			return &r[mid];
		if (c < 0)
			lo = mid + 1;
		else
			hi = mid;
	}
	return 0;
}

bool alt_tar_header_pax(const alt_tar_header *h, alt_str key, alt_str *value) {
	/* the reader's records are sorted; a writer's header may not be */
	for (size_t i = h->npax; i-- > 0;)
		if (alt_str_eq(h->pax[i].key, key)) {
			if (value)
				*value = h->pax[i].value;
			return true;
		}
	return false;
}
