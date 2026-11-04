package x64

import (
	"fmt"
	"math"
	"os"
	"strings"

	"github.com/ipoluianov/altc/cc"
	"github.com/ipoluianov/altc/elf"
)

var argGP = []Reg{RDI, RSI, RDX, RCX, R8, R9}

// gen is the code generator of one translation unit
type gen struct {
	secOf    map[*Asm]*elf.Section // the sections of the code and assembly text
	a        *Asm
	obj      *elf.Object
	prog     *cc.Program
	fn       *cc.Obj
	depth    int
	labels   map[string]Label
	retLabel Label
	defined  map[string]*cc.Obj // symbols defined in this unit
	consts   map[string]string  // rodata constant pool: bytes -> symbol
	rodata   *elf.Section
	constSec []constEntry
	uniq     int
	errs     []error

	// per function
	retBufOff   int64 // offset of the saved hidden return pointer
	gpUsed      int   // registers taken by named params (for va_start)
	fpUsed      int
	stackArgSz  int64
	vaTemps     map[*cc.Node]int64
	overAligned map[*cc.Obj]int64 // locals reached through an aligned pointer
	listing     *strings.Builder

	// inline assembly
	asmSlots   map[*cc.AsmOperand]int64 // frame slots of asm operands
	asmSaveOff int64                    // save area of callee-saved registers (0: none)
	asmSecs    []*asmSection            // sections created by assembly text
	topLabels  map[string]*TextLabel    // labels of file-scope asm
	topSyms    map[string]*asmSym       // what file-scope asm says about symbols

	// register variables
	regVars    map[*cc.Obj]Reg // locals living in callee-saved registers
	regSaved   []Reg           // registers saved in the prologue
	regSaveOff int64           // their save area
	noRegAlloc bool            // keep every local in the frame
	fastMath   bool            // floating-point operations may be reassociated
	debug      bool            // DWARF debug information
	dbgFuncs   []dbgFunc       // the functions described in .debug_info
	asmFiles   map[int]string  // .file entries of assembly text
	usedIR     bool            // the last function was compiled by the IR backend
	isa        ISA             // instruction set extensions of the function being compiled
	baseISA    ISA             // those of the unit (command line)

	// temporaries in callee-saved registers not used by variables
	tempRegs  []Reg    // available registers
	tempTop   int      // registers in use
	tempUsed  int      // most registers in use at once
	tempLimit int      // registers the function may use (-1: all)
	dryLabels []string // labels created by the trial generation

	xmmVars []*cc.Obj // float variables in xmm8-xmm15 (spilled around calls)

	irReason string           // why the IR backend did not take the current function
	inlineOK map[*cc.Obj]bool // functions that calls may be replaced by
	noInline bool             // no inlining at all
}

type constEntry struct {
	name  string
	data  []byte
	align int64
}

// Result of code generation
type Result struct {
	Obj     *elf.Object
	Listing string // GNU assembler text (when requested)
}

// Options of code generation
type Options struct {
	Listing    bool // produce GNU assembler text
	NoRegAlloc bool // keep every local variable in the frame
	ISA        ISA  // enabled instruction set extensions
	FastMath   bool // floating-point operations may be reassociated
	Debug      bool // DWARF debug information (line tables, functions, variables)
}

// Generate compiles a parsed program to an object
func Generate(prog *cc.Program, name string, wantListing bool) (*Result, error) {
	return GenerateWith(prog, name, Options{Listing: wantListing})
}

// GenerateWith compiles a parsed program to an object with options
func GenerateWith(prog *cc.Program, name string, opts Options) (*Result, error) {
	wantListing := opts.Listing
	g := &gen{
		noRegAlloc: opts.NoRegAlloc,
		fastMath:   opts.FastMath,
		debug:      opts.Debug,
		isa:        opts.ISA,
		baseISA:    opts.ISA,
		a:          &Asm{SymPrefix: "t", AVX: opts.ISA.Has(ISAAVX)},
		obj:        elf.NewObject(name),
		prog:       prog,
		labels:     map[string]Label{},
		defined:    map[string]*cc.Obj{},
		consts:     map[string]string{},
		inlineOK:   map[*cc.Obj]bool{},
		noInline:   os.Getenv("ALTC_NOINLINE") != "",
	}
	if wantListing {
		g.listing = &strings.Builder{}
		g.a.Listing = g.listing
	}
	if err := g.run(); err != nil {
		return nil, err
	}
	res := &Result{Obj: g.obj}
	if g.listing != nil {
		res.Listing = g.listing.String()
	}
	return res, nil
}

type genError struct{ err error }

func (g *gen) fail(n *cc.Node, format string, args ...any) {
	msg := fmt.Sprintf(format, args...)
	if n != nil && n.Tok != nil {
		msg = n.Tok.Pos() + ": error: " + msg
	}
	panic(genError{fmt.Errorf("%s", msg)})
}

func alignTo(n, a int64) int64 {
	if a <= 1 {
		return n
	}
	return (n + a - 1) / a * a
}

// ---- liveness of static inline functions ----

func (g *gen) liveObjects() map[*cc.Obj]bool {
	live := map[*cc.Obj]bool{}
	byName := map[string]*cc.Obj{}
	for _, o := range g.prog.Globals {
		if o.IsDefinition {
			byName[o.AsmName] = o
		}
	}
	var mark func(o *cc.Obj)
	mark = func(o *cc.Obj) {
		if live[o] {
			return
		}
		live[o] = true
		for _, r := range o.Refs {
			mark(r)
			if d := byName[r.AsmName]; d != nil {
				mark(d)
			}
		}
		for _, rl := range o.Relocs {
			if d := byName[rl.Label]; d != nil {
				mark(d)
			}
		}
	}
	for _, o := range g.prog.Globals {
		if o.IsFunction && o.IsStatic && o.IsInline && !o.IsUsed {
			continue
		}
		if o.IsFunction && o.Body == nil && !o.IsUsed {
			continue
		}
		mark(o)
	}
	return live
}

