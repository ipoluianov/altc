/* The contextual autoescaper of html/template, after Go's
   html/template (context.go, transition.go, escape.go, error.go,
   template.go; BSD-3-Clause, see LICENSE-Go). */
#include "template_internal.h"
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

enum {
	S_TEXT,
	S_TAG,
	S_ATTR_NAME,
	S_AFTER_NAME,
	S_BEFORE_VALUE,
	S_HTML_CMT,
	S_RCDATA,
	S_ATTR,
	S_URL,
	S_SRCSET,
	S_JS,
	S_JS_DQ_STR,
	S_JS_SQ_STR,
	S_JS_TMPL_LIT,
	S_JS_REGEXP,
	S_JS_BLOCK_CMT,
	S_JS_LINE_CMT,
	S_JS_HTML_OPEN_CMT,
	S_JS_HTML_CLOSE_CMT,
	S_CSS,
	S_CSS_DQ_STR,
	S_CSS_SQ_STR,
	S_CSS_DQ_URL,
	S_CSS_SQ_URL,
	S_CSS_URL,
	S_CSS_BLOCK_CMT,
	S_CSS_LINE_CMT,
	S_ERROR,
	S_META_CONTENT,
	S_META_CONTENT_URL,
	S_DEAD,
};

static const char *state_names[] = {"stateText", "stateTag", "stateAttrName", "stateAfterName", "stateBeforeValue",
	"stateHTMLCmt", "stateRCDATA", "stateAttr", "stateURL", "stateSrcset", "stateJS", "stateJSDqStr", "stateJSSqStr",
	"stateJSTmplLit", "stateJSRegexp", "stateJSBlockCmt", "stateJSLineCmt", "stateJSHTMLOpenCmt", "stateJSHTMLCloseCmt",
	"stateCSS", "stateCSSDqStr", "stateCSSSqStr", "stateCSSDqURL", "stateCSSSqURL", "stateCSSURL", "stateCSSBlockCmt",
	"stateCSSLineCmt", "stateError", "stateMetaContent", "stateMetaContentURL", "stateDead"};

enum { D_NONE, D_DOUBLE_QUOTE, D_SINGLE_QUOTE, D_SPACE_OR_TAG_END };
static const char *delim_names[] = {"delimNone", "delimDoubleQuote", "delimSingleQuote", "delimSpaceOrTagEnd"};

enum { U_NONE, U_PRE_QUERY, U_QUERY_OR_FRAG, U_UNKNOWN };
static const char *url_part_names[] = {"urlPartNone", "urlPartPreQuery", "urlPartQueryOrFrag", "urlPartUnknown"};

enum { J_REGEXP, J_DIV_OP, J_UNKNOWN };
static const char *js_ctx_names[] = {"jsCtxRegexp", "jsCtxDivOp", "jsCtxUnknown"};

enum { E_NONE, E_SCRIPT, E_STYLE, E_TEXTAREA, E_TITLE, E_META };
static const char *element_names[] = {"elementNone", "elementScript", "elementStyle", "elementTextarea", "elementTitle",
	"elementMeta"};

enum { A_NONE, A_SCRIPT, A_SCRIPT_TYPE, A_STYLE, A_URL, A_SRCSET, A_META_CONTENT };
static const char *attr_names[] = {"attrNone", "attrScript", "attrScriptType", "attrStyle", "attrURL", "attrSrcset",
	"attrMetaContent"};

/* an Error: the code is not kept (the texts do not show it) */
typedef struct herr {
	const tnode *node;
	alt_str name;
	int line;
	alt_str desc;
} herr;

/* a Go slice of ints: copies of a context share the array */
typedef struct {
	int *p;
	size_t len, cap;
	bool nonnil;
} islice;

typedef struct {
	uint8_t state, delim, url_part, js_ctx, attr, element;
	islice depth;
	const tnode *n;
	herr *err;
} hctx;


/* the escaping's scratch arena (per escape run, set by the escaper) */
typedef struct {
	alt_arena *a;  /* scratch */
	alt_arena *pa; /* the set's (edits that stay) */
} harena;

static __thread harena *cur;

static void *halloc(size_t n) { return alt_arena_zalloc(cur->a, n ? n : 1); }

static alt_str hstr(alt_buf *b) {
	alt_str r = tstr_dup(cur->a, alt_buf_str(b));
	alt_buf_free(b);
	return r;
}

/* Go's size classes, for the capacities append and Clone give */
static size_t roundup_cap(size_t n) {
	static const size_t classes[] = {8, 16, 24, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 288, 320,
		352, 384, 416, 448, 480, 512, 576, 640, 704, 768, 896, 1024};
	size_t bytes = n * sizeof(long long);
	for (size_t i = 0; i < sizeof classes / sizeof classes[0]; i++)
		if (bytes <= classes[i])
			return classes[i] / sizeof(long long);
	return n;
}

static islice islice_append(islice s, int v) {
	if (s.len < s.cap) {
		s.p[s.len++] = v;
		s.nonnil = true;
		return s;
	}
	size_t nc = s.cap ? 2 * s.cap : 1;
	if (s.cap >= 256)
		nc = s.cap + (s.cap + 768) / 4;
	nc = roundup_cap(nc);
	int *p = halloc(nc * sizeof(int));
	if (s.len)
		memcpy(p, s.p, s.len * sizeof(int));
	p[s.len] = v;
	return (islice){p, s.len + 1, nc, true};
}

static islice islice_clone(islice s) {
	if (!s.nonnil)
		return s;
	if (!s.len)
		return (islice){0, 0, 0, true};
	size_t nc = roundup_cap(s.len);
	int *p = halloc(nc * sizeof(int));
	memcpy(p, s.p, s.len * sizeof(int));
	return (islice){p, s.len, nc, true};
}

static bool islice_eq(islice a, islice b) {
	if (a.len != b.len)
		return false;
	for (size_t i = 0; i < a.len; i++)
		if (a.p[i] != b.p[i])
			return false;
	return true;
}

static void islice_string(alt_buf *b, islice s) {
	alt_buf_append_byte(b, '[');
	for (size_t i = 0; i < s.len; i++) {
		if (i)
			alt_buf_append_byte(b, ' ');
		alt_buf_append_int(b, s.p[i]);
	}
	alt_buf_append_byte(b, ']');
}

static hctx ctx0(void) {
	hctx c;
	memset(&c, 0, sizeof c);
	return c;
}

static hctx ctx_state(int state) {
	hctx c = ctx0();
	c.state = (uint8_t)state;
	return c;
}

static bool ctx_eq(const hctx *c, const hctx *d) {
	return c->state == d->state && c->delim == d->delim && c->url_part == d->url_part && c->js_ctx == d->js_ctx &&
		islice_eq(c->depth, d->depth) && c->attr == d->attr && c->element == d->element && c->err == d->err;
}

static hctx ctx_clone(hctx c) {
	c.depth = islice_clone(c.depth);
	return c;
}

static void herr_string(alt_buf *b, const herr *e);

static void ctx_string(alt_buf *b, const hctx *c) {
	alt_buf_printf(b, "{%s %s %s %s ", state_names[c->state], delim_names[c->delim], url_part_names[c->url_part],
		js_ctx_names[c->js_ctx]);
	islice_string(b, c->depth);
	alt_buf_printf(b, " %s %s ", attr_names[c->attr], element_names[c->element]);
	if (c->err)
		herr_string(b, c->err);
	else
		alt_buf_append_cstr(b, "<nil>");
	alt_buf_append_byte(b, '}');
}

static alt_str mangle(const hctx *c, alt_str name) {
	if (c->state == S_TEXT)
		return name;
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_str(&b, name);
	alt_buf_append_cstr(&b, "$htmltemplate_");
	alt_buf_append_cstr(&b, state_names[c->state]);
	if (c->delim != D_NONE)
		alt_buf_printf(&b, "_%s", delim_names[c->delim]);
	if (c->url_part != U_NONE)
		alt_buf_printf(&b, "_%s", url_part_names[c->url_part]);
	if (c->js_ctx != J_REGEXP)
		alt_buf_printf(&b, "_%s", js_ctx_names[c->js_ctx]);
	if (c->depth.nonnil) {
		alt_buf_append_cstr(&b, "_jsBraceDepth(");
		islice_string(&b, c->depth);
		alt_buf_append_byte(&b, ')');
	}
	if (c->attr != A_NONE)
		alt_buf_printf(&b, "_%s", attr_names[c->attr]);
	if (c->element != E_NONE)
		alt_buf_printf(&b, "_%s", element_names[c->element]);
	return hstr(&b);
}

