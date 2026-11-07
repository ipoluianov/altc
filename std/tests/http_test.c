/* The HTTP server and client of the library against each other, in one
   process (also run under the sanitizers) */
#include "check.h"
#include <alt/net/http.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

static _Atomic int requests;
static char addr[64];
static alt_http_server *srv;

static void hello(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	atomic_fetch_add(&requests, 1);
	alt_http_header_set_cstr(alt_http_rw_header(w), "X-Remote", r->remote_addr.p);
	alt_http_rw_write_cstr(w, "hello");
}

static void echo(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str body;
	if (alt_http_read_body(r, 0, &body) < 0) {
		alt_http_error(w, "bad body", 400);
		return;
	}
	alt_http_header_set(alt_http_rw_header(w), ALT_STR("X-Method"), r->method);
	alt_http_header_set_cstr(alt_http_rw_header(w), "X-Chunked", r->chunked ? "yes" : "no");
	alt_http_rw_write(w, body.p, body.n);
}

static void redirect(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	long long n = 0;
	alt_str_to_int(alt_http_path_value(r, "n"), 10, &n);
	char to[32];
	if (n > 0)
		snprintf(to, sizeof to, "/r/%lld", n - 1);
	else
		snprintf(to, sizeof to, "/final");
	alt_http_redirect(w, r, to, n % 2 ? 302 : 301);
}

static void see_other(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_http_redirect(w, r, "/echo", 303);
}

static void temporary(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_http_redirect(w, r, "/echo", 307);
}

static void final(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_http_rw_printf(w, "final %.*s", (int)r->method.n, r->method.p);
}

static void slow(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_ctx_wait(r->ctx, alt_deadline(500 * ALT_MS));
	alt_http_rw_write_cstr(w, "slow");
}

static void megabyte(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	char buf[4096];
	for (int i = 0; i < 256; i++) {
		memset(buf, 'a' + i % 26, sizeof buf);
		alt_http_rw_write(w, buf, sizeof buf);
	}
}

static void serve_task(void *arg) {
	alt_listener *l = arg;
	int rc = alt_http_server_serve(srv, l);
	CHECK_ERR(rc, -ALT_ECLOSED);
}

static alt_http_client *cl;

static void url(char *out, size_t n, const char *path) { snprintf(out, n, "http://%s%s", addr, path); }

static void basics(void) {
	char u[128];
	alt_http_response *resp;
	url(u, sizeof u, "/hello");
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK_INT(resp->status, 200);
	CHECK_STR(resp->status_text, "200 OK");
	CHECK_INT(resp->content_length, 5);
	CHECK_STR(alt_http_header_value(&resp->header, "Content-Type"), "text/plain; charset=utf-8");
	alt_str body;
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_STR(body, "hello");
	alt_http_response_free(resp);
	/* HEAD */
	alt_http_client_request head = {"HEAD", u, 0, 0, 0};
	CHECK_OK(alt_http_client_do(cl, &head, &resp, 0));
	CHECK_INT(resp->status, 200);
	CHECK_INT(resp->content_length, 5);
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_INT(body.n, 0);
	alt_http_response_free(resp);
	/* not found */
	url(u, sizeof u, "/missing");
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK_INT(resp->status, 404);
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_STR(body, "404 page not found\n");
	alt_http_response_free(resp);
	/* a big chunked response */
	url(u, sizeof u, "/mb");
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK(resp->chunked);
	CHECK_INT(resp->content_length, -1);
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_INT(body.n, 1 << 20);
	CHECK(body.p[0] == 'a' && body.p[(1 << 20) - 1] == 'a' + 255 % 26);
	alt_http_response_free(resp);
	/* errors */
	CHECK_ERR(alt_http_get(cl, "ftp://example.com/", &resp, 0), -ALT_EHTTP_SCHEME);
	CHECK_ERR(alt_http_get(cl, "::bad", &resp, 0), -ALT_EURL_PARSE);
	CHECK_ERR(alt_http_get(cl, "http://127.0.0.1:1/", &resp, 0), -ECONNREFUSED);
}

