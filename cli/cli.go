// Package cli is the command-line interface of the compiler
package cli

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"sync"

	"github.com/ipoluianov/altc/app"
	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

const usage = `AltC - a C compiler for linux/amd64

Usage:
  altc build [dir] [-o file] [-j N]   build the project in dir (default: current dir)
  altc run [dir] [-- args...]         build and run the project
  altc init [dir]                     create altc.json and main.c
  altc [options] files...             compile like gcc
  altc ide [dir]                      open the IDE (also: altc without arguments)
  altc version

Options (gcc compatible):
  -o file     output file (default a.out)
  -c          compile to object files only
  -S          output assembly
  -E          preprocess only
  -I dir      add include directory      -isystem dir, -iquote dir
  -D name[=v] define a macro             -U name
  -L dir      add library directory      -l name   link with a library
  -std=STD    c89, c99, c11, c17, c23 (or gnu89...gnu23; default gnu17)
  -include f  include a file first       -nostdinc
  -w          suppress warnings          -s  strip the symbol table
  -static     accepted for compatibility: executables are always static
  -v          verbose
Ignored for compatibility: -O*, -g*, -W*, -f*, -m64, -pipe, -pthread, -MD, -MMD, -MP
`

// Main runs the command line and returns the exit code
func Main(args []string) int {
	if len(args) == 0 {
		fmt.Print(usage)
		return 0
	}
	switch args[0] {
	case "build":
		return cmdBuild(args[1:], false)
	case "run":
		return cmdBuild(args[1:], true)
	case "init":
		return cmdInit(args[1:])
	case "version", "--version":
		fmt.Printf("%s %s\n", app.DisplayName, app.Version)
		return 0
	case "help", "-h", "--help":
		fmt.Print(usage)
		return 0
	}
	// "altc dir" builds a folder
	if len(args) == 1 && !strings.HasPrefix(args[0], "-") {
		if st, err := os.Stat(args[0]); err == nil && st.IsDir() {
			return cmdBuild(args, false)
		}
	}
	return gccMode(args)
}

func printDiags(ds []driver.Diagnostic) {
	for _, d := range ds {
		fmt.Fprintln(os.Stderr, d.String())
	}
}

func cmdBuild(args []string, run bool) int {
	dir := "."
	out := ""
	jobs := 0
	verbose := false
	var runArgs []string
	for i := 0; i < len(args); i++ {
		a := args[i]
		switch {
		case a == "--":
			runArgs = args[i+1:]
			i = len(args)
		case a == "-o" && i+1 < len(args):
			i++
			out = args[i]
		case a == "-j" && i+1 < len(args):
			i++
			jobs, _ = strconv.Atoi(args[i])
		case a == "-v":
			verbose = true
		case strings.HasPrefix(a, "-"):
			fmt.Fprintf(os.Stderr, "altc: unknown option %s\n", a)
			return 2
		default:
			dir = a
		}
	}
	p, err := driver.LoadProject(dir)
	if err != nil {
		fmt.Fprintln(os.Stderr, "altc:", err)
		return 1
	}
	if out != "" {
		abs, _ := filepath.Abs(out)
		p.Output = abs
	}
	opt := driver.BuildOptions{Jobs: jobs}
	if verbose {
		opt.Log = func(s string) { fmt.Fprintln(os.Stderr, s) }
	}
	res := p.Build(context.Background(), opt)
	printDiags(res.Diagnostics)
	if !res.OK {
		return 1
	}
	if !run {
		if verbose {
			fmt.Fprintln(os.Stderr, "built", res.Output)
		}
		return 0
	}
	if len(runArgs) == 0 {
		runArgs = p.Args
	}
	return runProgram(res.Output, runArgs, p.Dir)
}

func runProgram(path string, args []string, dir string) int {
	cmd := exec.Command(path, args...)
	cmd.Stdin, cmd.Stdout, cmd.Stderr = os.Stdin, os.Stdout, os.Stderr
	cmd.Dir = dir
	signal.Ignore(os.Interrupt)
	err := cmd.Run()
	if ee, ok := err.(*exec.ExitError); ok {
		return ee.ExitCode()
	}
	if err != nil {
		fmt.Fprintln(os.Stderr, "altc:", err)
		return 1
	}
	return 0
}

func cmdInit(args []string) int {
	dir := "."
	if len(args) > 0 {
		dir = args[0]
	}
	if err := os.MkdirAll(dir, 0o755); err != nil {
		fmt.Fprintln(os.Stderr, "altc:", err)
		return 1
	}
	p, err := driver.LoadProject(dir)
	if err != nil {
		fmt.Fprintln(os.Stderr, "altc:", err)
		return 1
	}
	if _, err := os.Stat(filepath.Join(p.Dir, driver.ProjectFileName)); err != nil {
		p.Std = "c17"
		if err := p.Save(); err != nil {
			fmt.Fprintln(os.Stderr, "altc:", err)
			return 1
		}
	}
	main := filepath.Join(p.Dir, "main.c")
	if _, err := os.Stat(main); err != nil {
		os.WriteFile(main, []byte("#include <stdio.h>\n\nint main(void) {\n    printf(\"Hello, world!\\n\");\n    return 0;\n}\n"), 0o644)
	}
	fmt.Println("created project in", p.Dir)
	return 0
}

