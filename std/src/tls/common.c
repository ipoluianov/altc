/* The parts of the TLS handshakes shared by both versions: key
   exchange, cipher suite and signature choice, certificates, the hello
   messages. */
#include "tls_internal.h"
#include <cpuid.h>

/* what this implementation signs and accepts (Go 1.27's list without ML-DSA and SHA-1) */
const uint16_t tls_sig_schemes[] = {S_RSA_PSS_256, S_ECDSA_P256, S_ED25519, S_RSA_PSS_384, S_RSA_PSS_512, S_RSA_PKCS1_SHA256,
	S_RSA_PKCS1_SHA384, S_RSA_PKCS1_SHA512, S_ECDSA_P384, S_ECDSA_P521};
const size_t tls_nsig_schemes = sizeof tls_sig_schemes / sizeof *tls_sig_schemes;

/* ---- key exchange ---- */

static alt_ec_curve curve_of_group(uint16_t g) {
	return g == ALT_TLS_P256 ? ALT_P256 : g == ALT_TLS_P384 ? ALT_P384 : g == ALT_TLS_P521 ? ALT_P521 : 0;
}

int tls_make_share(keyshare *k, uint16_t g) {
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

int tls_shared_secret(const keyshare *k, const uint8_t *peer, size_t n, uint8_t *out, size_t *nout) {
	if (k->group == ALT_TLS_X25519) {
		*nout = 32;
		return n == 32 ? alt_x25519(out, k->priv, peer) : -ALT_EKEY;
	}
	alt_ec_curve c = curve_of_group(k->group);
	*nout = alt_ec_scalar_size(c);
	return alt_ecdh(c, k->priv, peer, n, out);
}

bool tls_group_supported(uint16_t g) { return g == ALT_TLS_X25519 || g == ALT_TLS_P256 || g == ALT_TLS_P384 || g == ALT_TLS_P521; }

static const uint16_t default_groups[] = {ALT_TLS_X25519, ALT_TLS_P256, ALT_TLS_P384, ALT_TLS_P521};

size_t tls_groups_of(const alt_tls_config *cfg, uint16_t *out) {
	const uint16_t *g = cfg->groups && cfg->ngroups ? cfg->groups : default_groups;
	size_t ng = cfg->groups && cfg->ngroups ? cfg->ngroups : 4, n = 0;
	for (size_t i = 0; i < ng && n < 8; i++)
		if (tls_group_supported(g[i]))
			out[n++] = g[i];
	return n;
}

/* ---- suites ---- */

bool tls_aes_hardware(void) {
	unsigned a, b, c, d;
	return __get_cpuid(1, &a, &b, &c, &d) && (c & bit_AES) && (c & bit_PCLMUL);
}

static bool suite13(uint16_t s) { return s >= 0x1301 && s <= 0x1303; }

/* Go's preference orders of the TLS 1.2 suites (ECDHE only: RSA key exchange and 3DES are off by default) */
static const uint16_t order12_aes[] = {ALT_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, ALT_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
	ALT_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384, ALT_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
	ALT_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256, ALT_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
	ALT_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA, ALT_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA, ALT_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
	ALT_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA};
static const uint16_t order12_noaes[] = {ALT_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
	ALT_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256, ALT_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
	ALT_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, ALT_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
	ALT_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384, ALT_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA, ALT_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
	ALT_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA, ALT_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA};

bool tls_suite_is12(uint16_t s) {
	for (size_t i = 0; i < sizeof order12_aes / sizeof *order12_aes; i++)
		if (order12_aes[i] == s)
			return true;
	return false;
}

static size_t configured(const alt_tls_config *cfg, bool (*ok)(uint16_t), uint16_t *out) {
	size_t n = 0;
	for (size_t i = 0; i < cfg->nsuites && n < 16; i++)
		if (ok(cfg->cipher_suites[i]))
			out[n++] = cfg->cipher_suites[i];
	return n;
}

size_t tls_suites13(const alt_tls_config *cfg, bool aes, uint16_t *out) {
	if (cfg->cipher_suites && cfg->nsuites) {
		size_t n = configured(cfg, suite13, out);
		if (n)
			return n;
		/* only TLS 1.2 suites configured: TLS 1.3's are not configurable in Go */
	}
	if (aes) {
		out[0] = ALT_TLS_AES_128_GCM_SHA256, out[1] = ALT_TLS_AES_256_GCM_SHA384, out[2] = ALT_TLS_CHACHA20_POLY1305_SHA256;
	} else {
		out[0] = ALT_TLS_CHACHA20_POLY1305_SHA256, out[1] = ALT_TLS_AES_128_GCM_SHA256, out[2] = ALT_TLS_AES_256_GCM_SHA384;
	}
	return 3;
}

size_t tls_suites12(const alt_tls_config *cfg, bool aes, uint16_t *out) {
	if (cfg->cipher_suites && cfg->nsuites) {
		size_t n = configured(cfg, tls_suite_is12, out);
		if (n || configured(cfg, suite13, (uint16_t[16]){0}))
			return n;
	}
	const uint16_t *o = aes ? order12_aes : order12_noaes;
	for (size_t i = 0; i < 10; i++)
		out[i] = o[i];
	return 10;
}

static bool aes_gcm_suite(unsigned s) {
	return s == ALT_TLS_AES_128_GCM_SHA256 || s == ALT_TLS_AES_256_GCM_SHA384 || s == ALT_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 ||
	       s == ALT_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 || s == ALT_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 ||
	       s == ALT_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384;
}

bool tls_aes_preferred(const tr *suites) {
	if (!tls_aes_hardware())
		return false;
	for (size_t k = 0; k + 1 < suites->n; k += 2) {
		unsigned s = (unsigned)suites->p[k] << 8 | suites->p[k + 1];
		if (suite13((uint16_t)s) || tls_suite_is12((uint16_t)s))
			return aes_gcm_suite(s);
	}
	return false;
}

int tls_version_range(const alt_tls_config *cfg, uint16_t *min, uint16_t *max) {
	uint16_t lo = cfg->min_version ? cfg->min_version : ALT_TLS_VERSION_12;
	uint16_t hi = cfg->max_version ? cfg->max_version : ALT_TLS_VERSION_13;
	if (lo < ALT_TLS_VERSION_12)
		lo = ALT_TLS_VERSION_12; /* TLS 1.0 and 1.1 are not implemented */
	if (hi > ALT_TLS_VERSION_13)
		hi = ALT_TLS_VERSION_13;
	if (lo > hi)
		return -1;
	*min = lo, *max = hi;
	return 0;
}

/* ---- signatures ---- */

alt_hash_kind tls_scheme_hash(uint16_t s) {
	switch (s) {
	case S_ECDSA_P384: case S_RSA_PSS_384: case S_RSA_PSS_PSS_384: case S_RSA_PKCS1_SHA384: return ALT_SHA384;
	case S_ECDSA_P521: case S_RSA_PSS_512: case S_RSA_PSS_PSS_512: case S_RSA_PKCS1_SHA512: return ALT_SHA512;
	case S_RSA_PKCS1_SHA1: case S_ECDSA_SHA1: return ALT_SHA1;
	default: return ALT_SHA256;
	}
}

/* the schemes our key can make (Go's signatureSchemesForPublicKey without the disabled ones) */
static size_t key_schemes(const alt_x509_private_key *k, uint16_t version, uint16_t *out) {
	size_t n = 0;
	switch (k->alg) {
	case ALT_X509_PK_ED25519: out[n++] = S_ED25519; break;
	case ALT_X509_PK_ECDSA:
		if (version < VERSION_13) {
			out[n++] = S_ECDSA_P256, out[n++] = S_ECDSA_P384, out[n++] = S_ECDSA_P521;
		} else
			out[n++] = k->curve == ALT_X509_P256 ? S_ECDSA_P256 : k->curve == ALT_X509_P384 ? S_ECDSA_P384 : S_ECDSA_P521;
		break;
	case ALT_X509_PK_RSA: {
		size_t bytes = k->rsa.n.n;
		while (bytes && k->rsa.n.p[k->rsa.n.n - bytes] == 0)
			bytes--;
		static const struct {
			uint16_t s;
			size_t min;
			bool v12only;
		} rsa[] = {{S_RSA_PSS_256, 32 * 2 + 2, false}, {S_RSA_PSS_384, 48 * 2 + 2, false}, {S_RSA_PSS_512, 64 * 2 + 2, false},
			{S_RSA_PKCS1_SHA256, 19 + 32 + 11, true}, {S_RSA_PKCS1_SHA384, 19 + 48 + 11, true},
			{S_RSA_PKCS1_SHA512, 19 + 64 + 11, true}};
		for (size_t i = 0; i < sizeof rsa / sizeof *rsa; i++)
			if (bytes >= rsa[i].min && (!rsa[i].v12only || version < VERSION_13))
				out[n++] = rsa[i].s;
		break;
	}
	default: break;
	}
	return n;
}

uint16_t tls_pick_scheme(const alt_x509_private_key *k, uint16_t version, const uint16_t *peer, size_t np) {
	uint16_t mine[8];
	size_t nm = key_schemes(k, version, mine);
	for (size_t i = 0; i < np; i++)
		for (size_t j = 0; j < nm; j++)
			if (peer[i] == mine[j])
				return mine[j];
	return 0;
}

int tls_sign(const alt_x509_private_key *k, uint16_t scheme, const uint8_t *content, size_t n, uint8_t *sig, size_t *nsig) {
	alt_hash_kind h = tls_scheme_hash(scheme);
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
	if (scheme == S_RSA_PKCS1_SHA256 || scheme == S_RSA_PKCS1_SHA384 || scheme == S_RSA_PKCS1_SHA512)
		return alt_rsa_sign_pkcs1v15(&rk, h, d, alt_hash_size(h), sig);
	return alt_rsa_sign_pss(&rk, h, d, alt_hash_size(h), sig, -1);
}

int tls_verify_sig(const alt_x509_cert *c, uint16_t scheme, bool v12, const uint8_t *content, size_t n, const uint8_t *sig,
	size_t ns) {
	alt_hash_kind h = tls_scheme_hash(scheme);
	uint8_t d[ALT_HASH_MAX_SIZE];
	alt_hash_data(h, content, n, d);
	switch (scheme) {
	case S_ED25519:
		return c->pk_alg == ALT_X509_PK_ED25519 && ns == 64 ? alt_ed25519_verify(c->pub.point.p, content, n, sig) : -ALT_EVERIFY;
	case S_ECDSA_P256: case S_ECDSA_P384: case S_ECDSA_P521: {
		/* TLS 1.3 binds the curve to the scheme; TLS 1.2 only the hash */
		int want = scheme == S_ECDSA_P256 ? ALT_X509_P256 : scheme == S_ECDSA_P384 ? ALT_X509_P384 : ALT_X509_P521;
		if (c->pk_alg != ALT_X509_PK_ECDSA || (!v12 && c->pub.curve != want))
			return -ALT_EVERIFY;
		return alt_ecdsa_verify((alt_ec_curve)c->pub.curve, c->pub.point.p, c->pub.point.n, d, alt_hash_size(h), sig, ns);
	}
	case S_RSA_PSS_256: case S_RSA_PSS_384: case S_RSA_PSS_512: {
		if (c->pk_alg != ALT_X509_PK_RSA)
			return -ALT_EVERIFY;
		alt_rsa_public pub = {c->pub.n.p, c->pub.n.n, c->pub.e};
		return alt_rsa_verify_pss(&pub, h, d, alt_hash_size(h), sig, ns, (int)alt_hash_size(h));
	}
	case S_RSA_PKCS1_SHA256: case S_RSA_PKCS1_SHA384: case S_RSA_PKCS1_SHA512: {
		if (!v12 || c->pk_alg != ALT_X509_PK_RSA)
			return -ALT_EVERIFY; /* not for TLS 1.3 handshakes */
		alt_rsa_public pub = {c->pub.n.p, c->pub.n.n, c->pub.e};
		return alt_rsa_verify_pkcs1v15(&pub, h, d, alt_hash_size(h), sig, ns);
	}
	default: return -ALT_EVERIFY; /* SHA-1 is disabled, as in Go 1.25+ */
	}
}

/* ---- certificates ---- */

int tls_send_certificate(alt_tls_conn *t, const alt_tls_cert *c, bool v13, const tr *context) {
	alt_buf m = ALT_BUF_INIT;
	tb b = {.b = &m};
	tb_u8(&b, HS_CERTIFICATE);
	tb_begin(&b, 3);
	if (v13) {
		tb_begin(&b, 1);
		if (context)
			tb_bytes(&b, context->p, context->n);
		tb_end(&b);
	}
	tb_begin(&b, 3);
	for (size_t i = 0; c && i < c->nchain; i++) {
		tb_begin(&b, 3);
		tb_bytes(&b, c->chain[i].p, c->chain[i].n);
		tb_end(&b);
		if (v13)
			tb_u16(&b, 0); /* no extensions */
	}
	tb_end(&b);
	tb_end(&b);
	int rc = m.err ? m.err : tls_send_handshake(t, &m);
	alt_buf_free(&m);
	return rc;
}

int tls_read_certificate(alt_tls_conn *t, tr body, bool v13, bool allow_empty) {
	tr ctx, list;
	if ((v13 && !tr_vec(&body, 1, &ctx)) || !tr_vec(&body, 3, &list) || body.n)
		return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate message");
	t->peer = 0;
	t->npeer = 0;
	while (list.n) {
		tr der, ext;
		if (!tr_vec(&list, 3, &der) || (v13 && !tr_vec(&list, 2, &ext)) || der.n == 0)
			return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse certificate message");
		uint8_t *copy = alt_arena_alloc(&t->arena, der.n);
		const alt_x509_cert **np = alt_arena_realloc(&t->arena, t->peer, t->npeer * sizeof *np, (t->npeer + 1) * sizeof *np);
		if (!copy || !np)
			return tls_fatal(t, A_INTERNAL_ERROR, -ENOMEM, "out of memory");
		memcpy(copy, der.p, der.n);
		alt_x509_cert *c;
		const char *why = "";
		if (alt_x509_parse(&t->arena, copy, der.n, &c, &why) < 0) {
			tls_fatal(t, A_BAD_CERTIFICATE, -ALT_ETLS_CERT, "");
			snprintf(t->err, sizeof t->err, t->client ? "tls: failed to parse certificate from server: %s"
								  : "tls: failed to parse client certificate: %s", why);
			return -ALT_ETLS_CERT;
		}
		t->peer = np;
		t->peer[t->npeer++] = c;
	}
	if (t->npeer == 0 && !allow_empty)
		return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: received empty certificates message");
	return 0;
}

int tls_verify_peer(alt_tls_conn *t, const char *name, const alt_x509_pool *roots, int eku) {
	alt_x509_pool *inter = alt_x509_pool_new();
	if (!inter)
		return -ENOMEM;
	for (size_t i = 1; i < t->npeer; i++)
		alt_x509_pool_add_der(inter, t->peer[i]->raw.p, t->peer[i]->raw.n);
	alt_x509_verify_opts o = {.dns_name = name, .roots = roots, .intermediates = inter, .key_usages = &eku, .nkey_usages = 1};
	const char *why = "";
	int rc = alt_x509_verify(t->peer[0], &o, &t->arena, &t->chains, &t->nchains, &why);
	alt_x509_pool_free(inter);
	if (rc < 0) {
		int alert = strstr(why, "unknown authority") ? A_UNKNOWN_CA : strstr(why, "expired") ? A_CERTIFICATE_EXPIRED : A_BAD_CERTIFICATE;
		tls_fatal(t, alert, -ALT_ETLS_CERT, "");
		snprintf(t->err, sizeof t->err, "tls: failed to verify certificate: %s", why);
		return -ALT_ETLS_CERT;
	}
	return 0;
}

/* ---- ClientHello ---- */

static bool is_ip(const char *s) {
	alt_ip ip;
	return alt_ip_parse(alt_str_cstr(s), &ip) == 0;
}

int tls_build_client_hello(alt_tls_conn *t, alt_buf *m, const hello_offer *o, const tr *cookie) {
	uint16_t s12[16], s13[16], groups[8];
	bool aes = tls_aes_hardware(), v12 = o->min_version <= VERSION_12, v13 = o->max_version >= VERSION_13;
	size_t n12 = v12 ? tls_suites12(&t->cfg, aes, s12) : 0, n13 = v13 ? tls_suites13(&t->cfg, aes, s13) : 0;
	size_t ng = tls_groups_of(&t->cfg, groups);
	tb b = {.b = m};
	tb_u8(&b, HS_CLIENT_HELLO);
	tb_begin(&b, 3);
	tb_u16(&b, VERSION_12);
	tb_bytes(&b, o->random, 32);
	tb_begin(&b, 1);
	tb_bytes(&b, o->sid, o->nsid);
	tb_end(&b);
	tb_begin(&b, 2);
	for (size_t i = 0; i < n12; i++)
		tb_u16(&b, s12[i]);
	for (size_t i = 0; i < n13; i++)
		tb_u16(&b, s13[i]);
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
	if (v12) {
		tb_u16(&b, EXT_POINT_FORMATS);
		tb_u16(&b, 2);
		tb_u8(&b, 1);
		tb_u8(&b, 0); /* uncompressed */
		if (t->cfg.session_cache) {
			const tls_session *s = o->session && o->session->version == VERSION_12 ? o->session : 0;
			tb_u16(&b, EXT_SESSION_TICKET);
			tb_begin(&b, 2);
			if (s)
				tb_bytes(&b, s->ticket, s->nticket);
			tb_end(&b);
		}
	}
	tb_u16(&b, EXT_SIGNATURE_ALGORITHMS);
	tb_begin(&b, 2);
	tb_begin(&b, 2);
	for (size_t i = 0; i < tls_nsig_schemes; i++)
		tb_u16(&b, tls_sig_schemes[i]);
	tb_end(&b);
	tb_end(&b);
	if (v12) {
		tb_u16(&b, EXT_RENEGOTIATION_INFO);
		tb_u16(&b, 1);
		tb_u8(&b, 0);
	}
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
	if (v12) {
		tb_u16(&b, EXT_EXTENDED_MASTER_SECRET);
		tb_u16(&b, 0);
	}
	if (v13) {
		tb_u16(&b, EXT_SUPPORTED_VERSIONS);
		tb_begin(&b, 2);
		tb_begin(&b, 1);
		tb_u16(&b, VERSION_13);
		if (v12)
			tb_u16(&b, VERSION_12);
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
		tb_u16(&b, o->share.group);
		tb_begin(&b, 2);
		tb_bytes(&b, o->share.pub, o->share.npub);
		tb_end(&b);
		tb_end(&b);
		tb_end(&b);
		const tls_session *s = o->session && o->session->version == VERSION_13 ? o->session : 0;
		if (s) {
			/* pre_shared_key, the last extension: the binder is filled in by the caller */
			size_t hl = alt_hash_size(o->psk_suite == ALT_TLS_AES_256_GCM_SHA384 ? ALT_SHA384 : ALT_SHA256);
			int64_t now = alt_wall_unix(alt_wall_now());
			uint32_t age = (uint32_t)((now - s->created) * 1000) + s->age_add;
			tb_u16(&b, EXT_PRE_SHARED_KEY);
			tb_begin(&b, 2);
			tb_begin(&b, 2);
			tb_begin(&b, 2);
			tb_bytes(&b, s->ticket, s->nticket);
			tb_end(&b);
			tb_u32(&b, age);
			tb_end(&b);
			tb_begin(&b, 2);
			tb_begin(&b, 1);
			for (size_t i = 0; i < hl; i++)
				tb_u8(&b, 0);
			tb_end(&b);
			tb_end(&b);
			tb_end(&b);
		}
	}
	tb_end(&b);
	tb_end(&b);
	return m->err;
}

int tls_parse_server_hello(alt_tls_conn *t, tr body, server_hello *sh) {
	unsigned comp;
	tr rnd, exts = {0, 0};
	memset(sh, 0, sizeof *sh);
	if (!tr_u16(&body, &sh->legacy_version) || !tr_take(&body, 32, &rnd) || !tr_vec(&body, 1, &sh->sid) ||
		!tr_u16(&body, &sh->suite) || !tr_u8(&body, &comp) || (body.n && (!tr_vec(&body, 2, &exts) || body.n)))
		return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
	memcpy(sh->random, rnd.p, 32);
	if (comp != 0)
		return tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server selected unsupported compression format");
	while (exts.n) {
		unsigned type;
		tr data;
		if (!tr_u16(&exts, &type) || !tr_vec(&exts, 2, &data))
			return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
		bool ok = true;
		tr list;
		switch (type) {
		case EXT_SUPPORTED_VERSIONS:
			ok = tr_u16(&data, &sh->version) && !data.n;
			sh->has_version = true;
			break;
		case EXT_KEY_SHARE:
			ok = tr_u16(&data, &sh->group) && (!data.n || (tr_vec(&data, 2, &sh->share) && !data.n));
			sh->has_share = sh->share.n > 0;
			break;
		case EXT_COOKIE:
			ok = tr_vec(&data, 2, &sh->cookie) && !data.n && sh->cookie.n;
			sh->has_cookie = true;
			break;
		case EXT_ALPN:
			ok = tr_vec(&data, 2, &list) && tr_vec(&list, 1, &sh->alpn) && !list.n && !data.n && sh->alpn.n;
			sh->has_alpn = true;
			break;
		case EXT_RENEGOTIATION_INFO:
			ok = tr_vec(&data, 1, &sh->reneg) && !data.n;
			sh->has_reneg = true;
			break;
		case EXT_EXTENDED_MASTER_SECRET:
			ok = !data.n;
			sh->ems = true;
			break;
		case EXT_SESSION_TICKET:
			ok = !data.n;
			sh->ticket = true;
			break;
		case EXT_PRE_SHARED_KEY:
			ok = tr_u16(&data, &sh->psk_index) && !data.n;
			sh->has_psk = true;
			break;
		case EXT_POINT_FORMATS: {
			tr pf;
			ok = tr_vec(&data, 1, &pf) && pf.n && !data.n;
			bool unc = false;
			for (size_t i = 0; ok && i < pf.n; i++)
				unc |= pf.p[i] == 0;
			if (ok && !unc)
				return tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE, "tls: server offered only incompatible point formats");
			sh->has_point_formats = true;
			break;
		}
		default: break;
		}
		if (!ok)
			return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ServerHello");
	}
	return 0;
}

