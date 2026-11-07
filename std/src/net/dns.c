/* The resolver: /etc/hosts and DNS (RFC 1035) over UDP, TCP for truncated
   answers; the search list and options of resolv.conf as in Go's net
   (dnsclient_unix.go, dnsconfig_unix.go; BSD-3-Clause, see LICENSE-Go) */
#define _GNU_SOURCE
#include "net_internal.h"
#include <alt/errors.h>
#include <alt/net.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ---- names ---- */

/* Go's isDomainName: labels of letters, digits, '-' and '_' */
static bool valid_name(const char *s) {
	size_t n = strlen(s);
	if (n == 0 || n > 254 || (n == 254 && s[n - 1] != '.'))
		return false;
	int label = 0;
	char last = '.';
	bool non_numeric = false;
	for (size_t i = 0; i < n; i++) {
		char c = s[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
			non_numeric = true;
			label++;
		} else if (c >= '0' && c <= '9') {
			label++;
		} else if (c == '-') {
			if (last == '.')
				return false;
			label++;
			non_numeric = true;
		} else if (c == '.') {
			if (last == '.' || last == '-')
				return false;
			if (label > 63 || label == 0)
				return false;
			label = 0;
		} else
			return false;
		last = c;
	}
	if (last == '-' || label > 63)
		return false;
	return non_numeric;
}

static bool name_eq(const char *a, const char *b) {
	/* ignoring case and one trailing dot */
	size_t na = strlen(a), nb = strlen(b);
	if (na && a[na - 1] == '.')
		na--;
	if (nb && b[nb - 1] == '.')
		nb--;
	return na == nb && alt_str_eq_fold(alt_str_make(a, na), alt_str_make(b, nb));
}

/* ---- messages ---- */

static void put16(uint8_t *p, unsigned v) {
	p[0] = (uint8_t)(v >> 8);
	p[1] = (uint8_t)v;
}
static unsigned get16(const uint8_t *p) { return (unsigned)p[0] << 8 | p[1]; }

int alt__dns_query(uint8_t *buf, size_t cap, uint16_t id, const char *name, int qtype) {
	size_t n = strlen(name);
	if (cap < 12 + n + 2 + 4 + 11)
		return -ENOSPC;
	memset(buf, 0, 12);
	put16(buf, id);
	buf[2] = 0x01; /* RD */
	put16(buf + 4, 1);  /* QDCOUNT */
	put16(buf + 10, 1); /* ARCOUNT: the OPT record */
	size_t p = 12;
	const char *s = name;
	while (*s) {
		const char *dot = strchr(s, '.');
		size_t len = dot ? (size_t)(dot - s) : strlen(s);
		if (len == 0 || len > 63)
			return -ALT_ENET_NOHOST;
		buf[p++] = (uint8_t)len;
		memcpy(buf + p, s, len);
		p += len;
		s += len;
		if (*s == '.')
			s++;
	}
	buf[p++] = 0;
	put16(buf + p, (unsigned)qtype);
	put16(buf + p + 2, 1); /* IN */
	p += 4;
	/* EDNS0 (RFC 6891): a UDP payload of 1232 bytes */
	buf[p++] = 0;
	put16(buf + p, 41);
	put16(buf + p + 2, 1232);
	memset(buf + p + 4, 0, 6);
	p += 10;
	return (int)p;
}

/* reads a possibly compressed name at *off into out (dotted, no
   trailing dot); pointers must go backwards */
static bool read_name(const uint8_t *msg, size_t len, size_t *off, char *out, size_t cap) {
	size_t p = *off, o = 0;
	bool jumped = false;
	int hops = 0;
	for (;;) {
		if (p >= len)
			return false;
		uint8_t l = msg[p];
		if (l == 0) {
			p++;
			break;
		}
		if ((l & 0xC0) == 0xC0) {
			if (p + 1 >= len || ++hops > 64)
				return false;
			size_t target = (size_t)(l & 0x3F) << 8 | msg[p + 1];
			if (target >= p)
				return false;
			if (!jumped)
				*off = p + 2;
			jumped = true;
			p = target;
			continue;
		}
		if (l & 0xC0)
			return false;
		if (p + 1 + l > len || o + l + 2 > cap)
			return false;
		if (o)
			out[o++] = '.';
		memcpy(out + o, msg + p + 1, l);
		o += l;
		p += 1 + (size_t)l;
	}
	out[o] = 0;
	if (!jumped)
		*off = p;
	return true;
}

