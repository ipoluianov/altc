package x64

import (
	"fmt"
	"math"
	"os"

	"github.com/ipoluianov/altc/cc"
)

var (
	irDisabled = os.Getenv("ALTC_NOIR") != ""
	irStats    = os.Getenv("ALTC_IR_STATS") != ""
	irStrict   = os.Getenv("ALTC_IR_STRICT") != ""
	irDump     = os.Getenv("ALTC_IR_DUMP")
)

// irFunction compiles a function through the IR; false means the caller
// must use the stack-machine generator
func (g *gen) irFunction(fn *cc.Obj) (ok bool) {
	g.irReason = ""
	// -O0 keeps every function in the stack machine, except those with
	// vector values, which only the IR backend compiles
	if g.noRegAlloc && !usesVectors(fn) || irDisabled {
		g.irReason = "the optimizing backend is disabled"
		return false
	}
	f, err := g.lowerFunction(fn)
	if err != nil {
		g.irReason = err.Error()
		if irStats {
			fmt.Fprintf(os.Stderr, "ir: %s: not supported: %v\n", fn.Name, err)
		}
		return false
	}
	f.curTok = nil // values made by the optimizations take the position of the code before
	f.curSite = nil
	f.isa = g.isa
	f.noVectorize = os.Getenv("ALTC_NOVECTORIZE") != ""
	f.fastMath = g.fastMath
	mk := g.a.mark()
	defer func() {
		if r := recover(); r != nil {
			if irStrict {
				panic(r)
			}
			if irStats {
				fmt.Fprintf(os.Stderr, "ir: %s: internal error: %v\n", fn.Name, r)
			}
			g.irReason = fmt.Sprint(r)
			g.a.reset(mk)
			ok = false
		}
	}()
	f.optimize()
	ra := f.allocate()
	if irDump != "" && (irDump == fn.Name || irDump == "*") {
		fmt.Fprintln(os.Stderr, f)
	}
	g.fn = fn
	g.asmSlots = map[*cc.AsmOperand]int64{}
	em := &irEmitter{g: g, a: g.a, f: f, ra: ra}
	em.emit()
	if g.debug {
		g.irDbg = em.debugInfo()
	}
	if irStats {
		fmt.Fprintf(os.Stderr, "ir: %s: ok\n", fn.Name)
	}
	return true
}

type irEmitter struct {
	g        *gen
	a        *Asm
	events   []dbgEvent // positions and their code offsets (-g)
	sites    []*dbgSite // inlined calls (-g)
	f        *irFunc
	ra       *irAlloc
	labels   map[*irBlock]Label
	epilogue Label
	saveOff  map[Reg]int32
	next     *irBlock
	flags    flagCond // condition of a fused comparison
	ymm      bool     // the function uses 256-bit registers: vzeroupper before calls and returns
	outArea  int64    // the outgoing argument area at the bottom of the frame
	peek     *irValue // the next instruction that emits code (in the same block)
	carryFor *irValue // an op128 high half whose low half just left the carry in the flags
	mulLow   *irValue // an opMul whose product is in rax (after the mul of an opMulHi)
}

type flagKind uint8

const (
	flagNormal flagKind = iota
	flagFEq             // equal and ordered
	flagFNe             // not equal or unordered
)

type flagCond struct {
	kind flagKind
	cond Cond
}

// argLocs places the arguments of a call or the parameters of a function
func argLocs(types []irType) ([]irLoc, int64, int) {
	locs := make([]irLoc, len(types))
	gp, fp := 0, 0
	stack := int64(0)
	for i, t := range types {
		switch {
		case t.isFloat() && fp < 8:
			locs[i] = irLoc{kind: locReg, reg: XMM0 + Reg(fp)}
			fp++
		case !t.isFloat() && gp < 6:
			locs[i] = irLoc{kind: locReg, reg: argGP[gp]}
			gp++
		default:
			locs[i] = irLoc{kind: locStack, off: int32(stack)}
			stack += 8
		}
	}
	return locs, stack, fp
}

