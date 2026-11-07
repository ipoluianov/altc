/* HTTP/2 (RFC 9113) for the server and the client, after Go's
   golang.org/x/net/http2 (BSD-3-Clause, see LICENSE-Go) in its defaults
   and checks: frames, HPACK, streams, flow control, SETTINGS, PING,
   GOAWAY.

   A connection has one reader (the server's connection task, the
   client's background task) and one writer task draining a queue of
   frames, so that reading never waits on a slow peer's window. Streams
   of a server run their handlers as tasks on the server's schedulers. */
#define _GNU_SOURCE
#include "../net/net_internal.h"
#include "http_internal.h"
#include <alt/crypto/tls.h>
#include <alt/net/hpack.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { F_DATA, F_HEADERS, F_PRIORITY, F_RST, F_SETTINGS, F_PUSH, F_PING, F_GOAWAY, F_WINDOW, F_CONT };
enum { FL_END_STREAM = 1, FL_ACK = 1, FL_END_HEADERS = 4, FL_PADDED = 8, FL_PRIORITY = 0x20 };
enum {
	E_NO, E_PROTOCOL, E_INTERNAL, E_FLOW, E_SETTINGS_TIMEOUT, E_CLOSED, E_FRAME_SIZE, E_REFUSED, E_CANCEL, E_COMPRESSION,
	E_CONNECT, E_CALM, E_SECURITY, E_HTTP11,
};
enum { S_TABLE = 1, S_PUSH, S_MAX_STREAMS, S_WINDOW, S_FRAME, S_HEADER_LIST };
enum {
	DEFAULT_WINDOW = 65535, MAX_WINDOW = 0x7fffffff, QUEUE_LIMIT = 256 << 10,
	/* Go's server: 1 MiB windows and frames, 250 streams */
	SERVER_WINDOW = 1 << 20, SERVER_CONN_WINDOW = 1 << 20, SERVER_FRAME = 1 << 20, SERVER_STREAMS = 250,
	/* Go's transport: 4 MiB per stream, 1 GiB per connection, 10 MiB of headers */
	CLIENT_WINDOW = 4 << 20, CLIENT_CONN_WINDOW = 1 << 30, CLIENT_HEADER_LIST = 10 << 20,
};

static const char preface[24] = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";

typedef struct qitem {
	struct qitem *next;
	size_t n;
	uint8_t d[];
} qitem;

struct alt__h2_stream {
	struct alt__h2_conn *c;
	uint32_t id;
	int refs;                   /* the connection's table, the user */
	bool in_table;
	/* receiving */
	alt_buf in;
	size_t in_off;
	bool in_end;                /* END_STREAM received */
	int64_t in_window;          /* what the peer may still send */
	uint32_t in_unacked;        /* read but not yet returned with WINDOW_UPDATE */
	uint64_t received;
	int64_t declared;           /* content-length of the peer's message, -1 none */
	/* sending */
	int64_t out_window;
	bool out_end;
	bool reset;                 /* by either side */
	uint32_t rst_code;
	bool retry;                 /* client: refused or above GOAWAY: not processed */
	/* the client's response */
	bool got_headers;
	int status;
	alt_arena *ra;
	alt_http_header *resp_header, *trailer;
	int64_t content_length;
	/* the server's request */
	alt_arena arena;
	alt_http_request *req;
	alt_ctx *ctx;
	bool too_large, need_continue;
	/* the client's request body, sent by a task of its own (as Go's transport) */
	alt_reader body;
	int64_t body_len;
	alt_ctx *body_ctx;
	alt_waitgroup body_wg;
	struct alt__h2_stream *next;
};

typedef struct alt__h2_conn {
	bool server;
	alt_reader rd;
	alt_writer wr;
	alt_bufreader *br, own_br;
	alt_conn *c;
	alt_tls_conn *tls;
	conn *hc;                   /* the server's connection */
	alt_mutex mu;
	alt_cond cond;              /* any change: data, windows, queue, streams */
	qitem *qh, *qt;
	size_t qbytes;
	bool writer_stop, writer_done;
	alt_hpack_encoder enc;
	alt_hpack_decoder dec;
	uint32_t peer_frame, peer_window, peer_streams, peer_header_list;
	uint32_t my_frame, my_window, my_streams, my_header_list, my_conn_window;
	int64_t out_window, in_window;
	uint32_t in_unacked;
	struct alt__h2_stream *streams;
	uint32_t nactive;
	uint32_t last_peer, next_id;
	bool goaway_sent, goaway_recv;
	uint32_t goaway_last;
	bool dead;
	int err;
	char errmsg[160];
	int refs;                   /* client: the pool, the reader, the streams */
	alt_waitgroup tasks;
	/* the client's pool */
	char key[300];
	alt_time idle_since;
	struct alt__h2_conn *next;
	bool in_pool;
	alt_sched *sched;
	bool settings_seen;
} h2conn;

typedef struct alt__h2_stream h2stream;

/* ---- the write queue ---- */

static void put32(uint8_t *p, uint32_t v) {
	p[0] = (uint8_t)(v >> 24), p[1] = (uint8_t)(v >> 16), p[2] = (uint8_t)(v >> 8), p[3] = (uint8_t)v;
}
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

static void frame_header(uint8_t *h, size_t len, int type, int flags, uint32_t id) {
	h[0] = (uint8_t)(len >> 16), h[1] = (uint8_t)(len >> 8), h[2] = (uint8_t)len;
	h[3] = (uint8_t)type, h[4] = (uint8_t)flags;
	put32(h + 5, id & 0x7fffffff);
}

/* appends an item (mu held): frames go out in order (the queue is short,
   control frames do not wait long; jumping ahead could pass the preface
   or the first SETTINGS) */
static void enqueue(h2conn *c, qitem *q) {
	q->next = 0;
	if (c->qt)
		c->qt->next = q;
	else
		c->qh = q;
	c->qt = q;
	c->qbytes += q->n;
	alt_cond_broadcast(&c->cond);
}

static qitem *item(size_t n) {
	qitem *q = malloc(sizeof *q + n);
	if (q)
		q->n = n;
	return q;
}

/* a whole frame (mu held) */
static void send_frame(h2conn *c, int type, int flags, uint32_t id, const void *p, size_t n) {
	if (c->writer_stop)
		return;
	qitem *q = item(9 + n);
	if (!q)
		return;
	frame_header(q->d, n, type, flags, id);
	if (n)
		memcpy(q->d + 9, p, n);
	enqueue(c, q);
}

static void send_rst(h2conn *c, uint32_t id, uint32_t code) {
	uint8_t p[4];
	put32(p, code);
	send_frame(c, F_RST, 0, id, p, 4);
}

static void send_window(h2conn *c, uint32_t id, uint32_t inc) {
	uint8_t p[4];
	put32(p, inc);
	send_frame(c, F_WINDOW, 0, id, p, 4);
}

static void send_goaway(h2conn *c, uint32_t code, const char *debug) {
	if (c->goaway_sent && code == E_NO)
		return;
	c->goaway_sent = true;
	uint8_t p[8 + 64];
	size_t dn = debug ? strlen(debug) : 0;
	if (dn > 64)
		dn = 64;
	put32(p, c->last_peer);
	put32(p + 4, code);
	if (dn)
		memcpy(p + 8, debug, dn);
	send_frame(c, F_GOAWAY, 0, 0, p, 8 + dn);
}

static void writer_task(void *arg) {
	h2conn *c = arg;
	alt_bufwriter bw;
	alt_bufwriter_init(&bw, c->wr, 32 << 10);
	alt_mutex_lock(&c->mu);
	for (;;) {
		while (!c->qh && !c->writer_stop)
			alt_cond_wait(&c->cond, &c->mu);
		if (!c->qh)
			break;
		qitem *list = c->qh;
		c->qh = c->qt = 0;
		c->qbytes = 0;
		alt_cond_broadcast(&c->cond);
		alt_mutex_unlock(&c->mu);
		int rc = 0;
		while (list) {
			qitem *n = list->next;
			if (rc >= 0) {
				ssize_t k = alt_bufwriter_write(&bw, list->d, list->n);
				if (k < 0)
					rc = (int)k;
			}
			free(list);
			list = n;
		}
		if (rc >= 0)
			rc = alt_bufwriter_flush(&bw);
		alt_mutex_lock(&c->mu);
		if (rc < 0) {
			if (!c->dead) {
				c->dead = true;
				c->err = rc;
			}
			c->writer_stop = true;
		}
	}
	c->writer_done = true;
	alt_cond_broadcast(&c->cond);
	alt_mutex_unlock(&c->mu);
	alt_bufwriter_free(&bw);
	alt_waitgroup_done(&c->tasks);
}

