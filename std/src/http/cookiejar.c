/* Client cookies after Go's net/http cookie.go and cookiejar (BSD-3-Clause,
   see LICENSE-Go) */
#include <alt/net.h>
#include <alt/net/cookiejar.h>
#include <alt/sync.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static alt_str trim(alt_str s) { return alt_str_trim(s, " \t\r\n"); } /* textproto.TrimString */

static bool token(alt_str s) {
	if (!s.n)
		return false;
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (c <= 0x20 || c >= 0x7f || strchr("()<>@,;:\\\"/[]?={}", c))
			return false;
	}
	return true;
}

static bool value_byte(unsigned char b) { return b >= 0x20 && b < 0x7f && b != '"' && b != ';' && b != '\\'; }

static bool parse_value(alt_str raw, bool allow_quote, alt_str *v, bool *quoted) {
	*quoted = false;
	if (allow_quote && raw.n > 1 && raw.p[0] == '"' && raw.p[raw.n - 1] == '"') {
		raw = alt_str_slice(raw, 1, raw.n - 1);
		*quoted = true;
	}
	for (size_t i = 0; i < raw.n; i++)
		if (!value_byte((unsigned char)raw.p[i]))
			return false;
	*v = raw;
	return true;
}

static bool ascii_lower_eq(alt_str s, const char *lit) {
	for (size_t i = 0; i < s.n; i++)
		if ((unsigned char)s.p[i] >= 0x80)
			return false;
	return alt_str_eq_fold(s, alt_str_cstr(lit));
}

static bool is_ascii(alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		if ((unsigned char)s.p[i] >= 0x80)
			return false;
	return true;
}

int alt_http_parse_set_cookie(alt_arena *a, alt_str line, alt_set_cookie *c, const char **why) {
	memset(c, 0, sizeof *c);
	alt_str all = trim(line), part, rest;
	if (!all.n) {
		if (why)
			*why = "http: blank cookie";
		return -EINVAL;
	}
	bool more = alt_str_cut_byte(all, ';', &part, &rest);
	part = trim(part);
	alt_str name, value;
	if (!alt_str_cut_byte(part, '=', &name, &value)) {
		if (why)
			*why = "http: '=' not found in cookie";
		return -EINVAL;
	}
	name = trim(name);
	if (!token(name)) {
		if (why)
			*why = "http: invalid cookie name";
		return -EINVAL;
	}
	if (!parse_value(value, true, &value, &c->quoted)) {
		if (why)
			*why = "http: invalid cookie value";
		return -EINVAL;
	}
	c->name = alt_arena_str(a, name);
	c->value = alt_arena_str(a, value);
	while (more) {
		more = alt_str_cut_byte(rest, ';', &part, &rest);
		part = trim(part);
		if (!part.n)
			continue;
		alt_str attr, val;
		if (!alt_str_cut_byte(part, '=', &attr, &val))
			attr = part, val = ALT_STR("");
		if (!is_ascii(attr))
			continue;
		bool q;
		if (!parse_value(val, false, &val, &q))
			continue; /* unparsed */
		if (ascii_lower_eq(attr, "samesite")) {
			c->same_site = !is_ascii(val) ? ALT_SAMESITE_DEFAULT : ascii_lower_eq(val, "lax") ? ALT_SAMESITE_LAX
				       : ascii_lower_eq(val, "strict") ? ALT_SAMESITE_STRICT : ascii_lower_eq(val, "none") ? ALT_SAMESITE_NONE
				       : ALT_SAMESITE_DEFAULT;
		} else if (ascii_lower_eq(attr, "secure"))
			c->secure = true;
		else if (ascii_lower_eq(attr, "httponly"))
			c->http_only = true;
		else if (ascii_lower_eq(attr, "domain"))
			c->domain = alt_arena_str(a, val);
		else if (ascii_lower_eq(attr, "max-age")) {
			/* strconv.Atoi, and no leading zeros */
			long long secs;
			alt_str digits = val;
			bool ok = digits.n > 0;
			size_t i = (digits.n && (digits.p[0] == '+' || digits.p[0] == '-')) ? 1 : 0;
			ok = ok && i < digits.n;
			for (size_t k = i; k < digits.n && ok; k++)
				ok = digits.p[k] >= '0' && digits.p[k] <= '9';
			if (ok && alt_str_to_int(digits, 10, &secs) == 0 && !(secs != 0 && val.p[0] == '0')) {
				c->max_age = secs <= 0 ? -1 : secs > 0x7fffffff ? 0x7fffffff : (int)secs;
			}
		} else if (ascii_lower_eq(attr, "expires")) {
			c->raw_expires = alt_arena_str(a, val);
			alt_wall t;
			alt_zone z;
			if (alt_wall_parse(val, "Mon, 02 Jan 2006 15:04:05 MST", &t, &z) == 0 ||
				alt_wall_parse(val, "Mon, 02-Jan-2006 15:04:05 MST", &t, &z) == 0)
				c->expires = t ? t : 1;
			else
				c->expires = 0;
		} else if (ascii_lower_eq(attr, "path"))
			c->path = alt_arena_str(a, val);
		else if (ascii_lower_eq(attr, "partitioned"))
			c->partitioned = true;
	}
	return 0;
}

