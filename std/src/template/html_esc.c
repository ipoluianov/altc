/* The escaping functions of html/template, after Go's html/template
   (html.go, js.go, css.go, url.go, content.go, attr.go) and html's
   UnescapeString (BSD-3-Clause, see LICENSE-Go). */
#include "template_internal.h"
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <string.h>

#define FAILSAFE "ZgotmplZ"

static alt_str arena_str(alt_arena *a, alt_buf *b) {
	alt_str r = tstr_dup(a, alt_buf_str(b));
	alt_buf_free(b);
	return r;
}

/* stringify: the text of the arguments and its content type */
static alt_str stringify(alt_arena *a, const tval *args, size_t n, int *ct) {
	*ct = ALT_HTML_PLAIN;
	if (n == 1) {
		tval v = args[0];
		if (v.k == V_STRING) {
			*ct = v.content;
			return v.s;
		}
	}
	tval *x = alt_arena_alloc(a, (n ? n : 1) * sizeof *x);
	size_t k = 0;
	for (size_t i = 0; i < n; i++) {
		tval v = args[i];
		v.iface = 0;
		if (v.k == V_NIL || v.k == V_INVALID || v.k == V_MISSING)
			continue;
		x[k++] = v;
	}
	alt_buf b = ALT_BUF_INIT;
	tfmt_sprint(&b, x, k);
	return arena_str(a, &b);
}

/* ---- HTML ---- */

typedef const char *table[128];

static const table html_table = {[0] = "\xef\xbf\xbd", ['"'] = "&#34;", ['&'] = "&amp;", ['\''] = "&#39;", ['+'] = "&#43;",
	['<'] = "&lt;", ['>'] = "&gt;"};
static const table html_norm_table = {[0] = "\xef\xbf\xbd", ['"'] = "&#34;", ['\''] = "&#39;", ['+'] = "&#43;", ['<'] = "&lt;",
	['>'] = "&gt;"};
static const table html_nospace_table = {[0] = "&#xfffd;", ['\t'] = "&#9;", ['\n'] = "&#10;", ['\v'] = "&#11;", ['\f'] = "&#12;",
	['\r'] = "&#13;", [' '] = "&#32;", ['"'] = "&#34;", ['&'] = "&amp;", ['\''] = "&#39;", ['+'] = "&#43;", ['<'] = "&lt;",
	['='] = "&#61;", ['>'] = "&gt;", ['`'] = "&#96;"};
static const table html_nospace_norm_table = {[0] = "&#xfffd;", ['\t'] = "&#9;", ['\n'] = "&#10;", ['\v'] = "&#11;",
	['\f'] = "&#12;", ['\r'] = "&#13;", [' '] = "&#32;", ['"'] = "&#34;", ['\''] = "&#39;", ['+'] = "&#43;", ['<'] = "&lt;",
	['='] = "&#61;", ['>'] = "&gt;", ['`'] = "&#96;"};

static alt_str html_replacer(alt_arena *a, alt_str s, const table t, bool bad_runes) {
	alt_buf b = ALT_BUF_INIT;
	size_t written = 0;
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (r < 128) {
			if (t[r]) {
				alt_buf_append(&b, s.p + written, i - written);
				alt_buf_append_cstr(&b, t[r]);
				written = i + w;
			}
		} else if (bad_runes) {
		} else if ((0xfdd0 <= r && r <= 0xfdef) || (0xfff0 <= r && r <= 0xffff)) {
			alt_buf_append(&b, s.p + written, i - written);
			alt_buf_printf(&b, "&#x%x;", (unsigned)r);
			written = i + w;
		}
		i += w;
	}
	if (written == 0) {
		alt_buf_free(&b);
		return s;
	}
	alt_buf_append(&b, s.p + written, s.n - written);
	return arena_str(a, &b);
}


/* stripTags is in html_escape.c (it runs the transitions) */
alt_str thtml_strip_tags(alt_arena *a, alt_str html);

static alt_str html_nospace_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (!s.n)
		return ALT_STR(FAILSAFE);
	if (t == ALT_HTML_HTML)
		return html_replacer(a, thtml_strip_tags(a, s), html_nospace_norm_table, false);
	return html_replacer(a, s, html_nospace_table, false);
}

static alt_str attr_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_HTML)
		return html_replacer(a, thtml_strip_tags(a, s), html_norm_table, true);
	return html_replacer(a, s, html_table, true);
}

static alt_str rcdata_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_HTML)
		return html_replacer(a, s, html_norm_table, true);
	return html_replacer(a, s, html_table, true);
}

static alt_str html_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_HTML)
		return s;
	return html_replacer(a, s, html_table, true);
}