int alt__dns_answer(const uint8_t *msg, size_t len, uint16_t id, const char *name, int qtype, alt_ip *out, int n, int max,
	int *rcode, bool *truncated) {
	if (len < 12 || get16(msg) != id || !(msg[2] & 0x80))
		return -ALT_ENET_DNSFORMAT;
	*truncated = msg[2] & 0x02;
	*rcode = msg[3] & 0x0F;
	unsigned qd = get16(msg + 4), an = get16(msg + 6);
	if (qd != 1)
		return -ALT_ENET_DNSFORMAT;
	size_t off = 12;
	char qname[256];
	if (!read_name(msg, len, &off, qname, sizeof qname) || off + 4 > len)
		return -ALT_ENET_DNSFORMAT;
	if (!name_eq(qname, name) || (int)get16(msg + off) != qtype)
		return -ALT_ENET_DNSFORMAT;
	off += 4;
	/* the answers: follow CNAMEs from the name asked for */
	typedef struct {
		char owner[256];
		int type;
		size_t rdoff, rdlen;
	} rr;
	enum { MAXRR = 64 };
	rr *rrs = malloc(MAXRR * sizeof *rrs);
	if (!rrs)
		return -ENOMEM;
	int nrr = 0;
	for (unsigned i = 0; i < an; i++) {
		char owner[256];
		if (!read_name(msg, len, &off, owner, sizeof owner) || off + 10 > len) {
			free(rrs);
			return -ALT_ENET_DNSFORMAT;
		}
		int type = (int)get16(msg + off), class = (int)get16(msg + off + 2);
		size_t rdlen = get16(msg + off + 8);
		off += 10;
		if (off + rdlen > len) {
			free(rrs);
			return -ALT_ENET_DNSFORMAT;
		}
		if (class == 1 && nrr < MAXRR && (type == ALT__DNS_A || type == ALT__DNS_AAAA || type == ALT__DNS_CNAME)) {
			strcpy(rrs[nrr].owner, owner);
			rrs[nrr].type = type;
			rrs[nrr].rdoff = off;
			rrs[nrr].rdlen = rdlen;
			nrr++;
		}
		off += rdlen;
	}
	char target[256];
	strcpy(target, qname);
	for (int hop = 0; hop < 16; hop++) {
		bool moved = false;
		for (int i = 0; i < nrr; i++)
			if (rrs[i].type == ALT__DNS_CNAME && name_eq(rrs[i].owner, target)) {
				size_t o = rrs[i].rdoff;
				char cn[256];
				if (!read_name(msg, len, &o, cn, sizeof cn)) {
					free(rrs);
					return -ALT_ENET_DNSFORMAT;
				}
				if (!name_eq(cn, target)) {
					strcpy(target, cn);
					moved = true;
				}
				break;
			}
		if (!moved)
			break;
	}
	for (int i = 0; i < nrr && n < max; i++) {
		if (rrs[i].type != qtype || !name_eq(rrs[i].owner, target))
			continue;
		const uint8_t *d = msg + rrs[i].rdoff;
		if (qtype == ALT__DNS_A && rrs[i].rdlen == 4)
			out[n++] = alt_ip_v4(d[0], d[1], d[2], d[3]);
		else if (qtype == ALT__DNS_AAAA && rrs[i].rdlen == 16) {
			alt_ip ip;
			memset(&ip, 0, sizeof ip);
			memcpy(ip.b, d, 16);
			ip.valid = true;
			out[n++] = ip;
		}
	}
	free(rrs);
	return n;
}

