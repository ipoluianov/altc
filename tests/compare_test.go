// Package tests compares programs compiled by AltC with the same programs
// compiled by gcc: each tests/c/*.c file must print the same output and
// exit with the same code.
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
	"github.com/ipoluianov/altc/elf"
)

func run(t *testing.T, exe string, dir string) (string, int) {
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	cmd := exec.CommandContext(ctx, exe)
	cmd.Dir = dir
	var out bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &out
	err := cmd.Run()
	// a parallel test may still hold the new file open across fork: retry
	for i := 0; i < 20 && err != nil && strings.Contains(err.Error(), "text file busy"); i++ {
		time.Sleep(50 * time.Millisecond)
		out.Reset()
		cmd = exec.CommandContext(ctx, exe)
		cmd.Dir = dir
		cmd.Stdout = &out
		cmd.Stderr = &out
		err = cmd.Run()
	}
	code := 0
	if ee, ok := err.(*exec.ExitError); ok {
		code = ee.ExitCode()
	} else if err != nil {
		t.Fatalf("run %s: %v", exe, err)
	}
	return out.String(), code
}

// stdFor reads an optional "// std: c99" line at the top of a test
func stdFor(src []byte) string {
	for _, l := range strings.SplitN(string(src), "\n", 5) {
		if strings.HasPrefix(l, "// std: ") {
			return strings.TrimSpace(l[8:])
		}
	}
	return "gnu17"
}

func TestCompareWithGCC(t *testing.T) {
	gcc, err := exec.LookPath("gcc")
	if err != nil {
		t.Skip("gcc not found")
	}
	files, _ := filepath.Glob("c/*.c")
	if only := os.Getenv("ONLY"); only != "" {
		files, _ = filepath.Glob("c/" + only + ".c")
	}
	tmp := t.TempDir()
	for _, f := range files {
		f := f
		name := strings.TrimSuffix(filepath.Base(f), ".c")
		t.Run(name, func(t *testing.T) {
			t.Parallel()
			src, err := os.ReadFile(f)
			if err != nil {
				t.Fatal(err)
			}
			for _, l := range strings.SplitN(string(src), "\n", 5) {
				if strings.HasPrefix(l, "// skip: ") {
					t.Skip(strings.TrimPrefix(l, "// skip: "))
				}
			}
			std := stdFor(src)
			flags := flagsFor(t, src)
			ref := filepath.Join(tmp, name+".gcc")
			if out, err := exec.Command(gcc, append(append([]string{"-std=" + std, "-w", "-O0"}, flags...), "-o", ref, f, "-lm")...).CombinedOutput(); err != nil {
				t.Fatalf("gcc: %v\n%s", err, out)
			}
			want, wantCode := run(t, ref, tmp)

			abs, _ := filepath.Abs(f)
			u := driver.CompileSource(abs, src, withFlags(testConfig(&driver.Config{Std: std, NoWarnings: true}), flags))
			if u.Obj == nil {
				for _, d := range u.Diagnostics {
					t.Error(d)
				}
				t.FailNow()
			}
			img, diags := driver.Link(&driver.LinkInput{Objects: []*elf.Object{u.Obj}})
			if img == nil {
				for _, d := range diags {
					t.Error(d)
				}
				t.FailNow()
			}
			exe := filepath.Join(tmp, name+".altc")
			if err := os.WriteFile(exe, img, 0o755); err != nil {
				t.Fatal(err)
			}
			got, gotCode := run(t, exe, tmp)
			if got != want || gotCode != wantCode {
				t.Errorf("output differs\n--- gcc (exit %d):\n%s\n--- altc (exit %d):\n%s", wantCode, want, gotCode, got)
			}
		})
	}
}
