/* Fetches with the library's client: each input line is
	METHOD <tab> URL [<tab> Header: value]...
   and the answer line is
	status | uncompressed | content-length | Content-Encoding | body hex
   or "error <message>" */
#include "../diff/common.h"
#include <alt/crypto/x509.h>
#include <alt/net/http.h>

int main(int argc, char **argv) {
	alt_http_client *c;
	alt_http_client_config cfg = {.disable_compression = argc > 1 && strcmp(argv[1], "nogzip") == 0};
	/* "roots=<pem file>": HTTPS roots */
	for (int i = 1; i < argc; i++)
		if (strncmp(argv[i], "roots=", 6) == 0) {
			alt_x509_pool *p = alt_x509_pool_new();
			FILE *f = fopen(argv[i] + 6, "rb");
			char buf[65536];
			size_t n = f ? fread(buf, 1, sizeof buf, f) : 0;
			if (f)
				fclose(f);
			alt_x509_pool_add_pem(p, buf, n);
			cfg.tls_roots = p;
		}
	if (alt_http_client_new(&c, &cfg) < 0)
		return 1;
	char *line, *f[16];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 16);
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_http_header h;
		alt_http_header_init(&h, &a);
		for (int i = 2; i < n; i++) {
			char *colon = strchr(f[i], ':');
			if (colon) {
				*colon = 0;
				alt_http_header_add(&h, alt_str_cstr(f[i]), alt_str_trim_space(alt_str_cstr(colon + 1)));
			}
		}
		alt_http_client_request req = {f[0], f[1], &h, 0, 0};
		alt_http_response *resp;
		int rc = alt_http_client_do(c, &req, &resp, 0);
		if (rc < 0) {
			printf("error %s\n", alt_strerror(rc));
			alt_arena_free(&a);
			continue;
		}
		alt_str body;
		rc = alt_http_response_body(resp, 0, &body);
		alt_str ce = alt_http_header_value(&resp->header, "Content-Encoding");
		printf("%d|%d|%lld|%.*s|", resp->status, resp->uncompressed, (long long)resp->content_length, (int)ce.n, ce.p);
		if (rc < 0)
			printf("body error %s", alt_strerror(rc));
		else
			diff_puthex(body.p, body.n);
		printf("\n");
		fflush(stdout);
		alt_http_response_free(resp);
		alt_arena_free(&a);
	}
	alt_http_client_free(c);
	return 0;
}
