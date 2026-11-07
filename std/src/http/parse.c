/* Reading HTTP/1.x messages and their bodies, as Go's net/http
   (request.go readRequest, response.go ReadResponse, transfer.go,
   internal/chunked.go) and net/textproto (readMIMEHeader); BSD-3-Clause,
   see LICENSE-Go */
#include "http_internal.h"
#include <alt/errors.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* a line of at most *budget bytes (counted with its line ending) */
static int read_line(alt_bufreader *br, alt_buf *line, size_t *budget) {
	alt_buf_reset(line);
	size_t before = alt_bufreader_buffered(br);
	(void)before;
	int rc = alt_bufreader_line(br, line, *budget);
	if (rc == -ALT_ETOOLARGE)
		return -ALT_EHTTP_HEADERTOOLARGE;
	if (rc < 0)
		return rc;
	size_t used = line->n + 2;
	*budget = used >= *budget ? 0 : *budget - used;
	return 0;
}

static alt_str trim_ows(alt_str s) { return alt_str_trim(s, " \t"); }

/* textproto's readMIMEHeader with the server's checks: field names are
   tokens, values have no control bytes; lines starting with a space or
   tab continue the previous one */
int alt__http_read_header(alt_bufreader *br, alt_arena *a, size_t *budget, alt_http_header *h, bool strict) {
	alt_buf line = ALT_BUF_INIT, kv = ALT_BUF_INIT;
	int rc = 0;
	alt_str peek;
	if (alt_bufreader_peek(br, 1, &peek) == 0 && (peek.p[0] == ' ' || peek.p[0] == '\t')) {
		rc = -ALT_EHTTP_BADREQUEST; /* malformed initial line */
		goto out;
	}
	for (;;) {
		if ((rc = read_line(br, &line, budget)) < 0) {
			if (rc == -ALT_EEOF || rc == -ALT_EUNEXPECTED_EOF)
				rc = -ALT_EUNEXPECTED_EOF;
			goto out;
		}
		if (line.n == 0)
			break; /* the end of the header */
		alt_buf_reset(&kv);
		alt_buf_append_str(&kv, trim_ows(alt_buf_str(&line)));
		if (alt_str_index_byte(alt_buf_str(&kv), ':') < 0) {
			rc = -ALT_EHTTP_BADREQUEST; /* missing colon */
			goto out;
		}
		/* continuation lines */
		while (alt_bufreader_peek(br, 1, &peek) == 0 && (peek.p[0] == ' ' || peek.p[0] == '\t')) {
			if ((rc = read_line(br, &line, budget)) < 0) {
				if (rc == -ALT_EEOF)
					rc = -ALT_EUNEXPECTED_EOF;
				goto out;
			}
			alt_buf_append_byte(&kv, ' ');
			alt_buf_append_str(&kv, trim_ows(alt_buf_str(&line)));
		}
		alt_str k, v;
		alt_str_cut_byte(alt_buf_str(&kv), ':', &k, &v);
		/* textproto accepts names of token bytes and spaces (servers
		   reject the spaces later, after the framing is checked) */
		bool ok = k.n > 0;
		for (size_t i = 0; i < k.n && ok; i++)
			ok = alt_http_valid_field_name(alt_str_make(k.p + i, 1)) || k.p[i] == ' ';
		if (!ok) {
			rc = -ALT_EHTTP_BADREQUEST;
			goto out;
		}
		(void)strict;
		for (size_t i = 0; i < v.n; i++) {
			unsigned char c = (unsigned char)v.p[i];
			if ((c < 0x20 && c != '\t') || c == 0x7f) {
				rc = -ALT_EHTTP_BADREQUEST;
				goto out;
			}
		}
		if ((rc = alt_http_header_add(h, k, alt_str_trim_left(v, " \t"))) < 0)
			goto out;
		if (h->n > 10000) {
			rc = -ALT_EHTTP_HEADERTOOLARGE;
			goto out;
		}
	}
out:
	alt_buf_free(&line);
	alt_buf_free(&kv);
	return rc;
}

static bool token(alt_str s) { return alt_http_valid_field_name(s); }

static bool parse_version(alt_str v, int *major, int *minor) {
	if (alt_str_eq_cstr(v, "HTTP/1.1")) {
		*major = 1, *minor = 1;
		return true;
	}
	if (alt_str_eq_cstr(v, "HTTP/1.0")) {
		*major = 1, *minor = 0;
		return true;
	}
	if (v.n != 8 || !alt_str_has_prefix(v, ALT_STR("HTTP/")) || v.p[6] != '.' || v.p[5] < '0' || v.p[5] > '9' || v.p[7] < '0' ||
		v.p[7] > '9')
		return false;
	*major = v.p[5] - '0', *minor = v.p[7] - '0';
	return true;
}

