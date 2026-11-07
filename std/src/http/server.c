/* The HTTP/1.1 server, after Go's net/http server.go (BSD-3-Clause, see
   LICENSE-Go): one task per connection on a few schedulers, a response
   writer that buffers the start of the body to choose between
   Content-Length and chunked encoding, keep-alive, graceful shutdown */
#define _GNU_SOURCE
#include "http_internal.h"
#include <alt/crypto/tls.h>
#include <alt/chan.h>
#include <alt/errors.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- dates, cached per second and thread ---- */

static __thread int64_t date_sec = -1;
static __thread char date_buf[40];

static const char *http_date(void) {
	alt_wall now = alt_wall_now();
	int64_t sec = alt_wall_unix(now);
	if (sec != date_sec) {
		alt_buf b = ALT_BUF_INIT;
		alt_http_format_date(&b, alt_wall_from_unix(sec, 0));
		snprintf(date_buf, sizeof date_buf, "%s", alt_buf_cstr(&b));
		alt_buf_free(&b);
		date_sec = sec;
	}
	return date_buf;
}

/* ---- the response writer ---- */

alt_http_header *alt_http_rw_header(alt_http_rw *w) { return &w->header; }
int alt_http_rw_status(const alt_http_rw *w) { return w->status ? w->status : 200; }

static bool body_allowed_for(int code) { return !(code >= 100 && code <= 199) && code != 204 && code != 304; }

static int put_raw(alt_http_rw *w, const void *p, size_t n) {
	if (w->h2)
		return alt__h2_send_data(w->h2, p, n, false);
	if (w->hc->werr)
		return -EPIPE;
	ssize_t k = alt_bufwriter_write(&w->hc->bw, p, n);
	if (k < 0) {
		w->hc->werr = true;
		return (int)k;
	}
	return 0;
}

/* writes body bytes with the framing chosen */
static int put_body(alt_http_rw *w, const void *p, size_t n) {
	if (n == 0 || w->head || !w->body_allowed)
		return 0;
	if (w->h2)
		return alt__h2_send_data(w->h2, p, n, false);
	if (w->chunked) {
		char hdr[24];
		int k = snprintf(hdr, sizeof hdr, "%zx\r\n", n);
		int rc = put_raw(w, hdr, (size_t)k);
		if (!rc)
			rc = put_raw(w, p, n);
		if (!rc)
			rc = put_raw(w, "\r\n", 2);
		return rc;
	}
	return put_raw(w, p, n);
}

/* sends the status line and the header; finishing tells whether the
   handler has returned (the whole body is in the pre-buffer) */
static int send_header(alt_http_rw *w, bool finishing) {
	alt_http_request *r = w->req;
	int code = w->status ? w->status : 200;
	alt_http_header *h = &w->header;
	alt_str cl;
	bool has_cl = alt_http_header_get(h, ALT_STR("Content-Length"), &cl);
	bool has_te = alt_http_header_get(h, ALT_STR("Transfer-Encoding"), 0);
	w->body_allowed = body_allowed_for(code);
	if (!alt_http_header_get(h, ALT_STR("Date"), 0))
		alt_http_header_set_cstr(h, "Date", http_date());
	if (w->body_allowed && !has_te && !alt_http_header_get(h, ALT_STR("Content-Type"), 0) && w->npre > 0)
		alt_http_header_set_cstr(h, "Content-Type", alt_http_detect_content_type(w->pre, w->npre));
	if (finishing && w->body_allowed && !has_te && !has_cl && (!w->head || w->npre > 0)) {
		char n[24];
		snprintf(n, sizeof n, "%zu", w->npre);
		alt_http_header_set_cstr(h, "Content-Length", n);
		has_cl = true;
	}
	if (!w->body_allowed) {
		/* no body: no framing */
		alt_http_header_del(h, ALT_STR("Transfer-Encoding"));
		if (code == 204)
			alt_http_header_del(h, ALT_STR("Content-Length"));
	} else if (w->h2) {
		/* HTTP/2 frames the body itself */
		alt_http_header_del(h, ALT_STR("Transfer-Encoding"));
	} else if (!has_cl && !w->head) {
		bool is11 = r->proto_major == 1 && r->proto_minor >= 1;
		if (is11 && (!has_te || alt_http_header_has_token(h, "Transfer-Encoding", "chunked"))) {
			w->chunked = true;
			alt_http_header_set_cstr(h, "Transfer-Encoding", "chunked");
		} else
			w->close_after = true; /* the body ends with the connection */
	}
	if (has_cl) {
		long long v;
		if (alt_str_to_int(alt_str_trim(cl.p ? cl : alt_http_header_value(h, "Content-Length"), " \t"), 10, &v) == 0 && v >= 0)
			w->declared = v;
	}
	if (w->h2) {
		/* the end of the stream comes with the header when there is nothing else */
		bool end = finishing && (w->npre == 0 || w->head || !w->body_allowed);
		int rc = alt__h2_send_headers(w->h2, code, h, end);
		w->sent = true;
		if (rc == 0 && w->npre && !w->head && w->body_allowed)
			rc = alt__h2_send_data(w->h2, w->pre, w->npre, finishing);
		w->npre = 0;
		return rc;
	}
	if (atomic_load(&w->hc->srv->shutting_down) || r->close)
		w->close_after = true;
	if (w->close_after)
		alt_http_header_set_cstr(h, "Connection", "close");
	else if (r->proto_major == 1 && r->proto_minor == 0)
		alt_http_header_set_cstr(h, "Connection", "keep-alive");
	alt_buf b = ALT_BUF_INIT;
	const char *text = alt_http_status_text(code);
	alt_buf_printf(&b, "HTTP/1.%d %03d ", r->proto_minor >= 1 ? 1 : 0, code);
	if (text[0])
		alt_buf_append_cstr(&b, text);
	else
		alt_buf_printf(&b, "status code %d", code);
	alt_buf_append_cstr(&b, "\r\n");
	alt_http_header_write(h, &b);
	alt_buf_append_cstr(&b, "\r\n");
	int rc = b.err ? b.err : put_raw(w, b.p, b.n);
	alt_buf_free(&b);
	w->sent = true;
	if (rc)
		return rc;
	rc = put_body(w, w->pre, w->npre);
	w->npre = 0;
	return rc;
}

