/* P hex -> fields of alt_url_parse (hex) | err
   R hex -> the same with alt_url_parse_request_uri
   X basehex refhex -> resolved string (hex) | err
   Q hex -> encoded values, pairs, error flag
   E hex -> query escape, path escape, query unescape, path unescape */
#include "common.h"
#include <alt/net/url.h>

static void hexs(alt_str s) {
	diff_puthex(s.p, s.n);
	putchar('|');
}

static void show(const alt_url *u) {
	hexs(u->scheme);
	hexs(u->opaque);
	printf("%d%d|", u->has_user, u->has_password);
	hexs(u->username);
	hexs(u->password);
	hexs(u->host);
	hexs(u->path);
	hexs(u->raw_path);
	hexs(u->raw_query);
	hexs(u->fragment);
	hexs(u->raw_fragment);
	printf("%d%d|", u->force_query, u->omit_host);
	alt_buf b = ALT_BUF_INIT;
	alt_url_string(u, &b);
	hexs(alt_buf_str(&b));
	alt_buf_reset(&b);
	alt_url_escaped_path(u, &b);
	hexs(alt_buf_str(&b));
	alt_buf_reset(&b);
	alt_url_request_uri(u, &b);
	hexs(alt_buf_str(&b));
	hexs(alt_url_hostname(u));
	hexs(alt_url_port(u));
	alt_buf_free(&b);
	printf("\n");
}

int main(void) {
	char *line, *f[3];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 3);
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_str in = alt_str_make(f[1], diff_unhex(f[1]));
		alt_url u;
		switch (line[0]) {
		case 'P':
		case 'R':
			if ((line[0] == 'P' ? alt_url_parse(&a, in, &u) : alt_url_parse_request_uri(&a, in, &u)) < 0)
				printf("err\n");
			else
				show(&u);
			break;
		case 'X': {
			alt_url base, ref, out;
			alt_str rs = alt_str_make(n > 2 ? f[2] : "", n > 2 ? diff_unhex(f[2]) : 0);
			if (alt_url_parse(&a, in, &base) < 0 || alt_url_parse(&a, rs, &ref) < 0 || alt_url_resolve(&a, &base, &ref, &out) < 0)
				printf("err\n");
			else {
				alt_buf b = ALT_BUF_INIT;
				alt_url_string(&out, &b);
				diff_puthex(b.p, b.n);
				printf("\n");
				alt_buf_free(&b);
			}
			break;
		}
		case 'Q': {
			alt_url_values v;
			alt_url_values_init(&v, &a);
			int rc = alt_url_parse_query(&v, in);
			alt_buf b = ALT_BUF_INIT;
			alt_url_values_encode(&v, &b);
			hexs(alt_buf_str(&b));
			printf("%d|", rc < 0);
			alt_str first;
			if (v.n && alt_url_values_get(&v, v.kv[0].key, &first))
				hexs(first);
			printf("\n");
			alt_buf_free(&b);
			break;
		}
		case 'E': {
			alt_buf b = ALT_BUF_INIT;
			alt_url_query_escape(&b, in);
			hexs(alt_buf_str(&b));
			alt_buf_reset(&b);
			alt_url_path_escape(&b, in);
			hexs(alt_buf_str(&b));
			alt_str o;
			if (alt_url_query_unescape(&a, in, &o) < 0)
				printf("err|");
			else
				hexs(o);
			if (alt_url_path_unescape(&a, in, &o) < 0)
				printf("err|");
			else
				hexs(o);
			printf("\n");
			alt_buf_free(&b);
			break;
		}
		}
		alt_arena_free(&a);
	}
	return 0;
}