static bool valid_host_byte(unsigned char c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return true;
	return c && strchr("!$%&()*+,-.:;=[']_~", c);
}

/* Content-Length values: all equal, decimal, at most 2^63-1 */
static int content_length(const alt_http_header *h, int64_t *n, bool *present) {
	*present = false;
	*n = -1;
	alt_str first = ALT_STR_NULL;
	for (size_t i = 0; i < h->n; i++) {
		if (!alt_str_eq_cstr(h->f[i].key, "Content-Length"))
			continue;
		alt_str v = trim_ows(h->f[i].value);
		if (!*present) {
			first = v;
			*present = true;
		} else if (!alt_str_eq(v, first))
			return -ALT_EHTTP_BADREQUEST;
	}
	if (!*present)
		return 0;
	if (first.n == 0)
		return -ALT_EHTTP_BADREQUEST;
	unsigned long long u = 0;
	for (size_t i = 0; i < first.n; i++) {
		if (first.p[i] < '0' || first.p[i] > '9')
			return -ALT_EHTTP_BADREQUEST;
		if (u > (0x7fffffffffffffffULL - 9) / 10)
			return -ALT_EHTTP_BADREQUEST;
		u = u * 10 + (unsigned)(first.p[i] - '0');
	}
	*n = (int64_t)u;
	return 0;
}

/* ---- bodies ---- */

static ssize_t body_read(void *obj, void *buf, size_t n);

static int body_fail(alt__http_body *b, int err) {
	b->err = err;
	return err;
}

/* the next chunk header: its size in b->left */
static int begin_chunk(alt__http_body *b) {
	alt_str line;
	int rc = alt_bufreader_slice(b->br, '\n', &line);
	if (rc == -ALT_EBUFFULL)
		return body_fail(b, -ALT_EHTTP_BADCHUNK); /* line too long */
	if (rc < 0)
		return body_fail(b, rc == -ALT_EEOF ? -ALT_EUNEXPECTED_EOF : rc);
	ssize_t cr = alt_str_index_byte(line, '\r');
	if (cr < 0 || (size_t)cr != line.n - 2)
		return body_fail(b, -ALT_EHTTP_BADCHUNK); /* bare LF, or a CR inside */
	line.n -= 2;
	if (line.n >= 4096)
		return body_fail(b, -ALT_EHTTP_BADCHUNK);
	b->excess += (int64_t)line.n + 2;
	line = alt_str_trim_right(line, " \t");
	alt_str_cut_byte(line, ';', &line, 0); /* extensions are ignored */
	if (line.n == 0)
		return body_fail(b, -ALT_EHTTP_BADCHUNK);
	uint64_t v = 0;
	for (size_t i = 0; i < line.n; i++) {
		char c = line.p[i];
		int d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
		if (d < 0 || i == 16)
			return body_fail(b, -ALT_EHTTP_BADCHUNK);
		v = v << 4 | (uint64_t)d;
	}
	b->left = v;
	/* chunk headers far larger than their data are an attack */
	b->excess -= 16 + 2 * (int64_t)(v > 1u << 30 ? 1u << 30 : v);
	if (b->excess < 0)
		b->excess = 0;
	if (b->excess > 16 * 1024)
		return body_fail(b, -ALT_EHTTP_BADCHUNK);
	return 0;
}

static int read_trailer(alt__http_body *b) {
	alt_str peek;
	int rc = alt_bufreader_peek(b->br, 2, &peek);
	if (rc == 0 && alt_str_eq_cstr(peek, "\r\n")) {
		alt_bufreader_discard(b->br, 2);
		return 0;
	}
	if (rc < 0)
		return body_fail(b, -ALT_EUNEXPECTED_EOF);
	size_t budget = b->max_trailer ? b->max_trailer : 1 << 20;
	rc = alt__http_read_header(b->br, b->trailer->arena, &budget, b->trailer, true);
	if (rc < 0)
		return body_fail(b, rc == -ALT_EHTTP_HEADERTOOLARGE ? rc : -ALT_EHTTP_BADCHUNK);
	return 0;
}