int alt_http_rw_write_header(alt_http_rw *w, int status) {
	if (w->hijacked)
		return -ALT_EHTTP_HIJACKED;
	if (status >= 100 && status <= 199 && status != 101) {
		/* an informational response goes out at once, with the header */
		if (w->sent)
			return -ALT_EHTTP_WRITTEN;
		if (w->h2)
			return alt__h2_send_headers(w->h2, status, &w->header, false);
		alt_buf b = ALT_BUF_INIT;
		alt_buf_printf(&b, "HTTP/1.1 %d %s\r\n", status, alt_http_status_text(status));
		alt_http_header_write(&w->header, &b);
		alt_buf_append_cstr(&b, "\r\n");
		int rc = put_raw(w, b.p, b.n);
		alt_buf_free(&b);
		if (!rc)
			rc = alt_bufwriter_flush(&w->hc->bw);
		return rc;
	}
	if (w->wrote_header)
		return -ALT_EHTTP_WRITTEN; /* superfluous */
	if (status < 100 || status > 999)
		status = 500;
	w->wrote_header = true;
	w->status = status;
	w->body_allowed = body_allowed_for(status);
	return 0;
}

ssize_t alt_http_rw_write(alt_http_rw *w, const void *p, size_t n) {
	if (w->hijacked)
		return -ALT_EHTTP_HIJACKED;
	if (!w->wrote_header)
		alt_http_rw_write_header(w, 200);
	if (n == 0)
		return 0;
	if (!w->body_allowed)
		return -ALT_EHTTP_WRITTEN; /* a body is not allowed for the status */
	if (w->declared < 0) {
		alt_str cl;
		long long v;
		if (alt_http_header_get(&w->header, ALT_STR("Content-Length"), &cl) && alt_str_to_int(alt_str_trim(cl, " \t"), 10, &v) == 0)
			w->declared = v;
	}
	if (w->declared >= 0 && w->written + n > (uint64_t)w->declared)
		return -ALT_EHTTP_CONTENTLENGTH;
	w->written += n;
	if (!w->sent) {
		if (w->npre + n <= PRE_BUFFER) {
			memcpy(w->pre + w->npre, p, n);
			w->npre += n;
			return (ssize_t)n;
		}
		int rc = send_header(w, false);
		if (rc)
			return rc;
	}
	int rc = put_body(w, p, n);
	return rc ? rc : (ssize_t)n;
}

int alt_http_rw_printf(alt_http_rw *w, const char *fmt, ...) {
	alt_buf b = ALT_BUF_INIT;
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(&b, fmt, ap);
	va_end(ap);
	ssize_t k = b.err ? b.err : alt_http_rw_write(w, b.p, b.n);
	alt_buf_free(&b);
	return k < 0 ? (int)k : 0;
}

