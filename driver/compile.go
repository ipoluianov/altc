package driver

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime/debug"
	"sort"
	"strconv"
	"strings"
	"sync"

	"github.com/ipoluianov/altc/cc"
	"github.com/ipoluianov/altc/elf"
	"github.com/ipoluianov/altc/include"
	"github.com/ipoluianov/altc/x64"
)

// Config holds the options of a compilation
type Config struct {
	Std         string   // "c89", "c99", "c11", "c17", "c23", "gnu17"...; default gnu17
	IncludeDirs []string // -I
	QuoteDirs   []string // -iquote
	SystemDirs  []string // -isystem
	Defines     []string // "DNAME=VALUE" or "UNAME" in command-line order
	PreInclude  []string // -include
	NoStdInc    bool     // -nostdinc: no built-in and no system headers
	NoWarnings  bool     // -w
	Listing     bool     // produce assembler text (-S)
}

// StdLevel converts a -std= value to the numeric level and dialect
func StdLevel(std string) (int, bool, error) {
	s := strings.ToLower(std)
	gnu := false
	switch {
	case s == "":
		return 2017, true, nil
	case strings.HasPrefix(s, "gnu"):
		gnu = true
		s = "c" + s[3:]
	case strings.HasPrefix(s, "iso9899:"):
		switch s[8:] {
		case "1990", "199409":
			s = "c89"
		case "1999":
			s = "c99"
		case "2011":
			s = "c11"
		case "2017", "2018":
			s = "c17"
		case "2024":
			s = "c23"
		}
	}
	switch s {
	case "c89", "c90", "ansi":
		return 1989, gnu, nil
	case "c99", "c9x":
		return 1999, gnu, nil
	case "c11", "c1x":
		return 2011, gnu, nil
	case "c17", "c18":
		return 2017, gnu, nil
	case "c23", "c2x", "c2y":
		return 2023, gnu, nil
	}
	return 0, false, fmt.Errorf("unknown C standard '%s'", std)
}

// systemIncludeDirs are searched after the built-in headers
var systemIncludeDirs = []string{"/usr/local/include", "/usr/include/x86_64-linux-gnu", "/usr/include"}

func (c *Config) ppConfig() (cc.PPConfig, error) {
	level, gnu, err := StdLevel(c.Std)
	if err != nil {
		return cc.PPConfig{}, err
	}
	pc := cc.PPConfig{Std: level, GNU: gnu, QuoteDirs: c.QuoteDirs, Defines: c.Defines, PreInclude: c.PreInclude}
	for _, d := range c.IncludeDirs {
		pc.Dirs = append(pc.Dirs, cc.IncludeDir{Path: d})
	}
	for _, d := range c.SystemDirs {
		pc.Dirs = append(pc.Dirs, cc.IncludeDir{Path: d, System: true})
	}
	if !c.NoStdInc {
		pc.Dirs = append(pc.Dirs, cc.IncludeDir{Path: include.Root, Embedded: include.FS, System: true})
		for _, d := range systemIncludeDirs {
			pc.Dirs = append(pc.Dirs, cc.IncludeDir{Path: d, System: true})
		}
	}
	return pc, nil
}

func toDiags(errs []*cc.Error, noWarn bool) []Diagnostic {
	var ds []Diagnostic
	for _, e := range errs {
		if e.Warning && noWarn {
			continue
		}
		sev := SevError
		if e.Warning {
			sev = SevWarning
		}
		ds = append(ds, Diagnostic{File: e.File, Line: e.Line, Col: e.Col, Severity: sev, Message: e.Msg})
	}
	return ds
}

func hasError(ds []Diagnostic) bool {
	for _, d := range ds {
		if d.Severity == SevError {
			return true
		}
	}
	return false
}

// Preprocess runs only the preprocessor and returns the text (-E)
func Preprocess(path string, data []byte, cfg *Config) (string, []Diagnostic) {
	pc, err := cfg.ppConfig()
	if err != nil {
		return "", []Diagnostic{{Severity: SevError, Message: err.Error()}}
	}
	if isAsmSource(path) {
		pc.Asm = true
	}
	return preprocessText(pc, path, data, cfg.NoWarnings)
}

