package tests

import (
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
)

// The ABI test generates functions that pass and return structs of random
// shapes together with scalars, compiles the callers with one compiler and
// the callees with the other, and checks that both agree on every value.

var abiScalars = []struct{ name, fmt string }{
	{"char", "%d"}, {"short", "%d"}, {"int", "%d"}, {"long", "%ld"}, {"float", "%.3f"}, {"double", "%.3f"},
	{"unsigned char", "%u"}, {"long double", "%.3Lf"}, {"__int128", ""}, {"double _Complex", ""}, {"float _Complex", ""}, {"long double _Complex", ""}, {"void *", "%p"},
}

type abiStruct struct {
	name   string
	fields []string // scalar type names (arrays as "T[n]")
}

func genStruct(r *rand.Rand, i int) abiStruct {
	n := 1 + r.Intn(5)
	s := abiStruct{name: fmt.Sprintf("S%d", i)}
	for j := 0; j < n; j++ {
		t := abiScalars[r.Intn(len(abiScalars)-1)].name // no pointers in structs
		if r.Intn(8) == 0 && t != "long double" {
			t = fmt.Sprintf("%s[%d]", t, 1+r.Intn(3))
		}
		s.fields = append(s.fields, t)
	}
	return s
}

func fieldDecl(t string, name string) string {
	if i := strings.Index(t, "["); i > 0 {
		return t[:i] + " " + name + t[i:]
	}
	return t + " " + name
}

// value expression for a field from a seed
func fillField(t, lhs, seed string) string {
	if i := strings.Index(t, "["); i > 0 {
		base := t[:i]
		var n int
		fmt.Sscanf(t[i+1:], "%d", &n)
		var sb strings.Builder
		for k := 0; k < n; k++ {
			fmt.Fprintf(&sb, "%s[%d] = (%s)(%s + %d); ", lhs, k, base, seed, k*3+1)
		}
		return sb.String()
	}
	return fmt.Sprintf("%s = (%s)(%s); ", lhs, t, seed)
}

func sumField(t, expr string) string {
	if i := strings.Index(t, "["); i > 0 {
		var n int
		fmt.Sscanf(t[i+1:], "%d", &n)
		var parts []string
		for k := 0; k < n; k++ {
			parts = append(parts, fmt.Sprintf("(double)%s[%d]", expr, k))
		}
		return strings.Join(parts, " + ")
	}
	if strings.Contains(t, "*") {
		return fmt.Sprintf("(double)(long)%s", expr)
	}
	return "(double)" + expr
}

func genABI(seed int64) (header, callee, caller string) {
	r := rand.New(rand.NewSource(seed))
	var h strings.Builder
	h.WriteString("#include <stdio.h>\n#include <stdarg.h>\n#include <complex.h>\n")
	var structs []abiStruct
	for i := 0; i < 12; i++ {
		s := genStruct(r, i)
		structs = append(structs, s)
		fmt.Fprintf(&h, "typedef struct { ")
		for j, f := range s.fields {
			fmt.Fprintf(&h, "%s; ", fieldDecl(f, fmt.Sprintf("f%d", j)))
		}
		fmt.Fprintf(&h, "} %s;\n", s.name)
	}
	var ce, cr strings.Builder
	ce.WriteString("#include \"abi.h\"\n")
	cr.WriteString("#include \"abi.h\"\n")
	var mainBody strings.Builder
	for fi := 0; fi < 30; fi++ {
		// parameter list: a mix of scalars and structs
		np := 1 + r.Intn(10)
		type param struct {
			t     string
			isStr bool
			st    abiStruct
		}
		var ps []param
		for k := 0; k < np; k++ {
			if r.Intn(3) == 0 {
				st := structs[r.Intn(len(structs))]
				ps = append(ps, param{t: st.name, isStr: true, st: st})
			} else {
				ps = append(ps, param{t: abiScalars[r.Intn(len(abiScalars)-1)].name})
			}
		}
		retSt := structs[r.Intn(len(structs))]
		variadic := r.Intn(4) == 0
		var decl []string
		for k, p := range ps {
			decl = append(decl, fieldDecl(p.t, fmt.Sprintf("p%d", k)))
		}
		if variadic {
			decl = append(decl, "...")
		}
		proto := fmt.Sprintf("%s f%d(%s)", retSt.name, fi, strings.Join(decl, ", "))
		fmt.Fprintf(&h, "%s;\n", proto)
		// callee: sums all values, prints them, builds the result from the sum
		fmt.Fprintf(&ce, "%s {\n\tdouble sum = 0;\n", proto)
		for k, p := range ps {
			if p.isStr {
				for j, f := range p.st.fields {
					fmt.Fprintf(&ce, "\tsum += %s;\n", sumField(f, fmt.Sprintf("p%d.f%d", k, j)))
				}
			} else {
				fmt.Fprintf(&ce, "\tsum += %s;\n", sumField(p.t, fmt.Sprintf("p%d", k)))
			}
		}
		if variadic {
			fmt.Fprintf(&ce, "\tva_list ap; va_start(ap, p%d);\n\tsum += va_arg(ap, int); sum += va_arg(ap, double); sum += (double)va_arg(ap, long double);\n", len(ps)-1)
			if len(structs) > 0 {
				fmt.Fprintf(&ce, "\t{ %s v = va_arg(ap, %s);", structs[0].name, structs[0].name)
				for j, f := range structs[0].fields {
					fmt.Fprintf(&ce, " sum += %s;", sumField(f, fmt.Sprintf("v.f%d", j)))
				}
				ce.WriteString(" }\n")
			}
			ce.WriteString("\tsum += va_arg(ap, long);\n\tva_end(ap);\n")
		}
		fmt.Fprintf(&ce, "\tprintf(\"f%d sum=%%.3f\\n\", sum);\n\t%s r;\n", fi, retSt.name)
		for j, f := range retSt.fields {
			fmt.Fprintf(&ce, "\t%s\n", fillField(f, fmt.Sprintf("r.f%d", j), fmt.Sprintf("(long)(sum + %d) %% 97", j)))
		}
		ce.WriteString("\treturn r;\n}\n")
		// caller
		var args []string
		for k, p := range ps {
			if p.isStr {
				fmt.Fprintf(&mainBody, "\t{ %s v%d_%d;", p.st.name, fi, k)
				for j, f := range p.st.fields {
					mainBody.WriteString(" " + fillField(f, fmt.Sprintf("v%d_%d.f%d", fi, k, j), fmt.Sprintf("%d", r.Intn(100))))
				}
				fmt.Fprintf(&mainBody, " a%d_%d = v%d_%d; }\n", fi, k, fi, k)
				fmt.Fprintf(&cr, "static %s a%d_%d;\n", p.st.name, fi, k)
				args = append(args, fmt.Sprintf("a%d_%d", fi, k))
			} else if strings.Contains(p.t, "*") {
				args = append(args, fmt.Sprintf("(void *)%d", r.Intn(1000)))
			} else {
				args = append(args, fmt.Sprintf("(%s)%d", p.t, r.Intn(100)))
			}
		}
		if variadic {
			args = append(args, "7", "2.5", "(long double)1.25")
			if len(structs) > 0 {
				fmt.Fprintf(&cr, "static %s va%d;\n", structs[0].name, fi)
				fmt.Fprintf(&mainBody, "\t{")
				for j, f := range structs[0].fields {
					mainBody.WriteString(" " + fillField(f, fmt.Sprintf("va%d.f%d", fi, j), fmt.Sprintf("%d", j+5)))
				}
				mainBody.WriteString(" }\n")
				args = append(args, fmt.Sprintf("va%d", fi))
			}
			args = append(args, "123456789012L")
		}
		fmt.Fprintf(&mainBody, "\t{ %s r = f%d(%s); double s = 0;", retSt.name, fi, strings.Join(args, ", "))
		for j, f := range retSt.fields {
			fmt.Fprintf(&mainBody, " s += %s;", sumField(f, fmt.Sprintf("r.f%d", j)))
		}
		fmt.Fprintf(&mainBody, " printf(\"r%d=%%.3f\\n\", s); }\n", fi)
	}
	fmt.Fprintf(&cr, "int main(void) {\n%s\treturn 0;\n}\n", mainBody.String())
	return h.String(), ce.String(), cr.String()
}