int alt_http_rw_flush(alt_http_rw *w) {
	if (w->hijacked)
		return -ALT_EHTTP_HIJACKED;
	if (!w->wrote_header)
		alt_http_rw_write_header(w, 200);
	if (!w->sent) {
		int rc = send_header(w, false);
		if (rc)
			return rc;
	}
	if (w->h2)
		return 0; /* the connection's writer sends what is queued */
	int rc = alt_bufwriter_flush(&w->hc->bw);
	if (rc)
		w->hc->werr = true;
	return rc;
}

static ssize_t rw_write(void *obj, const void *p, size_t n) { return alt_http_rw_write(obj, p, n); }
alt_writer alt_http_rw_writer(alt_http_rw *w) { return (alt_writer){rw_write, w}; }

/* after the handler returned */
void alt__http_rw_finish(alt_http_rw *w) {
	if (w->hijacked || w->finished)
		return;
	w->finished = true;
	if (!w->wrote_header)
		alt_http_rw_write_header(w, 200);
	if (w->h2) {
		if (!w->sent)
			send_header(w, true);
		else if (!alt__h2_stream_ended(w->h2))
			alt__h2_send_data(w->h2, 0, 0, true);
		return;
	}
	if (!w->sent)
		send_header(w, true);
	else if (w->chunked)
		put_raw(w, "0\r\n\r\n", 5);
	if (w->declared >= 0 && w->written < (uint64_t)w->declared && !w->head)
		w->close_after = true; /* the client waits for bytes that never come */
	if (alt_bufwriter_flush(&w->hc->bw))
		w->hc->werr = true;
}

void alt_http_error(alt_http_rw *w, const char *msg, int code) {
	alt_http_header *h = &w->header;
	alt_http_header_del(h, ALT_STR("Content-Length"));
	alt_http_header_set_cstr(h, "Content-Type", "text/plain; charset=utf-8");
	alt_http_header_set_cstr(h, "X-Content-Type-Options", "nosniff");
	alt_http_rw_write_header(w, code);
	alt_http_rw_write_cstr(w, msg);
	alt_http_rw_write(w, "\n", 1);
}

void alt_http_not_found(alt_http_rw *w, alt_http_request *r) {
	(void)r;
	alt_http_error(w, "404 page not found", 404);
}

int alt_http_rw_hijack(alt_http_rw *w, alt_conn **c, alt_bufreader **br) {
	if (w->h2)
		return -ALT_EHTTP_UNSUPPORTED; /* as Go: HTTP/2 connections cannot be hijacked */
	if (w->hijacked)
		return -ALT_EHTTP_HIJACKED;
	if (w->sent)
		return -ALT_EHTTP_WRITTEN;
	alt_bufreader *nb = malloc(sizeof *nb);
	if (!nb)
		return -ENOMEM;
	alt_bufwriter_flush(&w->hc->bw);
	*nb = w->hc->br;
	w->hc->br.buf = 0; /* moved */
	w->hijacked = true;
	w->hc->hijacked = true;
	*c = w->hc->c;
	*br = nb;
	return 0;
}

/* ---- redirects ---- */

static void html_escape(alt_buf *b, alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		switch (s.p[i]) {
		case '&': alt_buf_append_cstr(b, "&amp;"); break;
		case '<': alt_buf_append_cstr(b, "&lt;"); break;
		case '>': alt_buf_append_cstr(b, "&gt;"); break;
		case '"': alt_buf_append_cstr(b, "&#34;"); break;
		case '\'': alt_buf_append_cstr(b, "&#39;"); break;
		default: alt_buf_append_byte(b, s.p[i]);
		}
	}
}

