/* The request multiplexer with Go 1.22 patterns, translated from Go's
   net/http pattern.go, routing_tree.go and server.go (ServeMux);
   BSD-3-Clause, see LICENSE-Go */
#include "http_internal.h"
#include <alt/errors.h>
#include <alt/sync.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *s;    /* literal, wildcard name, or "/" for {$} */
	bool wild, multi;
} segment;

typedef struct pattern {
	char *str, *method, *host;
	segment *segs;
	int nsegs;
	alt_http_handler h;
} pattern;

typedef struct node {
	char **keys;          /* literal children */
	struct node **kids;
	int nkids;
	struct node *empty;   /* single wildcard (or empty host/method) */
	struct node *multi;   /* multi wildcard */
	pattern *pat;         /* leaf */
} node;

struct alt_http_mux {
	alt_rwmutex mu;
	node *root;
	pattern **pats;
	int npats;
};

/* ---- path cleaning (Go's path.Clean and cleanPath) ---- */

static size_t clean(const char *p, size_t n, char *out) {
	/* p starts with '/' */
	size_t w = 0, r = 0;
	out[w++] = '/';
	r = 1;
	while (r < n) {
		if (p[r] == '/')
			r++;
		else if (p[r] == '.' && (r + 1 == n || p[r + 1] == '/'))
			r++;
		else if (p[r] == '.' && p[r + 1] == '.' && (r + 2 == n || p[r + 2] == '/')) {
			r += 2;
			if (w > 1) {
				w--;
				while (w > 1 && out[w - 1] != '/')
					w--;
				if (w > 1)
					w--; /* the slash before the dropped element */
			}
		} else {
			if (w > 1)
				out[w++] = '/';
			while (r < n && p[r] != '/')
				out[w++] = p[r++];
		}
	}
	return w;
}

/* cleanPath of Go's server: "" -> "/", a leading '/', the trailing slash kept */
static alt_str clean_path(alt_arena *a, alt_str p) {
	if (p.n == 0)
		return ALT_STR("/");
	char *in = alt_arena_alloc(a, p.n + 2);
	char *out = alt_arena_alloc(a, p.n + 3);
	if (!in || !out)
		return p;
	size_t n = 0;
	if (p.p[0] != '/')
		in[n++] = '/';
	memcpy(in + n, p.p, p.n);
	n += p.n;
	size_t w = clean(in, n, out);
	if (in[n - 1] == '/' && !(w == 1 && out[0] == '/'))
		out[w++] = '/';
	out[w] = 0;
	return alt_str_make(out, w);
}

/* ---- patterns ---- */

static bool valid_method(alt_str m) { return alt_http_valid_field_name(m); }

static bool valid_wildcard_name(const char *s) {
	/* letters, '_', digits after the first (ASCII and UTF-8 letters) */
	if (!*s)
		return false;
	for (size_t i = 0; s[i]; i++) {
		unsigned char c = (unsigned char)s[i];
		bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c >= 0x80;
		bool digit = c >= '0' && c <= '9';
		if (!letter && !(i > 0 && digit))
			return false;
	}
	return true;
}

static char *dup(alt_str s) { return alt_str_dup(s); }

static void pattern_free(pattern *p) {
	if (!p)
		return;
	for (int i = 0; i < p->nsegs; i++)
		free(p->segs[i].s);
	free(p->segs);
	free(p->str);
	free(p->method);
	free(p->host);
	free(p);
}

static int add_seg(pattern *p, const char *s, size_t n, bool wild, bool multi) {
	segment *ns = realloc(p->segs, (size_t)(p->nsegs + 1) * sizeof *ns);
	if (!ns)
		return -ENOMEM;
	p->segs = ns;
	ns[p->nsegs].s = dup(alt_str_make(s ? s : "", n));
	ns[p->nsegs].wild = wild;
	ns[p->nsegs].multi = multi;
	if (!ns[p->nsegs].s)
		return -ENOMEM;
	p->nsegs++;
	return 0;
}