func (e *irEmitter) emit() {
	f, a := e.f, e.a
	// frame
	off := int64(0)
	for _, s := range f.Slots {
		if s.Fixed {
			continue
		}
		off += s.Size
		off = alignTo(off, max(s.Align, 1))
		s.Offset = -off
	}
	e.saveOff = map[Reg]int32{}
	for _, r := range e.ra.usedCallee {
		off += 8
		e.saveOff[r] = int32(-off)
	}
	outgoing := int64(0)
	for _, b := range f.Blocks {
		for _, v := range b.Instrs {
			if v.Op == opCall {
				outgoing = max(outgoing, v.Call.StackSize)
			}
		}
	}
	e.outArea = alignTo(outgoing, 16)
	frame := alignTo(off, 16) + e.outArea
	for _, p := range f.Params {
		if p != nil && p.Type == iV256 {
			e.ymm = true
		}
	}
	for _, b := range f.Blocks {
		for _, v := range b.Instrs {
			if v.Type == iV256 {
				e.ymm = true
			}
		}
	}

	e.labels = map[*irBlock]Label{}
	for _, b := range f.Blocks {
		e.labels[b] = a.NewLabel()
	}
	e.epilogue = a.NewLabel()

	a.FramePrologue()
	if frame > 0 {
		a.Alu(SUB, 8, Q(RSP), Imm(frame))
	}
	for _, r := range e.ra.usedCallee {
		a.SaveReg(r, e.saveOff[r])
	}
	if vs := f.VaSlot; vs != nil {
		off := int32(vs.Offset)
		for i, r := range argGP {
			a.Mov(8, M(RBP, off+int32(i*8)), Q(r))
		}
		skip := a.NewLabel()
		a.Test(1, B(RAX), B(RAX))
		a.Jcc(CondE, skip)
		for i := 0; i < 8; i++ {
			a.MovF(true, M(RBP, off+48+int32(i*16)), X(XMM0+Reg(i)))
		}
		a.Bind(skip)
	}
	// parameters to their locations
	var moves []pmove
	for i, p := range f.Params {
		if p == nil || !p.needsLoc() {
			continue
		}
		pl := f.PLocs[i]
		src := irLoc{kind: locReg, reg: pl.Reg}
		if pl.Reg == NoReg {
			src = irLoc{kind: locStack, off: int32(pl.Stack)}
		}
		moves = append(moves, pmove{dst: p.loc, src: src, t: p.Type})
	}
	e.parallelMove(moves)

	// blocks that only pass control on are not emitted
	var blocks []*irBlock
	for _, b := range f.Blocks {
		if b != f.Entry && e.target(b) != b {
			continue
		}
		blocks = append(blocks, b)
	}
	pos := map[*irBlock]int{}
	for i, b := range blocks {
		pos[b] = i
	}
	for i, b := range blocks {
		e.next = nil
		if i+1 < len(blocks) {
			e.next = blocks[i+1]
		}
		if i > 0 && e.loopHead(b, i, pos) {
			// loop heads start on a 16-byte boundary when at most 10 bytes
			// of padding get there (gcc's .p2align 4,,10)
			if pad := (16 - len(a.Code)%16) % 16; pad > 0 && pad <= 10 {
				if a.Listing != nil {
					a.Listing.WriteString("\t.p2align 4,,10\n")
				}
				a.Nops(pad)
			}
		}
		a.Bind(e.labels[b])
		dbg := e.g.debug
		if dbg {
			e.events = append(e.events, dbgEvent{b.start, len(a.Code)})
		}
		for i, v := range b.Instrs {
			e.peek = nil
			for _, w := range b.Instrs[i+1:] {
				if !w.Op.remat() {
					e.peek = w
					break
				}
			}
			lm := a.lineMark()
			if v.tok != nil {
				e.g.lineAt(v.tok)
			}
			if dbg {
				e.events = append(e.events, dbgEvent{v.pos, len(a.Code)})
			}
			lo := len(a.Code)
			e.value(v)
			if len(a.Code) == lo {
				a.lineRestore(lm) // no code (a constant, a fused compare): the line goes to what comes next
			}
			if dbg && v.site != nil && len(a.Code) > lo {
				if !e.seen(v.site) {
					e.sites = append(e.sites, v.site)
				}
				noteSite(v.site, lo, len(a.Code))
			}
		}
		if dbg {
			e.events = append(e.events, dbgEvent{b.end - 4, len(a.Code)})
		}
		e.terminator(b)
		if dbg {
			e.events = append(e.events, dbgEvent{b.end, len(a.Code)})
		}
	}
	a.Bind(e.epilogue)
	if e.ymm && f.RetType != iV256 {
		e.text("vzeroupper")
	}
	for _, r := range e.ra.usedCallee {
		a.Mov(8, Q(r), M(RBP, e.saveOff[r]))
	}
	a.FrameReturn(false)
}

// ---- locations and operands ----

func (e *irEmitter) locMem(l irLoc) Mem {
	if l.slot != nil {
		return M(RBP, int32(l.slot.Offset))
	}
	return M(RBP, l.off)
}

func (e *irEmitter) regOf(v *irValue) (Reg, bool) {
	if v.loc.kind == locReg && !v.Op.remat() {
		return v.loc.reg, true
	}
	return NoReg, false
}

func (e *irEmitter) inReg(v *irValue, r Reg) bool {
	vr, ok := e.regOf(v)
	return ok && vr == r
}

// materialize puts an integer value into r
func (e *irEmitter) materialize(v *irValue, r Reg) {
	switch v.Op {
	case opConst:
		e.a.MovImm(r, v.AuxInt)
		return
	case opGlobal:
		e.a.Lea(r, MSym(v.Sym, 0))
		return
	case opSlot:
		e.a.Lea(r, M(RBP, int32(v.Slot.Offset)))
		return
	}
	switch v.loc.kind {
	case locReg:
		if v.loc.reg != r {
			e.a.Mov(8, Q(r), Q(v.loc.reg))
		}
	case locStack:
		e.a.Mov(8, Q(r), e.locMem(v.loc))
	default:
		panic(fmt.Sprintf("v%d has no location", v.ID))
	}
}

// ireg returns a register holding v, using scratch when needed
func (e *irEmitter) ireg(v *irValue, scratch Reg) Reg {
	if r, ok := e.regOf(v); ok {
		return r
	}
	e.materialize(v, scratch)
	return scratch
}

// iop returns an operand of size sz for v: a register, memory or (when
// allowed) an immediate
func (e *irEmitter) iop(v *irValue, sz int, scratch Reg, allowImm bool) Operand {
	if allowImm && v.Op == opConst && fitsI32(v.AuxInt) {
		return Imm(v.AuxInt)
	}
	if r, ok := e.regOf(v); ok {
		return Sized(r, sz)
	}
	if !v.Op.remat() && v.loc.kind == locStack {
		return e.locMem(v.loc)
	}
	e.materialize(v, scratch)
	return Sized(scratch, sz)
}

// dst is the register a result is computed in
func (e *irEmitter) dst(v *irValue, scratch Reg) Reg {
	if v.loc.kind == locReg {
		return v.loc.reg
	}
	return scratch
}

// finish stores a result computed in r to the location of v
func (e *irEmitter) finish(v *irValue, r Reg) {
	switch v.loc.kind {
	case locReg:
		if v.loc.reg == r {
			return
		}
		if r.IsXMM() {
			e.xcopy(v.loc.reg, r, v.Type)
		} else {
			e.a.Mov(8, Q(v.loc.reg), Q(r))
		}
	case locStack:
		if r.IsXMM() {
			e.xstore(e.locMem(v.loc), r, v.Type)
		} else {
			e.a.Mov(8, e.locMem(v.loc), Q(r))
		}
	}
}

func (e *irEmitter) floatConstMem(t irType, f float64) Mem {
	var data []byte
	if t == iF32 {
		b := math.Float32bits(float32(f))
		data = []byte{byte(b), byte(b >> 8), byte(b >> 16), byte(b >> 24)}
	} else {
		b := math.Float64bits(f)
		for i := 0; i < 8; i++ {
			data = append(data, byte(b>>(8*i)))
		}
	}
	return MSym(e.g.constLabel(data, int64(len(data))), 0)
}

// fmaterialize puts a float value into the xmm register x
func (e *irEmitter) fmaterialize(v *irValue, x Reg) {
	if v.Type.isVec() {
		e.vmaterialize(v, x)
		return
	}
	dbl := v.Type == iF64
	if v.Op == opConst {
		if math.Float64bits(v.F) == 0 {
			e.a.Xorp(true, x, X(x))
			return
		}
		e.a.MovF(dbl, X(x), e.floatConstMem(v.Type, v.F))
		return
	}
	switch v.loc.kind {
	case locReg:
		if v.loc.reg != x {
			e.a.Movaps(x, v.loc.reg)
		}
	case locStack:
		e.a.MovF(dbl, X(x), e.locMem(v.loc))
	default:
		panic(fmt.Sprintf("v%d has no location", v.ID))
	}
}

