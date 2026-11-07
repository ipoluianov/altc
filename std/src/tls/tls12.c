/* TLS 1.2 (RFC 5246) with ECDHE (RFC 8422), the extended master secret
   (RFC 7627), secure renegotiation indication (RFC 5746, no
   renegotiation), session tickets (RFC 5077) and downgrade protection
   (RFC 8446 4.1.3). Messages and checks follow Go's crypto/tls. */
#include "tls_internal.h"

typedef struct {
	uint16_t id;
	bool ecdsa;              /* ECDHE_ECDSA (else ECDHE_RSA) */
	int cipher;
	size_t key_len, iv_len, mac_len;
	alt_hash_kind prf;
} suite12;

static const suite12 suites[] = {
	{ALT_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, true, C_AES_GCM, 16, 4, 0, ALT_SHA256},
	{ALT_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, false, C_AES_GCM, 16, 4, 0, ALT_SHA256},
	{ALT_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384, true, C_AES_GCM, 32, 4, 0, ALT_SHA384},
	{ALT_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384, false, C_AES_GCM, 32, 4, 0, ALT_SHA384},
	{ALT_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256, true, C_CHACHA, 32, 12, 0, ALT_SHA256},
	{ALT_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256, false, C_CHACHA, 32, 12, 0, ALT_SHA256},
	{ALT_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA, true, C_AES_CBC_SHA, 16, 16, 20, ALT_SHA256},
	{ALT_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA, false, C_AES_CBC_SHA, 16, 16, 20, ALT_SHA256},
	{ALT_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA, true, C_AES_CBC_SHA, 32, 16, 20, ALT_SHA256},
	{ALT_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA, false, C_AES_CBC_SHA, 32, 16, 20, ALT_SHA256},
};

static const suite12 *suite_of(uint16_t id) {
	for (size_t i = 0; i < sizeof suites / sizeof *suites; i++)
		if (suites[i].id == id)
			return &suites[i];
	return 0;
}

static const char downgrade12[8] = {'D', 'O', 'W', 'N', 'G', 'R', 'D', 1};

/* ---- the PRF (RFC 5246 5) ---- */

static void prf(alt_hash_kind h, const uint8_t *secret, size_t ns, const char *label, const uint8_t *s1, size_t n1,
	const uint8_t *s2, size_t n2, uint8_t *out, size_t n) {
	size_t hl = alt_hash_size(h), ll = strlen(label);
	uint8_t a[ALT_HASH_MAX_SIZE], blk[ALT_HASH_MAX_SIZE];
	alt_hmac m;
	/* A(1) = HMAC(secret, seed) */
	alt_hmac_init(&m, h, secret, ns);
	alt_hmac_write(&m, label, ll);
	alt_hmac_write(&m, s1, n1);
	alt_hmac_write(&m, s2, n2);
	alt_hmac_sum(&m, a);
	while (n) {
		alt_hmac_init(&m, h, secret, ns);
		alt_hmac_write(&m, a, hl);
		alt_hmac_write(&m, label, ll);
		alt_hmac_write(&m, s1, n1);
		alt_hmac_write(&m, s2, n2);
		alt_hmac_sum(&m, blk);
		size_t k = n < hl ? n : hl;
		memcpy(out, blk, k);
		out += k, n -= k;
		alt_hmac_init(&m, h, secret, ns);
		alt_hmac_write(&m, a, hl);
		alt_hmac_sum(&m, a);
	}
	memset(&m, 0, sizeof m);
	memset(blk, 0, sizeof blk);
}

static void master_secret(alt_tls_conn *t, const suite12 *s, const uint8_t *pms, size_t npms) {
	if (t->ems) {
		uint8_t th[ALT_HASH_MAX_SIZE];
		alt_hash_data(s->prf, t->transcript.p, t->transcript.n, th);
		prf(s->prf, pms, npms, "extended master secret", th, alt_hash_size(s->prf), 0, 0, t->master, 48);
	} else
		prf(s->prf, pms, npms, "master secret", t->client_random, 32, t->server_random, 32, t->master, 48);
}

static void init_half(half *x, const suite12 *s, const uint8_t *key, const uint8_t *iv, const uint8_t *mac) {
	memset(x, 0, sizeof *x);
	x->on = 1;
	x->v12 = true;
	x->suite = s->id;
	x->cipher = s->cipher;
	x->key_len = s->key_len;
	memcpy(x->key, key, s->key_len);
	memcpy(x->iv, iv, s->iv_len < 12 ? s->iv_len : 12);
	if (s->mac_len)
		memcpy(x->mac, mac, s->mac_len);
	if (s->cipher == C_AES_GCM)
		alt_gcm_init(&x->gcm, key, s->key_len);
	else if (s->cipher == C_AES_CBC_SHA)
		alt_aes_init(&x->aes, key, s->key_len);
}

