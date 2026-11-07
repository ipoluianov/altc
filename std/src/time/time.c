/* Clocks, calendar arithmetic, Go-style layouts and durations.

   The calendar conversions use Howard Hinnant's days_from_civil and
   civil_from_days algorithms (public domain). The layout grammar is that
   of Go's time package (BSD-3-Clause, see LICENSE-Go): nextStdChunk's
   recognition of the reference values, translated to C. */
#include <alt/errors.h>
#include <alt/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ---- clocks ---- */

alt_time alt_now(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (alt_time)ts.tv_sec * ALT_SEC + ts.tv_nsec;
}

alt_time alt_deadline(alt_dur d) {
	alt_time now = alt_now();
	if (d <= 0)
		return now;
	if (d >= ALT_FOREVER - now)
		return ALT_FOREVER;
	return now + d;
}

alt_dur alt_until(alt_time t) {
	if (t == ALT_FOREVER)
		return ALT_FOREVER;
	alt_time now = alt_now();
	if (t == ALT_NOWAIT)
		return INT64_MIN;
	return t - now;
}

alt_dur alt_since(alt_time t) { return alt_now() - t; }

alt_wall alt_wall_now(void) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (alt_wall)ts.tv_sec * ALT_SEC + ts.tv_nsec;
}

/* ---- zones ---- */

const alt_zone alt_zone_utc = {0, "UTC"};

alt_zone alt_zone_fixed(const char *name, int offset) {
	alt_zone z = {offset, ""};
	if (name) {
		strncpy(z.name, name, sizeof z.name - 1);
		z.name[sizeof z.name - 1] = 0;
	}
	return z;
}

void alt_zone_local(alt_wall t, alt_zone *z) {
	time_t s = (time_t)alt_wall_unix(t);
	struct tm tm;
	tzset();
	if (!localtime_r(&s, &tm)) {
		*z = alt_zone_utc;
		return;
	}
	*z = alt_zone_fixed(tm.tm_zone, (int)tm.tm_gmtoff);
}

/* ---- calendar ---- */

static int64_t floordiv(int64_t a, int64_t b) { return a / b - (a % b != 0 && (a < 0) != (b < 0)); }
static int64_t floormod(int64_t a, int64_t b) { return a - floordiv(a, b) * b; }

static int64_t days_from_civil(int64_t y, int m, int d) {
	y -= m <= 2;
	int64_t era = (y >= 0 ? y : y - 399) / 400;
	int64_t yoe = y - era * 400;
	int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
	int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + doe - 719468;
}

static void civil_from_days(int64_t z, int64_t *y, int *m, int *d) {
	z += 719468;
	int64_t era = (z >= 0 ? z : z - 146096) / 146097;
	int64_t doe = z - era * 146097;
	int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	int64_t mp = (5 * doy + 2) / 153;
	*d = (int)(doy - (153 * mp + 2) / 5 + 1);
	*m = (int)(mp < 10 ? mp + 3 : mp - 9);
	*y = yoe + era * 400 + (*m <= 2);
}

static bool leap(int64_t y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); }

static int days_in(int64_t y, int m) {
	static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return m == 2 && leap(y) ? 29 : days[m - 1];
}

void alt_wall_date(alt_wall t, const alt_zone *z, alt_date *d) {
	int off = z ? z->offset : 0;
	int64_t sec = floordiv(t, ALT_SEC) + off;
	d->nsec = (int)floormod(t, ALT_SEC);
	int64_t days = floordiv(sec, 86400);
	int64_t rem = sec - days * 86400;
	d->hour = (int)(rem / 3600);
	d->min = (int)(rem / 60 % 60);
	d->sec = (int)(rem % 60);
	civil_from_days(days, &d->year, &d->month, &d->day);
	d->wday = (int)floormod(days + 4, 7);
	d->yday = (int)(days - days_from_civil(d->year, 1, 1) + 1);
	d->offset = off;
}

alt_wall alt_date_wall(const alt_date *d) {
	int64_t m0 = (int64_t)d->month - 1;
	int64_t y = d->year + floordiv(m0, 12);
	int m = (int)floormod(m0, 12) + 1;
	int64_t days = days_from_civil(y, m, 1) + d->day - 1;
	/* unsigned: dates beyond the range of alt_wall wrap as in Go */
	uint64_t sec = (uint64_t)days * 86400 + (uint64_t)(int64_t)d->hour * 3600 + (uint64_t)(int64_t)d->min * 60 +
		(uint64_t)(int64_t)d->sec - (uint64_t)(int64_t)d->offset;
	return (alt_wall)(sec * (uint64_t)ALT_SEC + (uint64_t)(int64_t)d->nsec);
}

