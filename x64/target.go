package x64

import (
	"fmt"
	"math/bits"
	"sort"
	"strings"

	"github.com/ipoluianov/altc/cc"
	"github.com/ipoluianov/altc/elf"
)

// Per-function instruction sets: __attribute__((target("avx2"))) compiles
// one function for more extensions than the command line enables, and
// target_clones("avx2", "default") compiles a version per target plus a
// dispatcher that jumps through a pointer chosen at startup.

// targetISA applies a target attribute ("avx2,fma", "arch=haswell",
// "no-avx", "default") to the extensions of the unit
func (g *gen) targetISA(target string) ISA {
	isa := g.baseISA
	for _, t := range strings.Split(target, ",") {
		t = strings.TrimSpace(t)
		switch {
		case t == "" || t == "default":
		case strings.HasPrefix(t, "arch="):
			isa.MachineFlag("-march=" + t[5:])
		case strings.HasPrefix(t, "tune="):
		default:
			isa.MachineFlag("-m" + t)
		}
	}
	return isa
}

// compileFunction generates a function for a target
func (g *gen) compileFunction(ob *cc.Obj, target string) {
	saved := g.isa
	g.isa = g.targetISA(target)
	g.a.AVX = g.isa.Has(ISAAVX)
	defer func() {
		g.isa = saved
		g.a.AVX = saved.Has(ISAAVX)
	}()
	g.a.CFIStartProc()
	g.lineAt(ob.Tok)
	g.usedIR = g.irFunction(ob)
	if !g.usedIR {
		g.function(ob)
	}
	g.a.CFIEndProc()
}

// targetFeatures lists the runtime checks of a clone target
func (g *gen) targetFeatures(target string) []string {
	var fs []string
	set := g.targetISA(target) &^ g.baseISA
	for _, f := range isaFeatures {
		if set&f.bit != 0 && f.bit != ISAXSAVE {
			fs = append(fs, f.name)
		}
	}
	return fs
}

func cloneSuffix(t string) string {
	r := strings.NewReplacer("=", "_", ",", "_", ".", "_", "-", "_")
	return r.Replace(t)
}

// targetClones emits the versions of a function, the pointer to the chosen
// one, the dispatcher (at sym) and the constructor choosing the version
func (g *gen) targetClones(ob *cc.Obj, sym *elf.Symbol, text, data, initArr *elf.Section, syms map[string]*elf.Symbol) {
	o := g.obj
	a := g.a
	ptrName := ob.AsmName + ".altc_ptr"

	// the dispatcher
	a.CFIStartProc()
	a.JmpInd(MSym(ptrName, 0))
	a.CFIEndProc()
	sym.Size = int64(len(a.Code)) - sym.Value

	// the versions
	type version struct {
		target string
		sym    *elf.Symbol
		isa    ISA
	}
	var versions []version
	var def *elf.Symbol
	for _, t := range ob.TargetClones {
		for len(a.Code)%16 != 0 {
			a.Code = append(a.Code, 0xCC)
		}
		name := ob.AsmName + "." + cloneSuffix(t)
		vs := o.AddLocal(&elf.Symbol{Name: name, Bind: elf.BindLocal, Type: elf.SymFunc, Section: text, Value: int64(len(a.Code))})
		syms[name] = vs
		if g.listing != nil {
			fmt.Fprintf(g.listing, "\n\t.type\t%s, @function\n%s:\n", name, name)
		}
		target := t
		if t == "default" {
			target = ""
		}
		g.compileFunction(ob, target)
		vs.Size = int64(len(a.Code)) - vs.Value
		if t == "default" {
			def = vs
			continue
		}
		versions = append(versions, version{t, vs, g.targetISA(target)})
	}
	if def == nil {
		g.fail(ob.Body, "target_clones of '%s' needs a \"default\" version", ob.Name)
	}
	// the most specific version first
	sort.SliceStable(versions, func(i, j int) bool {
		return bits.OnesCount32(uint32(versions[i].isa)) > bits.OnesCount32(uint32(versions[j].isa))
	})

	// the pointer, initially to the default version
	off := alignTo(int64(len(data.Data)), 8)
	for int64(len(data.Data)) < off+8 {
		data.Data = append(data.Data, 0)
	}
	ptr := o.AddLocal(&elf.Symbol{Name: ptrName, Bind: elf.BindLocal, Type: elf.SymObject, Section: data, Value: off, Size: 8})
	syms[ptrName] = ptr
	data.Relocs = append(data.Relocs, elf.Reloc{Offset: off, Type: elf.R_X86_64_64, Sym: def})

	// the constructor
	for len(a.Code)%16 != 0 {
		a.Code = append(a.Code, 0xCC)
	}
	rname := ob.AsmName + ".altc_resolver"
	rs := o.AddLocal(&elf.Symbol{Name: rname, Bind: elf.BindLocal, Type: elf.SymFunc, Section: text, Value: int64(len(a.Code))})
	syms[rname] = rs
	if g.listing != nil {
		fmt.Fprintf(g.listing, "\n\t.type\t%s, @function\n%s:\n", rname, rname)
	}
	a.CFIStartProc()
	a.FramePrologue()
	a.Call("__cpu_indicator_init")
	done := a.NewLabel()
	for _, v := range versions {
		next := a.NewLabel()
		for _, f := range g.targetFeatures(v.target) {
			// the feature bit in libgcc's processor model
			bit, ok := cc.CPUFeatureBit(f)
			if !ok {
				g.fail(ob.Body, "target_clones: unknown feature '%s'", f)
			}
			w := MSym("__cpu_model", 12)
			if bit >= 32 {
				w = MSym("__cpu_features2", int32((bit-32)/32*4))
				bit = (bit - 32) % 32
			}
			a.Test(4, w, Imm(int64(1)<<bit))
			a.Jcc(CondE, next)
		}
		a.Lea(RAX, MSym(v.sym.Name, 0))
		a.Mov(8, MSym(ptrName, 0), Q(RAX))
		a.Jmp(done)
		a.Bind(next)
	}
	a.Bind(done)
	a.FrameReturn(false)
	a.CFIEndProc()
	rs.Size = int64(len(a.Code)) - rs.Value
	g.addArrayEntry(initArr, rs)
}