static int parse_pattern(const char *str, pattern **out) {
	alt_str s = alt_str_cstr(str);
	if (s.n == 0)
		return -ALT_EHTTP_PATTERN;
	pattern *p = calloc(1, sizeof *p);
	if (!p)
		return -ENOMEM;
	p->str = strdup(str);
	alt_str method = ALT_STR(""), rest = s;
	ssize_t i = alt_str_index_any(s, " \t");
	if (i >= 0) {
		method = alt_str_slice(s, 0, (size_t)i);
		rest = alt_str_trim_left(alt_str_from(s, (size_t)i + 1), " \t");
	}
	int rc = -ALT_EHTTP_PATTERN;
	if (method.n && !valid_method(method))
		goto fail;
	p->method = dup(method);
	ssize_t sl = alt_str_index_byte(rest, '/');
	if (sl < 0)
		goto fail; /* host/path missing / */
	alt_str host = alt_str_slice(rest, 0, (size_t)sl);
	if (alt_str_index_byte(host, '{') >= 0)
		goto fail;
	p->host = dup(host);
	rest = alt_str_from(rest, (size_t)sl);
	if (method.n && !alt_str_eq_cstr(method, "CONNECT")) {
		alt_arena a;
		alt_arena_init(&a, 0);
		bool unclean = !alt_str_eq(rest, clean_path(&a, rest));
		alt_arena_free(&a);
		if (unclean)
			goto fail; /* can never match */
	}
	while (rest.n) {
		rest = alt_str_from(rest, 1);
		if (rest.n == 0) {
			if ((rc = add_seg(p, 0, 0, true, true)))
				goto fail;
			break;
		}
		ssize_t j = alt_str_index_byte(rest, '/');
		size_t k = j < 0 ? rest.n : (size_t)j;
		alt_str seg = alt_str_slice(rest, 0, k);
		rest = alt_str_from(rest, k);
		rc = -ALT_EHTTP_PATTERN;
		ssize_t b = alt_str_index_byte(seg, '{');
		if (b < 0) {
			alt_arena a;
			alt_arena_init(&a, 0);
			alt_str u;
			if (alt_url_path_unescape(&a, seg, &u) < 0)
				u = seg;
			rc = add_seg(p, u.p, u.n, false, false);
			alt_arena_free(&a);
			if (rc)
				goto fail;
			continue;
		}
		if (b != 0 || seg.p[seg.n - 1] != '}')
			goto fail;
		alt_str name = alt_str_slice(seg, 1, seg.n - 1);
		if (alt_str_eq_cstr(name, "$")) {
			if (rest.n)
				goto fail; /* {$} not at end */
			if ((rc = add_seg(p, "/", 1, false, false)))
				goto fail;
			break;
		}
		bool multi = alt_str_has_suffix(name, ALT_STR("..."));
		if (multi) {
			name.n -= 3;
			if (rest.n)
				goto fail;
		}
		char nb[128];
		if (name.n == 0 || name.n >= sizeof nb)
			goto fail;
		memcpy(nb, name.p, name.n);
		nb[name.n] = 0;
		if (!valid_wildcard_name(nb))
			goto fail;
		for (int q = 0; q < p->nsegs; q++)
			if (p->segs[q].wild && strcmp(p->segs[q].s, nb) == 0)
				goto fail; /* duplicate name */
		if ((rc = add_seg(p, nb, name.n, true, multi)))
			goto fail;
	}
	*out = p;
	return 0;
fail:
	pattern_free(p);
	return rc;
}

/* ---- relationships ---- */

enum { EQUIVALENT, MORE_GENERAL, MORE_SPECIFIC, DISJOINT, OVERLAPS };

static int inverse(int r) { return r == MORE_SPECIFIC ? MORE_GENERAL : r == MORE_GENERAL ? MORE_SPECIFIC : r; }

static int combine(int r1, int r2) {
	switch (r1) {
	case EQUIVALENT: return r2;
	case DISJOINT: return DISJOINT;
	case OVERLAPS: return r2 == DISJOINT ? DISJOINT : OVERLAPS;
	}
	if (r2 == EQUIVALENT)
		return r1;
	if (r2 == inverse(r1))
		return OVERLAPS;
	return r2;
}

