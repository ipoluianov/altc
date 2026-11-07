package driver

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"

	"github.com/ipoluianov/altc/elf"
)

// ErrNoTestFiles is returned by BuildTest for a project without *_test.c
var ErrNoTestFiles = fmt.Errorf("no test files")

// TestFunc is a test or benchmark found in a test file
type TestFunc struct {
	Name string
	File string
	Line int
}

// TestBinary describes a built test binary
type TestBinary struct {
	Tests      []TestFunc
	Benchmarks []TestFunc
	TestMain   bool
}

// isTestName applies Go's rule: the prefix, then nothing or a rune that
// is not a lowercase letter (TestFoo, Test_foo, Test; not Testfoo)
func isTestName(name, prefix string) bool {
	if !strings.HasPrefix(name, prefix) {
		return false
	}
	if len(name) == len(prefix) {
		return true
	}
	r, _ := utf8.DecodeRuneInString(name[len(prefix):])
	return !unicode.IsLower(r)
}

var testDefRe = regexp.MustCompile(`(?m)^[ \t]*(?:extern[ \t]+)?(void|int)[ \t\n]+((?:Test|Benchmark)\w*)[ \t]*\(([^)]*)\)[ \t\n]*\{`)

var paramRe = map[string]*regexp.Regexp{
	"Test":      regexp.MustCompile(`^\s*alt_test\s*\*\s*\w*\s*$`),
	"Benchmark": regexp.MustCompile(`^\s*alt_bench\s*\*\s*\w*\s*$`),
	"TestMain":  regexp.MustCompile(`^\s*alt_testing_m\s*\*\s*\w*\s*$`),
}

// findTests lists the test functions of a test file in source order:
// global functions of the object named Test.../Benchmark... with the
// signature checked in the source
func findTests(path string, src []byte, obj *elf.Object, bin *TestBinary) []Diagnostic {
	defined := map[string]bool{}
	for _, s := range obj.Symbols {
		if s.Bind != elf.BindLocal && s.Section != nil && s.Type == elf.SymFunc {
			defined[s.Name] = true
		}
	}
	var diags []Diagnostic
	seen := map[string]bool{}
	for _, m := range testDefRe.FindAllSubmatchIndex(src, -1) {
		ret, name, params := string(src[m[2]:m[3]]), string(src[m[4]:m[5]]), string(src[m[6]:m[7]])
		line := 1 + strings.Count(string(src[:m[4]]), "\n")
		if !defined[name] || seen[name] {
			continue
		}
		seen[name] = true
		bad := func(want string) {
			diags = append(diags, Diagnostic{File: path, Line: line, Col: 1, Severity: SevError,
				Message: fmt.Sprintf("wrong signature for %s, must be: %s", name, want)})
		}
		switch {
		case name == "TestMain":
			if ret != "int" || !paramRe["TestMain"].MatchString(params) {
				bad("int TestMain(alt_testing_m *m)")
				continue
			}
			bin.TestMain = true
		case isTestName(name, "Benchmark"):
			if ret != "void" || !paramRe["Benchmark"].MatchString(params) {
				bad("void " + name + "(alt_bench *b)")
				continue
			}
			bin.Benchmarks = append(bin.Benchmarks, TestFunc{name, path, line})
		case isTestName(name, "Test"):
			if ret != "void" || !paramRe["Test"].MatchString(params) {
				bad("void " + name + "(alt_test *t)")
				continue
			}
			bin.Tests = append(bin.Tests, TestFunc{name, path, line})
		}
	}
	return diags
}

