package x64

import (
	"math"

	"github.com/ipoluianov/altc/cc"
)

// ---- casts ----

type typeID int

const (
	tI8 typeID = iota
	tI16
	tI32
	tI64
	tU8
	tU16
	tU32
	tU64
	tF32
	tF64
	tF80
)

func getTypeID(t *cc.Type) typeID {
	switch t.Kind {
	case cc.TyFloat:
		return tF32
	case cc.TyDouble:
		return tF64
	case cc.TyLongDouble:
		return tF80
	case cc.TyBool:
		return tU8
	case cc.TyPtr, cc.TyArray, cc.TyVLA, cc.TyFunc, cc.TyNullptr:
		return tU64
	}
	uns := t.IsUnsigned()
	switch t.Size() {
	case 1:
		if uns {
			return tU8
		}
		return tI8
	case 2:
		if uns {
			return tU16
		}
		return tI16
	case 4:
		if uns {
			return tU32
		}
		return tI32
	}
	if uns {
		return tU64
	}
	return tI64
}

func (g *gen) cast(from, to *cc.Type) {
	if to.Kind == cc.TyVoid {
		if isLD(from) {
			g.a.FstpSt(0)
		}
		return
	}
	if to.IsRecord() || from.IsRecord() {
		return
	}
	if g.cast128(from, to) {
		return
	}
	if to.Kind == cc.TyBool {
		g.cmpZero(from)
		g.a.Setcc(CondNE, RAX)
		if from.IsFlonum() {
			g.a.Setcc(CondP, RDX)
			g.a.Alu(OR, 1, B(RAX), B(RDX))
		}
		g.a.Movzx(4, RAX, 1, B(RAX))
		return
	}
	f, t := getTypeID(from), getTypeID(to)
	if f == t {
		return
	}
	isInt := func(x typeID) bool { return x <= tU64 }
	switch {
	case isInt(f) && isInt(t):
		g.intToInt(t)
	case isInt(f) && (t == tF32 || t == tF64):
		dbl := t == tF64
		if f == tU64 {
			neg := g.a.NewLabel()
			end := g.a.NewLabel()
			g.a.Test(8, Q(RAX), Q(RAX))
			g.a.Jcc(CondS, neg)
			g.a.CvtIntToF(dbl, XMM0, 8, Q(RAX))
			g.a.Jmp(end)
			g.a.Bind(neg)
			g.a.Mov(8, Q(RDI), Q(RAX))
			g.a.Alu(AND, 4, D(RDI), Imm(1))
			g.a.ShiftImm(SHR, 8, Q(RAX), 1)
			g.a.Alu(OR, 8, Q(RAX), Q(RDI))
			g.a.CvtIntToF(dbl, XMM0, 8, Q(RAX))
			g.a.FArith(FAdd, dbl, XMM0, X(XMM0))
			g.a.Bind(end)
			return
		}
		g.a.CvtIntToF(dbl, XMM0, 8, Q(RAX))
	case (f == tF32 || f == tF64) && isInt(t):
		dbl := f == tF64
		if t == tU64 {
			// values >= 2^63 need a subtraction first
			big := g.a.NewLabel()
			end := g.a.NewLabel()
			if dbl {
				g.a.MovImm(RAX, int64(math.Float64bits(9223372036854775808.0)))
				g.a.MovqToX(8, XMM1, RAX)
			} else {
				g.a.Mov(4, D(RAX), Imm(int64(math.Float32bits(9223372036854775808.0))))
				g.a.MovqToX(4, XMM1, RAX)
			}
			g.a.Ucomi(dbl, XMM0, X(XMM1))
			g.a.Jcc(CondAE, big)
			g.a.CvtFToInt(dbl, 8, RAX, X(XMM0))
			g.a.Jmp(end)
			g.a.Bind(big)
			g.a.FArith(FSub, dbl, XMM0, X(XMM1))
			g.a.CvtFToInt(dbl, 8, RAX, X(XMM0))
			g.a.MovImm(RDI, math.MinInt64)
			g.a.Alu(XOR, 8, Q(RAX), Q(RDI))
			g.a.Bind(end)
			return
		}
		g.a.CvtFToInt(dbl, 8, RAX, X(XMM0))
		g.intToInt(t)
	case f == tF32 && t == tF64:
		g.a.CvtFF(true, XMM0, X(XMM0))
	case f == tF64 && t == tF32:
		g.a.CvtFF(false, XMM0, X(XMM0))
	case t == tF80:
		switch {
		case f == tF32 || f == tF64:
			g.a.Alu(SUB, 8, Q(RSP), Imm(8))
			g.a.MovF(f == tF64, M(RSP, 0), X(XMM0))
			if f == tF64 {
				g.a.Fld(8, M(RSP, 0))
			} else {
				g.a.Fld(4, M(RSP, 0))
			}
			g.a.Alu(ADD, 8, Q(RSP), Imm(8))
		default:
			g.a.Push(RAX)
			g.a.Fild(8, M(RSP, 0))
			g.a.Pop(RAX)
			if f == tU64 {
				skip := g.a.NewLabel()
				g.a.Test(8, Q(RAX), Q(RAX))
				g.a.Jcc(CondNS, skip)
				// add 2^64
				two64 := make([]byte, 4)
				bits := math.Float32bits(18446744073709551616.0)
				two64[0], two64[1], two64[2], two64[3] = byte(bits), byte(bits>>8), byte(bits>>16), byte(bits>>24)
				g.a.Fld(4, MSym(g.constLabel(two64, 4), 0))
				g.a.Faddp()
				g.a.Bind(skip)
			}
		}
	case f == tF80:
		switch t {
		case tF32, tF64:
			g.a.Alu(SUB, 8, Q(RSP), Imm(8))
			if t == tF64 {
				g.a.Fstp(8, M(RSP, 0))
			} else {
				g.a.Fstp(4, M(RSP, 0))
			}
			g.a.MovF(t == tF64, X(XMM0), M(RSP, 0))
			g.a.Alu(ADD, 8, Q(RSP), Imm(8))
		default:
			g.a.Alu(SUB, 8, Q(RSP), Imm(16))
			if t == tU64 {
				// subtract 2^63 when the value is that large
				big := g.a.NewLabel()
				end := g.a.NewLabel()
				c := make([]byte, 4)
				bits := math.Float32bits(9223372036854775808.0)
				c[0], c[1], c[2], c[3] = byte(bits), byte(bits>>8), byte(bits>>16), byte(bits>>24)
				lbl := g.constLabel(c, 4)
				g.a.Fld(4, MSym(lbl, 0))
				g.a.Fucomip(1) // flags: 2^63 vs value
				g.a.Jcc(CondBE, big)
				g.ldToInt64()
				g.a.Jmp(end)
				g.a.Bind(big)
				g.a.Fld(4, MSym(lbl, 0))
				g.a.Fsubp()
				g.ldToInt64()
				g.a.MovImm(RDI, math.MinInt64)
				g.a.Alu(XOR, 8, Q(RAX), Q(RDI))
				g.a.Bind(end)
			} else {
				g.ldToInt64()
				g.intToInt(t)
			}
			g.a.Alu(ADD, 8, Q(RSP), Imm(16))
		}
	}
}

