package x64

import (
	"fmt"
	"math"

	"github.com/ipoluianov/altc/cc"
)

// irUnsupported aborts lowering of a function the IR does not handle
type irUnsupported struct{ reason string }

func unsupported(format string, args ...any) {
	panic(irUnsupported{fmt.Sprintf(format, args...)})
}

// irTypeOf maps a C type to the machine type of its values; aggregates,
// arrays and functions are represented by their address
func irTypeOf(t *cc.Type) irType {
	switch {
	case t.Kind == cc.TyFloat:
		return iF32
	case t.Kind == cc.TyDouble:
		return iF64
	case t.Kind == cc.TyVector:
		return vecIRType(t)
	case t.Kind == cc.TyLongDouble, t.Kind == cc.TyComplex, t.Kind == cc.TyVLA:
		unsupported("type %d", t.Kind)
	case t.Kind == cc.TyPtr, t.Kind == cc.TyNullptr, t.Kind == cc.TyArray, t.Kind == cc.TyFunc,
		t.Kind == cc.TyStruct, t.Kind == cc.TyUnion:
		return iI64
	case t.IsInteger():
		switch t.Size() {
		case 1, 2, 4:
			return iI32
		case 8:
			return iI64
		}
		unsupported("integer of %d bytes", t.Size())
	case t.Kind == cc.TyVoid:
		return iNone
	}
	unsupported("type %d", t.Kind)
	return iNone
}

// isAddrType tells whether expressions of type t evaluate to an address
func isAddrType(t *cc.Type) bool {
	return t.Kind == cc.TyArray || t.Kind == cc.TyFunc || t.Kind == cc.TyStruct || t.Kind == cc.TyUnion
}

func unsignedType(t *cc.Type) bool {
	return t.Kind == cc.TyPtr || t.Kind == cc.TyNullptr || t.IsArray() || t.Kind == cc.TyFunc || t.IsUnsigned()
}

func cSize(t *cc.Type) int64 {
	if t.Kind == cc.TyArray || t.Kind == cc.TyFunc {
		return 8
	}
	return t.Size()
}

type phiPending struct {
	v   *cc.Obj
	phi *irValue
}

type irBuilder struct {
	g        *gen
	fn       *cc.Obj
	f        *irFunc
	cur      *irBlock
	promoted map[*cc.Obj]bool
	slots    map[*cc.Obj]*irSlot
	labels   map[string]*irBlock
	pending  map[*irBlock][]phiPending
	varType  map[*cc.Obj]irType

	analyzed map[*cc.Obj]bool // functions whose locals were classified
	inl      *inlineCtx       // the innermost function being inlined
	inlineN  int              // inlined copies so far (label prefixes)

	gpUsed, fpUsed int     // registers taken by named parameters
	overflow       *irSlot // the first variadic stack argument

	retInMem  bool      // the result goes through a hidden pointer
	retPtr    *irValue  // that pointer
	retChunks []irChunk // a struct result returned in registers
}

// irChunk is an eightbyte of a struct passed in a register
type irChunk struct {
	off  int64
	size int64
	t    irType // iI64, or iF32/iF64 for SSE eightbytes
}

// chunksOf splits a struct passed in registers into eightbytes
func chunksOf(t *cc.Type) []irChunk {
	var cs []irChunk
	sz := t.Size()
	for k := int64(0); k*8 < sz; k++ {
		c := irChunk{off: k * 8, size: min(8, sz-k*8), t: iI64}
		if hasFlonum(t, k*8, k*8+8, 0) {
			c.t = iF64
			if c.size == 4 {
				c.t = iF32
			}
		}
		cs = append(cs, c)
	}
	return cs
}

func (b *irBuilder) param(loc irParamLoc) *irValue {
	pv := b.add(opParam, loc.Type)
	pv.AuxInt = int64(len(b.f.Params))
	b.f.Params = append(b.f.Params, pv)
	b.f.PLocs = append(b.f.PLocs, loc)
	return pv
}

// offset adds a constant to an address
func (b *irBuilder) offset(addr *irValue, off int64) *irValue {
	if off == 0 {
		return addr
	}
	return b.add(opAdd, iI64, addr, b.iconst(iI64, off))
}

// loadN loads n bytes (1..8) as a chunk of type t
func (b *irBuilder) loadN(addr *irValue, n int64, t irType) *irValue {
	v := b.add(opLoad, t, addr)
	v.AuxInt = n
	return v
}

// storeN stores the low n bytes (1..8) of a value
func (b *irBuilder) storeN(addr, val *irValue, n int64) {
	v := b.add(opStore, iNone, val, addr)
	v.AuxInt = n
}

// lowerFunction builds the SSA form of a function
func (g *gen) lowerFunction(fn *cc.Obj) (f *irFunc, err error) {
	defer func() {
		if r := recover(); r != nil {
			if u, ok := r.(irUnsupported); ok {
				f, err = nil, fmt.Errorf("%s", u.reason)
				return
			}
			panic(r)
		}
	}()
	b := &irBuilder{
		g: g, fn: fn, f: &irFunc{Name: fn.Name},
		promoted: map[*cc.Obj]bool{}, slots: map[*cc.Obj]*irSlot{},
		labels: map[string]*irBlock{}, pending: map[*irBlock][]phiPending{},
		varType: map[*cc.Obj]irType{}, analyzed: map[*cc.Obj]bool{},
	}
	b.build()
	return b.f, nil
}