static void bodies(void) {
	char u[128];
	url(u, sizeof u, "/echo");
	alt_http_response *resp;
	alt_str body;
	CHECK_OK(alt_http_post(cl, u, "text/plain", ALT_STR("posted data"), &resp, 0));
	CHECK_INT(resp->status, 200);
	CHECK_STR(alt_http_header_value(&resp->header, "X-Method"), "POST");
	CHECK_STR(alt_http_header_value(&resp->header, "X-Chunked"), "no");
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_STR(body, "posted data");
	alt_http_response_free(resp);
	/* a body of unknown length goes chunked */
	char *big = malloc(300000);
	for (int i = 0; i < 300000; i++)
		big[i] = (char)(i % 97);
	alt_str_reader sr;
	alt_reader rd = alt_str_reader_init(&sr, alt_str_make(big, 300000));
	alt_http_client_request req = {"PUT", u, 0, &rd, -1};
	CHECK_OK(alt_http_client_do(cl, &req, &resp, 0));
	CHECK_STR(alt_http_header_value(&resp->header, "X-Chunked"), "yes");
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_INT(body.n, 300000);
	CHECK(body.n == 300000 && memcmp(body.p, big, 300000) == 0);
	alt_http_response_free(resp);
	free(big);
}

static void redirects(void) {
	char u[128];
	alt_http_response *resp;
	alt_str body;
	url(u, sizeof u, "/r/5");
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK_INT(resp->status, 200);
	CHECK_STR(resp->url.path, "/final");
	CHECK_OK(alt_http_response_body(resp, 0, &body));
	CHECK_STR(body, "final GET");
	alt_http_response_free(resp);
	url(u, sizeof u, "/r/20");
	CHECK_ERR(alt_http_get(cl, u, &resp, 0), -ALT_EHTTP_REDIRECTS);
	/* 303: POST becomes GET without a body */
	url(u, sizeof u, "/see-other");
	CHECK_OK(alt_http_post(cl, u, "text/plain", ALT_STR("x"), &resp, 0));
	CHECK_STR(alt_http_header_value(&resp->header, "X-Method"), "GET");
	alt_http_response_free(resp);
	/* 307 with a body that cannot be sent again: the redirect is returned */
	url(u, sizeof u, "/temporary");
	CHECK_OK(alt_http_post(cl, u, "text/plain", ALT_STR("x"), &resp, 0));
	CHECK_INT(resp->status, 307);
	alt_http_response_free(resp);
	/* 307 without a body keeps the method */
	alt_http_client_request del = {"DELETE", u, 0, 0, 0};
	CHECK_OK(alt_http_client_do(cl, &del, &resp, 0));
	CHECK_STR(alt_http_header_value(&resp->header, "X-Method"), "DELETE");
	alt_http_response_free(resp);
}

static void reuse(void) {
	alt_http_client *c;
	alt_http_client_new(&c, 0);
	char u[128];
	url(u, sizeof u, "/hello");
	alt_str first = ALT_STR_NULL;
	bool same = true;
	alt_arena a;
	alt_arena_init(&a, 0);
	for (int i = 0; i < 20; i++) {
		alt_http_response *resp;
		CHECK_OK(alt_http_get(c, u, &resp, 0));
		alt_str remote = alt_arena_str(&a, alt_http_header_value(&resp->header, "X-Remote"));
		if (i == 0)
			first = remote;
		else
			same &= alt_str_eq(first, remote);
		alt_str body;
		alt_http_response_body(resp, 0, &body);
		alt_http_response_free(resp);
	}
	CHECK(same); /* one connection served all */
	alt_arena_free(&a);
	alt_http_client_free(c);
}

static alt_waitgroup wg = ALT_WAITGROUP_INIT;
static _Atomic int ok_count;