func (g *gen) run() (err error) {
	defer func() {
		if r := recover(); r != nil {
			ge, ok := r.(genError)
			if !ok {
				panic(r)
			}
			err = ge.err
		}
	}()
	live := g.liveObjects()
	o := g.obj
	text := o.AddSection(".text", elf.SecText, 16)
	g.rodata = o.AddSection(".rodata", elf.SecROData, 16)
	data := o.AddSection(".data", elf.SecData, 8)
	bss := o.AddSection(".bss", elf.SecBSS, 8)
	tdata := o.AddSection(".tdata", elf.SecTData, 8)
	tbss := o.AddSection(".tbss", elf.SecTBSS, 8)
	initArr := o.AddSection(".init_array", elf.SecInitArray, 8)
	finiArr := o.AddSection(".fini_array", elf.SecFiniArray, 8)

	syms := map[string]*elf.Symbol{}
	symFor := func(ob *cc.Obj) *elf.Symbol {
		if s, ok := syms[ob.AsmName]; ok {
			return s
		}
		var s *elf.Symbol
		if ob.IsStatic || strings.HasPrefix(ob.AsmName, ".L") {
			s = o.AddLocal(&elf.Symbol{Name: ob.AsmName, Bind: elf.BindLocal})
		} else {
			s = o.Symbol(ob.AsmName)
			if ob.IsWeak || ob.IsInline && ob.IsFunction && !ob.IsStatic {
				s.Bind = elf.BindWeak
			}
			s.Hidden = ob.IsHidden
		}
		syms[ob.AsmName] = s
		return s
	}
	// collect definitions
	for _, ob := range g.prog.Globals {
		if !live[ob] || !ob.IsDefinition {
			continue
		}
		if ob.IsFunction && (ob.Body == nil || ob.InlineOnly) {
			continue
		}
		if ob.Alias != "" {
			continue
		}
		if prev, ok := g.defined[ob.AsmName]; ok && prev != ob {
			continue
		}
		g.defined[ob.AsmName] = ob
	}
	// data objects
	for _, ob := range g.prog.Globals {
		if g.defined[ob.AsmName] != ob || ob.IsFunction {
			continue
		}
		s := symFor(ob)
		s.Size = ob.Ty.Size()
		s.Type = elf.SymObject
		align := ob.Align
		if ob.Ty.Kind == cc.TyArray && ob.Ty.Size() >= 16 && align < 16 {
			align = 16
		}
		if align < 1 {
			align = 1
		}
		var sec *elf.Section
		allZero := len(ob.Relocs) == 0 && isZero(ob.InitData)
		switch {
		case ob.IsTLS:
			s.Type = elf.SymTLS
			if ob.InitData != nil && !allZero {
				sec = tdata
			} else {
				sec = tbss
			}
		case ob.Section != "":
			sec = g.customSection(ob.Section)
		case ob.InitData == nil && !ob.IsStatic && ob.IsTentative:
			// tentative definition: a common symbol (like -fcommon)
			s.Type = elf.SymCommon
			s.Value = align
			s.Size = ob.Ty.Size()
			if g.listing != nil {
				fmt.Fprintf(g.listing, "\t.comm\t%s,%d,%d\n", ob.AsmName, s.Size, align)
			}
			continue
		case ob.InitData == nil || allZero:
			sec = bss
		case ob.IsConst && len(ob.Relocs) == 0:
			sec = g.rodata
		default:
			sec = data
		}
		g.placeData(sec, s, ob, align)
	}
	if err := g.topLevelAsm(); err != nil {
		return err
	}
	// functions; static inline ones come last and only when something
	// still refers to them (calls that were not inlined, addresses)
	var funcs, deferred []*cc.Obj
	for _, ob := range g.prog.Globals {
		if g.defined[ob.AsmName] != ob || !ob.IsFunction {
			continue
		}
		if ob.IsStatic && (ob.IsInline || ob.AlwaysInline) && !ob.LabelAddrs && !ob.Ctor && !ob.Dtor {
			deferred = append(deferred, ob)
			continue
		}
		funcs = append(funcs, ob)
	}
	referenced := func(name string) bool {
		for _, r := range g.a.Relocs {
			if r.Sym == name {
				return true
			}
		}
		for _, s := range g.asmSecs {
			for _, r := range s.a.Relocs {
				if r.Sym == name {
					return true
				}
			}
		}
		for _, x := range g.prog.Globals {
			for _, r := range x.Relocs {
				if r.Label == name {
					return true
				}
			}
		}
		return false
	}
	for i := 0; i < len(funcs) || len(deferred) > 0; i++ {
		if i >= len(funcs) {
			// emit the deferred functions referenced so far
			var rest []*cc.Obj
			for _, ob := range deferred {
				if referenced(ob.AsmName) {
					funcs = append(funcs, ob)
				} else {
					rest = append(rest, ob)
				}
			}
			if len(rest) == len(deferred) {
				break
			}
			deferred = rest
		}
		ob := funcs[i]
		s := symFor(ob)
		s.Type = elf.SymFunc
		for len(g.a.Code)%16 != 0 {
			g.a.Code = append(g.a.Code, 0xCC)
		}
		start := int64(len(g.a.Code))
		s.Section = text
		s.Value = start
		if g.listing != nil {
			fmt.Fprintf(g.listing, "\n\t.text\n")
			if !ob.IsStatic {
				fmt.Fprintf(g.listing, "\t.globl\t%s\n", ob.AsmName)
			}
			fmt.Fprintf(g.listing, "\t.type\t%s, @function\n%s:\n", ob.AsmName, ob.AsmName)
		}
		if len(ob.TargetClones) > 0 {
			g.targetClones(ob, s, text, data, initArr, syms)
			continue
		}
		g.compileFunction(ob, ob.Target)
		s.Size = int64(len(g.a.Code)) - start
		if g.debug {
			g.dbgFuncs = append(g.dbgFuncs, dbgFunc{ob: ob, start: int(start), end: len(g.a.Code), frame: g.noRegAlloc && !g.usedIR})
		}
		if ob.LabelAddrs {
			// symbols for labels whose addresses are stored in static data
			cc.Walk(ob.Body, func(n *cc.Node) {
				if n.Kind == cc.NLabel {
					name := cc.LabelSymbol(ob.AsmName, n.Label)
					pos := g.a.LabelPos(g.label(n.UniqueLabel))
					syms[name] = o.AddLocal(&elf.Symbol{Name: name, Bind: elf.BindLocal, Section: text, Value: int64(pos)})
				}
			})
		}
		if ob.Ctor {
			g.addArrayEntry(g.prioSection(initArr, ob.Priority), s)
		}
		if ob.Dtor {
			g.addArrayEntry(g.prioSection(finiArr, ob.Priority), s)
		}
	}
	if err := g.a.Finish(); err != nil {
		return err
	}
	text.Data = g.a.Code
	if g.a.MaxAlign > text.Align {
		text.Align = g.a.MaxAlign
	}
	pend, err := g.finishAsm(text, syms)
	if err != nil {
		return err
	}
	// aliases
	for _, ob := range g.prog.Globals {
		if ob.Alias == "" {
			continue
		}
		target, ok := syms[ob.Alias]
		if !ok {
			return fmt.Errorf("%s: alias target '%s' is not defined in this file", ob.Name, ob.Alias)
		}
		s := symFor(ob)
		s.Section, s.Value, s.Size, s.Type = target.Section, target.Value, target.Size, target.Type
		if ob.IFunc {
			s.Type, s.IFunc = elf.SymFunc, true
		}
	}
	if err := g.ehFrame(syms); err != nil {
		return err
	}
	g.asmLines()
	g.debugSections(text, syms)
	// constant pool
	for _, c := range g.constSec {
		off := alignTo(int64(len(g.rodata.Data)), c.align)
		for int64(len(g.rodata.Data)) < off {
			g.rodata.Data = append(g.rodata.Data, 0)
		}
		s := o.AddLocal(&elf.Symbol{Name: c.name, Bind: elf.BindLocal, Section: g.rodata, Value: off, Type: elf.SymObject})
		syms[c.name] = s
		g.rodata.Data = append(g.rodata.Data, c.data...)
		if g.listing != nil {
			fmt.Fprintf(g.listing, "\t.section\t.rodata\n\t.align\t%d\n%s:\n", c.align, c.name)
			g.listData(c.data, nil)
		}
	}
	// code relocations
	pend = append(pend, pendingRelocs{text, g.a.Relocs})
	for _, pr := range pend {
		for _, r := range pr.relocs {
			s, ok := syms[r.Sym]
			if !ok {
				s = o.Symbol(r.Sym)
				syms[r.Sym] = s
			}
			pr.sec.Relocs = append(pr.sec.Relocs, elf.Reloc{Offset: r.Offset, Type: r.Type, Sym: s, Addend: r.Addend})
		}
	}
	// data relocations
	for _, ob := range g.prog.Globals {
		if g.defined[ob.AsmName] != ob || ob.IsFunction || len(ob.Relocs) == 0 {
			continue
		}
		s := syms[ob.AsmName]
		if s.Section == nil {
			continue
		}
		for _, r := range ob.Relocs {
			if r.Minus != "" {
				a, b := syms[r.Label], syms[r.Minus]
				if a == nil || b == nil {
					return fmt.Errorf("%s: undefined label in a label difference", ob.Name)
				}
				elf.PutLE(s.Section.Data[s.Value+r.Offset:], uint64(a.Value-b.Value+r.Addend), int(r.Size))
				continue
			}
			ts, ok := syms[r.Label]
			if !ok {
				ts = o.Symbol(r.Label)
				syms[r.Label] = ts
			}
			s.Section.Relocs = append(s.Section.Relocs, elf.Reloc{Offset: s.Value + r.Offset, Type: elf.R_X86_64_64, Sym: ts, Addend: r.Addend})
		}
	}
	for _, ar := range []*elf.Section{initArr, finiArr} {
		for i := range ar.Relocs {
			_ = i
		}
	}
	return nil
}

func isZero(b []byte) bool {
	for _, x := range b {
		if x != 0 {
			return false
		}
	}
	return true
}

func (g *gen) customSection(name string) *elf.Section {
	for _, s := range g.obj.Sections {
		if s.Name == name {
			return s
		}
	}
	kind := elf.SecData
	switch {
	case strings.HasPrefix(name, ".rodata"):
		kind = elf.SecROData
	case strings.HasPrefix(name, ".bss"):
		kind = elf.SecBSS
	case strings.HasPrefix(name, ".text"):
		kind = elf.SecText
	case strings.HasPrefix(name, ".init_array"):
		kind = elf.SecInitArray
	case strings.HasPrefix(name, ".fini_array"):
		kind = elf.SecFiniArray
	}
	return g.obj.AddSection(name, kind, 8)
}

func (g *gen) placeData(sec *elf.Section, s *elf.Symbol, ob *cc.Obj, align int64) {
	if align > sec.Align {
		sec.Align = align
	}
	size := max(ob.Ty.Size(), int64(len(ob.InitData)))
	if size < 0 {
		size = 0
	}
	if sec.Kind == elf.SecBSS || sec.Kind == elf.SecTBSS {
		off := alignTo(sec.Size, align)
		sec.Size = off + size
		s.Section, s.Value = sec, off
		if g.listing != nil {
			fmt.Fprintf(g.listing, "\t.local\t%s\n\t.comm\t%s,%d,%d\n", ob.AsmName, ob.AsmName, size, align)
		}
		return
	}
	off := alignTo(int64(len(sec.Data)), align)
	for int64(len(sec.Data)) < off {
		sec.Data = append(sec.Data, 0)
	}
	s.Section, s.Value = sec, off
	d := ob.InitData
	if int64(len(d)) < size {
		d = append(append([]byte(nil), d...), make([]byte, size-int64(len(d)))...)
	}
	sec.Data = append(sec.Data, d[:size]...)
	if g.listing != nil {
		fmt.Fprintf(g.listing, "\t.section\t%s\n\t.align\t%d\n", sec.Name, align)
		if !ob.IsStatic {
			fmt.Fprintf(g.listing, "\t.globl\t%s\n", ob.AsmName)
		}
		fmt.Fprintf(g.listing, "%s:\n", ob.AsmName)
		g.listData(d[:size], ob.Relocs)
	}
}