func (e *irEmitter) freg(v *irValue, scratch Reg) Reg {
	if r, ok := e.regOf(v); ok {
		return r
	}
	e.fmaterialize(v, scratch)
	return scratch
}

// fop returns an xmm register or memory operand holding a float value
func (e *irEmitter) fop(v *irValue, scratch Reg) Operand {
	if r, ok := e.regOf(v); ok {
		return X(r)
	}
	if v.Type.isVec() {
		e.vmaterialize(v, scratch)
		return X(scratch)
	}
	if v.Op == opConst {
		if math.Float64bits(v.F) == 0 {
			e.a.Xorp(true, scratch, X(scratch))
			return X(scratch)
		}
		return e.floatConstMem(v.Type, v.F)
	}
	if v.loc.kind == locStack {
		return e.locMem(v.loc)
	}
	panic(fmt.Sprintf("v%d has no location", v.ID))
}

// memOf builds the memory operand of a load or store
func (e *irEmitter) memOf(m *irMem, args []*irValue, bs, is Reg) Mem {
	if m.Sym != "" {
		return MSym(m.Sym, int32(m.Disp))
	}
	mm := Mem{Base: NoReg, Index: NoReg}
	i := 0
	if m.HasBase {
		mm.Base = e.ireg(args[i], bs)
		i++
	}
	if m.HasIndex {
		mm.Index = e.ireg(args[i], is)
		mm.Scale = m.Scale
	}
	disp := m.Disp
	if m.Slot != nil {
		mm.Base = RBP
		disp += m.Slot.Offset
	}
	mm.Disp = int32(disp)
	return mm
}

// ---- instructions ----

var intConds = map[irOp]Cond{
	opEq: CondE, opNe: CondNE, opLt: CondL, opLe: CondLE, opGt: CondG, opGe: CondGE,
	opULt: CondB, opULe: CondBE, opUGt: CondA, opUGe: CondAE,
}

var swappedCompare = map[irOp]irOp{
	opEq: opEq, opNe: opNe, opLt: opGt, opLe: opGe, opGt: opLt, opGe: opLe,
	opULt: opUGt, opULe: opUGe, opUGt: opULt, opUGe: opULe,
}

