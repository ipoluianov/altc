/* TLS 1.3 (RFC 8446): the key schedule, the client and server handshakes
   after the hellos, resumption with PSKs from tickets, post-handshake
   messages. */
#include "tls_internal.h"

static alt_hash_kind suite_hash(uint16_t s) { return s == ALT_TLS_AES_256_GCM_SHA384 ? ALT_SHA384 : ALT_SHA256; }
static size_t suite_key_len(uint16_t s) { return s == ALT_TLS_AES_128_GCM_SHA256 ? 16 : 32; }

enum { TICKET_LIFETIME = 7 * 24 * 3600 }; /* seconds, Go's maxSessionTicketLifetime */

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

void tls_set_keys13(half *x, uint16_t suite, const uint8_t *secret) {
	alt_hash_kind h = suite_hash(suite);
	size_t hl = alt_hash_size(h);
	memcpy(x->secret, secret, hl);
	x->suite = suite;
	x->v12 = false;
	x->cipher = suite == ALT_TLS_CHACHA20_POLY1305_SHA256 ? C_CHACHA : C_AES_GCM;
	x->key_len = suite_key_len(suite);
	expand_label(h, secret, "key", "", 0, x->key, x->key_len);
	expand_label(h, secret, "iv", "", 0, x->iv, 12);
	if (x->cipher == C_AES_GCM)
		alt_gcm_init(&x->gcm, x->key, x->key_len);
	x->seq = 0;
	x->on = 1;
}

void tls_next_secret13(half *x) {
	alt_hash_kind h = suite_hash(x->suite);
	uint8_t s[48];
	expand_label(h, x->secret, "traffic upd", "", 0, s, alt_hash_size(h));
	tls_set_keys13(x, x->suite, s);
}

/* ---- signatures (CertificateVerify) ---- */

/* the signed content: 64 spaces, the context string, 0, the transcript hash */
static size_t signed_content(alt_tls_conn *t, bool server, uint8_t *out) {
	memset(out, ' ', 64);
	memcpy(out + 64, server ? "TLS 1.3, server CertificateVerify" : "TLS 1.3, client CertificateVerify", 33);
	out[97] = 0;
	transcript_hash(t, out + 98);
	return 98 + alt_hash_size(suite_hash(t->suite));
}