int tls_check_alpn(alt_tls_conn *t, const tr *proto) {
	if (t->cfg.nalpn == 0)
		return tls_fatal(t, A_UNSUPPORTED_EXTENSION, -ALT_ETLS_HANDSHAKE, "tls: server advertised unrequested ALPN extension");
	bool ok = false;
	for (size_t i = 0; i < t->cfg.nalpn; i++)
		ok |= strlen(t->cfg.alpn[i]) == proto->n && memcmp(t->cfg.alpn[i], proto->p, proto->n) == 0;
	if (!ok)
		return tls_fatal(t, A_UNSUPPORTED_EXTENSION, -ALT_ETLS_HANDSHAKE, "tls: server selected unadvertised ALPN protocol");
	memcpy(t->alpn, proto->p, proto->n);
	t->alpn[proto->n] = 0;
	return 0;
}

int tls_parse_client_hello(alt_tls_conn *t, tr body, client_hello *ch) {
	memset(ch, 0, sizeof *ch);
	if (!tr_u16(&body, &ch->legacy_version) || !tr_take(&body, 32, &ch->random) || !tr_vec(&body, 1, &ch->sid) ||
		ch->sid.n > 32 || !tr_vec(&body, 2, &ch->suites) || !tr_vec(&body, 1, &ch->comp))
		return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
	if (body.n && (!tr_vec(&body, 2, &ch->exts) || body.n))
		return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
	tr e = ch->exts;
	uint16_t seen[64];
	size_t nseen = 0;
	while (e.n) {
		unsigned type;
		tr d;
		if (!tr_u16(&e, &type) || !tr_vec(&e, 2, &d))
			return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
		for (size_t i = 0; i < nseen; i++)
			if (seen[i] == type)
				return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: client sent duplicate extensions");
		if (nseen < 64)
			seen[nseen++] = (uint16_t)type;
		tr list, x;
		bool ok = true;
		switch (type) {
		case EXT_SERVER_NAME:
			ok = tr_vec(&d, 2, &list);
			while (ok && list.n) {
				unsigned nt;
				if (!tr_u8(&list, &nt) || !tr_vec(&list, 2, &x)) {
					ok = false;
					break;
				}
				if (nt == 0 && !ch->has_sni) {
					if (x.n == 0 || x.n >= sizeof ch->sni || memchr(x.p, 0, x.n) || x.p[x.n - 1] == '.') {
						ok = false;
						break;
					}
					memcpy(ch->sni, x.p, x.n);
					ch->has_sni = true;
				}
			}
			break;
		case EXT_SUPPORTED_GROUPS:
			ok = tr_vec(&d, 2, &list) && list.n % 2 == 0;
			ch->has_groups = true;
			while (ok && list.n) {
				unsigned g;
				tr_u16(&list, &g);
				if (ch->ngroups < 32)
					ch->groups[ch->ngroups++] = (uint16_t)g;
			}
			break;
		case EXT_POINT_FORMATS:
			ok = tr_vec(&d, 1, &list) && list.n;
			ch->has_point_formats = true;
			for (size_t i = 0; ok && i < list.n; i++)
				ch->uncompressed_points |= list.p[i] == 0;
			break;
		case EXT_SIGNATURE_ALGORITHMS:
			ok = tr_vec(&d, 2, &list) && list.n % 2 == 0 && list.n;
			while (ok && list.n) {
				unsigned s;
				tr_u16(&list, &s);
				if (ch->nschemes < 64)
					ch->schemes[ch->nschemes++] = (uint16_t)s;
			}
			break;
		case EXT_ALPN:
			ok = tr_vec(&d, 2, &list) && list.n;
			ch->has_alpn = true;
			while (ok && list.n) {
				if (!tr_vec(&list, 1, &x) || x.n == 0) {
					ok = false;
					break;
				}
				if (ch->nalpn < 16) {
					memcpy(ch->alpn[ch->nalpn], x.p, x.n);
					ch->alpn[ch->nalpn++][x.n] = 0;
				}
			}
			break;
		case EXT_SUPPORTED_VERSIONS:
			ok = tr_vec(&d, 1, &list) && list.n % 2 == 0 && list.n;
			ch->has_versions = true;
			while (ok && list.n) {
				unsigned ver;
				tr_u16(&list, &ver);
				if (ch->nversions < 16)
					ch->versions[ch->nversions++] = (uint16_t)ver;
			}
			break;
		case EXT_KEY_SHARE:
			ok = tr_vec(&d, 2, &list);
			while (ok && list.n) {
				unsigned g;
				if (!tr_u16(&list, &g) || !tr_vec(&list, 2, &x) || x.n == 0) {
					ok = false;
					break;
				}
				if (ch->nshares < 16)
					ch->shares[ch->nshares].group = (uint16_t)g, ch->shares[ch->nshares++].key = x;
			}
			break;
		case EXT_COOKIE:
			ok = tr_vec(&d, 2, &ch->cookie);
			ch->has_cookie = true;
			break;
		case EXT_PSK_MODES:
			ok = tr_vec(&d, 1, &list);
			ch->has_psk_modes = true;
			for (size_t i = 0; ok && i < list.n; i++)
				ch->psk_dhe_ke |= list.p[i] == 1;
			break;
		case EXT_EXTENDED_MASTER_SECRET:
			ok = d.n == 0;
			ch->ems = true;
			break;
		case EXT_RENEGOTIATION_INFO:
			ok = tr_vec(&d, 1, &ch->reneg) && !d.n;
			ch->reneg_info = true;
			break;
		case EXT_SESSION_TICKET:
			ch->ticket = d;
			ch->has_ticket_ext = true;
			d.n = 0;
			break;
		case EXT_EARLY_DATA:
			ch->early_data = true;
			break;
		case EXT_PRE_SHARED_KEY: {
			/* must be the last extension (RFC 8446 4.2.11) */
			if (e.n) {
				ok = false;
				break;
			}
			tr ids;
			ok = tr_vec(&d, 2, &ids) && tr_vec(&d, 2, &ch->binders) && !d.n && ids.n;
			while (ok && ids.n) {
				tr id;
				uint32_t age;
				if (!tr_vec(&ids, 2, &id) || !tr_u32(&ids, &age) || id.n == 0) {
					ok = false;
					break;
				}
				if (ch->npsk < 8)
					ch->psk[ch->npsk].identity = id, ch->psk[ch->npsk++].age = age;
			}
			ch->binders_off = 2 + ch->binders.n; /* the binders' bytes at the end of the message */
			break;
		}
		}
		if (!ok)
			return tls_fatal(t, A_DECODE_ERROR, -ALT_ETLS_HANDSHAKE, "tls: failed to parse ClientHello");
	}
	return 0;
}

