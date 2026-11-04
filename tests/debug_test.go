package tests

import (
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

// TestDebugInfo runs gdb on programs with DWARF debug information made by
// AltC (-g -O0), by AltC's assembler from gcc -g -S output, and by AltC's
// linker from a gcc -g object, and checks lines, arguments, locals,
// globals, structures and the backtrace.
func TestDebugInfo(t *testing.T) {
	gdb, err := exec.LookPath("gdb")
	if err != nil {
		t.Skip("gdb not found")
	}
	gcc, err := exec.LookPath("gcc")
	if err != nil {
		t.Skip("gcc not found")
	}
	tmp := t.TempDir()
	src, _ := filepath.Abs("debug/point.c")
	data, err := os.ReadFile(src)
	if err != nil {
		t.Fatal(err)
	}
	link := func(t *testing.T, objs ...*elf.Object) string {
		img, diags := driver.Link(&driver.LinkInput{Objects: objs})
		if img == nil {
			t.Fatal(diags)
		}
		exe := filepath.Join(tmp, strings.ReplaceAll(t.Name(), "/", "_"))
		if err := os.WriteFile(exe, img, 0o755); err != nil {
			t.Fatal(err)
		}
		return exe
	}
	compile := func(t *testing.T, path string, src []byte, cfg *driver.Config) *elf.Object {
		u := driver.CompileSource(path, src, cfg)
		if u.Obj == nil {
			t.Fatal(u.Diagnostics)
		}
		return u.Obj
	}
	check := func(t *testing.T, exe string, locals bool) {
		cmds := []string{"break area", "run", "bt", "info args", "print *p", "next", "next", "print counter", "print scale", "print arr", "finish"}
		want := []string{"area (p=", "k=1) at", "point.c:9", "in main () at", "point.c:19",
			`{x = 3, y = 4, name = `, `"corner", flag = 5}`, "$2 = 7", "$3 = 1.5", "{1, 2, 3, 4}", "Value returned is $5 = 12"}
		if locals {
			cmds = append(cmds, "info locals")
			want = append(want, "total = 0", "i = 0")
		}
		var args []string
		for _, c := range cmds {
			args = append(args, "-ex", c)
		}
		out, _ := exec.Command(gdb, append(append([]string{"-batch", "-nx"}, args...), exe)...).CombinedOutput()
		for _, w := range want {
			if !strings.Contains(string(out), w) {
				t.Errorf("gdb output lacks %q:\n%s", w, out)
				return
			}
		}
	}
	t.Run("altc", func(t *testing.T) {
		check(t, link(t, compile(t, src, data, &driver.Config{Std: "gnu17", NoWarnings: true, NoRegAlloc: true, Debug: true})), true)
	})
	t.Run("gcc_asm", func(t *testing.T) {
		s := filepath.Join(tmp, "point.s")
		if out, err := exec.Command(gcc, "-g", "-O0", "-S", "-o", s, src).CombinedOutput(); err != nil {
			t.Fatalf("%v\n%s", err, out)
		}
		asm, _ := os.ReadFile(s)
		check(t, link(t, compile(t, s, asm, &driver.Config{Std: "gnu17", NoWarnings: true})), true)
	})
	t.Run("gcc_obj", func(t *testing.T) {
		o := filepath.Join(tmp, "point.o")
		if out, err := exec.Command(gcc, "-g", "-O0", "-c", "-o", o, src).CombinedOutput(); err != nil {
			t.Fatalf("%v\n%s", err, out)
		}
		in := &driver.LinkInput{}
		if err := in.AddFile(o); err != nil {
			t.Fatal(err)
		}
		img, diags := driver.Link(in)
		if img == nil {
			t.Fatal(diags)
		}
		exe := filepath.Join(tmp, "gcc_obj")
		os.WriteFile(exe, img, 0o755)
		check(t, exe, true)
	})
}
