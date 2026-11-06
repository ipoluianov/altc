package tests

import (
	"fmt"
	"os"
	"path/filepath"
	"sync"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

// TestRepeatedLinks links programs many times in one process, one after
// the other and at the same time, as the IDE and the test suite do: the
// runtime objects shared by the links (with libgcc's __cpu_model, used by
// both programs) must not keep anything from another link.
func TestRepeatedLinks(t *testing.T) {
	dir := t.TempDir()
	progs := []string{"c/ifunc.c", "c/target_clones.c", "c/big_data.c"}
	want := map[string]string{}
	for i, p := range progs {
		out, ok := runIn(t, buildIn(t, dir, p, fmt.Sprintf("ref%d", i)))
		if !ok {
			return
		}
		want[p] = out
	}
	// the links run at the same time; the programs are written and run
	// afterwards, one by one
	imgs := make([][]byte, 24)
	var wg sync.WaitGroup
	for i := range imgs {
		wg.Add(1)
		go func() {
			defer wg.Done()
			imgs[i] = linkIn(t, progs[i%len(progs)])
		}()
	}
	wg.Wait()
	for i, img := range imgs {
		if img == nil {
			continue
		}
		p := progs[i%len(progs)]
		out, ok := runIn(t, writeExe(t, dir, fmt.Sprintf("p%d", i), img))
		if ok && out != want[p] {
			t.Errorf("%s (link %d): output differs:\n%s\nwant:\n%s", p, i, out, want[p])
		}
	}
}

func runIn(t *testing.T, exe string) (string, bool) {
	if exe == "" {
		return "", false
	}
	out, code := run(t, exe, filepath.Dir(exe))
	if code != 0 {
		t.Errorf("%s: exit %d\n%s", filepath.Base(exe), code, out)
		return "", false
	}
	return out, true
}

// buildIn compiles and links one test program in this process
func buildIn(t *testing.T, dir, file, name string) string {
	img := linkIn(t, file)
	if img == nil {
		return ""
	}
	return writeExe(t, dir, name, img)
}

// linkIn compiles and links a program in memory
func linkIn(t *testing.T, file string) []byte {
	src, _ := filepath.Abs(file)
	data, err := os.ReadFile(src)
	if err != nil {
		t.Error(err)
		return nil
	}
	u := driver.CompileSource(src, data, &driver.Config{Std: "gnu17", NoWarnings: true})
	if u.Obj == nil {
		t.Error(u.Diagnostics)
		return nil
	}
	img, diags := driver.Link(&driver.LinkInput{Objects: []*elf.Object{u.Obj}})
	if img == nil {
		t.Error(diags)
		return nil
	}
	return img
}

func writeExe(t *testing.T, dir, name string, img []byte) string {
	exe := filepath.Join(dir, name)
	if err := os.WriteFile(exe, img, 0o755); err != nil {
		t.Error(err)
		return ""
	}
	return exe
}
