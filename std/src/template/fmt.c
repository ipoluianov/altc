/* The part of Go's fmt (print.go, format.go; BSD-3-Clause, see
   LICENSE-Go) that applies to the values of templates: Sprint, Sprintln
   and Sprintf on bools, ints, float64s, complex128s, strings, nil, and
   the map[string]any and []any of JSON. */
#include "template_internal.h"
#include <alt/strconv.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <stdlib.h>
#include <string.h>

void tquote_with(alt_buf *b, alt_str s, char quote, bool ascii);
void tquote_rune(alt_buf *b, int32_t r, bool ascii);

/* ---- values ---- */

/* the capacity Go's append gives a slice of n interfaces grown one by
   one from empty, as encoding/json does (exact up to 512 items) */
static size_t json_cap(size_t n) {
	if (n == 0)
		return 0;
	if (n > 512)
		return n;
	size_t c = 1;
	while (c < n)
		c *= 2;
	return c;
}

tval tjson_val(const alt_json *j, bool iface) {
	tval v;
	memset(&v, 0, sizeof v);
	v.iface = iface;
	if (!j) {
		v.k = iface ? V_NIL : V_INVALID;
		return v;
	}
	switch (j->type) {
	case ALT_JSON_NULL:
		v.k = iface ? V_NIL : V_INVALID;
		break;
	case ALT_JSON_BOOL:
		v.k = V_BOOL;
		v.b = j->b;
		break;
	case ALT_JSON_NUMBER:
		v.k = V_FLOAT;
		v.f = j->num;
		break;
	case ALT_JSON_STRING:
		v.k = V_STRING;
		v.s = j->str;
		break;
	case ALT_JSON_ARRAY:
		v.k = V_SLICE;
		v.j = j;
		v.len = j->n;
		v.cap = json_cap(j->n);
		break;
	case ALT_JSON_OBJECT:
		v.k = V_MAP;
		v.j = j;
		break;
	}
	return v;
}

tval tslice_at(const tval *v, size_t i) {
	size_t k = v->off + i;
	if (k < v->j->n)
		return tjson_val(v->j->items[k], true);
	return tjson_val(0, true);
}

typedef struct {
	const alt_json_member *m;
	size_t i;
} mitem;

static int mitem_cmp(const void *a, const void *b) {
	const mitem *x = a, *y = b;
	int c = alt_str_cmp(x->m->key, y->m->key);
	if (c)
		return c;
	return x->i < y->i ? -1 : x->i > y->i;
}

size_t tmap_sorted(alt_arena *a, const alt_json *obj, const alt_json_member ***out) {
	size_t n = obj->n;
	if (!n) {
		*out = 0;
		return 0;
	}
	mitem *t = malloc(n * sizeof *t);
	for (size_t i = 0; i < n; i++)
		t[i] = (mitem){&obj->members[i], i};
	qsort(t, n, sizeof *t, mitem_cmp);
	const alt_json_member **r = alt_arena_alloc(a, n * sizeof *r);
	size_t k = 0;
	for (size_t i = 0; i < n; i++) {
		if (i + 1 < n && alt_str_eq(t[i].m->key, t[i + 1].m->key))
			continue; /* the last one wins */
		r[k++] = t[i].m;
	}
	free(t);
	*out = r;
	return k;
}

static const char *content_names[] = {"string", "template.CSS", "template.HTML", "template.HTMLAttr", "template.JS",
	"template.JSStr", "template.URL", "template.Srcset"};

const char *tval_type_name(const tval *v) {
	switch (v->k) {
	case V_BOOL: return "bool";
	case V_INT: return "int";
	case V_UINT8: return "uint8";
	case V_FLOAT: return "float64";
	case V_COMPLEX: return "complex128";
	case V_STRING: return v->content < 8 ? content_names[v->content] : "string";
	case V_MAP: return "map[string]interface {}";
	case V_SLICE: return "[]interface {}";
	}
	return "<nil>";
}

const char *tval_static_type_name(const tval *v) {
	if (v->iface)
		return "interface {}";
	return tval_type_name(v);
}

/* ---- the printer ---- */

typedef struct {
	alt_buf *buf;
	bool wid_present, prec_present, minus, plus, sharp, space, zero, plus_v, sharp_v;
	int wid, prec;
	bool erroring;
	bool reordered, good_arg_num;
	alt_arena tmp;
} pp;