/* ---- the jar ---- */

typedef struct {
	char *name, *value, *domain, *path;
	bool quoted, secure, host_only, persistent;
	int64_t expires;    /* unix seconds */
	int64_t creation;   /* unix nanoseconds (Go's time resolution) */
	uint64_t seq;
	char key[260];      /* the jar key */
} entry;

struct alt_cookie_jar {
	alt_mutex mu;
	entry *e;
	size_t n, cap;
	uint64_t next_seq;
};

int alt_cookie_jar_new(alt_cookie_jar **out) {
	alt_cookie_jar *j = calloc(1, sizeof *j);
	if (!j)
		return -ENOMEM;
	j->mu = (alt_mutex)ALT_MUTEX_INIT;
	*out = j;
	return 0;
}

static void entry_free(entry *e) {
	free(e->name);
	free(e->value);
	free(e->domain);
	free(e->path);
}

void alt_cookie_jar_free(alt_cookie_jar *j) {
	if (!j)
		return;
	for (size_t i = 0; i < j->n; i++)
		entry_free(&j->e[i]);
	free(j->e);
	free(j);
}

/* Punycode (RFC 3492) of a label */
static int punycode(alt_buf *b, alt_str label) {
	uint32_t cps[256];
	size_t n = 0;
	for (size_t i = 0; i < label.n && n < 256;) {
		unsigned char c = (unsigned char)label.p[i];
		uint32_t cp;
		size_t k = c < 0x80 ? 1 : c < 0xe0 ? 2 : c < 0xf0 ? 3 : 4;
		if (i + k > label.n)
			return -EINVAL;
		cp = k == 1 ? c : k == 2 ? c & 0x1f : k == 3 ? c & 0x0f : c & 0x07;
		for (size_t m = 1; m < k; m++)
			cp = cp << 6 | ((unsigned char)label.p[i + m] & 0x3f);
		cps[n++] = cp;
		i += k;
	}
	alt_buf_append_cstr(b, "xn--");
	size_t basic = 0;
	for (size_t i = 0; i < n; i++)
		if (cps[i] < 0x80) {
			alt_buf_append_byte(b, (int)cps[i]);
			basic++;
		}
	if (basic)
		alt_buf_append_byte(b, '-');
	uint32_t code = 0x80, bias = 72, delta = 0;
	for (size_t h = basic; h < n;) {
		uint32_t m = UINT32_MAX;
		for (size_t i = 0; i < n; i++)
			if (cps[i] >= code && cps[i] < m)
				m = cps[i];
		if ((uint64_t)(m - code) * (h + 1) + delta > UINT32_MAX)
			return -EINVAL;
		delta += (m - code) * (uint32_t)(h + 1);
		code = m;
		for (size_t i = 0; i < n; i++) {
			if (cps[i] < code)
				delta++;
			if (cps[i] == code) {
				uint32_t q = delta;
				for (uint32_t k = 36;; k += 36) {
					uint32_t t = k <= bias ? 1 : k >= bias + 26 ? 26 : k - bias;
					if (q < t)
						break;
					uint32_t d = t + (q - t) % (36 - t);
					alt_buf_append_byte(b, d < 26 ? 'a' + (int)d : '0' + (int)d - 26);
					q = (q - t) / (36 - t);
				}
				alt_buf_append_byte(b, q < 26 ? 'a' + (int)q : '0' + (int)q - 26);
				/* adapt */
				uint32_t dd = h == basic ? delta / 700 : delta / 2;
				dd += dd / (uint32_t)(h + 1);
				uint32_t k = 0;
				while (dd > ((36 - 1) * 26) / 2) {
					dd /= 36 - 1;
					k += 36;
				}
				bias = k + (36 * dd) / (dd + 38);
				delta = 0;
				h++;
			}
		}
		delta++;
		code++;
	}
	return 0;
}

