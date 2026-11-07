/* The HTTP/1.1 client, after Go's net/http client.go and transport.go
   (BSD-3-Clause, see LICENSE-Go): a pool of idle connections per host,
   redirects, timeouts through contexts */
#define _GNU_SOURCE
#include "../net/net_internal.h"
#include "http_internal.h"
#include <alt/compress/gzip.h>
#include <alt/crypto/tls.h>
#include <alt/net/cookiejar.h>
#include <alt/errors.h>
#include <alt/sync.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

typedef struct pconn {
	struct pconn *next;
	char key[300];      /* "https:host:port" or "http:host:port" */
	alt_conn *c;
	alt_tls_conn *tls;
	alt_bufreader br;
	alt_bufwriter bw;
	alt_time idle_since;
} pconn;

struct alt_http_client {
	alt_http_client_config cfg;
	alt_mutex mu;
	pconn *idle;
	/* HTTP/2 connections, shared by concurrent requests */
	struct alt__h2_conn *h2[32];
	size_t nh2;
	alt_sched *bg;      /* their reader and writer tasks */
};

typedef struct {
	alt_http_client *client;
	pconn *pc;
	alt_ctx *ctx;       /* the client's timeout, freed with the response */
	alt_arena arena;
	alt_reader raw;     /* the body as received */
	alt_gzip_reader *gz; /* decompressing raw, made at the first read */
	int gz_err;
	struct alt__h2_stream *h2s; /* the HTTP/2 stream of the response */
} resp_state;

int alt_http_client_new(alt_http_client **out, const alt_http_client_config *cfg) {
	alt_http_client *c = calloc(1, sizeof *c);
	if (!c)
		return -ENOMEM;
	if (cfg)
		c->cfg = *cfg;
	if (!c->cfg.max_redirects)
		c->cfg.max_redirects = 10;
	if (!c->cfg.max_idle_per_host)
		c->cfg.max_idle_per_host = 2;
	if (!c->cfg.idle_timeout)
		c->cfg.idle_timeout = 90 * ALT_SEC;
	if (!c->cfg.dial_timeout)
		c->cfg.dial_timeout = 30 * ALT_SEC;
	if (!c->cfg.user_agent)
		c->cfg.user_agent = "AltC-http-client/1.0";
	c->mu = (alt_mutex)ALT_MUTEX_INIT;
	*out = c;
	return 0;
}

static void pconn_close(pconn *pc) {
	if (!pc)
		return;
	if (pc->tls)
		alt_tls_close(pc->tls); /* closes c */
	else
		alt_conn_close(pc->c);
	alt_bufreader_free(&pc->br);
	alt_bufwriter_free(&pc->bw);
	free(pc);
}

void alt_http_client_close_idle(alt_http_client *c) {
	alt_mutex_lock(&c->mu);
	pconn *list = c->idle;
	c->idle = 0;
	/* HTTP/2 connections without streams go too (as Go's CloseIdleConnections) */
	struct alt__h2_conn *drop[32];
	size_t nd = 0;
	for (size_t i = 0; i < c->nh2;) {
		bool idle;
		alt__h2_client_idle_since(c->h2[i], &idle);
		if (idle) {
			drop[nd++] = c->h2[i];
			c->h2[i] = c->h2[--c->nh2];
		} else
			i++;
	}
	alt_mutex_unlock(&c->mu);
	for (size_t i = 0; i < nd; i++)
		alt__h2_client_release(drop[i]);
	while (list) {
		pconn *n = list->next;
		pconn_close(list);
		list = n;
	}
}

void alt_http_client_free(alt_http_client *c) {
	if (!c)
		return;
	alt_http_client_close_idle(c);
	alt_mutex_lock(&c->mu);
	size_t n = c->nh2;
	struct alt__h2_conn *all[32];
	memcpy(all, c->h2, n * sizeof *all);
	c->nh2 = 0;
	alt_mutex_unlock(&c->mu);
	for (size_t i = 0; i < n; i++)
		alt__h2_client_release(all[i]);
	if (c->bg)
		alt_sched_stop(c->bg); /* waits for the connections' tasks */
	free(c);
}

