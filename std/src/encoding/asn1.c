/* DER, after golang.org/x/crypto/cryptobyte (asn1.go, string.go;
   BSD-3-Clause, see LICENSE-Go) */
#include <alt/encoding/asn1.h>
#include <alt/strconv.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

bool alt_oid_eq(const alt_oid *a, const alt_oid *b) {
	return a->n == b->n && memcmp(a->arc, b->arc, (size_t)a->n * sizeof *a->arc) == 0;
}

int alt_oid_format(alt_buf *b, const alt_oid *o) {
	for (int i = 0; i < o->n; i++) {
		if (i)
			alt_buf_append_byte(b, '.');
		alt_buf_append_uint(b, o->arc[i]);
	}
	return b->err;
}

/* Go's readASN1: short and long lengths (up to four bytes, minimal) */
static bool read_header(alt_der *s, alt_der *out, unsigned *out_tag, bool skip_header) {
	if (s->n < 2)
		return false;
	unsigned tag = s->p[0], lb = s->p[1];
	if ((tag & 0x1f) == 0x1f)
		return false;
	uint64_t length, hl;
	if (!(lb & 0x80)) {
		length = lb + 2;
		hl = 2;
	} else {
		unsigned ll = lb & 0x7f;
		if (ll == 0 || ll > 4 || s->n < 2 + ll)
			return false;
		uint32_t l32 = 0;
		for (unsigned i = 0; i < ll; i++)
			l32 = l32 << 8 | s->p[2 + i];
		if (l32 < 128 || l32 >> ((ll - 1) * 8) == 0)
			return false;
		hl = 2 + ll;
		length = hl + l32;
	}
	if (length > s->n)
		return false;
	if (out_tag)
		*out_tag = tag;
	*out = alt_der_make(s->p + (skip_header ? hl : 0), (size_t)(length - (skip_header ? hl : 0)));
	s->p += length;
	s->n -= (size_t)length;
	return true;
}

bool alt_der_read(alt_der *s, alt_der *out, unsigned tag) {
	alt_der save = *s, o;
	unsigned t;
	if (!read_header(s, &o, &t, true) || t != tag) {
		*s = save;
		return false;
	}
	*out = o;
	return true;
}

bool alt_der_read_element(alt_der *s, alt_der *out, unsigned tag) {
	alt_der save = *s, o;
	unsigned t;
	if (!read_header(s, &o, &t, false) || t != tag) {
		*s = save;
		return false;
	}
	*out = o;
	return true;
}

bool alt_der_read_any(alt_der *s, alt_der *out, unsigned *tag) { return read_header(s, out, tag, true); }
bool alt_der_read_any_element(alt_der *s, alt_der *out, unsigned *tag) { return read_header(s, out, tag, false); }

bool alt_der_peek(alt_der s, unsigned tag) { return s.n > 0 && s.p[0] == tag; }

bool alt_der_skip(alt_der *s, unsigned tag) {
	alt_der x;
	return alt_der_read(s, &x, tag);
}

bool alt_der_read_optional(alt_der *s, alt_der *out, bool *present, unsigned tag) {
	bool here = alt_der_peek(*s, tag);
	if (present)
		*present = here;
	if (here && !alt_der_read(s, out, tag))
		return false;
	if (!here)
		*out = alt_der_make(0, 0);
	return true;
}

bool alt_der_skip_optional(alt_der *s, unsigned tag) {
	if (!alt_der_peek(*s, tag))
		return true;
	return alt_der_skip(s, tag);
}

bool alt_der_read_bool(alt_der *s, bool *out) {
	alt_der save = *s, b;
	if (!alt_der_read(s, &b, ALT_ASN1_BOOLEAN) || b.n != 1 || (b.p[0] != 0 && b.p[0] != 0xff)) {
		*s = save;
		return false;
	}
	*out = b.p[0] == 0xff;
	return true;
}

static bool check_int(alt_der b) {
	if (b.n == 0)
		return false;
	if (b.n == 1)
		return true;
	return !((b.p[0] == 0 && !(b.p[1] & 0x80)) || (b.p[0] == 0xff && (b.p[1] & 0x80)));
}

static bool read_int_contents(alt_der *s, alt_der *b, unsigned tag) {
	alt_der save = *s;
	if (!alt_der_read(s, b, tag) || !check_int(*b)) {
		*s = save;
		return false;
	}
	return true;
}

