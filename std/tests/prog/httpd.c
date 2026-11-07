/* A test server for tests/stdhttp_test.go: prints "listening ADDR" and
   serves until its standard input ends, then shuts down gracefully.
   Arguments: the root of /static/, and optionally the address. */
#include <alt/chan.h>
#include <alt/net/http.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void hello(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_http_rw_write_cstr(w, "hello, world\n");
}

static void echo(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str body;
	int rc = alt_http_read_body(r, 64 << 20, &body);
	if (rc < 0) {
		alt_http_error(w, alt_strerror(rc), 400);
		return;
	}
	alt_http_header *h = alt_http_rw_header(w);
	alt_http_header_set(h, ALT_STR("X-Method"), r->method);
	char n[32];
	snprintf(n, sizeof n, "%zu", body.n);
	alt_http_header_set_cstr(h, "X-Len", n);
	alt_http_header_set_cstr(h, "X-Chunked", r->chunked ? "1" : "0");
	alt_http_header_set_cstr(h, "Content-Type", "application/octet-stream");
	for (size_t i = 0; i < r->trailer.n; i++)
		alt_http_header_add(h, ALT_STR("X-Trailer"), r->trailer.f[i].value);
	alt_http_rw_write(w, body.p, body.n);
}

static void item(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str id = alt_http_path_value(r, "id");
	alt_http_rw_printf(w, "item %.*s pattern %.*s", (int)id.n, id.p, (int)r->pattern.n, r->pattern.p);
}

static void item_edit(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str id = alt_http_path_value(r, "id");
	alt_http_rw_printf(w, "edit %.*s", (int)id.n, id.p);
}

static void files(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str p = alt_http_path_value(r, "path");
	alt_http_rw_printf(w, "path [%.*s]", (int)p.n, p.p);
}

static void exact_root(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_http_rw_write_cstr(w, "root");
}

static void big(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	long long n = 0;
	alt_str_to_int(alt_http_query_value(r, "n"), 10, &n);
	char buf[1000];
	for (long long i = 0; i < n;) {
		size_t k = 0;
		for (; k < sizeof buf && i < n; k++, i++)
			buf[k] = (char)('a' + i % 26);
		if (alt_http_rw_write(w, buf, k) < 0)
			break;
	}
}

static void sleeper(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	long long ms = 0;
	alt_str_to_int(alt_http_query_value(r, "ms"), 10, &ms);
	alt_sleep(ms * ALT_MS);
	alt_http_rw_printf(w, "slept %lld", ms);
}

static void form(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	const alt_url_values *f;
	int rc = alt_http_parse_form(r, 0, &f);
	if (rc < 0) {
		alt_http_error(w, alt_strerror(rc), 400);
		return;
	}
	for (size_t i = 0; i < f->n; i++)
		alt_http_rw_printf(w, "%.*s=%.*s\n", (int)f->kv[i].key.n, f->kv[i].key.p, (int)f->kv[i].value.n, f->kv[i].value.p);
}

static void stream(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	for (int i = 0; i < 5; i++) {
		alt_http_rw_printf(w, "event %d\n", i);
		alt_http_rw_flush(w);
		alt_sleep(10 * ALT_MS);
	}
}

static void status(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	long long code = 0;
	alt_str_to_int(alt_http_path_value(r, "code"), 10, &code);
	alt_http_rw_write_header(w, (int)code);
	alt_http_rw_write_cstr(w, "body");
}

static void headers(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_buf b = ALT_BUF_INIT;
	alt_http_header_write(&r->header, &b);
	alt_http_rw_printf(w, "%s %.*s %.*s host=%.*s\n", alt_buf_cstr(&b), (int)r->method.n, r->method.p, (int)r->request_uri.n,
		r->request_uri.p, (int)r->host.n, r->host.p);
	alt_buf_free(&b);
	alt_str c;
	if (alt_http_cookie(r, "session", &c))
		alt_http_rw_printf(w, "cookie=%.*s\n", (int)c.n, c.p);
}

static void redir(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_http_redirect(w, r, "/hello", 302);
}

static void cookie(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_http_cookie_spec c = {.name = "session", .value = "abc123", .path = "/", .max_age = 60, .http_only = true, .same_site = "Lax"};
	alt_http_set_cookie(w, &c);
	alt_http_rw_write_cstr(w, "set");
}

static void hijack(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_conn *c;
	alt_bufreader *br;
	if (alt_http_rw_hijack(w, &c, &br) < 0)
		return;
	const char *msg = "HTTP/1.1 200 OK\r\nContent-Length: 8\r\nConnection: close\r\n\r\nhijacked";
	alt_conn_write(c, msg, strlen(msg));
	alt_conn_close(c);
	alt_bufreader_free(br);
	free(br);
}