static int compare_methods(const pattern *p1, const pattern *p2) {
	if (strcmp(p1->method, p2->method) == 0)
		return EQUIVALENT;
	if (!p1->method[0])
		return MORE_GENERAL;
	if (!p2->method[0])
		return MORE_SPECIFIC;
	if (strcmp(p1->method, "GET") == 0 && strcmp(p2->method, "HEAD") == 0)
		return MORE_GENERAL;
	if (strcmp(p2->method, "GET") == 0 && strcmp(p1->method, "HEAD") == 0)
		return MORE_SPECIFIC;
	return DISJOINT;
}

static int compare_segments(const segment *s1, const segment *s2) {
	if (s1->multi && s2->multi)
		return EQUIVALENT;
	if (s1->multi)
		return MORE_GENERAL;
	if (s2->multi)
		return MORE_SPECIFIC;
	if (s1->wild && s2->wild)
		return EQUIVALENT;
	if (s1->wild)
		return strcmp(s2->s, "/") == 0 ? DISJOINT : MORE_GENERAL;
	if (s2->wild)
		return strcmp(s1->s, "/") == 0 ? DISJOINT : MORE_SPECIFIC;
	return strcmp(s1->s, s2->s) == 0 ? EQUIVALENT : DISJOINT;
}

static int compare_paths(const pattern *p1, const pattern *p2) {
	bool m1 = p1->segs[p1->nsegs - 1].multi, m2 = p2->segs[p2->nsegs - 1].multi;
	if (p1->nsegs != p2->nsegs && !m1 && !m2)
		return DISJOINT;
	int rel = EQUIVALENT, i = 0;
	for (; i < p1->nsegs && i < p2->nsegs; i++) {
		rel = combine(rel, compare_segments(&p1->segs[i], &p2->segs[i]));
		if (rel == DISJOINT)
			return rel;
	}
	if (i == p1->nsegs && i == p2->nsegs)
		return rel;
	if (p1->nsegs < p2->nsegs && m1)
		return combine(rel, MORE_GENERAL);
	if (p2->nsegs < p1->nsegs && m2)
		return combine(rel, MORE_SPECIFIC);
	return DISJOINT;
}

static bool conflicts(const pattern *p1, const pattern *p2) {
	if (strcmp(p1->host, p2->host) != 0)
		return false;
	int m = compare_methods(p1, p2);
	if (m == DISJOINT)
		return false;
	int rel = combine(m, compare_paths(p1, p2));
	return rel == EQUIVALENT || rel == OVERLAPS;
}

/* ---- the routing tree ---- */

static node *find_child(const node *n, alt_str key) {
	if (!n)
		return 0;
	if (key.n == 0)
		return n->empty;
	for (int i = 0; i < n->nkids; i++)
		if (alt_str_eq_cstr(key, n->keys[i]))
			return n->kids[i];
	return 0;
}

static node *add_child(node *n, const char *key) {
	if (!key[0]) {
		if (!n->empty)
			n->empty = calloc(1, sizeof *n);
		return n->empty;
	}
	node *c = find_child(n, alt_str_cstr(key));
	if (c)
		return c;
	char **k = realloc(n->keys, (size_t)(n->nkids + 1) * sizeof *k);
	if (!k)
		return 0;
	n->keys = k;
	node **kids = realloc(n->kids, (size_t)(n->nkids + 1) * sizeof *kids);
	if (!kids)
		return 0;
	n->kids = kids;
	c = calloc(1, sizeof *c);
	char *ks = strdup(key);
	if (!c || !ks) {
		free(c);
		free(ks);
		return 0;
	}
	n->keys[n->nkids] = ks;
	n->kids[n->nkids] = c;
	n->nkids++;
	return c;
}