/* the key block: the pending keys of both directions (the reading one waits for the peer's ChangeCipherSpec) */
static void establish_keys(alt_tls_conn *t, const suite12 *s, half *out_next) {
	uint8_t kb[2 * (20 + 32 + 16)];
	size_t n = 2 * (s->mac_len + s->key_len + s->iv_len);
	prf(s->prf, t->master, 48, "key expansion", t->server_random, 32, t->client_random, 32, kb, n);
	const uint8_t *cmac = kb, *smac = cmac + s->mac_len, *ckey = smac + s->mac_len, *skey = ckey + s->key_len,
		      *civ = skey + s->key_len, *siv = civ + s->iv_len;
	if (t->client) {
		init_half(out_next, s, ckey, civ, cmac);
		init_half(&t->pending_in, s, skey, siv, smac);
	} else {
		init_half(out_next, s, skey, siv, smac);
		init_half(&t->pending_in, s, ckey, civ, cmac);
	}
	t->has_pending_in = true;
	memset(kb, 0, sizeof kb);
}

/* sends ChangeCipherSpec and switches the writing keys */
static int change_out(alt_tls_conn *t, half *next) {
	int rc = tls_send_ccs(t);
	if (rc < 0)
		return rc;
	alt_mutex_lock(&t->out_mu);
	t->out = *next;
	alt_mutex_unlock(&t->out_mu);
	memset(next, 0, sizeof *next);
	return 0;
}

static void finished_data(alt_tls_conn *t, const suite12 *s, bool client, uint8_t out[12]) {
	uint8_t th[ALT_HASH_MAX_SIZE];
	alt_hash_data(s->prf, t->transcript.p, t->transcript.n, th);
	prf(s->prf, t->master, 48, client ? "client finished" : "server finished", th, alt_hash_size(s->prf), 0, 0, out, 12);
}

static int send_finished(alt_tls_conn *t, const suite12 *s) {
	uint8_t v[12];
	finished_data(t, s, t->client, v);
	alt_buf m = ALT_BUF_INIT;
	tb b = {.b = &m};
	tb_u8(&b, HS_FINISHED);
	tb_u24(&b, 12);
	tb_bytes(&b, v, 12);
	int rc = m.err ? m.err : tls_send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

/* the peer's Finished, which must come under the new keys (after its ChangeCipherSpec) */
static int read_finished(alt_tls_conn *t, const suite12 *s) {
	uint8_t v[12];
	finished_data(t, s, !t->client, v);
	alt_buf m = ALT_BUF_INIT;
	int ty, rc = tls_read_handshake(t, &ty, &m);
	if (rc == 0) {
		if (ty != HS_FINISHED || !t->in.on)
			rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE,
				ty != HS_FINISHED ? "tls: unexpected handshake message (wanted Finished)" : "tls: Finished before ChangeCipherSpec");
		else if (m.n != 16 || !alt_ct_eq(m.p + 4, v, 12))
			rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE,
				t->client ? "tls: server's Finished message was incorrect" : "tls: client's Finished message is incorrect");
		else
			alt_buf_append(&t->transcript, m.p, m.n);
	}
	alt_buf_free(&m);
	return rc;
}

/* ECDHE parameters: curve type 3 (named), the group, the point */
static void ecdhe_params(tb *b, const keyshare *k) {
	tb_u8(b, 3);
	tb_u16(b, k->group);
	tb_begin(b, 1);
	tb_bytes(b, k->pub, k->npub);
	tb_end(b);
}

/* signs or verifies client_random || server_random || params */
static size_t ske_content(alt_tls_conn *t, const uint8_t *params, size_t np, uint8_t *out) {
	memcpy(out, t->client_random, 32);
	memcpy(out + 32, t->server_random, 32);
	memcpy(out + 64, params, np);
	return 64 + np;
}

static bool scheme_is_rsa(uint16_t s) {
	return s == S_RSA_PKCS1_SHA256 || s == S_RSA_PKCS1_SHA384 || s == S_RSA_PKCS1_SHA512 || s == S_RSA_PSS_256 ||
	       s == S_RSA_PSS_384 || s == S_RSA_PSS_512;
}

static int new_session_ticket(alt_tls_conn *t, alt_buf *in, tr *ticket) {
	tr body;
	unsigned hint_hi, hint_lo;
	int rc = tls_expect(t, HS_NEW_SESSION_TICKET, in, &body);
	if (rc < 0)
		return rc;
	if (!tr_u16(&body, &hint_hi) || !tr_u16(&body, &hint_lo) || !tr_vec(&body, 2, ticket) || body.n)
		return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse NewSessionTicket");
	alt_buf_append(&t->transcript, in->p, in->n);
	return 0;
}