func (b *irBuilder) build() {
	fn := b.fn
	ft := fn.Ty
	if ft.Variadic && fn.VaArea == nil || ft.OldStyle && len(fn.Params) > 0 || fn.LabelAddrs || fn.Body == nil {
		unsupported("old-style function")
	}
	ret := ft.Ret
	switch {
	case ret.Kind == cc.TyLongDouble || retInX87(ret) || retInX87Pair(ret) || is128(ret):
		unsupported("x87 or 128-bit return")
	case ret.IsRecord():
		if _, _, ok := irStructRegs(ret); ok {
			b.retChunks = chunksOf(ret)
		} else {
			b.retInMem = true
			b.f.RetType = iI64
		}
	default:
		b.f.RetType = irTypeOf(ret)
	}

	b.analyzeLocals(fn)

	f := b.f
	f.Entry = f.newBlock()
	f.Entry.sealed = true
	b.cur = f.Entry

	// parameters
	gp, fp := 0, 0
	stackOff := int64(16)
	if b.retInMem {
		b.retPtr = b.param(irParamLoc{Reg: argGP[0], Type: iI64})
		gp = 1
	}
	for _, p := range fn.Params {
		t := p.Ty
		if t.Kind == cc.TyLongDouble || is128(t) || hasLD(t) {
			unsupported("x87 or 128-bit parameter")
		}
		if t.IsRecord() {
			if t.Size() == 0 {
				continue
			}
			ng, nf, ok := irStructRegs(t)
			if ok && gp+ng <= 6 && fp+nf <= 8 {
				addr := b.slotAddr(p)
				for _, c := range chunksOf(t) {
					loc := irParamLoc{Type: c.t}
					if c.t.isFloat() {
						loc.Reg = XMM0 + Reg(fp)
						fp++
					} else {
						loc.Reg = argGP[gp]
						gp++
					}
					pv := b.param(loc)
					b.storeN(b.offset(addr, c.off), pv, c.size)
				}
				continue
			}
			if t.Align() >= 16 {
				stackOff = alignTo(stackOff, 16)
			}
			b.slots[p] = &irSlot{Size: t.Size(), Align: 8, Offset: stackOff, Fixed: true, Name: p.Name}
			stackOff += alignTo(t.Size(), 8)
			continue
		}
		it := irTypeOf(t)
		loc := irParamLoc{Reg: NoReg, Type: it}
		switch {
		case it.inXMM() && fp < 8:
			loc.Reg = XMM0 + Reg(fp)
			fp++
		case !it.inXMM() && gp < 6:
			loc.Reg = argGP[gp]
			gp++
		default:
			if it.isVec() {
				stackOff = alignTo(stackOff, int64(it.size()))
			}
			loc.Stack = stackOff
			stackOff += int64(max(it.size(), 8))
		}
		pv := b.param(loc)
		val := b.narrow(pv, t)
		if b.promoted[p] {
			b.writeVar(p, val)
			continue
		}
		if loc.Reg == NoReg {
			// the incoming stack argument is the variable's home
			b.slots[p] = &irSlot{Size: int64(max(it.size(), 8)), Align: 8, Offset: loc.Stack, Fixed: true, Name: p.Name}
			continue
		}
		b.store(b.slotAddr(p), val, t)
	}

	if fn.VaArea != nil {
		b.slotAddr(fn.VaArea)
		f.VaSlot = b.slots[fn.VaArea]
		b.gpUsed, b.fpUsed = gp, fp
		b.overflow = &irSlot{Fixed: true, Offset: stackOff, Name: "overflow"}
	}

	b.stmt(fn.Body)
	if b.cur != nil {
		if b.retInMem {
			b.ret(b.retPtr)
		} else if fn.Name == "main" && f.RetType != iNone {
			b.ret(b.iconst(f.RetType, 0))
		} else {
			b.ret(nil)
		}
	}
	for _, blk := range f.Blocks {
		b.seal(blk)
	}
}

// analyzeLocals decides which locals of fn become SSA values: those
// whose address is never taken
func (b *irBuilder) analyzeLocals(fn *cc.Obj) {
	if b.analyzed[fn] {
		return
	}
	b.analyzed[fn] = true
	taken := map[*cc.Obj]bool{}
	var addrOf func(n *cc.Node)
	addrOf = func(n *cc.Node) {
		switch n.Kind {
		case cc.NVar:
			taken[n.Var] = true
		case cc.NComma:
			addrOf(n.RHS)
		case cc.NMember, cc.NCast:
			addrOf(n.LHS)
		case cc.NCond:
			addrOf(n.Then)
			addrOf(n.Els)
		case cc.NAssign:
			addrOf(n.LHS)
		}
	}
	cc.Walk(fn.Body, func(n *cc.Node) {
		if n.Kind == cc.NAddr {
			addrOf(n.LHS)
		}
	})
	for _, v := range fn.Locals {
		t := v.Ty
		if taken[v] || v.RegName != "" || t.Volatile || t.Atomic || v.Align > 16 && !t.IsVector() {
			continue
		}
		if t.Kind == cc.TyPtr || t.Kind == cc.TyNullptr || t.IsInteger() && t.Size() <= 8 || t.Kind == cc.TyFloat || t.Kind == cc.TyDouble || t.IsVector() {
			b.promoted[v] = true
			b.varType[v] = irTypeOf(t)
		}
	}
}

// ---- building blocks ----

func (b *irBuilder) ensure() {
	if b.cur == nil {
		b.cur = b.f.newBlock()
		b.cur.sealed = true // unreachable
	}
}

func (b *irBuilder) add(op irOp, t irType, args ...*irValue) *irValue {
	b.ensure()
	v := b.f.newValue(b.cur, op, t, args...)
	b.cur.Instrs = append(b.cur.Instrs, v)
	return v
}

func (b *irBuilder) iconst(t irType, c int64) *irValue {
	if t == iI32 {
		c = int64(int32(c))
	}
	v := b.add(opConst, t)
	v.AuxInt = c
	return v
}

func (b *irBuilder) fconst(t irType, c float64) *irValue {
	if t == iF32 {
		c = float64(float32(c))
	}
	v := b.add(opConst, t)
	v.F = c
	return v
}

func (b *irBuilder) jump(to *irBlock) {
	if b.cur == nil {
		return
	}
	b.cur.Kind = bJump
	addEdge(b.cur, to)
	b.cur = nil
}

func (b *irBuilder) branch(c *irValue, t, f *irBlock) {
	if b.cur == nil {
		return
	}
	b.cur.Kind = bBranch
	b.cur.Ctrl = c
	addEdge(b.cur, t)
	addEdge(b.cur, f)
	b.cur = nil
}

func (b *irBuilder) ret(v *irValue) {
	b.ensure()
	b.cur.Kind = bRet
	b.cur.Ctrl = v
	b.cur = nil
}

func (b *irBuilder) labelBlock(name string) *irBlock {
	if b.inl != nil {
		name = b.inl.prefix + name
	}
	if blk, ok := b.labels[name]; ok {
		return blk
	}
	blk := b.f.newBlock()
	b.labels[name] = blk
	return blk
}

// ---- SSA variables ----

func (b *irBuilder) writeVar(v *cc.Obj, val *irValue) {
	b.ensure()
	b.cur.defs[v] = val
}

func (b *irBuilder) readVar(v *cc.Obj) *irValue {
	b.ensure()
	return b.readVarIn(v, b.cur)
}

func (b *irBuilder) newPhi(blk *irBlock, t irType) *irValue {
	phi := b.f.newValue(blk, opPhi, t)
	blk.Phis = append(blk.Phis, phi)
	return phi
}

func (b *irBuilder) undef(blk *irBlock, t irType) *irValue {
	v := b.f.newValue(blk, opConst, t)
	blk.Instrs = append(blk.Instrs, v)
	return v
}

