/* A TLS echo server of the library for tests/stdtls_test.go:
	tlsd cert.pem key.pem [cert2.pem key2.pem] [-alpn a,b] [-auth N -ca ca.pem]
   prints "listening <addr>"; every connection answers each line with
	<line> | <suite> <group> <alpn> <sni> <client cert CN>
   ("big N" sends N bytes back; "keyupdate" updates the keys first);
   stops when stdin closes. */
#include <alt/crypto/tls.h>
#include <alt/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

static alt_tls_config cfg;
static alt_listener *ln;

static void serve(void *arg) {
	alt_conn *c = arg;
	alt_tls_conn *t;
	if (alt_tls_server(&t, c, &cfg) < 0)
		return;
	if (alt_tls_handshake(t) < 0) {
		fprintf(stderr, "handshake: %s\n", alt_tls_error(t));
		alt_tls_close(t);
		return;
	}
	alt_bufreader br;
	alt_bufreader_init(&br, alt_tls_reader(t), 0);
	alt_buf line = ALT_BUF_INIT, out = ALT_BUF_INIT;
	for (;;) {
		alt_buf_reset(&line);
		int rc = alt_bufreader_line(&br, &line, 1 << 20);
		if (rc < 0)
			break;
		alt_buf_reset(&out);
		alt_tls_state st;
		alt_tls_connection_state(t, &st);
		const char *l = alt_buf_cstr(&line);
		if (strncmp(l, "big ", 4) == 0) {
			long n = atol(l + 4);
			for (long i = 0; i < n; i++)
				alt_buf_append_byte(&out, 'a' + (int)(i % 26));
			alt_buf_append_byte(&out, '\n');
		} else {
			if (strcmp(l, "keyupdate") == 0)
				alt_tls_key_update(t);
			alt_str cn = st.npeer_certs ? alt_x509_name_get(&st.peer_certs[0]->subject, &ALT_OID(2, 5, 4, 3)) : ALT_STR("-");
			alt_buf_printf(&out, "%s | %04x %04x %s %s %.*s\n", l, st.cipher_suite, st.group, st.alpn[0] ? st.alpn : "-",
				st.server_name[0] ? st.server_name : "-", (int)cn.n, cn.p);
		}
		if (alt_tls_write(t, out.p, out.n) < 0)
			break;
	}
	alt_buf_free(&line);
	alt_buf_free(&out);
	alt_bufreader_free(&br);
	alt_tls_close(t);
}

static void acceptor(void *arg) {
	(void)arg;
	for (;;) {
		alt_conn *c;
		if (alt_accept(ln, &c, 0) < 0)
			return;
		alt_go(serve, c);
	}
}

/* a thread of its own: stdin blocks */
static void *watcher(void *arg) {
	(void)arg;
	char b[64];
	while (read(0, b, sizeof b) > 0) {
	}
	alt_listener_close(ln);
	return 0;
}

static void run(void *arg) {
	(void)arg;
	pthread_t th;
	pthread_create(&th, 0, watcher, 0);
	alt_go(acceptor, 0);
}

int main(int argc, char **argv) {
	static const alt_tls_cert *certs[4];
	static const char *alpn[8];
	size_t ncerts = 0, nalpn = 0;
	alt_x509_pool *cas = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-alpn") && i + 1 < argc) {
			for (char *save = 0, *t = strtok_r(argv[++i], ",", &save); t && nalpn < 8; t = strtok_r(0, ",", &save))
				alpn[nalpn++] = t;
		} else if (!strcmp(argv[i], "-auth") && i + 1 < argc)
			cfg.client_auth = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-ca") && i + 1 < argc) {
			cas = alt_x509_pool_new();
			FILE *f = fopen(argv[++i], "rb");
			char buf[65536];
			size_t n = f ? fread(buf, 1, sizeof buf, f) : 0;
			if (f)
				fclose(f);
			alt_x509_pool_add_pem(cas, buf, n);
		} else if (i + 1 < argc && ncerts < 4) {
			alt_tls_cert *c;
			const char *why = "";
			if (alt_tls_load_key_pair(&c, argv[i], argv[i + 1], &why) < 0) {
				fprintf(stderr, "key pair: %s\n", why);
				return 1;
			}
			certs[ncerts++] = c;
			i++;
		}
	}
	cfg.certs = certs;
	cfg.ncerts = ncerts;
	cfg.alpn = alpn;
	cfg.nalpn = nalpn;
	cfg.client_cas = cas;
	if (alt_listen(&ln, "tcp", "127.0.0.1:0") < 0)
		return 1;
	alt_buf a = ALT_BUF_INIT;
	alt_addr_format(alt_listener_addr(ln), &a);
	printf("listening %s\n", alt_buf_cstr(&a));
	fflush(stdout);
	alt_run(run, 0);
	return 0;
}