// ldToInt64 converts st(0) to a 64-bit integer in rax with truncation
// (needs 16 bytes of scratch at (rsp))
func (g *gen) ldToInt64() {
	g.a.Fnstcw(M(RSP, 0))
	g.a.Movzx(4, RAX, 2, M(RSP, 0))
	g.a.Alu(OR, 4, D(RAX), Imm(0xC00))
	g.a.Mov(2, M(RSP, 2), W(RAX))
	g.a.Fldcw(M(RSP, 2))
	g.a.Fistp(8, M(RSP, 8))
	g.a.Fldcw(M(RSP, 0))
	g.a.Mov(8, Q(RAX), M(RSP, 8))
}

func (g *gen) intToInt(t typeID) {
	switch t {
	case tI8:
		g.a.Movsx(8, RAX, 1, B(RAX))
	case tU8:
		g.a.Movzx(4, RAX, 1, B(RAX))
	case tI16:
		g.a.Movsx(8, RAX, 2, W(RAX))
	case tU16:
		g.a.Movzx(4, RAX, 2, W(RAX))
	case tI32:
		g.a.Movsx(8, RAX, 4, D(RAX))
	case tU32:
		g.a.Mov(4, D(RAX), D(RAX))
	}
}

// ---- calls ----

type argSlot struct {
	stack bool
	off   int64
}

func (g *gen) genCall(n *cc.Node) {
	ft := n.FuncTy
	ret := n.Ty
	gp, fp := 0, 0
	retInMem := false
	if ret.IsRecord() && !retInX87(ret) && !retInX87Pair(ret) {
		if _, _, ok := structRegs(ret); !ok {
			retInMem = true
			gp++
		}
	}
	slots := make([]argSlot, len(n.Args))
	stackOff := int64(0)
	for i, a := range n.Args {
		t := a.Ty
		switch {
		case t.IsRecord():
			if t.Size() == 0 {
				slots[i].stack = false
				continue
			}
			ng, nf, ok := structRegs(t)
			if ok && gp+ng <= 6 && fp+nf <= 8 {
				gp += ng
				fp += nf
				continue
			}
			al := int64(8)
			if t.Align() >= 16 {
				al = 16
			}
			stackOff = alignTo(stackOff, al)
			slots[i] = argSlot{true, stackOff}
			stackOff += alignTo(t.Size(), 8)
		case isSSE(t):
			if fp < 8 {
				fp++
				continue
			}
			slots[i] = argSlot{true, stackOff}
			stackOff += 8
		case isLD(t):
			stackOff = alignTo(stackOff, 16)
			slots[i] = argSlot{true, stackOff}
			stackOff += 16
		case is128(t):
			if gp+2 <= 6 {
				gp += 2
				continue
			}
			stackOff = alignTo(stackOff, 16)
			slots[i] = argSlot{true, stackOff}
			stackOff += 16
		default:
			if gp < 6 {
				gp++
				continue
			}
			slots[i] = argSlot{true, stackOff}
			stackOff += 8
		}
	}
	area := alignTo(stackOff, 8)
	if (int64(g.depth)*8+area)%16 != 0 {
		area += 8
	}
	if area > 0 {
		g.a.Alu(SUB, 8, Q(RSP), Imm(area))
		g.depth += int(area / 8)
	}
	// stack arguments go straight into the outgoing area
	for i, a := range n.Args {
		if !slots[i].stack {
			continue
		}
		g.genExpr(a)
		off := int32(slots[i].off)
		t := a.Ty
		switch {
		case t.IsRecord():
			g.a.Lea(R10, M(RSP, off))
			g.copyMem(R10, RAX, t.Size())
		case isFloat(t):
			g.a.MovF(false, M(RSP, off), X(XMM0))
		case isDouble(t):
			g.a.MovF(true, M(RSP, off), X(XMM0))
		case isLD(t):
			g.a.Fstp(10, M(RSP, off))
		case is128(t):
			g.a.Mov(8, M(RSP, off), Q(RAX))
			g.a.Mov(8, M(RSP, off+8), Q(RDX))
		default:
			g.a.Mov(8, M(RSP, off), Q(RAX))
		}
	}
	// the callee for indirect calls
	direct := ""
	fn := n.LHS
	if fn.Kind == cc.NVar && fn.Var.IsFunction && !fn.Var.IsLocal {
		direct = fn.Var.AsmName
	} else {
		g.genExpr(fn)
		g.push()
	}
	// register arguments: push in order, then pop into registers
	type dest struct {
		fp    bool
		reg   Reg
		inTmp bool // the value waits in a temporary register
		tmp   Reg
	}
	var dests []dest
	type deferred struct {
		reg  Reg
		load func(Reg)
	}
	var late []deferred
	gp, fp = 0, 0
	if retInMem {
		gp = 1
	}
	// the last evaluated argument goes to its register without the stack
	lastEval := -1
	for i, a := range n.Args {
		if !slots[i].stack && g.argLoader(a) == nil {
			lastEval = i
		}
	}
	if lastEval >= 0 {
		t := n.Args[lastEval].Ty
		if t.IsRecord() || is128(t) {
			lastEval = -1
		}
	}
	var lastMove func()
	for i, a := range n.Args {
		if slots[i].stack {
			continue
		}
		t := a.Ty
		if ld := g.argLoader(a); ld != nil {
			if isSSE(t) {
				late = append(late, deferred{XMM0 + Reg(fp), ld})
				fp++
			} else {
				late = append(late, deferred{argGP[gp], ld})
				gp++
			}
			continue
		}
		g.genExpr(a)
		if i == lastEval {
			if isSSE(t) {
				r := XMM0 + Reg(fp)
				lastMove = func() {
					if r != XMM0 {
						g.a.MovF(true, X(r), X(XMM0))
					}
				}
				fp++
			} else {
				r := argGP[gp]
				lastMove = func() { g.a.Mov(8, Q(r), Q(RAX)) }
				gp++
			}
			continue
		}
		switch {
		case t.IsRecord():
			if t.Size() == 0 {
				continue
			}
			g.a.Mov(8, Q(R10), Q(RAX))
			sz := t.Size()
			for k := int64(0); k*8 < sz; k++ {
				chunk := min(8, sz-k*8)
				if hasFlonum(t, k*8, k*8+8, 0) {
					g.loadSized(R11, M(R10, int32(k*8)), chunk)
					g.a.Push(R11)
					g.depth++
					dests = append(dests, dest{fp: true, reg: XMM0 + Reg(fp)})
					fp++
				} else {
					g.loadSized(R11, M(R10, int32(k*8)), chunk)
					g.a.Push(R11)
					g.depth++
					dests = append(dests, dest{reg: argGP[gp]})
					gp++
				}
			}
		case isSSE(t):
			if tr, ok := g.saveTempF(); ok {
				dests = append(dests, dest{true, XMM0 + Reg(fp), true, tr})
			} else {
				g.pushf()
				dests = append(dests, dest{fp: true, reg: XMM0 + Reg(fp)})
			}
			fp++
		case is128(t):
			g.push()
			g.a.Push(RDX)
			g.depth++
			dests = append(dests, dest{reg: argGP[gp]}, dest{reg: argGP[gp+1]})
			gp += 2
		default:
			if tr, ok := g.saveTemp(); ok {
				dests = append(dests, dest{false, argGP[gp], true, tr})
			} else {
				g.push()
				dests = append(dests, dest{reg: argGP[gp]})
			}
			gp++
		}
	}
	if lastMove != nil {
		lastMove()
	}
	for i := len(dests) - 1; i >= 0; i-- {
		d := dests[i]
		switch {
		case d.inTmp && d.fp:
			g.a.MovqToX(8, d.reg, d.tmp)
			g.freeTemp()
		case d.inTmp:
			g.a.Mov(8, Q(d.reg), Q(d.tmp))
			g.freeTemp()
		case d.fp:
			g.popf(d.reg)
		default:
			g.pop(d.reg)
		}
	}
	for _, d := range late {
		d.load(d.reg)
	}
	if direct == "" {
		g.pop(R10)
	}
	if retInMem {
		g.a.Lea(RDI, M(RBP, int32(n.RetBuffer.Offset)))
	}
	if ft.Variadic || ft.OldStyle {
		g.a.Mov(4, D(RAX), Imm(int64(fp)))
	}
	g.spillXMM()
	if direct != "" {
		g.a.Call(direct)
	} else {
		g.a.CallInd(Q(R10))
	}
	g.reloadXMM()
	if area > 0 {
		g.a.Alu(ADD, 8, Q(RSP), Imm(area))
		g.depth -= int(area / 8)
	}
	// the result
	switch {
	case ret.IsRecord():
		if n.RetBuffer == nil {
			return
		}
		if retInX87(ret) {
			g.a.Fstp(10, M(RBP, int32(n.RetBuffer.Offset)))
		} else if retInX87Pair(ret) {
			g.a.Fstp(10, M(RBP, int32(n.RetBuffer.Offset)))
			g.a.Fstp(10, M(RBP, int32(n.RetBuffer.Offset+16)))
		} else if !retInMem {
			g.copyRetToBuffer(ret, n.RetBuffer.Offset)
		}
		g.a.Lea(RAX, M(RBP, int32(n.RetBuffer.Offset)))
	case ret.Kind == cc.TyBool:
		g.a.Movzx(4, RAX, 1, B(RAX))
	case is128(ret):
	case ret.IsInteger():
		g.extendTo64(ret)
	}
}