/* ---- streams ---- */

static h2stream *find(h2conn *c, uint32_t id) {
	for (h2stream *s = c->streams; s; s = s->next)
		if (s->id == id)
			return s;
	return 0;
}

static void stream_free(h2stream *s) {
	alt_buf_free(&s->in);
	alt_arena_free(&s->arena);
	free(s);
}

/* drops a reference (mu held); the connection may go with the last stream (client) */
static void unref(h2stream *s) {
	if (--s->refs == 0)
		stream_free(s);
}

/* data nobody will read goes back to the connection's window (mu held) */
static void discard_unread(h2conn *c, h2stream *s) {
	size_t unread = s->in.n - s->in_off;
	s->in_off = s->in.n;
	if (c->dead)
		return;
	c->in_unacked += (uint32_t)unread;
	if (c->in_unacked >= c->my_conn_window / 4) {
		send_window(c, 0, c->in_unacked);
		c->in_window += c->in_unacked;
		c->in_unacked = 0;
	}
}

/* the stream leaves the table: closed both ways or reset (mu held) */
static void remove_stream(h2conn *c, h2stream *s) {
	if (!s->in_table)
		return;
	for (h2stream **pp = &c->streams; *pp; pp = &(*pp)->next)
		if (*pp == s) {
			*pp = s->next;
			break;
		}
	s->in_table = false;
	c->nactive--;
	/* a reset stream's data is never read: it goes back to the connection's window */
	if (s->reset)
		discard_unread(c, s);
	alt_cond_broadcast(&c->cond);
	unref(s);
}

static void maybe_close(h2conn *c, h2stream *s) {
	if ((s->in_end && s->out_end) || s->reset)
		remove_stream(c, s);
}

static void reset_stream(h2conn *c, h2stream *s, uint32_t code, bool by_peer) {
	if (s->reset)
		return;
	s->reset = true;
	s->rst_code = code;
	if (!by_peer)
		send_rst(c, s->id, code);
	if (s->ctx)
		alt_ctx_cancel(s->ctx);
	alt_cond_broadcast(&c->cond);
	remove_stream(c, s);
}

/* ---- connection errors ---- */

static int conn_fail(h2conn *c, uint32_t code, int err, const char *fmt, ...) __attribute__((format(printf, 4, 5)));
static int conn_fail(h2conn *c, uint32_t code, int err, const char *fmt, ...) {
	alt_mutex_lock(&c->mu);
	if (!c->dead) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(c->errmsg, sizeof c->errmsg, fmt, ap);
		va_end(ap);
		if (code != (uint32_t)-1)
			send_goaway(c, code, c->errmsg);
		c->dead = true;
		c->err = err;
	}
	c->writer_stop = true;
	for (h2stream *s = c->streams; s; s = s->next)
		if (s->ctx)
			alt_ctx_cancel(s->ctx);
	alt_cond_broadcast(&c->cond);
	alt_mutex_unlock(&c->mu);
	return err;
}

/* ---- header blocks ---- */

typedef struct {
	alt_hpack_field *f;
	size_t n, cap;
	uint64_t size;
	uint64_t limit;
	bool too_large;
	alt_arena *a;
} fields;

static int collect(void *arg, const alt_hpack_field *f) {
	fields *fs = arg;
	fs->size += alt_hpack_field_size(f);
	if (fs->limit && fs->size > fs->limit) {
		fs->too_large = true;
		return 0; /* decode on to keep the table in step, keep nothing */
	}
	if (fs->n == fs->cap) {
		size_t cap = fs->cap ? fs->cap * 2 : 16;
		alt_hpack_field *nf = alt_arena_realloc(fs->a, fs->f, fs->cap * sizeof *nf, cap * sizeof *nf);
		if (!nf)
			return -ENOMEM;
		fs->f = nf;
		fs->cap = cap;
	}
	fs->f[fs->n++] = *f;
	return 0;
}

static bool lower_token(alt_str s) {
	if (!s.n)
		return false;
	for (size_t i = 0; i < s.n; i++) {
		unsigned char ch = (unsigned char)s.p[i];
		if (ch >= 'A' && ch <= 'Z')
			return false;
	}
	return alt_http_valid_field_name(s);
}

static bool connection_specific(alt_str n) {
	return alt_str_eq_cstr(n, "connection") || alt_str_eq_cstr(n, "keep-alive") || alt_str_eq_cstr(n, "proxy-connection") ||
	       alt_str_eq_cstr(n, "transfer-encoding") || alt_str_eq_cstr(n, "upgrade");
}

/* checks the regular fields of a message (RFC 9113 8.2): false if malformed */
static bool regular_ok(const fields *fs, size_t from) {
	for (size_t i = from; i < fs->n; i++) {
		alt_str n = fs->f[i].name, v = fs->f[i].value;
		if (n.n && n.p[0] == ':')
			return false; /* pseudo after regular */
		if (!lower_token(n) || connection_specific(n) || !alt_http_valid_field_value(v))
			return false;
		if (alt_str_eq_cstr(n, "te") && !alt_str_eq_cstr(v, "trailers"))
			return false;
	}
	return true;
}

static int64_t parse_cl(alt_str v) {
	long long n;
	if (v.n == 0 || alt_str_to_int(v, 10, &n) < 0 || n < 0)
		return -2;
	for (size_t i = 0; i < v.n; i++)
		if (v.p[i] < '0' || v.p[i] > '9')
			return -2;
	return n;
}

/* ---- reading ---- */

static int read_exact(h2conn *c, void *p, size_t n) {
	ssize_t k = alt_read_full(alt_bufreader_reader(c->br), p, n);
	if (k < 0)
		return (int)k;
	return (size_t)k == n ? 0 : -ALT_EUNEXPECTED_EOF;
}

/* strips padding: the payload of a padded frame */
static int unpad(h2conn *c, int flags, uint8_t **p, size_t *n) {
	if (!(flags & FL_PADDED))
		return 0;
	if (*n < 1)
		return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "padded frame too short");
	size_t pad = (*p)[0];
	if (pad >= *n)
		return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "pad length too large");
	*p += 1;
	*n -= 1 + pad;
	return 0;
}

static void apply_settings(h2conn *c, const uint8_t *p, size_t n, int *err) {
	*err = 0;
	for (size_t i = 0; i + 6 <= n; i += 6) {
		unsigned id = (unsigned)p[i] << 8 | p[i + 1];
		uint32_t v = get32(p + i + 2);
		switch (id) {
		case S_TABLE: alt_hpack_encoder_set_max_table_size_limit(&c->enc, v); break;
		case S_PUSH:
			if (v > 1 || (!c->server && v != 0))
				*err = E_PROTOCOL;
			break;
		case S_MAX_STREAMS: c->peer_streams = v; break;
		case S_WINDOW: {
			if (v > MAX_WINDOW) {
				*err = E_FLOW;
				break;
			}
			int64_t delta = (int64_t)v - c->peer_window;
			c->peer_window = v;
			for (h2stream *s = c->streams; s; s = s->next) {
				s->out_window += delta;
				if (s->out_window > MAX_WINDOW)
					*err = E_FLOW;
			}
			break;
		}
		case S_FRAME:
			if (v < 16384 || v > 16777215)
				*err = E_PROTOCOL;
			else
				c->peer_frame = v;
			break;
		case S_HEADER_LIST: c->peer_header_list = v; break;
		default: break; /* unknown settings are ignored */
		}
	}
	alt_cond_broadcast(&c->cond);
}

static void server_headers(h2conn *c, uint32_t id, fields *fs, bool end, alt_arena *a);
static void client_headers(h2conn *c, h2stream *s, fields *fs, bool end);

