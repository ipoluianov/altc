/* TLS connections: the version negotiation, the connection API, key
   pairs. The record layer is in record.c, the handshakes in tls13.c and
   tls12.c. */
#include "tls_internal.h"
#include <alt/encoding/pem.h>

/* ---- the version and the session to offer ---- */

/* a cached session the client may offer: its version, suite and certificates still fit */
static tls_session *client_session(alt_tls_conn *t, hello_offer *o) {
	char key[300];
	tls_cache_key(t, key, sizeof key);
	tls_session *s = tls_cache_get(t->cfg.session_cache, key);
	if (!s)
		return 0;
	int64_t now = alt_wall_unix(alt_wall_now());
	bool ok = s->version >= o->min_version && s->version <= o->max_version && now < s->use_by && s->nticket;
	uint16_t list[16];
	size_t n = 0;
	if (ok)
		n = s->version == VERSION_13 ? tls_suites13(&t->cfg, tls_aes_hardware(), list) : tls_suites12(&t->cfg, tls_aes_hardware(), list);
	bool listed = false;
	for (size_t i = 0; i < n; i++)
		listed |= list[i] == s->suite;
	ok = ok && listed;
	/* the server's certificates of the session must still verify (Go checks the chains and the name) */
	if (ok && (ok = tls_session_peer_certs(t, s) == 0 && t->npeer > 0) && !t->cfg.insecure_skip_verify) {
		alt_x509_pool *inter = alt_x509_pool_new();
		int eku = ALT_X509_EKU_SERVER_AUTH;
		for (size_t i = 1; inter && i < t->npeer; i++)
			alt_x509_pool_add_der(inter, t->peer[i]->raw.p, t->peer[i]->raw.n);
		alt_x509_verify_opts vo = {.dns_name = t->cfg.server_name, .roots = t->cfg.roots, .intermediates = inter, .key_usages = &eku, .nkey_usages = 1};
		ok = inter && alt_x509_verify(t->peer[0], &vo, &t->arena, &t->chains, &t->nchains, 0) == 0;
		alt_x509_pool_free(inter);
	}
	if (!ok) {
		t->peer = 0, t->npeer = 0, t->chains = 0, t->nchains = 0;
		tls_session_free(s);
		return 0;
	}
	if (s->version == VERSION_13)
		o->psk_suite = s->suite;
	return s;
}

int tls_client_handshake(alt_tls_conn *t) {
	hello_offer o;
	memset(&o, 0, sizeof o);
	int rc;
	if (!t->cfg.insecure_skip_verify && (!t->cfg.server_name || !t->cfg.server_name[0]))
		return tls_fail(t, -EINVAL, "tls: either ServerName or InsecureSkipVerify must be specified in the tls.Config");
	size_t nproto = 0;
	for (size_t i = 0; i < t->cfg.nalpn; i++) {
		size_t l = strlen(t->cfg.alpn[i]);
		if (l == 0 || l > 255)
			return tls_fail(t, -EINVAL, "tls: invalid NextProtos value");
		nproto += 1 + l;
	}
	if (nproto > 0xffff)
		return tls_fail(t, -EINVAL, "tls: NextProtos values too large");
	if (tls_version_range(&t->cfg, &o.min_version, &o.max_version) < 0)
		return tls_fail(t, -EINVAL, "tls: no supported versions satisfy MinVersion and MaxVersion");
	if ((rc = alt_crypto_rand(o.random, 32)) < 0 || (rc = alt_crypto_rand(o.sid, 32)) < 0)
		return tls_fail(t, rc, "tls: no random bytes");
	o.nsid = 32;
	memcpy(t->client_random, o.random, 32);
	uint16_t groups[8];
	size_t ng = tls_groups_of(&t->cfg, groups);
	if (ng == 0)
		return tls_fail(t, -EINVAL, "tls: no supported key exchange methods (CurveIDs)");
	/* a key share for the first group (as Go, without hybrid groups) */
	if (o.max_version >= VERSION_13 && (rc = tls_make_share(&o.share, groups[0])) < 0)
		return tls_fail(t, rc, "tls: key generation failed");
	if (t->cfg.session_cache)
		o.session = client_session(t, &o);
	alt_buf m = ALT_BUF_INIT, in = ALT_BUF_INIT;
	if ((rc = tls_build_client_hello(t, &m, &o, 0)) < 0)
		goto out;
	tls_client_binder(t, &o, &m);
	if ((rc = tls_send_handshake(t, &m)) < 0)
		goto out;
	size_t ch1 = t->transcript.n;
	tr body;
	server_hello sh;
	t->ccs_ok = o.max_version >= VERSION_13;
	if ((rc = tls_expect(t, HS_SERVER_HELLO, &in, &body)) < 0 || (rc = tls_parse_server_hello(t, body, &sh)) < 0)
		goto out;
	unsigned v = sh.has_version ? sh.version : sh.legacy_version;
	if (v < o.min_version || v > o.max_version || (v == VERSION_13 && !sh.has_version) || (sh.has_version && v != VERSION_13)) {
		char msg[96];
		snprintf(msg, sizeof msg, "tls: server selected unsupported protocol version %x", v);
		rc = tls_fatal(t, A_PROTOCOL_VERSION, -ALT_ETLS_HANDSHAKE, msg);
		goto out;
	}
	/* downgrade canaries (RFC 8446 4.1.3) */
	if (o.max_version == VERSION_13 && v <= VERSION_12 &&
		(memcmp(sh.random + 24, "DOWNGRD\x01", 8) == 0 || memcmp(sh.random + 24, "DOWNGRD\x00", 8) == 0)) {
		rc = tls_fatal(t, A_ILLEGAL_PARAMETER, -ALT_ETLS_HANDSHAKE,
			"tls: downgrade attempt detected, possibly due to a MitM attack or a broken middlebox");
		goto out;
	}
	t->version = (uint16_t)v;
	if (v == VERSION_13)
		rc = tls_client13(t, &o, &sh, &in, ch1);
	else {
		t->ccs_ok = false;
		rc = tls_client12(t, &o, &sh, &in);
	}
out:
	if (rc < 0 && o.session && t->did_resume) {
		/* a session that failed to resume is not offered again */
		char key[300];
		tls_cache_key(t, key, sizeof key);
		tls_cache_remove(t->cfg.session_cache, key);
	}
	tls_session_free(o.session);
	memset(&o, 0, sizeof o);
	alt_buf_free(&m);
	alt_buf_free(&in);
	return rc;
}