/* attrType */
static const struct {
	const char *name;
	int t;
} attr_types[] = {
	{"accept", ALT_HTML_PLAIN}, {"accept-charset", 8}, {"action", ALT_HTML_URL}, {"alt", ALT_HTML_PLAIN},
	{"archive", ALT_HTML_URL}, {"async", 8}, {"autocomplete", ALT_HTML_PLAIN}, {"autofocus", ALT_HTML_PLAIN},
	{"autoplay", ALT_HTML_PLAIN}, {"background", ALT_HTML_URL}, {"border", ALT_HTML_PLAIN}, {"checked", ALT_HTML_PLAIN},
	{"cite", ALT_HTML_URL}, {"challenge", 8}, {"charset", 8}, {"class", ALT_HTML_PLAIN}, {"classid", ALT_HTML_URL},
	{"codebase", ALT_HTML_URL}, {"cols", ALT_HTML_PLAIN}, {"colspan", ALT_HTML_PLAIN}, {"content", 8},
	{"contenteditable", ALT_HTML_PLAIN}, {"contextmenu", ALT_HTML_PLAIN}, {"controls", ALT_HTML_PLAIN},
	{"coords", ALT_HTML_PLAIN}, {"crossorigin", 8}, {"data", ALT_HTML_URL}, {"datetime", ALT_HTML_PLAIN},
	{"default", ALT_HTML_PLAIN}, {"defer", 8}, {"dir", ALT_HTML_PLAIN}, {"dirname", ALT_HTML_PLAIN},
	{"disabled", ALT_HTML_PLAIN}, {"draggable", ALT_HTML_PLAIN}, {"dropzone", ALT_HTML_PLAIN}, {"enctype", 8},
	{"for", ALT_HTML_PLAIN}, {"form", 8}, {"formaction", ALT_HTML_URL}, {"formenctype", 8}, {"formmethod", 8},
	{"formnovalidate", 8}, {"formtarget", ALT_HTML_PLAIN}, {"headers", ALT_HTML_PLAIN}, {"height", ALT_HTML_PLAIN},
	{"hidden", ALT_HTML_PLAIN}, {"high", ALT_HTML_PLAIN}, {"href", ALT_HTML_URL}, {"hreflang", ALT_HTML_PLAIN},
	{"http-equiv", 8}, {"icon", ALT_HTML_URL}, {"id", ALT_HTML_PLAIN}, {"ismap", ALT_HTML_PLAIN}, {"keytype", 8},
	{"kind", ALT_HTML_PLAIN}, {"label", ALT_HTML_PLAIN}, {"lang", ALT_HTML_PLAIN}, {"language", 8},
	{"list", ALT_HTML_PLAIN}, {"longdesc", ALT_HTML_URL}, {"loop", ALT_HTML_PLAIN}, {"low", ALT_HTML_PLAIN},
	{"manifest", ALT_HTML_URL}, {"max", ALT_HTML_PLAIN}, {"maxlength", ALT_HTML_PLAIN}, {"media", ALT_HTML_PLAIN},
	{"mediagroup", ALT_HTML_PLAIN}, {"method", 8}, {"min", ALT_HTML_PLAIN}, {"multiple", ALT_HTML_PLAIN},
	{"name", ALT_HTML_PLAIN}, {"novalidate", 8}, {"open", ALT_HTML_PLAIN}, {"optimum", ALT_HTML_PLAIN},
	{"pattern", 8}, {"placeholder", ALT_HTML_PLAIN}, {"poster", ALT_HTML_URL}, {"profile", ALT_HTML_URL},
	{"preload", ALT_HTML_PLAIN}, {"pubdate", ALT_HTML_PLAIN}, {"radiogroup", ALT_HTML_PLAIN}, {"readonly", ALT_HTML_PLAIN},
	{"rel", 8}, {"required", ALT_HTML_PLAIN}, {"reversed", ALT_HTML_PLAIN}, {"rows", ALT_HTML_PLAIN},
	{"rowspan", ALT_HTML_PLAIN}, {"sandbox", 8}, {"spellcheck", ALT_HTML_PLAIN}, {"scope", ALT_HTML_PLAIN},
	{"scoped", ALT_HTML_PLAIN}, {"seamless", ALT_HTML_PLAIN}, {"selected", ALT_HTML_PLAIN}, {"shape", ALT_HTML_PLAIN},
	{"size", ALT_HTML_PLAIN}, {"sizes", ALT_HTML_PLAIN}, {"span", ALT_HTML_PLAIN}, {"src", ALT_HTML_URL},
	{"srcdoc", ALT_HTML_HTML}, {"srclang", ALT_HTML_PLAIN}, {"srcset", ALT_HTML_SRCSET}, {"start", ALT_HTML_PLAIN},
	{"step", ALT_HTML_PLAIN}, {"style", ALT_HTML_CSS}, {"tabindex", ALT_HTML_PLAIN}, {"target", ALT_HTML_PLAIN},
	{"title", ALT_HTML_PLAIN}, {"type", 8}, {"usemap", ALT_HTML_URL}, {"value", 8}, {"width", ALT_HTML_PLAIN},
	{"wrap", ALT_HTML_PLAIN}, {"xmlns", ALT_HTML_URL},
};