// runnerSource is the main function of the test binary
func runnerSource(bin *TestBinary) string {
	var b strings.Builder
	b.WriteString("#include <alt/testing.h>\n#include <stddef.h>\n")
	for _, t := range bin.Tests {
		fmt.Fprintf(&b, "void %s(alt_test *);\n", t.Name)
	}
	for _, t := range bin.Benchmarks {
		fmt.Fprintf(&b, "void %s(alt_bench *);\n", t.Name)
	}
	if bin.TestMain {
		b.WriteString("int TestMain(alt_testing_m *);\n")
	}
	table := func(name string, fs []TestFunc) {
		fmt.Fprintf(&b, "static const alt_test_def %s[] = {\n", name)
		for _, t := range fs {
			fmt.Fprintf(&b, "\t{%q, %s},\n", t.Name, t.Name)
		}
		if len(fs) == 0 {
			b.WriteString("\t{0, 0},\n")
		}
		b.WriteString("};\n")
	}
	table("tests", bin.Tests)
	table("benches", bin.Benchmarks)
	tm := "NULL"
	if bin.TestMain {
		tm = "TestMain"
	}
	fmt.Fprintf(&b, "int main(int argc, char **argv) {\n\treturn alt_testing_main(argc, argv, tests, %d, benches, %d, %s);\n}\n",
		len(bin.Tests), len(bin.Benchmarks), tm)
	return b.String()
}

// BuildTest builds the test binary of the project into out: the
// project's sources (their main hidden) with its *_test.c files and a
// generated main that runs the tests. ErrNoTestFiles (in the result's
// diagnostics) when there are none.
func (p *Project) BuildTest(ctx context.Context, opt BuildOptions, out string) (*BuildResult, *TestBinary) {
	start := time.Now()
	res := &BuildResult{}
	log := func(format string, args ...any) {
		if opt.Log != nil {
			opt.Log(fmt.Sprintf(format, args...))
		}
	}
	fail := func(msg string) (*BuildResult, *TestBinary) {
		res.Diagnostics = append(res.Diagnostics, Diagnostic{Severity: SevError, Message: msg})
		res.Duration = time.Since(start)
		return res, nil
	}
	cfg, libs, libDirs, err := p.Config()
	if err != nil {
		return fail(err.Error())
	}
	files, err := p.SourceFiles()
	if err != nil {
		return fail(err.Error())
	}
	tests, err := p.TestFiles()
	if err != nil {
		return fail(err.Error())
	}
	if len(tests) == 0 {
		return fail(ErrNoTestFiles.Error())
	}
	all := append(append([]string(nil), files...), tests...)
	units := p.compileAll(ctx, all, cfg, opt.Jobs, log)
	if ctx.Err() != nil {
		return fail("build cancelled")
	}
	bin := &TestBinary{}
	in := &LinkInput{}
	for i, u := range units {
		res.Diagnostics = append(res.Diagnostics, u.Diagnostics...)
		if u.Obj == nil {
			continue
		}
		if i < len(files) {
			// the program's own main gives way to the test runner's
			for _, s := range u.Obj.Symbols {
				if s.Name == "main" && s.Section != nil && s.Bind != elf.BindLocal {
					s.Bind = elf.BindLocal
				}
			}
		} else {
			src, err := os.ReadFile(all[i])
			if err != nil {
				return fail(err.Error())
			}
			res.Diagnostics = append(res.Diagnostics, findTests(all[i], src, u.Obj, bin)...)
		}
		in.Objects = append(in.Objects, u.Obj)
	}
	if res.HasErrors() {
		res.Duration = time.Since(start)
		return res, nil
	}
	runner := CompileSource(filepath.Join(p.Dir, "_testmain.c"), []byte(runnerSource(bin)), cfg)
	res.Diagnostics = append(res.Diagnostics, runner.Diagnostics...)
	if runner.Obj == nil {
		return fail("cannot compile the test runner")
	}
	in.Objects = append(in.Objects, runner.Obj)
	for _, l := range libs {
		if err := in.AddLib(l, libDirs); err != nil {
			return fail(err.Error())
		}
	}
	if err := PrepareLibc(func(s string) { log("%s", s) }); err != nil {
		return fail(err.Error())
	}
	log("linking %s", filepath.Base(out))
	img, diags := Link(in)
	res.Diagnostics = append(res.Diagnostics, diags...)
	if img == nil {
		res.Duration = time.Since(start)
		return res, nil
	}
	if err := WriteExecutable(out, img); err != nil {
		return fail(err.Error())
	}
	res.OK = true
	res.Output = out
	res.Duration = time.Since(start)
	return res, bin
}