// preprocessText runs the preprocessor and prints the tokens as text
// (the -E output, also the input of the assembler for .S files)
func preprocessText(pc cc.PPConfig, path string, data []byte, noWarn bool) (string, []Diagnostic) {
	pp := cc.NewPreprocessor(pc)
	toks := pp.Run(path, data)
	var sb strings.Builder
	var lastFile *cc.File
	lastLine := int32(0)
	for i, t := range toks {
		if t.Kind == cc.TEOF {
			break
		}
		if t.Kind == cc.TPragma {
			sb.WriteString("\n#pragma " + t.Text + "\n")
			continue
		}
		if t.BOL || i == 0 {
			loc := t
			if t.File != lastFile && t.File != nil {
				fmt.Fprintf(&sb, "\n# %d %s\n", t.Line, strconv.Quote(t.File.Name))
				lastFile = t.File
				lastLine = t.Line
			} else if t.File == lastFile && t.Line > lastLine && t.Line-lastLine < 8 {
				for lastLine < t.Line {
					sb.WriteByte('\n')
					lastLine++
				}
			} else if t.File == lastFile && t.Line != lastLine {
				fmt.Fprintf(&sb, "\n# %d %s\n", t.Line, strconv.Quote(t.File.Name))
				lastLine = t.Line
			}
			_ = loc
		} else if t.Space {
			sb.WriteByte(' ')
		}
		sb.WriteString(t.Text)
	}
	sb.WriteByte('\n')
	return sb.String(), toDiags(pp.Errors, noWarn)
}

// IsSource tells whether a file is compiled by CompileSource (.c, .s, .S)
func IsSource(path string) bool {
	return strings.HasSuffix(path, ".c") || isAsmSource(path)
}

func isAsmSource(path string) bool {
	return strings.HasSuffix(path, ".s") || strings.HasSuffix(path, ".S")
}

// assembleSource assembles a .s file (or the preprocessed text of a .S file)
func assembleSource(path, text string, u *Unit) {
	res, err := x64.Generate(&cc.Program{TopAsm: []string{text}}, path, false)
	if err != nil {
		d := Diagnostic{File: path, Severity: SevError, Message: err.Error()}
		msg := strings.TrimPrefix(err.Error(), "error: ")
		if f, l, rest, ok := splitFileLine(msg); ok {
			d.File, d.Line, d.Message = f, l, rest
		}
		u.Diagnostics = append(u.Diagnostics, d)
		return
	}
	u.Obj = res.Obj
}

// splitFileLine parses "file:line: msg" produced for preprocessed sources
func splitFileLine(msg string) (string, int, string, bool) {
	i := strings.Index(msg, ": ")
	if i < 0 {
		return "", 0, "", false
	}
	head := msg[:i]
	j := strings.LastIndexByte(head, ':')
	if j < 0 {
		return "", 0, "", false
	}
	line, err := strconv.Atoi(head[j+1:])
	if err != nil {
		return "", 0, "", false
	}
	return head[:j], line, msg[i+2:], true
}

// Unit is the result of compiling one source file
type Unit struct {
	Obj         *elf.Object
	Listing     string
	Diagnostics []Diagnostic
	Deps        []string
}

// CompileSource compiles one source file to an object: C, assembler (.s)
// or assembler with C preprocessing (.S)
func CompileSource(path string, data []byte, cfg *Config) (u *Unit) {
	u = &Unit{}
	pc, err := cfg.ppConfig()
	if err != nil {
		u.Diagnostics = append(u.Diagnostics, Diagnostic{Severity: SevError, Message: err.Error()})
		return u
	}
	defer func() {
		if r := recover(); r != nil {
			u.Obj = nil
			msg := fmt.Sprintf("internal compiler error: %v", r)
			if os.Getenv("ALTC_DEBUG") != "" {
				msg += "\n" + string(debug.Stack())
			}
			u.Diagnostics = append(u.Diagnostics, Diagnostic{File: path, Severity: SevError, Message: msg})
		}
	}()
	switch {
	case strings.HasSuffix(path, ".s"):
		assembleSource(path, string(data), u)
		return u
	case strings.HasSuffix(path, ".S"):
		pc.Asm = true
		text, diags := preprocessText(pc, path, data, cfg.NoWarnings)
		u.Diagnostics = diags
		if hasError(diags) {
			return u
		}
		assembleSource(path, text, u)
		return u
	}
	pp := cc.NewPreprocessor(pc)
	toks := pp.Run(path, data)
	u.Deps = pp.Deps
	u.Diagnostics = toDiags(pp.Errors, cfg.NoWarnings)
	if hasError(u.Diagnostics) {
		return u
	}
	prog, perrs := cc.Parse(toks, cc.Options{Std: pc.Std, GNU: pc.GNU})
	u.Diagnostics = append(u.Diagnostics, toDiags(perrs, cfg.NoWarnings)...)
	if hasError(u.Diagnostics) {
		return u
	}
	res, err := x64.Generate(prog, path, cfg.Listing)
	if err != nil {
		u.Diagnostics = append(u.Diagnostics, parseGenError(path, err))
		return u
	}
	u.Obj = res.Obj
	u.Listing = res.Listing
	return u
}

