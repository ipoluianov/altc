/* The TLS record layer (RFC 8446 5, RFC 5246 6): framing, protection in
   both versions, alerts, handshake message reassembly. */
#include "tls_internal.h"
#include <stdarg.h>

int tls_fail(alt_tls_conn *t, int code, const char *fmt, ...) {
	if (!t->err[0]) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(t->err, sizeof t->err, fmt, ap);
		va_end(ap);
	}
	return code;
}

const char *tls_alert_text(int a) {
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
	case A_INAPPROPRIATE_FALLBACK: return "inappropriate fallback";
	case A_INTERNAL_ERROR: return "internal error";
	case A_USER_CANCELED: return "user canceled";
	case A_NO_RENEGOTIATION: return "no renegotiation";
	case A_MISSING_EXTENSION: return "missing extension";
	case A_UNSUPPORTED_EXTENSION: return "unsupported extension";
	case A_UNRECOGNIZED_NAME: return "unrecognized name";
	case A_CERTIFICATE_REQUIRED: return "certificate required";
	case A_NO_APPLICATION_PROTOCOL: return "no application protocol";
	}
	return "alert";
}

static int write_raw(alt_tls_conn *t, const void *p, size_t n) {
	if (t->write_err)
		return t->write_err;
	ssize_t w = alt_write_all(t->wr, p, n);
	if (w < 0) {
		tls_fail(t, (int)w, "tls: write: %s", alt_strerror((int)w));
		return t->write_err = (int)w;
	}
	return 0;
}

/* the per-record nonce: the IV xor the sequence number (1.3 and ChaCha20 in 1.2) */
static void xor_nonce(const half *x, uint8_t n[12]) {
	memcpy(n, x->iv, 12);
	for (int i = 0; i < 8; i++)
		n[4 + i] ^= (uint8_t)(x->seq >> (56 - 8 * i));
}

static void put_seq(uint8_t *p, uint64_t seq) {
	for (int i = 0; i < 8; i++)
		p[i] = (uint8_t)(seq >> (56 - 8 * i));
}

/* TLS 1.2 additional data: seq, type, version, plaintext length */
static void aad12(uint8_t ad[13], uint64_t seq, int type, size_t n) {
	put_seq(ad, seq);
	ad[8] = (uint8_t)type, ad[9] = 3, ad[10] = 3, ad[11] = (uint8_t)(n >> 8), ad[12] = (uint8_t)n;
}

/* HMAC-SHA1(seq, header, data) (RFC 5246 6.2.3.1); extra is hashed after the sum to even out timing (as Go) */
static void mac12(const half *x, int type, const uint8_t *data, size_t n, const uint8_t *extra, size_t nextra, uint8_t out[20]) {
	alt_hmac m;
	uint8_t ad[13];
	aad12(ad, x->seq, type, n);
	alt_hmac_init(&m, ALT_SHA1, x->mac, 20);
	alt_hmac_write(&m, ad, 13);
	alt_hmac_write(&m, data, n);
	alt_hmac_sum(&m, out);
	if (nextra)
		alt_hmac_write(&m, extra, nextra);
	memset(&m, 0, sizeof m);
}