static int insert(node *root, pattern *p) {
	node *n = add_child(root, p->host);
	if (n)
		n = add_child(n, p->method);
	for (int i = 0; n && i < p->nsegs; i++) {
		const segment *s = &p->segs[i];
		if (s->multi) {
			if (!n->multi)
				n->multi = calloc(1, sizeof *n);
			n = n->multi;
		} else if (s->wild)
			n = add_child(n, "");
		else
			n = add_child(n, s->s);
	}
	if (!n)
		return -ENOMEM;
	n->pat = p;
	return 0;
}

static void node_free(node *n) {
	if (!n)
		return;
	for (int i = 0; i < n->nkids; i++) {
		free(n->keys[i]);
		node_free(n->kids[i]);
	}
	free(n->keys);
	free(n->kids);
	node_free(n->empty);
	node_free(n->multi);
	free(n);
}

/* matches collected while walking */
typedef struct {
	alt_str v[16];
	int n;
} matches;

/* the first segment of path, unescaped */
static void first_segment(alt_arena *a, alt_str path, alt_str *seg, alt_str *rest) {
	if (alt_str_eq_cstr(path, "/")) {
		*seg = ALT_STR("/");
		*rest = ALT_STR("");
		return;
	}
	path = alt_str_from(path, 1);
	ssize_t i = alt_str_index_byte(path, '/');
	size_t k = i < 0 ? path.n : (size_t)i;
	alt_str s = alt_str_slice(path, 0, k), u;
	*seg = alt_url_path_unescape(a, s, &u) == 0 ? u : s;
	*rest = alt_str_from(path, k);
}

static node *match_path(alt_arena *a, node *n, alt_str path, matches *m) {
	if (!n)
		return 0;
	if (path.n == 0)
		return n->pat ? n : 0;
	alt_str seg, rest;
	first_segment(a, path, &seg, &rest);
	int saved = m->n;
	node *r = match_path(a, find_child(n, seg), rest, m); /* an empty segment finds the wildcard child, as in Go */
	if (r)
		return r;
	m->n = saved;
	if (!alt_str_eq_cstr(seg, "/") && n->empty && m->n < 16) {
		m->v[m->n++] = seg;
		r = match_path(a, n->empty, rest, m);
		if (r)
			return r;
		m->n = saved;
	}
	if (n->multi && n->multi->pat) {
		alt_str all = alt_str_from(path, 1), u;
		if (m->n < 16)
			m->v[m->n++] = alt_url_path_unescape(a, all, &u) == 0 ? u : all;
		return n->multi;
	}
	return 0;
}

static node *match_method_path(alt_arena *a, node *n, alt_str method, alt_str path, matches *m) {
	if (!n)
		return 0;
	node *r;
	m->n = 0;
	if (method.n && (r = match_path(a, find_child(n, method), path, m)))
		return r;
	if (alt_str_eq_cstr(method, "HEAD")) {
		m->n = 0;
		if ((r = match_path(a, find_child(n, ALT_STR("GET")), path, m)))
			return r;
	}
	m->n = 0;
	return match_path(a, n->empty, path, m);
}

static node *tree_match(alt_arena *a, node *root, alt_str host, alt_str method, alt_str path, matches *m) {
	node *r;
	if (host.n && (r = match_method_path(a, find_child(root, host), method, path, m)))
		return r;
	return match_method_path(a, root->empty, method, path, m);
}

static bool exact_match(const node *n, alt_str path) {
	if (!n)
		return false;
	if (!n->pat->segs[n->pat->nsegs - 1].multi)
		return true;
	if (path.n && path.p[path.n - 1] != '/')
		return false;
	return (size_t)n->pat->nsegs == alt_str_count(path, ALT_STR("/"));
}

