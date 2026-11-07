#include "check.h"
#include <alt/crypto/tls.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

/* a CA, an Ed25519 server certificate for test.example and its key (made by Go, 2025-2045) */
static const char ca_pem[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBXjCCAQOgAwIBAgIBATAKBggqhkjOPQQDAjAWMRQwEgYDVQQDEwtUTFMgdGVz\n"
	"dCBDQTAeFw0yNTAxMDEwMDAwMDBaFw00NTAxMDEwMDAwMDBaMBYxFDASBgNVBAMT\n"
	"C1RMUyB0ZXN0IENBMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE0thwCSEMchRq\n"
	"u5hiKC+kEAXpbaoLue7c0IHNg4NC9uhqPY8Z7cBTY5i5uE77sBRZ5pZwwojwdgdp\n"
	"EcXU9txW/qNCMEAwDgYDVR0PAQH/BAQDAgIEMA8GA1UdEwEB/wQFMAMBAf8wHQYD\n"
	"VR0OBBYEFAZDqTVVu0BNgWle8lvy7OXt7bInMAoGCCqGSM49BAMCA0kAMEYCIQD4\n"
	"uWGRWfmlLUDoycv/2ESQqnj0swSEFXiKeWGrjgKS5gIhAO1Gy3Ta6/BBS/G+S53r\n"
	"XXXLmQiZDt9jdbO7Rdkwddyq\n"
	"-----END CERTIFICATE-----\n"
	;
static const char server_pem[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBRzCB7qADAgECAgECMAoGCCqGSM49BAMCMBYxFDASBgNVBAMTC1RMUyB0ZXN0\n"
	"IENBMB4XDTI1MDEwMTAwMDAwMFoXDTQ1MDEwMTAwMDAwMFowETEPMA0GA1UEAxMG\n"
	"c2VydmVyMCowBQYDK2VwAyEAzFRYSfSCjAtSUvXwI3EngRB+MouJvj3CxOnquoBI\n"
	"ps+jYTBfMA4GA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDATAfBgNV\n"
	"HSMEGDAWgBQGQ6k1VbtATYFpXvJb8uzl7e2yJzAXBgNVHREEEDAOggx0ZXN0LmV4\n"
	"YW1wbGUwCgYIKoZIzj0EAwIDSAAwRQIgfP/CgQ+lT9tV4Pvat92KP/lJRmrHM7Rj\n"
	"qxIPRmQWz18CIQD0T/LigC7q/Tg/rY2QgQuliTfRJFmnjQ1QS5p7UzurRg==\n"
	"-----END CERTIFICATE-----\n"
	;
static const char server_key[] =
	"-----BEGIN PRIVATE KEY-----\n"
	"MC4CAQAwBQYDK2VwBCIEIAYmzGA/fx5bi4rao+j60dmmW4/45vVcnqjLzMruhKGZ\n"
	"-----END PRIVATE KEY-----\n"
	;

typedef struct {
	int fd;
	alt_tls_config cfg;
	int rc;
	size_t got;
	char err[256];
} side;

enum { DATA = 1 << 20 };

static void *server(void *arg) {
	side *s = arg;
	alt_tls_conn *t;
	alt_tls_server_stream(&t, alt_fd_reader(&s->fd), alt_fd_writer(&s->fd), &s->cfg);
	s->rc = alt_tls_handshake(t);
	if (s->rc == 0) {
		/* echo until close_notify */
		char buf[10000];
		ssize_t n;
		while ((n = alt_tls_read(t, buf, sizeof buf)) > 0) {
			s->got += (size_t)n;
			if (alt_tls_write(t, buf, (size_t)n) < 0)
				break;
		}
		if (n < 0)
			s->rc = (int)n;
		alt_tls_close_write(t);
	}
	snprintf(s->err, sizeof s->err, "%s", alt_tls_error(t));
	alt_tls_close(t);
	shutdown(s->fd, SHUT_RDWR);
	return 0;
}

static void *client_writer(void *arg) {
	alt_tls_conn *t = arg;
	char buf[7000];
	for (size_t i = 0, sent = 0; sent < DATA; i++) {
		size_t k = sizeof buf < DATA - sent ? sizeof buf : DATA - sent;
		for (size_t j = 0; j < k; j++)
			buf[j] = (char)((sent + j) % 251);
		if (alt_tls_write(t, buf, k) < 0)
			break;
		sent += k;
		if (i % 40 == 0)
			alt_tls_key_update(t);
	}
	alt_tls_close_write(t);
	return 0;
}

static void exchange(const char *name, uint16_t suite, uint16_t group, bool expect_ok) {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	alt_tls_cert *cert;
	const char *why = "";
	CHECK_OK(alt_tls_key_pair(&cert, ALT_STR(server_pem), ALT_STR(server_key), &why));
	const alt_tls_cert *certs[1] = {cert};
	side sv = {.fd = fds[0], .cfg = {.certs = certs, .ncerts = 1}};
	pthread_t th;
	pthread_create(&th, 0, server, &sv);
	alt_x509_pool *roots = alt_x509_pool_new();
	alt_x509_pool_add_pem(roots, ca_pem, strlen(ca_pem));
	uint16_t suites[1] = {suite}, groups[2] = {group, ALT_TLS_X25519};
	alt_tls_config cc = {.server_name = name, .roots = roots, .cipher_suites = suites, .nsuites = 1, .groups = groups, .ngroups = 2};
	int fd = fds[1];
	alt_tls_conn *t;
	alt_tls_client_stream(&t, alt_fd_reader(&fd), alt_fd_writer(&fd), &cc);
	int rc = alt_tls_handshake(t);
	if (!expect_ok) {
		CHECK(rc < 0);
		CHECK(strstr(alt_tls_error(t), "not test.") != 0 || strstr(alt_tls_error(t), "valid for") != 0);
	} else {
		CHECK_OK(rc);
		alt_tls_state st;
		alt_tls_connection_state(t, &st);
		CHECK_INT(st.cipher_suite, suite);
		CHECK_INT(st.group, ALT_TLS_X25519); /* the server prefers it: a retry when the first share was another */
		CHECK_INT(st.nverified_chains, 1);
		/* writing and reading at once, from two threads */
		pthread_t w;
		pthread_create(&w, 0, client_writer, t);
		char buf[5000];
		size_t got = 0;
		bool same = true;
		ssize_t n;
		while ((n = alt_tls_read(t, buf, sizeof buf)) > 0) {
			for (ssize_t i = 0; i < n; i++)
				same &= buf[i] == (char)((got + (size_t)i) % 251);
			got += (size_t)n;
		}
		pthread_join(w, 0);
		CHECK_INT(n, 0);
		CHECK_INT(got, DATA);
		CHECK(same);
	}
	alt_tls_close(t);
	shutdown(fd, SHUT_RDWR);
	pthread_join(th, 0);
	if (expect_ok) {
		CHECK_INT(sv.rc, 0);
		CHECK_INT(sv.got, DATA);
	} else
		CHECK(sv.rc < 0);
	close(fds[0]);
	close(fds[1]);
	alt_x509_pool_free(roots);
	alt_tls_cert_free(cert);
}

static void suites_and_groups(void) {
	exchange("test.example", ALT_TLS_AES_128_GCM_SHA256, ALT_TLS_X25519, true);
	exchange("test.example", ALT_TLS_AES_256_GCM_SHA384, ALT_TLS_P256, true);
	exchange("test.example", ALT_TLS_CHACHA20_POLY1305_SHA256, ALT_TLS_P384, true);
	exchange("test.example", ALT_TLS_AES_128_GCM_SHA256, ALT_TLS_P521, true);
}

static void bad_name(void) { exchange("other.example", ALT_TLS_AES_128_GCM_SHA256, ALT_TLS_X25519, false); }

int main(void) {
	signal(SIGPIPE, SIG_IGN); /* the peer may close first; plain fd writers do not avoid the signal */
	RUN(suites_and_groups);
	RUN(bad_name);
	return check_done();
}