// copyRetToBuffer stores a struct returned in registers into a local
func (g *gen) copyRetToBuffer(t *cc.Type, off int64) {
	gpR := []Reg{RAX, RDX}
	gi, fi := 0, 0
	sz := t.Size()
	for k := int64(0); k*8 < sz; k++ {
		chunk := min(8, sz-k*8)
		m := M(RBP, int32(off+k*8))
		if hasFlonum(t, k*8, k*8+8, 0) {
			if chunk == 4 {
				g.a.MovF(false, m, X(XMM0+Reg(fi)))
			} else {
				g.a.MovF(true, m, X(XMM0+Reg(fi)))
			}
			fi++
		} else {
			g.storeSized(m, gpR[gi], chunk)
			gi++
		}
	}
}

// copyStructToRet loads a struct (address in rax) into return registers
func (g *gen) copyStructToRet(t *cc.Type) {
	g.a.Mov(8, Q(RDI), Q(RAX))
	sz := t.Size()
	gpR := []Reg{RAX, RDX}
	gi, fi := 0, 0
	for k := int64(0); k*8 < sz; k++ {
		chunk := min(8, sz-k*8)
		m := M(RDI, int32(k*8))
		if hasFlonum(t, k*8, k*8+8, 0) {
			if chunk == 4 {
				g.a.MovF(false, X(XMM0+Reg(fi)), m)
			} else {
				g.a.MovF(true, X(XMM0+Reg(fi)), m)
			}
			fi++
		} else {
			g.loadSized(gpR[gi], m, chunk)
			gi++
		}
	}
}

// ---- functions ----

// function generates a function. When registers are left over by the
// variables, a trial generation counts how many of them the temporaries
// need, so that the prologue saves only those.
func (g *gen) function(fn *cc.Obj) {
	g.tempLimit = -1
	if !g.noRegAlloc && fn.Body != nil && !hasAsm(fn.Body) {
		mk := g.a.mark()
		listing := g.a.Listing
		g.a.Listing = nil
		g.dryLabels = []string{}
		g.functionBody(fn)
		for _, name := range g.dryLabels {
			delete(g.labels, name)
		}
		g.dryLabels = nil
		g.a.reset(mk)
		g.a.Listing = listing
		g.tempLimit = g.tempUsed
	}
	g.functionBody(fn)
}

