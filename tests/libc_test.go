package tests

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
	"github.com/ipoluianov/altc/libc"
)

// TestLibcAgainstGCC checks the C library compiled by AltC against the
// same sources compiled by gcc with musl's flags: the programs of
// tests/libc (formatted I/O, libm on special values, strings, time,
// regex, iconv, malloc, threads, signals...) must print exactly the same
// with both libraries, compiled by gcc and by AltC.
func TestLibcAgainstGCC(t *testing.T) {
	if testing.Short() {
		t.Skip("builds the C library with gcc")
	}
	gcc := needGCC(t)
	gccLib := gccLibc(t, gcc)
	files, _ := filepath.Glob("libc/*.c")
	if only := os.Getenv("ONLY"); only != "" {
		files, _ = filepath.Glob("libc/" + only + ".c")
	}
	tmp := tempDir(t)
	for _, f := range files {
		name := strings.TrimSuffix(filepath.Base(f), ".c")
		t.Run(name, func(t *testing.T) {
			t.Parallel()
			// -frounding-math: the programs change the rounding mode (without it
			// gcc expands rint inline for the default rounding)
			obj := gcc.object(t, filepath.Join(tmp, name+".gcc.o"), f, "-O2", "-w", "-frounding-math")
			link := func(exe, libcDir string, objs ...*elf.Object) string {
				in := &driver.LinkInput{Objects: objs, LibcDir: libcDir}
				if len(objs) == 0 {
					if err := in.AddFile(obj); err != nil {
						t.Fatal(err)
					}
				}
				return linkTo(t, filepath.Join(tmp, exe), in)
			}
			want, code := run(t, link(name+".ref", gccLib), tmp)
			if code != 0 {
				t.Fatalf("reference (gcc, libc by gcc): exit %d\n%s", code, want)
			}
			src, _ := os.ReadFile(f)
			abs, _ := filepath.Abs(f)
			u := driver.CompileSource(abs, src, testConfig(&driver.Config{Std: "gnu17", NoWarnings: true}))
			if u.Obj == nil {
				t.Fatal(u.Diagnostics)
			}
			for _, c := range []struct{ what, exe string }{
				{"gcc, libc by altc", link(name+".gcc", "")},
				{"altc, libc by altc", link(name+".altc", "", u.Obj)},
			} {
				got, code := run(t, c.exe, tmp)
				if got != want || code != 0 {
					t.Errorf("%s: exit %d, %s", c.what, code, firstDiff(want, got))
				}
			}
		})
	}
}

// firstDiff describes the first differing line of two outputs
func firstDiff(want, got string) string {
	wl, gl := strings.Split(want, "\n"), strings.Split(got, "\n")
	for i := range wl {
		if i >= len(gl) {
			return fmt.Sprintf("output ends at line %d, want %q", i+1, wl[i])
		}
		if wl[i] != gl[i] {
			return fmt.Sprintf("line %d differs:\n got %q\nwant %q", i+1, gl[i], wl[i])
		}
	}
	if len(gl) > len(wl) {
		return fmt.Sprintf("extra output from line %d: %q", len(wl)+1, gl[len(wl)])
	}
	return "same output"
}

var (
	gccLibcOnce sync.Once
	gccLibcDir  string
	gccLibcErr  string
)

// gccLibc builds the C library with gcc (musl's flags) into a library
// directory as the driver uses it
func gccLibc(t *testing.T, gcc *gccTool) string {
	gccLibcOnce.Do(func() {
		src := filepath.Join(scratch, "libcsrc")
		for _, d := range []string{libc.Root, "altc"} {
			if err := extract(libc.FS, d, filepath.Join(src, d)); err != nil {
				gccLibcErr = err.Error()
				return
			}
		}
		incC, _ := filepath.Abs("../include/c")
		inner := []string{"-std=c99", "-nostdinc", "-ffreestanding", "-fexcess-precision=standard", "-frounding-math",
			"-fno-strict-aliasing", "-D_XOPEN_SOURCE=700", "-O2", "-fno-tree-loop-distribute-patterns",
			"-fno-stack-protector", "-w"}
		for _, d := range append(append([]string{}, libc.InternalDirs...), libc.Include) {
			inner = append(inner, "-I", filepath.Join(src, d))
		}
		hosted := append(append([]string{}, gcc.args...), "-isystem", incC, "-std=gnu17", "-O2", "-w")
		srcs := libc.Sources()
		extra := map[string]bool{libc.StartFiles.Start: true}
		for _, f := range libc.Extra() {
			srcs = append(srcs, f)
			extra[f] = true
		}
		crt := []string{libc.StartFiles.Crt1, libc.StartFiles.Crti, libc.StartFiles.Start, libc.StartFiles.Crtn}
		all := append(append([]string{}, srcs...), crt...)
		out := filepath.Join(scratch, "gcclibc")
		os.MkdirAll(filepath.Join(out, "obj"), 0o755)
		objs := make([]string, len(all))
		errs := make([]string, len(all))
		jobs := make(chan int)
		var wg sync.WaitGroup
		for w := 0; w < runtime.NumCPU(); w++ {
			wg.Add(1)
			go func() {
				defer wg.Done()
				for i := range jobs {
					objs[i] = filepath.Join(out, "obj", fmt.Sprintf("%d.o", i))
					args := inner
					if extra[all[i]] {
						args = hosted
					}
					if i >= len(srcs) {
						args = append(append([]string{}, args...), "-DCRT")
					}
					args = append(append([]string{}, args...), "-c", "-o", objs[i], filepath.Join(src, all[i]))
					if b, err := exec.Command(gcc.path, args...).CombinedOutput(); err != nil {
						errs[i] = fmt.Sprintf("%s: %v\n%s", all[i], err, b)
					}
				}
			}()
		}
		for i := range all {
			jobs <- i
		}
		close(jobs)
		wg.Wait()
		for _, e := range errs {
			if e != "" {
				gccLibcErr = e
				return
			}
		}
		var members []elf.ArchiveMember
		for i, s := range srcs {
			data, err := os.ReadFile(objs[i])
			if err != nil {
				gccLibcErr = err.Error()
				return
			}
			members = append(members, elf.ArchiveMember{Name: strings.TrimSuffix(filepath.Base(s), filepath.Ext(s)) + ".o", Data: data})
		}
		ar, err := elf.WriteArchive(members)
		if err == nil {
			err = os.WriteFile(filepath.Join(out, "libc.a"), ar, 0o644)
		}
		for i, n := range driver.CrtNames {
			if err == nil {
				err = os.Rename(objs[len(srcs)+i], filepath.Join(out, n))
			}
		}
		if err != nil {
			gccLibcErr = err.Error()
			return
		}
		gccLibcDir = out
	})
	if gccLibcDir == "" {
		t.Fatal("building the C library with gcc: " + gccLibcErr)
	}
	return gccLibcDir
}
