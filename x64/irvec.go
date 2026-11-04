package x64

import (
	"encoding/binary"
	"math"

	"github.com/ipoluianov/altc/cc"
)

// Vector values in the IR: GNU vector types and the operands of the
// instruction builtin used by <immintrin.h>. They live in xmm/ymm
// registers; operations become opVInsn values naming the SSE instruction
// (its VEX form is used when AVX is enabled).

// vecIRType maps a vector type to its IR type
func vecIRType(t *cc.Type) irType {
	switch t.Size() {
	case 16:
		return iV128
	case 32:
		return iV256
	}
	unsupported("vector of %d bytes", t.Size())
	return iNone
}

// elemKind names the element type of a vector like SSE mnemonics do
func elemKind(t *cc.Type) string {
	e := t.Base
	switch {
	case e.Kind == cc.TyFloat:
		return "ps"
	case e.Kind == cc.TyDouble:
		return "pd"
	}
	return [...]string{1: "b", 2: "w", 4: "d", 8: "q"}[e.Size()]
}

func (b *irBuilder) needAVX(t irType) {
	if t == iV256 && !b.g.isa.Has(ISAAVX) {
		unsupported("256-bit vectors need -mavx")
	}
}

// vinsn adds a vector instruction
func (b *irBuilder) vinsn(t irType, mn string, args ...*irValue) *irValue {
	b.needAVX(t)
	for _, a := range args {
		b.needAVX(a.Type)
	}
	v := b.add(opVInsn, t, args...)
	v.Sym = mn
	return v
}

func (b *irBuilder) vinsnImm(t irType, mn string, imm int64, args ...*irValue) *irValue {
	v := b.vinsn(t, mn, args...)
	v.AuxInt = imm
	v.Signed = true
	return v
}

// vconst makes a vector constant
func (b *irBuilder) vconst(t irType, data []byte) *irValue {
	v := b.add(opConst, t)
	v.Data = data
	return v
}

// splat repeats the bytes of one element over a vector
func splat(t irType, elem []byte) []byte {
	var d []byte
	for len(d) < t.size() {
		d = append(d, elem...)
	}
	return d
}

// elemBytes encodes a scalar constant as one element of the vector type
func elemBytes(vt *cc.Type, v *irValue) ([]byte, bool) {
	e := vt.Base
	sz := int(e.Size())
	buf := make([]byte, 8)
	switch {
	case v.Op != opConst:
		return nil, false
	case e.Kind == cc.TyFloat:
		binary.LittleEndian.PutUint32(buf, math.Float32bits(float32(v.F)))
	case e.Kind == cc.TyDouble:
		binary.LittleEndian.PutUint64(buf, math.Float64bits(v.F))
	case v.Type.isFloat():
		return nil, false
	default:
		binary.LittleEndian.PutUint64(buf, uint64(v.AuxInt))
	}
	return buf[:sz], true
}

// broadcast converts a scalar (already of the element type) to a vector
func (b *irBuilder) broadcast(x *irValue, vt *cc.Type) *irValue {
	t := vecIRType(vt)
	b.needAVX(t)
	if d, ok := elemBytes(vt, x); ok {
		return b.vconst(t, splat(t, d))
	}
	v := b.add(opVBroadcast, t, x)
	v.Sym = elemKind(vt)
	return v
}

func (b *irBuilder) vzero(t irType) *irValue {
	return b.vconst(t, make([]byte, t.size()))
}

func (b *irBuilder) vones(t irType) *irValue {
	d := make([]byte, t.size())
	for i := range d {
		d[i] = 0xFF
	}
	return b.vconst(t, d)
}

// vecConvert implements casts to and from vector types
func (b *irBuilder) vecConvert(v *irValue, from, to *cc.Type) *irValue {
	if from.IsVector() {
		return v // reinterpretation: the same register
	}
	return b.broadcast(v, to)
}