func (g *gen) listData(d []byte, relocs []cc.InitReloc) {
	rel := map[int64]cc.InitReloc{}
	for _, r := range relocs {
		rel[r.Offset] = r
	}
	for i := int64(0); i < int64(len(d)); {
		if r, ok := rel[i]; ok {
			fmt.Fprintf(g.listing, "\t.quad\t%s%+d\n", r.Label, r.Addend)
			i += 8
			continue
		}
		fmt.Fprintf(g.listing, "\t.byte\t%d\n", d[i])
		i++
	}
}

// prioSection returns .init_array.NNNNN for prioritized constructors
func (g *gen) prioSection(base *elf.Section, prio int64) *elf.Section {
	if prio <= 0 {
		return base
	}
	name := fmt.Sprintf("%s.%05d", base.Name, prio)
	for _, s := range g.obj.Sections {
		if s.Name == name {
			return s
		}
	}
	return g.obj.AddSection(name, base.Kind, 8)
}

func (g *gen) addArrayEntry(sec *elf.Section, s *elf.Symbol) {
	off := int64(len(sec.Data))
	sec.Data = append(sec.Data, make([]byte, 8)...)
	sec.Relocs = append(sec.Relocs, elf.Reloc{Offset: off, Type: elf.R_X86_64_64, Sym: s})
}

// constLabel returns a rodata symbol holding the bytes
func (g *gen) constLabel(data []byte, align int64) string {
	key := fmt.Sprintf("%d:%x", align, data)
	if n, ok := g.consts[key]; ok {
		return n
	}
	g.uniq++
	name := fmt.Sprintf(".LC%d", g.uniq)
	g.consts[key] = name
	g.constSec = append(g.constSec, constEntry{name, data, align})
	return name
}

func (g *gen) label(name string) Label {
	if l, ok := g.labels[name]; ok {
		return l
	}
	l := g.a.NewLabel()
	g.labels[name] = l
	if g.dryLabels != nil {
		g.dryLabels = append(g.dryLabels, name)
	}
	return l
}

// ---- stack helpers ----

func (g *gen) push() {
	g.a.Push(RAX)
	g.depth++
}

func (g *gen) pop(r Reg) {
	g.a.Pop(r)
	g.depth--
}

func (g *gen) pushf() {
	g.a.Alu(SUB, 8, Q(RSP), Imm(8))
	g.a.MovF(true, M(RSP, 0), X(XMM0))
	g.depth++
}

func (g *gen) popf(r Reg) {
	g.a.MovF(true, X(r), M(RSP, 0))
	g.a.Alu(ADD, 8, Q(RSP), Imm(8))
	g.depth--
}

// pushLD spills st(0) to the stack (16 bytes)
func (g *gen) pushLD() {
	g.a.Alu(SUB, 8, Q(RSP), Imm(16))
	g.a.Fstp(10, M(RSP, 0))
	g.depth += 2
}

func (g *gen) popLD() {
	g.a.Fld(10, M(RSP, 0))
	g.a.Alu(ADD, 8, Q(RSP), Imm(16))
	g.depth -= 2
}

// ---- type helpers ----

func isFloat(t *cc.Type) bool  { return t.Kind == cc.TyFloat }
func isDouble(t *cc.Type) bool { return t.Kind == cc.TyDouble }
func isSSE(t *cc.Type) bool    { return t.Kind == cc.TyFloat || t.Kind == cc.TyDouble }
func isLD(t *cc.Type) bool     { return t.Kind == cc.TyLongDouble }

func hasFlonum(t *cc.Type, lo, hi, off int64) bool {
	switch t.Kind {
	case cc.TyStruct, cc.TyUnion, cc.TyComplex:
		for _, m := range t.Rec.Members {
			if !hasFlonum(m.Ty, lo, hi, off+m.Offset) {
				return false
			}
		}
		return true
	case cc.TyArray:
		bs := t.Base.Size()
		for i := int64(0); i < t.ArrayLen; i++ {
			if !hasFlonum(t.Base, lo, hi, off+bs*i) {
				return false
			}
		}
		return true
	}
	return off+max(t.Size(), 1) <= lo || hi <= off || t.Kind == cc.TyFloat || t.Kind == cc.TyDouble
}

func hasLD(t *cc.Type) bool {
	switch t.Kind {
	case cc.TyStruct, cc.TyUnion, cc.TyComplex:
		for _, m := range t.Rec.Members {
			if hasLD(m.Ty) {
				return true
			}
		}
	case cc.TyArray:
		return hasLD(t.Base)
	case cc.TyLongDouble:
		return true
	}
	return false
}

// structRegs classifies a struct for register passing: the number of
// GP and SSE eightbytes, or ok=false when it goes in memory
func structRegs(t *cc.Type) (gp, fp int, ok bool) {
	sz := t.Size()
	if sz > 16 || sz == 0 || hasLD(t) {
		return 0, 0, false
	}
	if hasFlonum(t, 0, 8, 0) {
		fp++
	} else {
		gp++
	}
	if sz > 8 {
		if hasFlonum(t, 8, 16, 0) {
			fp++
		} else {
			gp++
		}
	}
	return gp, fp, true
}

// retInX87Pair tells whether a value is returned in st(0) and st(1)
// (complex long double, class COMPLEX_X87)
func retInX87Pair(t *cc.Type) bool { return t.IsComplex() && isLD(t.Base) }

// storeElem stores the value just computed (xmm0 / st0 / rax) of a floating type
func (g *gen) storeElem(t *cc.Type, m Mem) {
	switch t.Kind {
	case cc.TyFloat:
		g.a.MovF(false, m, X(XMM0))
	case cc.TyDouble:
		g.a.MovF(true, m, X(XMM0))
	case cc.TyLongDouble:
		g.a.Fstp(10, m)
	}
}

// genComplexCast converts to, from or between complex types
func (g *gen) genComplexCast(n *cc.Node) {
	from, to := n.LHS.Ty, n.Ty
	g.genExpr(n.LHS)
	switch {
	case to.IsComplex() && !from.IsComplex():
		off := g.vaTemps[n]
		bt := to.Base
		g.cast(from, bt)
		g.storeElem(bt, M(RBP, int32(off)))
		g.genNum(&cc.Node{Kind: cc.NNum, Ty: bt})
		g.storeElem(bt, M(RBP, int32(off+bt.Size())))
		g.a.Lea(RAX, M(RBP, int32(off)))
	case to.IsComplex():
		off := g.vaTemps[n]
		fb, tb := from.Base, to.Base
		g.push()
		g.load(fb)
		g.cast(fb, tb)
		g.storeElem(tb, M(RBP, int32(off)))
		g.a.Mov(8, Q(RAX), M(RSP, 0))
		g.a.Alu(ADD, 8, Q(RAX), Imm(fb.Size()))
		g.load(fb)
		g.cast(fb, tb)
		g.storeElem(tb, M(RBP, int32(off+tb.Size())))
		g.pop(RAX)
		g.a.Lea(RAX, M(RBP, int32(off)))
	case to.Kind == cc.TyBool:
		fb := from.Base
		g.push()
		g.load(fb)
		g.cast(fb, to)
		g.a.Mov(8, Q(RDX), Q(RAX))
		g.a.Mov(8, Q(RAX), M(RSP, 0))
		g.a.Mov(8, M(RSP, 0), Q(RDX))
		g.a.Alu(ADD, 8, Q(RAX), Imm(fb.Size()))
		g.load(fb)
		g.cast(fb, to)
		g.pop(RDX)
		g.a.Alu(OR, 4, D(RAX), D(RDX))
	case to.Kind == cc.TyVoid:
	default:
		// complex -> real: the real part
		g.load(from.Base)
		g.cast(from.Base, to)
	}
}

// retInX87 tells whether a struct is returned in st(0): its only content
// is one long double (classes X87, X87UP)
func retInX87(t *cc.Type) bool {
	if !t.IsRecord() || t.Size() != 16 {
		return false
	}
	for t.IsRecord() {
		var only *cc.Member
		for _, m := range t.Rec.Members {
			if m.Ty.Size() == 0 {
				continue
			}
			if only != nil {
				return false
			}
			only = m
		}
		if only == nil || only.Offset != 0 {
			return false
		}
		t = only.Ty
		if t.Kind == cc.TyArray && t.ArrayLen == 1 {
			t = t.Base
		}
	}
	return isLD(t)
}

// ---- loads and stores ----

