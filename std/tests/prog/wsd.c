/* A WebSocket server of the library for tests/stdwebsocket_test.go:
	wsd [-tls cert key] [-compress] [-snct] [-cnct] [-limit N] [-frame N]
	    [-protos a,b] [-anyorigin] [-level N]
   prints "listening <addr>", then "end <path> <rc> <code> <reason>"
   when a connection ends (the error that ended reading, the peer's close
   code). Paths:
	/echo       every message back with its type (whole messages)
	/stream     every message back, read and written in pieces of 1000 bytes
	/close      a close frame with ?code= and ?reason= first, then echo
	/ping       a ping "srv-ping" first; a pong is answered with the text
	            "pong:<payload>"; then echo
	/info       the text "proto=<p> ext=<e>" first, then echo
	/hdr        an upgrade with response headers (X-Ws, Set-Cookie), then echo
	/drop       reads one message, then closes the connection without a close frame
	/upgrade    a plain HTTP request: "upgrade=<alt_ws_is_upgrade>"
   stops when stdin closes */
#include <alt/crypto/tls.h>
#include <alt/net/http.h>
#include <alt/net/websocket.h>
#include <alt/task.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static alt_http_server *srv;
static alt_ws_upgrader up;
static pthread_mutex_t out_mu = PTHREAD_MUTEX_INITIALIZER;

static const char *errname(int rc) {
	static char buf[32];
	switch (rc) {
	case 0: return "ok";
	case -EPROTO: return "EPROTO";
	case -EILSEQ: return "EILSEQ";
	case -EMSGSIZE: return "EMSGSIZE";
	case -ETIMEDOUT: return "ETIMEDOUT";
	case -EINVAL: return "EINVAL";
	case -ECONNRESET: return "ECONNRESET";
	case -EPIPE: return "EPIPE";
	case -ALT_ECLOSED: return "ECLOSED";
	case -ALT_EUNEXPECTED_EOF: return "EUNEXPECTED_EOF";
	case -ALT_EFLATE_CORRUPT: return "ECORRUPT";
	}
	snprintf(buf, sizeof buf, "E%d", rc);
	return buf;
}

static bool any_origin(const alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	return true;
}

static int on_pong(alt_ws_conn *c, alt_str p, void *arg) {
	(void)arg;
	char m[160];
	int n = snprintf(m, sizeof m, "pong:%.*s", (int)p.n, p.p);
	return alt_ws_write_message(c, ALT_WS_TEXT, m, (size_t)n);
}

static void end(alt_ws_conn *c, const char *path, int rc) {
	alt_str reason;
	int code = alt_ws_close_code(c, &reason);
	pthread_mutex_lock(&out_mu);
	printf("end %s %s %d %.*s\n", path, errname(rc), code, (int)reason.n, reason.p);
	fflush(stdout);
	pthread_mutex_unlock(&out_mu);
	alt_ws_close(c, ALT_WS_CLOSE_NORMAL, "", 2 * ALT_SEC);
}

static void echo(alt_ws_conn *c, const char *path) {
	alt_buf m = ALT_BUF_INIT;
	int type, rc;
	while ((rc = alt_ws_read_message(c, &type, &m)) == 0)
		if ((rc = alt_ws_write_message(c, type, m.p, m.n)) < 0)
			break;
	alt_buf_free(&m);
	end(c, path, rc);
}

static void stream(alt_ws_conn *c) {
	int type, rc;
	alt_reader r;
	while ((rc = alt_ws_next_reader(c, &type, &r)) == 0) {
		alt_writer w;
		if ((rc = alt_ws_next_writer(c, type, &w)) < 0)
			break;
		char buf[1000];
		ssize_t k;
		while ((k = alt_read(r, buf, sizeof buf)) > 0)
			if ((rc = (int)alt_write(w, buf, (size_t)k)) < 0)
				break;
		if (k < 0) {
			rc = (int)k;
			break;
		}
		if ((rc = alt_ws_close_writer(c)) < 0)
			break;
	}
	end(c, "/stream", rc);
}