func (g *gen) functionBody(fn *cc.Obj) {
	g.fn = fn
	if usesVectors(fn) {
		reason := g.irReason
		if reason == "" {
			reason = "-O0"
		}
		g.fail(fn.Body, "vector types in '%s' need the optimizing backend, which cannot compile it: %s", fn.Name, reason)
	}
	g.depth = 0
	g.retLabel = g.a.NewLabel()
	ft := fn.Ty
	// parameters passed in memory live in the caller's frame
	gp, fp := 0, 0
	retInMem := false
	if ft.Ret.IsRecord() && !retInX87(ft.Ret) && !retInX87Pair(ft.Ret) {
		if _, _, ok := structRegs(ft.Ret); !ok {
			retInMem = true
			gp = 1
		}
	}
	onStack := map[*cc.Obj]bool{}
	stackOff := int64(16)
	type regParam struct {
		v      *cc.Obj
		gp, fp int
	}
	var regParams []regParam
	for _, p := range fn.Params {
		t := p.Ty
		switch {
		case t.IsRecord():
			if t.Size() == 0 {
				continue
			}
			ng, nf, ok := structRegs(t)
			if ok && gp+ng <= 6 && fp+nf <= 8 {
				regParams = append(regParams, regParam{p, gp, fp})
				gp += ng
				fp += nf
				continue
			}
			if t.Align() >= 16 {
				stackOff = alignTo(stackOff, 16)
			}
			p.Offset = stackOff
			stackOff += alignTo(t.Size(), 8)
			onStack[p] = true
		case isSSE(t):
			if fp < 8 {
				regParams = append(regParams, regParam{p, gp, fp})
				fp++
				continue
			}
			p.Offset = stackOff
			stackOff += 8
			onStack[p] = true
		case isLD(t):
			stackOff = alignTo(stackOff, 16)
			p.Offset = stackOff
			stackOff += 16
			onStack[p] = true
		case is128(t):
			if gp+2 <= 6 {
				regParams = append(regParams, regParam{p, gp, fp})
				gp += 2
				continue
			}
			stackOff = alignTo(stackOff, 16)
			p.Offset = stackOff
			stackOff += 16
			onStack[p] = true
		default:
			if gp < 6 {
				regParams = append(regParams, regParam{p, gp, fp})
				gp++
				continue
			}
			p.Offset = stackOff
			stackOff += 8
			onStack[p] = true
		}
	}
	g.gpUsed, g.fpUsed = gp, fp
	g.stackArgSz = stackOff - 16
	g.allocRegVars(fn)
	// local variables
	offset := int64(0)
	g.overAligned = map[*cc.Obj]int64{}
	isParam := map[*cc.Obj]bool{}
	for _, p := range fn.Params {
		isParam[p] = true
	}
	for _, v := range fn.Locals {
		if onStack[v] {
			continue
		}
		if r, ok := g.regVars[v]; ok && !r.IsXMM() {
			continue
		}
		sz := v.Ty.Size()
		if v.Ty.Kind == cc.TyVLA {
			sz = 8
		}
		if sz < 0 {
			sz = 0
		}
		al := v.Align
		if al < v.Ty.Align() {
			al = v.Ty.Align()
		}
		if v.Ty.Kind == cc.TyVLA {
			al = 8
		}
		if sz >= 16 && al < 16 && v.Ty.Kind == cc.TyArray {
			al = 16
		}
		if al > 16 && !isParam[v] {
			// over-aligned: reserve room to align at run time and keep a pointer
			offset += sz + al - 1
			offset = alignTo(offset, 16)
			area := -offset
			offset = alignTo(offset+8, 8)
			v.Offset = -offset
			g.overAligned[v] = area
			continue
		}
		offset += sz
		offset = alignTo(offset, max(al, 1))
		v.Offset = -offset
	}
	if retInMem {
		offset = alignTo(offset+8, 8)
		g.retBufOff = -offset
	}
	// temporaries for va_arg of small structs
	g.vaTemps = map[*cc.Node]int64{}
	cc.Walk(fn.Body, func(n *cc.Node) {
		if n.Kind == cc.NVaArg && n.Ty.Base.IsRecord() {
			offset = alignTo(offset+16, 16)
			g.vaTemps[n] = -offset
		}
		// temporaries for complex values built from parts or converted
		if n.Kind == cc.NMakeComplex || n.Kind == cc.NCast && n.Ty.IsComplex() {
			offset = alignTo(offset+n.Ty.Size(), 16)
			g.vaTemps[n] = -offset
		}
	})
	offset = g.asmFrame(fn, offset)
	offset = g.regFrame(offset)
	stackSize := alignTo(offset, 16)

	g.a.Push(RBP)
	g.a.Mov(8, Q(RBP), Q(RSP))
	if stackSize > 0 {
		g.a.Alu(SUB, 8, Q(RSP), Imm(stackSize))
	}
	g.saveCalleeRegs()
	g.saveVarRegs()
	g.a.Mov(8, M(RBP, int32(fn.AllocaBottom.Offset)), Q(RSP))
	for v, area := range g.overAligned {
		al := max(v.Align, v.Ty.Align())
		g.a.Lea(RAX, M(RBP, int32(area+al-1)))
		g.a.Alu(AND, 8, Q(RAX), Imm(-al))
		g.a.Mov(8, M(RBP, int32(v.Offset)), Q(RAX))
	}
	if retInMem {
		g.a.Mov(8, M(RBP, int32(g.retBufOff)), Q(RDI))
	}
	if fn.VaArea != nil {
		off := fn.VaArea.Offset
		for i, r := range argGP {
			g.a.Mov(8, M(RBP, int32(off+int64(i*8))), Q(r))
		}
		skip := g.a.NewLabel()
		g.a.Test(1, B(RAX), B(RAX))
		g.a.Jcc(CondE, skip)
		for i := 0; i < 8; i++ {
			g.a.MovF(true, M(RBP, int32(off+48+int64(i*16))), X(XMM0+Reg(i)))
		}
		g.a.Bind(skip)
	}
	for _, rp := range regParams {
		v, t := rp.v, rp.v.Ty
		switch {
		case t.IsRecord():
			sz := t.Size()
			gi, fi := rp.gp, rp.fp
			for k := int64(0); k*8 < sz; k++ {
				chunk := min(8, sz-k*8)
				m := M(RBP, int32(v.Offset+k*8))
				if hasFlonum(t, k*8, k*8+8, 0) {
					if chunk == 4 {
						g.a.MovF(false, m, X(XMM0+Reg(fi)))
					} else {
						g.a.MovF(true, m, X(XMM0+Reg(fi)))
					}
					fi++
				} else {
					g.storeSized(m, argGP[gi], chunk)
					gi++
				}
			}
		case isSSE(t) && g.regVars[v] != 0:
			g.a.MovF(true, X(g.regVars[v]), X(XMM0+Reg(rp.fp)))
		case isFloat(t):
			g.a.MovF(false, M(RBP, int32(v.Offset)), X(XMM0+Reg(rp.fp)))
		case isDouble(t):
			g.a.MovF(true, M(RBP, int32(v.Offset)), X(XMM0+Reg(rp.fp)))
		case is128(t):
			g.a.Mov(8, M(RBP, int32(v.Offset)), Q(argGP[rp.gp]))
			g.a.Mov(8, M(RBP, int32(v.Offset+8)), Q(argGP[rp.gp+1]))
		default:
			if r, ok := g.varReg(v); ok {
				g.extendInto(r, t, argGP[rp.gp])
				continue
			}
			g.storeSized(M(RBP, int32(v.Offset)), argGP[rp.gp], t.Size())
		}
	}
	for _, v := range fn.Params {
		if r, ok := g.varReg(v); ok && onStack[v] {
			if r.IsXMM() {
				g.a.MovF(isDouble(v.Ty), X(r), M(RBP, int32(v.Offset)))
				continue
			}
			g.loadInto(r, v.Ty, M(RBP, int32(v.Offset)))
		}
	}
	g.genStmt(fn.Body)
	if fn.Name == "main" {
		g.a.Alu(XOR, 4, D(RAX), D(RAX))
	}
	g.a.Bind(g.retLabel)
	g.restoreVarRegs()
	g.restoreCalleeRegs()
	g.a.Leave()
	g.a.Ret()
	if g.depth != 0 {
		g.fail(fn.Body, "internal error: unbalanced stack in %s (%d)", fn.Name, g.depth)
	}
}

