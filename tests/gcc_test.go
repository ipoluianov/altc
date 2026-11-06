package tests

import (
	"io/fs"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strings"
	"sync"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
	"github.com/ipoluianov/altc/libc"
)

// gcc serves only as a reference. It compiles with the headers of the
// built-in C library (-nostdinc, gcc's own headers first as in a native
// musl system, then the C library's), and AltC's linker links its objects
// with the built-in library, so that both programs use the same library.
// The tests that need it are skipped without it.

// scratch holds files shared by the tests; it is removed afterwards
// unless ALTC_TEST_KEEP names a directory to keep it in
var scratch string

func TestMain(m *testing.M) {
	keep := os.Getenv("ALTC_TEST_KEEP")
	dir := keep
	if keep == "" {
		var err error
		if dir, err = os.MkdirTemp("", "altc-tests-"); err != nil {
			panic(err)
		}
	}
	scratch = dir
	code := m.Run()
	if keep == "" {
		os.RemoveAll(dir)
	}
	os.Exit(code)
}

// tempDir is a test's directory: kept under ALTC_TEST_KEEP when set
func tempDir(t *testing.T) string {
	if os.Getenv("ALTC_TEST_KEEP") == "" {
		return t.TempDir()
	}
	d := filepath.Join(scratch, strings.ReplaceAll(t.Name(), "/", "_"))
	os.MkdirAll(d, 0o755)
	return d
}

type gccTool struct {
	path string
	args []string // the include options
}

var (
	gccOnce sync.Once
	gccRef  *gccTool
	gccWhy  string
)

// needGCC returns the reference compiler or skips the test
func needGCC(t *testing.T) *gccTool {
	t.Helper()
	gccOnce.Do(func() {
		p, err := exec.LookPath("gcc")
		if err != nil {
			gccWhy = "gcc not found"
			return
		}
		out, err := exec.Command(p, "-print-file-name=include").Output()
		if err != nil {
			gccWhy = "gcc -print-file-name: " + err.Error()
			return
		}
		inc := filepath.Join(scratch, "include")
		if err := extract(libc.FS, libc.Include, inc); err != nil {
			gccWhy = err.Error()
			return
		}
		gccRef = &gccTool{path: p, args: []string{"-nostdinc", "-isystem", strings.TrimSpace(string(out)), "-isystem", inc}}
	})
	if gccRef == nil {
		t.Skip(gccWhy)
	}
	return gccRef
}

// extract writes the files of dir in fsys to the directory to
func extract(fsys fs.FS, dir, to string) error {
	return fs.WalkDir(fsys, dir, func(p string, d fs.DirEntry, err error) error {
		if err != nil || d.IsDir() {
			return err
		}
		data, err := fs.ReadFile(fsys, p)
		if err != nil {
			return err
		}
		out := filepath.Join(to, filepath.FromSlash(strings.TrimPrefix(p, dir+"/")))
		if err := os.MkdirAll(filepath.Dir(out), 0o755); err != nil {
			return err
		}
		return os.WriteFile(out, data, 0o644)
	})
}

// run runs gcc with the include options
func (g *gccTool) run(t *testing.T, args ...string) {
	t.Helper()
	if out, err := exec.Command(g.path, append(append([]string{}, g.args...), args...)...).CombinedOutput(); err != nil {
		t.Fatalf("gcc %s: %v\n%s", strings.Join(args, " "), err, out)
	}
}

// object compiles src (C or assembler) to the object out
func (g *gccTool) object(t *testing.T, out, src string, flags ...string) string {
	t.Helper()
	g.run(t, append(append([]string{"-c", "-o", out}, flags...), src)...)
	return out
}

// program compiles the sources with gcc and links them with AltC's linker
// and library into the executable exe
func (g *gccTool) program(t *testing.T, exe string, srcs []string, flags ...string) string {
	t.Helper()
	in := &driver.LinkInput{}
	for i, s := range srcs {
		o := strings.TrimSuffix(exe, path.Ext(exe)) + "." + filepath.Base(s) + "." + string(rune('a'+i)) + ".o"
		if err := in.AddFile(g.object(t, o, s, flags...)); err != nil {
			t.Fatal(err)
		}
	}
	return linkTo(t, exe, in)
}

// linkTo links the input into the executable exe
func linkTo(t *testing.T, exe string, in *driver.LinkInput) string {
	t.Helper()
	img, diags := driver.Link(in)
	if img == nil {
		for _, d := range diags {
			t.Error(d)
		}
		t.FailNow()
	}
	if err := os.WriteFile(exe, img, 0o755); err != nil {
		t.Fatal(err)
	}
	return exe
}

// linkObjects links objects into the executable exe
func linkObjects(t *testing.T, exe string, objs ...*elf.Object) string {
	t.Helper()
	return linkTo(t, exe, &driver.LinkInput{Objects: objs})
}