// load replaces the address in rax with the value of type t
func (g *gen) load(t *cc.Type) {
	switch t.Kind {
	case cc.TyArray, cc.TyStruct, cc.TyUnion, cc.TyComplex, cc.TyFunc, cc.TyVLA:
		return
	case cc.TyFloat:
		g.a.MovF(false, X(XMM0), M(RAX, 0))
		return
	case cc.TyDouble:
		g.a.MovF(true, X(XMM0), M(RAX, 0))
		return
	case cc.TyLongDouble:
		g.a.Fld(10, M(RAX, 0))
		return
	}
	sz := t.Size()
	uns := t.IsUnsigned()
	if sz == 16 {
		g.a.Mov(8, Q(RDX), M(RAX, 8))
		g.a.Mov(8, Q(RAX), M(RAX, 0))
		return
	}
	switch sz {
	case 1:
		if uns {
			g.a.Movzx(4, RAX, 1, M(RAX, 0))
		} else {
			g.a.Movsx(8, RAX, 1, M(RAX, 0))
		}
	case 2:
		if uns {
			g.a.Movzx(4, RAX, 2, M(RAX, 0))
		} else {
			g.a.Movsx(8, RAX, 2, M(RAX, 0))
		}
	case 4:
		if uns {
			g.a.Mov(4, D(RAX), M(RAX, 0))
		} else {
			g.a.Movsx(8, RAX, 4, M(RAX, 0))
		}
	default:
		g.a.Mov(8, Q(RAX), M(RAX, 0))
	}
}

// store writes the value to the address popped from the stack
func (g *gen) store(t *cc.Type) {
	g.pop(RDI)
	switch t.Kind {
	case cc.TyStruct, cc.TyUnion, cc.TyComplex:
		g.copyMem(RDI, RAX, t.Size())
		return
	case cc.TyFloat:
		g.a.MovF(false, M(RDI, 0), X(XMM0))
		return
	case cc.TyDouble:
		g.a.MovF(true, M(RDI, 0), X(XMM0))
		return
	case cc.TyLongDouble:
		g.a.Fstp(10, M(RDI, 0))
		g.a.Fld(10, M(RDI, 0))
		return
	}
	switch t.Size() {
	case 1:
		g.a.Mov(1, M(RDI, 0), B(RAX))
	case 2:
		g.a.Mov(2, M(RDI, 0), W(RAX))
	case 4:
		g.a.Mov(4, M(RDI, 0), D(RAX))
	case 16:
		g.a.Mov(8, M(RDI, 0), Q(RAX))
		g.a.Mov(8, M(RDI, 8), Q(RDX))
	default:
		g.a.Mov(8, M(RDI, 0), Q(RAX))
	}
}

// copyMem copies n bytes from (src) to (dst) using r8 as scratch
func (g *gen) copyMem(dst, src Reg, n int64) {
	if n > 128 {
		// rep movsb: needs rdi, rsi, rcx
		g.a.Push(RSI)
		g.a.Push(RDI)
		g.a.Push(RCX)
		if src != RSI {
			if dst == RSI {
				g.a.Mov(8, Q(R11), Q(RSI))
				g.a.Mov(8, Q(RSI), Q(src))
				g.a.Mov(8, Q(RDI), Q(R11))
			} else {
				g.a.Mov(8, Q(RSI), Q(src))
			}
		}
		if dst != RDI && dst != RSI {
			g.a.Mov(8, Q(RDI), Q(dst))
		}
		g.a.MovImm(RCX, n)
		g.a.RepMovsb()
		g.a.Pop(RCX)
		g.a.Pop(RDI)
		g.a.Pop(RSI)
		return
	}
	i := int64(0)
	for ; i+8 <= n; i += 8 {
		g.a.Mov(8, Q(R8), M(src, int32(i)))
		g.a.Mov(8, M(dst, int32(i)), Q(R8))
	}
	if i+4 <= n {
		g.a.Mov(4, D(R8), M(src, int32(i)))
		g.a.Mov(4, M(dst, int32(i)), D(R8))
		i += 4
	}
	if i+2 <= n {
		g.a.Mov(2, W(R8), M(src, int32(i)))
		g.a.Mov(2, M(dst, int32(i)), W(R8))
		i += 2
	}
	if i < n {
		g.a.Mov(1, B(R8), M(src, int32(i)))
		g.a.Mov(1, M(dst, int32(i)), B(R8))
	}
}

// storeSized writes the low n (1..8) bytes of a register to memory
func (g *gen) storeSized(m Mem, r Reg, n int64) {
	switch n {
	case 1:
		g.a.Mov(1, m, B(r))
	case 2:
		g.a.Mov(2, m, W(r))
	case 4:
		g.a.Mov(4, m, D(r))
	case 8:
		g.a.Mov(8, m, Q(r))
	case 16:
		g.a.Mov(8, m, Q(RAX))
		m.Disp += 8
		g.a.Mov(8, m, Q(RDX))
	default:
		g.a.Mov(8, Q(R11), Q(r))
		for i := int64(0); i < n; i++ {
			mm := m
			mm.Disp += int32(i)
			g.a.Mov(1, mm, B(R11))
			g.a.ShiftImm(SHR, 8, Q(R11), 8)
		}
	}
}

// loadSized reads n (1..8) bytes from memory into a register
func (g *gen) loadSized(r Reg, m Mem, n int64) {
	switch n {
	case 1:
		g.a.Movzx(4, r, 1, m)
	case 2:
		g.a.Movzx(4, r, 2, m)
	case 4:
		g.a.Mov(4, D(r), m)
	case 8:
		g.a.Mov(8, Q(r), m)
	default:
		g.a.Alu(XOR, 4, D(r), D(r))
		for i := n - 1; i >= 0; i-- {
			mm := m
			mm.Disp += int32(i)
			g.a.ShiftImm(SHL, 8, Q(r), 8)
			g.a.Mov(1, B(r), mm)
		}
	}
}

// ---- addresses ----

func (g *gen) isLocalDef(o *cc.Obj) bool {
	d, ok := g.defined[o.AsmName]
	return ok && d != nil
}

func (g *gen) genAddr(n *cc.Node) {
	switch n.Kind {
	case cc.NVar:
		v := n.Var
		if _, ok := g.varReg(v); ok {
			g.fail(n, "internal error: address of register variable %s", v.Name)
		}
		if v.IsLocal {
			if _, ok := g.overAligned[v]; ok || v.Ty.Kind == cc.TyVLA {
				g.a.Mov(8, Q(RAX), M(RBP, int32(v.Offset)))
				return
			}
			g.a.Lea(RAX, M(RBP, int32(v.Offset)))
			return
		}
		if v.IsTLS {
			if g.isLocalDef(v) {
				g.a.Mov(8, Q(RAX), Mem{FS: true, Base: NoReg, Index: NoReg})
				g.a.Lea(RAX, Mem{Base: RAX, Index: NoReg, Sym: v.AsmName, Rel: RelTPOFF})
			} else {
				g.a.Mov(8, Q(RAX), Mem{Base: RIP, Index: NoReg, Sym: v.AsmName, Rel: RelGOTTPOF})
				g.a.Alu(ADD, 8, Q(RAX), Mem{FS: true, Base: NoReg, Index: NoReg})
			}
			return
		}
		if g.isLocalDef(v) || v.IsStatic {
			g.a.Lea(RAX, MSym(v.AsmName, 0))
			return
		}
		g.a.Mov(8, Q(RAX), MGot(v.AsmName))
		return
	case cc.NDeref:
		g.genExpr(n.LHS)
		return
	case cc.NComma:
		g.genDiscard(n.LHS)
		g.genAddr(n.RHS)
		return
	case cc.NMember:
		g.genAddr(n.LHS)
		if n.Member.Offset != 0 {
			g.a.Alu(ADD, 8, Q(RAX), Imm(n.Member.Offset))
		}
		return
	case cc.NFuncall:
		if n.RetBuffer != nil || n.Ty.IsRecord() {
			g.genExpr(n)
			return
		}
	case cc.NAssign, cc.NCond, cc.NStmtExpr:
		if n.Ty.IsRecord() {
			g.genExpr(n)
			return
		}
	case cc.NVLAPtr:
		g.a.Lea(RAX, M(RBP, int32(n.Var.Offset)))
		return
	case cc.NCast:
		if n.Ty.IsComplex() {
			g.genExpr(n)
			return
		}
		if n.Ty.IsRecord() {
			g.genAddr(n.LHS)
			return
		}
	case cc.NVaArg, cc.NMakeComplex:
		g.genExpr(n)
		return
	}
	g.fail(n, "not an lvalue")
}

// ---- expressions ----

// genDiscard evaluates an expression for its side effects only
func (g *gen) genDiscard(n *cc.Node) {
	if n == nil {
		return
	}
	n = pureDiscard(n)
	if n.Kind == cc.NAssign && n.LHS.Kind == cc.NVar {
		if r, ok := g.varReg(n.LHS.Var); ok {
			g.assignRegVar(n, r, false)
			return
		}
	}
	if n.Kind == cc.NAssign && g.storeConst(n) {
		return
	}
	g.genExpr(n)
	if n.Ty != nil && isLD(n.Ty) {
		g.a.FstpSt(0)
	}
}