// ---- statements ----

func (g *gen) genStmt(n *cc.Node) {
	switch n.Kind {
	case cc.NIf:
		els := g.a.NewLabel()
		end := g.a.NewLabel()
		g.genBranchFalse(n.Cond, els)
		g.genStmt(n.Then)
		if n.Els != nil {
			g.a.Jmp(end)
		}
		g.a.Bind(els)
		if n.Els != nil {
			g.genStmt(n.Els)
		}
		g.a.Bind(end)
	case cc.NFor:
		if n.Init != nil {
			g.genStmt(n.Init)
		}
		begin := g.a.NewLabel()
		brk := g.label(n.BrkLabel)
		cont := g.label(n.ContLabel)
		g.stackSave(n.StackSave)
		g.a.Bind(begin)
		g.stackRestore(n.StackSave)
		if n.Cond != nil {
			g.genBranchFalse(n.Cond, brk)
		}
		g.genStmt(n.Then)
		g.a.Bind(cont)
		if n.Inc != nil {
			g.genDiscard(n.Inc)
		}
		g.a.Jmp(begin)
		g.a.Bind(brk)
		g.stackRestore(n.StackSave)
	case cc.NDo:
		begin := g.a.NewLabel()
		brk := g.label(n.BrkLabel)
		cont := g.label(n.ContLabel)
		g.stackSave(n.StackSave)
		g.a.Bind(begin)
		g.stackRestore(n.StackSave)
		g.genStmt(n.Then)
		g.a.Bind(cont)
		g.genBranchFalse(n.Cond, brk)
		g.a.Jmp(begin)
		g.a.Bind(brk)
		g.stackRestore(n.StackSave)
	case cc.NSwitch:
		g.genExpr(n.Cond)
		for _, c := range n.Cases {
			l := g.label(c.Label)
			if c.Begin == c.End {
				if fitsI32(c.Begin) {
					g.a.Alu(CMP, 8, Q(RAX), Imm(c.Begin))
				} else {
					g.a.MovImm(RDI, c.Begin)
					g.a.Alu(CMP, 8, Q(RAX), Q(RDI))
				}
				g.a.Jcc(CondE, l)
				continue
			}
			g.a.Mov(8, Q(RDI), Q(RAX))
			g.a.MovImm(RSI, c.Begin)
			g.a.Alu(SUB, 8, Q(RDI), Q(RSI))
			g.a.MovImm(RSI, c.End-c.Begin)
			g.a.Alu(CMP, 8, Q(RDI), Q(RSI))
			g.a.Jcc(CondBE, l)
		}
		brk := g.label(n.BrkLabel)
		if n.Default != nil {
			g.a.Jmp(g.label(n.Default.Label))
		} else {
			g.a.Jmp(brk)
		}
		g.genStmt(n.Then)
		g.a.Bind(brk)
	case cc.NCase:
		g.a.Bind(g.label(n.Label))
		g.genStmt(n.LHS)
	case cc.NBlock:
		g.stackSave(n.StackSave)
		for _, s := range n.Body {
			g.genStmt(s)
		}
		g.stackRestore(n.StackSave)
	case cc.NGoto:
		g.stackRestore(n.StackSave)
		g.a.Jmp(g.label(n.UniqueLabel))
	case cc.NGotoExpr:
		g.genExpr(n.LHS)
		g.a.JmpInd(Q(RAX))
	case cc.NLabel:
		g.a.Bind(g.label(n.UniqueLabel))
		g.genStmt(n.LHS)
	case cc.NReturn:
		if n.LHS != nil {
			g.genExpr(n.LHS)
			t := n.LHS.Ty
			ret := g.fn.Ty.Ret
			switch {
			case ret.Kind == cc.TyVoid:
				if isLD(t) {
					g.a.FstpSt(0)
				}
			case retInX87(ret):
				g.a.Fld(10, M(RAX, 0))
			case retInX87Pair(ret):
				g.a.Fld(10, M(RAX, 16))
				g.a.Fld(10, M(RAX, 0))
			case ret.IsRecord():
				if _, _, ok := structRegs(ret); ok {
					g.copyStructToRet(ret)
				} else {
					g.a.Mov(8, Q(RDI), M(RBP, int32(g.retBufOff)))
					g.copyMem(RDI, RAX, ret.Size())
					g.a.Mov(8, Q(RAX), M(RBP, int32(g.retBufOff)))
				}
			}
		}
		g.a.Jmp(g.retLabel)
	case cc.NExprStmt:
		g.genDiscard(n.LHS)
	case cc.NAsm:
		if n.Asm != nil {
			g.genAsm(n)
		}
	default:
		g.genDiscard(n)
	}
}

// stackSave remembers the stack pointer and the alloca bottom before VLAs
func (g *gen) stackSave(v *cc.Obj) {
	if v == nil {
		return
	}
	g.a.Mov(8, M(RBP, int32(v.Offset)), Q(RSP))
	g.a.Mov(8, Q(RAX), M(RBP, int32(g.fn.AllocaBottom.Offset)))
	g.a.Mov(8, M(RBP, int32(v.Offset+8)), Q(RAX))
}

// stackRestore releases the VLA space allocated since stackSave
func (g *gen) stackRestore(v *cc.Obj) {
	if v == nil {
		return
	}
	g.a.Mov(8, Q(RSP), M(RBP, int32(v.Offset)))
	g.a.Mov(8, Q(RAX), M(RBP, int32(v.Offset+8)))
	g.a.Mov(8, M(RBP, int32(g.fn.AllocaBottom.Offset)), Q(RAX))
}

// ---- varargs ----