static void clearflags(pp *p) {
	p->wid_present = p->prec_present = p->minus = p->plus = p->sharp = p->space = p->zero = p->plus_v = p->sharp_v = false;
	p->wid = p->prec = 0;
}

static void write_padding(pp *p, int n) {
	if (n <= 0)
		return;
	char c = p->zero && !p->minus ? '0' : ' ';
	alt_buf_reserve(p->buf, (size_t)n);
	for (int i = 0; i < n; i++)
		alt_buf_append_byte(p->buf, c);
}

static void pad(pp *p, const char *s, size_t n) {
	if (!p->wid_present || p->wid == 0) {
		alt_buf_append(p->buf, s, n);
		return;
	}
	int width = p->wid - (int)alt_utf8_rune_count(s, n);
	if (!p->minus) {
		write_padding(p, width);
		alt_buf_append(p->buf, s, n);
	} else {
		alt_buf_append(p->buf, s, n);
		write_padding(p, width);
	}
}

static void pad_str(pp *p, alt_str s) { pad(p, s.p, s.n); }
static void pad_cstr(pp *p, const char *s) { pad(p, s, strlen(s)); }

static void fmt_boolean(pp *p, bool v) { pad_cstr(p, v ? "true" : "false"); }

static const char ldigits[] = "0123456789abcdefx";
static const char udigits[] = "0123456789ABCDEFX";

static void fmt_unicode_flags(pp *p, unsigned long long u) {
	char small[80];
	int prec = 4;
	size_t size = sizeof small;
	char *buf = small;
	if (p->prec_present && p->prec > 4) {
		prec = p->prec;
		size_t width = 2 + (size_t)prec + 2 + 4 + 1;
		if (width > size) {
			buf = malloc(width);
			size = width;
		}
	}
	size_t i = size;
	if (p->sharp && u <= 0x10ffff && alt_unicode_is_print((int32_t)u)) {
		buf[--i] = '\'';
		char tmp[4];
		size_t w = alt_utf8_encode(tmp, (int32_t)u);
		i -= w;
		memcpy(buf + i, tmp, w);
		buf[--i] = '\'';
		buf[--i] = ' ';
	}
	while (u >= 16) {
		buf[--i] = udigits[u & 15];
		prec--;
		u >>= 4;
	}
	buf[--i] = udigits[u];
	prec--;
	while (prec > 0) {
		buf[--i] = '0';
		prec--;
	}
	buf[--i] = '+';
	buf[--i] = 'U';
	bool oz = p->zero;
	p->zero = false;
	pad(p, buf + i, size - i);
	p->zero = oz;
	if (buf != small)
		free(buf);
}

void tfmt_unicode(alt_buf *b, int32_t r, bool sharp) {
	pp p;
	memset(&p, 0, sizeof p);
	p.buf = b;
	p.sharp = sharp;
	fmt_unicode_flags(&p, (unsigned long long)(uint32_t)r);
}

static void fmt_integer(pp *p, unsigned long long u, int base, bool is_signed, int verb, const char *digits) {
	bool negative = is_signed && (long long)u < 0;
	if (negative)
		u = -u;
	char small[68];
	char *buf = small;
	size_t size = sizeof small;
	if (p->wid_present || p->prec_present) {
		size_t width = 3 + (size_t)p->wid + (size_t)p->prec;
		if (width > size) {
			buf = malloc(width);
			size = width;
		}
	}
	int prec = 0;
	if (p->prec_present) {
		prec = p->prec;
		if (prec == 0 && u == 0) {
			bool oz = p->zero;
			p->zero = false;
			write_padding(p, p->wid);
			p->zero = oz;
			if (buf != small)
				free(buf);
			return;
		}
	} else if (p->zero && !p->minus && p->wid_present) {
		prec = p->wid;
		if (negative || p->plus || p->space)
			prec--;
	}
	size_t i = size;
	switch (base) {
	case 10:
		while (u >= 10) {
			unsigned long long next = u / 10;
			buf[--i] = (char)('0' + u - next * 10);
			u = next;
		}
		break;
	case 16:
		while (u >= 16) {
			buf[--i] = digits[u & 15];
			u >>= 4;
		}
		break;
	case 8:
		while (u >= 8) {
			buf[--i] = (char)('0' + (u & 7));
			u >>= 3;
		}
		break;
	case 2:
		while (u >= 2) {
			buf[--i] = (char)('0' + (u & 1));
			u >>= 1;
		}
		break;
	}
	buf[--i] = digits[u];
	while (i > 0 && prec > (int)(size - i))
		buf[--i] = '0';
	if (p->sharp) {
		switch (base) {
		case 2:
			buf[--i] = 'b';
			buf[--i] = '0';
			break;
		case 8:
			if (buf[i] != '0')
				buf[--i] = '0';
			break;
		case 16:
			buf[--i] = digits[16];
			buf[--i] = '0';
			break;
		}
	}
	if (verb == 'O') {
		buf[--i] = 'o';
		buf[--i] = '0';
	}
	if (negative)
		buf[--i] = '-';
	else if (p->plus)
		buf[--i] = '+';
	else if (p->space)
		buf[--i] = ' ';
	bool oz = p->zero;
	p->zero = false;
	pad(p, buf + i, size - i);
	p->zero = oz;
	if (buf != small)
		free(buf);
}