func (e *irEmitter) value(v *irValue) {
	a := e.a
	switch v.Op {
	case opConst, opGlobal, opSlot, opParam:
		return
	case opGlobalGOT:
		if !v.needsLoc() {
			return
		}
		d := e.dst(v, R11)
		a.Mov(8, Q(d), MGot(v.Sym))
		e.finish(v, d)
	case opTLS:
		if !v.needsLoc() {
			return
		}
		d := e.dst(v, R11)
		if v.Signed {
			a.Mov(8, Q(d), Mem{FS: true, Base: NoReg, Index: NoReg})
			a.Lea(d, Mem{Base: d, Index: NoReg, Sym: v.Sym, Rel: RelTPOFF})
		} else {
			a.Mov(8, Q(d), Mem{Base: RIP, Index: NoReg, Sym: v.Sym, Rel: RelGOTTPOF})
			a.Alu(ADD, 8, Q(d), Mem{FS: true, Base: NoReg, Index: NoReg})
		}
		e.finish(v, d)
	case opAdd, opSub:
		if w := e.peek; carryPair(v, w) {
			// the low half of a 128-bit sum: the high half adds the carry
			e.intBinaryFlags(v)
			e.carryFor = w
			return
		}
		e.intBinary(v)
	case opMul, opAnd, opOr, opXor:
		if v == e.mulLow {
			e.mulLow = nil
			e.finish(v, RAX)
			return
		}
		e.intBinary(v)
	case opShl, opShr, opSar:
		e.shift(v)
	case opDiv, opUDiv, opMod, opUMod:
		e.divide(v)
	case opMulHi:
		e.materialize(v.Args[0], RAX)
		var src Operand
		if r, ok := e.regOf(v.Args[1]); ok {
			src = Q(r)
		} else if !v.Args[1].Op.remat() && v.Args[1].loc.kind == locStack {
			src = e.locMem(v.Args[1].loc)
		} else {
			e.materialize(v.Args[1], R10)
			src = Q(R10)
		}
		if v.Signed {
			a.Unary(IMUL, 8, src)
		} else {
			a.Unary(MUL, 8, src)
			// the low half of the same product follows: it is in rax
			if w := e.peek; w != nil && w.Op == opMul && w.Type == iI64 &&
				(w.Args[0] == v.Args[0] && w.Args[1] == v.Args[1] || w.Args[0] == v.Args[1] && w.Args[1] == v.Args[0]) {
				e.mulLow = w
			}
		}
		e.finish(v, RDX)
	case opNeg, opNot:
		sz := v.Type.size()
		d := e.dst(v, R11)
		e.materialize(v.Args[0], d)
		op := NEG
		if v.Op == opNot {
			op = NOT
		}
		a.Unary(op, sz, Sized(d, sz))
		e.finish(v, d)
		if w := e.peek; v.Op == opNeg && carryPair(v, w) {
			e.carryFor = w // neg sets the borrow of 0 - x
		}
	case opEq, opNe, opLt, opLe, opGt, opGe, opULt, opULe, opUGt, opUGe:
		c := e.intCompare(v)
		if v.fused {
			e.flags = flagCond{flagNormal, c}
			return
		}
		e.setFlagsResult(v, flagCond{flagNormal, c})
	case opFEq, opFNe, opFLt, opFLe, opFGt, opFGe:
		fc := e.floatCompare(v)
		if v.fused {
			e.flags = fc
			return
		}
		e.setFlagsResult(v, fc)
	case opFAdd, opFSub, opFMul, opFDiv:
		e.floatBinary(v)
	case opFNeg:
		dbl := v.Type == iF64
		d := e.dst(v, XMM15)
		e.fmaterialize(v.Args[0], d)
		mask := make([]byte, 16)
		if dbl {
			mask[7] = 0x80
		} else {
			mask[3] = 0x80
		}
		a.Xorp(dbl, d, MSym(e.g.constLabel(mask, 16), 0))
		e.finish(v, d)
	case opSExt8, opSExt16, opZExt8, opZExt16:
		e.extend(v)
	case opSExt32:
		d := e.dst(v, R11)
		a.Movsx(8, d, 4, e.iop(v.Args[0], 4, R10, false))
		e.finish(v, d)
	case opZExt32, opTrunc:
		d := e.dst(v, R11)
		a.Mov(4, D(d), e.iop(v.Args[0], 4, R10, false))
		e.finish(v, d)
	case opCvtI2F:
		x := v.Args[0]
		d := e.dst(v, XMM15)
		a.CvtIntToF(v.Type == iF64, d, x.Type.size(), e.iop(x, x.Type.size(), R10, false))
		e.finish(v, d)
	case opCvtF2I:
		d := e.dst(v, R11)
		a.CvtFToInt(v.Args[0].Type == iF64, v.Type.size(), d, e.fop(v.Args[0], XMM14))
		e.finish(v, d)
	case opCvtU2F:
		e.cvtU2F(v)
	case opCvtF2U:
		e.cvtF2U(v)
	case opFExt, opFTrunc:
		d := e.dst(v, XMM15)
		a.CvtFF(v.Op == opFExt, d, e.fop(v.Args[0], XMM14))
		e.finish(v, d)
	case opLoad:
		e.load(v)
	case opStore:
		e.store(v)
	case opMemzero:
		e.memzero(v)
	case opMemcpy:
		e.memcpy(v)
	case opFence:
		a.Mfence()
	case opTrap:
		a.Ud2()
	case opAsm:
		e.asm(v)
	case opVInsn:
		e.vinsn(v)
	case opVBroadcast:
		e.vbroadcast(v)
	case opBuiltin:
		e.materialize(v.Args[0], RAX)
		e.g.unaryBuiltin(v.Sym, int(v.AuxInt))
		e.finish(v, RAX)
	case opFAbs:
		dbl := v.Type == iF64
		d := e.dst(v, XMM15)
		e.fmaterialize(v.Args[0], d)
		mask := make([]byte, 16)
		if dbl {
			for i := 0; i < 7; i++ {
				mask[i] = 0xFF
			}
			mask[7] = 0x7F
		} else {
			mask[0], mask[1], mask[2], mask[3] = 0xFF, 0xFF, 0xFF, 0x7F
		}
		a.Andp(dbl, d, MSym(e.g.constLabel(mask, 16), 0))
		e.finish(v, d)
	case opFBits:
		d := e.dst(v, R11)
		a.MovqFromX(v.Type.size(), d, e.freg(v.Args[0], XMM14))
		e.finish(v, d)
	case opCall:
		e.call(v)
	case opX87:
		e.x87(v)
	case opAtomic:
		e.atomic(v)
	case op128:
		e.op128(v)
	case opAlloca:
		// below the frame, under the outgoing arguments (which move down)
		e.materialize(v.Args[0], RAX)
		a.Alu(ADD, 8, Q(RAX), Imm(15))
		a.Alu(AND, 8, Q(RAX), Imm(-16))
		a.Alu(SUB, 8, Q(RSP), Q(RAX))
		if v.needsLoc() {
			d := e.dst(v, R11)
			a.Lea(d, M(RSP, int32(e.outArea)))
			e.finish(v, d)
		}
	case opStackSave:
		if v.needsLoc() {
			d := e.dst(v, R11)
			a.Mov(8, Q(d), Q(RSP))
			e.finish(v, d)
		}
	case opStackRestore:
		a.Mov(8, Q(RSP), Q(e.ireg(v.Args[0], R11)))
	case opPair:
		panic("a 128-bit pair is left in the IR")
	default:
		panic(fmt.Sprintf("cannot emit %s", opNames[v.Op]))
	}
}

func (e *irEmitter) intBinary(v *irValue) {
	a := e.a
	sz := v.Type.size()
	x, y := v.Args[0], v.Args[1]
	d := e.dst(v, R11)
	if v.Op.commutative() && e.inReg(y, d) && !e.inReg(x, d) {
		x, y = y, x
	}
	if e.inReg(y, d) && x != y {
		d = R11
	}
	if v.Op == opAdd {
		if c, ok := constOf(y); ok && fitsI32(c) {
			if xr, ok := e.regOf(x); ok && xr != d {
				a.Lea(d, M(xr, int32(c)))
				e.finish(v, d)
				return
			}
		}
	}
	e.materialize(x, d)
	switch v.Op {
	case opAdd:
		a.Alu(ADD, sz, Sized(d, sz), e.iop(y, sz, R10, true))
	case opSub:
		a.Alu(SUB, sz, Sized(d, sz), e.iop(y, sz, R10, true))
	case opAnd:
		a.Alu(AND, sz, Sized(d, sz), e.iop(y, sz, R10, true))
	case opOr:
		a.Alu(OR, sz, Sized(d, sz), e.iop(y, sz, R10, true))
	case opXor:
		a.Alu(XOR, sz, Sized(d, sz), e.iop(y, sz, R10, true))
	case opMul:
		if c, ok := constOf(y); ok && fitsI32(c) {
			a.Imul3(sz, d, Sized(d, sz), c)
		} else {
			a.Imul2(sz, d, e.iop(y, sz, R10, false))
		}
	}
	e.finish(v, d)
}

// intBinaryFlags emits an add or sub that leaves its carry in the flags
// (no lea)
func (e *irEmitter) intBinaryFlags(v *irValue) {
	a := e.a
	x, y := v.Args[0], v.Args[1]
	d := e.dst(v, R11)
	if v.Op == opAdd && e.inReg(y, d) && !e.inReg(x, d) {
		x, y = y, x
	}
	if e.inReg(y, d) && x != y {
		d = R11
	}
	e.materialize(x, d)
	op := ADD
	if v.Op == opSub {
		op = SUB
	}
	a.Alu(op, 8, Q(d), e.iop(y, 8, R10, true))
	e.finish(v, d)
}

