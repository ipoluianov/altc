/* c pat -> "ok <nsub> <names,>" or the compile error
   a pat text [longest] -> all matches with submatches: "[s e s e ...]..."
   r pat text repl -> ReplaceAllString;  l pat text repl -> ReplaceAllLiteralString
   s pat text n -> Split ("|" separated, hex);  q text -> QuoteMeta
   (pat, text, repl in hex) */
#include "common.h"
#include <alt/regexp.h>

static void show(void *arg, const ssize_t *m) {
	size_t n = *(size_t *)arg;
	printf("[");
	for (size_t i = 0; i < n; i++)
		printf("%s%zd", i ? " " : "", m[i]);
	printf("]");
}

int main(void) {
	char *line, *f[8];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 8);
		alt_str pat = {0, 0};
		if (n > 1 && f[0][0] != 'q') {
			size_t k = diff_unhex(f[1]);
			pat = alt_str_make(f[1], k);
		}
		alt_regexp *re = 0;
		alt_buf err = ALT_BUF_INIT, b = ALT_BUF_INIT;
		if (f[0][0] == 'q') {
			size_t k = diff_unhex(f[1]);
			alt_regexp_quote_meta(&b, alt_str_make(f[1], k));
			diff_puthex(b.p ? b.p : "", b.n);
			printf("\n");
			alt_buf_free(&b);
			continue;
		}
		bool posix = f[0][0] == 'p';
		int rc = posix ? alt_regexp_compile_posix(&re, pat, &err) : alt_regexp_compile(&re, pat, &err);
		if (rc < 0) {
			printf("err %s\n", alt_buf_cstr(&err));
			alt_buf_free(&err);
			continue;
		}
		if (f[0][0] == 'c' || posix) {
			printf("ok %zu ", alt_regexp_num_subexp(re));
			for (size_t i = 0; i <= alt_regexp_num_subexp(re); i++) {
				alt_str nm = alt_regexp_subexp_name(re, i);
				printf("%.*s,", (int)nm.n, nm.p);
			}
		} else {
			size_t k = diff_unhex(f[2]);
			alt_str text = alt_str_make(f[2], k);
			if (f[0][0] == 'a') {
				if (n > 3 && f[3][0] == 'L')
					alt_regexp_longest(re);
				size_t nc = 2 * (alt_regexp_num_subexp(re) + 1);
				alt_regexp_find_all(re, text, -1, show, &nc);
				printf(" %d", alt_regexp_match(re, text));
			} else if (f[0][0] == 'r' || f[0][0] == 'l') {
				size_t kr = diff_unhex(f[3]);
				if (f[0][0] == 'r')
					alt_regexp_replace_all(re, &b, text, alt_str_make(f[3], kr));
				else
					alt_regexp_replace_all_literal(re, &b, text, alt_str_make(f[3], kr));
				diff_puthex(b.p ? b.p : "", b.n);
			} else if (f[0][0] == 's') {
				alt_arena a;
				alt_arena_init(&a, 0);
				alt_str *parts;
				size_t np;
				alt_regexp_split(re, &a, text, atol(f[3]), &parts, &np);
				printf("%zu:", np);
				for (size_t i = 0; i < np; i++) {
					printf("|");
					diff_puthex(parts[i].p ? parts[i].p : "", parts[i].n);
				}
				alt_arena_free(&a);
			}
		}
		printf("\n");
		fflush(stdout);
		alt_regexp_free(re);
		alt_buf_free(&err);
		alt_buf_free(&b);
	}
	return 0;
}
