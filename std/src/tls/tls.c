/* TLS 1.3 (RFC 8446): the record layer, the key schedule, the client
   and server handshakes. Behaviour follows Go's crypto/tls where the RFC
   leaves room (preferences, compatibility mode, alerts, messages). */
#include <alt/crypto/aes.h>
#include <alt/crypto/chacha20poly1305.h>
#include <alt/crypto/ecdsa.h>
#include <alt/crypto/ed25519.h>
#include <alt/crypto/kdf.h>
#include <alt/crypto/rsa.h>
#include <alt/crypto/tls.h>
#include <alt/encoding/pem.h>
#include <alt/sync.h>
#include <cpuid.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	REC_CCS = 20, REC_ALERT = 21, REC_HANDSHAKE = 22, REC_APP = 23,
	HS_CLIENT_HELLO = 1, HS_SERVER_HELLO = 2, HS_NEW_SESSION_TICKET = 4, HS_ENCRYPTED_EXTENSIONS = 8, HS_CERTIFICATE = 11,
	HS_CERTIFICATE_REQUEST = 13, HS_CERTIFICATE_VERIFY = 15, HS_FINISHED = 20, HS_KEY_UPDATE = 24, HS_MESSAGE_HASH = 254,
	EXT_SERVER_NAME = 0, EXT_SUPPORTED_GROUPS = 10, EXT_SIGNATURE_ALGORITHMS = 13, EXT_ALPN = 16, EXT_SUPPORTED_VERSIONS = 43,
	EXT_COOKIE = 44, EXT_PSK_MODES = 45, EXT_CERT_AUTHORITIES = 47, EXT_SIG_ALGS_CERT = 50, EXT_KEY_SHARE = 51,
	MAX_PLAINTEXT = 16384, MAX_CIPHERTEXT = 16384 + 256, VERSION_13 = 0x0304,
};

/* alerts */
enum {
	A_CLOSE_NOTIFY = 0, A_UNEXPECTED_MESSAGE = 10, A_BAD_RECORD_MAC = 20, A_RECORD_OVERFLOW = 22, A_HANDSHAKE_FAILURE = 40,
	A_BAD_CERTIFICATE = 42, A_UNSUPPORTED_CERTIFICATE = 43, A_CERTIFICATE_EXPIRED = 45, A_CERTIFICATE_UNKNOWN = 46,
	A_ILLEGAL_PARAMETER = 47, A_UNKNOWN_CA = 48, A_DECODE_ERROR = 50, A_DECRYPT_ERROR = 51, A_PROTOCOL_VERSION = 70,
	A_INTERNAL_ERROR = 80, A_MISSING_EXTENSION = 109, A_UNSUPPORTED_EXTENSION = 110, A_UNRECOGNIZED_NAME = 112,
	A_CERTIFICATE_REQUIRED = 116, A_NO_APPLICATION_PROTOCOL = 120,
};

/* signature schemes */
enum {
	S_RSA_PKCS1_SHA256 = 0x0401, S_RSA_PKCS1_SHA384 = 0x0501, S_RSA_PKCS1_SHA512 = 0x0601, S_ECDSA_P256 = 0x0403,
	S_ECDSA_P384 = 0x0503, S_ECDSA_P521 = 0x0603, S_RSA_PSS_256 = 0x0804, S_RSA_PSS_384 = 0x0805, S_RSA_PSS_512 = 0x0806,
	S_ED25519 = 0x0807, S_RSA_PSS_PSS_256 = 0x0809, S_RSA_PSS_PSS_384 = 0x080a, S_RSA_PSS_PSS_512 = 0x080b, S_RSA_PKCS1_SHA1 = 0x0201,
	S_ECDSA_SHA1 = 0x0203,
};

/* what this implementation signs and accepts in CertificateVerify (Go's order) */
static const uint16_t sig_schemes[] = {S_ECDSA_P256, S_ED25519, S_RSA_PSS_256, S_RSA_PSS_384, S_RSA_PSS_512, S_RSA_PKCS1_SHA256,
	S_RSA_PKCS1_SHA384, S_RSA_PKCS1_SHA512, S_ECDSA_P384, S_ECDSA_P521, S_RSA_PKCS1_SHA1, S_ECDSA_SHA1};

typedef struct {
	int on;                  /* 0: plaintext */
	uint16_t suite;
	alt_gcm gcm;
	uint8_t key[32], iv[12];
	uint64_t seq;
	uint8_t secret[48];
} half;

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
	/* reading */
	uint8_t rec[5 + MAX_CIPHERTEXT];
	alt_buf hs;              /* handshake bytes not yet taken */
	uint8_t app[MAX_CIPHERTEXT];
	size_t app_off, app_n;
	bool eof;                /* close_notify received */
	int read_err, write_err;
	bool closed_write;
	bool ccs_ok;             /* a compatibility ChangeCipherSpec may come */
	/* the result */
	uint16_t suite, group;
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
	bool key_update_pending;
};

/* ---- errors ---- */

static int fail(alt_tls_conn *t, int code, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
static int fail(alt_tls_conn *t, int code, const char *fmt, ...) {
	if (!t->err[0]) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(t->err, sizeof t->err, fmt, ap);
		va_end(ap);
	}
	return code;
}

const char *alt_tls_error(const alt_tls_conn *t) { return t->err; }

static const char *alert_text(int a) {
	switch (a) {
	case A_CLOSE_NOTIFY: return "close notify";
	case A_UNEXPECTED_MESSAGE: return "unexpected message";
	case A_BAD_RECORD_MAC: return "bad record MAC";
	case A_RECORD_OVERFLOW: return "record overflow";
	case A_HANDSHAKE_FAILURE: return "handshake failure";
	case A_BAD_CERTIFICATE: return "bad certificate";
	case A_UNSUPPORTED_CERTIFICATE: return "unsupported certificate";
	case A_CERTIFICATE_EXPIRED: return "expired certificate";
	case A_CERTIFICATE_UNKNOWN: return "unknown certificate";
	case A_ILLEGAL_PARAMETER: return "illegal parameter";
	case A_UNKNOWN_CA: return "unknown certificate authority";
	case A_DECODE_ERROR: return "error decoding message";
	case A_DECRYPT_ERROR: return "error decrypting message";
	case A_PROTOCOL_VERSION: return "protocol version not supported";
	case A_INTERNAL_ERROR: return "internal error";
	case A_MISSING_EXTENSION: return "missing extension";
	case A_UNSUPPORTED_EXTENSION: return "unsupported extension";
	case A_UNRECOGNIZED_NAME: return "unrecognized name";
	case A_CERTIFICATE_REQUIRED: return "certificate required";
	case A_NO_APPLICATION_PROTOCOL: return "no application protocol";
	}
	return "alert";
}

/* ---- message building and reading ---- */

typedef struct {
	alt_buf *b;
	size_t open[8];
	int lens[8];
	int depth;
} tb;