int tls_write_record(alt_tls_conn *t, int type, const void *data, size_t n) {
	uint8_t *rec = t->wrec;
	half *x = &t->out;
	if (!x->on) {
		/* the first ClientHello goes out as TLS 1.0 records (as Go) */
		rec[0] = (uint8_t)type, rec[1] = 3, rec[2] = type == REC_HANDSHAKE && t->client && !t->version ? 1 : 3;
		rec[3] = (uint8_t)(n >> 8), rec[4] = (uint8_t)n;
		memcpy(rec + 5, data, n);
		return write_raw(t, rec, 5 + n);
	}
	size_t total;
	uint8_t nn[12];
	if (!x->v12) {
		size_t inner = n + 1;
		total = inner + 16;
		rec[0] = REC_APP, rec[1] = 3, rec[2] = 3, rec[3] = (uint8_t)(total >> 8), rec[4] = (uint8_t)total;
		uint8_t *pt = t->wpt;
		memcpy(pt, data, n);
		pt[n] = (uint8_t)type;
		xor_nonce(x, nn);
		if (x->cipher == C_CHACHA)
			alt_chacha20poly1305_seal(x->key, nn, rec + 5, pt, inner, rec, 5);
		else
			alt_gcm_seal(&x->gcm, rec + 5, nn, 12, pt, inner, rec, 5);
		memset(pt, 0, inner);
	} else {
		rec[0] = (uint8_t)type, rec[1] = 3, rec[2] = 3;
		uint8_t ad[13];
		switch (x->cipher) {
		case C_AES_GCM:
			/* the explicit nonce is the sequence number (as Go) */
			memcpy(nn, x->iv, 4);
			put_seq(nn + 4, x->seq);
			memcpy(rec + 5, nn + 4, 8);
			aad12(ad, x->seq, type, n);
			alt_gcm_seal(&x->gcm, rec + 13, nn, 12, data, n, ad, 13);
			total = 8 + n + 16;
			break;
		case C_CHACHA:
			xor_nonce(x, nn);
			aad12(ad, x->seq, type, n);
			alt_chacha20poly1305_seal(x->key, nn, rec + 5, data, n, ad, 13);
			total = n + 16;
			break;
		default: { /* AES-CBC with HMAC-SHA1: a random explicit IV, MAC then pad then encrypt */
			uint8_t *pt = t->wpt, iv[16];
			memcpy(pt, data, n);
			mac12(x, type, data, n, 0, 0, pt + n);
			size_t len = n + 20, pad = 16 - len % 16;
			for (size_t i = 0; i < pad; i++)
				pt[len + i] = (uint8_t)(pad - 1);
			len += pad;
			int rc = alt_crypto_rand(iv, 16);
			if (rc < 0)
				return t->write_err = tls_fail(t, rc, "tls: no random bytes");
			memcpy(rec + 5, iv, 16);
			alt_aes_cbc_encrypt(&x->aes, iv, rec + 21, pt, len);
			memset(pt, 0, len);
			total = 16 + len;
		}
		}
		rec[3] = (uint8_t)(total >> 8), rec[4] = (uint8_t)total;
	}
	x->seq++;
	return write_raw(t, rec, 5 + total);
}

int tls_write_records(alt_tls_conn *t, int type, const void *data, size_t n) {
	const uint8_t *p = data;
	do {
		size_t k = n < MAX_PLAINTEXT ? n : MAX_PLAINTEXT;
		int rc = tls_write_record(t, type, p, k);
		if (rc < 0)
			return rc;
		p += k, n -= k;
	} while (n);
	return 0;
}

void tls_send_alert(alt_tls_conn *t, int level, int desc) {
	uint8_t a[2] = {(uint8_t)level, (uint8_t)desc};
	alt_mutex_lock(&t->out_mu);
	if (!t->write_err)
		tls_write_record(t, REC_ALERT, a, 2);
	alt_mutex_unlock(&t->out_mu);
}

int tls_fatal(alt_tls_conn *t, int alert, int code, const char *msg) {
	if (alert >= 0)
		tls_send_alert(t, 2, alert);
	tls_fail(t, code, "%s", msg);
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

static int bad_record(alt_tls_conn *t, int alert, const char *msg) {
	tls_send_alert(t, 2, alert);
	return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "%s", msg);
}

/* Go's extractPadding: the padding length (zeroed when bad) and 0xff if it is good, in constant time */
static size_t cbc_padding(const uint8_t *p, size_t n, uint8_t *good) {
	uint8_t plen = p[n - 1];
	unsigned tt = (unsigned)(n - 1) - plen;
	uint8_t g = (uint8_t)((int32_t)~tt >> 31);
	size_t check = n < 256 ? n : 256;
	for (size_t i = 0; i < check; i++) {
		unsigned u = (unsigned)plen - (unsigned)i;
		uint8_t mask = (uint8_t)((int32_t)~u >> 31);
		uint8_t b = p[n - 1 - i];
		g &= (uint8_t)~((mask & plen) ^ (mask & b));
	}
	g &= (uint8_t)(g << 4);
	g &= (uint8_t)(g << 2);
	g &= (uint8_t)(g << 1);
	g = (uint8_t)((int8_t)g >> 7);
	plen &= g;
	*good = g;
	return (size_t)plen + 1;
}