static alt_str truncate_str(pp *p, alt_str s) {
	if (p->prec_present) {
		int n = p->prec;
		for (size_t i = 0; i < s.n;) {
			n--;
			if (n < 0)
				return alt_str_slice(s, 0, i);
			int32_t r;
			i += alt_utf8_decode(s.p + i, s.n - i, &r);
		}
	}
	return s;
}

static void fmt_s(pp *p, alt_str s) { pad_str(p, truncate_str(p, s)); }

static void fmt_sx(pp *p, alt_str s, const char *digits) {
	size_t length = s.n;
	if (p->prec_present && (size_t)p->prec < length)
		length = (size_t)p->prec;
	int width = 2 * (int)length;
	if (width > 0) {
		if (p->space) {
			if (p->sharp)
				width *= 2;
			width += (int)length - 1;
		} else if (p->sharp)
			width += 2;
	} else {
		if (p->wid_present)
			write_padding(p, p->wid);
		return;
	}
	if (p->wid_present && p->wid > width && !p->minus)
		write_padding(p, p->wid - width);
	alt_buf *b = p->buf;
	if (p->sharp) {
		alt_buf_append_byte(b, '0');
		alt_buf_append_byte(b, digits[16]);
	}
	for (size_t i = 0; i < length; i++) {
		if (p->space && i > 0) {
			alt_buf_append_byte(b, ' ');
			if (p->sharp) {
				alt_buf_append_byte(b, '0');
				alt_buf_append_byte(b, digits[16]);
			}
		}
		unsigned char c = (unsigned char)s.p[i];
		alt_buf_append_byte(b, digits[c >> 4]);
		alt_buf_append_byte(b, digits[c & 15]);
	}
	if (p->wid_present && p->wid > width && p->minus)
		write_padding(p, p->wid - width);
}

static bool can_backquote(alt_str s) {
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		i += w;
		if (w > 1) {
			if (r == 0xfeff)
				return false;
			continue;
		}
		if (r == 0xfffd)
			return false;
		if ((r < ' ' && r != '\t') || r == '`' || r == 0x7f)
			return false;
	}
	return true;
}

static void fmt_q(pp *p, alt_str s) {
	s = truncate_str(p, s);
	alt_buf q = ALT_BUF_INIT;
	if (p->sharp && can_backquote(s)) {
		alt_buf_append_byte(&q, '`');
		alt_buf_append_str(&q, s);
		alt_buf_append_byte(&q, '`');
	} else
		tquote_with(&q, s, '"', p->plus);
	pad_str(p, alt_buf_str(&q));
	alt_buf_free(&q);
}

static void fmt_c(pp *p, unsigned long long c) {
	int32_t r = c > 0x10ffff ? 0xfffd : (int32_t)c;
	char tmp[4];
	size_t w = alt_utf8_encode(tmp, r);
	pad(p, tmp, w);
}

static void fmt_qc(pp *p, unsigned long long c) {
	int32_t r = c > 0x10ffff ? 0xfffd : (int32_t)c;
	alt_buf q = ALT_BUF_INIT;
	tquote_rune(&q, r, p->plus);
	pad_str(p, alt_buf_str(&q));
	alt_buf_free(&q);
}

