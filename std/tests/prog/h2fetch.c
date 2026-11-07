/* A client of the library for tests/stdhttp2_test.go:
	h2fetch [roots=ca.pem] [h2c] [noh2] [insecure] [jar]
   one command per input line, one answer line each:
	get URL               -> <status> <proto> <length> <fnv64 of the body>
	post URL SIZE         -> the same for a POST of SIZE bytes ('a' + i % 26)
	par N URL             -> N concurrent GETs: "par <ok count> <ms>"
	cancel URL            -> reads a line of the body, drops the rest: "cancelled <status>"
	headers URL K=V...    -> GET with those headers; the body as one line ('|' for newlines)
   errors: "error <message>" */
#include "../diff/common.h"
#include <alt/crypto/x509.h>
#include <alt/net/cookiejar.h>
#include <alt/net/http.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <stdatomic.h>

static alt_http_client *client;

static uint64_t fnv(const char *p, size_t n) {
	uint64_t h = 14695981039346656037ull;
	for (size_t i = 0; i < n; i++)
		h = (h ^ (uint8_t)p[i]) * 1099511628211ull;
	return h;
}

typedef struct {
	int64_t total, off;
} pattern;

static ssize_t pattern_read(void *obj, void *buf, size_t n) {
	pattern *p = obj;
	if (p->off >= p->total)
		return 0;
	size_t k = n < (size_t)(p->total - p->off) ? n : (size_t)(p->total - p->off);
	for (size_t i = 0; i < k; i++)
		((char *)buf)[i] = (char)('a' + (p->off + (int64_t)i) % 26);
	p->off += (int64_t)k;
	return (ssize_t)k;
}

static void report(const char *method, const char *url, int64_t size, alt_http_header *h) {
	pattern pt = {size, 0};
	alt_reader rd = {pattern_read, &pt};
	alt_http_client_request req = {method, url, h, size > 0 ? &rd : 0, size};
	alt_http_response *resp;
	int rc = alt_http_client_do(client, &req, &resp, 0);
	if (rc < 0) {
		printf("error %s\n", alt_strerror(rc));
		return;
	}
	alt_buf b = ALT_BUF_INIT;
	ssize_t n = alt_read_all(resp->body, &b, 256 << 20);
	if (n < 0)
		printf("error body %s\n", alt_strerror((int)n));
	else
		printf("%d %.*s %zu %016llx\n", resp->status, (int)resp->proto.n, resp->proto.p, b.n,
			(unsigned long long)fnv(b.p ? b.p : "", b.n));
	alt_buf_free(&b);
	alt_http_response_free(resp);
}

typedef struct {
	const char *url;
	_Atomic int *ok;
	alt_waitgroup *wg;
} par_arg;

static void par_task(void *arg) {
	par_arg *a = arg;
	alt_http_response *resp;
	if (alt_http_get(client, a->url, &resp, 0) == 0) {
		alt_str body;
		if (alt_http_response_body(resp, 0, &body) == 0 && resp->status == 200)
			atomic_fetch_add(a->ok, 1);
		alt_http_response_free(resp);
	}
	alt_waitgroup_done(a->wg);
}

static void run(void *arg) {
	(void)arg;
	char *line, *f[32];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 32);
		if (!strcmp(f[0], "get"))
			report("GET", f[1], 0, 0);
		else if (!strcmp(f[0], "head"))
			report("HEAD", f[1], 0, 0);
		else if (!strcmp(f[0], "post"))
			report("POST", f[1], atoll(f[2]), 0);
		else if (!strcmp(f[0], "par")) {
			int k = atoi(f[1]);
			_Atomic int ok = 0;
			alt_waitgroup wg = ALT_WAITGROUP_INIT;
			par_arg *args = calloc((size_t)k, sizeof *args);
			alt_time t0 = alt_now();
			alt_waitgroup_add(&wg, k);
			for (int i = 0; i < k; i++) {
				args[i] = (par_arg){f[2], &ok, &wg};
				alt_go(par_task, &args[i]);
			}
			alt_waitgroup_wait(&wg);
			printf("par %d %lld\n", atomic_load(&ok), (long long)((alt_now() - t0) / ALT_MS));
			free(args);
		} else if (!strcmp(f[0], "cancel")) {
			alt_http_response *resp;
			int rc = alt_http_get(client, f[1], &resp, 0);
			if (rc < 0) {
				printf("error %s\n", alt_strerror(rc));
			} else {
				char b[8];
				alt_read(resp->body, b, sizeof b);
				printf("cancelled %d\n", resp->status);
				alt_http_response_free(resp);
			}
		} else if (!strcmp(f[0], "headers")) {
			alt_arena a;
			alt_arena_init(&a, 0);
			alt_http_header h;
			alt_http_header_init(&h, &a);
			for (int i = 2; i < n; i++) {
				char *eq = strchr(f[i], '=');
				if (eq) {
					*eq = 0;
					alt_http_header_add_cstr(&h, f[i], eq + 1);
				}
			}
			alt_http_client_request req = {"GET", f[1], &h, 0, 0};
			alt_http_response *resp;
			int rc = alt_http_client_do(client, &req, &resp, 0);
			if (rc < 0)
				printf("error %s\n", alt_strerror(rc));
			else {
				alt_str body;
				alt_http_response_body(resp, 0, &body);
				for (size_t i = 0; i < body.n; i++)
					putchar(body.p[i] == '\n' ? '|' : body.p[i]);
				printf("\n");
				alt_http_response_free(resp);
			}
			alt_arena_free(&a);
		} else if (!strcmp(f[0], "closeidle")) {
			alt_http_client_close_idle(client);
			printf("closed\n");
		}
		fflush(stdout);
	}
}

int main(int argc, char **argv) {
	alt_http_client_config cfg = {0};
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "roots=", 6) == 0) {
			alt_x509_pool *p = alt_x509_pool_new();
			FILE *fl = fopen(argv[i] + 6, "rb");
			char buf[65536];
			size_t n = fl ? fread(buf, 1, sizeof buf, fl) : 0;
			if (fl)
				fclose(fl);
			alt_x509_pool_add_pem(p, buf, n);
			cfg.tls_roots = p;
		} else if (!strcmp(argv[i], "h2c"))
			cfg.h2c = true;
		else if (!strcmp(argv[i], "noh2"))
			cfg.disable_http2 = true;
		else if (!strcmp(argv[i], "insecure"))
			cfg.tls_insecure_skip_verify = true;
		else if (!strcmp(argv[i], "jar"))
			alt_cookie_jar_new(&cfg.jar);
	}
	if (alt_http_client_new(&client, &cfg) < 0)
		return 1;
	alt_run(run, 0);
	alt_http_client_free(client);
	return 0;
}
