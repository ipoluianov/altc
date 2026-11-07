/* URLs, translated from Go's net/url (url.go, gen_encoding_table.go;
   BSD-3-Clause, see LICENSE-Go) */
#include <alt/errors.h>
#include <alt/net.h>
#include <alt/net/url.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

enum {
	ENC_PATH = 1,
	ENC_PATH_SEGMENT = 2,
	ENC_HOST = 4,
	ENC_ZONE = 8,
	ENC_USER_PASSWORD = 16,
	ENC_QUERY_COMPONENT = 32,
	ENC_FRAGMENT = 64,
};

static bool should_escape(unsigned char c, int mode) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return false;
	if (mode == ENC_HOST || mode == ENC_ZONE) {
		switch (c) {
		case '!': case '$': case '&': case '\'': case '(': case ')': case '*': case '+': case ',': case ';': case '=':
		case ':': case '[': case ']': case '<': case '>': case '"':
			return false;
		}
	}
	switch (c) {
	case '-': case '_': case '.': case '~':
		return false;
	case '$': case '&': case '+': case ',': case '/': case ':': case ';': case '=': case '?': case '@':
		switch (mode) {
		case ENC_PATH: return c == '?';
		case ENC_PATH_SEGMENT: return c == '/' || c == ';' || c == ',' || c == '?';
		case ENC_USER_PASSWORD: return c == '@' || c == '/' || c == '?' || c == ':';
		case ENC_QUERY_COMPONENT: return true;
		case ENC_FRAGMENT: return false;
		}
	}
	if (mode == ENC_FRAGMENT) {
		switch (c) {
		case '!': case '(': case ')': case '*':
			return false;
		}
	}
	return true;
}

static bool ishex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
static unsigned unhex(char c) { return c <= '9' ? (unsigned)(c - '0') : (unsigned)((c | 32) - 'a' + 10); }

static int unescape(alt_arena *a, alt_str s, int mode, alt_str *out) {
	size_t n = 0;
	bool plus = false;
	for (size_t i = 0; i < s.n;) {
		unsigned char c = (unsigned char)s.p[i];
		if (c == '%') {
			n++;
			if (i + 2 >= s.n || !ishex(s.p[i + 1]) || !ishex(s.p[i + 2]))
				return -ALT_EURL_ESCAPE;
			bool pct25 = s.p[i + 1] == '2' && s.p[i + 2] == '5';
			if (mode == ENC_HOST && unhex(s.p[i + 1]) < 8 && !pct25)
				return -ALT_EURL_ESCAPE;
			if (mode == ENC_ZONE) {
				unsigned v = unhex(s.p[i + 1]) << 4 | unhex(s.p[i + 2]);
				if (!pct25 && v != ' ' && should_escape((unsigned char)v, ENC_HOST))
					return -ALT_EURL_ESCAPE;
			}
			i += 3;
		} else if (c == '+') {
			plus = mode == ENC_QUERY_COMPONENT;
			i++;
		} else {
			if ((mode == ENC_HOST || mode == ENC_ZONE) && c < 0x80 && should_escape(c, mode))
				return -ALT_EURL_HOST;
			i++;
		}
	}
	if (n == 0 && !plus) {
		*out = s;
		return 0;
	}
	char *p = alt_arena_alloc(a, s.n + 1);
	if (!p)
		return -ENOMEM;
	size_t j = 0;
	for (size_t i = 0; i < s.n; i++) {
		if (s.p[i] == '%') {
			p[j++] = (char)(unhex(s.p[i + 1]) << 4 | unhex(s.p[i + 2]));
			i += 2;
		} else if (s.p[i] == '+')
			p[j++] = mode == ENC_QUERY_COMPONENT ? ' ' : '+';
		else
			p[j++] = s.p[i];
	}
	p[j] = 0;
	*out = alt_str_make(p, j);
	return 0;
}

static int escape(alt_buf *b, alt_str s, int mode) {
	static const char hex[] = "0123456789ABCDEF";
	int rc = alt_buf_reserve(b, s.n);
	if (rc)
		return rc;
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (c == ' ' && mode == ENC_QUERY_COMPONENT)
			alt_buf_append_byte(b, '+');
		else if (should_escape(c, mode)) {
			char e[3] = {'%', hex[c >> 4], hex[c & 15]};
			alt_buf_append(b, e, 3);
		} else
			alt_buf_append_byte(b, c);
	}
	return b->err;
}