static void tb_u8(tb *t, unsigned v) { alt_buf_append_byte(t->b, (int)(v & 0xff)); }
static void tb_u16(tb *t, unsigned v) {
	tb_u8(t, v >> 8);
	tb_u8(t, v);
}
static void tb_u24(tb *t, unsigned v) {
	tb_u8(t, v >> 16);
	tb_u16(t, v);
}
static void tb_bytes(tb *t, const void *p, size_t n) { alt_buf_append(t->b, p, n); }
static void tb_begin(tb *t, int lenbytes) {
	t->lens[t->depth] = lenbytes;
	for (int i = 0; i < lenbytes; i++)
		tb_u8(t, 0);
	t->open[t->depth++] = t->b->n;
}
static void tb_end(tb *t) {
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

static bool tr_u8(tr *r, unsigned *v) {
	if (r->n < 1)
		return false;
	*v = r->p[0];
	r->p++, r->n--;
	return true;
}
static bool tr_u16(tr *r, unsigned *v) {
	if (r->n < 2)
		return false;
	*v = (unsigned)r->p[0] << 8 | r->p[1];
	r->p += 2, r->n -= 2;
	return true;
}
static bool tr_u24(tr *r, unsigned *v) {
	if (r->n < 3)
		return false;
	*v = (unsigned)r->p[0] << 16 | (unsigned)r->p[1] << 8 | r->p[2];
	r->p += 3, r->n -= 3;
	return true;
}
static bool tr_take(tr *r, size_t n, tr *out) {
	if (r->n < n)
		return false;
	*out = (tr){r->p, n};
	r->p += n, r->n -= n;
	return true;
}
static bool tr_vec(tr *r, int lenbytes, tr *out) {
	unsigned n;
	bool ok = lenbytes == 1 ? tr_u8(r, &n) : lenbytes == 2 ? tr_u16(r, &n) : tr_u24(r, &n);
	return ok && tr_take(r, n, out);
}

/* ---- suites, hashes, keys ---- */

static alt_hash_kind suite_hash(uint16_t s) { return s == ALT_TLS_AES_256_GCM_SHA384 ? ALT_SHA384 : ALT_SHA256; }
static size_t suite_key_len(uint16_t s) { return s == ALT_TLS_AES_128_GCM_SHA256 ? 16 : 32; }

static void expand_label(alt_hash_kind h, const uint8_t *secret, const char *label, const void *ctx, size_t nctx, uint8_t *out,
	size_t n) {
	uint8_t info[512];
	size_t ll = strlen(label), k = 0;
	info[k++] = (uint8_t)(n >> 8);
	info[k++] = (uint8_t)n;
	info[k++] = (uint8_t)(6 + ll);
	memcpy(info + k, "tls13 ", 6);
	k += 6;
	memcpy(info + k, label, ll);
	k += ll;
	info[k++] = (uint8_t)nctx;
	memcpy(info + k, ctx, nctx);
	k += nctx;
	alt_hkdf_expand(h, secret, alt_hash_size(h), info, k, out, n);
}

static void transcript_hash(alt_tls_conn *t, uint8_t *out) { alt_hash_data(suite_hash(t->suite), t->transcript.p, t->transcript.n, out); }

static void derive_secret(alt_tls_conn *t, const uint8_t *secret, const char *label, bool with_transcript, uint8_t *out) {
	alt_hash_kind h = suite_hash(t->suite);
	uint8_t th[ALT_HASH_MAX_SIZE];
	if (with_transcript)
		transcript_hash(t, th);
	else
		alt_hash_data(h, "", 0, th);
	expand_label(h, secret, label, th, alt_hash_size(h), out, alt_hash_size(h));
}

static void set_keys(half *x, uint16_t suite, const uint8_t *secret) {
	alt_hash_kind h = suite_hash(suite);
	size_t hl = alt_hash_size(h);
	memcpy(x->secret, secret, hl);
	x->suite = suite;
	expand_label(h, secret, "key", "", 0, x->key, suite_key_len(suite));
	expand_label(h, secret, "iv", "", 0, x->iv, 12);
	if (suite != ALT_TLS_CHACHA20_POLY1305_SHA256)
		alt_gcm_init(&x->gcm, x->key, suite_key_len(suite));
	x->seq = 0;
	x->on = 1;
}

static void next_secret(half *x) {
	alt_hash_kind h = suite_hash(x->suite);
	uint8_t s[48];
	expand_label(h, x->secret, "traffic upd", "", 0, s, alt_hash_size(h));
	set_keys(x, x->suite, s);
}

static void nonce(const half *x, uint8_t n[12]) {
	memcpy(n, x->iv, 12);
	for (int i = 0; i < 8; i++)
		n[4 + i] ^= (uint8_t)(x->seq >> (56 - 8 * i));
}

/* ---- the record layer ---- */

static int write_raw(alt_tls_conn *t, const void *p, size_t n) {
	if (t->write_err)
		return t->write_err;
	ssize_t w = alt_write_all(t->wr, p, n);
	if (w < 0) {
		fail(t, (int)w, "tls: write: %s", alt_strerror((int)w));
		return t->write_err = (int)w;
	}
	return 0;
}

/* one record (out_mu held); data up to MAX_PLAINTEXT */
static int write_record(alt_tls_conn *t, int type, const void *data, size_t n) {
	uint8_t rec[5 + MAX_CIPHERTEXT];
	if (!t->out.on) {
		rec[0] = (uint8_t)type, rec[1] = 3, rec[2] = type == REC_HANDSHAKE && t->client && !t->done ? 1 : 3;
		rec[3] = (uint8_t)(n >> 8), rec[4] = (uint8_t)n;
		memcpy(rec + 5, data, n);
		return write_raw(t, rec, 5 + n);
	}
	size_t inner = n + 1, total = inner + 16;
	rec[0] = REC_APP, rec[1] = 3, rec[2] = 3, rec[3] = (uint8_t)(total >> 8), rec[4] = (uint8_t)total;
	uint8_t pt[MAX_PLAINTEXT + 1];
	memcpy(pt, data, n);
	pt[n] = (uint8_t)type;
	uint8_t nn[12];
	nonce(&t->out, nn);
	if (t->out.suite == ALT_TLS_CHACHA20_POLY1305_SHA256)
		alt_chacha20poly1305_seal(t->out.key, nn, rec + 5, pt, inner, rec, 5);
	else
		alt_gcm_seal(&t->out.gcm, rec + 5, nn, 12, pt, inner, rec, 5);
	t->out.seq++;
	memset(pt, 0, n);
	return write_raw(t, rec, 5 + total);
}

static int write_records(alt_tls_conn *t, int type, const void *data, size_t n) {
	const uint8_t *p = data;
	do {
		size_t k = n < MAX_PLAINTEXT ? n : MAX_PLAINTEXT;
		int rc = write_record(t, type, p, k);
		if (rc < 0)
			return rc;
		p += k, n -= k;
	} while (n);
	return 0;
}

static void send_alert(alt_tls_conn *t, int level, int desc) {
	uint8_t a[2] = {(uint8_t)level, (uint8_t)desc};
	alt_mutex_lock(&t->out_mu);
	if (!t->write_err)
		write_record(t, REC_ALERT, a, 2);
	alt_mutex_unlock(&t->out_mu);
}

/* a fatal error during the handshake: the alert, the message */
static int fatal(alt_tls_conn *t, int alert, int code, const char *msg) {
	if (alert >= 0)
		send_alert(t, 2, alert);
	fail(t, code, "%s", msg);
	return code;
}

static ssize_t read_full(alt_reader r, void *buf, size_t n) {
	size_t got = 0;
	while (got < n) {
		ssize_t k = alt_read(r, (char *)buf + got, n - got);
		if (k < 0)
			return k;
		if (k == 0)
			return got ? -ALT_EUNEXPECTED_EOF : 0;
		got += (size_t)k;
	}
	return (ssize_t)got;
}

/* reads one record (in_mu held): its type and plaintext in *data
   (pointing into t->rec), or an error (after sending an alert) */
static int read_record(alt_tls_conn *t, int *type, uint8_t **data, size_t *n) {
	for (;;) {
		ssize_t k = read_full(t->rd, t->rec, 5);
		if (k < 0)
			return t->read_err = fail(t, (int)k, "tls: %s", alt_strerror((int)k));
		if (k == 0)
			return t->read_err = fail(t, -ALT_EUNEXPECTED_EOF, "EOF");
		int ty = t->rec[0];
		size_t len = (size_t)t->rec[3] << 8 | t->rec[4];
		if (ty < REC_CCS || ty > REC_APP) {
			/* not TLS: Go's "first record does not look like a TLS handshake" */
			send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, !t->done && !t->in.on ? "tls: first record does not look like a TLS handshake"
											: "tls: unexpected record type");
		}
		if (len > (t->in.on ? (size_t)MAX_CIPHERTEXT : (size_t)MAX_PLAINTEXT)) {
			send_alert(t, 2, A_RECORD_OVERFLOW);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: oversized record received with length %zu", len);
		}
		k = read_full(t->rd, t->rec + 5, len);
		if (k < 0 || (size_t)k != len)
			return t->read_err = fail(t, k < 0 ? (int)k : -ALT_EUNEXPECTED_EOF, "unexpected EOF");
		uint8_t *body = t->rec + 5;
		if (ty == REC_CCS) {
			/* compatibility mode (RFC 8446 D.4): one byte 1, only during the handshake, never encrypted */
			if (!t->ccs_ok || len != 1 || body[0] != 1) {
				send_alert(t, 2, A_UNEXPECTED_MESSAGE);
				return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: unexpected change cipher spec");
			}
			continue;
		}
		if (!t->in.on) {
			if (ty == REC_APP) {
				send_alert(t, 2, A_UNEXPECTED_MESSAGE);
				return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: unexpected application data");
			}
			*type = ty, *data = body, *n = len;
			return 0;
		}
		if (ty != REC_APP) {
			send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: unencrypted record after the keys changed");
		}
		uint8_t nn[12];
		nonce(&t->in, nn);
		int rc = len < 17 ? -ALT_EAUTH
				  : t->in.suite == ALT_TLS_CHACHA20_POLY1305_SHA256 ? alt_chacha20poly1305_open(t->in.key, nn, body, body, len, t->rec, 5)
										   : alt_gcm_open(&t->in.gcm, body, nn, 12, body, len, t->rec, 5);
		if (rc < 0) {
			send_alert(t, 2, A_BAD_RECORD_MAC);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: invalid record MAC");
		}
		t->in.seq++;
		size_t m = len - 16;
		while (m > 0 && body[m - 1] == 0)
			m--;
		if (m == 0) {
			send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: record without a content type");
		}
		*type = body[m - 1];
		*data = body;
		*n = m - 1;
		if (*n > MAX_PLAINTEXT) {
			send_alert(t, 2, A_RECORD_OVERFLOW);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: oversized record");
		}
		if (*type == REC_CCS) {
			send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: encrypted change cipher spec");
		}
		return 0;
	}
}

/* an alert record: close_notify is the end of the data, the others are fatal */
static int on_alert(alt_tls_conn *t, const uint8_t *d, size_t n) {
	if (n != 2) {
		send_alert(t, 2, A_DECODE_ERROR);
		return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: bad alert");
	}
	if (d[1] == A_CLOSE_NOTIFY) {
		t->eof = true;
		return 0;
	}
	if (d[1] == 90) /* user_canceled: a warning to ignore */
		return 1;
	return t->read_err = fail(t, -ALT_ETLS_ALERT, "remote error: tls: %s", alert_text(d[1]));
}

/* the next handshake message (type, body), appended to the transcript
   unless it is a post-handshake message */
static int read_handshake(alt_tls_conn *t, int *type, alt_buf *msg) {
	for (;;) {
		if (t->hs.n >= 4) {
			size_t len = (size_t)(uint8_t)t->hs.p[1] << 16 | (size_t)(uint8_t)t->hs.p[2] << 8 | (uint8_t)t->hs.p[3];
			if (len > 1 << 18)
				return fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: handshake message too large");
			if (t->hs.n >= 4 + len) {
				*type = (uint8_t)t->hs.p[0];
				alt_buf_reset(msg);
				alt_buf_append(msg, t->hs.p, 4 + len);
				alt_buf_consume(&t->hs, 4 + len);
				return 0;
			}
		}
		int ty;
		uint8_t *d;
		size_t n;
		int rc = read_record(t, &ty, &d, &n);
		if (rc < 0)
			return rc;
		if (ty == REC_ALERT) {
			rc = on_alert(t, d, n);
			if (rc < 0)
				return rc;
			if (rc == 0)
				return t->read_err = fail(t, -ALT_EUNEXPECTED_EOF, "EOF");
			continue;
		}
		if (ty != REC_HANDSHAKE)
			return fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected record during the handshake");
		if (n == 0)
			return fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: empty handshake record");
		alt_buf_append(&t->hs, d, n);
	}
}

/* a handshake message of the given type: the body in *body (msg holds it) */
static int expect(alt_tls_conn *t, int want, alt_buf *msg, tr *body) {
	int ty, rc = read_handshake(t, &ty, msg);
	if (rc < 0)
		return rc;
	if (ty != want) {
		char m[96];
		snprintf(m, sizeof m, "tls: unexpected handshake message type %d (wanted %d)", ty, want);
		return fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, m);
	}
	*body = (tr){(const uint8_t *)msg->p + 4, msg->n - 4};
	return 0;
}

static int send_handshake(alt_tls_conn *t, const alt_buf *msg) {
	alt_buf_append(&t->transcript, msg->p, msg->n);
	alt_mutex_lock(&t->out_mu);
	int rc = write_records(t, REC_HANDSHAKE, msg->p, msg->n);
	alt_mutex_unlock(&t->out_mu);
	return rc;
}

/* keys may change only at message boundaries (RFC 8446 5.1) */
static int keys_boundary(alt_tls_conn *t) {
	if (t->hs.n)
		return fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: handshake message spans a key change");
	return 0;
}

/* ---- key exchange ---- */

typedef struct {
	uint16_t group;
	uint8_t priv[66], pub[133];
	size_t npub;
} keyshare;

static alt_ec_curve curve_of_group(uint16_t g) {
	return g == ALT_TLS_P256 ? ALT_P256 : g == ALT_TLS_P384 ? ALT_P384 : g == ALT_TLS_P521 ? ALT_P521 : 0;
}

static int make_share(keyshare *k, uint16_t g) {
	k->group = g;
	if (g == ALT_TLS_X25519) {
		int rc = alt_crypto_rand(k->priv, 32);
		if (rc < 0)
			return rc;
		alt_x25519_base(k->pub, k->priv);
		k->npub = 32;
		return 0;
	}
	alt_ec_curve c = curve_of_group(g);
	k->npub = alt_ec_point_size(c);
	return alt_ec_keygen(c, k->priv, k->pub);
}

