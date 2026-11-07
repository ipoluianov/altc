/* Resumable sessions: the client's LRU cache (Go's
   NewLRUClientSessionCache) and the server's tickets. A ticket is the
   session encrypted with AES-256-GCM under a key derived from the
   configured ticket key (or a random one per process). */
#include "tls_internal.h"

void tls_session_free(tls_session *s) {
	if (!s)
		return;
	free(s->ticket);
	free(s->certs);
	memset(s, 0, sizeof *s);
	free(s);
}

static tls_session *session_copy(const tls_session *s) {
	tls_session *c = malloc(sizeof *c);
	if (!c)
		return 0;
	*c = *s;
	c->ticket = s->nticket ? malloc(s->nticket) : 0;
	c->certs = s->ncerts_bytes ? malloc(s->ncerts_bytes) : 0;
	if ((s->nticket && !c->ticket) || (s->ncerts_bytes && !c->certs)) {
		free(c->ticket);
		free(c->certs);
		free(c);
		return 0;
	}
	if (s->nticket)
		memcpy(c->ticket, s->ticket, s->nticket);
	if (s->ncerts_bytes)
		memcpy(c->certs, s->certs, s->ncerts_bytes);
	return c;
}

/* ---- the client's cache ---- */

typedef struct {
	char key[300];
	tls_session *s;
	uint64_t used;
} entry;

struct alt_tls_session_cache {
	alt_mutex mu;
	entry *e;
	size_t n, cap;
	uint64_t clock;
};

alt_tls_session_cache *alt_tls_session_cache_new(size_t capacity) {
	alt_tls_session_cache *c = calloc(1, sizeof *c);
	if (!c)
		return 0;
	c->cap = capacity ? capacity : 64;
	c->e = calloc(c->cap, sizeof *c->e);
	if (!c->e) {
		free(c);
		return 0;
	}
	c->mu = (alt_mutex)ALT_MUTEX_INIT;
	return c;
}

void alt_tls_session_cache_free(alt_tls_session_cache *c) {
	if (!c)
		return;
	for (size_t i = 0; i < c->n; i++)
		tls_session_free(c->e[i].s);
	free(c->e);
	free(c);
}

void tls_cache_key(const alt_tls_conn *t, char *key, size_t n) {
	/* Go's clientSessionCacheKey: the server name, else the address */
	if (t->cfg.server_name && t->cfg.server_name[0]) {
		snprintf(key, n, "%s", t->cfg.server_name);
		return;
	}
	key[0] = 0;
	if (t->conn) {
		alt_buf b = ALT_BUF_INIT;
		if (alt_addr_format(alt_conn_remote_addr(t->conn), &b) == 0 && !b.err)
			snprintf(key, n, "%.*s", (int)b.n, b.p);
		alt_buf_free(&b);
	}
}

static entry *find(alt_tls_session_cache *c, const char *key) {
	for (size_t i = 0; i < c->n; i++)
		if (strcmp(c->e[i].key, key) == 0)
			return &c->e[i];
	return 0;
}

tls_session *tls_cache_get(alt_tls_session_cache *c, const char *key) {
	if (!c || !key[0])
		return 0;
	alt_mutex_lock(&c->mu);
	entry *e = find(c, key);
	tls_session *s = 0;
	if (e) {
		e->used = ++c->clock;
		s = session_copy(e->s);
	}
	alt_mutex_unlock(&c->mu);
	return s;
}

void tls_cache_put(alt_tls_session_cache *c, const char *key, tls_session *s) {
	if (!c || !key[0]) {
		tls_session_free(s);
		return;
	}
	alt_mutex_lock(&c->mu);
	entry *e = find(c, key);
	if (!e) {
		if (c->n < c->cap)
			e = &c->e[c->n++];
		else {
			/* the least recently used goes */
			e = &c->e[0];
			for (size_t i = 1; i < c->n; i++)
				if (c->e[i].used < e->used)
					e = &c->e[i];
		}
		tls_session_free(e->s);
		e->s = 0;
		snprintf(e->key, sizeof e->key, "%s", key);
	} else
		tls_session_free(e->s);
	e->s = s;
	e->used = ++c->clock;
	alt_mutex_unlock(&c->mu);
}

void tls_cache_remove(alt_tls_session_cache *c, const char *key) {
	if (!c)
		return;
	alt_mutex_lock(&c->mu);
	entry *e = find(c, key);
	if (e) {
		tls_session_free(e->s);
		*e = c->e[--c->n];
	}
	alt_mutex_unlock(&c->mu);
}

/* ---- certificates of a session ---- */

int tls_session_set_certs(tls_session *s, const alt_tls_conn *t) {
	size_t n = 0;
	for (size_t i = 0; i < t->npeer; i++)
		n += 3 + t->peer[i]->raw.n;
	free(s->certs);
	s->certs = 0;
	s->ncerts_bytes = 0;
	if (!n)
		return 0;
	uint8_t *p = malloc(n);
	if (!p)
		return -ENOMEM;
	size_t k = 0;
	for (size_t i = 0; i < t->npeer; i++) {
		size_t l = t->peer[i]->raw.n;
		p[k] = (uint8_t)(l >> 16), p[k + 1] = (uint8_t)(l >> 8), p[k + 2] = (uint8_t)l;
		memcpy(p + k + 3, t->peer[i]->raw.p, l);
		k += 3 + l;
	}
	s->certs = p;
	s->ncerts_bytes = n;
	return 0;
}

