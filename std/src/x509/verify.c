/* Certificate verification, after Go's crypto/x509 (verify.go,
   constraints.go, cert_pool.go, x509.go's checkSignature, root_linux.go;
   BSD-3-Clause, see LICENSE-Go): the same chain building, checks and
   messages. Policy constraints are not evaluated. */
#include <alt/crypto/ecdsa.h>
#include <alt/crypto/ed25519.h>
#include <alt/crypto/rsa.h>
#include <alt/crypto/x509.h>
#include <alt/encoding/pem.h>
#include <alt/net.h>
#include <alt/net/url.h>
#include <alt/strconv.h>
#include <alt/sync.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* ---- pools ---- */

struct alt_x509_pool {
	alt_arena a;
	alt_x509_cert **certs;
	size_t n, cap;
};

alt_x509_pool *alt_x509_pool_new(void) {
	alt_x509_pool *p = calloc(1, sizeof *p);
	if (p)
		alt_arena_init(&p->a, 0);
	return p;
}

void alt_x509_pool_free(alt_x509_pool *p) {
	if (!p)
		return;
	alt_arena_free(&p->a);
	free(p->certs);
	free(p);
}

static bool raw_eq(alt_der a, alt_der b) { return a.n == b.n && (a.n == 0 || memcmp(a.p, b.p, a.n) == 0); }

int alt_x509_pool_add_der(alt_x509_pool *p, const void *der, size_t n) {
	void *copy = alt_arena_alloc(&p->a, n ? n : 1);
	if (!copy)
		return -ENOMEM;
	memcpy(copy, der, n);
	alt_x509_cert *c;
	int rc = alt_x509_parse(&p->a, copy, n, &c, 0);
	if (rc < 0)
		return rc;
	/* a certificate already in the pool is not added again (as Go's AddCert) */
	for (size_t i = 0; i < p->n; i++)
		if (raw_eq(p->certs[i]->raw, c->raw))
			return 0;
	if (p->n == p->cap) {
		size_t cap = p->cap ? 2 * p->cap : 16;
		alt_x509_cert **x = realloc(p->certs, cap * sizeof *x);
		if (!x)
			return -ENOMEM;
		p->certs = x;
		p->cap = cap;
	}
	p->certs[p->n++] = c;
	return 0;
}

int alt_x509_pool_add_pem(alt_x509_pool *p, const void *pem, size_t n) {
	alt_arena tmp;
	alt_arena_init(&tmp, 0);
	alt_str rest = alt_str_make(pem, n);
	alt_pem_block b;
	int added = 0;
	while (alt_pem_decode(&tmp, &rest, &b)) {
		if (!alt_str_eq_cstr(b.type, "CERTIFICATE") || b.nheaders)
			continue;
		if (alt_x509_pool_add_der(p, b.bytes.p, b.bytes.n) == 0)
			added++;
	}
	alt_arena_free(&tmp);
	return added;
}

size_t alt_x509_pool_len(const alt_x509_pool *p) { return p ? p->n : 0; }
const alt_x509_cert *alt_x509_pool_get(const alt_x509_pool *p, size_t i) { return i < p->n ? p->certs[i] : 0; }

static bool pool_contains(const alt_x509_pool *p, const alt_x509_cert *c) {
	for (size_t i = 0; p && i < p->n; i++)
		if (raw_eq(p->certs[i]->raw, c->raw))
			return true;
	return false;
}

/* ---- system roots (root_linux.go, root_unix.go) ---- */

static const char *cert_files[] = {
	"/etc/ssl/certs/ca-certificates.crt",
	"/etc/pki/tls/certs/ca-bundle.crt",
	"/etc/ssl/ca-bundle.pem",
	"/etc/pki/tls/cacert.pem",
	"/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",
	"/etc/ssl/cert.pem",
};
static const char *cert_dirs[] = {"/etc/ssl/certs", "/etc/pki/tls/certs"};

static bool read_file(const char *path, alt_buf *b) {
	FILE *f = fopen(path, "rb");
	if (!f)
		return false;
	char tmp[65536];
	size_t k;
	while ((k = fread(tmp, 1, sizeof tmp, f)) > 0)
		alt_buf_append(b, tmp, k);
	fclose(f);
	return true;
}

static alt_x509_pool *system_pool;
static alt_once system_once = ALT_ONCE_INIT;

static void load_dir(alt_x509_pool *p, const char *dir) {
	DIR *d = opendir(dir);
	if (!d)
		return;
	struct dirent *e;
	while ((e = readdir(d))) {
		if (e->d_name[0] == '.')
			continue;
		char path[4096];
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		struct stat st;
		if (stat(path, &st) < 0 || !S_ISREG(st.st_mode))
			continue;
		alt_buf b = ALT_BUF_INIT;
		if (read_file(path, &b))
			alt_x509_pool_add_pem(p, b.p, b.n);
		alt_buf_free(&b);
	}
	closedir(d);
}

static void load_system(void *arg) {
	(void)arg;
	alt_x509_pool *p = alt_x509_pool_new();
	if (!p)
		return;
	const char *file = getenv("SSL_CERT_FILE"), *dir = getenv("SSL_CERT_DIR");
	alt_buf b = ALT_BUF_INIT;
	if (file) {
		if (read_file(file, &b))
			alt_x509_pool_add_pem(p, b.p, b.n);
	} else
		for (size_t i = 0; i < sizeof cert_files / sizeof *cert_files; i++)
			if (read_file(cert_files[i], &b)) {
				alt_x509_pool_add_pem(p, b.p, b.n);
				break;
			}
	alt_buf_free(&b);
	if (dir) {
		char *dirs = strdup(dir), *save = 0;
		for (char *t = strtok_r(dirs, ":", &save); t; t = strtok_r(0, ":", &save))
			load_dir(p, t);
		free(dirs);
	} else if (p->n == 0)
		for (size_t i = 0; i < sizeof cert_dirs / sizeof *cert_dirs; i++)
			load_dir(p, cert_dirs[i]);
	if (p->n == 0) {
		alt_x509_pool_free(p);
		p = 0;
	}
	system_pool = p;
}

const alt_x509_pool *alt_x509_system_roots(void) {
	alt_once_do(&system_once, load_system, 0);
	return system_pool;
}

/* ---- signatures ---- */