/* ---- configuration ---- */

static void default_servers(alt_resolver *r) {
	alt_ip lo4 = alt_ip_v4(127, 0, 0, 1), lo6;
	alt_ip_parse(ALT_STR("::1"), &lo6);
	r->servers[0] = alt_addr_from_ip(&lo4, 53);
	r->servers[1] = alt_addr_from_ip(&lo6, 53);
	r->nservers = 2;
}

int alt_resolver_load(alt_resolver *r, const char *path) {
	memset(r, 0, sizeof *r);
	r->ndots = 1;
	r->timeout = 5 * ALT_SEC;
	r->attempts = 2;
	FILE *f = fopen(path ? path : "/etc/resolv.conf", "re");
	if (f) {
		char line[1024];
		while (fgets(line, sizeof line, f)) {
			if (line[0] == '#' || line[0] == ';')
				continue;
			alt_str rest = alt_str_cstr(line), key, v;
			if (!alt_str_fields_next(&rest, &key))
				continue;
			if (alt_str_eq_cstr(key, "nameserver")) {
				alt_ip ip;
				if (alt_str_fields_next(&rest, &v) && alt_ip_parse(v, &ip) == 0 && r->nservers < 8)
					r->servers[r->nservers++] = alt_addr_from_ip(&ip, 53);
			} else if (alt_str_eq_cstr(key, "domain") || alt_str_eq_cstr(key, "search")) {
				r->nsearch = 0;
				while (alt_str_fields_next(&rest, &v) && r->nsearch < 6) {
					if (v.n >= sizeof r->search[0] - 1)
						continue;
					memcpy(r->search[r->nsearch], v.p, v.n);
					r->search[r->nsearch][v.n] = 0;
					r->nsearch++;
				}
			} else if (alt_str_eq_cstr(key, "options")) {
				while (alt_str_fields_next(&rest, &v)) {
					long long x;
					if (alt_str_has_prefix(v, ALT_STR("ndots:")) && alt_str_to_int(alt_str_from(v, 6), 10, &x) == 0)
						r->ndots = x < 0 ? 0 : x > 15 ? 15 : (int)x;
					else if (alt_str_has_prefix(v, ALT_STR("timeout:")) && alt_str_to_int(alt_str_from(v, 8), 10, &x) == 0)
						r->timeout = (x < 1 ? 1 : x) * ALT_SEC;
					else if (alt_str_has_prefix(v, ALT_STR("attempts:")) && alt_str_to_int(alt_str_from(v, 9), 10, &x) == 0)
						r->attempts = x < 1 ? 1 : x > 5 ? 5 : (int)x;
					else if (alt_str_eq_cstr(v, "rotate"))
						r->rotate = true;
				}
			}
		}
		fclose(f);
	}
	if (r->nservers == 0)
		default_servers(r);
	if (r->nsearch == 0) {
		/* the domain of the host name */
		char host[256];
		if (gethostname(host, sizeof host) == 0) {
			host[sizeof host - 1] = 0;
			char *dot = strchr(host, '.');
			if (dot && dot[1] && strlen(dot + 1) < sizeof r->search[0]) {
				strcpy(r->search[0], dot + 1);
				r->nsearch = 1;
			}
		}
	}
	return f ? 0 : -errno;
}

/* ---- /etc/hosts ---- */

static int hosts_lookup(const char *path, const char *name, alt_ip *out, int max) {
	FILE *f = fopen(path, "re");
	if (!f)
		return 0;
	char line[1024];
	int n = 0;
	while (n < max && fgets(line, sizeof line, f)) {
		char *hash = strchr(line, '#');
		if (hash)
			*hash = 0;
		alt_str rest = alt_str_cstr(line), addr, h;
		if (!alt_str_fields_next(&rest, &addr))
			continue;
		alt_ip ip;
		if (alt_ip_parse(addr, &ip) < 0)
			continue;
		while (alt_str_fields_next(&rest, &h)) {
			char hn[256];
			if (h.n >= sizeof hn)
				continue;
			memcpy(hn, h.p, h.n);
			hn[h.n] = 0;
			if (name_eq(hn, name)) {
				bool dup = false;
				for (int i = 0; i < n; i++)
					dup |= alt_ip_equal(&out[i], &ip);
				if (!dup)
					out[n++] = ip;
				break;
			}
		}
	}
	fclose(f);
	return n;
}