static ssize_t body_read(void *obj, void *buf, size_t n) {
	alt__http_body *b = obj;
	if (b->err)
		return b->err;
	if (b->done || n == 0)
		return 0;
	if (b->before_read) {
		/* the server's "100 Continue" */
		void (*f)(void *) = b->before_read;
		b->before_read = 0;
		f(b->before_arg);
	}
	switch (b->kind) {
	case ALT__BODY_NONE:
		b->done = true;
		return 0;
	case ALT__BODY_LENGTH: {
		if (b->left == 0) {
			b->done = true;
			return 0;
		}
		if (n > b->left)
			n = (size_t)b->left;
		ssize_t k = alt_bufreader_read(b->br, buf, n);
		if (k < 0)
			return body_fail(b, (int)k);
		if (k == 0)
			return body_fail(b, -ALT_EUNEXPECTED_EOF);
		b->left -= (uint64_t)k;
		b->read += (uint64_t)k;
		if (b->left == 0)
			b->done = true;
		return k;
	}
	case ALT__BODY_CLOSE: {
		ssize_t k = alt_bufreader_read(b->br, buf, n);
		if (k < 0)
			return body_fail(b, (int)k);
		if (k == 0)
			b->done = true;
		b->read += (uint64_t)k;
		return k;
	}
	}
	/* chunked */
	size_t got = 0;
	while (got < n) {
		if (b->check_end) {
			char crlf[2];
			ssize_t k = alt_read_full(alt_bufreader_reader(b->br), crlf, 2);
			if (k != 2)
				return body_fail(b, k < 0 && k != -ALT_EUNEXPECTED_EOF ? (int)k : -ALT_EUNEXPECTED_EOF);
			if (crlf[0] != '\r' || crlf[1] != '\n')
				return body_fail(b, -ALT_EHTTP_BADCHUNK);
			b->check_end = false;
		}
		if (b->left == 0) {
			/* a new chunk; do not wait for one when there is data already */
			if (got > 0 && alt_str_index_byte(alt_str_make(b->br->buf + b->br->r, alt_bufreader_buffered(b->br)), '\n') < 0)
				break;
			int rc = begin_chunk(b);
			if (rc < 0)
				return got ? (ssize_t)got : rc;
			if (b->left == 0) {
				rc = read_trailer(b);
				if (rc < 0)
					return got ? (ssize_t)got : rc;
				b->done = true;
				break;
			}
			continue;
		}
		size_t want = n - got;
		if (want > b->left)
			want = (size_t)b->left;
		ssize_t k = alt_bufreader_read(b->br, (char *)buf + got, want);
		if (k < 0)
			return got ? (ssize_t)got : body_fail(b, (int)k);
		if (k == 0)
			return body_fail(b, -ALT_EUNEXPECTED_EOF);
		got += (size_t)k;
		b->left -= (uint64_t)k;
		b->read += (uint64_t)k;
		if (b->left == 0)
			b->check_end = true;
		/* one read of the underlying stream per call, as Go */
		if (alt_bufreader_buffered(b->br) == 0)
			break;
	}
	return (ssize_t)got;
}

void alt__http_body_init(alt__http_body *b, alt_bufreader *br, int kind, uint64_t length, alt_http_header *trailer) {
	memset(b, 0, sizeof *b);
	b->br = br;
	b->kind = kind;
	b->left = length;
	b->trailer = trailer;
	b->done = kind == ALT__BODY_NONE || (kind == ALT__BODY_LENGTH && length == 0);
}

alt_reader alt__http_body_reader(alt__http_body *b) { return (alt_reader){body_read, b}; }

/* ---- requests ---- */

