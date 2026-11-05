package x64

import (
	"math/bits"

	"github.com/ipoluianov/altc/cc"
)

// 128-bit integers in the IR. A value of type __int128 is a pair of i64
// halves: while lowering, an expression of the type evaluates to an opPair
// whose Args are the low and high halves, and variables of the type are
// split into two SSA variables (or live in a 16-byte slot). Every
// operation works on the halves, so no pair is left once the function is
// built (they are pure and dead). Carries, double shifts and comparisons
// that need both halves at once are op128 values; each computes one half
// (or a condition) with add/adc, sub/sbb, shld/shrd and cmov.

type i128Kind int64

const (
	i128AddHi i128Kind = iota // Args: alo, ahi, blo, bhi: the high half of a + b
	i128SubHi                 // the high half of a - b
	i128ShlLo                 // Args: lo, n (0..127): the low half of a << n
	i128ShlHi                 // Args: lo, hi, n
	i128ShrLo                 // Args: lo, hi, n: the low half of a >> n (logical)
	i128ShrHi                 // Args: hi, n
	i128SarLo                 // Args: lo, hi, n: the low half of a >> n (arithmetic)
	i128SarHi                 // Args: hi, n
	i128Lt                    // Args: alo, ahi, blo, bhi; i32 result, also as flags for a branch
	i128Le
	i128ULt
	i128ULe
)

var i128Names = [...]string{"addhi", "subhi", "shllo", "shlhi", "shrlo", "shrhi", "sarlo", "sarhi", "lt", "le", "ult", "ule"}

func (k i128Kind) isCompare() bool { return k >= i128Lt }

// ---- lowering ----

// pair makes a 128-bit value of two i64 halves
func (b *irBuilder) pair(lo, hi *irValue) *irValue {
	b.ensure()
	v := b.f.newValue(b.cur, opPair, iI128, lo, hi)
	b.cur.Instrs = append(b.cur.Instrs, v)
	return v
}

// halves splits a 128-bit value
func halves(v *irValue) (lo, hi *irValue) {
	v = v.resolve()
	if v.Op != opPair {
		panic("128-bit value is not a pair")
	}
	return v.Args[0].resolve(), v.Args[1].resolve()
}

func (b *irBuilder) const128(lo, hi int64) *irValue {
	return b.pair(b.iconst(iI64, lo), b.iconst(iI64, hi))
}

func (b *irBuilder) op128(k i128Kind, t irType, args ...*irValue) *irValue {
	v := b.add(op128, t, args...)
	v.AuxInt = int64(k)
	return v
}

// halfVar is a half of a 128-bit variable
type halfVar struct {
	v    *cc.Obj
	half int // 1: low, 2: high
}

// halfVars are the SSA variables of the halves of a 128-bit variable
func (b *irBuilder) halfVars(v *cc.Obj) [2]*cc.Obj {
	h, ok := b.halves128[v]
	if !ok {
		h = [2]*cc.Obj{{Name: v.Name + ".lo"}, {Name: v.Name + ".hi"}}
		b.varType[h[0]], b.varType[h[1]] = iI64, iI64
		b.halves128[v] = h
		b.halfOf[h[0]], b.halfOf[h[1]] = halfVar{v, 1}, halfVar{v, 2}
	}
	return h
}

func (b *irBuilder) load128(addr *irValue, t *cc.Type) *irValue {
	lo := b.loadN(addr, 8, iI64)
	hi := b.loadN(b.offset(addr, 8), 8, iI64)
	if t.Volatile || t.Atomic {
		lo.vol, hi.vol = true, true
	}
	return b.pair(lo, hi)
}

func (b *irBuilder) store128(addr, v *irValue) {
	lo, hi := halves(v)
	b.storeN(addr, lo, 8)
	b.storeN(b.offset(addr, 8), hi, 8)
}