static bool signed_value(int64_t *out, alt_der n) {
	if (n.n > 8)
		return false;
	uint64_t v = 0;
	for (size_t i = 0; i < n.n; i++)
		v = v << 8 | n.p[i];
	unsigned sh = (unsigned)(64 - n.n * 8);
	if (sh < 64 && sh > 0)
		v = (uint64_t)((int64_t)(v << sh) >> sh);
	*out = (int64_t)v;
	return true;
}

bool alt_der_read_int64(alt_der *s, int64_t *out) {
	alt_der save = *s, b;
	if (!read_int_contents(s, &b, ALT_ASN1_INTEGER) || !signed_value(out, b)) {
		*s = save;
		return false;
	}
	return true;
}

bool alt_der_read_uint64(alt_der *s, uint64_t *out) {
	alt_der save = *s, b;
	if (!read_int_contents(s, &b, ALT_ASN1_INTEGER) || b.n > 9 || (b.n == 9 && b.p[0] != 0) || (b.p[0] & 0x80)) {
		*s = save;
		return false;
	}
	uint64_t v = 0;
	for (size_t i = 0; i < b.n; i++)
		v = v << 8 | b.p[i];
	*out = v;
	return true;
}

bool alt_der_read_uint_bytes(alt_der *s, alt_der *out) {
	alt_der save = *s, b;
	if (!read_int_contents(s, &b, ALT_ASN1_INTEGER) || (b.p[0] & 0x80)) {
		*s = save;
		return false;
	}
	while (b.n > 1 && b.p[0] == 0)
		b.p++, b.n--;
	*out = b;
	return true;
}

bool alt_der_read_int_bytes(alt_der *s, alt_der *out) { return read_int_contents(s, out, ALT_ASN1_INTEGER); }

bool alt_der_read_enum(alt_der *s, int64_t *out) {
	alt_der save = *s, b;
	if (!read_int_contents(s, &b, ALT_ASN1_ENUM) || !signed_value(out, b)) {
		*s = save;
		return false;
	}
	return true;
}

/* Go's readBase128Int: at most four bytes, below 2^31, minimal */
static bool base128(alt_der *s, uint32_t *out) {
	uint32_t ret = 0;
	for (int i = 0; s->n > 0; i++) {
		if (i == 5 || ret >= 1u << (31 - 7))
			return false;
		ret <<= 7;
		uint8_t b = s->p[0];
		s->p++, s->n--;
		if (i == 0 && b == 0x80)
			return false;
		ret |= b & 0x7f;
		if (!(b & 0x80)) {
			*out = ret;
			return true;
		}
	}
	return false;
}

bool alt_der_read_oid_bytes(alt_der *s, alt_der *out) {
	alt_der save = *s, b;
	if (!alt_der_read(s, &b, ALT_ASN1_OID) || b.n == 0)
		goto fail;
	alt_der x = b;
	uint32_t v;
	while (x.n > 0)
		if (!base128(&x, &v))
			goto fail;
	*out = b;
	return true;
fail:
	*s = save;
	return false;
}

bool alt_oid_from_bytes(alt_der b, alt_oid *out) {
	uint32_t v;
	if (b.n == 0 || !base128(&b, &v))
		return false;
	out->arc[0] = v < 80 ? v / 40 : 2;
	out->arc[1] = v < 80 ? v % 40 : v - 80;
	out->n = 2;
	while (b.n > 0) {
		if (out->n == 32 || !base128(&b, &v))
			return false;
		out->arc[out->n++] = v;
	}
	return true;
}

bool alt_der_read_oid(alt_der *s, alt_oid *out) {
	alt_der save = *s, b;
	if (!alt_der_read_oid_bytes(s, &b) || !alt_oid_from_bytes(b, out)) {
		*s = save;
		return false;
	}
	return true;
}

bool alt_der_read_bit_string(alt_der *s, alt_der *bytes, size_t *bits) {
	alt_der save = *s, b;
	if (!alt_der_read(s, &b, ALT_ASN1_BIT_STRING) || b.n == 0)
		goto fail;
	unsigned pad = b.p[0];
	b.p++, b.n--;
	if (pad > 7 || (b.n == 0 && pad != 0) || (b.n > 0 && (b.p[b.n - 1] & ((1u << pad) - 1))))
		goto fail;
	*bytes = b;
	*bits = b.n * 8 - pad;
	return true;
fail:
	*s = save;
	return false;
}

bool alt_der_read_bit_string_bytes(alt_der *s, alt_der *out) {
	alt_der save = *s, b;
	if (!alt_der_read(s, &b, ALT_ASN1_BIT_STRING) || b.n == 0 || b.p[0] != 0) {
		*s = save;
		return false;
	}
	*out = alt_der_make(b.p + 1, b.n - 1);
	return true;
}

