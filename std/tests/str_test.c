#include "check.h"
#include <alt/arena.h>
#include <alt/str.h>
#include <limits.h>

static void basics(void) {
	alt_str s = ALT_STR("hello, world");
	CHECK_INT(s.n, 12);
	CHECK(alt_str_eq(s, alt_str_cstr("hello, world")));
	CHECK(!alt_str_eq(s, ALT_STR("hello")));
	CHECK(alt_str_eq_fold(ALT_STR("Content-Type"), ALT_STR("content-TYPE")));
	CHECK(!alt_str_eq_fold(ALT_STR("a"), ALT_STR("ab")));
	CHECK(alt_str_has_prefix(s, ALT_STR("hello")));
	CHECK(!alt_str_has_prefix(ALT_STR("he"), ALT_STR("hello")));
	CHECK(alt_str_has_suffix(s, ALT_STR("world")));
	CHECK(alt_str_has_prefix(s, ALT_STR("")));
	CHECK(alt_str_has_prefix_fold(ALT_STR("HTTP/1.1"), ALT_STR("http/")));
	CHECK_STR(alt_str_slice(s, 7, 12), "world");
	CHECK_STR(alt_str_slice(s, 7, 100), "world");
	CHECK_STR(alt_str_slice(s, 20, 30), "");
	CHECK_STR(alt_str_from(s, 5), ", world");
	CHECK_INT(alt_str_cmp(ALT_STR("a"), ALT_STR("b")), -1);
	CHECK_INT(alt_str_cmp(ALT_STR("ab"), ALT_STR("a")), 1);
	CHECK_INT(alt_str_cmp(ALT_STR(""), ALT_STR("")), 0);
	char with_nul[] = {'a', 0, 'b'};
	alt_str z = alt_str_make(with_nul, 3);
	CHECK_INT(alt_str_index_byte(z, 'b'), 2);
	CHECK(alt_str_eq(z, alt_str_make("a\0b", 3)));
}

static void search(void) {
	alt_str s = ALT_STR("chicken kitchen");
	CHECK_INT(alt_str_index(s, ALT_STR("ken")), 4);
	CHECK_INT(alt_str_last_index(s, ALT_STR("en")), 13);
	CHECK_INT(alt_str_index(s, ALT_STR("dmr")), -1);
	CHECK_INT(alt_str_index(s, ALT_STR("")), 0);
	CHECK_INT(alt_str_last_index(s, ALT_STR("")), 15);
	CHECK_INT(alt_str_index(ALT_STR("aaab"), ALT_STR("aab")), 1);
	CHECK_INT(alt_str_index(ALT_STR("ab"), ALT_STR("abc")), -1);
	CHECK_INT(alt_str_index_byte(s, 'k'), 4);
	CHECK_INT(alt_str_last_index_byte(s, 'k'), 8);
	CHECK_INT(alt_str_index_byte(ALT_STR(""), 'k'), -1);
	CHECK_INT(alt_str_index_any(s, "xyzt"), 10);
	CHECK_INT(alt_str_index_any(s, "q"), -1);
	CHECK_INT(alt_str_count(ALT_STR("cheese"), ALT_STR("e")), 3);
	CHECK_INT(alt_str_count(ALT_STR("five"), ALT_STR("")), 5);
	CHECK_INT(alt_str_count(ALT_STR("aaaa"), ALT_STR("aa")), 2);
	CHECK(alt_str_contains(s, ALT_STR("kit")));
}

static void trimming(void) {
	CHECK_STR(alt_str_trim_space(ALT_STR(" \t\r\n hi there \n")), "hi there");
	CHECK_STR(alt_str_trim_space(ALT_STR("   ")), "");
	CHECK_STR(alt_str_trim(ALT_STR("xxhixyx"), "xy"), "hi");
	CHECK_STR(alt_str_trim_left(ALT_STR("00012"), "0"), "12");
	CHECK_STR(alt_str_trim_right(ALT_STR("12.500"), "0"), "12.5");
	CHECK_STR(alt_str_trim_prefix(ALT_STR("/api/x"), ALT_STR("/api")), "/x");
	CHECK_STR(alt_str_trim_prefix(ALT_STR("/x"), ALT_STR("/api")), "/x");
	CHECK_STR(alt_str_trim_suffix(ALT_STR("a.tar.gz"), ALT_STR(".gz")), "a.tar");
}