/* ---- the client ---- */

static void cache_session(alt_tls_conn *t, const tr *ticket) {
	if (!t->cfg.session_cache || !ticket->n)
		return;
	tls_session *s = calloc(1, sizeof *s);
	if (!s)
		return;
	s->version = VERSION_12;
	s->suite = t->suite;
	s->group = t->group;
	s->ems = t->ems;
	memcpy(s->secret, t->master, 48);
	s->nsecret = 48;
	s->created = alt_wall_unix(alt_wall_now());
	s->use_by = s->created + 7 * 24 * 3600;
	snprintf(s->alpn, sizeof s->alpn, "%s", t->alpn);
	s->ticket = malloc(ticket->n);
	if (!s->ticket || tls_session_set_certs(s, t) < 0) {
		tls_session_free(s);
		return;
	}
	memcpy(s->ticket, ticket->p, ticket->n);
	s->nticket = ticket->n;
	char key[300];
	tls_cache_key(t, key, sizeof key);
	tls_cache_put(t->cfg.session_cache, key, s);
}

int tls_client12(alt_tls_conn *t, hello_offer *o, server_hello *sh, alt_buf *in) {
	int rc;
	tr body;
	alt_buf m = ALT_BUF_INIT, ticket_copy = ALT_BUF_INIT;
	half out_next;
	memset(&out_next, 0, sizeof out_next);
	uint16_t s12[16];
	size_t n12 = tls_suites12(&t->cfg, tls_aes_hardware(), s12);
	const suite12 *s = 0;
	for (size_t i = 0; i < n12; i++)
		if (s12[i] == sh->suite)
			s = suite_of(s12[i]);
	if (!s) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: server chose an unconfigured cipher suite");
		goto out;
	}
	tls_session *sess = o->session && o->session->version == VERSION_12 ? o->session : 0;
	bool echoed = o->nsid && sh->sid.n == o->nsid && memcmp(sh->sid.p, o->sid, o->nsid) == 0;
	if (!sess && echoed) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server echoed TLS 1.3 compatibility session ID in TLS 1.2");
		goto out;
	}
	if (sh->has_reneg && sh->reneg.n) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: initial handshake had non-empty renegotiation extension");
		goto out;
	}
	t->secure_reneg = sh->has_reneg;
	if (sh->has_alpn && (rc = tls_check_alpn(t, &sh->alpn)) < 0)
		goto out;
	bool resumed = sess && echoed;
	if (resumed) {
		if (sess->suite != sh->suite) {
			rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: server resumed a session with a different cipher suite");
			goto out;
		}
		if (sess->ems != sh->ems) {
			rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: server resumed a session with a different EMS extension");
			goto out;
		}
	}
	t->suite = s->id;
	t->ems = sh->ems;
	memcpy(t->server_random, sh->random, 32);
	alt_buf_append(&t->transcript, in->p, in->n);
	tr ticket = {0, 0};
	if (resumed) {
		/* the abbreviated handshake: [NewSessionTicket], the server's ChangeCipherSpec and Finished first */
		t->did_resume = true;
		t->group = sess->group;
		memcpy(t->master, sess->secret, 48);
		if (!t->alpn[0] && sess->alpn[0])
			snprintf(t->alpn, sizeof t->alpn, "%s", sess->alpn);
		establish_keys(t, s, &out_next);
		if (sh->ticket && (rc = new_session_ticket(t, in, &ticket)) < 0)
			goto out;
		alt_buf_append(&ticket_copy, ticket.p, ticket.n);
		if ((rc = read_finished(t, s)) < 0 || (rc = change_out(t, &out_next)) < 0 || (rc = send_finished(t, s)) < 0)
			goto out;
		tr tc = {(const uint8_t *)ticket_copy.p, ticket_copy.n};
		cache_session(t, &tc);
		rc = 0;
		goto out;
	}
	/* a full handshake: the certificates restored from the session do not count */
	t->peer = 0, t->npeer = 0, t->chains = 0, t->nchains = 0;
	if ((rc = tls_expect(t, HS_CERTIFICATE, in, &body)) < 0 || (rc = tls_read_certificate(t, body, false, false)) < 0)
		goto out;
	alt_buf_append(&t->transcript, in->p, in->n);
	const alt_x509_cert *leaf = t->peer[0];
	if (!t->cfg.insecure_skip_verify && (rc = tls_verify_peer(t, t->cfg.server_name, t->cfg.roots, ALT_X509_EKU_SERVER_AUTH)) < 0)
		goto out;
	bool ec_key = leaf->pk_alg == ALT_X509_PK_ECDSA || leaf->pk_alg == ALT_X509_PK_ED25519;
	if (leaf->pk_alg != ALT_X509_PK_RSA && !ec_key) {
		rc = tls_fatal(t, A_UNSUPPORTED_CERTIFICATE, -ALT_ETLS_HANDSHAKE, "tls: server's certificate contains an unsupported type of public key");
		goto out;
	}
	/* ServerKeyExchange */
	int ty;
	if ((rc = tls_read_handshake(t, &ty, in)) < 0)
		goto out;
	if (ty == HS_CERTIFICATE_STATUS) {
		rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: received unexpected CertificateStatus message");
		goto out;
	}
	if (ty != HS_SERVER_KEY_EXCHANGE) {
		rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: missing ServerKeyExchange message");
		goto out;
	}
	tr skx = {(const uint8_t *)in->p + 4, in->n - 4}, point, sig;
	unsigned ctype, group, scheme;
	const uint8_t *params = skx.p;
	if (!tr_u8(&skx, &ctype) || !tr_u16(&skx, &group) || !tr_vec(&skx, 1, &point)) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid ServerKeyExchange message");
		goto out;
	}
	size_t nparams = (size_t)(skx.p - params);
	if (ctype != 3) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported curve");
		goto out;
	}
	if (!tr_u16(&skx, &scheme) || !tr_vec(&skx, 2, &sig) || skx.n) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid ServerKeyExchange message");
		goto out;
	}
	uint16_t groups[8];
	size_t ng = tls_groups_of(&t->cfg, groups);
	bool listed = false;
	for (size_t i = 0; i < ng; i++)
		listed |= groups[i] == group;
	if (!listed) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unoffered curve");
		goto out;
	}
	keyshare share;
	uint8_t pms[66];
	size_t npms;
	if ((rc = tls_make_share(&share, (uint16_t)group)) < 0)
		goto out;
	if (tls_shared_secret(&share, point.p, point.n, pms, &npms) < 0) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid ServerKeyExchange message");
		goto out;
	}
	bool offered = false;
	for (size_t i = 0; i < tls_nsig_schemes; i++)
		offered |= tls_sig_schemes[i] == scheme;
	if (!offered) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: certificate used with invalid signature algorithm");
		goto out;
	}
	if (scheme_is_rsa((uint16_t)scheme) == s->ecdsa) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid ServerKeyExchange message");
		goto out;
	}
	{
		uint8_t content[64 + 4 + 133];
		size_t nc = ske_content(t, params, nparams, content);
		if (tls_verify_sig(leaf, (uint16_t)scheme, true, content, nc, sig.p, sig.n) < 0) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid signature by the server certificate");
			goto out;
		}
	}
	t->group = (uint16_t)group;
	alt_buf_append(&t->transcript, in->p, in->n);
	/* CertificateRequest (optional), ServerHelloDone */
	if ((rc = tls_read_handshake(t, &ty, in)) < 0)
		goto out;
	bool cert_requested = false;
	uint16_t peer_schemes[64];
	size_t npeer = 0;
	if (ty == HS_CERTIFICATE_REQUEST) {
		tr b2 = {(const uint8_t *)in->p + 4, in->n - 4}, types, algs, cas;
		if (!tr_vec(&b2, 1, &types) || !tr_vec(&b2, 2, &algs) || algs.n % 2 || !tr_vec(&b2, 2, &cas) || b2.n) {
			rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse CertificateRequest");
			goto out;
		}
		while (algs.n && npeer < 64) {
			unsigned a;
			tr_u16(&algs, &a);
			peer_schemes[npeer++] = (uint16_t)a;
		}
		cert_requested = true;
		alt_buf_append(&t->transcript, in->p, in->n);
		if ((rc = tls_read_handshake(t, &ty, in)) < 0)
			goto out;
	}
	if (ty != HS_SERVER_HELLO_DONE || in->n != 4) {
		rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected handshake message (wanted ServerHelloDone)");
		goto out;
	}
	alt_buf_append(&t->transcript, in->p, in->n);
	/* our flight: [Certificate], ClientKeyExchange, [CertificateVerify], ChangeCipherSpec, Finished */
	const alt_tls_cert *mine = 0;
	uint16_t my_scheme = 0;
	if (cert_requested) {
		for (size_t i = 0; i < t->cfg.ncerts && !mine; i++)
			if ((my_scheme = tls_pick_scheme(&t->cfg.certs[i]->key, VERSION_12, peer_schemes, npeer)))
				mine = t->cfg.certs[i];
		if ((rc = tls_send_certificate(t, mine, false, 0)) < 0)
			goto out;
	}
	alt_buf_reset(&m);
	tb b = {.b = &m};
	tb_u8(&b, HS_CLIENT_KEY_EXCHANGE);
	tb_begin(&b, 3);
	tb_begin(&b, 1);
	tb_bytes(&b, share.pub, share.npub);
	tb_end(&b);
	tb_end(&b);
	memset(&share, 0, sizeof share);
	if ((rc = m.err ? m.err : tls_send_handshake(t, &m)) < 0)
		goto out;
	master_secret(t, s, pms, npms);
	memset(pms, 0, sizeof pms);
	if (mine) {
		uint8_t sg[ALT_RSA_MAX_BYTES];
		size_t nsg;
		if (tls_sign(&mine->key, my_scheme, (const uint8_t *)t->transcript.p, t->transcript.n, sg, &nsg) < 0) {
			rc = tls_fatal(t, A_INTERNAL_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to sign handshake");
			goto out;
		}
		alt_buf_reset(&m);
		tb_u8(&b, HS_CERTIFICATE_VERIFY);
		tb_begin(&b, 3);
		tb_u16(&b, my_scheme);
		tb_begin(&b, 2);
		tb_bytes(&b, sg, nsg);
		tb_end(&b);
		tb_end(&b);
		if ((rc = m.err ? m.err : tls_send_handshake(t, &m)) < 0)
			goto out;
	}
	establish_keys(t, s, &out_next);
	if ((rc = change_out(t, &out_next)) < 0 || (rc = send_finished(t, s)) < 0)
		goto out;
	if (sh->ticket) {
		if ((rc = new_session_ticket(t, in, &ticket)) < 0)
			goto out;
		alt_buf_append(&ticket_copy, ticket.p, ticket.n);
	}
	if ((rc = read_finished(t, s)) < 0)
		goto out;
	tr tc = {(const uint8_t *)ticket_copy.p, ticket_copy.n};
	cache_session(t, &tc);
	rc = 0;
out:
	memset(&out_next, 0, sizeof out_next);
	alt_buf_free(&m);
	alt_buf_free(&ticket_copy);
	return rc;
}

