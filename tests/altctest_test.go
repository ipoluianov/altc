package tests

import (
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
	"testing"
	"time"

	"github.com/ipoluianov/altc/driver"
)

// The tests of `altc test`: the C testing library (<alt/testing.h>) and
// the driver's test binaries, checked against Go's testing package on
// the same suite (testdata/altctest/c and testdata/altctest/go).

var (
	reCallSite = regexp.MustCompile(`\S+_test\.(c|go):\d+: `)
	reDuration = regexp.MustCompile(`\(\d+\.\d\ds\)`)
)

func normalizeTestOutput(s string) string {
	s = reCallSite.ReplaceAllString(s, "F: ")
	s = reDuration.ReplaceAllString(s, "(D)")
	return s
}

// buildCTestBinary builds the test binary of a C project with altc
func buildCTestBinary(t *testing.T, dir, exe string) *driver.TestBinary {
	t.Helper()
	p, err := driver.LoadProject(dir)
	if err != nil {
		t.Fatal(err)
	}
	res, bin := p.BuildTest(context.Background(), driver.BuildOptions{}, exe)
	if !res.OK {
		for _, d := range res.Diagnostics {
			t.Error(d)
		}
		t.FailNow()
	}
	return bin
}

func runBinary(t *testing.T, exe, dir string, args ...string) (string, int) {
	t.Helper()
	ctx, cancel := context.WithTimeout(context.Background(), 60*time.Second)
	defer cancel()
	cmd := exec.CommandContext(ctx, exe, args...)
	cmd.Dir = dir
	for _, e := range os.Environ() {
		if !strings.HasPrefix(e, "ALTC_SUITE_VAR=") {
			cmd.Env = append(cmd.Env, e)
		}
	}
	out, err := cmd.CombinedOutput()
	code := 0
	if ee, ok := err.(*exec.ExitError); ok {
		code = ee.ExitCode()
	} else if err != nil {
		t.Fatal(err)
	}
	return string(out), code
}

func TestAltcTestAgainstGo(t *testing.T) {
	tmp := tempDir(t)
	cdir, _ := filepath.Abs("testdata/altctest/c")
	gdir, _ := filepath.Abs("testdata/altctest/go")
	cexe := filepath.Join(tmp, "c.test")
	gexe := filepath.Join(tmp, "go.test")
	bin := buildCTestBinary(t, cdir, cexe)
	if len(bin.Tests) != 15 || bin.TestMain || len(bin.Benchmarks) != 0 {
		t.Fatalf("found %+v", bin)
	}
	cmd := exec.Command("go", "test", "-c", "-o", gexe)
	cmd.Dir = gdir
	cmd.Env = append(os.Environ(), "GOFLAGS=-mod=mod", "GOWORK=off")
	if out, err := cmd.CombinedOutput(); err != nil {
		t.Fatalf("go test -c: %v\n%s", err, out)
	}
	runs := [][]string{
		{},
		{"-test.v"},
		{"-test.v=false"},
		{"-test.run=Nested"},
		{"-test.run=Nested/a/b", "-test.v"},
		{"-test.run=Nested/a$", "-test.v"},
		{"-test.run=Nested/a#01", "-test.v"},
		{"-test.run=Nested/with_space", "-test.v"},
		{"-test.run=Nested/^$", "-test.v"},
		{"-test.run=/a", "-test.v"},
		{"-test.run=Log|Skip", "-test.v"},
		{"-test.run=Log|Nested/a/b", "-test.v"},
		{"-test.run=[/]|Pass", "-test.v"},
		{"-test.skip=Fatal", "-test.run=F"},
		{"-test.skip=Nested/a", "-test.run=Nested", "-test.v"},
		{"-test.run=Nope"},
		{"-test.run=Nope", "-test.v"},
		{"-test.list=."},
		{"-test.list=Sub"},
		{"-test.count=2", "-test.run=Pass|Nested", "-test.v"},
		{"-test.count=0"},
		{"-test.failfast", "-test.run=Error|Fatal|Pass"},
		{"-test.failfast", "-test.run=Pass|Error|Fatal", "-test.v"},
		{"-test.short", "-test.run=Short", "-test.v"},
		{"-test.run=Short", "-test.v"},
		{"-test.run=Cleanup|FailNow|SubSkip|SubFatal", "-test.v"},
		{"-test.run=Cleanup|FailNow|SubSkip|SubFatal"},
		{"-test.run=Parallel|TopPar|Pass", "-test.v"},
		{"-test.run=Parallel|TopPar|Pass", "-test.v", "-test.parallel=1"},
		{"-test.run=Name|Env", "-test.v"},
		{"-test.run=Name/x", "-test.v"},
	}
	for _, args := range runs {
		gout, gcode := runBinary(t, gexe, gdir, args...)
		cout, ccode := runBinary(t, cexe, cdir, args...)
		if normalizeTestOutput(cout) != normalizeTestOutput(gout) || ccode != gcode {
			t.Errorf("%v: exit %d (Go %d)\n--- altc:\n%s\n--- go:\n%s", args, ccode, gcode, cout, gout)
		}
	}
	// a bad pattern: the same message and exit code
	for _, pat := range []string{"(", "a/(", "a|b/["} {
		gout, gcode := runBinary(t, gexe, gdir, "-test.run="+pat)
		cout, ccode := runBinary(t, cexe, cdir, "-test.run="+pat)
		if cout != gout || ccode != gcode {
			t.Errorf("-test.run=%s: exit %d (Go %d)\n%q\n%q", pat, ccode, gcode, cout, gout)
		}
	}
	// the call sites are the C file's lines
	out, _ := runBinary(t, cexe, cdir, "-test.run=Error")
	if !strings.Contains(out, "    suite_test.c:14: e1\n") {
		t.Errorf("call site:\n%s", out)
	}
}

