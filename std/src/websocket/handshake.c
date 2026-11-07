/* WebSocket opening handshakes (RFC 6455 4, RFC 7692 5 and 7): the
   server's upgrade of an HTTP request and the client's dial. The checks
   and their order, the error responses and the same-origin default
   follow gorilla/websocket's Upgrader (BSD-2-Clause, Copyright 2013 The
   Gorilla WebSocket Authors: "Redistribution and use in source and
   binary forms, with or without modification, are permitted provided
   that ... retain the above copyright notice, this list of conditions
   and the following disclaimer"); the code is AltC's own. */
#include "ws_internal.h"
#include "../net/net_internal.h"
#include <alt/crypto/hash.h>
#include <alt/encoding/base64.h>
#include <alt/net/url.h>
#include <stdlib.h>
#include <string.h>

static const char GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void alt_ws_accept_key(alt_str key, char out[29]) {
	alt_hash h;
	alt_hash_init(&h, ALT_SHA1);
	alt_hash_write(&h, key.p, key.n);
	alt_hash_write(&h, GUID, sizeof GUID - 1);
	uint8_t sum[20];
	alt_hash_sum(&h, sum);
	alt_buf b = ALT_BUF_INIT;
	alt_base64_encode(&alt_base64_std, &b, sum, sizeof sum);
	memcpy(out, b.n == 28 ? b.p : "", b.n == 28 ? 29 : 1);
	alt_buf_free(&b);
}

/* ---- Sec-WebSocket-Extensions ---- */

static bool tchar(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c && strchr("!#$%&'*+-.^_`|~", c));
}

static void ows(alt_str s, size_t *i) {
	while (*i < s.n && (s.p[*i] == ' ' || s.p[*i] == '\t'))
		(*i)++;
}

static alt_str token(alt_str s, size_t *i) {
	size_t st = *i;
	while (*i < s.n && tchar((unsigned char)s.p[*i]))
		(*i)++;
	return alt_str_make(s.p + st, *i - st);
}

int alt__ws_parse_extensions(const alt_http_header *h, alt_arena *a, alt__ws_ext *out, int max) {
	int n = 0;
	for (size_t f = 0; f < h->n; f++) {
		if (!alt_str_eq_fold(h->f[f].key, ALT_STR("Sec-Websocket-Extensions")))
			continue;
		alt_str s = h->f[f].value;
		size_t i = 0;
		for (;;) {
			ows(s, &i);
			if (i == s.n)
				break;
			if (s.p[i] == ',') { /* empty list elements are allowed (RFC 7230 7) */
				i++;
				continue;
			}
			alt__ws_ext e = {0};
			e.name = token(s, &i);
			if (!e.name.n)
				return -1;
			for (;;) {
				ows(s, &i);
				if (i == s.n || s.p[i] == ',')
					break;
				if (s.p[i] != ';')
					return -1;
				i++;
				ows(s, &i);
				alt__ws_param p = {0};
				p.name = token(s, &i);
				if (!p.name.n)
					return -1;
				ows(s, &i);
				if (i < s.n && s.p[i] == '=') {
					i++;
					ows(s, &i);
					p.has_value = true;
					if (i < s.n && s.p[i] == '"') {
						/* a quoted-string whose content must be a token (RFC 6455 9.1) */
						alt_buf v = ALT_BUF_INIT;
						i++;
						while (i < s.n && s.p[i] != '"') {
							if (s.p[i] == '\\' && i + 1 < s.n)
								i++;
							alt_buf_append_byte(&v, s.p[i++]);
						}
						if (i == s.n) {
							alt_buf_free(&v);
							return -1;
						}
						i++;
						p.value = alt_arena_str(a, alt_buf_str(&v));
						alt_buf_free(&v);
						size_t k = 0;
						alt_str t = token(p.value, &k);
						if (!t.n || k != p.value.n)
							return -1;
					} else if (!(p.value = token(s, &i)).n)
						return -1;
				}
				if (e.nparams == (int)(sizeof e.params / sizeof e.params[0]))
					return -1;
				e.params[e.nparams++] = p;
			}
			if (n == max)
				return -1;
			out[n++] = e;
			if (i < s.n)
				i++; /* ',' */
		}
	}
	return n;
}

int alt__ws_window_bits(alt_str v) {
	if (v.n == 1 && v.p[0] >= '8' && v.p[0] <= '9')
		return v.p[0] - '0';
	if (v.n == 2 && v.p[0] == '1' && v.p[1] >= '0' && v.p[1] <= '5')
		return 10 + v.p[1] - '0';
	return -1;
}