/* one frame: 0 to go on, < 0 when the connection ends */
static int read_one(h2conn *c, alt_buf *payload, alt_buf *block) {
	uint8_t h[9];
	int rc = read_exact(c, h, 9);
	if (rc < 0)
		return rc;
	size_t len = (size_t)h[0] << 16 | (size_t)h[1] << 8 | h[2];
	int type = h[3], flags = h[4];
	uint32_t id = get32(h + 5) & 0x7fffffff;
	if (len > c->my_frame)
		return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "frame too large (%zu bytes)", len);
	alt_buf_reset(payload);
	if (alt_buf_reserve(payload, len) < 0)
		return -ENOMEM;
	if (len && (rc = read_exact(c, payload->p, len)) < 0)
		return rc;
	alt_buf_truncate(payload, len);
	uint8_t *p = (uint8_t *)payload->p;
	if (!c->settings_seen && (type != F_SETTINGS || (flags & FL_ACK)))
		return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "the first frame must be SETTINGS");
	switch (type) {
	case F_SETTINGS: {
		if (id)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "SETTINGS on a stream");
		if (flags & FL_ACK) {
			if (len)
				return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "SETTINGS ACK with a payload");
			return 0;
		}
		if (len % 6)
			return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "SETTINGS of %zu bytes", len);
		c->settings_seen = true;
		alt_mutex_lock(&c->mu);
		int err;
		apply_settings(c, p, len, &err);
		if (!err)
			send_frame(c, F_SETTINGS, FL_ACK, 0, 0, 0);
		alt_mutex_unlock(&c->mu);
		if (err)
			return conn_fail(c, (uint32_t)err, -ALT_EHTTP2_PROTOCOL, "invalid setting");
		return 0;
	}
	case F_PING:
		if (id)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "PING on a stream");
		if (len != 8)
			return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "PING of %zu bytes", len);
		if (!(flags & FL_ACK)) {
			alt_mutex_lock(&c->mu);
			send_frame(c, F_PING, FL_ACK, 0, p, 8);
			alt_mutex_unlock(&c->mu);
		}
		return 0;
	case F_GOAWAY: {
		if (id)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "GOAWAY on a stream");
		if (len < 8)
			return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "GOAWAY too short");
		uint32_t last = get32(p) & 0x7fffffff, code = get32(p + 4);
		alt_mutex_lock(&c->mu);
		c->goaway_recv = true;
		c->goaway_last = last;
		/* streams the server did not take: the client may try them elsewhere */
		for (h2stream *s = c->streams, *n; s; s = n) {
			n = s->next;
			if (!c->server && s->id > last) {
				s->retry = true;
				reset_stream(c, s, E_REFUSED, true);
			}
		}
		alt_cond_broadcast(&c->cond);
		alt_mutex_unlock(&c->mu);
		if (code != E_NO)
			return conn_fail(c, (uint32_t)-1, -ALT_EHTTP2_GOAWAY, "received GOAWAY with error code %u", code);
		return 0;
	}
	case F_WINDOW: {
		if (len != 4)
			return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "WINDOW_UPDATE of %zu bytes", len);
		uint32_t inc = get32(p) & 0x7fffffff;
		alt_mutex_lock(&c->mu);
		if (id == 0) {
			if (inc == 0 || c->out_window + inc > MAX_WINDOW) {
				alt_mutex_unlock(&c->mu);
				return conn_fail(c, inc ? E_FLOW : E_PROTOCOL, -ALT_EHTTP2_FLOW, "connection WINDOW_UPDATE of %u", inc);
			}
			c->out_window += inc;
		} else {
			h2stream *s = find(c, id);
			if (s) {
				if (inc == 0)
					reset_stream(c, s, E_PROTOCOL, false);
				else if (s->out_window + inc > MAX_WINDOW)
					reset_stream(c, s, E_FLOW, false);
				else
					s->out_window += inc;
			} else if (id > (c->server ? c->last_peer : c->next_id)) {
				alt_mutex_unlock(&c->mu);
				return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "WINDOW_UPDATE on idle stream %u", id);
			}
		}
		alt_cond_broadcast(&c->cond);
		alt_mutex_unlock(&c->mu);
		return 0;
	}
	case F_RST: {
		if (id == 0)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "RST_STREAM on stream 0");
		if (len != 4)
			return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "RST_STREAM of %zu bytes", len);
		alt_mutex_lock(&c->mu);
		h2stream *s = find(c, id);
		bool idle = !s && id > (c->server ? c->last_peer : c->next_id);
		if (s) {
			s->retry = !c->server && get32(p) == E_REFUSED;
			reset_stream(c, s, get32(p), true);
		}
		alt_mutex_unlock(&c->mu);
		if (idle)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "RST_STREAM on idle stream %u", id);
		return 0;
	}
	case F_PRIORITY:
		if (id == 0)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "PRIORITY on stream 0");
		if (len != 5) {
			alt_mutex_lock(&c->mu);
			send_rst(c, id, E_FRAME_SIZE);
			alt_mutex_unlock(&c->mu);
		}
		return 0;
	case F_PUSH:
		return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "PUSH_PROMISE (push is disabled)");
	case F_CONT:
		return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "CONTINUATION without HEADERS");
	case F_DATA: {
		if (id == 0)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "DATA on stream 0");
		size_t n = len;
		if ((rc = unpad(c, flags, &p, &n)) < 0)
			return rc;
		alt_mutex_lock(&c->mu);
		/* the whole frame counts against the windows */
		c->in_window -= (int64_t)len;
		if (c->in_window < 0) {
			alt_mutex_unlock(&c->mu);
			return conn_fail(c, E_FLOW, -ALT_EHTTP2_FLOW, "the connection's flow control window was exceeded");
		}
		h2stream *s = find(c, id);
		uint32_t back = (uint32_t)(len - n); /* the padding goes back at once */
		if (!s || s->in_end) {
			back = (uint32_t)len;
			bool idle = !s && id > (c->server ? c->last_peer : c->next_id);
			if (idle) {
				alt_mutex_unlock(&c->mu);
				return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "DATA on idle stream %u", id);
			}
			if (s)
				reset_stream(c, s, E_CLOSED, false);
			else
				send_rst(c, id, E_CLOSED);
		} else {
			s->in_window -= (int64_t)len;
			if (s->in_window < 0) {
				reset_stream(c, s, E_FLOW, false);
				back = (uint32_t)len;
			} else {
				s->received += n;
				if (s->declared >= 0 && s->received > (uint64_t)s->declared) {
					reset_stream(c, s, E_PROTOCOL, false);
					back = (uint32_t)len;
				} else {
					alt_buf_append(&s->in, p, n);
					if (flags & FL_END_STREAM) {
						if (s->declared >= 0 && s->received != (uint64_t)s->declared)
							reset_stream(c, s, E_PROTOCOL, false);
						else {
							s->in_end = true;
							maybe_close(c, s);
						}
					}
				}
			}
		}
		c->in_unacked += back;
		if (c->in_unacked >= c->my_conn_window / 4) {
			send_window(c, 0, c->in_unacked);
			c->in_window += c->in_unacked;
			c->in_unacked = 0;
		}
		alt_cond_broadcast(&c->cond);
		alt_mutex_unlock(&c->mu);
		return 0;
	}
	case F_HEADERS: {
		if (id == 0)
			return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "HEADERS on stream 0");
		size_t n = len;
		if ((rc = unpad(c, flags, &p, &n)) < 0)
			return rc;
		if (flags & FL_PRIORITY) {
			if (n < 5)
				return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "HEADERS priority too short");
			if ((get32(p) & 0x7fffffff) == id) {
				alt_mutex_lock(&c->mu);
				send_rst(c, id, E_PROTOCOL);
				alt_mutex_unlock(&c->mu);
			}
			p += 5, n -= 5;
		}
		alt_buf_reset(block);
		alt_buf_append(block, p, n);
		bool end = flags & FL_END_STREAM;
		/* CONTINUATION frames follow, nothing else */
		while (!(flags & FL_END_HEADERS)) {
			if ((rc = read_exact(c, h, 9)) < 0)
				return rc;
			size_t cl = (size_t)h[0] << 16 | (size_t)h[1] << 8 | h[2];
			if (h[3] != F_CONT || (get32(h + 5) & 0x7fffffff) != id)
				return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "expected CONTINUATION of stream %u", id);
			if (cl > c->my_frame)
				return conn_fail(c, E_FRAME_SIZE, -ALT_EHTTP2_PROTOCOL, "frame too large");
			if (block->n + cl > (size_t)c->my_header_list + (64 << 10))
				return conn_fail(c, E_CALM, -ALT_EHTTP2_PROTOCOL, "header block too large");
			flags = h[4];
			size_t at = block->n;
			if (alt_buf_reserve(block, cl) < 0)
				return -ENOMEM;
			if (cl && (rc = read_exact(c, block->p + at, cl)) < 0)
				return rc;
			alt_buf_truncate(block, at + cl);
		}
		/* the block is decoded in any case: the table must stay in step */
		alt_arena *a = malloc(sizeof *a);
		if (!a)
			return -ENOMEM;
		alt_arena_init(a, 4096);
		fields fs = {.a = a, .limit = c->my_header_list};
		if (alt_hpack_decode(&c->dec, block->p, block->n, a, collect, &fs) < 0) {
			alt_arena_free(a);
			free(a);
			return conn_fail(c, E_COMPRESSION, -ALT_EHPACK, "%s", alt_hpack_error(&c->dec));
		}
		if (c->server)
			server_headers(c, id, &fs, end, a); /* takes a */
		else {
			alt_mutex_lock(&c->mu);
			h2stream *s = find(c, id);
			bool idle = !s && id > c->next_id;
			if (s)
				client_headers(c, s, &fs, end);
			alt_mutex_unlock(&c->mu);
			alt_arena_free(a);
			free(a);
			if (idle)
				return conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "HEADERS on idle stream %u", id);
		}
		return 0;
	}
	default:
		return 0; /* unknown frame types are ignored */
	}
}