// writeProject creates a project folder with the given files
func writeProject(t *testing.T, files map[string]string) string {
	t.Helper()
	dir := filepath.Join(tempDir(t), "proj")
	for name, text := range files {
		p := filepath.Join(dir, name)
		os.MkdirAll(filepath.Dir(p), 0o755)
		if err := os.WriteFile(p, []byte(text), 0o644); err != nil {
			t.Fatal(err)
		}
	}
	return dir
}

// TestAltcTestProject: the program's main is left out, its functions are
// callable from the tests, the ordinary build leaves the tests out; and
// what Go cannot share with C: parallel tests, crashes, the timeout,
// TestMain, TempDir, benchmarks, signatures.
func TestAltcTestProject(t *testing.T) {
	dir := writeProject(t, map[string]string{
		"main.c": `#include <stdio.h>
int add(int a, int b) { return a + b; }
int main(void) { printf("%d\n", add(2, 3)); return 0; }
`,
		"lib/util.c": `int twice(int x) { return 2 * x; }
`,
		"calc_test.c": `#include <alt/testing.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int add(int a, int b);
int twice(int x);
void TestAdd(alt_test *t) {
	if (add(2, 2) != 4 || twice(3) != 6)
		alt_test_errorf(t, "wrong");
}
static int active, peak;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static void par(alt_test *t, void *arg) {
	(void)arg;
	alt_test_parallel(t);
	pthread_mutex_lock(&m);
	if (++active > peak) peak = active;
	pthread_mutex_unlock(&m);
	usleep(100000);
	pthread_mutex_lock(&m);
	active--;
	pthread_mutex_unlock(&m);
}
void TestParallelMany(alt_test *t) {
	for (int i = 0; i < 8; i++)
		alt_test_run(t, "p", par, NULL);
}
void TestPeak(alt_test *t) {
	if (getenv("WANT_PEAK") && peak != atoi(getenv("WANT_PEAK")))
		alt_test_errorf(t, "peak %d, want %s", peak, getenv("WANT_PEAK"));
}
static char saved[4096];
static void check_gone(void *arg) { (void)arg; }
void TestTempDir(alt_test *t) {
	const char *d1 = alt_test_temp_dir(t), *d2 = alt_test_temp_dir(t);
	struct stat st;
	if (!strcmp(d1, d2) || stat(d1, &st) || stat(d2, &st))
		alt_test_errorf(t, "temp dirs %s %s", d1, d2);
	char f[4200];
	snprintf(f, sizeof f, "%s/x", d1);
	FILE *fp = fopen(f, "w");
	fputs("data", fp);
	fclose(fp);
	snprintf(saved, sizeof saved, "%s", d1);
	alt_test_cleanup(t, check_gone, NULL);
}
void TestTempDirGone(alt_test *t) {
	struct stat st;
	if (saved[0] && stat(saved, &st) == 0)
		alt_test_errorf(t, "%s still exists", saved);
}
void TestCrash(alt_test *t) {
	if (!getenv("CRASH")) alt_test_skipf(t, "no CRASH");
	alt_test_logf(t, "about to crash");
	volatile int *p = 0;
	*p = 1;
}
void TestHang(alt_test *t) {
	if (!getenv("HANG")) alt_test_skipf(t, "no HANG");
	sleep(30);
}
static void fatal_thread_fn(alt_test *t, void *arg) { (void)arg; alt_test_fatalf(t, "from sub"); }
void TestFatalInSub(alt_test *t) {
	if (!getenv("FAILS")) return;
	alt_test_run(t, "x", fatal_thread_fn, NULL);
}
void BenchmarkAdd(alt_bench *b) {
	volatile int s = 0;
	for (size_t i = 0; i < alt_bench_n(b); i++) s += add(1, 2);
	alt_bench_set_bytes(b, 8);
}
static void sub_bench(alt_bench *b, void *arg) {
	volatile int s = 0;
	(void)arg;
	while (alt_bench_loop(b)) s += twice(3);
	alt_bench_report_metric(b, 3.5, "things/op");
}
void BenchmarkSubs(alt_bench *b) {
	alt_bench_run(b, "one", sub_bench, NULL);
	alt_bench_run(b, "two", sub_bench, NULL);
}
static int setup_done;
int TestMain(alt_testing_m *m) {
	setup_done = 1;
	int code = alt_testing_m_run(m);
	printf("teardown %d\n", code);
	return code;
}
void TestSetup(alt_test *t) {
	if (!setup_done) alt_test_errorf(t, "TestMain did not run first");
}
`,
	})
	p, err := driver.LoadProject(dir)
	if err != nil {
		t.Fatal(err)
	}
	// the ordinary build has no tests in it and keeps main
	res := p.Build(context.Background(), driver.BuildOptions{})
	if !res.OK {
		t.Fatalf("build: %v", res.Diagnostics)
	}
	if out, code := runBinary(t, res.Output, dir); out != "5\n" || code != 0 {
		t.Fatalf("program: %q %d", out, code)
	}
	exe := filepath.Join(tempDir(t), "proj.test")
	bin := buildCTestBinary(t, dir, exe)
	var names []string
	for _, f := range bin.Tests {
		names = append(names, f.Name)
	}
	if strings.Join(names, " ") != "TestAdd TestParallelMany TestPeak TestTempDir TestTempDirGone TestCrash TestHang TestFatalInSub TestSetup" ||
		!bin.TestMain || len(bin.Benchmarks) != 2 {
		t.Fatalf("found %v %+v", names, bin)
	}
	check := func(want string, wantCode int, env []string, args ...string) string {
		t.Helper()
		ctx, cancel := context.WithTimeout(context.Background(), 60*time.Second)
		defer cancel()
		cmd := exec.CommandContext(ctx, exe, args...)
		cmd.Dir = dir
		cmd.Env = append(os.Environ(), env...)
		outb, err := cmd.CombinedOutput()
		out := string(outb)
		code := 0
		if ee, ok := err.(*exec.ExitError); ok {
			code = ee.ExitCode()
		}
		if code != wantCode || !strings.Contains(out, want) {
			t.Errorf("%v %v: exit %d (want %d), want %q in\n%s", env, args, code, wantCode, want, out)
		}
		return out
	}
	check("PASS\nteardown 0\n", 0, nil)
	// -parallel bounds the parallel subtests; 8 x 100ms in parallel
	start := time.Now()
	check("PASS", 0, []string{"WANT_PEAK=8"}, "-test.run=ParallelMany|Peak", "-test.parallel=8")
	if d := time.Since(start); d > 700*time.Millisecond {
		t.Errorf("8 parallel tests of 100ms took %v", d)
	}
	check("PASS", 0, []string{"WANT_PEAK=2"}, "-test.run=ParallelMany|Peak", "-test.parallel=2")
	check("PASS", 0, []string{"WANT_PEAK=1"}, "-test.run=ParallelMany|Peak", "-test.parallel=1")
	check("--- FAIL: TestFatalInSub (", 1, []string{"FAILS=1"}, "-test.run=FatalInSub")
	out := check("panic: signal SIGSEGV: segmentation violation\n", 2, []string{"CRASH=1"}, "-test.run=Crash")
	if !strings.Contains(out, "--- FAIL: TestCrash\n    calc_test.c:") || !strings.Contains(out, "about to crash") {
		t.Errorf("crash report:\n%s", out)
	}
	out = check("panic: test timed out after 300ms\nrunning tests:\n\tTestHang (0s)\n", 2, []string{"HANG=1"}, "-test.run=Hang", "-test.timeout=300ms")
	_ = out
	out = check("PASS", 0, nil, "-test.run=^$", "-test.bench=.", "-test.benchtime=50ms")
	for _, re := range []string{`(?m)^BenchmarkAdd-\d+\s+\d+\s+[\d.]+ ns/op\s+[\d.]+ MB/s$`,
		`(?m)^BenchmarkSubs/one-\d+\s+\d+\s+[\d.]+ ns/op\s+3\.500 things/op$`, `(?m)^BenchmarkSubs/two-\d+\s`, "(?m)^goos: linux$"} {
		if !regexp.MustCompile(re).MatchString(out) {
			t.Errorf("benchmarks: no %s in\n%s", re, out)
		}
	}
	out = check("PASS", 0, nil, "-test.run=^$", "-test.bench=Add", "-test.benchtime=1000x", "-test.count=3")
	if n := len(regexp.MustCompile(`(?m)^BenchmarkAdd-\d+\s+1000\s`).FindAllString(out, -1)); n != 3 {
		t.Errorf("-benchtime=1000x -count=3: %d lines\n%s", n, out)
	}
	check("flag provided but not defined: -test.nope", 2, nil, "-test.nope")

	// wrong signatures are errors at build time
	bad := writeProject(t, map[string]string{"x_test.c": "#include <alt/testing.h>\nvoid TestBad(int x) {}\nint TestMain(void) { return 0; }\n"})
	pb, _ := driver.LoadProject(bad)
	r, _ := pb.BuildTest(context.Background(), driver.BuildOptions{}, filepath.Join(tempDir(t), "bad.test"))
	var msgs []string
	for _, d := range r.Diagnostics {
		msgs = append(msgs, d.String())
	}
	all := strings.Join(msgs, "\n")
	if r.OK || !strings.Contains(all, "x_test.c:2:1: error: wrong signature for TestBad, must be: void TestBad(alt_test *t)") ||
		!strings.Contains(all, "wrong signature for TestMain, must be: int TestMain(alt_testing_m *m)") {
		t.Errorf("bad signatures: %v\n%s", r.OK, all)
	}
	// no test files
	none := writeProject(t, map[string]string{"main.c": "int main(void) { return 0; }\n"})
	pn, _ := driver.LoadProject(none)
	r, _ = pn.BuildTest(context.Background(), driver.BuildOptions{}, filepath.Join(tempDir(t), "none.test"))
	if r.OK || len(r.Diagnostics) != 1 || r.Diagnostics[0].Message != driver.ErrNoTestFiles.Error() {
		t.Errorf("no test files: %v", r.Diagnostics)
	}
}