/* the server's answer to one permessage-deflate offer: false when it is
   malformed or asks for what we cannot do */
static bool accept_offer(const alt__ws_ext *e, const alt_ws_upgrader *u, alt_buf *resp, bool *wr_reset, bool *rd_reset) {
	bool snct = false, cnct = false, smwb = false, cmwb = false;
	for (int i = 0; i < e->nparams; i++) {
		const alt__ws_param *p = &e->params[i];
		if (alt_str_eq_fold(p->name, ALT_STR("server_no_context_takeover"))) {
			if (snct || p->has_value)
				return false;
			snct = true;
		} else if (alt_str_eq_fold(p->name, ALT_STR("client_no_context_takeover"))) {
			if (cnct || p->has_value)
				return false;
			cnct = true;
		} else if (alt_str_eq_fold(p->name, ALT_STR("server_max_window_bits"))) {
			/* our compressor's window is always 15 bits */
			if (smwb || !p->has_value || alt__ws_window_bits(p->value) != 15)
				return false;
			smwb = true;
		} else if (alt_str_eq_fold(p->name, ALT_STR("client_max_window_bits"))) {
			if (cmwb || (p->has_value && alt__ws_window_bits(p->value) < 0))
				return false;
			cmwb = true; /* any window decompresses */
		} else
			return false;
	}
	alt_buf_append_cstr(resp, "permessage-deflate");
	*wr_reset = snct || u->server_no_context_takeover;
	if (*wr_reset)
		alt_buf_append_cstr(resp, "; server_no_context_takeover");
	*rd_reset = cnct || u->client_no_context_takeover;
	if (*rd_reset)
		alt_buf_append_cstr(resp, "; client_no_context_takeover");
	if (smwb)
		alt_buf_append_cstr(resp, "; server_max_window_bits=15");
	return true;
}

/* ---- the server ---- */

static int reject(const alt_ws_upgrader *u, alt_http_rw *w, alt_http_request *r, int status, const char *reason) {
	if (u->error)
		u->error(w, r, status, reason, u->error_arg);
	else {
		alt_http_header_set_cstr(alt_http_rw_header(w), "Sec-WebSocket-Version", "13");
		alt_http_error(w, alt_http_status_text(status), status);
	}
	return -ALT_EHTTP_BADREQUEST;
}

static bool same_origin(const alt_http_request *r) {
	alt_str origin;
	if (!alt_http_header_get(&r->header, ALT_STR("Origin"), &origin))
		return true;
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_url u;
	bool ok = alt_url_parse(&a, origin, &u) == 0 && alt_str_eq_fold(u.host, r->host);
	alt_arena_free(&a);
	return ok;
}

static bool valid_key(alt_str key) {
	if (!key.n)
		return false;
	for (size_t i = 0; i < key.n; i++)
		if (key.p[i] == '\r' || key.p[i] == '\n')
			return false;
	alt_buf b = ALT_BUF_INIT;
	bool ok = alt_base64_decode(&alt_base64_std, &b, key) == 0 && b.n == 16;
	alt_buf_free(&b);
	return ok;
}

bool alt_ws_is_upgrade(const alt_http_request *r) {
	return alt_http_header_has_token(&r->header, "Connection", "upgrade") && alt_http_header_has_token(&r->header, "Upgrade", "websocket");
}

size_t alt_ws_request_subprotocols(const alt_http_request *r, alt_str *out, size_t max) {
	size_t n = 0;
	for (size_t i = 0; i < r->header.n; i++) {
		if (!alt_str_eq_fold(r->header.f[i].key, ALT_STR("Sec-Websocket-Protocol")))
			continue;
		alt_str rest = r->header.f[i].value, part;
		while (alt_str_split_next(&rest, ALT_STR(","), &part)) {
			part = alt_str_trim(part, " \t");
			if (part.n && n < max)
				out[n++] = part;
		}
	}
	return n;
}

static char *dup_str(alt_str s) {
	char *p = malloc(s.n + 1);
	if (p) {
		memcpy(p, s.p, s.n);
		p[s.n] = 0;
	}
	return p;
}

