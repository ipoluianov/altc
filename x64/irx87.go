package x64

import (
	"math"

	"github.com/ipoluianov/altc/cc"
)

// long double in the IR: an x87 value lives in memory (16 bytes, the
// 80-bit value in the first 10) and an expression of type long double
// evaluates to the address of its value, as struct values do. Variables
// of the type stay in frame slots; each operation (opX87) reads its
// operands from memory and writes its result into a fresh slot, so the x87
// register stack is empty between IR values. Calls pass long double
// arguments in memory and return them in st(0), like the System V ABI.

type x87Kind int64

const (
	x87Add x87Kind = iota // Args: dst, a, b
	x87Sub
	x87Mul
	x87Div
	x87Neg // Args: dst, a
	x87Abs
	x87FromF32 // Args: dst, src (memory holding the value)
	x87FromF64
	x87FromI32
	x87FromI64
	x87FromU64
	x87ToF32 // Args: dst, src (the converted value is stored at dst)
	x87ToF64
	x87ToI64 // dst is a 16-byte scratch slot: the integer at dst, control words after it
	x87ToU64
	x87Eq // Args: a, b; i32 result
	x87Ne
	x87Lt
	x87Le
	x87Unord // isunordered(a, b)
)

var x87Names = [...]string{"add", "sub", "mul", "div", "neg", "abs", "fromf32", "fromf64", "fromi32", "fromi64",
	"fromu64", "tof32", "tof64", "toi64", "tou64", "eq", "ne", "lt", "le", "unord"}

func (k x87Kind) isCompare() bool { return k >= x87Eq }

// ---- lowering ----

// ldSlot is a fresh frame slot for a long double result
func (b *irBuilder) ldSlot() *irValue {
	return b.slotRef(b.newSlot(16, 16))
}

// x87 adds an operation writing a new long double; it returns its address
func (b *irBuilder) x87(k x87Kind, args ...*irValue) *irValue {
	dst := b.ldSlot()
	v := b.add(opX87, iNone, append([]*irValue{dst}, args...)...)
	v.AuxInt = int64(k)
	b.ldOwned[dst] = true
	return dst
}

func (b *irBuilder) x87Compare(k x87Kind, x, y *irValue) *irValue {
	v := b.add(opX87, iI32, x, y)
	v.AuxInt = int64(k)
	return v
}

// ldConst is the address of a long double constant in .rodata
func (b *irBuilder) ldConst(n *cc.Node) *irValue {
	var data []byte
	if n.LD != nil {
		data = cc.Float80(n.LD)
	} else {
		data = cc.Float80FromFloat64(n.FVal)
	}
	return b.ldBytes(data)
}

func (b *irBuilder) ldBytes(data []byte) *irValue {
	data = append(append([]byte(nil), data...), make([]byte, 16-len(data))...)
	a := b.add(opGlobal, iI64)
	a.Sym = b.g.constLabel(data, 16)
	b.ldOwned[a] = true
	return a
}

func (b *irBuilder) ldZero() *irValue { return b.ldBytes(cc.Float80FromFloat64(0)) }

// ldTruth compares a long double with zero
func (b *irBuilder) ldTruth(v *irValue) *irValue {
	return b.x87Compare(x87Ne, v, b.ldZero())
}

// ldBinary lowers an arithmetic operator or comparison on long doubles
func (b *irBuilder) ldBinary(n *cc.Node, x, y *irValue) *irValue {
	switch n.Kind {
	case cc.NAdd:
		return b.x87(x87Add, x, y)
	case cc.NSub:
		return b.x87(x87Sub, x, y)
	case cc.NMul:
		return b.x87(x87Mul, x, y)
	case cc.NDiv:
		return b.x87(x87Div, x, y)
	case cc.NEq:
		return b.x87Compare(x87Eq, x, y)
	case cc.NNe:
		return b.x87Compare(x87Ne, x, y)
	case cc.NLt:
		return b.x87Compare(x87Lt, x, y)
	case cc.NLe:
		return b.x87Compare(x87Le, x, y)
	}
	unsupported("long double operator %d", n.Kind)
	return nil
}