/* a usable HTTP/2 connection to key (expired idle ones are dropped) */
static struct alt__h2_conn *take_h2(alt_http_client *c, const char *key) {
	struct alt__h2_conn *found = 0, *drop[32];
	size_t nd = 0;
	alt_time now = alt_now();
	alt_mutex_lock(&c->mu);
	for (size_t i = 0; i < c->nh2;) {
		struct alt__h2_conn *h = c->h2[i];
		bool idle;
		alt_time since = alt__h2_client_idle_since(h, &idle);
		bool usable = alt__h2_client_usable(h);
		if (!usable || (idle && now - since > c->cfg.idle_timeout)) {
			drop[nd++] = h;
			c->h2[i] = c->h2[--c->nh2];
			continue;
		}
		if (!found && strcmp(alt__h2_client_key(h), key) == 0)
			found = h;
		i++;
	}
	alt_mutex_unlock(&c->mu);
	for (size_t i = 0; i < nd; i++)
		alt__h2_client_release(drop[i]);
	return found;
}

static void drop_h2(alt_http_client *c, struct alt__h2_conn *h) {
	bool had = false;
	alt_mutex_lock(&c->mu);
	for (size_t i = 0; i < c->nh2; i++)
		if (c->h2[i] == h) {
			c->h2[i] = c->h2[--c->nh2];
			had = true;
			break;
		}
	alt_mutex_unlock(&c->mu);
	if (had)
		alt__h2_client_release(h);
}

/* a new HTTP/2 connection over a connected (and, for https, TLS) connection */
static int start_h2(alt_http_client *c, alt_conn *nc, alt_tls_conn *tls, const char *key, struct alt__h2_conn **out) {
	alt_mutex_lock(&c->mu);
	int rc = 0;
	if (!c->bg)
		rc = alt_sched_start(&c->bg);
	alt_mutex_unlock(&c->mu);
	if (rc == 0)
		rc = alt__h2_client_new(out, nc, tls, c->bg, key);
	if (rc < 0) {
		if (tls)
			alt_tls_close(tls);
		else
			alt_conn_close(nc);
		return rc;
	}
	alt_mutex_lock(&c->mu);
	bool kept = c->nh2 < 32;
	if (kept)
		c->h2[c->nh2++] = *out;
	alt_mutex_unlock(&c->mu);
	if (!kept) {
		/* the pool is full: this one serves the request, then goes */
		alt__h2_client_release(*out);
		return -ENOMEM;
	}
	return 0;
}

/* an idle connection that the server has not closed meanwhile */
static bool alive(pconn *pc) {
	char b;
	ssize_t k = recv(alt_conn_fd(pc->c), &b, 1, MSG_PEEK | MSG_DONTWAIT);
	return k < 0 && (errno == EAGAIN || errno == EWOULDBLOCK);
}

static pconn *take_idle(alt_http_client *c, const char *key) {
	alt_time now = alt_now();
	pconn *found = 0, *dead = 0;
	alt_mutex_lock(&c->mu);
	for (pconn **pp = &c->idle; *pp;) {
		pconn *pc = *pp;
		bool expired = now - pc->idle_since > c->cfg.idle_timeout;
		if (expired || (!found && strcmp(pc->key, key) == 0)) {
			*pp = pc->next;
			if (expired) {
				pc->next = dead;
				dead = pc;
			} else
				found = pc;
			continue;
		}
		pp = &pc->next;
	}
	alt_mutex_unlock(&c->mu);
	while (dead) {
		pconn *n = dead->next;
		pconn_close(dead);
		dead = n;
	}
	if (found && !alive(found)) {
		pconn_close(found);
		return 0;
	}
	return found;
}

static void put_idle(alt_http_client *c, pconn *pc) {
	pc->idle_since = alt_now();
	alt_conn_set_deadline(pc->c, ALT_FOREVER);
	alt__conn_set_ctx(pc->c, 0);
	int same = 0;
	alt_mutex_lock(&c->mu);
	for (pconn *p = c->idle; p; p = p->next)
		same += strcmp(p->key, pc->key) == 0;
	bool keep = same < c->cfg.max_idle_per_host;
	if (keep) {
		pc->next = c->idle;
		c->idle = pc;
	}
	alt_mutex_unlock(&c->mu);
	if (!keep)
		pconn_close(pc);
}

/* key: "http:host:port" or "https:host:port"; hostname for TLS; an
   HTTP/2 connection (ALPN or h2c) comes in *h2 instead */
