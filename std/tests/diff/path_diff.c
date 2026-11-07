/* lex p -> Clean|Base|Dir|Ext|IsAbs|IsLocal|Split dir|Split file
   join a b ... -> Join;  match pat name -> 1/0/bad;  rel base targ -> result or "err <msg>"
   glob pat -> the names;  walk root [skip-name] -> the visited paths ("S" marks SkipDir at that name)
   (all fields in hex except numbers) */
#include "common.h"
#include <alt/path.h>

static alt_str arg(char *s) { return alt_str_make(s, diff_unhex(s)); }

typedef struct {
	alt_str skip;
	int all;
} wa;

static int visit(void *a, alt_str path, const alt_dir_entry *d, int err) {
	wa *w = a;
	printf("%.*s%s%s;", (int)path.n, path.p, d && d->is_dir ? "/" : "", err ? "!" : "");
	if (d && w->skip.n && alt_str_eq(d->name, w->skip))
		return w->all ? ALT_PATH_SKIP_ALL : ALT_PATH_SKIP_DIR;
	return 0;
}

int main(void) {
	char *line, *f[64];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 64);
		alt_buf b = ALT_BUF_INIT;
		if (!strcmp(f[0], "lex")) {
			alt_str p = arg(f[1]), d, fl;
			alt_path_clean(&b, p);
			printf("%s|%.*s|", alt_buf_cstr(&b), (int)alt_path_base(p).n, alt_path_base(p).p);
			alt_buf_reset(&b);
			alt_path_dir(&b, p);
			alt_str e = alt_path_ext(p);
			alt_path_split(p, &d, &fl);
			printf("%s|%.*s|%d|%d|%.*s|%.*s", alt_buf_cstr(&b), (int)e.n, e.p, alt_path_is_abs(p), alt_path_is_local(p), (int)d.n, d.p,
				(int)fl.n, fl.p);
		} else if (!strcmp(f[0], "join")) {
			alt_str parts[64];
			for (int i = 1; i < n; i++)
				parts[i - 1] = arg(f[i]);
			alt_path_join(&b, parts, (size_t)(n - 1));
			printf("[%s]", alt_buf_cstr(&b));
		} else if (!strcmp(f[0], "match")) {
			alt_str pat = arg(f[1]), name = arg(f[2]);
			int rc = alt_path_match(pat, name);
			printf("%s", rc < 0 ? "bad" : rc ? "1" : "0");
		} else if (!strcmp(f[0], "rel")) {
			alt_str base = arg(f[1]), targ = arg(f[2]);
			const char *why = "";
			if (alt_path_rel(&b, base, targ, &why) < 0)
				printf("err %s", why);
			else
				printf("%s", alt_buf_cstr(&b));
		} else if (!strcmp(f[0], "glob")) {
			alt_arena a;
			alt_arena_init(&a, 0);
			alt_str *names;
			size_t nn;
			if (alt_path_glob(&a, arg(f[1]), &names, &nn) < 0)
				printf("bad");
			for (size_t i = 0; i < nn; i++)
				printf("%.*s;", (int)names[i].n, names[i].p);
			alt_arena_free(&a);
		} else if (!strcmp(f[0], "walk")) {
			wa w = {n > 2 ? arg(f[2]) : ALT_STR(""), n > 3};
			alt_path_walk(arg(f[1]), visit, &w);
		}
		printf("\n");
		alt_buf_free(&b);
		fflush(stdout);
	}
	return 0;
}
