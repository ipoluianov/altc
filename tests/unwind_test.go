package tests

import (
	"os"
	"os/exec"
	"path/filepath"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

// TestUnwindThroughAltC checks the call frame information of AltC's code:
// a C++ exception thrown below an AltC function is caught above it, and
// pthread_cancel runs the cleanup handlers of gcc-compiled frames above
// AltC frames. Each program is linked by gcc and by AltC's linker.
func TestUnwindThroughAltC(t *testing.T) {
	cases := []struct {
		name, gccSrc, gccCmd, altcSrc, want string
		libs                                []string
	}{
		{"cxx_throw", "unwind/throw_main.cpp", "g++", "unwind/throw_mid.c", "caught boom sentinel=12345\n", []string{"stdc++"}},
		{"pthread_cancel", "unwind/cancel_main.c", "gcc", "unwind/cancel_inner.c", "cleanup outer\njoined canceled=1\n", nil},
	}
	tmp := t.TempDir()
	for _, c := range cases {
		for _, flags := range [][]string{nil, {"-O2"}, {"-O2", "-mavx2"}} {
			c, flags := c, flags
			t.Run(c.name, func(t *testing.T) {
				cc, err := exec.LookPath(c.gccCmd)
				if err != nil {
					t.Skip(c.gccCmd + " not found")
				}
				var isaFlags []string
				for _, f := range flags {
					if f != "-O2" {
						isaFlags = append(isaFlags, f)
					}
				}
				flagsFor(t, []byte("// flags: "+join(isaFlags)+"\n"))
				gobj := filepath.Join(tmp, c.name+".gcc.o")
				if out, err := exec.Command(cc, "-O2", "-fexceptions", "-c", "-o", gobj, c.gccSrc).CombinedOutput(); err != nil {
					t.Fatalf("%s: %v\n%s", c.gccCmd, err, out)
				}
				src, err := os.ReadFile(c.altcSrc)
				if err != nil {
					t.Fatal(err)
				}
				abs, _ := filepath.Abs(c.altcSrc)
				cfg := &driver.Config{Std: "gnu17", NoWarnings: true, NoRegAlloc: len(flags) == 0}
				u := driver.CompileSource(abs, src, withFlags(cfg, isaFlags))
				if u.Obj == nil {
					t.Fatal(u.Diagnostics)
				}
				aobj := filepath.Join(tmp, c.name+".altc.o")
				if err := os.WriteFile(aobj, elf.WriteObject(u.Obj), 0o644); err != nil {
					t.Fatal(err)
				}
				// linked by gcc
				exe := filepath.Join(tmp, c.name+".gcc")
				if out, err := exec.Command(cc, "-o", exe, gobj, aobj, "-pthread").CombinedOutput(); err != nil {
					t.Fatalf("link: %v\n%s", err, out)
				}
				if got, code := run(t, exe, tmp); got != c.want || code != 0 {
					t.Errorf("linked by %s: exit %d\n%s", c.gccCmd, code, got)
				}
				// linked by AltC
				in := &driver.LinkInput{Objects: []*elf.Object{u.Obj}}
				if err := in.AddFile(gobj); err != nil {
					t.Fatal(err)
				}
				for _, l := range c.libs {
					if err := in.AddLib(l, driver.DefaultLibDirs); err != nil {
						t.Skip(err)
					}
				}
				img, diags := driver.Link(in)
				if img == nil {
					t.Fatal(diags)
				}
				exe2 := filepath.Join(tmp, c.name+".altc")
				if err := os.WriteFile(exe2, img, 0o755); err != nil {
					t.Fatal(err)
				}
				if got, code := run(t, exe2, tmp); got != c.want || code != 0 {
					t.Errorf("linked by AltC: exit %d\n%s", code, got)
				}
			})
		}
	}
}

func join(s []string) string {
	r := ""
	for i, x := range s {
		if i > 0 {
			r += " "
		}
		r += x
	}
	return r
}
