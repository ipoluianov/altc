package driver

import (
	"fmt"
	"os"
	"path"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"strings"
	"sync"

	"github.com/ipoluianov/altc/elf"
	"github.com/ipoluianov/altc/std"
)

// AltC's standard library (package std) is compiled like the C library:
// on first use, by this compiler, into the build cache (libalt.a).

// stdVirtual is the directory under which the compiler sees the
// library's sources while building it
const stdVirtual = "/altc/std"

var (
	stdOnce sync.Once
	stdLib  []byte
	stdErr  error
)

// getStd returns libalt.a, building it into the cache when missing
func getStd(log func(string)) ([]byte, error) {
	stdOnce.Do(func() { stdLib, stdErr = loadStd(log) })
	return stdLib, stdErr
}

func loadStd(log func(string)) ([]byte, error) {
	cache, err := CacheDir()
	if err != nil {
		return nil, err
	}
	dir := filepath.Join(cache, "std", "alt-"+compilerKey())
	file := filepath.Join(dir, "libalt.a")
	if data, err := os.ReadFile(file); err == nil {
		return data, nil
	}
	if log != nil {
		log("building the standard library (once) into " + dir)
	}
	ar, err := compileStd()
	if err != nil {
		return nil, err
	}
	if err := os.MkdirAll(filepath.Dir(dir), 0o755); err != nil {
		return nil, err
	}
	tmp, err := os.MkdirTemp(filepath.Dir(dir), ".tmp-")
	if err != nil {
		return nil, err
	}
	defer os.RemoveAll(tmp)
	if err := os.WriteFile(filepath.Join(tmp, "libalt.a"), ar, 0o644); err != nil {
		return nil, err
	}
	if err := os.Rename(tmp, dir); err != nil {
		if _, e := os.Stat(dir); e != nil {
			return nil, err
		}
	}
	pruneLibcs(filepath.Dir(dir), dir)
	return os.ReadFile(file)
}

// StdConfig is how the standard library is compiled
func StdConfig() *Config {
	return &Config{Std: "gnu17", ReadFile: libcReadFile}
}

// StdSources are the virtual paths of the library's sources, as the
// compiler sees them (read through Config.ReadFile of StdConfig)
func StdSources() []string {
	var out []string
	for _, s := range std.Sources() {
		out = append(out, stdVirtual+"/"+s)
	}
	return out
}

// compileStd compiles the library into an archive
func compileStd() ([]byte, error) {
	srcs := StdSources()
	objs := make([][]byte, len(srcs))
	errs := make([]string, len(srcs))
	cfg := StdConfig()
	defer debug.SetGCPercent(debug.SetGCPercent(400))
	jobs := make(chan int)
	var wg sync.WaitGroup
	for w := 0; w < runtime.NumCPU(); w++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for i := range jobs {
				data, err := libcReadFile(srcs[i])
				if err != nil {
					errs[i] = err.Error()
					continue
				}
				c := *cfg
				u := CompileSource(srcs[i], data, &c)
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
	for i := range srcs {
		jobs <- i
	}
	close(jobs)
	wg.Wait()
	var failed []string
	for _, e := range errs {
		if e != "" {
			failed = append(failed, e)
		}
	}
	if len(failed) > 0 {
		return nil, fmt.Errorf("building the standard library failed:\n%s", strings.Join(failed, "\n"))
	}
	members := make([]elf.ArchiveMember, len(srcs))
	for i, s := range srcs {
		rel := strings.TrimPrefix(s, stdVirtual+"/src/")
		members[i] = elf.ArchiveMember{Name: strings.TrimSuffix(rel, path.Ext(rel)) + ".o", Data: objs[i]}
	}
	return elf.WriteArchive(members)
}