/* canonicalHost: no port, no trailing dot, ASCII (punycode), lowercase */
static int canonical_host(alt_str host, char *out, size_t cap) {
	size_t colons = 0;
	for (size_t i = 0; i < host.n; i++)
		colons += host.p[i] == ':';
	bool has_port = colons == 1 || (colons > 1 && host.n && host.p[0] == '[' && alt_str_index(host, ALT_STR("]:")) >= 0);
	if (has_port) {
		alt_str h, port;
		if (alt_split_host_port(host, &h, &port) < 0)
			return -EINVAL;
		host = h;
	}
	if (host.n && host.p[host.n - 1] == '.')
		host.n--;
	alt_buf b = ALT_BUF_INIT;
	if (is_ascii(host))
		alt_buf_append_str(&b, host);
	else {
		alt_str rest = host, label;
		bool first = true, more = true;
		while (more) {
			more = alt_str_cut_byte(rest, '.', &label, &rest);
			if (!first)
				alt_buf_append_byte(&b, '.');
			first = false;
			if (is_ascii(label))
				alt_buf_append_str(&b, label);
			else if (punycode(&b, label) < 0) {
				alt_buf_free(&b);
				return -EINVAL;
			}
		}
	}
	if (b.n + 1 > cap) {
		alt_buf_free(&b);
		return -EINVAL;
	}
	for (size_t i = 0; i < b.n; i++)
		out[i] = b.p[i] >= 'A' && b.p[i] <= 'Z' ? b.p[i] + 32 : b.p[i];
	out[b.n] = 0;
	alt_buf_free(&b);
	return 0;
}

static bool is_ip(const char *h) {
	if (strchr(h, ':') || strchr(h, '%'))
		return true;
	alt_ip ip;
	return alt_ip_parse(alt_str_cstr(h), &ip) == 0;
}

/* jarKey without a public suffix list: the last two labels */
static void jar_key(const char *host, char *out, size_t cap) {
	if (is_ip(host)) {
		snprintf(out, cap, "%s", host);
		return;
	}
	const char *last = strrchr(host, '.');
	if (!last || last == host) {
		snprintf(out, cap, "%s", host);
		return;
	}
	const char *prev = 0;
	for (const char *p = host; p < last; p++)
		if (*p == '.')
			prev = p;
	snprintf(out, cap, "%s", prev ? prev + 1 : host);
}

static bool has_dot_suffix(const char *s, const char *suffix) {
	size_t n = strlen(s), m = strlen(suffix);
	return n > m && s[n - m - 1] == '.' && strcmp(s + n - m, suffix) == 0;
}

static char *dup(alt_str s) {
	char *p = malloc(s.n + 1);
	if (p) {
		memcpy(p, s.p, s.n);
		p[s.n] = 0;
	}
	return p;
}