static void cutting(void) {
	alt_str a, b;
	CHECK(alt_str_cut(ALT_STR("key=value=x"), ALT_STR("="), &a, &b));
	CHECK_STR(a, "key");
	CHECK_STR(b, "value=x");
	CHECK(!alt_str_cut(ALT_STR("novalue"), ALT_STR("="), &a, &b));
	CHECK_STR(a, "novalue");
	CHECK_STR(b, "");
	CHECK(alt_str_cut_byte(ALT_STR("host:80"), ':', &a, &b));
	CHECK_STR(b, "80");

	/* strings.Split semantics */
	const char *want[] = {"a", "", "b", "c", ""};
	alt_str rest = ALT_STR("a,,b,c,"), f;
	int k = 0;
	while (alt_str_split_next(&rest, ALT_STR(","), &f)) {
		if (k < 5)
			CHECK_STR(f, want[k]);
		k++;
	}
	CHECK_INT(k, 5);
	rest = ALT_STR("");
	k = 0;
	while (alt_str_split_next(&rest, ALT_STR(","), &f))
		k++;
	CHECK_INT(k, 1);
	rest = ALT_STR("a--b");
	k = 0;
	while (alt_str_split_next(&rest, ALT_STR("--"), &f))
		k++;
	CHECK_INT(k, 2);

	/* strings.Fields */
	const char *fw[] = {"GET", "/index.html", "HTTP/1.1"};
	rest = ALT_STR("  GET   /index.html\tHTTP/1.1  ");
	k = 0;
	while (alt_str_fields_next(&rest, &f)) {
		if (k < 3)
			CHECK_STR(f, fw[k]);
		k++;
	}
	CHECK_INT(k, 3);
}

static void numbers(void) {
	long long v;
	unsigned long long u;
	CHECK_OK(alt_str_to_int(ALT_STR("-12345"), 10, &v));
	CHECK_INT(v, -12345);
	CHECK_OK(alt_str_to_int(ALT_STR("+7"), 10, &v));
	CHECK_INT(v, 7);
	CHECK_OK(alt_str_to_int(ALT_STR("0x1F"), 0, &v));
	CHECK_INT(v, 31);
	CHECK_OK(alt_str_to_int(ALT_STR("0o17"), 0, &v));
	CHECK_INT(v, 15);
	CHECK_OK(alt_str_to_int(ALT_STR("017"), 0, &v));
	CHECK_INT(v, 15);
	CHECK_OK(alt_str_to_int(ALT_STR("0b101"), 0, &v));
	CHECK_INT(v, 5);
	CHECK_OK(alt_str_to_int(ALT_STR("zz"), 36, &v));
	CHECK_INT(v, 36 * 36 - 1);
	CHECK_OK(alt_str_to_int(ALT_STR("9223372036854775807"), 10, &v));
	CHECK_INT(v, LLONG_MAX);
	CHECK_OK(alt_str_to_int(ALT_STR("-9223372036854775808"), 10, &v));
	CHECK(v == LLONG_MIN);
	CHECK_ERR(alt_str_to_int(ALT_STR("9223372036854775808"), 10, &v), -ALT_ERANGE);
	CHECK_INT(v, LLONG_MAX);
	CHECK_ERR(alt_str_to_int(ALT_STR("-9223372036854775809"), 10, &v), -ALT_ERANGE);
	CHECK_ERR(alt_str_to_int(ALT_STR(""), 10, &v), -ALT_ESYNTAX);
	CHECK_ERR(alt_str_to_int(ALT_STR("-"), 10, &v), -ALT_ESYNTAX);
	CHECK_ERR(alt_str_to_int(ALT_STR("12a"), 10, &v), -ALT_ESYNTAX);
	CHECK_ERR(alt_str_to_int(ALT_STR(" 1"), 10, &v), -ALT_ESYNTAX);
	CHECK_OK(alt_str_to_uint(ALT_STR("18446744073709551615"), 10, &u));
	CHECK(u == ULLONG_MAX);
	CHECK_ERR(alt_str_to_uint(ALT_STR("18446744073709551616"), 10, &u), -ALT_ERANGE);
	CHECK_ERR(alt_str_to_uint(ALT_STR("99999999999999999999x"), 10, &u), -ALT_ESYNTAX);
	CHECK_ERR(alt_str_to_uint(ALT_STR("-1"), 10, &u), -ALT_ESYNTAX);
	CHECK_ERR(alt_str_to_uint(ALT_STR("12"), 1, &u), -ALT_ESYNTAX);
	CHECK_OK(alt_str_to_uint(ALT_STR("ff"), 16, &u));
	CHECK_INT(u, 255);
}