/* opens a TLS 1.2 record in place: the plaintext in *data, *n */
static int open12(alt_tls_conn *t, int ty, uint8_t *body, size_t len, uint8_t **data, size_t *n) {
	half *x = &t->in;
	uint8_t nn[12], ad[13];
	switch (x->cipher) {
	case C_AES_GCM:
		if (len < 8 + 16)
			return -1;
		memcpy(nn, x->iv, 4);
		memcpy(nn + 4, body, 8);
		aad12(ad, x->seq, ty, len - 24);
		if (alt_gcm_open(&x->gcm, body + 8, nn, 12, body + 8, len - 8, ad, 13) < 0)
			return -1;
		*data = body + 8, *n = len - 24;
		return 0;
	case C_CHACHA:
		if (len < 16)
			return -1;
		xor_nonce(x, nn);
		aad12(ad, x->seq, ty, len - 16);
		if (alt_chacha20poly1305_open(x->key, nn, body, body, len, ad, 13) < 0)
			return -1;
		*data = body, *n = len - 16;
		return 0;
	default: {
		/* AES-CBC: the IV, then at least one block holding the MAC and the padding */
		if (len % 16 || len < 16 + 32)
			return -1;
		uint8_t iv[16];
		memcpy(iv, body, 16);
		uint8_t *p = body + 16;
		size_t m = len - 16;
		alt_aes_cbc_decrypt(&x->aes, iv, p, p, m);
		uint8_t good;
		size_t pad = cbc_padding(p, m, &good);
		/* n = m - 20 - pad, clamped at 0 without a branch on secret data */
		size_t k = m - 20 - pad;
		size_t neg = (size_t)0 - (k >> (sizeof k * 8 - 1)); /* all ones if k < 0 */
		k &= ~neg;
		uint8_t mac[20];
		mac12(x, ty, p, k, p + k + 20, m - k - 20, mac);
		int ok = alt_ct_eq(mac, p + k, 20) & (good & 1);
		memset(mac, 0, sizeof mac);
		if (!ok)
			return -1;
		*data = p, *n = k;
		return 0;
	}
	}
}

int tls_read_record(alt_tls_conn *t, int *type, uint8_t **data, size_t *n) {
	for (;;) {
		ssize_t k = read_full(t->rd, t->rec, 5);
		if (k < 0)
			return t->read_err = tls_fail(t, (int)k, "tls: %s", alt_strerror((int)k));
		if (k == 0) {
			t->raw_eof = true;
			return t->read_err = tls_fail(t, -ALT_EUNEXPECTED_EOF, "EOF");
		}
		int ty = t->rec[0];
		size_t len = (size_t)t->rec[3] << 8 | t->rec[4];
		if (ty < REC_CCS || ty > REC_APP)
			/* not TLS: Go's "first record does not look like a TLS handshake" */
			return bad_record(t, A_UNEXPECTED_MESSAGE, !t->done && !t->in.on ? "tls: first record does not look like a TLS handshake"
										      : "tls: unexpected record type");
		if (t->version && t->rec[1] != 3) {
			return bad_record(t, A_PROTOCOL_VERSION, "tls: received record with version that is not TLS");
		}
		size_t lim = !t->in.on ? (size_t)MAX_PLAINTEXT : t->in.v12 ? (size_t)MAX_CIPHERTEXT : (size_t)MAX_PLAINTEXT + 256;
		if (len > lim) {
			tls_send_alert(t, 2, A_RECORD_OVERFLOW);
			return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: oversized record received with length %zu", len);
		}
		k = read_full(t->rd, t->rec + 5, len);
		if (k < 0 || (size_t)k != len)
			return t->read_err = tls_fail(t, k < 0 ? (int)k : -ALT_EUNEXPECTED_EOF, "unexpected EOF");
		uint8_t *body = t->rec + 5;
		if (ty == REC_CCS && t->version != VERSION_12) {
			/* TLS 1.3 compatibility mode (RFC 8446 D.4): one byte 1, only during the handshake, never encrypted */
			if (!t->ccs_ok || len != 1 || body[0] != 1)
				return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: unexpected change cipher spec");
			continue;
		}
		if (!t->in.on) {
			if (ty == REC_APP)
				return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: unexpected application data");
			if (ty == REC_CCS) {
				/* TLS 1.2: switches the reading keys at a message boundary, when the handshake expects it */
				if (len != 1 || body[0] != 1)
					return bad_record(t, A_DECODE_ERROR, "tls: bad change cipher spec");
				if (!t->has_pending_in || t->hs.n)
					return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: unexpected change cipher spec");
				t->in = t->pending_in;
				t->has_pending_in = false;
				memset(&t->pending_in, 0, sizeof t->pending_in);
				*type = ty, *data = body, *n = len;
				return 0;
			}
			*type = ty, *data = body, *n = len;
			return 0;
		}
		if (t->in.v12) {
			if (ty == REC_CCS)
				return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: unexpected change cipher spec");
			if (open12(t, ty, body, len, data, n) < 0)
				return bad_record(t, A_BAD_RECORD_MAC, "tls: invalid record MAC");
			t->in.seq++;
			if (*n > MAX_PLAINTEXT)
				return bad_record(t, A_RECORD_OVERFLOW, "tls: oversized record");
			*type = ty;
			return 0;
		}
		if (ty != REC_APP)
			return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: unencrypted record after the keys changed");
		uint8_t nn[12];
		xor_nonce(&t->in, nn);
		int rc = len < 17 ? -ALT_EAUTH
				  : t->in.cipher == C_CHACHA ? alt_chacha20poly1305_open(t->in.key, nn, body, body, len, t->rec, 5)
							     : alt_gcm_open(&t->in.gcm, body, nn, 12, body, len, t->rec, 5);
		if (rc < 0)
			return bad_record(t, A_BAD_RECORD_MAC, "tls: invalid record MAC");
		t->in.seq++;
		size_t m = len - 16;
		while (m > 0 && body[m - 1] == 0)
			m--;
		if (m == 0)
			return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: record without a content type");
		*type = body[m - 1];
		*data = body;
		*n = m - 1;
		if (*n > MAX_PLAINTEXT)
			return bad_record(t, A_RECORD_OVERFLOW, "tls: oversized record");
		if (*type == REC_CCS)
			return bad_record(t, A_UNEXPECTED_MESSAGE, "tls: encrypted change cipher spec");
		return 0;
	}
}