/* the methods that match host and path (sorted, comma-separated) */
static void matching_methods(alt_arena *a, node *root, alt_str host, alt_str path, alt_buf *out) {
	const char *ms[32];
	int nm = 0;
	for (int pass = 0; pass < 2; pass++) {
		alt_str p = path;
		if (pass == 1) {
			if (alt_str_has_suffix(path, ALT_STR("/")))
				break;
			alt_buf b = ALT_BUF_INIT;
			alt_buf_append_str(&b, path);
			alt_buf_append_byte(&b, '/');
			p = alt_arena_str(a, alt_buf_str(&b));
			alt_buf_free(&b);
		}
		for (int h = 0; h < 2; h++) {
			node *hn = h == 0 ? (host.n ? find_child(root, host) : 0) : root->empty;
			if (!hn)
				continue;
			for (int i = 0; i < hn->nkids; i++) {
				matches m = {{{0}}, 0};
				if (!match_path(a, hn->kids[i], p, &m))
					continue;
				bool seen = false;
				for (int k = 0; k < nm; k++)
					seen |= strcmp(ms[k], hn->keys[i]) == 0;
				if (!seen && nm < 31)
					ms[nm++] = hn->keys[i];
			}
		}
	}
	bool get = false, head = false;
	for (int k = 0; k < nm; k++) {
		get |= strcmp(ms[k], "GET") == 0;
		head |= strcmp(ms[k], "HEAD") == 0;
	}
	if (get && !head)
		ms[nm++] = "HEAD";
	for (int i = 1; i < nm; i++)
		for (int j = i; j > 0 && strcmp(ms[j - 1], ms[j]) > 0; j--) {
			const char *t = ms[j];
			ms[j] = ms[j - 1];
			ms[j - 1] = t;
		}
	for (int k = 0; k < nm; k++) {
		if (k)
			alt_buf_append_cstr(out, ", ");
		alt_buf_append_cstr(out, ms[k]);
	}
}

/* ---- the multiplexer ---- */

int alt_http_mux_new(alt_http_mux **out) {
	alt_http_mux *m = calloc(1, sizeof *m);
	if (!m)
		return -ENOMEM;
	m->mu = (alt_rwmutex)ALT_RWMUTEX_INIT;
	m->root = calloc(1, sizeof *m->root);
	if (!m->root) {
		free(m);
		return -ENOMEM;
	}
	*out = m;
	return 0;
}

void alt_http_mux_free(alt_http_mux *m) {
	if (!m)
		return;
	node_free(m->root);
	for (int i = 0; i < m->npats; i++)
		pattern_free(m->pats[i]);
	free(m->pats);
	free(m);
}

int alt_http_mux_handle(alt_http_mux *m, const char *str, alt_http_handler h) {
	if (!h.fn)
		return -EINVAL;
	pattern *p;
	int rc = parse_pattern(str, &p);
	if (rc)
		return rc;
	p->h = h;
	alt_rwmutex_lock(&m->mu);
	for (int i = 0; i < m->npats; i++)
		if (conflicts(p, m->pats[i])) {
			alt_rwmutex_unlock(&m->mu);
			pattern_free(p);
			return -ALT_EHTTP_CONFLICT;
		}
	pattern **np = realloc(m->pats, (size_t)(m->npats + 1) * sizeof *np);
	if (!np || (rc = insert(m->root, p)) < 0) {
		if (np)
			m->pats = np;
		alt_rwmutex_unlock(&m->mu);
		pattern_free(p);
		return rc < 0 ? rc : -ENOMEM;
	}
	m->pats = np;
	m->pats[m->npats++] = p;
	alt_rwmutex_unlock(&m->mu);
	return 0;
}

int alt_http_mux_handle_func(alt_http_mux *m, const char *pat, alt_http_handler_fn fn, void *arg) {
	return alt_http_mux_handle(m, pat, (alt_http_handler){fn, arg});
}

static alt_str strip_host_port(alt_str h) {
	/* the host without its port, as Go's stripHostPort */
	if (alt_str_index_byte(h, ':') < 0)
		return h;
	alt_str host, port;
	if (alt_split_host_port(h, &host, &port) < 0)
		return h;
	return host;
}

/* a temporary redirect to an escaped path (Go's urlFromEscaped) */
static void redirect_to(alt_http_rw *w, alt_http_request *r, alt_str escaped, alt_str query) {
	alt_url u = {0};
	if (alt_url_set_path(r->arena, &u, escaped) < 0)
		u.path = escaped;
	u.raw_query = query;
	alt_buf b = ALT_BUF_INIT;
	alt_url_string(&u, &b);
	alt_http_redirect(w, r, alt_buf_cstr(&b), 307);
	alt_buf_free(&b);
}

