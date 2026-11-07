/* Media types after Go's mime/mediatype.go (BSD-3-Clause, see LICENSE-Go) */
#include <alt/mime.h>
#include <alt/sort.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool tspecial(char c) { return c && strchr("()<>@,;:\\\"/[]?=", c) != 0; }
static bool token_char(char c) { return (unsigned char)c > 0x20 && (unsigned char)c < 0x7f && !tspecial(c); }
static bool is_token(alt_str s) {
	if (!s.n)
		return false;
	for (size_t i = 0; i < s.n; i++)
		if (!token_char(s.p[i]))
			return false;
	return true;
}

/* unicode.IsSpace from the left */
static alt_str trim_left_space(alt_str s) {
	while (s.n) {
		int32_t r;
		size_t k = alt_utf8_decode(s.p, s.n, &r);
		if (!alt_unicode_is_space(r))
			break;
		s.p += k, s.n -= k;
	}
	return s;
}

static alt_str trim_space(alt_str s) {
	s = trim_left_space(s);
	while (s.n) {
		int32_t r;
		size_t k = alt_utf8_decode_last(s.p, s.n, &r);
		if (!alt_unicode_is_space(r))
			break;
		s.n -= k;
	}
	return s;
}

/* strings.ToLower (Unicode) */
static alt_str to_lower(alt_arena *a, alt_str s) {
	alt_buf b = ALT_BUF_INIT;
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t k = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (r == 0xfffd && k == 1) {
			alt_buf_append(&b, "\xef\xbf\xbd", 3);
		} else {
			char e[4];
			alt_buf_append(&b, e, alt_utf8_encode(e, alt_unicode_to_lower(r)));
		}
		i += k;
	}
	alt_str r = alt_arena_str(a, alt_buf_str(&b));
	alt_buf_free(&b);
	return r;
}

static void consume_token(alt_str v, alt_str *tok, alt_str *rest) {
	size_t i = 0;
	while (i < v.n && token_char(v.p[i]))
		i++;
	*tok = alt_str_slice(v, 0, i);
	*rest = alt_str_slice(v, i, v.n);
}

static void consume_value(alt_arena *a, alt_str v, alt_str *value, alt_str *rest) {
	*value = ALT_STR("");
	*rest = v;
	if (!v.n)
		return;
	if (v.p[0] != '"') {
		consume_token(v, value, rest);
		return;
	}
	alt_buf b = ALT_BUF_INIT;
	for (size_t i = 1; i < v.n; i++) {
		char r = v.p[i];
		if (r == '"') {
			*value = alt_arena_str(a, alt_buf_str(&b));
			*rest = alt_str_slice(v, i + 1, v.n);
			alt_buf_free(&b);
			return;
		}
		if (r == '\\' && i + 1 < v.n && tspecial(v.p[i + 1])) {
			alt_buf_append_byte(&b, v.p[i + 1]);
			i++;
			continue;
		}
		if (r == '\r' || r == '\n')
			break;
		alt_buf_append_byte(&b, r);
	}
	alt_buf_free(&b);
	*value = ALT_STR("");
	*rest = v;
}

static bool consume_param(alt_arena *a, alt_str v, alt_str *param, alt_str *value, alt_str *rest) {
	alt_str r = trim_left_space(v);
	if (!r.n || r.p[0] != ';')
		return false;
	r = trim_left_space(alt_str_slice(r, 1, r.n));
	alt_str tok;
	consume_token(r, &tok, &r);
	if (!tok.n)
		return false;
	*param = to_lower(a, tok);
	r = trim_left_space(r);
	if (!r.n || r.p[0] != '=')
		return false;
	r = trim_left_space(alt_str_slice(r, 1, r.n));
	alt_str val, r2;
	consume_value(a, r, &val, &r2);
	if (val.n == 0 && r2.n == r.n && r2.p == r.p)
		return false;
	*value = val;
	*rest = r2;
	return true;
}