/* ---- the server ---- */

static int send_ticket12(alt_tls_conn *t) {
	tls_session s = {.version = VERSION_12, .suite = t->suite, .group = t->group, .ems = t->ems, .nsecret = 48, .created = alt_wall_unix(alt_wall_now())};
	memcpy(s.secret, t->master, 48);
	snprintf(s.alpn, sizeof s.alpn, "%s", t->alpn);
	alt_buf ticket = ALT_BUF_INIT, m = ALT_BUF_INIT;
	int rc = tls_session_set_certs(&s, t);
	if (rc == 0)
		rc = tls_seal_ticket(t, &s, &ticket);
	if (rc == 0) {
		tb b = {.b = &m};
		tb_u8(&b, HS_NEW_SESSION_TICKET);
		tb_begin(&b, 3);
		tb_u32(&b, 0); /* lifetime hint: unspecified (as Go) */
		tb_begin(&b, 2);
		tb_bytes(&b, ticket.p, ticket.n);
		tb_end(&b);
		tb_end(&b);
		rc = m.err ? m.err : tls_send_handshake(t, &m);
	}
	free(s.certs);
	memset(s.secret, 0, sizeof s.secret);
	alt_buf_free(&ticket);
	alt_buf_free(&m);
	return rc;
}

static bool offers(const client_hello *ch, uint16_t id) {
	for (size_t k = 0; k + 1 < ch->suites.n; k += 2)
		if (((ch->suites.p[k] << 8) | ch->suites.p[k + 1]) == id)
			return true;
	return false;
}