static void mux_serve(alt_http_rw *w, alt_http_request *r, void *arg) {
	alt_http_mux *m = arg;
	alt_arena *a = r->arena;
	alt_buf eb = ALT_BUF_INIT;
	alt_url_escaped_path(&r->url, &eb);
	alt_str escaped = alt_arena_str(a, alt_buf_str(&eb));
	alt_buf_free(&eb);
	alt_str host, path = escaped;
	bool connect = alt_str_eq_cstr(r->method, "CONNECT");
	matches mt = {{{0}}, 0};
	node *n;
	alt_rwmutex_rlock(&m->mu);
	if (connect)
		host = r->host;
	else {
		host = strip_host_port(r->host);
		path = clean_path(a, path);
	}
	n = tree_match(a, m->root, connect ? r->url.host : host, r->method, path, &mt);
	if (!exact_match(n, path) && !alt_str_has_suffix(path, ALT_STR("/")) && path.n) {
		/* /tree when only /tree/ exists */
		alt_buf b = ALT_BUF_INIT;
		alt_buf_append_str(&b, path);
		alt_buf_append_byte(&b, '/');
		alt_str slashed = alt_arena_str(a, alt_buf_str(&b));
		alt_buf_free(&b);
		matches m2 = {{{0}}, 0};
		node *n2 = tree_match(a, m->root, connect ? r->url.host : host, r->method, slashed, &m2);
		if (exact_match(n2, slashed)) {
			alt_rwmutex_runlock(&m->mu);
			redirect_to(w, r, slashed, r->url.raw_query);
			return;
		}
	}
	if (connect)
		n = tree_match(a, m->root, r->host, r->method, path, &mt);
	else if (!alt_str_eq(path, escaped)) {
		alt_rwmutex_runlock(&m->mu);
		redirect_to(w, r, path, r->url.raw_query);
		return;
	}
	if (!n) {
		alt_buf allow = ALT_BUF_INIT;
		matching_methods(a, m->root, host, path, &allow);
		alt_rwmutex_runlock(&m->mu);
		if (allow.n) {
			alt_http_header_set(alt_http_rw_header(w), ALT_STR("Allow"), alt_buf_str(&allow));
			alt_http_error(w, "Method Not Allowed", 405);
		} else
			alt_http_not_found(w, r);
		alt_buf_free(&allow);
		return;
	}
	pattern *p = n->pat;
	alt_http_handler h = p->h;
	r->pattern = alt_arena_str(a, alt_str_cstr(p->str));
	int k = 0;
	for (int i = 0; i < p->nsegs && k < mt.n; i++)
		if (p->segs[i].wild) {
			if (p->segs[i].s[0])
				alt_http_set_path_value(r, p->segs[i].s, mt.v[k]);
			k++;
		}
	alt_rwmutex_runlock(&m->mu);
	h.fn(w, r, h.arg);
}

alt_http_handler alt_http_mux_handler(alt_http_mux *m) { return (alt_http_handler){mux_serve, m}; }

/* ---- StripPrefix ---- */

static void strip_serve(alt_http_rw *w, alt_http_request *r, void *arg) {
	alt_http_strip_prefix *sp = arg;
	alt_str pre = alt_str_cstr(sp->prefix);
	alt_buf raw = ALT_BUF_INIT;
	alt_str p = alt_str_trim_prefix(r->url.path, pre);
	alt_str rp = alt_str_trim_prefix(r->url.raw_path, pre);
	if (p.n < r->url.path.n && (r->url.raw_path.n == 0 || rp.n < r->url.raw_path.n)) {
		r->url.path = p;
		r->url.raw_path = rp;
		alt_buf_free(&raw);
		sp->h.fn(w, r, sp->h.arg);
		return;
	}
	alt_buf_free(&raw);
	alt_http_not_found(w, r);
}

alt_http_handler alt_http_strip_prefix_handler(alt_http_strip_prefix *s) { return (alt_http_handler){strip_serve, s}; }