func (g *gen) cmpZero(t *cc.Type) {
	switch t.Kind {
	case cc.TyFloat, cc.TyDouble:
		g.a.Xorp(true, XMM1, X(XMM1))
		g.a.Ucomi(isDouble(t), XMM0, X(XMM1))
		return
	case cc.TyLongDouble:
		g.a.Fldz()
		g.a.Fucomip(1)
		g.a.FstpSt(0)
		return
	}
	if is128(t) {
		g.a.Mov(8, Q(RCX), Q(RAX))
		g.a.Alu(OR, 8, Q(RCX), Q(RDX))
		return
	}
	if t.IsInteger() && t.Size() <= 4 {
		g.a.Alu(CMP, 4, D(RAX), Imm(0))
	} else {
		g.a.Alu(CMP, 8, Q(RAX), Imm(0))
	}
}

// genCond evaluates a condition and sets ZF (ZF=1 means false); for
// floats the NaN case counts as true
func (g *gen) genBranchFalse(n *cc.Node, target Label) {
	switch n.Kind {
	case cc.NLogAnd:
		g.genBranchFalse(n.LHS, target)
		g.genBranchFalse(n.RHS, target)
		return
	case cc.NLogOr:
		ok := g.a.NewLabel()
		g.genBranchTrue(n.LHS, ok)
		g.genBranchFalse(n.RHS, target)
		g.a.Bind(ok)
		return
	case cc.NNot:
		g.genBranchTrue(n.LHS, target)
		return
	case cc.NEq, cc.NNe, cc.NLt, cc.NLe:
		if isIntCompare(n) {
			c := g.genIntCompare(n)
			g.a.Jcc(c^1, target)
			return
		}
	}
	g.genExpr(n)
	t := n.Ty
	if t.IsFlonum() {
		g.cmpZero(t)
		// false only when equal and ordered
		skip := g.a.NewLabel()
		g.a.Jcc(CondP, skip)
		g.a.Jcc(CondE, target)
		g.a.Bind(skip)
		return
	}
	g.cmpZero(t)
	g.a.Jcc(CondE, target)
}

// genBranchTrue jumps to target when the condition holds
func (g *gen) genBranchTrue(n *cc.Node, target Label) {
	switch n.Kind {
	case cc.NLogAnd:
		no := g.a.NewLabel()
		g.genBranchFalse(n.LHS, no)
		g.genBranchTrue(n.RHS, target)
		g.a.Bind(no)
		return
	case cc.NLogOr:
		g.genBranchTrue(n.LHS, target)
		g.genBranchTrue(n.RHS, target)
		return
	case cc.NNot:
		g.genBranchFalse(n.LHS, target)
		return
	case cc.NEq, cc.NNe, cc.NLt, cc.NLe:
		if isIntCompare(n) {
			c := g.genIntCompare(n)
			g.a.Jcc(c, target)
			return
		}
	}
	g.genExpr(n)
	t := n.Ty
	g.cmpZero(t)
	if t.IsFlonum() {
		g.a.Jcc(CondP, target) // NaN is true
	}
	g.a.Jcc(CondNE, target)
}

func isIntCompare(n *cc.Node) bool {
	lt := n.LHS.Ty
	return !lt.IsFlonum() && !lt.IsRecord() && !is128(lt)
}

func (g *gen) genExpr(n *cc.Node) {
	switch n.Kind {
	case cc.NNull:
		return
	case cc.NNum:
		g.genNum(n)
		return
	case cc.NNeg:
		g.genExpr(n.LHS)
		switch n.Ty.Kind {
		case cc.TyFloat:
			g.a.Mov(4, D(RAX), Imm(math.MinInt32))
			g.a.MovqToX(4, XMM1, RAX)
			g.a.Xorp(false, XMM0, X(XMM1))
		case cc.TyDouble:
			g.a.MovImm(RAX, math.MinInt64)
			g.a.MovqToX(8, XMM1, RAX)
			g.a.Xorp(true, XMM0, X(XMM1))
		case cc.TyLongDouble:
			g.a.Fchs()
		case cc.TyInt128:
			g.a.Unary(NEG, 8, Q(RAX))
			g.a.Alu(ADC, 8, Q(RDX), Imm(0))
			g.a.Unary(NEG, 8, Q(RDX))
		default:
			g.a.Unary(NEG, intSize(n.Ty), Sized(RAX, intSize(n.Ty)))
			g.fixInt32(n.Ty)
		}
		return
	case cc.NVar:
		if r, ok := g.varReg(n.Var); ok {
			if r.IsXMM() {
				g.a.MovF(true, X(XMM0), X(r))
			} else {
				g.a.Mov(8, Q(RAX), Q(r))
			}
			return
		}
		if !n.Var.IsLocal && directScalar(n.Ty) {
			if m, ok := g.lvalMem(n); ok {
				g.loadDirect(n.Ty, m)
				return
			}
		}
		if n.Var.IsLocal && n.Ty.IsScalar() && n.Ty.Kind != cc.TyLongDouble && n.Ty.Size() <= 8 && !g.isOver(n.Var) {
			g.loadDirect(n.Ty, M(RBP, int32(n.Var.Offset)))
			return
		}
		g.genAddr(n)
		g.load(n.Ty)
		return
	case cc.NMember:
		if directScalar(n.Ty) {
			if m, ok := g.lvalMem(n); ok {
				g.loadDirect(n.Ty, m)
				return
			}
		}
		g.genAddr(n)
		g.load(n.Ty)
		m := n.Member
		if m.Bitfield {
			g.a.ShiftImm(SHL, 8, Q(RAX), int(64-m.BitWidth-m.BitOffset))
			if m.Ty.IsUnsigned() {
				g.a.ShiftImm(SHR, 8, Q(RAX), int(64-m.BitWidth))
			} else {
				g.a.ShiftImm(SAR, 8, Q(RAX), int(64-m.BitWidth))
			}
		}
		return
	case cc.NDeref:
		if directScalar(n.Ty) {
			if m, ok := g.lvalMem(n); ok {
				g.loadDirect(n.Ty, m)
				return
			}
		}
		g.genExpr(n.LHS)
		g.load(n.Ty)
		return
	case cc.NAddr:
		g.genAddr(n.LHS)
		return
	case cc.NAssign:
		g.genAssign(n)
		return
	case cc.NStmtExpr:
		for _, s := range n.Body {
			if s == n.Body[len(n.Body)-1] && s.Kind == cc.NExprStmt {
				g.genExpr(s.LHS)
				return
			}
			g.genStmt(s)
		}
		return
	case cc.NComma:
		g.genDiscard(n.LHS)
		g.genExpr(n.RHS)
		return
	case cc.NCast:
		if n.Ty.IsComplex() || n.LHS.Ty.IsComplex() {
			g.genComplexCast(n)
			return
		}
		if v, ok := intConst(n); ok {
			g.a.MovImm(RAX, v)
			return
		}
		if f, ok := floatConst(n); ok {
			g.genNum(&cc.Node{Kind: cc.NNum, Ty: n.Ty, FVal: f, Tok: n.Tok})
			return
		}
		g.genExpr(n.LHS)
		g.cast(n.LHS.Ty, n.Ty)
		return
	case cc.NMakeComplex:
		off := g.vaTemps[n]
		bt := n.Ty.Base
		g.genExpr(n.LHS)
		g.storeElem(bt, M(RBP, int32(off)))
		g.genExpr(n.RHS)
		g.storeElem(bt, M(RBP, int32(off+bt.Size())))
		g.a.Lea(RAX, M(RBP, int32(off)))
		return
	case cc.NMemzero:
		g.memzero(n.Var)
		return
	case cc.NCond:
		els := g.a.NewLabel()
		end := g.a.NewLabel()
		g.genBranchFalse(n.Cond, els)
		g.genExpr(n.Then)
		g.a.Jmp(end)
		g.a.Bind(els)
		g.genExpr(n.Els)
		g.a.Bind(end)
		return
	case cc.NNot:
		g.genExpr(n.LHS)
		g.cmpZero(n.LHS.Ty)
		g.a.Setcc(CondE, RAX)
		if n.LHS.Ty.IsFlonum() {
			g.a.Setcc(CondNP, RDX)
			g.a.Alu(AND, 1, B(RAX), B(RDX))
		}
		g.a.Movzx(4, RAX, 1, B(RAX))
		return
	case cc.NBitNot:
		g.genExpr(n.LHS)
		if is128(n.Ty) {
			g.a.Unary(NOT, 8, Q(RAX))
			g.a.Unary(NOT, 8, Q(RDX))
			return
		}
		g.a.Unary(NOT, intSize(n.Ty), Sized(RAX, intSize(n.Ty)))
		g.fixInt32(n.Ty)
		return
	case cc.NLogAnd:
		f := g.a.NewLabel()
		end := g.a.NewLabel()
		g.genBranchFalse(n.LHS, f)
		g.genBranchFalse(n.RHS, f)
		g.a.Mov(4, D(RAX), Imm(1))
		g.a.Jmp(end)
		g.a.Bind(f)
		g.a.Alu(XOR, 4, D(RAX), D(RAX))
		g.a.Bind(end)
		return
	case cc.NLogOr:
		t := g.a.NewLabel()
		f := g.a.NewLabel()
		end := g.a.NewLabel()
		g.genBranchFalse(n.LHS, f)
		g.a.Jmp(t)
		g.a.Bind(f)
		f2 := g.a.NewLabel()
		g.genBranchFalse(n.RHS, f2)
		g.a.Bind(t)
		g.a.Mov(4, D(RAX), Imm(1))
		g.a.Jmp(end)
		g.a.Bind(f2)
		g.a.Alu(XOR, 4, D(RAX), D(RAX))
		g.a.Bind(end)
		return
	case cc.NFuncall:
		g.genCall(n)
		return
	case cc.NLabelVal:
		g.a.LabelAddr(RAX, g.label(n.UniqueLabel))
		return
	case cc.NCas:
		g.genCas(n)
		return
	case cc.NExch:
		g.genExpr(n.CasAddr)
		g.push()
		g.genExpr(n.RHS)
		g.pop(RDI)
		sz := int(n.CasAddr.Ty.Base.Size())
		g.a.Xchg(sz, M(RDI, 0), RAX)
		g.extendTo64(n.CasAddr.Ty.Base)
		return
	case cc.NFetchOp:
		g.genFetchOp(n)
		return
	case cc.NFence:
		g.a.Mfence()
		return
	case cc.NVaStart:
		g.genVaStart(n)
		return
	case cc.NVaArg:
		g.genVaArg(n)
		return
	case cc.NVaCopy:
		g.genExpr(n.LHS)
		g.push()
		g.genExpr(n.RHS)
		g.pop(RDI)
		g.copyMem(RDI, RAX, 24)
		return
	case cc.NAlloca:
		g.genExpr(n.LHS)
		g.genAlloca()
		return
	case cc.NBuiltin:
		g.genBuiltin(n)
		return
	case cc.NFrameAddr:
		g.a.Mov(8, Q(RAX), Q(RBP))
		return
	}
	// binary operators
	lt := n.LHS.Ty
	if lt.IsFlonum() && isSSE(lt) {
		g.genFloatBinary(n)
		return
	}
	if isLD(lt) {
		g.genLDBinary(n)
		return
	}
	if is128(lt) {
		g.genInt128Binary(n)
		return
	}
	g.genIntBinary(n)
}

