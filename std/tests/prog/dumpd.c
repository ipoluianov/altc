/* A server that answers every request with what it parsed (for
   tests/stdhttp_test.go, compared with Go's server): prints
   "listening ADDR", serves until stdin ends */
#include <alt/net/http.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void hex(alt_buf *b, alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		alt_buf_printf(b, "%02x", (unsigned char)s.p[i]);
}

static void dump(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_buf b = ALT_BUF_INIT;
	alt_buf_printf(&b, "%.*s|%.*s|%.*s|%.*s|%lld|%d|%d|", (int)r->method.n, r->method.p, (int)r->request_uri.n, r->request_uri.p,
		(int)r->proto.n, r->proto.p, (int)r->host.n, r->host.p, (long long)r->content_length, r->chunked, r->close);
	/* the header without Host, sorted */
	alt_http_header h;
	alt_http_header_init(&h, r->arena);
	for (size_t i = 0; i < r->header.n; i++)
		if (!alt_str_eq_cstr(r->header.f[i].key, "Host"))
			alt_http_header_add(&h, r->header.f[i].key, r->header.f[i].value);
	alt_buf hb = ALT_BUF_INIT;
	alt_http_header_write(&h, &hb);
	hex(&b, alt_buf_str(&hb));
	alt_buf_free(&hb);
	alt_buf_append_byte(&b, '|');
	alt_buf body = ALT_BUF_INIT;
	ssize_t n = alt_read_all(r->body, &body, 1 << 20);
	if (n < 0)
		alt_buf_append_cstr(&b, "bodyerr");
	else
		hex(&b, alt_buf_str(&body));
	alt_buf_free(&body);
	alt_buf_append_byte(&b, '|');
	alt_http_header tr;
	alt_http_header_init(&tr, r->arena);
	for (size_t i = 0; i < r->trailer.n; i++)
		alt_http_header_add(&tr, r->trailer.f[i].key, r->trailer.f[i].value);
	alt_buf tb = ALT_BUF_INIT;
	alt_http_header_write(&tr, &tb);
	hex(&b, alt_buf_str(&tb));
	alt_buf_free(&tb);
	alt_http_header_set_cstr(alt_http_rw_header(w), "Content-Type", "text/plain");
	alt_http_rw_write(w, b.p, b.n);
	alt_buf_free(&b);
}

static alt_http_server *server;

static void *watch(void *arg) {
	(void)arg;
	char buf[64];
	while (read(0, buf, sizeof buf) > 0) {
	}
	alt_http_server_close(server);
	return 0;
}

int main(void) {
	alt_http_server_config cfg = {0};
	cfg.handler = (alt_http_handler){dump, 0};
	cfg.max_header_bytes = 8192;
	if (alt_http_server_new(&server, &cfg) < 0)
		return 1;
	alt_listener *l;
	if (alt_listen(&l, "tcp", "127.0.0.1:0") < 0)
		return 1;
	alt_buf a = ALT_BUF_INIT;
	alt_addr_format(alt_listener_addr(l), &a);
	printf("listening %s\n", alt_buf_cstr(&a));
	fflush(stdout);
	pthread_t th;
	pthread_create(&th, 0, watch, 0);
	alt_http_server_serve(server, l);
	pthread_join(th, 0);
	alt_http_server_free(server);
	return 0;
}