// carryPair tells whether v computes the low half of the 128-bit sum,
// difference or negation whose high half is w
func carryPair(v, w *irValue) bool {
	if w == nil || w.Op != op128 || v.Type != iI64 {
		return false
	}
	k := i128Kind(w.AuxInt)
	switch {
	case v.Op == opAdd && k == i128AddHi:
		return v.Args[0] == w.Args[0] && v.Args[1] == w.Args[2] || v.Args[0] == w.Args[2] && v.Args[1] == w.Args[0]
	case v.Op == opSub && k == i128SubHi:
		return v.Args[0] == w.Args[0] && v.Args[1] == w.Args[2]
	case v.Op == opNeg && k == i128SubHi:
		c, ok := constOf(w.Args[0])
		return ok && c == 0 && w.Args[2] == v.Args[0]
	}
	return false
}

func (e *irEmitter) shift(v *irValue) {
	a := e.a
	sz := v.Type.size()
	op := map[irOp]ShiftOp{opShl: SHL, opShr: SHR, opSar: SAR}[v.Op]
	x, y := v.Args[0], v.Args[1]
	if c, ok := constOf(y); ok {
		d := e.dst(v, R11)
		e.materialize(x, d)
		a.ShiftImm(op, sz, Sized(d, sz), int(c&int64(sz*8-1)))
		e.finish(v, d)
		return
	}
	e.materialize(y, RCX)
	d := e.dst(v, R11)
	e.materialize(x, d)
	a.ShiftCL(op, sz, Sized(d, sz))
	e.finish(v, d)
}

func (e *irEmitter) divide(v *irValue) {
	a := e.a
	sz := v.Type.size()
	x, y := v.Args[0], v.Args[1]
	e.materialize(x, RAX)
	var div Operand
	if r, ok := e.regOf(y); ok {
		div = Sized(r, sz)
	} else if !y.Op.remat() && y.loc.kind == locStack {
		div = e.locMem(y.loc)
	} else {
		e.materialize(y, R10)
		div = Sized(R10, sz)
	}
	signed := v.Op == opDiv || v.Op == opMod
	if signed {
		if sz == 8 {
			a.Cqo()
		} else {
			a.Cdq()
		}
		a.Unary(IDIV, sz, div)
	} else {
		a.Alu(XOR, 4, D(RDX), D(RDX))
		a.Unary(DIV, sz, div)
	}
	res := RAX
	if v.Op == opMod || v.Op == opUMod {
		res = RDX
	}
	e.finish(v, res)
}

// intCompare sets the flags and returns the condition that holds when
// the comparison is true
func (e *irEmitter) intCompare(v *irValue) Cond {
	op := v.Op
	x, y := v.Args[0], v.Args[1]
	sz := x.Type.size()
	if x.Op.remat() && !y.Op.remat() {
		x, y = y, x
		op = swappedCompare[op]
	}
	var left Operand
	if r, ok := e.regOf(x); ok {
		left = Sized(r, sz)
	} else if !x.Op.remat() && x.loc.kind == locStack {
		left = e.locMem(x.loc)
	} else {
		e.materialize(x, R10)
		left = Sized(R10, sz)
	}
	right := e.iop(y, sz, R11, true)
	if _, lm := left.(Mem); lm {
		if _, rm := right.(Mem); rm {
			e.materialize(y, R11)
			right = Sized(R11, sz)
		}
	}
	e.a.Alu(CMP, sz, left, right)
	return intConds[op]
}

func (e *irEmitter) floatCompare(v *irValue) flagCond {
	x, y := v.Args[0], v.Args[1]
	dbl := x.Type == iF64
	fc := flagCond{kind: flagNormal}
	switch v.Op {
	case opFGt:
		fc.cond = CondA
	case opFGe:
		fc.cond = CondAE
	case opFLt:
		x, y = y, x
		fc.cond = CondA
	case opFLe:
		x, y = y, x
		fc.cond = CondAE
	case opFEq:
		fc.kind = flagFEq
	case opFNe:
		fc.kind = flagFNe
	}
	xr := e.freg(x, XMM14)
	e.a.Ucomi(dbl, xr, e.fop(y, XMM15))
	return fc
}

// setFlagsResult turns flags into 0 or 1
func (e *irEmitter) setFlagsResult(v *irValue, fc flagCond) {
	a := e.a
	switch fc.kind {
	case flagNormal:
		a.Setcc(fc.cond, RAX)
	case flagFEq:
		a.Setcc(CondE, RAX)
		a.Setcc(CondNP, RDX)
		a.Alu(AND, 1, B(RAX), B(RDX))
	case flagFNe:
		a.Setcc(CondNE, RAX)
		a.Setcc(CondP, RDX)
		a.Alu(OR, 1, B(RAX), B(RDX))
	}
	if !v.needsLoc() {
		return
	}
	d := e.dst(v, R11)
	a.Movzx(4, d, 1, B(RAX))
	e.finish(v, d)
}

func (e *irEmitter) floatBinary(v *irValue) {
	dbl := v.Type == iF64
	x, y := v.Args[0], v.Args[1]
	d := e.dst(v, XMM15)
	if v.Op.commutative() && e.inReg(y, d) && !e.inReg(x, d) {
		x, y = y, x
	}
	if e.inReg(y, d) && x != y {
		d = XMM15
	}
	e.fmaterialize(x, d)
	op := map[irOp]FOp{opFAdd: FAdd, opFSub: FSub, opFMul: FMul, opFDiv: FDiv}[v.Op]
	e.a.FArith(op, dbl, d, e.fop(y, XMM14))
	e.finish(v, d)
}

func (e *irEmitter) extend(v *irValue) {
	x := v.Args[0]
	srcSize := 1
	if v.Op == opSExt16 || v.Op == opZExt16 {
		srcSize = 2
	}
	var src Operand
	if r, ok := e.regOf(x); ok {
		src = Sized(r, srcSize)
	} else if !x.Op.remat() && x.loc.kind == locStack {
		src = e.locMem(x.loc)
	} else {
		e.materialize(x, R10)
		src = Sized(R10, srcSize)
	}
	d := e.dst(v, R11)
	dsz := v.Type.size()
	if v.Op == opSExt8 || v.Op == opSExt16 {
		e.a.Movsx(dsz, d, srcSize, src)
	} else {
		e.a.Movzx(4, d, srcSize, src)
	}
	e.finish(v, d)
}