static int dial(alt_http_client *c, const char *key, const char *hostname, alt_ctx *ctx, pconn **out, struct alt__h2_conn **h2) {
	*h2 = 0;
	pconn *pc = calloc(1, sizeof *pc);
	if (!pc)
		return -ENOMEM;
	snprintf(pc->key, sizeof pc->key, "%s", key);
	bool https = strncmp(key, "https:", 6) == 0;
	const char *addr = strchr(key, ':') + 1;
	alt_ctx *dctx;
	int rc = alt_ctx_with_timeout(&dctx, ctx, c->cfg.dial_timeout);
	if (rc == 0) {
		rc = alt_dial(&pc->c, "tcp", addr, dctx);
		if (rc == 0 && https) {
			static const char *const h1[] = {"http/1.1"}, *const h2s[] = {"h2", "http/1.1"};
			alt_tls_config tc = {.alpn = c->cfg.disable_http2 ? h1 : h2s, .nalpn = c->cfg.disable_http2 ? 1 : 2,
				.server_name = hostname, .roots = c->cfg.tls_roots, .insecure_skip_verify = c->cfg.tls_insecure_skip_verify};
			if ((rc = alt_tls_client(&pc->tls, pc->c, &tc)) < 0)
				alt_conn_close(pc->c);
			else {
				alt_time dl = alt_ctx_deadline(dctx);
				if (dl)
					alt_conn_set_deadline(pc->c, dl);
				rc = alt_tls_handshake(pc->tls);
				alt_conn_set_deadline(pc->c, ALT_FOREVER);
				if (rc < 0) {
					alt_tls_close(pc->tls);
					pc->tls = 0;
				}
			}
		}
		alt_ctx_free(dctx);
	}
	if (rc < 0) {
		free(pc);
		return rc;
	}
	alt_tls_state ts;
	if (pc->tls)
		alt_tls_connection_state(pc->tls, &ts);
	if ((pc->tls && strcmp(ts.alpn, "h2") == 0) || (!https && c->cfg.h2c && !c->cfg.disable_http2)) {
		rc = start_h2(c, pc->c, pc->tls, key, h2);
		free(pc);
		return rc;
	}
	alt_reader rd = pc->tls ? alt_tls_reader(pc->tls) : alt_conn_reader(pc->c);
	alt_writer wr = pc->tls ? alt_tls_writer(pc->tls) : alt_conn_writer(pc->c);
	if ((rc = alt_bufreader_init(&pc->br, rd, 4096)) < 0 || (rc = alt_bufwriter_init(&pc->bw, wr, 4096)) < 0) {
		if (pc->tls)
			alt_tls_close(pc->tls);
		else
			alt_conn_close(pc->c);
		alt_bufreader_free(&pc->br);
		free(pc);
		return rc;
	}
	*out = pc;
	return 0;
}

/* writes the request: 0 or an error */
static int write_request(alt_http_client *c, pconn *pc, const char *method, const alt_url *u, const alt_http_header *extra,
	alt_reader *body, int64_t length, bool gzip) {
	alt_buf b = ALT_BUF_INIT;
	alt_buf_printf(&b, "%s ", method);
	alt_url_request_uri(u, &b);
	alt_buf_append_cstr(&b, " HTTP/1.1\r\nHost: ");
	alt_buf_append_str(&b, u->host);
	alt_buf_append_cstr(&b, "\r\n");
	bool ua = false;
	if (extra)
		for (size_t i = 0; i < extra->n; i++) {
			alt_str k = extra->f[i].key;
			if (alt_str_eq_fold(k, ALT_STR("Host")) || alt_str_eq_fold(k, ALT_STR("Content-Length")) ||
				alt_str_eq_fold(k, ALT_STR("Transfer-Encoding")))
				continue;
			ua |= alt_str_eq_fold(k, ALT_STR("User-Agent"));
			alt_buf_append_str(&b, k);
			alt_buf_append_cstr(&b, ": ");
			alt_buf_append_str(&b, extra->f[i].value);
			alt_buf_append_cstr(&b, "\r\n");
		}
	if (!ua)
		alt_buf_printf(&b, "User-Agent: %s\r\n", c->cfg.user_agent);
	if (gzip)
		alt_buf_append_cstr(&b, "Accept-Encoding: gzip\r\n");
	bool chunked = body && length < 0;
	if (chunked)
		alt_buf_append_cstr(&b, "Transfer-Encoding: chunked\r\n");
	else if (body || strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0 || strcmp(method, "PATCH") == 0)
		alt_buf_printf(&b, "Content-Length: %lld\r\n", (long long)(body ? length : 0));
	alt_buf_append_cstr(&b, "\r\n");
	int rc = b.err ? b.err : (int)alt_bufwriter_write(&pc->bw, b.p, b.n);
	alt_buf_free(&b);
	if (rc < 0)
		return rc;
	if (body) {
		char buf[16384];
		int64_t sent = 0;
		for (;;) {
			size_t want = sizeof buf;
			if (!chunked && length - sent < (int64_t)want)
				want = (size_t)(length - sent);
			if (!chunked && want == 0)
				break;
			ssize_t k = alt_read(*body, buf, want);
			if (k < 0)
				return (int)k;
			if (k == 0) {
				if (!chunked && sent < length)
					return -ALT_EUNEXPECTED_EOF;
				break;
			}
			if (chunked) {
				char hdr[24];
				int hn = snprintf(hdr, sizeof hdr, "%zx\r\n", (size_t)k);
				if ((rc = (int)alt_bufwriter_write(&pc->bw, hdr, (size_t)hn)) < 0 || (rc = (int)alt_bufwriter_write(&pc->bw, buf, (size_t)k)) < 0 ||
					(rc = (int)alt_bufwriter_write(&pc->bw, "\r\n", 2)) < 0)
					return rc;
			} else if ((rc = (int)alt_bufwriter_write(&pc->bw, buf, (size_t)k)) < 0)
				return rc;
			sent += k;
		}
		if (chunked && (rc = (int)alt_bufwriter_write(&pc->bw, "0\r\n\r\n", 5)) < 0)
			return rc;
	}
	return alt_bufwriter_flush(&pc->bw);
}