static int shared_secret(const keyshare *k, const uint8_t *peer, size_t n, uint8_t *out, size_t *nout) {
	if (k->group == ALT_TLS_X25519) {
		*nout = 32;
		return n == 32 ? alt_x25519(out, k->priv, peer) : -ALT_EKEY;
	}
	alt_ec_curve c = curve_of_group(k->group);
	*nout = alt_ec_scalar_size(c);
	return alt_ecdh(c, k->priv, peer, n, out);
}

static bool group_supported(uint16_t g) { return g == ALT_TLS_X25519 || g == ALT_TLS_P256 || g == ALT_TLS_P384 || g == ALT_TLS_P521; }
static bool suite_supported(uint16_t s) { return s >= 0x1301 && s <= 0x1303; }

static const uint16_t default_groups[] = {ALT_TLS_X25519, ALT_TLS_P256, ALT_TLS_P384, ALT_TLS_P521};

static bool aes_hardware(void) {
	unsigned a, b, c, d;
	return __get_cpuid(1, &a, &b, &c, &d) && (c & bit_AES) && (c & bit_PCLMUL);
}

/* Go's default order: AES-GCM first with AES hardware, ChaCha20 first without */
static size_t suites_of(const alt_tls_config *cfg, uint16_t *out) {
	if (cfg->cipher_suites && cfg->nsuites) {
		size_t n = 0;
		for (size_t i = 0; i < cfg->nsuites && n < 8; i++)
			if (suite_supported(cfg->cipher_suites[i]))
				out[n++] = cfg->cipher_suites[i];
		return n;
	}
	if (aes_hardware()) {
		out[0] = ALT_TLS_AES_128_GCM_SHA256, out[1] = ALT_TLS_AES_256_GCM_SHA384, out[2] = ALT_TLS_CHACHA20_POLY1305_SHA256;
	} else {
		out[0] = ALT_TLS_CHACHA20_POLY1305_SHA256, out[1] = ALT_TLS_AES_128_GCM_SHA256, out[2] = ALT_TLS_AES_256_GCM_SHA384;
	}
	return 3;
}

static size_t groups_of(const alt_tls_config *cfg, uint16_t *out) {
	const uint16_t *g = cfg->groups && cfg->ngroups ? cfg->groups : default_groups;
	size_t ng = cfg->groups && cfg->ngroups ? cfg->ngroups : 4, n = 0;
	for (size_t i = 0; i < ng && n < 8; i++)
		if (group_supported(g[i]))
			out[n++] = g[i];
	return n;
}

/* ---- signatures (CertificateVerify) ---- */

static const uint8_t *cv_context(bool server, size_t *n) {
	static uint8_t s[98], c[98];
	static bool init;
	if (!init) {
		memset(s, ' ', 64);
		memcpy(s + 64, "TLS 1.3, server CertificateVerify", 33);
		memset(c, ' ', 64);
		memcpy(c + 64, "TLS 1.3, client CertificateVerify", 33);
		init = true;
	}
	*n = 64 + 33;
	return server ? s : c;
}

static alt_hash_kind scheme_hash(uint16_t s) {
	switch (s) {
	case S_ECDSA_P384: case S_RSA_PSS_384: case S_RSA_PSS_PSS_384: case S_RSA_PKCS1_SHA384: return ALT_SHA384;
	case S_ECDSA_P521: case S_RSA_PSS_512: case S_RSA_PSS_PSS_512: case S_RSA_PKCS1_SHA512: return ALT_SHA512;
	case S_RSA_PKCS1_SHA1: case S_ECDSA_SHA1: return ALT_SHA1;
	default: return ALT_SHA256;
	}
}

/* the signed content: 64 spaces, the context string, 0, the transcript hash */
static size_t signed_content(alt_tls_conn *t, bool server, uint8_t *out) {
	size_t n;
	const uint8_t *ctx = cv_context(server, &n);
	memcpy(out, ctx, n);
	out[n] = 0;
	transcript_hash(t, out + n + 1);
	return n + 1 + alt_hash_size(suite_hash(t->suite));
}

/* the scheme for our key, among the peer's: 0 if none */
static uint16_t pick_scheme(const alt_x509_private_key *k, const uint16_t *peer, size_t np) {
	uint16_t mine[3];
	size_t nm = 0;
	switch (k->alg) {
	case ALT_X509_PK_ED25519: mine[nm++] = S_ED25519; break;
	case ALT_X509_PK_ECDSA:
		mine[nm++] = k->curve == ALT_X509_P256 ? S_ECDSA_P256 : k->curve == ALT_X509_P384 ? S_ECDSA_P384 : S_ECDSA_P521;
		break;
	case ALT_X509_PK_RSA:
		mine[nm++] = S_RSA_PSS_256, mine[nm++] = S_RSA_PSS_384, mine[nm++] = S_RSA_PSS_512;
		break;
	default: return 0;
	}
	for (size_t i = 0; i < nm; i++)
		for (size_t k2 = 0; k2 < np; k2++)
			if (peer[k2] == mine[i])
				return mine[i];
	return 0;
}

static int sign(const alt_x509_private_key *k, uint16_t scheme, const uint8_t *content, size_t n, uint8_t *sig, size_t *nsig) {
	alt_hash_kind h = scheme_hash(scheme);
	uint8_t d[ALT_HASH_MAX_SIZE];
	if (scheme == S_ED25519) {
		alt_ed25519_sign(sig, k->ed25519, content, n);
		*nsig = 64;
		return 0;
	}
	alt_hash_data(h, content, n, d);
	if (k->alg == ALT_X509_PK_ECDSA) {
		int rc = alt_ecdsa_sign((alt_ec_curve)k->curve, k->ec_priv, d, alt_hash_size(h), sig);
		if (rc < 0)
			return rc;
		*nsig = (size_t)rc;
		return 0;
	}
	alt_rsa_private rk = {{k->rsa.n.p, k->rsa.n.n, k->rsa.e}, k->rsa.d.p, k->rsa.p.p, k->rsa.q.p, k->rsa.dp.p, k->rsa.dq.p,
		k->rsa.qinv.p, k->rsa.d.n, k->rsa.p.n, k->rsa.q.n, k->rsa.dp.n, k->rsa.dq.n, k->rsa.qinv.n};
	*nsig = k->rsa.n.n;
	return alt_rsa_sign_pss(&rk, h, d, alt_hash_size(h), sig, -1);
}

/* checks the peer's CertificateVerify signature with its leaf certificate */
static int verify_sig(const alt_x509_cert *c, uint16_t scheme, const uint8_t *content, size_t n, const uint8_t *sig, size_t ns) {
	alt_hash_kind h = scheme_hash(scheme);
	uint8_t d[ALT_HASH_MAX_SIZE];
	alt_hash_data(h, content, n, d);
	switch (scheme) {
	case S_ED25519:
		return c->pk_alg == ALT_X509_PK_ED25519 && ns == 64 ? alt_ed25519_verify(c->pub.point.p, content, n, sig) : -ALT_EVERIFY;
	case S_ECDSA_P256: case S_ECDSA_P384: case S_ECDSA_P521: {
		int want = scheme == S_ECDSA_P256 ? ALT_X509_P256 : scheme == S_ECDSA_P384 ? ALT_X509_P384 : ALT_X509_P521;
		if (c->pk_alg != ALT_X509_PK_ECDSA || c->pub.curve != want)
			return -ALT_EVERIFY;
		return alt_ecdsa_verify((alt_ec_curve)c->pub.curve, c->pub.point.p, c->pub.point.n, d, alt_hash_size(h), sig, ns);
	}
	case S_RSA_PSS_256: case S_RSA_PSS_384: case S_RSA_PSS_512: {
		if (c->pk_alg != ALT_X509_PK_RSA)
			return -ALT_EVERIFY;
		alt_rsa_public pub = {c->pub.n.p, c->pub.n.n, c->pub.e};
		return alt_rsa_verify_pss(&pub, h, d, alt_hash_size(h), sig, ns, (int)alt_hash_size(h));
	}
	default: return -ALT_EVERIFY; /* PKCS #1 v1.5 and SHA-1 are not for TLS 1.3 handshakes */
	}
}

/* ---- certificates ---- */

