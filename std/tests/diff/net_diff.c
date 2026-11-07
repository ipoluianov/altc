/* I ip -> formatted | err (and flags: loopback private unspecified)
   S hostport -> host|port | err
   J host port -> joined */
#include "common.h"
#include <alt/net.h>

int main(void) {
	char *line, *f[3];
	alt_buf b = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 3);
		alt_buf_reset(&b);
		if (line[0] == 'I') {
			alt_ip ip;
			if (alt_ip_parse(alt_str_cstr(n > 1 ? f[1] : ""), &ip) < 0)
				printf("err\n");
			else {
				alt_ip_format(&ip, &b);
				printf("%s %d%d%d\n", alt_buf_cstr(&b), alt_ip_is_loopback(&ip), alt_ip_is_private(&ip), alt_ip_is_unspecified(&ip));
			}
		} else if (line[0] == 'S') {
			alt_str h, p;
			if (alt_split_host_port(alt_str_cstr(n > 1 ? f[1] : ""), &h, &p) < 0)
				printf("err\n");
			else
				printf("%.*s|%.*s\n", (int)h.n, h.p, (int)p.n, p.p);
		} else if (line[0] == 'J') {
			alt_join_host_port(&b, alt_str_cstr(f[1]), alt_str_cstr(n > 2 ? f[2] : ""));
			printf("%s\n", alt_buf_cstr(&b));
		}
	}
	alt_buf_free(&b);
	return 0;
}
