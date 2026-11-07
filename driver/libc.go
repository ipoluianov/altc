package driver

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"io"
	"os"
	"path"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/ipoluianov/altc/elf"
	"github.com/ipoluianov/altc/libc"
	"github.com/ipoluianov/altc/std"
)

// The C library (musl, package libc) is compiled by the compiler itself on
// first use and kept in the build cache, under a key that changes with
// every build of the compiler: a cached library is always the one this
// compiler produces.

// libcVirtual is the directory under which the compiler sees the musl
// sources while building the library (read from the embedded files)
const libcVirtual = "/altc/libc"

// libcReadFile reads files of the virtual musl tree, others from disk
func libcReadFile(p string) ([]byte, error) {
	s := filepath.ToSlash(p)
	if rest, ok := strings.CutPrefix(s, libcVirtual+"/"); ok {
		return libc.FS.ReadFile(path.Clean(rest))
	}
	if rest, ok := strings.CutPrefix(s, stdVirtual+"/"); ok {
		return std.FS.ReadFile(path.Clean(rest))
	}
	return os.ReadFile(p)
}

// virtualPath is the virtual path of a file of the libc FS
func virtualPath(p string) string { return libcVirtual + "/" + p }

// CacheDir is the build cache: $ALTC_CACHE, else altc in the user's cache
// directory (~/.cache, %LocalAppData%, ~/Library/Caches)
func CacheDir() (string, error) {
	if d := os.Getenv("ALTC_CACHE"); d != "" {
		return d, nil
	}
	d, err := os.UserCacheDir()
	if err != nil {
		return "", fmt.Errorf("no cache directory (set ALTC_CACHE): %v", err)
	}
	return filepath.Join(d, "altc"), nil
}

var (
	buildKeyOnce sync.Once
	buildKey     string
)

// compilerKey identifies this build of the compiler: the hash of its
// executable
func compilerKey() string {
	buildKeyOnce.Do(func() {
		h := sha256.New()
		if exe, err := os.Executable(); err == nil {
			if f, err := os.Open(exe); err == nil {
				io.Copy(h, f)
				f.Close()
			}
		}
		buildKey = hex.EncodeToString(h.Sum(nil))[:20]
	})
	return buildKey
}

// libcConfig is how the library is compiled: musl's flags (-std=c99
// -ffreestanding -nostdinc -D_XOPEN_SOURCE=700, its include directories)
func libcConfig() *Config {
	cfg := &Config{Std: "c99", NoStdInc: true, NoWarnings: true, ReadFile: libcReadFile}
	for _, d := range libc.Defines {
		cfg.Defines = append(cfg.Defines, "D"+d)
	}
	for _, d := range append(append([]string{}, libc.InternalDirs...), libc.Include) {
		cfg.SystemDirs = append(cfg.SystemDirs, virtualPath(d))
	}
	return cfg
}

// builtLibc is the compiled library
type builtLibc struct {
	archive                 []byte // libc.a
	crt1, crti, start, crtn *elf.Object
}

var (
	libcOnce sync.Once
	libcRes  *builtLibc
	libcErr  error
)

// PrepareLibc makes the C library and AltC's standard library ready,
// building them into the cache when they are not there yet (seconds, once
// for every build of the compiler); log, when not nil, is told before
// building
func PrepareLibc(log func(string)) error {
	if _, err := getLibc(log); err != nil {
		return err
	}
	_, err := getStd(log)
	return err
}

// getLibc returns the library, building it into the cache when missing
func getLibc(log func(string)) (*builtLibc, error) {
	libcOnce.Do(func() { libcRes, libcErr = loadLibc(log) })
	return libcRes, libcErr
}

// CrtNames are the startup objects in a library directory, in the order
// of libc.StartFiles
var CrtNames = []string{"crt1.o", "crti.o", "altcstart.o", "crtn.o"}

func loadLibc(log func(string)) (*builtLibc, error) {
	cache, err := CacheDir()
	if err != nil {
		return nil, err
	}
	dir := filepath.Join(cache, "libc", "musl-"+libc.Version()+"-"+compilerKey())
	if lib, err := readLibc(dir); err == nil {
		// the time of use, for pruning (at most once an hour)
		if st, err := os.Stat(dir); err == nil && time.Since(st.ModTime()) > time.Hour {
			now := time.Now()
			os.Chtimes(dir, now, now)
		}
		return lib, nil
	}
	if log != nil {
		log(fmt.Sprintf("building the C library (musl %s, once) into %s", libc.Version(), dir))
	}
	files, err := compileLibc()
	if err != nil {
		return nil, err
	}
	// written to a temporary directory and renamed: another process may
	// build the same library at the same time
	if err := os.MkdirAll(filepath.Dir(dir), 0o755); err != nil {
		return nil, err
	}
	tmp, err := os.MkdirTemp(filepath.Dir(dir), ".tmp-")
	if err != nil {
		return nil, err
	}
	defer os.RemoveAll(tmp)
	for name, data := range files {
		if err := os.WriteFile(filepath.Join(tmp, name), data, 0o644); err != nil {
			return nil, err
		}
	}
	if err := os.Rename(tmp, dir); err != nil {
		if _, e := os.Stat(dir); e != nil {
			return nil, err
		}
		// built by another process meanwhile
	}
	pruneLibcs(filepath.Dir(dir), dir)
	return readLibc(dir)
}

