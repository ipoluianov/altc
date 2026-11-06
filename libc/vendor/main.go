// Command vendor copies a musl release into libc/musl in the layout the
// compiler embeds: x86_64 only, the public headers installed (include/
// holds the generic and x86_64 bits and the generated alltypes.h and
// syscall.h, as "make install-headers" does), src/internal/version.h
// generated. The patches in libc/patches are applied afterwards with
// git apply.
//
//	go run ./libc/vendor /path/to/musl-1.2.5
package main

import (
	"fmt"
	"io/fs"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strings"
)

const arch = "x86_64"

// architectures whose directories are dropped
var otherArchs = map[string]bool{
	"aarch64": true, "arm": true, "i386": true, "loongarch64": true, "m68k": true, "microblaze": true,
	"mips": true, "mips64": true, "mipsn32": true, "or1k": true, "powerpc": true, "powerpc64": true,
	"riscv32": true, "riscv64": true, "s390x": true, "sh": true, "x32": true,
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "usage: go run ./libc/vendor <musl source dir>")
		os.Exit(2)
	}
	src := os.Args[1]
	dst := filepath.Join("libc", "musl")
	if _, err := os.Stat(filepath.Join("libc", "libc.go")); err != nil {
		fail(fmt.Errorf("run from the repository root"))
	}
	check(os.RemoveAll(dst))
	for _, f := range []string{"COPYRIGHT", "VERSION", "README"} {
		copyFile(filepath.Join(src, f), filepath.Join(dst, f))
	}
	// sources: everything of src/ but other architectures and the old malloc
	copyTree(filepath.Join(src, "src"), filepath.Join(dst, "src"), func(rel string, d fs.DirEntry) bool {
		if d.IsDir() {
			return !otherArchs[d.Name()] && rel != "malloc/oldmalloc"
		}
		return true
	})
	// startup files of static executables
	copyFile(filepath.Join(src, "crt", "crt1.c"), filepath.Join(dst, "crt", "crt1.c"))
	for _, f := range []string{"crti.s", "crtn.s"} {
		copyFile(filepath.Join(src, "crt", arch, f), filepath.Join(dst, "crt", arch, f))
	}
	// internal architecture headers (the bits go to include/)
	for _, a := range []string{arch, "generic"} {
		copyTree(filepath.Join(src, "arch", a), filepath.Join(dst, "arch", a), func(rel string, d fs.DirEntry) bool {
			return rel != "bits"
		})
	}
	// installed headers
	inc := filepath.Join(dst, "include")
	copyTree(filepath.Join(src, "include"), inc, func(rel string, d fs.DirEntry) bool {
		return !strings.HasSuffix(rel, ".in")
	})
	for _, a := range []string{"generic", arch} { // the architecture's bits replace the generic ones
		copyTree(filepath.Join(src, "arch", a, "bits"), filepath.Join(inc, "bits"), func(rel string, d fs.DirEntry) bool {
			return !strings.HasSuffix(rel, ".in")
		})
	}
	writeFile(filepath.Join(inc, "bits", "alltypes.h"), allTypes(
		read(filepath.Join(src, "arch", arch, "bits", "alltypes.h.in"))+read(filepath.Join(src, "include", "alltypes.h.in"))))
	sc := read(filepath.Join(src, "arch", arch, "bits", "syscall.h.in"))
	var sys strings.Builder
	sys.WriteString(sc)
	for _, l := range strings.SplitAfter(sc, "\n") {
		if strings.Contains(l, "__NR_") {
			sys.WriteString(strings.Replace(l, "__NR_", "SYS_", 1))
		}
	}
	writeFile(filepath.Join(inc, "bits", "syscall.h"), sys.String())
	version := strings.TrimSpace(read(filepath.Join(src, "VERSION")))
	writeFile(filepath.Join(dst, "src", "internal", "version.h"), fmt.Sprintf("#define VERSION \"%s\"\n", version))

	patches, _ := filepath.Glob(filepath.Join("libc", "patches", "*.patch"))
	sort.Strings(patches)
	for _, p := range patches {
		cmd := exec.Command("git", "apply", "--directory="+filepath.ToSlash(dst), p)
		cmd.Stdout, cmd.Stderr = os.Stdout, os.Stderr
		if err := cmd.Run(); err != nil {
			fail(fmt.Errorf("%s: %v", p, err))
		}
		fmt.Println("applied", p)
	}
	fmt.Println("musl", version, "vendored into", dst)
}

var (
	typedefRe = regexp.MustCompile(`^TYPEDEF (.*) ([^ ]*);$`)
	structRe  = regexp.MustCompile(`^(STRUCT|UNION) * ([^ ]*) (.*);$`)
)

// allTypes does what musl's tools/mkalltypes.sed does
func allTypes(in string) string {
	var b strings.Builder
	for _, l := range strings.SplitAfter(in, "\n") {
		line := strings.TrimSuffix(l, "\n")
		if m := typedefRe.FindStringSubmatch(line); m != nil {
			fmt.Fprintf(&b, "#if defined(__NEED_%[2]s) && !defined(__DEFINED_%[2]s)\ntypedef %[1]s %[2]s;\n#define __DEFINED_%[2]s\n#endif\n\n", m[1], m[2])
			continue
		}
		if m := structRe.FindStringSubmatch(line); m != nil {
			kw := strings.ToLower(m[1])
			fmt.Fprintf(&b, "#if defined(__NEED_%[1]s_%[2]s) && !defined(__DEFINED_%[1]s_%[2]s)\n%[1]s %[2]s %[3]s;\n#define __DEFINED_%[1]s_%[2]s\n#endif\n\n", kw, m[2], m[3])
			continue
		}
		b.WriteString(l)
	}
	return b.String()
}

func copyTree(from, to string, keep func(rel string, d fs.DirEntry) bool) {
	check(filepath.WalkDir(from, func(p string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		rel, _ := filepath.Rel(from, p)
		if rel == "." {
			return nil
		}
		if !keep(filepath.ToSlash(rel), d) {
			if d.IsDir() {
				return filepath.SkipDir
			}
			return nil
		}
		if !d.IsDir() {
			copyFile(p, filepath.Join(to, rel))
		}
		return nil
	}))
}

func copyFile(from, to string) { writeFile(to, read(from)) }

func read(p string) string {
	data, err := os.ReadFile(p)
	check(err)
	return string(data)
}

func writeFile(p, data string) {
	check(os.MkdirAll(filepath.Dir(p), 0o755))
	check(os.WriteFile(p, []byte(data), 0o644))
}

func check(err error) {
	if err != nil {
		fail(err)
	}
}

func fail(err error) {
	fmt.Fprintln(os.Stderr, "vendor:", err)
	os.Exit(1)
}