func (g *gen) genVaStart(n *cc.Node) {
	g.genExpr(n.LHS) // address of the va_list element
	g.a.Mov(4, M(RAX, 0), Imm(int64(g.gpUsed*8)))
	g.a.Mov(4, M(RAX, 4), Imm(int64(48+g.fpUsed*16)))
	g.a.Lea(RDX, M(RBP, int32(16+g.stackArgSz)))
	g.a.Mov(8, M(RAX, 8), Q(RDX))
	g.a.Lea(RDX, M(RBP, int32(g.fn.VaArea.Offset)))
	g.a.Mov(8, M(RAX, 16), Q(RDX))
}

// genVaArg leaves the address of the next argument of type n.Ty.Base in rax
func (g *gen) genVaArg(n *cc.Node) {
	t := n.Ty.Base
	g.genExpr(n.LHS)
	g.a.Mov(8, Q(RCX), Q(RAX)) // rcx = va_list
	end := g.a.NewLabel()
	mem := g.a.NewLabel()
	switch {
	case t.IsRecord():
		ng, nf, ok := structRegs(t)
		if !ok {
			break
		}
		if ng > 0 {
			g.a.Alu(CMP, 4, M(RCX, 0), Imm(int64(48-8*ng)))
			g.a.Jcc(CondA, mem)
		}
		if nf > 0 {
			g.a.Alu(CMP, 4, M(RCX, 4), Imm(int64(176-16*nf)))
			g.a.Jcc(CondA, mem)
		}
		tmp := int32(g.vaTemps[n])
		sz := t.Size()
		for k := int64(0); k*8 < sz; k++ {
			g.a.Mov(8, Q(RDX), M(RCX, 16)) // reg_save_area
			if hasFlonum(t, k*8, k*8+8, 0) {
				g.a.Mov(4, D(RSI), M(RCX, 4))
				g.a.Alu(ADD, 8, Q(RDX), Q(RSI))
				g.a.Mov(8, Q(RDX), M(RDX, 0))
				g.a.Alu(ADD, 4, M(RCX, 4), Imm(16))
			} else {
				g.a.Mov(4, D(RSI), M(RCX, 0))
				g.a.Alu(ADD, 8, Q(RDX), Q(RSI))
				g.a.Mov(8, Q(RDX), M(RDX, 0))
				g.a.Alu(ADD, 4, M(RCX, 0), Imm(8))
			}
			g.a.Mov(8, M(RBP, tmp+int32(k*8)), Q(RDX))
		}
		g.a.Lea(RAX, M(RBP, tmp))
		g.a.Jmp(end)
	case isSSE(t):
		g.a.Alu(CMP, 4, M(RCX, 4), Imm(160))
		g.a.Jcc(CondA, mem)
		g.a.Mov(4, D(RAX), M(RCX, 4))
		g.a.Alu(ADD, 8, Q(RAX), M(RCX, 16))
		g.a.Alu(ADD, 4, M(RCX, 4), Imm(16))
		g.a.Jmp(end)
	case isLD(t):
	case is128(t):
		g.a.Alu(CMP, 4, M(RCX, 0), Imm(32))
		g.a.Jcc(CondA, mem)
		g.a.Mov(4, D(RAX), M(RCX, 0))
		g.a.Alu(ADD, 8, Q(RAX), M(RCX, 16))
		g.a.Alu(ADD, 4, M(RCX, 0), Imm(16))
		g.a.Jmp(end)
	default:
		g.a.Alu(CMP, 4, M(RCX, 0), Imm(40))
		g.a.Jcc(CondA, mem)
		g.a.Mov(4, D(RAX), M(RCX, 0))
		g.a.Alu(ADD, 8, Q(RAX), M(RCX, 16))
		g.a.Alu(ADD, 4, M(RCX, 0), Imm(8))
		g.a.Jmp(end)
	}
	// from the overflow area
	g.a.Bind(mem)
	g.a.Mov(8, Q(RAX), M(RCX, 8))
	if t.Align() >= 16 {
		g.a.Alu(ADD, 8, Q(RAX), Imm(15))
		g.a.Alu(AND, 8, Q(RAX), Imm(-16))
	}
	sz := alignTo(max(t.Size(), 8), 8)
	g.a.Lea(RDX, M(RAX, int32(sz)))
	g.a.Mov(8, M(RCX, 8), Q(RDX))
	g.a.Bind(end)
}

// ---- builtins ----

func (g *gen) genBuiltin(n *cc.Node) {
	switch n.Builtin {
	case "trap":
		g.a.Ud2()
		return
	case "frame_address":
		g.a.Mov(8, Q(RAX), Q(RBP))
		return
	case "return_address":
		g.a.Mov(8, Q(RAX), M(RBP, 8))
		return
	case "add_overflow", "sub_overflow", "mul_overflow":
		g.genOverflow(n)
		return
	case "isunordered":
		x, y := n.Args[0], n.Args[1]
		g.genExpr(&cc.Node{Kind: cc.NNe, LHS: x, RHS: x, Ty: n.Ty, Tok: n.Tok})
		g.push()
		g.genExpr(&cc.Node{Kind: cc.NNe, LHS: y, RHS: y, Ty: n.Ty, Tok: n.Tok})
		g.pop(RDI)
		g.a.Alu(OR, 4, D(RAX), D(RDI))
		return
	case "fpclassify":
		g.genFpclassify(n)
		return
	}
	arg := n.Args[0]
	g.genExpr(arg)
	at := arg.Ty
	sz := int(at.Size())
	if sz < 4 {
		sz = 4
	}
	if g.unaryBuiltin(n.Builtin, sz) {
		return
	}
	switch n.Builtin {
	case "isnan", "isinf", "isfinite", "isnormal", "signbit", "signbitf", "signbitl", "isinf_sign":
		g.genFloatClass(n.Builtin, at)
	default:
		g.fail(n, "builtin %s is not supported", n.Builtin)
	}
}

// unaryIntBuiltins work on rax and use only rax, rcx and rdx
var unaryIntBuiltins = map[string]bool{
	"bswap16": true, "bswap32": true, "bswap64": true, "clz": true, "ctz": true,
	"ffs": true, "popcount": true, "parity": true, "clrsb": true,
}