/* strconv.AppendFloat(v, verb, prec, 64), with the 'x', 'X' and 'b' forms */
static void format_float(alt_buf *b, double v, int verb, int prec) {
	if (verb != 'x' && verb != 'X' && verb != 'b') {
		alt_format_float(b, v, (char)verb, prec);
		return;
	}
	uint64_t bits;
	memcpy(&bits, &v, 8);
	bool neg = bits >> 63;
	int exp = (int)(bits >> 52) & 0x7ff;
	uint64_t mant = bits & ((1ull << 52) - 1);
	if (exp == 0x7ff) {
		alt_buf_append_cstr(b, mant ? "NaN" : neg ? "-Inf" : "+Inf");
		return;
	}
	if (exp == 0)
		exp++;
	else
		mant |= 1ull << 52;
	exp += -1023;
	if (verb == 'b') {
		if (neg)
			alt_buf_append_byte(b, '-');
		alt_buf_append_uint(b, mant);
		alt_buf_append_byte(b, 'p');
		exp -= 52;
		if (exp >= 0)
			alt_buf_append_byte(b, '+');
		alt_buf_append_int(b, exp);
		return;
	}
	if (mant == 0)
		exp = 0;
	mant <<= 60 - 52;
	while (mant != 0 && (mant & (1ull << 60)) == 0) {
		mant <<= 1;
		exp--;
	}
	if (prec >= 0 && prec < 15) {
		unsigned shift = (unsigned)prec * 4;
		uint64_t extra = (mant << shift) & ((1ull << 60) - 1);
		mant >>= 60 - shift;
		if ((extra | (mant & 1)) > (1ull << 59))
			mant++;
		mant <<= 60 - shift;
		if (mant & (1ull << 61)) {
			mant >>= 1;
			exp++;
		}
	}
	const char *hex = verb == 'X' ? "0123456789ABCDEF" : "0123456789abcdef";
	if (neg)
		alt_buf_append_byte(b, '-');
	alt_buf_append_byte(b, '0');
	alt_buf_append_byte(b, verb);
	alt_buf_append_byte(b, (int)('0' + ((mant >> 60) & 1)));
	mant <<= 4;
	if (prec < 0 && mant != 0) {
		alt_buf_append_byte(b, '.');
		while (mant != 0) {
			alt_buf_append_byte(b, hex[(mant >> 60) & 15]);
			mant <<= 4;
		}
	} else if (prec > 0) {
		alt_buf_append_byte(b, '.');
		for (int i = 0; i < prec; i++) {
			alt_buf_append_byte(b, hex[(mant >> 60) & 15]);
			mant <<= 4;
		}
	}
	alt_buf_append_byte(b, verb == 'X' ? 'P' : 'p');
	if (exp < 0) {
		alt_buf_append_byte(b, '-');
		exp = -exp;
	} else
		alt_buf_append_byte(b, '+');
	if (exp < 10)
		alt_buf_append_byte(b, '0');
	alt_buf_append_int(b, exp);
}

static void fmt_float(pp *p, double v, int verb, int prec) {
	if (p->prec_present)
		prec = p->prec;
	alt_buf nb = ALT_BUF_INIT;
	alt_buf_append_byte(&nb, '+');
	format_float(&nb, v, verb, prec);
	char *num = nb.p;
	size_t n = nb.n;
	if (num[1] == '-' || num[1] == '+') {
		num++;
		n--;
	} else
		num[0] = '+';
	if (p->space && num[0] == '+' && !p->plus)
		num[0] = ' ';
	if (num[1] == 'I' || num[1] == 'N') {
		bool oz = p->zero;
		p->zero = false;
		if (num[1] == 'N' && !p->space && !p->plus) {
			num++;
			n--;
		}
		pad(p, num, n);
		p->zero = oz;
		alt_buf_free(&nb);
		return;
	}
	alt_buf sb = ALT_BUF_INIT;
	if (p->sharp && verb != 'b') {
		int digits = 0;
		switch (verb) {
		case 'v':
		case 'g':
		case 'G':
		case 'x':
			digits = prec;
			if (digits == -1)
				digits = 6;
		}
		char tail[16];
		size_t tn = 0;
		bool has_dot = false, saw_nonzero = false;
		for (size_t i = 1; i < n; i++) {
			char c = num[i];
			if (c == '.') {
				has_dot = true;
				continue;
			}
			if (c == 'p' || c == 'P' || ((c == 'e' || c == 'E') && verb != 'x' && verb != 'X')) {
				tn = n - i < sizeof tail ? n - i : sizeof tail;
				memcpy(tail, num + i, tn);
				n = i;
				break;
			}
			if (c != '0')
				saw_nonzero = true;
			if (saw_nonzero)
				digits--;
		}
		alt_buf_append(&sb, num, n);
		if (!has_dot) {
			if (n == 2 && num[1] == '0')
				digits--;
			alt_buf_append_byte(&sb, '.');
		}
		while (digits > 0) {
			alt_buf_append_byte(&sb, '0');
			digits--;
		}
		alt_buf_append(&sb, tail, tn);
		num = sb.p;
		n = sb.n;
	}
	if (p->plus || num[0] != '+') {
		if (p->zero && !p->minus && p->wid_present && p->wid > (int)n) {
			alt_buf_append_byte(p->buf, num[0]);
			write_padding(p, p->wid - (int)n);
			alt_buf_append(p->buf, num + 1, n - 1);
		} else
			pad(p, num, n);
	} else
		pad(p, num + 1, n - 1);
	alt_buf_free(&sb);
	alt_buf_free(&nb);
}

