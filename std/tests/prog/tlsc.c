/* A TLS client of the library for tests/stdtls_test.go:
	tlsc addr [-name n] [-roots ca.pem] [-insecure] [-alpn a,b] [-suites 1301,1303]
	     [-groups 17,1d] [-cert c.pem k.pem] [-keyupdate] [-min 303] [-max 303]
	     [-conns N] [-v]
   prints "ok <suite> <group> <alpn> <chains>" (with -v then
   "v <version> <resumed>") or "error <message>", then sends each stdin
   line and prints the answer line ("big N" answers are reported as
   "big <length> <ok>"). With -conns N, N connections share a session
   cache: the first N-1 only send "ping" and read the answer (which brings
   TLS 1.3 tickets). */
#include <alt/crypto/tls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	alt_tls_config cfg = {0};
	static const char *alpn[8];
	static uint16_t suites[8], groups[8];
	static const alt_tls_cert *certs[1];
	bool keyupdate = false, verbose = false;
	int conns = 1;
	const char *addr = argv[1];
	for (int i = 2; i < argc; i++) {
		if (!strcmp(argv[i], "-name"))
			cfg.server_name = argv[++i];
		else if (!strcmp(argv[i], "-insecure"))
			cfg.insecure_skip_verify = true;
		else if (!strcmp(argv[i], "-keyupdate"))
			keyupdate = true;
		else if (!strcmp(argv[i], "-v"))
			verbose = true;
		else if (!strcmp(argv[i], "-conns"))
			conns = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-min"))
			cfg.min_version = (uint16_t)strtol(argv[++i], 0, 16);
		else if (!strcmp(argv[i], "-max"))
			cfg.max_version = (uint16_t)strtol(argv[++i], 0, 16);
		else if (!strcmp(argv[i], "-roots")) {
			alt_x509_pool *p = alt_x509_pool_new();
			FILE *f = fopen(argv[++i], "rb");
			char buf[65536];
			size_t n = f ? fread(buf, 1, sizeof buf, f) : 0;
			if (f)
				fclose(f);
			alt_x509_pool_add_pem(p, buf, n);
			cfg.roots = p;
		} else if (!strcmp(argv[i], "-alpn")) {
			for (char *save = 0, *t = strtok_r(argv[++i], ",", &save); t && cfg.nalpn < 8; t = strtok_r(0, ",", &save))
				alpn[cfg.nalpn++] = t;
			cfg.alpn = alpn;
		} else if (!strcmp(argv[i], "-suites") || !strcmp(argv[i], "-groups")) {
			bool s = argv[i][1] == 's';
			size_t *n = s ? &cfg.nsuites : &cfg.ngroups;
			uint16_t *arr = s ? suites : groups;
			for (char *save = 0, *t = strtok_r(argv[++i], ",", &save); t && *n < 8; t = strtok_r(0, ",", &save))
				arr[(*n)++] = (uint16_t)strtol(t, 0, 16);
			if (s)
				cfg.cipher_suites = suites;
			else
				cfg.groups = groups;
		} else if (!strcmp(argv[i], "-cert")) {
			alt_tls_cert *c;
			const char *why = "";
			if (alt_tls_load_key_pair(&c, argv[i + 1], argv[i + 2], &why) < 0) {
				printf("error key pair: %s\n", why);
				return 0;
			}
			certs[0] = c;
			cfg.certs = certs;
			cfg.ncerts = 1;
			i += 2;
		}
	}
	if (conns > 1)
		cfg.session_cache = alt_tls_session_cache_new(0);
	alt_tls_conn *t;
	int rc;
	for (int k = 0;; k++) {
		t = 0;
		rc = alt_tls_dial(&t, "tcp", addr, &cfg, 0);
		if (rc < 0) {
			printf("error %s\n", t ? alt_tls_error(t) : alt_strerror(rc));
			return 0;
		}
		alt_tls_state st;
		alt_tls_connection_state(t, &st);
		printf("ok %04x %04x %s %zu\n", st.cipher_suite, st.group, st.alpn[0] ? st.alpn : "-", st.nverified_chains);
		if (verbose)
			printf("v %04x %d\n", st.version, st.did_resume);
		fflush(stdout);
		if (k == conns - 1)
			break;
		char b[512];
		ssize_t n;
		if (alt_tls_write(t, "ping\n", 5) < 0 || (n = alt_tls_read(t, b, sizeof b)) <= 0) {
			printf("ping error %s\n", alt_tls_error(t));
			return 0;
		}
		alt_tls_close(t);
	}
	if (keyupdate)
		alt_tls_key_update(t);
	alt_bufreader br;
	alt_bufreader_init(&br, alt_tls_reader(t), 0);
	char line[4096];
	alt_buf ans = ALT_BUF_INIT;
	while (fgets(line, sizeof line, stdin)) {
		if (alt_tls_write(t, line, strlen(line)) < 0) {
			printf("write error %s\n", alt_tls_error(t));
			break;
		}
		alt_buf_reset(&ans);
		rc = alt_bufreader_line(&br, &ans, 64 << 20);
		if (rc < 0) {
			printf("read error %s\n", alt_tls_error(t));
			break;
		}
		if (strncmp(line, "big ", 4) == 0) {
			bool ok = true;
			for (size_t i = 0; i < ans.n; i++)
				ok &= ans.p[i] == 'a' + (int)(i % 26);
			printf("big %zu %s\n", ans.n, ok ? "ok" : "bad");
		} else
			printf("%s\n", alt_buf_cstr(&ans));
		fflush(stdout);
	}
	alt_buf_free(&ans);
	alt_bufreader_free(&br);
	alt_tls_close(t);
	return 0;
}