// loadInto loads an integer or pointer from memory into a register
func (g *gen) loadInto(r Reg, t *cc.Type, m Mem) {
	uns := t.IsUnsigned()
	switch t.Size() {
	case 1:
		if uns {
			g.a.Movzx(4, r, 1, m)
		} else {
			g.a.Movsx(8, r, 1, m)
		}
	case 2:
		if uns {
			g.a.Movzx(4, r, 2, m)
		} else {
			g.a.Movsx(8, r, 2, m)
		}
	case 4:
		if uns {
			g.a.Mov(4, D(r), m)
		} else {
			g.a.Movsx(8, r, 4, m)
		}
	default:
		g.a.Mov(8, Q(r), m)
	}
}

// loadDirect loads a scalar from memory into rax/xmm0
func (g *gen) loadDirect(t *cc.Type, m Mem) {
	switch t.Kind {
	case cc.TyFloat:
		g.a.MovF(false, X(XMM0), m)
		return
	case cc.TyDouble:
		g.a.MovF(true, X(XMM0), m)
		return
	}
	uns := t.IsUnsigned()
	switch t.Size() {
	case 1:
		if uns {
			g.a.Movzx(4, RAX, 1, m)
		} else {
			g.a.Movsx(8, RAX, 1, m)
		}
	case 2:
		if uns {
			g.a.Movzx(4, RAX, 2, m)
		} else {
			g.a.Movsx(8, RAX, 2, m)
		}
	case 4:
		if uns {
			g.a.Mov(4, D(RAX), m)
		} else {
			g.a.Movsx(8, RAX, 4, m)
		}
	default:
		g.a.Mov(8, Q(RAX), m)
	}
}

func (g *gen) isOver(v *cc.Obj) bool {
	_, ok := g.overAligned[v]
	return ok
}

// fixInt32 re-extends a 32-bit signed result to 64 bits
func (g *gen) fixInt32(t *cc.Type) {
	if intSize(t) == 4 && !t.IsUnsigned() {
		g.a.Movsx(8, RAX, 4, D(RAX))
	}
}

func intSize(t *cc.Type) int {
	if t.Size() == 8 || t.Kind == cc.TyPtr {
		return 8
	}
	return 4
}

// extendTo64 sign/zero-extends rax from the size of t
func (g *gen) extendTo64(t *cc.Type) {
	uns := t.IsUnsigned()
	switch t.Size() {
	case 1:
		if uns {
			g.a.Movzx(4, RAX, 1, B(RAX))
		} else {
			g.a.Movsx(8, RAX, 1, B(RAX))
		}
	case 2:
		if uns {
			g.a.Movzx(4, RAX, 2, W(RAX))
		} else {
			g.a.Movsx(8, RAX, 2, W(RAX))
		}
	case 4:
		if uns {
			g.a.Mov(4, D(RAX), D(RAX))
		} else {
			g.a.Movsx(8, RAX, 4, D(RAX))
		}
	}
}

func (g *gen) genNum(n *cc.Node) {
	switch n.Ty.Kind {
	case cc.TyFloat:
		bits := math.Float32bits(float32(n.FVal))
		if bits == 0 {
			g.a.Xorp(false, XMM0, X(XMM0))
			return
		}
		g.a.Mov(4, D(RAX), Imm(int64(bits)))
		g.a.MovqToX(4, XMM0, RAX)
		return
	case cc.TyDouble:
		bits := math.Float64bits(n.FVal)
		if bits == 0 {
			g.a.Xorp(true, XMM0, X(XMM0))
			return
		}
		g.a.MovImm(RAX, int64(bits))
		g.a.MovqToX(8, XMM0, RAX)
		return
	case cc.TyLongDouble:
		var data []byte
		if n.LD != nil {
			data = cc.Float80(n.LD)
		} else {
			data = cc.Float80FromFloat64(n.FVal)
		}
		data = append(data, make([]byte, 6)...)
		if isZero(data) {
			g.a.Fldz()
			return
		}
		g.a.Fld(10, MSym(g.constLabel(data, 16), 0))
		return
	}
	g.a.MovImm(RAX, n.Val)
	if is128(n.Ty) {
		g.extend128(cc.TypeLong)
	}
}

func (g *gen) genAssign(n *cc.Node) {
	lhs := n.LHS
	if lhs.Kind == cc.NVar {
		if r, ok := g.varReg(lhs.Var); ok {
			g.assignRegVar(n, r, true)
			return
		}
	}
	// fast path: local scalar variable
	if lhs.Kind == cc.NVar && lhs.Var.IsLocal && lhs.Ty.IsScalar() && !isLD(lhs.Ty) && lhs.Var.Ty.Kind != cc.TyVLA && !g.isOver(lhs.Var) {
		g.genExpr(n.RHS)
		m := M(RBP, int32(lhs.Var.Offset))
		switch lhs.Ty.Kind {
		case cc.TyFloat:
			g.a.MovF(false, m, X(XMM0))
		case cc.TyDouble:
			g.a.MovF(true, m, X(XMM0))
		default:
			g.storeSized(m, RAX, lhs.Ty.Size())
		}
		return
	}
	if directScalar(lhs.Ty) && lhs.Kind != cc.NVar {
		if m, ok := g.lvalMem(lhs); ok {
			g.genExpr(n.RHS)
			g.storeAt(lhs.Ty, m)
			return
		}
	}
	if directScalar(lhs.Ty) && !(lhs.Kind == cc.NMember && lhs.Member.Bitfield) {
		g.genAddr(lhs)
		if t, ok := g.saveTemp(); ok {
			g.genExpr(n.RHS)
			g.storeAt(lhs.Ty, M(t, 0))
			g.freeTemp()
			return
		}
		g.push()
		g.genExpr(n.RHS)
		g.store(n.Ty)
		return
	}
	g.genAddr(lhs)
	g.push()
	g.genExpr(n.RHS)
	if lhs.Kind == cc.NMember && lhs.Member.Bitfield {
		m := lhs.Member
		w := m.BitWidth
		g.a.Mov(8, Q(R8), Q(RAX))
		g.a.Mov(8, Q(RDI), Q(RAX))
		mask := int64(-1)
		if w < 64 {
			mask = int64(1)<<uint(w) - 1
		}
		g.a.MovImm(R9, mask)
		g.a.Alu(AND, 8, Q(RDI), Q(R9))
		if m.BitOffset > 0 {
			g.a.ShiftImm(SHL, 8, Q(RDI), int(m.BitOffset))
		}
		g.a.Mov(8, Q(RAX), M(RSP, 0))
		g.load(m.Ty)
		g.a.MovImm(R9, mask<<uint(m.BitOffset))
		g.a.Unary(NOT, 8, Q(R9))
		g.a.Alu(AND, 8, Q(RAX), Q(R9))
		g.a.Alu(OR, 8, Q(RAX), Q(RDI))
		g.store(m.Ty)
		// the value of the assignment is the new bit-field value
		g.a.Mov(8, Q(RAX), Q(R8))
		if w < 64 {
			g.a.ShiftImm(SHL, 8, Q(RAX), int(64-w))
			if m.Ty.IsUnsigned() {
				g.a.ShiftImm(SHR, 8, Q(RAX), int(64-w))
			} else {
				g.a.ShiftImm(SAR, 8, Q(RAX), int(64-w))
			}
		}
		return
	}
	g.store(n.Ty)
}