static void print_arg(pp *p, const tval *arg, int verb);
static void print_value(pp *p, const tval *v, int verb, int depth);

static void write_rune(alt_buf *b, int verb) {
	char tmp[4];
	alt_buf_append(b, tmp, alt_utf8_encode(tmp, verb));
}

/* badVerb: arg is the top-level argument (or NULL), v the value */
static void bad_verb(pp *p, const tval *v, int verb) {
	p->erroring = true;
	alt_buf_append_cstr(p->buf, "%!");
	write_rune(p->buf, verb);
	alt_buf_append_byte(p->buf, '(');
	if (v && v->k != V_INVALID && v->k != V_NIL) {
		alt_buf_append_cstr(p->buf, tval_type_name(v));
		alt_buf_append_byte(p->buf, '=');
		print_arg(p, v, 'v');
	} else
		alt_buf_append_cstr(p->buf, "<nil>");
	alt_buf_append_byte(p->buf, ')');
	p->erroring = false;
}

static void fmt_bool(pp *p, const tval *v, int verb) {
	if (verb == 't' || verb == 'v')
		fmt_boolean(p, v->b);
	else
		bad_verb(p, v, verb);
}

static void fmt0x64(pp *p, unsigned long long v, bool leading0x) {
	bool sharp = p->sharp;
	p->sharp = leading0x;
	fmt_integer(p, v, 16, false, 'v', ldigits);
	p->sharp = sharp;
}

static void fmt_int(pp *p, const tval *a, unsigned long long v, bool is_signed, int verb) {
	switch (verb) {
	case 'v':
		if (p->sharp_v && !is_signed)
			fmt0x64(p, v, true);
		else
			fmt_integer(p, v, 10, is_signed, verb, ldigits);
		break;
	case 'd': fmt_integer(p, v, 10, is_signed, verb, ldigits); break;
	case 'b': fmt_integer(p, v, 2, is_signed, verb, ldigits); break;
	case 'o':
	case 'O': fmt_integer(p, v, 8, is_signed, verb, ldigits); break;
	case 'x': fmt_integer(p, v, 16, is_signed, verb, ldigits); break;
	case 'X': fmt_integer(p, v, 16, is_signed, verb, udigits); break;
	case 'c': fmt_c(p, v); break;
	case 'q': fmt_qc(p, v); break;
	case 'U': fmt_unicode_flags(p, v); break;
	default: bad_verb(p, a, verb);
	}
}

static void fmt_float_verb(pp *p, const tval *a, double v, int verb) {
	switch (verb) {
	case 'v': fmt_float(p, v, 'g', -1); break;
	case 'b':
	case 'g':
	case 'G':
	case 'x':
	case 'X': fmt_float(p, v, verb, -1); break;
	case 'f':
	case 'e':
	case 'E': fmt_float(p, v, verb, 6); break;
	case 'F': fmt_float(p, v, 'f', 6); break;
	default: bad_verb(p, a, verb);
	}
}

static void fmt_complex(pp *p, const tval *a, int verb) {
	switch (verb) {
	case 'v':
	case 'b':
	case 'g':
	case 'G':
	case 'x':
	case 'X':
	case 'f':
	case 'F':
	case 'e':
	case 'E': {
		bool old = p->plus;
		alt_buf_append_byte(p->buf, '(');
		fmt_float_verb(p, a, a->f, verb);
		p->plus = true;
		fmt_float_verb(p, a, a->im, verb);
		alt_buf_append_cstr(p->buf, "i)");
		p->plus = old;
		break;
	}
	default:
		bad_verb(p, a, verb);
	}
}

