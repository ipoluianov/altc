/* Headers, status texts, dates (after Go's net/http and net/textproto;
   BSD-3-Clause, see LICENSE-Go) */
#include <alt/errors.h>
#include <alt/net/http.h>
#include <alt/time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

const char *alt_http_status_text(int code) {
	switch (code) {
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 102: return "Processing";
	case 103: return "Early Hints";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 207: return "Multi-Status";
	case 208: return "Already Reported";
	case 226: return "IM Used";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 307: return "Temporary Redirect";
	case 308: return "Permanent Redirect";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Timeout";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Request Entity Too Large";
	case 414: return "Request URI Too Long";
	case 415: return "Unsupported Media Type";
	case 416: return "Requested Range Not Satisfiable";
	case 417: return "Expectation Failed";
	case 418: return "I'm a teapot";
	case 421: return "Misdirected Request";
	case 422: return "Unprocessable Entity";
	case 423: return "Locked";
	case 424: return "Failed Dependency";
	case 425: return "Too Early";
	case 426: return "Upgrade Required";
	case 428: return "Precondition Required";
	case 429: return "Too Many Requests";
	case 431: return "Request Header Fields Too Large";
	case 451: return "Unavailable For Legal Reasons";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Timeout";
	case 505: return "HTTP Version Not Supported";
	case 506: return "Variant Also Negotiates";
	case 507: return "Insufficient Storage";
	case 508: return "Loop Detected";
	case 510: return "Not Extended";
	case 511: return "Network Authentication Required";
	}
	return "";
}

/* token characters of RFC 9110 (textproto's validHeaderFieldByte) */
static bool field_byte(unsigned char c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return true;
	return c && strchr("!#$%&'*+-.^_`|~", c);
}

bool alt_http_valid_field_name(alt_str s) {
	if (s.n == 0)
		return false;
	for (size_t i = 0; i < s.n; i++)
		if (!field_byte((unsigned char)s.p[i]))
			return false;
	return true;
}

bool alt_http_valid_field_value(alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if ((c < 0x20 && c != '\t') || c == 0x7f)
			return false;
	}
	return true;
}

alt_str alt_http_canonical_key(alt_arena *a, alt_str key) {
	bool canon = true;
	for (size_t i = 0; i < key.n; i++)
		if (!field_byte((unsigned char)key.p[i]))
			canon = false;
	char *p = alt_arena_alloc(a, key.n + 1);
	if (!p)
		return ALT_STR_NULL;
	memcpy(p, key.p, key.n);
	p[key.n] = 0;
	if (canon) {
		bool upper = true;
		for (size_t i = 0; i < key.n; i++) {
			char c = p[i];
			if (upper && c >= 'a' && c <= 'z')
				c -= 32;
			else if (!upper && c >= 'A' && c <= 'Z')
				c += 32;
			p[i] = c;
			upper = c == '-';
		}
	}
	return alt_str_make(p, key.n);
}

void alt_http_header_init(alt_http_header *h, alt_arena *a) {
	h->f = 0;
	h->n = h->cap = 0;
	h->arena = a;
}

int alt_http_header_add(alt_http_header *h, alt_str key, alt_str value) {
	if (h->n == h->cap) {
		size_t cap = h->cap ? h->cap * 2 : 16;
		alt_http_field *f = alt_arena_realloc(h->arena, h->f, h->cap * sizeof *f, cap * sizeof *f);
		if (!f)
			return -ENOMEM;
		h->f = f;
		h->cap = cap;
	}
	alt_str k = alt_http_canonical_key(h->arena, key), v = alt_arena_str(h->arena, value);
	if (!k.p || !v.p)
		return -ENOMEM;
	h->f[h->n++] = (alt_http_field){k, v};
	return 0;
}

void alt_http_header_del(alt_http_header *h, alt_str key) {
	size_t j = 0;
	for (size_t i = 0; i < h->n; i++)
		if (!alt_str_eq_fold(h->f[i].key, key))
			h->f[j++] = h->f[i];
	h->n = j;
}

int alt_http_header_set(alt_http_header *h, alt_str key, alt_str value) {
	/* the first occurrence keeps its place */
	for (size_t i = 0; i < h->n; i++)
		if (alt_str_eq_fold(h->f[i].key, key)) {
			alt_str v = alt_arena_str(h->arena, value);
			if (!v.p)
				return -ENOMEM;
			h->f[i].value = v;
			size_t j = i + 1;
			for (size_t k = i + 1; k < h->n; k++)
				if (!alt_str_eq_fold(h->f[k].key, key))
					h->f[j++] = h->f[k];
			h->n = j;
			return 0;
		}
	return alt_http_header_add(h, key, value);
}

bool alt_http_header_get(const alt_http_header *h, alt_str key, alt_str *value) {
	for (size_t i = 0; i < h->n; i++)
		if (alt_str_eq_fold(h->f[i].key, key)) {
			if (value)
				*value = h->f[i].value;
			return true;
		}
	if (value)
		*value = ALT_STR("");
	return false;
}

alt_str alt_http_header_value(const alt_http_header *h, const char *key) {
	alt_str v;
	alt_http_header_get(h, alt_str_cstr(key), &v);
	return v;
}