static void declared(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_http_header_set_cstr(alt_http_rw_header(w), "Content-Length", "10");
	alt_http_rw_write_cstr(w, "0123456789");
}

static void json(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_http_header_set_cstr(alt_http_rw_header(w), "Content-Type", "application/json");
	alt_http_rw_write_cstr(w, "{\"ok\":true}");
}

static void html(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	alt_http_rw_write_cstr(w, "<!DOCTYPE html><html><body>hi</body></html>");
}

static void empty(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)w, (void)r, (void)arg;
}

static alt_http_server *server;

static void *watch_stdin(void *arg) {
	(void)arg;
	char buf[64];
	while (read(0, buf, sizeof buf) > 0) {
	}
	alt_ctx *ctx;
	alt_ctx_with_timeout(&ctx, 0, 5 * ALT_SEC);
	int rc = alt_http_server_shutdown(server, ctx);
	fprintf(stderr, "shutdown: %s\n", rc ? alt_strerror(rc) : "ok");
	alt_ctx_free(ctx);
	return 0;
}

static alt_http_file_server static_files;
static alt_http_strip_prefix strip = {"/static", {0, 0}};

int main(int argc, char **argv) {
	alt_http_mux *m;
	alt_http_mux_new(&m);
	static_files.root = argc > 1 ? argv[1] : ".";
	strip.h = alt_http_file_server_handler(&static_files);
	struct {
		const char *pattern;
		alt_http_handler_fn fn;
	} routes[] = {
		{"GET /hello", hello}, {"/echo", echo}, {"GET /items/{id}", item}, {"POST /items/{id}/edit", item_edit},
		{"GET /files/{path...}", files}, {"GET /{$}", exact_root}, {"GET /big", big}, {"GET /sleep", sleeper},
		{"/form", form}, {"GET /stream", stream}, {"/status/{code}", status}, {"/headers", headers},
		{"GET /redirect", redir}, {"GET /cookie", cookie}, {"GET /hijack", hijack}, {"GET /declared", declared},
		{"GET /json", json}, {"GET /html", html}, {"GET /empty", empty}, {"GET /dir/", files},
	};
	for (size_t i = 0; i < sizeof routes / sizeof *routes; i++) {
		int rc = alt_http_mux_handle_func(m, routes[i].pattern, routes[i].fn, 0);
		if (rc < 0) {
			fprintf(stderr, "%s: %s\n", routes[i].pattern, alt_strerror(rc));
			return 1;
		}
	}
	alt_http_mux_handle(m, "/static/", alt_http_strip_prefix_handler(&strip));
	/* conflicts are refused, as Go's ServeMux panics */
	if (alt_http_mux_handle_func(m, "GET /items/{name}", hello, 0) != -ALT_EHTTP_CONFLICT ||
		alt_http_mux_handle_func(m, "GET /{a}/b", hello, 0) != -ALT_EHTTP_CONFLICT ||
		alt_http_mux_handle_func(m, "GET /x/{a}/{a}", hello, 0) != -ALT_EHTTP_PATTERN ||
		alt_http_mux_handle_func(m, "GET /x/{a...}/y", hello, 0) != -ALT_EHTTP_PATTERN ||
		alt_http_mux_handle_func(m, "GET x", hello, 0) != -ALT_EHTTP_PATTERN) {
		fprintf(stderr, "conflict checks failed\n");
		return 1;
	}
	alt_http_server_config cfg = {0};
	cfg.handler = alt_http_mux_handler(m);
	cfg.addr = argc > 2 ? argv[2] : "127.0.0.1:0";
	cfg.read_header_timeout = 5 * ALT_SEC;
	cfg.idle_timeout = 30 * ALT_SEC;
	cfg.max_header_bytes = 16 << 10;
	if (alt_http_server_new(&server, &cfg) < 0)
		return 1;
	alt_listener *l;
	int rc = alt_listen(&l, "tcp", cfg.addr);
	if (rc < 0) {
		fprintf(stderr, "listen: %s\n", alt_strerror(rc));
		return 1;
	}
	alt_buf a = ALT_BUF_INIT;
	alt_addr_format(alt_listener_addr(l), &a);
	printf("listening %s\n", alt_buf_cstr(&a));
	fflush(stdout);
	alt_buf_free(&a);
	pthread_t th;
	pthread_create(&th, 0, watch_stdin, 0);
	rc = alt_http_server_serve(server, l);
	pthread_join(th, 0);
	alt_http_server_free(server);
	alt_http_mux_free(m);
	return rc == -ALT_ECLOSED ? 0 : 2;
}
