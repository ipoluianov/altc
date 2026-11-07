/* An HTTPS server of the library for tests/stdtls_test.go:
	httpsd cert.pem key.pem
   prints "listening <addr>"; answers every request with
	<method> <path> <proto> <tls suite> <alpn> <body length>
   and stops when stdin closes */
#include <alt/crypto/tls.h>
#include <alt/net/http.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static alt_http_server *srv;

static void handle(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str body;
	alt_http_read_body(r, 10 << 20, &body);
	alt_tls_state st = {0};
	if (r->tls)
		alt_tls_connection_state(r->tls, &st);
	alt_http_rw_printf(w, "%.*s %.*s %.*s %04x %s %zu\n", (int)r->method.n, r->method.p, (int)r->url.path.n, r->url.path.p,
		(int)r->proto.n, r->proto.p, st.cipher_suite, st.alpn && st.alpn[0] ? st.alpn : "-", body.n);
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
	if (argc < 3)
		return 2;
	alt_tls_cert *cert;
	const char *why = "";
	if (alt_tls_load_key_pair(&cert, argv[1], argv[2], &why) < 0) {
		fprintf(stderr, "%s\n", why);
		return 1;
	}
	const alt_tls_cert *certs[1] = {cert};
	alt_tls_config tc = {.certs = certs, .ncerts = 1};
	alt_http_server_config cfg = {.handler = {handle, 0}, .tls = &tc, .threads = 2};
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
	alt_http_server_serve(srv, ln);
	return 0;
}