static bool upper_ascii(char c) { return c >= 'A' && c <= 'Z'; }

/* ---- layouts ---- */

enum {
	STD_NONE,
	STD_LONG_MONTH, STD_MONTH, STD_NUM_MONTH, STD_ZERO_MONTH,
	STD_LONG_WEEKDAY, STD_WEEKDAY,
	STD_DAY, STD_UNDER_DAY, STD_ZERO_DAY, STD_UNDER_YDAY, STD_ZERO_YDAY,
	STD_HOUR, STD_HOUR12, STD_ZERO_HOUR12,
	STD_MINUTE, STD_ZERO_MINUTE, STD_SECOND, STD_ZERO_SECOND,
	STD_LONG_YEAR, STD_YEAR, STD_PM_UPPER, STD_PM_LOWER,
	STD_TZ,
	STD_ISO_TZ, STD_ISO_SECONDS_TZ, STD_ISO_SHORT_TZ, STD_ISO_COLON_TZ, STD_ISO_COLON_SECONDS_TZ,
	STD_NUM_TZ, STD_NUM_SECONDS_TZ, STD_NUM_SHORT_TZ, STD_NUM_COLON_TZ, STD_NUM_COLON_SECONDS_TZ,
	STD_FRAC0, STD_FRAC9,
};

typedef struct {
	size_t prefix; /* literal bytes before the element */
	int std;
	size_t len;    /* bytes of the element in the layout */
	int digits;    /* fraction digits */
	char sep;      /* fraction separator */
} chunk;

static bool lower_at(const char *s) { return *s >= 'a' && *s <= 'z'; }
static bool has(const char *s, const char *w) { return strncmp(s, w, strlen(w)) == 0; }

static chunk next_chunk(const char *l) {
	chunk c = {0, STD_NONE, 0, 0, 0};
	for (size_t i = 0; l[i]; i++) {
		const char *p = l + i;
		c.prefix = i;
		switch (p[0]) {
		case 'J':
			if (has(p, "Jan")) {
				if (has(p, "January")) {
					c.std = STD_LONG_MONTH, c.len = 7;
					return c;
				}
				if (!lower_at(p + 3)) {
					c.std = STD_MONTH, c.len = 3;
					return c;
				}
			}
			break;
		case 'M':
			if (has(p, "Mon")) {
				if (has(p, "Monday")) {
					c.std = STD_LONG_WEEKDAY, c.len = 6;
					return c;
				}
				if (!lower_at(p + 3)) {
					c.std = STD_WEEKDAY, c.len = 3;
					return c;
				}
			}
			if (has(p, "MST")) {
				c.std = STD_TZ, c.len = 3;
				return c;
			}
			break;
		case '0':
			if (p[1] >= '1' && p[1] <= '6') {
				static const int zx[] = {STD_ZERO_MONTH, STD_ZERO_DAY, STD_ZERO_HOUR12, STD_ZERO_MINUTE, STD_ZERO_SECOND, STD_YEAR};
				c.std = zx[p[1] - '1'], c.len = 2;
				return c;
			}
			if (p[1] == '0' && p[2] == '2') {
				c.std = STD_ZERO_YDAY, c.len = 3;
				return c;
			}
			break;
		case '1':
			if (p[1] == '5') {
				c.std = STD_HOUR, c.len = 2;
				return c;
			}
			c.std = STD_NUM_MONTH, c.len = 1;
			return c;
		case '2':
			if (has(p, "2006")) {
				c.std = STD_LONG_YEAR, c.len = 4;
				return c;
			}
			c.std = STD_DAY, c.len = 1;
			return c;
		case '_':
			if (p[1] == '2') {
				if (has(p + 1, "2006")) { /* a literal _ and the year */
					c.prefix = i + 1, c.std = STD_LONG_YEAR, c.len = 4;
					return c;
				}
				c.std = STD_UNDER_DAY, c.len = 2;
				return c;
			}
			if (p[1] == '_' && p[2] == '2') {
				c.std = STD_UNDER_YDAY, c.len = 3;
				return c;
			}
			break;
		case '3':
			c.std = STD_HOUR12, c.len = 1;
			return c;
		case '4':
			c.std = STD_MINUTE, c.len = 1;
			return c;
		case '5':
			c.std = STD_SECOND, c.len = 1;
			return c;
		case 'P':
			if (p[1] == 'M') {
				c.std = STD_PM_UPPER, c.len = 2;
				return c;
			}
			break;
		case 'p':
			if (p[1] == 'm') {
				c.std = STD_PM_LOWER, c.len = 2;
				return c;
			}
			break;
		case '-':
		case 'Z': {
			bool iso = p[0] == 'Z';
			static const struct {
				const char *s;
				int num, iso;
			} tz[] = {
				{"070000", STD_NUM_SECONDS_TZ, STD_ISO_SECONDS_TZ},
				{"07:00:00", STD_NUM_COLON_SECONDS_TZ, STD_ISO_COLON_SECONDS_TZ},
				{"0700", STD_NUM_TZ, STD_ISO_TZ},
				{"07:00", STD_NUM_COLON_TZ, STD_ISO_COLON_TZ},
				{"07", STD_NUM_SHORT_TZ, STD_ISO_SHORT_TZ},
			};
			for (size_t k = 0; k < sizeof tz / sizeof *tz; k++)
				if (has(p + 1, tz[k].s)) {
					c.std = iso ? tz[k].iso : tz[k].num, c.len = 1 + strlen(tz[k].s);
					return c;
				}
			break;
		}
		case '.':
		case ',':
			if (p[1] == '0' || p[1] == '9') {
				char ch = p[1];
				size_t j = 1;
				while (p[j] == ch)
					j++;
				if (!(p[j] >= '0' && p[j] <= '9')) {
					c.std = ch == '0' ? STD_FRAC0 : STD_FRAC9;
					c.len = j, c.digits = (int)j - 1, c.sep = p[0];
					return c;
				}
			}
			break;
		}
	}
	c.prefix = strlen(l);
	return c;
}