static bool is_comment(int s) {
	switch (s) {
	case S_HTML_CMT:
	case S_JS_BLOCK_CMT:
	case S_JS_LINE_CMT:
	case S_JS_HTML_OPEN_CMT:
	case S_JS_HTML_CLOSE_CMT:
	case S_CSS_BLOCK_CMT:
	case S_CSS_LINE_CMT:
		return true;
	}
	return false;
}

static bool is_in_tag(int s) { return s == S_TAG || s == S_ATTR_NAME || s == S_AFTER_NAME || s == S_BEFORE_VALUE || s == S_ATTR; }

static bool is_in_script_literal(int s) { return s == S_JS_DQ_STR || s == S_JS_SQ_STR || s == S_JS_TMPL_LIT || s == S_JS_REGEXP; }

/* ---- errors ---- */

static herr *herrorf(const tnode *node, int line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static herr *herrorf(const tnode *node, int line, const char *fmt, ...) {
	herr *e = halloc(sizeof *e);
	e->node = node;
	e->line = line;
	alt_buf b = ALT_BUF_INIT;
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(&b, fmt, ap);
	va_end(ap);
	e->desc = hstr(&b);
	return e;
}

static void herr_string(alt_buf *b, const herr *e) {
	if (e->node) {
		const tnode *n = e->node;
		alt_str text = alt_str_slice(n->tr->text, 0, (size_t)n->pos);
		ssize_t nl = alt_str_last_index_byte(text, '\n');
		int byte_num = nl < 0 ? n->pos : n->pos - (int)(nl + 1);
		int line = 1 + (int)alt_str_count(text, ALT_STR("\n"));
		alt_buf_printf(b, "html/template:%.*s:%d:%d: %.*s", (int)n->tr->parse_name.n, n->tr->parse_name.p, line, byte_num,
			(int)e->desc.n, e->desc.p);
	} else if (e->line)
		alt_buf_printf(b, "html/template:%.*s:%d: %.*s", (int)e->name.n, e->name.p, e->line, (int)e->desc.n, e->desc.p);
	else if (e->name.n)
		alt_buf_printf(b, "html/template:%.*s: %.*s", (int)e->name.n, e->name.p, (int)e->desc.n, e->desc.p);
	else
		alt_buf_printf(b, "html/template: %.*s", (int)e->desc.n, e->desc.p);
}

static hctx ctx_err(herr *e) {
	hctx c = ctx_state(S_ERROR);
	c.err = e;
	return c;
}

/* %q and %.32q of bytes, for messages */
static const char *q(alt_str s, size_t max_runes) {
	if (max_runes) {
		size_t k = 0, runes = 0;
		while (k < s.n && runes < max_runes) {
			int32_t r;
			k += alt_utf8_decode(s.p + k, s.n - k, &r);
			runes++;
		}
		s = alt_str_slice(s, 0, k);
	}
	alt_buf b = ALT_BUF_INIT;
	tquote(&b, s);
	return hstr(&b).p;
}

/* ---- transitions ---- */

static size_t eat_white_space(alt_str s, size_t i) {
	for (size_t j = i; j < s.n; j++) {
		switch (s.p[j]) {
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case '\r':
			break;
		default:
			return j;
		}
	}
	return s.n;
}

static bool ascii_alpha(char c) { return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'); }
static bool ascii_alnum(char c) { return ascii_alpha(c) || ('0' <= c && c <= '9'); }

static size_t eat_tag_name(alt_str s, size_t i, int *e) {
	*e = E_NONE;
	if (i == s.n || !ascii_alpha(s.p[i]))
		return i;
	size_t j = i + 1;
	while (j < s.n) {
		char x = s.p[j];
		if (ascii_alnum(x)) {
			j++;
			continue;
		}
		if ((x == ':' || x == '-') && j + 1 < s.n && ascii_alnum(s.p[j + 1])) {
			j += 2;
			continue;
		}
		break;
	}
	alt_str name = alt_str_slice(s, i, j);
	static const char *names[] = {"", "script", "style", "textarea", "title", "meta"};
	for (int k = 1; k < 6; k++)
		if (name.n == strlen(names[k]) && alt_str_eq_fold(name, alt_str_cstr(names[k])))
			*e = k;
	return j;
}

/* eatAttrName: the end, or SIZE_MAX with *err */
static size_t eat_attr_name(alt_str s, size_t i, herr **err) {
	for (size_t j = i; j < s.n; j++) {
		switch (s.p[j]) {
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case '\r':
		case '=':
		case '>':
			return j;
		case '\'':
		case '"':
		case '<':
			*err = herrorf(0, 0, "%s in attribute name: %s", q(alt_str_slice(s, j, j + 1), 0), q(s, 32));
			return (size_t)-1;
		}
	}
	return s.n;
}

static const uint8_t element_content_type[] = {S_TEXT, S_JS, S_CSS, S_RCDATA, S_RCDATA, S_TEXT};

static hctx t_text(hctx c, alt_str s, size_t *nread) {
	size_t k = 0;
	for (;;) {
		ssize_t x = alt_str_index_byte(alt_str_from(s, k), '<');
		if (x < 0 || k + (size_t)x + 1 == s.n) {
			*nread = s.n;
			return c;
		}
		size_t i = k + (size_t)x;
		if (i + 4 <= s.n && !memcmp(s.p + i, "<!--", 4)) {
			*nread = i + 4;
			return ctx_state(S_HTML_CMT);
		}
		i++;
		bool end = false;
		if (s.p[i] == '/') {
			if (i + 1 == s.n) {
				*nread = s.n;
				return c;
			}
			end = true;
			i++;
		}
		int e;
		size_t j = eat_tag_name(s, i, &e);
		if (j != i) {
			if (end)
				e = E_NONE;
			hctx r = ctx_state(S_TAG);
			r.element = (uint8_t)e;
			*nread = j;
			return r;
		}
		k = j;
	}
}

static hctx t_tag(hctx c, alt_str s, size_t *nread) {
	size_t i = eat_white_space(s, 0);
	if (i == s.n) {
		*nread = s.n;
		return c;
	}
	if (s.p[i] == '>') {
		*nread = i + 1;
		if (c.element == E_META)
			return ctx_state(S_TEXT);
		hctx r = ctx_state(element_content_type[c.element]);
		r.element = c.element;
		return r;
	}
	herr *err = 0;
	size_t j = eat_attr_name(s, i, &err);
	if (err) {
		*nread = s.n;
		return ctx_err(err);
	}
	int attr = A_NONE;
	if (i == j) {
		*nread = s.n;
		return ctx_err(herrorf(0, 0, "expected space, attr name, or end of tag, but got %s", q(alt_str_from(s, i), 0)));
	}
	alt_buf lb = ALT_BUF_INIT;
	alt_buf_append(&lb, s.p + i, j - i);
	alt_ascii_lower(lb.p, lb.n);
	/* strings.ToLower: non-ASCII letters do not matter for the names compared */
	alt_str name = hstr(&lb);
	if (c.element == E_SCRIPT && alt_str_eq_cstr(name, "type"))
		attr = A_SCRIPT_TYPE;
	else if (c.element == E_META && alt_str_eq_cstr(name, "content"))
		attr = A_META_CONTENT;
	else {
		switch (thtml_attr_type(name)) {
		case ALT_HTML_URL: attr = A_URL; break;
		case ALT_HTML_CSS: attr = A_STYLE; break;
		case ALT_HTML_JS: attr = A_SCRIPT; break;
		case ALT_HTML_SRCSET: attr = A_SRCSET; break;
		}
	}
	hctx r = ctx_state(j == s.n ? S_ATTR_NAME : S_AFTER_NAME);
	r.element = c.element;
	r.attr = (uint8_t)attr;
	*nread = j;
	return r;
}

static hctx t_attr_name(hctx c, alt_str s, size_t *nread) {
	herr *err = 0;
	size_t i = eat_attr_name(s, 0, &err);
	if (err) {
		*nread = s.n;
		return ctx_err(err);
	}
	if (i != s.n)
		c.state = S_AFTER_NAME;
	*nread = i;
	return c;
}

static hctx t_after_name(hctx c, alt_str s, size_t *nread) {
	size_t i = eat_white_space(s, 0);
	if (i == s.n) {
		*nread = s.n;
		return c;
	}
	if (s.p[i] != '=') {
		c.state = S_TAG;
		*nread = i;
		return c;
	}
	c.state = S_BEFORE_VALUE;
	*nread = i + 1;
	return c;
}

static const uint8_t attr_start_states[] = {S_ATTR, S_JS, S_ATTR, S_CSS, S_URL, S_SRCSET, S_META_CONTENT};

static hctx t_before_value(hctx c, alt_str s, size_t *nread) {
	size_t i = eat_white_space(s, 0);
	if (i == s.n) {
		*nread = s.n;
		return c;
	}
	int delim = D_SPACE_OR_TAG_END;
	if (s.p[i] == '\'') {
		delim = D_SINGLE_QUOTE;
		i++;
	} else if (s.p[i] == '"') {
		delim = D_DOUBLE_QUOTE;
		i++;
	}
	c.state = attr_start_states[c.attr];
	c.delim = (uint8_t)delim;
	*nread = i;
	return c;
}

static hctx t_html_cmt(hctx c, alt_str s, size_t *nread) {
	ssize_t i = alt_str_index(s, ALT_STR("-->"));
	if (i >= 0) {
		*nread = (size_t)i + 3;
		return ctx0();
	}
	*nread = s.n;
	return c;
}

static const char *special_tag_end_markers[] = {"", "script", "style", "textarea", "title", ""};

static ssize_t index_tag_end(alt_str s, const char *tag) {
	size_t res = 0, tn = strlen(tag);
	while (s.n > 0) {
		ssize_t i = alt_str_index(s, ALT_STR("</"));
		if (i < 0)
			return -1;
		s = alt_str_from(s, (size_t)i + 2);
		if (tn <= s.n && alt_str_eq_fold(alt_str_cstr(tag), alt_str_slice(s, 0, tn))) {
			s = alt_str_from(s, tn);
			if (s.n > 0 && strchr("> \t\n\f/", s.p[0]) && s.p[0])
				return (ssize_t)(res + (size_t)i);
			res += tn;
		}
		res += (size_t)i + 2;
	}
	return -1;
}

static hctx t_special_tag_end(hctx c, alt_str s, size_t *nread) {
	if (c.element != E_NONE) {
		if (c.element == E_SCRIPT && (is_in_script_literal(c.state) || is_comment(c.state))) {
			*nread = s.n;
			return c;
		}
		ssize_t i = index_tag_end(s, special_tag_end_markers[c.element]);
		if (i != -1) {
			*nread = (size_t)i;
			return ctx0();
		}
	}
	*nread = s.n;
	return c;
}

static hctx t_attr(hctx c, alt_str s, size_t *nread) {
	*nread = s.n;
	return c;
}

static hctx t_url(hctx c, alt_str s, size_t *nread) {
	if (alt_str_index_any(s, "#?") >= 0)
		c.url_part = U_QUERY_OR_FRAG;
	else if (s.n != eat_white_space(s, 0) && c.url_part == U_NONE)
		c.url_part = U_PRE_QUERY;
	*nread = s.n;
	return c;
}

static hctx t_js(hctx c, alt_str s, size_t *nread) {
	ssize_t si = alt_str_index_any(s, "\"`'/{}<-#");
	if (si < 0) {
		c.js_ctx = (uint8_t)thtml_next_js_ctx(s, c.js_ctx);
		*nread = s.n;
		return c;
	}
	size_t i = (size_t)si;
	c.js_ctx = (uint8_t)thtml_next_js_ctx(alt_str_slice(s, 0, i), c.js_ctx);
	switch (s.p[i]) {
	case '"':
		c.state = S_JS_DQ_STR;
		c.js_ctx = J_REGEXP;
		break;
	case '\'':
		c.state = S_JS_SQ_STR;
		c.js_ctx = J_REGEXP;
		break;
	case '`':
		c.state = S_JS_TMPL_LIT;
		c.js_ctx = J_REGEXP;
		break;
	case '/':
		if (i + 1 < s.n && s.p[i + 1] == '/') {
			c.state = S_JS_LINE_CMT;
			i++;
		} else if (i + 1 < s.n && s.p[i + 1] == '*') {
			c.state = S_JS_BLOCK_CMT;
			i++;
		} else if (c.js_ctx == J_REGEXP)
			c.state = S_JS_REGEXP;
		else if (c.js_ctx == J_DIV_OP)
			c.js_ctx = J_REGEXP;
		else {
			*nread = s.n;
			return ctx_err(herrorf(0, 0, "'/' could start a division or regexp: %s", q(alt_str_from(s, i), 32)));
		}
		break;
	case '<':
		if (i + 3 < s.n && !memcmp(s.p + i, "<!--", 4)) {
			c.state = S_JS_HTML_OPEN_CMT;
			i += 3;
		}
		break;
	case '-':
		if (i + 2 < s.n && !memcmp(s.p + i, "-->", 3)) {
			c.state = S_JS_HTML_CLOSE_CMT;
			i += 2;
		}
		break;
	case '#':
		if (i + 1 < s.n && s.p[i + 1] == '!') {
			c.state = S_JS_LINE_CMT;
			i++;
		}
		break;
	case '{':
		if (c.depth.len == 0) {
			c.js_ctx = (uint8_t)thtml_next_js_ctx(alt_str_slice(s, i, i + 1), c.js_ctx);
			*nread = i + 1;
			return c;
		}
		c.depth.p[c.depth.len - 1]++;
		c.js_ctx = (uint8_t)thtml_next_js_ctx(alt_str_slice(s, i, i + 1), c.js_ctx);
		break;
	case '}':
		if (c.depth.len == 0) {
			c.js_ctx = (uint8_t)thtml_next_js_ctx(alt_str_slice(s, i, i + 1), c.js_ctx);
			*nread = i + 1;
			return c;
		}
		c.depth.p[c.depth.len - 1]--;
		if (c.depth.p[c.depth.len - 1] >= 0) {
			c.js_ctx = (uint8_t)thtml_next_js_ctx(alt_str_slice(s, i, i + 1), c.js_ctx);
			*nread = i + 1;
			return c;
		}
		c.depth.len--;
		c.state = S_JS_TMPL_LIT;
		break;
	}
	*nread = i + 1;
	return c;
}

static hctx t_js_tmpl(hctx c, alt_str s, size_t *nread) {
	size_t k = 0;
	for (;;) {
		ssize_t x = alt_str_index_any(alt_str_from(s, k), "`\\$");
		if (x < 0)
			break;
		size_t i = k + (size_t)x;
		switch (s.p[i]) {
		case '\\':
			i++;
			if (i == s.n) {
				*nread = s.n;
				return ctx_err(herrorf(0, 0, "unfinished escape sequence in JS string: %s", q(s, 0)));
			}
			break;
		case '$':
			if (s.n >= i + 2 && s.p[i + 1] == '{') {
				c.depth = islice_append(c.depth, 0);
				c.state = S_JS;
				*nread = i + 2;
				return c;
			}
			break;
		case '`':
			c.state = S_JS;
			*nread = i + 1;
			return c;
		}
		k = i + 1;
	}
	*nread = s.n;
	return c;
}

static hctx t_js_delimited(hctx c, alt_str s, size_t *nread) {
	const char *specials = "\\\"";
	if (c.state == S_JS_SQ_STR)
		specials = "\\'";
	else if (c.state == S_JS_REGEXP)
		specials = "\\/[]";
	size_t k = 0;
	bool in_charset = false;
	for (;;) {
		ssize_t x = alt_str_index_any(alt_str_from(s, k), specials);
		if (x < 0)
			break;
		size_t i = k + (size_t)x;
		switch (s.p[i]) {
		case '\\':
			i++;
			if (i == s.n) {
				*nread = s.n;
				return ctx_err(herrorf(0, 0, "unfinished escape sequence in JS string: %s", q(s, 0)));
			}
			break;
		case '[':
			in_charset = true;
			break;
		case ']':
			in_charset = false;
			break;
		case '/':
			if (i > 0 && i + 7 <= s.n && alt_str_eq_fold(alt_str_slice(s, i - 1, i + 7), ALT_STR("</script")))
				i++;
			else if (!in_charset) {
				c.state = S_JS;
				c.js_ctx = J_DIV_OP;
				*nread = i + 1;
				return c;
			}
			break;
		default:
			if (!in_charset) {
				c.state = S_JS;
				c.js_ctx = J_DIV_OP;
				*nread = i + 1;
				return c;
			}
		}
		k = i + 1;
	}
	if (in_charset) {
		*nread = s.n;
		return ctx_err(herrorf(0, 0, "unfinished JS regexp charset: %s", q(s, 0)));
	}
	*nread = s.n;
	return c;
}

static hctx t_block_cmt(hctx c, alt_str s, size_t *nread) {
	ssize_t i = alt_str_index(s, ALT_STR("*/"));
	if (i < 0) {
		*nread = s.n;
		return c;
	}
	c.state = c.state == S_JS_BLOCK_CMT ? S_JS : S_CSS;
	*nread = (size_t)i + 2;
	return c;
}

static hctx t_line_cmt(hctx c, alt_str s, size_t *nread) {
	int end;
	ssize_t i = -1;
	if (c.state == S_CSS_LINE_CMT) {
		end = S_CSS;
		i = alt_str_index_any(s, "\n\f\r");
	} else {
		end = S_JS;
		for (size_t j = 0; j < s.n; j++) {
			if (s.p[j] == '\n' || s.p[j] == '\r' ||
				(j + 2 < s.n && (unsigned char)s.p[j] == 0xe2 && (unsigned char)s.p[j + 1] == 0x80 &&
					((unsigned char)s.p[j + 2] == 0xa8 || (unsigned char)s.p[j + 2] == 0xa9))) {
				i = (ssize_t)j;
				break;
			}
		}
	}
	if (i < 0) {
		*nread = s.n;
		return c;
	}
	c.state = (uint8_t)end;
	*nread = (size_t)i;
	return c;
}

static hctx t_css(hctx c, alt_str s, size_t *nread) {
	size_t k = 0;
	for (;;) {
		ssize_t x = alt_str_index_any(alt_str_from(s, k), "(\"'/");
		if (x < 0) {
			*nread = s.n;
			return c;
		}
		size_t i = k + (size_t)x;
		switch (s.p[i]) {
		case '(': {
			alt_str p = alt_str_trim_right(alt_str_slice(s, 0, i), "\t\n\f\r ");
			if (thtml_ends_with_css_keyword(p, "url")) {
				alt_str rest = alt_str_trim_left(alt_str_from(s, i + 1), "\t\n\f\r ");
				size_t j = s.n - rest.n;
				if (j != s.n && s.p[j] == '"') {
					c.state = S_CSS_DQ_URL;
					j++;
				} else if (j != s.n && s.p[j] == '\'') {
					c.state = S_CSS_SQ_URL;
					j++;
				} else
					c.state = S_CSS_URL;
				*nread = j;
				return c;
			}
			break;
		}
		case '/':
			if (i + 1 < s.n) {
				if (s.p[i + 1] == '/') {
					c.state = S_CSS_LINE_CMT;
					*nread = i + 2;
					return c;
				}
				if (s.p[i + 1] == '*') {
					c.state = S_CSS_BLOCK_CMT;
					*nread = i + 2;
					return c;
				}
			}
			break;
		case '"':
			c.state = S_CSS_DQ_STR;
			*nread = i + 1;
			return c;
		case '\'':
			c.state = S_CSS_SQ_STR;
			*nread = i + 1;
			return c;
		}
		k = i + 1;
	}
}

static hctx t_css_str(hctx c, alt_str s, size_t *nread) {
	const char *end_and_esc;
	switch (c.state) {
	case S_CSS_DQ_STR:
	case S_CSS_DQ_URL:
		end_and_esc = "\\\"";
		break;
	case S_CSS_SQ_STR:
	case S_CSS_SQ_URL:
		end_and_esc = "\\'";
		break;
	default:
		end_and_esc = "\\\t\n\f\r )";
	}
	size_t k = 0;
	for (;;) {
		ssize_t x = alt_str_index_any(alt_str_from(s, k), end_and_esc);
		if (x < 0) {
			size_t n;
			c = t_url(c, thtml_decode_css(cur->a, alt_str_from(s, k)), &n);
			*nread = k + n;
			return c;
		}
		size_t i = k + (size_t)x;
		if (s.p[i] == '\\') {
			i++;
			if (i == s.n) {
				*nread = s.n;
				return ctx_err(herrorf(0, 0, "unfinished escape sequence in CSS string: %s", q(s, 0)));
			}
		} else {
			c.state = S_CSS;
			*nread = i + 1;
			return c;
		}
		size_t n;
		c = t_url(c, thtml_decode_css(cur->a, alt_str_slice(s, 0, i + 1)), &n);
		k = i + 1;
	}
}

static hctx t_error(hctx c, alt_str s, size_t *nread) {
	*nread = s.n;
	return c;
}

static hctx t_meta_content(hctx c, alt_str s, size_t *nread) {
	for (size_t i = 0; i < s.n; i++) {
		if (i + 3 + 1 <= s.n && alt_str_eq_fold(alt_str_slice(s, i, i + 3), ALT_STR("url"))) {
			size_t j = eat_white_space(s, i + 3);
			if (j < s.n && s.p[j] == '=') {
				c.state = S_META_CONTENT_URL;
				*nread = j + 1;
				return c;
			}
		}
	}
	*nread = s.n;
	return c;
}

static hctx t_meta_content_url(hctx c, alt_str s, size_t *nread) {
	for (size_t i = 0; i < s.n; i++) {
		if (s.p[i] == ';') {
			c.state = S_META_CONTENT;
			*nread = i + 1;
			return c;
		}
	}
	*nread = s.n;
	return c;
}

static hctx transition(int state, hctx c, alt_str s, size_t *nread) {
	switch (state) {
	case S_TEXT: return t_text(c, s, nread);
	case S_TAG: return t_tag(c, s, nread);
	case S_ATTR_NAME: return t_attr_name(c, s, nread);
	case S_AFTER_NAME: return t_after_name(c, s, nread);
	case S_BEFORE_VALUE: return t_before_value(c, s, nread);
	case S_HTML_CMT: return t_html_cmt(c, s, nread);
	case S_RCDATA: return t_special_tag_end(c, s, nread);
	case S_ATTR: return t_attr(c, s, nread);
	case S_URL:
	case S_SRCSET: return t_url(c, s, nread);
	case S_META_CONTENT: return t_meta_content(c, s, nread);
	case S_META_CONTENT_URL: return t_meta_content_url(c, s, nread);
	case S_JS: return t_js(c, s, nread);
	case S_JS_DQ_STR:
	case S_JS_SQ_STR:
	case S_JS_REGEXP: return t_js_delimited(c, s, nread);
	case S_JS_TMPL_LIT: return t_js_tmpl(c, s, nread);
	case S_JS_BLOCK_CMT:
	case S_CSS_BLOCK_CMT: return t_block_cmt(c, s, nread);
	case S_JS_LINE_CMT:
	case S_JS_HTML_OPEN_CMT:
	case S_JS_HTML_CLOSE_CMT:
	case S_CSS_LINE_CMT: return t_line_cmt(c, s, nread);
	case S_CSS: return t_css(c, s, nread);
	case S_CSS_DQ_STR:
	case S_CSS_SQ_STR:
	case S_CSS_DQ_URL:
	case S_CSS_SQ_URL:
	case S_CSS_URL: return t_css_str(c, s, nread);
	case S_ERROR: return t_error(c, s, nread);
	}
	/* stateDead and others have no transition (Go would panic) */
	*nread = s.n;
	return c;
}

static const char *delim_ends[] = {"", "\"", "'", " \t\n\f\r>"};

static hctx context_after_text(hctx c, alt_str s, size_t *nread) {
	if (c.delim == D_NONE) {
		size_t i;
		hctx c1 = t_special_tag_end(c, s, &i);
		if (i == 0) {
			*nread = 0;
			return c1;
		}
		return transition(c.state, c, alt_str_slice(s, 0, i), nread);
	}
	ssize_t x = alt_str_index_any(s, delim_ends[c.delim]);
	size_t i = x < 0 ? s.n : (size_t)x;
	if (c.delim == D_SPACE_OR_TAG_END) {
		ssize_t j = alt_str_index_any(alt_str_slice(s, 0, i), "\"'<=`");
		if (j >= 0) {
			*nread = s.n;
			return ctx_err(herrorf(0, 0, "%s in unquoted attr: %s", q(alt_str_slice(s, (size_t)j, (size_t)j + 1), 0),
				q(alt_str_slice(s, 0, i), 0)));
		}
	}
	if (i == s.n) {
		alt_str u = thtml_unescape(cur->a, s);
		while (u.n) {
			size_t i1;
			c = transition(c.state, c, u, &i1);
			u = alt_str_from(u, i1);
		}
		*nread = s.n;
		return c;
	}
	int element = c.element;
	if (c.state == S_ATTR && c.element == E_SCRIPT && c.attr == A_SCRIPT_TYPE && !thtml_is_js_type(alt_str_slice(s, 0, i)))
		element = E_NONE;
	if (c.delim != D_SPACE_OR_TAG_END)
		i++;
	hctx r = ctx_state(S_TAG);
	r.element = (uint8_t)element;
	*nread = i;
	return r;
}

alt_str thtml_strip_tags(alt_arena *a, alt_str html) {
	harena h = {a, a}, *saved = cur;
	cur = &h;
	alt_buf b = ALT_BUF_INIT;
	hctx c = ctx0();
	size_t i = 0;
	bool all_text = true;
	while (i != html.n) {
		if (c.delim == D_NONE) {
			int st = c.state;
			if (c.element != E_NONE && !is_in_tag(st))
				st = S_RCDATA;
			size_t nread;
			hctx d = transition(st, c, alt_str_from(html, i), &nread);
			size_t i1 = i + nread;
			if (c.state == S_TEXT || c.state == S_RCDATA) {
				size_t j = i1;
				if (d.state != c.state) {
					for (size_t j1 = j; j1-- > i;)
						if (html.p[j1] == '<') {
							j = j1;
							break;
						}
				}
				alt_buf_append(&b, html.p + i, j - i);
			} else
				all_text = false;
			c = d;
			i = i1;
			continue;
		}
		ssize_t x = alt_str_index_any(alt_str_from(html, i), delim_ends[c.delim]);
		if (x < 0)
			break;
		size_t i1 = i + (size_t)x;
		if (c.delim != D_SPACE_OR_TAG_END)
			i1++;
		hctx r = ctx_state(S_TAG);
		r.element = c.element;
		c = r;
		i = i1;
	}
	alt_str res;
	if (all_text) {
		alt_buf_free(&b);
		res = html;
	} else {
		if (c.state == S_TEXT || c.state == S_RCDATA)
			alt_buf_append(&b, html.p + i, html.n - i);
		res = tstr_dup(a, alt_buf_str(&b));
		alt_buf_free(&b);
	}
	cur = saved;
	return res;
}

/* ---- the escaper ---- */

typedef struct {
	alt_str name;
	hctx c;
} hout;

typedef struct {
	alt_str name;
	ttree *tree;
} hderived;

typedef struct {
	tnode *n;
	alt_str *cmds;
	size_t ncmds;
} haedit;

typedef struct {
	tnode *n;
	alt_str name;
} htedit;

typedef struct {
	tnode *n;
	alt_str text;
} hxedit;

typedef struct hrange {
	struct hrange *outer;
	hctx *breaks, *conts;
	size_t nb, nc;
} hrange;

typedef struct {
	alt_template *t; /* any template of the set */
	hout *out;
	size_t nout;
	hderived *der;
	size_t nder;
	alt_str *called;
	size_t ncalled;
	haedit *ae;
	size_t nae;
	htedit *te;
	size_t nte;
	hxedit *xe;
	size_t nxe;
	hrange *rc;
	int depth;
} hesc;

#define GROW(arr, n) ((arr) = alt_arena_realloc(cur->a, (arr), (n) * sizeof *(arr), ((n) + 1) * sizeof *(arr)))

static hctx *out_find(hesc *e, alt_str name) {
	for (size_t i = 0; i < e->nout; i++)
		if (alt_str_eq(e->out[i].name, name))
			return &e->out[i].c;
	return 0;
}

static void out_set(hesc *e, alt_str name, hctx c) {
	hctx *p = out_find(e, name);
	if (p) {
		*p = c;
		return;
	}
	GROW(e->out, e->nout);
	e->out[e->nout++] = (hout){name, c};
}

static bool called_has(hesc *e, alt_str name) {
	for (size_t i = 0; i < e->ncalled; i++)
		if (alt_str_eq(e->called[i], name))
			return true;
	return false;
}

static void called_add(hesc *e, alt_str name) {
	if (called_has(e, name))
		return;
	GROW(e->called, e->ncalled);
	e->called[e->ncalled++] = name;
}

static ttree *derived_find(hesc *e, alt_str name) {
	for (size_t i = 0; i < e->nder; i++)
		if (alt_str_eq(e->der[i].name, name))
			return e->der[i].tree;
	return 0;
}

static void derived_set(hesc *e, alt_str name, ttree *t) {
	for (size_t i = 0; i < e->nder; i++)
		if (alt_str_eq(e->der[i].name, name)) {
			e->der[i].tree = t;
			return;
		}
	GROW(e->der, e->nder);
	e->der[e->nder++] = (hderived){name, t};
}

static void edit_action(hesc *e, tnode *n, alt_str *cmds, size_t k) {
	for (size_t i = 0; i < e->nae; i++)
		if (e->ae[i].n == n) {
			e->ae[i].cmds = cmds;
			e->ae[i].ncmds = k;
			return;
		}
	GROW(e->ae, e->nae);
	e->ae[e->nae++] = (haedit){n, cmds, k};
}

static void edit_template(hesc *e, tnode *n, alt_str name) {
	for (size_t i = 0; i < e->nte; i++)
		if (e->te[i].n == n) {
			e->te[i].name = name;
			return;
		}
	GROW(e->te, e->nte);
	e->te[e->nte++] = (htedit){n, name};
}

static void edit_text(hesc *e, tnode *n, alt_str text) {
	for (size_t i = 0; i < e->nxe; i++)
		if (e->xe[i].n == n) {
			e->xe[i].text = text;
			return;
		}
	GROW(e->xe, e->nxe);
	e->xe[e->nxe++] = (hxedit){n, text};
}

/* a template's tree by name: of the set or derived */
static ttree *etemplate(hesc *e, alt_str name) {
	alt_template *t = ttemplate_lookup(e->t, name);
	if (t)
		return t->tree;
	return derived_find(e, name);
}

static bool etemplate_exists(hesc *e, alt_str name) { return ttemplate_lookup(e->t, name) || derived_find(e, name); }

static hctx escape_node(hesc *e, hctx c, tnode *n);
static hctx escape_list(hesc *e, hctx c, tnode *n);

static hctx nudge(hctx c) {
	switch (c.state) {
	case S_TAG:
		c.state = S_ATTR_NAME;
		break;
	case S_BEFORE_VALUE:
		c.state = attr_start_states[c.attr];
		c.delim = D_SPACE_OR_TAG_END;
		c.attr = A_NONE;
		break;
	case S_AFTER_NAME:
		c.state = S_ATTR_NAME;
		c.attr = A_NONE;
		break;
	}
	return c;
}

static hctx join(hctx a, hctx b, const tnode *node, const char *node_name) {
	if (a.state == S_ERROR)
		return a;
	if (b.state == S_ERROR)
		return b;
	if (a.state == S_DEAD)
		return b;
	if (b.state == S_DEAD)
		return a;
	if (ctx_eq(&a, &b))
		return a;
	hctx c = a;
	c.url_part = b.url_part;
	if (ctx_eq(&c, &b)) {
		c.url_part = U_UNKNOWN;
		return c;
	}
	c = a;
	c.js_ctx = b.js_ctx;
	if (ctx_eq(&c, &b)) {
		c.js_ctx = J_UNKNOWN;
		return c;
	}
	hctx na = nudge(a), nb = nudge(b);
	if (!(ctx_eq(&na, &a) && ctx_eq(&nb, &b))) {
		hctx j = join(na, nb, node, node_name);
		if (j.state != S_ERROR)
			return j;
	}
	alt_buf sa = ALT_BUF_INIT, sb = ALT_BUF_INIT;
	ctx_string(&sa, &a);
	ctx_string(&sb, &b);
	herr *err = herrorf(node, 0, "{{%s}} branches end in different contexts: %s, %s", node_name, alt_buf_cstr(&sa), alt_buf_cstr(&sb));
	alt_buf_free(&sa);
	alt_buf_free(&sb);
	return ctx_err(err);
}

static bool is_predefined(alt_str s) { return alt_str_eq_cstr(s, "html") || alt_str_eq_cstr(s, "urlquery"); }

static hctx escape_action(hesc *e, hctx c, tnode *n) {
	tnode *pipe = n->node;
	if (pipe->ndecl != 0)
		return c;
	c = nudge(c);
	for (size_t pos = 0; pos < pipe->nn; pos++) {
		tnode *id = pipe->nodes[pos]->nodes[0];
		if (id->type != N_IDENTIFIER)
			continue;
		if (is_predefined(id->text)) {
			if (pos < pipe->nn - 1 || (c.state == S_ATTR && c.delim == D_SPACE_OR_TAG_END && alt_str_eq_cstr(id->text, "html")))
				return ctx_err(herrorf(n, n->line, "predefined escaper %s disallowed in template", q(id->text, 0)));
		}
	}
	alt_str *s = halloc(4 * sizeof *s);
	size_t k = 0;
#define ADD(id) (s[k++] = alt_str_cstr(thtml_escaper_name(id)))
	switch (c.state) {
	case S_ERROR:
		return c;
	case S_URL:
	case S_CSS_DQ_STR:
	case S_CSS_SQ_STR:
	case S_CSS_DQ_URL:
	case S_CSS_SQ_URL:
	case S_CSS_URL:
		switch (c.url_part) {
		case U_NONE:
			ADD(ESC_URLFILTER);
			/* fallthrough */
		case U_PRE_QUERY:
			if (c.state == S_CSS_DQ_STR || c.state == S_CSS_SQ_STR)
				ADD(ESC_CSS);
			else
				ADD(ESC_URLNORM);
			break;
		case U_QUERY_OR_FRAG:
			ADD(ESC_URL);
			break;
		case U_UNKNOWN: {
			alt_buf b = ALT_BUF_INIT;
			tnode_string(&b, n);
			herr *err = herrorf(n, n->line, "%s appears in an ambiguous context within a URL", alt_buf_cstr(&b));
			alt_buf_free(&b);
			return ctx_err(err);
		}
		}
		break;
	case S_META_CONTENT:
		break;
	case S_META_CONTENT_URL:
		ADD(ESC_URLFILTER);
		break;
	case S_JS:
		ADD(ESC_JSVAL);
		c.js_ctx = J_DIV_OP;
		break;
	case S_JS_DQ_STR:
	case S_JS_SQ_STR:
		ADD(ESC_JSSTR);
		break;
	case S_JS_TMPL_LIT:
		ADD(ESC_JSTMPLLIT);
		break;
	case S_JS_REGEXP:
		ADD(ESC_JSREGEXP);
		break;
	case S_CSS:
		ADD(ESC_CSSVALUE);
		break;
	case S_TEXT:
		ADD(ESC_HTML);
		break;
	case S_RCDATA:
		ADD(ESC_RCDATA);
		break;
	case S_ATTR:
		break;
	case S_ATTR_NAME:
	case S_TAG:
		c.state = S_ATTR_NAME;
		ADD(ESC_HTMLNAME);
		break;
	case S_SRCSET:
		ADD(ESC_SRCSET);
		break;
	default:
		if (is_comment(c.state))
			ADD(ESC_COMMENT);
	}
	switch (c.delim) {
	case D_NONE:
		break;
	case D_SPACE_OR_TAG_END:
		ADD(ESC_NOSPACE);
		break;
	default:
		ADD(ESC_ATTR);
	}
#undef ADD
	edit_action(e, n, s, k);
	return c;
}

static hctx join_range(hctx c0, hrange *rc) {
	for (size_t i = 0; i < rc->nb; i++) {
		hctx c = rc->breaks[i];
		c0 = join(c0, c, c.n, "range");
		if (c0.state == S_ERROR) {
			herr *err = c0.err;
			err->line = c.n->line;
			alt_buf b = ALT_BUF_INIT;
			alt_buf_printf(&b, "at range loop break: %.*s", (int)err->desc.n, err->desc.p);
			err->desc = hstr(&b);
			return c0;
		}
	}
	for (size_t i = 0; i < rc->nc; i++) {
		hctx c = rc->conts[i];
		c0 = join(c0, c, c.n, "range");
		if (c0.state == S_ERROR) {
			herr *err = c0.err;
			err->line = c.n->line;
			alt_buf b = ALT_BUF_INIT;
			alt_buf_printf(&b, "at range loop continue: %.*s", (int)err->desc.n, err->desc.p);
			err->desc = hstr(&b);
			return c0;
		}
	}
	return c0;
}

enum { FILTER_NONE, FILTER_BODY };

static hctx escape_list_conditionally(hesc *e, hctx c, tnode *n, int filter, alt_str tname, hctx tc, bool *ok);

static hctx escape_branch(hesc *e, hctx c, tnode *n, const char *node_name) {
	bool is_range = !strcmp(node_name, "range");
	if (is_range) {
		hrange *r = halloc(sizeof *r);
		r->outer = e->rc;
		e->rc = r;
	}
	hctx c0 = escape_list(e, ctx_clone(c), n->list);
	if (is_range) {
		if (c0.state != S_ERROR)
			c0 = join_range(c0, e->rc);
		e->rc = e->rc->outer;
		if (c0.state == S_ERROR)
			return c0;
		hrange *r = halloc(sizeof *r);
		r->outer = e->rc;
		e->rc = r;
		bool ok;
		hctx c1 = escape_list_conditionally(e, c0, n->list, FILTER_NONE, ALT_STR_NULL, ctx0(), &ok);
		c0 = join(c0, c1, n, node_name);
		if (c0.state == S_ERROR) {
			e->rc = e->rc->outer;
			herr *err = c0.err;
			err->line = n->line;
			alt_buf b = ALT_BUF_INIT;
			alt_buf_printf(&b, "on range loop re-entry: %.*s", (int)err->desc.n, err->desc.p);
			err->desc = hstr(&b);
			return c0;
		}
		c0 = join_range(c0, e->rc);
		e->rc = e->rc->outer;
		if (c0.state == S_ERROR)
			return c0;
	}
	hctx c1 = escape_list(e, ctx_clone(c), n->else_list);
	return join(c0, c1, n, node_name);
}

static hctx escape_list(hesc *e, hctx c, tnode *n) {
	if (!n)
		return c;
	for (size_t i = 0; i < n->nn; i++) {
		c = escape_node(e, c, n->nodes[i]);
		if (c.state == S_DEAD)
			break;
	}
	return c;
}

static void esc_init(hesc *e, alt_template *t) {
	memset(e, 0, sizeof *e);
	e->t = t;
}

static hctx escape_tree(hesc *e, hctx c, const tnode *node, alt_str name, int line, alt_str *dname_out);

static hctx escape_list_conditionally(hesc *e, hctx c, tnode *n, int filter, alt_str tname, hctx tc, bool *ok) {
	hesc e1;
	esc_init(&e1, e->t);
	e1.rc = e->rc;
	e1.depth = e->depth;
	for (size_t i = 0; i < e->nout; i++)
		out_set(&e1, e->out[i].name, e->out[i].c);
	c = escape_list(&e1, c, n);
	*ok = false;
	if (filter == FILTER_BODY) {
		if (c.state == S_ERROR)
			*ok = false;
		else if (!called_has(&e1, tname))
			*ok = true;
		else
			*ok = ctx_eq(&tc, &c);
	}
	if (*ok) {
		for (size_t i = 0; i < e1.nout; i++)
			out_set(e, e1.out[i].name, e1.out[i].c);
		for (size_t i = 0; i < e1.nder; i++)
			derived_set(e, e1.der[i].name, e1.der[i].tree);
		for (size_t i = 0; i < e1.ncalled; i++)
			called_add(e, e1.called[i]);
		for (size_t i = 0; i < e1.nae; i++)
			edit_action(e, e1.ae[i].n, e1.ae[i].cmds, e1.ae[i].ncmds);
		for (size_t i = 0; i < e1.nte; i++)
			edit_template(e, e1.te[i].n, e1.te[i].name);
		for (size_t i = 0; i < e1.nxe; i++)
			edit_text(e, e1.xe[i].n, e1.xe[i].text);
	}
	return c;
}

static hctx escape_template_body(hesc *e, hctx c, alt_str name, ttree *t, bool *ok) {
	out_set(e, name, c);
	return escape_list_conditionally(e, c, t->root, FILTER_BODY, name, c, ok);
}

static hctx compute_out_ctx(hesc *e, hctx c, alt_str name, ttree *t) {
	bool ok;
	hctx c1 = escape_template_body(e, c, name, t, &ok);
	if (!ok) {
		bool ok2;
		hctx c2 = escape_template_body(e, c1, name, t, &ok2);
		if (ok2) {
			c1 = c2;
			ok = true;
		}
	}
	if (!ok && c1.state != S_ERROR)
		return ctx_err(herrorf(t->root, 0, "cannot compute output context for template %.*s", (int)name.n, name.p));
	return c1;
}

/* the C stack does not grow: deep recursion of templates is an error */
#define MAX_ESCAPE_DEPTH 1000

static hctx escape_tree(hesc *e, hctx c, const tnode *node, alt_str name, int line, alt_str *dname_out) {
	alt_str dname = mangle(&c, name);
	*dname_out = dname;
	called_add(e, dname);
	hctx *out = out_find(e, dname);
	if (out)
		return *out;
	ttree *t = etemplate(e, name);
	if (!t) {
		if (thtml_entry(e->t->common, name, false))
			return ctx_err(herrorf(node, line, "%s is an incomplete or empty template", q(name, 0)));
		return ctx_err(herrorf(node, line, "no such template %s", q(name, 0)));
	}
	if (!alt_str_eq(dname, name)) {
		ttree *dt = etemplate(e, dname);
		if (!dt) {
			alt_arena *pa = cur->pa;
			dt = alt_arena_zalloc(pa, sizeof *dt);
			dt->name = tstr_dup(pa, dname);
			dt->root = tnode_copy(pa, t->root);
			derived_set(e, dt->name, dt);
		}
		t = dt;
		name = dname;
	}
	if (++e->depth > MAX_ESCAPE_DEPTH) {
		e->depth--;
		return ctx_err(herrorf(node, line, "exceeded maximum template depth (%d)", MAX_ESCAPE_DEPTH));
	}
	hctx r = compute_out_ctx(e, c, name, t);
	e->depth--;
	return r;
}

static hctx escape_template_node(hesc *e, hctx c, tnode *n) {
	alt_str name;
	c = escape_tree(e, c, n, n->text, n->line, &name);
	if (!alt_str_eq(name, n->text))
		edit_template(e, n, name);
	return c;
}

static bool contains_special_script_tag(alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		if (s.p[i] == '<') {
			alt_str r = alt_str_from(s, i + 1);
			if (alt_str_has_prefix_fold(r, ALT_STR("script")) || alt_str_has_prefix_fold(r, ALT_STR("/script")) ||
				alt_str_has_prefix(r, ALT_STR("!--")))
				return true;
		}
	return false;
}

static void escape_special_script_tags(alt_buf *b, alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		if (s.p[i] == '<') {
			alt_str r = alt_str_from(s, i + 1);
			if (alt_str_has_prefix_fold(r, ALT_STR("script")) || alt_str_has_prefix_fold(r, ALT_STR("/script")) ||
				alt_str_has_prefix(r, ALT_STR("!--"))) {
				alt_buf_append_cstr(b, "\\x3C");
				continue;
			}
		}
		alt_buf_append_byte(b, s.p[i]);
	}
}

static bool contains_any_line_term(alt_str s) {
	if (alt_str_index_any(s, "\n\r") >= 0)
		return true;
	return alt_str_contains(s, ALT_STR("\xe2\x80\xa8")) || alt_str_contains(s, ALT_STR("\xe2\x80\xa9"));
}

static hctx escape_text(hesc *e, hctx c, tnode *n) {
	alt_str s = n->text;
	size_t written = 0, i = 0;
	alt_buf b = ALT_BUF_INIT;
	while (i != s.n) {
		size_t nread;
		hctx c1 = context_after_text(c, alt_str_from(s, i), &nread);
		size_t i1 = i + nread;
		if (c.state == S_TEXT || c.state == S_RCDATA) {
			size_t end = i1;
			if (c1.state != c.state) {
				for (size_t j = end; j-- > i;)
					if (s.p[j] == '<') {
						end = j;
						break;
					}
			}
			for (size_t j = i; j < end; j++) {
				if (s.p[j] == '<' && !alt_str_has_prefix_fold(alt_str_from(s, j), ALT_STR("<!DOCTYPE"))) {
					alt_buf_append(&b, s.p + written, j - written);
					alt_buf_append_cstr(&b, "&lt;");
					written = j + 1;
				}
			}
		} else if (is_comment(c.state) && c.delim == D_NONE) {
			if (c.state == S_JS_BLOCK_CMT) {
				if (contains_any_line_term(alt_str_slice(s, written, i1)))
					alt_buf_append_byte(&b, '\n');
				else
					alt_buf_append_byte(&b, ' ');
			} else if (c.state == S_CSS_BLOCK_CMT)
				alt_buf_append_byte(&b, ' ');
			written = i1;
		}
		if (c.state != c1.state && is_comment(c1.state) && c1.delim == D_NONE) {
			size_t cs = i1 - 2;
			if (c1.state == S_HTML_CMT || c1.state == S_JS_HTML_OPEN_CMT)
				cs -= 2;
			else if (c1.state == S_JS_HTML_CLOSE_CMT)
				cs -= 1;
			alt_buf_append(&b, s.p + written, cs - written);
			written = i1;
		}
		if (is_in_script_literal(c.state) && contains_special_script_tag(alt_str_slice(s, i, i1))) {
			alt_buf_append(&b, s.p + written, i - written);
			escape_special_script_tags(&b, alt_str_slice(s, i, i1));
			written = i1;
		}
		if (i == i1 && c.state == c1.state) {
			/* Go panics: "infinite loop"; stop instead */
			c = ctx_err(herrorf(0, 0, "infinite loop from %s", state_names[c.state]));
			break;
		}
		c = c1;
		i = i1;
	}
	if (written != 0 && c.state != S_ERROR) {
		if (!is_comment(c.state) || c.delim != D_NONE)
			alt_buf_append(&b, s.p + written, s.n - written);
		edit_text(e, n, tstr_dup(cur->pa, alt_buf_str(&b)));
	}
	alt_buf_free(&b);
	return c;
}

static hctx escape_node(hesc *e, hctx c, tnode *n) {
	switch (n->type) {
	case N_ACTION:
		return escape_action(e, c, n);
	case N_BREAK:
	case N_CONTINUE: {
		c.n = n;
		hrange *r = e->rc;
		if (n->type == N_BREAK) {
			GROW(r->breaks, r->nb);
			r->breaks[r->nb++] = c;
		} else {
			GROW(r->conts, r->nc);
			r->conts[r->nc++] = c;
		}
		return ctx_state(S_DEAD);
	}
	case N_COMMENT:
		return c;
	case N_IF:
		return escape_branch(e, c, n, "if");
	case N_LIST:
		return escape_list(e, c, n);
	case N_RANGE:
		return escape_branch(e, c, n, "range");
	case N_TEMPLATE:
		return escape_template_node(e, c, n);
	case N_TEXT:
		return escape_text(e, c, n);
	case N_WITH:
		return escape_branch(e, c, n, "with");
	}
	return c;
}

/* ---- committing the edits ---- */

static const char *equiv(alt_str e) {
	static const char *const m[][2] = {{"_html_template_attrescaper", "html"}, {"_html_template_htmlescaper", "html"},
		{"_html_template_rcdataescaper", "html"}, {"_html_template_urlescaper", "urlquery"},
		{"_html_template_urlnormalizer", "urlquery"}};
	for (size_t i = 0; i < 5; i++)
		if (alt_str_eq_cstr(e, m[i][0]))
			return m[i][1];
	return 0;
}

static bool norm_eq(alt_str a, alt_str b) {
	const char *x = equiv(a), *y = equiv(b);
	alt_str na = x ? alt_str_cstr(x) : a, nb = y ? alt_str_cstr(y) : b;
	return alt_str_eq(na, nb);
}

static bool redundant(alt_str last, alt_str next) {
	static const char *const m[][2] = {{"_html_template_commentescaper", "_html_template_attrescaper"},
		{"_html_template_commentescaper", "_html_template_htmlescaper"}, {"_html_template_cssescaper", "_html_template_attrescaper"},
		{"_html_template_jsregexpescaper", "_html_template_attrescaper"}, {"_html_template_jsstrescaper", "_html_template_attrescaper"},
		{"_html_template_jstmpllitescaper", "_html_template_attrescaper"}, {"_html_template_urlescaper", "_html_template_urlnormalizer"}};
	for (size_t i = 0; i < sizeof m / sizeof m[0]; i++)
		if (alt_str_eq_cstr(last, m[i][0]) && alt_str_eq_cstr(next, m[i][1]))
			return true;
	return false;
}

static tnode *new_ident_cmd(alt_arena *a, alt_str name, const tnode *pipe) {
	tnode *cmd = tnode_new(a, N_COMMAND, 0, pipe->tr);
	tnode *id = tnode_new(a, N_IDENTIFIER, pipe->pos, pipe->tr);
	id->text = tstr_dup(a, name);
	tnode_append(a, cmd, id);
	return cmd;
}

/* appendCmd onto list (a command list as in a PIPE node) */
static void append_cmd(alt_arena *a, tnode *list, tnode *cmd) {
	if (list->nn) {
		tnode *last = list->nodes[list->nn - 1]->nodes[0], *next = cmd->nodes[0];
		if (last->type == N_IDENTIFIER && next->type == N_IDENTIFIER && redundant(last->text, next->text))
			return;
	}
	tnode_append(a, list, cmd);
}

static void ensure_pipeline_contains(alt_arena *a, tnode *p, alt_str *s, size_t ns) {
	if (!ns)
		return;
	size_t plen = p->nn;
	if (plen > 0) {
		tnode *last = p->nodes[plen - 1];
		tnode *id = last->nodes[0];
		if (id->type == N_IDENTIFIER && is_predefined(id->text)) {
			alt_str esc = id->text;
			if (p->nn == 1 && last->nn > 1) {
				tnode *ea = tnode_new(a, N_IDENTIFIER, id->pos, 0);
				ea->tr = id->tr;
				ea->text = ALT_STR("_eval_args_");
				last->nodes[0] = ea;
				append_cmd(a, p, new_ident_cmd(a, esc, p));
				plen++;
			}
			bool dup = false;
			for (size_t i = 0; i < ns; i++)
				if (norm_eq(esc, s[i])) {
					s[i] = esc;
					dup = true;
				}
			if (dup)
				plen--;
		}
	}
	tnode tmp;
	memset(&tmp, 0, sizeof tmp);
	tmp.type = N_PIPE;
	for (size_t i = 0; i < plen; i++)
		tnode_append(a, &tmp, p->nodes[i]);
	for (size_t i = 0; i < ns; i++) {
		const char *ne = equiv(s[i]);
		alt_str norm = ne ? alt_str_cstr(ne) : s[i];
		bool inserted = false;
		for (size_t j = 0; j < plen; j++) {
			tnode *id = p->nodes[j]->nodes[0];
			if (id->type == N_IDENTIFIER) {
				const char *x = equiv(id->text);
				if (alt_str_eq(x ? alt_str_cstr(x) : id->text, norm))
					inserted = true;
			}
		}
		if (!inserted)
			append_cmd(a, &tmp, new_ident_cmd(a, s[i], p));
	}
	p->nodes = tmp.nodes;
	p->nn = tmp.nn;
	p->ncap = tmp.ncap;
}

static void commit(hesc *e) {
	tcommon *c = e->t->common;
	alt_arena *pa = &c->arena;
	for (size_t i = 0; i < e->nder; i++)
		ttemplate_add_tree(e->t, e->der[i].name, e->der[i].tree);
	for (size_t i = 0; i < e->nae; i++)
		ensure_pipeline_contains(pa, e->ae[i].n->node, e->ae[i].cmds, e->ae[i].ncmds);
	for (size_t i = 0; i < e->nte; i++)
		e->te[i].n->text = tstr_dup(pa, e->te[i].name);
	for (size_t i = 0; i < e->nxe; i++)
		e->xe[i].n->text = e->xe[i].text;
}

/* escapeTemplate: 0, or -EINVAL with the error recorded for name */
static int escape_template(alt_template *t, tnode *node, alt_str name, alt_buf *errb) {
	tcommon *c = t->common;
	alt_arena scratch;
	alt_arena_init(&scratch, 0);
	harena h = {&scratch, &c->arena}, *saved = cur;
	cur = &h;
	hesc e;
	esc_init(&e, t);
	alt_str dname;
	hctx ctx = escape_tree(&e, ctx0(), node, name, 0, &dname);
	herr *err = 0;
	if (ctx.err) {
		err = ctx.err;
		err->name = name;
	} else if (ctx.state != S_TEXT) {
		alt_buf b = ALT_BUF_INIT;
		ctx_string(&b, &ctx);
		err = herrorf(0, 0, "ends in a non-text context: %s", alt_buf_cstr(&b));
		alt_buf_free(&b);
		err->name = name;
	}
	int rc = 0;
	hentry *he = thtml_entry(c, name, false);
	if (err) {
		alt_buf b = ALT_BUF_INIT;
		herr_string(&b, err);
		alt_buf_append_str(errb, alt_buf_str(&b));
		if (he) {
			he->state = -1;
			alt_buf_reset(&he->err);
			alt_buf_append_str(&he->err, alt_buf_str(&b));
			alt_template *tt = ttemplate_lookup(t, name);
			if (tt)
				tt->tree = 0;
			if (t != tt && alt_str_eq(t->name, name))
				t->tree = 0;
		}
		alt_buf_free(&b);
		rc = -EINVAL;
	} else {
		commit(&e);
		if (he)
			he->state = 1;
	}
	cur = saved;
	alt_arena_free(&scratch);
	return rc;
}

int thtml_escape_template(alt_template *t, alt_buf *err) {
	tcommon *c = t->common;
	alt_mutex_lock(&c->mu);
	c->escaped = true;
	hentry *he = thtml_entry(c, t->name, false);
	int rc = 0;
	if (!he || he->state == 0) {
		if (!t->tree) {
			alt_buf_append_cstr(err, "template: ");
			tquote(err, t->name);
			alt_buf_append_cstr(err, " is an incomplete or empty template");
			rc = -EINVAL;
		} else
			rc = escape_template(t, t->tree->root, t->name, err);
	} else if (he->state < 0) {
		alt_buf_append_str(err, alt_buf_str(&he->err));
		rc = -EINVAL;
	}
	alt_mutex_unlock(&c->mu);
	return rc;
}

int thtml_execute_name(alt_template *t, alt_buf *out, alt_str name, const alt_json *data, alt_buf *err) {
	tcommon *c = t->common;
	alt_mutex_lock(&c->mu);
	c->escaped = true;
	hentry *he = thtml_entry(c, name, false);
	alt_template *tt = ttemplate_lookup(t, name);
	int rc = 0;
	if (!he) {
		alt_buf_append_cstr(err, "html/template: ");
		tquote(err, name);
		alt_buf_append_cstr(err, " is undefined");
		rc = -EINVAL;
	} else if (he->state < 0) {
		alt_buf_append_str(err, alt_buf_str(&he->err));
		rc = -EINVAL;
	} else if (!tt || !tt->tree || !tt->tree->root) {
		alt_buf_append_cstr(err, "html/template: ");
		tquote(err, name);
		alt_buf_append_cstr(err, " is an incomplete template");
		rc = -EINVAL;
	} else if (he->state == 0)
		rc = escape_template(tt, tt->tree->root, name, err);
	alt_mutex_unlock(&c->mu);
	if (rc < 0)
		return rc;
	return texec(tt, out, data, err);
}

int alt_html_template_escape(alt_template *t, alt_buf *err) {
	alt_buf tmp = ALT_BUF_INIT;
	int rc = thtml_escape_template(t, err ? err : &tmp);
	alt_buf_free(&tmp);
	return rc;
}