static void fmt_string(pp *p, const tval *a, int verb) {
	switch (verb) {
	case 'v':
		if (p->sharp_v)
			fmt_q(p, a->s);
		else
			fmt_s(p, a->s);
		break;
	case 's': fmt_s(p, a->s); break;
	case 'x': fmt_sx(p, a->s, ldigits); break;
	case 'X': fmt_sx(p, a->s, udigits); break;
	case 'q': fmt_q(p, a->s); break;
	default: bad_verb(p, a, verb);
	}
}

static void fmt_pointer(pp *p, const tval *v, int verb) {
	if (v->k != V_MAP && v->k != V_SLICE) {
		bad_verb(p, v, verb);
		return;
	}
	unsigned long long u = (unsigned long long)(uintptr_t)v->j;
	if (v->k == V_SLICE)
		u += v->off * 16;
	switch (verb) {
	case 'v':
		if (p->sharp_v) {
			alt_buf_append_byte(p->buf, '(');
			alt_buf_append_cstr(p->buf, tval_type_name(v));
			alt_buf_append_cstr(p->buf, ")(");
			fmt0x64(p, u, true);
			alt_buf_append_byte(p->buf, ')');
		} else
			fmt0x64(p, u, !p->sharp);
		break;
	case 'p':
		fmt0x64(p, u, !p->sharp);
		break;
	case 'b':
	case 'o':
	case 'd':
	case 'x':
	case 'X':
		fmt_int(p, v, u, false, verb);
		break;
	default:
		bad_verb(p, v, verb);
	}
}

/* the value of a non-interface kind */
static void print_concrete(pp *p, const tval *v, int verb, int depth) {
	switch (v->k) {
	case V_BOOL: fmt_bool(p, v, verb); break;
	case V_INT: fmt_int(p, v, (unsigned long long)v->i, true, verb); break;
	case V_UINT8: fmt_int(p, v, (unsigned long long)v->i, false, verb); break;
	case V_FLOAT: fmt_float_verb(p, v, v->f, verb); break;
	case V_COMPLEX: fmt_complex(p, v, verb); break;
	case V_STRING: fmt_string(p, v, verb); break;
	case V_MAP: {
		if (p->sharp_v) {
			alt_buf_append_cstr(p->buf, "map[string]interface {}{");
		} else
			alt_buf_append_cstr(p->buf, "map[");
		const alt_json_member **ms;
		size_t n = tmap_sorted(&p->tmp, v->j, &ms);
		for (size_t i = 0; i < n; i++) {
			if (i > 0) {
				if (p->sharp_v)
					alt_buf_append_cstr(p->buf, ", ");
				else
					alt_buf_append_byte(p->buf, ' ');
			}
			tval key;
			memset(&key, 0, sizeof key);
			key.k = V_STRING;
			key.s = ms[i]->key;
			print_value(p, &key, verb, depth + 1);
			alt_buf_append_byte(p->buf, ':');
			tval e = tjson_val(ms[i]->value, true);
			print_value(p, &e, verb, depth + 1);
		}
		alt_buf_append_byte(p->buf, p->sharp_v ? '}' : ']');
		break;
	}
	case V_SLICE:
		if (p->sharp_v)
			alt_buf_append_cstr(p->buf, "[]interface {}{");
		else
			alt_buf_append_byte(p->buf, '[');
		for (size_t i = 0; i < v->len; i++) {
			if (i > 0) {
				if (p->sharp_v)
					alt_buf_append_cstr(p->buf, ", ");
				else
					alt_buf_append_byte(p->buf, ' ');
			}
			tval e = tslice_at(v, i);
			print_value(p, &e, verb, depth + 1);
		}
		alt_buf_append_byte(p->buf, p->sharp_v ? '}' : ']');
		break;
	}
}

static void print_value(pp *p, const tval *v, int verb, int depth) {
	if (v->iface) {
		if (v->k == V_NIL || v->k == V_INVALID) {
			if (p->sharp_v)
				alt_buf_append_cstr(p->buf, "interface {}(nil)");
			else
				alt_buf_append_cstr(p->buf, "<nil>");
			return;
		}
		tval e = *v;
		e.iface = 0;
		print_value(p, &e, verb, depth + 1);
		return;
	}
	if (v->k == V_INVALID || v->k == V_NIL || v->k == V_MISSING) {
		if (depth == 0)
			alt_buf_append_cstr(p->buf, "<invalid reflect.Value>");
		else if (verb == 'v')
			alt_buf_append_cstr(p->buf, "<nil>");
		else
			bad_verb(p, 0, verb);
		return;
	}
	print_concrete(p, v, verb, depth);
}