static bool is_localhost(const char *host) {
	size_t n = strlen(host);
	if (n && host[n - 1] == '.')
		n--;
	const char *s = host;
	for (size_t i = 0; i < n; i++)
		if (host[i] == '.')
			s = host + i + 1;
	size_t k = (size_t)(host + n - s);
	return k == 9 && alt_str_eq_fold(alt_str_make(s, 9), ALT_STR("localhost"));
}

static entry *find(alt_cookie_jar *j, const char *key, const char *domain, const char *path, const char *name) {
	for (size_t i = 0; i < j->n; i++) {
		entry *e = &j->e[i];
		if (!strcmp(e->key, key) && !strcmp(e->domain, domain) && !strcmp(e->path, path) && !strcmp(e->name, name))
			return e;
	}
	return 0;
}

static void remove_at(alt_cookie_jar *j, size_t i) {
	entry_free(&j->e[i]);
	j->e[i] = j->e[--j->n];
}

void alt_cookie_jar_set(alt_cookie_jar *j, const alt_url *u, const alt_set_cookie *cs, size_t n, int64_t now) {
	if (!n || (!alt_str_eq_cstr(u->scheme, "http") && !alt_str_eq_cstr(u->scheme, "https")))
		return;
	char host[256], key[260];
	if (canonical_host(u->host, host, sizeof host) < 0)
		return;
	jar_key(host, key, sizeof key);
	/* the default path */
	alt_str def = ALT_STR("/");
	if (u->path.n && u->path.p[0] == '/') {
		ssize_t i = alt_str_last_index_byte(u->path, '/');
		if (i > 0)
			def = alt_str_slice(u->path, 0, (size_t)i);
	}
	if (!now)
		now = alt_wall_unix(alt_wall_now());
	alt_wall wnow = alt_wall_now();
	alt_mutex_lock(&j->mu);
	for (size_t k = 0; k < n; k++) {
		const alt_set_cookie *c = &cs[k];
		alt_str path = c->path.n && c->path.p[0] == '/' ? c->path : def;
		/* domainAndType */
		char domain[256];
		bool host_only;
		if (!c->domain.n) {
			snprintf(domain, sizeof domain, "%s", host);
			host_only = true;
		} else if (is_ip(host)) {
			if (!alt_str_eq_cstr(c->domain, host))
				continue;
			snprintf(domain, sizeof domain, "%s", host);
			host_only = true;
		} else {
			alt_str d = c->domain;
			if (d.n && d.p[0] == '.')
				d = alt_str_slice(d, 1, d.n);
			if (!d.n || d.p[0] == '.' || !is_ascii(d) || d.p[d.n - 1] == '.' || d.n >= sizeof domain)
				continue;
			for (size_t i = 0; i < d.n; i++)
				domain[i] = d.p[i] >= 'A' && d.p[i] <= 'Z' ? d.p[i] + 32 : d.p[i];
			domain[d.n] = 0;
			if (strcmp(host, domain) != 0 && !has_dot_suffix(host, domain))
				continue;
			host_only = false;
		}
		char pathz[1024];
		snprintf(pathz, sizeof pathz, "%.*s", (int)path.n, path.p);
		char namez[1024];
		snprintf(namez, sizeof namez, "%.*s", (int)c->name.n, c->name.p);
		/* the expiry: a removal or the time */
		bool remove = false, persistent = false;
		int64_t expires = 253402300799; /* 9999-12-31T23:59:59Z */
		if (c->max_age < 0)
			remove = true;
		else if (c->max_age > 0) {
			expires = now + c->max_age;
			persistent = true;
		} else if (c->expires) {
			int64_t ex = alt_wall_unix(c->expires);
			if (c->expires <= wnow)
				remove = true;
			else {
				expires = ex;
				persistent = true;
			}
		}
		entry *old = find(j, key, domain, pathz, namez);
		if (remove) {
			if (old)
				remove_at(j, (size_t)(old - j->e));
			continue;
		}
		if (!old) {
			if (j->n == j->cap) {
				size_t cap = j->cap ? j->cap * 2 : 16;
				entry *ne = realloc(j->e, cap * sizeof *ne);
				if (!ne)
					break;
				j->e = ne;
				j->cap = cap;
			}
			old = &j->e[j->n++];
			memset(old, 0, sizeof *old);
			old->name = dup(c->name);
			old->domain = dup(alt_str_cstr(domain));
			old->path = dup(path);
			snprintf(old->key, sizeof old->key, "%s", key);
			old->creation = wnow;
			old->seq = j->next_seq++;
		} else
			free(old->value);
		old->value = dup(c->value);
		old->quoted = c->quoted;
		old->secure = c->secure;
		old->host_only = host_only;
		old->persistent = persistent;
		old->expires = expires;
	}
	alt_mutex_unlock(&j->mu);
}