func (b *irBuilder) readVarIn(v *cc.Obj, blk *irBlock) *irValue {
	if val, ok := blk.defs[v]; ok {
		return val.resolve()
	}
	t := b.varType[v]
	var val *irValue
	switch {
	case !blk.sealed:
		val = b.newPhi(blk, t)
		b.pending[blk] = append(b.pending[blk], phiPending{v, val})
	case len(blk.Preds) == 0:
		val = b.undef(blk, t)
	case len(blk.Preds) == 1:
		val = b.readVarIn(v, blk.Preds[0])
	default:
		phi := b.newPhi(blk, t)
		blk.defs[v] = phi
		val = b.addPhiOperands(v, phi)
	}
	blk.defs[v] = val
	return val
}

func (b *irBuilder) addPhiOperands(v *cc.Obj, phi *irValue) *irValue {
	for _, p := range phi.Block.Preds {
		phi.Args = append(phi.Args, b.readVarIn(v, p))
	}
	return tryRemoveTrivialPhi(phi)
}

// tryRemoveTrivialPhi replaces a phi whose operands are all the same value
func tryRemoveTrivialPhi(phi *irValue) *irValue {
	var same *irValue
	for _, a := range phi.Args {
		a = a.resolve()
		if a == same || a == phi {
			continue
		}
		if same != nil {
			return phi
		}
		same = a
	}
	if same == nil {
		return phi // only self references: unreachable loop; keep
	}
	phi.fwd = same
	return same
}

func (b *irBuilder) seal(blk *irBlock) {
	if blk.sealed {
		return
	}
	blk.sealed = true
	for _, p := range b.pending[blk] {
		b.addPhiOperands(p.v, p.phi)
	}
	delete(b.pending, blk)
}

func (b *irBuilder) newTemp(t irType) *cc.Obj {
	v := &cc.Obj{Name: ".tmp"}
	b.varType[v] = t
	return v
}

// ---- memory ----

func (b *irBuilder) slotAddr(v *cc.Obj) *irValue {
	s, ok := b.slots[v]
	if !ok {
		t := v.Ty
		if t.Kind == cc.TyVLA {
			unsupported("VLA")
		}
		sz := t.Size()
		if sz < 0 {
			sz = 0
		}
		al := max(v.Align, t.Align(), 1)
		if sz >= 16 && al < 16 && t.Kind == cc.TyArray {
			al = 16
		}
		if al > 16 && hasVectorAlign(t, al) {
			al = 16 // vectors are accessed with unaligned moves
		}
		if al > 16 {
			unsupported("over-aligned local")
		}
		s = &irSlot{Size: sz, Align: al, Name: v.Name}
		b.f.Slots = append(b.f.Slots, s)
		b.slots[v] = s
	}
	a := b.add(opSlot, iI64)
	a.Slot = s
	return a
}

func (b *irBuilder) load(addr *irValue, t *cc.Type) *irValue {
	it := irTypeOf(t)
	v := b.add(opLoad, it, addr)
	v.AuxInt = cSize(t)
	v.Signed = !unsignedType(t)
	v.vol = t.Volatile
	return v
}

func (b *irBuilder) store(addr, val *irValue, t *cc.Type) {
	v := b.add(opStore, iNone, val, addr)
	v.AuxInt = cSize(t)
}

// ---- statements ----

func (b *irBuilder) stmt(n *cc.Node) {
	if n == nil {
		return
	}
	switch n.Kind {
	case cc.NIf:
		then, els, end := b.f.newBlock(), b.f.newBlock(), b.f.newBlock()
		b.cond(n.Cond, then, els)
		b.seal(then)
		b.seal(els)
		b.cur = then
		b.stmt(n.Then)
		b.jump(end)
		b.cur = els
		b.stmt(n.Els)
		b.jump(end)
		b.seal(end)
		b.cur = end
	case cc.NFor:
		if n.StackSave != nil {
			unsupported("VLA scope")
		}
		b.stmt(n.Init)
		if n.Cond != nil && duplicable(n.Cond) {
			// rotated: if (cond) do { body; inc } while (cond)
			pre := b.f.newBlock()
			pre.isPreHead = true
			brk, cont := b.labelBlock(n.BrkLabel), b.labelBlock(n.ContLabel)
			b.cond(n.Cond, pre, brk)
			b.seal(pre)
			b.cur = pre
			head := b.f.newBlock()
			b.jump(head)
			b.cur = head
			b.stmt(n.Then)
			b.jump(cont)
			b.seal(cont)
			b.cur = cont
			if n.Inc != nil {
				b.expr(n.Inc)
			}
			b.cond(n.Cond, head, brk)
			b.seal(head)
			b.seal(brk)
			b.cur = brk
			return
		}
		head := b.loopEntry()
		brk, cont := b.labelBlock(n.BrkLabel), b.labelBlock(n.ContLabel)
		body := b.f.newBlock()
		if n.Cond != nil {
			b.cond(n.Cond, body, brk)
		} else {
			b.jump(body)
		}
		b.seal(body)
		b.cur = body
		b.stmt(n.Then)
		b.jump(cont)
		b.seal(cont)
		b.cur = cont
		if n.Inc != nil {
			b.expr(n.Inc)
		}
		b.jump(head)
		b.seal(head)
		b.seal(brk)
		b.cur = brk
	case cc.NDo:
		if n.StackSave != nil {
			unsupported("VLA scope")
		}
		head := b.loopEntry()
		brk, cont := b.labelBlock(n.BrkLabel), b.labelBlock(n.ContLabel)
		b.stmt(n.Then)
		b.jump(cont)
		b.seal(cont)
		b.cur = cont
		b.cond(n.Cond, head, brk)
		b.seal(head)
		b.seal(brk)
		b.cur = brk
	case cc.NSwitch:
		b.switchStmt(n)
	case cc.NCase:
		lb := b.labelBlock(n.Label)
		b.jump(lb)
		b.cur = lb
		b.stmt(n.LHS)
	case cc.NBlock:
		if n.StackSave != nil {
			unsupported("VLA scope")
		}
		for _, s := range n.Body {
			b.stmt(s)
		}
	case cc.NGoto:
		if n.StackSave != nil {
			unsupported("goto out of VLA scope")
		}
		b.jump(b.labelBlock(n.UniqueLabel))
	case cc.NLabel:
		lb := b.labelBlock(n.UniqueLabel)
		b.jump(lb)
		b.cur = lb
		b.stmt(n.LHS)
	case cc.NReturn:
		var v *irValue
		if n.LHS != nil {
			v = b.expr(n.LHS)
		}
		if in := b.inl; in != nil {
			if in.result != nil && v != nil {
				b.writeVar(in.result, v)
			}
			b.jump(in.exit)
			return
		}
		switch {
		case b.retInMem:
			if v != nil {
				c := b.add(opMemcpy, iNone, b.retPtr, v)
				c.AuxInt = b.fn.Ty.Ret.Size()
			}
			b.ret(b.retPtr)
			return
		case b.retChunks != nil:
			if v == nil {
				b.ret(nil)
				return
			}
			var vals []*irValue
			for _, c := range b.retChunks {
				vals = append(vals, b.loadN(b.offset(v, c.off), c.size, c.t))
			}
			b.ensure()
			b.cur.RetVals = vals
			b.ret(nil)
			return
		}
		if b.f.RetType == iNone {
			v = nil
		}
		b.ret(v)
	case cc.NExprStmt:
		b.expr(n.LHS)
	case cc.NAsm:
		if n.Asm != nil {
			b.asmStmt(n)
		}
	case cc.NGotoExpr:
		unsupported("computed goto")
	default:
		b.expr(n)
	}
}

