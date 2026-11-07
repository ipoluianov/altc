/* The same tests as ../go/suite_test.go: altc test's output must be go
   test's (TestAltcTestAgainstGo) */
#include <alt/testing.h>
#include <stdlib.h>
#include <string.h>

void TestLog(alt_test *t) {
	alt_test_logf(t, "hello\nworld");
	alt_test_logf(t, "trail\n");
	alt_test_logf(t, "%d+%d", 2, 3);
}

void TestError(alt_test *t) {
	alt_test_errorf(t, "e1");
	alt_test_logf(t, "after");
}

void TestFatal(alt_test *t) {
	alt_test_fatalf(t, "stop");
	alt_test_errorf(t, "not reached");
}

void TestSkip(alt_test *t) {
	alt_test_skipf(t, "skipping");
	alt_test_errorf(t, "not reached");
}

static void deep(alt_test *t, void *arg) {
	(void)arg;
	alt_test_errorf(t, "deep");
}

static void level_a(alt_test *t, void *arg) {
	(void)arg;
	alt_test_run(t, "b", deep, NULL);
}

static void pass(alt_test *t, void *arg) {
	if (arg)
		alt_test_logf(t, "%s", (const char *)arg);
}

void TestNested(alt_test *t) {
	alt_test_run(t, "a", level_a, NULL);
	alt_test_run(t, "a", pass, "second a");
	alt_test_run(t, "a#01", pass, "explicit");
	alt_test_run(t, "with space", pass, "spaced");
	alt_test_run(t, "", pass, "empty");
	alt_test_run(t, "", pass, NULL);
	alt_test_run(t, "tab\tx", pass, NULL);
	alt_test_run(t, "ctl\x01", pass, NULL);
	alt_test_run(t, "uni\xc3\xa9", pass, NULL);
}

static void fatal_sub(alt_test *t, void *arg) {
	(void)arg;
	alt_test_fatalf(t, "sub fatal");
}

void TestSubFatal(alt_test *t) {
	bool ok = alt_test_run(t, "f", fatal_sub, NULL);
	alt_test_logf(t, "run returned %s", ok ? "true" : "false");
	ok = alt_test_run(t, "g", pass, NULL);
	alt_test_logf(t, "run returned %s", ok ? "true" : "false");
}

static void log_c1(void *t) { alt_test_logf(t, "c1"); }
static void log_c2(void *t) { alt_test_logf(t, "c2"); }

void TestCleanup(alt_test *t) {
	alt_test_cleanup(t, log_c1, t);
	alt_test_cleanup(t, log_c2, t);
	alt_test_errorf(t, "fail");
}

static void par_sub(alt_test *t, void *arg) {
	(void)arg;
	alt_test_parallel(t);
	alt_test_logf(t, "in parallel");
}

void TestParallelOne(alt_test *t) {
	alt_test_run(t, "p", par_sub, NULL);
	alt_test_logf(t, "parent body done");
}

void TestTopPar(alt_test *t) {
	alt_test_parallel(t);
	alt_test_logf(t, "top parallel");
}

void TestPass(alt_test *t) { (void)t; }

void TestFailNow(alt_test *t) {
	alt_test_fail(t);
	alt_test_logf(t, "failed: %s", alt_test_failed(t) ? "true" : "false");
	alt_test_fail_now(t);
}

static void skip_sub(alt_test *t, void *arg) {
	(void)arg;
	alt_test_skip_now(t);
}

void TestSubSkip(alt_test *t) {
	alt_test_run(t, "s", skip_sub, NULL);
	alt_test_logf(t, "skipped: %s", "true");
}

void TestShort(alt_test *t) {
	if (alt_testing_short())
		alt_test_skipf(t, "short mode");
	alt_test_logf(t, "long mode");
}

void TestName(alt_test *t) { alt_test_logf(t, "name %s", alt_test_name(t)); }

static void env_sub(alt_test *t, void *arg) {
	(void)arg;
	alt_test_setenv(t, "ALTC_SUITE_VAR", "inner");
	alt_test_logf(t, "env %s", getenv("ALTC_SUITE_VAR"));
}

void TestEnv(alt_test *t) {
	alt_test_run(t, "set", env_sub, NULL);
	const char *v = getenv("ALTC_SUITE_VAR");
	alt_test_logf(t, "env after %s", v ? v : "unset");
}