/* the settings and window a side announces first */
static void send_preamble(h2conn *c) {
	uint8_t p[6 * 5];
	size_t n = 0;
	struct {
		unsigned id;
		uint32_t v;
	} s[5];
	int k = 0;
	if (c->server) {
		s[k].id = S_FRAME, s[k++].v = c->my_frame;
		s[k].id = S_MAX_STREAMS, s[k++].v = c->my_streams;
		s[k].id = S_HEADER_LIST, s[k++].v = c->my_header_list;
		s[k].id = S_WINDOW, s[k++].v = c->my_window;
	} else {
		s[k].id = S_PUSH, s[k++].v = 0;
		s[k].id = S_WINDOW, s[k++].v = c->my_window;
		s[k].id = S_HEADER_LIST, s[k++].v = c->my_header_list;
	}
	for (int i = 0; i < k; i++) {
		p[n] = (uint8_t)(s[i].id >> 8), p[n + 1] = (uint8_t)s[i].id;
		put32(p + n + 2, s[i].v);
		n += 6;
	}
	alt_mutex_lock(&c->mu);
	if (!c->server) {
		qitem *q = item(sizeof preface);
		if (q) {
			memcpy(q->d, preface, sizeof preface);
			enqueue(c, q);
		}
	}
	send_frame(c, F_SETTINGS, 0, 0, p, n);
	if (c->my_conn_window > DEFAULT_WINDOW) {
		send_window(c, 0, c->my_conn_window - DEFAULT_WINDOW);
		c->in_window = c->my_conn_window;
	}
	alt_mutex_unlock(&c->mu);
}

static h2conn *conn_new(bool server, alt_reader rd, alt_writer wr) {
	h2conn *c = calloc(1, sizeof *c);
	if (!c)
		return 0;
	c->server = server;
	c->rd = rd;
	c->wr = wr;
	c->mu = (alt_mutex)ALT_MUTEX_INIT;
	c->cond = (alt_cond)ALT_COND_INIT;
	c->tasks = (alt_waitgroup)ALT_WAITGROUP_INIT;
	alt_hpack_encoder_init(&c->enc);
	alt_hpack_decoder_init(&c->dec, 4096);
	c->peer_frame = 16384;
	c->peer_window = DEFAULT_WINDOW;
	c->peer_streams = UINT32_MAX;
	c->peer_header_list = UINT32_MAX;
	c->out_window = DEFAULT_WINDOW;
	c->in_window = DEFAULT_WINDOW;
	return c;
}

static void conn_free(h2conn *c) {
	while (c->qh) {
		qitem *n = c->qh->next;
		free(c->qh);
		c->qh = n;
	}
	alt_hpack_encoder_free(&c->enc);
	alt_hpack_decoder_free(&c->dec);
	free(c);
}

/* ---- sending messages ---- */

static void lower_name(alt_buf *b, alt_str k) {
	alt_buf_reset(b);
	for (size_t i = 0; i < k.n; i++) {
		char ch = k.p[i];
		alt_buf_append_byte(b, ch >= 'A' && ch <= 'Z' ? ch + 32 : ch);
	}
}

/* encodes and queues a header block (mu held): HEADERS, then CONTINUATION frames */
static void queue_block(h2conn *c, uint32_t id, const alt_buf *blk, bool end) {
	size_t max = c->peer_frame, n = blk->n, off = 0;
	size_t nframes = n ? (n + max - 1) / max : 1;
	qitem *q = item(n + 9 * nframes);
	if (!q)
		return;
	size_t w = 0;
	for (size_t i = 0; i < nframes; i++) {
		size_t k = n - off < max ? n - off : max;
		int flags = (i == nframes - 1 ? FL_END_HEADERS : 0) | (i == 0 && end ? FL_END_STREAM : 0);
		frame_header(q->d + w, k, i == 0 ? F_HEADERS : F_CONT, flags, id);
		if (k)
			memcpy(q->d + w + 9, blk->p + off, k);
		w += 9 + k;
		off += k;
	}
	q->n = w;
	enqueue(c, q);
}

static void encode(h2conn *c, alt_buf *blk, const char *name, alt_str value) {
	alt_hpack_field f = {alt_str_cstr(name), value, false};
	alt_hpack_encode(&c->enc, blk, &f);
}

/* the regular fields of h, lowercase, without the connection-specific ones */
static void encode_header(h2conn *c, alt_buf *blk, const alt_http_header *h, bool request) {
	alt_buf name = ALT_BUF_INIT;
	for (size_t i = 0; h && i < h->n; i++) {
		alt_str k = h->f[i].key;
		lower_name(&name, k);
		alt_str ln = alt_buf_str(&name);
		if (connection_specific(ln) || (request && (alt_str_eq_cstr(ln, "host") || alt_str_eq_cstr(ln, "content-length"))))
			continue;
		if (alt_str_eq_cstr(ln, "te") && !alt_str_eq_fold(h->f[i].value, ALT_STR("trailers")))
			continue;
		alt_hpack_field f = {ln, h->f[i].value, alt_str_eq_cstr(ln, "authorization") || alt_str_eq_cstr(ln, "proxy-authorization")};
		alt_hpack_encode(&c->enc, blk, &f);
	}
	alt_buf_free(&name);
}

/* writes p as DATA frames within the windows; end closes the stream */
static int send_data(h2conn *c, h2stream *s, const uint8_t *p, size_t n, bool end, alt_ctx *ctx) {
	alt_mutex_lock(&c->mu);
	int rc = 0;
	for (;;) {
		if (c->dead || c->writer_stop) {
			rc = c->err ? c->err : -ALT_ECLOSED;
			break;
		}
		if (s->reset) {
			rc = -ALT_EHTTP2_STREAM;
			break;
		}
		if (s->out_end) {
			rc = -ALT_ECLOSED;
			break;
		}
		if (ctx && (rc = alt_ctx_err(ctx)) < 0)
			break;
		if (n == 0) {
			if (end) {
				send_frame(c, F_DATA, FL_END_STREAM, s->id, 0, 0);
				s->out_end = true;
				maybe_close(c, s);
			}
			break;
		}
		int64_t avail = s->out_window < c->out_window ? s->out_window : c->out_window;
		if (avail > (int64_t)c->peer_frame)
			avail = c->peer_frame;
		if (avail <= 0 || c->qbytes > QUEUE_LIMIT) {
			if (ctx)
				alt_cond_wait_until(&c->cond, &c->mu, alt_deadline(50 * ALT_MS));
			else
				alt_cond_wait(&c->cond, &c->mu);
			continue;
		}
		size_t k = n < (size_t)avail ? n : (size_t)avail;
		bool last = k == n && end;
		send_frame(c, F_DATA, last ? FL_END_STREAM : 0, s->id, p, k);
		s->out_window -= (int64_t)k;
		c->out_window -= (int64_t)k;
		p += k, n -= k;
		if (last) {
			s->out_end = true;
			maybe_close(c, s);
			break;
		}
	}
	alt_mutex_unlock(&c->mu);
	return rc;
}

