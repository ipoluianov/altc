/* parse hex -> name|value|quoted|path|domain|max_age|secure|httponly|samesite|has_expires|expires_unix or "err <msg>"
   jar op... -> after each "G url" the Cookie header ("S url hex,hex,..." stores cookies) */
#include "common.h"
#include <alt/net/cookiejar.h>

int main(void) {
	char *line, *f[512];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 512);
		alt_arena a;
		alt_arena_init(&a, 0);
		if (!strcmp(f[0], "parse")) {
			size_t k = diff_unhex(f[1]);
			alt_set_cookie c;
			const char *why = "";
			if (alt_http_parse_set_cookie(&a, alt_str_make(f[1], k), &c, &why) < 0)
				printf("err %s", why);
			else
				printf("%.*s|%.*s|%d|%.*s|%.*s|%d|%d|%d|%d|%d|%lld", (int)c.name.n, c.name.p, (int)c.value.n, c.value.p, c.quoted,
					(int)c.path.n, c.path.p, (int)c.domain.n, c.domain.p, c.max_age, c.secure, c.http_only, c.same_site,
					c.expires != 0, c.expires ? (long long)alt_wall_unix(c.expires) : 0LL);
		} else {
			alt_cookie_jar *j;
			alt_cookie_jar_new(&j);
			for (int i = 1; i < n; i++) {
				char *op = f[i];
				char *sp = strchr(op, ' ');
				if (!sp)
					continue;
				*sp = 0;
				char *url = sp + 1, *rest = strchr(url, ' ');
				if (rest)
					*rest++ = 0;
				alt_url u;
				if (alt_url_parse(&a, alt_str_cstr(url), &u) < 0) {
					printf("[badurl]");
					continue;
				}
				if (op[0] == 'S' && rest) {
					alt_set_cookie cs[64];
					size_t nc = 0;
					for (char *save = 0, *t = strtok_r(rest, ",", &save); t && nc < 64; t = strtok_r(0, ",", &save)) {
						size_t k = diff_unhex(t);
						if (alt_http_parse_set_cookie(&a, alt_str_make(t, k), &cs[nc], 0) == 0)
							nc++;
					}
					alt_cookie_jar_set(j, &u, cs, nc, 0);
				} else if (op[0] == 'G') {
					alt_buf b = ALT_BUF_INIT;
					alt_cookie_jar_header(j, &u, &b, 0);
					printf("[%s]", alt_buf_cstr(&b));
					alt_buf_free(&b);
				}
			}
			alt_cookie_jar_free(j);
		}
		printf("\n");
		fflush(stdout);
		alt_arena_free(&a);
	}
	return 0;
}