static const char *const long_months[] = {"January", "February", "March", "April", "May", "June", "July", "August",
	"September", "October", "November", "December"};
static const char *const long_days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static void pad_int(alt_buf *b, int64_t v, int width, char padc) {
	char tmp[24];
	bool neg = v < 0;
	uint64_t u = neg ? 0 - (uint64_t)v : (uint64_t)v;
	int i = sizeof tmp;
	do {
		tmp[--i] = (char)('0' + u % 10);
		u /= 10;
	} while (u);
	if (neg)
		alt_buf_append_byte(b, '-');
	for (int k = (int)sizeof tmp - i; k < width; k++)
		alt_buf_append_byte(b, padc);
	alt_buf_append(b, tmp + i, sizeof tmp - (size_t)i);
}

static void put_tz(alt_buf *b, int std, int off) {
	bool iso = std >= STD_ISO_TZ && std <= STD_ISO_COLON_SECONDS_TZ;
	if (iso && off == 0) {
		alt_buf_append_byte(b, 'Z');
		return;
	}
	int kind = iso ? std - STD_ISO_TZ : std - STD_NUM_TZ; /* 0 hhmm, 1 hhmmss, 2 hh, 3 hh:mm, 4 hh:mm:ss */
	/* as Go: the sign is that of the offset in minutes */
	int zone = off / 60, abs = off;
	if (zone < 0) {
		alt_buf_append_byte(b, '-');
		zone = -zone, abs = -abs;
	} else
		alt_buf_append_byte(b, '+');
	pad_int(b, zone / 60, 2, '0');
	if (kind == 3 || kind == 4)
		alt_buf_append_byte(b, ':');
	if (kind != 2)
		pad_int(b, zone % 60, 2, '0');
	if (kind == 1 || kind == 4) {
		if (kind == 4)
			alt_buf_append_byte(b, ':');
		pad_int(b, abs % 60, 2, '0');
	}
}