int thtml_attr_type(alt_str name) {
	alt_str prefix, short_name;
	if (alt_str_has_prefix(name, ALT_STR("data-")))
		name = alt_str_from(name, 5);
	else if (alt_str_cut_byte(name, ':', &prefix, &short_name)) {
		if (alt_str_eq_cstr(prefix, "xmlns"))
			return ALT_HTML_URL;
		name = short_name;
	}
	for (size_t i = 0; i < sizeof attr_types / sizeof attr_types[0]; i++)
		if (alt_str_eq_cstr(name, attr_types[i].name))
			return attr_types[i].t;
	if (alt_str_has_prefix(name, ALT_STR("on")))
		return ALT_HTML_JS;
	if (alt_str_contains(name, ALT_STR("src")) || alt_str_contains(name, ALT_STR("uri")) || alt_str_contains(name, ALT_STR("url")))
		return ALT_HTML_URL;
	return ALT_HTML_PLAIN;
}

/* strings.ToLower */
static alt_str to_lower(alt_arena *a, alt_str s) {
	alt_buf b = ALT_BUF_INIT;
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (w == 1 && r == 0xfffd)
			alt_buf_append(&b, "\xef\xbf\xbd", 3);
		else {
			char tmp[4];
			alt_buf_append(&b, tmp, alt_utf8_encode(tmp, alt_unicode_to_lower(r)));
		}
		i += w;
	}
	return arena_str(a, &b);
}

static alt_str html_name_filter(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_HTML_ATTR)
		return s;
	if (!s.n)
		return ALT_STR(FAILSAFE);
	s = to_lower(a, s);
	if (thtml_attr_type(s) != ALT_HTML_PLAIN)
		return ALT_STR(FAILSAFE);
	for (size_t i = 0; i < s.n; i++) {
		char c = s.p[i];
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')))
			return ALT_STR(FAILSAFE);
	}
	return s;
}

/* ---- JavaScript ---- */

static bool is_js_ident_part(int32_t r) {
	return r == '$' || (r >= '0' && r <= '9') || (r >= 'A' && r <= 'Z') || r == '_' || (r >= 'a' && r <= 'z');
}

static bool is_js_space(int32_t r) {
	switch (r) {
	case '\f': case '\n': case '\r': case '\t': case '\v': case ' ': case 0xa0: case 0x1680: case 0x2028: case 0x2029:
	case 0x202f: case 0x205f: case 0x3000: case 0xfeff:
		return true;
	}
	return r >= 0x2000 && r <= 0x200a;
}

int thtml_next_js_ctx(alt_str s, int preceding) {
	/* bytes.TrimRight(s, jsWhitespace) */
	while (s.n) {
		int32_t r;
		size_t w = alt_utf8_decode_last(s.p, s.n, &r);
		if (w == 1 && r == 0xfffd && (unsigned char)s.p[s.n - 1] >= 0x80)
			break;
		if (!is_js_space(r))
			break;
		s.n -= w;
	}
	if (!s.n)
		return preceding;
	size_t n = s.n;
	char c = s.p[n - 1];
	switch (c) {
	case '+':
	case '-': {
		size_t start = n - 1;
		while (start > 0 && s.p[start - 1] == c)
			start--;
		return ((n - start) & 1) ? 0 : 1;
	}
	case '.':
		if (n != 1 && '0' <= s.p[n - 2] && s.p[n - 2] <= '9')
			return 1;
		return 0;
	case ',': case '<': case '>': case '=': case '*': case '%': case '&': case '|': case '^': case '?': case '!': case '~':
	case '(': case '[': case ':': case ';': case '{': case '}':
		return 0;
	default: {
		size_t j = n;
		while (j > 0 && is_js_ident_part((unsigned char)s.p[j - 1]))
			j--;
		alt_str w = alt_str_from(s, j);
		static const char *kws[] = {"break", "case", "continue", "delete", "do", "else", "finally", "in", "instanceof",
			"return", "throw", "try", "typeof", "void"};
		for (size_t i = 0; i < sizeof kws / sizeof kws[0]; i++)
			if (alt_str_eq_cstr(w, kws[i]))
				return 0;
	}
	}
	return 1;
}

static alt_json *val_json(alt_arena *a, const tval *v, bool *bad) {
	switch (v->k) {
	case V_BOOL: return alt_json_new_bool(a, v->b);
	case V_INT:
	case V_UINT8: return alt_json_new_int(a, v->i);
	case V_FLOAT: return alt_json_new_number(a, v->f);
	case V_STRING: return alt_json_new_string(a, v->s);
	case V_MAP: return (alt_json *)v->j;
	case V_SLICE: {
		alt_json *arr = alt_json_new_array(a);
		for (size_t i = 0; i < v->len; i++) {
			tval e = tslice_at(v, i);
			e.iface = 0;
			alt_json_append(arr, val_json(a, &e, bad));
		}
		return arr;
	}
	case V_COMPLEX:
		*bad = true;
		return alt_json_new_null(a);
	}
	return alt_json_new_null(a);
}