/* the body of the peer's message */
static ssize_t body_read(h2conn *c, h2stream *s, void *buf, size_t n, alt_ctx *ctx) {
	alt_mutex_lock(&c->mu);
	ssize_t res;
	for (;;) {
		size_t have = s->in.n - s->in_off;
		if (have) {
			size_t k = n < have ? n : have;
			memcpy(buf, s->in.p + s->in_off, k);
			s->in_off += k;
			if (s->in_off == s->in.n) {
				alt_buf_reset(&s->in);
				s->in_off = 0;
			}
			/* the windows open again as the reader takes data */
			if (!c->dead) {
				c->in_unacked += (uint32_t)k;
				if (c->in_unacked >= c->my_conn_window / 4) {
					send_window(c, 0, c->in_unacked);
					c->in_window += c->in_unacked;
					c->in_unacked = 0;
				}
				if (!s->in_end && !s->reset) {
					s->in_unacked += (uint32_t)k;
					if (s->in_unacked >= c->my_window / 4) {
						send_window(c, s->id, s->in_unacked);
						s->in_window += s->in_unacked;
						s->in_unacked = 0;
					}
				}
			}
			res = (ssize_t)k;
			break;
		}
		if (s->in_end) {
			res = 0;
			break;
		}
		if (s->reset) {
			res = -ALT_EHTTP2_STREAM;
			break;
		}
		if (c->dead) {
			res = c->err ? c->err : -ALT_EUNEXPECTED_EOF;
			break;
		}
		if (n == 0) {
			res = 0;
			break;
		}
		if (ctx && (res = alt_ctx_err(ctx)) < 0)
			break;
		if (ctx)
			alt_cond_wait_until(&c->cond, &c->mu, alt_deadline(50 * ALT_MS));
		else
			alt_cond_wait(&c->cond, &c->mu);
	}
	alt_mutex_unlock(&c->mu);
	return res;
}

/* ======== the server ======== */

typedef struct {
	h2stream *s;
} stream_body;

static ssize_t server_body_read(void *obj, void *buf, size_t n) {
	h2stream *s = obj;
	h2conn *c = s->c;
	if (s->need_continue) {
		s->need_continue = false;
		alt_buf blk = ALT_BUF_INIT;
		alt_mutex_lock(&c->mu);
		encode(c, &blk, ":status", ALT_STR("100"));
		queue_block(c, s->id, &blk, false);
		alt_mutex_unlock(&c->mu);
		alt_buf_free(&blk);
	}
	return body_read(c, s, buf, n, 0);
}

int alt__h2_send_headers(struct alt__h2_stream *s, int status, const alt_http_header *h, bool end) {
	h2conn *c = s->c;
	alt_buf blk = ALT_BUF_INIT;
	char st[8];
	snprintf(st, sizeof st, "%03d", status);
	alt_mutex_lock(&c->mu);
	int rc = 0;
	if (c->dead)
		rc = c->err ? c->err : -ALT_ECLOSED;
	else if (s->reset)
		rc = -ALT_EHTTP2_STREAM;
	else {
		encode(c, &blk, ":status", alt_str_cstr(st));
		encode_header(c, &blk, h, false);
		queue_block(c, s->id, &blk, end);
		if (end) {
			s->out_end = true;
			maybe_close(c, s);
		}
	}
	alt_mutex_unlock(&c->mu);
	alt_buf_free(&blk);
	return rc;
}

int alt__h2_send_data(struct alt__h2_stream *s, const void *p, size_t n, bool end) {
	return send_data(s->c, s, p, n, end, 0);
}

bool alt__h2_stream_ended(const struct alt__h2_stream *s) { return s->out_end; }

/* builds the request of a new stream from its fields */
static bool build_request(h2conn *c, h2stream *s, fields *fs, bool end) {
	alt_arena *a = &s->arena;
	alt_http_request *r = alt_arena_zalloc(a, sizeof *r);
	alt__http_request_state *rs = alt_arena_zalloc(a, sizeof *rs);
	if (!r || !rs)
		return false;
	r->internal = rs;
	rs->conn = c->hc;
	alt_str method = ALT_STR_NULL, scheme = ALT_STR_NULL, path = ALT_STR_NULL, authority = ALT_STR_NULL;
	bool hm = false, hs = false, hp = false, ha = false;
	size_t i = 0;
	for (; i < fs->n && fs->f[i].name.n && fs->f[i].name.p[0] == ':'; i++) {
		alt_str n = fs->f[i].name, v = fs->f[i].value;
		bool *seen;
		alt_str *dst;
		if (alt_str_eq_cstr(n, ":method"))
			seen = &hm, dst = &method;
		else if (alt_str_eq_cstr(n, ":scheme"))
			seen = &hs, dst = &scheme;
		else if (alt_str_eq_cstr(n, ":path"))
			seen = &hp, dst = &path;
		else if (alt_str_eq_cstr(n, ":authority"))
			seen = &ha, dst = &authority;
		else
			return false; /* unknown or response pseudo-headers */
		if (*seen)
			return false;
		*seen = true;
		*dst = v;
	}
	if (!regular_ok(fs, i) || !hm || method.n == 0)
		return false;
	bool connect = alt_str_eq_cstr(method, "CONNECT");
	if (connect ? (hs || hp || !ha) : (!hs || !hp || path.n == 0))
		return false;
	r->method = method;
	r->proto = ALT_STR("HTTP/2.0");
	r->proto_major = 2;
	alt_http_header_init(&r->header, a);
	alt_http_header_init(&r->trailer, a);
	alt_buf cookies = ALT_BUF_INIT;
	for (; i < fs->n; i++) {
		alt_str n = fs->f[i].name, v = fs->f[i].value;
		if (alt_str_eq_cstr(n, "cookie")) {
			/* split cookie fields join again (RFC 9113 8.2.3) */
			if (cookies.n)
				alt_buf_append_cstr(&cookies, "; ");
			alt_buf_append_str(&cookies, v);
			continue;
		}
		alt_http_header_add(&r->header, n, v);
	}
	if (cookies.n)
		alt_http_header_add(&r->header, ALT_STR("cookie"), alt_buf_str(&cookies));
	alt_buf_free(&cookies);
	r->request_uri = connect ? authority : path;
	if (connect) {
		r->url.host = authority;
	} else if (alt_url_parse_request_uri(a, path, &r->url) < 0)
		return false;
	alt_str host;
	r->host = ha ? authority : alt_http_header_get(&r->header, ALT_STR("Host"), &host) ? host : ALT_STR("");
	if (!r->url.host.n)
		r->url.host = r->host;
	r->content_length = -1;
	s->declared = -1;
	alt_str cl;
	if (alt_http_header_get(&r->header, ALT_STR("Content-Length"), &cl)) {
		int64_t v = parse_cl(cl);
		if (v < -1)
			return false;
		r->content_length = v;
		s->declared = v;
		if (end && v != 0)
			return false;
	}
	if (end)
		r->content_length = 0;
	alt_str expect;
	if (!end && alt_http_header_get(&r->header, ALT_STR("Expect"), &expect) && alt_str_eq_fold(expect, ALT_STR("100-continue")))
		s->need_continue = true;
	r->body = (alt_reader){server_body_read, s};
	r->remote_addr = alt_str_cstr(c->hc->remote);
	r->tls = c->hc->tls;
	r->arena = a;
	s->req = r;
	return true;
}

static void stream_task(void *arg);