int alt_wall_format(alt_buf *b, alt_wall t, const alt_zone *z, const char *layout) {
	alt_date d;
	alt_wall_date(t, z, &d);
	int off = z ? z->offset : 0;
	const char *l = layout;
	while (*l) {
		chunk c = next_chunk(l);
		alt_buf_append(b, l, c.prefix);
		if (c.std == STD_NONE)
			break;
		l += c.prefix + c.len;
		switch (c.std) {
		case STD_YEAR: pad_int(b, d.year % 100, 2, '0'); break;
		case STD_LONG_YEAR: pad_int(b, d.year, 4, '0'); break;
		case STD_MONTH: alt_buf_append(b, long_months[d.month - 1], 3); break;
		case STD_LONG_MONTH: alt_buf_append_cstr(b, long_months[d.month - 1]); break;
		case STD_NUM_MONTH: pad_int(b, d.month, 0, '0'); break;
		case STD_ZERO_MONTH: pad_int(b, d.month, 2, '0'); break;
		case STD_WEEKDAY: alt_buf_append(b, long_days[d.wday], 3); break;
		case STD_LONG_WEEKDAY: alt_buf_append_cstr(b, long_days[d.wday]); break;
		case STD_DAY: pad_int(b, d.day, 0, '0'); break;
		case STD_UNDER_DAY: pad_int(b, d.day, 2, ' '); break;
		case STD_ZERO_DAY: pad_int(b, d.day, 2, '0'); break;
		case STD_UNDER_YDAY: pad_int(b, d.yday, 3, ' '); break;
		case STD_ZERO_YDAY: pad_int(b, d.yday, 3, '0'); break;
		case STD_HOUR: pad_int(b, d.hour, 2, '0'); break;
		case STD_HOUR12: pad_int(b, d.hour % 12 ? d.hour % 12 : 12, 0, '0'); break;
		case STD_ZERO_HOUR12: pad_int(b, d.hour % 12 ? d.hour % 12 : 12, 2, '0'); break;
		case STD_MINUTE: pad_int(b, d.min, 0, '0'); break;
		case STD_ZERO_MINUTE: pad_int(b, d.min, 2, '0'); break;
		case STD_SECOND: pad_int(b, d.sec, 0, '0'); break;
		case STD_ZERO_SECOND: pad_int(b, d.sec, 2, '0'); break;
		case STD_PM_UPPER: alt_buf_append_cstr(b, d.hour >= 12 ? "PM" : "AM"); break;
		case STD_PM_LOWER: alt_buf_append_cstr(b, d.hour >= 12 ? "pm" : "am"); break;
		case STD_TZ:
			if (z && z->name[0])
				alt_buf_append_cstr(b, z->name);
			else if (!z)
				alt_buf_append_cstr(b, "UTC");
			else
				put_tz(b, STD_NUM_TZ, off); /* no name: -0700, as Go */
			break;
		case STD_FRAC0:
		case STD_FRAC9: {
			char digits[10];
			snprintf(digits, sizeof digits, "%09d", d.nsec);
			int n = c.digits > 9 ? 9 : c.digits;
			if (c.std == STD_FRAC9) {
				while (n > 0 && digits[n - 1] == '0')
					n--;
				if (n == 0)
					break;
			}
			alt_buf_append_byte(b, c.sep);
			alt_buf_append(b, digits, (size_t)n);
			break;
		}
		default: put_tz(b, c.std, off); break;
		}
	}
	return b->err;
}

/* ---- parsing (a translation of Go's time.parse with Local = UTC:
   zone abbreviations other than UTC are not resolved) ---- */

static bool isdig(alt_str v, size_t i) { return i < v.n && v.p[i] >= '0' && v.p[i] <= '9'; }

/* getnum: one or two digits (fixed: exactly two) */
static bool getnum(alt_str *v, bool fixed, int *out) {
	if (!isdig(*v, 0))
		return false;
	if (!isdig(*v, 1)) {
		if (fixed)
			return false;
		*out = v->p[0] - '0';
		*v = alt_str_from(*v, 1);
		return true;
	}
	*out = (v->p[0] - '0') * 10 + (v->p[1] - '0');
	*v = alt_str_from(*v, 2);
	return true;
}

static bool getnum3(alt_str *v, bool fixed, int *out) {
	int n = 0;
	size_t i = 0;
	for (; i < 3 && isdig(*v, i); i++)
		n = n * 10 + (v->p[i] - '0');
	if (i == 0 || (fixed && i != 3))
		return false;
	*out = n;
	*v = alt_str_from(*v, i);
	return true;
}

/* atoi of a whole string with an optional sign */
static bool atoi_str(alt_str s, int64_t *out) {
	bool neg = false;
	if (s.n && (s.p[0] == '-' || s.p[0] == '+')) {
		neg = s.p[0] == '-';
		s = alt_str_from(s, 1);
	}
	if (s.n == 0)
		return false;
	uint64_t x = 0;
	for (size_t i = 0; i < s.n; i++) {
		if (s.p[i] < '0' || s.p[i] > '9')
			return false;
		if (x > (UINT64_MAX - 9) / 10)
			return false;
		x = x * 10 + (uint64_t)(s.p[i] - '0');
	}
	*out = neg ? -(int64_t)x : (int64_t)x;
	return true;
}