static alt_str js_val_escaper(alt_arena *a, const tval *args, size_t n) {
	tval v;
	if (n == 1) {
		v = args[0];
		v.iface = 0;
		if (v.k == V_STRING && v.content == ALT_HTML_JS)
			return v.s;
		if (v.k == V_STRING && v.content == ALT_HTML_JS_STR) {
			alt_buf b = ALT_BUF_INIT;
			alt_buf_append_byte(&b, '"');
			alt_buf_append_str(&b, v.s);
			alt_buf_append_byte(&b, '"');
			return arena_str(a, &b);
		}
	} else {
		tval *x = alt_arena_alloc(a, (n ? n : 1) * sizeof *x);
		for (size_t i = 0; i < n; i++) {
			x[i] = args[i];
			x[i].iface = 0;
			if (x[i].k == V_NIL)
				x[i].k = V_INVALID;
		}
		alt_buf b = ALT_BUF_INIT;
		tfmt_sprint(&b, x, n);
		memset(&v, 0, sizeof v);
		v.k = V_STRING;
		v.s = arena_str(a, &b);
	}
	bool bad = false;
	alt_json *j = val_json(a, &v, &bad);
	if (bad)
		return ALT_STR(" /* json: unsupported type: complex128 */null ");
	alt_buf b = ALT_BUF_INIT;
	alt_json_opts o = {0, 0, true, false};
	alt_json_write(&b, j, &o);
	alt_str js = arena_str(a, &b);
	if (!js.n)
		return ALT_STR(" null ");
	int32_t first, last;
	alt_utf8_decode(js.p, js.n, &first);
	alt_utf8_decode_last(js.p, js.n, &last);
	bool pad = is_js_ident_part(first) || is_js_ident_part(last);
	alt_buf out = ALT_BUF_INIT;
	if (pad)
		alt_buf_append_byte(&out, ' ');
	size_t written = 0;
	bool any = false;
	for (size_t i = 0; i < js.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(js.p + i, js.n - i, &r);
		if (r == 0x2028 || r == 0x2029) {
			alt_buf_append(&out, js.p + written, i - written);
			alt_buf_append_cstr(&out, r == 0x2028 ? "\\u2028" : "\\u2029");
			written = i + w;
			any = true;
		}
		i += w;
	}
	if (out.n) {
		alt_buf_append(&out, js.p + written, js.n - written);
		if (pad)
			alt_buf_append_byte(&out, ' ');
		return arena_str(a, &out);
	}
	(void)any;
	alt_buf_free(&out);
	return js;
}

static const char *low_unicode[32] = {"\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007",
	"\\u0008", "\\t", "\\n", "\\u000b", "\\f", "\\r", "\\u000e", "\\u000f", "\\u0010", "\\u0011", "\\u0012", "\\u0013", "\\u0014",
	"\\u0015", "\\u0016", "\\u0017", "\\u0018", "\\u0019", "\\u001a", "\\u001b", "\\u001c", "\\u001d", "\\u001e", "\\u001f"};

static const table js_str_table = {[0] = "\\u0000", ['\t'] = "\\t", ['\n'] = "\\n", ['\v'] = "\\u000b", ['\f'] = "\\f",
	['\r'] = "\\r", ['"'] = "\\u0022", ['`'] = "\\u0060", ['&'] = "\\u0026", ['\''] = "\\u0027", ['+'] = "\\u002b",
	['/'] = "\\/", ['<'] = "\\u003c", ['>'] = "\\u003e", ['\\'] = "\\\\"};
static const table js_bq_table = {[0] = "\\u0000", ['\t'] = "\\t", ['\n'] = "\\n", ['\v'] = "\\u000b", ['\f'] = "\\f",
	['\r'] = "\\r", ['"'] = "\\u0022", ['`'] = "\\u0060", ['&'] = "\\u0026", ['\''] = "\\u0027", ['+'] = "\\u002b",
	['/'] = "\\/", ['<'] = "\\u003c", ['>'] = "\\u003e", ['\\'] = "\\\\", ['$'] = "\\u0024", ['{'] = "\\u007b",
	['}'] = "\\u007d"};
static const table js_str_norm_table = {[0] = "\\u0000", ['\t'] = "\\t", ['\n'] = "\\n", ['\v'] = "\\u000b", ['\f'] = "\\f",
	['\r'] = "\\r", ['"'] = "\\u0022", ['&'] = "\\u0026", ['\''] = "\\u0027", ['`'] = "\\u0060", ['+'] = "\\u002b",
	['/'] = "\\/", ['<'] = "\\u003c", ['>'] = "\\u003e"};
