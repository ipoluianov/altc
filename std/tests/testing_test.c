/* The testing library (<alt/testing.h>) run as a test binary: the
   program runs itself as a child with go test's flags and checks the
   output; it runs under the sanitizers too (threads, longjmp, parallel
   tests). tests/altctest_test.go compares the output with Go's. */
#include "check.h"
#include <alt/testing.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void T_log(alt_test *t) {
	alt_test_logf(t, "line one\nline two");
	alt_test_errorf(t, "an error");
}

static void T_pass(alt_test *t) { (void)t; }

static _Atomic int active, peak, finished;

static void par(alt_test *t, void *arg) {
	(void)arg;
	alt_test_parallel(t);
	int a = ++active;
	int p = peak;
	while (a > p && !atomic_compare_exchange_weak(&peak, &p, a))
		;
	usleep(20000);
	active--;
	finished++;
	alt_test_logf(t, "done");
}

static void T_parallel(alt_test *t) {
	for (int i = 0; i < 6; i++)
		alt_test_run(t, "p", par, NULL);
}

static void T_peak(alt_test *t) {
	char *want = getenv("WANT_PEAK");
	if (want && peak != atoi(want))
		alt_test_errorf(t, "peak %d, want %s", (int)peak, want);
	if (finished != 6)
		alt_test_errorf(t, "%d parallel tests finished", (int)finished);
}

static void deep3(alt_test *t, void *arg) {
	(void)arg;
	alt_test_fatalf(t, "level 3");
}
static void deep2(alt_test *t, void *arg) {
	(void)arg;
	alt_test_run(t, "c", deep3, NULL);
	alt_test_logf(t, "after c");
}
static void deep1(alt_test *t, void *arg) {
	(void)arg;
	alt_test_run(t, "b", deep2, NULL);
}

static void T_deep(alt_test *t) { alt_test_run(t, "a", deep1, NULL); }

static int cleanups;
static void count_cleanup(void *arg) {
	(void)arg;
	cleanups++;
}

static void T_tempdir(alt_test *t) {
	const char *d = alt_test_temp_dir(t);
	struct stat st;
	if (stat(d, &st) < 0 || !S_ISDIR(st.st_mode))
		alt_test_errorf(t, "no dir %s", d);
	alt_test_cleanup(t, count_cleanup, NULL);
}

static void T_after(alt_test *t) {
	if (cleanups != 1)
		alt_test_errorf(t, "cleanups %d", cleanups);
}

/* threads of the test's own: logging from them is allowed */
static void *logger(void *t) {
	for (int i = 0; i < 50; i++)
		alt_test_logf(t, "from a thread %d", i);
	return NULL;
}

static void T_threads(alt_test *t) {
	pthread_t th[4];
	for (int i = 0; i < 4; i++)
		pthread_create(&th[i], NULL, logger, t);
	for (int i = 0; i < 4; i++)
		pthread_join(th[i], NULL);
}

static void B_sum(alt_bench *b) {
	volatile long s = 0;
	for (size_t i = 0; i < alt_bench_n(b); i++)
		s += (long)i;
}

static void B_loop(alt_bench *b) {
	volatile long s = 0;
	while (alt_bench_loop(b))
		s++;
	alt_bench_report_metric(b, 2, "x/op");
}

static const alt_test_def tests[] = {
	{"TestLog", T_log},
	{"TestPass", T_pass},
	{"TestParallel", T_parallel},
	{"TestPeak", T_peak},
	{"TestDeep", T_deep},
	{"TestTempDir", T_tempdir},
	{"TestAfter", T_after},
	{"TestThreads", T_threads},
};
static const alt_test_def benches[] = {{"BenchmarkSum", B_sum}, {"BenchmarkLoop", B_loop}};

/* runs this program as a test binary with args: the output and exit code */
static int child(const char *self, const char *env, const char **args, alt_buf *out) {
	int p[2];
	if (pipe(p) < 0)
		return -1;
	pid_t pid = fork();
	if (pid == 0) {
		dup2(p[1], 1);
		dup2(p[1], 2);
		close(p[0]);
		close(p[1]);
		if (env)
			putenv((char *)env);
		const char *argv[16] = {self, "-child"};
		int n = 2;
		for (; args[n - 2]; n++)
			argv[n] = args[n - 2];
		argv[n] = NULL;
		execv(self, (char *const *)argv);
		_exit(127);
	}
	close(p[1]);
	char buf[4096];
	ssize_t k;
	while ((k = read(p[0], buf, sizeof buf)) > 0)
		alt_buf_append(out, buf, (size_t)k);
	close(p[0]);
	int st;
	waitpid(pid, &st, 0);
	return WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st);
}