void alt_http_redirect(alt_http_rw *w, alt_http_request *r, const char *target, int code) {
	alt_arena *a = r->arena;
	alt_str loc = alt_str_cstr(target);
	alt_url u;
	/* a relative path is resolved against the request's path, as Go */
	if (alt_url_parse(a, loc, &u) == 0 && u.scheme.n == 0 && u.host.n == 0) {
		alt_str path = loc, query = ALT_STR_NULL;
		bool hasq = alt_str_cut_byte(loc, '?', &path, &query);
		if (path.n == 0 || path.p[0] != '/') {
			alt_str old = r->url.path.n ? r->url.path : ALT_STR("/");
			ssize_t i = alt_str_last_index_byte(old, '/');
			alt_buf b = ALT_BUF_INIT;
			alt_buf_append_str(&b, alt_str_slice(old, 0, (size_t)(i + 1)));
			alt_buf_append_str(&b, path);
			path = alt_arena_str(a, alt_buf_str(&b));
			alt_buf_free(&b);
		}
		/* clean the path, keeping a trailing slash */
		bool trailing = path.n && path.p[path.n - 1] == '/';
		alt_url tmp = {0};
		alt_url base = {0};
		alt_url ref = {0};
		base.path = ALT_STR("/");
		if (alt_url_set_path(a, &ref, path) == 0 && alt_url_resolve(a, &base, &ref, &tmp) == 0) {
			alt_buf b = ALT_BUF_INIT;
			alt_url_escaped_path(&tmp, &b);
			if (trailing && !(b.n && b.p[b.n - 1] == '/'))
				alt_buf_append_byte(&b, '/');
			if (hasq) {
				alt_buf_append_byte(&b, '?');
				alt_buf_append_str(&b, query);
			}
			loc = alt_arena_str(a, alt_buf_str(&b));
			alt_buf_free(&b);
		}
	}
	alt_http_header *h = &w->header;
	bool had_ct = alt_http_header_get(h, ALT_STR("Content-Type"), 0);
	alt_http_header_set(h, ALT_STR("Location"), loc);
	if (!had_ct && (alt_str_eq_cstr(r->method, "GET") || alt_str_eq_cstr(r->method, "HEAD")))
		alt_http_header_set_cstr(h, "Content-Type", "text/html; charset=utf-8");
	alt_http_rw_write_header(w, code);
	if (!had_ct && alt_str_eq_cstr(r->method, "GET")) {
		alt_buf b = ALT_BUF_INIT;
		alt_buf_append_cstr(&b, "<a href=\"");
		html_escape(&b, loc);
		alt_buf_printf(&b, "\">%s</a>.\n\n", alt_http_status_text(code));
		alt_http_rw_write(w, b.p, b.n);
		alt_buf_free(&b);
	}
}

/* ---- request helpers ---- */

alt_str alt_http_path_value(const alt_http_request *r, const char *name) {
	alt_str n = alt_str_cstr(name);
	for (int i = 0; i < r->npath_values; i++)
		if (alt_str_eq(r->path_values[i].name, n))
			return r->path_values[i].value;
	return ALT_STR("");
}

int alt_http_set_path_value(alt_http_request *r, const char *name, alt_str value) {
	alt_str n = alt_str_cstr(name);
	for (int i = 0; i < r->npath_values; i++)
		if (alt_str_eq(r->path_values[i].name, n)) {
			r->path_values[i].value = value;
			return 0;
		}
	if (r->npath_values == 16)
		return -ALT_ETOOLARGE;
	r->path_values[r->npath_values].name = alt_arena_str(r->arena, n);
	r->path_values[r->npath_values].value = value;
	r->npath_values++;
	return 0;
}

const alt_url_values *alt_http_query(alt_http_request *r) {
	alt__http_request_state *st = alt__http_state(r);
	if (!st->query_parsed) {
		alt_url_values_init(&st->query, r->arena);
		alt_url_parse_query(&st->query, r->url.raw_query);
		st->query_parsed = true;
	}
	return &st->query;
}

alt_str alt_http_query_value(alt_http_request *r, const char *key) {
	alt_str v;
	alt_url_values_get(alt_http_query(r), alt_str_cstr(key), &v);
	return v;
}

int alt_http_read_body(alt_http_request *r, size_t max, alt_str *body) {
	alt_buf b = ALT_BUF_INIT;
	ssize_t n = alt_read_all(r->body, &b, max ? max : 10 << 20);
	if (n < 0) {
		alt_buf_free(&b);
		return n == -ALT_ETOOLARGE ? -ALT_EHTTP_BODYTOOLARGE : (int)n;
	}
	*body = alt_arena_str(r->arena, alt_buf_str(&b));
	alt_buf_free(&b);
	return body->p ? 0 : -ENOMEM;
}

int alt_http_parse_form(alt_http_request *r, size_t max_body, const alt_url_values **form) {
	alt__http_request_state *st = alt__http_state(r);
	if (st->form_parsed) {
		*form = &st->form;
		return 0;
	}
	st->form_parsed = true;
	alt_url_values_init(&st->form, r->arena);
	int err = 0;
	bool post = alt_str_eq_cstr(r->method, "POST") || alt_str_eq_cstr(r->method, "PUT") || alt_str_eq_cstr(r->method, "PATCH");
	alt_str ct = alt_http_header_value(&r->header, "Content-Type"), mt;
	alt_str_cut_byte(ct, ';', &mt, 0);
	if (post && alt_str_eq_fold(alt_str_trim_space(mt), ALT_STR("application/x-www-form-urlencoded"))) {
		alt_str body;
		int rc = alt_http_read_body(r, max_body, &body);
		if (rc < 0)
			err = rc;
		else
			err = alt_url_parse_query(&st->form, body);
	}
	/* then the query (Go puts the body's values first) */
	int e2 = alt_url_parse_query(&st->form, r->url.raw_query);
	if (!err)
		err = e2;
	*form = &st->form;
	return err;
}