/* Go's checkForResumption for TLS 1.2: the session of the client's ticket, if it can be used */
static tls_session *server_session(alt_tls_conn *t, const client_hello *ch, bool ecdhe_ok, bool ec_sign, bool rsa_sign, int *rc) {
	*rc = 0;
	if (t->cfg.session_tickets_disabled || !ch->has_ticket_ext || !ch->ticket.n)
		return 0;
	tls_session *s = tls_open_ticket(t, ch->ticket.p, ch->ticket.n);
	if (!s)
		return 0;
	const suite12 *su = suite_of(s->suite);
	int ca = t->cfg.client_auth;
	bool needs_cert = ca == ALT_TLS_REQUIRE_ANY_CLIENT_CERT || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT;
	int64_t now = alt_wall_unix(alt_wall_now());
	if (s->version != VERSION_12 || !su || !offers(ch, s->suite) || !ecdhe_ok || !(su->ecdsa ? ec_sign : rsa_sign) ||
		now - s->created > 7 * 24 * 3600 || (needs_cert && !s->ncerts_bytes) || (!s->ems && ch->ems)) {
		tls_session_free(s);
		return 0;
	}
	if (s->ems && !ch->ems) {
		tls_session_free(s);
		*rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: session supported extended_master_secret but client does not");
		return 0;
	}
	return s;
}