static const table js_regexp_table = {[0] = "\\u0000", ['\t'] = "\\t", ['\n'] = "\\n", ['\v'] = "\\u000b", ['\f'] = "\\f",
	['\r'] = "\\r", ['"'] = "\\u0022", ['$'] = "\\$", ['&'] = "\\u0026", ['\''] = "\\u0027", ['('] = "\\(", [')'] = "\\)",
	['*'] = "\\*", ['+'] = "\\u002b", ['-'] = "\\-", ['.'] = "\\.", ['/'] = "\\/", ['<'] = "\\u003c", ['>'] = "\\u003e",
	['?'] = "\\?", ['['] = "\\[", ['\\'] = "\\\\", [']'] = "\\]", ['^'] = "\\^", ['{'] = "\\{", ['|'] = "\\|", ['}'] = "\\}"};

static alt_str js_replace(alt_arena *a, alt_str s, const table t) {
	alt_buf b = ALT_BUF_INIT;
	size_t written = 0;
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		const char *repl;
		if (r < 32)
			repl = low_unicode[r];
		else if (r < 128 && t[r])
			repl = t[r];
		else if (r == 0x2028)
			repl = "\\u2028";
		else if (r == 0x2029)
			repl = "\\u2029";
		else {
			i += w;
			continue;
		}
		alt_buf_append(&b, s.p + written, i - written);
		alt_buf_append_cstr(&b, repl);
		written = i + w;
		i += w;
	}
	if (written == 0) {
		alt_buf_free(&b);
		return s;
	}
	alt_buf_append(&b, s.p + written, s.n - written);
	return arena_str(a, &b);
}

static alt_str js_str_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_JS_STR)
		return js_replace(a, s, js_str_norm_table);
	return js_replace(a, s, js_str_table);
}

static alt_str js_tmpl_lit_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	return js_replace(a, stringify(a, args, n, &t), js_bq_table);
}

static alt_str js_regexp_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = js_replace(a, stringify(a, args, n, &t), js_regexp_table);
	if (!s.n)
		return ALT_STR("(?:)");
	return s;
}

bool thtml_is_js_type(alt_str mime) {
	alt_str before, after;
	alt_str_cut_byte(mime, ';', &before, &after);
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_str m = alt_str_trim_space(to_lower(&a, before));
	static const char *types[] = {"", "application/ecmascript", "application/javascript", "application/json",
		"application/ld+json", "application/x-ecmascript", "application/x-javascript", "module", "text/ecmascript",
		"text/javascript", "text/javascript1.0", "text/javascript1.1", "text/javascript1.2", "text/javascript1.3",
		"text/javascript1.4", "text/javascript1.5", "text/jscript", "text/livescript", "text/x-ecmascript",
		"text/x-javascript"};
	bool ok = false;
	for (size_t i = 0; i < sizeof types / sizeof types[0]; i++)
		if (alt_str_eq_cstr(m, types[i]))
			ok = true;
	alt_arena_free(&a);
	return ok;
}

/* ---- CSS ---- */

static bool is_css_nmchar(int32_t r) {
	return ('a' <= r && r <= 'z') || ('A' <= r && r <= 'Z') || ('0' <= r && r <= '9') || r == '-' || r == '_' ||
		(0x80 <= r && r <= 0xd7ff) || (0xe000 <= r && r <= 0xfffd) || (0x10000 <= r && r <= 0x10ffff);
}

bool thtml_ends_with_css_keyword(alt_str b, const char *kw) {
	size_t k = strlen(kw);
	if (b.n < k)
		return false;
	size_t i = b.n - k;
	if (i != 0) {
		int32_t r;
		alt_utf8_decode_last(b.p, i, &r);
		if (is_css_nmchar(r))
			return false;
	}
	for (size_t j = 0; j < k; j++) {
		char c = b.p[i + j];
		if (c >= 'A' && c <= 'Z')
			c = (char)(c + 32);
		if (c != kw[j])
			return false;
	}
	return true;
}