// loopEntry ends the current block with a jump through a preheader to a
// new (unsealed) loop header and continues in the header
func (b *irBuilder) loopEntry() *irBlock {
	pre := b.f.newBlock()
	pre.isPreHead = true
	b.ensure()
	b.jump(pre)
	b.seal(pre)
	b.cur = pre
	head := b.f.newBlock()
	b.jump(head)
	b.cur = head
	return head
}

func (b *irBuilder) switchStmt(n *cc.Node) {
	v := b.expr(n.Cond)
	brk := b.labelBlock(n.BrkLabel)
	var targets []*irBlock
	for _, c := range n.Cases {
		target := b.labelBlock(c.Label)
		targets = append(targets, target)
		var cmp *irValue
		if c.Begin == c.End {
			cmp = b.add(opEq, iI32, v, b.iconst(v.Type, c.Begin))
		} else {
			d := b.add(opSub, v.Type, v, b.iconst(v.Type, c.Begin))
			cmp = b.add(opULe, iI32, d, b.iconst(v.Type, c.End-c.Begin))
		}
		next := b.f.newBlock()
		b.branch(cmp, target, next)
		b.seal(next)
		b.cur = next
	}
	if n.Default != nil {
		d := b.labelBlock(n.Default.Label)
		targets = append(targets, d)
		b.jump(d)
	} else {
		b.jump(brk)
	}
	b.stmt(n.Then)
	b.jump(brk)
	for _, t := range targets {
		b.seal(t)
	}
	b.seal(brk)
	b.cur = brk
}

// duplicable tells whether an expression can be lowered twice (no labels
// or statements inside)
func duplicable(n *cc.Node) bool {
	ok := true
	cc.Walk(n, func(x *cc.Node) {
		switch x.Kind {
		case cc.NStmtExpr, cc.NLabel, cc.NCase, cc.NLabelVal, cc.NAsm, cc.NVaArg:
			ok = false
		}
	})
	return ok
}

// rangeCheck matches lo <= x && x <= hi for a side-effect-free integer x
// and constants lo <= hi; it is lowered as (unsigned)(x - lo) <= hi - lo
func rangeCheck(n *cc.Node) (x *cc.Node, lo, hi int64, ok bool) {
	if n.Kind != cc.NLogAnd || n.LHS.Kind != cc.NLe || n.RHS.Kind != cc.NLe {
		return nil, 0, 0, false
	}
	l, r := n.LHS, n.RHS
	c1, ok1 := intConst(l.LHS)
	c2, ok2 := intConst(r.RHS)
	if !ok1 || !ok2 || !sameVar(l.RHS, r.LHS) || c1 > c2 {
		return nil, 0, 0, false
	}
	t := l.RHS.Ty
	if !t.IsInteger() || unsignedType(t) || t.Size() > 8 {
		return nil, 0, 0, false
	}
	return l.RHS, c1, c2, true
}

// sameVar tells whether two expressions read the same variable
func sameVar(a, b *cc.Node) bool {
	for a.Kind == cc.NCast && b.Kind == cc.NCast && a.Ty.Kind == b.Ty.Kind && a.Ty.Size() == b.Ty.Size() {
		a, b = a.LHS, b.LHS
	}
	return a.Kind == cc.NVar && b.Kind == cc.NVar && a.Var == b.Var && !a.Ty.Volatile
}

// cond lowers a condition as control flow
func (b *irBuilder) cond(n *cc.Node, t, f *irBlock) {
	if x, lo, hi, ok := rangeCheck(n); ok {
		v := b.expr(x)
		d := b.add(opSub, v.Type, v, b.iconst(v.Type, lo))
		b.branch(b.add(opULe, iI32, d, b.iconst(v.Type, hi-lo)), t, f)
		return
	}
	switch n.Kind {
	case cc.NLogAnd:
		mid := b.f.newBlock()
		b.cond(n.LHS, mid, f)
		b.seal(mid)
		b.cur = mid
		b.cond(n.RHS, t, f)
		return
	case cc.NLogOr:
		mid := b.f.newBlock()
		b.cond(n.LHS, t, mid)
		b.seal(mid)
		b.cur = mid
		b.cond(n.RHS, t, f)
		return
	case cc.NNot:
		b.cond(n.LHS, f, t)
		return
	}
	v := b.expr(n)
	b.branch(b.truth(v), t, f)
}

// truth turns a scalar into a condition value
func (b *irBuilder) truth(v *irValue) *irValue {
	if v.Type.isVec() {
		unsupported("vector used as a condition")
	}
	if v.Op.isCompare() {
		return v
	}
	if v.Type.isFloat() {
		return b.add(opFNe, iI32, v, b.fconst(v.Type, 0))
	}
	return b.add(opNe, iI32, v, b.iconst(v.Type, 0))
}

// ---- expressions ----