// parseGenError turns "file:line:col: error: msg" back into a diagnostic
func parseGenError(path string, err error) Diagnostic {
	msg := err.Error()
	if i := strings.Index(msg, ": error: "); i > 0 {
		pos := strings.Split(msg[:i], ":")
		if len(pos) >= 3 {
			line, _ := strconv.Atoi(pos[len(pos)-2])
			col, _ := strconv.Atoi(pos[len(pos)-1])
			return Diagnostic{File: strings.Join(pos[:len(pos)-2], ":"), Line: line, Col: col, Severity: SevError, Message: msg[i+9:]}
		}
	}
	return Diagnostic{File: path, Severity: SevError, Message: msg}
}

// ---- runtime support ----

// runtimeSource provides what glibc keeps in libc_nonshared.a, so that
// linking needs only the shared libc.so.6
const runtimeSource = `
typedef unsigned long size_t;
void *__dso_handle __attribute__((visibility("hidden"))) = &__dso_handle;
int __cxa_atexit(void (*f)(void *), void *arg, void *dso);
int __cxa_at_quick_exit(void (*f)(void *), void *dso);
int __register_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void), void *dso);
int atexit(void (*f)(void)) { return __cxa_atexit((void (*)(void *))f, 0, __dso_handle); }
int at_quick_exit(void (*f)(void)) { return __cxa_at_quick_exit((void (*)(void *))f, __dso_handle); }
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
	return __register_atfork(prepare, parent, child, __dso_handle);
}
void __stack_chk_fail(void);
void __stack_chk_fail_local(void) { __stack_chk_fail(); }

/* 128-bit integer helpers (the libgcc names) */
typedef unsigned __int128 __altc_u128;
typedef __int128 __altc_i128;
static __altc_u128 __altc_udivmod(__altc_u128 a, __altc_u128 b, __altc_u128 *rem) {
	if (b == 0) { volatile int z = 0; return 1 / z; }
	if ((a >> 64) == 0 && (b >> 64) == 0) {
		unsigned long x = (unsigned long)a, y = (unsigned long)b;
		if (rem) *rem = x % y;
		return x / y;
	}
	__altc_u128 q = 0, r = 0;
	for (int i = 127; i >= 0; i--) {
		r = (r << 1) | ((a >> i) & 1);
		if (r >= b) { r -= b; q |= (__altc_u128)1 << i; }
	}
	if (rem) *rem = r;
	return q;
}
__altc_u128 __udivti3(__altc_u128 a, __altc_u128 b) { return __altc_udivmod(a, b, 0); }
__altc_u128 __umodti3(__altc_u128 a, __altc_u128 b) { __altc_u128 r; __altc_udivmod(a, b, &r); return r; }
__altc_i128 __divti3(__altc_i128 a, __altc_i128 b) {
	int neg = (a < 0) != (b < 0);
	__altc_u128 q = __altc_udivmod(a < 0 ? -(__altc_u128)a : (__altc_u128)a, b < 0 ? -(__altc_u128)b : (__altc_u128)b, 0);
	return neg ? -(__altc_i128)q : (__altc_i128)q;
}
__altc_i128 __modti3(__altc_i128 a, __altc_i128 b) {
	__altc_u128 r;
	__altc_udivmod(a < 0 ? -(__altc_u128)a : (__altc_u128)a, b < 0 ? -(__altc_u128)b : (__altc_u128)b, &r);
	return a < 0 ? -(__altc_i128)r : (__altc_i128)r;
}
double __floatuntidf(__altc_u128 a) {
	return (double)(unsigned long)(a >> 64) * 18446744073709551616.0 + (double)(unsigned long)a;
}
double __floattidf(__altc_i128 a) { return a < 0 ? -__floatuntidf(-(__altc_u128)a) : __floatuntidf((__altc_u128)a); }
__altc_u128 __fixunsdfti(double d) {
	if (!(d > 0)) return 0;
	unsigned long hi = (unsigned long)(d / 18446744073709551616.0);
	double rest = d - (double)hi * 18446744073709551616.0;
	if (rest < 0) rest = 0;
	return ((__altc_u128)hi << 64) + (unsigned long)rest;
}
__altc_i128 __fixdfti(double d) { return d < 0 ? -(__altc_i128)__fixunsdfti(-d) : (__altc_i128)__fixunsdfti(d); }

/* complex division (the libgcc names), Smith's algorithm */
#define __ALTC_CDIV(NAME, T) \
T _Complex NAME(T a, T b, T c, T d) { \
	T ratio, denom, x, y; \
	T ac = c < 0 ? -c : c, ad = d < 0 ? -d : d; \
	if (ac < ad) { \
		ratio = c / d; denom = c * ratio + d; \
		x = (a * ratio + b) / denom; y = (b * ratio - a) / denom; \
	} else { \
		ratio = d / c; denom = d * ratio + c; \
		x = (b * ratio + a) / denom; y = (b - a * ratio) / denom; \
	} \
	return __builtin_complex(x, y); \
}
__ALTC_CDIV(__divsc3, float)
__ALTC_CDIV(__divdc3, double)
__ALTC_CDIV(__divxc3, long double)

/* complex multiplication with the C11 Annex G recovery of infinities */
#define __ALTC_CMUL(NAME, T, INF) \
T _Complex NAME(T a, T b, T c, T d) { \
	T ac = a * c, bd = b * d, ad = a * d, bc = b * c; \
	T x = ac - bd, y = ad + bc; \
	if (__builtin_isnan(x) && __builtin_isnan(y)) { \
		int recalc = 0; \
		if (__builtin_isinf(a) || __builtin_isinf(b)) { \
			a = __builtin_copysignl(__builtin_isinf(a) ? 1 : 0, a); \
			b = __builtin_copysignl(__builtin_isinf(b) ? 1 : 0, b); \
			if (__builtin_isnan(c)) c = __builtin_copysignl(0, c); \
			if (__builtin_isnan(d)) d = __builtin_copysignl(0, d); \
			recalc = 1; \
		} \
		if (__builtin_isinf(c) || __builtin_isinf(d)) { \
			c = __builtin_copysignl(__builtin_isinf(c) ? 1 : 0, c); \
			d = __builtin_copysignl(__builtin_isinf(d) ? 1 : 0, d); \
			if (__builtin_isnan(a)) a = __builtin_copysignl(0, a); \
			if (__builtin_isnan(b)) b = __builtin_copysignl(0, b); \
			recalc = 1; \
		} \
		if (!recalc && (__builtin_isinf(ac) || __builtin_isinf(bd) || __builtin_isinf(ad) || __builtin_isinf(bc))) { \
			if (__builtin_isnan(a)) a = __builtin_copysignl(0, a); \
			if (__builtin_isnan(b)) b = __builtin_copysignl(0, b); \
			if (__builtin_isnan(c)) c = __builtin_copysignl(0, c); \
			if (__builtin_isnan(d)) d = __builtin_copysignl(0, d); \
			recalc = 1; \
		} \
		if (recalc) { \
			x = INF * (a * c - b * d); \
			y = INF * (a * d + b * c); \
		} \
	} \
	return __builtin_complex(x, y); \
}
__ALTC_CMUL(__mulsc3, float, __builtin_inff())
__ALTC_CMUL(__muldc3, double, __builtin_inf())
__ALTC_CMUL(__mulxc3, long double, __builtin_infl())

/* integer powers: __builtin_powi */
#define __ALTC_POWI(NAME, T) \
T NAME(T x, int m) { \
	unsigned n = m < 0 ? -(unsigned)m : (unsigned)m; \
	T y = n % 2 ? x : 1; \
	while (n >>= 1) { x = x * x; if (n % 2) y = y * x; } \
	return m < 0 ? 1 / y : y; \
}
__ALTC_POWI(__powisf2, float)
__ALTC_POWI(__powidf2, double)
__ALTC_POWI(__powixf2, long double)

/* bit operations */
int __popcountdi2(long a) { unsigned long x = a; int n = 0; while (x) { x &= x - 1; n++; } return n; }
int __popcountsi2(int a) { return __popcountdi2((unsigned)a); }
int __popcountti2(__altc_i128 a) { return __popcountdi2((long)a) + __popcountdi2((long)((__altc_u128)a >> 64)); }
int __paritydi2(long a) { return __popcountdi2(a) & 1; }
int __paritysi2(int a) { return __popcountdi2((unsigned)a) & 1; }
int __clzdi2(long a) { unsigned long x = a; int n = 0; if (!x) return 64; while (!(x >> 63)) { x <<= 1; n++; } return n; }
int __clzsi2(int a) { return __clzdi2((unsigned)a) - 32; }
int __ctzdi2(long a) { unsigned long x = a; int n = 0; if (!x) return 64; while (!(x & 1)) { x >>= 1; n++; } return n; }
int __ctzsi2(int a) { return (unsigned)a ? __ctzdi2((unsigned)a) : 32; }
int __ffsdi2(long a) { return a ? __ctzdi2(a) + 1 : 0; }
int __ffssi2(int a) { return a ? __ctzdi2((unsigned)a) + 1 : 0; }
int __bswapsi2(int a) { return __builtin_bswap32(a); }
long __bswapdi2(long a) { return __builtin_bswap64(a); }

/* more 128-bit conversions and arithmetic */
float __floatuntisf(__altc_u128 a) { return (float)__floatuntidf(a); }
float __floattisf(__altc_i128 a) { return (float)__floattidf(a); }
long double __floatuntixf(__altc_u128 a) {
	return (long double)(unsigned long)(a >> 64) * 18446744073709551616.0L + (long double)(unsigned long)a;
}
long double __floattixf(__altc_i128 a) { return a < 0 ? -__floatuntixf(-(__altc_u128)a) : __floatuntixf((__altc_u128)a); }
__altc_u128 __fixunssfti(float f) { return __fixunsdfti(f); }
__altc_i128 __fixsfti(float f) { return __fixdfti(f); }
__altc_u128 __fixunsxfti(long double d) {
	if (!(d > 0)) return 0;
	unsigned long hi = (unsigned long)(d / 18446744073709551616.0L);
	long double rest = d - (long double)hi * 18446744073709551616.0L;
	if (rest < 0) rest = 0;
	return ((__altc_u128)hi << 64) + (unsigned long)rest;
}
__altc_i128 __fixxfti(long double d) { return d < 0 ? -(__altc_i128)__fixunsxfti(-d) : (__altc_i128)__fixunsxfti(d); }
__altc_i128 __multi3(__altc_i128 a, __altc_i128 b) { return a * b; }
__altc_i128 __ashlti3(__altc_i128 a, int n) { return a << n; }
__altc_i128 __ashrti3(__altc_i128 a, int n) { return a >> n; }
__altc_i128 __lshrti3(__altc_i128 a, int n) { return (__altc_i128)((__altc_u128)a >> n); }
`

