/* An HTTP/2 (and HTTP/1.1) server of the library for tests/stdhttp2_test.go:
	h2d cert.pem key.pem [-h2c] [-streams N] [-noh2]
   (cert "-": plain HTTP, with -h2c HTTP/2 by prior knowledge); prints
   "listening <addr>". Paths:
	/echo           the request body back, as it comes
	/big/N          N bytes ('a' + i % 26), flushed every 64 KiB
	/slow/MS        waits, then answers
	/status/N       that status with a short body
	/headers        the request header, one "Key: value" per line
	/ctx            waits for the request to be cancelled (5 s at most)
	/cancelled      how many /ctx requests saw the cancellation
	/redirect       302 to /final
	/noread         answers at once without reading the body
	/shutdown       starts a graceful shutdown (after answering)
	/form           a multipart form: "v <name>=<value>" and "f <field> <file> <size> <fnv64>" lines
	other           "<method> <path> <proto> <body length>"
   stops when stdin closes */
#include <alt/crypto/tls.h>
#include <alt/mime/multipart.h>
#include <alt/net/http.h>
#include <alt/task.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static alt_http_server *srv;
static _Atomic int cancelled;

static void shutdown_task(void *arg) {
	(void)arg;
	alt_sleep(50 * ALT_MS);
	alt_http_server_shutdown(srv, 0);
}

static void handle(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	alt_str p = r->url.path;
	char path[256];
	snprintf(path, sizeof path, "%.*s", (int)p.n, p.p);
	if (!strcmp(path, "/echo")) {
		alt_http_header_set_cstr(alt_http_rw_header(w), "Content-Type", "application/octet-stream");
		alt_http_rw_write_header(w, 200);
		alt_http_rw_flush(w);
		char buf[32768];
		ssize_t n;
		while ((n = alt_read(r->body, buf, sizeof buf)) > 0)
			if (alt_http_rw_write(w, buf, (size_t)n) < 0)
				break;
		return;
	}
	if (!strncmp(path, "/big/", 5)) {
		long n = atol(path + 5);
		char buf[65536];
		for (long off = 0; off < n;) {
			size_t k = n - off < (long)sizeof buf ? (size_t)(n - off) : sizeof buf;
			for (size_t i = 0; i < k; i++)
				buf[i] = (char)('a' + (off + (long)i) % 26);
			if (alt_http_rw_write(w, buf, k) < 0)
				return;
			alt_http_rw_flush(w);
			off += (long)k;
		}
		return;
	}
	if (!strncmp(path, "/slow/", 6)) {
		alt_sleep(atol(path + 6) * ALT_MS);
		alt_http_rw_printf(w, "slow %s\n", path + 6);
		return;
	}
	if (!strncmp(path, "/status/", 8)) {
		alt_http_rw_write_header(w, atoi(path + 8));
		alt_http_rw_write_cstr(w, "status body\n");
		return;
	}
	if (!strcmp(path, "/headers")) {
		for (size_t i = 0; i < r->header.n; i++)
			alt_http_rw_printf(w, "%.*s: %.*s\n", (int)r->header.f[i].key.n, r->header.f[i].key.p, (int)r->header.f[i].value.n,
				r->header.f[i].value.p);
		alt_http_rw_printf(w, "Host=%.*s\n", (int)r->host.n, r->host.p);
		return;
	}
	if (!strcmp(path, "/ctx")) {
		alt_http_rw_write_header(w, 200);
		alt_http_rw_write_cstr(w, "waiting\n");
		alt_http_rw_flush(w);
		if (alt_ctx_wait(r->ctx, alt_deadline(5 * ALT_SEC)) == -ECANCELED)
			atomic_fetch_add(&cancelled, 1);
		return;
	}
	if (!strcmp(path, "/cancelled")) {
		alt_http_rw_printf(w, "%d\n", atomic_load(&cancelled));
		return;
	}
	if (!strcmp(path, "/redirect")) {
		alt_http_redirect(w, r, "/final", 302);
		return;
	}
	if (!strcmp(path, "/noread")) {
		alt_http_rw_write_cstr(w, "not read\n");
		return;
	}
	if (!strcmp(path, "/form")) {
		const alt_multipart_form *form;
		int rc = alt_http_parse_multipart_form(r, 64 << 20, &form);
		if (rc < 0) {
			alt_http_error(w, alt_strerror(rc), 400);
			return;
		}
		for (size_t i = 0; i < form->values.n; i++)
			alt_http_rw_printf(w, "v %.*s=%.*s\n", (int)form->values.kv[i].key.n, form->values.kv[i].key.p,
				(int)form->values.kv[i].value.n, form->values.kv[i].value.p);
		for (size_t i = 0; i < form->nfiles; i++) {
			const alt_multipart_file *mf = &form->files[i];
			uint64_t h = 14695981039346656037ull;
			for (size_t k = 0; k < mf->data.n; k++)
				h = (h ^ (uint8_t)mf->data.p[k]) * 1099511628211ull;
			alt_http_rw_printf(w, "f %.*s %.*s %zu %016llx %.*s\n", (int)mf->field.n, mf->field.p, (int)mf->file_name.n,
				mf->file_name.p, mf->data.n, (unsigned long long)h, (int)mf->content_type.n, mf->content_type.p);
		}
		return;
	}
	if (!strcmp(path, "/shutdown")) {
		alt_http_rw_write_cstr(w, "shutting down\n");
		alt_go(shutdown_task, 0);
		return;
	}
	alt_str body;
	alt_http_read_body(r, 64 << 20, &body);
	alt_http_rw_printf(w, "%.*s %.*s %.*s %zu\n", (int)r->method.n, r->method.p, (int)r->url.path.n, r->url.path.p, (int)r->proto.n,
		r->proto.p, body.n);
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
	alt_http_server_config cfg = {.handler = {handle, 0}, .threads = 2};
	for (int i = 3; i < argc; i++) {
		if (!strcmp(argv[i], "-h2c"))
			cfg.h2c = true;
		else if (!strcmp(argv[i], "-noh2"))
			cfg.disable_http2 = true;
		else if (!strcmp(argv[i], "-streams") && i + 1 < argc)
			cfg.http2_max_concurrent_streams = (uint32_t)atoi(argv[++i]);
	}
	static alt_tls_config tc;
	static const alt_tls_cert *certs[1];
	if (strcmp(argv[1], "-")) {
		alt_tls_cert *cert;
		const char *why = "";
		if (alt_tls_load_key_pair(&cert, argv[1], argv[2], &why) < 0) {
			fprintf(stderr, "%s\n", why);
			return 1;
		}
		certs[0] = cert;
		tc.certs = certs;
		tc.ncerts = 1;
		cfg.tls = &tc;
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
	/* after a graceful shutdown: wait for stdin to close */
	pthread_join(th, 0);
	return 0;
}
