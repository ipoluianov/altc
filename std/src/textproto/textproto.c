/* Text protocols after Go's net/textproto (BSD-3-Clause, see LICENSE-Go):
   the Reader (lines, continued lines, code lines and responses, dot
   encoding, MIME headers with Go's validation and errors), the Writer,
   the Pipeline and Conn. */
#include <alt/net/textproto.h>
#include <alt/strconv.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- header keys ---- */

/* RFC 7230 tchar */
static bool valid_field_byte(unsigned char c) {
	if (c >= '0' && c <= '9')
		return true;
	if ((c | 32) >= 'a' && (c | 32) <= 'z')
		return true;
	return c && strchr("!#$%&'*+-.^_`|~", c);
}

/* field-vchar, SP, HTAB, obs-text */
static bool valid_value_byte(unsigned char c) { return c >= 0x80 || (c >= 0x20 && c != 0x7f) || c == '\t'; }

/* canonicalMIMEHeaderKey on a writable copy: false for invalid keys
   (left as they are); keys with spaces are valid but not canonicalized */
static bool canon_in_place(char *a, size_t n) {
	if (!n)
		return false;
	bool nocanon = false;
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)a[i];
		if (valid_field_byte(c))
			continue;
		if (c == ' ') {
			nocanon = true;
			continue;
		}
		return false;
	}
	if (nocanon)
		return true;
	bool upper = true;
	for (size_t i = 0; i < n; i++) {
		char c = a[i];
		if (upper && c >= 'a' && c <= 'z')
			c -= 32;
		else if (!upper && c >= 'A' && c <= 'Z')
			c += 32;
		a[i] = c;
		upper = c == '-';
	}
	return true;
}

/* CanonicalMIMEHeaderKey: whether s needs changes */
static bool needs_canon(alt_str s) {
	bool upper = true;
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (!valid_field_byte(c))
			return false;
		if (upper && c >= 'a' && c <= 'z')
			return true;
		if (!upper && c >= 'A' && c <= 'Z')
			return true;
		upper = c == '-';
	}
	return false;
}

int alt_textproto_canonical_key(alt_buf *b, alt_str key) {
	size_t start = b->n;
	int rc = alt_buf_append_str(b, key);
	if (rc)
		return rc;
	if (needs_canon(key))
		canon_in_place(b->p + start, key.n);
	return 0;
}

static alt_str canon_arena(alt_arena *a, alt_str key) {
	if (!needs_canon(key))
		return alt_arena_str(a, key);
	alt_str s = alt_arena_str(a, key);
	if (s.p)
		canon_in_place((char *)s.p, s.n);
	return s;
}

alt_str alt_textproto_trim(alt_str s) {
	while (s.n && strchr(" \t\n\r", s.p[0]) && s.p[0])
		s.p++, s.n--;
	while (s.n && s.p[s.n - 1] && strchr(" \t\n\r", s.p[s.n - 1]))
		s.n--;
	return s;
}

/* ---- MIME headers ---- */

void alt_textproto_header_init(alt_textproto_header *h, alt_arena *a) { *h = (alt_textproto_header){0, 0, 0, a}; }

static int add_field(alt_textproto_header *h, alt_str key, alt_str value) {
	if (h->n == h->cap) {
		size_t cap = h->cap ? 2 * h->cap : 8;
		void *f = alt_arena_realloc(h->arena, h->f, h->cap * sizeof *h->f, cap * sizeof *h->f);
		if (!f)
			return -ENOMEM;
		h->f = f;
		h->cap = cap;
	}
	h->f[h->n++] = (alt_textproto_field){key, value};
	return 0;
}

int alt_textproto_header_add(alt_textproto_header *h, alt_str key, alt_str value) {
	alt_str k = canon_arena(h->arena, key), v = alt_arena_str(h->arena, value);
	if (!k.p || !v.p)
		return -ENOMEM;
	return add_field(h, k, v);
}

/* the key as stored: canonicalized in a scratch buffer */
static alt_str lookup_key(alt_str key, alt_buf *tmp) {
	if (!needs_canon(key))
		return key;
	alt_textproto_canonical_key(tmp, key);
	return alt_buf_str(tmp);
}