// ext128 extends an integer (an i32 or i64 value of type from) to 128 bits
func (b *irBuilder) ext128(v *irValue, from *cc.Type) *irValue {
	if is128(from) {
		return v
	}
	if isAddrType(from) || from.Kind == cc.TyPtr || from.Kind == cc.TyNullptr {
		from = cc.TypeULong
	}
	uns := unsignedType(from)
	lo := b.intConvert(v, cSize(from), uns, 8, uns)
	if uns {
		return b.pair(lo, b.iconst(iI64, 0))
	}
	return b.pair(lo, b.add(opSar, iI64, lo, b.iconst(iI64, 63)))
}

// truth128 is a 128-bit value compared with zero (i32 0 or 1)
func (b *irBuilder) truth128(v *irValue) *irValue {
	lo, hi := halves(v)
	return b.add(opNe, iI32, b.add(opOr, iI64, lo, hi), b.iconst(iI64, 0))
}

// convert128 converts between a 128-bit integer and another scalar type
// (at least one of them is 128 bits wide)
func (b *irBuilder) convert128(v *irValue, from, to *cc.Type) *irValue {
	switch {
	case is128(from) && is128(to):
		return v
	case to.Kind == cc.TyBool:
		return b.truth128(v)
	case is128(from) && (to.IsInteger() || to.Kind == cc.TyPtr):
		lo, _ := halves(v)
		if to.Kind == cc.TyPtr {
			return lo
		}
		return b.intConvert(lo, 8, true, cSize(to), unsignedType(to))
	case is128(to) && (from.IsInteger() || from.Kind == cc.TyPtr || isAddrType(from) || from.Kind == cc.TyNullptr):
		return b.ext128(v, from)
	}
	// floating point: the libgcc conversions
	fl, uns := from, unsignedType(to) // the floating type, the signedness of the integer
	if is128(from) {
		fl, uns = to, unsignedType(from)
	}
	fk := map[cc.TypeKind]string{cc.TyFloat: "sf", cc.TyDouble: "df", cc.TyLongDouble: "xf"}[fl.Kind]
	if fk == "" {
		unsupported("128-bit conversion")
	}
	var name string
	switch {
	case is128(to) && uns:
		name = "__fixuns" + fk + "ti"
	case is128(to):
		name = "__fix" + fk + "ti"
	case uns:
		name = "__floatunti" + fk
	default:
		name = "__floatti" + fk
	}
	return b.libcall128(name, []*irValue{v}, []*cc.Type{from}, to)
}

