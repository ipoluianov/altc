/* regexp: linear time on patterns that make backtrackers explode, and
   the API (also run under the sanitizers; tests/stdregexp_test.go
   compares with Go) */
#include "check.h"
#include <alt/regexp.h>
#include <alt/time.h>

static void linear(void) {
	static const char *pats[] = {"(a*)*b", "(x+x+)+y", "(a|aa)+$", "(.*)*z", "^(\\w+\\s?)*$!"};
	size_t n = 100000;
	char *s = malloc(n + 1);
	for (size_t i = 0; i < n; i++)
		s[i] = i % 7 == 6 ? ' ' : (i % 2 ? 'a' : 'x');
	memset(s, 'a', n / 2);
	s[n] = 0;
	for (size_t k = 0; k < sizeof pats / sizeof *pats; k++) {
		alt_regexp *re;
		CHECK_OK(alt_regexp_compile(&re, alt_str_cstr(pats[k]), 0));
		alt_time t0 = alt_now();
		ssize_t m[16];
		alt_regexp_find_submatch(re, alt_str_make(s, n), m);
		CHECK(alt_since(t0) < 5 * ALT_SEC);
		alt_regexp_free(re);
	}
	free(s);
}

static void api(void) {
	alt_regexp *re;
	alt_buf err = ALT_BUF_INIT;
	CHECK_ERR(alt_regexp_compile(&re, ALT_STR("a(b"), &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "error parsing regexp: missing closing ): `a(b`");
	alt_buf_free(&err);
	CHECK_OK(alt_regexp_compile(&re, ALT_STR("(?P<user>\\w+)@(?P<host>[\\w.]+)"), 0));
	CHECK_INT(alt_regexp_num_subexp(re), 2);
	CHECK_INT(alt_regexp_subexp_index(re, ALT_STR("host")), 2);
	alt_buf b = ALT_BUF_INIT;
	alt_regexp_replace_all(re, &b, ALT_STR("mail bob@example.com, al@x.org"), ALT_STR("${host}/$user"));
	CHECK_STR(alt_buf_cstr(&b), "mail example.com/bob, x.org/al");
	alt_buf_free(&b);
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_str *parts;
	size_t np;
	alt_regexp *sp;
	CHECK_OK(alt_regexp_compile(&sp, ALT_STR("\\s*,\\s*"), 0));
	CHECK_OK(alt_regexp_split(sp, &a, ALT_STR("a , b,c ,d"), -1, &parts, &np));
	CHECK_INT(np, 4);
	CHECK(alt_str_eq_cstr(parts[3], "d"));
	alt_arena_free(&a);
	alt_regexp_free(sp);
	alt_regexp_free(re);
	CHECK_INT(alt_regexp_match_string(ALT_STR("^\\p{Greek}+$"), ALT_STR("αβγ")), 1);
	CHECK_INT(alt_regexp_match_string(ALT_STR("(?i)straße"), ALT_STR("STRASSE")), 0);
}

int main(void) {
	RUN(linear);
	RUN(api);
	return check_done();
}