// unaryBuiltin computes an integer builtin of the value in rax
func (g *gen) unaryBuiltin(name string, sz int) bool {
	switch name {
	case "bswap16":
		g.a.ShiftImm(ROL, 2, W(RAX), 8)
		g.a.Movzx(4, RAX, 2, W(RAX))
	case "bswap32":
		g.a.Bswap(4, RAX)
	case "bswap64":
		g.a.Bswap(8, RAX)
	case "clz":
		if g.isa.Has(ISALZCNT) {
			g.a.Lzcnt(sz, RAX, Sized(RAX, sz))
			break
		}
		g.a.Bsr(sz, RAX, Sized(RAX, sz))
		g.a.Alu(XOR, 4, D(RAX), Imm(int64(sz*8-1)))
	case "ctz":
		if g.isa.Has(ISABMI) {
			g.a.Tzcnt(sz, RAX, Sized(RAX, sz))
			break
		}
		g.a.Bsf(sz, RAX, Sized(RAX, sz))
	case "ffs":
		g.a.Bsf(sz, RAX, Sized(RAX, sz))
		g.a.Mov(4, D(RDX), Imm(-1))
		g.a.Cmov(CondE, 4, RAX, D(RDX))
		g.a.Alu(ADD, 4, D(RAX), Imm(1))
	case "popcount", "parity":
		if g.isa.Has(ISAPOPCNT) {
			g.a.Popcnt(sz, RAX, Sized(RAX, sz))
		} else {
			g.popcount(sz)
		}
		if name == "parity" {
			g.a.Alu(AND, 4, D(RAX), Imm(1))
		}
	case "clrsb":
		// count leading redundant sign bits: clz(x ^ (x >> (bits-1))) - 1
		g.a.Mov(8, Q(RDX), Q(RAX))
		g.a.ShiftImm(SAR, sz, Sized(RDX, sz), sz*8-1)
		g.a.Alu(XOR, sz, Sized(RAX, sz), Sized(RDX, sz))
		g.a.ShiftImm(SHL, sz, Sized(RAX, sz), 1)
		g.a.Alu(OR, sz, Sized(RAX, sz), Imm(1))
		g.a.Bsr(sz, RAX, Sized(RAX, sz))
		g.a.Alu(XOR, 4, D(RAX), Imm(int64(sz*8-1)))
	default:
		return false
	}
	return true
}

// popcount counts the set bits of rax (size 4 or 8) with SWAR arithmetic
func (g *gen) popcount(sz int) {
	if sz == 4 {
		g.a.Mov(4, D(RAX), D(RAX))
	}
	g.a.Mov(8, Q(RDX), Q(RAX))
	g.a.ShiftImm(SHR, 8, Q(RDX), 1)
	g.a.MovImm(RCX, 0x5555555555555555)
	g.a.Alu(AND, 8, Q(RDX), Q(RCX))
	g.a.Alu(SUB, 8, Q(RAX), Q(RDX))
	g.a.MovImm(RCX, 0x3333333333333333)
	g.a.Mov(8, Q(RDX), Q(RAX))
	g.a.Alu(AND, 8, Q(RAX), Q(RCX))
	g.a.ShiftImm(SHR, 8, Q(RDX), 2)
	g.a.Alu(AND, 8, Q(RDX), Q(RCX))
	g.a.Alu(ADD, 8, Q(RAX), Q(RDX))
	g.a.Mov(8, Q(RDX), Q(RAX))
	g.a.ShiftImm(SHR, 8, Q(RDX), 4)
	g.a.Alu(ADD, 8, Q(RAX), Q(RDX))
	g.a.MovImm(RCX, 0x0f0f0f0f0f0f0f0f)
	g.a.Alu(AND, 8, Q(RAX), Q(RCX))
	g.a.MovImm(RCX, 0x0101010101010101)
	g.a.Imul2(8, RAX, Q(RCX))
	g.a.ShiftImm(SHR, 8, Q(RAX), 56)
}

// toDoubleXMM0 converts the value just computed (of type t) to a double in xmm0
func (g *gen) toDoubleXMM0(t *cc.Type) {
	switch t.Kind {
	case cc.TyFloat:
		g.a.CvtFF(true, XMM0, X(XMM0))
	case cc.TyLongDouble:
		g.cast(t, cc.TypeDouble)
	}
}

func (g *gen) genFloatClass(op string, t *cc.Type) {
	if op == "signbit" || op == "signbitf" || op == "signbitl" {
		switch t.Kind {
		case cc.TyFloat:
			g.a.MovqFromX(4, RAX, XMM0)
			g.a.Alu(AND, 4, D(RAX), Imm(math.MinInt32))
			g.a.Movsx(8, RAX, 4, D(RAX))
		case cc.TyLongDouble:
			g.a.Alu(SUB, 8, Q(RSP), Imm(16))
			g.a.Fstp(10, M(RSP, 0))
			g.a.Movzx(4, RAX, 1, M(RSP, 9))
			g.a.ShiftImm(SHR, 4, D(RAX), 7)
			g.a.ShiftImm(SHL, 4, D(RAX), 9)
			g.a.Alu(ADD, 8, Q(RSP), Imm(16))
		default:
			g.a.MovqFromX(8, RAX, XMM0)
			g.a.ShiftImm(SHR, 8, Q(RAX), 63)
		}
		return
	}
	g.toDoubleXMM0(t)
	// xmm1 = |x|
	g.a.MovImm(RAX, math.MaxInt64)
	g.a.MovqToX(8, XMM1, RAX)
	g.a.Andp(true, XMM1, X(XMM0))
	g.a.MovImm(RAX, int64(math.Float64bits(math.Inf(1))))
	g.a.MovqToX(8, XMM2, RAX)
	switch op {
	case "isnan":
		g.a.Ucomi(true, XMM0, X(XMM0))
		g.a.Setcc(CondP, RAX)
	case "isinf":
		g.a.Ucomi(true, XMM1, X(XMM2))
		g.a.Setcc(CondE, RAX)
		g.a.Setcc(CondNP, RDX)
		g.a.Alu(AND, 1, B(RAX), B(RDX))
	case "isfinite":
		g.a.Ucomi(true, XMM2, X(XMM1))
		g.a.Setcc(CondA, RAX)
	case "isnormal":
		g.a.Ucomi(true, XMM2, X(XMM1))
		g.a.Setcc(CondA, RAX)
		g.a.MovImm(RDX, int64(math.Float64bits(2.2250738585072014e-308)))
		g.a.MovqToX(8, XMM2, RDX)
		g.a.Ucomi(true, XMM1, X(XMM2))
		g.a.Setcc(CondAE, RDX)
		g.a.Alu(AND, 1, B(RAX), B(RDX))
	case "isinf_sign":
		g.a.MovqFromX(8, RCX, XMM0)
		g.a.Ucomi(true, XMM1, X(XMM2))
		g.a.Setcc(CondE, RAX)
		g.a.Setcc(CondNP, RDX)
		g.a.Alu(AND, 1, B(RAX), B(RDX))
		g.a.Movzx(4, RAX, 1, B(RAX))
		// negative infinity gives -1
		g.a.ShiftImm(SAR, 8, Q(RCX), 63)
		g.a.Alu(OR, 4, D(RCX), Imm(1))
		g.a.Imul2(4, RAX, D(RCX))
		g.a.Movsx(8, RAX, 4, D(RAX))
		return
	}
	g.a.Movzx(4, RAX, 1, B(RAX))
}