int alt_url_query_escape(alt_buf *b, alt_str s) { return escape(b, s, ENC_QUERY_COMPONENT); }
int alt_url_path_escape(alt_buf *b, alt_str s) { return escape(b, s, ENC_PATH_SEGMENT); }
int alt_url_query_unescape(alt_arena *a, alt_str s, alt_str *out) { return unescape(a, s, ENC_QUERY_COMPONENT, out); }
int alt_url_path_unescape(alt_arena *a, alt_str s, alt_str *out) { return unescape(a, s, ENC_PATH_SEGMENT, out); }

static bool has_ctl(alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		if ((unsigned char)s.p[i] < ' ' || s.p[i] == 0x7f)
			return true;
	return false;
}

static int get_scheme(alt_str raw, alt_str *scheme, alt_str *rest) {
	for (size_t i = 0; i < raw.n; i++) {
		char c = raw.p[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
			continue;
		if ((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.') {
			if (i == 0)
				break;
			continue;
		}
		if (c == ':') {
			if (i == 0)
				return -ALT_EURL_PARSE; /* missing protocol scheme */
			*scheme = alt_str_slice(raw, 0, i);
			*rest = alt_str_from(raw, i + 1);
			return 0;
		}
		break;
	}
	*scheme = ALT_STR_NULL;
	*rest = raw;
	return 0;
}

static bool valid_optional_port(alt_str port) {
	if (port.n == 0)
		return true;
	if (port.p[0] != ':')
		return false;
	for (size_t i = 1; i < port.n; i++)
		if (port.p[i] < '0' || port.p[i] > '9')
			return false;
	return true;
}

static bool valid_userinfo(alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
			continue;
		if (strchr("-._:~!$&'()*+,;=%@", c) && c)
			continue;
		return false;
	}
	return true;
}

static alt_str concat(alt_arena *a, alt_str x, alt_str y, alt_str z) {
	char *p = alt_arena_alloc(a, x.n + y.n + z.n + 1);
	if (!p)
		return ALT_STR_NULL;
	memcpy(p, x.p, x.n);
	memcpy(p + x.n, y.p, y.n);
	memcpy(p + x.n + y.n, z.p, z.n);
	p[x.n + y.n + z.n] = 0;
	return alt_str_make(p, x.n + y.n + z.n);
}

static int parse_host(alt_arena *a, alt_str scheme, alt_str host, alt_str *out) {
	ssize_t ob = alt_str_last_index_byte(host, '[');
	int rc;
	if (ob > 0)
		return -ALT_EURL_HOST; /* invalid IP-literal */
	if (ob == 0) {
		ssize_t cb = alt_str_last_index_byte(host, ']');
		if (cb < 0)
			return -ALT_EURL_HOST;
		alt_str colon_port = alt_str_from(host, (size_t)cb + 1), ucp;
		if (!valid_optional_port(colon_port))
			return -ALT_EURL_PORT;
		if ((rc = unescape(a, colon_port, ENC_HOST, &ucp)))
			return rc;
		alt_str hostname = alt_str_slice(host, 1, (size_t)cb), uh;
		ssize_t z = alt_str_index(hostname, ALT_STR("%25"));
		if (z >= 0) {
			alt_str hp, zp;
			if ((rc = unescape(a, alt_str_slice(hostname, 0, (size_t)z), ENC_HOST, &hp)))
				return rc;
			if ((rc = unescape(a, alt_str_from(hostname, (size_t)z), ENC_ZONE, &zp)))
				return rc;
			uh = concat(a, hp, zp, ALT_STR(""));
		} else if ((rc = unescape(a, hostname, ENC_HOST, &uh)))
			return rc;
		alt_ip ip;
		if (alt_ip_parse(uh, &ip) < 0 || ip.is4)
			return -ALT_EURL_HOST;
		*out = concat(a, ALT_STR("["), uh, concat(a, ALT_STR("]"), ucp, ALT_STR("")));
		return out->p ? 0 : -ENOMEM;
	}
	ssize_t i = alt_str_index_byte(host, ':');
	if (i >= 0) {
		ssize_t last = alt_str_last_index_byte(host, ':');
		if (last != i && !(alt_str_eq_cstr(scheme, "http") || alt_str_eq_cstr(scheme, "https")))
			i = last;
		if (!valid_optional_port(alt_str_from(host, (size_t)i)))
			return -ALT_EURL_PORT;
	}
	return unescape(a, host, ENC_HOST, out);
}

static int parse_authority(alt_arena *a, alt_url *u, alt_str authority) {
	ssize_t at = alt_str_last_index_byte(authority, '@');
	int rc = parse_host(a, u->scheme, at < 0 ? authority : alt_str_from(authority, (size_t)at + 1), &u->host);
	if (rc)
		return rc;
	if (at < 0)
		return 0;
	alt_str info = alt_str_slice(authority, 0, (size_t)at);
	if (!valid_userinfo(info))
		return -ALT_EURL_PARSE;
	u->has_user = true;
	alt_str user, pass;
	if (!alt_str_cut_byte(info, ':', &user, &pass))
		return unescape(a, info, ENC_USER_PASSWORD, &u->username);
	if ((rc = unescape(a, user, ENC_USER_PASSWORD, &u->username)))
		return rc;
	u->has_password = true;
	return unescape(a, pass, ENC_USER_PASSWORD, &u->password);
}

int alt_url_set_path(alt_arena *a, alt_url *u, alt_str p) {
	/* p is the encoded path */
	alt_str path;
	int rc = unescape(a, p, ENC_PATH, &path);
	if (rc)
		return rc;
	u->path = path;
	alt_buf b = ALT_BUF_INIT;
	escape(&b, path, ENC_PATH);
	u->raw_path = alt_str_eq(alt_buf_str(&b), p) ? ALT_STR_NULL : p;
	rc = b.err;
	alt_buf_free(&b);
	return rc;
}

static int set_fragment(alt_arena *a, alt_url *u, alt_str f) {
	alt_str frag;
	int rc = unescape(a, f, ENC_FRAGMENT, &frag);
	if (rc)
		return rc;
	u->fragment = frag;
	alt_buf b = ALT_BUF_INIT;
	escape(&b, frag, ENC_FRAGMENT);
	u->raw_fragment = alt_str_eq(alt_buf_str(&b), f) ? ALT_STR_NULL : f;
	rc = b.err;
	alt_buf_free(&b);
	return rc;
}

static int parse(alt_arena *a, alt_str raw, bool via_request, alt_url *u) {
	memset(u, 0, sizeof *u);
	if (has_ctl(raw))
		return -ALT_EURL_PARSE;
	if (raw.n == 0 && via_request)
		return -ALT_EURL_PARSE;
	if (alt_str_eq_cstr(raw, "*")) {
		u->path = ALT_STR("*");
		return 0;
	}
	alt_str rest;
	int rc = get_scheme(raw, &u->scheme, &rest);
	if (rc)
		return rc;
	if (u->scheme.n) {
		char *s = alt_arena_alloc(a, u->scheme.n + 1);
		if (!s)
			return -ENOMEM;
		memcpy(s, u->scheme.p, u->scheme.n);
		s[u->scheme.n] = 0;
		alt_ascii_lower(s, u->scheme.n);
		u->scheme = alt_str_make(s, u->scheme.n);
	}
	if (alt_str_has_suffix(rest, ALT_STR("?")) && alt_str_count(rest, ALT_STR("?")) == 1) {
		u->force_query = true;
		rest.n--;
	} else {
		alt_str q;
		if (alt_str_cut_byte(rest, '?', &rest, &q))
			u->raw_query = q;
	}
	if (!alt_str_has_prefix(rest, ALT_STR("/"))) {
		if (u->scheme.n) {
			u->opaque = rest;
			return 0;
		}
		if (via_request)
			return -ALT_EURL_PARSE;
		alt_str seg;
		alt_str_cut_byte(rest, '/', &seg, 0);
		if (alt_str_index_byte(seg, ':') >= 0)
			return -ALT_EURL_PARSE; /* first path segment cannot contain colon */
	}
	if ((u->scheme.n || (!via_request && !alt_str_has_prefix(rest, ALT_STR("///")))) && alt_str_has_prefix(rest, ALT_STR("//"))) {
		alt_str authority = alt_str_from(rest, 2);
		rest = ALT_STR("");
		ssize_t i = alt_str_index_byte(authority, '/');
		if (i >= 0) {
			rest = alt_str_from(authority, (size_t)i);
			authority = alt_str_slice(authority, 0, (size_t)i);
		}
		if ((rc = parse_authority(a, u, authority)))
			return rc;
	} else if (u->scheme.n && alt_str_has_prefix(rest, ALT_STR("/")))
		u->omit_host = true;
	return alt_url_set_path(a, u, rest);
}

int alt_url_parse(alt_arena *a, alt_str raw, alt_url *u) {
	alt_str base, frag;
	bool has_frag = alt_str_cut_byte(raw, '#', &base, &frag);
	int rc = parse(a, base, false, u);
	if (rc || !has_frag || frag.n == 0)
		return rc;
	return set_fragment(a, u, frag);
}

int alt_url_parse_request_uri(alt_arena *a, alt_str raw, alt_url *u) { return parse(a, raw, true, u); }

static bool valid_encoded(alt_str s, int mode) {
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (strchr("!$&'()*+,;=:@[]%", c) && c)
			continue;
		if (should_escape(c, mode))
			return false;
	}
	return true;
}