static int hexv(char c) {
	return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

static bool percent_unescape(alt_arena *a, alt_str s, alt_str *out) {
	alt_buf b = ALT_BUF_INIT;
	for (size_t i = 0; i < s.n;) {
		if (s.p[i] != '%') {
			alt_buf_append_byte(&b, s.p[i++]);
			continue;
		}
		if (i + 2 >= s.n || hexv(s.p[i + 1]) < 0 || hexv(s.p[i + 2]) < 0) {
			alt_buf_free(&b);
			return false;
		}
		alt_buf_append_byte(&b, hexv(s.p[i + 1]) << 4 | hexv(s.p[i + 2]));
		i += 3;
	}
	*out = alt_arena_str(a, alt_buf_str(&b));
	alt_buf_free(&b);
	return true;
}

static bool decode2231(alt_arena *a, alt_str v, alt_str *out) {
	alt_str charset, rest, lang, vals;
	if (!alt_str_cut_byte(v, '\'', &charset, &rest) || !alt_str_cut_byte(rest, '\'', &lang, &vals))
		return false;
	if (!alt_str_eq_fold(charset, ALT_STR("us-ascii")) && !alt_str_eq_fold(charset, ALT_STR("utf-8")))
		return false;
	return percent_unescape(a, vals, out);
}

typedef struct {
	alt_mime_param *p;
	size_t n, cap;
} plist;

static alt_mime_param *pfind(plist *l, alt_str k) {
	for (size_t i = 0; i < l->n; i++)
		if (alt_str_eq(l->p[i].key, k))
			return &l->p[i];
	return 0;
}

static int padd(alt_arena *a, plist *l, alt_str k, alt_str v) {
	if (l->n == l->cap) {
		size_t cap = l->cap ? l->cap * 2 : 8;
		alt_mime_param *np = alt_arena_realloc(a, l->p, l->cap * sizeof *np, cap * sizeof *np);
		if (!np)
			return -ENOMEM;
		l->p = np;
		l->cap = cap;
	}
	l->p[l->n++] = (alt_mime_param){k, v};
	return 0;
}

static int cmp_param(const void *x, const void *y, void *arg) {
	(void)arg;
	return alt_str_cmp(((const alt_mime_param *)x)->key, ((const alt_mime_param *)y)->key);
}

int alt_mime_parse_media_type(alt_arena *a, alt_str v, alt_str *type, alt_mime_param **params, size_t *n, const char **why) {
	*params = 0;
	*n = 0;
	alt_str base, after;
	if (!alt_str_cut_byte(v, ';', &base, &after))
		base = v;
	alt_str mt = trim_space(to_lower(a, base));
	*type = ALT_STR("");
	/* checkMediaTypeDisposition */
	alt_str tok, rest;
	consume_token(mt, &tok, &rest);
	const char *err = 0;
	if (!tok.n)
		err = "mime: no media type";
	else if (rest.n) {
		if (rest.p[0] != '/')
			err = "mime: expected slash after first token";
		else {
			alt_str sub, r2;
			consume_token(alt_str_slice(rest, 1, rest.n), &sub, &r2);
			if (!sub.n)
				err = "mime: expected token after slash";
			else if (r2.n)
				err = "mime: unexpected content after media subtype";
		}
	}
	if (err) {
		if (why)
			*why = err;
		return -ALT_EMIME;
	}
	*type = mt;
	plist params_l = {0}, cont = {0}; /* cont: the starred keys: "base\0key" */
	alt_str r = alt_str_slice(v, base.n, v.n);
	while (r.n) {
		r = trim_left_space(r);
		if (!r.n)
			break;
		alt_str key, value, rr;
		if (!consume_param(a, r, &key, &value, &rr)) {
			if (alt_str_eq_cstr(trim_space(r), ";"))
				break;
			if (why)
				*why = "mime: invalid media parameter";
			return -ALT_EMIME;
		}
		alt_str kb, ks;
		bool starred = alt_str_cut_byte(key, '*', &kb, &ks);
		plist *pm = starred ? &cont : &params_l;
		alt_mime_param *ex = pfind(pm, key);
		if (ex && !alt_str_eq(ex->value, value)) {
			*type = ALT_STR("");
			if (why)
				*why = "mime: duplicate parameter name";
			return -ALT_EMIME;
		}
		if (!ex && padd(a, pm, key, value) < 0)
			return -ENOMEM;
		r = rr;
	}
	/* the continuations: each base name once */
	for (size_t i = 0; i < cont.n; i++) {
		alt_str kb, ks;
		alt_str_cut_byte(cont.p[i].key, '*', &kb, &ks);
		bool seen = false;
		for (size_t j = 0; j < i && !seen; j++) {
			alt_str b2, s2;
			alt_str_cut_byte(cont.p[j].key, '*', &b2, &s2);
			seen = alt_str_eq(b2, kb);
		}
		if (seen)
			continue;
		char single[300];
		snprintf(single, sizeof single, "%.*s*", (int)kb.n, kb.p);
		alt_mime_param *sp = pfind(&cont, alt_str_cstr(single));
		if (sp) {
			alt_str dec;
			if (decode2231(a, sp->value, &dec)) {
				alt_mime_param *e = pfind(&params_l, kb);
				if (e)
					e->value = dec;
				else if (padd(a, &params_l, kb, dec) < 0)
					return -ENOMEM;
			}
			continue;
		}
		alt_buf b = ALT_BUF_INIT;
		bool valid = false;
		for (int k = 0;; k++) {
			char simple[320], enc[330];
			snprintf(simple, sizeof simple, "%.*s*%d", (int)kb.n, kb.p, k);
			snprintf(enc, sizeof enc, "%s*", simple);
			alt_mime_param *e = pfind(&cont, alt_str_cstr(simple));
			if (e) {
				valid = true;
				alt_buf_append_str(&b, e->value);
				continue;
			}
			e = pfind(&cont, alt_str_cstr(enc));
			if (!e)
				break;
			valid = true;
			alt_str dec;
			if (k == 0) {
				if (decode2231(a, e->value, &dec))
					alt_buf_append_str(&b, dec);
			} else {
				if (!percent_unescape(a, e->value, &dec))
					dec = ALT_STR("");
				alt_buf_append_str(&b, dec);
			}
		}
		if (valid) {
			alt_str val = alt_arena_str(a, alt_buf_str(&b));
			alt_mime_param *e = pfind(&params_l, kb);
			if (e)
				e->value = val;
			else if (padd(a, &params_l, kb, val) < 0) {
				alt_buf_free(&b);
				return -ENOMEM;
			}
		}
		alt_buf_free(&b);
	}
	if (params_l.n)
		alt_sort(params_l.p, params_l.n, sizeof *params_l.p, cmp_param, 0);
	*params = params_l.p;
	*n = params_l.n;
	return 0;
}

alt_str alt_mime_param_get(const alt_mime_param *p, size_t n, const char *key) {
	for (size_t i = 0; i < n; i++)
		if (alt_str_eq_cstr(p[i].key, key))
			return p[i].value;
	return ALT_STR("");
}

static void ascii_lower(alt_buf *b, alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		alt_buf_append_byte(b, s.p[i] >= 'A' && s.p[i] <= 'Z' ? s.p[i] + 32 : s.p[i]);
}

int alt_mime_format_media_type(alt_buf *out, alt_str t, const alt_mime_param *params, size_t n) {
	alt_buf b = ALT_BUF_INIT;
	alt_str major, sub;
	int rc = 0;
	if (!alt_str_cut_byte(t, '/', &major, &sub)) {
		if (!is_token(t))
			rc = -EINVAL;
		else
			ascii_lower(&b, t);
	} else if (!is_token(major) || !is_token(sub))
		rc = -EINVAL;
	else {
		ascii_lower(&b, major);
		alt_buf_append_byte(&b, '/');
		ascii_lower(&b, sub);
	}
	/* the parameters sorted by key */
	alt_mime_param *sorted = n ? malloc(n * sizeof *sorted) : 0;
	if (n && !sorted)
		rc = -ENOMEM;
	if (rc == 0 && n) {
		memcpy(sorted, params, n * sizeof *sorted);
		alt_sort(sorted, n, sizeof *sorted, cmp_param, 0);
	}
	static const char hex[] = "0123456789ABCDEF";
	for (size_t i = 0; rc == 0 && i < n; i++) {
		alt_str k = sorted[i].key, v = sorted[i].value;
		alt_buf_append_cstr(&b, "; ");
		if (!is_token(k)) {
			rc = -EINVAL;
			break;
		}
		ascii_lower(&b, k);
		bool enc = false;
		for (size_t j = 0; j < v.n; j++) {
			unsigned char c = (unsigned char)v.p[j];
			if ((c < ' ' || c > '~') && c != '\t')
				enc = true;
		}
		if (enc) {
			alt_buf_append_cstr(&b, "*=utf-8''");
			for (size_t j = 0; j < v.n; j++) {
				unsigned char c = (unsigned char)v.p[j];
				if (c <= ' ' || c >= 0x7f || c == '*' || c == '\'' || c == '%' || tspecial((char)c)) {
					alt_buf_append_byte(&b, '%');
					alt_buf_append_byte(&b, hex[c >> 4]);
					alt_buf_append_byte(&b, hex[c & 15]);
				} else
					alt_buf_append_byte(&b, c);
			}
			continue;
		}
		alt_buf_append_byte(&b, '=');
		if (is_token(v)) {
			alt_buf_append_str(&b, v);
			continue;
		}
		alt_buf_append_byte(&b, '"');
		for (size_t j = 0; j < v.n; j++) {
			if (v.p[j] == '"' || v.p[j] == '\\')
				alt_buf_append_byte(&b, '\\');
			alt_buf_append_byte(&b, v.p[j]);
		}
		alt_buf_append_byte(&b, '"');
	}
	free(sorted);
	if (rc == 0)
		alt_buf_append(out, b.p, b.n);
	alt_buf_free(&b);
	return rc ? rc : out->err;
}