static const struct {
	alt_x509_sig_alg alg;
	const char *name;
	alt_x509_pk_alg pk;
	int hash;              /* alt_hash_kind, -1 none */
	bool pss;
} sig_details[] = {
	{ALT_X509_MD5_RSA, "MD5-RSA", ALT_X509_PK_RSA, ALT_MD5, false},
	{ALT_X509_SHA1_RSA, "SHA1-RSA", ALT_X509_PK_RSA, ALT_SHA1, false},
	{ALT_X509_SHA256_RSA, "SHA256-RSA", ALT_X509_PK_RSA, ALT_SHA256, false},
	{ALT_X509_SHA384_RSA, "SHA384-RSA", ALT_X509_PK_RSA, ALT_SHA384, false},
	{ALT_X509_SHA512_RSA, "SHA512-RSA", ALT_X509_PK_RSA, ALT_SHA512, false},
	{ALT_X509_SHA256_RSAPSS, "SHA256-RSAPSS", ALT_X509_PK_RSA, ALT_SHA256, true},
	{ALT_X509_SHA384_RSAPSS, "SHA384-RSAPSS", ALT_X509_PK_RSA, ALT_SHA384, true},
	{ALT_X509_SHA512_RSAPSS, "SHA512-RSAPSS", ALT_X509_PK_RSA, ALT_SHA512, true},
	{ALT_X509_DSA_SHA1, "DSA-SHA1", ALT_X509_PK_DSA, ALT_SHA1, false},
	{ALT_X509_DSA_SHA256, "DSA-SHA256", ALT_X509_PK_DSA, ALT_SHA256, false},
	{ALT_X509_ECDSA_SHA1, "ECDSA-SHA1", ALT_X509_PK_ECDSA, ALT_SHA1, false},
	{ALT_X509_ECDSA_SHA256, "ECDSA-SHA256", ALT_X509_PK_ECDSA, ALT_SHA256, false},
	{ALT_X509_ECDSA_SHA384, "ECDSA-SHA384", ALT_X509_PK_ECDSA, ALT_SHA384, false},
	{ALT_X509_ECDSA_SHA512, "ECDSA-SHA512", ALT_X509_PK_ECDSA, ALT_SHA512, false},
	{ALT_X509_ED25519, "Ed25519", ALT_X509_PK_ED25519, -1, false},
};

static const char *pk_names[] = {"UnknownPublicKeyAlgorithm", "RSA", "DSA", "ECDSA", "Ed25519", "ML-DSA"};
static const char *pk_types[] = {"<nil>", "*rsa.PublicKey", "*dsa.PublicKey", "*ecdsa.PublicKey", "ed25519.PublicKey", "*mldsa.PublicKey"};

/* Go's checkSignature (SHA-1 not allowed, as for certificates) */
static int check_signature(const alt_x509_cert *key, alt_x509_sig_alg alg, const void *signed_data, size_t n,
	const void *sig, size_t nsig, const char **why) {
	int hash = -2;
	alt_x509_pk_alg pk = ALT_X509_PK_UNKNOWN;
	const char *name = "0";
	bool pss = false;
	for (size_t i = 0; i < sizeof sig_details / sizeof *sig_details; i++)
		if (sig_details[i].alg == alg) {
			hash = sig_details[i].hash, pk = sig_details[i].pk, name = sig_details[i].name, pss = sig_details[i].pss;
			break;
		}
	static char msg[128];
	if (hash == -2 || (hash == -1 && pk != ALT_X509_PK_ED25519)) {
		*why = "x509: cannot verify signature: algorithm unimplemented";
		return -ALT_EX509_VERIFY;
	}
	if (hash == ALT_MD5 || hash == ALT_SHA1) {
		snprintf(msg, sizeof msg, "x509: cannot verify signature: insecure algorithm %s", name);
		*why = msg;
		return -ALT_EX509_VERIFY;
	}
	uint8_t digest[ALT_HASH_MAX_SIZE];
	const void *msgp = signed_data;
	size_t msgn = n;
	if (hash >= 0) {
		alt_hash_data((alt_hash_kind)hash, signed_data, n, digest);
		msgp = digest;
		msgn = alt_hash_size((alt_hash_kind)hash);
	}
	alt_x509_pk_alg have = key->pk_alg;
	if (have == ALT_X509_PK_DSA || have == ALT_X509_PK_MLDSA || have == ALT_X509_PK_UNKNOWN) {
		*why = "x509: cannot verify signature: algorithm unimplemented";
		return -ALT_EX509_VERIFY;
	}
	if (pk != have) {
		snprintf(msg, sizeof msg, "x509: signature algorithm specifies an %s public key, but have public key of type %s", pk_names[pk],
			pk_types[have]);
		*why = msg;
		return -ALT_EX509_VERIFY;
	}
	int rc;
	switch (have) {
	case ALT_X509_PK_RSA: {
		alt_rsa_public pub = {key->pub.n.p, key->pub.n.n, key->pub.e};
		rc = pss ? alt_rsa_verify_pss(&pub, (alt_hash_kind)hash, msgp, msgn, sig, nsig, (int)msgn)
			 : alt_rsa_verify_pkcs1v15(&pub, (alt_hash_kind)hash, msgp, msgn, sig, nsig);
		if (rc < 0) {
			*why = rc == -ALT_EKEY ? "crypto/rsa: invalid public key" : "crypto/rsa: verification error";
			return -ALT_EX509_VERIFY;
		}
		return 0;
	}
	case ALT_X509_PK_ECDSA:
		if (alt_ecdsa_verify((alt_ec_curve)key->pub.curve, key->pub.point.p, key->pub.point.n, msgp, msgn, sig, nsig) < 0) {
			*why = "x509: ECDSA verification failure";
			return -ALT_EX509_VERIFY;
		}
		return 0;
	case ALT_X509_PK_ED25519:
		if (nsig != 64 || alt_ed25519_verify(key->pub.point.p, msgp, msgn, sig) < 0) {
			*why = "x509: Ed25519 verification failure";
			return -ALT_EX509_VERIFY;
		}
		return 0;
	default:
		*why = "x509: cannot verify signature: algorithm unimplemented";
		return -ALT_EX509_VERIFY;
	}
}

int alt_x509_check_signature(const alt_x509_cert *c, alt_x509_sig_alg alg, const void *signed_data, size_t n, const void *sig,
	size_t nsig, const char **why) {
	const char *w = 0;
	int rc = check_signature(c, alg, signed_data, n, sig, nsig, &w);
	if (why)
		*why = w;
	return rc;
}

int alt_x509_check_signature_from(const alt_x509_cert *c, const alt_x509_cert *parent, const char **why) {
	const char *w = 0;
	int rc = 0;
	if ((parent->version == 3 && !parent->basic_constraints_valid) || (parent->basic_constraints_valid && !parent->is_ca) ||
		(parent->key_usage != 0 && !(parent->key_usage & ALT_X509_KU_CERT_SIGN))) {
		w = "x509: invalid signature: parent certificate cannot sign this kind of certificate";
		rc = -ALT_EX509_VERIFY;
	} else if (parent->pk_alg == ALT_X509_PK_UNKNOWN) {
		w = "x509: cannot verify signature: algorithm unimplemented";
		rc = -ALT_EX509_VERIFY;
	} else
		rc = check_signature(parent, c->sig_alg, c->raw_tbs.p, c->raw_tbs.n, c->signature.p, c->signature.n, &w);
	if (why)
		*why = w;
	return rc;
}

/* ---- host names ---- */

static bool is_lower_ok(alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (c >= 'A' && c <= 'Z')
			return false;
	}
	return true;
}

/* Go's toLowerCaseASCII (bytes; invalid UTF-8 kept as it is) */
static alt_str lower_ascii(alt_arena *a, alt_str s) {
	if (is_lower_ok(s))
		return s;
	char *p = alt_arena_alloc(a, s.n ? s.n : 1);
	for (size_t i = 0; i < s.n; i++)
		p[i] = (char)((s.p[i] >= 'A' && s.p[i] <= 'Z') ? s.p[i] + 32 : s.p[i]);
	return alt_str_make(p, s.n);
}