// vecUnary lowers -v and ~v
func (b *irBuilder) vecUnary(n *cc.Node) *irValue {
	vt := n.Ty
	t := vecIRType(vt)
	x := b.expr(n.LHS)
	if n.Kind == cc.NBitNot {
		return b.vinsn(t, "pxor", x, b.vones(t))
	}
	switch k := elemKind(vt); k {
	case "ps", "pd":
		sign := []byte{0, 0, 0, 0x80}
		if k == "pd" {
			sign = []byte{0, 0, 0, 0, 0, 0, 0, 0x80}
		}
		return b.vinsn(t, "xor"+k, x, b.vconst(t, splat(t, sign)))
	default:
		return b.vinsn(t, "psub"+k, b.vzero(t), x)
	}
}

// vecBinary lowers element-wise operators
func (b *irBuilder) vecBinary(n *cc.Node) *irValue {
	ot := n.LHS.Ty // the operand vector type
	t := vecIRType(ot)
	k := elemKind(ot)
	float := k == "ps" || k == "pd"
	uns := ot.Base.IsUnsigned()
	x := b.expr(n.LHS)
	y := b.expr(n.RHS)
	sse41 := b.g.isa.Has(ISASSE41)
	avx2 := b.g.isa.Has(ISAAVX2)
	if t == iV256 && !float && !avx2 && n.Kind != cc.NBitAnd && n.Kind != cc.NBitOr && n.Kind != cc.NBitXor {
		unsupported("256-bit integer vectors need -mavx2")
	}
	switch n.Kind {
	case cc.NAdd:
		if float {
			return b.vinsn(t, "add"+k, x, y)
		}
		return b.vinsn(t, "padd"+k, x, y)
	case cc.NSub:
		if float {
			return b.vinsn(t, "sub"+k, x, y)
		}
		return b.vinsn(t, "psub"+k, x, y)
	case cc.NMul:
		switch {
		case float:
			return b.vinsn(t, "mul"+k, x, y)
		case k == "w":
			return b.vinsn(t, "pmullw", x, y)
		case k == "d" && sse41:
			return b.vinsn(t, "pmulld", x, y)
		case k == "d" && t == iV128:
			// SSE2: even lanes and odd lanes with pmuludq, then interleaved
			even := b.vinsn(t, "pmuludq", x, y)
			odd := b.vinsn(t, "pmuludq", b.vinsnImm(t, "psrlq", 32, x), b.vinsnImm(t, "psrlq", 32, y))
			return b.vinsn(t, "punpckldq", b.vinsnImm(t, "pshufd", 8, even), b.vinsnImm(t, "pshufd", 8, odd))
		}
		unsupported("vector multiplication of these elements")
	case cc.NDiv:
		if float {
			return b.vinsn(t, "div"+k, x, y)
		}
		unsupported("integer vector division")
	case cc.NBitAnd:
		return b.vinsn(t, "pand", x, y)
	case cc.NBitOr:
		return b.vinsn(t, "por", x, y)
	case cc.NBitXor:
		return b.vinsn(t, "pxor", x, y)
	case cc.NShl, cc.NShr:
		mn := "psll" + k
		if n.Kind == cc.NShr {
			mn = "psrl" + k
			if !uns {
				mn = "psra" + k
			}
		}
		if k == "b" || mn == "psraq" {
			unsupported("vector shift of these elements")
		}
		if y.Op == opVBroadcast || y.Op == opConst {
			// the same count for every element
			var cnt *irValue
			if y.Op == opConst {
				c := int64(binary.LittleEndian.Uint64(append(append([]byte(nil), y.Data[:ot.Base.Size()]...), make([]byte, 8)...)))
				return b.vinsnImm(t, mn, min(c, int64(ot.Base.Size()*8)), x)
			}
			cnt = y.Args[0]
			if cnt.Type == iI32 {
				cnt = b.add(opZExt32, iI64, cnt)
			}
			cv := b.vinsn(iV128, "movq", cnt)
			return b.vinsn(t, mn, x, cv)
		}
		if avx2 && (k == "d" || k == "q") && mn != "psraq" {
			return b.vinsn(t, mn[:4]+"v"+k, x, y)
		}
		unsupported("shift by a vector")
	case cc.NEq, cc.NNe:
		var m *irValue
		switch {
		case float:
			m = b.vinsnImm(t, "cmp"+k, map[cc.NodeKind]int64{cc.NEq: 0, cc.NNe: 4}[n.Kind], x, y)
			return m
		case k == "q" && !sse41:
			unsupported("64-bit vector comparison needs -msse4.1")
		}
		m = b.vinsn(t, "pcmpeq"+k, x, y)
		if n.Kind == cc.NNe {
			m = b.vinsn(t, "pxor", m, b.vones(t))
		}
		return m
	case cc.NLt, cc.NLe:
		if float {
			return b.vinsnImm(t, "cmp"+k, map[cc.NodeKind]int64{cc.NLt: 1, cc.NLe: 2}[n.Kind], x, y)
		}
		if k == "q" && !b.g.isa.Has(ISASSE42) {
			unsupported("64-bit vector comparison needs -msse4.2")
		}
		if uns {
			// flip the sign bits: unsigned order becomes signed order
			sign := make([]byte, ot.Base.Size())
			sign[len(sign)-1] = 0x80
			s := b.vconst(t, splat(t, sign))
			x = b.vinsn(t, "pxor", x, s)
			y = b.vinsn(t, "pxor", y, s)
		}
		if n.Kind == cc.NLt {
			return b.vinsn(t, "pcmpgt"+k, y, x)
		}
		gt := b.vinsn(t, "pcmpgt"+k, x, y)
		return b.vinsn(t, "pxor", gt, b.vones(t))
	}
	unsupported("vector operator %d", n.Kind)
	return nil
}