bool alt_http_header_has_token(const alt_http_header *h, const char *key, const char *token) {
	alt_str k = alt_str_cstr(key), t = alt_str_cstr(token);
	for (size_t i = 0; i < h->n; i++) {
		if (!alt_str_eq_fold(h->f[i].key, k))
			continue;
		alt_str rest = h->f[i].value, part;
		while (alt_str_split_next(&rest, ALT_STR(","), &part))
			if (alt_str_eq_fold(alt_str_trim(part, " \t"), t))
				return true;
	}
	return false;
}

int alt_http_header_write(const alt_http_header *h, alt_buf *b) {
	/* sorted by key, the values of a key in order (stable) */
	size_t n = h->n;
	size_t *idx = malloc((n ? n : 1) * sizeof *idx);
	if (!idx)
		return -ENOMEM;
	for (size_t i = 0; i < n; i++)
		idx[i] = i;
	for (size_t i = 1; i < n; i++) {
		size_t x = idx[i], j = i;
		while (j > 0 && alt_str_cmp(h->f[idx[j - 1]].key, h->f[x].key) > 0) {
			idx[j] = idx[j - 1];
			j--;
		}
		idx[j] = x;
	}
	for (size_t i = 0; i < n; i++) {
		const alt_http_field *f = &h->f[idx[i]];
		alt_buf_append_str(b, f->key);
		alt_buf_append_cstr(b, ": ");
		/* line breaks in values would split the header: they become spaces */
		for (size_t k = 0; k < f->value.n; k++) {
			char c = f->value.p[k];
			alt_buf_append_byte(b, c == '\r' || c == '\n' ? ' ' : c);
		}
		alt_buf_append_cstr(b, "\r\n");
	}
	free(idx);
	return b->err;
}

/* ---- dates ---- */

int alt_http_format_date(alt_buf *b, alt_wall t) { return alt_wall_format(b, t, &alt_zone_utc, ALT_LAYOUT_HTTP); }

int alt_http_parse_date(alt_str s, alt_wall *t) {
	/* the three forms of RFC 9110 section 5.6.7, as Go's http.ParseTime */
	static const char *const layouts[] = {ALT_LAYOUT_HTTP, ALT_LAYOUT_RFC850, ALT_LAYOUT_ANSIC};
	for (size_t i = 0; i < 3; i++)
		if (alt_wall_parse(s, layouts[i], t, 0) == 0)
			return 0;
	return -ALT_ESYNTAX;
}

/* ---- cookies ---- */

bool alt_http_cookie(const alt_http_request *r, const char *name, alt_str *value) {
	alt_str want = alt_str_cstr(name);
	for (size_t i = 0; i < r->header.n; i++) {
		if (!alt_str_eq_cstr(r->header.f[i].key, "Cookie"))
			continue;
		alt_str rest = r->header.f[i].value, part;
		while (alt_str_split_next(&rest, ALT_STR(";"), &part)) {
			part = alt_str_trim(part, " ");
			alt_str k, v;
			if (!alt_str_cut_byte(part, '=', &k, &v))
				continue;
			if (alt_str_eq(k, want)) {
				if (v.n >= 2 && v.p[0] == '"' && v.p[v.n - 1] == '"')
					v = alt_str_slice(v, 1, v.n - 1);
				if (value)
					*value = v;
				return true;
			}
		}
	}
	return false;
}

static bool cookie_value_byte(unsigned char c) { return c >= 0x20 && c < 0x7f && c != '"' && c != ';' && c != '\\'; }

int alt_http_set_cookie(alt_http_rw *w, const alt_http_cookie_spec *c) {
	if (!c->name || !alt_http_valid_field_name(alt_str_cstr(c->name)))
		return -EINVAL;
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, c->name);
	alt_buf_append_byte(&b, '=');
	const char *v = c->value ? c->value : "";
	bool quote = strchr(v, ' ') || strchr(v, ',');
	if (quote)
		alt_buf_append_byte(&b, '"');
	for (const char *p = v; *p; p++)
		if (cookie_value_byte((unsigned char)*p))
			alt_buf_append_byte(&b, *p);
	if (quote)
		alt_buf_append_byte(&b, '"');
	if (c->path)
		alt_buf_printf(&b, "; Path=%s", c->path);
	if (c->domain)
		alt_buf_printf(&b, "; Domain=%s", c->domain[0] == '.' ? c->domain + 1 : c->domain);
	if (c->expires) {
		alt_buf_append_cstr(&b, "; Expires=");
		alt_http_format_date(&b, c->expires);
	}
	if (c->max_age > 0)
		alt_buf_printf(&b, "; Max-Age=%d", c->max_age);
	else if (c->max_age < 0)
		alt_buf_append_cstr(&b, "; Max-Age=0");
	if (c->http_only)
		alt_buf_append_cstr(&b, "; HttpOnly");
	if (c->secure)
		alt_buf_append_cstr(&b, "; Secure");
	if (c->same_site)
		alt_buf_printf(&b, "; SameSite=%s", c->same_site);
	int rc = b.err ? b.err : alt_http_header_add(alt_http_rw_header(w), ALT_STR("Set-Cookie"), alt_buf_str(&b));
	alt_buf_free(&b);
	return rc;
}