// toLD converts a scalar of type from to long double
func (b *irBuilder) toLD(v *irValue, from *cc.Type) *irValue {
	if isLD(from) {
		return v
	}
	if isAddrType(from) {
		from = cc.TypeULong
	}
	tmp := b.slotRef(b.newSlot(16, 16))
	switch {
	case from.Kind == cc.TyFloat:
		b.storeN(tmp, v, 4)
		return b.x87(x87FromF32, tmp)
	case from.Kind == cc.TyDouble:
		b.storeN(tmp, v, 8)
		return b.x87(x87FromF64, tmp)
	case !from.IsInteger() && from.Kind != cc.TyPtr:
		unsupported("conversion to long double")
	}
	fs, fu := cSize(from), unsignedType(from)
	switch {
	case fs < 4 || fs == 4 && !fu:
		b.storeN(tmp, v, 4) // already extended to 32 bits
		return b.x87(x87FromI32, tmp)
	case fs == 4:
		b.storeN(tmp, b.add(opZExt32, iI64, v), 8)
		return b.x87(x87FromI64, tmp)
	case fu:
		b.storeN(tmp, v, 8)
		return b.x87(x87FromU64, tmp)
	}
	b.storeN(tmp, v, 8)
	return b.x87(x87FromI64, tmp)
}

// fromLD converts a long double to the scalar type to
func (b *irBuilder) fromLD(v *irValue, to *cc.Type) *irValue {
	switch {
	case isLD(to):
		return v
	case to.Kind == cc.TyBool:
		return b.ldTruth(v)
	case to.Kind == cc.TyFloat:
		return b.loadN(b.x87(x87ToF32, v), 4, iF32)
	case to.Kind == cc.TyDouble:
		return b.loadN(b.x87(x87ToF64, v), 8, iF64)
	case !to.IsInteger():
		unsupported("conversion from long double")
	}
	ts, tu := cSize(to), unsignedType(to)
	k := x87ToI64
	if ts == 8 && tu {
		k = x87ToU64
	}
	i := b.loadN(b.x87(k, v), 8, iI64)
	return b.intConvert(i, 8, ts == 8 && tu, ts, tu)
}

// ldBuiltin lowers the floating-point classification builtins on a long
// double
func (b *irBuilder) ldBuiltin(name string, x *irValue) (*irValue, bool) {
	inf := b.ldBytes(cc.Float80FromFloat64(math.Inf(1)))
	switch name {
	case "isnan":
		return b.x87Compare(x87Unord, x, x), true
	case "isinf":
		return b.x87Compare(x87Eq, b.x87(x87Abs, x), inf), true
	case "isfinite":
		return b.x87Compare(x87Lt, b.x87(x87Abs, x), inf), true
	case "signbit", "signbitf", "signbitl":
		// the sign is the top bit of byte 9
		top := b.loadN(b.offset(x, 9), 1, iI32)
		return b.add(opShr, iI32, top, b.iconst(iI32, 7)), true
	}
	return nil, false
}

// ---- emission ----

// x87Mem is the memory operand of an address value
func (e *irEmitter) x87Mem(v *irValue, scratch Reg) Mem {
	switch v.Op {
	case opSlot:
		return M(RBP, int32(v.Slot.Offset))
	case opGlobal:
		return MSym(v.Sym, 0)
	}
	return M(e.ireg(v, scratch), 0)
}