int tls_on_alert(alt_tls_conn *t, const uint8_t *d, size_t n) {
	if (n != 2)
		return bad_record(t, A_DECODE_ERROR, "tls: bad alert");
	if (d[1] == A_CLOSE_NOTIFY) {
		t->eof = true;
		return 0;
	}
	/* warnings: user_canceled and (in TLS 1.2) other warning-level alerts are ignored, as Go */
	if (d[1] == A_USER_CANCELED || (d[0] == 1 && t->version == VERSION_12 && d[1] != A_NO_RENEGOTIATION))
		return 1;
	return t->read_err = tls_fail(t, -ALT_ETLS_ALERT, "remote error: tls: %s", tls_alert_text(d[1]));
}

int tls_read_handshake(alt_tls_conn *t, int *type, alt_buf *msg) {
	for (;;) {
		if (t->hs.n >= 4) {
			size_t len = (size_t)(uint8_t)t->hs.p[1] << 16 | (size_t)(uint8_t)t->hs.p[2] << 8 | (uint8_t)t->hs.p[3];
			if (len > 1 << 18)
				return tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: handshake message too large");
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
		int rc = tls_read_record(t, &ty, &d, &n);
		if (rc < 0)
			return rc;
		if (ty == REC_ALERT) {
			rc = tls_on_alert(t, d, n);
			if (rc < 0)
				return rc;
			if (rc == 0)
				return t->read_err = tls_fail(t, -ALT_EUNEXPECTED_EOF, "EOF");
			continue;
		}
		if (ty == REC_CCS)
			continue; /* TLS 1.2: the keys switched; the Finished follows */
		if (ty != REC_HANDSHAKE)
			return tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected record during the handshake");
		if (n == 0)
			return tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: empty handshake record");
		alt_buf_append(&t->hs, d, n);
	}
}

int tls_expect(alt_tls_conn *t, int want, alt_buf *msg, tr *body) {
	int ty, rc = tls_read_handshake(t, &ty, msg);
	if (rc < 0)
		return rc;
	if (ty != want) {
		char m[96];
		snprintf(m, sizeof m, "tls: unexpected handshake message type %d (wanted %d)", ty, want);
		return tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, m);
	}
	*body = (tr){(const uint8_t *)msg->p + 4, msg->n - 4};
	return 0;
}

int tls_send_handshake(alt_tls_conn *t, const alt_buf *msg) {
	alt_buf_append(&t->transcript, msg->p, msg->n);
	alt_mutex_lock(&t->out_mu);
	int rc = tls_write_records(t, REC_HANDSHAKE, msg->p, msg->n);
	alt_mutex_unlock(&t->out_mu);
	return rc;
}

int tls_keys_boundary(alt_tls_conn *t) {
	if (t->hs.n)
		return tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: handshake message spans a key change");
	return 0;
}

int tls_send_ccs(alt_tls_conn *t) {
	static const uint8_t ccs[6] = {REC_CCS, 3, 3, 0, 1, 1};
	alt_mutex_lock(&t->out_mu);
	int rc = write_raw(t, ccs, 6);
	alt_mutex_unlock(&t->out_mu);
	return rc;
}