static void server_headers(h2conn *c, uint32_t id, fields *fs, bool end, alt_arena *a) {
	alt_mutex_lock(&c->mu);
	h2stream *s = find(c, id);
	if (s || id <= c->last_peer) {
		/* trailers of an open stream; anything else on a known stream is wrong */
		if (!s) {
			alt_mutex_unlock(&c->mu);
			alt_arena_free(a);
			free(a);
			conn_fail(c, E_CLOSED, -ALT_EHTTP2_PROTOCOL, "HEADERS on closed stream %u", id);
			return;
		}
		if (s->in_end || !end || !regular_ok(fs, 0))
			reset_stream(c, s, s->in_end ? E_CLOSED : E_PROTOCOL, false);
		else {
			for (size_t i = 0; i < fs->n; i++)
				alt_http_header_add(&s->req->trailer, fs->f[i].name, fs->f[i].value);
			if (s->declared >= 0 && s->received != (uint64_t)s->declared)
				reset_stream(c, s, E_PROTOCOL, false);
			else {
				s->in_end = true;
				maybe_close(c, s);
			}
		}
		alt_mutex_unlock(&c->mu);
		alt_arena_free(a);
		free(a);
		return;
	}
	if (id % 2 == 0) {
		alt_mutex_unlock(&c->mu);
		alt_arena_free(a);
		free(a);
		conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "client opened an even stream %u", id);
		return;
	}
	c->last_peer = id;
	alt_http_server *srv = c->hc->srv;
	if (c->goaway_sent || atomic_load(&srv->shutting_down)) {
		alt_mutex_unlock(&c->mu);
		alt_arena_free(a);
		free(a);
		return; /* after GOAWAY new streams are ignored */
	}
	if (c->nactive >= c->my_streams) {
		send_rst(c, id, E_REFUSED);
		alt_mutex_unlock(&c->mu);
		alt_arena_free(a);
		free(a);
		return;
	}
	s = calloc(1, sizeof *s);
	if (!s) {
		alt_mutex_unlock(&c->mu);
		alt_arena_free(a);
		free(a);
		return;
	}
	s->c = c;
	s->id = id;
	s->refs = 2;
	s->in_window = c->my_window;
	s->out_window = c->peer_window;
	s->in_end = end;
	s->arena = *a; /* the fields live in it */
	free(a);
	s->too_large = fs->too_large;
	if (!s->too_large && !build_request(c, s, fs, end)) {
		send_rst(c, id, E_PROTOCOL);
		alt_mutex_unlock(&c->mu);
		stream_free(s);
		return;
	}
	if (s->too_large) {
		/* answered 431 as Go does: a minimal request for the handler of errors */
		alt_http_request *r = alt_arena_zalloc(&s->arena, sizeof *r);
		alt__http_request_state *rs = alt_arena_zalloc(&s->arena, sizeof *rs);
		if (!r || !rs) {
			send_rst(c, id, E_INTERNAL);
			alt_mutex_unlock(&c->mu);
			stream_free(s);
			return;
		}
		r->internal = rs;
		r->method = ALT_STR("GET");
		r->proto = ALT_STR("HTTP/2.0");
		r->proto_major = 2;
		alt_http_header_init(&r->header, &s->arena);
		alt_http_header_init(&r->trailer, &s->arena);
		r->arena = &s->arena;
		r->body = (alt_reader){server_body_read, s};
		s->req = r;
	}
	s->in_table = true;
	s->next = c->streams;
	c->streams = s;
	c->nactive++;
	atomic_store(&c->hc->state, CONN_ACTIVE);
	alt_ctx_new(&s->ctx, srv->base);
	s->req->ctx = s->ctx;
	alt_waitgroup_add(&c->tasks, 1);
	alt_mutex_unlock(&c->mu);
	unsigned w = atomic_fetch_add(&srv->rr, 1) % (unsigned)srv->nworkers;
	if (alt_go_on(srv->workers[w], stream_task, s, 0) < 0)
		stream_task(s);
}

static void stream_task(void *arg) {
	h2stream *s = arg;
	h2conn *c = s->c;
	alt_http_server *srv = c->hc->srv;
	alt_http_rw w;
	memset(&w, 0, sizeof w);
	w.hc = c->hc;
	w.h2 = s;
	w.req = s->req;
	w.declared = -1;
	w.head = alt_str_eq_cstr(s->req->method, "HEAD");
	w.body_allowed = true;
	w.pre = alt_arena_alloc(&s->arena, PRE_BUFFER);
	alt_http_header_init(&w.header, &s->arena);
	if (w.pre) {
		if (s->too_large)
			alt_http_error(&w, "431 Request Header Fields Too Large", 431);
		else
			srv->cfg.handler.fn(&w, s->req, srv->cfg.handler.arg);
		alt__http_rw_finish(&w);
	}
	alt_mutex_lock(&c->mu);
	/* the client still sending a body nobody reads: stop it (Go resets with NO_ERROR) */
	if (!s->in_end && !s->reset && s->out_end)
		reset_stream(c, s, E_NO, false);
	else if (!s->out_end && !s->reset)
		reset_stream(c, s, E_INTERNAL, false);
	discard_unread(c, s);
	alt_ctx_cancel(s->ctx);
	alt_ctx *ctx = s->ctx;
	s->ctx = 0;
	if (s->in_table && s->in_end && s->out_end)
		remove_stream(c, s);
	unref(s);
	if (c->nactive == 0) {
		atomic_store(&c->hc->state, CONN_IDLE);
		/* the reader waits for frames: wake it for the idle timeout or a shutdown */
		if (atomic_load(&srv->shutting_down))
			alt_conn_set_read_deadline(c->hc->c, alt_deadline(0));
	}
	alt_cond_broadcast(&c->cond);
	alt_mutex_unlock(&c->mu);
	alt_ctx_free(ctx);
	alt_waitgroup_done(&c->tasks);
}

void alt__h2_serve(conn *hc) {
	alt_http_server *srv = hc->srv;
	h2conn *c = conn_new(true, alt_bufreader_reader(&hc->br), hc->tls ? alt_tls_writer(hc->tls) : alt_conn_writer(hc->c));
	if (!c)
		return;
	c->hc = hc;
	c->br = &hc->br;
	c->c = hc->c;
	c->my_frame = SERVER_FRAME;
	c->my_window = SERVER_WINDOW;
	c->my_conn_window = SERVER_CONN_WINDOW;
	c->my_streams = srv->cfg.http2_max_concurrent_streams ? srv->cfg.http2_max_concurrent_streams : SERVER_STREAMS;
	size_t mh = srv->cfg.max_header_bytes ? srv->cfg.max_header_bytes : 1 << 20;
	c->my_header_list = (uint32_t)(mh + 32 * 100); /* Go adds room for the per-field overhead */
	hc->h2 = c;
	alt_dur idle = srv->cfg.idle_timeout ? srv->cfg.idle_timeout : srv->cfg.read_timeout;
	alt_waitgroup_add(&c->tasks, 1);
	if (alt_go_on(alt_sched_current(), writer_task, c, 0) < 0) {
		conn_free(c);
		hc->h2 = 0;
		return;
	}
	send_preamble(c);
	uint8_t pre[24];
	alt_conn_set_read_deadline(hc->c, alt_deadline(srv->cfg.read_header_timeout ? srv->cfg.read_header_timeout : 10 * ALT_SEC));
	int rc = read_exact(c, pre, 24);
	if (rc == 0 && memcmp(pre, preface, 24) != 0)
		rc = conn_fail(c, E_PROTOCOL, -ALT_EHTTP2_PROTOCOL, "bogus greeting");
	alt_buf payload = ALT_BUF_INIT, block = ALT_BUF_INIT;
	atomic_store(&hc->state, CONN_IDLE);
	while (rc == 0) {
		alt_mutex_lock(&c->mu);
		bool idle_now = c->nactive == 0;
		bool stop = atomic_load(&srv->closed) || (atomic_load(&srv->shutting_down) && idle_now);
		if (atomic_load(&srv->shutting_down) && !c->goaway_sent)
			send_goaway(c, E_NO, 0);
		alt_mutex_unlock(&c->mu);
		if (stop)
			break;
		alt_conn_set_read_deadline(hc->c, idle_now && idle ? alt_deadline(idle) : ALT_FOREVER);
		rc = read_one(c, &payload, &block);
		if (rc == -ETIMEDOUT) {
			alt_mutex_lock(&c->mu);
			bool still_idle = c->nactive == 0;
			alt_mutex_unlock(&c->mu);
			if (atomic_load(&srv->shutting_down) || atomic_load(&srv->closed))
				rc = 0; /* the loop sends GOAWAY and ends */
			else if (still_idle) {
				alt_mutex_lock(&c->mu);
				send_goaway(c, E_NO, "idle timeout");
				alt_mutex_unlock(&c->mu);
				break;
			} else
				rc = 0;
		}
	}
	alt_buf_free(&payload);
	alt_buf_free(&block);
	/* the end: no new streams; the handlers finish (or are cancelled), the writer drains */
	alt_mutex_lock(&c->mu);
	if (!c->goaway_sent && !c->dead)
		send_goaway(c, E_NO, 0);
	if (rc < 0 || atomic_load(&srv->closed)) {
		c->dead = true;
		if (!c->err)
			c->err = rc < 0 ? rc : -ALT_ECLOSED;
		for (h2stream *s = c->streams; s; s = s->next)
			if (s->ctx)
				alt_ctx_cancel(s->ctx);
	}
	alt_cond_broadcast(&c->cond);
	alt_mutex_unlock(&c->mu);
	/* wait for the handlers, then stop the writer */
	alt_mutex_lock(&c->mu);
	while (c->nactive > 0 && !c->dead)
		alt_cond_wait_until(&c->cond, &c->mu, alt_deadline(100 * ALT_MS));
	c->writer_stop = true;
	alt_cond_broadcast(&c->cond);
	alt_mutex_unlock(&c->mu);
	alt_waitgroup_wait(&c->tasks);
	/* the streams left (reset by the end) */
	while (c->streams) {
		h2stream *s = c->streams;
		c->streams = s->next;
		if (--s->refs == 0)
			stream_free(s);
	}
	hc->h2 = 0;
	conn_free(c);
}