/* the raw form when valid and equivalent, else the escaped decoded form */
static int escaped(alt_buf *b, alt_str raw, alt_str dec, int mode) {
	if (raw.n && valid_encoded(raw, mode)) {
		alt_arena tmp;
		alt_arena_init(&tmp, 0);
		alt_str d;
		bool same = unescape(&tmp, raw, mode, &d) == 0 && alt_str_eq(d, dec);
		alt_arena_free(&tmp);
		if (same)
			return alt_buf_append_str(b, raw);
	}
	if (mode == ENC_PATH && alt_str_eq_cstr(dec, "*"))
		return alt_buf_append_cstr(b, "*");
	return escape(b, dec, mode);
}

int alt_url_escaped_path(const alt_url *u, alt_buf *b) { return escaped(b, u->raw_path, u->path, ENC_PATH); }
int alt_url_escaped_fragment(const alt_url *u, alt_buf *b) { return escaped(b, u->raw_fragment, u->fragment, ENC_FRAGMENT); }

int alt_url_string(const alt_url *u, alt_buf *b) {
	size_t start = b->n;
	if (u->scheme.n) {
		alt_buf_append_str(b, u->scheme);
		alt_buf_append_byte(b, ':');
	}
	if (u->opaque.n)
		alt_buf_append_str(b, u->opaque);
	else {
		if (u->scheme.n || u->host.n || u->has_user) {
			if (!(u->omit_host && u->host.n == 0 && !u->has_user)) {
				if (u->host.n || u->path.n || u->has_user)
					alt_buf_append_cstr(b, "//");
				if (u->has_user) {
					escape(b, u->username, ENC_USER_PASSWORD);
					if (u->has_password) {
						alt_buf_append_byte(b, ':');
						escape(b, u->password, ENC_USER_PASSWORD);
					}
					alt_buf_append_byte(b, '@');
				}
				if (u->host.n)
					escape(b, u->host, ENC_HOST);
			}
		}
		alt_buf path = ALT_BUF_INIT;
		alt_url_escaped_path(u, &path);
		alt_str p = alt_buf_str(&path);
		if (u->omit_host && u->host.n == 0 && !u->has_user && alt_str_has_prefix(p, ALT_STR("//"))) {
			alt_buf_append_cstr(b, "%2F");
			p = alt_str_from(p, 1);
		}
		if (p.n && p.p[0] != '/' && u->host.n)
			alt_buf_append_byte(b, '/');
		if (b->n == start) {
			alt_str seg;
			alt_str_cut_byte(p, '/', &seg, 0);
			if (alt_str_index_byte(seg, ':') >= 0)
				alt_buf_append_cstr(b, "./");
		}
		alt_buf_append_str(b, p);
		alt_buf_free(&path);
	}
	if (u->force_query || u->raw_query.n) {
		alt_buf_append_byte(b, '?');
		alt_buf_append_str(b, u->raw_query);
	}
	if (u->fragment.n) {
		alt_buf_append_byte(b, '#');
		alt_url_escaped_fragment(u, b);
	}
	return b->err;
}

