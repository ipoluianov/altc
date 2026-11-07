/* The builtin functions' table and escapers of templates, after Go's
   text/template/funcs.go (BSD-3-Clause, see LICENSE-Go). */
#include "template_internal.h"
#include <alt/net/url.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <string.h>

#define B(lit, kind) {{lit, sizeof(lit) - 1}, kind, 0, 0, 0, 0}

static const tfunc builtin_table[] = {
	B("and", F_AND), B("call", F_CALL), B("html", F_HTML), B("index", F_INDEX), B("slice", F_SLICE), B("js", F_JS),
	B("len", F_LEN), B("not", F_NOT), B("or", F_OR), B("print", F_PRINT), B("printf", F_PRINTF), B("println", F_PRINTLN),
	B("urlquery", F_URLQUERY), B("eq", F_EQ), B("ge", F_GE), B("gt", F_GT), B("le", F_LE), B("lt", F_LT), B("ne", F_NE),
};

const tfunc *tbuiltin_find(alt_str name) {
	for (size_t i = 0; i < sizeof builtin_table / sizeof builtin_table[0]; i++)
		if (alt_str_eq(builtin_table[i].name, name))
			return &builtin_table[i];
	return 0;
}

const tfunc *tfuncs_find(const tfuncs *fs, alt_str name) {
	if (!fs)
		return 0;
	for (size_t i = fs->n; i-- > 0;)
		if (alt_str_eq(fs->f[i].name, name))
			return &fs->f[i];
	return 0;
}

/* ---- escapers ---- */

void thtml_escape(alt_buf *b, alt_str s) {
	size_t last = 0;
	for (size_t i = 0; i < s.n; i++) {
		const char *h;
		switch (s.p[i]) {
		case 0: h = "\xef\xbf\xbd"; break;
		case '"': h = "&#34;"; break;
		case '\'': h = "&#39;"; break;
		case '&': h = "&amp;"; break;
		case '<': h = "&lt;"; break;
		case '>': h = "&gt;"; break;
		default: continue;
		}
		alt_buf_append(b, s.p + last, i - last);
		alt_buf_append_cstr(b, h);
		last = i + 1;
	}
	alt_buf_append(b, s.p + last, s.n - last);
}

static bool js_is_special(int32_t r) {
	switch (r) {
	case '\\':
	case '\'':
	case '"':
	case '<':
	case '>':
	case '&':
	case '=':
		return true;
	}
	return r < ' ' || 0x80 <= r;
}

void tjs_escape(alt_buf *b, alt_str s) {
	static const char hex[] = "0123456789ABCDEF";
	size_t last = 0;
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (!js_is_special(c))
			continue;
		alt_buf_append(b, s.p + last, i - last);
		if (c < 0x80) {
			switch (c) {
			case '\\': alt_buf_append_cstr(b, "\\\\"); break;
			case '\'': alt_buf_append_cstr(b, "\\'"); break;
			case '"': alt_buf_append_cstr(b, "\\\""); break;
			case '<': alt_buf_append_cstr(b, "\\u003C"); break;
			case '>': alt_buf_append_cstr(b, "\\u003E"); break;
			case '&': alt_buf_append_cstr(b, "\\u0026"); break;
			case '=': alt_buf_append_cstr(b, "\\u003D"); break;
			default:
				alt_buf_append_cstr(b, "\\u00");
				alt_buf_append_byte(b, hex[c >> 4]);
				alt_buf_append_byte(b, hex[c & 15]);
			}
		} else {
			int32_t r;
			size_t size = alt_utf8_decode(s.p + i, s.n - i, &r);
			if (alt_unicode_is_print(r))
				alt_buf_append(b, s.p + i, size);
			else
				alt_buf_printf(b, "\\u%04X", (unsigned)r);
			i += size - 1;
		}
		last = i + 1;
	}
	alt_buf_append(b, s.p + last, s.n - last);
}

void turl_query_escape(alt_buf *b, alt_str s) { alt_url_query_escape(b, s); }

alt_str teval_args(alt_arena *a, const tval *args, size_t n) {
	if (n == 1) {
		tval v = args[0];
		if (v.k == V_STRING && v.content == 0)
			return v.s;
	}
	tval *x = alt_arena_alloc(a, (n ? n : 1) * sizeof *x);
	for (size_t i = 0; i < n; i++) {
		x[i] = args[i];
		x[i].iface = 0;
		if (x[i].k == V_NIL || x[i].k == V_INVALID) {
			memset(&x[i], 0, sizeof x[i]);
			x[i].k = V_STRING;
			x[i].s = ALT_STR("<no value>");
		}
	}
	alt_buf b = ALT_BUF_INIT;
	tfmt_sprint(&b, x, n);
	alt_str r = tstr_dup(a, alt_buf_str(&b));
	alt_buf_free(&b);
	return r;
}

int alt_template_html_escape(alt_buf *b, alt_str s) {
	thtml_escape(b, s);
	return b->err;
}

int alt_template_js_escape(alt_buf *b, alt_str s) {
	tjs_escape(b, s);
	return b->err;
}
