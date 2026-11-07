/* TLS internals shared by the record layer (record.c), the common parts
   of the handshakes (common.c), TLS 1.3 (tls13.c), TLS 1.2 (tls12.c), the
   session cache and tickets (session.c) and the connection API (tls.c).
   Behaviour follows Go's crypto/tls where the RFCs leave room. */
#ifndef ALT_TLS_INTERNAL_H
#define ALT_TLS_INTERNAL_H

#include <alt/crypto/aes.h>
#include <alt/crypto/chacha20poly1305.h>
#include <alt/crypto/ecdsa.h>
#include <alt/crypto/ed25519.h>
#include <alt/crypto/kdf.h>
#include <alt/crypto/rsa.h>
#include <alt/crypto/tls.h>
#include <alt/sync.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	REC_CCS = 20, REC_ALERT = 21, REC_HANDSHAKE = 22, REC_APP = 23,
	HS_HELLO_REQUEST = 0, HS_CLIENT_HELLO = 1, HS_SERVER_HELLO = 2, HS_NEW_SESSION_TICKET = 4, HS_ENCRYPTED_EXTENSIONS = 8,
	HS_CERTIFICATE = 11, HS_SERVER_KEY_EXCHANGE = 12, HS_CERTIFICATE_REQUEST = 13, HS_SERVER_HELLO_DONE = 14,
	HS_CERTIFICATE_VERIFY = 15, HS_CLIENT_KEY_EXCHANGE = 16, HS_FINISHED = 20, HS_CERTIFICATE_STATUS = 22, HS_KEY_UPDATE = 24,
	HS_MESSAGE_HASH = 254,
	EXT_SERVER_NAME = 0, EXT_STATUS_REQUEST = 5, EXT_SUPPORTED_GROUPS = 10, EXT_POINT_FORMATS = 11, EXT_SIGNATURE_ALGORITHMS = 13,
	EXT_ALPN = 16, EXT_SCT = 18, EXT_EXTENDED_MASTER_SECRET = 23, EXT_SESSION_TICKET = 35, EXT_PRE_SHARED_KEY = 41,
	EXT_EARLY_DATA = 42, EXT_SUPPORTED_VERSIONS = 43, EXT_COOKIE = 44, EXT_PSK_MODES = 45, EXT_CERT_AUTHORITIES = 47,
	EXT_SIG_ALGS_CERT = 50, EXT_KEY_SHARE = 51, EXT_RENEGOTIATION_INFO = 0xff01,
	MAX_PLAINTEXT = 16384, MAX_CIPHERTEXT = 16384 + 2048, MAX_WRITE_CIPHERTEXT = 16384 + 256,
	VERSION_12 = 0x0303, VERSION_13 = 0x0304,
	SCSV_RENEGOTIATION = 0x00ff, SCSV_FALLBACK = 0x5600,
};

/* alerts */
enum {
	A_CLOSE_NOTIFY = 0, A_UNEXPECTED_MESSAGE = 10, A_BAD_RECORD_MAC = 20, A_RECORD_OVERFLOW = 22, A_HANDSHAKE_FAILURE = 40,
	A_BAD_CERTIFICATE = 42, A_UNSUPPORTED_CERTIFICATE = 43, A_CERTIFICATE_EXPIRED = 45, A_CERTIFICATE_UNKNOWN = 46,
	A_ILLEGAL_PARAMETER = 47, A_UNKNOWN_CA = 48, A_DECODE_ERROR = 50, A_DECRYPT_ERROR = 51, A_PROTOCOL_VERSION = 70,
	A_INAPPROPRIATE_FALLBACK = 86, A_INTERNAL_ERROR = 80, A_USER_CANCELED = 90, A_NO_RENEGOTIATION = 100,
	A_MISSING_EXTENSION = 109, A_UNSUPPORTED_EXTENSION = 110, A_UNRECOGNIZED_NAME = 112, A_CERTIFICATE_REQUIRED = 116,
	A_NO_APPLICATION_PROTOCOL = 120,
};

/* signature schemes */
enum {
	S_RSA_PKCS1_SHA256 = 0x0401, S_RSA_PKCS1_SHA384 = 0x0501, S_RSA_PKCS1_SHA512 = 0x0601, S_ECDSA_P256 = 0x0403,
	S_ECDSA_P384 = 0x0503, S_ECDSA_P521 = 0x0603, S_RSA_PSS_256 = 0x0804, S_RSA_PSS_384 = 0x0805, S_RSA_PSS_512 = 0x0806,
	S_ED25519 = 0x0807, S_RSA_PSS_PSS_256 = 0x0809, S_RSA_PSS_PSS_384 = 0x080a, S_RSA_PSS_PSS_512 = 0x080b, S_RSA_PKCS1_SHA1 = 0x0201,
	S_ECDSA_SHA1 = 0x0203,
};