int alt_url_request_uri(const alt_url *u, alt_buf *b) {
	if (u->opaque.n) {
		if (alt_str_has_prefix(u->opaque, ALT_STR("//"))) {
			alt_buf_append_str(b, u->scheme);
			alt_buf_append_byte(b, ':');
		}
		alt_buf_append_str(b, u->opaque);
	} else {
		size_t start = b->n;
		alt_url_escaped_path(u, b);
		if (b->n == start)
			alt_buf_append_byte(b, '/');
	}
	if (u->force_query || u->raw_query.n) {
		alt_buf_append_byte(b, '?');
		alt_buf_append_str(b, u->raw_query);
	}
	return b->err;
}

static void split_host_port(alt_str hp, alt_str *host, alt_str *port) {
	*host = hp;
	*port = ALT_STR_NULL;
	ssize_t c = alt_str_last_index_byte(hp, ':');
	if (c >= 0 && valid_optional_port(alt_str_from(hp, (size_t)c))) {
		*host = alt_str_slice(hp, 0, (size_t)c);
		*port = alt_str_from(hp, (size_t)c + 1);
	}
	if (alt_str_has_prefix(*host, ALT_STR("[")) && alt_str_has_suffix(*host, ALT_STR("]")))
		*host = alt_str_slice(*host, 1, host->n - 1);
}