var (
	runtimeOnce sync.Once
	runtimeObjs []*elf.Object
	runtimeErr  error
)

// runtimeObjects compiles the runtime support once per process; each
// function becomes its own lazily linked object
func runtimeObjects() ([]*elf.Object, error) {
	runtimeOnce.Do(func() {
		u := CompileSource("<altc-runtime>", []byte(runtimeSource), &Config{Std: "gnu17", NoStdInc: true})
		if u.Obj == nil {
			msgs := []string{}
			for _, d := range u.Diagnostics {
				msgs = append(msgs, d.String())
			}
			runtimeErr = fmt.Errorf("runtime: %s", strings.Join(msgs, "; "))
			return
		}
		runtimeObjs = []*elf.Object{u.Obj}
	})
	return runtimeObjs, runtimeErr
}

// ---- libraries ----

// DefaultLibDirs are searched for -l libraries
var DefaultLibDirs = []string{"/lib/x86_64-linux-gnu", "/usr/lib/x86_64-linux-gnu", "/lib64", "/usr/lib64",
	"/usr/local/lib", "/lib", "/usr/lib"}

// libraries merged into libc.so.6 since glibc 2.34
var mergedLibs = map[string]bool{"c": true, "pthread": true, "dl": true, "rt": true, "util": true, "anl": true}