// insnBuiltin lowers __builtin_altc_insn(probe, "mnemonic", operands...)
func (b *irBuilder) insnBuiltin(n *cc.Node) *irValue {
	var args []*irValue
	for _, a := range n.Args {
		x := b.expr(a)
		if x == nil {
			unsupported("void operand")
		}
		args = append(args, x)
	}
	t := irTypeOf(n.Ty)
	mn := n.AsmText
	if n.Begin == 1 {
		return b.vinsnImm(t, mn, n.Val, args...)
	}
	// shifts by a general-purpose count: the immediate form for constants,
	// else the count in a vector register
	if immShifts[mn] && len(args) == 2 && !args[1].Type.inXMM() {
		if c, ok := constOf(args[1]); ok {
			if c < 0 || c > 255 {
				c = 255
			}
			return b.vinsnImm(t, mn, c, args[0])
		}
		cnt := args[1]
		if cnt.Type == iI32 {
			cnt = b.add(opZExt32, iI64, cnt)
		}
		return b.vinsn(t, mn, args[0], b.vinsn(iV128, "movq", cnt))
	}
	return b.vinsn(t, mn, args...)
}

// hasVector tells whether a struct or union has a vector member
func hasVector(t *cc.Type) bool {
	switch t.Kind {
	case cc.TyVector:
		return true
	case cc.TyArray:
		return hasVector(t.Base)
	case cc.TyStruct, cc.TyUnion:
		for _, m := range t.Rec.Members {
			if hasVector(m.Ty) {
				return true
			}
		}
	}
	return false
}

// irStructRegs classifies a struct passed by value; structs holding
// vectors (SSEUP classes) are not handled
func irStructRegs(t *cc.Type) (int, int, bool) {
	if hasVector(t) {
		unsupported("struct with vector members passed by value")
	}
	return structRegs(t)
}

// usesVectors tells whether a function works with vector values (only
// the IR backend handles them)
func usesVectors(fn *cc.Obj) bool {
	if fn.Ty.Ret.IsVector() {
		return true
	}
	for _, p := range fn.Params {
		if p.Ty.IsVector() {
			return true
		}
	}
	found := false
	cc.Walk(fn.Body, func(n *cc.Node) {
		if n.Ty != nil && n.Ty.IsVector() {
			found = true
		}
	})
	return found
}

// hasVectorAlign tells whether the alignment al of a local comes from its
// vector type (a vector or an array or struct of vectors), not from an
// explicit request
func hasVectorAlign(t *cc.Type, al int64) bool {
	switch t.Kind {
	case cc.TyVector:
		return t.Align() == al
	case cc.TyArray:
		return hasVectorAlign(t.Base, al)
	}
	return false
}