/* ---- queries ---- */

static uint16_t next_id(void) {
	static _Atomic unsigned seed;
	unsigned s = atomic_load(&seed);
	if (s == 0) {
		uint64_t t = (uint64_t)alt_now() ^ (uint64_t)getpid() << 32;
		s = (unsigned)(t ^ t >> 29) | 1;
	}
	s ^= s << 13;
	s ^= s >> 17;
	s ^= s << 5;
	atomic_store(&seed, s);
	return (uint16_t)s;
}

/* one exchange with one server: the number of addresses added, or <0;
   *rcode the server's answer code */
static int exchange(const alt_addr *server, alt_ctx *ctx, alt_dur timeout, const char *name, int qtype, alt_ip *out, int n, int max,
	int *rcode) {
	uint8_t q[512];
	uint16_t id = next_id();
	int qlen = alt__dns_query(q, sizeof q, id, name, qtype);
	if (qlen < 0)
		return qlen;
	alt_time deadline = alt_time_min(alt_deadline(timeout), alt_ctx_deadline(ctx));
	alt_conn *c;
	int rc = alt__net_dial_addr(&c, SOCK_DGRAM, server, deadline, ctx);
	if (rc < 0)
		return rc;
	alt_conn_set_deadline(c, deadline);
	uint8_t ans[1500];
	bool truncated = false;
	rc = (int)alt_conn_write(c, q, (size_t)qlen);
	while (rc >= 0) {
		ssize_t k = alt_conn_read(c, ans, sizeof ans);
		if (k < 0) {
			rc = (int)k;
			break;
		}
		rc = alt__dns_answer(ans, (size_t)k, id, name, qtype, out, n, max, rcode, &truncated);
		if (rc == -ALT_ENET_DNSFORMAT) {
			rc = 0; /* not our answer: keep waiting */
			continue;
		}
		break;
	}
	alt_conn_close(c);
	if (rc < 0 || !truncated)
		return rc < 0 ? rc : rc - n;
	/* truncated: the whole answer over TCP */
	rc = alt__net_dial_addr(&c, SOCK_STREAM, server, deadline, ctx);
	if (rc < 0)
		return rc;
	alt_conn_set_deadline(c, deadline);
	uint8_t pre[2];
	put16(pre, (unsigned)qlen);
	uint8_t *big = 0;
	if (alt_conn_write(c, pre, 2) < 0 || alt_conn_write(c, q, (size_t)qlen) < 0)
		rc = -EIO;
	else if (alt_read_full(alt_conn_reader(c), pre, 2) != 2)
		rc = -ALT_ENET_DNSFORMAT;
	else {
		size_t l = get16(pre);
		big = malloc(l ? l : 1);
		if (!big)
			rc = -ENOMEM;
		else if ((size_t)alt_read_full(alt_conn_reader(c), big, l) != l)
			rc = -ALT_ENET_DNSFORMAT;
		else
			rc = alt__dns_answer(big, l, id, name, qtype, out, n, max, rcode, &truncated);
	}
	free(big);
	alt_conn_close(c);
	return rc < 0 ? rc : rc - n;
}

enum { RCODE_OK = 0, RCODE_SERVFAIL = 2, RCODE_NXDOMAIN = 3 };

/* the addresses of one fully qualified name: >= 0, -ALT_ENET_NOHOST
   when it does not exist, other errors when the servers failed */