int tls_session_peer_certs(alt_tls_conn *t, const tls_session *s) {
	tr r = {s->certs, s->ncerts_bytes};
	t->peer = 0;
	t->npeer = 0;
	t->chains = 0;
	t->nchains = 0;
	while (r.n) {
		tr der;
		if (!tr_vec(&r, 3, &der) || !der.n)
			return -ALT_ETLS_HANDSHAKE;
		uint8_t *copy = alt_arena_alloc(&t->arena, der.n);
		const alt_x509_cert **np = alt_arena_realloc(&t->arena, t->peer, t->npeer * sizeof *np, (t->npeer + 1) * sizeof *np);
		if (!copy || !np)
			return -ENOMEM;
		memcpy(copy, der.p, der.n);
		alt_x509_cert *c;
		if (alt_x509_parse(&t->arena, copy, der.n, &c, 0) < 0)
			return -ALT_ETLS_HANDSHAKE;
		t->peer = np;
		t->peer[t->npeer++] = c;
	}
	return 0;
}

/* ---- tickets ---- */

static uint8_t process_key[32];
static alt_once process_once = ALT_ONCE_INIT;

static void make_process_key(void *arg) {
	(void)arg;
	alt_crypto_rand(process_key, 32);
}

static void ticket_gcm(alt_tls_conn *t, alt_gcm *g) {
	const uint8_t *k = t->cfg.ticket_key;
	if (!k) {
		alt_once_do(&process_once, make_process_key, 0);
		k = process_key;
	}
	uint8_t key[32];
	alt_hmac_data(ALT_SHA256, k, 32, "altc tls ticket key", 19, key);
	alt_gcm_init(g, key, 32);
	memset(key, 0, sizeof key);
}

enum { TICKET_MAGIC = 0xa1 };

int tls_seal_ticket(alt_tls_conn *t, const tls_session *s, alt_buf *out) {
	alt_buf pt = ALT_BUF_INIT;
	tb b = {.b = &pt};
	tb_u16(&b, s->version);
	tb_u16(&b, s->suite);
	tb_u16(&b, s->group);
	tb_u8(&b, s->ems);
	tb_u32(&b, (uint32_t)((uint64_t)s->created >> 32));
	tb_u32(&b, (uint32_t)s->created);
	tb_begin(&b, 1);
	tb_bytes(&b, s->secret, s->nsecret);
	tb_end(&b);
	tb_begin(&b, 1);
	tb_bytes(&b, s->alpn, strlen(s->alpn));
	tb_end(&b);
	tb_begin(&b, 3);
	tb_bytes(&b, s->certs, s->ncerts_bytes);
	tb_end(&b);
	uint8_t nonce[12];
	int rc = pt.err ? pt.err : alt_crypto_rand(nonce, 12);
	if (rc == 0) {
		alt_gcm g;
		ticket_gcm(t, &g);
		size_t base = out->n;
		alt_buf_append_byte(out, TICKET_MAGIC);
		alt_buf_append(out, nonce, 12);
		if (alt_buf_reserve(out, pt.n + 16) < 0 || out->err)
			rc = out->err ? out->err : -ENOMEM;
		else {
			alt_gcm_seal(&g, out->p + out->n, nonce, 12, pt.p, pt.n, out->p + base, 13);
			alt_buf_truncate(out, out->n + pt.n + 16);
		}
		memset(&g, 0, sizeof g);
	}
	memset(pt.p, 0, pt.n);
	alt_buf_free(&pt);
	return rc;
}

tls_session *tls_open_ticket(alt_tls_conn *t, const uint8_t *p, size_t n) {
	if (n < 13 + 16 || p[0] != TICKET_MAGIC)
		return 0;
	uint8_t *pt = malloc(n);
	if (!pt)
		return 0;
	alt_gcm g;
	ticket_gcm(t, &g);
	int rc = alt_gcm_open(&g, pt, p + 1, 12, p + 13, n - 13, p, 13);
	memset(&g, 0, sizeof g);
	tls_session *s = 0;
	if (rc == 0) {
		tr r = {pt, n - 13 - 16}, secret, alpn, certs;
		unsigned version, suite, group, ems;
		uint32_t hi, lo;
		if (tr_u16(&r, &version) && tr_u16(&r, &suite) && tr_u16(&r, &group) && tr_u8(&r, &ems) && tr_u32(&r, &hi) && tr_u32(&r, &lo) &&
			tr_vec(&r, 1, &secret) && secret.n <= 48 && tr_vec(&r, 1, &alpn) && tr_vec(&r, 3, &certs) && !r.n &&
			(s = calloc(1, sizeof *s))) {
			s->version = (uint16_t)version;
			s->suite = (uint16_t)suite;
			s->group = (uint16_t)group;
			s->ems = ems != 0;
			s->created = (int64_t)((uint64_t)hi << 32 | lo);
			memcpy(s->secret, secret.p, secret.n);
			s->nsecret = secret.n;
			memcpy(s->alpn, alpn.p, alpn.n);
			s->alpn[alpn.n] = 0;
			if (certs.n) {
				s->certs = malloc(certs.n);
				if (!s->certs) {
					free(s);
					s = 0;
				} else {
					memcpy(s->certs, certs.p, certs.n);
					s->ncerts_bytes = certs.n;
				}
			}
		}
	}
	memset(pt, 0, n);
	free(pt);
	return s;
}
