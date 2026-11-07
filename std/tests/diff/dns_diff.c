/* resolves names with the servers given as arguments:
   name search ndots timeout_ms attempts -> addresses | err <message> */
#include "common.h"
#include <alt/net.h>

int main(int argc, char **argv) {
	alt_resolver r;
	memset(&r, 0, sizeof r);
	for (int i = 1; i < argc && r.nservers < 8; i++) {
		alt_str h, p;
		alt_ip ip;
		unsigned long long port;
		if (alt_split_host_port(alt_str_cstr(argv[i]), &h, &p) || alt_ip_parse(h, &ip) || alt_str_to_uint(p, 10, &port))
			return 2;
		r.servers[r.nservers++] = alt_addr_from_ip(&ip, (uint16_t)port);
	}
	r.hosts = "";
	char *line, *f[5];
	alt_buf b = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 5);
		if (n < 5)
			return 3;
		r.nsearch = 0;
		if (f[1][0]) {
			snprintf(r.search[0], sizeof r.search[0], "%s", f[1]);
			r.nsearch = 1;
		}
		r.ndots = atoi(f[2]);
		r.timeout = atoi(f[3]) * ALT_MS;
		r.attempts = atoi(f[4]);
		alt_ip out[32];
		int k = alt_resolver_lookup(&r, 0, f[0], out, 32);
		if (k < 0) {
			printf("err %s\n", alt_strerror(k));
			continue;
		}
		alt_buf_reset(&b);
		for (int i = 0; i < k; i++) {
			if (i)
				alt_buf_append_byte(&b, ' ');
			alt_ip_format(&out[i], &b);
		}
		printf("%s\n", alt_buf_cstr(&b));
	}
	alt_buf_free(&b);
	return 0;
}