int alt_http_read_request(alt_bufreader *br, alt_arena *a, size_t max_header, alt_http_request *r) {
	memset(r, 0, sizeof *r);
	r->arena = a;
	alt_http_header_init(&r->header, a);
	alt_http_header_init(&r->trailer, a);
	r->content_length = 0;
	size_t budget = max_header ? max_header : 1 << 20;
	budget += 4096; /* as Go's server: MaxHeaderBytes plus a margin */
	alt_buf line = ALT_BUF_INIT;
	int rc = read_line(br, &line, &budget);
	if (rc < 0) {
		alt_buf_free(&line);
		return rc == -ALT_EUNEXPECTED_EOF ? -ALT_EHTTP_BADREQUEST : rc;
	}
	alt_str l = alt_arena_str(a, alt_buf_str(&line)), m, rest, uri, proto;
	alt_buf_free(&line);
	if (!l.p)
		return -ENOMEM;
	if (!alt_str_cut_byte(l, ' ', &m, &rest) || !alt_str_cut_byte(rest, ' ', &uri, &proto))
		return -ALT_EHTTP_BADREQUEST;
	if (!token(m) || !parse_version(proto, &r->proto_major, &r->proto_minor))
		return -ALT_EHTTP_BADREQUEST;
	r->method = m;
	r->request_uri = uri;
	r->proto = proto;
	bool authority = alt_str_eq_cstr(m, "CONNECT") && !alt_str_has_prefix(uri, ALT_STR("/"));
	alt_str target = uri;
	if (authority) {
		char *p = alt_arena_printf(a, "http://%.*s", (int)uri.n, uri.p);
		if (!p)
			return -ENOMEM;
		target = alt_str_cstr(p);
	}
	if (alt_url_parse_request_uri(a, target, &r->url) < 0)
		return -ALT_EHTTP_BADREQUEST;
	if (authority)
		r->url.scheme = ALT_STR_NULL;
	if ((rc = alt__http_read_header(br, a, &budget, &r->header, true)) < 0)
		return rc == -ALT_EUNEXPECTED_EOF || rc == -ALT_EEOF ? -ALT_EHTTP_BADREQUEST : rc;
	/* Host (the count is checked while reading, the rest by the server
	   after the framing) */
	int hosts = 0;
	alt_str host = ALT_STR("");
	for (size_t i = 0; i < r->header.n; i++)
		if (alt_str_eq_cstr(r->header.f[i].key, "Host")) {
			host = r->header.f[i].value;
			hosts++;
		}
	if (hosts > 1)
		return -ALT_EHTTP_BADREQUEST;
	bool at_least11 = r->proto_major > 1 || (r->proto_major == 1 && r->proto_minor >= 1);
	r->host = r->url.host.n ? r->url.host : host;
	/* Connection */
	bool close_tok = alt_http_header_has_token(&r->header, "Connection", "close");
	if (r->proto_major == 1 && r->proto_minor == 0)
		r->close = close_tok || !alt_http_header_has_token(&r->header, "Connection", "keep-alive");
	else
		r->close = close_tok;
	/* Transfer-Encoding: only chunked, only in HTTP/1.1 */
	int tes = 0;
	alt_str te = ALT_STR_NULL;
	for (size_t i = 0; i < r->header.n; i++)
		if (alt_str_eq_cstr(r->header.f[i].key, "Transfer-Encoding")) {
			te = r->header.f[i].value;
			tes++;
		}
	if (tes) {
		alt_http_header_del(&r->header, ALT_STR("Transfer-Encoding"));
		if (at_least11) {
			if (tes != 1 || !alt_str_eq_fold(te, ALT_STR("chunked")))
				return -ALT_EHTTP_UNSUPPORTED; /* answered with 501 */
			r->chunked = true;
		}
	}
	int64_t n;
	bool has_cl;
	if ((rc = content_length(&r->header, &n, &has_cl)) < 0)
		return rc;
	/* Trailer: declared keys must be allowed; the header is dropped */
	alt_str trailer_decl;
	if (r->chunked && alt_http_header_get(&r->header, ALT_STR("Trailer"), &trailer_decl)) {
		for (size_t i = 0; i < r->header.n; i++) {
			if (!alt_str_eq_cstr(r->header.f[i].key, "Trailer"))
				continue;
			alt_str keys = r->header.f[i].value, key;
			while (alt_str_split_next(&keys, ALT_STR(","), &key)) {
				key = trim_ows(key);
				if (alt_str_eq_fold(key, ALT_STR("Transfer-Encoding")) || alt_str_eq_fold(key, ALT_STR("Trailer")) ||
					alt_str_eq_fold(key, ALT_STR("Content-Length")))
					return -ALT_EHTTP_BADREQUEST;
			}
		}
		alt_http_header_del(&r->header, ALT_STR("Trailer"));
	}
	/* the server's checks: version, Host, field names */
	if (r->proto_major != 1)
		return -EPROTONOSUPPORT; /* answered with 505 */
	if (at_least11 && hosts == 0 && !alt_str_eq_cstr(m, "CONNECT"))
		return -ALT_EHTTP_BADREQUEST;
	for (size_t i = 0; i < host.n; i++)
		if (!valid_host_byte((unsigned char)host.p[i]))
			return -ALT_EHTTP_BADREQUEST;
	for (size_t i = 0; i < r->header.n; i++)
		if (!alt_http_valid_field_name(r->header.f[i].key))
			return -ALT_EHTTP_BADREQUEST;
	alt__http_request_state *st = alt_arena_zalloc(a, sizeof *st);
	if (!st)
		return -ENOMEM;
	r->internal = st;
	if (r->chunked) {
		alt_http_header_del(&r->header, ALT_STR("Content-Length"));
		r->content_length = -1;
		alt__http_body_init(&st->body, br, ALT__BODY_CHUNKED, 0, &r->trailer);
	} else if (has_cl) {
		if (tes == 0) {
			/* several equal values become one */
			alt_str one = trim_ows(alt_http_header_value(&r->header, "Content-Length"));
			alt_http_header_set(&r->header, ALT_STR("Content-Length"), one);
		}
		r->content_length = n;
		alt__http_body_init(&st->body, br, ALT__BODY_LENGTH, (uint64_t)n, &r->trailer);
	} else {
		r->content_length = 0;
		alt__http_body_init(&st->body, br, ALT__BODY_NONE, 0, &r->trailer);
	}
	r->body = alt__http_body_reader(&st->body);
	return 0;
}