/* ---- serving connections ---- */

static void conn_log(alt_http_server *s, const char *fmt, ...) {
	if (!s->cfg.log)
		return;
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof msg, fmt, ap);
	va_end(ap);
	s->cfg.log(s->cfg.log_arg, msg);
}

static void global_options(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	if (r->content_length != 0) {
		/* read up to 4 KiB of the body, as Go */
		char buf[4096];
		alt_limit_reader l;
		alt_read(alt_limit_reader_init(&l, r->body, sizeof buf), buf, sizeof buf);
	}
	alt_http_header_set_cstr(&w->header, "Content-Length", "0");
}

static void send_continue(void *arg) {
	alt_http_rw *w = arg;
	if (w->sent || w->hc->werr)
		return;
	alt_bufwriter_cstr(&w->hc->bw, "HTTP/1.1 100 Continue\r\n\r\n");
	alt_bufwriter_flush(&w->hc->bw);
}

/* a plain error response, then the connection closes */
static void reject(conn *hc, int code, const char *text) {
	char resp[512];
	int n = snprintf(resp, sizeof resp,
		"HTTP/1.1 %d %s\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\n%d %s", code,
		alt_http_status_text(code), code, text);
	alt_conn_set_write_deadline(hc->c, alt_deadline(ALT_SEC));
	alt_bufwriter_write(&hc->bw, resp, (size_t)n);
	alt_bufwriter_flush(&hc->bw);
}

static void set_state(conn *hc, int st) { atomic_store(&hc->state, st); }

static alt_time header_timeout_of(const alt_http_server_config *cfg) {
	alt_dur d = cfg->read_header_timeout ? cfg->read_header_timeout : cfg->read_timeout;
	return d ? alt_deadline(d) : ALT_FOREVER;
}