int tls_server_handshake(alt_tls_conn *t) {
	alt_buf in = ALT_BUF_INIT, raw = ALT_BUF_INIT;
	tr body;
	client_hello ch;
	int rc;
	uint16_t vmin, vmax;
	if (tls_version_range(&t->cfg, &vmin, &vmax) < 0)
		return tls_fail(t, -EINVAL, "tls: no supported versions satisfy MinVersion and MaxVersion");
	if ((rc = tls_expect(t, HS_CLIENT_HELLO, &in, &body)) < 0)
		goto out;
	/* the hello stays in raw: ch points into it */
	alt_buf_append(&raw, in.p, in.n);
	if ((rc = tls_parse_client_hello(t, (tr){(const uint8_t *)raw.p + 4, raw.n - 4}, &ch)) < 0)
		goto out;
	alt_buf_append(&t->transcript, raw.p, raw.n);
	/* the version: the client's list, or its legacy version (Go's readClientHello) */
	uint16_t vers[16];
	size_t nv = 0;
	if (ch.has_versions)
		for (size_t i = 0; i < ch.nversions; i++)
			vers[nv++] = ch.versions[i];
	else if (ch.legacy_version >= VERSION_12)
		vers[nv++] = VERSION_12, vers[nv++] = 0x0302, vers[nv++] = 0x0301;
	else
		for (unsigned v = ch.legacy_version; v >= 0x0301 && nv < 3; v--)
			vers[nv++] = (uint16_t)v;
	uint16_t v = 0;
	for (unsigned want = vmax; want >= vmin && !v; want--)
		for (size_t i = 0; i < nv; i++)
			if (vers[i] == want) {
				v = (uint16_t)want;
				break;
			}
	if (!v) {
		char msg[160];
		size_t k = (size_t)snprintf(msg, sizeof msg, "tls: client offered only unsupported versions: [");
		for (size_t i = 0; i < nv && k < sizeof msg - 8; i++)
			k += (size_t)snprintf(msg + k, sizeof msg - k, "%s%x", i ? " " : "", vers[i]);
		if (k < sizeof msg - 2)
			snprintf(msg + k, sizeof msg - k, "]");
		rc = tls_fatal(t, A_PROTOCOL_VERSION, -ALT_ETLS_HANDSHAKE, msg);
		goto out;
	}
	t->version = v;
	rc = v == VERSION_13 ? tls_server13(t, &ch, &raw) : tls_server12(t, &ch, &raw);
out:
	alt_buf_free(&in);
	alt_buf_free(&raw);
	return rc;
}