func (b *irBuilder) expr(n *cc.Node) *irValue {
	switch n.Kind {
	case cc.NNull:
		return nil
	case cc.NNum:
		t := irTypeOf(n.Ty)
		if t.isFloat() {
			return b.fconst(t, n.FVal)
		}
		return b.iconst(t, normInt(n.Val, n.Ty))
	case cc.NVar:
		if b.promoted[n.Var] {
			return b.readVar(n.Var)
		}
		if isAddrType(n.Ty) {
			return b.addr(n)
		}
		return b.load(b.addr(n), n.Ty)
	case cc.NMember:
		if n.Member.Bitfield {
			return b.loadBitfield(n)
		}
		if isAddrType(n.Ty) {
			return b.addr(n)
		}
		return b.load(b.addr(n), n.Ty)
	case cc.NDeref:
		p := b.expr(n.LHS)
		if isAddrType(n.Ty) {
			return p
		}
		return b.load(p, n.Ty)
	case cc.NAddr:
		return b.addr(n.LHS)
	case cc.NAssign:
		return b.assign(n)
	case cc.NComma:
		b.expr(n.LHS)
		return b.expr(n.RHS)
	case cc.NCast:
		if n.Ty.Kind == cc.TyVoid {
			b.expr(n.LHS)
			return nil
		}
		if n.Ty.IsComplex() || n.LHS.Ty.IsComplex() {
			unsupported("complex")
		}
		if n.Ty.IsVector() || n.LHS.Ty.IsVector() {
			return b.vecConvert(b.expr(n.LHS), n.LHS.Ty, n.Ty)
		}
		v := b.expr(n.LHS)
		if isAddrType(n.Ty) {
			return v
		}
		return b.convert(v, n.LHS.Ty, n.Ty)
	case cc.NCond:
		return b.condExpr(n)
	case cc.NNot:
		v := b.expr(n.LHS)
		if v.Type.isFloat() {
			return b.add(opFEq, iI32, v, b.fconst(v.Type, 0))
		}
		return b.add(opEq, iI32, v, b.iconst(v.Type, 0))
	case cc.NLogAnd, cc.NLogOr:
		return b.boolValue(n)
	case cc.NNeg, cc.NBitNot:
		if n.Ty.IsVector() {
			return b.vecUnary(n)
		}
	}
	switch n.Kind {
	case cc.NNeg:
		v := b.expr(n.LHS)
		if v.Type.isFloat() {
			return b.add(opFNeg, v.Type, v)
		}
		return b.add(opNeg, v.Type, v)
	case cc.NBitNot:
		v := b.expr(n.LHS)
		return b.add(opNot, v.Type, v)
	case cc.NFuncall:
		return b.call(n)
	case cc.NStmtExpr:
		for i, s := range n.Body {
			if i == len(n.Body)-1 && s.Kind == cc.NExprStmt {
				return b.expr(s.LHS)
			}
			b.stmt(s)
		}
		return nil
	case cc.NMemzero:
		v := n.Var
		if b.promoted[v] {
			b.writeVar(v, b.iconst(b.varType[v], 0))
			return nil
		}
		if sz := v.Ty.Size(); sz > 0 {
			z := b.add(opMemzero, iNone, b.slotAddr(v))
			z.AuxInt = sz
		}
		return nil
	case cc.NFence:
		b.add(opFence, iNone)
		return nil
	case cc.NBuiltin:
		return b.builtin(n)
	case cc.NVaStart:
		if b.f.VaSlot == nil {
			unsupported("va_start")
		}
		ap := b.expr(n.LHS)
		b.storeN(ap, b.iconst(iI32, int64(b.gpUsed*8)), 4)
		b.storeN(b.offset(ap, 4), b.iconst(iI32, int64(48+b.fpUsed*16)), 4)
		b.storeN(b.offset(ap, 8), b.slotRef(b.overflow), 8)
		b.storeN(b.offset(ap, 16), b.slotRef(b.f.VaSlot), 8)
		return nil
	case cc.NVaCopy:
		dst := b.expr(n.LHS)
		src := b.expr(n.RHS)
		c := b.add(opMemcpy, iNone, dst, src)
		c.AuxInt = 24
		return nil
	case cc.NVaArg:
		return b.vaArg(n)
	case cc.NAdd, cc.NSub, cc.NMul, cc.NDiv, cc.NMod, cc.NBitAnd, cc.NBitOr, cc.NBitXor, cc.NShl, cc.NShr,
		cc.NEq, cc.NNe, cc.NLt, cc.NLe:
		return b.binary(n)
	}
	unsupported("expression %d", n.Kind)
	return nil
}

// addr lowers the address of an lvalue
func (b *irBuilder) addr(n *cc.Node) *irValue {
	switch n.Kind {
	case cc.NVar:
		v := n.Var
		if b.promoted[v] {
			unsupported("address of a register variable")
		}
		if v.IsLocal {
			return b.slotAddr(v)
		}
		if v.IsTLS {
			a := b.add(opTLS, iI64)
			a.Sym = v.AsmName
			a.Signed = b.g.isLocalDef(v)
			return a
		}
		if b.g.isLocalDef(v) || v.IsStatic {
			a := b.add(opGlobal, iI64)
			a.Sym = v.AsmName
			return a
		}
		a := b.add(opGlobalGOT, iI64)
		a.Sym = v.AsmName
		return a
	case cc.NDeref:
		return b.expr(n.LHS)
	case cc.NComma:
		b.expr(n.LHS)
		return b.addr(n.RHS)
	case cc.NMember:
		if n.Member.Bitfield {
			unsupported("address of a bit-field")
		}
		base := b.addr(n.LHS)
		if n.Member.Offset == 0 {
			return base
		}
		return b.add(opAdd, iI64, base, b.iconst(iI64, n.Member.Offset))
	case cc.NAssign, cc.NCond, cc.NStmtExpr, cc.NFuncall:
		if isAddrType(n.Ty) {
			return b.expr(n)
		}
	case cc.NCast:
		if isAddrType(n.Ty) {
			return b.addr(n.LHS)
		}
	}
	unsupported("lvalue %d", n.Kind)
	return nil
}

func (b *irBuilder) assign(n *cc.Node) *irValue {
	lhs := n.LHS
	if lhs.Kind == cc.NVar && b.promoted[lhs.Var] {
		v := b.expr(n.RHS)
		b.writeVar(lhs.Var, v)
		return v
	}
	if lhs.Kind == cc.NMember && lhs.Member.Bitfield {
		return b.storeBitfield(lhs, n.RHS)
	}
	if isAddrType(lhs.Ty) {
		if lhs.Ty.Kind == cc.TyArray || lhs.Ty.Kind == cc.TyFunc {
			unsupported("array assignment")
		}
		dst := b.addr(lhs)
		src := b.expr(n.RHS)
		c := b.add(opMemcpy, iNone, dst, src)
		c.AuxInt = lhs.Ty.Size()
		return dst
	}
	irTypeOf(lhs.Ty)
	a := b.addr(lhs)
	v := b.expr(n.RHS)
	b.store(a, v, lhs.Ty)
	return v
}