/* ======== the client ======== */

static void client_headers(h2conn *c, h2stream *s, fields *fs, bool end) {
	if (s->got_headers) {
		/* trailers */
		if (!end || s->in_end || !regular_ok(fs, 0)) {
			reset_stream(c, s, E_PROTOCOL, false);
			return;
		}
		for (size_t i = 0; i < fs->n; i++)
			alt_http_header_add(s->trailer, fs->f[i].name, fs->f[i].value);
		s->in_end = true;
		maybe_close(c, s);
		return;
	}
	alt_str status = ALT_STR_NULL;
	size_t i = 0;
	bool seen = false;
	for (; i < fs->n && fs->f[i].name.n && fs->f[i].name.p[0] == ':'; i++) {
		if (!alt_str_eq_cstr(fs->f[i].name, ":status") || seen) {
			reset_stream(c, s, E_PROTOCOL, false);
			return;
		}
		seen = true;
		status = fs->f[i].value;
	}
	long long code;
	if (!seen || status.n != 3 || alt_str_to_int(status, 10, &code) < 0 || code < 100 || !regular_ok(fs, i)) {
		reset_stream(c, s, E_PROTOCOL, false);
		return;
	}
	if (code < 200) {
		if (end)
			reset_stream(c, s, E_PROTOCOL, false);
		return; /* informational responses are skipped (100 continue, 103) */
	}
	s->status = (int)code;
	s->content_length = -1;
	for (; i < fs->n; i++) {
		alt_http_header_add(s->resp_header, fs->f[i].name, fs->f[i].value);
		if (alt_str_eq_cstr(fs->f[i].name, "content-length")) {
			int64_t v = parse_cl(fs->f[i].value);
			if (v < 0) {
				reset_stream(c, s, E_PROTOCOL, false);
				return;
			}
			s->content_length = v;
			s->declared = v;
		}
	}
	s->got_headers = true;
	if (end) {
		s->in_end = true;
		maybe_close(c, s);
	}
	alt_cond_broadcast(&c->cond);
}

static void client_conn_unref(h2conn *c);

static void reader_task(void *arg) {
	h2conn *c = arg;
	alt_buf payload = ALT_BUF_INIT, block = ALT_BUF_INIT;
	int rc;
	while ((rc = read_one(c, &payload, &block)) == 0) {
	}
	alt_buf_free(&payload);
	alt_buf_free(&block);
	conn_fail(c, (uint32_t)-1, rc, "%s", alt_strerror(rc));
	alt_mutex_lock(&c->mu);
	/* every stream ends with the connection */
	for (h2stream *s = c->streams, *n; s; s = n) {
		n = s->next;
		reset_stream(c, s, E_CANCEL, true);
	}
	alt_mutex_unlock(&c->mu);
	/* closing the connection stops the writer if it waits on the network */
	alt_conn_set_deadline(c->c, alt_deadline(0));
	alt_waitgroup_done(&c->tasks);
	client_conn_unref(c);
}

int alt__h2_client_new(struct alt__h2_conn **out, alt_conn *conn_, alt_tls_conn *tls, alt_sched *sched, const char *key) {
	alt_reader rd = tls ? alt_tls_reader(tls) : alt_conn_reader(conn_);
	alt_writer wr = tls ? alt_tls_writer(tls) : alt_conn_writer(conn_);
	h2conn *c = conn_new(false, rd, wr);
	if (!c)
		return -ENOMEM;
	if (alt_bufreader_init(&c->own_br, rd, 32 << 10) < 0) {
		conn_free(c);
		return -ENOMEM;
	}
	c->br = &c->own_br;
	c->c = conn_;
	c->tls = tls;
	c->sched = sched;
	c->my_frame = 16384;
	c->my_window = CLIENT_WINDOW;
	c->my_conn_window = CLIENT_CONN_WINDOW;
	c->my_header_list = CLIENT_HEADER_LIST;
	c->next_id = 1;
	c->settings_seen = true; /* the server's SETTINGS is checked as it comes, not required first by us */
	c->refs = 2;             /* the pool, the reader */
	snprintf(c->key, sizeof c->key, "%s", key);
	alt_conn_set_deadline(conn_, ALT_FOREVER);
	alt__conn_set_ctx(conn_, 0);
	alt_waitgroup_add(&c->tasks, 2);
	send_preamble(c);
	if (alt_go_on(sched, writer_task, c, 0) < 0) {
		alt_bufreader_free(&c->own_br);
		conn_free(c);
		return -ENOMEM;
	}
	if (alt_go_on(sched, reader_task, c, 0) < 0) {
		conn_fail(c, (uint32_t)-1, -ENOMEM, "no task");
		alt_waitgroup_done(&c->tasks);
		c->refs--;
	}
	*out = c;
	return 0;
}

const char *alt__h2_client_key(const struct alt__h2_conn *c) { return c->key; }

bool alt__h2_client_usable(struct alt__h2_conn *c) {
	alt_mutex_lock(&c->mu);
	bool ok = !c->dead && !c->goaway_recv && !c->goaway_sent && c->next_id < 0x7fff0000;
	alt_mutex_unlock(&c->mu);
	return ok;
}

static void client_conn_unref(h2conn *c) {
	alt_mutex_lock(&c->mu);
	bool last = --c->refs == 0;
	alt_mutex_unlock(&c->mu);
	if (!last)
		return;
	alt_mutex_lock(&c->mu);
	c->writer_stop = true;
	alt_cond_broadcast(&c->cond);
	alt_mutex_unlock(&c->mu);
	alt_waitgroup_wait(&c->tasks);
	if (c->tls)
		alt_tls_close(c->tls);
	else
		alt_conn_close(c->c);
	alt_bufreader_free(&c->own_br);
	conn_free(c);
}

/* the pool lets the connection go: it closes once its streams are done */
void alt__h2_client_release(struct alt__h2_conn *c) {
	alt_mutex_lock(&c->mu);
	if (!c->goaway_sent && !c->dead) {
		send_goaway(c, E_NO, 0);
	}
	/* the reader ends when the server closes after our GOAWAY, or now */
	if (c->nactive == 0)
		alt_conn_set_read_deadline(c->c, alt_deadline(0));
	alt_mutex_unlock(&c->mu);
	client_conn_unref(c);
}