static void expect(const char *self, const char *env, int code, const char *want, const char *notwant, const char **args) {
	alt_buf out = ALT_BUF_INIT;
	int c = child(self, env, args, &out);
	/* durations as (X) */
	for (size_t i = 0; i + 6 < out.n; i++) {
		size_t j = i + 1;
		while (j < out.n && out.p[j] >= '0' && out.p[j] <= '9')
			j++;
		if (out.p[i] == '(' && j > i + 1 && j + 4 < out.n && out.p[j] == '.' && out.p[j + 3] == 's' && out.p[j + 4] == ')') {
			memmove(out.p + i + 2, out.p + j + 4, out.n - (j + 4) + 1);
			out.p[i + 1] = 'X';
			out.n -= j + 4 - (i + 2);
		}
	}
	if (c != code)
		FAILED("%s: exit %d, want %d\n%s", args[0] ? args[0] : "", c, code, alt_buf_cstr(&out));
	if (!strstr(alt_buf_cstr(&out), want))
		FAILED("no \"%s\" in\n%s", want, alt_buf_cstr(&out));
	if (notwant && strstr(alt_buf_cstr(&out), notwant))
		FAILED("\"%s\" in\n%s", notwant, alt_buf_cstr(&out));
	alt_buf_free(&out);
}

int main(int argc, char **argv) {
	if (argc > 1 && !strcmp(argv[1], "-child")) {
		argv[1] = argv[0];
		return alt_testing_main(argc - 1, argv + 1, tests, sizeof tests / sizeof *tests, benches, 2, NULL);
	}
	const char *self = "/proc/self/exe";
	char path[4096];
	ssize_t n = readlink(self, path, sizeof path - 1);
	if (n > 0) {
		path[n] = 0;
		self = path;
	}
	expect(self, NULL, 1,
		"    testing_test.c:14: line one\n        line two\n    testing_test.c:15: an error\n", "TestPass",
		(const char *[]){NULL});
	expect(self, NULL, 0, "PASS\n", NULL, (const char *[]){"-test.run=Pass", NULL});
	expect(self, NULL, 0, "=== RUN   TestPass\n--- PASS: TestPass (X)", NULL, (const char *[]){"-test.run=Pass", "-test.v", NULL});
	expect(self, "WANT_PEAK=3", 0, "PASS\n", NULL, (const char *[]){"-test.run=Parallel|Peak", "-test.parallel=3", NULL});
	expect(self, "WANT_PEAK=1", 0, "=== CONT  TestParallel/p#05\n", NULL,
		(const char *[]){"-test.run=Parallel|Peak", "-test.parallel=1", "-test.v", NULL});
	expect(self, NULL, 1,
		"--- FAIL: TestDeep/a/b (X)\n"
		"            --- FAIL: TestDeep/a/b/c (X)\n                testing_test.c:50: level 3\n"
		"            testing_test.c:55: after c\n",
		NULL, (const char *[]){"-test.run=Deep", NULL});
	expect(self, NULL, 0, "PASS\n", NULL, (const char *[]){"-test.run=TempDir|After", NULL});
	expect(self, NULL, 0, "from a thread 49\n", NULL, (const char *[]){"-test.run=Threads", "-test.v", NULL});
	expect(self, NULL, 0, "PASS\n", "Benchmark", (const char *[]){"-test.run=Threads", NULL});
	expect(self, NULL, 0, "x/op\n", NULL, (const char *[]){"-test.run=^$", "-test.bench=.", "-test.benchtime=20ms", NULL});
	expect(self, NULL, 0, "BenchmarkSum", NULL, (const char *[]){"-test.run=^$", "-test.bench=Sum", "-test.benchtime=100x", NULL});
	expect(self, NULL, 0, "TestDeep\n", "PASS", (const char *[]){"-test.list=Deep", NULL});
	expect(self, NULL, 1, "testing: invalid regexp for element 0 of -test.run", NULL, (const char *[]){"-test.run=(", NULL});
	expect(self, NULL, 0, "testing: warning: no tests to run\nPASS\n", NULL, (const char *[]){"-test.run=Nothing", NULL});
	expect(self, NULL, 1, "--- FAIL: TestLog", "TestDeep", (const char *[]){"-test.failfast", NULL});
	return check_done();
}