alt_str alt_url_hostname(const alt_url *u) {
	alt_str h, p;
	split_host_port(u->host, &h, &p);
	return h;
}

alt_str alt_url_port(const alt_url *u) {
	alt_str h, p;
	split_host_port(u->host, &h, &p);
	return p;
}

bool alt_url_is_abs(const alt_url *u) { return u->scheme.n != 0; }

/* RFC 3986 section 5.2.4, as Go's resolvePath */
static alt_str resolve_path(alt_arena *a, alt_str base, alt_str ref) {
	alt_str full;
	if (ref.n == 0)
		full = base;
	else if (ref.p[0] != '/') {
		ssize_t i = alt_str_last_index_byte(base, '/');
		full = concat(a, alt_str_slice(base, 0, (size_t)(i + 1)), ref, ALT_STR(""));
	} else
		full = ref;
	if (full.n == 0)
		return ALT_STR("");
	char *dst = alt_arena_alloc(a, full.n + 2);
	if (!dst)
		return ALT_STR_NULL;
	size_t n = 0;
	dst[n++] = '/';
	alt_str elem = ALT_STR_NULL, rem = full;
	bool found = true, first = true;
	while (found) {
		found = alt_str_cut_byte(rem, '/', &elem, &rem);
		if (alt_str_eq_cstr(elem, ".")) {
			first = false;
			continue;
		}
		if (alt_str_eq_cstr(elem, "..")) {
			ssize_t i = alt_str_last_index_byte(alt_str_make(dst + 1, n - 1), '/');
			n = i >= 0 ? (size_t)i + 1 : 1;
			first = n == 1;
		} else {
			if (!first)
				dst[n++] = '/';
			memcpy(dst + n, elem.p, elem.n);
			n += elem.n;
			first = false;
		}
	}
	if (alt_str_eq_cstr(elem, ".") || alt_str_eq_cstr(elem, ".."))
		dst[n++] = '/';
	dst[n] = 0;
	if (n > 1 && dst[1] == '/')
		return alt_str_make(dst + 1, n - 1);
	return alt_str_make(dst, n);
}

static alt_str escaped_path_arena(alt_arena *a, const alt_url *u) {
	alt_buf b = ALT_BUF_INIT;
	alt_url_escaped_path(u, &b);
	alt_str s = alt_arena_str(a, alt_buf_str(&b));
	alt_buf_free(&b);
	return s;
}

int alt_url_resolve(alt_arena *a, const alt_url *base, const alt_url *ref, alt_url *out) {
	*out = *ref;
	if (ref->scheme.n == 0)
		out->scheme = base->scheme;
	if (ref->scheme.n || ref->host.n || ref->has_user)
		return alt_url_set_path(a, out, resolve_path(a, escaped_path_arena(a, ref), ALT_STR("")));
	if (ref->opaque.n) {
		out->has_user = out->has_password = false;
		out->host = out->path = ALT_STR_NULL;
		return 0;
	}
	if (ref->path.n == 0 && !ref->force_query && ref->raw_query.n == 0) {
		out->raw_query = base->raw_query;
		if (ref->fragment.n == 0) {
			out->fragment = base->fragment;
			out->raw_fragment = base->raw_fragment;
		}
	}
	if (ref->path.n == 0 && base->opaque.n) {
		out->opaque = base->opaque;
		out->has_user = out->has_password = false;
		out->host = out->path = ALT_STR_NULL;
		return 0;
	}
	out->host = base->host;
	out->has_user = base->has_user;
	out->has_password = base->has_password;
	out->username = base->username;
	out->password = base->password;
	return alt_url_set_path(a, out, resolve_path(a, escaped_path_arena(a, base), escaped_path_arena(a, ref)));
}

