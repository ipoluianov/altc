/* HTTP/2 (h2c) between the library's server and client in one process:
   multiplexing from tasks and threads, flow control, timeouts,
   cancellation, shutdown (also run under the sanitizers) */
#include "check.h"
#include <alt/net/http.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

static char addr[64];
static alt_http_server *srv;
static alt_http_client *cl;
static _Atomic int ok_count;
static alt_waitgroup wg = ALT_WAITGROUP_INIT;

static void hello(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_http_rw_printf(w, "hello %.*s", (int)r->proto.n, r->proto.p);
}

static void echo(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	char buf[8192];
	ssize_t n;
	while ((n = alt_read(r->body, buf, sizeof buf)) > 0)
		if (alt_http_rw_write(w, buf, (size_t)n) < 0)
			break;
}

static void slow(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_ctx_wait(r->ctx, alt_deadline(500 * ALT_MS));
	alt_http_rw_write_cstr(w, "slow");
}

static void big(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)r, (void)arg;
	static char chunk[65536];
	for (size_t i = 0; i < sizeof chunk; i++)
		chunk[i] = (char)(i % 251);
	for (int i = 0; i < 80; i++)
		if (alt_http_rw_write(w, chunk, sizeof chunk) < 0)
			return;
}

static void serve_task(void *arg) {
	alt_listener *l = arg;
	CHECK_ERR(alt_http_server_serve(srv, l), -ALT_ECLOSED);
}

static void url(char *out, size_t n, const char *path) { snprintf(out, n, "http://%s%s", addr, path); }

static void basics(void) {
	char u[128];
	url(u, sizeof u, "/hello");
	for (int i = 0; i < 3; i++) {
		alt_http_response *resp;
		CHECK_OK(alt_http_get(cl, u, &resp, 0));
		CHECK_INT(resp->status, 200);
		CHECK(alt_str_eq_cstr(resp->proto, "HTTP/2.0"));
		alt_str body;
		CHECK_OK(alt_http_response_body(resp, 0, &body));
		CHECK(alt_str_eq_cstr(body, "hello HTTP/2.0"));
		CHECK(alt_str_eq_cstr(alt_http_header_value(&resp->header, "Content-Length"), "14"));
		alt_http_response_free(resp);
	}
	url(u, sizeof u, "/nothing");
	alt_http_response *resp;
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK_INT(resp->status, 404);
	alt_http_response_free(resp);
}

static void bodies(void) {
	char u[128];
	url(u, sizeof u, "/echo");
	static const size_t sizes[] = {0, 1, 100, 16384, 16385, 65535, 65536, 200000, 3000000};
	for (size_t k = 0; k < sizeof sizes / sizeof *sizes; k++) {
		size_t n = sizes[k];
		char *p = malloc(n + 1);
		for (size_t i = 0; i < n; i++)
			p[i] = (char)('a' + (i * 7) % 26);
		alt_http_response *resp;
		CHECK_OK(alt_http_post(cl, u, "application/octet-stream", alt_str_make(p, n), &resp, 0));
		alt_str body;
		CHECK_OK(alt_http_response_body(resp, 8 << 20, &body));
		CHECK_INT(body.n, n);
		CHECK(body.n == n && memcmp(body.p, p, n) == 0);
		alt_http_response_free(resp);
		free(p);
	}
	url(u, sizeof u, "/big");
	alt_http_response *resp;
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	alt_str body;
	CHECK_OK(alt_http_response_body(resp, 8 << 20, &body));
	CHECK_INT(body.n, 80 * 65536);
	bool same = true;
	for (size_t i = 0; i < body.n; i++)
		same &= body.p[i] == (char)((i % 65536) % 251);
	CHECK(same);
	alt_http_response_free(resp);
}

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

static void timeouts_and_cancel(void) {
	alt_http_client *c;
	alt_http_client_config cfg = {.h2c = true, .timeout = 100 * ALT_MS};
	alt_http_client_new(&c, &cfg);
	char u[128];
	url(u, sizeof u, "/slow");
	alt_http_response *resp;
	alt_time t0 = alt_now();
	CHECK_ERR(alt_http_get(c, u, &resp, 0), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 100 * ALT_MS && alt_since(t0) < 400 * ALT_MS);
	/* the connection is fine after the stream was cancelled */
	url(u, sizeof u, "/hello");
	CHECK_OK(alt_http_get(c, u, &resp, 0));
	CHECK_INT(resp->status, 200);
	alt_http_response_free(resp);
	alt_http_client_free(c);
	/* a response dropped while its body comes */
	url(u, sizeof u, "/big");
	for (int i = 0; i < 5; i++) {
		CHECK_OK(alt_http_get(cl, u, &resp, 0));
		char b[100];
		CHECK(alt_read(resp->body, b, sizeof b) > 0);
		alt_http_response_free(resp);
	}
	url(u, sizeof u, "/hello");
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK_INT(resp->status, 200);
	alt_http_response_free(resp);
	/* idle connections closed, then a new one */
	alt_http_client_close_idle(cl);
	CHECK_OK(alt_http_get(cl, u, &resp, 0));
	CHECK_INT(resp->status, 200);
	alt_http_response_free(resp);
}

static void in_flight(void *arg) {
	(void)arg;
	char u[128];
	url(u, sizeof u, "/slow");
	alt_http_response *resp;
	if (alt_http_get(cl, u, &resp, 0) == 0) {
		alt_str body;
		if (alt_http_response_body(resp, 0, &body) == 0 && alt_str_eq_cstr(body, "slow"))
			atomic_fetch_add(&ok_count, 1);
		alt_http_response_free(resp);
	}
}

int main(void) {
	alt_http_mux *m;
	alt_http_mux_new(&m);
	alt_http_mux_handle_func(m, "GET /hello", hello, 0);
	alt_http_mux_handle_func(m, "/echo", echo, 0);
	alt_http_mux_handle_func(m, "GET /slow", slow, 0);
	alt_http_mux_handle_func(m, "GET /big", big, 0);
	alt_http_server_config cfg = {0};
	cfg.handler = alt_http_mux_handler(m);
	cfg.threads = 2;
	cfg.h2c = true;
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
	alt_http_client_config cc = {.h2c = true};
	alt_http_client_new(&cl, &cc);
	RUN(basics);
	RUN(bodies);
	RUN(concurrency);
	RUN(timeouts_and_cancel);
	/* a graceful shutdown waits for the request in flight */
	atomic_store(&ok_count, 0);
	alt_sched *bg;
	alt_sched_start(&bg);
	alt_go_on(bg, in_flight, 0, 0);
	alt_sleep(100 * ALT_MS);
	alt_ctx *ctx;
	alt_ctx_with_timeout(&ctx, 0, 3 * ALT_SEC);
	CHECK_OK(alt_http_server_shutdown(srv, ctx));
	alt_ctx_free(ctx);
	alt_sched_stop(bg);
	CHECK_INT(atomic_load(&ok_count), 1);
	alt_http_client_free(cl);
	alt_sched_stop(acc);
	alt_http_server_free(srv);
	alt_http_mux_free(m);
	return check_done();
}