func (e *irEmitter) cvtU2F(v *irValue) {
	a := e.a
	dbl := v.Type == iF64
	e.materialize(v.Args[0], R10)
	d := e.dst(v, XMM15)
	neg, end := a.NewLabel(), a.NewLabel()
	a.Test(8, Q(R10), Q(R10))
	a.Jcc(CondS, neg)
	a.CvtIntToF(dbl, d, 8, Q(R10))
	a.Jmp(end)
	a.Bind(neg)
	a.Mov(8, Q(R11), Q(R10))
	a.Alu(AND, 4, D(R11), Imm(1))
	a.ShiftImm(SHR, 8, Q(R10), 1)
	a.Alu(OR, 8, Q(R10), Q(R11))
	a.CvtIntToF(dbl, d, 8, Q(R10))
	a.FArith(FAdd, dbl, d, X(d))
	a.Bind(end)
	e.finish(v, d)
}

func (e *irEmitter) cvtF2U(v *irValue) {
	a := e.a
	x := v.Args[0]
	dbl := x.Type == iF64
	e.fmaterialize(x, XMM14)
	a.MovF(dbl, X(XMM15), e.floatConstMem(x.Type, 9223372036854775808.0))
	d := e.dst(v, R11)
	big, end := a.NewLabel(), a.NewLabel()
	a.Ucomi(dbl, XMM14, X(XMM15))
	a.Jcc(CondAE, big)
	a.CvtFToInt(dbl, 8, d, X(XMM14))
	a.Jmp(end)
	a.Bind(big)
	a.FArith(FSub, dbl, XMM14, X(XMM15))
	a.CvtFToInt(dbl, 8, d, X(XMM14))
	a.MovImm(R10, math.MinInt64)
	a.Alu(XOR, 8, Q(d), Q(R10))
	a.Bind(end)
	e.finish(v, d)
}

func (e *irEmitter) load(v *irValue) {
	a := e.a
	m := e.memOf(v.Mem, v.Args, RAX, R10)
	if v.Type.inXMM() {
		d := e.dst(v, XMM15)
		e.xload(d, v.Type, m)
		e.finish(v, d)
		return
	}
	d := e.dst(v, R11)
	dsz := v.Type.size()
	switch v.AuxInt {
	case 1, 2:
		if v.Signed {
			a.Movsx(dsz, d, int(v.AuxInt), m)
		} else {
			a.Movzx(4, d, int(v.AuxInt), m)
		}
	case 4:
		if dsz == 8 {
			if v.Signed {
				a.Movsx(8, d, 4, m)
			} else {
				a.Mov(4, D(d), m)
			}
		} else {
			a.Mov(4, D(d), m)
		}
	case 8:
		a.Mov(8, Q(d), m)
	default:
		e.loadBytes(d, m, v.AuxInt)
	}
	e.finish(v, d)
}

func (e *irEmitter) store(v *irValue) {
	a := e.a
	val := v.Args[0]
	m := e.memOf(v.Mem, v.Args[1:], RAX, R10)
	sz := int(v.AuxInt)
	if val.Type.inXMM() {
		e.xstore(m, e.freg(val, XMM15), val.Type)
		return
	}
	if sz != 1 && sz != 2 && sz != 4 && sz != 8 {
		e.materialize(val, R11)
		e.storeBytes(m, R11, int64(sz))
		return
	}
	if c, ok := constOf(val); ok && fitsI32(c) {
		switch sz {
		case 1:
			c = int64(int8(c))
		case 2:
			c = int64(int16(c))
		case 4:
			c = int64(int32(c))
		}
		a.Mov(sz, m, Imm(c))
		return
	}
	r := e.ireg(val, R11)
	a.Mov(sz, m, Sized(r, sz))
}

// memzero clears AuxInt bytes at the address in Args[0]
func (e *irEmitter) memzero(v *irValue) {
	a := e.a
	n := v.AuxInt
	a.Alu(XOR, 4, D(R11), D(R11))
	var base Mem
	if p := v.Args[0]; p.Op == opSlot {
		base = M(RBP, int32(p.Slot.Offset))
	} else {
		base = M(e.ireg(p, RAX), 0)
	}
	i := int64(0)
	if n > 128 {
		a.Lea(RAX, base)
		a.MovImm(RCX, n/8)
		loop := a.NewLabel()
		a.Bind(loop)
		a.Mov(8, M(RAX, 0), Q(R11))
		a.Alu(ADD, 8, Q(RAX), Imm(8))
		a.Alu(SUB, 8, Q(RCX), Imm(1))
		a.Jcc(CondNE, loop)
		base = M(RAX, 0)
		n %= 8
	}
	for ; i+8 <= n; i += 8 {
		a.Mov(8, mdisp(base, i), Q(R11))
	}
	for ; i+4 <= n; i += 4 {
		a.Mov(4, mdisp(base, i), D(R11))
	}
	for ; i < n; i++ {
		a.Mov(1, mdisp(base, i), B(R11))
	}
}

func mdisp(m Mem, d int64) Mem {
	m.Disp += int32(d)
	return m
}

// memcpy copies AuxInt bytes from Args[1] to Args[0]
func (e *irEmitter) memcpy(v *irValue) {
	a := e.a
	n := v.AuxInt
	if n > 128 {
		e.materialize(v.Args[0], RAX)
		e.materialize(v.Args[1], R10)
		a.MovImm(RCX, n/8)
		loop := a.NewLabel()
		a.Bind(loop)
		a.Mov(8, Q(R11), M(R10, 0))
		a.Mov(8, M(RAX, 0), Q(R11))
		a.Alu(ADD, 8, Q(RAX), Imm(8))
		a.Alu(ADD, 8, Q(R10), Imm(8))
		a.Alu(SUB, 8, Q(RCX), Imm(1))
		a.Jcc(CondNE, loop)
		e.copyTail(M(RAX, 0), M(R10, 0), n%8)
		return
	}
	dst := M(e.ireg(v.Args[0], RAX), 0)
	src := M(e.ireg(v.Args[1], R10), 0)
	e.copyTail(dst, src, n)
}

func (e *irEmitter) copyTail(dst, src Mem, n int64) {
	a := e.a
	i := int64(0)
	for ; i+8 <= n; i += 8 {
		a.Mov(8, Q(R11), mdisp(src, i))
		a.Mov(8, mdisp(dst, i), Q(R11))
	}
	for ; i+4 <= n; i += 4 {
		a.Mov(4, D(R11), mdisp(src, i))
		a.Mov(4, mdisp(dst, i), D(R11))
	}
	for ; i < n; i++ {
		a.Movzx(4, R11, 1, mdisp(src, i))
		a.Mov(1, mdisp(dst, i), B(R11))
	}
}