static bool is_hex(char c) { return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'); }

static int hexval(char c) { return c <= '9' ? c - '0' : (c | 32) - 'a' + 10; }

static alt_str skip_css_space(alt_str c) {
	if (!c.n)
		return c;
	switch (c.p[0]) {
	case '\t':
	case '\n':
	case '\f':
	case ' ':
		return alt_str_from(c, 1);
	case '\r':
		if (c.n >= 2 && c.p[1] == '\n')
			return alt_str_from(c, 2);
		return alt_str_from(c, 1);
	}
	return c;
}

alt_str thtml_decode_css(alt_arena *a, alt_str s) {
	if (alt_str_index_byte(s, '\\') < 0)
		return s;
	alt_buf b = ALT_BUF_INIT;
	while (s.n) {
		ssize_t i = alt_str_index_byte(s, '\\');
		if (i < 0)
			i = (ssize_t)s.n;
		alt_buf_append(&b, s.p, (size_t)i);
		s = alt_str_from(s, (size_t)i);
		if (s.n < 2)
			break;
		if (is_hex(s.p[1])) {
			size_t j = 2;
			while (j < s.n && j < 7 && is_hex(s.p[j]))
				j++;
			int32_t r = 0;
			for (size_t k = 1; k < j; k++)
				r = r << 4 | hexval(s.p[k]);
			if (r > 0x10ffff) {
				r /= 16;
				j--;
			}
			char tmp[4];
			alt_buf_append(&b, tmp, alt_utf8_encode(tmp, r));
			s = skip_css_space(alt_str_from(s, j));
		} else {
			int32_t r;
			size_t n = alt_utf8_decode(s.p + 1, s.n - 1, &r);
			alt_buf_append(&b, s.p + 1, n);
			s = alt_str_from(s, 1 + n);
		}
	}
	return arena_str(a, &b);
}

static bool is_css_space(char b) { return b == '\t' || b == '\n' || b == '\f' || b == '\r' || b == ' '; }

static const table css_table = {[0] = "\\0", ['\t'] = "\\9", ['\n'] = "\\a", ['\f'] = "\\c", ['\r'] = "\\d", ['"'] = "\\22",
	['&'] = "\\26", ['\''] = "\\27", ['('] = "\\28", [')'] = "\\29", ['+'] = "\\2b", ['/'] = "\\2f", [':'] = "\\3a",
	[';'] = "\\3b", ['<'] = "\\3c", ['>'] = "\\3e", ['\\'] = "\\\\", ['{'] = "\\7b", ['}'] = "\\7d"};

static alt_str css_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	alt_buf b = ALT_BUF_INIT;
	size_t written = 0;
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (!(r < 128 && css_table[r])) {
			i += w;
			continue;
		}
		const char *repl = css_table[r];
		alt_buf_append(&b, s.p + written, i - written);
		alt_buf_append_cstr(&b, repl);
		written = i + w;
		if (strcmp(repl, "\\\\") && (written == s.n || is_hex(s.p[written]) || is_css_space(s.p[written])))
			alt_buf_append_byte(&b, ' ');
		i += w;
	}
	if (written == 0) {
		alt_buf_free(&b);
		return s;
	}
	alt_buf_append(&b, s.p + written, s.n - written);
	return arena_str(a, &b);
}

static alt_str css_value_filter(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_CSS)
		return s;
	alt_str b = thtml_decode_css(a, s);
	alt_buf id = ALT_BUF_INIT;
	for (size_t i = 0; i < b.n; i++) {
		char c = b.p[i];
		switch (c) {
		case 0: case '"': case '\'': case '(': case ')': case '/': case ';': case '@': case '[': case '\\': case ']': case '`':
		case '{': case '}': case '<': case '>':
			alt_buf_free(&id);
			return ALT_STR(FAILSAFE);
		case '-':
			if (i != 0 && b.p[i - 1] == '-') {
				alt_buf_free(&id);
				return ALT_STR(FAILSAFE);
			}
			break;
		default:
			if ((unsigned char)c < 0x80 && is_css_nmchar(c))
				alt_buf_append_byte(&id, c >= 'A' && c <= 'Z' ? c + 32 : c);
		}
	}
	bool bad = alt_str_contains(alt_buf_str(&id), ALT_STR("expression")) || alt_str_contains(alt_buf_str(&id), ALT_STR("mozbinding"));
	alt_buf_free(&id);
	if (bad)
		return ALT_STR(FAILSAFE);
	return b;
}

/* ---- URLs ---- */

static bool is_safe_url(alt_str s) {
	alt_str protocol, rest;
	if (alt_str_cut_byte(s, ':', &protocol, &rest) && alt_str_index_byte(protocol, '/') < 0) {
		if (!alt_str_eq_fold(protocol, ALT_STR("http")) && !alt_str_eq_fold(protocol, ALT_STR("https")) &&
			!alt_str_eq_fold(protocol, ALT_STR("mailto")))
			return false;
	}
	return true;
}

static alt_str url_filter(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_URL)
		return s;
	if (!is_safe_url(s))
		return ALT_STR("#" FAILSAFE);
	return s;
}

static bool process_url_onto(alt_str s, bool norm, alt_buf *b) {
	size_t written = 0;
	for (size_t i = 0; i < s.n; i++) {
		char c = s.p[i];
		switch (c) {
		case '!': case '#': case '$': case '&': case '*': case '+': case ',': case '/': case ':': case ';': case '=': case '?':
		case '@': case '[': case ']':
			if (norm)
				continue;
			break;
		case '-': case '.': case '_': case '~':
			continue;
		case '%':
			if (norm && i + 2 < s.n && is_hex(s.p[i + 1]) && is_hex(s.p[i + 2]))
				continue;
			break;
		default:
			if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
				continue;
		}
		alt_buf_append(b, s.p + written, i - written);
		alt_buf_printf(b, "%%%02x", (unsigned char)c);
		written = i + 1;
	}
	alt_buf_append(b, s.p + written, s.n - written);
	return written != 0;
}

