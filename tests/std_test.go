package tests

import (
	"bytes"
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/ipoluianov/altc/driver"
)

// TestStd builds and runs the C tests of the standard library
// (std/tests/*_test.c): each is a program that checks itself and exits
// with 0 when everything passed. ONLY=name runs one of them.
func TestStd(t *testing.T) {
	files, _ := filepath.Glob("../std/tests/*_test.c")
	if only := os.Getenv("ONLY"); only != "" {
		files, _ = filepath.Glob("../std/tests/" + only + "_test.c")
	}
	if len(files) == 0 {
		t.Fatal("no tests")
	}
	tmp := tempDir(t)
	for _, f := range files {
		name := strings.TrimSuffix(filepath.Base(f), "_test.c")
		t.Run(name, func(t *testing.T) {
			t.Parallel()
			exe := buildStdTest(t, f, filepath.Join(tmp, name))
			out, code := runTimeout(t, exe, tmp, 120*time.Second)
			if code != 0 {
				t.Errorf("exit %d\n%s", code, out)
			} else if testing.Verbose() {
				t.Log(out)
			}
		})
	}
}

// buildStdTest compiles a C test (with the test flags) and links it
func buildStdTest(t *testing.T, src, exe string) string {
	t.Helper()
	abs, _ := filepath.Abs(src)
	data, err := os.ReadFile(abs)
	if err != nil {
		t.Fatal(err)
	}
	cfg := testConfig(&driver.Config{Std: "gnu17", IncludeDirs: []string{filepath.Dir(abs)}})
	u := driver.CompileSource(abs, data, cfg)
	if u.Obj == nil {
		for _, d := range u.Diagnostics {
			t.Error(d)
		}
		t.FailNow()
	}
	return linkObjects(t, exe, u.Obj)
}

// runTimeout runs a program, killing it after the timeout
func runTimeout(t *testing.T, exe, dir string, timeout time.Duration, args ...string) (string, int) {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	var out bytes.Buffer
	var err error
	for i := 0; i < 20; i++ {
		out.Reset()
		cmd := exec.CommandContext(ctx, exe, args...)
		cmd.Dir = dir
		cmd.Stdout = &out
		cmd.Stderr = &out
		err = cmd.Run()
		if err == nil || !strings.Contains(err.Error(), "text file busy") {
			break
		}
		time.Sleep(50 * time.Millisecond)
	}
	if ctx.Err() != nil {
		return out.String() + "\n(timed out)", -1
	}
	if ee, ok := err.(*exec.ExitError); ok {
		return out.String(), ee.ExitCode()
	} else if err != nil {
		t.Fatalf("run %s: %v", exe, err)
	}
	return out.String(), 0
}

// TestStdWarnings compiles the standard library with gcc -Wall -Wextra
// -Werror (gcc serves as a static checker only)
func TestStdWarnings(t *testing.T) {
	gcc := needGCC(t)
	incStd, _ := filepath.Abs("../std/include")
	files, _ := filepath.Glob("../std/src/*/*.c")
	more, _ := filepath.Glob("../std/src/*/*/*.c")
	files = append(files, more...)
	for _, f := range files {
		t.Run(strings.TrimPrefix(f, "../std/src/"), func(t *testing.T) {
			t.Parallel()
			args := append(append([]string{}, gcc.args...), "-I", incStd, "-std=gnu17", "-fsyntax-only", "-Wall", "-Wextra",
				"-Wno-unused-parameter", "-Wshadow", "-Wpointer-arith", "-Wstrict-prototypes", "-Werror", f)
			if out, err := exec.Command(gcc.path, args...).CombinedOutput(); err != nil {
				t.Errorf("%v\n%s", err, out)
			}
		})
	}
}

// TestStdSanitizers builds the standard library and its C tests with the
// system's gcc and C library under AddressSanitizer with
// UndefinedBehaviorSanitizer, and under ThreadSanitizer, and runs them
// (gcc and its sanitizer runtimes serve as checkers only)
func TestStdSanitizers(t *testing.T) {
	if testing.Short() {
		t.Skip("slow")
	}
	gcc, err := exec.LookPath("gcc")
	if err != nil {
		t.Skip("gcc not found")
	}
	var srcs []string
	for _, pat := range []string{"../std/src/*/*.c", "../std/src/*/*.s", "../std/src/*/*/*.c"} {
		m, _ := filepath.Glob(pat)
		srcs = append(srcs, m...)
	}
	var plat []string
	for _, s := range srcs {
		b := filepath.Base(s)
		if strings.Contains(b, "_windows") || strings.Contains(b, "_darwin") || strings.Contains(b, "_arm64") {
			continue
		}
		plat = append(plat, s)
	}
	tests, _ := filepath.Glob("../std/tests/*_test.c")
	if only := os.Getenv("ONLY"); only != "" {
		tests, _ = filepath.Glob("../std/tests/" + only + "_test.c")
	}
	tmp := tempDir(t)
	for _, san := range []struct{ name, flags, env string }{
		{"asan", "-fsanitize=address,undefined -fno-sanitize-recover=undefined", "ASAN_OPTIONS=detect_leaks=1:abort_on_error=0"},
		{"tsan", "-fsanitize=thread", "TSAN_OPTIONS=halt_on_error=1:second_deadlock_stack=1"},
	} {
		for _, f := range tests {
			name := strings.TrimSuffix(filepath.Base(f), "_test.c")
			t.Run(san.name+"/"+name, func(t *testing.T) {
				t.Parallel()
				exe := filepath.Join(tmp, san.name+"_"+name)
				args := append([]string{"-std=gnu17", "-O1", "-g", "-fno-omit-frame-pointer", "-I", "../std/include", "-I", "../std/tests",
					"-o", exe, f}, plat...)
				args = append(strings.Fields(san.flags), args...)
				args = append(args, "-lpthread")
				if out, err := exec.Command(gcc, args...).CombinedOutput(); err != nil {
					if strings.Contains(string(out), "cannot find -l") || strings.Contains(string(out), "libasan") || strings.Contains(string(out), "libtsan") {
						t.Skipf("sanitizer runtime missing: %s", out)
					}
					t.Fatalf("%v\n%s", err, out)
				}
				cmd := exec.Command(exe)
				cmd.Dir = tmp
				cmd.Env = append(os.Environ(), san.env)
				out, err := cmd.CombinedOutput()
				if err != nil {
					t.Errorf("%v\n%s", err, out)
				}
			})
		}
	}
}
