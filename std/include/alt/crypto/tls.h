/* AltC standard library: TLS 1.3 (RFC 8446; Go's crypto/tls).

   A TLS connection wraps a connection (or any reader/writer pair) and
   reads and writes application data; the handshake runs on the first
   read or write, or explicitly with alt_tls_handshake. Reads and writes
   may run concurrently (one reader, one writer at a time).

	alt_tls_config cfg = {.server_name = "example.com"};
	alt_tls_conn *t;
	alt_tls_dial(&t, "tcp", "example.com:443", &cfg, ctx);
	alt_tls_write(t, "GET / HTTP/1.0\r\n\r\n", 18);

   Cipher suites: TLS_AES_128_GCM_SHA256, TLS_AES_256_GCM_SHA384,
   TLS_CHACHA20_POLY1305_SHA256. Groups: X25519, P-256, P-384, P-521.
   Certificates: ECDSA, Ed25519, RSA (RSA-PSS signatures). */
#ifndef ALT_CRYPTO_TLS_H
#define ALT_CRYPTO_TLS_H

#include <alt/context.h>
#include <alt/crypto/x509.h>
#include <alt/io.h>
#include <alt/net.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ALT_TLS_AES_128_GCM_SHA256 = 0x1301,
	ALT_TLS_AES_256_GCM_SHA384 = 0x1302,
	ALT_TLS_CHACHA20_POLY1305_SHA256 = 0x1303,
};
enum { ALT_TLS_X25519 = 0x001d, ALT_TLS_P256 = 0x0017, ALT_TLS_P384 = 0x0018, ALT_TLS_P521 = 0x0019 };
enum { ALT_TLS_NO_CLIENT_CERT, ALT_TLS_REQUEST_CLIENT_CERT, ALT_TLS_REQUIRE_ANY_CLIENT_CERT, ALT_TLS_VERIFY_CLIENT_CERT_IF_GIVEN,
	ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT }; /* Go's ClientAuthType */

/* a certificate chain (DER, leaf first) and its private key */
typedef struct {
	const alt_str *chain;
	size_t nchain;
	alt_x509_private_key key;
} alt_tls_cert;

/* loads a certificate chain and a key from PEM files (tls.LoadX509KeyPair):
   the result owns its memory (free with alt_tls_cert_free) */
int alt_tls_load_key_pair(alt_tls_cert **out, const char *cert_file, const char *key_file, const char **why);
/* from PEM text in memory (tls.X509KeyPair) */
int alt_tls_key_pair(alt_tls_cert **out, alt_str cert_pem, alt_str key_pem, const char **why);
void alt_tls_cert_free(alt_tls_cert *c);

typedef struct alt_tls_config_s {
	/* both sides */
	const alt_tls_cert *const *certs;  /* the server's (chosen by the client's server name) or the client's */
	size_t ncerts;
	const char *const *alpn;           /* application protocols in preference order (NextProtos) */
	size_t nalpn;
	const uint16_t *cipher_suites;     /* preference order; NULL: the defaults */
	size_t nsuites;
	const uint16_t *groups;            /* key exchange preference (CurvePreferences); NULL: the defaults */
	size_t ngroups;
	/* the client */
	const char *server_name;           /* SNI and the name verified; NULL: taken from the dial address */
	const alt_x509_pool *roots;        /* NULL: the system's */
	bool insecure_skip_verify;
	/* the server */
	int client_auth;
	const alt_x509_pool *client_cas;
	alt_dur handshake_timeout;         /* 0: none (deadlines of the connection apply) */
} alt_tls_config;

typedef struct alt_tls_conn alt_tls_conn;

/* wraps c (owned: closed with the TLS connection) */
int alt_tls_client(alt_tls_conn **out, alt_conn *c, const alt_tls_config *cfg);
int alt_tls_server(alt_tls_conn **out, alt_conn *c, const alt_tls_config *cfg);
/* over any stream (not owned) */
int alt_tls_client_stream(alt_tls_conn **out, alt_reader r, alt_writer w, const alt_tls_config *cfg);
int alt_tls_server_stream(alt_tls_conn **out, alt_reader r, alt_writer w, const alt_tls_config *cfg);
/* connects and handshakes */
int alt_tls_dial(alt_tls_conn **out, const char *network, const char *address, const alt_tls_config *cfg, alt_ctx *ctx);

int alt_tls_handshake(alt_tls_conn *t);
/* application data: a count, 0 after the peer's close_notify, or an error */
ssize_t alt_tls_read(alt_tls_conn *t, void *buf, size_t n);
ssize_t alt_tls_write(alt_tls_conn *t, const void *buf, size_t n);
/* sends close_notify (no more writes) */
int alt_tls_close_write(alt_tls_conn *t);
/* sends close_notify, closes the connection, frees t */
int alt_tls_close(alt_tls_conn *t);
/* asks for new traffic keys (both directions) */
int alt_tls_key_update(alt_tls_conn *t);
alt_reader alt_tls_reader(alt_tls_conn *t);
alt_writer alt_tls_writer(alt_tls_conn *t);
alt_conn *alt_tls_net_conn(alt_tls_conn *t);

/* the reason of the last failure ("tls: ..." as Go's texts where they exist) */
const char *alt_tls_error(const alt_tls_conn *t);

typedef struct {
	bool handshake_complete;
	uint16_t version, cipher_suite, group;
	const char *alpn;                  /* "" when none */
	const char *server_name;           /* the client's SNI */
	const alt_x509_cert *const *peer_certs;
	size_t npeer_certs;
	const alt_x509_chain *verified_chains;
	size_t nverified_chains;
} alt_tls_state;

void alt_tls_connection_state(const alt_tls_conn *t, alt_tls_state *s);

#ifdef __cplusplus
}
#endif

#endif