// boolValue computes && and || as 0 or 1
func (b *irBuilder) boolValue(n *cc.Node) *irValue {
	t, f, join := b.f.newBlock(), b.f.newBlock(), b.f.newBlock()
	b.cond(n, t, f)
	b.seal(t)
	b.seal(f)
	tmp := b.newTemp(iI32)
	b.cur = t
	b.writeVar(tmp, b.iconst(iI32, 1))
	b.jump(join)
	b.cur = f
	b.writeVar(tmp, b.iconst(iI32, 0))
	b.jump(join)
	b.seal(join)
	b.cur = join
	return b.readVar(tmp)
}

func (b *irBuilder) condExpr(n *cc.Node) *irValue {
	t, f, join := b.f.newBlock(), b.f.newBlock(), b.f.newBlock()
	b.cond(n.Cond, t, f)
	b.seal(t)
	b.seal(f)
	if n.Ty.Kind == cc.TyVoid {
		b.cur = t
		b.expr(n.Then)
		b.jump(join)
		b.cur = f
		b.expr(n.Els)
		b.jump(join)
		b.seal(join)
		b.cur = join
		return nil
	}
	tmp := b.newTemp(irTypeOf(n.Ty))
	b.cur = t
	b.writeVar(tmp, b.expr(n.Then))
	b.jump(join)
	b.cur = f
	b.writeVar(tmp, b.expr(n.Els))
	b.jump(join)
	b.seal(join)
	b.cur = join
	return b.readVar(tmp)
}

func (b *irBuilder) binary(n *cc.Node) *irValue {
	if n.Ty.IsVector() || n.LHS.Ty.IsVector() {
		return b.vecBinary(n)
	}
	lt := n.LHS.Ty
	x := b.expr(n.LHS)
	y := b.expr(n.RHS)
	if x == nil || y == nil {
		unsupported("void operand")
	}
	if x.Type.isFloat() {
		if x.Type != y.Type {
			unsupported("mixed float operands")
		}
		op := map[cc.NodeKind]irOp{cc.NAdd: opFAdd, cc.NSub: opFSub, cc.NMul: opFMul, cc.NDiv: opFDiv,
			cc.NEq: opFEq, cc.NNe: opFNe, cc.NLt: opFLt, cc.NLe: opFLe}[n.Kind]
		if op == opInvalid {
			unsupported("float operator")
		}
		if op.isCompare() {
			return b.add(op, iI32, x, y)
		}
		return b.add(op, x.Type, x, y)
	}
	uns := unsignedType(lt)
	switch n.Kind {
	case cc.NEq, cc.NNe, cc.NLt, cc.NLe:
		// a pointer compared with an integer (a null pointer constant)
		if x.Type == iI32 && y.Type == iI64 {
			x = b.intConvert(x, cSize(n.LHS.Ty), unsignedType(n.LHS.Ty), 8, true)
		}
		if y.Type == iI32 && x.Type == iI64 {
			y = b.intConvert(y, cSize(n.RHS.Ty), unsignedType(n.RHS.Ty), 8, true)
		}
		if x.Type != y.Type {
			unsupported("mixed compare operands")
		}
		var op irOp
		switch n.Kind {
		case cc.NEq:
			op = opEq
		case cc.NNe:
			op = opNe
		case cc.NLt:
			op = opLt
			if uns {
				op = opULt
			}
		default:
			op = opLe
			if uns {
				op = opULe
			}
		}
		return b.add(op, iI32, x, y)
	case cc.NShl, cc.NShr:
		op := opShl
		if n.Kind == cc.NShr {
			op = opSar
			if uns {
				op = opShr
			}
		}
		return b.add(op, irTypeOf(n.Ty), x, y)
	}
	t := irTypeOf(n.Ty)
	if x.Type != t || y.Type != t {
		unsupported("mixed operands")
	}
	resUns := unsignedType(n.Ty)
	var op irOp
	switch n.Kind {
	case cc.NAdd:
		op = opAdd
	case cc.NSub:
		op = opSub
	case cc.NMul:
		op = opMul
	case cc.NDiv:
		op = opDiv
		if resUns {
			op = opUDiv
		}
	case cc.NMod:
		op = opMod
		if resUns {
			op = opUMod
		}
	case cc.NBitAnd:
		op = opAnd
	case cc.NBitOr:
		op = opOr
	case cc.NBitXor:
		op = opXor
	}
	return b.add(op, t, x, y)
}

// narrow re-extends a value of a narrow integer type (parameters and
// call results, whose upper bits the ABI leaves unspecified)
func (b *irBuilder) narrow(v *irValue, t *cc.Type) *irValue {
	if !t.IsInteger() || t.Size() > 2 {
		return v
	}
	uns := t.IsUnsigned() || t.Kind == cc.TyBool
	switch {
	case t.Size() == 1 && uns:
		return b.add(opZExt8, iI32, v)
	case t.Size() == 1:
		return b.add(opSExt8, iI32, v)
	case uns:
		return b.add(opZExt16, iI32, v)
	}
	return b.add(opSExt16, iI32, v)
}

// convert implements C conversions between scalar types
func (b *irBuilder) convert(v *irValue, from, to *cc.Type) *irValue {
	if v == nil {
		unsupported("void value")
	}
	if to.Kind == cc.TyBool {
		if from.Kind == cc.TyBool {
			return v
		}
		if v.Type.isFloat() {
			return b.add(opFNe, iI32, v, b.fconst(v.Type, 0))
		}
		return b.add(opNe, iI32, v, b.iconst(v.Type, 0))
	}
	tt := irTypeOf(to)
	if isAddrType(from) {
		from = cc.TypeULong
	}
	ft := irTypeOf(from)
	fs, ts := cSize(from), cSize(to)
	fu, tu := unsignedType(from), unsignedType(to)
	switch {
	case !ft.isFloat() && !tt.isFloat():
		return b.intConvert(v, fs, fu, ts, tu)
	case !ft.isFloat():
		if fs == 8 && fu {
			return b.add(opCvtU2F, tt, v)
		}
		if fs == 4 && fu {
			v = b.add(opZExt32, iI64, v)
		}
		return b.add(opCvtI2F, tt, v)
	case !tt.isFloat():
		switch {
		case ts == 8 && tu:
			return b.add(opCvtF2U, iI64, v)
		case ts == 8:
			return b.add(opCvtF2I, iI64, v)
		case ts == 4 && tu:
			return b.add(opTrunc, iI32, b.add(opCvtF2I, iI64, v))
		case ts == 4:
			return b.add(opCvtF2I, iI32, v)
		}
		return b.intConvert(b.add(opCvtF2I, iI32, v), 4, false, ts, tu)
	}
	if ft == iF32 && tt == iF64 {
		return b.add(opFExt, iF64, v)
	}
	if ft == iF64 && tt == iF32 {
		return b.add(opFTrunc, iF32, v)
	}
	return v
}