static int send_certificate(alt_tls_conn *t, const alt_tls_cert *c, const tr *context) {
	alt_buf m = ALT_BUF_INIT;
	tb b = {.b = &m};
	tb_u8(&b, HS_CERTIFICATE);
	tb_begin(&b, 3);
	tb_begin(&b, 1);
	if (context)
		tb_bytes(&b, context->p, context->n);
	tb_end(&b);
	tb_begin(&b, 3);
	for (size_t i = 0; c && i < c->nchain; i++) {
		tb_begin(&b, 3);
		tb_bytes(&b, c->chain[i].p, c->chain[i].n);
		tb_end(&b);
		tb_u16(&b, 0); /* no extensions */
	}
	tb_end(&b);
	tb_end(&b);
	int rc = m.err ? m.err : send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

static int send_cert_verify(alt_tls_conn *t, const alt_tls_cert *c, uint16_t scheme, bool server) {
	uint8_t content[200], sig[ALT_RSA_MAX_BYTES];
	size_t n = signed_content(t, server, content), ns;
	int rc = sign(&c->key, scheme, content, n, sig, &ns);
	if (rc < 0)
		return fatal(t, A_INTERNAL_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to sign handshake");
	alt_buf m = ALT_BUF_INIT;
	tb b = {.b = &m};
	tb_u8(&b, HS_CERTIFICATE_VERIFY);
	tb_begin(&b, 3);
	tb_u16(&b, scheme);
	tb_begin(&b, 2);
	tb_bytes(&b, sig, ns);
	tb_end(&b);
	tb_end(&b);
	rc = m.err ? m.err : send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

/* reads Certificate: the parsed chain in t->peer */
static int read_certificate(alt_tls_conn *t, tr body, bool allow_empty) {
	tr ctx, list;
	if (!tr_vec(&body, 1, &ctx) || !tr_vec(&body, 3, &list) || body.n)
		return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate message");
	t->peer = 0;
	t->npeer = 0;
	while (list.n) {
		tr der, ext;
		if (!tr_vec(&list, 3, &der) || !tr_vec(&list, 2, &ext) || der.n == 0)
			return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate message");
		uint8_t *copy = alt_arena_alloc(&t->arena, der.n);
		const alt_x509_cert **np = alt_arena_realloc(&t->arena, t->peer, t->npeer * sizeof *np, (t->npeer + 1) * sizeof *np);
		if (!copy || !np)
			return fatal(t, A_INTERNAL_ERROR, -ENOMEM, "out of memory");
		memcpy(copy, der.p, der.n);
		alt_x509_cert *c;
		const char *why = "";
		if (alt_x509_parse(&t->arena, copy, der.n, &c, &why) < 0) {
			fatal(t, A_BAD_CERTIFICATE, -ALT_ETLS_CERT, "");
			snprintf(t->err, sizeof t->err, "tls: failed to parse certificate from server: %s", why);
			return -ALT_ETLS_CERT;
		}
		t->peer = np;
		t->peer[t->npeer++] = c;
	}
	if (t->npeer == 0 && !allow_empty)
		return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: received empty certificates message");
	return 0;
}

/* verifies the peer's chain (Go's verifyServerCertificate / client certs) */
static int verify_peer(alt_tls_conn *t, const char *name, const alt_x509_pool *roots, int eku) {
	alt_x509_pool *inter = alt_x509_pool_new();
	if (!inter)
		return -ENOMEM;
	for (size_t i = 1; i < t->npeer; i++)
		alt_x509_pool_add_der(inter, t->peer[i]->raw.p, t->peer[i]->raw.n);
	alt_x509_verify_opts o = {name, roots, inter, 0, &eku, 1};
	const char *why = "";
	int rc = alt_x509_verify(t->peer[0], &o, &t->arena, &t->chains, &t->nchains, &why);
	alt_x509_pool_free(inter);
	if (rc < 0) {
		int alert = strstr(why, "unknown authority") ? A_UNKNOWN_CA : strstr(why, "expired") ? A_CERTIFICATE_EXPIRED : A_BAD_CERTIFICATE;
		fatal(t, alert, -ALT_ETLS_CERT, "");
		snprintf(t->err, sizeof t->err, "tls: failed to verify certificate: %s", why);
		return -ALT_ETLS_CERT;
	}
	return 0;
}

static int read_cert_verify(alt_tls_conn *t, bool server, const uint16_t *offered, size_t noffered) {
	uint8_t content[200];
	size_t n = signed_content(t, server, content);
	alt_buf m = ALT_BUF_INIT;
	tr body, sig;
	unsigned scheme;
	int rc = expect(t, HS_CERTIFICATE_VERIFY, &m, &body);
	if (rc < 0)
		goto out;
	if (!tr_u16(&body, &scheme) || !tr_vec(&body, 2, &sig) || body.n) {
		rc = fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate verify");
		goto out;
	}
	bool ok = false;
	for (size_t i = 0; i < noffered; i++)
		ok |= offered[i] == scheme;
	if (!ok || scheme == S_RSA_PKCS1_SHA256 || scheme == S_RSA_PKCS1_SHA384 || scheme == S_RSA_PKCS1_SHA512 ||
		scheme == S_RSA_PKCS1_SHA1 || scheme == S_ECDSA_SHA1) {
		rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: certificate used with invalid signature algorithm");
		goto out;
	}
	if (verify_sig(t->peer[0], (uint16_t)scheme, content, n, sig.p, sig.n) < 0) {
		rc = fatal(t, A_DECRYPT_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid signature by the peer certificate");
		goto out;
	}
	alt_buf_append(&t->transcript, m.p, m.n);
out:
	alt_buf_free(&m);
	return rc;
}

/* Finished: HMAC(finished_key, transcript) */
static void finished_mac(alt_tls_conn *t, const uint8_t *base, uint8_t *out) {
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	uint8_t fk[48], th[48];
	expand_label(h, base, "finished", "", 0, fk, hl);
	transcript_hash(t, th);
	alt_hmac_data(h, fk, hl, th, hl, out);
}

static int send_finished(alt_tls_conn *t, const uint8_t *base) {
	uint8_t mac[48];
	finished_mac(t, base, mac);
	size_t hl = alt_hash_size(suite_hash(t->suite));
	alt_buf m = ALT_BUF_INIT;
	tb b = {.b = &m};
	tb_u8(&b, HS_FINISHED);
	tb_u24(&b, (unsigned)hl);
	tb_bytes(&b, mac, hl);
	int rc = m.err ? m.err : send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

static int read_finished(alt_tls_conn *t, const uint8_t *base) {
	uint8_t mac[48];
	finished_mac(t, base, mac);
	size_t hl = alt_hash_size(suite_hash(t->suite));
	alt_buf m = ALT_BUF_INIT;
	tr body;
	int rc = expect(t, HS_FINISHED, &m, &body);
	if (rc == 0) {
		if (body.n != hl || !alt_ct_eq(body.p, mac, hl))
			rc = fatal(t, A_DECRYPT_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid finished hash");
		else
			alt_buf_append(&t->transcript, m.p, m.n);
	}
	alt_buf_free(&m);
	return rc;
}

static int send_ccs(alt_tls_conn *t) {
	static const uint8_t ccs[6] = {REC_CCS, 3, 3, 0, 1, 1};
	alt_mutex_lock(&t->out_mu);
	int rc = write_raw(t, ccs, 6);
	alt_mutex_unlock(&t->out_mu);
	return rc;
}

/* the application secrets from the master secret (transcript up to the server's Finished) */
static void app_secrets(alt_tls_conn *t, uint8_t *c_ap, uint8_t *s_ap) {
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	uint8_t derived[48], zero[48] = {0};
	derive_secret(t, t->hs_secret, "derived", false, derived);
	alt_hkdf_extract(h, derived, hl, zero, hl, t->master);
	derive_secret(t, t->master, "c ap traffic", true, c_ap);
	derive_secret(t, t->master, "s ap traffic", true, s_ap);
}

static void hs_secrets(alt_tls_conn *t, const uint8_t *shared, size_t ns, uint8_t *c_hs, uint8_t *s_hs) {
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	uint8_t early[48], derived[48], zero[48] = {0};
	alt_hkdf_extract(h, 0, 0, zero, hl, early);
	derive_secret(t, early, "derived", false, derived);
	alt_hkdf_extract(h, derived, hl, shared, ns, t->hs_secret);
	derive_secret(t, t->hs_secret, "c hs traffic", true, c_hs);
	derive_secret(t, t->hs_secret, "s hs traffic", true, s_hs);
}

/* HelloRetryRequest: ServerHello with this random (SHA-256 of "HelloRetryRequest") */
static const uint8_t hrr_random[32] = {0xcf, 0x21, 0xad, 0x74, 0xe5, 0x9a, 0x61, 0x11, 0xbe, 0x1d, 0x8c, 0x02, 0x1e, 0x65, 0xb8, 0x91,
	0xc2, 0xa2, 0x11, 0x16, 0x7a, 0xbb, 0x8c, 0x5e, 0x07, 0x9e, 0x09, 0xe2, 0xc8, 0xa8, 0x33, 0x9c};

/* replaces the transcript's first ClientHello by its message_hash (RFC 8446 4.4.1) */
static void transcript_hrr(alt_tls_conn *t, size_t ch_len) {
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	uint8_t d[48];
	alt_hash_data(h, t->transcript.p, ch_len, d);
	alt_buf rest = ALT_BUF_INIT;
	alt_buf_append(&rest, t->transcript.p + ch_len, t->transcript.n - ch_len);
	alt_buf_reset(&t->transcript);
	uint8_t hdr[4] = {HS_MESSAGE_HASH, 0, 0, (uint8_t)hl};
	alt_buf_append(&t->transcript, hdr, 4);
	alt_buf_append(&t->transcript, d, hl);
	alt_buf_append(&t->transcript, rest.p, rest.n);
	alt_buf_free(&rest);
}

/* ---- the client ---- */

static bool is_ip(const char *s) {
	alt_ip ip;
	return alt_ip_parse(alt_str_cstr(s), &ip) == 0;
}

static int build_client_hello(alt_tls_conn *t, alt_buf *m, const uint8_t random[32], const uint8_t sid[32], const keyshare *shares,
	size_t nshares, const tr *cookie) {
	uint16_t suites[8], groups[8];
	size_t ns = suites_of(&t->cfg, suites), ng = groups_of(&t->cfg, groups);
	tb b = {.b = m};
	tb_u8(&b, HS_CLIENT_HELLO);
	tb_begin(&b, 3);
	tb_u16(&b, 0x0303);
	tb_bytes(&b, random, 32);
	tb_u8(&b, 32);
	tb_bytes(&b, sid, 32);
	tb_begin(&b, 2);
	for (size_t i = 0; i < ns; i++)
		tb_u16(&b, suites[i]);
	tb_end(&b);
	tb_u8(&b, 1);
	tb_u8(&b, 0);
	tb_begin(&b, 2); /* extensions */
	const char *sn = t->cfg.server_name;
	if (sn && sn[0] && !is_ip(sn)) {
		size_t l = strlen(sn);
		if (l && sn[l - 1] == '.')
			l--; /* Go sends the name without a trailing dot */
		tb_u16(&b, EXT_SERVER_NAME);
		tb_begin(&b, 2);
		tb_begin(&b, 2);
		tb_u8(&b, 0);
		tb_begin(&b, 2);
		tb_bytes(&b, sn, l);
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
	}
	tb_u16(&b, EXT_SUPPORTED_GROUPS);
	tb_begin(&b, 2);
	tb_begin(&b, 2);
	for (size_t i = 0; i < ng; i++)
		tb_u16(&b, groups[i]);
	tb_end(&b);
	tb_end(&b);
	tb_u16(&b, EXT_SIGNATURE_ALGORITHMS);
	tb_begin(&b, 2);
	tb_begin(&b, 2);
	for (size_t i = 0; i < sizeof sig_schemes / sizeof *sig_schemes; i++)
		tb_u16(&b, sig_schemes[i]);
	tb_end(&b);
	tb_end(&b);
	if (t->cfg.nalpn) {
		tb_u16(&b, EXT_ALPN);
		tb_begin(&b, 2);
		tb_begin(&b, 2);
		for (size_t i = 0; i < t->cfg.nalpn; i++) {
			tb_begin(&b, 1);
			tb_bytes(&b, t->cfg.alpn[i], strlen(t->cfg.alpn[i]));
			tb_end(&b);
		}
		tb_end(&b);
		tb_end(&b);
	}
	tb_u16(&b, EXT_SUPPORTED_VERSIONS);
	tb_begin(&b, 2);
	tb_begin(&b, 1);
	tb_u16(&b, VERSION_13);
	tb_end(&b);
	tb_end(&b);
	if (cookie) {
		tb_u16(&b, EXT_COOKIE);
		tb_begin(&b, 2);
		tb_begin(&b, 2);
		tb_bytes(&b, cookie->p, cookie->n);
		tb_end(&b);
		tb_end(&b);
	}
	tb_u16(&b, EXT_PSK_MODES);
	tb_begin(&b, 2);
	tb_begin(&b, 1);
	tb_u8(&b, 1); /* psk_dhe_ke */
	tb_end(&b);
	tb_end(&b);
	tb_u16(&b, EXT_KEY_SHARE);
	tb_begin(&b, 2);
	tb_begin(&b, 2);
	for (size_t i = 0; i < nshares; i++) {
		tb_u16(&b, shares[i].group);
		tb_begin(&b, 2);
		tb_bytes(&b, shares[i].pub, shares[i].npub);
		tb_end(&b);
	}
	tb_end(&b);
	tb_end(&b);
	tb_end(&b);
	tb_end(&b);
	return m->err;
}

typedef struct {
	uint8_t random[32];
	tr sid;
	unsigned suite, version, group;
	tr share, cookie;
	bool has_version, has_share, has_cookie;
} server_hello;

static int parse_server_hello(alt_tls_conn *t, tr body, server_hello *sh) {
	unsigned v, comp;
	tr rnd, exts;
	memset(sh, 0, sizeof *sh);
	if (!tr_u16(&body, &v) || !tr_take(&body, 32, &rnd) || !tr_vec(&body, 1, &sh->sid) || !tr_u16(&body, &sh->suite) ||
		!tr_u8(&body, &comp) || !tr_vec(&body, 2, &exts) || body.n)
		return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
	memcpy(sh->random, rnd.p, 32);
	if (comp != 0)
		return fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported compression format");
	while (exts.n) {
		unsigned type;
		tr data;
		if (!tr_u16(&exts, &type) || !tr_vec(&exts, 2, &data))
			return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
		switch (type) {
		case EXT_SUPPORTED_VERSIONS:
			if (!tr_u16(&data, &sh->version) || data.n)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
			sh->has_version = true;
			break;
		case EXT_KEY_SHARE:
			if (!tr_u16(&data, &sh->group))
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
			if (data.n && (!tr_vec(&data, 2, &sh->share) || data.n))
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
			sh->has_share = sh->share.n > 0;
			break;
		case EXT_COOKIE:
			if (!tr_vec(&data, 2, &sh->cookie) || data.n || sh->cookie.n == 0)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
			sh->has_cookie = true;
			break;
		default:
			break;
		}
	}
	return 0;
}

static int client_handshake(alt_tls_conn *t) {
	uint8_t random[32], sid[32];
	int rc;
	if (!t->cfg.insecure_skip_verify && (!t->cfg.server_name || !t->cfg.server_name[0]))
		return fail(t, -EINVAL, "tls: either ServerName or InsecureSkipVerify must be specified in the tls.Config");
	if ((rc = alt_crypto_rand(random, 32)) < 0 || (rc = alt_crypto_rand(sid, 32)) < 0)
		return fail(t, rc, "tls: no random bytes");
	uint16_t groups[8];
	size_t ng = groups_of(&t->cfg, groups);
	if (ng == 0)
		return fail(t, -EINVAL, "tls: no supported groups");
	/* a key share for the first group (as Go, without hybrid groups) */
	keyshare share;
	if ((rc = make_share(&share, groups[0])) < 0)
		return fail(t, rc, "tls: key generation failed");
	alt_buf m = ALT_BUF_INIT, in = ALT_BUF_INIT;
	if ((rc = build_client_hello(t, &m, random, sid, &share, 1, 0)) < 0 || (rc = send_handshake(t, &m)) < 0)
		goto out;
	size_t ch1 = t->transcript.n;
	tr body;
	server_hello sh;
	t->ccs_ok = true;
	if ((rc = expect(t, HS_SERVER_HELLO, &in, &body)) < 0 || (rc = parse_server_hello(t, body, &sh)) < 0)
		goto out;
	uint16_t suites[8];
	size_t ns = suites_of(&t->cfg, suites);
	if (!sh.has_version || sh.version != VERSION_13) {
		rc = fatal(t, A_PROTOCOL_VERSION, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported protocol version (TLS 1.3 needed)");
		goto out;
	}
	bool offered = false;
	for (size_t i = 0; i < ns; i++)
		offered |= suites[i] == sh.suite;
	if (!offered || sh.sid.n != 32 || memcmp(sh.sid.p, sid, 32)) {
		rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, !offered ? "tls: server chose an unconfigured cipher suite"
										  : "tls: server echoed an invalid session ID");
		goto out;
	}
	t->suite = (uint16_t)sh.suite;
	alt_buf_append(&t->transcript, in.p, in.n);
	if (memcmp(sh.random, hrr_random, 32) == 0) {
		/* HelloRetryRequest: a new share for the group the server asks for */
		bool ok = sh.group != share.group && group_supported((uint16_t)sh.group) && !sh.has_share;
		bool listed = false;
		for (size_t i = 0; i < ng; i++)
			listed |= groups[i] == sh.group;
		if (!ok || !listed) {
			rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported group");
			goto out;
		}
		transcript_hrr(t, ch1);
		if ((rc = make_share(&share, (uint16_t)sh.group)) < 0)
			goto out;
		alt_buf_reset(&m);
		/* the cookie lives in in, which is not reused before the new ClientHello is built */
		alt_buf cookie = ALT_BUF_INIT;
		alt_buf_append(&cookie, sh.cookie.p, sh.cookie.n);
		tr ck = {(const uint8_t *)cookie.p, cookie.n};
		if ((rc = send_ccs(t)) < 0 || (rc = build_client_hello(t, &m, random, sid, &share, 1, sh.has_cookie ? &ck : 0)) < 0 ||
			(rc = send_handshake(t, &m)) < 0) {
			alt_buf_free(&cookie);
			goto out;
		}
		alt_buf_free(&cookie);
		uint16_t hrr_suite = t->suite;
		if ((rc = expect(t, HS_SERVER_HELLO, &in, &body)) < 0 || (rc = parse_server_hello(t, body, &sh)) < 0)
			goto out;
		if (memcmp(sh.random, hrr_random, 32) == 0 || sh.suite != hrr_suite || !sh.has_version || sh.version != VERSION_13) {
			rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server sent an invalid ServerHello after the retry");
			goto out;
		}
		alt_buf_append(&t->transcript, in.p, in.n);
	}
	if (!sh.has_share || sh.group != share.group) {
		rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported group");
		goto out;
	}
	t->group = share.group;
	uint8_t shared[66], c_hs[48], s_hs[48];
	size_t nshared;
	if (shared_secret(&share, sh.share.p, sh.share.n, shared, &nshared) < 0) {
		rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid server key share");
		goto out;
	}
	memset(&share, 0, sizeof share);
	hs_secrets(t, shared, nshared, c_hs, s_hs);
	memset(shared, 0, sizeof shared);
	if ((rc = keys_boundary(t)) < 0)
		goto out;
	set_keys(&t->in, t->suite, s_hs);
	/* EncryptedExtensions */
	if ((rc = expect(t, HS_ENCRYPTED_EXTENSIONS, &in, &body)) < 0)
		goto out;
	tr exts;
	if (!tr_vec(&body, 2, &exts) || body.n) {
		rc = fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse EncryptedExtensions");
		goto out;
	}
	while (exts.n) {
		unsigned type;
		tr data;
		if (!tr_u16(&exts, &type) || !tr_vec(&exts, 2, &data)) {
			rc = fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse EncryptedExtensions");
			goto out;
		}
		if (type == EXT_ALPN) {
			tr list, proto;
			if (!tr_vec(&data, 2, &list) || !tr_vec(&list, 1, &proto) || list.n || data.n || proto.n == 0) {
				rc = fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ALPN");
				goto out;
			}
			bool ok = false;
			for (size_t i = 0; i < t->cfg.nalpn; i++)
				ok |= strlen(t->cfg.alpn[i]) == proto.n && memcmp(t->cfg.alpn[i], proto.p, proto.n) == 0;
			if (!ok) {
				rc = fatal(t, A_UNSUPPORTED_EXTENSION, -ALT_ETLS_HANDSHAKE, "tls: server advertised unrequested ALPN extension");
				goto out;
			}
			memcpy(t->alpn, proto.p, proto.n);
			t->alpn[proto.n] = 0;
		}
	}
	alt_buf_append(&t->transcript, in.p, in.n);
	/* CertificateRequest (optional), Certificate, CertificateVerify, Finished */
	int ty;
	if ((rc = read_handshake(t, &ty, &in)) < 0)
		goto out;
	bool cert_requested = false;
	alt_buf req_ctx = ALT_BUF_INIT;
	uint16_t peer_schemes[64];
	size_t npeer_schemes = 0;
	if (ty == HS_CERTIFICATE_REQUEST) {
		tr b2 = {(const uint8_t *)in.p + 4, in.n - 4}, ctx, ex;
		if (!tr_vec(&b2, 1, &ctx) || !tr_vec(&b2, 2, &ex) || b2.n) {
			rc = fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse CertificateRequest");
			alt_buf_free(&req_ctx);
			goto out;
		}
		alt_buf_append(&req_ctx, ctx.p, ctx.n);
		while (ex.n) {
			unsigned type;
			tr data, list;
			if (!tr_u16(&ex, &type) || !tr_vec(&ex, 2, &data))
				break;
			if (type == EXT_SIGNATURE_ALGORITHMS && tr_vec(&data, 2, &list))
				while (list.n >= 2 && npeer_schemes < 64) {
					unsigned s;
					tr_u16(&list, &s);
					peer_schemes[npeer_schemes++] = (uint16_t)s;
				}
		}
		cert_requested = true;
		alt_buf_append(&t->transcript, in.p, in.n);
		if ((rc = read_handshake(t, &ty, &in)) < 0) {
			alt_buf_free(&req_ctx);
			goto out;
		}
	}
	if (ty != HS_CERTIFICATE) {
		rc = fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected handshake message (wanted Certificate)");
		alt_buf_free(&req_ctx);
		goto out;
	}
	if ((rc = read_certificate(t, (tr){(const uint8_t *)in.p + 4, in.n - 4}, false)) < 0) {
		alt_buf_free(&req_ctx);
		goto out;
	}
	alt_buf_append(&t->transcript, in.p, in.n);
	if (!t->cfg.insecure_skip_verify) {
		const char *name = t->cfg.server_name;
		if ((rc = verify_peer(t, name, t->cfg.roots, ALT_X509_EKU_SERVER_AUTH)) < 0) {
			alt_buf_free(&req_ctx);
			goto out;
		}
	}
	if ((rc = read_cert_verify(t, true, sig_schemes, sizeof sig_schemes / sizeof *sig_schemes)) < 0 || (rc = read_finished(t, s_hs)) < 0) {
		alt_buf_free(&req_ctx);
		goto out;
	}
	uint8_t c_ap[48], s_ap[48];
	app_secrets(t, c_ap, s_ap);
	if ((rc = keys_boundary(t)) < 0) {
		alt_buf_free(&req_ctx);
		goto out;
	}
	set_keys(&t->in, t->suite, s_ap);
	/* our second flight: compatibility CCS, our handshake keys */
	if ((rc = send_ccs(t)) < 0) {
		alt_buf_free(&req_ctx);
		goto out;
	}
	alt_mutex_lock(&t->out_mu);
	set_keys(&t->out, t->suite, c_hs);
	alt_mutex_unlock(&t->out_mu);
	if (cert_requested) {
		/* the first certificate whose key fits the server's schemes (Go's getClientCertificate) */
		const alt_tls_cert *mine = 0;
		uint16_t scheme = 0;
		for (size_t i = 0; i < t->cfg.ncerts && !mine; i++)
			if ((scheme = pick_scheme(&t->cfg.certs[i]->key, peer_schemes, npeer_schemes)))
				mine = t->cfg.certs[i];
		tr ctx = {(const uint8_t *)req_ctx.p, req_ctx.n};
		if ((rc = send_certificate(t, mine, &ctx)) < 0 || (mine && (rc = send_cert_verify(t, mine, scheme, false)) < 0)) {
			alt_buf_free(&req_ctx);
			goto out;
		}
	}
	alt_buf_free(&req_ctx);
	if ((rc = send_finished(t, c_hs)) < 0)
		goto out;
	alt_mutex_lock(&t->out_mu);
	set_keys(&t->out, t->suite, c_ap);
	alt_mutex_unlock(&t->out_mu);
	t->ccs_ok = false;
	memset(c_hs, 0, sizeof c_hs);
	memset(s_hs, 0, sizeof s_hs);
	rc = 0;
out:
	alt_buf_free(&m);
	alt_buf_free(&in);
	return rc;
}

/* ---- the server ---- */

typedef struct {
	tr random, sid, suites, comp, exts;
	char sni[256];
	bool has_sni;
	uint16_t groups[32], schemes[64];
	size_t ngroups, nschemes;
	bool v13;
	struct {
		uint16_t group;
		tr key;
	} shares[16];
	size_t nshares;
	char alpn[16][256];
	size_t nalpn;
	tr cookie;
	bool has_cookie, has_psk_modes;
} client_hello;

static int parse_client_hello(alt_tls_conn *t, tr body, client_hello *ch) {
	unsigned v;
	memset(ch, 0, sizeof *ch);
	if (!tr_u16(&body, &v) || !tr_take(&body, 32, &ch->random) || !tr_vec(&body, 1, &ch->sid) || !tr_vec(&body, 2, &ch->suites) ||
		!tr_vec(&body, 1, &ch->comp))
		return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
	if (body.n && (!tr_vec(&body, 2, &ch->exts) || body.n))
		return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
	tr e = ch->exts;
	uint16_t seen[64];
	size_t nseen = 0;
	while (e.n) {
		unsigned type;
		tr d;
		if (!tr_u16(&e, &type) || !tr_vec(&e, 2, &d))
			return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
		for (size_t i = 0; i < nseen; i++)
			if (seen[i] == type)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: client sent duplicate extensions");
		if (nseen < 64)
			seen[nseen++] = (uint16_t)type;
		tr list, x;
		switch (type) {
		case EXT_SERVER_NAME:
			if (!tr_vec(&d, 2, &list))
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			while (list.n) {
				unsigned nt;
				if (!tr_u8(&list, &nt) || !tr_vec(&list, 2, &x))
					return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
				if (nt == 0 && !ch->has_sni) {
					if (x.n == 0 || x.n >= sizeof ch->sni || memchr(x.p, 0, x.n) || x.p[x.n - 1] == '.')
						return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
					memcpy(ch->sni, x.p, x.n);
					ch->has_sni = true;
				}
			}
			break;
		case EXT_SUPPORTED_GROUPS:
			if (!tr_vec(&d, 2, &list) || list.n % 2)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			while (list.n) {
				unsigned g;
				tr_u16(&list, &g);
				if (ch->ngroups < 32)
					ch->groups[ch->ngroups++] = (uint16_t)g;
			}
			break;
		case EXT_SIGNATURE_ALGORITHMS:
			if (!tr_vec(&d, 2, &list) || list.n % 2 || list.n == 0)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			while (list.n) {
				unsigned s;
				tr_u16(&list, &s);
				if (ch->nschemes < 64)
					ch->schemes[ch->nschemes++] = (uint16_t)s;
			}
			break;
		case EXT_ALPN:
			if (!tr_vec(&d, 2, &list) || list.n == 0)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			while (list.n) {
				if (!tr_vec(&list, 1, &x) || x.n == 0)
					return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
				if (ch->nalpn < 16) {
					memcpy(ch->alpn[ch->nalpn], x.p, x.n);
					ch->alpn[ch->nalpn++][x.n] = 0;
				}
			}
			break;
		case EXT_SUPPORTED_VERSIONS:
			if (!tr_vec(&d, 1, &list) || list.n % 2)
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			while (list.n) {
				unsigned ver;
				tr_u16(&list, &ver);
				ch->v13 |= ver == VERSION_13;
			}
			break;
		case EXT_KEY_SHARE:
			if (!tr_vec(&d, 2, &list))
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			while (list.n) {
				unsigned g;
				if (!tr_u16(&list, &g) || !tr_vec(&list, 2, &x) || x.n == 0)
					return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
				if (ch->nshares < 16)
					ch->shares[ch->nshares].group = (uint16_t)g, ch->shares[ch->nshares++].key = x;
			}
			break;
		case EXT_COOKIE:
			if (!tr_vec(&d, 2, &ch->cookie))
				return fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
			ch->has_cookie = true;
			break;
		case EXT_PSK_MODES:
			ch->has_psk_modes = true;
			break;
		}
	}
	return 0;
}

/* the certificate for the client's name (Go's getCertificate: a name
   match, else the first) */
static const alt_tls_cert *choose_cert(alt_tls_conn *t, const client_hello *ch, alt_arena *a) {
	if (t->cfg.ncerts == 0)
		return 0;
	if (!ch->has_sni || t->cfg.ncerts == 1)
		return t->cfg.certs[0];
	for (size_t i = 0; i < t->cfg.ncerts; i++) {
		const alt_tls_cert *c = t->cfg.certs[i];
		alt_x509_cert *x;
		if (c->nchain && alt_x509_parse(a, c->chain[0].p, c->chain[0].n, &x, 0) == 0 &&
			alt_x509_verify_hostname(x, ch->sni, a, 0) == 0)
			return c;
	}
	return t->cfg.certs[0];
}

static int server_handshake(alt_tls_conn *t) {
	alt_buf in = ALT_BUF_INIT, m = ALT_BUF_INIT, ch_raw = ALT_BUF_INIT;
	alt_arena scratch;
	alt_arena_init(&scratch, 0);
	tr body;
	client_hello ch;
	int rc;
	if ((rc = expect(t, HS_CLIENT_HELLO, &in, &body)) < 0)
		goto out;
	/* the first hello stays in ch_raw: ch points into it through a retry */
	alt_buf_append(&ch_raw, in.p, in.n);
	if ((rc = parse_client_hello(t, (tr){(const uint8_t *)ch_raw.p + 4, ch_raw.n - 4}, &ch)) < 0)
		goto out;
	t->ccs_ok = true;
	alt_buf_append(&t->transcript, in.p, in.n);
	if (!ch.v13) {
		rc = fatal(t, A_PROTOCOL_VERSION, -ALT_ETLS_HANDSHAKE, "tls: client offered only unsupported versions (TLS 1.3 needed)");
		goto out;
	}
	if (ch.comp.n != 1 || ch.comp.p[0] != 0) {
		rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: TLS 1.3 client supports illegal compression methods");
		goto out;
	}
	/* the suite: ours in preference order, ChaCha20 first when the client puts it first (as Go) */
	uint16_t mine[8];
	size_t nm = suites_of(&t->cfg, mine);
	if (!t->cfg.cipher_suites && ch.suites.n >= 2 && ((ch.suites.p[0] << 8) | ch.suites.p[1]) == ALT_TLS_CHACHA20_POLY1305_SHA256) {
		mine[0] = ALT_TLS_CHACHA20_POLY1305_SHA256, mine[1] = ALT_TLS_AES_128_GCM_SHA256, mine[2] = ALT_TLS_AES_256_GCM_SHA384;
		nm = 3;
	}
	t->suite = 0;
	for (size_t i = 0; i < nm && !t->suite; i++)
		for (size_t k = 0; k + 1 < ch.suites.n; k += 2)
			if (((ch.suites.p[k] << 8) | ch.suites.p[k + 1]) == mine[i]) {
				t->suite = mine[i];
				break;
			}
	if (!t->suite) {
		rc = fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: no cipher suite supported by both client and server");
		goto out;
	}
	if (ch.nschemes == 0) {
		rc = fatal(t, A_MISSING_EXTENSION, -ALT_ETLS_HANDSHAKE, "tls: client sent no signature algorithms");
		goto out;
	}
	/* the group: ours in preference order, among the client's */
	uint16_t groups[8];
	size_t ng = groups_of(&t->cfg, groups);
	uint16_t group = 0;
	for (size_t i = 0; i < ng && !group; i++)
		for (size_t k = 0; k < ch.ngroups; k++)
			if (ch.groups[k] == groups[i]) {
				group = groups[i];
				break;
			}
	if (!group) {
		rc = fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: no ECDHE curve supported by both client and server");
		goto out;
	}
	int share_i = -1;
	bool sent_hrr_ccs = false;
	for (size_t k = 0; k < ch.nshares; k++)
		if (ch.shares[k].group == group)
			share_i = (int)k;
	if (share_i < 0) {
		/* HelloRetryRequest */
		size_t ch1 = t->transcript.n;
		alt_buf_reset(&m);
		tb b = {.b = &m};
		tb_u8(&b, HS_SERVER_HELLO);
		tb_begin(&b, 3);
		tb_u16(&b, 0x0303);
		tb_bytes(&b, hrr_random, 32);
		tb_begin(&b, 1);
		tb_bytes(&b, ch.sid.p, ch.sid.n);
		tb_end(&b);
		tb_u16(&b, t->suite);
		tb_u8(&b, 0);
		tb_begin(&b, 2);
		tb_u16(&b, EXT_SUPPORTED_VERSIONS);
		tb_u16(&b, 2);
		tb_u16(&b, VERSION_13);
		tb_u16(&b, EXT_KEY_SHARE);
		tb_u16(&b, 2);
		tb_u16(&b, group);
		tb_end(&b);
		tb_end(&b);
		transcript_hrr(t, ch1);
		if ((rc = send_handshake(t, &m)) < 0)
			goto out;
		if (ch.sid.n && (rc = send_ccs(t)) < 0)
			goto out;
		sent_hrr_ccs = ch.sid.n > 0;
		/* the second ClientHello: the same, with the share */
		if ((rc = expect(t, HS_CLIENT_HELLO, &in, &body)) < 0)
			goto out;
		client_hello c2;
		if ((rc = parse_client_hello(t, body, &c2)) < 0)
			goto out;
		share_i = -1;
		for (size_t k = 0; k < c2.nshares; k++)
			if (c2.shares[k].group == group)
				share_i = (int)k;
		if (share_i < 0 || c2.nshares != 1 || c2.suites.n != ch.suites.n || memcmp(c2.suites.p, ch.suites.p, ch.suites.n) ||
			c2.sid.n != ch.sid.n || memcmp(c2.sid.p, ch.sid.p, ch.sid.n)) {
			rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: client sent invalid key share in second ClientHello");
			goto out;
		}
		alt_buf_append(&t->transcript, in.p, in.n);
		/* the original hello's other fields are what count; the new one holds the share */
		ch.shares[0] = c2.shares[share_i];
		ch.nshares = 1;
		share_i = 0; /* its key points into in, not reused before the secret is made */
	}
	t->group = group;
	if (ch.has_sni)
		snprintf(t->sni, sizeof t->sni, "%s", ch.sni);
	/* ALPN: our first protocol the client offers */
	if (t->cfg.nalpn && ch.nalpn) {
		for (size_t i = 0; i < t->cfg.nalpn && !t->alpn[0]; i++)
			for (size_t k = 0; k < ch.nalpn; k++)
				if (strcmp(t->cfg.alpn[i], ch.alpn[k]) == 0) {
					snprintf(t->alpn, sizeof t->alpn, "%s", ch.alpn[k]);
					break;
				}
		if (!t->alpn[0]) {
			rc = fatal(t, A_NO_APPLICATION_PROTOCOL, -ALT_ETLS_HANDSHAKE, "tls: client requested unsupported application protocols");
			goto out;
		}
	}
	const alt_tls_cert *cert = choose_cert(t, &ch, &scratch);
	uint16_t scheme = cert ? pick_scheme(&cert->key, ch.schemes, ch.nschemes) : 0;
	if (!cert || !scheme) {
		rc = fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE,
			cert ? "tls: peer doesn't support any of the certificate's signature algorithms" : "tls: no certificates configured");
		goto out;
	}
	/* our share, the secrets */
	keyshare share;
	uint8_t shared[66], c_hs[48], s_hs[48], random[32];
	size_t nshared;
	if ((rc = make_share(&share, group)) < 0 || (rc = alt_crypto_rand(random, 32)) < 0)
		goto out;
	if (shared_secret(&share, ch.shares[share_i].key.p, ch.shares[share_i].key.n, shared, &nshared) < 0) {
		rc = fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid client key share");
		goto out;
	}
	alt_buf_reset(&m);
	tb b = {.b = &m};
	tb_u8(&b, HS_SERVER_HELLO);
	tb_begin(&b, 3);
	tb_u16(&b, 0x0303);
	tb_bytes(&b, random, 32);
	tb_begin(&b, 1);
	tb_bytes(&b, ch.sid.p, ch.sid.n);
	tb_end(&b);
	tb_u16(&b, t->suite);
	tb_u8(&b, 0);
	tb_begin(&b, 2);
	tb_u16(&b, EXT_SUPPORTED_VERSIONS);
	tb_u16(&b, 2);
	tb_u16(&b, VERSION_13);
	tb_u16(&b, EXT_KEY_SHARE);
	tb_begin(&b, 2);
	tb_u16(&b, group);
	tb_begin(&b, 2);
	tb_bytes(&b, share.pub, share.npub);
	tb_end(&b);
	tb_end(&b);
	tb_end(&b);
	tb_end(&b);
	if ((rc = send_handshake(t, &m)) < 0)
		goto out;
	if (ch.sid.n && !sent_hrr_ccs && (rc = send_ccs(t)) < 0)
		goto out;
	hs_secrets(t, shared, nshared, c_hs, s_hs);
	memset(shared, 0, sizeof shared);
	memset(&share, 0, sizeof share);
	if ((rc = keys_boundary(t)) < 0)
		goto out;
	alt_mutex_lock(&t->out_mu);
	set_keys(&t->out, t->suite, s_hs);
	alt_mutex_unlock(&t->out_mu);
	set_keys(&t->in, t->suite, c_hs);
	/* EncryptedExtensions */
	alt_buf_reset(&m);
	tb_u8(&b, HS_ENCRYPTED_EXTENSIONS);
	tb_begin(&b, 3);
	tb_begin(&b, 2);
	if (t->alpn[0]) {
		tb_u16(&b, EXT_ALPN);
		tb_begin(&b, 2);
		tb_begin(&b, 2);
		tb_begin(&b, 1);
		tb_bytes(&b, t->alpn, strlen(t->alpn));
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
	}
	tb_end(&b);
	tb_end(&b);
	if ((rc = send_handshake(t, &m)) < 0)
		goto out;
	bool want_cert = t->cfg.client_auth != ALT_TLS_NO_CLIENT_CERT;
	if (want_cert) {
		alt_buf_reset(&m);
		tb_u8(&b, HS_CERTIFICATE_REQUEST);
		tb_begin(&b, 3);
		tb_u8(&b, 0); /* empty context */
		tb_begin(&b, 2);
		tb_u16(&b, EXT_SIGNATURE_ALGORITHMS);
		tb_begin(&b, 2);
		tb_begin(&b, 2);
		for (size_t i = 0; i < sizeof sig_schemes / sizeof *sig_schemes; i++)
			tb_u16(&b, sig_schemes[i]);
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
		if ((rc = send_handshake(t, &m)) < 0)
			goto out;
	}
	if ((rc = send_certificate(t, cert, 0)) < 0 || (rc = send_cert_verify(t, cert, scheme, true)) < 0 ||
		(rc = send_finished(t, s_hs)) < 0)
		goto out;
	uint8_t c_ap[48], s_ap[48];
	app_secrets(t, c_ap, s_ap);
	alt_mutex_lock(&t->out_mu);
	set_keys(&t->out, t->suite, s_ap);
	alt_mutex_unlock(&t->out_mu);
	if (want_cert) {
		alt_buf_reset(&in);
		if ((rc = expect(t, HS_CERTIFICATE, &in, &body)) < 0 || (rc = read_certificate(t, body, true)) < 0)
			goto out;
		alt_buf_append(&t->transcript, in.p, in.n);
		int ca = t->cfg.client_auth;
		if (t->npeer == 0 && (ca == ALT_TLS_REQUIRE_ANY_CLIENT_CERT || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT)) {
			rc = fatal(t, A_CERTIFICATE_REQUIRED, -ALT_ETLS_HANDSHAKE, "tls: client didn't provide a certificate");
			goto out;
		}
		if (t->npeer && (ca == ALT_TLS_VERIFY_CLIENT_CERT_IF_GIVEN || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT) &&
			(rc = verify_peer(t, 0, t->cfg.client_cas, ALT_X509_EKU_CLIENT_AUTH)) < 0)
			goto out;
		if (t->npeer && (rc = read_cert_verify(t, false, sig_schemes, sizeof sig_schemes / sizeof *sig_schemes)) < 0)
			goto out;
	}
	if ((rc = read_finished(t, c_hs)) < 0 || (rc = keys_boundary(t)) < 0)
		goto out;
	set_keys(&t->in, t->suite, c_ap);
	t->ccs_ok = false;
	memset(c_hs, 0, sizeof c_hs);
	memset(s_hs, 0, sizeof s_hs);
	rc = 0;
out:
	alt_buf_free(&in);
	alt_buf_free(&m);
	alt_buf_free(&ch_raw);
	alt_arena_free(&scratch);
	return rc;
}

/* ---- the connection ---- */

int alt_tls_handshake(alt_tls_conn *t) {
	alt_mutex_lock(&t->hs_mu);
	if (t->done || t->hs_err) {
		int rc = t->hs_err;
		alt_mutex_unlock(&t->hs_mu);
		return rc;
	}
	if (t->cfg.handshake_timeout && t->conn)
		alt_conn_set_deadline(t->conn, alt_deadline(t->cfg.handshake_timeout));
	alt_mutex_lock(&t->in_mu);
	int rc = t->client ? client_handshake(t) : server_handshake(t);
	alt_mutex_unlock(&t->in_mu);
	if (t->cfg.handshake_timeout && t->conn)
		alt_conn_set_deadline(t->conn, ALT_FOREVER);
	if (rc < 0) {
		t->hs_err = rc;
		if (!t->err[0])
			fail(t, rc, "tls: %s", alt_strerror(rc));
	} else
		t->done = true;
	alt_buf_free(&t->transcript);
	alt_mutex_unlock(&t->hs_mu);
	return rc;
}

/* a post-handshake message: tickets are dropped (no resumption), key updates applied */
static int post_handshake(alt_tls_conn *t) {
	alt_buf m = ALT_BUF_INIT;
	int ty, rc = read_handshake(t, &ty, &m);
	if (rc < 0) {
		alt_buf_free(&m);
		return rc;
	}
	if (ty == HS_NEW_SESSION_TICKET && t->client) {
		alt_buf_free(&m);
		return 0;
	}
	if (ty == HS_KEY_UPDATE) {
		if (m.n != 5 || (uint8_t)m.p[4] > 1) {
			alt_buf_free(&m);
			send_alert(t, 2, A_DECODE_ERROR);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: invalid key update");
		}
		bool requested = m.p[4] == 1;
		alt_buf_free(&m);
		if (t->hs.n) {
			send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: handshake message spans a key change");
		}
		next_secret(&t->in);
		if (requested) {
			alt_mutex_lock(&t->out_mu);
			uint8_t ku[5] = {HS_KEY_UPDATE, 0, 0, 1, 0};
			rc = write_record(t, REC_HANDSHAKE, ku, 5);
			if (rc == 0)
				next_secret(&t->out);
			alt_mutex_unlock(&t->out_mu);
			if (rc < 0)
				return rc;
		}
		return 0;
	}
	alt_buf_free(&m);
	send_alert(t, 2, A_UNEXPECTED_MESSAGE);
	return t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: received unexpected handshake message of type %d", ty);
}

ssize_t alt_tls_read(alt_tls_conn *t, void *buf, size_t n) {
	int rc = alt_tls_handshake(t);
	if (rc < 0)
		return rc;
	alt_mutex_lock(&t->in_mu);
	ssize_t res;
	for (;;) {
		if (t->app_n) {
			size_t k = n < t->app_n ? n : t->app_n;
			memcpy(buf, t->app + t->app_off, k);
			t->app_off += k;
			t->app_n -= k;
			res = (ssize_t)k;
			break;
		}
		if (t->eof) {
			res = 0;
			break;
		}
		if (t->read_err) {
			res = t->read_err;
			break;
		}
		if (n == 0) {
			res = 0;
			break;
		}
		if (t->hs.n) {
			if ((rc = post_handshake(t)) < 0) {
				res = rc;
				break;
			}
			continue;
		}
		int ty;
		uint8_t *d;
		size_t len;
		if ((rc = read_record(t, &ty, &d, &len)) < 0) {
			res = rc;
			break;
		}
		if (ty == REC_APP) {
			memcpy(t->app, d, len);
			t->app_off = 0;
			t->app_n = len;
		} else if (ty == REC_ALERT) {
			if ((rc = on_alert(t, d, len)) < 0) {
				res = rc;
				break;
			}
		} else if (ty == REC_HANDSHAKE) {
			if (len == 0) {
				send_alert(t, 2, A_UNEXPECTED_MESSAGE);
				res = t->read_err = fail(t, -ALT_ETLS_RECORD, "tls: empty handshake record");
				break;
			}
			alt_buf_append(&t->hs, d, len);
		}
	}
	alt_mutex_unlock(&t->in_mu);
	return res;
}

ssize_t alt_tls_write(alt_tls_conn *t, const void *p, size_t n) {
	int rc = alt_tls_handshake(t);
	if (rc < 0)
		return rc;
	alt_mutex_lock(&t->out_mu);
	if (t->closed_write)
		rc = -ALT_ECLOSED;
	else if (t->key_update_pending) {
		uint8_t ku[5] = {HS_KEY_UPDATE, 0, 0, 1, 1};
		rc = write_record(t, REC_HANDSHAKE, ku, 5);
		if (rc == 0) {
			next_secret(&t->out);
			t->key_update_pending = false;
		}
	}
	if (rc == 0)
		rc = write_records(t, REC_APP, p, n);
	alt_mutex_unlock(&t->out_mu);
	return rc < 0 ? rc : (ssize_t)n;
}

int alt_tls_key_update(alt_tls_conn *t) {
	int rc = alt_tls_handshake(t);
	if (rc < 0)
		return rc;
	alt_mutex_lock(&t->out_mu);
	uint8_t ku[5] = {HS_KEY_UPDATE, 0, 0, 1, 1}; /* update_requested: the peer updates too */
	rc = write_record(t, REC_HANDSHAKE, ku, 5);
	if (rc == 0)
		next_secret(&t->out);
	alt_mutex_unlock(&t->out_mu);
	return rc;
}

int alt_tls_close_write(alt_tls_conn *t) {
	alt_mutex_lock(&t->out_mu);
	int rc = 0;
	if (!t->closed_write) {
		t->closed_write = true;
		uint8_t a[2] = {1, A_CLOSE_NOTIFY};
		rc = write_record(t, REC_ALERT, a, 2);
	}
	alt_mutex_unlock(&t->out_mu);
	return rc;
}

static void conn_free(alt_tls_conn *t) {
	alt_buf_free(&t->hs);
	alt_buf_free(&t->transcript);
	alt_arena_free(&t->arena);
	memset(&t->in, 0, sizeof t->in);
	memset(&t->out, 0, sizeof t->out);
	free(t);
}

int alt_tls_close(alt_tls_conn *t) {
	if (!t)
		return 0;
	int rc = 0;
	if (t->done)
		rc = alt_tls_close_write(t);
	if (t->conn) {
		int r2 = alt_conn_close(t->conn);
		if (rc == 0)
			rc = r2;
	}
	conn_free(t);
	return rc;
}

static ssize_t reader_read(void *obj, void *buf, size_t n) { return alt_tls_read(obj, buf, n); }
static ssize_t writer_write(void *obj, const void *p, size_t n) { return alt_tls_write(obj, p, n); }
alt_reader alt_tls_reader(alt_tls_conn *t) { return (alt_reader){reader_read, t}; }
alt_writer alt_tls_writer(alt_tls_conn *t) { return (alt_writer){writer_write, t}; }
alt_conn *alt_tls_net_conn(alt_tls_conn *t) { return t->conn; }

static int make(alt_tls_conn **out, alt_conn *c, alt_reader r, alt_writer w, const alt_tls_config *cfg, bool client) {
	alt_tls_conn *t = calloc(1, sizeof *t);
	if (!t)
		return -ENOMEM;
	t->conn = c;
	t->rd = r;
	t->wr = w;
	t->client = client;
	if (cfg)
		t->cfg = *cfg;
	t->hs_mu = (alt_mutex)ALT_MUTEX_INIT;
	t->in_mu = (alt_mutex)ALT_MUTEX_INIT;
	t->out_mu = (alt_mutex)ALT_MUTEX_INIT;
	alt_arena_init(&t->arena, 0);
	*out = t;
	return 0;
}

int alt_tls_client(alt_tls_conn **out, alt_conn *c, const alt_tls_config *cfg) {
	return make(out, c, alt_conn_reader(c), alt_conn_writer(c), cfg, true);
}
int alt_tls_server(alt_tls_conn **out, alt_conn *c, const alt_tls_config *cfg) {
	return make(out, c, alt_conn_reader(c), alt_conn_writer(c), cfg, false);
}
int alt_tls_client_stream(alt_tls_conn **out, alt_reader r, alt_writer w, const alt_tls_config *cfg) {
	return make(out, 0, r, w, cfg, true);
}
int alt_tls_server_stream(alt_tls_conn **out, alt_reader r, alt_writer w, const alt_tls_config *cfg) {
	return make(out, 0, r, w, cfg, false);
}

int alt_tls_dial(alt_tls_conn **out, const char *network, const char *address, const alt_tls_config *cfg, alt_ctx *ctx) {
	alt_conn *c;
	int rc = alt_dial(&c, network, address, ctx);
	if (rc < 0)
		return rc;
	alt_tls_config cc = cfg ? *cfg : (alt_tls_config){0};
	char host[256] = "";
	if (!cc.server_name || !cc.server_name[0]) {
		alt_str h, port;
		if (alt_split_host_port(alt_str_cstr(address), &h, &port) == 0 && h.n < sizeof host) {
			memcpy(host, h.p, h.n);
			host[h.n] = 0;
		}
		cc.server_name = host;
	}
	alt_tls_conn *t;
	if ((rc = alt_tls_client(&t, c, &cc)) < 0) {
		alt_conn_close(c);
		return rc;
	}
	alt_time dl = alt_ctx_deadline(ctx);
	if (dl)
		alt_conn_set_deadline(c, dl);
	rc = alt_tls_handshake(t);
	if (dl)
		alt_conn_set_deadline(c, ALT_FOREVER);
	/* the host name lives on this stack: the handshake used it, keep the configured one */
	t->cfg.server_name = cfg ? cfg->server_name : 0;
	if (rc < 0) {
		*out = t; /* for alt_tls_error; the caller closes */
		return rc;
	}
	*out = t;
	return 0;
}

void alt_tls_connection_state(const alt_tls_conn *t, alt_tls_state *s) {
	memset(s, 0, sizeof *s);
	s->handshake_complete = t->done;
	s->version = t->done ? VERSION_13 : 0;
	s->cipher_suite = t->suite;
	s->group = t->group;
	s->alpn = t->alpn;
	s->server_name = t->client ? (t->cfg.server_name ? t->cfg.server_name : "") : t->sni;
	s->peer_certs = t->peer;
	s->npeer_certs = t->npeer;
	s->verified_chains = t->chains;
	s->nverified_chains = t->nchains;
}

/* ---- key pairs ---- */

typedef struct {
	alt_tls_cert c;
	alt_arena a;
} owned_cert;

int alt_tls_key_pair(alt_tls_cert **out, alt_str cert_pem, alt_str key_pem, const char **why) {
	owned_cert *o = calloc(1, sizeof *o);
	if (!o)
		return -ENOMEM;
	alt_arena_init(&o->a, 0);
	alt_pem_block b;
	alt_str rest = cert_pem;
	alt_str *chain = 0;
	size_t n = 0;
	while (alt_pem_decode(&o->a, &rest, &b)) {
		if (!alt_str_eq_cstr(b.type, "CERTIFICATE"))
			continue;
		chain = alt_arena_realloc(&o->a, chain, n * sizeof *chain, (n + 1) * sizeof *chain);
		if (!chain)
			goto nomem;
		chain[n++] = b.bytes;
	}
	if (n == 0) {
		if (why)
			*why = "tls: failed to find any PEM data in certificate input";
		goto bad;
	}
	rest = key_pem;
	bool found = false;
	while (alt_pem_decode(&o->a, &rest, &b))
		if (alt_str_eq_cstr(b.type, "PRIVATE KEY") || alt_str_has_suffix(b.type, ALT_STR(" PRIVATE KEY"))) {
			found = true;
			break;
		}
	if (!found) {
		if (why)
			*why = "tls: failed to find any PEM data in key input";
		goto bad;
	}
	const char *w = 0;
	if (alt_x509_parse_private_key(&o->a, b.bytes.p, b.bytes.n, &o->c.key, &w) < 0) {
		if (why)
			*why = "tls: failed to parse private key";
		goto bad;
	}
	/* the key must belong to the leaf (as Go checks) */
	alt_x509_cert *leaf;
	if (alt_x509_parse(&o->a, chain[0].p, chain[0].n, &leaf, &w) < 0) {
		if (why)
			*why = w;
		goto bad;
	}
	bool match = false;
	const alt_x509_private_key *k = &o->c.key;
	if (leaf->pk_alg == k->alg) {
		if (k->alg == ALT_X509_PK_ED25519)
			match = memcmp(leaf->pub.point.p, k->ed25519 + 32, 32) == 0;
		else if (k->alg == ALT_X509_PK_ECDSA)
			match = leaf->pub.curve == k->curve && leaf->pub.point.n == alt_ec_point_size((alt_ec_curve)k->curve) &&
				memcmp(leaf->pub.point.p, k->ec_pub, leaf->pub.point.n) == 0;
		else
			match = leaf->pub.n.n == k->rsa.n.n && memcmp(leaf->pub.n.p, k->rsa.n.p, k->rsa.n.n) == 0 && leaf->pub.e == k->rsa.e;
	}
	if (!match) {
		if (why)
			*why = "tls: private key does not match public key";
		goto bad;
	}
	o->c.chain = chain;
	o->c.nchain = n;
	*out = &o->c;
	return 0;
nomem:
	alt_arena_free(&o->a);
	free(o);
	return -ENOMEM;
bad:
	alt_arena_free(&o->a);
	free(o);
	return -ALT_EX509;
}

void alt_tls_cert_free(alt_tls_cert *c) {
	if (!c)
		return;
	owned_cert *o = (owned_cert *)c;
	memset(&o->c.key, 0, sizeof o->c.key);
	alt_arena_free(&o->a);
	free(o);
}

static bool slurp(const char *path, alt_buf *b) {
	FILE *f = fopen(path, "rb");
	if (!f)
		return false;
	char tmp[16384];
	size_t k;
	while ((k = fread(tmp, 1, sizeof tmp, f)) > 0)
		alt_buf_append(b, tmp, k);
	fclose(f);
	return true;
}

int alt_tls_load_key_pair(alt_tls_cert **out, const char *cert_file, const char *key_file, const char **why) {
	alt_buf c = ALT_BUF_INIT, k = ALT_BUF_INIT;
	int rc;
	if (!slurp(cert_file, &c) || !slurp(key_file, &k)) {
		if (why)
			*why = "open: no such file";
		rc = -ENOENT;
	} else
		rc = alt_tls_key_pair(out, alt_buf_str(&c), alt_buf_str(&k), why);
	/* the key bytes were copied into the pair's arena by PEM decoding; wipe ours */
	if (k.p)
		memset(k.p, 0, k.n);
	alt_buf_free(&c);
	alt_buf_free(&k);
	return rc;
}
