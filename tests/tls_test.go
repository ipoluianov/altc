package tests

import (
	"os"
	"os/exec"
	"path/filepath"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

// TestTLSRelax links gcc -fPIC objects whose thread-local accesses use the
// general and local dynamic models (__tls_get_addr): the linker relaxes
// them to local exec for variables of the executable and to initial exec
// for a variable of a shared library, dynamically and with -static
func TestTLSRelax(t *testing.T) {
	gcc, err := exec.LookPath("gcc")
	if err != nil {
		t.Skip("gcc not found")
	}
	tmp := t.TempDir()
	sh := func(args ...string) {
		t.Helper()
		if out, err := exec.Command(gcc, args...).CombinedOutput(); err != nil {
			t.Fatalf("gcc %v: %v\n%s", args, err, out)
		}
	}
	mainO := filepath.Join(tmp, "pic_main.o")
	libO := filepath.Join(tmp, "lib.o")
	libSo := filepath.Join(tmp, "libtlsvar.so")
	sh("-fPIC", "-O2", "-c", "-o", mainO, "tls/pic_main.c")
	sh("-fPIC", "-O2", "-c", "-o", libO, "tls/lib.c")
	sh("-shared", "-o", libSo, libO)
	src, err := os.ReadFile("tls/altc_part.c")
	if err != nil {
		t.Fatal(err)
	}
	abs, _ := filepath.Abs("tls/altc_part.c")
	u := driver.CompileSource(abs, src, &driver.Config{Std: "gnu17"})
	if u.Obj == nil {
		t.Fatal(u.Diagnostics)
	}
	altcO := filepath.Join(tmp, "altc_part.o")
	if err := os.WriteFile(altcO, elf.WriteObject(u.Obj), 0o644); err != nil {
		t.Fatal(err)
	}
	ref := filepath.Join(tmp, "ref")
	sh("-o", ref, mainO, altcO, libO, "-pthread")
	want, _ := run(t, ref, tmp)
	if want == "" {
		t.Fatal("no reference output")
	}
	for _, c := range []struct {
		name   string
		static bool
		lib    string
	}{{"shared_lib", false, libSo}, {"object", false, libO}, {"static", true, libO}} {
		t.Run(c.name, func(t *testing.T) {
			in := &driver.LinkInput{Objects: []*elf.Object{u.Obj}, Static: c.static}
			for _, f := range []string{mainO, c.lib} {
				if err := in.AddFile(f); err != nil {
					t.Fatal(err)
				}
			}
			img, diags := driver.Link(in)
			if img == nil {
				t.Fatal(diags)
			}
			exe := filepath.Join(tmp, "tls_"+c.name)
			if err := os.WriteFile(exe, img, 0o755); err != nil {
				t.Fatal(err)
			}
			cmd := exec.Command(exe)
			cmd.Env = append(os.Environ(), "LD_LIBRARY_PATH="+tmp)
			out, err := cmd.CombinedOutput()
			if err != nil || string(out) != want {
				t.Errorf("%v\ngot:\n%s\nwant:\n%s", err, out, want)
			}
		})
	}
}