static void cutspace(alt_str *v) {
	while (v->n && v->p[0] == ' ')
		*v = alt_str_from(*v, 1);
}

/* skip removes the literal prefix from v, runs of spaces matching runs of
   spaces */
static bool skip(alt_str *v, const char *prefix, size_t n) {
	alt_str pre = alt_str_make(prefix, n);
	while (pre.n) {
		if (pre.p[0] == ' ') {
			if (v->n && v->p[0] != ' ')
				return false;
			cutspace(&pre);
			cutspace(v);
			continue;
		}
		if (!v->n || v->p[0] != pre.p[0])
			return false;
		pre = alt_str_from(pre, 1);
		*v = alt_str_from(*v, 1);
	}
	return true;
}

static int lookup(alt_str *v, const char *const *names, int n, size_t len) {
	for (int i = 0; i < n; i++) {
		size_t l = len ? len : strlen(names[i]);
		if (v->n < l)
			continue;
		bool ok = true;
		for (size_t k = 0; k < l && ok; k++) {
			unsigned c1 = (unsigned char)v->p[k], c2 = (unsigned char)names[i][k];
			if (c1 != c2) {
				c1 |= 0x20, c2 |= 0x20;
				ok = c1 == c2 && c1 >= 'a' && c1 <= 'z';
			}
		}
		if (ok) {
			*v = alt_str_from(*v, l);
			return i;
		}
	}
	return -1;
}

/* the length of a signed offset +h or -hh (at most 23), 0 if none */
static size_t signed_offset(alt_str v) {
	if (!v.n || (v.p[0] != '+' && v.p[0] != '-'))
		return 0;
	size_t i = 1;
	uint64_t x = 0;
	while (i < v.n && v.p[i] >= '0' && v.p[i] <= '9') {
		if (x < 1000)
			x = x * 10 + (uint64_t)(v.p[i] - '0');
		i++;
	}
	if (i == 1 || x > 23)
		return 0;
	return i;
}

/* Go's parseTimeZone: the length of a zone abbreviation, 0 if none */
static size_t tz_len(alt_str v) {
	if (v.n < 3)
		return 0;
	if (v.n >= 4 && (alt_str_has_prefix(v, ALT_STR("ChST")) || alt_str_has_prefix(v, ALT_STR("MeST"))))
		return 4;
	if (alt_str_has_prefix(v, ALT_STR("GMT")))
		return 3 + (v.n > 3 ? signed_offset(alt_str_from(v, 3)) : 0);
	if (v.p[0] == '+' || v.p[0] == '-')
		return signed_offset(v);
	size_t up = 0;
	while (up < 6 && up < v.n && upper_ascii(v.p[up]))
		up++;
	switch (up) {
	case 5: return v.p[4] == 'T' ? 5 : 0;
	case 4: return v.p[3] == 'T' || alt_str_has_prefix(v, ALT_STR("WITA")) ? 4 : 0;
	case 3: return 3;
	}
	return 0;
}

/* nbytes counts the separator */
static bool nanoseconds(alt_str v, size_t nbytes, int *ns) {
	if (!v.n || (v.p[0] != '.' && v.p[0] != ','))
		return false;
	if (nbytes > 10)
		nbytes = 10;
	int64_t x;
	if (!atoi_str(alt_str_slice(v, 1, nbytes), &x) || x < 0)
		return false;
	for (size_t i = 0; i < 10 - nbytes; i++)
		x *= 10;
	*ns = (int)x;
	return true;
}