func (e *irEmitter) call(v *irValue) {
	a := e.a
	c := v.Call
	args := v.Args[:c.NArgs]
	// stack arguments first: their sources are still intact
	for i, x := range args {
		l := c.Locs[i]
		if l.Reg != NoReg {
			continue
		}
		m := M(RSP, int32(l.Stack))
		switch {
		case l.Copy > 0:
			e.copyTail(m, M(e.ireg(x, RAX), 0), l.Copy)
		case x.Type.inXMM():
			e.xstore(m, e.freg(x, XMM15), x.Type)
		default:
			if k, ok := constOf(x); ok && fitsI32(k) {
				a.Mov(8, m, Imm(k))
			} else {
				a.Mov(8, m, Q(e.ireg(x, R11)))
			}
		}
	}
	if c.Direct == "" {
		e.materialize(v.Args[c.NArgs], R11)
	}
	var moves []pmove
	for i, x := range args {
		if l := c.Locs[i]; l.Reg != NoReg {
			moves = append(moves, pmove{dst: irLoc{kind: locReg, reg: l.Reg}, val: x, t: x.Type})
		}
	}
	e.parallelMove(moves)
	if c.Variadic {
		a.Mov(4, D(RAX), Imm(int64(c.NFP)))
	}
	if e.ymm {
		wide := false
		for _, x := range args {
			wide = wide || x.Type == iV256
		}
		if !wide {
			e.text("vzeroupper")
		}
	}
	if c.Direct != "" {
		a.Call(c.Direct)
	} else {
		a.CallInd(Q(R11))
	}
	if c.RetX87 != nil {
		a.Fstp(10, M(RBP, int32(c.RetX87.Offset)))
		if c.RetX87Im != nil {
			a.Fstp(10, M(RBP, int32(c.RetX87Im.Offset)))
		}
		return
	}
	if c.RetSlot != nil {
		gi, fi := 0, 0
		for _, ch := range c.RetChunks {
			m := M(RBP, int32(c.RetSlot.Offset+ch.off))
			if ch.t.isFloat() {
				a.MovF(ch.t == iF64, m, X(XMM0+Reg(fi)))
				fi++
				continue
			}
			e.storeBytes(m, []Reg{RAX, RDX}[gi], ch.size)
			gi++
		}
		return
	}
	if v.needsLoc() {
		if v.Type.inXMM() {
			e.finish(v, XMM0)
		} else {
			e.finish(v, RAX)
		}
	}
}

// storeBytes stores the low n bytes (1..8) of r; r may be changed
func (e *irEmitter) storeBytes(m Mem, r Reg, n int64) {
	a := e.a
	switch n {
	case 1, 2, 4, 8:
		a.Mov(int(n), m, Sized(r, int(n)))
		return
	}
	if r != R11 {
		a.Mov(8, Q(R11), Q(r))
	}
	for i := int64(0); i < n; i++ {
		a.Mov(1, mdisp(m, i), B(R11))
		a.ShiftImm(SHR, 8, Q(R11), 8)
	}
}

// loadBytes loads n bytes (1..8) zero-extended into r
func (e *irEmitter) loadBytes(r Reg, m Mem, n int64) {
	a := e.a
	switch n {
	case 1, 2:
		a.Movzx(4, r, int(n), m)
	case 4:
		a.Mov(4, D(r), m)
	case 8:
		a.Mov(8, Q(r), m)
	default:
		a.Alu(XOR, 4, D(r), D(r))
		for i := n - 1; i >= 0; i-- {
			a.ShiftImm(SHL, 8, Q(r), 8)
			a.Mov(1, B(r), mdisp(m, i))
		}
	}
}

// ---- terminators ----

func (e *irEmitter) terminator(b *irBlock) {
	a := e.a
	switch b.Kind {
	case bJump:
		s := b.Succs[0]
		e.phiMoves(b, s)
		if t := e.target(s); t != e.next {
			a.Jmp(e.labels[t])
		}
	case bBranch:
		c := b.Ctrl
		fc := e.flags
		if !c.fused {
			if r, ok := e.regOf(c); ok {
				a.Test(4, D(r), D(r))
			} else if !c.Op.remat() && c.loc.kind == locStack {
				a.Alu(CMP, 4, e.locMem(c.loc), Imm(0))
			} else {
				e.materialize(c, R10)
				a.Test(4, D(R10), D(R10))
			}
			fc = flagCond{flagNormal, CondNE}
		}
		ts, fs := e.target(b.Succs[0]), e.target(b.Succs[1])
		t, f := e.labels[ts], e.labels[fs]
		tNext, fNext := ts == e.next, fs == e.next
		switch fc.kind {
		case flagNormal:
			switch {
			case tNext:
				a.Jcc(fc.cond^1, f)
			case fNext:
				a.Jcc(fc.cond, t)
			default:
				a.Jcc(fc.cond, t)
				a.Jmp(f)
			}
		case flagFEq:
			a.Jcc(CondP, f)
			if tNext {
				a.Jcc(CondNE, f)
			} else {
				a.Jcc(CondE, t)
				if !fNext {
					a.Jmp(f)
				}
			}
		case flagFNe:
			a.Jcc(CondP, t)
			if tNext {
				a.Jcc(CondE, f)
			} else {
				a.Jcc(CondNE, t)
				if !fNext {
					a.Jmp(f)
				}
			}
		}
	case bRet:
		if e.f.RetX87Pair {
			// the imaginary part goes to st(1), the real part to st(0)
			if len(b.RetVals) == 2 {
				a.Fld(10, e.x87Mem(b.RetVals[1], RAX))
				a.Fld(10, e.x87Mem(b.RetVals[0], RAX))
			} else {
				a.Fldz()
				a.Fldz()
			}
		} else if len(b.RetVals) > 0 {
			var moves []pmove
			gi, fi := 0, 0
			for _, x := range b.RetVals {
				r := []Reg{RAX, RDX}[min(gi, 1)]
				if x.Type.inXMM() {
					r = XMM0 + Reg(fi)
					fi++
				} else {
					gi++
				}
				moves = append(moves, pmove{dst: irLoc{kind: locReg, reg: r}, val: x, t: x.Type})
			}
			e.parallelMove(moves)
		}
		if e.f.RetX87 {
			// the value goes to st(0)
			if c := b.Ctrl; c != nil {
				a.Fld(10, e.x87Mem(c, RAX))
			} else {
				a.Fldz()
			}
		} else if c := b.Ctrl; c != nil {
			if c.Type.inXMM() {
				e.fmaterialize(c, XMM0)
			} else {
				e.materialize(c, RAX)
			}
		}
		if e.next != nil {
			a.Jmp(e.epilogue)
		}
	default:
		panic("block without terminator")
	}
}