void alt_textproto_header_del(alt_textproto_header *h, alt_str key) {
	alt_buf tmp = ALT_BUF_INIT;
	alt_str k = lookup_key(key, &tmp);
	size_t j = 0;
	for (size_t i = 0; i < h->n; i++)
		if (!alt_str_eq(h->f[i].key, k))
			h->f[j++] = h->f[i];
	h->n = j;
	alt_buf_free(&tmp);
}

int alt_textproto_header_set(alt_textproto_header *h, alt_str key, alt_str value) {
	alt_textproto_header_del(h, key);
	return alt_textproto_header_add(h, key, value);
}

bool alt_textproto_header_lookup(const alt_textproto_header *h, alt_str key, alt_str *value) {
	alt_buf tmp = ALT_BUF_INIT;
	alt_str k = lookup_key(key, &tmp);
	bool found = false;
	for (size_t i = 0; i < h->n && !found; i++)
		if (alt_str_eq(h->f[i].key, k)) {
			if (value)
				*value = h->f[i].value;
			found = true;
		}
	alt_buf_free(&tmp);
	return found;
}

alt_str alt_textproto_header_get(const alt_textproto_header *h, alt_str key) {
	alt_str v = ALT_STR("");
	alt_textproto_header_lookup(h, key, &v);
	return v;
}

size_t alt_textproto_header_values(const alt_textproto_header *h, alt_str key, alt_str *out, size_t max) {
	alt_buf tmp = ALT_BUF_INIT;
	alt_str k = lookup_key(key, &tmp);
	size_t n = 0;
	for (size_t i = 0; i < h->n; i++)
		if (alt_str_eq(h->f[i].key, k)) {
			if (n < max)
				out[n] = h->f[i].value;
			n++;
		}
	alt_buf_free(&tmp);
	return n;
}

/* ---- reader ---- */

void alt_textproto_reader_init(alt_textproto_reader *r, alt_bufreader *br) {
	memset(r, 0, sizeof *r);
	r->br = br;
}

void alt_textproto_reader_free(alt_textproto_reader *r) {
	alt_buf_free(&r->err);
	alt_buf_free(&r->tmp);
}

const char *alt_textproto_error(const alt_textproto_reader *r) { return alt_buf_cstr(&r->err); }

/* records Go's text of an error code */
static int io_err(alt_textproto_reader *r, int rc) {
	alt_buf_reset(&r->err);
	alt_buf_append_cstr(&r->err, rc == -ALT_EEOF ? "EOF" : rc == -ALT_EUNEXPECTED_EOF ? "unexpected EOF" :
		rc == -ALT_ETOOLARGE ? "message too large" : alt_strerror(rc));
	return rc;
}

/* a ProtocolError: prefix and the quoted s */
static int proto_err(alt_textproto_reader *r, const char *prefix, alt_str s) {
	alt_buf_reset(&r->err);
	alt_buf_append_cstr(&r->err, prefix);
	alt_quote(&r->err, s);
	return -EPROTO;
}

/* an *Error: "%03d %q" */
static int code_err(alt_textproto_reader *r, int code, alt_str msg) {
	alt_buf_reset(&r->err);
	alt_buf_printf(&r->err, "%03d ", code);
	alt_quote(&r->err, msg);
	r->code = code;
	return -EREMOTEIO;
}

/* bufio.Reader.ReadLine: a chunk of a line, *more when the line goes on;
   an error only when there is no data */
static int buf_readline(alt_bufreader *b, alt_str *l, bool *more) {
	alt_str s;
	int rc = alt_bufreader_slice(b, '\n', &s);
	*more = false;
	if (rc == -ALT_EBUFFULL) {
		if (s.n && s.p[s.n - 1] == '\r') {
			b->r--; /* let the next call see "\r\n" */
			s.n--;
		}
		*l = s;
		*more = true;
		return 0;
	}
	if (!s.n)
		return rc ? rc : -ALT_EEOF;
	if (rc && b->err == 0)
		b->err = rc;
	if (s.p[s.n - 1] == '\n') {
		s.n--;
		if (s.n && s.p[s.n - 1] == '\r')
			s.n--;
	}
	*l = s;
	return 0;
}