/* the body of a gzip response: the decompressor starts at the first read
   (an empty body is an empty one, as in Go) */
static ssize_t gunzip_read(void *obj, void *buf, size_t n) {
	resp_state *st = obj;
	if (st->gz_err)
		return st->gz_err == 1 ? 0 : st->gz_err;
	if (!st->gz) {
		int rc = alt_gzip_reader_new(&st->gz, st->raw);
		if (rc < 0) {
			st->gz = 0;
			st->gz_err = rc == -ALT_EEOF ? 1 : rc;
			return rc == -ALT_EEOF ? 0 : rc;
		}
	}
	return alt_gzip_read(st->gz, buf, n);
}

static bool is_redirect(int code) { return code == 301 || code == 302 || code == 303 || code == 307 || code == 308; }

int alt_http_client_do(alt_http_client *c, const alt_http_client_request *req, alt_http_response **out, alt_ctx *ctx) {
	*out = 0;
	resp_state *st = calloc(1, sizeof *st);
	if (!st)
		return -ENOMEM;
	st->client = c;
	alt_arena_init(&st->arena, 8192);
	int rc = 0;
	if (c->cfg.timeout)
		rc = alt_ctx_with_timeout(&st->ctx, ctx, c->cfg.timeout);
	else
		rc = alt_ctx_new(&st->ctx, ctx);
	if (rc < 0) {
		free(st);
		return rc;
	}
	alt_url u;
	const char *method = req->method ? req->method : "GET";
	if (alt_url_parse(&st->arena, alt_str_cstr(req->url), &u) < 0) {
		rc = -ALT_EURL_PARSE;
		goto fail;
	}
	alt_reader *body = req->body;
	int64_t length = req->content_length;
	alt_str v;
	bool gzip = !c->cfg.disable_compression && strcmp(method, "HEAD") != 0 &&
		       !(req->header && alt_http_header_get(req->header, ALT_STR("Accept-Encoding"), &v) && v.n) &&
		       !(req->header && alt_http_header_get(req->header, ALT_STR("Range"), &v) && v.n);
	for (int redirects = 0;; redirects++) {
		bool https = alt_str_eq_cstr(u.scheme, "https");
		if (!https && !alt_str_eq_cstr(u.scheme, "http")) {
			rc = -ALT_EHTTP_SCHEME;
			goto fail;
		}
		if (u.host.n == 0) {
			rc = -ALT_EURL_HOST;
			goto fail;
		}
		alt_str hostname = alt_url_hostname(&u), port = alt_url_port(&u);
		char key[300];
		alt_buf kb = ALT_BUF_INIT;
		alt_buf_append_cstr(&kb, https ? "https:" : "http:");
		alt_join_host_port(&kb, hostname, port.n ? port : https ? ALT_STR("443") : ALT_STR("80"));
		snprintf(key, sizeof key, "%s", alt_buf_cstr(&kb));
		char hn[256];
		snprintf(hn, sizeof hn, "%.*s", (int)hostname.n, hostname.p);
		alt_buf_free(&kb);
		alt_time deadline = alt_ctx_deadline(st->ctx);
		alt_http_response *resp = alt_arena_zalloc(&st->arena, sizeof *resp);
		if (!resp) {
			rc = -ENOMEM;
			goto fail;
		}
		/* a reused connection may turn out dead: then once more on a new
		   one (only without a body, which cannot be sent again) */
		/* the jar's cookies join the request's header (Go's Client.send) */
		const alt_http_header *hdr = req->header;
		alt_http_header *withc = 0;
		if (c->cfg.jar) {
			alt_buf cb = ALT_BUF_INIT;
			alt_str have;
			if (req->header && alt_http_header_get(req->header, ALT_STR("Cookie"), &have) && have.n)
				alt_buf_append_str(&cb, have);
			alt_cookie_jar_header(c->cfg.jar, &u, &cb, 0);
			if (cb.n) {
				withc = alt_arena_zalloc(&st->arena, sizeof *withc);
				if (withc) {
					alt_http_header_init(withc, &st->arena);
					for (size_t i = 0; req->header && i < req->header->n; i++)
						if (!alt_str_eq_fold(req->header->f[i].key, ALT_STR("Cookie")))
							alt_http_header_add(withc, req->header->f[i].key, req->header->f[i].value);
					alt_http_header_set(withc, ALT_STR("Cookie"), alt_buf_str(&cb));
					hdr = withc;
				}
			}
			alt_buf_free(&cb);
		}
		pconn *pc = 0;
		struct alt__h2_stream *h2s = 0;
		for (int attempt = 0;; attempt++) {
			struct alt__h2_conn *h2 = take_h2(c, key);
			pc = h2 ? 0 : take_idle(c, key);
			bool reused = pc != 0 || h2 != 0;
			if (!pc && !h2 && (rc = dial(c, key, hn, st->ctx, &pc, &h2)) < 0)
				goto fail;
			if (h2) {
				bool retry;
				rc = alt__h2_round_trip(h2, method, &u, hdr, body, body ? length : 0, gzip, c->cfg.user_agent, &st->arena,
					resp, st->ctx, &h2s, &retry);
				if (rc < 0) {
					if (retry || !alt__h2_client_usable(h2))
						drop_h2(c, h2);
					/* not processed by the server (GOAWAY, REFUSED_STREAM): once more, without a body to resend */
					if (retry && attempt < 2 && !body && !alt_ctx_err(st->ctx))
						continue;
					goto fail;
				}
				break;
			}
			alt_conn_set_deadline(pc->c, deadline);
			alt__conn_set_ctx(pc->c, st->ctx);
			rc = alt_ctx_err(st->ctx);
			if (rc == 0)
				rc = write_request(c, pc, method, &u, hdr, body, length, gzip);
			if (rc == 0) {
				do
					rc = alt_http_read_response(&pc->br, &st->arena, c->cfg.max_header_bytes, method, resp);
				while (rc == 0 && resp->status >= 100 && resp->status <= 199 && resp->status != 101);
			}
			if (rc < 0 && reused && attempt == 0 && !body && rc != -ETIMEDOUT && rc != -ECANCELED && !alt_ctx_err(st->ctx)) {
				pconn_close(pc);
				pc = 0;
				continue;
			}
			break;
		}
		if (rc < 0) {
			pconn_close(pc);
			goto fail;
		}
		st->pc = pc;
		st->h2s = h2s;
		resp->url = u;
		if (c->cfg.jar) {
			/* the response's cookies go to the jar */
			alt_set_cookie sc[64];
			size_t nsc = 0;
			for (size_t i = 0; i < resp->header.n && nsc < 64; i++)
				if (alt_str_eq_fold(resp->header.f[i].key, ALT_STR("Set-Cookie")) &&
					alt_http_parse_set_cookie(&st->arena, resp->header.f[i].value, &sc[nsc], 0) == 0)
					nsc++;
			alt_cookie_jar_set(c->cfg.jar, &u, sc, nsc, 0);
		}
		alt_str loc;
		bool follow = is_redirect(resp->status) && c->cfg.max_redirects > 0 &&
			alt_http_header_get(&resp->header, ALT_STR("Location"), &loc) && loc.n;
		if (follow && (resp->status == 307 || resp->status == 308) && body)
			follow = false; /* the body cannot be sent again */
		if (!follow) {
			resp->internal = st;
			st->raw = resp->body;
			/* as Go: only responses that may have a body */
			if (gzip && resp->content_length != 0 && alt_str_eq_fold(alt_http_header_value(&resp->header, "Content-Encoding"), ALT_STR("gzip"))) {
				resp->body = (alt_reader){gunzip_read, st};
				alt_http_header_del(&resp->header, ALT_STR("Content-Encoding"));
				alt_http_header_del(&resp->header, ALT_STR("Content-Length"));
				resp->content_length = -1;
				resp->uncompressed = true;
			}
			*out = resp;
			return 0;
		}
		if (redirects + 1 > c->cfg.max_redirects) {
			rc = -ALT_EHTTP_REDIRECTS;
			goto fail;
		}
		/* drain a small body to reuse the connection */
		alt_limit_reader lim;
		ssize_t k = alt_copy(alt_discard, alt_limit_reader_init(&lim, resp->body, 2 << 10));
		if (h2s) {
			alt__h2_stream_done(h2s); /* cancels what is left */
			st->h2s = 0;
		} else {
			alt__http_body *rb = resp->internal;
			if (k >= 0 && rb->done && !resp->close)
				put_idle(c, pc);
			else
				pconn_close(pc);
		}
		st->pc = 0;
		alt_url next, ref;
		if (alt_url_parse(&st->arena, loc, &ref) < 0 || alt_url_resolve(&st->arena, &u, &ref, &next) < 0) {
			rc = -ALT_EHTTP_BADRESPONSE;
			goto fail;
		}
		u = next;
		if (resp->status == 301 || resp->status == 302 || resp->status == 303) {
			if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0)
				method = "GET";
			body = 0;
			length = 0;
		}
	}