func (b *irBuilder) intConvert(v *irValue, fs int64, fu bool, ts int64, tu bool) *irValue {
	switch {
	case ts == 8:
		if v.Type == iI64 {
			return v
		}
		if fu {
			return b.add(opZExt32, iI64, v)
		}
		return b.add(opSExt32, iI64, v)
	case ts == 4:
		if v.Type == iI64 {
			return b.add(opTrunc, iI32, v)
		}
		return v
	}
	if fs < ts && (fu || !tu) || fs == ts && fu == tu {
		return v
	}
	if v.Type == iI64 {
		v = b.add(opTrunc, iI32, v)
	}
	switch {
	case ts == 1 && tu:
		return b.add(opZExt8, iI32, v)
	case ts == 1:
		return b.add(opSExt8, iI32, v)
	case tu:
		return b.add(opZExt16, iI32, v)
	}
	return b.add(opSExt16, iI32, v)
}

func (b *irBuilder) call(n *cc.Node) *irValue {
	rt := n.Ty
	if rt.Kind == cc.TyLongDouble || retInX87(rt) || retInX87Pair(rt) || is128(rt) {
		unsupported("x87 or 128-bit call result")
	}
	ft := n.FuncTy
	direct := ""
	fnode := n.LHS
	if fnode.Kind == cc.NVar && fnode.Var.IsFunction && !fnode.Var.IsLocal {
		if returnsTwice[fnode.Var.Name] {
			unsupported("returns twice")
		}
		direct = fnode.Var.AsmName
	}
	if direct != "" {
		if v, ok := b.tryInline(n, fnode.Var); ok {
			return v
		}
	}
	call := &irCall{Direct: direct, Variadic: ft != nil && (ft.Variadic || ft.OldStyle)}
	var args []*irValue
	gp, fp := 0, 0
	stack := int64(0)
	push := func(v *irValue, l callLoc) {
		args = append(args, v)
		call.Locs = append(call.Locs, l)
	}
	// a struct result: a buffer in the frame
	var buf *irSlot
	if rt.IsRecord() {
		buf = b.newSlot(rt.Size(), max(rt.Align(), 8))
		if _, _, ok := irStructRegs(rt); ok {
			call.RetSlot = buf
			call.RetChunks = chunksOf(rt)
		} else {
			push(b.slotRef(buf), callLoc{Reg: argGP[0]})
			gp = 1
		}
	}
	for _, a := range n.Args {
		t := a.Ty
		if t.Kind == cc.TyLongDouble || is128(t) || hasLD(t) {
			unsupported("x87 or 128-bit argument")
		}
		if t.IsRecord() {
			if t.Size() == 0 {
				b.expr(a)
				continue
			}
			addr := b.expr(a)
			ng, nf, ok := irStructRegs(t)
			if ok && gp+ng <= 6 && fp+nf <= 8 {
				for _, c := range chunksOf(t) {
					v := b.loadN(b.offset(addr, c.off), c.size, c.t)
					if c.t.isFloat() {
						push(v, callLoc{Reg: XMM0 + Reg(fp)})
						fp++
					} else {
						push(v, callLoc{Reg: argGP[gp]})
						gp++
					}
				}
				continue
			}
			if t.Align() >= 16 {
				stack = alignTo(stack, 16)
			}
			push(addr, callLoc{Reg: NoReg, Stack: stack, Copy: t.Size()})
			stack += alignTo(t.Size(), 8)
			continue
		}
		v := b.expr(a)
		if v == nil {
			unsupported("void argument")
		}
		switch {
		case v.Type.inXMM() && fp < 8:
			push(v, callLoc{Reg: XMM0 + Reg(fp)})
			fp++
		case !v.Type.inXMM() && gp < 6:
			push(v, callLoc{Reg: argGP[gp]})
			gp++
		default:
			if v.Type.isVec() {
				stack = alignTo(stack, int64(v.Type.size()))
			}
			push(v, callLoc{Reg: NoReg, Stack: stack})
			stack += int64(max(v.Type.size(), 8))
		}
	}
	call.NArgs = len(args)
	call.NFP = fp
	call.StackSize = stack
	if direct == "" {
		args = append(args, b.expr(fnode))
	}
	t := iNone
	if !rt.IsRecord() {
		t = irTypeOf(rt)
	}
	c := b.add(opCall, t, args...)
	c.Call = call
	if buf != nil {
		return b.slotRef(buf)
	}
	if t == iNone {
		return nil
	}
	return b.narrow(c, rt)
}

func (b *irBuilder) newSlot(size, align int64) *irSlot {
	s := &irSlot{Size: size, Align: align, Name: "tmp"}
	b.f.Slots = append(b.f.Slots, s)
	return s
}

func (b *irBuilder) slotRef(s *irSlot) *irValue {
	a := b.add(opSlot, iI64)
	a.Slot = s
	return a
}

func (b *irBuilder) builtin(n *cc.Node) *irValue {
	name := n.Builtin
	switch name {
	case "altc_insn":
		return b.insnBuiltin(n)
	case "trap":
		b.add(opTrap, iNone)
		return nil
	case "isunordered":
		x, y := b.expr(n.Args[0]), b.expr(n.Args[1])
		if !x.Type.isFloat() || !y.Type.isFloat() {
			unsupported("isunordered")
		}
		return b.add(opOr, iI32, b.add(opFNe, iI32, x, x), b.add(opFNe, iI32, y, y))
	}
	if len(n.Args) != 1 {
		unsupported("builtin %s", name)
	}
	arg := n.Args[0]
	if unaryIntBuiltins[name] {
		x := b.expr(arg)
		if x.Type.isFloat() {
			unsupported("builtin %s", name)
		}
		v := b.add(opBuiltin, irTypeOf(n.Ty), x)
		v.Sym = name
		v.AuxInt = int64(max(cSize(arg.Ty), 4))
		return v
	}
	x := b.expr(arg)
	if !x.Type.isFloat() {
		unsupported("builtin %s", name)
	}
	t := x.Type
	inf := b.fconst(t, math.Inf(1))
	switch name {
	case "isnan":
		return b.add(opFNe, iI32, x, x)
	case "isinf":
		return b.add(opFEq, iI32, b.add(opFAbs, t, x), inf)
	case "isfinite":
		return b.add(opFLt, iI32, b.add(opFAbs, t, x), inf)
	case "isnormal":
		abs := b.add(opFAbs, t, x)
		least := math.SmallestNonzeroFloat64 * (1 << 52) // DBL_MIN
		if t == iF32 {
			least = float64(math.SmallestNonzeroFloat32) * (1 << 23) // FLT_MIN
		}
		return b.add(opAnd, iI32, b.add(opFGe, iI32, abs, b.fconst(t, least)), b.add(opFLt, iI32, abs, inf))
	case "signbit", "signbitf":
		it := iI64
		if t == iF32 {
			it = iI32
		}
		bitsv := b.add(opFBits, it, x)
		if it == iI32 {
			// like gcc: the sign bit in place
			return b.add(opAnd, iI32, bitsv, b.iconst(iI32, math.MinInt32))
		}
		return b.add(opTrunc, iI32, b.add(opShr, it, bitsv, b.iconst(it, 63)))
	}
	unsupported("builtin %s", name)
	return nil
}