const alt_tls_cert *tls_choose_cert(alt_tls_conn *t, const client_hello *ch, alt_arena *a) {
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

int tls_negotiate_alpn(alt_tls_conn *t, const client_hello *ch) {
	t->alpn[0] = 0;
	if (!t->cfg.nalpn || !ch->nalpn)
		return 0;
	bool h11_fallback = false;
	for (size_t i = 0; i < t->cfg.nalpn; i++)
		for (size_t k = 0; k < ch->nalpn; k++) {
			if (strcmp(t->cfg.alpn[i], ch->alpn[k]) == 0) {
				snprintf(t->alpn, sizeof t->alpn, "%s", ch->alpn[k]);
				return 0;
			}
			h11_fallback |= strcmp(t->cfg.alpn[i], "h2") == 0 && strcmp(ch->alpn[k], "http/1.1") == 0;
		}
	/* http/1.1 clients may connect to h2-only servers as if without ALPN (as Go) */
	if (h11_fallback)
		return 0;
	char m[256];
	size_t k = (size_t)snprintf(m, sizeof m, "tls: client requested unsupported application protocols ([");
	for (size_t i = 0; i < ch->nalpn && k < sizeof m - 8; i++)
		k += (size_t)snprintf(m + k, sizeof m - k, "%s\"%s\"", i ? " " : "", ch->alpn[i]);
	snprintf(m + (k < sizeof m ? k : sizeof m - 3), sizeof m - (k < sizeof m ? k : sizeof m - 3), "])");
	return tls_fatal(t, A_NO_APPLICATION_PROTOCOL, -ALT_ETLS_HANDSHAKE, m);
}