fail:
	if (st->pc)
		pconn_close(st->pc);
	if (st->h2s)
		alt__h2_stream_done(st->h2s);
	alt_ctx_free(st->ctx);
	alt_arena_free(&st->arena);
	free(st);
	return rc;
}

void alt_http_response_free(alt_http_response *r) {
	if (!r)
		return;
	resp_state *st = r->internal;
	if (!st)
		return;
	if (st->h2s)
		alt__h2_stream_done(st->h2s);
	if (st->pc) {
		/* the body was read to its end: the connection can serve again */
		const alt__http_body *b = st->raw.obj;
		bool reuse = !r->close && b && b->done && !b->err;
		if (reuse)
			put_idle(st->client, st->pc);
		else
			pconn_close(st->pc);
	}
	alt_gzip_reader_free(st->gz);
	alt_ctx_free(st->ctx);
	alt_arena_free(&st->arena); /* r itself lives there */
	free(st);
}

int alt_http_response_body(alt_http_response *r, size_t max, alt_str *body) {
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

int alt_http_get(alt_http_client *c, const char *url, alt_http_response **resp, alt_ctx *ctx) {
	alt_http_client_request req = {"GET", url, 0, 0, 0};
	return alt_http_client_do(c, &req, resp, ctx);
}

int alt_http_post(alt_http_client *c, const char *url, const char *content_type, alt_str body, alt_http_response **resp, alt_ctx *ctx) {
	alt_arena a;
	alt_arena_init(&a, 512);
	alt_http_header h;
	alt_http_header_init(&h, &a);
	alt_http_header_set_cstr(&h, "Content-Type", content_type);
	/* with HTTP/2 the body is read while the response comes: it lives with the response */
	struct {
		alt_str_reader sr;
		alt_reader rd;
		char data[];
	} *pb = malloc(sizeof *pb + body.n + 1);
	if (!pb) {
		alt_arena_free(&a);
		return -ENOMEM;
	}
	memcpy(pb->data, body.p, body.n);
	pb->rd = alt_str_reader_init(&pb->sr, alt_str_make(pb->data, body.n));
	alt_http_client_request req = {"POST", url, &h, &pb->rd, (int64_t)body.n};
	int rc = alt_http_client_do(c, &req, resp, ctx);
	alt_arena_free(&a);
	if (rc < 0 || alt_arena_defer((*resp)->arena, free, pb) < 0) {
		if (rc == 0) {
			alt_http_response_free(*resp);
			*resp = 0;
			rc = -ENOMEM;
		}
		free(pb);
	}
	return rc;
}