/* Go's validHostname */
static bool valid_hostname(alt_str host, bool pattern) {
	if (!pattern && host.n && host.p[host.n - 1] == '.')
		host.n--;
	if (host.n == 0 || alt_str_eq_cstr(host, "*"))
		return false;
	alt_str rest = host, part;
	int i = 0;
	for (bool more = true; more; i++) {
		more = alt_str_cut_byte(rest, '.', &part, &rest); /* the whole rest when there is no dot */
		if (part.n == 0)
			return false;
		if (pattern && i == 0 && alt_str_eq_cstr(part, "*"))
			continue;
		/* range over runes: anything not ASCII fails */
		for (size_t j = 0; j < part.n; j++) {
			unsigned char c = (unsigned char)part.p[j];
			if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c == '-' && j != 0) || c == '_')
				continue;
			return false;
		}
	}
	return true;
}

static bool match_exactly(alt_arena *a, alt_str x, alt_str y) {
	if (x.n == 0 || alt_str_eq_cstr(x, ".") || y.n == 0 || alt_str_eq_cstr(y, "."))
		return false;
	return alt_str_eq(lower_ascii(a, x), lower_ascii(a, y));
}

/* Go's matchHostnames with the host split at dots */
static bool match_hostnames(alt_arena *a, alt_str pattern, alt_str host) {
	pattern = lower_ascii(a, pattern);
	if (pattern.n == 0 || host.n == 0)
		return false;
	/* splitHostname drops one trailing dot */
	if (host.p[host.n - 1] == '.')
		host.n--;
	alt_str pr = pattern, hr = host, pp, hp;
	int i = 0;
	for (;; i++) {
		bool pm = alt_str_cut_byte(pr, '.', &pp, &pr), hm = alt_str_cut_byte(hr, '.', &hp, &hr);
		if (pm != hm)
			return false;
		if (!(i == 0 && alt_str_eq_cstr(pp, "*")) && !alt_str_eq(pp, hp))
			return false;
		if (!pm)
			return true;
	}
}

static bool has_ext(const alt_x509_cert *c, uint32_t last) {
	for (size_t i = 0; i < c->nexts; i++) {
		const alt_oid *o = &c->exts[i].id;
		if (o->n == 4 && o->arc[0] == 2 && o->arc[1] == 5 && o->arc[2] == 29 && o->arc[3] == last)
			return true;
	}
	return false;
}

static alt_str common_name(const alt_x509_name *n) {
	alt_str cn = alt_str_make("", 0);
	for (size_t i = 0; i < n->n; i++)
		if (alt_oid_eq(&n->attrs[i].type, &ALT_OID(2, 5, 4, 3)))
			cn = n->attrs[i].value; /* the last one, as Go's FillFromRDNSequence */
	return cn;
}

