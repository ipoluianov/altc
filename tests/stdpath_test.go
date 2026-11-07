package tests

import (
	"encoding/hex"
	"fmt"
	"io/fs"
	"math/rand"
	"os"
	"path"
	"path/filepath"
	"strings"
	"testing"
)

func goPath(f []string) string {
	u := func(s string) string { b, _ := hex.DecodeString(s); return string(b) }
	switch f[0] {
	case "lex":
		p := u(f[1])
		d, fl := filepath.Split(p)
		return fmt.Sprintf("%s|%s|%s|%s|%d|%d|%s|%s", filepath.Clean(p), filepath.Base(p), filepath.Dir(p), filepath.Ext(p),
			b2i(filepath.IsAbs(p)), b2i(filepath.IsLocal(p)), d, fl)
	case "join":
		var parts []string
		for _, x := range f[1:] {
			parts = append(parts, u(x))
		}
		return "[" + filepath.Join(parts...) + "]"
	case "match":
		ok, err := path.Match(u(f[1]), u(f[2]))
		if err != nil {
			return "bad"
		}
		return fmt.Sprint(b2i(ok))
	case "rel":
		r, err := filepath.Rel(u(f[1]), u(f[2]))
		if err != nil {
			return "err " + err.Error()
		}
		return r
	case "glob":
		m, err := filepath.Glob(u(f[1]))
		out := ""
		if err != nil {
			out = "bad"
		}
		for _, x := range m {
			out += x + ";"
		}
		return out
	case "walk":
		skip := ""
		if len(f) > 2 {
			skip = u(f[2])
		}
		var out strings.Builder
		filepath.WalkDir(u(f[1]), func(p string, d fs.DirEntry, err error) error {
			dir := ""
			if d != nil && d.IsDir() {
				dir = "/"
			}
			bang := ""
			if err != nil {
				bang = "!"
			}
			out.WriteString(p + dir + bang + ";")
			if d != nil && skip != "" && d.Name() == skip {
				if len(f) > 3 {
					return filepath.SkipAll
				}
				return filepath.SkipDir
			}
			return nil
		})
		return out.String()
	}
	return "unknown"
}

func TestDiffPath(t *testing.T) {
	r := rand.New(rand.NewSource(1003))
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	pieces := []string{"a", "b", ".", "..", "/", "//", "c.txt", ".hidden", "x.tar.gz", "", "é", "...", "a b"}
	rp := func() string {
		var s strings.Builder
		for k := r.Intn(7); k > 0; k-- {
			s.WriteString(pieces[r.Intn(len(pieces))])
			if r.Intn(2) == 0 {
				s.WriteString("/")
			}
		}
		return s.String()
	}
	var in []string
	for i := 0; i < 3000; i++ {
		in = append(in, "lex\t"+h(rp()))
		in = append(in, "join\t"+h(rp())+"\t"+h(rp())+"\t"+h(rp()))
		in = append(in, "rel\t"+h(rp())+"\t"+h(rp()))
	}
	pats := []string{"*", "a*", "*.txt", "?", "a?c", "[a-c]", "[^a]x", "\\*", "a\\", "[", "[]", "[a-]", "[-a]", "*/b", "a/*", "**",
		"[\\]]", "[é]", "?.go", "a[", "*[", "[!a]", "x*y*z"}
	names := []string{"a", "abc", "a.txt", "b", "", "a/b", "x", "é", "*", "]", "-", "xyz", "xaybz", "ab/c"}
	for _, p := range pats {
		for _, nm := range names {
			in = append(in, "match\t"+h(p)+"\t"+h(nm))
		}
	}
	// a tree for Glob and WalkDir
	dir := t.TempDir()
	for _, p := range []string{"a/x.txt", "a/y.go", "a/sub/z.txt", "b/x.txt", "c.txt", "d/e/f/g.txt", ".h"} {
		os.MkdirAll(filepath.Join(dir, filepath.Dir(p)), 0o755)
		os.WriteFile(filepath.Join(dir, p), nil, 0o644)
	}
	os.Symlink("a", filepath.Join(dir, "link"))
	for _, g := range []string{"*", "*/*.txt", "a/*", "*/sub/*", "?.txt", "[ab]/x.txt", "nope/*", "a/[", "*/*/*/*", "link/*", "", "c.txt"} {
		in = append(in, "glob\t"+h(filepath.Join(dir, g)))
	}
	in = append(in, "walk\t"+h(dir), "walk\t"+h(dir)+"\t"+h("sub"), "walk\t"+h(dir)+"\t"+h("b")+"\tall",
		"walk\t"+h(dir)+"\t"+h("x.txt"), "walk\t"+h(filepath.Join(dir, "missing")))
	var want []string
	for _, l := range in {
		want = append(want, goPath(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "path", in), want)
}