/* ---- responses ---- */

int alt_http_read_response(alt_bufreader *br, alt_arena *a, size_t max_header, const char *method, alt_http_response *r) {
	memset(r, 0, sizeof *r);
	r->arena = a;
	alt_http_header_init(&r->header, a);
	alt_http_header_init(&r->trailer, a);
	size_t budget = max_header ? max_header : 1 << 20;
	alt_buf line = ALT_BUF_INIT;
	int rc = read_line(br, &line, &budget);
	if (rc < 0) {
		alt_buf_free(&line);
		return rc == -ALT_EUNEXPECTED_EOF ? -ALT_EHTTP_BADRESPONSE : rc;
	}
	alt_str l = alt_arena_str(a, alt_buf_str(&line)), proto, status;
	alt_buf_free(&line);
	if (!l.p)
		return -ENOMEM;
	if (!alt_str_cut_byte(l, ' ', &proto, &status))
		return -ALT_EHTTP_BADRESPONSE;
	r->proto = proto;
	r->status_text = alt_str_trim_left(status, " ");
	alt_str code, text;
	alt_str_cut_byte(r->status_text, ' ', &code, &text);
	if (code.n != 3)
		return -ALT_EHTTP_BADRESPONSE;
	long long sc;
	if (alt_str_to_int(code, 10, &sc) < 0 || sc < 0)
		return -ALT_EHTTP_BADRESPONSE;
	r->status = (int)sc;
	if (!parse_version(proto, &r->proto_major, &r->proto_minor))
		return -ALT_EHTTP_BADRESPONSE;
	if ((rc = alt__http_read_header(br, a, &budget, &r->header, false)) < 0)
		return rc == -ALT_EHTTP_HEADERTOOLARGE ? rc : -ALT_EHTTP_BADRESPONSE;
	bool at_least11 = r->proto_major > 1 || (r->proto_major == 1 && r->proto_minor >= 1);
	bool close_tok = alt_http_header_has_token(&r->header, "Connection", "close");
	r->close = r->proto_major < 1 || (r->proto_major == 1 && r->proto_minor == 0
						 ? close_tok || !alt_http_header_has_token(&r->header, "Connection", "keep-alive")
						 : close_tok);
	int tes = 0;
	alt_str te = ALT_STR_NULL;
	for (size_t i = 0; i < r->header.n; i++)
		if (alt_str_eq_cstr(r->header.f[i].key, "Transfer-Encoding")) {
			te = r->header.f[i].value;
			tes++;
		}
	if (tes) {
		alt_http_header_del(&r->header, ALT_STR("Transfer-Encoding"));
		if (at_least11) {
			if (tes != 1 || !alt_str_eq_fold(te, ALT_STR("chunked")))
				return -ALT_EHTTP_BADRESPONSE;
			r->chunked = true;
		}
	}
	int64_t n;
	bool has_cl;
	if (content_length(&r->header, &n, &has_cl) < 0)
		return -ALT_EHTTP_BADRESPONSE;
	alt__http_body *b = alt_arena_zalloc(a, sizeof *b);
	if (!b)
		return -ENOMEM;
	r->internal = b;
	bool head = method && strcmp(method, "HEAD") == 0;
	bool no_body = head || r->status / 100 == 1 || r->status == 204 || r->status == 304;
	r->content_length = has_cl ? n : -1;
	if (no_body) {
		if (!head)
			r->content_length = has_cl ? n : 0;
		alt__http_body_init(b, br, ALT__BODY_NONE, 0, &r->trailer);
	} else if (r->chunked) {
		alt_http_header_del(&r->header, ALT_STR("Content-Length"));
		r->content_length = -1;
		alt__http_body_init(b, br, ALT__BODY_CHUNKED, 0, &r->trailer);
	} else if (has_cl)
		alt__http_body_init(b, br, ALT__BODY_LENGTH, (uint64_t)n, &r->trailer);
	else {
		r->close = true; /* the body ends with the connection */
		alt__http_body_init(b, br, ALT__BODY_CLOSE, 0, &r->trailer);
	}
	r->body = alt__http_body_reader(b);
	return 0;
}