static void close_dot(alt_textproto_reader *r);

/* readLineSlice: the line (up to lim bytes, lim < 0: no limit) into line */
static int read_line_slice(alt_textproto_reader *r, alt_buf *line, int64_t lim) {
	close_dot(r);
	alt_buf_reset(line);
	for (;;) {
		alt_str l;
		bool more;
		int rc = buf_readline(r->br, &l, &more);
		if (rc < 0)
			return rc;
		if (lim >= 0 && (int64_t)line->n + (int64_t)l.n > lim)
			return -ALT_ETOOLARGE;
		if (alt_buf_append_str(line, l))
			return -ENOMEM;
		if (!more)
			return 0;
	}
}

int alt_textproto_read_line(alt_textproto_reader *r, alt_buf *line) {
	int rc = read_line_slice(r, line, -1);
	return rc < 0 ? io_err(r, rc) : 0;
}

static alt_str trim_ws(alt_str s) {
	while (s.n && (s.p[0] == ' ' || s.p[0] == '\t'))
		s.p++, s.n--;
	while (s.n && (s.p[s.n - 1] == ' ' || s.p[s.n - 1] == '\t'))
		s.n--;
	return s;
}

/* skipSpace: the number of spaces and tabs skipped */
static int skip_space(alt_textproto_reader *r) {
	int n = 0;
	for (;;) {
		int c = alt_bufreader_byte(r->br);
		if (c < 0)
			break;
		if (c != ' ' && c != '\t') {
			r->br->r--;
			break;
		}
		n++;
	}
	return n;
}

/* readContinuedLineSlice: validate (when set) checks the first line;
   out gets the joined line */
static int read_continued(alt_textproto_reader *r, alt_buf *out, int64_t lim, bool validate) {
	alt_buf *first = &r->tmp;
	int rc = read_line_slice(r, first, lim);
	alt_buf_reset(out);
	if (rc < 0)
		return io_err(r, rc);
	if (!first->n)
		return 0;
	if (validate && !memchr(first->p, ':', first->n))
		return proto_err(r, "malformed MIME header: missing colon: ", alt_buf_str(first));
	alt_buf_append_str(out, trim_ws(alt_buf_str(first)));
	if (lim < 0)
		lim = INT64_MAX;
	lim -= (int64_t)out->n;
	while (skip_space(r) > 0) {
		alt_buf_append_byte(out, ' ');
		if ((int64_t)out->n >= lim)
			return io_err(r, -ALT_ETOOLARGE);
		if (read_line_slice(r, first, lim - (int64_t)out->n) < 0)
			break;
		alt_buf_append_str(out, trim_ws(alt_buf_str(first)));
	}
	return out->err;
}

int alt_textproto_read_continued_line(alt_textproto_reader *r, alt_buf *line) { return read_continued(r, line, -1, false); }

/* parseCodeLine: *continued is set before the code is checked, as Go */
static int parse_code_line(alt_textproto_reader *r, alt_str line, int expect, int *code, bool *continued, alt_str *msg) {
	*code = 0;
	*continued = false;
	*msg = ALT_STR("");
	if (line.n < 4 || (line.p[3] != ' ' && line.p[3] != '-'))
		return proto_err(r, "short response: ", line);
	*continued = line.p[3] == '-';
	/* strconv.Atoi of the three bytes */
	alt_str num = alt_str_slice(line, 0, 3);
	bool neg = false, ok = true;
	if (num.p[0] == '+' || num.p[0] == '-') {
		neg = num.p[0] == '-';
		num = alt_str_from(num, 1);
	}
	int v = 0;
	for (size_t i = 0; i < num.n; i++) {
		if (num.p[i] < '0' || num.p[i] > '9') {
			ok = false;
			break;
		}
		v = v * 10 + (num.p[i] - '0');
	}
	*code = ok ? (neg ? -v : v) : 0;
	if (!ok || *code < 100)
		return proto_err(r, "invalid response code: ", line);
	*msg = alt_str_from(line, 4);
	if ((1 <= expect && expect < 10 && *code / 100 != expect) || (10 <= expect && expect < 100 && *code / 10 != expect) ||
		(100 <= expect && expect < 1000 && *code != expect))
		return -EREMOTEIO; /* the caller builds the error */
	return 0;
}