static void serve_conn(void *arg) {
	conn *hc = arg;
	alt_http_server *s = hc->srv;
	const alt_http_server_config *cfg = &s->cfg;
	/* HTTP/2: chosen by ALPN, or (h2c) a client starting with the preface */
	if (hc->tls) {
		alt_conn_set_deadline(hc->c, header_timeout_of(cfg) == ALT_FOREVER ? alt_deadline(10 * ALT_SEC) : header_timeout_of(cfg));
		int rc = alt_tls_handshake(hc->tls);
		alt_conn_set_deadline(hc->c, ALT_FOREVER);
		if (rc < 0) {
			conn_log(s, "http: TLS handshake error from %s: %s", hc->remote, alt_tls_error(hc->tls));
			return;
		}
		alt_tls_state st;
		alt_tls_connection_state(hc->tls, &st);
		if (strcmp(st.alpn, "h2") == 0) {
			alt__h2_serve(hc);
			return;
		}
	} else if (cfg->h2c) {
		alt_str pre;
		alt_conn_set_read_deadline(hc->c, header_timeout_of(cfg));
		if (alt_bufreader_peek(&hc->br, 3, &pre) == 0 && memcmp(pre.p, "PRI", 3) == 0) {
			alt__h2_serve(hc);
			return;
		}
	}
	size_t max_header = cfg->max_header_bytes ? cfg->max_header_bytes : 1 << 20;
	alt_dur header_timeout = cfg->read_header_timeout ? cfg->read_header_timeout : cfg->read_timeout;
	alt_dur idle = cfg->idle_timeout ? cfg->idle_timeout : cfg->read_timeout;
	bool first = true;
	for (;;) {
		if (atomic_load(&s->shutting_down) || atomic_load(&s->closed))
			break;
		/* wait for the next request (the idle timeout) */
		if (!first) {
			set_state(hc, CONN_IDLE);
			if (alt_bufreader_buffered(&hc->br) == 0) {
				alt_conn_set_read_deadline(hc->c, idle ? alt_deadline(idle) : ALT_FOREVER);
				alt_str peek;
				if (alt_bufreader_peek(&hc->br, 1, &peek) < 0)
					break;
			}
			if (atomic_load(&s->shutting_down))
				break;
		}
		first = false;
		set_state(hc, CONN_ACTIVE);
		alt_time start = alt_now();
		alt_conn_set_read_deadline(hc->c, header_timeout ? alt_deadline(header_timeout) : ALT_FOREVER);
		alt_conn_set_write_deadline(hc->c, ALT_FOREVER);
		alt_arena_reset(&hc->arena);
		alt_http_request *r = alt_arena_zalloc(&hc->arena, sizeof *r);
		if (!r)
			break;
		int rc = alt_http_read_request(&hc->br, &hc->arena, max_header, r);
		if (rc < 0) {
			if (rc == -ALT_EHTTP_HEADERTOOLARGE)
				reject(hc, 431, "Request Header Fields Too Large");
			else if (rc == -ALT_EHTTP_UNSUPPORTED)
				reject(hc, 501, "Unsupported transfer encoding");
			else if (rc == -EPROTONOSUPPORT)
				reject(hc, 505, "HTTP Version Not Supported");
			else if (rc == -ALT_EHTTP_BADREQUEST)
				reject(hc, 400, "Bad Request");
			/* the end of the stream, timeouts, closing: quietly */
			break;
		}
		/* the body's deadline: the read timeout from the start */
		alt_conn_set_read_deadline(hc->c, cfg->read_timeout ? start + cfg->read_timeout : ALT_FOREVER);
		alt_conn_set_write_deadline(hc->c, cfg->write_timeout ? alt_deadline(cfg->write_timeout) : ALT_FOREVER);
		r->remote_addr = alt_str_cstr(hc->remote);
		r->tls = hc->tls;
		alt__http_request_state *st = alt__http_state(r);
		st->conn = hc;
		alt_ctx *ctx = 0;
		alt_ctx_new(&ctx, s->base);
		r->ctx = ctx;
		alt_http_rw w = {0};
		w.hc = hc;
		w.req = r;
		w.declared = -1;
		w.head = alt_str_eq_cstr(r->method, "HEAD");
		w.body_allowed = true;
		w.pre = alt_arena_alloc(&hc->arena, PRE_BUFFER);
		alt_http_header_init(&w.header, &hc->arena);
		/* Expect */
		alt_str expect;
		bool expect_continue = false;
		if (alt_http_header_get(&r->header, ALT_STR("Expect"), &expect)) {
			if (alt_str_eq_fold(expect, ALT_STR("100-continue")) && r->proto_minor >= 1 && r->content_length != 0) {
				expect_continue = true;
				st->body.before_read = send_continue;
				st->body.before_arg = &w;
			} else if (!alt_str_eq_fold(expect, ALT_STR("100-continue"))) {
				w.close_after = true;
				alt_http_error(&w, "417 Expectation Failed", 417);
				alt__http_rw_finish(&w);
				alt_ctx_free(ctx);
				break;
			}
		}
		if (!w.pre) {
			alt_ctx_free(ctx);
			break;
		}
		if (alt_str_eq_cstr(r->request_uri, "*") && alt_str_eq_cstr(r->method, "OPTIONS"))
			global_options(&w, r, 0);
		else
			cfg->handler.fn(&w, r, cfg->handler.arg);
		alt_ctx_cancel(ctx);
		if (w.hijacked) {
			alt_ctx_free(ctx);
			return; /* the connection is the handler's now */
		}
		alt__http_rw_finish(&w);
		alt_ctx_free(ctx);
		if (hc->werr || w.close_after || r->close)
			break;
		/* the rest of the body, so that the next request can be read */
		if (!st->body.done) {
			if (expect_continue && st->body.before_read)
				break; /* never asked for: the client may send it anyway */
			alt_limit_reader l;
			ssize_t k = alt_copy(alt_discard, alt_limit_reader_init(&l, r->body, MAX_POST_HANDLER_READ));
			if (k < 0 || !st->body.done)
				break;
		}
	}
	set_state(hc, CONN_DONE);
}

static void conn_task(void *arg) {
	conn *hc = arg;
	alt_http_server *s = hc->srv;
	serve_conn(hc);
	alt_mutex_lock(&s->mu);
	if (hc->prev)
		hc->prev->next = hc->next;
	else
		s->conns = hc->next;
	if (hc->next)
		hc->next->prev = hc->prev;
	alt_mutex_unlock(&s->mu);
	if (!hc->hijacked) {
		alt_bufwriter_flush(&hc->bw);
		if (hc->tls)
			alt_tls_close(hc->tls); /* closes c too */
		else
			alt_conn_close(hc->c);
	}
	alt_bufreader_free(&hc->br);
	alt_bufwriter_free(&hc->bw);
	alt_arena_free(&hc->arena);
	free(hc);
	alt_waitgroup_done(&s->active);
}

