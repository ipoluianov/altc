package x64

import "github.com/ipoluianov/altc/cc"

// Complex numbers in the IR. The front end lowers the arithmetic to
// operations on the parts; here a value of type float _Complex or double
// _Complex is a pair (opPair of type iC32 or iC64) of its real and
// imaginary parts, as a 128-bit integer is a pair of halves (ir128.go):
// variables of the type are split into two SSA variables, memory holds
// the parts one after the other, and the calling convention takes them
// apart. A long double _Complex (iCLD) is a pair of long double values,
// which are addresses of x87 values in memory (irx87.go): its variables
// stay in frame slots, it goes to functions in memory and comes back in
// st(0) and st(1).

// isCplx tells whether values of type t are complex pairs
func isCplx(t *cc.Type) bool { return t.IsComplex() }

// isCLD tells whether t is long double _Complex
func isCLD(t *cc.Type) bool { return t.IsComplex() && isLD(t.Base) }

// cplxConst is a complex constant
func (b *irBuilder) cplxConst(t irType, re, im float64) *irValue {
	if t == iCLD {
		return b.pairOf(t, b.ldBytes(cc.Float80FromFloat64(re)), b.ldBytes(cc.Float80FromFloat64(im)))
	}
	return b.pairOf(t, b.fconst(t.part(), re), b.fconst(t.part(), im))
}

// cplxZero is a zero part for a complex type
func (b *irBuilder) cplxZero(t irType) *irValue {
	if t == iCLD {
		return b.ldZero()
	}
	return b.fconst(t.part(), 0)
}

func (b *irBuilder) loadCplx(addr *irValue, t *cc.Type) *irValue {
	if t.Atomic {
		unsupported("atomic complex")
	}
	it := irTypeOf(t)
	if it == iCLD {
		// the parts are where they are, as for long double values
		return b.pairOf(it, addr, b.offset(addr, 16))
	}
	n := int64(it.size() / 2)
	re := b.loadN(addr, n, it.part())
	im := b.loadN(b.offset(addr, n), n, it.part())
	if t.Volatile {
		re.vol, im.vol = true, true
	}
	return b.pairOf(it, re, im)
}

func (b *irBuilder) storeCplx(addr, v *irValue, t *cc.Type) {
	if t.Atomic {
		unsupported("atomic complex")
	}
	re, im := halves(v)
	if v.Type == iCLD {
		for i, x := range []*irValue{re, im} {
			c := b.add(opMemcpy, iNone, b.offset(addr, int64(16*i)), x)
			c.AuxInt = 10
		}
		return
	}
	n := int64(re.Type.size())
	b.storeN(addr, re, n)
	b.storeN(b.offset(addr, n), im, n)
}

// cast converts a scalar value between any two scalar types
func (b *irBuilder) cast(v *irValue, from, to *cc.Type) *irValue {
	switch {
	case isLD(from):
		return b.fromLD(v, to)
	case isLD(to):
		return b.toLD(v, from)
	}
	return b.convert(v, from, to)
}

// cplxConvert converts to, from or between complex types
func (b *irBuilder) cplxConvert(v *irValue, from, to *cc.Type) *irValue {
	switch {
	case from.IsComplex() && to.IsComplex():
		re, im := halves(v)
		return b.pairOf(irTypeOf(to), b.cast(re, from.Base, to.Base), b.cast(im, from.Base, to.Base))
	case to.IsComplex():
		t := irTypeOf(to)
		return b.pairOf(t, b.cast(v, from, to.Base), b.cplxZero(t))
	case to.Kind == cc.TyBool:
		return b.cplxTruth(v)
	}
	re, _ := halves(v)
	return b.cast(re, from.Base, to)
}

// cplxTruth compares a complex value with zero (i32 0 or 1)
func (b *irBuilder) cplxTruth(v *irValue) *irValue {
	re, im := halves(v)
	if v.Type == iCLD {
		return b.add(opOr, iI32, b.ldTruth(re), b.ldTruth(im))
	}
	t := re.Type
	return b.add(opOr, iI32, b.add(opFNe, iI32, re, b.fconst(t, 0)), b.add(opFNe, iI32, im, b.fconst(t, 0)))
}

// cplxPart is the real or imaginary part of a complex value (member m of
// the complex type)
func cplxPart(v *irValue, m *cc.Member) *irValue {
	re, im := halves(v)
	if m.Offset == 0 {
		return re
	}
	return im
}

// cplxSetPart replaces the real or imaginary part of a complex value
func (b *irBuilder) cplxSetPart(v *irValue, m *cc.Member, x *irValue) *irValue {
	re, im := halves(v)
	if m.Offset == 0 {
		re = x
	} else {
		im = x
	}
	return b.pairOf(v.Type, re, im)
}

// inMemory tells whether a complex lvalue lives in memory (its parts
// are read with loads), not in SSA variables or as a computed value
func (b *irBuilder) inMemory(n *cc.Node) bool {
	switch n.Kind {
	case cc.NVar:
		return !b.promoted[n.Var]
	case cc.NDeref:
		return true
	case cc.NMember:
		return isAddrType(n.LHS.Ty) // a member of a struct or union
	case cc.NComma:
		return b.inMemory(n.RHS)
	}
	return false
}

// The calling convention: float _Complex is one SSE eightbyte (both parts
// in the low half of an xmm register), double _Complex two.

// cplxPack puts the parts of a float _Complex into one f64 value
func (b *irBuilder) cplxPack(v *irValue) *irValue {
	re, im := halves(v)
	p := b.add(opVInsn, iF64, re, im)
	p.Sym = "unpcklps"
	return p
}

// cplxUnpack takes a float _Complex apart from its packed form
func (b *irBuilder) cplxUnpack(x *irValue) *irValue {
	re := b.add(opVInsn, iF32, x)
	re.Sym = "" // the low element as it is
	im := b.add(opVInsn, iF32, x, x)
	im.Sym = "shufps"
	im.Signed, im.AuxInt = true, 1 // element 1 to element 0
	return b.pairOf(iC32, re, im)
}

// cplxRegs are the values passed in registers for a complex value
func (b *irBuilder) cplxRegs(v *irValue) []*irValue {
	if v.Type == iC32 {
		return []*irValue{b.cplxPack(v)}
	}
	re, im := halves(v)
	return []*irValue{re, im}
}

// cplxChunks are the eightbytes of a complex value in registers
func cplxChunks(t irType) []irChunk {
	if t == iC32 {
		return []irChunk{{off: 0, size: 8, t: iF64}}
	}
	return []irChunk{{off: 0, size: 8, t: iF64}, {off: 8, size: 8, t: iF64}}
}

// ownParts copies the parts of a complex long double made of other
// values into memory of their own: a part may be the memory of the
// variable it is stored to (z = CMPLXL(cimagl(z), creall(z)))
func (b *irBuilder) ownParts(v *irValue) *irValue {
	re, im := halves(v)
	own := func(x *irValue) *irValue {
		if b.ldOwned[x] {
			return x
		}
		s := b.ldSlot()
		c := b.add(opMemcpy, iNone, s, x)
		c.AuxInt = 10
		b.ldOwned[s] = true
		return s
	}
	re = own(re)
	return b.pairOf(iCLD, re, own(im))
}