int alt_textproto_read_code_line(alt_textproto_reader *r, int expect, int *code, alt_buf *msg) {
	alt_buf line = ALT_BUF_INIT;
	int rc = alt_textproto_read_line(r, &line);
	*code = 0;
	alt_buf_reset(msg);
	if (rc < 0) {
		alt_buf_free(&line);
		return rc;
	}
	bool continued;
	alt_str m;
	rc = parse_code_line(r, alt_buf_str(&line), expect, code, &continued, &m);
	alt_buf_append_str(msg, m);
	if (rc == -EREMOTEIO)
		rc = code_err(r, *code, m);
	else if (rc == 0 && continued)
		rc = proto_err(r, "unexpected multi-line response: ", m);
	alt_buf_free(&line);
	return rc;
}

int alt_textproto_read_response(alt_textproto_reader *r, int expect, int *code, alt_buf *msg) {
	alt_buf line = ALT_BUF_INIT;
	*code = 0;
	alt_buf_reset(msg);
	int rc = alt_textproto_read_line(r, &line);
	if (rc < 0) {
		alt_buf_free(&line);
		return rc;
	}
	bool continued;
	alt_str m;
	rc = parse_code_line(r, alt_buf_str(&line), expect, code, &continued, &m);
	bool multi = continued;
	alt_buf_append_str(msg, m);
	if (rc == -EREMOTEIO)
		code_err(r, *code, m);
	/* keep the first line's error text while reading on */
	alt_buf saved = ALT_BUF_INIT;
	alt_buf_append_str(&saved, alt_buf_str(&r->err));
	while (continued) {
		int e = alt_textproto_read_line(r, &line);
		if (e < 0) {
			*code = 0;
			alt_buf_reset(msg);
			alt_buf_free(&line);
			alt_buf_free(&saved);
			return e;
		}
		int code2;
		alt_str more;
		int pe = parse_code_line(r, alt_buf_str(&line), 0, &code2, &continued, &more);
		alt_buf_append_byte(msg, '\n');
		if (pe < 0 || code2 != *code) {
			alt_buf_append_str(msg, alt_str_trim_right(alt_buf_str(&line), "\r\n"));
			continued = true;
			continue;
		}
		alt_buf_append_str(msg, more);
	}
	alt_buf_reset(&r->err);
	alt_buf_append_str(&r->err, alt_buf_str(&saved));
	alt_buf_free(&saved);
	alt_buf_free(&line);
	if (rc < 0 && multi && msg->n)
		rc = code_err(r, *code, alt_buf_str(msg));
	return rc < 0 ? rc : msg->err;
}

/* ---- dot reader ---- */

enum { D_BEGIN, D_DOT, D_DOTCR, D_CR, D_DATA, D_EOF };

static ssize_t dot_read(void *obj, void *buf, size_t len) {
	alt_textproto_reader *r = obj;
	alt_bufreader *br = r->br;
	unsigned char *b = buf;
	size_t n = 0;
	int err = 0;
	if (r->dot_err) {
		err = r->dot_err;
		r->dot_err = 0;
		r->dot = false;
		return err;
	}
	if (!r->dot)
		return 0;
	while (n < len && r->dot_state != D_EOF) {
		int c = alt_bufreader_byte(br);
		if (c < 0) {
			err = c == -ALT_EEOF ? -ALT_EUNEXPECTED_EOF : c;
			break;
		}
		switch (r->dot_state) {
		case D_BEGIN:
			if (c == '.') {
				r->dot_state = D_DOT;
				continue;
			}
			if (c == '\r') {
				r->dot_state = D_CR;
				continue;
			}
			r->dot_state = D_DATA;
			break;
		case D_DOT:
			if (c == '\r') {
				r->dot_state = D_DOTCR;
				continue;
			}
			if (c == '\n') {
				r->dot_state = D_EOF;
				continue;
			}
			r->dot_state = D_DATA;
			break;
		case D_DOTCR:
			if (c == '\n') {
				r->dot_state = D_EOF;
				continue;
			}
			br->r--;
			c = '\r';
			r->dot_state = D_DATA;
			break;
		case D_CR:
			if (c == '\n') {
				r->dot_state = D_BEGIN;
				break;
			}
			br->r--;
			c = '\r';
			r->dot_state = D_DATA;
			break;
		case D_DATA:
			if (c == '\r') {
				r->dot_state = D_CR;
				continue;
			}
			if (c == '\n')
				r->dot_state = D_BEGIN;
			break;
		}
		b[n++] = (unsigned char)c;
	}
	if (err && !n) {
		r->dot = false;
		return io_err(r, err);
	}
	if (err) {
		io_err(r, err);
		r->dot_err = err; /* returned by the next call */
	} else if (r->dot_state == D_EOF)
		r->dot = false;
	return (ssize_t)n;
}