func (e *irEmitter) x87(v *irValue) {
	a := e.a
	k := x87Kind(v.AuxInt)
	if k.isCompare() {
		x := e.x87Mem(v.Args[0], R10)
		y := e.x87Mem(v.Args[1], R11)
		// fucomip compares st(0) with st(1) and sets ZF, PF, CF like
		// ucomisd: st(0) = the left operand of a "greater" test
		var fc flagCond
		switch k {
		case x87Lt: // y > x
			a.Fld(10, x)
			a.Fld(10, y)
			fc = flagCond{flagNormal, CondA}
		case x87Le:
			a.Fld(10, x)
			a.Fld(10, y)
			fc = flagCond{flagNormal, CondAE}
		case x87Eq:
			a.Fld(10, y)
			a.Fld(10, x)
			fc = flagCond{kind: flagFEq}
		case x87Ne:
			a.Fld(10, y)
			a.Fld(10, x)
			fc = flagCond{kind: flagFNe}
		case x87Unord:
			a.Fld(10, y)
			a.Fld(10, x)
			fc = flagCond{flagNormal, CondP}
		}
		a.Fucomip(1)
		a.FstpSt(0)
		e.setFlagsResult(v, fc)
		return
	}
	dst := e.x87Mem(v.Args[0], R11)
	src := e.x87Mem(v.Args[1], R10)
	switch k {
	case x87Add, x87Sub, x87Mul, x87Div:
		a.Fld(10, src)
		a.Fld(10, e.x87Mem(v.Args[2], RAX))
		switch k {
		case x87Add:
			a.Faddp()
		case x87Sub:
			a.Fsubp()
		case x87Mul:
			a.Fmulp()
		default:
			a.Fdivp()
		}
		a.Fstp(10, dst)
	case x87Neg, x87Abs:
		a.Fld(10, src)
		if k == x87Neg {
			a.Fchs()
		} else {
			a.Fabs()
		}
		a.Fstp(10, dst)
	case x87FromF32:
		a.Fld(4, src)
		a.Fstp(10, dst)
	case x87FromF64:
		a.Fld(8, src)
		a.Fstp(10, dst)
	case x87FromI32:
		a.Fild(4, src)
		a.Fstp(10, dst)
	case x87FromI64, x87FromU64:
		a.Fild(8, src)
		if k == x87FromU64 {
			// a negative signed value means 2^64 more
			skip := a.NewLabel()
			a.Alu(CMP, 8, src, Imm(0))
			a.Jcc(CondNS, skip)
			a.Fld(4, MSym(e.g.constLabel(f32bytes(18446744073709551616.0), 4), 0))
			a.Faddp()
			a.Bind(skip)
		}
		a.Fstp(10, dst)
	case x87ToF32:
		a.Fld(10, src)
		a.Fstp(4, dst)
	case x87ToF64:
		a.Fld(10, src)
		a.Fstp(8, dst)
	case x87ToI64:
		a.Fld(10, src)
		e.x87Truncate(dst)
	case x87ToU64:
		// values from 2^63 up: subtract 2^63 and set the top bit
		big, end := a.NewLabel(), a.NewLabel()
		two63 := MSym(e.g.constLabel(f32bytes(9223372036854775808.0), 4), 0)
		a.Fld(10, src)
		a.Fld(4, two63)
		a.Fucomip(1) // 2^63 vs the value
		a.Jcc(CondBE, big)
		e.x87Truncate(dst)
		a.Jmp(end)
		a.Bind(big)
		a.Fld(4, two63)
		a.Fsubp()
		e.x87Truncate(dst)
		a.MovImm(RAX, math.MinInt64)
		a.Alu(XOR, 8, dst, Q(RAX))
		a.Bind(end)
	default:
		panic("bad x87 operation")
	}
}

// x87Truncate stores st(0) as a 64-bit integer at m, rounding toward zero
// (the control words go to m+8 and m+10), and pops it
func (e *irEmitter) x87Truncate(m Mem) {
	a := e.a
	cw, cwTrunc := mdisp(m, 8), mdisp(m, 10)
	a.Fnstcw(cw)
	a.Movzx(4, RAX, 2, cw)
	a.Alu(OR, 4, D(RAX), Imm(0xC00))
	a.Mov(2, cwTrunc, W(RAX))
	a.Fldcw(cwTrunc)
	a.Fistp(8, m)
	a.Fldcw(cw)
}

func f32bytes(f float32) []byte {
	bits := math.Float32bits(f)
	return []byte{byte(bits), byte(bits >> 8), byte(bits >> 16), byte(bits >> 24)}
}