// target skips blocks that only jump on without moving anything
func (e *irEmitter) target(b *irBlock) *irBlock {
	for i := 0; i < 8 && b.Kind == bJump && len(b.Instrs) == 0 && len(b.Phis) == 0 && !e.hasMoves(b, b.Succs[0]); i++ {
		b = b.Succs[0]
	}
	return b
}

// hasMoves tells whether the edge b->s needs phi moves
func (e *irEmitter) hasMoves(b, s *irBlock) bool {
	for j, p := range s.Preds {
		if p != b {
			continue
		}
		for _, phi := range s.Phis {
			if phi.needsLoc() {
				a := phi.Args[j]
				if a.Op.remat() || !e.sameLoc(phi.loc, a.loc) {
					return true
				}
			}
		}
	}
	return false
}

func (e *irEmitter) phiMoves(b, s *irBlock) {
	if len(s.Phis) == 0 {
		return
	}
	j := -1
	for i, p := range s.Preds {
		if p == b {
			j = i
			break
		}
	}
	var moves []pmove
	for _, phi := range s.Phis {
		if !phi.needsLoc() {
			continue
		}
		moves = append(moves, pmove{dst: phi.loc, val: phi.Args[j], t: phi.Type})
	}
	e.parallelMove(moves)
}

// ---- parallel moves ----

// pmove copies a value (val) or a fixed location (src) to dst
type pmove struct {
	dst irLoc
	src irLoc
	val *irValue
	t   irType
}

func (m *pmove) source() irLoc {
	if m.val != nil && !m.val.Op.remat() {
		return m.val.loc
	}
	if m.val != nil {
		return irLoc{}
	}
	return m.src
}

func (e *irEmitter) sameLoc(x, y irLoc) bool {
	if x.kind != y.kind || x.kind == locNone {
		return false
	}
	if x.kind == locReg {
		return x.reg == y.reg
	}
	return e.locMem(x).Disp == e.locMem(y).Disp
}

func (e *irEmitter) parallelMove(moves []pmove) {
	var pending []pmove
	for _, m := range moves {
		if m.dst.kind == locNone || e.sameLoc(m.dst, m.source()) {
			continue
		}
		pending = append(pending, m)
	}
	for len(pending) > 0 {
		progress := false
		for i := 0; i < len(pending); i++ {
			m := pending[i]
			blocked := false
			for j := range pending {
				if j != i && e.sameLoc(pending[j].source(), m.dst) {
					blocked = true
					break
				}
			}
			if blocked {
				continue
			}
			e.move(m)
			pending = append(pending[:i], pending[i+1:]...)
			i--
			progress = true
		}
		if progress {
			continue
		}
		// a cycle: park one source in a scratch register
		m := pending[0]
		src := m.source()
		scratch := irLoc{kind: locReg, reg: R10}
		if m.t.isFloat() {
			scratch.reg = XMM15
		}
		e.move(pmove{dst: scratch, src: src, t: m.t})
		for k := range pending {
			if e.sameLoc(pending[k].source(), src) {
				pending[k].val = nil
				pending[k].src = scratch
			}
		}
	}
}

func (e *irEmitter) move(m pmove) {
	a := e.a
	src := m.source()
	if m.t.inXMM() {
		t := m.t
		switch {
		case m.dst.kind == locReg && m.val != nil && src.kind == locNone:
			e.fmaterialize(m.val, m.dst.reg)
		case m.dst.kind == locReg && src.kind == locReg:
			e.xcopy(m.dst.reg, src.reg, t)
		case m.dst.kind == locReg:
			e.xload(m.dst.reg, t, e.locMem(src))
		case src.kind == locReg:
			e.xstore(e.locMem(m.dst), src.reg, t)
		case src.kind == locStack:
			e.xload(XMM14, t, e.locMem(src))
			e.xstore(e.locMem(m.dst), XMM14, t)
		default:
			e.fmaterialize(m.val, XMM14)
			e.xstore(e.locMem(m.dst), XMM14, t)
		}
		return
	}
	switch {
	case m.dst.kind == locReg && m.val != nil && src.kind == locNone:
		e.materialize(m.val, m.dst.reg)
	case m.dst.kind == locReg && src.kind == locReg:
		a.Mov(8, Q(m.dst.reg), Q(src.reg))
	case m.dst.kind == locReg:
		if m.t == iI32 {
			a.Mov(4, D(m.dst.reg), e.locMem(src))
		} else {
			a.Mov(8, Q(m.dst.reg), e.locMem(src))
		}
	case src.kind == locReg:
		a.Mov(8, e.locMem(m.dst), Q(src.reg))
	case src.kind == locStack:
		a.Mov(8, Q(R11), e.locMem(src))
		a.Mov(8, e.locMem(m.dst), Q(R11))
	default:
		if k, ok := constOf(m.val); ok && fitsI32(k) {
			a.Mov(8, e.locMem(m.dst), Imm(k))
			return
		}
		e.materialize(m.val, R11)
		a.Mov(8, e.locMem(m.dst), Q(R11))
	}
}

// loopHead tells whether the block at index i of the emitted blocks is
// the target of a backward jump
func (e *irEmitter) loopHead(b *irBlock, i int, pos map[*irBlock]int) bool {
	var seen map[*irBlock]bool
	var from func(p *irBlock) bool
	from = func(p *irBlock) bool {
		if j, ok := pos[p]; ok {
			return j >= i
		}
		// a block that only passes control on: look through it
		if seen == nil {
			seen = map[*irBlock]bool{}
		}
		if seen[p] {
			return false
		}
		seen[p] = true
		for _, q := range p.Preds {
			if from(q) {
				return true
			}
		}
		return false
	}
	for _, p := range b.Preds {
		if from(p) {
			return true
		}
	}
	return false
}