/* record protection */
enum { C_AES_GCM = 1, C_CHACHA, C_AES_CBC_SHA };

typedef struct {
	int on;                  /* 0: plaintext */
	int cipher;              /* C_* */
	bool v12;                /* TLS 1.2 framing (no inner type; explicit nonce or IV; MAC) */
	uint16_t suite;
	alt_gcm gcm;
	alt_aes aes;
	uint8_t key[32], iv[12], mac[20];
	size_t key_len;
	uint64_t seq;
	uint8_t secret[48];      /* TLS 1.3: the traffic secret (for key updates) */
} half;

/* a resumable session (Go's SessionState) */
typedef struct tls_session {
	uint16_t version, suite, group;  /* group: of the original handshake (reported on resumption, as Go) */
	bool ems;                        /* TLS 1.2: extended master secret */
	uint8_t secret[48];              /* TLS 1.2: the master secret; 1.3: the resumption PSK */
	size_t nsecret;
	int64_t created;                 /* unix seconds */
	/* the client side: what to send */
	uint8_t *ticket;
	size_t nticket;
	int64_t use_by;                  /* unix seconds: the end of the ticket lifetime */
	uint32_t age_add;
	/* the peer's certificates (DER, leaf first): restored on resumption */
	uint8_t *certs;                  /* the concatenation, each prefixed with a 3-byte length */
	size_t ncerts_bytes;
	char alpn[256];
} tls_session;

struct alt_tls_conn {
	alt_conn *conn;          /* owned when set */
	alt_reader rd;
	alt_writer wr;
	bool client;
	alt_tls_config cfg;
	alt_mutex hs_mu, in_mu, out_mu;
	bool done, hs_started;
	int hs_err;
	char err[256];
	half in, out;
	half pending_in;         /* TLS 1.2: activated by the peer's ChangeCipherSpec */
	bool has_pending_in;
	/* reading */
	uint8_t rec[5 + MAX_CIPHERTEXT];
	alt_buf hs;              /* handshake bytes not yet taken */
	uint8_t app[MAX_CIPHERTEXT];
	size_t app_off, app_n;
	bool eof;                /* close_notify received */
	bool raw_eof;            /* the connection ended at a record boundary */
	int read_err, write_err;
	bool closed_write;
	bool ccs_ok;             /* TLS 1.3: a compatibility ChangeCipherSpec may come */
	/* writing (out_mu held) */
	uint8_t wrec[5 + MAX_WRITE_CIPHERTEXT];
	uint8_t wpt[MAX_PLAINTEXT + 64];
	/* the result */
	uint16_t version, suite, group;
	bool did_resume, ems, secure_reneg;
	char alpn[256];
	char sni[256];
	alt_arena arena;         /* peer certificates, chains */
	const alt_x509_cert **peer;
	size_t npeer;
	alt_x509_chain *chains;
	size_t nchains;
	/* the handshake */
	alt_buf transcript;
	uint8_t master[48], hs_secret[48];
	uint8_t resumption[48];  /* TLS 1.3 resumption master secret (both sides, for tickets) */
	uint8_t client_random[32], server_random[32];
	bool key_update_pending;
	int tickets_sent;
};

/* ---- message building and reading ---- */

typedef struct {
	alt_buf *b;
	size_t open[8];
	int lens[8];
	int depth;
} tb;

static inline void tb_u8(tb *t, unsigned v) { alt_buf_append_byte(t->b, (int)(v & 0xff)); }
static inline void tb_u16(tb *t, unsigned v) {
	tb_u8(t, v >> 8);
	tb_u8(t, v);
}
static inline void tb_u24(tb *t, unsigned v) {
	tb_u8(t, v >> 16);
	tb_u16(t, v);
}
static inline void tb_u32(tb *t, uint32_t v) {
	tb_u16(t, v >> 16);
	tb_u16(t, v);
}
static inline void tb_bytes(tb *t, const void *p, size_t n) { alt_buf_append(t->b, p, n); }
static inline void tb_begin(tb *t, int lenbytes) {
	t->lens[t->depth] = lenbytes;
	for (int i = 0; i < lenbytes; i++)
		tb_u8(t, 0);
	t->open[t->depth++] = t->b->n;
}
static inline void tb_end(tb *t) {
	int lb = t->lens[--t->depth];
	size_t start = t->open[t->depth], n = t->b->n - start;
	if (t->b->err)
		return;
	for (int i = 0; i < lb; i++)
		t->b->p[start - lb + i] = (char)(n >> (8 * (lb - 1 - i)));
}