typedef struct {
	h2stream *s;
	alt_ctx *ctx;
} client_body;

static void body_task(void *arg) {
	h2stream *s = arg;
	h2conn *c = s->c;
	char buf[16384];
	int64_t sent = 0, length = s->body_len;
	int rc = 0;
	for (;;) {
		size_t want = sizeof buf;
		if (length >= 0 && length - sent < (int64_t)want)
			want = (size_t)(length - sent);
		ssize_t k = want ? alt_read(s->body, buf, want) : 0;
		if (k < 0) {
			rc = (int)k;
			break;
		}
		if (k == 0 && length >= 0 && sent < length) {
			rc = -ALT_EUNEXPECTED_EOF;
			break;
		}
		bool last = k == 0 || (length >= 0 && sent + k == length);
		if ((rc = send_data(c, s, (const uint8_t *)buf, (size_t)k, last, s->body_ctx)) < 0)
			break;
		sent += k;
		if (last)
			break;
	}
	/* a failed body ends the stream; a reset by the server (an early answer) is no failure here */
	if (rc < 0 && rc != -ALT_EHTTP2_STREAM) {
		alt_mutex_lock(&c->mu);
		reset_stream(c, s, E_CANCEL, false);
		alt_mutex_unlock(&c->mu);
	}
	alt_waitgroup_done(&s->body_wg);
}

static ssize_t client_body_read(void *obj, void *buf, size_t n) {
	client_body *b = obj;
	return body_read(b->s->c, b->s, buf, n, b->ctx);
}

/* sends a request on a new stream and waits for the response's header:
   0 with resp filled, or an error (*retry: the server did not process it) */
int alt__h2_round_trip(struct alt__h2_conn *c, const char *method, const alt_url *u, const alt_http_header *extra, alt_reader *body,
	int64_t length, bool gzip, const char *ua, alt_arena *a, alt_http_response *resp, alt_ctx *ctx, struct alt__h2_stream **out,
	bool *retry) {
	*retry = false;
	*out = 0;
	h2stream *s = calloc(1, sizeof *s);
	client_body *cb = alt_arena_zalloc(a, sizeof *cb);
	if (!s || !cb) {
		free(s);
		return -ENOMEM;
	}
	s->c = c;
	s->refs = 2;
	s->declared = -1;
	s->ra = a;
	s->resp_header = &resp->header;
	s->trailer = &resp->trailer;
	alt_http_header_init(&resp->header, a);
	alt_http_header_init(&resp->trailer, a);
	bool has_body = body && length != 0;
	alt_buf blk = ALT_BUF_INIT, path = ALT_BUF_INIT;
	alt_url_request_uri(u, &path);
	alt_mutex_lock(&c->mu);
	int rc = 0;
	/* the peer's limit of concurrent streams: wait for one to end */
	while (!c->dead && !c->goaway_recv && c->nactive >= c->peer_streams && (rc = alt_ctx_err(ctx)) == 0)
		alt_cond_wait_until(&c->cond, &c->mu, alt_deadline(50 * ALT_MS));
	if (rc == 0 && (c->dead || c->goaway_recv)) {
		rc = -ALT_EHTTP2_GOAWAY;
		*retry = true;
	}
	if (rc < 0) {
		alt_mutex_unlock(&c->mu);
		free(s);
		alt_buf_free(&blk);
		alt_buf_free(&path);
		return rc;
	}
	s->id = c->next_id;
	c->next_id += 2;
	s->in_window = c->my_window;
	s->out_window = c->peer_window;
	s->in_table = true;
	s->next = c->streams;
	c->streams = s;
	c->nactive++;
	encode(c, &blk, ":authority", u->host);
	encode(c, &blk, ":method", alt_str_cstr(method));
	encode(c, &blk, ":path", alt_buf_str(&path));
	encode(c, &blk, ":scheme", u->scheme);
	encode_header(c, &blk, extra, true);
	alt_str v;
	if (!(extra && alt_http_header_get(extra, ALT_STR("User-Agent"), &v)))
		encode(c, &blk, "user-agent", alt_str_cstr(ua));
	if (gzip)
		encode(c, &blk, "accept-encoding", ALT_STR("gzip"));
	char cl[24];
	if (has_body && length > 0) {
		snprintf(cl, sizeof cl, "%lld", (long long)length);
		encode(c, &blk, "content-length", alt_str_cstr(cl));
	} else if (!has_body && (!strcmp(method, "POST") || !strcmp(method, "PUT") || !strcmp(method, "PATCH")))
		encode(c, &blk, "content-length", ALT_STR("0"));
	queue_block(c, s->id, &blk, !has_body);
	if (!has_body)
		s->out_end = true;
	alt_mutex_unlock(&c->mu);
	alt_buf_free(&blk);
	alt_buf_free(&path);
	/* the body, while the response comes */
	if (has_body) {
		s->body = *body;
		s->body_len = length;
		s->body_ctx = ctx;
		s->body_wg = (alt_waitgroup)ALT_WAITGROUP_INIT;
		alt_waitgroup_add(&s->body_wg, 1);
		if (alt_go_on(c->sched, body_task, s, 0) < 0)
			body_task(s);
	}
	/* the response's header */
	alt_mutex_lock(&c->mu);
	while (rc == 0 && !s->got_headers && !s->reset && !c->dead && (rc = alt_ctx_err(ctx)) == 0)
		alt_cond_wait_until(&c->cond, &c->mu, alt_deadline(50 * ALT_MS));
	if (rc == 0 && !s->got_headers) {
		*retry = s->retry;
		rc = s->reset ? (s->retry ? -ALT_EHTTP2_GOAWAY : -ALT_EHTTP2_STREAM) : c->err ? c->err : -ALT_EUNEXPECTED_EOF;
	}
	if (rc < 0) {
		reset_stream(c, s, E_CANCEL, false);
		alt_mutex_unlock(&c->mu);
		if (has_body)
			alt_waitgroup_wait(&s->body_wg);
		alt_mutex_lock(&c->mu);
		unref(s);
		alt_mutex_unlock(&c->mu);
		return rc;
	}
	resp->status = s->status;
	resp->arena = a;
	resp->proto = ALT_STR("HTTP/2.0");
	resp->proto_major = 2;
	resp->proto_minor = 0;
	resp->content_length = s->content_length;
	if (!strcmp(method, "HEAD"))
		resp->content_length = s->content_length;
	char st[64];
	snprintf(st, sizeof st, "%d %s", s->status, alt_http_status_text(s->status));
	resp->status_text = alt_arena_str(a, alt_str_cstr(st));
	c->refs++; /* the stream keeps the connection */
	alt_mutex_unlock(&c->mu);
	cb->s = s;
	cb->ctx = ctx;
	resp->body = (alt_reader){client_body_read, cb};
	*out = s;
	return 0;
}

/* the response is done: an unfinished stream is cancelled (as Go) */
void alt__h2_stream_done(struct alt__h2_stream *s) {
	h2conn *c = s->c;
	alt_mutex_lock(&c->mu);
	if ((!s->in_end || !s->out_end) && !s->reset)
		reset_stream(c, s, E_CANCEL, false);
	alt_mutex_unlock(&c->mu);
	if (s->body_ctx)
		alt_waitgroup_wait(&s->body_wg); /* the request body is not read after this */
	alt_mutex_lock(&c->mu);
	discard_unread(c, s);
	unref(s);
	alt_mutex_unlock(&c->mu);
	client_conn_unref(c);
}

/* whether the body was read to its end without errors */
bool alt__h2_stream_complete(const struct alt__h2_stream *s) { return s->in_end && s->in_off >= s->in.n && !s->reset; }

alt_time alt__h2_client_idle_since(struct alt__h2_conn *c, bool *idle) {
	alt_mutex_lock(&c->mu);
	*idle = c->nactive == 0;
	if (!*idle)
		c->idle_since = 0;
	else if (!c->idle_since)
		c->idle_since = alt_now();
	alt_time t = c->idle_since;
	alt_mutex_unlock(&c->mu);
	return t;
}