static void handle(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	char path[64];
	snprintf(path, sizeof path, "%.*s", (int)r->url.path.n, r->url.path.p);
	if (!strcmp(path, "/upgrade")) {
		alt_http_rw_printf(w, "upgrade=%d\n", alt_ws_is_upgrade(r));
		return;
	}
	alt_http_header *rh = 0, hdr;
	if (!strcmp(path, "/hdr")) {
		alt_http_header_init(&hdr, r->arena);
		alt_http_header_add_cstr(&hdr, "X-Ws", "yes");
		alt_http_header_add_cstr(&hdr, "Set-Cookie", "id=1");
		rh = &hdr;
	}
	alt_ws_conn *c;
	if (alt_ws_upgrade(&up, w, r, rh, &c) < 0)
		return;
	if (!strcmp(path, "/stream")) {
		stream(c);
		return;
	}
	if (!strcmp(path, "/close")) {
		int code;
		alt_str v = alt_http_query_value(r, "code");
		char tmp[16];
		snprintf(tmp, sizeof tmp, "%.*s", (int)v.n, v.p);
		code = atoi(tmp);
		alt_str reason = alt_http_query_value(r, "reason");
		char rb[128];
		snprintf(rb, sizeof rb, "%.*s", (int)reason.n, reason.p);
		/* the close handshake started here: wait for the answer */
		int rc = alt_ws_write_close(c, code, rb, alt_deadline(ALT_SEC));
		int type;
		alt_buf m = ALT_BUF_INIT;
		if (rc == 0)
			while ((rc = alt_ws_read_message(c, &type, &m)) == 0) {
			}
		alt_buf_free(&m);
		end(c, "/close", rc);
		return;
	}
	if (!strcmp(path, "/ping")) {
		alt_ws_set_pong_handler(c, on_pong, 0);
		alt_ws_ping(c, "srv-ping", 8, alt_deadline(ALT_SEC));
	}
	if (!strcmp(path, "/info")) {
		char m[512];
		int n = snprintf(m, sizeof m, "proto=%s ext=%s", alt_ws_subprotocol(c), alt_ws_extensions(c));
		alt_ws_write_message(c, ALT_WS_TEXT, m, (size_t)n);
	}
	if (!strcmp(path, "/drop")) {
		int type;
		alt_buf m = ALT_BUF_INIT;
		alt_ws_read_message(c, &type, &m);
		alt_buf_free(&m);
		alt_ws_free(c);
		return;
	}
	echo(c, path);
}

static void *watcher(void *arg) {
	(void)arg;
	char b[64];
	while (read(0, b, sizeof b) > 0) {
	}
	alt_http_server_close(srv);
	return 0;
}

int main(int argc, char **argv) {
	alt_http_server_config cfg = {.handler = {handle, 0}, .threads = 2};
	static alt_tls_config tc;
	static const alt_tls_cert *certs[1];
	static const char *protos[8];
	up.read_limit = 64 << 20;
	for (int i = 1; i < argc; i++) {
		const char *a = argv[i];
		if (!strcmp(a, "-tls") && i + 2 < argc) {
			alt_tls_cert *cert;
			const char *why = "";
			if (alt_tls_load_key_pair(&cert, argv[i + 1], argv[i + 2], &why) < 0) {
				fprintf(stderr, "%s\n", why);
				return 1;
			}
			i += 2;
			certs[0] = cert;
			tc.certs = certs;
			tc.ncerts = 1;
			cfg.tls = &tc;
			cfg.disable_http2 = true;
		} else if (!strcmp(a, "-compress"))
			up.enable_compression = true;
		else if (!strcmp(a, "-snct"))
			up.server_no_context_takeover = true;
		else if (!strcmp(a, "-cnct"))
			up.client_no_context_takeover = true;
		else if (!strcmp(a, "-anyorigin"))
			up.check_origin = any_origin;
		else if (!strcmp(a, "-limit") && i + 1 < argc)
			up.read_limit = (size_t)atol(argv[++i]);
		else if (!strcmp(a, "-frame") && i + 1 < argc)
			up.write_buffer_size = (size_t)atol(argv[++i]);
		else if (!strcmp(a, "-level") && i + 1 < argc)
			up.compression_level = atoi(argv[++i]);
		else if (!strcmp(a, "-protos") && i + 1 < argc) {
			char *s = strdup(argv[++i]), *tok, *save;
			size_t n = 0;
			for (tok = strtok_r(s, ",", &save); tok && n < 8; tok = strtok_r(0, ",", &save))
				protos[n++] = tok;
			up.subprotocols = protos;
			up.nsubprotocols = n;
		}
	}
	if (alt_http_server_new(&srv, &cfg) < 0)
		return 1;
	alt_listener *ln;
	if (alt_listen(&ln, "tcp", "127.0.0.1:0") < 0)
		return 1;
	alt_buf a = ALT_BUF_INIT;
	alt_addr_format(alt_listener_addr(ln), &a);
	printf("listening %s\n", alt_buf_cstr(&a));
	fflush(stdout);
	pthread_t th;
	pthread_create(&th, 0, watcher, 0);
	int rc = alt_http_server_serve(srv, ln);
	fprintf(stderr, "serve: %s\n", alt_strerror(rc));
	pthread_join(th, 0);
	return 0;
}
