package driver

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
	"sync"
	"time"

	"github.com/ipoluianov/altc/elf"
)

// Config returns the compiler configuration of the project
func (p *Project) Config() (*Config, []string, []string, error) {
	cfg := &Config{Std: p.Std, IncludeDirs: p.IncludePaths()}
	for _, d := range p.Defines {
		cfg.Defines = append(cfg.Defines, "D"+d)
	}
	libs := append([]string(nil), p.Libs...)
	var libDirs []string
	for _, d := range p.LibDirs {
		libDirs = append(libDirs, p.abs(d))
	}
	for i := 0; i < len(p.CFlags); i++ {
		f := p.CFlags[i]
		arg := func(prefix string) string {
			if len(f) > len(prefix) {
				return f[len(prefix):]
			}
			if i+1 < len(p.CFlags) {
				i++
				return p.CFlags[i]
			}
			return ""
		}
		switch {
		case strings.HasPrefix(f, "-D"):
			cfg.Defines = append(cfg.Defines, "D"+arg("-D"))
		case strings.HasPrefix(f, "-U"):
			cfg.Defines = append(cfg.Defines, "U"+arg("-U"))
		case strings.HasPrefix(f, "-I"):
			cfg.IncludeDirs = append(cfg.IncludeDirs, p.abs(arg("-I")))
		case strings.HasPrefix(f, "-std="):
			cfg.Std = f[5:]
		case f == "-w":
			cfg.NoWarnings = true
		case f == "-g" || f == "-ggdb" || strings.HasPrefix(f, "-gdwarf"):
			cfg.Debug = true
		case f == "-g0":
			cfg.Debug = false
		case strings.HasPrefix(f, "-m"), strings.HasPrefix(f, "-f"), f == "-Ofast":
			if _, err := cfg.MachineFlag(f); err != nil {
				return nil, nil, nil, err
			}
		case strings.HasPrefix(f, "-l"):
			libs = append(libs, arg("-l"))
		case strings.HasPrefix(f, "-L"):
			libDirs = append(libDirs, p.abs(arg("-L")))
		case strings.HasPrefix(f, "-include"):
			cfg.PreInclude = append(cfg.PreInclude, p.abs(arg("-include")))
		}
	}
	if _, _, err := StdLevel(cfg.Std); err != nil {
		return nil, nil, nil, err
	}
	return cfg, libs, libDirs, nil
}

// Build compiles and links the project into OutputPath().
// Cancelling ctx stops the build as soon as possible.
func (p *Project) Build(ctx context.Context, opt BuildOptions) *BuildResult {
	start := time.Now()
	res := &BuildResult{}
	log := func(format string, args ...any) {
		if opt.Log != nil {
			opt.Log(fmt.Sprintf(format, args...))
		}
	}
	fail := func(msg string) *BuildResult {
		res.Diagnostics = append(res.Diagnostics, Diagnostic{Severity: SevError, Message: msg})
		res.Duration = time.Since(start)
		return res
	}
	cfg, libs, libDirs, err := p.Config()
	if err != nil {
		return fail(err.Error())
	}
	files, err := p.SourceFiles()
	if err != nil {
		return fail(err.Error())
	}
	if len(files) == 0 {
		return fail("no .c files in " + p.Dir)
	}
	jobs := opt.Jobs
	if jobs <= 0 {
		jobs = runtime.NumCPU()
	}
	units := make([]*Unit, len(files))
	var wg sync.WaitGroup
	sem := make(chan struct{}, jobs)
	for i, f := range files {
		wg.Add(1)
		go func(i int, f string) {
			defer wg.Done()
			sem <- struct{}{}
			defer func() { <-sem }()
			if ctx.Err() != nil {
				return
			}
			rel, _ := filepath.Rel(p.Dir, f)
			log("compiling %s", rel)
			data, err := os.ReadFile(f)
			if err != nil {
				units[i] = &Unit{Diagnostics: []Diagnostic{{File: f, Severity: SevError, Message: err.Error()}}}
				return
			}
			units[i] = CompileSource(f, data, cfg)
		}(i, f)
	}
	wg.Wait()
	if ctx.Err() != nil {
		return fail("build cancelled")
	}
	in := &LinkInput{Static: p.Static || slices.Contains(p.CFlags, "-static")}
	for _, u := range units {
		res.Diagnostics = append(res.Diagnostics, u.Diagnostics...)
		if u.Obj != nil {
			in.Objects = append(in.Objects, u.Obj)
		}
	}
	if res.HasErrors() {
		res.Duration = time.Since(start)
		return res
	}
	for _, l := range libs {
		if err := in.AddLib(l, libDirs); err != nil {
			return fail(err.Error())
		}
	}
	out := p.OutputPath()
	rel, _ := filepath.Rel(p.Dir, out)
	log("linking %s", rel)
	img, diags := Link(in)
	res.Diagnostics = append(res.Diagnostics, diags...)
	if img == nil {
		res.Duration = time.Since(start)
		return res
	}
	if err := WriteExecutable(out, img); err != nil {
		return fail(err.Error())
	}
	res.OK = true
	res.Output = out
	res.Duration = time.Since(start)
	log("done in %.2fs", res.Duration.Seconds())
	return res
}

// WriteExecutable writes the image replacing the file atomically (so a
// running copy of the program does not block the build)
func WriteExecutable(path string, img []byte) error {
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
		return err
	}
	tmp := path + ".tmp"
	if err := os.WriteFile(tmp, img, 0o755); err != nil {
		return err
	}
	return os.Rename(tmp, path)
}

var _ = elf.NewObject