// binary128 lowers an arithmetic operator, shift or comparison with
// 128-bit operands
func (b *irBuilder) binary128(n *cc.Node, x, y *irValue) *irValue {
	lt, rt := n.LHS.Ty, n.RHS.Ty
	if n.Kind == cc.NShl || n.Kind == cc.NShr {
		// the count: an int (or the low half of a 128-bit count)
		cnt := y
		switch c, ok := constOf(y); {
		case is128(rt):
			cnt, _ = halves(y)
		case ok:
			cnt = b.iconst(iI64, c)
		default:
			cnt = b.intConvert(y, cSize(rt), unsignedType(rt), 8, true)
		}
		if !is128(lt) {
			unsupported("shift of a narrow value by a 128-bit count")
		}
		return b.shift128(n.Kind, unsignedType(lt), x, cnt)
	}
	if !is128(lt) || !is128(rt) {
		unsupported("mixed 128-bit operands")
	}
	alo, ahi := halves(x)
	blo, bhi := halves(y)
	uns := unsignedType(lt)
	switch n.Kind {
	case cc.NAdd:
		return b.pair(b.add(opAdd, iI64, alo, blo), b.op128(i128AddHi, iI64, alo, ahi, blo, bhi))
	case cc.NSub:
		return b.pair(b.add(opSub, iI64, alo, blo), b.op128(i128SubHi, iI64, alo, ahi, blo, bhi))
	case cc.NMul:
		hh := b.add(opMulHi, iI64, alo, blo) // unsigned; the low half follows (one mul for both)
		lo := b.add(opMul, iI64, alo, blo)
		hi := b.add(opAdd, iI64, hh, b.add(opAdd, iI64, b.add(opMul, iI64, alo, bhi), b.add(opMul, iI64, ahi, blo)))
		return b.pair(lo, hi)
	case cc.NDiv, cc.NMod:
		name := map[bool]map[cc.NodeKind]string{
			false: {cc.NDiv: "__divti3", cc.NMod: "__modti3"},
			true:  {cc.NDiv: "__udivti3", cc.NMod: "__umodti3"},
		}[unsignedType(n.Ty)][n.Kind]
		return b.libcall128(name, []*irValue{x, y}, []*cc.Type{lt, rt}, n.Ty)
	case cc.NBitAnd, cc.NBitOr, cc.NBitXor:
		op := map[cc.NodeKind]irOp{cc.NBitAnd: opAnd, cc.NBitOr: opOr, cc.NBitXor: opXor}[n.Kind]
		return b.pair(b.add(op, iI64, alo, blo), b.add(op, iI64, ahi, bhi))
	case cc.NEq, cc.NNe:
		d := b.add(opOr, iI64, b.add(opXor, iI64, alo, blo), b.add(opXor, iI64, ahi, bhi))
		op := opEq
		if n.Kind == cc.NNe {
			op = opNe
		}
		return b.add(op, iI32, d, b.iconst(iI64, 0))
	case cc.NLt, cc.NLe:
		k := i128Lt
		switch {
		case n.Kind == cc.NLt && uns:
			k = i128ULt
		case n.Kind == cc.NLe && uns:
			k = i128ULe
		case n.Kind == cc.NLe:
			k = i128Le
		}
		return b.op128(k, iI32, alo, ahi, blo, bhi)
	}
	unsupported("128-bit operator %d", n.Kind)
	return nil
}

// shift128 shifts a 128-bit value by an i64 count (0..127)
func (b *irBuilder) shift128(kind cc.NodeKind, uns bool, x, cnt *irValue) *irValue {
	lo, hi := halves(x)
	if c, ok := constOf(cnt); ok {
		c &= 127
		k := func(n int64) *irValue { return b.iconst(iI64, n) }
		switch {
		case c == 0:
			return x
		case kind == cc.NShl && c < 64:
			return b.pair(b.add(opShl, iI64, lo, k(c)),
				b.add(opOr, iI64, b.add(opShl, iI64, hi, k(c)), b.add(opShr, iI64, lo, k(64-c))))
		case kind == cc.NShl:
			return b.pair(k(0), b.add(opShl, iI64, lo, k(c-64)))
		case c < 64:
			nlo := b.add(opOr, iI64, b.add(opShr, iI64, lo, k(c)), b.add(opShl, iI64, hi, k(64-c)))
			if uns {
				return b.pair(nlo, b.add(opShr, iI64, hi, k(c)))
			}
			return b.pair(nlo, b.add(opSar, iI64, hi, k(c)))
		case uns:
			return b.pair(b.add(opShr, iI64, hi, k(c-64)), k(0))
		}
		return b.pair(b.add(opSar, iI64, hi, k(c-64)), b.add(opSar, iI64, hi, k(63)))
	}
	switch {
	case kind == cc.NShl:
		return b.pair(b.op128(i128ShlLo, iI64, lo, cnt), b.op128(i128ShlHi, iI64, lo, hi, cnt))
	case uns:
		return b.pair(b.op128(i128ShrLo, iI64, lo, hi, cnt), b.op128(i128ShrHi, iI64, hi, cnt))
	}
	return b.pair(b.op128(i128SarLo, iI64, lo, hi, cnt), b.op128(i128SarHi, iI64, hi, cnt))
}

func (b *irBuilder) neg128(x *irValue) *irValue {
	lo, hi := halves(x)
	z := b.iconst(iI64, 0)
	return b.pair(b.add(opNeg, iI64, lo), b.op128(i128SubHi, iI64, z, z, lo, hi))
}