static void buffers(void) {
	alt_buf b = ALT_BUF_INIT;
	CHECK_STR(alt_buf_cstr(&b), "");
	CHECK_INT(alt_buf_str(&b).n, 0);
	CHECK_OK(alt_buf_append_cstr(&b, "abc"));
	CHECK_OK(alt_buf_append_byte(&b, '-'));
	CHECK_OK(alt_buf_append_int(&b, -42));
	CHECK_OK(alt_buf_append_uint(&b, 18446744073709551615ULL));
	CHECK_OK(alt_buf_printf(&b, "|%05d|%s", 7, "x"));
	CHECK_STR(alt_buf_str(&b), "abc--4218446744073709551615|00007|x");
	CHECK_INT(strlen(alt_buf_cstr(&b)), b.n);
	/* a long printf goes beyond the small buffer */
	alt_buf_reset(&b);
	CHECK_OK(alt_buf_printf(&b, "%0500d", 1));
	CHECK_INT(b.n, 500);
	CHECK(b.p[499] == '1' && b.p[0] == '0' && b.p[500] == 0);
	/* appending from itself */
	alt_buf_reset(&b);
	alt_buf_append_cstr(&b, "xy");
	for (int i = 0; i < 10; i++)
		CHECK_OK(alt_buf_append(&b, b.p, b.n));
	CHECK_INT(b.n, 2048);
	CHECK(b.p[2046] == 'x' && b.p[2047] == 'y');
	alt_buf_consume(&b, 2040);
	CHECK_STR(alt_buf_str(&b), "xyxyxyxy");
	alt_buf_truncate(&b, 3);
	CHECK_STR(alt_buf_str(&b), "xyx");
	size_t n;
	char *p = alt_buf_detach(&b, &n);
	CHECK_INT(n, 3);
	CHECK(strcmp(p, "xyx") == 0);
	CHECK(b.p == 0 && b.n == 0);
	free(p);
	alt_buf_append(&b, "", 0);
	CHECK_STR(alt_buf_cstr(&b), "");
	alt_buf_free(&b);
	alt_buf_free(&b);
	char *d = alt_str_dup(ALT_STR("dup"));
	CHECK(strcmp(d, "dup") == 0);
	free(d);
	char up[] = "MiXeD-123";
	alt_ascii_lower(up, strlen(up));
	CHECK_STR(up, "mixed-123");
	alt_ascii_upper(up, strlen(up));
	CHECK_STR(up, "MIXED-123");
}

static int deferred[4], ndeferred;
static void note(void *arg) { deferred[ndeferred++] = (int)(intptr_t)arg; }

static void arenas(void) {
	alt_arena a;
	alt_arena_init(&a, 256);
	void *ptrs[1000];
	for (int i = 0; i < 1000; i++) {
		size_t n = (size_t)(i % 50) + 1;
		unsigned char *p = alt_arena_alloc(&a, n);
		CHECK(p != 0);
		CHECK(((uintptr_t)p & 15) == 0);
		memset(p, i & 0xff, n);
		ptrs[i] = p;
	}
	int bad = 0;
	for (int i = 0; i < 1000; i++) {
		unsigned char *p = ptrs[i];
		for (size_t k = 0; k < (size_t)(i % 50) + 1; k++)
			bad += p[k] != (unsigned char)(i & 0xff);
	}
	CHECK_INT(bad, 0);
	/* a large allocation does not disturb small ones */
	char *big = alt_arena_alloc(&a, 10000);
	memset(big, 'b', 10000);
	char *small = alt_arena_alloc(&a, 8);
	CHECK(small != 0);
	strcpy(small, "abc");
	CHECK_STR(alt_arena_strdup(&a, "copy"), "copy");
	CHECK_STR(alt_arena_str(&a, ALT_STR("slice")), "slice");
	CHECK_STR(alt_arena_printf(&a, "%d-%s", 12, "x"), "12-x");
	char *z = alt_arena_zalloc(&a, 100);
	int nonzero = 0;
	for (int i = 0; i < 100; i++)
		nonzero += z[i] != 0;
	CHECK_INT(nonzero, 0);
	/* growing the last allocation in place */
	char *g = alt_arena_alloc(&a, 16);
	memcpy(g, "0123456789abcdef", 16);
	char *g2 = alt_arena_realloc(&a, g, 16, 48);
	CHECK(g2 == g);
	char *other = alt_arena_alloc(&a, 4);
	char *g3 = alt_arena_realloc(&a, g2, 48, 64);
	CHECK(g3 != g2 && memcmp(g3, "0123456789abcdef", 16) == 0);
	(void)other;
	CHECK_OK(alt_arena_defer(&a, note, (void *)1));
	CHECK_OK(alt_arena_defer(&a, note, (void *)2));
	alt_arena_reset(&a);
	CHECK_INT(ndeferred, 2);
	CHECK_INT(deferred[0], 2);
	CHECK_INT(deferred[1], 1);
	CHECK_INT(a.used, 0);
	char *again = alt_arena_alloc(&a, 10);
	CHECK(again != 0);
	alt_arena_defer(&a, note, (void *)3);
	alt_arena_free(&a);
	CHECK_INT(ndeferred, 3);
	alt_arena_free(&a);
	/* an arena used without init (zeroed) works too */
	alt_arena zero = {0};
	CHECK(alt_arena_alloc(&zero, 5) != 0);
	alt_arena_free(&zero);
}

static void errors(void) {
	CHECK_STR(alt_strerror(0), "success");
	CHECK_STR(alt_strerror(-ALT_ECLOSED), "use of a closed object");
	CHECK_STR(alt_strerror(ALT_EHTTP_BADREQUEST), "malformed HTTP request");
	CHECK(strlen(alt_strerror(-ETIMEDOUT)) > 0);
	CHECK_STR(alt_strerror(-ENOENT), strerror(ENOENT));
	CHECK_STR(alt_strerror(-(ALT_ELAST + 5)), "unknown error");
}

int main(void) {
	RUN(basics);
	RUN(search);
	RUN(trimming);
	RUN(cutting);
	RUN(numbers);
	RUN(buffers);
	RUN(arenas);
	RUN(errors);
	return check_done();
}