// to64 widens an integer for bit manipulation (the upper bits are
// masked or shifted out afterwards)
func (b *irBuilder) to64(v *irValue) *irValue {
	if v.Type == iI64 {
		return v
	}
	return b.add(opZExt32, iI64, v)
}

// fromBits extracts the bit-field value from the low bits of a 64-bit
// word: shifted to the top and back with the member's signedness
func (b *irBuilder) fieldValue(word *irValue, m *cc.Member) *irValue {
	w, off := m.BitWidth, m.BitOffset
	x := word
	if up := 64 - w - off; up > 0 {
		x = b.add(opShl, iI64, x, b.iconst(iI64, up))
	}
	op := opSar
	if unsignedType(m.Ty) {
		op = opShr
	}
	if w < 64 {
		x = b.add(op, iI64, x, b.iconst(iI64, 64-w))
	}
	if irTypeOf(m.Ty) == iI32 {
		x = b.add(opTrunc, iI32, x)
	}
	return x
}

func (b *irBuilder) loadBitfield(n *cc.Node) *irValue {
	m := n.Member
	irTypeOf(m.Ty)
	base := b.addr(n.LHS)
	a := b.offset(base, m.Offset)
	return b.fieldValue(b.to64(b.load(a, m.Ty)), m)
}

func (b *irBuilder) storeBitfield(lhs, rhs *cc.Node) *irValue {
	m := lhs.Member
	irTypeOf(m.Ty)
	a := b.offset(b.addr(lhs.LHS), m.Offset)
	v := b.to64(b.expr(rhs))
	w, off := m.BitWidth, m.BitOffset
	mask := int64(-1)
	if w < 64 {
		mask = int64(1)<<uint(w) - 1
	}
	nb := b.add(opAnd, iI64, v, b.iconst(iI64, mask))
	if off > 0 {
		nb = b.add(opShl, iI64, nb, b.iconst(iI64, off))
	}
	old := b.to64(b.load(a, m.Ty))
	old = b.add(opAnd, iI64, old, b.iconst(iI64, ^(mask<<uint(off))))
	b.store(a, b.add(opOr, iI64, old, nb), m.Ty)
	// the value of the assignment is the new field value
	shifted := v
	if w < 64 {
		shifted = b.add(opShl, iI64, v, b.iconst(iI64, 64-w))
	}
	x := shifted
	if w < 64 {
		op := opSar
		if unsignedType(m.Ty) {
			op = opShr
		}
		x = b.add(op, iI64, shifted, b.iconst(iI64, 64-w))
	}
	if irTypeOf(m.Ty) == iI32 {
		x = b.add(opTrunc, iI32, x)
	}
	return x
}

// vaArg returns the address of the next variadic argument of type
// n.Ty.Base, taking it from the register save area while registers last
func (b *irBuilder) vaArg(n *cc.Node) *irValue {
	t := n.Ty.Base
	if t.Kind == cc.TyLongDouble || is128(t) || hasLD(t) {
		unsupported("va_arg of x87 or 128-bit type")
	}
	ap := b.expr(n.LHS)
	result := b.newTemp(iI64)
	regs, mem, end := b.f.newBlock(), b.f.newBlock(), b.f.newBlock()
	var ng, nf int
	var chunks []irChunk
	switch {
	case t.IsRecord():
		var ok bool
		ng, nf, ok = irStructRegs(t)
		if !ok {
			ng, nf = -1, -1
		} else {
			chunks = chunksOf(t)
		}
	case isSSE(t):
		nf = 1
	default:
		ng = 1
	}
	if ng < 0 {
		b.jump(mem)
	} else {
		// both counts must still fit: test them one after the other
		var conds []*irValue
		if ng > 0 {
			gpo := b.loadN(ap, 4, iI32)
			conds = append(conds, b.add(opULe, iI32, gpo, b.iconst(iI32, int64(48-8*ng))))
		}
		if nf > 0 {
			fpo := b.loadN(b.offset(ap, 4), 4, iI32)
			conds = append(conds, b.add(opULe, iI32, fpo, b.iconst(iI32, int64(176-16*nf))))
		}
		c := conds[0]
		if len(conds) == 2 {
			c = b.add(opAnd, iI32, conds[0], conds[1])
		}
		b.branch(c, regs, mem)
	}
	b.seal(regs)
	b.seal(mem)

	// from the register save area
	b.cur = regs
	if ng >= 0 {
		save := b.loadN(b.offset(ap, 16), 8, iI64)
		next := func(field int64, step int64) *irValue {
			o := b.loadN(b.offset(ap, field), 4, iI32)
			b.storeN(b.offset(ap, field), b.add(opAdd, iI32, o, b.iconst(iI32, step)), 4)
			return b.add(opAdd, iI64, save, b.add(opZExt32, iI64, o))
		}
		if chunks == nil {
			if nf > 0 {
				b.writeVar(result, next(4, 16))
			} else {
				b.writeVar(result, next(0, 8))
			}
		} else {
			tmp := b.newSlot(alignTo(t.Size(), 8), max(t.Align(), 8))
			for _, ch := range chunks {
				var src *irValue
				if ch.t.isFloat() {
					src = next(4, 16)
				} else {
					src = next(0, 8)
				}
				b.storeN(b.offset(b.slotRef(tmp), ch.off), b.loadN(src, 8, iI64), 8)
			}
			b.writeVar(result, b.slotRef(tmp))
		}
		b.jump(end)
	}

	// from the overflow area
	b.cur = mem
	p := b.loadN(b.offset(ap, 8), 8, iI64)
	if t.Align() >= 16 {
		p = b.add(opAnd, iI64, b.add(opAdd, iI64, p, b.iconst(iI64, 15)), b.iconst(iI64, -16))
	}
	b.storeN(b.offset(ap, 8), b.add(opAdd, iI64, p, b.iconst(iI64, alignTo(max(t.Size(), 8), 8))), 8)
	b.writeVar(result, p)
	b.jump(end)

	b.seal(end)
	b.cur = end
	return b.readVar(result)
}