static void close_dot(alt_textproto_reader *r) {
	char buf[128];
	while (r->dot)
		dot_read(r, buf, sizeof buf);
}

alt_reader alt_textproto_dot_reader(alt_textproto_reader *r) {
	close_dot(r);
	r->dot = true;
	r->dot_state = D_BEGIN;
	return (alt_reader){dot_read, r};
}

int alt_textproto_read_dot_bytes(alt_textproto_reader *r, alt_buf *b) {
	alt_textproto_dot_reader(r);
	char buf[4096];
	for (;;) {
		ssize_t k = dot_read(r, buf, sizeof buf);
		if (k <= 0)
			return (int)k;
		if (alt_buf_append(b, buf, (size_t)k))
			return -ENOMEM;
	}
}

int alt_textproto_read_dot_lines(alt_textproto_reader *r, alt_arena *a, alt_str **lines, size_t *n) {
	alt_buf line = ALT_BUF_INIT;
	alt_str *v = 0;
	size_t nv = 0, cap = 0;
	int rc = 0;
	for (;;) {
		rc = read_line_slice(r, &line, -1);
		if (rc < 0) {
			if (rc == -ALT_EEOF)
				rc = -ALT_EUNEXPECTED_EOF;
			io_err(r, rc);
			break;
		}
		alt_str l = alt_buf_str(&line);
		if (l.n && l.p[0] == '.') {
			if (l.n == 1)
				break;
			l = alt_str_from(l, 1);
		}
		if (nv == cap) {
			size_t nc = cap ? 2 * cap : 8;
			alt_str *nvp = alt_arena_realloc(a, v, cap * sizeof *v, nc * sizeof *v);
			if (!nvp) {
				rc = -ENOMEM;
				break;
			}
			v = nvp;
			cap = nc;
		}
		v[nv] = alt_arena_str(a, l);
		if (!v[nv].p) {
			rc = -ENOMEM;
			break;
		}
		nv++;
	}
	alt_buf_free(&line);
	*lines = v;
	*n = nv;
	return rc;
}

/* ---- MIME header ---- */

