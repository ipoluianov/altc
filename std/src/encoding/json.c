/* JSON after Go's encoding/json (scanner.go, decode.go unquote,
   encode.go; BSD-3-Clause, see LICENSE-Go): an iterative parser (the
   depth does not grow the C stack), Go's string decoding and encoding */
#include <alt/encoding/json.h>
#include <alt/strconv.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_DEPTH = 10000 };

/* ---- values ---- */

static alt_json *new_value(alt_arena *a, alt_json_type t) {
	alt_json *v = alt_arena_zalloc(a, sizeof *v);
	if (v) {
		v->type = t;
		v->arena = a;
	}
	return v;
}

alt_json *alt_json_new_null(alt_arena *a) { return new_value(a, ALT_JSON_NULL); }

alt_json *alt_json_new_bool(alt_arena *a, bool b) {
	alt_json *v = new_value(a, ALT_JSON_BOOL);
	if (v)
		v->b = b;
	return v;
}

alt_json *alt_json_new_number(alt_arena *a, double d) {
	alt_json *v = new_value(a, ALT_JSON_NUMBER);
	if (v)
		v->num = d;
	return v;
}

alt_json *alt_json_new_int(alt_arena *a, long long i) {
	alt_json *v = new_value(a, ALT_JSON_NUMBER);
	if (v) {
		v->num = (double)i;
		char *t = alt_arena_printf(a, "%lld", i);
		v->str = t ? alt_str_cstr(t) : ALT_STR_NULL;
	}
	return v;
}

alt_json *alt_json_new_string(alt_arena *a, alt_str s) {
	alt_json *v = new_value(a, ALT_JSON_STRING);
	if (v) {
		v->str = alt_arena_str(a, s);
		if (!v->str.p)
			return 0;
	}
	return v;
}

alt_json *alt_json_new_array(alt_arena *a) { return new_value(a, ALT_JSON_ARRAY); }
alt_json *alt_json_new_object(alt_arena *a) { return new_value(a, ALT_JSON_OBJECT); }

static int grow(alt_json *v, size_t elem) {
	if (v->n < v->cap)
		return 0;
	size_t cap = v->cap ? v->cap * 2 : 4;
	void *p = alt_arena_realloc(v->arena, v->items, v->cap * elem, cap * elem);
	if (!p)
		return -ENOMEM;
	v->items = p;
	v->cap = cap;
	return 0;
}

int alt_json_append(alt_json *arr, alt_json *item) {
	if (!arr || arr->type != ALT_JSON_ARRAY || !item)
		return -EINVAL;
	int rc = grow(arr, sizeof *arr->items);
	if (rc)
		return rc;
	arr->items[arr->n++] = item;
	return 0;
}

static int add_member(alt_json *obj, alt_str key, alt_json *value) {
	int rc = grow(obj, sizeof *obj->members);
	if (rc)
		return rc;
	obj->members[obj->n].key = key;
	obj->members[obj->n].value = value;
	obj->n++;
	return 0;
}

int alt_json_set(alt_json *obj, const char *key, alt_json *value) {
	if (!obj || obj->type != ALT_JSON_OBJECT || !value)
		return -EINVAL;
	alt_str k = alt_str_cstr(key);
	for (size_t i = obj->n; i > 0; i--)
		if (alt_str_eq(obj->members[i - 1].key, k)) {
			obj->members[i - 1].value = value;
			return 0;
		}
	alt_str copy = alt_arena_str(obj->arena, k);
	if (!copy.p)
		return -ENOMEM;
	return add_member(obj, copy, value);
}

alt_json *alt_json_get_str(const alt_json *obj, alt_str key) {
	if (!obj || obj->type != ALT_JSON_OBJECT)
		return 0;
	for (size_t i = obj->n; i > 0; i--)
		if (alt_str_eq(obj->members[i - 1].key, key))
			return obj->members[i - 1].value;
	return 0;
}

alt_json *alt_json_get(const alt_json *obj, const char *key) { return alt_json_get_str(obj, alt_str_cstr(key)); }

alt_json *alt_json_at(const alt_json *arr, size_t i) {
	if (!arr || arr->type != ALT_JSON_ARRAY || i >= arr->n)
		return 0;
	return arr->items[i];
}