typedef struct {
	const uint8_t *p;
	size_t n;
} tr;

static inline bool tr_u8(tr *r, unsigned *v) {
	if (r->n < 1)
		return false;
	*v = r->p[0];
	r->p++, r->n--;
	return true;
}
static inline bool tr_u16(tr *r, unsigned *v) {
	if (r->n < 2)
		return false;
	*v = (unsigned)r->p[0] << 8 | r->p[1];
	r->p += 2, r->n -= 2;
	return true;
}
static inline bool tr_u24(tr *r, unsigned *v) {
	if (r->n < 3)
		return false;
	*v = (unsigned)r->p[0] << 16 | (unsigned)r->p[1] << 8 | r->p[2];
	r->p += 3, r->n -= 3;
	return true;
}
static inline bool tr_u32(tr *r, uint32_t *v) {
	unsigned a, b;
	if (!tr_u16(r, &a) || !tr_u16(r, &b))
		return false;
	*v = (uint32_t)a << 16 | b;
	return true;
}
static inline bool tr_take(tr *r, size_t n, tr *out) {
	if (r->n < n)
		return false;
	*out = (tr){r->p, n};
	r->p += n, r->n -= n;
	return true;
}
static inline bool tr_vec(tr *r, int lenbytes, tr *out) {
	unsigned n;
	bool ok = lenbytes == 1 ? tr_u8(r, &n) : lenbytes == 2 ? tr_u16(r, &n) : tr_u24(r, &n);
	return ok && tr_take(r, n, out);
}

/* ---- the record layer (record.c) ---- */

#define tls_fail alt__tls_fail
#define tls_fatal alt__tls_fatal
#define tls_send_alert alt__tls_send_alert
#define tls_write_record alt__tls_write_record
#define tls_write_records alt__tls_write_records
#define tls_read_record alt__tls_read_record
#define tls_on_alert alt__tls_on_alert
#define tls_read_handshake alt__tls_read_handshake
#define tls_expect alt__tls_expect
#define tls_send_handshake alt__tls_send_handshake
#define tls_send_ccs alt__tls_send_ccs
#define tls_keys_boundary alt__tls_keys_boundary
#define tls_alert_text alt__tls_alert_text