// keptLibcs is how many libraries of other compiler builds stay in the
// cache (the most recently used ones)
const keptLibcs = 3

// pruneLibcs removes the libraries of other compiler builds but the most
// recently used ones
func pruneLibcs(parent, current string) {
	ents, err := os.ReadDir(parent)
	if err != nil {
		return
	}
	type entry struct {
		path string
		used time.Time
	}
	var old []entry
	for _, e := range ents {
		p := filepath.Join(parent, e.Name())
		if !e.IsDir() || p == current {
			continue
		}
		if st, err := e.Info(); err == nil {
			old = append(old, entry{p, st.ModTime()})
		}
	}
	sort.Slice(old, func(i, j int) bool { return old[i].used.After(old[j].used) })
	for i, e := range old {
		// temporary directories of builds that did not finish: after a day
		if i >= keptLibcs && !strings.HasPrefix(filepath.Base(e.path), ".tmp-") || time.Since(e.used) > 24*time.Hour {
			os.RemoveAll(e.path)
		}
	}
}

func readLibc(dir string) (*builtLibc, error) {
	lib := &builtLibc{}
	data, err := os.ReadFile(filepath.Join(dir, "libc.a"))
	if err != nil {
		return nil, err
	}
	lib.archive = data
	for i, p := range []**elf.Object{&lib.crt1, &lib.crti, &lib.start, &lib.crtn} {
		f := filepath.Join(dir, CrtNames[i])
		data, err := os.ReadFile(f)
		if err != nil {
			return nil, err
		}
		if *p, err = elf.ReadObject(f, data); err != nil {
			return nil, err
		}
	}
	return lib, nil
}

// compileLibc compiles the library: libc.a and the startup objects
func compileLibc() (map[string][]byte, error) {
	srcs := append(libc.Sources(), libc.Extra()...)
	crt := []string{libc.StartFiles.Crt1, libc.StartFiles.Crti, libc.StartFiles.Start, libc.StartFiles.Crtn}
	all := append(append([]string{}, srcs...), crt...)
	extra := map[string]bool{}
	for _, f := range append(libc.Extra(), libc.StartFiles.Start) {
		extra[f] = true
	}
	objs := make([][]byte, len(all))
	errs := make([]string, len(all))
	cfg := libcConfig()
	// many small compilations at once spend most of the time collecting
	// garbage at the default rate: twice as fast with GOGC=400
	defer debug.SetGCPercent(debug.SetGCPercent(400))
	jobs := make(chan int)
	var wg sync.WaitGroup
	for w := 0; w < runtime.NumCPU(); w++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for i := range jobs {
				c := *cfg
				if extra[all[i]] {
					c = Config{Std: "gnu17", NoWarnings: true, ReadFile: libcReadFile}
				}
				if i >= len(srcs) {
					c.Defines = append(append([]string{}, c.Defines...), "DCRT")
				}
				p := virtualPath(all[i])
				data, err := libcReadFile(p)
				if err != nil {
					errs[i] = err.Error()
					continue
				}
				u := CompileSource(p, data, &c)
				if u.Obj == nil {
					var msgs []string
					for _, d := range u.Diagnostics {
						if d.Severity == SevError {
							msgs = append(msgs, d.String())
						}
					}
					errs[i] = strings.Join(msgs, "\n")
					continue
				}
				objs[i] = elf.WriteObject(u.Obj)
			}
		}()
	}
	for i := range all {
		jobs <- i
	}
	close(jobs)
	wg.Wait()
	var failed []string
	for i, e := range errs {
		if e != "" {
			failed = append(failed, e)
			if len(failed) == 10 {
				failed = append(failed, fmt.Sprintf("... (%d files)", len(all)-i))
				break
			}
		}
	}
	if len(failed) > 0 {
		return nil, fmt.Errorf("building the C library failed:\n%s", strings.Join(failed, "\n"))
	}
	members := make([]elf.ArchiveMember, len(srcs))
	for i, s := range srcs {
		rel := strings.TrimPrefix(strings.TrimPrefix(s, libc.Root+"/"), "src/")
		members[i] = elf.ArchiveMember{Name: strings.TrimSuffix(rel, path.Ext(rel)) + ".o", Data: objs[i]}
	}
	ar, err := elf.WriteArchive(members)
	if err != nil {
		return nil, err
	}
	files := map[string][]byte{"libc.a": ar}
	for i, n := range CrtNames {
		files[n] = objs[len(srcs)+i]
	}
	return files, nil
}