alt_json *alt_json_path(const alt_json *v, const char *path) {
	const char *p = path;
	while (v && *p) {
		if (*p == '.') {
			p++;
			continue;
		}
		if (*p == '[') {
			char *end;
			unsigned long i = strtoul(p + 1, &end, 10);
			if (*end != ']')
				return 0;
			v = alt_json_at(v, i);
			p = end + 1;
			continue;
		}
		size_t n = strcspn(p, ".[");
		v = alt_json_get_str(v, alt_str_make(p, n));
		p += n;
	}
	return (alt_json *)v;
}

bool alt_json_get_bool(const alt_json *v, bool *out) {
	if (!v || v->type != ALT_JSON_BOOL)
		return false;
	*out = v->b;
	return true;
}

bool alt_json_get_number(const alt_json *v, double *out) {
	if (!v || v->type != ALT_JSON_NUMBER)
		return false;
	*out = v->num;
	return true;
}

bool alt_json_get_int(const alt_json *v, long long *out) {
	if (!v || v->type != ALT_JSON_NUMBER)
		return false;
	if (v->str.n && alt_str_to_int(v->str, 10, out) == 0)
		return true;
	if (!v->str.n && v->num == floor(v->num) && fabs(v->num) < 9.2e18) {
		*out = (long long)v->num;
		return true;
	}
	return false;
}

bool alt_json_get_string(const alt_json *v, alt_str *out) {
	if (!v || v->type != ALT_JSON_STRING)
		return false;
	*out = v->str;
	return true;
}

/* ---- parsing ---- */

typedef struct {
	const char *p;
	size_t n, i;
	alt_arena *a;
} lexer;