func (b *irBuilder) not128(x *irValue) *irValue {
	lo, hi := halves(x)
	return b.pair(b.add(opNot, iI64, lo), b.add(opNot, iI64, hi))
}

// libcall128 calls a runtime function: args of the given C types, result
// of type rt (128-bit results come back in rax:rdx)
func (b *irBuilder) libcall128(name string, vals []*irValue, types []*cc.Type, rt *cc.Type) *irValue {
	call := &irCall{Direct: name}
	var args []*irValue
	gp, fp := 0, 0
	stack := int64(0)
	push := func(v *irValue, l callLoc) {
		args = append(args, v)
		call.Locs = append(call.Locs, l)
	}
	for i, v := range vals {
		t := types[i]
		switch {
		case is128(t):
			lo, hi := halves(v)
			push(lo, callLoc{Reg: argGP[gp]})
			push(hi, callLoc{Reg: argGP[gp+1]})
			gp += 2
		case isLD(t):
			stack = alignTo(stack, 16)
			push(v, callLoc{Reg: NoReg, Stack: stack, Copy: 16})
			stack += 16
		case v.Type.inXMM():
			push(v, callLoc{Reg: XMM0 + Reg(fp)})
			fp++
		default:
			push(v, callLoc{Reg: argGP[gp]})
			gp++
		}
	}
	call.NArgs = len(args)
	call.NFP = fp
	call.StackSize = stack
	return b.finishCall128(call, args, rt)
}

// finishCall128 adds a call whose arguments are set up and returns its
// result of type rt: a 128-bit result is stored by the call to a slot
func (b *irBuilder) finishCall128(call *irCall, args []*irValue, rt *cc.Type) *irValue {
	switch {
	case is128(rt):
		buf := b.newSlot(16, 16)
		call.RetSlot = buf
		call.RetChunks = []irChunk{{off: 0, size: 8, t: iI64}, {off: 8, size: 8, t: iI64}}
		c := b.add(opCall, iNone, args...)
		c.Call = call
		return b.load128(b.slotRef(buf), rt)
	case isLD(rt):
		buf := b.newSlot(16, 16)
		call.RetX87 = buf
		c := b.add(opCall, iNone, args...)
		c.Call = call
		return b.slotRef(buf)
	}
	c := b.add(opCall, irTypeOf(rt), args...)
	c.Call = call
	return c
}

// ---- folding ----

