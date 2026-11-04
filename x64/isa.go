package x64

import (
	"fmt"
	"os"
	"sort"
	"strings"
)

// ISA is a set of instruction set extensions beyond the x86-64 baseline
// (which has SSE2). Code is generated for the baseline unless -march or
// -m<feature> options enable more, like gcc does.
type ISA uint32

const (
	ISASSE3 ISA = 1 << iota
	ISASSSE3
	ISASSE41
	ISASSE42
	ISAPOPCNT
	ISAAVX
	ISAAVX2
	ISAFMA
	ISABMI
	ISABMI2
	ISALZCNT
	ISAF16C
	ISAAES
	ISAPCLMUL
	ISASHA
	ISAMOVBE
	ISARDRND
	ISAXSAVE
)

// isaFeature describes an extension: its -m option name, the macro gcc
// defines for it and the extensions it implies
type isaFeature struct {
	bit     ISA
	name    string
	macro   string
	implies ISA
}

var isaFeatures = []isaFeature{
	{ISASSE3, "sse3", "__SSE3__", 0},
	{ISASSSE3, "ssse3", "__SSSE3__", ISASSE3},
	{ISASSE41, "sse4.1", "__SSE4_1__", ISASSSE3},
	{ISASSE42, "sse4.2", "__SSE4_2__", ISASSE41},
	{ISAPOPCNT, "popcnt", "__POPCNT__", 0},
	{ISAAVX, "avx", "__AVX__", ISASSE42},
	{ISAAVX2, "avx2", "__AVX2__", ISAAVX},
	{ISAFMA, "fma", "__FMA__", ISAAVX},
	{ISABMI, "bmi", "__BMI__", 0},
	{ISABMI2, "bmi2", "__BMI2__", 0},
	{ISALZCNT, "lzcnt", "__LZCNT__", 0},
	{ISAF16C, "f16c", "__F16C__", ISAAVX},
	{ISAAES, "aes", "__AES__", 0},
	{ISAPCLMUL, "pclmul", "__PCLMUL__", 0},
	{ISASHA, "sha", "__SHA__", 0},
	{ISAMOVBE, "movbe", "__MOVBE__", 0},
	{ISARDRND, "rdrnd", "__RDRND__", 0},
	{ISAXSAVE, "xsave", "__XSAVE__", 0},
}

// closure adds the implied extensions
func (s ISA) closure() ISA {
	for changed := true; changed; {
		changed = false
		for _, f := range isaFeatures {
			if s&f.bit != 0 && s&f.implies != f.implies {
				s |= f.implies
				changed = true
			}
		}
	}
	return s
}

// without removes an extension and everything that needs it
func (s ISA) without(bit ISA) ISA {
	s &^= bit
	for changed := true; changed; {
		changed = false
		for _, f := range isaFeatures {
			if s&f.bit != 0 && f.implies&bit != 0 {
				s &^= f.bit
				bit |= f.bit
				changed = true
			}
		}
	}
	return s
}

// Has tells whether all the given extensions are enabled
func (s ISA) Has(bits ISA) bool { return s&bits == bits }

const (
	isaV2 = ISASSE3 | ISASSSE3 | ISASSE41 | ISASSE42 | ISAPOPCNT
	isaV3 = isaV2 | ISAAVX | ISAAVX2 | ISABMI | ISABMI2 | ISAF16C | ISAFMA | ISALZCNT | ISAMOVBE | ISAXSAVE
)

// marchTable maps -march values to extension sets
var marchTable = map[string]ISA{
	"x86-64":         0,
	"k8":             0,
	"x86-64-v2":      isaV2,
	"x86-64-v3":      isaV3,
	"core2":          ISASSE3 | ISASSSE3,
	"nehalem":        isaV2,
	"corei7":         isaV2,
	"westmere":       isaV2 | ISAAES | ISAPCLMUL,
	"sandybridge":    isaV2 | ISAAES | ISAPCLMUL | ISAAVX | ISAXSAVE,
	"corei7-avx":     isaV2 | ISAAES | ISAPCLMUL | ISAAVX | ISAXSAVE,
	"ivybridge":      isaV2 | ISAAES | ISAPCLMUL | ISAAVX | ISAXSAVE | ISAF16C | ISARDRND,
	"haswell":        isaV3 | ISAAES | ISAPCLMUL | ISARDRND,
	"core-avx2":      isaV3 | ISAAES | ISAPCLMUL | ISARDRND,
	"broadwell":      isaV3 | ISAAES | ISAPCLMUL | ISARDRND,
	"skylake":        isaV3 | ISAAES | ISAPCLMUL | ISARDRND,
	"alderlake":      isaV3 | ISAAES | ISAPCLMUL | ISARDRND | ISASHA,
	"btver2":         isaV2 | ISAAES | ISAPCLMUL | ISAAVX | ISABMI | ISAF16C | ISAMOVBE | ISALZCNT,
	"bdver4":         isaV3 | ISAAES | ISAPCLMUL,
	"znver1":         isaV3 | ISAAES | ISAPCLMUL | ISASHA | ISARDRND,
	"znver2":         isaV3 | ISAAES | ISAPCLMUL | ISASHA | ISARDRND,
	"znver3":         isaV3 | ISAAES | ISAPCLMUL | ISASHA | ISARDRND,
	"tremont":        isaV2 | ISAAES | ISAPCLMUL | ISASHA | ISAMOVBE | ISARDRND,
	"goldmont":       isaV2 | ISAAES | ISAPCLMUL | ISASHA | ISAMOVBE | ISARDRND,
	"silvermont":     isaV2 | ISAAES | ISAPCLMUL | ISAMOVBE | ISARDRND,
	"generic":        0,
	"x86-64-generic": 0,
}