/* ---- the server ---- */

int alt_http_server_new(alt_http_server **out, const alt_http_server_config *cfg) {
	if (!cfg->handler.fn)
		return -EINVAL;
	alt_http_server *s = calloc(1, sizeof *s);
	if (!s)
		return -ENOMEM;
	s->cfg = *cfg;
	s->mu = (alt_mutex)ALT_MUTEX_INIT;
	s->active = (alt_waitgroup)ALT_WAITGROUP_INIT;
	int rc = alt_ctx_new(&s->base, 0);
	if (rc) {
		free(s);
		return rc;
	}
	int n = cfg->threads > 0 ? cfg->threads : alt_num_cpu();
	s->workers = calloc((size_t)n, sizeof *s->workers);
	if (!s->workers) {
		alt_ctx_free(s->base);
		free(s);
		return -ENOMEM;
	}
	for (int i = 0; i < n; i++) {
		if ((rc = alt_sched_start(&s->workers[i])) < 0)
			break;
		s->nworkers++;
	}
	if (s->nworkers == 0) {
		free(s->workers);
		alt_ctx_free(s->base);
		free(s);
		return rc;
	}
	*out = s;
	return 0;
}

static bool temporary(int rc) { return rc == -EMFILE || rc == -ENFILE || rc == -ENOBUFS || rc == -ENOMEM || rc == -EAGAIN; }

int alt_http_server_serve(alt_http_server *s, alt_listener *l) {
	alt_mutex_lock(&s->mu);
	if (atomic_load(&s->shutting_down) || atomic_load(&s->closed) || s->nlisteners == 8) {
		alt_mutex_unlock(&s->mu);
		alt_listener_close(l);
		return -ALT_ECLOSED;
	}
	s->listeners[s->nlisteners++] = l;
	if (!s->has_addr) {
		s->addr = *alt_listener_addr(l);
		s->has_addr = true;
	}
	alt_mutex_unlock(&s->mu);
	alt_dur backoff = 0;
	for (;;) {
		alt_conn *c;
		int rc = alt_accept(l, &c, 0);
		if (rc < 0) {
			if (atomic_load(&s->shutting_down) || atomic_load(&s->closed))
				return -ALT_ECLOSED;
			if (temporary(rc)) {
				backoff = backoff ? backoff * 2 : 5 * ALT_MS;
				if (backoff > ALT_SEC)
					backoff = ALT_SEC;
				conn_log(s, "http: accept error: %s; retrying in %lld ms", alt_strerror(rc), (long long)(backoff / ALT_MS));
				alt_sleep(backoff);
				continue;
			}
			return rc;
		}
		backoff = 0;
		conn *hc = calloc(1, sizeof *hc);
		alt_tls_conn *tc = 0;
		if (hc && s->cfg.tls) {
			alt_tls_config tcfg = *s->cfg.tls;
			static const char *const h1[] = {"http/1.1"}, *const h2[] = {"h2", "http/1.1"};
			if (!tcfg.nalpn) {
				/* Go's default: HTTP/2 offered first */
				if (s->cfg.disable_http2)
					tcfg.alpn = h1, tcfg.nalpn = 1;
				else
					tcfg.alpn = h2, tcfg.nalpn = 2;
			}
			if (alt_tls_server(&tc, c, &tcfg) < 0)
				tc = 0;
		}
		alt_reader rd = tc ? alt_tls_reader(tc) : alt_conn_reader(c);
		alt_writer wr = tc ? alt_tls_writer(tc) : alt_conn_writer(c);
		if (!hc || (s->cfg.tls && !tc) || alt_bufreader_init(&hc->br, rd, 4096) < 0) {
			free(hc);
			alt_conn_close(c);
			continue;
		}
		hc->tls = tc;
		if (alt_bufwriter_init(&hc->bw, wr, 4096) < 0) {
			alt_bufreader_free(&hc->br);
			free(hc);
			alt_conn_close(c);
			continue;
		}
		hc->srv = s;
		hc->c = c;
		alt_arena_init(&hc->arena, 8192);
		alt_buf ra = ALT_BUF_INIT;
		alt_addr_format(alt_conn_remote_addr(c), &ra);
		snprintf(hc->remote, sizeof hc->remote, "%s", alt_buf_cstr(&ra));
		alt_buf_free(&ra);
		alt_mutex_lock(&s->mu);
		hc->next = s->conns;
		if (s->conns)
			s->conns->prev = hc;
		s->conns = hc;
		alt_mutex_unlock(&s->mu);
		alt_waitgroup_add(&s->active, 1);
		unsigned i = atomic_fetch_add(&s->rr, 1) % (unsigned)s->nworkers;
		if (alt_go_on(s->workers[i], conn_task, hc, 0) < 0)
			conn_task(hc); /* cannot happen in practice: serve it here */
	}
}