static void client_task(void *arg) {
	(void)arg;
	char u[128];
	url(u, sizeof u, "/echo");
	for (int i = 0; i < 10; i++) {
		alt_http_response *resp;
		char msg[32];
		int n = snprintf(msg, sizeof msg, "msg %d", i);
		if (alt_http_post(cl, u, "text/plain", alt_str_make(msg, (size_t)n), &resp, 0) == 0) {
			alt_str body;
			if (alt_http_response_body(resp, 0, &body) == 0 && alt_str_eq(body, alt_str_make(msg, (size_t)n)))
				atomic_fetch_add(&ok_count, 1);
			alt_http_response_free(resp);
		}
	}
	alt_waitgroup_done(&wg);
}

static void clients_main(void *arg) {
	(void)arg;
	for (int i = 0; i < 100; i++)
		alt_go(client_task, 0);
}

static void *thread_client(void *arg) {
	client_task(arg);
	return 0;
}

static void concurrency(void) {
	atomic_store(&ok_count, 0);
	alt_waitgroup_add(&wg, 100 + 4);
	pthread_t th[4];
	for (int i = 0; i < 4; i++)
		pthread_create(&th[i], 0, thread_client, 0);
	CHECK_OK(alt_run(clients_main, 0));
	alt_waitgroup_wait(&wg);
	for (int i = 0; i < 4; i++)
		pthread_join(th[i], 0);
	CHECK_INT(atomic_load(&ok_count), 1040);
}

static void timeouts(void) {
	alt_http_client *c;
	alt_http_client_config cfg = {0};
	cfg.timeout = 100 * ALT_MS;
	alt_http_client_new(&c, &cfg);
	char u[128];
	url(u, sizeof u, "/slow");
	alt_http_response *resp;
	alt_time t0 = alt_now();
	CHECK_ERR(alt_http_get(c, u, &resp, 0), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 100 * ALT_MS && alt_since(t0) < 400 * ALT_MS);
	alt_http_client_free(c);
	/* a cancelled context */
	alt_ctx *ctx;
	alt_ctx_new(&ctx, 0);
	alt_ctx_cancel(ctx);
	CHECK_ERR(alt_http_get(cl, u, &resp, ctx), -ECANCELED);
	alt_ctx_free(ctx);
}

int main(void) {
	alt_http_mux *m;
	alt_http_mux_new(&m);
	alt_http_mux_handle_func(m, "GET /hello", hello, 0);
	alt_http_mux_handle_func(m, "/echo", echo, 0);
	alt_http_mux_handle_func(m, "GET /r/{n}", redirect, 0);
	alt_http_mux_handle_func(m, "/see-other", see_other, 0);
	alt_http_mux_handle_func(m, "/temporary", temporary, 0);
	alt_http_mux_handle_func(m, "/final", final, 0);
	alt_http_mux_handle_func(m, "GET /slow", slow, 0);
	alt_http_mux_handle_func(m, "GET /mb", megabyte, 0);
	alt_http_server_config cfg = {0};
	cfg.handler = alt_http_mux_handler(m);
	cfg.threads = 2;
	CHECK_OK(alt_http_server_new(&srv, &cfg));
	alt_listener *l;
	CHECK_OK(alt_listen(&l, "tcp", "127.0.0.1:0"));
	alt_buf b = ALT_BUF_INIT;
	alt_addr_format(alt_listener_addr(l), &b);
	snprintf(addr, sizeof addr, "%s", alt_buf_cstr(&b));
	alt_buf_free(&b);
	alt_sched *acc;
	alt_sched_start(&acc);
	alt_go_on(acc, serve_task, l, 0);
	alt_http_client_new(&cl, 0);
	RUN(basics);
	RUN(bodies);
	RUN(redirects);
	RUN(reuse);
	RUN(concurrency);
	RUN(timeouts);
	alt_http_client_free(cl);
	alt_ctx *ctx;
	alt_ctx_with_timeout(&ctx, 0, 2 * ALT_SEC);
	CHECK_OK(alt_http_server_shutdown(srv, ctx));
	alt_ctx_free(ctx);
	alt_sched_stop(acc);
	alt_http_server_free(srv);
	alt_http_mux_free(m);
	return check_done();
}