static int send_cert_verify(alt_tls_conn *t, const alt_tls_cert *c, uint16_t scheme, bool server) {
	uint8_t content[200], sig[ALT_RSA_MAX_BYTES];
	size_t n = signed_content(t, server, content), ns;
	int rc = tls_sign(&c->key, scheme, content, n, sig, &ns);
	if (rc < 0)
		return tls_fatal(t, A_INTERNAL_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to sign handshake");
	alt_buf m = ALT_BUF_INIT;
	tb b = {.b = &m};
	tb_u8(&b, HS_CERTIFICATE_VERIFY);
	tb_begin(&b, 3);
	tb_u16(&b, scheme);
	tb_begin(&b, 2);
	tb_bytes(&b, sig, ns);
	tb_end(&b);
	tb_end(&b);
	rc = m.err ? m.err : tls_send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

static int read_cert_verify(alt_tls_conn *t, bool server) {
	uint8_t content[200];
	size_t n = signed_content(t, server, content);
	alt_buf m = ALT_BUF_INIT;
	tr body, sig;
	unsigned scheme;
	int rc = tls_expect(t, HS_CERTIFICATE_VERIFY, &m, &body);
	if (rc < 0)
		goto out;
	if (!tr_u16(&body, &scheme) || !tr_vec(&body, 2, &sig) || body.n) {
		rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate verify");
		goto out;
	}
	bool ok = false;
	for (size_t i = 0; i < tls_nsig_schemes; i++)
		ok |= tls_sig_schemes[i] == scheme;
	if (!ok || scheme == S_RSA_PKCS1_SHA256 || scheme == S_RSA_PKCS1_SHA384 || scheme == S_RSA_PKCS1_SHA512) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: certificate used with invalid signature algorithm");
		goto out;
	}
	if (tls_verify_sig(t->peer[0], (uint16_t)scheme, false, content, n, sig.p, sig.n) < 0) {
		rc = tls_fatal(t, A_DECRYPT_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid signature by the peer certificate");
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

static void finished_msg(alt_tls_conn *t, const uint8_t *base, alt_buf *m) {
	uint8_t mac[48];
	finished_mac(t, base, mac);
	size_t hl = alt_hash_size(suite_hash(t->suite));
	tb b = {.b = m};
	tb_u8(&b, HS_FINISHED);
	tb_u24(&b, (unsigned)hl);
	tb_bytes(&b, mac, hl);
}

static int send_finished(alt_tls_conn *t, const uint8_t *base) {
	alt_buf m = ALT_BUF_INIT;
	finished_msg(t, base, &m);
	int rc = m.err ? m.err : tls_send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

static int read_finished(alt_tls_conn *t, const uint8_t *base) {
	uint8_t mac[48];
	finished_mac(t, base, mac);
	size_t hl = alt_hash_size(suite_hash(t->suite));
	alt_buf m = ALT_BUF_INIT;
	tr body;
	int rc = tls_expect(t, HS_FINISHED, &m, &body);
	if (rc == 0) {
		if (body.n != hl || !alt_ct_eq(body.p, mac, hl))
			rc = tls_fatal(t, A_DECRYPT_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid finished hash");
		else
			alt_buf_append(&t->transcript, m.p, m.n);
	}
	alt_buf_free(&m);
	return rc;
}

/* ---- the key schedule ---- */

/* the early secret from the PSK (zeros without one) */
static void early_secret(alt_hash_kind h, const uint8_t *psk, uint8_t *early) {
	size_t hl = alt_hash_size(h);
	uint8_t zero[48] = {0};
	alt_hkdf_extract(h, 0, 0, psk ? psk : zero, hl, early);
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

static void hs_secrets(alt_tls_conn *t, const uint8_t *psk, const uint8_t *shared, size_t ns, uint8_t *c_hs, uint8_t *s_hs) {
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	uint8_t early[48], derived[48];
	early_secret(h, psk, early);
	derive_secret(t, early, "derived", false, derived);
	alt_hkdf_extract(h, derived, hl, shared, ns, t->hs_secret);
	derive_secret(t, t->hs_secret, "c hs traffic", true, c_hs);
	derive_secret(t, t->hs_secret, "s hs traffic", true, s_hs);
	memset(early, 0, sizeof early);
}

/* the PSK binder over the transcript's first prefix bytes and the hello without its binders */
static void binder(alt_tls_conn *t, alt_hash_kind h, const uint8_t *psk, size_t prefix, const void *hello, size_t n, uint8_t *out) {
	size_t hl = alt_hash_size(h);
	uint8_t early[48], bk[48], fk[48], th[48], empty[48];
	early_secret(h, psk, early);
	alt_hash_data(h, "", 0, empty);
	expand_label(h, early, "res binder", empty, hl, bk, hl);
	expand_label(h, bk, "finished", "", 0, fk, hl);
	alt_hash st;
	alt_hash_init(&st, h);
	alt_hash_write(&st, t->transcript.p, prefix);
	alt_hash_write(&st, hello, n);
	alt_hash_sum(&st, th);
	alt_hmac_data(h, fk, hl, th, hl, out);
	memset(early, 0, sizeof early);
	memset(bk, 0, sizeof bk);
	memset(fk, 0, sizeof fk);
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

void tls_client_binder(alt_tls_conn *t, hello_offer *o, alt_buf *m) {
	if (!o->session || o->session->version != VERSION_13 || m->err)
		return;
	alt_hash_kind h = suite_hash(o->psk_suite);
	size_t hl = alt_hash_size(h);
	size_t cut = m->n - (2 + 1 + hl);
	binder(t, h, o->session->secret, t->transcript.n, m->p, cut, (uint8_t *)m->p + m->n - hl);
}

int tls_client13(alt_tls_conn *t, hello_offer *o, server_hello *sh, alt_buf *in, size_t ch1) {
	int rc;
	tr body;
	alt_buf m = ALT_BUF_INIT;
	alt_buf req_ctx = ALT_BUF_INIT;
	uint16_t suites[16], groups[8];
	size_t ns = tls_suites13(&t->cfg, tls_aes_hardware(), suites), ng = tls_groups_of(&t->cfg, groups);
	bool offered = false;
	for (size_t i = 0; i < ns; i++)
		offered |= suites[i] == sh->suite;
	if (!offered || sh->sid.n != o->nsid || memcmp(sh->sid.p, o->sid, o->nsid)) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, !offered ? "tls: server chose an unconfigured cipher suite"
									      : "tls: server echoed an invalid session ID");
		goto out;
	}
	t->suite = (uint16_t)sh->suite;
	alt_buf_append(&t->transcript, in->p, in->n);
	if (memcmp(sh->random, hrr_random, 32) == 0) {
		/* HelloRetryRequest: a new share for the group the server asks for */
		bool ok = sh->group != o->share.group && tls_group_supported((uint16_t)sh->group) && !sh->has_share;
		bool listed = false;
		for (size_t i = 0; i < ng; i++)
			listed |= groups[i] == sh->group;
		if (!ok || !listed) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported group");
			goto out;
		}
		transcript_hrr(t, ch1);
		if ((rc = tls_make_share(&o->share, (uint16_t)sh->group)) < 0)
			goto out;
		/* a PSK whose hash differs from the suite's is dropped (as Go) */
		if (o->session && o->session->version == VERSION_13 && suite_hash(o->psk_suite) != suite_hash(t->suite))
			o->session = 0;
		/* the cookie lives in in, which is not reused before the new ClientHello is built */
		alt_buf cookie = ALT_BUF_INIT;
		alt_buf_append(&cookie, sh->cookie.p, sh->cookie.n);
		tr ck = {(const uint8_t *)cookie.p, cookie.n};
		rc = tls_build_client_hello(t, &m, o, sh->has_cookie ? &ck : 0);
		alt_buf_free(&cookie);
		if (rc < 0)
			goto out;
		tls_client_binder(t, o, &m);
		if ((rc = tls_send_ccs(t)) < 0 || (rc = tls_send_handshake(t, &m)) < 0)
			goto out;
		uint16_t hrr_suite = t->suite;
		if ((rc = tls_expect(t, HS_SERVER_HELLO, in, &body)) < 0 || (rc = tls_parse_server_hello(t, body, sh)) < 0)
			goto out;
		if (memcmp(sh->random, hrr_random, 32) == 0 || sh->suite != hrr_suite || !sh->has_version || sh->version != VERSION_13) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server sent an invalid ServerHello after the retry");
			goto out;
		}
		alt_buf_append(&t->transcript, in->p, in->n);
	}
	if (!sh->has_share || sh->group != o->share.group) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported group");
		goto out;
	}
	/* the PSK: accepted when the server names it */
	const uint8_t *psk = 0;
	bool have_psk = o->session && o->session->version == VERSION_13;
	if (sh->has_psk) {
		if (!have_psk || sh->psk_index != 0) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected an invalid PSK");
			goto out;
		}
		if (suite_hash(o->psk_suite) != suite_hash(t->suite)) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected an invalid PSK and cipher suite pair");
			goto out;
		}
		psk = o->session->secret;
		t->did_resume = true;
		if (o->session->alpn[0])
			snprintf(t->alpn, sizeof t->alpn, "%s", o->session->alpn);
	} else {
		/* a full handshake: drop the certificates restored from the session */
		t->peer = 0, t->npeer = 0, t->chains = 0, t->nchains = 0;
	}
	t->group = o->share.group;
	uint8_t shared[66], c_hs[48], s_hs[48];
	size_t nshared;
	if (tls_shared_secret(&o->share, sh->share.p, sh->share.n, shared, &nshared) < 0) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid server key share");
		goto out;
	}
	memset(&o->share, 0, sizeof o->share);
	hs_secrets(t, psk, shared, nshared, c_hs, s_hs);
	memset(shared, 0, sizeof shared);
	if ((rc = tls_keys_boundary(t)) < 0)
		goto out;
	tls_set_keys13(&t->in, t->suite, s_hs);
	/* EncryptedExtensions */
	if ((rc = tls_expect(t, HS_ENCRYPTED_EXTENSIONS, in, &body)) < 0)
		goto out;
	tr exts;
	if (!tr_vec(&body, 2, &exts) || body.n) {
		rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse EncryptedExtensions");
		goto out;
	}
	while (exts.n) {
		unsigned type;
		tr data;
		if (!tr_u16(&exts, &type) || !tr_vec(&exts, 2, &data)) {
			rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse EncryptedExtensions");
			goto out;
		}
		if (type == EXT_ALPN) {
			tr list, proto;
			if (!tr_vec(&data, 2, &list) || !tr_vec(&list, 1, &proto) || list.n || data.n || proto.n == 0) {
				rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ALPN");
				goto out;
			}
			if ((rc = tls_check_alpn(t, &proto)) < 0)
				goto out;
		}
	}
	alt_buf_append(&t->transcript, in->p, in->n);
	bool cert_requested = false;
	uint16_t peer_schemes[64];
	size_t npeer_schemes = 0;
	if (!t->did_resume) {
		/* CertificateRequest (optional), Certificate, CertificateVerify */
		int ty;
		if ((rc = tls_read_handshake(t, &ty, in)) < 0)
			goto out;
		if (ty == HS_CERTIFICATE_REQUEST) {
			tr b2 = {(const uint8_t *)in->p + 4, in->n - 4}, ctx, ex;
			if (!tr_vec(&b2, 1, &ctx) || !tr_vec(&b2, 2, &ex) || b2.n) {
				rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse CertificateRequest");
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
			alt_buf_append(&t->transcript, in->p, in->n);
			if ((rc = tls_read_handshake(t, &ty, in)) < 0)
				goto out;
		}
		if (ty != HS_CERTIFICATE) {
			rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected handshake message (wanted Certificate)");
			goto out;
		}
		if ((rc = tls_read_certificate(t, (tr){(const uint8_t *)in->p + 4, in->n - 4}, true, false)) < 0)
			goto out;
		alt_buf_append(&t->transcript, in->p, in->n);
		if (!t->cfg.insecure_skip_verify && (rc = tls_verify_peer(t, t->cfg.server_name, t->cfg.roots, ALT_X509_EKU_SERVER_AUTH)) < 0)
			goto out;
		if ((rc = read_cert_verify(t, true)) < 0)
			goto out;
	}
	if ((rc = read_finished(t, s_hs)) < 0)
		goto out;
	uint8_t c_ap[48], s_ap[48];
	app_secrets(t, c_ap, s_ap);
	if ((rc = tls_keys_boundary(t)) < 0)
		goto out;
	tls_set_keys13(&t->in, t->suite, s_ap);
	/* our second flight: compatibility CCS, our handshake keys */
	if ((rc = tls_send_ccs(t)) < 0)
		goto out;
	alt_mutex_lock(&t->out_mu);
	tls_set_keys13(&t->out, t->suite, c_hs);
	alt_mutex_unlock(&t->out_mu);
	if (cert_requested) {
		/* the first certificate whose key fits the server's schemes (Go's getClientCertificate) */
		const alt_tls_cert *mine = 0;
		uint16_t scheme = 0;
		for (size_t i = 0; i < t->cfg.ncerts && !mine; i++)
			if ((scheme = tls_pick_scheme(&t->cfg.certs[i]->key, VERSION_13, peer_schemes, npeer_schemes)))
				mine = t->cfg.certs[i];
		tr ctx = {(const uint8_t *)req_ctx.p, req_ctx.n};
		if ((rc = tls_send_certificate(t, mine, true, &ctx)) < 0 || (mine && (rc = send_cert_verify(t, mine, scheme, false)) < 0))
			goto out;
	}
	if ((rc = send_finished(t, c_hs)) < 0)
		goto out;
	alt_mutex_lock(&t->out_mu);
	tls_set_keys13(&t->out, t->suite, c_ap);
	alt_mutex_unlock(&t->out_mu);
	derive_secret(t, t->master, "res master", true, t->resumption);
	t->ccs_ok = false;
	memset(c_hs, 0, sizeof c_hs);
	memset(s_hs, 0, sizeof s_hs);
	rc = 0;
out:
	alt_buf_free(&m);
	alt_buf_free(&req_ctx);
	return rc;
}

/* ---- the server ---- */

/* a session from the client's PSK identities: the PSK in psk, or false (Go's checkForResumption) */
static bool server_psk(alt_tls_conn *t, const client_hello *ch, const alt_buf *raw, size_t prefix, uint8_t *psk, int *rc,
	unsigned *index) {
	*rc = 0;
	if (t->cfg.session_tickets_disabled || !ch->npsk || !ch->psk_dhe_ke)
		return false;
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	int64_t now = alt_wall_unix(alt_wall_now());
	tr binders = ch->binders;
	for (size_t i = 0; i < ch->npsk; i++) {
		tr bd;
		if (!tr_vec(&binders, 1, &bd)) {
			*rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid PSK binders");
			return false;
		}
		tls_session *s = tls_open_ticket(t, ch->psk[i].identity.p, ch->psk[i].identity.n);
		if (!s)
			continue;
		int ca = t->cfg.client_auth;
		bool needs_cert = ca == ALT_TLS_REQUIRE_ANY_CLIENT_CERT || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT;
		bool skip = s->version != VERSION_13 || now - s->created > TICKET_LIFETIME || suite_hash(s->suite) != h ||
			    (needs_cert && !s->ncerts_bytes) || s->nsecret != hl;
		if (skip) {
			tls_session_free(s);
			continue;
		}
		uint8_t want[48];
		binder(t, h, s->secret, prefix, raw->p, raw->n - ch->binders_off, want);
		if (bd.n != hl || !alt_ct_eq(want, bd.p, hl)) {
			tls_session_free(s);
			*rc = tls_fatal(t, A_DECRYPT_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid PSK binder");
			return false;
		}
		memcpy(psk, s->secret, hl);
		if (s->ncerts_bytes && (*rc = tls_session_peer_certs(t, s)) < 0) {
			tls_session_free(s);
			return false;
		}
		tls_session_free(s);
		*index = (unsigned)i;
		return true;
	}
	return false;
}

/* NewSessionTicket for the resumption secret (Go sends one, with an empty nonce) */
static int send_ticket(alt_tls_conn *t, const uint8_t *res_master) {
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	tls_session s = {.version = VERSION_13, .suite = t->suite, .group = t->group, .nsecret = hl, .created = alt_wall_unix(alt_wall_now())};
	expand_label(h, res_master, "resumption", "", 0, s.secret, hl);
	snprintf(s.alpn, sizeof s.alpn, "%s", t->alpn);
	int rc = tls_session_set_certs(&s, t);
	alt_buf ticket = ALT_BUF_INIT, m = ALT_BUF_INIT;
	uint32_t age_add;
	if (rc == 0)
		rc = alt_crypto_rand(&age_add, 4);
	if (rc == 0)
		rc = tls_seal_ticket(t, &s, &ticket);
	if (rc == 0) {
		tb b = {.b = &m};
		tb_u8(&b, HS_NEW_SESSION_TICKET);
		tb_begin(&b, 3);
		tb_u32(&b, TICKET_LIFETIME);
		tb_u32(&b, age_add);
		tb_u8(&b, 0); /* nonce */
		tb_begin(&b, 2);
		tb_bytes(&b, ticket.p, ticket.n);
		tb_end(&b);
		tb_u16(&b, 0);
		tb_end(&b);
		/* not part of the transcript */
		if (m.err)
			rc = m.err;
		else {
			alt_mutex_lock(&t->out_mu);
			rc = tls_write_records(t, REC_HANDSHAKE, m.p, m.n);
			alt_mutex_unlock(&t->out_mu);
		}
		t->tickets_sent++;
	}
	free(s.certs);
	memset(s.secret, 0, sizeof s.secret);
	alt_buf_free(&ticket);
	alt_buf_free(&m);
	return rc;
}

int tls_server13(alt_tls_conn *t, client_hello *ch, alt_buf *ch_raw) {
	alt_buf in = ALT_BUF_INIT, m = ALT_BUF_INIT, ch2_raw = ALT_BUF_INIT;
	alt_arena scratch;
	alt_arena_init(&scratch, 0);
	tr body;
	int rc;
	t->ccs_ok = true;
	if (ch->comp.n != 1 || ch->comp.p[0] != 0) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: TLS 1.3 client supports illegal compression methods");
		goto out;
	}
	/* the suite: ours in preference order (Go's: AES-GCM first unless the client prefers ChaCha20) */
	uint16_t mine[16];
	size_t nm = tls_suites13(&t->cfg, tls_aes_preferred(&ch->suites), mine);
	t->suite = 0;
	for (size_t i = 0; i < nm && !t->suite; i++)
		for (size_t k = 0; k + 1 < ch->suites.n; k += 2)
			if (((ch->suites.p[k] << 8) | ch->suites.p[k + 1]) == mine[i]) {
				t->suite = mine[i];
				break;
			}
	if (!t->suite) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: no cipher suite supported by both client and server");
		goto out;
	}
	/* the group: ours in preference order, among the client's */
	uint16_t groups[8];
	size_t ng = tls_groups_of(&t->cfg, groups);
	uint16_t group = 0;
	for (size_t i = 0; i < ng && !group; i++)
		for (size_t k = 0; k < ch->ngroups; k++)
			if (ch->groups[k] == groups[i]) {
				group = groups[i];
				break;
			}
	if (!group) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: no ECDHE curve supported by both client and server");
		goto out;
	}
	int share_i = -1;
	bool sent_hrr_ccs = false;
	for (size_t k = 0; k < ch->nshares; k++)
		if (ch->shares[k].group == group)
			share_i = (int)k;
	alt_buf *hello_raw = ch_raw;
	if (share_i < 0) {
		/* HelloRetryRequest */
		size_t ch1 = t->transcript.n;
		tb b = {.b = &m};
		tb_u8(&b, HS_SERVER_HELLO);
		tb_begin(&b, 3);
		tb_u16(&b, VERSION_12);
		tb_bytes(&b, hrr_random, 32);
		tb_begin(&b, 1);
		tb_bytes(&b, ch->sid.p, ch->sid.n);
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
		if ((rc = tls_send_handshake(t, &m)) < 0)
			goto out;
		if (ch->sid.n && (rc = tls_send_ccs(t)) < 0)
			goto out;
		sent_hrr_ccs = ch->sid.n > 0;
		/* the second ClientHello: the same, with the share; it is what counts from here (as Go) */
		if ((rc = tls_expect(t, HS_CLIENT_HELLO, &in, &body)) < 0)
			goto out;
		alt_buf_append(&ch2_raw, in.p, in.n);
		client_hello c2;
		if ((rc = tls_parse_client_hello(t, (tr){(const uint8_t *)ch2_raw.p + 4, ch2_raw.n - 4}, &c2)) < 0)
			goto out;
		share_i = -1;
		for (size_t k = 0; k < c2.nshares; k++)
			if (c2.shares[k].group == group)
				share_i = (int)k;
		if (share_i < 0 || c2.nshares != 1 || c2.suites.n != ch->suites.n || memcmp(c2.suites.p, ch->suites.p, ch->suites.n) ||
			c2.sid.n != ch->sid.n || memcmp(c2.sid.p, ch->sid.p, ch->sid.n)) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: client sent invalid key share in second ClientHello");
			goto out;
		}
		int si = share_i;
		*ch = c2;
		ch->shares[0] = c2.shares[si];
		ch->nshares = 1;
		share_i = 0;
		hello_raw = &ch2_raw;
	}
	t->group = group;
	if (ch->has_sni)
		snprintf(t->sni, sizeof t->sni, "%s", ch->sni);
	/* resumption: the PSK binder covers the transcript before this hello */
	uint8_t psk[48];
	unsigned psk_index = 0;
	size_t prefix = hello_raw == ch_raw ? t->transcript.n - ch_raw->n : t->transcript.n;
	bool resumed = server_psk(t, ch, hello_raw, prefix, psk, &rc, &psk_index);
	if (rc < 0)
		goto out;
	if (hello_raw != ch_raw)
		alt_buf_append(&t->transcript, hello_raw->p, hello_raw->n);
	if ((rc = tls_negotiate_alpn(t, ch)) < 0)
		goto out;
	const alt_tls_cert *cert = 0;
	uint16_t scheme = 0;
	if (!resumed) {
		if (ch->nschemes == 0) {
			rc = tls_fatal(t, A_MISSING_EXTENSION, -ALT_ETLS_HANDSHAKE, "tls: client sent no signature algorithms");
			goto out;
		}
		cert = tls_choose_cert(t, ch, &scratch);
		scheme = cert ? tls_pick_scheme(&cert->key, VERSION_13, ch->schemes, ch->nschemes) : 0;
		if (!cert || !scheme) {
			rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE,
				cert ? "tls: peer doesn't support any of the certificate's signature algorithms" : "tls: no certificates configured");
			goto out;
		}
	}
	t->did_resume = resumed;
	/* our share, the secrets */
	keyshare share;
	uint8_t shared[66], c_hs[48], s_hs[48], random[32];
	size_t nshared;
	if ((rc = tls_make_share(&share, group)) < 0 || (rc = alt_crypto_rand(random, 32)) < 0)
		goto out;
	if (tls_shared_secret(&share, ch->shares[share_i].key.p, ch->shares[share_i].key.n, shared, &nshared) < 0) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid client key share");
		goto out;
	}
	alt_buf_reset(&m);
	tb b = {.b = &m};
	tb_u8(&b, HS_SERVER_HELLO);
	tb_begin(&b, 3);
	tb_u16(&b, VERSION_12);
	tb_bytes(&b, random, 32);
	tb_begin(&b, 1);
	tb_bytes(&b, ch->sid.p, ch->sid.n);
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
	if (resumed) {
		tb_u16(&b, EXT_PRE_SHARED_KEY);
		tb_u16(&b, 2);
		tb_u16(&b, psk_index);
	}
	tb_end(&b);
	tb_end(&b);
	if ((rc = tls_send_handshake(t, &m)) < 0)
		goto out;
	if (ch->sid.n && !sent_hrr_ccs && (rc = tls_send_ccs(t)) < 0)
		goto out;
	hs_secrets(t, resumed ? psk : 0, shared, nshared, c_hs, s_hs);
	memset(psk, 0, sizeof psk);
	memset(shared, 0, sizeof shared);
	memset(&share, 0, sizeof share);
	if ((rc = tls_keys_boundary(t)) < 0)
		goto out;
	alt_mutex_lock(&t->out_mu);
	tls_set_keys13(&t->out, t->suite, s_hs);
	alt_mutex_unlock(&t->out_mu);
	tls_set_keys13(&t->in, t->suite, c_hs);
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
	if ((rc = tls_send_handshake(t, &m)) < 0)
		goto out;
	bool want_cert = !resumed && t->cfg.client_auth != ALT_TLS_NO_CLIENT_CERT;
	if (want_cert) {
		alt_buf_reset(&m);
		tb_u8(&b, HS_CERTIFICATE_REQUEST);
		tb_begin(&b, 3);
		tb_u8(&b, 0); /* empty context */
		tb_begin(&b, 2);
		tb_u16(&b, EXT_SIGNATURE_ALGORITHMS);
		tb_begin(&b, 2);
		tb_begin(&b, 2);
		for (size_t i = 0; i < tls_nsig_schemes; i++)
			tb_u16(&b, tls_sig_schemes[i]);
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
		if ((rc = tls_send_handshake(t, &m)) < 0)
			goto out;
	}
	if (!resumed && ((rc = tls_send_certificate(t, cert, true, 0)) < 0 || (rc = send_cert_verify(t, cert, scheme, true)) < 0))
		goto out;
	if ((rc = send_finished(t, s_hs)) < 0)
		goto out;
	uint8_t c_ap[48], s_ap[48];
	app_secrets(t, c_ap, s_ap);
	alt_mutex_lock(&t->out_mu);
	tls_set_keys13(&t->out, t->suite, s_ap);
	alt_mutex_unlock(&t->out_mu);
	bool tickets = !t->cfg.session_tickets_disabled && ch->psk_dhe_ke;
	if (tickets && !want_cert) {
		/* Go's first-flight tickets: the client's Finished is known in advance */
		alt_buf fin = ALT_BUF_INIT;
		finished_msg(t, c_hs, &fin);
		size_t mark = t->transcript.n;
		alt_buf_append(&t->transcript, fin.p, fin.n);
		uint8_t res[48];
		derive_secret(t, t->master, "res master", true, res);
		alt_buf_truncate(&t->transcript, mark);
		alt_buf_free(&fin);
		rc = send_ticket(t, res);
		memset(res, 0, sizeof res);
		if (rc < 0)
			goto out;
	}
	if (want_cert) {
		alt_buf_reset(&in);
		if ((rc = tls_expect(t, HS_CERTIFICATE, &in, &body)) < 0 || (rc = tls_read_certificate(t, body, true, true)) < 0)
			goto out;
		alt_buf_append(&t->transcript, in.p, in.n);
		int ca = t->cfg.client_auth;
		if (t->npeer == 0 && (ca == ALT_TLS_REQUIRE_ANY_CLIENT_CERT || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT)) {
			rc = tls_fatal(t, A_CERTIFICATE_REQUIRED, -ALT_ETLS_HANDSHAKE, "tls: client didn't provide a certificate");
			goto out;
		}
		if (t->npeer && (ca == ALT_TLS_VERIFY_CLIENT_CERT_IF_GIVEN || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT) &&
			(rc = tls_verify_peer(t, 0, t->cfg.client_cas, ALT_X509_EKU_CLIENT_AUTH)) < 0)
			goto out;
		if (t->npeer && (rc = read_cert_verify(t, false)) < 0)
			goto out;
	}
	if ((rc = read_finished(t, c_hs)) < 0 || (rc = tls_keys_boundary(t)) < 0)
		goto out;
	tls_set_keys13(&t->in, t->suite, c_ap);
	if (tickets && want_cert) {
		uint8_t res[48];
		derive_secret(t, t->master, "res master", true, res);
		rc = send_ticket(t, res);
		memset(res, 0, sizeof res);
		if (rc < 0)
			goto out;
	}
	t->ccs_ok = false;
	memset(c_hs, 0, sizeof c_hs);
	memset(s_hs, 0, sizeof s_hs);
	rc = 0;
