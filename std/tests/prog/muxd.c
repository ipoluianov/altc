/* Registers the patterns read from stdin (one per line, up to an empty
   line) and prints "ok", "conflict" or "invalid" for each, then
   "listening ADDR"; every handler answers "pattern|name=value;..." */
#include <alt/net/http.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void handler(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_str(&b, r->pattern);
	alt_buf_append_byte(&b, '|');
	for (int i = 0; i < r->npath_values; i++)
		alt_buf_printf(&b, "%.*s=%.*s;", (int)r->path_values[i].name.n, r->path_values[i].name.p, (int)r->path_values[i].value.n,
			r->path_values[i].value.p);
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
	alt_http_mux *m;
	alt_http_mux_new(&m);
	char line[512];
	while (fgets(line, sizeof line, stdin)) {
		line[strcspn(line, "\n")] = 0;
		if (!line[0])
			break;
		int rc = alt_http_mux_handle_func(m, line, handler, 0);
		printf("%s\n", rc == 0 ? "ok" : rc == -ALT_EHTTP_CONFLICT ? "conflict" : "invalid");
	}
	alt_http_server_config cfg = {0};
	cfg.handler = alt_http_mux_handler(m);
	alt_http_server_new(&server, &cfg);
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
	alt_http_mux_free(m);
	return 0;
}