int alt_ws_upgrade(const alt_ws_upgrader *u, alt_http_rw *w, alt_http_request *r, const alt_http_header *rh, alt_ws_conn **out) {
	static const alt_ws_upgrader none;
	if (!u)
		u = &none;
	*out = 0;
#define BAD "websocket: the client is not using the websocket protocol: "
	if (!alt_http_header_has_token(&r->header, "Connection", "upgrade"))
		return reject(u, w, r, 400, BAD "'upgrade' token not found in 'Connection' header");
	if (!alt_http_header_has_token(&r->header, "Upgrade", "websocket"))
		return reject(u, w, r, 400, BAD "'websocket' token not found in 'Upgrade' header");
	if (!alt_str_eq_cstr(r->method, "GET"))
		return reject(u, w, r, 405, BAD "request method is not GET");
	if (r->proto_major == 1 && r->proto_minor == 0)
		return reject(u, w, r, 400, BAD "HTTP/1.1 or later is required");
#undef BAD
	if (!alt_http_header_has_token(&r->header, "Sec-Websocket-Version", "13"))
		return reject(u, w, r, 426, "websocket: unsupported version: 13 not found in 'Sec-Websocket-Version' header");
	alt_str v;
	if (rh && alt_http_header_get(rh, ALT_STR("Sec-Websocket-Extensions"), &v))
		return reject(u, w, r, 500, "websocket: application specific 'Sec-WebSocket-Extensions' headers are unsupported");
	if (!(u->check_origin ? u->check_origin(r, u->check_origin_arg) : same_origin(r)))
		return reject(u, w, r, 403, "websocket: request origin not allowed by Upgrader.CheckOrigin");
	alt_str key = alt_str_trim(alt_http_header_value(&r->header, "Sec-Websocket-Key"), " \t");
	if (!valid_key(key))
		return reject(u, w, r, 400,
			"websocket: not a websocket handshake: 'Sec-WebSocket-Key' header must be Base64 encoded value of 16-byte in length");

	/* the subprotocol */
	alt_str proto = ALT_STR_NULL;
	if (u->subprotocols) {
		alt_str asked[32];
		size_t na = alt_ws_request_subprotocols(r, asked, 32);
		for (size_t i = 0; i < u->nsubprotocols && !proto.n; i++)
			for (size_t k = 0; k < na; k++)
				if (alt_str_eq_cstr(asked[k], u->subprotocols[i])) {
					proto = asked[k];
					break;
				}
	} else if (rh)
		proto = alt_http_header_value(rh, "Sec-Websocket-Protocol");

	/* compression: the first offer we can accept */
	alt_buf ext = ALT_BUF_INIT;
	bool compress = false, wr_reset = false, rd_reset = false;
	if (u->enable_compression) {
		alt__ws_ext offers[16];
		int n = alt__ws_parse_extensions(&r->header, r->arena, offers, 16);
		for (int i = 0; i < n && !compress; i++)
			if (alt_str_eq_fold(offers[i].name, ALT_STR("permessage-deflate"))) {
				alt_buf_reset(&ext);
				compress = accept_offer(&offers[i], u, &ext, &wr_reset, &rd_reset);
			}
		if (!compress)
			alt_buf_reset(&ext);
	}

	alt_conn *nc;
	alt_bufreader *br;
	alt_tls_conn *tls = r->tls;
	int rc = alt_http_rw_hijack(w, &nc, &br);
	if (rc < 0) {
		alt_buf_free(&ext);
		reject(u, w, r, 500, rc == -ALT_EHTTP_UNSUPPORTED ? "websocket: the connection cannot be taken over (HTTP/2)" : alt_strerror(rc));
		return -ALT_EHTTP_BADREQUEST;
	}
	alt_conn_set_deadline(nc, ALT_FOREVER);

	char accept[29];
	alt_ws_accept_key(key, accept);
	alt_buf b = ALT_BUF_INIT;
	alt_buf_printf(&b, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n", accept);
	if (proto.n)
		alt_buf_printf(&b, "Sec-WebSocket-Protocol: %.*s\r\n", (int)proto.n, proto.p);
	if (compress)
		alt_buf_printf(&b, "Sec-WebSocket-Extensions: %s\r\n", ext.p);
	for (size_t i = 0; rh && i < rh->n; i++) {
		const alt_http_field *f = &rh->f[i];
		if (alt_str_eq_fold(f->key, ALT_STR("Sec-Websocket-Protocol")))
			continue;
		alt_buf_append_str(&b, f->key);
		alt_buf_append_cstr(&b, ": ");
		for (size_t k = 0; k < f->value.n; k++)
			alt_buf_append_byte(&b, (unsigned char)f->value.p[k] <= 31 ? ' ' : f->value.p[k]);
		alt_buf_append_cstr(&b, "\r\n");
	}
	alt_buf_append_cstr(&b, "\r\n");
	alt_writer wr = tls ? alt_tls_writer(tls) : alt_conn_writer(nc);
	alt_ws_conn *c = 0;
	if (b.err)
		rc = b.err;
	else if ((rc = alt__ws_new(&c, true, br, wr, nc, tls)) == 0) {
		br = 0;
		c->compress = compress;
		c->wr_reset = wr_reset;
		c->rd_reset = rd_reset;
		if (u->compression_level)
			alt_ws_set_compression_level(c, u->compression_level);
		c->read_limit = u->read_limit;
		if (u->write_buffer_size)
			c->frame_size = u->write_buffer_size;
		if ((proto.n && !(c->subprotocol = dup_str(proto))) || (compress && !(c->extensions = dup_str(alt_buf_str(&ext)))))
			rc = -ENOMEM;
	}
	if (rc == 0) {
		if (u->handshake_timeout)
			alt_conn_set_write_deadline(nc, alt_deadline(u->handshake_timeout));
		ssize_t k = alt_write_all(wr, b.p, b.n);
		alt_conn_set_write_deadline(nc, ALT_FOREVER);
		if (k < 0)
			rc = (int)k;
	}
	alt_buf_free(&b);
	alt_buf_free(&ext);
	if (rc < 0) {
		if (c)
			alt_ws_free(c);
		else {
			alt_bufreader_free(br);
			free(br);
			if (tls)
				alt_tls_close(tls);
			else
				alt_conn_close(nc);
		}
		return rc;
	}
	*out = c;
	return 0;
}

/* ---- the client ---- */

typedef struct {
	alt_arena arena;
	alt_http_response r;
	alt_str_reader body;
} ws_resp;

void alt_ws_response_free(alt_http_response *resp) {
	if (!resp)
		return;
	ws_resp *w = (ws_resp *)((char *)resp - offsetof(ws_resp, r));
	alt_arena_free(&w->arena);
	free(w);
}

/* the response's extensions against our offer */
static bool check_extensions(const alt_http_header *h, alt_arena *a, bool offered, bool *compress, bool *rd_reset, bool *wr_reset) {
	alt__ws_ext e[8];
	int n = alt__ws_parse_extensions(h, a, e, 8);
	if (n < 0)
		return false;
	if (n == 0)
		return true;
	if (!offered || n > 1 || !alt_str_eq_fold(e[0].name, ALT_STR("permessage-deflate")))
		return false;
	bool snct = false, cnct = false, smwb = false;
	for (int i = 0; i < e[0].nparams; i++) {
		const alt__ws_param *p = &e[0].params[i];
		if (alt_str_eq_fold(p->name, ALT_STR("server_no_context_takeover"))) {
			if (snct || p->has_value)
				return false;
			snct = true;
		} else if (alt_str_eq_fold(p->name, ALT_STR("client_no_context_takeover"))) {
			if (cnct || p->has_value)
				return false;
			cnct = true;
		} else if (alt_str_eq_fold(p->name, ALT_STR("server_max_window_bits"))) {
			if (smwb || !p->has_value || alt__ws_window_bits(p->value) < 0)
				return false;
			smwb = true; /* any window decompresses */
		} else
			return false; /* client_max_window_bits was not offered */
	}
	*compress = true;
	*rd_reset = snct;
	*wr_reset = *wr_reset || cnct;
	return true;
}

int alt_ws_dial(const alt_ws_dialer *d, const char *url, alt_ctx *ctx, alt_ws_conn **out, alt_http_response **resp_out) {
	static const alt_ws_dialer none;
	if (!d)
		d = &none;
	*out = 0;
	if (resp_out)
		*resp_out = 0;
	ws_resp *wr = calloc(1, sizeof *wr);
	if (!wr)
		return -ENOMEM;
	alt_arena_init(&wr->arena, 0);
	alt_arena *a = &wr->arena;
	alt_conn *nc = 0;
	alt_tls_conn *tls = 0;
	alt_bufreader *br = 0;
	alt_ctx *hctx = 0;
	char *server_name = 0;
	alt_ws_conn *c = 0;
	alt_buf req = ALT_BUF_INIT;
	bool have_resp = false;
	alt_url u;
	int rc = alt_url_parse(a, alt_str_cstr(url), &u);
	if (rc < 0)
		goto out;
	bool secure = alt_str_eq_cstr(u.scheme, "wss");
	if (!secure && !alt_str_eq_cstr(u.scheme, "ws")) {
		rc = -ALT_EHTTP_SCHEME;
		goto out;
	}
	alt_str hostname = alt_url_hostname(&u), port = alt_url_port(&u);
	if (!hostname.n) {
		rc = -ALT_EURL_HOST;
		goto out;
	}
	alt_buf addr = ALT_BUF_INIT;
	alt_join_host_port(&addr, hostname, port.n ? port : alt_str_cstr(secure ? "443" : "80"));
	char *addrs = alt_arena_strdup(a, alt_buf_cstr(&addr));
	alt_buf_free(&addr);

	/* the request */
	char keyb[25];
	{
		uint8_t raw[16];
		if ((rc = alt_crypto_rand(raw, sizeof raw)) < 0)
			goto out;
		alt_buf k = ALT_BUF_INIT;
		alt_base64_encode(&alt_base64_std, &k, raw, sizeof raw);
		memcpy(keyb, k.p, 25);
		alt_buf_free(&k);
	}
	alt_buf target = ALT_BUF_INIT;
	alt_url_request_uri(&u, &target);
	alt_str host = u.host;
	if (d->header) {
		alt_str hv;
		if (alt_http_header_get(d->header, ALT_STR("Host"), &hv))
			host = hv;
	}
	alt_buf_printf(&req, "GET %s HTTP/1.1\r\nHost: %.*s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"
		"Sec-WebSocket-Version: 13\r\n", target.n ? target.p : "/", (int)host.n, host.p, keyb);
	alt_buf_free(&target);
	if (d->nsubprotocols) {
		alt_buf_append_cstr(&req, "Sec-WebSocket-Protocol: ");
		for (size_t i = 0; i < d->nsubprotocols; i++)
			alt_buf_printf(&req, "%s%s", i ? ", " : "", d->subprotocols[i]);
		alt_buf_append_cstr(&req, "\r\n");
	}
	if (d->enable_compression)
		alt_buf_printf(&req, "Sec-WebSocket-Extensions: permessage-deflate%s\r\n",
			d->no_context_takeover ? "; server_no_context_takeover; client_no_context_takeover" : "");
	for (size_t i = 0; d->header && i < d->header->n; i++) {
		const alt_http_field *f = &d->header->f[i];
		static const char *const own[] = {"Upgrade", "Connection", "Sec-Websocket-Key", "Sec-Websocket-Version", "Sec-Websocket-Extensions",
			"Sec-Websocket-Protocol"};
		for (size_t k = 0; k < sizeof own / sizeof own[0]; k++)
			if (alt_str_eq_fold(f->key, alt_str_cstr(own[k]))) {
				rc = -EINVAL;
				goto out;
			}
		if (!alt_http_valid_field_name(f->key) || !alt_http_valid_field_value(f->value)) {
			rc = -EINVAL;
			goto out;
		}
		if (alt_str_eq_fold(f->key, ALT_STR("Host")))
			continue;
		alt_buf_printf(&req, "%.*s: %.*s\r\n", (int)f->key.n, f->key.p, (int)f->value.n, f->value.p);
	}
	alt_buf_append_cstr(&req, "\r\n");
	if (req.err) {
		rc = req.err;
		goto out;
	}

	/* connect, TLS, exchange */
	if ((rc = alt_ctx_with_timeout(&hctx, ctx, d->handshake_timeout ? d->handshake_timeout : 45 * ALT_SEC)) < 0)
		goto out;
	if ((rc = alt_dial(&nc, "tcp", addrs, hctx)) < 0)
		goto out;
	alt__conn_set_ctx(nc, hctx);
	alt_time dl = alt_ctx_deadline(hctx);
	if (dl)
		alt_conn_set_deadline(nc, dl);
	if (secure) {
		alt_tls_config tc = d->tls ? *d->tls : (alt_tls_config){0};
		if (!tc.server_name || !tc.server_name[0]) {
			if (!(server_name = dup_str(hostname))) {
				rc = -ENOMEM;
				goto out;
			}
			tc.server_name = server_name;
		}
		if ((rc = alt_tls_client(&tls, nc, &tc)) < 0)
			goto out;
		if ((rc = alt_tls_handshake(tls)) < 0)
			goto out;
	}
	alt_writer w = tls ? alt_tls_writer(tls) : alt_conn_writer(nc);
	ssize_t k = alt_write_all(w, req.p, req.n);
	if (k < 0) {
		rc = (int)k;
		goto out;
	}
	if (!(br = malloc(sizeof *br)) || (rc = alt_bufreader_init(br, tls ? alt_tls_reader(tls) : alt_conn_reader(nc), 4096)) < 0) {
		free(br);
		br = 0;
		rc = -ENOMEM;
		goto out;
	}
	alt_http_response *resp = &wr->r;
	if ((rc = alt_http_read_response(br, a, d->max_header_bytes, "GET", resp)) < 0) {
		if (rc == -ALT_EEOF)
			rc = -ALT_EUNEXPECTED_EOF;
		goto out;
	}
	have_resp = true;
	char accept[29];
	alt_ws_accept_key(alt_str_cstr(keyb), accept);
	alt_str got;
	bool ok = resp->status == 101 && alt_http_header_has_token(&resp->header, "Upgrade", "websocket") &&
		  alt_http_header_has_token(&resp->header, "Connection", "upgrade") &&
		  alt_http_header_get(&resp->header, ALT_STR("Sec-Websocket-Accept"), &got) && alt_str_eq_cstr(alt_str_trim(got, " \t"), accept);
	if (!ok) {
		/* a bit of the body, for the caller's diagnostics */
		alt_buf body = ALT_BUF_INIT;
		alt_limit_reader lr;
		if (resp->status != 101)
			alt_read_all(alt_limit_reader_init(&lr, resp->body, 1024), &body, 0);
		resp->body = alt_str_reader_init(&wr->body, alt_arena_str(a, alt_buf_str(&body)));
		alt_buf_free(&body);
		resp->internal = 0;
		rc = -ALT_EHTTP_BADRESPONSE;
		goto out;
	}
	resp->body = alt_str_reader_init(&wr->body, ALT_STR(""));
	resp->internal = 0;
	/* the subprotocol must be one offered */
	alt_str proto = alt_str_trim(alt_http_header_value(&resp->header, "Sec-Websocket-Protocol"), " \t");
	if (proto.n) {
		bool found = false;
		for (size_t i = 0; i < d->nsubprotocols && !found; i++)
			found = alt_str_eq_cstr(proto, d->subprotocols[i]);
		if (!found) {
			rc = -ALT_EHTTP_BADRESPONSE;
			goto out;
		}
	}
	bool compress = false, rd_reset = false, wr_reset = d->enable_compression && d->no_context_takeover;
	if (!check_extensions(&resp->header, a, d->enable_compression, &compress, &rd_reset, &wr_reset)) {
		rc = -ALT_EHTTP_BADRESPONSE;
		goto out;
	}
	alt__conn_set_ctx(nc, 0);
	alt_conn_set_deadline(nc, ALT_FOREVER);
	if ((rc = alt__ws_new(&c, false, br, w, nc, tls)) < 0)
		goto out;
	br = 0;
	nc = 0;
	tls = 0;
	c->server_name = server_name;
	server_name = 0;
	c->compress = compress;
	c->rd_reset = rd_reset;
	c->wr_reset = compress && wr_reset;
	if (d->compression_level)
		alt_ws_set_compression_level(c, d->compression_level);
	c->read_limit = d->read_limit;
	if (d->write_buffer_size)
		c->frame_size = d->write_buffer_size;
	alt_str exts = alt_http_header_value(&resp->header, "Sec-Websocket-Extensions");
	if ((proto.n && !(c->subprotocol = dup_str(proto))) || (compress && !(c->extensions = dup_str(exts)))) {
		rc = -ENOMEM;
		goto out;
	}
	*out = c;
	c = 0;
	rc = 0;
out:
	if (c) {
		alt_ws_free(c);
		*out = 0;
	}
	if (br) {
		alt_bufreader_free(br);
		free(br);
	}
	if (tls)
		alt_tls_close(tls);
	else if (nc)
		alt_conn_close(nc);
	free(server_name);
	alt_ctx_free(hctx);
	alt_buf_free(&req);
	if (have_resp && resp_out) {
		*resp_out = &wr->r;
		return rc;
	}
	alt_arena_free(&wr->arena);
	free(wr);
	return rc;
}