static void print_arg(pp *p, const tval *arg, int verb) {
	if (arg->k == V_INVALID || arg->k == V_NIL || arg->k == V_MISSING) {
		if (verb == 'T' || verb == 'v')
			pad_cstr(p, "<nil>");
		else
			bad_verb(p, 0, verb);
		return;
	}
	if (verb == 'T') {
		fmt_s(p, alt_str_cstr(tval_type_name(arg)));
		return;
	}
	if (verb == 'p') {
		fmt_pointer(p, arg, 'p');
		return;
	}
	tval v = *arg;
	v.iface = 0;
	print_concrete(p, &v, verb, 0);
}

static void pp_init(pp *p, alt_buf *b) {
	memset(p, 0, sizeof *p);
	p->buf = b;
	alt_arena_init(&p->tmp, 0);
}

static bool is_string_arg(const tval *v) { return v->k == V_STRING; }

void tfmt_sprint(alt_buf *b, const tval *a, size_t n) {
	pp p;
	pp_init(&p, b);
	bool prev = false;
	for (size_t i = 0; i < n; i++) {
		bool is = is_string_arg(&a[i]);
		if (i > 0 && !is && !prev)
			alt_buf_append_byte(b, ' ');
		print_arg(&p, &a[i], 'v');
		prev = is;
	}
	alt_arena_free(&p.tmp);
}

void tfmt_sprintln(alt_buf *b, const tval *a, size_t n) {
	pp p;
	pp_init(&p, b);
	for (size_t i = 0; i < n; i++) {
		if (i > 0)
			alt_buf_append_byte(b, ' ');
		print_arg(&p, &a[i], 'v');
	}
	alt_buf_append_byte(b, '\n');
	alt_arena_free(&p.tmp);
}

static bool too_large(int x) { return x > 1000000 || x < -1000000; }

static int parsenum(alt_str s, size_t start, size_t end, bool *isnum, size_t *newi) {
	int num = 0;
	*isnum = false;
	if (start >= end) {
		*newi = end;
		return 0;
	}
	size_t i;
	for (i = start; i < end && s.p[i] >= '0' && s.p[i] <= '9'; i++) {
		if (too_large(num)) {
			*isnum = false;
			*newi = end;
			return 0;
		}
		num = num * 10 + (s.p[i] - '0');
		*isnum = true;
	}
	*newi = i;
	return num;
}

static int int_from_arg(const tval *a, size_t n, size_t *argnum, bool *isint) {
	int num = 0;
	*isint = false;
	if (*argnum < n) {
		const tval *v = &a[*argnum];
		if (v->k == V_INT || v->k == V_UINT8) {
			num = v->i > 2000000 ? 2000000 : v->i < -2000000 ? -2000000 : (int)v->i;
			*isint = true;
		}
		(*argnum)++;
		if (too_large(num)) {
			num = 0;
			*isint = false;
		}
	}
	return num;
}

static bool parse_arg_number(alt_str f, int *index, size_t *wid) {
	if (f.n < 3) {
		*wid = 1;
		return false;
	}
	for (size_t i = 1; i < f.n; i++) {
		if (f.p[i] == ']') {
			bool ok;
			size_t newi;
			int width = parsenum(f, 1, i, &ok, &newi);
			if (!ok || newi != i) {
				*wid = i + 1;
				*index = 0;
				return false;
			}
			*index = width - 1;
			*wid = i + 1;
			return true;
		}
	}
	*wid = 1;
	return false;
}

static size_t arg_number(pp *p, size_t argnum, alt_str format, size_t *i, size_t numargs, bool *found) {
	*found = false;
	if (format.n <= *i || format.p[*i] != '[')
		return argnum;
	p->reordered = true;
	int index = 0;
	size_t wid;
	bool ok = parse_arg_number(alt_str_from(format, *i), &index, &wid);
	if (ok && 0 <= index && (size_t)index < numargs) {
		*i += wid;
		*found = true;
		return (size_t)index;
	}
	p->good_arg_num = false;
	*i += wid;
	*found = ok;
	return argnum;
}