int alt_http_server_listen_and_serve(alt_http_server *s) {
	alt_listener *l;
	int rc = alt_listen(&l, "tcp", s->cfg.addr ? s->cfg.addr : ":http");
	if (rc < 0)
		return rc;
	return alt_http_server_serve(s, l);
}

const alt_addr *alt_http_server_addr(alt_http_server *s) { return s->has_addr ? &s->addr : 0; }

static void close_listeners(alt_http_server *s) {
	alt_mutex_lock(&s->mu);
	int n = s->nlisteners;
	alt_listener *ls[8];
	memcpy(ls, s->listeners, sizeof ls);
	s->nlisteners = 0;
	alt_mutex_unlock(&s->mu);
	for (int i = 0; i < n; i++)
		alt_listener_close(ls[i]);
}

/* wakes connections waiting for a request (all of them: force) */
static int nudge_conns(alt_http_server *s, bool force) {
	int left = 0;
	alt_mutex_lock(&s->mu);
	for (conn *hc = s->conns; hc; hc = hc->next) {
		int st = atomic_load(&hc->state);
		if (force || st == CONN_IDLE || st == CONN_NEW)
			alt_conn_set_read_deadline(hc->c, alt_deadline(0));
		if (force)
			alt_conn_set_write_deadline(hc->c, alt_deadline(0));
		left++;
	}
	alt_mutex_unlock(&s->mu);
	return left;
}

int alt_http_server_shutdown(alt_http_server *s, alt_ctx *ctx) {
	atomic_store(&s->shutting_down, true);
	close_listeners(s);
	alt_dur wait = ALT_MS;
	for (;;) {
		nudge_conns(s, false);
		if (alt_waitgroup_wait_until(&s->active, alt_deadline(wait)) == 0)
			return 0;
		int e = alt_ctx_err(ctx);
		if (e)
			return e;
		/* Go polls with a growing interval up to half a second */
		wait = wait * 2 > 500 * ALT_MS ? 500 * ALT_MS : wait * 2;
		if (ctx && alt_ctx_deadline(ctx) != ALT_FOREVER && alt_until(alt_ctx_deadline(ctx)) < wait)
			wait = alt_until(alt_ctx_deadline(ctx)) > 0 ? alt_until(alt_ctx_deadline(ctx)) : ALT_MS;
	}
}

int alt_http_server_close(alt_http_server *s) {
	atomic_store(&s->closed, true);
	alt_ctx_cancel(s->base);
	close_listeners(s);
	nudge_conns(s, true);
	alt_waitgroup_wait(&s->active);
	return 0;
}

void alt_http_server_free(alt_http_server *s) {
	if (!s)
		return;
	alt_http_server_close(s);
	for (int i = 0; i < s->nworkers; i++)
		alt_sched_stop(s->workers[i]);
	free(s->workers);
	alt_ctx_free(s->base);
	free(s);
}

int alt_http_listen_and_serve(const char *addr, alt_http_handler h) {
	alt_http_server_config cfg = {0};
	cfg.addr = addr;
	cfg.handler = h;
	alt_http_server *s;
	int rc = alt_http_server_new(&s, &cfg);
	if (rc < 0)
		return rc;
	rc = alt_http_server_listen_and_serve(s);
	alt_http_server_free(s);
	return rc;
}

int alt_http_listen_and_serve_tls(const char *addr, const char *cert_file, const char *key_file, alt_http_handler h) {
	alt_tls_cert *cert;
	const char *why = "";
	int rc = alt_tls_load_key_pair(&cert, cert_file, key_file, &why);
	if (rc < 0)
		return rc;
	const alt_tls_cert *certs[1] = {cert};
	alt_tls_config tc = {.certs = certs, .ncerts = 1};
	alt_http_server_config cfg = {0};
	cfg.addr = addr ? addr : ":https";
	cfg.handler = h;
	cfg.tls = &tc;
	alt_http_server *s;
	if ((rc = alt_http_server_new(&s, &cfg)) == 0) {
		rc = alt_http_server_listen_and_serve(s);
		alt_http_server_free(s);
	}
	alt_tls_cert_free(cert);
	return rc;
}