static bool ws(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

static void skip_ws(lexer *l) {
	while (l->i < l->n && ws(l->p[l->i]))
		l->i++;
}

static int hexval(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int getu4(const char *p, size_t left) {
	if (left < 6 || p[0] != '\\' || p[1] != 'u')
		return -1;
	int v = 0;
	for (int k = 2; k < 6; k++) {
		int h = hexval(p[k]);
		if (h < 0)
			return -1;
		v = v << 4 | h;
	}
	return v;
}

/* a string at l->i (at the quote): its decoded value */
static int lex_string(lexer *l, alt_str *out) {
	size_t start = ++l->i;
	bool plain = true;
	/* the end, validating the syntax */
	for (;;) {
		if (l->i >= l->n)
			return -ALT_EJSON_SYNTAX;
		unsigned char c = (unsigned char)l->p[l->i];
		if (c == '"')
			break;
		if (c < 0x20)
			return -ALT_EJSON_SYNTAX;
		if (c == '\\') {
			plain = false;
			if (l->i + 1 >= l->n)
				return -ALT_EJSON_SYNTAX;
			char e = l->p[l->i + 1];
			if (e == 'u') {
				if (l->i + 5 >= l->n)
					return -ALT_EJSON_SYNTAX;
				for (int k = 2; k < 6; k++)
					if (hexval(l->p[l->i + (size_t)k]) < 0)
						return -ALT_EJSON_SYNTAX;
				l->i += 6;
				continue;
			}
			if (!strchr("\"\\/bfnrt", e) || !e)
				return -ALT_EJSON_SYNTAX;
			l->i += 2;
			continue;
		}
		if (c >= 0x80)
			plain = false; /* UTF-8 is checked while decoding */
		l->i++;
	}
	size_t end = l->i++;
	alt_str raw = alt_str_make(l->p + start, end - start);
	if (plain && alt_utf8_valid(raw.p, raw.n)) {
		*out = raw;
		return 0;
	}
	char *buf = alt_arena_alloc(l->a, raw.n * 3 + 1);
	if (!buf)
		return -ENOMEM;
	size_t w = 0;
	for (size_t r = 0; r < raw.n;) {
		unsigned char c = (unsigned char)raw.p[r];
		if (c == '\\') {
			char e = raw.p[r + 1];
			switch (e) {
			case 'b': buf[w++] = '\b'; r += 2; continue;
			case 'f': buf[w++] = '\f'; r += 2; continue;
			case 'n': buf[w++] = '\n'; r += 2; continue;
			case 'r': buf[w++] = '\r'; r += 2; continue;
			case 't': buf[w++] = '\t'; r += 2; continue;
			case 'u': break;
			default: buf[w++] = e; r += 2; continue;
			}
			int32_t rr = getu4(raw.p + r, raw.n - r);
			r += 6;
			if (rr >= 0xD800 && rr < 0xE000) {
				int32_t rr1 = getu4(raw.p + r, raw.n - r);
				if (rr < 0xDC00 && rr1 >= 0xDC00 && rr1 < 0xE000) {
					rr = 0x10000 + ((rr - 0xD800) << 10) + (rr1 - 0xDC00);
					r += 6;
				} else
					rr = ALT_UTF8_RUNE_ERROR;
			}
			w += alt_utf8_encode(buf + w, rr);
			continue;
		}
		if (c < 0x80) {
			buf[w++] = (char)c;
			r++;
			continue;
		}
		int32_t rr;
		size_t k = alt_utf8_decode(raw.p + r, raw.n - r, &rr);
		if (rr == ALT_UTF8_RUNE_ERROR && k == 1) {
			w += alt_utf8_encode(buf + w, ALT_UTF8_RUNE_ERROR);
			r++;
			continue;
		}
		memcpy(buf + w, raw.p + r, k);
		w += k;
		r += k;
	}
	buf[w] = 0;
	*out = alt_str_make(buf, w);
	return 0;
}

static bool digit(char c) { return c >= '0' && c <= '9'; }

static int lex_number(lexer *l, alt_json *v) {
	size_t start = l->i;
	const char *p = l->p;
	if (l->i < l->n && p[l->i] == '-')
		l->i++;
	if (l->i >= l->n || !digit(p[l->i]))
		return -ALT_EJSON_SYNTAX;
	if (p[l->i] == '0')
		l->i++;
	else
		while (l->i < l->n && digit(p[l->i]))
			l->i++;
	if (l->i < l->n && p[l->i] == '.') {
		l->i++;
		if (l->i >= l->n || !digit(p[l->i]))
			return -ALT_EJSON_SYNTAX;
		while (l->i < l->n && digit(p[l->i]))
			l->i++;
	}
	if (l->i < l->n && (p[l->i] == 'e' || p[l->i] == 'E')) {
		l->i++;
		if (l->i < l->n && (p[l->i] == '+' || p[l->i] == '-'))
			l->i++;
		if (l->i >= l->n || !digit(p[l->i]))
			return -ALT_EJSON_SYNTAX;
		while (l->i < l->n && digit(p[l->i]))
			l->i++;
	}
	v->str = alt_str_make(p + start, l->i - start);
	alt_parse_float(v->str, &v->num); /* out of range: +-Inf */
	return 0;
}

static bool lex_word(lexer *l, const char *w) {
	size_t n = strlen(w);
	if (l->n - l->i < n || memcmp(l->p + l->i, w, n) != 0)
		return false;
	l->i += n;
	return true;
}

int alt_json_parse(alt_arena *a, alt_str text, alt_json **out, size_t *err_offset) {
	lexer l = {text.p, text.n, 0, a};
	alt_json **stack = malloc(64 * sizeof *stack);
	size_t depth = 0, cap = 64;
	alt_str pending_key = ALT_STR_NULL;
	alt_json *root = 0;
	int rc = 0;
	if (!stack)
		return -ENOMEM;
	for (;;) {
		skip_ws(&l);
		alt_json *top = depth ? stack[depth - 1] : 0;
		/* in an object: a key first, or the end */
		if (top && top->type == ALT_JSON_OBJECT && !pending_key.p) {
			if (l.i < l.n && l.p[l.i] == '}' && top->n == 0) {
				l.i++;
				goto closed;
			}
			if (l.i >= l.n || l.p[l.i] != '"') {
				rc = -ALT_EJSON_SYNTAX;
				break;
			}
			if ((rc = lex_string(&l, &pending_key)) < 0)
				break;
			if (!pending_key.p)
				pending_key = ALT_STR("");
			skip_ws(&l);
			if (l.i >= l.n || l.p[l.i] != ':') {
				rc = -ALT_EJSON_SYNTAX;
				break;
			}
			l.i++;
			skip_ws(&l);
		} else if (top && top->type == ALT_JSON_ARRAY && top->n == 0 && l.i < l.n && l.p[l.i] == ']') {
			l.i++;
			goto closed;
		}
		/* a value */
		if (l.i >= l.n) {
			rc = -ALT_EJSON_SYNTAX;
			break;
		}
		alt_json *v;
		char c = l.p[l.i];
		bool container = false;
		if (c == '{' || c == '[') {
			v = new_value(a, c == '{' ? ALT_JSON_OBJECT : ALT_JSON_ARRAY);
			l.i++;
			container = true;
		} else if (c == '"') {
			v = new_value(a, ALT_JSON_STRING);
			if (v && (rc = lex_string(&l, &v->str)) < 0)
				break;
		} else if (c == '-' || digit(c)) {
			v = new_value(a, ALT_JSON_NUMBER);
			if (v && (rc = lex_number(&l, v)) < 0)
				break;
		} else if (lex_word(&l, "true")) {
			v = alt_json_new_bool(a, true);
		} else if (lex_word(&l, "false")) {
			v = alt_json_new_bool(a, false);
		} else if (lex_word(&l, "null")) {
			v = alt_json_new_null(a);
		} else {
			rc = -ALT_EJSON_SYNTAX;
			break;
		}
		if (!v) {
			rc = -ENOMEM;
			break;
		}
		if (top) {
			if (top->type == ALT_JSON_OBJECT) {
				rc = add_member(top, pending_key, v);
				pending_key = ALT_STR_NULL;
			} else
				rc = alt_json_append(top, v);
			if (rc)
				break;
		} else
			root = v;
		if (container) {
			if (depth == MAX_DEPTH) {
				rc = -ALT_EJSON_DEPTH;
				break;
			}
			if (depth == cap) {
				cap *= 2;
				alt_json **ns = realloc(stack, cap * sizeof *ns);
				if (!ns) {
					rc = -ENOMEM;
					break;
				}
				stack = ns;
			}
			stack[depth++] = v;
			continue;
		}
		if (!top)
			goto done;
	next:
		/* after a value inside a container: ',' or its end */
		skip_ws(&l);
		top = stack[depth - 1];
		if (l.i < l.n && l.p[l.i] == ',') {
			l.i++;
			continue;
		}
		if (l.i < l.n && l.p[l.i] == (top->type == ALT_JSON_OBJECT ? '}' : ']')) {
			l.i++;
			goto closed;
		}
		rc = -ALT_EJSON_SYNTAX;
		break;
	closed:
		depth--;
		if (depth == 0)
			goto done;
		goto next;
	}
	free(stack);
	if (err_offset)
		*err_offset = l.i;
	return rc ? rc : -ALT_EJSON_SYNTAX;
done:
	free(stack);
	skip_ws(&l);
	if (l.i != l.n) {
		if (err_offset)
			*err_offset = l.i;
		return -ALT_EJSON_SYNTAX; /* after the top-level value */
	}
	*out = root;
	return 0;
}

bool alt_json_valid(alt_str text) {
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_json *v;
	bool ok = alt_json_parse(&a, text, &v, 0) == 0;
	alt_arena_free(&a);
	return ok;
}

/* ---- writing ---- */

int alt_json_quote(alt_buf *b, alt_str s, bool escape_html) {
	static const char hex[] = "0123456789abcdef";
	alt_buf_append_byte(b, '"');
	size_t start = 0;
	for (size_t i = 0; i < s.n;) {
		unsigned char c = (unsigned char)s.p[i];
		if (c < 0x80) {
			bool safe = c >= 0x20 && c != '"' && c != '\\' && !(escape_html && (c == '<' || c == '>' || c == '&'));
			if (safe) {
				i++;
				continue;
			}
			alt_buf_append(b, s.p + start, i - start);
			switch (c) {
			case '\\':
			case '"':
				alt_buf_append_byte(b, '\\');
				alt_buf_append_byte(b, c);
				break;
			case '\b': alt_buf_append_cstr(b, "\\b"); break;
			case '\f': alt_buf_append_cstr(b, "\\f"); break;
			case '\n': alt_buf_append_cstr(b, "\\n"); break;
			case '\r': alt_buf_append_cstr(b, "\\r"); break;
			case '\t': alt_buf_append_cstr(b, "\\t"); break;
			default:
				alt_buf_append_cstr(b, "\\u00");
				alt_buf_append_byte(b, hex[c >> 4]);
				alt_buf_append_byte(b, hex[c & 15]);
			}
			start = ++i;
			continue;
		}
		int32_t r;
		size_t k = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (r == ALT_UTF8_RUNE_ERROR && k == 1) {
			alt_buf_append(b, s.p + start, i - start);
			alt_buf_append_cstr(b, "\\ufffd");
			start = ++i;
			continue;
		}
		if (r == 0x2028 || r == 0x2029) {
			alt_buf_append(b, s.p + start, i - start);
			alt_buf_append_cstr(b, "\\u202");
			alt_buf_append_byte(b, hex[r & 15]);
			i += k;
			start = i;
			continue;
		}
		i += k;
	}
	alt_buf_append(b, s.p + start, s.n - start);
	return alt_buf_append_byte(b, '"');
}

/* Go's float64 encoding: 'f' or, for very small and large values, 'e'
   with a one-digit negative exponent where possible */
static int write_number(alt_buf *b, double f) {
	if (isnan(f) || isinf(f))
		return -EINVAL;
	double abs = fabs(f);
	char fmt = 'f';
	if (abs != 0 && (abs < 1e-6 || abs >= 1e21))
		fmt = 'e';
	size_t start = b->n;
	alt_format_float(b, f, fmt, -1);
	if (fmt == 'e') {
		size_t n = b->n - start;
		char *p = b->p + start;
		if (n >= 4 && p[n - 4] == 'e' && p[n - 3] == '-' && p[n - 2] == '0') {
			p[n - 2] = p[n - 1];
			alt_buf_truncate(b, b->n - 1);
		}
	}
	return b->err;
}

static void newline(alt_buf *b, const alt_json_opts *o, int depth) {
	alt_buf_append_byte(b, '\n');
	if (o->prefix)
		alt_buf_append_cstr(b, o->prefix);
	for (int i = 0; i < depth; i++)
		alt_buf_append_cstr(b, o->indent);
}

static int key_cmp(const void *x, const void *y) {
	const alt_json_member *a = x, *c = y;
	int r = alt_str_cmp(a->key, c->key);
	if (r)
		return r;
	return a < c ? -1 : a > c; /* stable by position */
}

static int write_value(alt_buf *b, const alt_json *v, const alt_json_opts *o, int depth) {
	bool html = !o->no_escape_html;
	switch (v->type) {
	case ALT_JSON_NULL: return alt_buf_append_cstr(b, "null");
	case ALT_JSON_BOOL: return alt_buf_append_cstr(b, v->b ? "true" : "false");
	case ALT_JSON_NUMBER: return write_number(b, v->num);
	case ALT_JSON_STRING: return alt_json_quote(b, v->str, html);
	case ALT_JSON_ARRAY:
		alt_buf_append_byte(b, '[');
		for (size_t i = 0; i < v->n; i++) {
			if (i)
				alt_buf_append_byte(b, ',');
			if (o->indent)
				newline(b, o, depth + 1);
			int rc = write_value(b, v->items[i], o, depth + 1);
			if (rc)
				return rc;
		}
		if (o->indent && v->n)
			newline(b, o, depth);
		return alt_buf_append_byte(b, ']');
	case ALT_JSON_OBJECT: {
		const alt_json_member *m = v->members;
		alt_json_member *sorted = 0;
		size_t n = v->n;
		if (o->sort_keys && n > 1) {
			sorted = malloc(n * sizeof *sorted);
			if (!sorted)
				return -ENOMEM;
			/* the last of each key, then sorted */
			size_t k = 0;
			for (size_t i = 0; i < n; i++) {
				bool later = false;
				for (size_t j = i + 1; j < n && !later; j++)
					later = alt_str_eq(v->members[i].key, v->members[j].key);
				if (!later)
					sorted[k++] = v->members[i];
			}
			qsort(sorted, k, sizeof *sorted, key_cmp);
			m = sorted;
			n = k;
		}
		alt_buf_append_byte(b, '{');
		int rc = 0;
		for (size_t i = 0; i < n && !rc; i++) {
			if (i)
				alt_buf_append_byte(b, ',');
			if (o->indent)
				newline(b, o, depth + 1);
			alt_json_quote(b, m[i].key, html);
			alt_buf_append_cstr(b, o->indent ? ": " : ":");
			rc = write_value(b, m[i].value, o, depth + 1);
		}
		free(sorted);
		if (rc)
			return rc;
		if (o->indent && n)
			newline(b, o, depth);
		return alt_buf_append_byte(b, '}');
	}
	}
	return -EINVAL;
}

int alt_json_write(alt_buf *b, const alt_json *v, const alt_json_opts *opts) {
	alt_json_opts none = {0};
	size_t start = b->n;
	int rc = write_value(b, v, opts ? opts : &none, 0);
	if (rc)
		alt_buf_truncate(b, start);
	return rc ? rc : b->err;
}

/* ---- the streaming writer ---- */

void alt_json_enc_init(alt_json_enc *e, alt_buf *b) {
	memset(e, 0, sizeof *e);
	e->b = b;
	e->escape_html = true;
	e->first[0] = true;
}

static void before_value(alt_json_enc *e) {
	if (e->err)
		return;
	if (e->depth > 0 && e->in_object[e->depth] && !e->after_key) {
		e->err = -EINVAL; /* a value in an object needs a key */
		return;
	}
	if (!e->after_key && !e->first[e->depth])
		alt_buf_append_byte(e->b, ',');
	e->first[e->depth] = false;
	e->after_key = false;
}

static void begin(alt_json_enc *e, char c, bool obj) {
	before_value(e);
	if (e->err)
		return;
	if (e->depth == 63) {
		e->err = -ALT_EJSON_DEPTH;
		return;
	}
	alt_buf_append_byte(e->b, c);
	e->depth++;
	e->first[e->depth] = true;
	e->in_object[e->depth] = obj;
}

static void end(alt_json_enc *e, char c, bool obj) {
	if (e->err)
		return;
	if (e->depth == 0 || e->in_object[e->depth] != obj || e->after_key) {
		e->err = -EINVAL;
		return;
	}
	alt_buf_append_byte(e->b, c);
	e->depth--;
}

void alt_json_begin_object(alt_json_enc *e) { begin(e, '{', true); }
void alt_json_end_object(alt_json_enc *e) { end(e, '}', true); }
void alt_json_begin_array(alt_json_enc *e) { begin(e, '[', false); }
void alt_json_end_array(alt_json_enc *e) { end(e, ']', false); }

void alt_json_key(alt_json_enc *e, const char *key) {
	if (e->err)
		return;
	if (e->depth == 0 || !e->in_object[e->depth] || e->after_key) {
		e->err = -EINVAL;
		return;
	}
	if (!e->first[e->depth])
		alt_buf_append_byte(e->b, ',');
	e->first[e->depth] = false;
	alt_json_quote(e->b, alt_str_cstr(key), e->escape_html);
	alt_buf_append_byte(e->b, ':');
	e->after_key = true;
}

void alt_json_string(alt_json_enc *e, alt_str s) {
	before_value(e);
	if (!e->err)
		alt_json_quote(e->b, s, e->escape_html);
}

void alt_json_cstring(alt_json_enc *e, const char *s) { alt_json_string(e, alt_str_cstr(s)); }

void alt_json_number(alt_json_enc *e, double d) {
	before_value(e);
	if (!e->err && write_number(e->b, d) < 0)
		e->err = -EINVAL;
}

void alt_json_int(alt_json_enc *e, long long v) {
	before_value(e);
	if (!e->err)
		alt_buf_append_int(e->b, v);
}

void alt_json_bool(alt_json_enc *e, bool b) {
	before_value(e);
	if (!e->err)
		alt_buf_append_cstr(e->b, b ? "true" : "false");
}

void alt_json_null(alt_json_enc *e) {
	before_value(e);
	if (!e->err)
		alt_buf_append_cstr(e->b, "null");
}

int alt_json_enc_err(const alt_json_enc *e) {
	if (e->err)
		return e->err;
	if (e->depth)
		return -EINVAL; /* not closed */
	return e->b->err;
}