static void ip_string(alt_buf *b, const uint8_t *ip, int len) {
	alt_ip x;
	memset(&x, 0, sizeof x);
	x.valid = true;
	if (len == 4) {
		x = alt_ip_v4(ip[0], ip[1], ip[2], ip[3]);
	} else {
		memcpy(x.b, ip, 16);
		static const uint8_t v4[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
		if (memcmp(ip, v4, 12) == 0)
			x = alt_ip_v4(ip[12], ip[13], ip[14], ip[15]);
	}
	alt_ip_format(&x, b);
}

/* Go's net.IP.Equal for a parsed address against a SAN */
static bool ip_equal(const alt_ip *ip, const uint8_t *san, int len) {
	if (len == 4)
		return ip->is4 && memcmp(ip->b + 12, san, 4) == 0;
	return memcmp(ip->b, san, 16) == 0;
}

static const char *hostname_error(alt_arena *a, const alt_x509_cert *c, alt_str h) {
	if (!has_ext(c, 17) && match_hostnames(a, common_name(&c->subject), h))
		return "x509: certificate relies on legacy Common Name field, use SANs instead";
	alt_buf v = ALT_BUF_INIT;
	alt_ip ip;
	const char *res;
	if (alt_ip_parse(h, &ip) == 0 && !ip.zone[0]) {
		if (c->nips == 0) {
			res = alt_arena_printf(a, "x509: cannot validate certificate for %.*s because it doesn't contain any IP SANs", (int)h.n, h.p);
			goto out;
		}
		if (c->nips >= 100) {
			res = alt_arena_printf(a, "x509: certificate is valid for %zu IP SANs, but none matched %.*s", c->nips, (int)h.n, h.p);
			goto out;
		}
		for (size_t i = 0; i < c->nips; i++) {
			if (v.n)
				alt_buf_append_cstr(&v, ", ");
			ip_string(&v, c->ips[i].b, c->ips[i].len);
		}
	} else {
		if (c->dns_names.n >= 100) {
			res = alt_arena_printf(a, "x509: certificate is valid for %zu names, but none matched %.*s", c->dns_names.n, (int)h.n, h.p);
			goto out;
		}
		for (size_t i = 0; i < c->dns_names.n; i++) {
			if (i)
				alt_buf_append_cstr(&v, ", ");
			alt_buf_append_str(&v, c->dns_names.items[i]);
		}
	}
	if (v.n == 0)
		res = alt_arena_printf(a, "x509: certificate is not valid for any names, but wanted to match %.*s", (int)h.n, h.p);
	else
		res = alt_arena_printf(a, "x509: certificate is valid for %s, not %.*s", alt_buf_cstr(&v), (int)h.n, h.p);
out:
	alt_buf_free(&v);
	return res;
}

int alt_x509_verify_hostname(const alt_x509_cert *c, const char *host, alt_arena *a, const char **why) {
	alt_str h = alt_str_cstr(host), cand = h;
	if (h.n >= 3 && h.p[0] == '[' && h.p[h.n - 1] == ']')
		cand = alt_str_slice(h, 1, h.n - 1);
	alt_ip ip;
	if (alt_ip_parse(cand, &ip) == 0) {
		for (size_t i = 0; i < c->nips; i++)
			if (ip_equal(&ip, c->ips[i].b, c->ips[i].len))
				return 0;
		/* HostnameError{c, ip.String()} */
		alt_buf s = ALT_BUF_INIT;
		alt_ip plain = ip;
		plain.zone[0] = 0;
		alt_ip_format(&plain, &s);
		if (why)
			*why = hostname_error(a, c, alt_arena_str(a, alt_buf_str(&s)));
		alt_buf_free(&s);
		return -ALT_EX509_VERIFY;
	}
	alt_str name = lower_ascii(a, h);
	bool valid = valid_hostname(name, false);
	for (size_t i = 0; i < c->dns_names.n; i++) {
		alt_str m = c->dns_names.items[i];
		if (valid && valid_hostname(m, true)) {
			if (match_hostnames(a, m, name))
				return 0;
		} else if (match_exactly(a, m, name))
			return 0;
	}
	if (why)
		*why = hostname_error(a, c, h);
	return -ALT_EX509_VERIFY;
}

/* ---- name constraints (constraints.go) ---- */

/* Go's domainNameValid */
static bool domain_valid(alt_str s, bool constraint) {
	if (s.n == 0)
		return true;
	if (s.p[s.n - 1] == '.')
		return false;
	if (constraint && s.p[0] == '.')
		s = alt_str_slice(s, 1, s.n);
	long last = -1;
	for (size_t i = 0; i <= s.n; i++) {
		if (i < s.n && ((unsigned char)s.p[i] < 33 || (unsigned char)s.p[i] > 126))
			return false;
		if (i == s.n || s.p[i] == '.') {
			if ((long)i - (last >= 0 ? last + 1 : 0) == 0)
				return false;
			last = (long)i;
		}
	}
	return true;
}

/* Go's dnsHasSuffix(a, b): b is a or below it */
static bool dns_has_suffix(alt_str a, alt_str b) {
	if (a.n > b.n)
		return false;
	size_t off = b.n - a.n;
	for (size_t i = a.n; i-- > 0;) {
		unsigned char x = (unsigned char)a.p[i], y = (unsigned char)b.p[i + off];
		if (x == y)
			continue;
		if (y < x) {
			unsigned char t = x;
			x = y, y = t;
		}
		if (x >= 'A' && x <= 'Z' && y == x + 32)
			continue;
		return false;
	}
	if (a.n && a.p[0] != '.' && b.n > a.n && b.p[b.n - a.n - 1] != '.')
		return false;
	return true;
}

static alt_str trim_first_label(alt_str s) {
	ssize_t i = alt_str_index_byte(s, '.');
	return i < 0 ? alt_str_make("", 0) : alt_str_slice(s, (size_t)i, s.n);
}

/* a constraint of the list matching name: NULL if none (newDNSConstraints + query) */
static const alt_str *dns_query(alt_arena *a, const alt_x509_strs *l, alt_str name, bool permitted, bool *all) {
	*all = false;
	if (l->n == 0)
		return 0;
	for (size_t i = 0; i < l->n; i++)
		if (l->items[i].n == 0) {
			*all = true;
			return &l->items[i];
		}
	for (size_t i = 0; i < l->n; i++)
		if (dns_has_suffix(l->items[i], name))
			return &l->items[i];
	if (!permitted && name.n > 0 && name.p[0] == '*') {
		alt_str t = trim_first_label(lower_ascii(a, name));
		for (size_t i = 0; i < l->n; i++) {
			alt_str parent = trim_first_label(lower_ascii(a, l->items[i]));
			if (parent.n && alt_str_eq(parent, t))
				return &l->items[i];
		}
	}
	return 0;
}

typedef struct {
	alt_str local, domain; /* local unescaped, domain lower-cased */
	bool ok;
} mailbox;

/* Go's parseRFC2821Mailbox */
static mailbox parse_mailbox(alt_arena *a, alt_str in) {
	mailbox mb = {0};
	if (in.n == 0)
		return mb;
	char *local = alt_arena_alloc(a, in.n + 1);
	size_t nl = 0, i = 0;
	if (in.p[0] == '"') {
		i = 1;
		for (;;) {
			if (i >= in.n)
				return mb;
			unsigned char c = (unsigned char)in.p[i++];
			if (c == '"')
				break;
			if (c == '\\') {
				if (i >= in.n)
					return mb;
				unsigned char n = (unsigned char)in.p[i];
				if (n == 11 || n == 12 || (1 <= n && n <= 9) || (14 <= n && n <= 127))
					local[nl++] = (char)n, i++;
				else
					return mb;
			} else if (c == 11 || c == 12 || c == 32 || c == 33 || c == 127 || (1 <= c && c <= 8) || (14 <= c && c <= 31) ||
				   (35 <= c && c <= 91) || (93 <= c && c <= 126))
				local[nl++] = (char)c;
			else
				return mb;
		}
	} else {
		while (i < in.n) {
			unsigned char c = (unsigned char)in.p[i];
			if (c == '\\') {
				i++;
				if (i >= in.n)
					return mb;
			} else if (!(('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
					   (c && strchr("!#$%&'*+-/=?^_`{|}~.", c))))
				break;
			local[nl++] = in.p[i++];
		}
		if (nl == 0 || local[0] == '.' || local[nl - 1] == '.')
			return mb;
		for (size_t k = 0; k + 1 < nl; k++)
			if (local[k] == '.' && local[k + 1] == '.')
				return mb;
	}
	if (i >= in.n || in.p[i] != '@')
		return mb;
	alt_str dom = alt_str_slice(in, i + 1, in.n);
	if (!domain_valid(dom, false) || alt_str_index_byte(dom, '@') >= 0)
		return mb;
	mb.local = alt_str_make(local, nl);
	mb.domain = dom;
	mb.ok = true;
	return mb;
}

static const alt_str *email_query(alt_arena *a, const alt_x509_strs *l, mailbox m, bool permitted, bool *any) {
	*any = l->n > 0;
	alt_x509_strs domains = {0};
	alt_str buf[64];
	domains.items = buf;
	for (size_t i = 0; i < l->n; i++) {
		if (alt_str_index_byte(l->items[i], '@') < 0) {
			if (domains.n < 64)
				buf[domains.n++] = l->items[i];
			continue;
		}
		mailbox c = parse_mailbox(a, l->items[i]);
		if (c.ok && alt_str_eq(c.local, m.local) && alt_str_eq(lower_ascii(a, c.domain), m.domain))
			return &l->items[i];
	}
	bool all;
	return dns_query(a, &domains, m.domain, permitted, &all);
}

static bool ip_in_net(const alt_x509_ipnet *n, const uint8_t *ip, int len) {
	/* net.IPNet.Contains: a 4-byte network against an IPv4 (or mapped) address */
	uint8_t x[16];
	int xl = len;
	memcpy(x, ip, (size_t)len);
	static const uint8_t v4[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
	if (len == 16 && memcmp(ip, v4, 12) == 0) {
		memmove(x, ip + 12, 4);
		xl = 4;
	}
	if (n->len != xl)
		return false;
	for (int i = 0; i < xl; i++)
		if ((x[i] & n->mask[i]) != (n->ip[i] & n->mask[i]))
			return false;
	return true;
}

static const char *q(alt_arena *a, alt_str s) {
	alt_buf b = ALT_BUF_INIT;
	alt_quote(&b, s);
	const char *r = alt_arena_strdup(a, alt_buf_cstr(&b));
	alt_buf_free(&b);
	return r;
}

/* one certificate's constraints against the names of a certificate below it */
static const char *check_constraints(alt_arena *a, const alt_x509_cert *ca, const alt_x509_cert *c) {
	bool all;
	for (size_t i = 0; i < c->nips; i++) {
		alt_buf s = ALT_BUF_INIT;
		ip_string(&s, c->ips[i].b, c->ips[i].len);
		alt_str ips = alt_arena_str(a, alt_buf_str(&s));
		alt_buf_free(&s);
		int len = c->ips[i].len;
		const uint8_t *ip = c->ips[i].b;
		if (ca->npermitted_ips) {
			bool found = false;
			for (size_t k = 0; k < ca->npermitted_ips && !found; k++)
				found = ca->permitted_ips[k].len == len && ip_in_net(&ca->permitted_ips[k], ip, len);
			if (!found)
				return alt_arena_printf(a, "IP address %s is not permitted by any constraint", q(a, ips));
		}
		for (size_t k = 0; k < ca->nexcluded_ips; k++)
			if (ca->excluded_ips[k].len == len && ip_in_net(&ca->excluded_ips[k], ip, len)) {
				alt_buf nb = ALT_BUF_INIT;
				ip_string(&nb, ca->excluded_ips[k].ip, len);
				int ones = 0;
				for (int j = 0; j < len; j++)
					ones += __builtin_popcount(ca->excluded_ips[k].mask[j]);
				alt_buf_printf(&nb, "/%d", ones);
				const char *r = alt_arena_printf(a, "IP address %s is excluded by constraint %s", q(a, ips), q(a, alt_buf_str(&nb)));
				alt_buf_free(&nb);
				return r;
			}
	}
	for (size_t i = 0; i < c->dns_names.n; i++) {
		alt_str d = c->dns_names.items[i];
		if (!domain_valid(d, false))
			return alt_arena_printf(a, "x509: cannot parse dnsName %s", q(a, d));
		if (ca->permitted_dns.n && !dns_query(a, &ca->permitted_dns, d, true, &all))
			return alt_arena_printf(a, "DNS name %s is not permitted by any constraint", q(a, d));
		const alt_str *ex = dns_query(a, &ca->excluded_dns, d, false, &all);
		if (ex)
			return alt_arena_printf(a, "DNS name %s is excluded by constraint %s", q(a, d), q(a, *ex));
	}
	for (size_t i = 0; i < c->uris.n; i++) {
		alt_url u;
		if (alt_url_parse(a, c->uris.items[i], &u) < 0)
			continue;
		alt_buf us = ALT_BUF_INIT;
		alt_url_string(&u, &us);
		alt_str ustr = alt_arena_str(a, alt_buf_str(&us));
		alt_buf_free(&us);
		alt_str host = lower_ascii(a, u.host);
		if (host.n == 0)
			return alt_arena_printf(a, "URI with empty host (%s) cannot be matched against constraints", q(a, ustr));
		if (alt_str_index_byte(host, ':') >= 0 && !alt_str_has_suffix(host, ALT_STR("]"))) {
			alt_str h2, port;
			if (alt_split_host_port(host, &h2, &port) < 0)
				return alt_arena_printf(a, "cannot parse URI host %s", q(a, u.host));
			host = h2;
		}
		alt_ip ip;
		if (alt_ip_parse(host, &ip) == 0 || (alt_str_has_prefix(host, ALT_STR("[")) && alt_str_has_suffix(host, ALT_STR("]"))))
			return alt_arena_printf(a, "URI with IP (%s) cannot be matched against constraints", q(a, ustr));
		if (!domain_valid(host, false))
			return alt_arena_printf(a, "x509: internal error: URI SAN %s failed to parse", q(a, ustr));
		if (ca->permitted_uris.n && !dns_query(a, &ca->permitted_uris, host, true, &all))
			return alt_arena_printf(a, "URI %s is not permitted by any constraint", q(a, ustr));
		const alt_str *ex = dns_query(a, &ca->excluded_uris, host, false, &all);
		if (ex)
			return alt_arena_printf(a, "URI %s is excluded by constraint %s", q(a, ustr), q(a, *ex));
	}
	for (size_t i = 0; i < c->emails.n; i++) {
		mailbox m = parse_mailbox(a, c->emails.items[i]);
		if (!m.ok)
			return alt_arena_printf(a, "cannot parse rfc822Name %s", q(a, c->emails.items[i]));
		m.domain = lower_ascii(a, m.domain);
		alt_str e = alt_arena_str(a, alt_str_cstr(alt_arena_printf(a, "%.*s@%.*s", (int)m.local.n, m.local.p, (int)m.domain.n, m.domain.p)));
		if (!domain_valid(m.domain, false))
			return alt_arena_printf(a, "x509: cannot parse rfc822Name %s", q(a, e));
		bool any;
		if (ca->permitted_emails.n && !email_query(a, &ca->permitted_emails, m, true, &any))
			return alt_arena_printf(a, "email address %s is not permitted by any constraint", q(a, e));
		const alt_str *ex = email_query(a, &ca->excluded_emails, m, false, &any);
		if (ex) {
			alt_str shown = *ex;
			if (alt_str_index_byte(*ex, '@') >= 0)
				shown = e;
			return alt_arena_printf(a, "email address %s is excluded by constraint %s", q(a, e), q(a, shown));
		}
	}
	return 0;
}

/* checkChainConstraints: every constraining certificate against the
   names of the certificates below it */
static const char *chain_constraints(alt_arena *a, const alt_x509_cert **chain, size_t n) {
	for (size_t i = 0; i < n; i++) {
		if (!has_ext(chain[i], 17))
			continue;
		for (size_t k = i + 1; k < n; k++) {
			if (!has_ext(chain[k], 30))
				continue;
			const char *e = check_constraints(a, chain[k], chain[i]);
			if (e)
				return e;
		}
	}
	return 0;
}

/* ---- chains ---- */

enum { LEAF, INTERMEDIATE, ROOT };

typedef struct {
	const alt_x509_verify_opts *o;
	alt_arena *a;
	int64_t now;
	int sig_checks;
	alt_x509_chain *chains;
	size_t n, cap;
} vctx;

static void fmt_time(char *buf, size_t n, int64_t t) {
	int64_t days = t >= 0 ? t / 86400 : -((-t + 86399) / 86400), rem = t - days * 86400;
	days += 719468;
	int64_t era = (days >= 0 ? days : days - 146096) / 146097, doe = days - era * 146097;
	int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365, doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	int64_t mp = (5 * doy + 2) / 153;
	int d = (int)(doy - (153 * mp + 2) / 5 + 1), m = (int)(mp < 10 ? mp + 3 : mp - 9);
	int64_t y = yoe + era * 400 + (m <= 2);
	snprintf(buf, n, "%04lld-%02d-%02dT%02d:%02d:%02dZ", (long long)y, m, d, (int)(rem / 3600), (int)(rem / 60 % 60), (int)(rem % 60));
}

static const char *is_valid(vctx *v, const alt_x509_cert *c, int type, const alt_x509_cert **chain, size_t len) {
	if (c->nunhandled_critical)
		return "x509: unhandled critical extension";
	if (len > 0 && !raw_eq(chain[len - 1]->raw_issuer, c->raw_subject))
		return "x509: issuer name does not match subject from issuing certificate";
	char a[40], b[40];
	if (v->now < c->not_before) {
		fmt_time(a, sizeof a, v->now);
		fmt_time(b, sizeof b, c->not_before);
		return alt_arena_printf(v->a, "x509: certificate has expired or is not yet valid: current time %s is before %s", a, b);
	}
	if (v->now > c->not_after) {
		fmt_time(a, sizeof a, v->now);
		fmt_time(b, sizeof b, c->not_after);
		return alt_arena_printf(v->a, "x509: certificate has expired or is not yet valid: current time %s is after %s", a, b);
	}
	if (type == INTERMEDIATE && (!c->basic_constraints_valid || !c->is_ca))
		return "x509: certificate is not authorized to sign other certificates";
	if (c->basic_constraints_valid && c->max_path_len >= 0 && (long)len - 1 > c->max_path_len)
		return "x509: too many intermediates for path length constraint";
	return 0;
}

/* Go's alreadyInChain */
static bool in_chain(const alt_x509_cert *cand, const alt_x509_cert **chain, size_t n) {
	const alt_x509_ext *cs = 0;
	for (size_t i = 0; i < cand->nexts; i++)
		if (alt_oid_eq(&cand->exts[i].id, &ALT_OID(2, 5, 29, 17)))
			cs = &cand->exts[i];
	for (size_t k = 0; k < n; k++) {
		const alt_x509_cert *c = chain[k];
		if (!raw_eq(cand->raw_subject, c->raw_subject) || !raw_eq(cand->raw_spki, c->raw_spki))
			continue;
		const alt_x509_ext *s = 0;
		for (size_t i = 0; i < c->nexts; i++)
			if (alt_oid_eq(&c->exts[i].id, &ALT_OID(2, 5, 29, 17)))
				s = &c->exts[i];
		if (!cs && !s)
			return true;
		if (!cs || !s)
			return false;
		if (raw_eq(cs->value, s->value))
			return true;
	}
	return false;
}

static bool add_chain(vctx *v, const alt_x509_cert **chain, size_t n) {
	if (v->n == v->cap) {
		size_t cap = v->cap ? 2 * v->cap : 4;
		alt_x509_chain *c = alt_arena_realloc(v->a, v->chains, v->cap * sizeof *c, cap * sizeof *c);
		if (!c)
			return false;
		v->chains = c;
		v->cap = cap;
	}
	const alt_x509_cert **copy = alt_arena_alloc(v->a, n * sizeof *copy);
	if (!copy)
		return false;
	memcpy(copy, chain, n * sizeof *copy);
	v->chains[v->n++] = (alt_x509_chain){copy, n};
	return true;
}

/* candidate parents in Go's order: matching key IDs, one missing, mismatching */
static size_t potential_parents(const alt_x509_pool *p, const alt_x509_cert *c, const alt_x509_cert **out, size_t max) {
	size_t n = 0;
	for (int pass = 0; pass < 3; pass++)
		for (size_t i = 0; p && i < p->n; i++) {
			const alt_x509_cert *cand = p->certs[i];
			if (!raw_eq(cand->raw_subject, c->raw_issuer))
				continue;
			bool match = raw_eq(cand->subject_key_id, c->authority_key_id);
			bool one = (cand->subject_key_id.n == 0 && c->authority_key_id.n > 0) ||
				   (cand->subject_key_id.n > 0 && c->authority_key_id.n == 0);
			int kind = match ? 0 : one ? 1 : 2;
			if (kind == pass && n < max)
				out[n++] = cand;
		}
	return n;
}

enum { MAX_SIG_CHECKS = 100, MAX_DEPTH = 32 };

static const char LIMIT[] = "x509: signature check attempts limit reached while verifying certificate chain";

static const char *unknown_authority(vctx *v, const char *hint, const alt_x509_cert *hint_cert);

/* buildChains with Go's error semantics: the last failed validity check
   or child search, else an unknown authority (with the first failure as
   the hint); NULL when chains were found */
static const char *build(vctx *v, const alt_x509_cert **chain, size_t len) {
	const alt_x509_cert *c = chain[len - 1];
	const char *err = 0, *hint = 0;
	const alt_x509_cert *hint_cert = 0;
	size_t before = v->n;
	for (int type = ROOT; type >= INTERMEDIATE && err != LIMIT; type--) {
		const alt_x509_pool *pool = type == ROOT ? v->o->roots : v->o->intermediates;
		const alt_x509_cert *cands[64];
		size_t nc = potential_parents(pool, c, cands, 64);
		for (size_t i = 0; i < nc; i++) {
			if (++v->sig_checks > MAX_SIG_CHECKS) {
				err = LIMIT;
				break;
			}
			const alt_x509_cert *cand = cands[i];
			if (cand->pk_alg == ALT_X509_PK_UNKNOWN || in_chain(cand, chain, len))
				continue;
			const char *e = 0;
			if (alt_x509_check_signature_from(c, cand, &e) < 0) {
				if (!hint)
					hint = e, hint_cert = cand;
				continue;
			}
			if ((e = is_valid(v, cand, type, chain, len))) {
				err = e;
				if (!hint)
					hint = e, hint_cert = cand;
				continue;
			}
			chain[len] = cand;
			if (type == ROOT)
				add_chain(v, chain, len + 1);
			else if (len + 1 >= MAX_DEPTH)
				err = "x509: certificate chain too long";
			else if ((err = build(v, chain, len + 1)) == LIMIT)
				break;
		}
	}
	if (v->n > before)
		return 0;
	return err ? err : unknown_authority(v, hint, hint_cert);
}

/* checkChainForKeyUsage */
static bool chain_key_usage(const alt_x509_chain *ch, const int *want, size_t nwant) {
	int usages[16];
	size_t n = nwant < 16 ? nwant : 16, remaining = n;
	memcpy(usages, want, n * sizeof *usages);
	for (size_t i = ch->n; i-- > 0;) {
		const alt_x509_cert *c = ch->certs[i];
		if (c->neku == 0 && c->nunknown_eku == 0)
			continue;
		bool any = false;
		for (size_t k = 0; k < c->neku; k++)
			any |= c->ext_key_usage[k] == ALT_X509_EKU_ANY;
		if (any)
			continue;
		for (size_t r = 0; r < n; r++) {
			if (usages[r] < 0)
				continue;
			bool have = false;
			for (size_t k = 0; k < c->neku; k++)
				have |= c->ext_key_usage[k] == usages[r];
			if (have)
				continue;
			usages[r] = -1;
			if (--remaining == 0)
				return false;
		}
	}
	return true;
}

static const char *serial_decimal(alt_arena *a, alt_der s) {
	uint8_t x[64];
	size_t n = s.n < 64 ? s.n : 64;
	memcpy(x, s.p, n);
	char digits[200];
	int nd = 0;
	for (;;) {
		bool zero = true;
		unsigned rem = 0;
		for (size_t i = 0; i < n; i++) {
			unsigned cur = rem << 8 | x[i];
			x[i] = (uint8_t)(cur / 10);
			rem = cur % 10;
			if (x[i])
				zero = false;
		}
		digits[nd++] = (char)('0' + rem);
		if (zero || nd >= 199)
			break;
	}
	char *out = alt_arena_alloc(a, (size_t)nd + 1);
	for (int i = 0; i < nd; i++)
		out[i] = digits[nd - 1 - i];
	out[nd] = 0;
	return out;
}

static const char *unknown_authority(vctx *v, const char *hint, const alt_x509_cert *hint_cert) {
	alt_arena *a = v->a;
	if (!hint)
		return "x509: certificate signed by unknown authority";
	alt_str name = common_name(&hint_cert->subject);
	if (name.n == 0) {
		for (size_t i = 0; i < hint_cert->subject.n; i++)
			if (alt_oid_eq(&hint_cert->subject.attrs[i].type, &ALT_OID(2, 5, 4, 10))) {
				name = hint_cert->subject.attrs[i].value;
				break;
			}
		if (name.n == 0)
			name = alt_str_cstr(alt_arena_printf(a, "serial:%s", serial_decimal(a, hint_cert->serial)));
	}
	return alt_arena_printf(a,
		"x509: certificate signed by unknown authority (possibly because of %s while trying to verify candidate authority certificate %s)",
		q(a, alt_str_cstr(hint)), q(a, name));
}

/* ---- certificate policies (Go's policiesValid: RFC 5280 6.1 as
   updated by RFC 9618, a graph of policy nodes per depth) ---- */

typedef struct pnode pnode;
typedef struct {
	pnode **p;
	size_t n, cap;
} pset;

struct pnode {
	alt_der valid;
	const alt_der *expected;
	size_t nexpected;
	pset parents, children;
};

typedef struct {
	alt_arena *a;
	pset *strata; /* strata[d]: the nodes at depth d */
	int depth;
	struct {
		alt_der e;
		pnode *n;
	} * index; /* the parent index: expected policy -> parents */
	size_t nindex;
	bool oom;
} pgraph;

static const uint8_t any_policy_der[] = {0x55, 0x1d, 0x20, 0x00}; /* 2.5.29.32.0 */
static const alt_der any_policy = {any_policy_der, sizeof any_policy_der};

static bool der_eq(alt_der x, alt_der y) { return x.n == y.n && (x.n == 0 || memcmp(x.p, y.p, x.n) == 0); }

static void pset_add(pgraph *g, pset *s, pnode *n) {
	for (size_t i = 0; i < s->n; i++)
		if (s->p[i] == n)
			return;
	if (s->n == s->cap) {
		size_t nc = s->cap ? 2 * s->cap : 4;
		pnode **np = alt_arena_realloc(g->a, s->p, s->cap * sizeof *s->p, nc * sizeof *s->p);
		if (!np) {
			g->oom = true;
			return;
		}
		s->p = np, s->cap = nc;
	}
	s->p[s->n++] = n;
}

static void pset_del(pset *s, pnode *n) {
	for (size_t i = 0; i < s->n; i++)
		if (s->p[i] == n) {
			s->p[i] = s->p[--s->n];
			return;
		}
}

static pnode *stratum_get(pset *s, alt_der policy) {
	for (size_t i = 0; i < s->n; i++)
		if (der_eq(s->p[i]->valid, policy))
			return s->p[i];
	return 0;
}

static pnode *pnode_new(pgraph *g, alt_der valid, pnode **parents, size_t np) {
	pnode *n = alt_arena_alloc(g->a, sizeof *n);
	alt_der *e = alt_arena_alloc(g->a, sizeof *e);
	if (!n || !e) {
		g->oom = true;
		return 0;
	}
	memset(n, 0, sizeof *n);
	n->valid = valid;
	*e = valid;
	n->expected = e, n->nexpected = 1;
	for (size_t i = 0; i < np; i++) {
		pset_add(g, &parents[i]->children, n);
		pset_add(g, &n->parents, parents[i]);
	}
	return n;
}

/* insert replaces the node of that policy at the current depth (a map in Go) */
static void pg_insert(pgraph *g, pnode *n) {
	pset *s = &g->strata[g->depth];
	for (size_t i = 0; i < s->n; i++)
		if (der_eq(s->p[i]->valid, n->valid)) {
			s->p[i] = n;
			return;
		}
	pset_add(g, s, n);
}

static void pg_delete_leaf(pgraph *g, alt_der policy) {
	pset *s = &g->strata[g->depth];
	pnode *n = stratum_get(s, policy);
	if (!n)
		return;
	for (size_t i = 0; i < n->parents.n; i++)
		pset_del(&n->parents.p[i]->children, n);
	for (size_t i = 0; i < n->children.n; i++)
		pset_del(&n->children.p[i]->parents, n);
	pset_del(s, n);
}

static void pg_prune(pgraph *g) {
	for (int d = g->depth - 1; d > 0; d--) {
		pset *s = &g->strata[d];
		for (size_t i = 0; i < s->n;) {
			pnode *n = s->p[i];
			if (n->children.n == 0) {
				for (size_t k = 0; k < n->parents.n; k++)
					pset_del(&n->parents.p[k]->children, n);
				s->p[i] = s->p[--s->n];
			} else
				i++;
		}
	}
}

static void pg_incr_depth(pgraph *g) {
	g->nindex = 0;
	pset *s = &g->strata[g->depth];
	size_t total = 0;
	for (size_t i = 0; i < s->n; i++)
		total += s->p[i]->nexpected;
	g->index = alt_arena_alloc(g->a, (total ? total : 1) * sizeof *g->index);
	if (!g->index) {
		g->oom = true;
		return;
	}
	for (size_t i = 0; i < s->n; i++)
		for (size_t k = 0; k < s->p[i]->nexpected; k++) {
			g->index[g->nindex].e = s->p[i]->expected[k];
			g->index[g->nindex++].n = s->p[i];
		}
	g->depth++;
}

static bool self_issued(const alt_x509_cert *c) { return der_eq(c->raw_issuer, c->raw_subject); }

static bool policies_valid(alt_arena *a, const alt_x509_cert *const *chain, size_t len, const alt_x509_verify_opts *o) {
	if (len == 1)
		return true;
	int n = (int)len - 1;
	pgraph g = {.a = a};
	g.strata = alt_arena_alloc(a, (size_t)(n + 2) * sizeof *g.strata);
	if (!g.strata)
		return false;
	memset(g.strata, 0, (size_t)(n + 2) * sizeof *g.strata);
	pnode *root = pnode_new(&g, any_policy, 0, 0);
	if (!root)
		return false;
	pset_add(&g, &g.strata[0], root);
	bool have_graph = true;
	int64_t inhibit_any = n + 1, explicit_policy = n + 1, policy_mapping = n + 1;
	for (int i = n - 1; i >= 0; i--) {
		const alt_x509_cert *cert = chain[i];
		bool self = self_issued(cert);
		if (cert->npolicies == 0)
			have_graph = false;
		if (explicit_policy == 0 && !have_graph)
			return false;
		if (have_graph) {
			pg_incr_depth(&g);
			bool has_any = false;
			for (size_t k = 0; k < cert->npolicies; k++) {
				alt_der pol = cert->policy_oids[k];
				if (der_eq(pol, any_policy)) {
					has_any = true;
					continue;
				}
				/* the parents expecting it, else the anyPolicy parent */
				pset parents = {0};
				for (size_t x = 0; x < g.nindex; x++)
					if (der_eq(g.index[x].e, pol))
						pset_add(&g, &parents, g.index[x].n);
				if (parents.n == 0) {
					pnode *ap = stratum_get(&g.strata[g.depth - 1], any_policy);
					if (ap)
						pset_add(&g, &parents, ap);
				}
				if (parents.n > 0) {
					pnode *nn = pnode_new(&g, pol, parents.p, parents.n);
					if (nn)
						pg_insert(&g, nn);
				}
			}
			if (has_any && (inhibit_any > 0 || (n - i < n && self))) {
				/* each expected policy of a parent without a leaf gets one */
				pset *prev = &g.strata[g.depth - 1], *leaves = &g.strata[g.depth];
				size_t nmiss = 0;
				struct {
					alt_der pol;
					pset parents;
				} *miss = 0;
				size_t capmiss = 0;
				size_t nleaves = leaves->n; /* the leaves before these insertions */
				for (size_t pi = 0; pi < prev->n; pi++) {
					pnode *p = prev->p[pi];
					for (size_t k = 0; k < p->nexpected; k++) {
						alt_der e = p->expected[k];
						bool leaf = false;
						for (size_t l = 0; l < nleaves && !leaf; l++)
							leaf = der_eq(leaves->p[l]->valid, e);
						if (leaf)
							continue;
						size_t m = 0;
						while (m < nmiss && !der_eq(miss[m].pol, e))
							m++;
						if (m == nmiss) {
							if (nmiss == capmiss) {
								size_t nc = capmiss ? 2 * capmiss : 4;
								void *np = alt_arena_realloc(a, miss, capmiss * sizeof *miss, nc * sizeof *miss);
								if (!np)
									return false;
								miss = np, capmiss = nc;
							}
							miss[nmiss].pol = e;
							miss[nmiss++].parents = (pset){0};
						}
						pset_add(&g, &miss[m].parents, p);
					}
				}
				for (size_t m = 0; m < nmiss; m++) {
					pnode *nn = pnode_new(&g, miss[m].pol, miss[m].parents.p, miss[m].parents.n);
					if (nn)
						pg_insert(&g, nn);
				}
			}
			pg_prune(&g);
			if (i != 0 && cert->npolicy_mappings > 0) {
				struct {
					alt_der issuer;
					alt_der *subjects;
					size_t n;
				} *maps = alt_arena_alloc(a, cert->npolicy_mappings * sizeof *maps);
				size_t nmaps = 0;
				if (!maps)
					return false;
				for (size_t k = 0; k < cert->npolicy_mappings; k++) {
					alt_der is = cert->policy_mappings[k].issuer, su = cert->policy_mappings[k].subject;
					if (policy_mapping > 0) {
						if (der_eq(is, any_policy) || der_eq(su, any_policy))
							return false;
						size_t m = 0;
						while (m < nmaps && !der_eq(maps[m].issuer, is))
							m++;
						if (m == nmaps) {
							maps[nmaps].issuer = is;
							maps[nmaps].subjects = alt_arena_alloc(a, cert->npolicy_mappings * sizeof(alt_der));
							if (!maps[nmaps].subjects)
								return false;
							maps[nmaps++].n = 0;
						}
						maps[m].subjects[maps[m].n++] = su;
					} else
						pg_delete_leaf(&g, is);
				}
				pg_prune(&g);
				for (size_t m = 0; m < nmaps; m++) {
					pnode *match = stratum_get(&g.strata[g.depth], maps[m].issuer);
					if (match) {
						match->expected = maps[m].subjects, match->nexpected = maps[m].n;
					} else if ((match = stratum_get(&g.strata[g.depth], any_policy))) {
						pnode *nn = pnode_new(&g, maps[m].issuer, &match, 1);
						if (nn) {
							nn->expected = maps[m].subjects, nn->nexpected = maps[m].n;
							pg_insert(&g, nn);
						}
					}
				}
			}
			if (g.oom)
				return false;
		}
		if (i != 0) {
			if (!self) {
				if (explicit_policy > 0)
					explicit_policy--;
				if (policy_mapping > 0)
					policy_mapping--;
				if (inhibit_any > 0)
					inhibit_any--;
			}
			if ((cert->require_explicit_policy > 0 || cert->require_explicit_policy_zero) &&
				cert->require_explicit_policy < explicit_policy)
				explicit_policy = cert->require_explicit_policy;
			if ((cert->inhibit_policy_mapping > 0 || cert->inhibit_policy_mapping_zero) &&
				cert->inhibit_policy_mapping < policy_mapping)
				policy_mapping = cert->inhibit_policy_mapping;
			if ((cert->inhibit_any_policy > 0 || cert->inhibit_any_policy_zero) && cert->inhibit_any_policy < inhibit_any)
				inhibit_any = cert->inhibit_any_policy;
		}
	}
	if (explicit_policy > 0)
		explicit_policy--;
	if (chain[0]->require_explicit_policy_zero)
		explicit_policy = 0;
	if (explicit_policy != 0)
		return true; /* the user-constrained set only matters when explicit */
	/* the valid policy node set: nodes (not anyPolicy) whose only parent
	   is anyPolicy, plus the anyPolicy leaf */
	pset valid = {0};
	if (have_graph) {
		for (int d = g.depth; d >= 0; d--)
			for (size_t i = 0; i < g.strata[d].n; i++) {
				pnode *x = g.strata[d].p[i];
				if (der_eq(x->valid, any_policy))
					continue;
				if (x->parents.n == 1 && der_eq(x->parents.p[0]->valid, any_policy))
					pset_add(&g, &valid, x);
			}
		pnode *cur_any = stratum_get(&g.strata[g.depth], any_policy);
		if (cur_any)
			pset_add(&g, &valid, cur_any);
	}
	/* the user's set is {anyPolicy} when empty or only anyPolicy */
	bool user_any = true;
	for (size_t k = 0; k < o->ncertificate_policies; k++)
		if (!der_eq(o->certificate_policies[k], any_policy))
			user_any = false;
	if (user_any)
		return valid.n > 0;
	/* the authority-constrained set intersected with the user's, or the
	   user's when the authority's has anyPolicy */
	bool authority_any = false;
	for (size_t i = 0; i < valid.n; i++) {
		if (der_eq(valid.p[i]->valid, any_policy))
			authority_any = true;
		for (size_t k = 0; k < o->ncertificate_policies; k++)
			if (der_eq(valid.p[i]->valid, o->certificate_policies[k]))
				return true;
	}
	return authority_any;
}

int alt_x509_verify(const alt_x509_cert *leaf, const alt_x509_verify_opts *opts, alt_arena *a, alt_x509_chain **chains,
	size_t *nchains, const char **why) {
	alt_x509_verify_opts o = *opts;
	const char *w = 0;
	*chains = 0;
	*nchains = 0;
	if (!o.roots && !(o.roots = alt_x509_system_roots())) {
		w = "x509: failed to load system roots and no roots provided";
		goto fail;
	}
	vctx v = {&o, a, o.now ? o.now : (int64_t)time(0), 0, 0, 0, 0};
	if ((w = is_valid(&v, leaf, LEAF, 0, 0)))
		goto fail;
	if (o.dns_name && o.dns_name[0] && alt_x509_verify_hostname(leaf, o.dns_name, a, &w) < 0)
		goto fail;
	const alt_x509_cert *chain[MAX_DEPTH + 1];
	chain[0] = leaf;
	if (pool_contains(o.roots, leaf))
		add_chain(&v, chain, 1);
	else if ((w = build(&v, chain, 1)))
		goto fail;
	bool any_usage = false;
	for (size_t i = 0; i < o.nkey_usages; i++)
		any_usage |= o.key_usages[i] == ALT_X509_EKU_ANY;
	static const int server_auth[1] = {ALT_X509_EKU_SERVER_AUTH};
	const int *usages = o.nkey_usages ? o.key_usages : server_auth;
	size_t nusages = o.nkey_usages ? o.nkey_usages : 1;
	size_t kept = 0, bad_usage = 0, bad_policies = 0;
	const char *constraint_err = 0;
	for (size_t i = 0; i < v.n; i++) {
		if (!policies_valid(a, v.chains[i].certs, v.chains[i].n, &o)) {
			bad_policies++;
			continue;
		}
		if (!any_usage && !chain_key_usage(&v.chains[i], usages, nusages)) {
			bad_usage++;
			continue;
		}
		const char *ce = chain_constraints(a, v.chains[i].certs, v.chains[i].n);
		if (ce) {
			if (!constraint_err)
				constraint_err = ce;
			continue;
		}
		v.chains[kept++] = v.chains[i];
	}
	if (kept == 0) {
		if (constraint_err)
			w = alt_arena_printf(a, "x509: a root or intermediate certificate is not authorized to sign for this name: %s", constraint_err);
		else if (bad_usage && !bad_policies)
			w = "x509: certificate specifies an incompatible key usage";
		else if (bad_usage)
			w = alt_arena_printf(a, "x509: no valid chains built: %zu candidate chains with incompatible key usage, %zu candidate chains with invalid policies",
				bad_usage, bad_policies);
		else if (bad_policies)
			w = alt_arena_printf(a, "x509: no valid chains built: %zu candidate chains with invalid policies", bad_policies);
		else
			w = "x509: no valid chains built";
		goto fail;
	}
	*chains = v.chains;
	*nchains = kept;
	return 0;
fail:
	if (why)
		*why = w;
	return -ALT_EX509_VERIFY;
}