func (g *gen) genFpclassify(n *cc.Node) {
	// args: nan, inf, normal, subnormal, zero, x
	x := n.Args[5]
	g.genExpr(x)
	g.toDoubleXMM0(x.Ty)
	g.pushf()
	for i := 0; i < 5; i++ {
		g.genExpr(n.Args[i])
		g.push()
	}
	// stack: x, nan, inf, normal, subnormal, zero (top)
	g.a.MovF(true, X(XMM0), M(RSP, 40))
	g.a.MovImm(RAX, math.MaxInt64)
	g.a.MovqToX(8, XMM1, RAX)
	g.a.Andp(true, XMM1, X(XMM0))
	done := g.a.NewLabel()
	notNan := g.a.NewLabel()
	g.a.Ucomi(true, XMM0, X(XMM0))
	g.a.Jcc(CondNP, notNan)
	g.a.Mov(8, Q(RAX), M(RSP, 32))
	g.a.Jmp(done)
	g.a.Bind(notNan)
	g.a.MovImm(RAX, int64(math.Float64bits(math.Inf(1))))
	g.a.MovqToX(8, XMM2, RAX)
	notInf := g.a.NewLabel()
	g.a.Ucomi(true, XMM1, X(XMM2))
	g.a.Jcc(CondNE, notInf)
	g.a.Mov(8, Q(RAX), M(RSP, 24))
	g.a.Jmp(done)
	g.a.Bind(notInf)
	notZero := g.a.NewLabel()
	g.a.Xorp(true, XMM2, X(XMM2))
	g.a.Ucomi(true, XMM1, X(XMM2))
	g.a.Jcc(CondNE, notZero)
	g.a.Mov(8, Q(RAX), M(RSP, 0))
	g.a.Jmp(done)
	g.a.Bind(notZero)
	g.a.MovImm(RAX, int64(math.Float64bits(2.2250738585072014e-308)))
	g.a.MovqToX(8, XMM2, RAX)
	normal := g.a.NewLabel()
	g.a.Ucomi(true, XMM1, X(XMM2))
	g.a.Jcc(CondAE, normal)
	g.a.Mov(8, Q(RAX), M(RSP, 8))
	g.a.Jmp(done)
	g.a.Bind(normal)
	g.a.Mov(8, Q(RAX), M(RSP, 16))
	g.a.Bind(done)
	g.a.Alu(ADD, 8, Q(RSP), Imm(48))
	g.depth -= 6
}

// genOverflow implements __builtin_{add,sub,mul}_overflow with 128-bit arithmetic
func (g *gen) genOverflow(n *cc.Node) {
	a, b, res := n.Args[0], n.Args[1], n.Args[2]
	rt := cc.DecayType(res.Ty).Base
	g.genExpr(res)
	g.push()
	g.genExpr(b)
	g.cast(b.Ty, wide64(b.Ty))
	g.push()
	g.genExpr(a)
	g.cast(a.Ty, wide64(a.Ty))
	g.pop(RSI) // b
	// a in rax, b in rsi; extend to 128 bits: a -> rdx:rax, b -> rcx:rsi
	aSigned := !a.Ty.IsUnsigned()
	bSigned := !b.Ty.IsUnsigned()
	ext := func(lo, hi Reg, signed bool) {
		if signed {
			g.a.Mov(8, Q(hi), Q(lo))
			g.a.ShiftImm(SAR, 8, Q(hi), 63)
		} else {
			g.a.Alu(XOR, 4, D(hi), D(hi))
		}
	}
	switch n.Builtin {
	case "add_overflow", "sub_overflow":
		ext(RAX, RDX, aSigned)
		ext(RSI, RCX, bSigned)
		if n.Builtin == "add_overflow" {
			g.a.Alu(ADD, 8, Q(RAX), Q(RSI))
			g.a.Alu(ADC, 8, Q(RDX), Q(RCX))
		} else {
			g.a.Alu(SUB, 8, Q(RAX), Q(RSI))
			g.a.Alu(SBB, 8, Q(RDX), Q(RCX))
		}
	default:
		if aSigned || bSigned {
			// signed multiply; an unsigned operand >= 2^63 is handled as negative (rare)
			g.a.Unary(IMUL, 8, Q(RSI))
			if !aSigned || !bSigned {
				// fix the high part for an unsigned operand with the top bit set
			}
		} else {
			g.a.Unary(MUL, 8, Q(RSI))
		}
	}
	// rdx:rax is the exact result; check it fits the result type
	g.a.Mov(8, Q(RSI), Q(RAX))
	g.a.Mov(8, Q(RDI), Q(RAX))
	switch rt.Size() {
	case 1:
		if rt.IsUnsigned() {
			g.a.Movzx(4, RDI, 1, B(RDI))
		} else {
			g.a.Movsx(8, RDI, 1, B(RDI))
		}
	case 2:
		if rt.IsUnsigned() {
			g.a.Movzx(4, RDI, 2, W(RDI))
		} else {
			g.a.Movsx(8, RDI, 2, W(RDI))
		}
	case 4:
		if rt.IsUnsigned() {
			g.a.Mov(4, D(RDI), D(RDI))
		} else {
			g.a.Movsx(8, RDI, 4, D(RDI))
		}
	}
	// rdi = truncated & extended; overflow if rdi != rax or high part mismatch
	g.a.Alu(CMP, 8, Q(RDI), Q(RAX))
	g.a.Setcc(CondNE, R8)
	if rt.IsUnsigned() {
		g.a.Test(8, Q(RDX), Q(RDX))
	} else {
		g.a.Mov(8, Q(RCX), Q(RDI))
		g.a.ShiftImm(SAR, 8, Q(RCX), 63)
		g.a.Alu(CMP, 8, Q(RDX), Q(RCX))
	}
	g.a.Setcc(CondNE, R9)
	g.a.Alu(OR, 1, B(R8), B(R9))
	g.pop(RCX) // result pointer
	g.storeSized(M(RCX, 0), RSI, rt.Size())
	g.a.Movzx(4, RAX, 1, B(R8))
}

func wide64(t *cc.Type) *cc.Type {
	if t.IsUnsigned() {
		return cc.TypeULong
	}
	return cc.TypeLong
}