func (g *gen) memzero(v *cc.Obj) {
	size := v.Ty.Size()
	if size <= 0 {
		return
	}
	if r, ok := g.varReg(v); ok {
		g.a.Alu(XOR, 4, D(r), D(r))
		return
	}
	off := v.Offset
	base := RBP
	if g.isOver(v) {
		g.a.Mov(8, Q(R11), M(RBP, int32(off)))
		base, off = R11, 0
	}
	if size > 64 {
		g.a.Push(RDI)
		g.a.Lea(RDI, M(base, int32(off)))
		g.a.MovImm(RCX, size)
		g.a.Alu(XOR, 4, D(RAX), D(RAX))
		g.a.RepStosb()
		g.a.Pop(RDI)
		return
	}
	g.a.Alu(XOR, 4, D(RAX), D(RAX))
	i := int64(0)
	for ; i+8 <= size; i += 8 {
		g.a.Mov(8, M(base, int32(off+i)), Q(RAX))
	}
	for ; i+4 <= size; i += 4 {
		g.a.Mov(4, M(base, int32(off+i)), D(RAX))
	}
	for ; i < size; i++ {
		g.a.Mov(1, M(base, int32(off+i)), B(RAX))
	}
}

// ---- binary operators ----

// simpleOperand returns a memory/immediate operand for side-effect-free
// right operands that can be used without pushing
func (g *gen) simpleRHS(n *cc.Node) (Operand, bool) {
	if n.Kind == cc.NNum && n.Ty.IsInteger() && fitsI32(n.Val) {
		return Imm(n.Val), true
	}
	return nil, false
}

// isSimpleVar tells whether n is a local scalar that can be loaded directly
func isSimpleVar(n *cc.Node) bool {
	return n.Kind == cc.NVar && n.Var.IsLocal && n.Var.Ty.Kind != cc.TyVLA && (n.Ty.IsInteger() || n.Ty.Kind == cc.TyPtr)
}

// intOperands evaluates the operands of an integer binary node: the left
// one into rax and the right one into rdi (or an immediate). It returns
// the operation size, signedness and the immediate (nil when in rdi).
func (g *gen) intOperands(n *cc.Node, allowImm bool) (int, bool, Operand) {
	lt := n.LHS.Ty
	sz := 4
	if lt.Size() == 8 || lt.Kind == cc.TyPtr || lt.IsArray() || lt.Kind == cc.TyFunc || lt.Kind == cc.TyNullptr {
		sz = 8
	}
	uns := lt.IsUnsigned() || lt.Kind == cc.TyPtr || lt.Kind == cc.TyNullptr || lt.IsArray() || lt.Kind == cc.TyFunc
	var imm Operand
	c, isConst := intConst(n.RHS)
	switch {
	case allowImm && isConst && fitsI32(c):
		g.genExpr(n.LHS)
		imm = Imm(c)
	case isReg(g.opReg(n.RHS, sz)):
		g.genExpr(n.LHS)
		r, _ := g.opReg(n.RHS, sz)
		imm = Sized(r, sz)
	case commutative(n.Kind) && isReg(g.opReg(n.LHS, sz)):
		g.genExpr(n.RHS)
		r, _ := g.opReg(n.LHS, sz)
		imm = Sized(r, sz)
	case isSimpleVar(n.RHS) && g.regVars[n.RHS.Var] != 0:
		// a register variable narrower than the operation
		g.genExpr(n.LHS)
		g.a.Mov(8, Q(RDI), Q(g.regVars[n.RHS.Var]))
	case isSimpleVar(n.RHS) && !g.isOver(n.RHS.Var):
		g.genExpr(n.LHS)
		g.loadInto(RDI, n.RHS.Ty, M(RBP, int32(n.RHS.Var.Offset)))
	case n.Kind != cc.NShl && n.Kind != cc.NShr && isMem(g.memOperand(n.RHS, sz)):
		g.genExpr(n.LHS)
		imm, _ = g.memOperand(n.RHS, sz)
	case commutative(n.Kind) && isMem(g.memOperand(n.LHS, sz)):
		g.genExpr(n.RHS)
		imm, _ = g.memOperand(n.LHS, sz)
	case g.isLeaf(n.LHS):
		g.genExpr(n.RHS)
		g.a.Mov(8, Q(RDI), Q(RAX))
		g.genExpr(n.LHS)
	default:
		g.genExpr(n.RHS)
		if t, ok := g.saveTemp(); ok {
			g.genExpr(n.LHS)
			imm = Sized(t, sz)
			g.freeTemp()
			break
		}
		g.push()
		g.genExpr(n.LHS)
		g.pop(RDI)
	}
	return sz, uns, imm
}

// genIntCompare emits the comparison and returns the condition that is
// true when the relation holds
func (g *gen) genIntCompare(n *cc.Node) Cond {
	if c, ok := g.compareRegVar(n); ok {
		return c
	}
	sz, uns, imm := g.intOperands(n, true)
	if imm != nil {
		g.a.Alu(CMP, sz, Sized(RAX, sz), imm)
	} else {
		g.a.Alu(CMP, sz, Sized(RAX, sz), Sized(RDI, sz))
	}
	return compareCond(n.Kind, uns)
}

func compareCond(k cc.NodeKind, uns bool) Cond {
	switch k {
	case cc.NEq:
		return CondE
	case cc.NNe:
		return CondNE
	case cc.NLt:
		if uns {
			return CondB
		}
		return CondL
	}
	if uns {
		return CondBE
	}
	return CondLE
}

func (g *gen) genIntBinary(n *cc.Node) {
	if n.Kind == cc.NEq || n.Kind == cc.NNe || n.Kind == cc.NLt || n.Kind == cc.NLe {
		c := g.genIntCompare(n)
		g.a.Setcc(c, RAX)
		g.a.Movzx(4, RAX, 1, B(RAX))
		return
	}
	sz, uns, imm := g.intOperands(n, n.Kind != cc.NDiv && n.Kind != cc.NMod)
	ax := Sized(RAX, sz)
	var src Operand = Sized(RDI, sz)
	if imm != nil {
		src = imm
	}
	switch n.Kind {
	case cc.NAdd:
		g.a.Alu(ADD, sz, ax, src)
	case cc.NSub:
		g.a.Alu(SUB, sz, ax, src)
	case cc.NMul:
		if c, ok := imm.(Imm); ok && c > 0 && c&(c-1) == 0 {
			k := 0
			for int64(1)<<k != int64(c) {
				k++
			}
			if k > 0 {
				g.a.ShiftImm(SHL, sz, ax, k)
			}
		} else if c, ok := imm.(Imm); ok {
			g.a.Imul3(sz, RAX, ax, int64(c))
		} else {
			g.a.Imul2(sz, RAX, src)
		}
	case cc.NDiv, cc.NMod:
		if uns {
			g.a.Alu(XOR, 4, D(RDX), D(RDX))
			g.a.Unary(DIV, sz, src)
		} else {
			if sz == 8 {
				g.a.Cqo()
			} else {
				g.a.Cdq()
			}
			g.a.Unary(IDIV, sz, src)
		}
		if n.Kind == cc.NMod {
			g.a.Mov(8, Q(RAX), Q(RDX))
		}
	case cc.NBitAnd:
		g.a.Alu(AND, sz, ax, src)
	case cc.NBitOr:
		g.a.Alu(OR, sz, ax, src)
	case cc.NBitXor:
		g.a.Alu(XOR, sz, ax, src)
	case cc.NShl, cc.NShr:
		op := SAR
		switch {
		case n.Kind == cc.NShl:
			op = SHL
		case uns:
			op = SHR
		}
		if c, ok := imm.(Imm); ok {
			g.a.ShiftImm(op, sz, ax, int(int64(c)&int64(sz*8-1)))
		} else {
			g.a.Mov(4, D(RCX), Sized(src.(R).Reg, 4))
			g.a.ShiftCL(op, sz, ax)
		}
	default:
		g.fail(n, "invalid expression")
	}
	// keep 32-bit signed results sign-extended to 64 bits (writing a
	// 32-bit register already zero-extends unsigned ones)
	if sz == 4 && !n.Ty.IsUnsigned() {
		g.a.Movsx(8, RAX, 4, D(RAX))
	}
}