// gccMode compiles files with gcc-like options
func gccMode(args []string) int {
	cfg := &driver.Config{}
	var inputs []string
	var libs, libDirs []string
	out := ""
	compileOnly, asmOnly, ppOnly, strip, verbose := false, false, false, false, false
	for i := 0; i < len(args); i++ {
		a := args[i]
		next := func(prefix string) (string, bool) {
			if len(a) > len(prefix) {
				return strings.TrimPrefix(a[len(prefix):], "="), true
			}
			if i+1 < len(args) {
				i++
				return args[i], true
			}
			fmt.Fprintf(os.Stderr, "altc: missing argument to '%s'\n", a)
			return "", false
		}
		var v string
		ok := true
		switch {
		case a == "-o":
			out, ok = next("-o")
		case strings.HasPrefix(a, "-o"):
			out = a[2:]
		case a == "-c":
			compileOnly = true
		case a == "-S":
			asmOnly = true
		case a == "-E":
			ppOnly = true
		case a == "-s":
			strip = true
		case a == "-v":
			verbose = true
		case a == "-w":
			cfg.NoWarnings = true
		case a == "-fpermissive":
			cfg.Permissive = true
		case a == "-nostdinc":
			cfg.NoStdInc = true
		case strings.HasPrefix(a, "-std="):
			cfg.Std = a[5:]
		case a == "-ansi":
			cfg.Std = "c89"
		case strings.HasPrefix(a, "-isystem"):
			v, ok = next("-isystem")
			cfg.SystemDirs = append(cfg.SystemDirs, v)
		case strings.HasPrefix(a, "-iquote"):
			v, ok = next("-iquote")
			cfg.QuoteDirs = append(cfg.QuoteDirs, v)
		case strings.HasPrefix(a, "-include"):
			v, ok = next("-include")
			cfg.PreInclude = append(cfg.PreInclude, v)
		case strings.HasPrefix(a, "-I"):
			v, ok = next("-I")
			cfg.IncludeDirs = append(cfg.IncludeDirs, v)
		case strings.HasPrefix(a, "-D"):
			v, ok = next("-D")
			cfg.Defines = append(cfg.Defines, "D"+v)
		case strings.HasPrefix(a, "-U"):
			v, ok = next("-U")
			cfg.Defines = append(cfg.Defines, "U"+v)
		case strings.HasPrefix(a, "-L"):
			v, ok = next("-L")
			libDirs = append(libDirs, v)
		case strings.HasPrefix(a, "-l"):
			v, ok = next("-l")
			libs = append(libs, v)
			inputs = append(inputs, "-l"+v)
		case a == "-MF" || a == "-MT" || a == "-MQ" || a == "-x":
			i++
		case a == "-static":
			// executables are always static
		case a == "-shared":
			fmt.Fprintf(os.Stderr, "altc: %s is not supported\n", a)
			return 1
		case a == "-O0":
			cfg.NoRegAlloc = true
		case a == "-g0":
			cfg.Debug = false
		case a == "-g" || a == "-ggdb" || a == "-g1" || a == "-g2" || a == "-g3" || a == "-ggdb3" || strings.HasPrefix(a, "-gdwarf"):
			cfg.Debug = true
		case strings.HasPrefix(a, "-m") && a != "-m64", a == "-Ofast", strings.HasPrefix(a, "-f") && strings.HasSuffix(a, "-math"),
			a == "-funsafe-math-optimizations", a == "-fno-unsafe-math-optimizations":
			if a == "-Ofast" {
				cfg.NoRegAlloc = false
			}
			if _, err := cfg.MachineFlag(a); err != nil {
				fmt.Fprintf(os.Stderr, "altc: %v\n", err)
				return 2
			}
		case strings.HasPrefix(a, "-O"), strings.HasPrefix(a, "-g"), strings.HasPrefix(a, "-W"), strings.HasPrefix(a, "-f"),
			strings.HasPrefix(a, "-m"), a == "-pipe", a == "-pthread", a == "-MD", a == "-MMD", a == "-MP", a == "-rdynamic",
			a == "-no-pie", a == "-pie", strings.HasPrefix(a, "--param"), a == "-pedantic", a == "-pedantic-errors":
		case strings.HasPrefix(a, "-") && a != "-":
			fmt.Fprintf(os.Stderr, "altc: unknown option '%s'\n", a)
			return 2
		default:
			inputs = append(inputs, a)
		}
		if !ok {
			return 2
		}
	}
	var sources []string
	for _, in := range inputs {
		if driver.IsSource(in) || in == "-" {
			sources = append(sources, in)
		}
	}
	if len(inputs) == 0 {
		fmt.Fprintln(os.Stderr, "altc: no input files")
		return 1
	}
	if (compileOnly || asmOnly || ppOnly) && out != "" && len(sources) > 1 {
		fmt.Fprintln(os.Stderr, "altc: cannot specify -o with -c, -S or -E with multiple files")
		return 1
	}
	readSrc := func(path string) ([]byte, error) {
		if path == "-" {
			return readAll(os.Stdin)
		}
		return os.ReadFile(path)
	}
	failed := false
	if ppOnly {
		for _, s := range sources {
			data, err := readSrc(s)
			if err != nil {
				fmt.Fprintln(os.Stderr, "altc:", err)
				return 1
			}
			text, diags := driver.Preprocess(s, data, cfg)
			printDiags(diags)
			if hasErr(diags) {
				failed = true
				continue
			}
			if out != "" && out != "-" {
				os.WriteFile(out, []byte(text), 0o644)
			} else {
				fmt.Print(text)
			}
		}
		if failed {
			return 1
		}
		return 0
	}
	cfg.Listing = asmOnly
	// compile all sources in parallel; results are used in input order
	units := map[string]*driver.Unit{}
	var mu sync.Mutex
	var wg sync.WaitGroup
	sem := make(chan struct{}, runtime.NumCPU())
	for _, in := range sources {
		if in == "-" {
			continue
		}
		wg.Add(1)
		go func(in string) {
			defer wg.Done()
			sem <- struct{}{}
			defer func() { <-sem }()
			data, err := os.ReadFile(in)
			var u *driver.Unit
			if err != nil {
				u = &driver.Unit{Diagnostics: []driver.Diagnostic{{Severity: driver.SevError, Message: err.Error()}}}
			} else {
				u = driver.CompileSource(in, data, cfg)
			}
			mu.Lock()
			units[in] = u
			mu.Unlock()
		}(in)
	}
	wg.Wait()
	link := &driver.LinkInput{}
	for _, in := range inputs {
		switch {
		case strings.HasPrefix(in, "-l"):
			if compileOnly || asmOnly {
				continue
			}
			if err := link.AddLib(in[2:], libDirs); err != nil {
				fmt.Fprintln(os.Stderr, "altc:", err)
				failed = true
			}
		case driver.IsSource(in) || in == "-":
			if verbose {
				fmt.Fprintln(os.Stderr, "compiling", in)
			}
			u := units[in]
			if u == nil {
				data, err := readSrc(in)
				if err != nil {
					fmt.Fprintln(os.Stderr, "altc:", err)
					failed = true
					continue
				}
				u = driver.CompileSource(in, data, cfg)
			}
			printDiags(u.Diagnostics)
			if u.Obj == nil {
				failed = true
				continue
			}
			base := strings.TrimSuffix(filepath.Base(in), filepath.Ext(in))
			switch {
			case asmOnly && strings.HasSuffix(in, ".s"):
				// already assembly: nothing to do (like gcc)
			case asmOnly:
				dst := out
				if dst == "" {
					dst = base + ".s"
				}
				if dst == "-" {
					fmt.Print(u.Listing)
				} else if err := os.WriteFile(dst, []byte(u.Listing), 0o644); err != nil {
					fmt.Fprintln(os.Stderr, "altc:", err)
					failed = true
				}
			case compileOnly:
				dst := out
				if dst == "" {
					dst = base + ".o"
				}
				if err := os.WriteFile(dst, elf.WriteObject(u.Obj), 0o644); err != nil {
					fmt.Fprintln(os.Stderr, "altc:", err)
					failed = true
				}
			default:
				link.Objects = append(link.Objects, u.Obj)
			}
		default:
			if compileOnly || asmOnly {
				continue
			}
			if err := link.AddFile(in); err != nil {
				fmt.Fprintln(os.Stderr, "altc:", err)
				failed = true
			}
		}
	}
	if failed {
		return 1
	}
	if compileOnly || asmOnly {
		return 0
	}
	if out == "" {
		out = "a.out"
	}
	if verbose {
		fmt.Fprintln(os.Stderr, "linking", out)
	}
	if err := driver.PrepareLibc(func(s string) { fmt.Fprintln(os.Stderr, "altc:", s) }); err != nil {
		fmt.Fprintln(os.Stderr, "altc:", err)
		return 1
	}
	link.Strip = strip
	img, diags := driver.Link(link)
	printDiags(diags)
	if img == nil {
		return 1
	}
	if err := driver.WriteExecutable(out, img); err != nil {
		fmt.Fprintln(os.Stderr, "altc:", err)
		return 1
	}
	return 0
}

func hasErr(ds []driver.Diagnostic) bool {
	for _, d := range ds {
		if d.Severity == driver.SevError {
			return true
		}
	}
	return false
}

func readAll(f *os.File) ([]byte, error) {
	var b []byte
	buf := make([]byte, 65536)
	for {
		n, err := f.Read(buf)
		b = append(b, buf[:n]...)
		if err != nil {
			if err.Error() == "EOF" {
				return b, nil
			}
			return b, err
		}
	}
}