static int lookup_fqdn(const alt_resolver *r, alt_ctx *ctx, const char *name, alt_ip *out, int max) {
	int n = 0, err = -ALT_ENET_DNS;
	bool exists = false;
	static _Atomic unsigned rot;
	unsigned start = r->rotate ? atomic_fetch_add(&rot, 1) : 0;
	for (int t = 0; t < 2; t++) {
		int qtype = t == 0 ? ALT__DNS_A : ALT__DNS_AAAA;
		bool answered = false;
		for (int a = 0; a < r->attempts && !answered; a++)
			for (int s = 0; s < r->nservers && !answered; s++) {
				int e = alt_ctx_err(ctx);
				if (e)
					return e;
				int rcode = 0;
				int k = exchange(&r->servers[(start + (unsigned)s) % (unsigned)r->nservers], ctx, r->timeout, name, qtype, out, n,
					max, &rcode);
				if (k < 0) {
					err = k == -ECONNREFUSED ? -ALT_ENET_DNS : k;
					continue;
				}
				if (rcode == RCODE_NXDOMAIN) {
					/* the name does not exist: no other type exists either */
					return n ? n : -ALT_ENET_NOHOST;
				}
				if (rcode != RCODE_OK) {
					err = -ALT_ENET_DNS;
					continue;
				}
				n += k;
				answered = exists = true;
			}
	}
	if (n)
		return n;
	return exists ? -ALT_ENET_NOHOST : err;
}

int alt_resolver_lookup(const alt_resolver *r, alt_ctx *ctx, const char *host, alt_ip *out, int max) {
	if (max <= 0)
		return -EINVAL;
	alt_ip ip;
	if (alt_ip_parse(alt_str_cstr(host), &ip) == 0) {
		out[0] = ip;
		return 1;
	}
	if (!valid_name(host))
		return -ALT_ENET_NOHOST;
	const char *hosts = r->hosts ? r->hosts : "/etc/hosts";
	if (hosts[0]) {
		int n = hosts_lookup(hosts, host, out, max);
		if (n > 0)
			return n;
	}
	/* RFC 6761: localhost and its subdomains are the loopback addresses */
	size_t hl = strlen(host);
	if (name_eq(host, "localhost") || (hl > 10 && alt_str_has_suffix(alt_str_make(host, hl - (host[hl - 1] == '.')), ALT_STR(".localhost")))) {
		out[0] = alt_ip_v4(127, 0, 0, 1);
		if (max > 1) {
			alt_ip_parse(ALT_STR("::1"), &out[1]);
			return 2;
		}
		return 1;
	}
	/* the names to try (Go's nameList) */
	char names[8][256];
	int nn = 0;
	bool rooted = host[hl - 1] == '.';
	if (rooted) {
		snprintf(names[nn++], 256, "%.*s", (int)(hl - 1), host);
	} else {
		int dots = 0;
		for (size_t i = 0; i < hl; i++)
			dots += host[i] == '.';
		bool first = dots >= r->ndots;
		if (first)
			snprintf(names[nn++], 256, "%s", host);
		for (int i = 0; i < r->nsearch && nn < 7; i++)
			if (hl + 1 + strlen(r->search[i]) < 255)
				snprintf(names[nn++], 256, "%s.%s", host, r->search[i]);
		if (!first)
			snprintf(names[nn++], 256, "%s", host);
	}
	int err = -ALT_ENET_NOHOST;
	for (int i = 0; i < nn; i++) {
		int n = lookup_fqdn(r, ctx, names[i], out, max);
		if (n > 0)
			return n;
		if (n == -ECANCELED || (n == -ETIMEDOUT && alt_ctx_err(ctx)))
			return n;
		if (n != -ALT_ENET_NOHOST)
			err = n; /* a failure beats "no such host" */
	}
	return err;
}

int alt_lookup_host(alt_ctx *ctx, const char *host, alt_ip *out, int max) {
	alt_resolver r;
	alt_resolver_load(&r, 0);
	return alt_resolver_lookup(&r, ctx, host, out, max);
}
