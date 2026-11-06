package tests

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

// TestTLSRelax links gcc -fPIC objects whose thread-local accesses use the
// general and local dynamic models (__tls_get_addr): the linker relaxes
// them to local exec. The reference is the same program compiled without
// -fPIC (local exec from the start).
func TestTLSRelax(t *testing.T) {
	gcc := needGCC(t)
	tmp := t.TempDir()
	src, err := os.ReadFile("tls/altc_part.c")
	if err != nil {
		t.Fatal(err)
	}
	abs, _ := filepath.Abs("tls/altc_part.c")
	u := driver.CompileSource(abs, src, &driver.Config{Std: "gnu17"})
	if u.Obj == nil {
		t.Fatal(u.Diagnostics)
	}
	gccSrcs := []string{"tls/pic_main.c", "tls/lib.c"}
	ref := &driver.LinkInput{}
	for _, s := range append(gccSrcs, "tls/altc_part.c") {
		if err := ref.AddFile(gcc.object(t, filepath.Join(tmp, filepath.Base(s)+".o"), s, "-O2")); err != nil {
			t.Fatal(err)
		}
	}
	want, _ := run(t, linkTo(t, filepath.Join(tmp, "ref"), ref), tmp)
	if want == "" {
		t.Fatal("no reference output")
	}
	in := &driver.LinkInput{Objects: []*elf.Object{u.Obj}}
	for _, s := range gccSrcs {
		if err := in.AddFile(gcc.object(t, filepath.Join(tmp, filepath.Base(s)+".pic.o"), s, "-O2", "-fPIC")); err != nil {
			t.Fatal(err)
		}
	}
	if got, code := run(t, linkTo(t, filepath.Join(tmp, "tls"), in), tmp); got != want || code != 0 {
		t.Errorf("exit %d\ngot:\n%s\nwant:\n%s", code, got, want)
	}
}