/* ---- query values ---- */

void alt_url_values_init(alt_url_values *v, alt_arena *a) {
	v->kv = 0;
	v->n = v->cap = 0;
	v->arena = a;
}

int alt_url_values_add(alt_url_values *v, alt_str key, alt_str value) {
	if (v->n == v->cap) {
		size_t cap = v->cap ? v->cap * 2 : 8;
		alt_url_kv *kv = alt_arena_realloc(v->arena, v->kv, v->cap * sizeof *kv, cap * sizeof *kv);
		if (!kv)
			return -ENOMEM;
		v->kv = kv;
		v->cap = cap;
	}
	v->kv[v->n++] = (alt_url_kv){key, value};
	return 0;
}

int alt_url_parse_query(alt_url_values *v, alt_str q) {
	if (alt_str_count(q, ALT_STR("&")) + 1 > 10000)
		return -ALT_ETOOLARGE;
	int err = 0;
	while (q.n) {
		alt_str pair;
		alt_str_cut_byte(q, '&', &pair, &q);
		if (alt_str_index_byte(pair, ';') >= 0) {
			err = -ALT_EURL_PARSE;
			continue;
		}
		if (pair.n == 0)
			continue;
		alt_str key, value, k, val;
		alt_str_cut_byte(pair, '=', &key, &value);
		int rc = unescape(v->arena, key, ENC_QUERY_COMPONENT, &k);
		if (rc == 0)
			rc = unescape(v->arena, value, ENC_QUERY_COMPONENT, &val);
		if (rc) {
			if (!err)
				err = rc;
			continue;
		}
		if ((rc = alt_url_values_add(v, k, val)))
			return rc;
	}
	return err;
}

bool alt_url_values_get(const alt_url_values *v, alt_str key, alt_str *value) {
	for (size_t i = 0; i < v->n; i++)
		if (alt_str_eq(v->kv[i].key, key)) {
			if (value)
				*value = v->kv[i].value;
			return true;
		}
	if (value)
		*value = ALT_STR("");
	return false;
}

bool alt_url_values_has(const alt_url_values *v, alt_str key) { return alt_url_values_get(v, key, 0); }

void alt_url_values_del(alt_url_values *v, alt_str key) {
	size_t j = 0;
	for (size_t i = 0; i < v->n; i++)
		if (!alt_str_eq(v->kv[i].key, key))
			v->kv[j++] = v->kv[i];
	v->n = j;
}

int alt_url_values_set(alt_url_values *v, alt_str key, alt_str value) {
	alt_url_values_del(v, key);
	return alt_url_values_add(v, key, value);
}

int alt_url_values_encode(const alt_url_values *v, alt_buf *b) {
	if (v->n == 0)
		return 0;
	alt_url_kv *sorted = malloc(v->n * sizeof *sorted);
	if (!sorted)
		return -ENOMEM;
	memcpy(sorted, v->kv, v->n * sizeof *sorted);
	/* a stable sort by key (insertion: query strings are short) */
	for (size_t i = 1; i < v->n; i++) {
		alt_url_kv x = sorted[i];
		size_t j = i;
		while (j > 0 && alt_str_cmp(sorted[j - 1].key, x.key) > 0) {
			sorted[j] = sorted[j - 1];
			j--;
		}
		sorted[j] = x;
	}
	size_t start = b->n;
	for (size_t i = 0; i < v->n; i++) {
		if (b->n > start)
			alt_buf_append_byte(b, '&');
		escape(b, sorted[i].key, ENC_QUERY_COMPONENT);
		alt_buf_append_byte(b, '=');
		escape(b, sorted[i].value, ENC_QUERY_COMPONENT);
	}
	free(sorted);
	return b->err;
}