void tfmt_sprintf(alt_buf *b, alt_str format, const tval *a, size_t n) {
	pp p;
	pp_init(&p, b);
	size_t end = format.n, argnum = 0;
	bool after_index = false;
	for (size_t i = 0; i < end;) {
		p.good_arg_num = true;
		size_t lasti = i;
		while (i < end && format.p[i] != '%')
			i++;
		if (i > lasti)
			alt_buf_append(b, format.p + lasti, i - lasti);
		if (i >= end)
			break;
		i++;
		clearflags(&p);
		bool done = false;
		for (; i < end; i++) {
			char c = format.p[i];
			if (c == '#')
				p.sharp = true;
			else if (c == '0')
				p.zero = true;
			else if (c == '+')
				p.plus = true;
			else if (c == '-')
				p.minus = true;
			else if (c == ' ')
				p.space = true;
			else {
				if ('a' <= c && c <= 'z' && argnum < n) {
					if (c == 'v' || c == 'w') {
						p.sharp_v = p.sharp;
						p.sharp = false;
						p.plus_v = p.plus;
						p.plus = false;
					}
					if (c == 'w')
						bad_verb(&p, &a[argnum], 'w');
					else
						print_arg(&p, &a[argnum], c);
					argnum++;
					i++;
					done = true;
				}
				break;
			}
		}
		if (done)
			continue;
		argnum = arg_number(&p, argnum, format, &i, n, &after_index);
		if (i < end && format.p[i] == '*') {
			i++;
			p.wid = int_from_arg(a, n, &argnum, &p.wid_present);
			if (!p.wid_present)
				alt_buf_append_cstr(b, "%!(BADWIDTH)");
			if (p.wid < 0) {
				p.wid = -p.wid;
				p.minus = true;
				p.zero = false;
			}
			after_index = false;
		} else {
			p.wid = parsenum(format, i, end, &p.wid_present, &i);
			if (after_index && p.wid_present)
				p.good_arg_num = false;
		}
		if (i + 1 < end && format.p[i] == '.') {
			i++;
			if (after_index)
				p.good_arg_num = false;
			argnum = arg_number(&p, argnum, format, &i, n, &after_index);
			if (i < end && format.p[i] == '*') {
				i++;
				p.prec = int_from_arg(a, n, &argnum, &p.prec_present);
				if (p.prec < 0) {
					p.prec = 0;
					p.prec_present = false;
				}
				if (!p.prec_present)
					alt_buf_append_cstr(b, "%!(BADPREC)");
				after_index = false;
			} else {
				p.prec = parsenum(format, i, end, &p.prec_present, &i);
				if (!p.prec_present) {
					p.prec = 0;
					p.prec_present = true;
				}
			}
		}
		if (!after_index)
			argnum = arg_number(&p, argnum, format, &i, n, &after_index);
		if (i >= end) {
			alt_buf_append_cstr(b, "%!(NOVERB)");
			break;
		}
		int32_t verb;
		size_t size = alt_utf8_decode(format.p + i, end - i, &verb);
		i += size;
		if (verb == '%')
			alt_buf_append_byte(b, '%');
		else if (!p.good_arg_num) {
			alt_buf_append_cstr(b, "%!");
			write_rune(b, verb);
			alt_buf_append_cstr(b, "(BADINDEX)");
		} else if (argnum >= n) {
			alt_buf_append_cstr(b, "%!");
			write_rune(b, verb);
			alt_buf_append_cstr(b, "(MISSING)");
		} else {
			if (verb == 'v' || verb == 'w') {
				p.sharp_v = p.sharp;
				p.sharp = false;
				p.plus_v = p.plus;
				p.plus = false;
			}
			if (verb == 'w')
				bad_verb(&p, &a[argnum], 'w');
			else
				print_arg(&p, &a[argnum], verb);
			argnum++;
		}
	}
	if (!p.reordered && argnum < n) {
		clearflags(&p);
		alt_buf_append_cstr(b, "%!(EXTRA ");
		for (size_t i = argnum; i < n; i++) {
			if (i > argnum)
				alt_buf_append_cstr(b, ", ");
			if (a[i].k == V_INVALID || a[i].k == V_NIL)
				alt_buf_append_cstr(b, "<nil>");
			else {
				alt_buf_append_cstr(b, tval_type_name(&a[i]));
				alt_buf_append_byte(b, '=');
				print_arg(&p, &a[i], 'v');
			}
		}
		alt_buf_append_byte(b, ')');
	}
	alt_arena_free(&p.tmp);
}