int alt_textproto_read_mime_header_limit(alt_textproto_reader *r, alt_textproto_header *h, int64_t max_memory, int64_t max_headers) {
	if (max_memory <= 0)
		max_memory = INT64_MAX;
	if (max_headers <= 0)
		max_headers = INT64_MAX;
	max_memory -= 400;
	const int64_t entry_overhead = 200;
	alt_str pk;
	int prc = alt_bufreader_peek(r->br, 1, &pk);
	if (prc == 0 && (pk.p[0] == ' ' || pk.p[0] == '\t')) {
		alt_buf line = ALT_BUF_INIT;
		int rc = read_line_slice(r, &line, 80);
		if (rc < 0)
			rc = io_err(r, rc);
		else
			rc = proto_err(r, "malformed MIME header initial line: ", alt_buf_str(&line));
		alt_buf_free(&line);
		return rc;
	}
	alt_buf kv = ALT_BUF_INIT;
	int rc;
	for (;;) {
		rc = read_continued(r, &kv, max_memory, true);
		if (!kv.n)
			break;
		alt_str k, v;
		alt_str all = alt_buf_str(&kv);
		if (!alt_str_cut_byte(all, ':', &k, &v)) {
			rc = proto_err(r, "malformed MIME header line: ", all);
			break;
		}
		/* canonicalized in place, as Go does: errors show the new key */
		bool ok = canon_in_place(kv.p, k.n);
		alt_str key = ok ? alt_arena_str(h->arena, k) : k;
		if (!key.p) {
			rc = -ENOMEM;
			break;
		}
		for (size_t i = 0; ok && i < v.n; i++)
			if (!valid_value_byte((unsigned char)v.p[i]))
				ok = false;
		if (!ok) {
			rc = proto_err(r, "malformed MIME header line: ", all);
			break;
		}
		if (--max_headers < 0) {
			h->n = 0;
			rc = io_err(r, -ALT_ETOOLARGE);
			break;
		}
		alt_str value = alt_str_trim_left(v, " \t");
		bool seen = false;
		for (size_t i = 0; i < h->n && !seen; i++)
			seen = alt_str_eq(h->f[i].key, key);
		if (!seen)
			max_memory -= (int64_t)key.n + entry_overhead;
		max_memory -= (int64_t)value.n;
		if (max_memory < 0) {
			rc = io_err(r, -ALT_ETOOLARGE);
			break;
		}
		alt_str val = alt_arena_str(h->arena, value);
		if (!val.p || add_field(h, key, val) < 0) {
			rc = -ENOMEM;
			break;
		}
		if (rc < 0)
			break;
	}
	alt_buf_free(&kv);
	return rc;
}

int alt_textproto_read_mime_header(alt_textproto_reader *r, alt_textproto_header *h) {
	return alt_textproto_read_mime_header_limit(r, h, 0, 0);
}

/* ---- writer ---- */

void alt_textproto_writer_init(alt_textproto_writer *w, alt_bufwriter *bw) {
	memset(w, 0, sizeof *w);
	w->bw = bw;
}

enum { W_BEGIN, W_BEGINLINE, W_CR, W_DATA };

int alt_textproto_dot_close(alt_textproto_writer *w) {
	if (!w->dot)
		return 0;
	w->dot = false;
	alt_bufwriter *bw = w->bw;
	switch (w->dot_state) {
	default:
		alt_bufwriter_byte(bw, '\r');
		/* fallthrough */
	case W_CR:
		alt_bufwriter_byte(bw, '\n');
		/* fallthrough */
	case W_BEGINLINE:
		alt_bufwriter_write(bw, ".\r\n", 3);
	}
	return alt_bufwriter_flush(bw);
}

int alt_textproto_vprintf_line(alt_textproto_writer *w, const char *fmt, va_list ap) {
	alt_textproto_dot_close(w);
	alt_buf b = ALT_BUF_INIT;
	alt_buf_vprintf(&b, fmt, ap);
	alt_buf_append(&b, "\r\n", 2);
	int rc = b.err;
	if (!rc)
		alt_bufwriter_write(w->bw, b.p, b.n);
	alt_buf_free(&b);
	int f = alt_bufwriter_flush(w->bw);
	return rc ? rc : f;
}

int alt_textproto_printf_line(alt_textproto_writer *w, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int rc = alt_textproto_vprintf_line(w, fmt, ap);
	va_end(ap);
	return rc;
}

static ssize_t dot_write(void *obj, const void *buf, size_t len) {
	alt_textproto_writer *w = obj;
	alt_bufwriter *bw = w->bw;
	const unsigned char *b = buf;
	size_t n = 0;
	while (n < len) {
		unsigned char c = b[n];
		switch (w->dot_state) {
		case W_BEGIN:
		case W_BEGINLINE:
			w->dot_state = W_DATA;
			if (c == '.')
				alt_bufwriter_byte(bw, '.');
			/* fallthrough */
		case W_DATA:
			if (c == '\r')
				w->dot_state = W_CR;
			if (c == '\n') {
				alt_bufwriter_byte(bw, '\r');
				w->dot_state = W_BEGINLINE;
			}
			break;
		case W_CR:
			w->dot_state = W_DATA;
			if (c == '\n')
				w->dot_state = W_BEGINLINE;
			break;
		}
		int rc = alt_bufwriter_byte(bw, c);
		if (rc < 0)
			return rc;
		n++;
	}
	return (ssize_t)n;
}