// fold128 computes an op128 of constant operands, and turns shifts by a
// constant count into plain 64-bit shifts
func (f *irFunc) fold128(v *irValue) {
	if f.shift128Const(v) {
		return
	}
	var c [4]uint64
	for i, a := range v.Args {
		k, ok := constOf(a)
		if !ok {
			return
		}
		c[i] = uint64(k)
	}
	b2i := func(x bool) int64 {
		if x {
			return 1
		}
		return 0
	}
	slt := func(alo, ahi, blo, bhi uint64) bool {
		if ahi != bhi {
			return int64(ahi) < int64(bhi)
		}
		return alo < blo
	}
	ult := func(alo, ahi, blo, bhi uint64) bool {
		if ahi != bhi {
			return ahi < bhi
		}
		return alo < blo
	}
	switch i128Kind(v.AuxInt) {
	case i128AddHi:
		_, carry := bits.Add64(c[0], c[2], 0)
		hi, _ := bits.Add64(c[1], c[3], carry)
		f.setConst(v, int64(hi))
	case i128SubHi:
		_, borrow := bits.Sub64(c[0], c[2], 0)
		hi, _ := bits.Sub64(c[1], c[3], borrow)
		f.setConst(v, int64(hi))
	case i128ShlLo:
		n := c[1] & 127
		if n >= 64 {
			f.setConst(v, 0)
		} else {
			f.setConst(v, int64(c[0]<<n))
		}
	case i128ShlHi:
		n := c[2] & 127
		switch {
		case n == 0:
			f.setConst(v, int64(c[1]))
		case n >= 64:
			f.setConst(v, int64(c[0]<<(n-64)))
		default:
			f.setConst(v, int64(c[1]<<n|c[0]>>(64-n)))
		}
	case i128ShrLo, i128SarLo:
		n := c[2] & 127
		hi := c[1]
		switch {
		case n == 0:
			f.setConst(v, int64(c[0]))
		case n >= 64 && i128Kind(v.AuxInt) == i128SarLo:
			f.setConst(v, int64(hi)>>(n-64))
		case n >= 64:
			f.setConst(v, int64(hi>>(n-64)))
		default:
			f.setConst(v, int64(c[0]>>n|hi<<(64-n)))
		}
	case i128ShrHi:
		n := c[1] & 127
		if n >= 64 {
			f.setConst(v, 0)
		} else {
			f.setConst(v, int64(c[0]>>n))
		}
	case i128SarHi:
		n := min(c[1]&127, 63)
		f.setConst(v, int64(c[0])>>n)
	case i128Lt:
		f.setConst(v, b2i(slt(c[0], c[1], c[2], c[3])))
	case i128Le:
		f.setConst(v, b2i(!slt(c[2], c[3], c[0], c[1])))
	case i128ULt:
		f.setConst(v, b2i(ult(c[0], c[1], c[2], c[3])))
	case i128ULe:
		f.setConst(v, b2i(!ult(c[2], c[3], c[0], c[1])))
	}
}

// shift128Const rewrites a 128-bit shift half whose count is a constant
func (f *irFunc) shift128Const(v *irValue) bool {
	k := i128Kind(v.AuxInt)
	if k > i128SarHi {
		return false
	}
	n, ok := constOf(v.Args[len(v.Args)-1])
	if !ok {
		return false
	}
	allConst := true
	for _, a := range v.Args {
		if _, ok := constOf(a); !ok {
			allConst = false
		}
	}
	if allConst {
		return false // computed by fold128
	}
	c := n & 127
	op := func(o irOp, x *irValue, cnt int64) *irValue {
		if cnt == 0 {
			return x
		}
		return f.before(v, o, iI64, x, f.constBefore(v, iI64, cnt))
	}
	or := func(x, y *irValue) *irValue { return f.before(v, opOr, iI64, x, y) }
	var w *irValue
	switch k {
	case i128ShlLo: // lo, n
		if c >= 64 {
			f.setConst(v, 0)
			return true
		}
		w = op(opShl, v.Args[0], c)
	case i128ShlHi: // lo, hi, n
		lo, hi := v.Args[0], v.Args[1]
		switch {
		case c >= 64:
			w = op(opShl, lo, c-64)
		case c == 0:
			w = hi
		default:
			w = or(op(opShl, hi, c), op(opShr, lo, 64-c))
		}
	case i128ShrLo, i128SarLo: // lo, hi, n
		lo, hi := v.Args[0], v.Args[1]
		o := opShr
		if k == i128SarLo {
			o = opSar
		}
		switch {
		case c >= 64:
			w = op(o, hi, c-64)
		case c == 0:
			w = lo
		default:
			w = or(op(opShr, lo, c), op(opShl, hi, 64-c))
		}
	case i128ShrHi: // hi, n
		if c >= 64 {
			f.setConst(v, 0)
			return true
		}
		w = op(opShr, v.Args[0], c)
	case i128SarHi: // hi, n
		w = op(opSar, v.Args[0], min(c, 63))
	}
	f.replace(v, w)
	return true
}

// ---- emission ----