out:
	alt_buf_free(&in);
	alt_buf_free(&m);
	alt_buf_free(&ch2_raw);
	alt_arena_free(&scratch);
	return rc;
}

/* ---- after the handshake ---- */

/* the client's NewSessionTicket: a session for the cache */
static int store_ticket(alt_tls_conn *t, const alt_buf *msg) {
	tr b = {(const uint8_t *)msg->p + 4, msg->n - 4}, nonce, ticket, ext;
	uint32_t lifetime, age_add;
	if (!tr_u32(&b, &lifetime) || !tr_u32(&b, &age_add) || !tr_vec(&b, 1, &nonce) || !tr_vec(&b, 2, &ticket) ||
		!tr_vec(&b, 2, &ext) || b.n || ticket.n == 0) {
		tls_send_alert(t, 2, A_DECODE_ERROR);
		return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: received malformed NewSessionTicket");
	}
	if (lifetime > TICKET_LIFETIME) {
		tls_send_alert(t, 2, A_ILLEGAL_PARAMETER);
		return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: received a session ticket with invalid lifetime");
	}
	if (!t->cfg.session_cache || lifetime == 0)
		return 0;
	alt_hash_kind h = suite_hash(t->suite);
	size_t hl = alt_hash_size(h);
	tls_session *s = calloc(1, sizeof *s);
	if (!s)
		return 0;
	s->version = VERSION_13;
	s->suite = t->suite;
	s->group = t->group;
	s->nsecret = hl;
	expand_label(h, t->resumption, "resumption", nonce.p, nonce.n, s->secret, hl);
	s->created = alt_wall_unix(alt_wall_now());
	s->use_by = s->created + lifetime;
	s->age_add = age_add;
	s->ticket = malloc(ticket.n);
	snprintf(s->alpn, sizeof s->alpn, "%s", t->alpn);
	if (!s->ticket || tls_session_set_certs(s, t) < 0) {
		tls_session_free(s);
		return 0;
	}
	memcpy(s->ticket, ticket.p, ticket.n);
	s->nticket = ticket.n;
	char key[300];
	tls_cache_key(t, key, sizeof key);
	tls_cache_put(t->cfg.session_cache, key, s);
	return 0;
}

int tls_post_handshake13(alt_tls_conn *t, int ty, alt_buf *m) {
	if (ty == HS_NEW_SESSION_TICKET && t->client)
		return store_ticket(t, m);
	if (ty == HS_KEY_UPDATE) {
		if (m->n != 5 || (uint8_t)m->p[4] > 1) {
			tls_send_alert(t, 2, A_DECODE_ERROR);
			return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: invalid key update");
		}
		bool requested = m->p[4] == 1;
		if (t->hs.n) {
			tls_send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: handshake message spans a key change");
		}
		tls_next_secret13(&t->in);
		if (requested) {
			alt_mutex_lock(&t->out_mu);
			uint8_t ku[5] = {HS_KEY_UPDATE, 0, 0, 1, 0};
			int rc = tls_write_record(t, REC_HANDSHAKE, ku, 5);
			if (rc == 0)
				tls_next_secret13(&t->out);
			alt_mutex_unlock(&t->out_mu);
			if (rc < 0)
				return rc;
		}
		return 0;
	}
	tls_send_alert(t, 2, A_UNEXPECTED_MESSAGE);
	return t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: received unexpected handshake message of type %d", ty);
}
