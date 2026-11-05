package tests

import (
	"fmt"
	"os"
	"os/exec"
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
	var wg sync.WaitGroup
	for i := 0; i < 24; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			p := progs[i%len(progs)]
			out, ok := runIn(t, buildIn(t, dir, p, fmt.Sprintf("p%d", i)))
			if ok && out != want[p] {
				t.Errorf("%s (link %d): output differs:\n%s\nwant:\n%s", p, i, out, want[p])
			}
		}()
	}
	wg.Wait()
}

func runIn(t *testing.T, exe string) (string, bool) {
	if exe == "" {
		return "", false
	}
	out, err := exec.Command(exe).CombinedOutput()
	if err != nil {
		t.Errorf("%s: %v\n%s", filepath.Base(exe), err, out)
		return "", false
	}
	return string(out), true
}

// buildIn compiles and links one test program in this process
func buildIn(t *testing.T, dir, file, name string) string {
	src, _ := filepath.Abs(file)
	data, err := os.ReadFile(src)
	if err != nil {
		t.Error(err)
		return ""
	}
	u := driver.CompileSource(src, data, &driver.Config{Std: "gnu17", NoWarnings: true})
	if u.Obj == nil {
		t.Error(u.Diagnostics)
		return ""
	}
	img, diags := driver.Link(&driver.LinkInput{Objects: []*elf.Object{u.Obj}})
	if img == nil {
		t.Error(diags)
		return ""
	}
	exe := filepath.Join(dir, name)
	if err := os.WriteFile(exe, img, 0o755); err != nil {
		t.Error(err)
		return ""
	}
	return exe
}