static alt_str url_processor(alt_arena *a, bool norm, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_URL)
		norm = true;
	alt_buf b = ALT_BUF_INIT;
	if (process_url_onto(s, norm, &b))
		return arena_str(a, &b);
	alt_buf_free(&b);
	return s;
}

static bool is_html_space(unsigned char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r'; }

static bool is_html_space_or_alnum(unsigned char c) {
	return is_html_space(c) || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static void filter_srcset_element(alt_str s, size_t left, size_t right, alt_buf *b) {
	size_t start = left;
	while (start < right && is_html_space((unsigned char)s.p[start]))
		start++;
	size_t end = right;
	for (size_t i = start; i < right; i++)
		if (is_html_space((unsigned char)s.p[i])) {
			end = i;
			break;
		}
	alt_str url = alt_str_slice(s, start, end);
	if (is_safe_url(url)) {
		bool ok = true;
		for (size_t i = end; i < right; i++)
			if (!is_html_space_or_alnum((unsigned char)s.p[i])) {
				ok = false;
				break;
			}
		if (ok) {
			alt_buf_append(b, s.p + left, start - left);
			process_url_onto(url, true, b);
			alt_buf_append(b, s.p + end, right - end);
			return;
		}
	}
	alt_buf_append_cstr(b, "#" FAILSAFE);
}

static alt_str srcset_escaper(alt_arena *a, const tval *args, size_t n) {
	int t;
	alt_str s = stringify(a, args, n, &t);
	if (t == ALT_HTML_SRCSET)
		return s;
	alt_buf b = ALT_BUF_INIT;
	if (t == ALT_HTML_URL) {
		alt_buf u = ALT_BUF_INIT;
		if (process_url_onto(s, true, &u))
			s = alt_buf_str(&u);
		for (size_t i = 0; i < s.n; i++) {
			if (s.p[i] == ',')
				alt_buf_append_cstr(&b, "%2c");
			else
				alt_buf_append_byte(&b, s.p[i]);
		}
		alt_buf_free(&u);
		return arena_str(a, &b);
	}
	size_t written = 0;
	for (size_t i = 0; i < s.n; i++) {
		if (s.p[i] == ',') {
			filter_srcset_element(s, written, i, &b);
			alt_buf_append_byte(&b, ',');
			written = i + 1;
		}
	}
	filter_srcset_element(s, written, s.n, &b);
	return arena_str(a, &b);
}

/* _eval_args_ */
static alt_str eval_args(alt_arena *a, const tval *args, size_t n) {
	if (n == 1 && args[0].k == V_STRING && args[0].content == 0)
		return args[0].s;
	tval *x = alt_arena_alloc(a, (n ? n : 1) * sizeof *x);
	for (size_t i = 0; i < n; i++) {
		x[i] = args[i];
		x[i].iface = 0;
		if (x[i].k == V_NIL)
			x[i].k = V_INVALID;
	}
	alt_buf b = ALT_BUF_INIT;
	tfmt_sprint(&b, x, n);
	return arena_str(a, &b);
}

/* ---- the table ---- */

#define E(lit, id) {{lit, sizeof(lit) - 1}, F_ESCAPER, 0, 0, 0, id}

static const tfunc escapers[ESC_COUNT] = {
	E("_html_template_attrescaper", ESC_ATTR),
	E("_html_template_commentescaper", ESC_COMMENT),
	E("_html_template_cssescaper", ESC_CSS),
	E("_html_template_cssvaluefilter", ESC_CSSVALUE),
	E("_html_template_htmlnamefilter", ESC_HTMLNAME),
	E("_html_template_htmlescaper", ESC_HTML),
	E("_html_template_jsregexpescaper", ESC_JSREGEXP),
	E("_html_template_jsstrescaper", ESC_JSSTR),
	E("_html_template_jstmpllitescaper", ESC_JSTMPLLIT),
	E("_html_template_jsvalescaper", ESC_JSVAL),
	E("_html_template_nospaceescaper", ESC_NOSPACE),
	E("_html_template_rcdataescaper", ESC_RCDATA),
	E("_html_template_srcsetescaper", ESC_SRCSET),
	E("_html_template_urlescaper", ESC_URL),
	E("_html_template_urlfilter", ESC_URLFILTER),
	E("_html_template_urlnormalizer", ESC_URLNORM),
	E("_eval_args_", ESC_EVALARGS),
};

const tfunc *thtml_escaper_func(alt_str name) {
	if (!alt_str_has_prefix(name, ALT_STR("_")))
		return 0;
	for (size_t i = 0; i < ESC_COUNT; i++)
		if (alt_str_eq(escapers[i].name, name))
			return &escapers[i];
	return 0;
}

const char *thtml_escaper_name(int esc) { return escapers[esc].name.p; }

alt_str thtml_run_escaper(alt_arena *a, int esc, const tval *args, size_t n) {
	switch (esc) {
	case ESC_ATTR: return attr_escaper(a, args, n);
	case ESC_COMMENT: return ALT_STR("");
	case ESC_CSS: return css_escaper(a, args, n);
	case ESC_CSSVALUE: return css_value_filter(a, args, n);
	case ESC_HTMLNAME: return html_name_filter(a, args, n);
	case ESC_HTML: return html_escaper(a, args, n);
	case ESC_JSREGEXP: return js_regexp_escaper(a, args, n);
	case ESC_JSSTR: return js_str_escaper(a, args, n);
	case ESC_JSTMPLLIT: return js_tmpl_lit_escaper(a, args, n);
	case ESC_JSVAL: return js_val_escaper(a, args, n);
	case ESC_NOSPACE: return html_nospace_escaper(a, args, n);
	case ESC_RCDATA: return rcdata_escaper(a, args, n);
	case ESC_SRCSET: return srcset_escaper(a, args, n);
	case ESC_URL: return url_processor(a, false, args, n);
	case ESC_URLFILTER: return url_filter(a, args, n);
	case ESC_URLNORM: return url_processor(a, true, args, n);
	case ESC_EVALARGS: return eval_args(a, args, n);
	}
	return ALT_STR("");
}

/* ---- html.UnescapeString ---- */

static const int32_t replacement_table[32] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030,
	0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC,
	0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};

static const thtml_entity *find_entity(alt_str name) {
	size_t lo = 0, hi = thtml_nentities;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		int c = alt_str_cmp(alt_str_cstr(thtml_entities[mid].name), name);
		if (c == 0)
			return &thtml_entities[mid];
		if (c < 0)
			lo = mid + 1;
		else
			hi = mid;
	}
	return 0;
}