int tls_fail(alt_tls_conn *t, int code, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
/* a fatal error during the handshake: the alert (if >= 0), the message */
int tls_fatal(alt_tls_conn *t, int alert, int code, const char *msg);
void tls_send_alert(alt_tls_conn *t, int level, int desc);
const char *tls_alert_text(int a);
/* one record (out_mu held), data up to MAX_PLAINTEXT */
int tls_write_record(alt_tls_conn *t, int type, const void *data, size_t n);
int tls_write_records(alt_tls_conn *t, int type, const void *data, size_t n);
/* one record (in_mu held): its type and plaintext (in t->rec) */
int tls_read_record(alt_tls_conn *t, int *type, uint8_t **data, size_t *n);
/* close_notify: 0 (t->eof set); a warning to ignore: 1; fatal: an error */
int tls_on_alert(alt_tls_conn *t, const uint8_t *d, size_t n);
/* the next handshake message (header included) */
int tls_read_handshake(alt_tls_conn *t, int *type, alt_buf *msg);
/* a handshake message of the given type: its body in *body (msg holds it) */
int tls_expect(alt_tls_conn *t, int want, alt_buf *msg, tr *body);
/* sends a handshake message and appends it to the transcript */
int tls_send_handshake(alt_tls_conn *t, const alt_buf *msg);
int tls_send_ccs(alt_tls_conn *t);
/* keys may change only at message boundaries */
int tls_keys_boundary(alt_tls_conn *t);

/* ---- the common parts (common.c) ---- */

typedef struct {
	uint16_t group;
	uint8_t priv[66], pub[133];
	size_t npub;
} keyshare;

typedef struct {
	tr random, sid, suites, comp, exts;
	unsigned legacy_version;
	char sni[256];
	bool has_sni;
	uint16_t groups[32], schemes[64], versions[16];
	size_t ngroups, nschemes, nversions;
	bool has_versions, has_groups, has_point_formats, uncompressed_points, has_alpn;
	struct {
		uint16_t group;
		tr key;
	} shares[16];
	size_t nshares;
	char alpn[16][256];
	size_t nalpn;
	tr cookie;
	bool has_cookie, has_psk_modes, psk_dhe_ke;
	bool ems, reneg_info, has_ticket_ext, early_data;
	tr reneg, ticket;
	/* pre_shared_key: the identities and binders (must be the last extension) */
	struct {
		tr identity;
		uint32_t age;
	} psk[8];
	size_t npsk;
	tr binders;
	size_t binders_off;              /* the size of the binders list ending the message (cut for the binder hash) */
} client_hello;

typedef struct {
	unsigned legacy_version;
	uint8_t random[32];
	tr sid;
	unsigned suite, version, group;
	tr share, cookie, alpn, reneg;
	bool has_version, has_share, has_cookie, has_alpn, has_reneg, ems, ticket, has_psk, has_point_formats;
	unsigned psk_index;
} server_hello;

/* what a client sends in its hello (version range, offers) */
typedef struct {
	uint16_t min_version, max_version;
	uint8_t random[32], sid[32];
	size_t nsid;
	keyshare share;
	tls_session *session;            /* being resumed (client) */
	uint8_t binder_key[48];
	uint16_t psk_suite;
} hello_offer;

#define tls_sig_schemes alt__tls_sig_schemes
#define tls_nsig_schemes alt__tls_nsig_schemes
#define tls_make_share alt__tls_make_share
#define tls_shared_secret alt__tls_shared_secret
#define tls_group_supported alt__tls_group_supported
#define tls_groups_of alt__tls_groups_of
#define tls_suites13 alt__tls_suites13
#define tls_suites12 alt__tls_suites12
#define tls_aes_hardware alt__tls_aes_hardware
#define tls_aes_preferred alt__tls_aes_preferred
#define tls_scheme_hash alt__tls_scheme_hash
#define tls_pick_scheme alt__tls_pick_scheme
#define tls_sign alt__tls_sign
#define tls_verify_sig alt__tls_verify_sig
#define tls_read_certificate alt__tls_read_certificate
#define tls_send_certificate alt__tls_send_certificate
#define tls_verify_peer alt__tls_verify_peer
#define tls_build_client_hello alt__tls_build_client_hello
#define tls_parse_client_hello alt__tls_parse_client_hello
#define tls_parse_server_hello alt__tls_parse_server_hello
#define tls_choose_cert alt__tls_choose_cert
#define tls_negotiate_alpn alt__tls_negotiate_alpn
#define tls_version_range alt__tls_version_range
#define tls_check_alpn alt__tls_check_alpn
#define tls_ct_zero alt__tls_ct_zero
#define tls_suite_is12 alt__tls_suite_is12

extern const uint16_t tls_sig_schemes[];
extern const size_t tls_nsig_schemes;

int tls_make_share(keyshare *k, uint16_t g);
int tls_shared_secret(const keyshare *k, const uint8_t *peer, size_t n, uint8_t *out, size_t *nout);
bool tls_group_supported(uint16_t g);
size_t tls_groups_of(const alt_tls_config *cfg, uint16_t *out);
/* the suites to use, in preference order (aes: AES-GCM first) */
size_t tls_suites13(const alt_tls_config *cfg, bool aes, uint16_t *out);
size_t tls_suites12(const alt_tls_config *cfg, bool aes, uint16_t *out);
bool tls_suite_is12(uint16_t s);
bool tls_aes_hardware(void);
/* Go's isAESGCMPreferred over the client's list (2-byte big-endian ids) */
bool tls_aes_preferred(const tr *suites);
/* the configured version range: 0 if empty */
int tls_version_range(const alt_tls_config *cfg, uint16_t *min, uint16_t *max);

alt_hash_kind tls_scheme_hash(uint16_t s);
/* the scheme for our key in the peer's preference order (Go 1.25+), for the version: 0 if none */
uint16_t tls_pick_scheme(const alt_x509_private_key *k, uint16_t version, const uint16_t *peer, size_t np);
/* signs content (the whole message: hashed inside unless Ed25519) */
int tls_sign(const alt_x509_private_key *k, uint16_t scheme, const uint8_t *content, size_t n, uint8_t *sig, size_t *nsig);
int tls_verify_sig(const alt_x509_cert *c, uint16_t scheme, bool v12, const uint8_t *content, size_t n, const uint8_t *sig,
	size_t ns);

/* Certificate: the 1.3 form has a context and per-entry extensions */
int tls_read_certificate(alt_tls_conn *t, tr body, bool v13, bool allow_empty);
int tls_send_certificate(alt_tls_conn *t, const alt_tls_cert *c, bool v13, const tr *context);
int tls_verify_peer(alt_tls_conn *t, const char *name, const alt_x509_pool *roots, int eku);

int tls_build_client_hello(alt_tls_conn *t, alt_buf *m, const hello_offer *o, const tr *cookie);
int tls_parse_client_hello(alt_tls_conn *t, tr body, client_hello *ch);
int tls_parse_server_hello(alt_tls_conn *t, tr body, server_hello *sh);
const alt_tls_cert *tls_choose_cert(alt_tls_conn *t, const client_hello *ch, alt_arena *a);
/* the server's ALPN choice into t->alpn: an error (after the alert) when there is no overlap */
int tls_negotiate_alpn(alt_tls_conn *t, const client_hello *ch);
/* the client's check of the server's ALPN answer */
int tls_check_alpn(alt_tls_conn *t, const tr *proto);

/* ---- the handshakes ---- */

#define tls_client13 alt__tls_client13
#define tls_server13 alt__tls_server13
#define tls_client12 alt__tls_client12
#define tls_server12 alt__tls_server12
#define tls_client_handshake alt__tls_client_handshake
#define tls_server_handshake alt__tls_server_handshake
#define tls_post_handshake13 alt__tls_post_handshake13
#define tls_set_keys13 alt__tls_set_keys13
#define tls_next_secret13 alt__tls_next_secret13
#define tls_client_binder alt__tls_client_binder

int tls_client_handshake(alt_tls_conn *t);
int tls_server_handshake(alt_tls_conn *t);
/* continue after the ServerHello (in holds it, already parsed into sh) */
int tls_client13(alt_tls_conn *t, hello_offer *o, server_hello *sh, alt_buf *in, size_t ch1_len);
int tls_client12(alt_tls_conn *t, hello_offer *o, server_hello *sh, alt_buf *in);
/* continue after the ClientHello (raw holds it, ch points into it) */
int tls_server13(alt_tls_conn *t, client_hello *ch, alt_buf *raw);
int tls_server12(alt_tls_conn *t, client_hello *ch, alt_buf *raw);
/* a post-handshake message in TLS 1.3 (tickets, key updates) */
int tls_post_handshake13(alt_tls_conn *t, int type, alt_buf *msg);
void tls_set_keys13(half *x, uint16_t suite, const uint8_t *secret);
void tls_next_secret13(half *x);
/* fills the PSK binder at the end of a hello built with a zero binder (when a TLS 1.3 session is offered) */
void tls_client_binder(alt_tls_conn *t, hello_offer *o, alt_buf *m);

/* ---- sessions (session.c) ---- */

#define tls_session_free alt__tls_session_free
#define tls_cache_get alt__tls_cache_get
#define tls_cache_put alt__tls_cache_put
#define tls_cache_remove alt__tls_cache_remove
#define tls_seal_ticket alt__tls_seal_ticket
#define tls_open_ticket alt__tls_open_ticket
#define tls_session_peer_certs alt__tls_session_peer_certs
#define tls_session_set_certs alt__tls_session_set_certs
#define tls_cache_key alt__tls_cache_key

void tls_session_free(tls_session *s);
/* the cache key: the server name or the address */
void tls_cache_key(const alt_tls_conn *t, char *key, size_t n);
/* a copy of the cached session (the caller frees it) or NULL */
tls_session *tls_cache_get(alt_tls_session_cache *c, const char *key);
/* takes s */
void tls_cache_put(alt_tls_session_cache *c, const char *key, tls_session *s);
void tls_cache_remove(alt_tls_session_cache *c, const char *key);
/* the server's tickets: the session encrypted with the configured (or a process-wide random) key */
int tls_seal_ticket(alt_tls_conn *t, const tls_session *s, alt_buf *out);
/* a session from a ticket, or NULL (bad, expired or foreign tickets are ignored) */
tls_session *tls_open_ticket(alt_tls_conn *t, const uint8_t *p, size_t n);
/* the peer's chain of the session into t->peer (parsed into t->arena) */
int tls_session_peer_certs(alt_tls_conn *t, const tls_session *s);
/* the peer's chain of t into s */
int tls_session_set_certs(tls_session *s, const alt_tls_conn *t);

#endif