/* days from 1970-01-01 to y-m-d (proleptic Gregorian) */
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

static int days_in(int64_t y, int m) {
	static const int dm[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
		return 29;
	return dm[m - 1];
}

static bool digits(const uint8_t *p, int n, int *out) {
	int v = 0;
	for (int i = 0; i < n; i++) {
		if (p[i] < '0' || p[i] > '9')
			return false;
		v = v * 10 + (p[i] - '0');
	}
	*out = v;
	return true;
}

/* the time of a UTCTime or GeneralizedTime, as Go's time.Parse with the
   layout and the check that formatting gives back the same text */
static bool der_time(alt_der b, bool gen, bool secs, int64_t *out) {
	const uint8_t *p = b.p;
	size_t n = b.n, need = (gen ? 4 : 2) + 8 + (secs ? 2 : 0);
	if (n < need + 1)
		return false;
	int year, mon, day, hour, min, sec = 0;
	if (!digits(p, gen ? 4 : 2, &year))
		return false;
	p += gen ? 4 : 2;
	if (!digits(p, 2, &mon) || !digits(p + 2, 2, &day) || !digits(p + 4, 2, &hour) || !digits(p + 6, 2, &min) ||
		(secs && !digits(p + 8, 2, &sec)))
		return false;
	p += secs ? 10 : 8;
	if (!gen)
		year += year < 50 ? 2000 : 1900;
	if (mon < 1 || mon > 12 || day < 1 || day > days_in(year, mon) || hour > 23 || min > 59 || sec > 59)
		return false;
	size_t left = n - need;
	int off = 0;
	if (left == 1 && p[0] == 'Z')
		off = 0;
	else if (left == 5 && (p[0] == '+' || p[0] == '-')) {
		int oh, om;
		if (!digits(p + 1, 2, &oh) || !digits(p + 3, 2, &om) || oh > 24 || om > 59 || (oh == 0 && om == 0))
			return false;
		off = (oh * 60 + om) * 60 * (p[0] == '-' ? -1 : 1);
	} else
		return false;
	*out = days_from_civil(year, mon, day) * 86400 + hour * 3600 + min * 60 + sec - off;
	return true;
}

bool alt_der_read_generalized_time(alt_der *s, int64_t *out) {
	alt_der save = *s, b;
	if (!alt_der_read(s, &b, ALT_ASN1_GENERALIZED_TIME) || !der_time(b, true, true, out)) {
		*s = save;
		return false;
	}
	return true;
}

bool alt_der_read_utc_time(alt_der *s, int64_t *out) {
	alt_der save = *s, b;
	if (!alt_der_read(s, &b, ALT_ASN1_UTC_TIME) || !(der_time(b, false, true, out) || der_time(b, false, false, out))) {
		*s = save;
		return false;
	}
	return true;
}

bool alt_der_read_optional_int64(alt_der *s, int64_t *out, unsigned tag, int64_t def) {
	alt_der save = *s, child;
	bool present;
	if (!alt_der_read_optional(s, &child, &present, tag))
		return false;
	if (!present) {
		*out = def;
		return true;
	}
	if (!alt_der_read_int64(&child, out) || child.n) {
		*s = save;
		return false;
	}
	return true;
}

/* ---- writing ---- */

void alt_der_builder_init(alt_der_builder *d, alt_buf *b) {
	d->b = b;
	d->depth = 0;
	d->err = 0;
}

static void header(alt_buf *b, unsigned tag, size_t n) {
	alt_buf_append_byte(b, (int)tag);
	if (n < 128) {
		alt_buf_append_byte(b, (int)n);
		return;
	}
	int ll = 0;
	for (size_t x = n; x; x >>= 8)
		ll++;
	alt_buf_append_byte(b, 0x80 | ll);
	for (int i = ll - 1; i >= 0; i--)
		alt_buf_append_byte(b, (int)(n >> (8 * i) & 0xff));
}

void alt_der_begin(alt_der_builder *d, unsigned tag) {
	if (d->depth == 32) {
		d->err = -EINVAL;
		return;
	}
	alt_buf_append_byte(d->b, (int)tag);
	alt_buf_append_byte(d->b, 0); /* the length, moved into place at the end */
	d->open[d->depth++] = d->b->n;
}

void alt_der_end(alt_der_builder *d) {
	if (d->depth == 0) {
		d->err = -EINVAL;
		return;
	}
	if (d->b->err)
		return;
	size_t start = d->open[--d->depth], n = d->b->n - start;
	if (n < 128) {
		d->b->p[start - 1] = (char)n;
		return;
	}
	int ll = 0;
	for (size_t x = n; x; x >>= 8)
		ll++;
	if (alt_buf_reserve(d->b, (size_t)ll) < 0)
		return;
	memmove(d->b->p + start + ll, d->b->p + start, n);
	d->b->p[start - 1] = (char)(0x80 | ll);
	for (int i = 0; i < ll; i++)
		d->b->p[start + i] = (char)(n >> (8 * (ll - 1 - i)) & 0xff);
	d->b->n += (size_t)ll;
}

void alt_der_add(alt_der_builder *d, unsigned tag, const void *p, size_t n) {
	header(d->b, tag, n);
	alt_buf_append(d->b, p, n);
}

void alt_der_add_int64(alt_der_builder *d, int64_t v) {
	uint8_t b[8];
	int n = 8;
	for (int i = 0; i < 8; i++)
		b[i] = (uint8_t)((uint64_t)v >> (56 - 8 * i));
	int start = 0;
	while (start < 7 && ((b[start] == 0 && !(b[start + 1] & 0x80)) || (b[start] == 0xff && (b[start + 1] & 0x80))))
		start++;
	alt_der_add(d, ALT_ASN1_INTEGER, b + start, (size_t)(n - start));
}

void alt_der_add_uint_bytes(alt_der_builder *d, const void *p, size_t n) {
	const uint8_t *s = p;
	while (n > 1 && s[0] == 0)
		s++, n--;
	if (n == 0) {
		alt_der_add(d, ALT_ASN1_INTEGER, "", 1);
		return;
	}
	if (s[0] & 0x80) {
		header(d->b, ALT_ASN1_INTEGER, n + 1);
		alt_buf_append_byte(d->b, 0);
		alt_buf_append(d->b, s, n);
	} else
		alt_der_add(d, ALT_ASN1_INTEGER, s, n);
}

static void base128_put(alt_buf *b, uint32_t v) {
	uint8_t t[5];
	int n = 0;
	do
		t[n++] = (uint8_t)(v & 0x7f);
	while (v >>= 7);
	for (int i = n - 1; i >= 0; i--)
		alt_buf_append_byte(b, t[i] | (i ? 0x80 : 0));
}

void alt_der_add_oid(alt_der_builder *d, const alt_oid *o) {
	if (o->n < 2 || o->arc[0] > 2 || (o->arc[0] < 2 && o->arc[1] >= 40)) {
		d->err = -EINVAL;
		return;
	}
	alt_buf t = ALT_BUF_INIT;
	base128_put(&t, o->arc[0] * 40 + o->arc[1]);
	for (int i = 2; i < o->n; i++)
		base128_put(&t, o->arc[i]);
	alt_der_add(d, ALT_ASN1_OID, t.p, t.n);
	alt_buf_free(&t);
}

void alt_der_add_bool(alt_der_builder *d, bool v) { alt_der_add(d, ALT_ASN1_BOOLEAN, v ? "\xff" : "\0", 1); }
void alt_der_add_null(alt_der_builder *d) { alt_der_add(d, ALT_ASN1_NULL, "", 0); }

void alt_der_add_bit_string(alt_der_builder *d, const void *p, size_t n) {
	header(d->b, ALT_ASN1_BIT_STRING, n + 1);
	alt_buf_append_byte(d->b, 0);
	alt_buf_append(d->b, p, n);
}

void alt_der_add_time(alt_der_builder *d, int64_t t) {
	int64_t days = t >= 0 ? t / 86400 : -((-t + 86399) / 86400), rem = t - days * 86400, y;
	int m, dd;
	civil_from_days(days, &y, &m, &dd);
	char buf[32];
	bool utc = y >= 1950 && y < 2050;
	int n = snprintf(buf, sizeof buf, utc ? "%02d%02d%02d%02d%02d%02dZ" : "%04d%02d%02d%02d%02d%02dZ", (int)(utc ? y % 100 : y), m,
		dd, (int)(rem / 3600), (int)(rem / 60 % 60), (int)(rem % 60));
	alt_der_add(d, utc ? ALT_ASN1_UTC_TIME : ALT_ASN1_GENERALIZED_TIME, buf, (size_t)n);
}

int alt_der_builder_err(const alt_der_builder *d) {
	if (d->err)
		return d->err;
	if (d->b->err)
		return d->b->err;
	return d->depth ? -EINVAL : 0;
}