func (e *irEmitter) op128(v *irValue) {
	a := e.a
	k := i128Kind(v.AuxInt)
	args := v.Args
	switch k {
	case i128AddHi, i128SubHi, i128Lt, i128Le, i128ULt, i128ULe:
		alo, ahi, blo, bhi := args[0], args[1], args[2], args[3]
		if e.carryFor == v {
			// the low half was just computed: only adc/sbb (the operands
			// are loaded without touching the flags)
			e.carryFor = nil
			if c, ok := constOf(ahi); ok && c == 0 {
				a.Mov(4, D(R11), Imm(0))
			} else {
				e.materialize(ahi, R11)
			}
			op := ADC
			if k == i128SubHi {
				op = SBB
			}
			a.Alu(op, 8, Q(R11), e.iop(bhi, 8, RDX, true))
			e.finish(v, R11)
			return
		}
		if k == i128Le || k == i128ULe {
			// a <= b is !(b < a)
			alo, ahi, blo, bhi = blo, bhi, alo, ahi
		}
		// both operand halves before the flags are set (loading a zero
		// constant would clobber them)
		e.materialize(alo, R10)
		e.materialize(ahi, R11)
		y0 := e.iop(blo, 8, RAX, true)
		y1 := e.iop(bhi, 8, RDX, true)
		lowOp, highOp := ADD, ADC
		if k != i128AddHi {
			lowOp, highOp = SUB, SBB
		}
		if k.isCompare() {
			lowOp = CMP
		}
		a.Alu(lowOp, 8, Q(R10), y0)
		a.Alu(highOp, 8, Q(R11), y1)
		if !k.isCompare() {
			e.finish(v, R11)
			return
		}
		c := map[i128Kind]Cond{i128Lt: CondL, i128Le: CondGE, i128ULt: CondB, i128ULe: CondAE}[k]
		if v.fused {
			e.flags = flagCond{flagNormal, c}
			return
		}
		e.setFlagsResult(v, flagCond{flagNormal, c})
	case i128ShlLo:
		e.materialize(args[1], RCX)
		e.materialize(args[0], R10)
		a.Alu(XOR, 4, D(R11), D(R11))
		a.ShiftCL(SHL, 8, Q(R10))
		a.Test(1, B(RCX), Imm(64))
		a.Cmov(CondNE, 8, R10, Q(R11))
		e.finish(v, R10)
	case i128ShlHi:
		e.materialize(args[2], RCX)
		e.materialize(args[0], R10)
		e.materialize(args[1], R11)
		a.ShxdCL(true, R11, R10)
		a.ShiftCL(SHL, 8, Q(R10))
		a.Test(1, B(RCX), Imm(64))
		a.Cmov(CondNE, 8, R11, Q(R10))
		e.finish(v, R11)
	case i128ShrLo, i128SarLo:
		e.materialize(args[2], RCX)
		e.materialize(args[0], R10)
		e.materialize(args[1], R11)
		a.ShxdCL(false, R10, R11)
		if k == i128SarLo {
			a.ShiftCL(SAR, 8, Q(R11))
		} else {
			a.ShiftCL(SHR, 8, Q(R11))
		}
		a.Test(1, B(RCX), Imm(64))
		a.Cmov(CondNE, 8, R10, Q(R11))
		e.finish(v, R10)
	case i128ShrHi:
		e.materialize(args[1], RCX)
		e.materialize(args[0], R11)
		a.Alu(XOR, 4, D(R10), D(R10))
		a.ShiftCL(SHR, 8, Q(R11))
		a.Test(1, B(RCX), Imm(64))
		a.Cmov(CondNE, 8, R11, Q(R10))
		e.finish(v, R11)
	case i128SarHi:
		e.materialize(args[1], RCX)
		e.materialize(args[0], R11)
		a.Mov(8, Q(R10), Q(R11))
		a.ShiftImm(SAR, 8, Q(R10), 63)
		a.ShiftCL(SAR, 8, Q(R11))
		a.Test(1, B(RCX), Imm(64))
		a.Cmov(CondNE, 8, R11, Q(R10))
		e.finish(v, R11)
	default:
		panic("bad 128-bit operation")
	}
}