func TestABIInterop(t *testing.T) {
	gcc := needGCC(t)
	for seed := int64(1); seed <= abiSeeds(); seed++ {
		seed := seed
		t.Run(fmt.Sprint(seed), func(t *testing.T) {
			dir := t.TempDir()
			if keep := os.Getenv("ABI_KEEP"); keep != "" {
				dir = filepath.Join(keep, fmt.Sprint(seed))
				os.MkdirAll(dir, 0o755)
			}
			h, callee, caller := genABI(seed)
			os.WriteFile(filepath.Join(dir, "abi.h"), []byte(h), 0o644)
			os.WriteFile(filepath.Join(dir, "callee.c"), []byte(callee), 0o644)
			os.WriteFile(filepath.Join(dir, "caller.c"), []byte(caller), 0o644)
			gccObj := func(name string) string {
				return gcc.object(t, filepath.Join(dir, name+".gcc.o"), filepath.Join(dir, name+".c"), "-w", "-O1", "-fno-pic")
			}
			altcObj := func(name string) *elf.Object {
				p := filepath.Join(dir, name+".c")
				src, _ := os.ReadFile(p)
				u := driver.CompileSource(p, src, testConfig(&driver.Config{NoWarnings: true}))
				if u.Obj == nil {
					t.Fatalf("altc %s: %v", name, u.Diagnostics)
				}
				return u.Obj
			}
			// reference: everything by gcc
			ref := &driver.LinkInput{}
			for _, o := range []string{gccObj("callee"), gccObj("caller")} {
				if err := ref.AddFile(o); err != nil {
					t.Fatal(err)
				}
			}
			want, _ := run(t, linkTo(t, filepath.Join(dir, "ref"), ref), dir)
			for _, mode := range []string{"altc-caller", "altc-callee", "altc-both"} {
				in := &driver.LinkInput{}
				switch mode {
				case "altc-caller":
					in.Objects = append(in.Objects, altcObj("caller"))
					in.AddFile(gccObj("callee"))
				case "altc-callee":
					in.Objects = append(in.Objects, altcObj("callee"))
					in.AddFile(gccObj("caller"))
				default:
					in.Objects = append(in.Objects, altcObj("caller"), altcObj("callee"))
				}
				img, diags := driver.Link(in)
				if img == nil {
					t.Fatalf("%s link: %v", mode, diags)
				}
				exe := filepath.Join(dir, mode)
				os.WriteFile(exe, img, 0o755)
				got, _ := run(t, exe, dir)
				if got != want {
					wl, gl := strings.Split(want, "\n"), strings.Split(got, "\n")
					for i := range wl {
						if i >= len(gl) || wl[i] != gl[i] {
							g := ""
							if i < len(gl) {
								g = gl[i]
							}
							t.Errorf("%s: first difference at line %d: gcc %q, altc %q (sources in %s)", mode, i+1, wl[i], g, dir)
							break
						}
					}
				}
			}
		})
	}
}

// abiSeeds is the number of generated cases (ABI_SEEDS overrides)
func abiSeeds() int64 {
	if s := os.Getenv("ABI_SEEDS"); s != "" {
		var n int64
		fmt.Sscan(s, &n)
		return n
	}
	return 6
}