// LinkInput is what the linker reads besides the compiled units
type LinkInput struct {
	Objects   []*elf.Object
	Archives  []*elf.Archive
	Shared    []*elf.SharedLib
	Strip     bool // do not write the symbol table
	sharedSet map[string]bool
}

func (in *LinkInput) addShared(path string) error {
	if in.sharedSet == nil {
		in.sharedSet = map[string]bool{}
	}
	real, err := filepath.EvalSymlinks(path)
	if err != nil {
		real = path
	}
	if in.sharedSet[real] {
		return nil
	}
	lib, err := elf.ReadSharedLib(path)
	if err != nil {
		return err
	}
	in.sharedSet[real] = true
	in.Shared = append(in.Shared, lib)
	return nil
}

// AddFile adds an object (.o), archive (.a) or shared library (.so) by path
func (in *LinkInput) AddFile(path string) error {
	data, err := os.ReadFile(path)
	if err != nil {
		return err
	}
	switch {
	case strings.HasPrefix(string(data), "!<arch>\n"):
		ar, err := elf.ReadArchive(path, data)
		if err != nil {
			return err
		}
		in.Archives = append(in.Archives, ar)
	case len(data) > 18 && string(data[:4]) == "\x7fELF" && data[16] == 3:
		return in.addShared(path)
	case len(data) > 4 && string(data[:4]) == "\x7fELF":
		obj, err := elf.ReadObject(path, data)
		if err != nil {
			return err
		}
		in.Objects = append(in.Objects, obj)
	default:
		// a GNU ld script (like libc.so)
		files, err := elf.ParseLinkerScript(path)
		if err != nil {
			return err
		}
		for _, f := range files {
			if strings.Contains(f, "libc_nonshared") {
				continue // our runtime replaces it
			}
			if err := in.AddFile(f); err != nil {
				return err
			}
		}
	}
	return nil
}