// cpuinfoFlags maps /proc/cpuinfo flags to extensions (for -march=native)
var cpuinfoFlags = map[string]ISA{
	"pni": ISASSE3, "ssse3": ISASSSE3, "sse4_1": ISASSE41, "sse4_2": ISASSE42, "popcnt": ISAPOPCNT,
	"avx": ISAAVX, "avx2": ISAAVX2, "fma": ISAFMA, "bmi1": ISABMI, "bmi2": ISABMI2, "abm": ISALZCNT,
	"f16c": ISAF16C, "aes": ISAAES, "pclmulqdq": ISAPCLMUL, "sha_ni": ISASHA, "movbe": ISAMOVBE,
	"rdrand": ISARDRND, "xsave": ISAXSAVE,
}

// NativeISA reads the extensions of the running processor
func NativeISA() ISA {
	data, err := os.ReadFile("/proc/cpuinfo")
	if err != nil {
		return 0
	}
	var s ISA
	for _, line := range strings.Split(string(data), "\n") {
		if !strings.HasPrefix(line, "flags") {
			continue
		}
		if i := strings.IndexByte(line, ':'); i >= 0 {
			for _, f := range strings.Fields(line[i+1:]) {
				s |= cpuinfoFlags[f]
			}
		}
		break
	}
	return s
}

// MachineFlag applies a -march=, -m<feature> or -mno-<feature> option.
// It reports whether the option was a machine option at all; other -m
// options (-mtune, -m64, -mfpmath=sse...) are accepted and ignored.
func (s *ISA) MachineFlag(opt string) (bool, error) {
	if !strings.HasPrefix(opt, "-m") {
		return false, nil
	}
	o := opt[2:]
	if v, ok := strings.CutPrefix(o, "arch="); ok {
		if v == "native" {
			*s = NativeISA().closure()
			return true, nil
		}
		set, ok := marchTable[v]
		if !ok {
			return true, fmt.Errorf("unsupported -march=%s (supported: %s, native)", v, strings.Join(marchNames(), ", "))
		}
		*s = set.closure()
		return true, nil
	}
	neg := false
	if v, ok := strings.CutPrefix(o, "no-"); ok {
		neg, o = true, v
	}
	if o == "sse4" {
		if neg {
			*s = s.without(ISASSE41)
		} else {
			*s = (*s | ISASSE42).closure()
		}
		return true, nil
	}
	if o == "abm" {
		o = "lzcnt"
	}
	for _, f := range isaFeatures {
		if f.name == o {
			if neg {
				*s = s.without(f.bit)
			} else {
				*s = (*s | f.bit).closure()
			}
			return true, nil
		}
	}
	return true, nil
}

func marchNames() []string {
	var ns []string
	for n := range marchTable {
		ns = append(ns, n)
	}
	sort.Strings(ns)
	return ns
}

// BaselineMacros enables the SSE2 baseline macros (__SSE2__...), which
// make code use <emmintrin.h> and friends
var BaselineMacros = true

// Macros returns the predefined macros of the instruction set, as gcc
// defines them (the SSE2 baseline included)
func (s ISA) Macros() []string {
	var ms []string
	if BaselineMacros {
		ms = append(ms, "__MMX__", "__MMX_WITH_SSE__", "__SSE__", "__SSE2__", "__FXSR__", "__SSE_MATH__", "__SSE2_MATH__")
	}
	for _, f := range isaFeatures {
		if s&f.bit != 0 {
			ms = append(ms, f.macro)
		}
	}
	return ms
}

// FeatureByName looks up an extension by its -m name
func FeatureByName(name string) (ISA, bool) {
	for _, f := range isaFeatures {
		if f.name == name {
			return f.bit, true
		}
	}
	return 0, false
}