static bool path_match(const char *ep, alt_str rp) {
	size_t n = strlen(ep);
	if (rp.n == n && memcmp(rp.p, ep, n) == 0)
		return true;
	if (rp.n >= n && memcmp(rp.p, ep, n) == 0)
		return ep[n - 1] == '/' || (rp.n > n && rp.p[n] == '/');
	return false;
}

typedef struct {
	entry *e;
} sel;

int alt_cookie_jar_header(alt_cookie_jar *j, const alt_url *u, alt_buf *b, int64_t now) {
	if (!alt_str_eq_cstr(u->scheme, "http") && !alt_str_eq_cstr(u->scheme, "https"))
		return 0;
	char host[256], key[260];
	if (canonical_host(u->host, host, sizeof host) < 0)
		return 0;
	jar_key(host, key, sizeof key);
	bool https = alt_str_eq_cstr(u->scheme, "https");
	alt_str path = u->path.n ? u->path : ALT_STR("/");
	if (!now)
		now = alt_wall_unix(alt_wall_now());
	alt_mutex_lock(&j->mu);
	entry **chosen = malloc((j->n ? j->n : 1) * sizeof *chosen);
	size_t nc = 0;
	for (size_t i = 0; chosen && i < j->n;) {
		entry *e = &j->e[i];
		if (strcmp(e->key, key) != 0) {
			i++;
			continue;
		}
		if (e->persistent && e->expires <= now) {
			remove_at(j, i);
			continue;
		}
		bool dm = !strcmp(e->domain, host) || (!e->host_only && has_dot_suffix(host, e->domain));
		alt_ip ip;
		bool sm = !e->secure || https || is_localhost(e->domain) ||
			  (alt_ip_parse(alt_str_cstr(e->domain), &ip) == 0 && alt_ip_is_loopback(&ip));
		if (dm && path_match(e->path, path) && sm)
			chosen[nc++] = e;
		i++;
	}
	/* by path (descending), then by creation and sequence */
	for (size_t a = 1; a < nc; a++)
		for (size_t k = a; k > 0; k--) {
			entry *x = chosen[k - 1], *y = chosen[k];
			int c = strcmp(y->path, x->path);
			bool after = c > 0 || (c == 0 && (y->creation < x->creation || (y->creation == x->creation && y->seq < x->seq)));
			if (!after)
				break;
			chosen[k - 1] = y, chosen[k] = x;
		}
	for (size_t i = 0; i < nc; i++) {
		entry *e = chosen[i];
		if (b->n)
			alt_buf_append_cstr(b, "; ");
		for (const char *p = e->name; *p; p++)
			alt_buf_append_byte(b, *p == '\n' || *p == '\r' ? '-' : *p);
		alt_buf_append_byte(b, '=');
		bool q = e->quoted || strpbrk(e->value, " ,");
		if (q)
			alt_buf_append_byte(b, '"');
		alt_buf_append_cstr(b, e->value);
		if (q)
			alt_buf_append_byte(b, '"');
	}
	free(chosen);
	alt_mutex_unlock(&j->mu);
	return b->err;
}