/* ---- the connection ---- */

const char *alt_tls_error(const alt_tls_conn *t) { return t->err; }

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
	int rc = t->client ? tls_client_handshake(t) : tls_server_handshake(t);
	alt_mutex_unlock(&t->in_mu);
	if (t->cfg.handshake_timeout && t->conn)
		alt_conn_set_deadline(t->conn, ALT_FOREVER);
	if (rc < 0) {
		t->hs_err = rc;
		if (!t->err[0])
			tls_fail(t, rc, "tls: %s", alt_strerror(rc));
	} else
		t->done = true;
	alt_buf_free(&t->transcript);
	alt_mutex_unlock(&t->hs_mu);
	return rc;
}

/* a post-handshake message: TLS 1.3 tickets and key updates; TLS 1.2 renegotiation is refused (as Go's default) */
static int post_handshake(alt_tls_conn *t) {
	alt_buf m = ALT_BUF_INIT;
	int ty, rc = tls_read_handshake(t, &ty, &m);
	if (rc == 0) {
		if (t->version == VERSION_13)
			rc = tls_post_handshake13(t, ty, &m);
		else if (t->client && ty == HS_HELLO_REQUEST && m.n == 4) {
			tls_send_alert(t, 1, A_NO_RENEGOTIATION);
			rc = t->read_err = tls_fail(t, -ALT_ETLS_HANDSHAKE, "local error: tls: no renegotiation");
		} else {
			tls_send_alert(t, 2, A_UNEXPECTED_MESSAGE);
			rc = t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: received unexpected handshake message of type %d", ty);
		}
	}
	alt_buf_free(&m);
	return rc;
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
		if ((rc = tls_read_record(t, &ty, &d, &len)) < 0) {
			/* the end of the connection at a record boundary without close_notify is the end of the data (as Go) */
			if (t->raw_eof) {
				t->eof = true;
				rc = 0;
			}
			res = rc;
			break;
		}
		if (ty == REC_APP) {
			memcpy(t->app, d, len);
			t->app_off = 0;
			t->app_n = len;
		} else if (ty == REC_ALERT) {
			if ((rc = tls_on_alert(t, d, len)) < 0) {
				res = rc;
				break;
			}
		} else if (ty == REC_HANDSHAKE) {
			if (len == 0) {
				tls_send_alert(t, 2, A_UNEXPECTED_MESSAGE);
				res = t->read_err = tls_fail(t, -ALT_ETLS_RECORD, "tls: empty handshake record");
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
		rc = tls_write_record(t, REC_HANDSHAKE, ku, 5);
		if (rc == 0) {
			tls_next_secret13(&t->out);
			t->key_update_pending = false;
		}
	}
	if (rc == 0)
		rc = tls_write_records(t, REC_APP, p, n);
	alt_mutex_unlock(&t->out_mu);
	return rc < 0 ? rc : (ssize_t)n;
}

int alt_tls_key_update(alt_tls_conn *t) {
	int rc = alt_tls_handshake(t);
	if (rc < 0)
		return rc;
	if (t->version != VERSION_13)
		return -ENOTSUP;
	alt_mutex_lock(&t->out_mu);
	uint8_t ku[5] = {HS_KEY_UPDATE, 0, 0, 1, 1}; /* update_requested: the peer updates too */
	rc = tls_write_record(t, REC_HANDSHAKE, ku, 5);
	if (rc == 0)
		tls_next_secret13(&t->out);
	alt_mutex_unlock(&t->out_mu);
	return rc;
}

int alt_tls_close_write(alt_tls_conn *t) {
	alt_mutex_lock(&t->out_mu);
	int rc = 0;
	if (!t->closed_write) {
		t->closed_write = true;
		uint8_t a[2] = {1, A_CLOSE_NOTIFY};
		rc = tls_write_record(t, REC_ALERT, a, 2);
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
	if (cc.server_name == host) {
		/* the name taken from the address lives in the connection (tickets come after the handshake) */
		snprintf(t->sni, sizeof t->sni, "%s", host);
		t->cfg.server_name = t->sni;
	}
	alt_time dl = alt_ctx_deadline(ctx);
	if (dl)
		alt_conn_set_deadline(c, dl);
	rc = alt_tls_handshake(t);
	if (dl)
		alt_conn_set_deadline(c, ALT_FOREVER);
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
	s->version = t->done ? t->version : 0;
	s->did_resume = t->did_resume;
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