alt_writer alt_textproto_dot_writer(alt_textproto_writer *w) {
	alt_textproto_dot_close(w);
	w->dot = true;
	w->dot_state = W_BEGIN;
	return (alt_writer){dot_write, w};
}

/* ---- pipeline ---- */

unsigned alt_textproto_next(alt_textproto_pipeline *p) {
	alt_mutex_lock(&p->mu);
	unsigned id = p->id++;
	alt_mutex_unlock(&p->mu);
	return id;
}

static void seq_start(alt_textproto_pipeline *p, unsigned *seq, unsigned id) {
	alt_mutex_lock(&p->mu);
	while (*seq != id)
		alt_cond_wait(&p->cond, &p->mu);
	alt_mutex_unlock(&p->mu);
}

static void seq_end(alt_textproto_pipeline *p, unsigned *seq, unsigned id) {
	alt_mutex_lock(&p->mu);
	if (*seq != id) {
		fprintf(stderr, "textproto: pipeline out of sync\n");
		abort();
	}
	(*seq)++;
	alt_cond_broadcast(&p->cond);
	alt_mutex_unlock(&p->mu);
}

void alt_textproto_start_request(alt_textproto_pipeline *p, unsigned id) { seq_start(p, &p->req, id); }
void alt_textproto_end_request(alt_textproto_pipeline *p, unsigned id) { seq_end(p, &p->req, id); }
void alt_textproto_start_response(alt_textproto_pipeline *p, unsigned id) { seq_start(p, &p->resp, id); }
void alt_textproto_end_response(alt_textproto_pipeline *p, unsigned id) { seq_end(p, &p->resp, id); }

/* ---- connections ---- */

int alt_textproto_conn_new(alt_textproto_conn **out, alt_reader rd, alt_writer wr) {
	alt_textproto_conn *c = calloc(1, sizeof *c);
	if (!c)
		return -ENOMEM;
	if (alt_bufreader_init(&c->br, rd, 4096) < 0 || alt_bufwriter_init(&c->bw, wr, 4096) < 0) {
		alt_bufreader_free(&c->br);
		free(c);
		return -ENOMEM;
	}
	alt_textproto_reader_init(&c->r, &c->br);
	alt_textproto_writer_init(&c->w, &c->bw);
	*out = c;
	return 0;
}

int alt_textproto_dial(alt_textproto_conn **out, const char *network, const char *address, alt_ctx *ctx) {
	alt_conn *conn;
	int rc = alt_dial(&conn, network, address, ctx);
	if (rc < 0)
		return rc;
	rc = alt_textproto_conn_new(out, alt_conn_reader(conn), alt_conn_writer(conn));
	if (rc < 0) {
		alt_conn_close(conn);
		return rc;
	}
	(*out)->conn = conn;
	return 0;
}

int alt_textproto_close(alt_textproto_conn *c) {
	if (!c)
		return 0;
	int rc = c->conn ? alt_conn_close(c->conn) : 0;
	alt_textproto_reader_free(&c->r);
	alt_bufreader_free(&c->br);
	alt_bufwriter_free(&c->bw);
	free(c);
	return rc;
}

int alt_textproto_vcmd(alt_textproto_conn *c, unsigned *id, const char *fmt, va_list ap) {
	unsigned i = alt_textproto_next(&c->p);
	alt_textproto_start_request(&c->p, i);
	int rc = alt_textproto_vprintf_line(&c->w, fmt, ap);
	alt_textproto_end_request(&c->p, i);
	if (id)
		*id = rc < 0 ? 0 : i;
	return rc;
}

int alt_textproto_cmd(alt_textproto_conn *c, unsigned *id, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int rc = alt_textproto_vcmd(c, id, fmt, ap);
	va_end(ap);
	return rc;
}