int tls_server12(alt_tls_conn *t, client_hello *ch, alt_buf *raw) {
	(void)raw;
	int rc;
	alt_buf in = ALT_BUF_INIT, m = ALT_BUF_INIT;
	alt_arena scratch;
	alt_arena_init(&scratch, 0);
	half out_next;
	memset(&out_next, 0, sizeof out_next);
	tls_session *sess = 0;
	bool null_comp = false;
	for (size_t i = 0; i < ch->comp.n; i++)
		null_comp |= ch->comp.p[i] == 0;
	if (!null_comp) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: client does not support uncompressed connections");
		goto out;
	}
	if (ch->reneg_info && ch->reneg.n) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: initial handshake had non-empty renegotiation extension");
		goto out;
	}
	t->secure_reneg = ch->reneg_info || offers(ch, SCSV_RENEGOTIATION);
	t->ems = ch->ems;
	memcpy(t->client_random, ch->random.p, 32);
	if (ch->has_sni)
		snprintf(t->sni, sizeof t->sni, "%s", ch->sni);
	if ((rc = tls_negotiate_alpn(t, ch)) < 0)
		goto out;
	const alt_tls_cert *cert = tls_choose_cert(t, ch, &scratch);
	if (!cert) {
		rc = tls_fatal(t, A_UNRECOGNIZED_NAME, -ALT_ETLS_HANDSHAKE, "tls: no certificates configured");
		goto out;
	}
	/* Go's supportsECDHE */
	uint16_t groups[8];
	size_t ng = tls_groups_of(&t->cfg, groups);
	uint16_t group = 0;
	for (size_t i = 0; i < ng && !group; i++)
		for (size_t k = 0; k < ch->ngroups; k++)
			if (ch->groups[k] == groups[i]) {
				group = groups[i];
				break;
			}
	if (ch->has_point_formats && !ch->uncompressed_points) {
		rc = tls_fatal(t, A_MISSING_EXTENSION, -ALT_ETLS_HANDSHAKE, "tls: client offered only incompatible point formats");
		goto out;
	}
	bool ecdhe_ok = group != 0;
	int alg = cert->key.alg;
	bool ec_sign = alg == ALT_X509_PK_ECDSA || alg == ALT_X509_PK_ED25519, rsa_sign = alg == ALT_X509_PK_RSA;
	sess = server_session(t, ch, ecdhe_ok, ec_sign, rsa_sign, &rc);
	if (rc < 0)
		goto out;
	const suite12 *s = 0;
	if (sess)
		s = suite_of(sess->suite);
	else {
		uint16_t pref[16];
		size_t np = tls_suites12(&t->cfg, tls_aes_preferred(&ch->suites), pref);
		for (size_t i = 0; i < np && !s; i++) {
			const suite12 *c = suite_of(pref[i]);
			if (c && offers(ch, c->id) && ecdhe_ok && (c->ecdsa ? ec_sign : rsa_sign))
				s = c;
		}
	}
	if (!s) {
		char msg[256];
		size_t k = (size_t)snprintf(msg, sizeof msg, "tls: no cipher suite supported by both client and server; client offered: [");
		for (size_t i = 0; i + 1 < ch->suites.n && k < sizeof msg - 8; i += 2)
			k += (size_t)snprintf(msg + k, sizeof msg - k, "%s%x", i ? " " : "", (ch->suites.p[i] << 8) | ch->suites.p[i + 1]);
		if (k < sizeof msg - 2)
			snprintf(msg + k, sizeof msg - k, "]");
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, msg);
		goto out;
	}
	uint16_t vmin, vmax;
	tls_version_range(&t->cfg, &vmin, &vmax);
	if (offers(ch, SCSV_FALLBACK) && vmax > VERSION_12) {
		rc = tls_fatal(t, A_INAPPROPRIATE_FALLBACK, -ALT_ETLS_HANDSHAKE, "tls: client using inappropriate protocol fallback");
		goto out;
	}
	t->suite = s->id;
	/* the server random, with the downgrade canary when TLS 1.3 was possible */
	if ((rc = alt_crypto_rand(t->server_random, 32)) < 0)
		goto out;
	if (vmax > VERSION_12)
		memcpy(t->server_random + 24, downgrade12, 8);
	bool ticket = ch->has_ticket_ext && !t->cfg.session_tickets_disabled;
	tb b = {.b = &m};
	tb_u8(&b, HS_SERVER_HELLO);
	tb_begin(&b, 3);
	tb_u16(&b, VERSION_12);
	tb_bytes(&b, t->server_random, 32);
	tb_begin(&b, 1);
	if (sess)
		tb_bytes(&b, ch->sid.p, ch->sid.n); /* echoing the client's ID means resumption (RFC 5077 3.4) */
	tb_end(&b);
	tb_u16(&b, s->id);
	tb_u8(&b, 0);
	tb_begin(&b, 2);
	if (t->secure_reneg) {
		tb_u16(&b, EXT_RENEGOTIATION_INFO);
		tb_u16(&b, 1);
		tb_u8(&b, 0);
	}
	if (t->ems) {
		tb_u16(&b, EXT_EXTENDED_MASTER_SECRET);
		tb_u16(&b, 0);
	}
	if (ticket) {
		tb_u16(&b, EXT_SESSION_TICKET);
		tb_u16(&b, 0);
	}
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
	if (!sess && ch->has_point_formats) {
		tb_u16(&b, EXT_POINT_FORMATS);
		tb_u16(&b, 2);
		tb_u8(&b, 1);
		tb_u8(&b, 0);
	}
	tb_end(&b);
	tb_end(&b);
	if ((rc = m.err ? m.err : tls_send_handshake(t, &m)) < 0)
		goto out;
	if (sess) {
		/* the abbreviated handshake: [NewSessionTicket], ChangeCipherSpec, Finished; then the client's */
		t->did_resume = true;
		t->group = sess->group;
		memcpy(t->master, sess->secret, 48);
		if (sess->ncerts_bytes && (rc = tls_session_peer_certs(t, sess)) < 0)
			goto out;
		establish_keys(t, s, &out_next);
		if ((ticket && (rc = send_ticket12(t)) < 0) || (rc = change_out(t, &out_next)) < 0 || (rc = send_finished(t, s)) < 0 ||
			(rc = read_finished(t, s)) < 0)
			goto out;
		rc = 0;
		goto out;
	}
	if ((rc = tls_send_certificate(t, cert, false, 0)) < 0)
		goto out;
	/* ServerKeyExchange */
	uint16_t scheme = tls_pick_scheme(&cert->key, VERSION_12, ch->schemes, ch->nschemes);
	if (!scheme) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, ch->nschemes ? "tls: peer doesn't support any of the certificate's signature algorithms"
									    : "tls: missing signature_algorithms from TLS 1.2 peer");
		goto out;
	}
	if (scheme_is_rsa(scheme) != !s->ecdsa) {
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: certificate cannot be used with the selected cipher suite");
		goto out;
	}
	keyshare share;
	if ((rc = tls_make_share(&share, group)) < 0)
		goto out;
	t->group = group;
	alt_buf params = ALT_BUF_INIT;
	tb pb = {.b = &params};
	ecdhe_params(&pb, &share);
	uint8_t content[64 + 4 + 133], sg[ALT_RSA_MAX_BYTES];
	size_t nc = ske_content(t, (const uint8_t *)params.p, params.n, content), nsg;
	if (tls_sign(&cert->key, scheme, content, nc, sg, &nsg) < 0) {
		alt_buf_free(&params);
		rc = tls_fatal(t, A_HANDSHAKE_FAILURE, -ALT_ETLS_HANDSHAKE, "tls: failed to sign ECDHE parameters");
		goto out;
	}
	alt_buf_reset(&m);
	tb_u8(&b, HS_SERVER_KEY_EXCHANGE);
	tb_begin(&b, 3);
	tb_bytes(&b, params.p, params.n);
	tb_u16(&b, scheme);
	tb_begin(&b, 2);
	tb_bytes(&b, sg, nsg);
	tb_end(&b);
	tb_end(&b);
	alt_buf_free(&params);
	if ((rc = m.err ? m.err : tls_send_handshake(t, &m)) < 0)
		goto out;
	bool want_cert = t->cfg.client_auth != ALT_TLS_NO_CLIENT_CERT;
	if (want_cert) {
		alt_buf_reset(&m);
		tb_u8(&b, HS_CERTIFICATE_REQUEST);
		tb_begin(&b, 3);
		tb_begin(&b, 1);
		tb_u8(&b, 1);  /* rsa_sign */
		tb_u8(&b, 64); /* ecdsa_sign */
		tb_end(&b);
		tb_begin(&b, 2);
		for (size_t i = 0; i < tls_nsig_schemes; i++)
			tb_u16(&b, tls_sig_schemes[i]);
		tb_end(&b);
		tb_begin(&b, 2);
		for (size_t i = 0; t->cfg.client_cas && i < alt_x509_pool_len(t->cfg.client_cas); i++) {
			const alt_x509_cert *ca = alt_x509_pool_get(t->cfg.client_cas, i);
			tb_begin(&b, 2);
			tb_bytes(&b, ca->raw_subject.p, ca->raw_subject.n);
			tb_end(&b);
		}
		tb_end(&b);
		tb_end(&b);
		if ((rc = m.err ? m.err : tls_send_handshake(t, &m)) < 0)
			goto out;
	}
	alt_buf_reset(&m);
	tb_u8(&b, HS_SERVER_HELLO_DONE);
	tb_u24(&b, 0);
	if ((rc = tls_send_handshake(t, &m)) < 0)
		goto out;
	/* the client's flight */
	int ty;
	tr body;
	if ((rc = tls_read_handshake(t, &ty, &in)) < 0)
		goto out;
	if (want_cert) {
		if (ty != HS_CERTIFICATE) {
			rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected handshake message (wanted Certificate)");
			goto out;
		}
		if ((rc = tls_read_certificate(t, (tr){(const uint8_t *)in.p + 4, in.n - 4}, false, true)) < 0)
			goto out;
		alt_buf_append(&t->transcript, in.p, in.n);
		int ca = t->cfg.client_auth;
		if (t->npeer == 0 && (ca == ALT_TLS_REQUIRE_ANY_CLIENT_CERT || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT)) {
			rc = tls_fatal(t, A_BAD_CERTIFICATE, -ALT_ETLS_HANDSHAKE, "tls: client didn't provide a certificate");
			goto out;
		}
		if (t->npeer && (ca == ALT_TLS_VERIFY_CLIENT_CERT_IF_GIVEN || ca == ALT_TLS_REQUIRE_AND_VERIFY_CLIENT_CERT) &&
			(rc = tls_verify_peer(t, 0, t->cfg.client_cas, ALT_X509_EKU_CLIENT_AUTH)) < 0)
			goto out;
		if ((rc = tls_read_handshake(t, &ty, &in)) < 0)
			goto out;
	}
	if (ty != HS_CLIENT_KEY_EXCHANGE) {
		rc = tls_fatal(t, A_UNEXPECTED_MESSAGE, -ALT_ETLS_HANDSHAKE, "tls: unexpected handshake message (wanted ClientKeyExchange)");
		goto out;
	}
	body = (tr){(const uint8_t *)in.p + 4, in.n - 4};
	tr point;
	uint8_t pms[66];
	size_t npms;
	if (!tr_vec(&body, 1, &point) || body.n || point.n == 0 || tls_shared_secret(&share, point.p, point.n, pms, &npms) < 0) {
		memset(&share, 0, sizeof share);
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: invalid ClientKeyExchange message");
		goto out;
	}
	memset(&share, 0, sizeof share);
	alt_buf_append(&t->transcript, in.p, in.n);
	master_secret(t, s, pms, npms);
	memset(pms, 0, sizeof pms);
	if (t->npeer) {
		/* CertificateVerify signs the transcript before it */
		if ((rc = tls_expect(t, HS_CERTIFICATE_VERIFY, &in, &body)) < 0)
			goto out;
		unsigned sc;
		tr sig;
		if (!tr_u16(&body, &sc) || !tr_vec(&body, 2, &sig) || body.n) {
			rc = tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate verify");
			goto out;
		}
		bool offered = false;
		for (size_t i = 0; i < tls_nsig_schemes; i++)
			offered |= tls_sig_schemes[i] == sc;
		if (!offered) {
			rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: client certificate used with invalid signature algorithm");
			goto out;
		}
		if (tls_verify_sig(t->peer[0], (uint16_t)sc, true, (const uint8_t *)t->transcript.p, t->transcript.n, sig.p, sig.n) < 0) {
			rc = tls_fatal(t, A_DECRYPT_ERROR, -ALT_ETLS_HANDSHAKE, "tls: invalid signature by the client certificate");
			goto out;
		}
		alt_buf_append(&t->transcript, in.p, in.n);
	}
	establish_keys(t, s, &out_next);
	if ((rc = read_finished(t, s)) < 0)
		goto out;
	if ((ticket && (rc = send_ticket12(t)) < 0) || (rc = change_out(t, &out_next)) < 0 || (rc = send_finished(t, s)) < 0)
		goto out;
	rc = 0;
out:
	tls_session_free(sess);
	memset(&out_next, 0, sizeof out_next);
	alt_buf_free(&in);
	alt_buf_free(&m);
	alt_arena_free(&scratch);
	return rc;
}
