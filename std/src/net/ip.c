/* IP addresses, socket addresses, host:port, after Go's net/netip and
   net (BSD-3-Clause, see LICENSE-Go) */
#define _GNU_SOURCE
#include <alt/errors.h>
#include <alt/net.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>

alt_ip alt_ip_v4(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	alt_ip ip = {{0}, true, true, ""};
	ip.b[10] = ip.b[11] = 0xff;
	ip.b[12] = a, ip.b[13] = b, ip.b[14] = c, ip.b[15] = d;
	return ip;
}

/* four decimal fields without leading zeros */
static bool parse4(alt_str s, uint8_t out[4]) {
	int field = 0, val = 0, digits = 0;
	for (size_t i = 0; i < s.n; i++) {
		char c = s.p[i];
		if (c >= '0' && c <= '9') {
			if (digits == 1 && val == 0)
				return false; /* a leading zero */
			val = val * 10 + (c - '0');
			digits++;
			if (val > 255)
				return false;
		} else if (c == '.') {
			if (digits == 0 || field == 3)
				return false;
			out[field++] = (uint8_t)val;
			val = 0, digits = 0;
		} else
			return false;
	}
	if (field != 3 || digits == 0)
		return false;
	out[3] = (uint8_t)val;
	return true;
}

static int hexv(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static bool parse6(alt_str s, uint8_t ip[16]) {
	int ellipsis = -1, i = 0;
	size_t p = 0;
	if (s.n >= 2 && s.p[0] == ':' && s.p[1] == ':') {
		ellipsis = 0;
		p = 2;
		if (p == s.n) {
			memset(ip, 0, 16);
			return true;
		}
	}
	while (i < 16) {
		/* a hexadecimal group */
		int v = 0, k = 0;
		while (p + (size_t)k < s.n && k < 5) {
			int h = hexv(s.p[p + (size_t)k]);
			if (h < 0)
				break;
			v = v * 16 + h;
			k++;
		}
		if (k == 0 || k > 4)
			return false;
		/* or an embedded IPv4 address at the end */
		if (p + (size_t)k < s.n && s.p[p + (size_t)k] == '.') {
			if (ellipsis < 0 && i != 12)
				return false;
			if (i + 4 > 16)
				return false;
			uint8_t v4[4];
			if (!parse4(alt_str_from(s, p), v4))
				return false;
			memcpy(ip + i, v4, 4);
			i += 4;
			p = s.n;
			break;
		}
		ip[i] = (uint8_t)(v >> 8);
		ip[i + 1] = (uint8_t)v;
		i += 2;
		p += (size_t)k;
		if (p == s.n)
			break;
		if (s.p[p] != ':' || p + 1 == s.n)
			return false;
		p++;
		if (s.p[p] == ':') {
			if (ellipsis >= 0)
				return false;
			ellipsis = i;
			p++;
			if (p == s.n)
				break;
		}
	}
	if (p != s.n)
		return false;
	if (i < 16) {
		if (ellipsis < 0)
			return false;
		int n = 16 - i;
		memmove(ip + ellipsis + n, ip + ellipsis, (size_t)(i - ellipsis));
		memset(ip + ellipsis, 0, (size_t)n);
	} else if (ellipsis >= 0)
		return false; /* "::" must stand for at least one group */
	return true;
}

int alt_ip_parse(alt_str s, alt_ip *ip) {
	memset(ip, 0, sizeof *ip);
	for (size_t i = 0; i < s.n; i++) {
		if (s.p[i] == '.') {
			uint8_t v[4];
			if (!parse4(s, v))
				return -ALT_ENET_ADDR;
			*ip = alt_ip_v4(v[0], v[1], v[2], v[3]);
			return 0;
		}
		if (s.p[i] == ':') {
			alt_str addr = s, zone = ALT_STR_NULL;
			ssize_t pc = alt_str_index_byte(s, '%');
			if (pc >= 0) {
				addr = alt_str_slice(s, 0, (size_t)pc);
				zone = alt_str_from(s, (size_t)pc + 1);
				if (zone.n == 0 || zone.n >= sizeof ip->zone)
					return -ALT_ENET_ADDR;
			}
			if (!parse6(addr, ip->b))
				return -ALT_ENET_ADDR;
			memcpy(ip->zone, zone.p ? zone.p : "", zone.n);
			ip->zone[zone.n] = 0;
			ip->valid = true;
			return 0;
		}
	}
	return -ALT_ENET_ADDR;
}

static bool mapped(const alt_ip *ip) {
	static const uint8_t prefix[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
	return memcmp(ip->b, prefix, 12) == 0;
}

int alt_ip_format(const alt_ip *ip, alt_buf *b) {
	if (!ip->valid)
		return alt_buf_append_cstr(b, "invalid IP");
	if (ip->is4)
		return alt_buf_printf(b, "%d.%d.%d.%d", ip->b[12], ip->b[13], ip->b[14], ip->b[15]);
	if (mapped(ip)) {
		alt_buf_printf(b, "::ffff:%d.%d.%d.%d", ip->b[12], ip->b[13], ip->b[14], ip->b[15]);
	} else {
		/* the longest run of at least two zero groups becomes "::" (the
		   first of equal ones) */
		int best = -1, bestlen = 0;
		for (int i = 0; i < 8;) {
			if (ip->b[2 * i] == 0 && ip->b[2 * i + 1] == 0) {
				int j = i;
				while (j < 8 && ip->b[2 * j] == 0 && ip->b[2 * j + 1] == 0)
					j++;
				if (j - i >= 2 && j - i > bestlen)
					best = i, bestlen = j - i;
				i = j;
			} else
				i++;
		}
		for (int i = 0; i < 8; i++) {
			if (i == best) {
				alt_buf_append_cstr(b, "::");
				i += bestlen - 1;
				continue;
			}
			if (i > 0 && i != best + bestlen)
				alt_buf_append_byte(b, ':');
			alt_buf_printf(b, "%x", ip->b[2 * i] << 8 | ip->b[2 * i + 1]);
		}
	}
	if (ip->zone[0]) {
		alt_buf_append_byte(b, '%');
		alt_buf_append_cstr(b, ip->zone);
	}
	return b->err;
}

bool alt_ip_equal(const alt_ip *a, const alt_ip *b) {
	return a->valid == b->valid && a->is4 == b->is4 && memcmp(a->b, b->b, 16) == 0 && strcmp(a->zone, b->zone) == 0;
}

bool alt_ip_is_loopback(const alt_ip *ip) {
	if (ip->is4 || mapped(ip)) /* IPv4-mapped addresses as IPv4, as in Go */
		return ip->b[12] == 127;
	static const uint8_t one[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	return memcmp(ip->b, one, 16) == 0;
}

bool alt_ip_is_unspecified(const alt_ip *ip) {
	if (ip->is4)
		return !ip->b[12] && !ip->b[13] && !ip->b[14] && !ip->b[15];
	static const uint8_t zero[16];
	return memcmp(ip->b, zero, 16) == 0;
}

bool alt_ip_is_private(const alt_ip *ip) {
	if (ip->is4 || mapped(ip))
		return ip->b[12] == 10 || (ip->b[12] == 172 && (ip->b[13] & 0xf0) == 16) || (ip->b[12] == 192 && ip->b[13] == 168);
	return (ip->b[0] & 0xfe) == 0xfc;
}

/* ---- socket addresses ---- */

alt_addr alt_addr_from_ip(const alt_ip *ip, uint16_t port) {
	alt_addr a;
	memset(&a, 0, sizeof a);
	if (ip->is4) {
		struct sockaddr_in *s = (struct sockaddr_in *)&a.ss;
		s->sin_family = AF_INET;
		s->sin_port = htons(port);
		memcpy(&s->sin_addr, ip->b + 12, 4);
		a.len = sizeof *s;
	} else {
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&a.ss;
		s->sin6_family = AF_INET6;
		s->sin6_port = htons(port);
		memcpy(&s->sin6_addr, ip->b, 16);
		if (ip->zone[0]) {
			unsigned idx = if_nametoindex(ip->zone);
			if (!idx) {
				unsigned long long z;
				if (alt_str_to_uint(alt_str_cstr(ip->zone), 10, &z) == 0)
					idx = (unsigned)z;
			}
			s->sin6_scope_id = idx;
		}
		a.len = sizeof *s;
	}
	return a;
}

bool alt_addr_ip(const alt_addr *a, alt_ip *ip, uint16_t *port) {
	memset(ip, 0, sizeof *ip);
	if (a->ss.ss_family == AF_INET) {
		const struct sockaddr_in *s = (const struct sockaddr_in *)&a->ss;
		const uint8_t *v = (const uint8_t *)&s->sin_addr;
		*ip = alt_ip_v4(v[0], v[1], v[2], v[3]);
		if (port)
			*port = ntohs(s->sin_port);
		return true;
	}
	if (a->ss.ss_family == AF_INET6) {
		const struct sockaddr_in6 *s = (const struct sockaddr_in6 *)&a->ss;
		memcpy(ip->b, &s->sin6_addr, 16);
		ip->valid = true;
		if (s->sin6_scope_id) {
			char name[IF_NAMESIZE];
			if (if_indextoname(s->sin6_scope_id, name) && strlen(name) < sizeof ip->zone)
				strcpy(ip->zone, name);
			else
				snprintf(ip->zone, sizeof ip->zone, "%u", s->sin6_scope_id);
		}
		if (port)
			*port = ntohs(s->sin6_port);
		return true;
	}
	return false;
}

int alt_addr_format(const alt_addr *a, alt_buf *b) {
	alt_ip ip;
	uint16_t port;
	if (alt_addr_ip(a, &ip, &port)) {
		/* IPv4-mapped IPv6 sockets addresses print as IPv4, as in Go */
		if (!ip.is4 && mapped(&ip) && !ip.zone[0])
			ip.is4 = true;
		if (!ip.is4)
			alt_buf_append_byte(b, '[');
		alt_ip_format(&ip, b);
		if (!ip.is4)
			alt_buf_append_byte(b, ']');
		return alt_buf_printf(b, ":%u", port);
	}
	if (a->ss.ss_family == AF_UNIX) {
		const struct sockaddr_un *u = (const struct sockaddr_un *)&a->ss;
		size_t n = a->len > offsetof(struct sockaddr_un, sun_path) ? a->len - offsetof(struct sockaddr_un, sun_path) : 0;
		if (n && u->sun_path[0] == 0) {
			alt_buf_append_byte(b, '@');
			return alt_buf_append(b, u->sun_path + 1, n - 1);
		}
		return alt_buf_append(b, u->sun_path, strnlen(u->sun_path, n));
	}
	return alt_buf_append_cstr(b, "?");
}

/* ---- host:port ---- */

int alt_split_host_port(alt_str hp, alt_str *host, alt_str *port) {
	ssize_t i = alt_str_last_index_byte(hp, ':');
	size_t j = 0, k = 0;
	if (i < 0)
		return -ALT_ENET_ADDR; /* missing port */
	if (hp.n && hp.p[0] == '[') {
		ssize_t end = alt_str_index_byte(hp, ']');
		if (end < 0)
			return -ALT_ENET_ADDR;
		if ((size_t)end + 1 == hp.n || (size_t)end + 1 != (size_t)i)
			return -ALT_ENET_ADDR;
		*host = alt_str_slice(hp, 1, (size_t)end);
		j = 1, k = (size_t)end + 1;
	} else {
		*host = alt_str_slice(hp, 0, (size_t)i);
		if (alt_str_index_byte(*host, ':') >= 0)
			return -ALT_ENET_ADDR; /* too many colons */
	}
	if (alt_str_index_byte(alt_str_from(hp, j), '[') >= 0 || alt_str_index_byte(alt_str_from(hp, k), ']') >= 0)
		return -ALT_ENET_ADDR;
	*port = alt_str_from(hp, (size_t)i + 1);
	return 0;
}

int alt_join_host_port(alt_buf *b, alt_str host, alt_str port) {
	bool v6 = alt_str_index_byte(host, ':') >= 0;
	if (v6)
		alt_buf_append_byte(b, '[');
	alt_buf_append_str(b, host);
	if (v6)
		alt_buf_append_byte(b, ']');
	alt_buf_append_byte(b, ':');
	return alt_buf_append_str(b, port);
}

static const struct {
	const char *name;
	uint16_t port;
} services[] = {
	{"ftp", 21}, {"ssh", 22}, {"telnet", 23}, {"smtp", 25}, {"domain", 53}, {"gopher", 70}, {"finger", 79}, {"http", 80},
	{"www", 80}, {"pop3", 110}, {"ntp", 123}, {"imap", 143}, {"snmp", 161}, {"ldap", 389}, {"https", 443}, {"submission", 587},
	{"ldaps", 636}, {"imaps", 993}, {"pop3s", 995}, {"socks", 1080}, {"mysql", 3306}, {"postgresql", 5432}, {"redis", 6379},
	{"http-alt", 8080},
};

int alt_lookup_port(const char *network, alt_str service, uint16_t *port) {
	(void)network;
	unsigned long long v;
	if (service.n && service.n <= 5 && alt_str_to_uint(service, 10, &v) == 0) {
		if (v > 65535)
			return -ALT_ENET_ADDR;
		*port = (uint16_t)v;
		return 0;
	}
	if (service.n == 0) {
		*port = 0;
		return 0;
	}
	for (size_t i = 0; i < sizeof services / sizeof *services; i++)
		if (alt_str_eq_fold(service, alt_str_cstr(services[i].name))) {
			*port = services[i].port;
			return 0;
		}
	/* /etc/services: "name port/proto aliases..." */
	FILE *f = fopen("/etc/services", "re");
	if (!f)
		return -ALT_ENET_ADDR;
	char line[512];
	int rc = -ALT_ENET_ADDR;
	while (rc != 0 && fgets(line, sizeof line, f)) {
		char *hash = strchr(line, '#');
		if (hash)
			*hash = 0;
		alt_str rest = alt_str_cstr(line), name, pp, alias;
		if (!alt_str_fields_next(&rest, &name) || !alt_str_fields_next(&rest, &pp))
			continue;
		bool match = alt_str_eq(name, service);
		while (!match && alt_str_fields_next(&rest, &alias))
			match = alt_str_eq(alias, service);
		alt_str num, proto;
		if (match && alt_str_cut_byte(pp, '/', &num, &proto) && alt_str_to_uint(num, 10, &v) == 0 && v <= 65535) {
			*port = (uint16_t)v;
			rc = 0;
		}
	}
	fclose(f);
	return rc;
}