static void put_rune(alt_buf *b, int32_t r) {
	char tmp[4];
	alt_buf_append(b, tmp, alt_utf8_encode(tmp, r));
}

/* one entity at the start of s (s[0] == '&'): the bytes consumed */
static size_t unescape_entity(alt_buf *b, alt_str s) {
	size_t i = 1;
	if (s.n <= 1) {
		alt_buf_append_byte(b, s.p[0]);
		return 1;
	}
	if (s.p[i] == '#') {
		if (s.n <= 3) {
			alt_buf_append_byte(b, s.p[0]);
			return 1;
		}
		i++;
		char c = s.p[i];
		bool hex = false;
		if (c == 'x' || c == 'X') {
			hex = true;
			i++;
		}
		uint32_t x = 0;
		while (i < s.n) {
			c = s.p[i];
			i++;
			if (hex) {
				if (is_hex(c)) {
					x = 16 * x + (uint32_t)hexval(c);
					continue;
				}
			} else if ('0' <= c && c <= '9') {
				x = 10 * x + (uint32_t)(c - '0');
				continue;
			}
			if (c != ';')
				i--;
			break;
		}
		if (i <= 3) {
			alt_buf_append_byte(b, s.p[0]);
			return 1;
		}
		int32_t r = (int32_t)x;
		if (0x80 <= r && r <= 0x9f)
			r = replacement_table[r - 0x80];
		else if (r == 0 || (0xd800 <= r && r <= 0xdfff) || r > 0x10ffff || r < 0)
			r = 0xfffd;
		put_rune(b, r);
		return i;
	}
	while (i < s.n) {
		char c = s.p[i];
		i++;
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
			continue;
		if (c != ';')
			i--;
		break;
	}
	alt_str name = alt_str_slice(s, 1, i);
	if (name.n) {
		const thtml_entity *e = find_entity(name);
		if (e) {
			put_rune(b, (int32_t)e->r1);
			if (e->r2)
				put_rune(b, (int32_t)e->r2);
			return i;
		}
		size_t max = name.n - 1;
		if (max > 6)
			max = 6;
		for (size_t j = max; j > 1; j--) {
			e = find_entity(alt_str_slice(name, 0, j));
			if (e && !e->r2) {
				put_rune(b, (int32_t)e->r1);
				return j + 1;
			}
		}
	}
	alt_buf_append(b, s.p, i);
	return i;
}

alt_str thtml_unescape(alt_arena *a, alt_str s) {
	if (alt_str_index_byte(s, '&') < 0)
		return s;
	alt_buf b = ALT_BUF_INIT;
	size_t src = 0;
	while (src < s.n) {
		ssize_t i = alt_str_index_byte(alt_str_from(s, src), '&');
		if (i < 0) {
			alt_buf_append(&b, s.p + src, s.n - src);
			break;
		}
		alt_buf_append(&b, s.p + src, (size_t)i);
		src += (size_t)i;
		src += unescape_entity(&b, alt_str_from(s, src));
	}
	return arena_str(a, &b);
}

int alt_html_escape_string(alt_buf *b, alt_str s) {
	thtml_escape(b, s);
	return b->err;
}

int alt_html_js_escape_string(alt_buf *b, alt_str s) {
	tjs_escape(b, s);
	return b->err;
}