func (g *gen) genFloatBinary(n *cc.Node) {
	dbl := isDouble(n.LHS.Ty)
	switch {
	case g.floatSrc(n.RHS) != nil:
		g.genExpr(n.LHS)
		g.a.MovF(dbl, X(XMM1), g.floatSrc(n.RHS))
	case (n.Kind == cc.NAdd || n.Kind == cc.NMul) && g.floatSrc(n.LHS) != nil:
		// commutative: the left operand is used in place
		g.genExpr(n.RHS)
		op := FAdd
		if n.Kind == cc.NMul {
			op = FMul
		}
		g.a.FArith(op, dbl, XMM0, g.floatSrc(n.LHS))
		return
	case g.floatLeaf(n.LHS):
		g.genExpr(n.RHS)
		g.a.MovF(true, X(XMM1), X(XMM0))
		g.genExpr(n.LHS)
	default:
		g.genExpr(n.RHS)
		if t, ok := g.saveTempF(); ok {
			g.genExpr(n.LHS)
			g.a.MovqToX(8, XMM1, t)
			g.freeTemp()
			break
		}
		g.pushf()
		g.genExpr(n.LHS)
		g.popf(XMM1)
	}
	switch n.Kind {
	case cc.NAdd:
		g.a.FArith(FAdd, dbl, XMM0, X(XMM1))
	case cc.NSub:
		g.a.FArith(FSub, dbl, XMM0, X(XMM1))
	case cc.NMul:
		g.a.FArith(FMul, dbl, XMM0, X(XMM1))
	case cc.NDiv:
		g.a.FArith(FDiv, dbl, XMM0, X(XMM1))
	case cc.NEq:
		g.a.Ucomi(dbl, XMM1, X(XMM0))
		g.a.Setcc(CondE, RAX)
		g.a.Setcc(CondNP, RDX)
		g.a.Alu(AND, 1, B(RAX), B(RDX))
		g.a.Movzx(4, RAX, 1, B(RAX))
	case cc.NNe:
		g.a.Ucomi(dbl, XMM1, X(XMM0))
		g.a.Setcc(CondNE, RAX)
		g.a.Setcc(CondP, RDX)
		g.a.Alu(OR, 1, B(RAX), B(RDX))
		g.a.Movzx(4, RAX, 1, B(RAX))
	case cc.NLt:
		g.a.Ucomi(dbl, XMM1, X(XMM0))
		g.a.Setcc(CondA, RAX)
		g.a.Movzx(4, RAX, 1, B(RAX))
	case cc.NLe:
		g.a.Ucomi(dbl, XMM1, X(XMM0))
		g.a.Setcc(CondAE, RAX)
		g.a.Movzx(4, RAX, 1, B(RAX))
	default:
		g.fail(n, "invalid operands to a floating-point operator")
	}
}

func (g *gen) genLDBinary(n *cc.Node) {
	g.genExpr(n.RHS)
	g.pushLD()
	g.genExpr(n.LHS)
	g.popLD() // st0 = rhs, st1 = lhs
	switch n.Kind {
	case cc.NAdd:
		g.a.Faddp()
	case cc.NSub:
		g.a.Fsubp()
	case cc.NMul:
		g.a.Fmulp()
	case cc.NDiv:
		g.a.Fdivp()
	case cc.NEq, cc.NNe, cc.NLt, cc.NLe:
		g.a.Fucomip(1) // compares st0 (rhs) with st1 (lhs)
		g.a.FstpSt(0)
		switch n.Kind {
		case cc.NEq:
			g.a.Setcc(CondE, RAX)
			g.a.Setcc(CondNP, RDX)
			g.a.Alu(AND, 1, B(RAX), B(RDX))
		case cc.NNe:
			g.a.Setcc(CondNE, RAX)
			g.a.Setcc(CondP, RDX)
			g.a.Alu(OR, 1, B(RAX), B(RDX))
		case cc.NLt:
			g.a.Setcc(CondA, RAX)
		case cc.NLe:
			g.a.Setcc(CondAE, RAX)
		}
		g.a.Movzx(4, RAX, 1, B(RAX))
	default:
		g.fail(n, "invalid operands to a long double operator")
	}
}

// ---- atomics ----

func (g *gen) genCas(n *cc.Node) {
	elem := n.CasAddr.Ty.Base
	sz := int(elem.Size())
	g.genExpr(n.CasAddr)
	g.push()
	g.genExpr(n.CasNew)
	g.push()
	g.genExpr(n.CasOld)
	if n.Begin == 2 {
		// CasOld is a pointer to the expected value
		g.a.Mov(8, Q(R8), Q(RAX))
		g.load(elem)
	}
	g.pop(RDX)
	g.pop(RDI)
	g.a.LockCmpxchg(sz, M(RDI, 0), RDX)
	switch n.Begin {
	case 1:
		g.extendTo64(elem)
	case 2:
		g.a.Setcc(CondE, RCX)
		ok := g.a.NewLabel()
		g.a.Jcc(CondE, ok)
		g.a.Mov(sz, M(R8, 0), Sized(RAX, sz))
		g.a.Bind(ok)
		g.a.Movzx(4, RAX, 1, B(RCX))
	default:
		g.a.Setcc(CondE, RAX)
		g.a.Movzx(4, RAX, 1, B(RAX))
	}
}

func (g *gen) genFetchOp(n *cc.Node) {
	elem := n.LHS.Ty.Base
	if n.LHS.Ty.Kind != cc.TyPtr {
		elem = cc.DecayType(n.LHS.Ty).Base
	}
	sz := int(elem.Size())
	g.genExpr(n.LHS)
	g.push()
	g.genExpr(n.RHS)
	if elem.Kind == cc.TyPtr && (n.Val == cc.FetchAdd || n.Val == cc.FetchSub) {
		// pointer arithmetic in units of the pointed-to type
		if s := elem.Base.Size(); s > 1 {
			g.a.Imul3(8, RAX, Q(RAX), s)
		}
	}
	g.a.Mov(8, Q(RSI), Q(RAX))
	g.pop(RDI)
	r := Sized(RSI, sz)
	switch n.Val {
	case cc.FetchAdd, cc.FetchSub:
		g.a.Mov(8, Q(RAX), Q(RSI))
		if n.Val == cc.FetchSub {
			g.a.Unary(NEG, 8, Q(RAX))
		}
		g.a.LockXadd(sz, M(RDI, 0), RAX)
		if n.Begin == 1 {
			if n.Val == cc.FetchSub {
				g.a.Alu(SUB, max(sz, 4), Sized(RAX, max(sz, 4)), Sized(RSI, max(sz, 4)))
			} else {
				g.a.Alu(ADD, max(sz, 4), Sized(RAX, max(sz, 4)), Sized(RSI, max(sz, 4)))
			}
		}
		g.extendTo64(elem)
		return
	}
	loop := g.a.NewLabel()
	g.a.Mov(sz, Sized(RAX, sz), M(RDI, 0))
	g.a.Bind(loop)
	g.a.Mov(8, Q(RDX), Q(RAX))
	switch n.Val {
	case cc.FetchAnd:
		g.a.Alu(AND, sz, Sized(RDX, sz), r)
	case cc.FetchOr:
		g.a.Alu(OR, sz, Sized(RDX, sz), r)
	case cc.FetchXor:
		g.a.Alu(XOR, sz, Sized(RDX, sz), r)
	case cc.FetchNand:
		g.a.Alu(AND, sz, Sized(RDX, sz), r)
		g.a.Unary(NOT, sz, Sized(RDX, sz))
	}
	g.a.LockCmpxchg(sz, M(RDI, 0), RDX)
	g.a.Jcc(CondNE, loop)
	if n.Begin == 1 {
		g.a.Mov(8, Q(RAX), Q(RDX))
	}
	g.extendTo64(elem)
}

// ---- alloca ----

func (g *gen) genAlloca() {
	bottom := int32(g.fn.AllocaBottom.Offset)
	g.a.Mov(8, Q(RDI), Q(RAX))
	g.a.Alu(ADD, 8, Q(RDI), Imm(15))
	g.a.Alu(AND, 8, Q(RDI), Imm(-16))
	// move the temporary area (pushed values) down by rdi bytes
	g.a.Mov(8, Q(RCX), M(RBP, bottom))
	g.a.Alu(SUB, 8, Q(RCX), Q(RSP))
	g.a.Mov(8, Q(RAX), Q(RSP))
	g.a.Alu(SUB, 8, Q(RSP), Q(RDI))
	g.a.Mov(8, Q(RDX), Q(RSP))
	loop := g.a.NewLabel()
	done := g.a.NewLabel()
	g.a.Bind(loop)
	g.a.Alu(CMP, 8, Q(RCX), Imm(0))
	g.a.Jcc(CondE, done)
	g.a.Mov(1, B(R8), M(RAX, 0))
	g.a.Mov(1, M(RDX, 0), B(R8))
	g.a.Alu(ADD, 8, Q(RDX), Imm(1))
	g.a.Alu(ADD, 8, Q(RAX), Imm(1))
	g.a.Alu(SUB, 8, Q(RCX), Imm(1))
	g.a.Jmp(loop)
	g.a.Bind(done)
	g.a.Mov(8, Q(RAX), M(RBP, bottom))
	g.a.Alu(SUB, 8, Q(RAX), Q(RDI))
	g.a.Mov(8, M(RBP, bottom), Q(RAX))
}
