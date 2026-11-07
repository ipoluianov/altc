#include "check.h"
#include <alt/log.h>
#include <pthread.h>

static void text_and_with(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_logger *l, *w, *w2;
	alt_logger_config cfg = {.out = alt_buf_writer(&out), .no_time = true};
	CHECK_OK(alt_logger_new(&l, &cfg));
	alt_log_info(l, "hello");
	alt_log_debug(l, "dropped");
	alt_log_warn(l, "two words", ALT_LOG_INT("n", -3), ALT_LOG_STR("s", "a=b"), ALT_LOG_BOOL("ok", true),
		ALT_LOG_DUR("d", 1500 * ALT_MS), ALT_LOG_ERR("err", -ALT_ECLOSED));
	char key[8] = "req", val[8] = "abc";
	alt_log_attr base[] = {ALT_LOG_STR(key, val)};
	CHECK_OK(alt_logger_with(&w, l, base, 1));
	strcpy(key, "XXX"), strcpy(val, "YYY"); /* copied by with */
	alt_log_attr more[] = {ALT_LOG_UINT("u", 7)};
	CHECK_OK(alt_logger_with(&w2, w, more, 1));
	alt_log_error(w2, "x", ALT_LOG_FLOAT("f", 0.5));
	alt_logger_set_level(l, ALT_LOG_DEBUG);
	CHECK(alt_logger_enabled(l, ALT_LOG_DEBUG));
	CHECK(!alt_logger_enabled(w, ALT_LOG_DEBUG)); /* its own level */
	alt_logf(l, ALT_LOG_DEBUG + 1, "n=%d", 5);
	alt_logf(l, ALT_LOG_DEBUG - 5, "below");
	alt_buf want = ALT_BUF_INIT;
	alt_buf_printf(&want, "level=INFO msg=hello\n"
			      "level=WARN msg=\"two words\" n=-3 s=\"a=b\" ok=true d=1.5s err=\"%s\"\n"
			      "level=ERROR msg=x req=abc u=7 f=0.5\n"
			      "level=DEBUG+1 msg=\"n=5\"\n",
		alt_strerror(-ALT_ECLOSED));
	CHECK_STR(alt_buf_str(&out), alt_buf_cstr(&want));
	alt_logger_free(w2);
	alt_logger_free(w);
	alt_logger_free(l);
	alt_buf_free(&out);
	alt_buf_free(&want);
}

static void json_and_time(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_logger *l;
	alt_logger_config cfg = {.out = alt_buf_writer(&out), .format = ALT_LOG_JSON, .utc = true};
	CHECK_OK(alt_logger_new(&l, &cfg));
	alt_log_info(l, "m", ALT_LOG_DUR("d", 3), ALT_LOG_S("s", alt_str_make("a\0b", 3)));
	/* {"time":"2026-11-07T10:00:00.123Z","level":"INFO","msg":"m","d":3,"s":"a\u0000b"} */
	alt_str s = alt_buf_str(&out);
	CHECK(alt_str_has_prefix(s, alt_str_cstr("{\"time\":\"20")));
	CHECK(s.n > 37 && s.p[32] == 'Z' && s.p[28] == '.');
	CHECK(alt_str_has_suffix(s, alt_str_cstr("\",\"level\":\"INFO\",\"msg\":\"m\",\"d\":3,\"s\":\"a\\u0000b\"}\n")));
	alt_logger_free(l);
	alt_buf_free(&out);
}

static alt_logger *shared;
enum { THREADS = 8, LINES = 2000 };

static void *writer(void *arg) {
	for (int i = 0; i < LINES; i++)
		alt_log_info(shared, "concurrent", ALT_LOG_INT("t", (long long)(intptr_t)arg), ALT_LOG_INT("i", i),
			ALT_LOG_STR("pad", "................................................................"));
	return 0;
}

static void concurrent(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_logger_config cfg = {.out = alt_buf_writer(&out), .no_time = true};
	CHECK_OK(alt_logger_new(&shared, &cfg));
	pthread_t th[THREADS];
	for (intptr_t i = 0; i < THREADS; i++)
		pthread_create(&th[i], 0, writer, (void *)i);
	for (int i = 0; i < THREADS; i++)
		pthread_join(th[i], 0);
	/* every line whole, every thread's lines in order */
	int next[THREADS] = {0}, lines = 0;
	alt_str rest = alt_buf_str(&out), line;
	while (alt_str_cut(rest, alt_str_cstr("\n"), &line, &rest)) {
		int t, i;
		if (sscanf(line.p, "level=INFO msg=concurrent t=%d i=%d pad=", &t, &i) != 2 || t < 0 || t >= THREADS ||
			i != next[t]++ || !alt_str_has_suffix(line, alt_str_cstr("....")))
			FAILED("bad line %.*s", (int)line.n, line.p);
		lines++;
	}
	CHECK_INT(lines, THREADS * LINES);
	alt_logger_free(shared);
	alt_buf_free(&out);
}

int main(void) {
	RUN(text_and_with);
	RUN(json_and_time);
	RUN(concurrent);
	return check_done();
}