// AddLib resolves -lname in the library directories
func (in *LinkInput) AddLib(name string, dirs []string) error {
	if mergedLibs[name] {
		return nil
	}
	if name == "m" {
		return nil // libm.so.6 is always linked
	}
	all := append(append([]string{}, dirs...), DefaultLibDirs...)
	for _, d := range all {
		p := filepath.Join(d, "lib"+name+".so")
		if _, err := os.Stat(p); err == nil {
			return in.AddFile(p)
		}
		p = filepath.Join(d, "lib"+name+".a")
		if _, err := os.Stat(p); err == nil {
			return in.AddFile(p)
		}
	}
	// no development symlink: use the versioned runtime library
	for _, d := range all {
		matches, _ := filepath.Glob(filepath.Join(d, "lib"+name+".so.*"))
		if len(matches) > 0 {
			sort.Strings(matches)
			return in.AddFile(matches[0])
		}
	}
	return fmt.Errorf("cannot find -l%s", name)
}

func findSystemLib(name string) string {
	for _, d := range DefaultLibDirs {
		p := filepath.Join(d, name)
		if _, err := os.Stat(p); err == nil {
			return p
		}
	}
	return ""
}

// Link builds an executable image from the inputs
func Link(in *LinkInput) ([]byte, []Diagnostic) {
	var diags []Diagnostic
	libc := findSystemLib("libc.so.6")
	if libc == "" {
		return nil, []Diagnostic{{Severity: SevError, Message: "libc.so.6 not found: a glibc-based Linux is required"}}
	}
	// libc and libm come first so their symbols win
	base := &LinkInput{}
	if err := base.addShared(libc); err != nil {
		return nil, []Diagnostic{{Severity: SevError, Message: err.Error()}}
	}
	if libm := findSystemLib("libm.so.6"); libm != "" {
		base.addShared(libm)
	}
	for _, s := range in.Shared {
		real, _ := filepath.EvalSymlinks(s.Path)
		if !base.sharedSet[real] {
			base.Shared = append(base.Shared, s)
		}
	}
	rt, err := runtimeObjects()
	if err != nil {
		return nil, []Diagnostic{{Severity: SevError, Message: err.Error()}}
	}
	ln := &elf.Linker{
		Objects:    in.Objects,
		Archives:   in.Archives,
		SharedLibs: base.Shared,
		Lazy:       append([]*elf.Object(nil), rt...),
		StripSyms:  in.Strip,
	}
	img, err := ln.Link()
	if err != nil {
		if le, ok := err.(*elf.LinkError); ok {
			for _, m := range le.Messages {
				diags = append(diags, Diagnostic{Severity: SevError, Message: m})
			}
		} else {
			diags = append(diags, Diagnostic{Severity: SevError, Message: err.Error()})
		}
		return nil, diags
	}
	return img, diags
}