static int days_before(int m) {
	static const int before[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
	return before[m];
}

int alt_wall_parse(alt_str v, const char *layout, alt_wall *t, alt_zone *zout) {
	int64_t year = 0, x;
	int month = -1, day = -1, yday = -1, hour = 0, min = 0, sec = 0, nsec = 0;
	bool pm_set = false, am_set = false, utc = false;
	int zone_offset = -1;
	alt_str zone_name = ALT_STR_NULL;
	const char *l = layout;
	for (;;) {
		chunk c = next_chunk(l);
		if (!skip(&v, l, c.prefix))
			return -ALT_ESYNTAX;
		if (c.std == STD_NONE) {
			if (v.n)
				return -ALT_ESYNTAX;
			break;
		}
		l += c.prefix + c.len;
		bool ok = true, range = false;
		switch (c.std) {
		case STD_YEAR:
			if (v.n < 2 || !atoi_str(alt_str_slice(v, 0, 2), &year)) {
				ok = false;
				break;
			}
			v = alt_str_from(v, 2);
			year += year >= 69 ? 1900 : 2000;
			break;
		case STD_LONG_YEAR:
			if (v.n < 4 || !isdig(v, 0) || !atoi_str(alt_str_slice(v, 0, 4), &year)) {
				ok = false;
				break;
			}
			v = alt_str_from(v, 4);
			break;
		case STD_MONTH: ok = (month = lookup(&v, long_months, 12, 3) + 1) > 0; break;
		case STD_LONG_MONTH: ok = (month = lookup(&v, long_months, 12, 0) + 1) > 0; break;
		case STD_NUM_MONTH:
		case STD_ZERO_MONTH:
			ok = getnum(&v, c.std == STD_ZERO_MONTH, &month);
			range = ok && (month <= 0 || month > 12);
			break;
		case STD_WEEKDAY: ok = lookup(&v, long_days, 7, 3) >= 0; break;
		case STD_LONG_WEEKDAY: ok = lookup(&v, long_days, 7, 0) >= 0; break;
		case STD_DAY:
		case STD_UNDER_DAY:
		case STD_ZERO_DAY:
			if (c.std == STD_UNDER_DAY && v.n && v.p[0] == ' ')
				v = alt_str_from(v, 1);
			ok = getnum(&v, c.std == STD_ZERO_DAY, &day);
			break;
		case STD_UNDER_YDAY:
		case STD_ZERO_YDAY:
			for (int i = 0; i < 2; i++)
				if (c.std == STD_UNDER_YDAY && v.n && v.p[0] == ' ')
					v = alt_str_from(v, 1);
			ok = getnum3(&v, c.std == STD_ZERO_YDAY, &yday);
			break;
		case STD_HOUR:
			ok = getnum(&v, false, &hour);
			range = hour >= 24;
			break;
		case STD_HOUR12:
		case STD_ZERO_HOUR12:
			ok = getnum(&v, c.std == STD_ZERO_HOUR12, &hour);
			range = hour > 12;
			break;
		case STD_MINUTE:
		case STD_ZERO_MINUTE:
			ok = getnum(&v, c.std == STD_ZERO_MINUTE, &min);
			range = min >= 60;
			break;
		case STD_SECOND:
		case STD_ZERO_SECOND:
			ok = getnum(&v, c.std == STD_ZERO_SECOND, &sec);
			if (!ok)
				break;
			if (sec >= 60) {
				range = true;
				break;
			}
			/* a fraction in the value that the layout does not have */
			if (v.n >= 2 && (v.p[0] == '.' || v.p[0] == ',') && isdig(v, 1)) {
				chunk nx = next_chunk(l);
				if (nx.std == STD_FRAC0 || nx.std == STD_FRAC9)
					break;
				size_t n = 2;
				while (isdig(v, n))
					n++;
				ok = nanoseconds(v, n, &nsec);
				v = alt_str_from(v, n);
			}
			break;
		case STD_PM_UPPER:
		case STD_PM_LOWER: {
			if (v.n < 2) {
				ok = false;
				break;
			}
			alt_str w = alt_str_slice(v, 0, 2);
			v = alt_str_from(v, 2);
			bool up = c.std == STD_PM_UPPER;
			if (alt_str_eq_cstr(w, up ? "PM" : "pm"))
				pm_set = true;
			else if (alt_str_eq_cstr(w, up ? "AM" : "am"))
				am_set = true;
			else
				ok = false;
			break;
		}
		case STD_TZ: {
			if (alt_str_has_prefix(v, ALT_STR("UTC"))) {
				utc = true;
				v = alt_str_from(v, 3);
				break;
			}
			size_t n = tz_len(v);
			if (!n) {
				ok = false;
				break;
			}
			zone_name = alt_str_slice(v, 0, n);
			v = alt_str_from(v, n);
			break;
		}
		case STD_FRAC0: {
			size_t nd = 1 + (size_t)c.digits;
			if (v.n < nd) {
				ok = false;
				break;
			}
			ok = nanoseconds(v, nd, &nsec);
			v = alt_str_from(v, nd);
			break;
		}
		case STD_FRAC9: {
			if (v.n < 2 || (v.p[0] != '.' && v.p[0] != ',') || !isdig(v, 1))
				break;
			size_t i = 0;
			while (isdig(v, i + 1))
				i++;
			ok = nanoseconds(v, 1 + i, &nsec);
			v = alt_str_from(v, 1 + i);
			break;
		}
		default: { /* numeric zones */
			int kind = c.std >= STD_NUM_TZ ? c.std - STD_NUM_TZ : c.std - STD_ISO_TZ; /* 0 hhmm 1 hhmmss 2 hh 3 hh:mm 4 hh:mm:ss */
			if (c.std < STD_NUM_TZ && v.n && v.p[0] == 'Z') {
				v = alt_str_from(v, 1);
				utc = true;
				break;
			}
			alt_str hh, mm = ALT_STR("00"), ss = ALT_STR("00");
			char sign;
			size_t need = kind == 3 ? 6 : kind == 2 ? 3 : kind == 4 ? 9 : kind == 1 ? 7 : 5;
			if (v.n < need) {
				ok = false;
				break;
			}
			if ((kind == 3 || kind == 4) && v.p[3] != ':') {
				ok = false;
				break;
			}
			if (kind == 4 && v.p[6] != ':') {
				ok = false;
				break;
			}
			sign = v.p[0];
			hh = alt_str_slice(v, 1, 3);
			if (kind == 3 || kind == 4)
				mm = alt_str_slice(v, 4, 6);
			else if (kind != 2)
				mm = alt_str_slice(v, 3, 5);
			if (kind == 4)
				ss = alt_str_slice(v, 7, 9);
			else if (kind == 1)
				ss = alt_str_slice(v, 5, 7);
			v = alt_str_from(v, need);
			int hr, mi, se;
			ok = getnum(&hh, true, &hr) && getnum(&mm, true, &mi) && getnum(&ss, true, &se);
			range = ok && (hr > 24 || mi > 60 || se > 60);
			zone_offset = (hr * 60 + mi) * 60 + se;
			if (sign == '-')
				zone_offset = -zone_offset;
			else if (sign != '+')
				ok = false;
			break;
		}
		}
		if (range)
			return -ALT_ERANGE;
		if (!ok)
			return -ALT_ESYNTAX;
	}
	if (pm_set && hour < 12)
		hour += 12;
	else if (am_set && hour == 12)
		hour = 0;
	if (yday >= 0) {
		int d = 0, m = 0;
		if (leap(year)) {
			if (yday == 31 + 29)
				m = 2, d = 29;
			else if (yday > 31 + 29)
				yday--;
		}
		if (yday < 1 || yday > 365)
			return -ALT_ERANGE;
		if (m == 0) {
			m = (yday - 1) / 31 + 1;
			if (days_before(m + 1) < yday)
				m++;
			d = yday - days_before(m);
		}
		if ((month >= 0 && month != m) || (day >= 0 && day != d))
			return -ALT_ERANGE;
		month = m, day = d;
	} else {
		if (month < 0)
			month = 1;
		if (day < 0)
			day = 1;
	}
	if (day < 1 || day > days_in(year, month))
		return -ALT_ERANGE;
	alt_date d = {year, month, day, hour, min, sec, nsec, 0, 0, 0};
	alt_zone z = alt_zone_utc;
	if (utc) {
		/* UTC */
	} else if (zone_offset != -1) {
		d.offset = zone_offset;
		if (!(zone_offset == 0 && (zone_name.n == 0 || alt_str_eq_cstr(zone_name, "UTC")))) {
			z = alt_zone_fixed("", zone_offset);
			if (zone_name.n) /* zone_name.p may be NULL */
				memcpy(z.name, zone_name.p, zone_name.n < 15 ? zone_name.n : 15);
			z.name[zone_name.n < 15 ? zone_name.n : 15] = 0;
		}
	} else if (zone_name.n) {
		/* an abbreviation: unknown zones get offset 0 (GMT+h its hours),
		   the instant is that of the date in UTC (as Go does) */
		int off = 0;
		if (zone_name.n > 3 && alt_str_has_prefix(zone_name, ALT_STR("GMT")) && atoi_str(alt_str_from(zone_name, 3), &x))
			off = (int)x * 3600;
		z = alt_zone_fixed("", off);
		memcpy(z.name, zone_name.p, zone_name.n < 15 ? zone_name.n : 15);
		z.name[zone_name.n < 15 ? zone_name.n : 15] = 0;
	}
	*t = alt_date_wall(&d);
	if (zout)
		*zout = z;
	return 0;
}

/* ---- durations ---- */

/* the fraction of v (prec digits), trailing zeros removed, into buf
   ending at end; returns the new start and v divided by 10^prec */
static char *frac(char *end, uint64_t *v, int prec) {
	bool print = false;
	char *p = end;
	for (int i = 0; i < prec; i++) {
		int digit = (int)(*v % 10);
		print = print || digit != 0;
		if (print)
			*--p = (char)('0' + digit);
		*v /= 10;
	}
	if (print)
		*--p = '.';
	return p;
}

static char *uint_back(char *p, uint64_t v) {
	do {
		*--p = (char)('0' + v % 10);
		v /= 10;
	} while (v);
	return p;
}

int alt_dur_format(alt_buf *b, alt_dur d) {
	char buf[40], *end = buf + sizeof buf, *p = end;
	uint64_t u = d < 0 ? 0 - (uint64_t)d : (uint64_t)d;
	if (u < (uint64_t)ALT_SEC) {
		int prec = 0;
		*--p = 's';
		if (u == 0)
			return alt_buf_append_cstr(b, "0s");
		if (u < (uint64_t)ALT_US) {
			*--p = 'n';
		} else if (u < (uint64_t)ALT_MS) {
			prec = 3;
			p -= 2;
			memcpy(p, "\xc2\xb5", 2); /* µ */
		} else {
			prec = 6;
			*--p = 'm';
		}
		p = frac(p, &u, prec);
		p = uint_back(p, u);
	} else {
		*--p = 's';
		p = frac(p, &u, 9);
		p = uint_back(p, u % 60);
		u /= 60;
		if (u > 0) {
			*--p = 'm';
			p = uint_back(p, u % 60);
			u /= 60;
			if (u > 0) {
				*--p = 'h';
				p = uint_back(p, u);
			}
		}
	}
	if (d < 0)
		*--p = '-';
	return alt_buf_append(b, p, (size_t)(end - p));
}

int alt_dur_parse(alt_str s, alt_dur *out) {
	*out = 0;
	bool neg = false;
	if (s.n && (s.p[0] == '-' || s.p[0] == '+')) {
		neg = s.p[0] == '-';
		s = alt_str_from(s, 1);
	}
	if (alt_str_eq_cstr(s, "0"))
		return 0;
	if (s.n == 0)
		return -ALT_ESYNTAX;
	unsigned __int128 total = 0;
	while (s.n) {
		uint64_t whole = 0, fnum = 0, fscale = 1;
		size_t i = 0;
		bool any = false;
		while (i < s.n && s.p[i] >= '0' && s.p[i] <= '9') {
			if (whole > (UINT64_MAX - 9) / 10)
				return -ALT_ERANGE;
			whole = whole * 10 + (uint64_t)(s.p[i] - '0');
			i++, any = true;
		}
		if (i < s.n && s.p[i] == '.') {
			i++;
			while (i < s.n && s.p[i] >= '0' && s.p[i] <= '9') {
				if (fscale < 1000000000000000000ULL) {
					fnum = fnum * 10 + (uint64_t)(s.p[i] - '0');
					fscale *= 10;
				}
				i++, any = true;
			}
		}
		if (!any)
			return -ALT_ESYNTAX;
		size_t u0 = i;
		while (i < s.n && s.p[i] != '.' && !(s.p[i] >= '0' && s.p[i] <= '9'))
			i++;
		alt_str unit = alt_str_slice(s, u0, i);
		uint64_t mul;
		if (alt_str_eq_cstr(unit, "ns"))
			mul = 1;
		else if (alt_str_eq_cstr(unit, "us") || alt_str_eq_cstr(unit, "\xc2\xb5s") || alt_str_eq_cstr(unit, "\xce\xbcs"))
			mul = (uint64_t)ALT_US;
		else if (alt_str_eq_cstr(unit, "ms"))
			mul = (uint64_t)ALT_MS;
		else if (alt_str_eq_cstr(unit, "s"))
			mul = (uint64_t)ALT_SEC;
		else if (alt_str_eq_cstr(unit, "m"))
			mul = (uint64_t)ALT_MIN;
		else if (alt_str_eq_cstr(unit, "h"))
			mul = (uint64_t)ALT_HOUR;
		else
			return -ALT_ESYNTAX;
		total += (unsigned __int128)whole * mul + (unsigned __int128)fnum * mul / fscale;
		if (total > (unsigned __int128)INT64_MAX + neg)
			return -ALT_ERANGE;
		s = alt_str_from(s, i);
	}
	*out = neg ? (alt_dur)(0 - (uint64_t)total) : (alt_dur)total;
	return 0;
}
