package cc

import (
	"encoding/binary"
	"math"
	"math/big"
	"strings"
)

type initializer struct {
	ty         *Type
	tok        *Token
	isFlexible bool
	expr       *Node
	children   []*initializer
	mem        *Member // union: the initialized member
}

func newInitializer(ty *Type, isFlexible bool) *initializer {
	init := &initializer{ty: ty}
	switch ty.Kind {
	case TyVector:
		init.children = make([]*initializer, ty.ArrayLen)
		for i := range init.children {
			init.children[i] = newInitializer(ty.Base, false)
		}
	case TyArray:
		if isFlexible && ty.ArrayLen < 0 {
			init.isFlexible = true
			return init
		}
		n := ty.ArrayLen
		if n < 0 {
			n = 0
		}
		init.children = make([]*initializer, n)
		for i := range init.children {
			init.children[i] = newInitializer(ty.Base, false)
		}
	case TyStruct, TyUnion:
		ms := ty.Rec.Members
		init.children = make([]*initializer, len(ms))
		for i, m := range ms {
			if isFlexible && ty.Rec.Flexible && i == len(ms)-1 && m.Ty.Kind == TyArray && m.Ty.ArrayLen < 0 {
				init.children[i] = &initializer{ty: m.Ty, isFlexible: true}
			} else {
				init.children[i] = newInitializer(m.Ty, false)
			}
		}
	}
	return init
}

func (p *parser) isEnd() bool {
	return p.is("}") || p.is(",") && p.peekAt(1).Is("}")
}

func (p *parser) consumeEnd() bool {
	if p.is("}") {
		p.next()
		return true
	}
	if p.is(",") && p.peekAt(1).Is("}") {
		p.next()
		p.next()
		return true
	}
	return false
}

func (p *parser) skipExcessElement() {
	if p.is("{") {
		p.next()
		for !p.consumeEnd() {
			p.skipExcessElement()
			if !p.is("}") {
				p.consume(",")
			}
		}
		return
	}
	p.warnAt(p.cur(), "excess elements in initializer")
	p.assign()
}

func (p *parser) stringInitializer(init *initializer) {
	t := p.cur()
	s := p.stringLiteral()
	data := s.Var.InitData
	es := s.Var.Ty.Base.Size()
	n := int64(len(data)) / es
	if init.isFlexible {
		*init = *newInitializer(arrayOf(init.ty.Base, n), false)
	}
	bs := init.ty.Base.Size()
	if bs != es {
		p.errorAt(t, "initializing an array of '%s' from a string literal of '%s'", init.ty.Base, s.Var.Ty.Base)
	}
	m := min(init.ty.ArrayLen, n)
	if init.ty.ArrayLen < n-1 {
		p.warnAt(t, "initializer-string for array is too long")
	}
	for i := int64(0); i < m; i++ {
		v := int64(0)
		for b := int64(0); b < es; b++ {
			v |= int64(data[i*es+b]) << (8 * b)
		}
		init.children[i].expr = &Node{Kind: NNum, Val: truncInt(v, init.ty.Base), Tok: t, Ty: init.ty.Base}
	}
}

func (p *parser) arrayDesignator(ty *Type) (int64, int64) {
	t := p.expect("[")
	begin := p.constExpr()
	if ty.ArrayLen >= 0 && begin >= ty.ArrayLen {
		p.errorAt(t, "array index in initializer exceeds array bounds")
	}
	end := begin
	if p.consume("...") {
		end = p.constExpr()
		if ty.ArrayLen >= 0 && end >= ty.ArrayLen {
			p.errorAt(t, "array index in initializer exceeds array bounds")
		}
		if end < begin {
			p.errorAt(t, "array index range in initializer is empty")
		}
	}
	p.expect("]")
	return begin, end
}

func (p *parser) structDesignator(ty *Type) *Member {
	p.expect(".")
	name := p.ident()
	for _, m := range ty.Rec.Members {
		if m.Anon && m.Ty.IsRecord() {
			if findMember(m.Ty.Rec, name.Text) != nil {
				p.pos-- // let the anonymous member see ".name" again
				p.pos--
				for p.toks[p.pos].Kind == TPragma {
					p.pos--
				}
				return m
			}
			continue
		}
		if m.Name == name.Text {
			return m
		}
	}
	p.errorAt(name, "'%s' has no member named '%s'", ty, name.Text)
	return nil
}

// designation parses designators followed by "=" and the initializer
func (p *parser) designation(init *initializer) {
	if p.is("[") {
		if init.ty.Kind != TyArray && init.ty.Kind != TyVector {
			p.errorAt(p.cur(), "array index in non-array initializer")
		}
		begin, end := p.arrayDesignator(init.ty)
		start := p.pos
		for j := begin; j <= end; j++ {
			p.pos = start
			if j < int64(len(init.children)) {
				p.designation(init.children[j])
			}
		}
		p.arrayInitializer2(init, end+1)
		return
	}
	if p.is(".") && init.ty.Kind == TyStruct {
		m := p.structDesignator(init.ty)
		p.designation(init.children[m.Idx])
		init.expr = nil
		p.structInitializer2(init, m.Idx+1)
		return
	}
	if p.is(".") && init.ty.Kind == TyUnion {
		m := p.structDesignator(init.ty)
		init.mem = m
		p.designation(init.children[m.Idx])
		return
	}
	if p.is(".") {
		p.errorAt(p.cur(), "field name not in record or union initializer")
	}
	if p.is("=") {
		p.next()
	}
	p.initializer2(init)
}

// countArrayInitElements counts the elements of an unsized array initializer
func (p *parser) countArrayInitElements(ty *Type) int64 {
	save := p.pos
	saveErrs := len(p.errs)
	first := true
	dummy := newInitializer(ty.Base, true)
	i, max := int64(0), int64(0)
	for !p.consumeEnd() {
		if !first {
			p.expect(",")
		}
		first = false
		if p.is("[") {
			p.next()
			i = p.constExpr()
			if p.consume("...") {
				i = p.constExpr()
			}
			p.expect("]")
			p.designation(dummy)
		} else {
			p.initializer2(dummy)
		}
		i++
		if i > max {
			max = i
		}
	}
	p.pos = save
	p.errs = p.errs[:saveErrs]
	return max
}

func (p *parser) arrayInitializer1(init *initializer) {
	p.expect("{")
	if init.isFlexible {
		n := p.countArrayInitElements(init.ty)
		*init = *newInitializer(arrayOf(init.ty.Base, n), false)
	}
	first := true
	for i := int64(0); !p.consumeEnd(); i++ {
		if !first {
			p.expect(",")
		}
		first = false
		if p.is("[") {
			begin, end := p.arrayDesignator(init.ty)
			start := p.pos
			endPos := p.pos
			for j := begin; j <= end; j++ {
				p.pos = start
				p.designation(init.children[j])
				endPos = p.pos
			}
			p.pos = endPos
			i = end
			continue
		}
		if i < int64(len(init.children)) {
			p.initializer2(init.children[i])
		} else {
			p.skipExcessElement()
		}
	}
}

func (p *parser) arrayInitializer2(init *initializer, i int64) {
	if init.isFlexible {
		save := p.pos
		n := int64(0)
		// count the remaining elements at this level
		dummy := newInitializer(init.ty.Base, false)
		saveErrs := len(p.errs)
		for !p.isEnd() && p.cur().Kind != TEOF {
			if n > 0 {
				p.expect(",")
			}
			if p.is("[") || p.is(".") {
				break
			}
			p.initializer2(dummy)
			n++
		}
		p.pos = save
		p.errs = p.errs[:saveErrs]
		*init = *newInitializer(arrayOf(init.ty.Base, n), false)
	}
	for ; i < int64(len(init.children)) && !p.isEnd(); i++ {
		start := p.pos
		if i > 0 {
			p.expect(",")
		}
		if p.is("[") || p.is(".") {
			p.pos = start
			return
		}
		p.initializer2(init.children[i])
	}
}

func (p *parser) structInitializer1(init *initializer) {
	p.expect("{")
	idx := 0
	first := true
	ms := init.ty.Rec.Members
	for !p.consumeEnd() {
		if !first {
			p.expect(",")
		}
		first = false
		if p.is(".") {
			m := p.structDesignator(init.ty)
			p.designation(init.children[m.Idx])
			idx = m.Idx + 1
			continue
		}
		// GNU old-style designator "field: value"
		if t := p.cur(); t.Kind == TIdent && p.peekAt(1).Is(":") && !t.kw() {
			for _, m := range ms {
				if m.Name == t.Text {
					p.next()
					p.next()
					p.initializer2(init.children[m.Idx])
					idx = m.Idx + 1
					break
				}
			}
			continue
		}
		// unnamed bit-fields take no initializer
		for idx < len(ms) && ms[idx].Bitfield && ms[idx].Name == "" {
			idx++
		}
		if idx < len(ms) {
			p.initializer2(init.children[idx])
			idx++
		} else {
			p.skipExcessElement()
		}
	}
}

func (p *parser) structInitializer2(init *initializer, idx int) {
	ms := init.ty.Rec.Members
	first := true
	for ; idx < len(ms) && !p.isEnd(); idx++ {
		if ms[idx].Bitfield && ms[idx].Name == "" {
			continue
		}
		start := p.pos
		if !first || idx > 0 {
			if !first {
				p.expect(",")
			} else if p.is(",") {
				p.expect(",")
			}
		}
		first = false
		if p.is("[") || p.is(".") {
			p.pos = start
			return
		}
		p.initializer2(init.children[idx])
	}
}

func (p *parser) unionInitializer(init *initializer) {
	ms := init.ty.Rec.Members
	if p.is("{") && p.peekAt(1).Is(".") {
		p.next()
		for {
			m := p.structDesignator(init.ty)
			init.mem = m
			p.designation(init.children[m.Idx])
			if p.consumeEnd() {
				return
			}
			p.expect(",")
			if !p.is(".") {
				for !p.consumeEnd() {
					p.skipExcessElement()
					p.consume(",")
				}
				return
			}
		}
	}
	if len(ms) == 0 {
		if p.consume("{") {
			p.consumeEnd()
		}
		return
	}
	init.mem = ms[0]
	if p.consume("{") {
		if p.consumeEnd() {
			return
		}
		p.initializer2(init.children[0])
		p.consume(",")
		for !p.is("}") {
			p.skipExcessElement()
			p.consume(",")
		}
		p.expect("}")
		return
	}
	p.initializer2(init.children[0])
}

func (p *parser) initializer2(init *initializer) {
	init.tok = p.cur()
	if init.ty.Kind == TyArray && p.cur().Kind == TStr {
		p.stringInitializer(init)
		return
	}
	// { "string" }, the string possibly made of adjacent literals
	k := 1
	for p.peekAt(k).Kind == TStr {
		k++
	}
	if init.ty.Kind == TyArray && p.is("{") && k > 1 && (p.peekAt(k).Is("}") || p.peekAt(k).Is(",") && p.peekAt(k+1).Is("}")) &&
		init.ty.Base.IsInteger() {
		p.next()
		p.stringInitializer(init)
		p.consumeEnd()
		return
	}
	switch init.ty.Kind {
	case TyArray:
		if p.is("{") {
			p.arrayInitializer1(init)
		} else {
			p.arrayInitializer2(init, 0)
		}
		return
	case TyVector:
		if p.is("{") {
			p.arrayInitializer1(init)
			return
		}
		init.expr = p.assign()
		addType(init.expr)
		p.checkAssign(init.ty, init.expr, init.tok, "initialization")
		return
	case TyVLA:
		p.errorAt(p.cur(), "variable-sized object may not be initialized")
	case TyStruct:
		if p.is("{") {
			p.structInitializer1(init)
			return
		}
		// a struct may be initialized with an expression of the same type
		save := p.pos
		e := p.assign()
		addType(e)
		if e.Ty.IsRecord() && e.Ty.Rec == init.ty.Rec {
			init.expr = e
			return
		}
		p.pos = save
		p.structInitializer2(init, 0)
		return
	case TyUnion:
		if !p.is("{") {
			save := p.pos
			e := p.assign()
			addType(e)
			if e.Ty.IsRecord() && e.Ty.Rec == init.ty.Rec {
				init.expr = e
				return
			}
			p.pos = save
		}
		p.unionInitializer(init)
		return
	}
	if p.is("{") {
		// a scalar in braces: int x = {3}; or {} (C23 zero)
		p.next()
		if p.consumeEnd() {
			init.expr = newNum(0, p.cur())
			return
		}
		p.initializer2(init)
		for p.consume(",") && !p.is("}") {
			p.skipExcessElement()
		}
		p.expect("}")
		return
	}
	init.expr = p.assign()
	addType(init.expr)
	p.checkAssign(init.ty, init.expr, init.tok, "initialization")
}

// parseInitializer parses an initializer for an object of type ty. The
// returned type is the completed type for unsized arrays.
func (p *parser) parseInitializer(ty *Type) (*initializer, *Type) {
	init := newInitializer(ty, true)
	p.initializer2(init)
	if ty.IsRecord() && ty.Rec.Flexible {
		// copy the struct type with the flexible member's actual length
		last := init.children[len(init.children)-1]
		if last.ty.Kind == TyArray && last.ty.ArrayLen > 0 {
			nt := *ty
			nr := *ty.Rec
			nr.Members = append([]*Member(nil), ty.Rec.Members...)
			lm := *nr.Members[len(nr.Members)-1]
			lm.Ty = last.ty
			nr.Members[len(nr.Members)-1] = &lm
			nr.Size = max(ty.Rec.Size, lm.Offset+last.ty.Size())
			nt.Rec = &nr
			return init, &nt
		}
		return init, ty
	}
	return init, init.ty
}

// ---- local initializers ----

type initDesg struct {
	next   *initDesg
	idx    int64
	member *Member
	v      *Obj
}

func (p *parser) initDesgExpr(d *initDesg, tok *Token) *Node {
	if d.v != nil {
		return newVarNode(d.v, tok)
	}
	if d.member != nil {
		return &Node{Kind: NMember, LHS: p.initDesgExpr(d.next, tok), Member: d.member, Tok: tok}
	}
	lhs := p.initDesgExpr(d.next, tok)
	rhs := newLong(d.idx, tok)
	return newUnary(NDeref, p.newAdd(lhs, rhs, tok), tok)
}

func (p *parser) createLvarInit(init *initializer, ty *Type, d *initDesg, tok *Token) *Node {
	if ty.Kind == TyVector && init.expr == nil {
		// element by element through a pointer to the elements
		n := &Node{Kind: NNull, Tok: tok}
		for i := int64(0); i < ty.ArrayLen && i < int64(len(init.children)); i++ {
			c := init.children[i]
			if c.expr == nil {
				continue
			}
			addr := newUnary(NAddr, p.initDesgExpr(d, tok), tok)
			addType(addr)
			ptr := &Node{Kind: NCast, LHS: addr, Ty: pointerTo(ty.Base), Tok: tok}
			elem := newUnary(NDeref, p.newAdd(ptr, newLong(i, tok), tok), tok)
			n = p.newBinary(NComma, n, p.newBinary(NAssign, elem, c.expr, tok), tok)
		}
		return n
	}
	if ty.Kind == TyArray {
		n := &Node{Kind: NNull, Tok: tok}
		for i := int64(0); i < ty.ArrayLen && i < int64(len(init.children)); i++ {
			d2 := &initDesg{next: d, idx: i}
			rhs := p.createLvarInit(init.children[i], ty.Base, d2, tok)
			n = p.newBinary(NComma, n, rhs, tok)
		}
		return n
	}
	if ty.Kind == TyStruct && init.expr == nil {
		n := &Node{Kind: NNull, Tok: tok}
		for i, m := range ty.Rec.Members {
			if i >= len(init.children) {
				break
			}
			d2 := &initDesg{next: d, member: m}
			rhs := p.createLvarInit(init.children[i], m.Ty, d2, tok)
			n = p.newBinary(NComma, n, rhs, tok)
		}
		return n
	}
	if ty.Kind == TyUnion && init.expr == nil {
		m := init.mem
		if m == nil {
			if len(ty.Rec.Members) == 0 {
				return &Node{Kind: NNull, Tok: tok}
			}
			m = ty.Rec.Members[0]
		}
		d2 := &initDesg{next: d, member: m}
		return p.createLvarInit(init.children[m.Idx], m.Ty, d2, tok)
	}
	if init.expr == nil {
		return &Node{Kind: NNull, Tok: tok}
	}
	lhs := p.initDesgExpr(d, tok)
	return p.newBinary(NAssign, lhs, init.expr, tok)
}

// lvarInitializer parses the initializer of a local variable and returns
// the code that zeroes it and stores the initial values
func (p *parser) lvarInitializer(v *Obj) *Node {
	tok := p.cur()
	init, ty := p.parseInitializer(v.Ty)
	if ty != v.Ty {
		v.Ty = ty
		if v.Align < ty.Align() {
			v.Align = ty.Align()
		}
	}
	d := &initDesg{v: v}
	lhs := &Node{Kind: NMemzero, Var: v, Tok: tok}
	if !v.Ty.IsRecord() && !v.Ty.IsArray() && !(v.Ty.IsVector() && init.expr == nil) {
		// scalars need no zeroing
		rhs := p.createLvarInit(init, v.Ty, d, tok)
		addType(rhs)
		return rhs
	}
	rhs := p.createLvarInit(init, v.Ty, d, tok)
	n := p.newBinary(NComma, lhs, rhs, tok)
	addType(n)
	return n
}

// ---- global initializers ----

func (p *parser) writeBuf(buf []byte, v uint64, sz int64) {
	for i := int64(0); i < sz; i++ {
		buf[i] = byte(v >> (8 * i))
	}
}

func readBuf(buf []byte, sz int64) uint64 {
	v := uint64(0)
	for i := int64(0); i < sz; i++ {
		v |= uint64(buf[i]) << (8 * i)
	}
	return v
}

// float80 encodes a long double value in x87 extended format
func float80(f *big.Float) []byte {
	out := make([]byte, 10)
	if f == nil || f.Sign() == 0 {
		if f != nil && f.Signbit() {
			out[9] = 0x80
		}
		return out
	}
	if f.IsInf() {
		out[7] = 0x80
		out[8] = 0xff
		out[9] = 0x7f
		if f.Signbit() {
			out[9] |= 0x80
		}
		return out
	}
	mant := new(big.Float).SetPrec(64)
	exp := f.MantExp(mant) // f = mant * 2^exp, 0.5 <= |mant| < 1
	neg := mant.Signbit()
	mant.Abs(mant)
	// mantissa with explicit integer bit: mant * 2^64
	mant.SetMantExp(mant, 64)
	m, _ := mant.Uint64()
	e := exp - 1 + 16383
	if e <= 0 {
		// denormal
		shift := uint(1 - e)
		if shift >= 64 {
			m = 0
		} else {
			m >>= shift
		}
		e = 0
	}
	if e >= 0x7fff {
		return float80(new(big.Float).SetInf(neg))
	}
	binary.LittleEndian.PutUint64(out, m)
	binary.LittleEndian.PutUint16(out[8:], uint16(e))
	if neg {
		out[9] |= 0x80
	}
	return out
}

func float80FromFloat64(f float64) []byte {
	if math.IsNaN(f) {
		out := make([]byte, 10)
		binary.LittleEndian.PutUint64(out, 0xC000000000000000)
		binary.LittleEndian.PutUint16(out[8:], 0x7fff)
		if math.Signbit(f) {
			out[9] |= 0x80
		}
		return out
	}
	if math.IsInf(f, 0) {
		return float80(new(big.Float).SetInf(f < 0))
	}
	bf := new(big.Float).SetPrec(64).SetFloat64(f)
	return float80(bf)
}

func (p *parser) writeGvarData(o *Obj, init *initializer, ty *Type, buf []byte, offset int64) {
	switch ty.Kind {
	case TyVector:
		if init.expr == nil {
			sz := ty.Base.Size()
			for i := int64(0); i < ty.ArrayLen && i < int64(len(init.children)); i++ {
				p.writeGvarData(o, init.children[i], ty.Base, buf, offset+sz*i)
			}
			return
		}
		p.errorAt(init.tok, "initializer element is not constant")
		return
	case TyArray:
		sz := ty.Base.Size()
		for i := int64(0); i < ty.ArrayLen && i < int64(len(init.children)); i++ {
			p.writeGvarData(o, init.children[i], ty.Base, buf, offset+sz*i)
		}
		return
	case TyStruct:
		if init.expr != nil {
			p.copyStructConst(o, init.expr, ty, buf, offset)
			return
		}
		for i, m := range ty.Rec.Members {
			if i >= len(init.children) {
				break
			}
			if m.Bitfield {
				e := init.children[i].expr
				if e == nil {
					continue
				}
				loc := buf[offset+m.Offset:]
				sz := m.Ty.Size()
				old := readBuf(loc, sz)
				nv := uint64(p.eval(e))
				mask := uint64(1)<<uint64(m.BitWidth) - 1
				if m.BitWidth == 64 {
					mask = ^uint64(0)
				}
				combined := old | (nv&mask)<<uint64(m.BitOffset)
				p.writeBuf(loc, combined, sz)
				continue
			}
			p.writeGvarData(o, init.children[i], m.Ty, buf, offset+m.Offset)
		}
		return
	case TyUnion:
		if init.expr != nil {
			p.copyStructConst(o, init.expr, ty, buf, offset)
			return
		}
		if init.mem == nil {
			return
		}
		p.writeGvarData(o, init.children[init.mem.Idx], init.mem.Ty, buf, offset)
		return
	}
	e := init.expr
	if e == nil {
		return
	}
	switch ty.Kind {
	case TyComplex:
		re, im := p.evalComplex(e)
		sz := ty.Base.Size()
		p.writeGvarData(o, &initializer{ty: ty.Base, expr: re}, ty.Base, buf, offset)
		p.writeGvarData(o, &initializer{ty: ty.Base, expr: im}, ty.Base, buf, offset+sz)
		return
	case TyFloat:
		f := p.evalDouble(e)
		binary.LittleEndian.PutUint32(buf[offset:], math.Float32bits(float32(f)))
		return
	case TyDouble:
		f := p.evalDouble(e)
		binary.LittleEndian.PutUint64(buf[offset:], math.Float64bits(f))
		return
	case TyLongDouble:
		if ld := p.evalLD(e); ld != nil {
			copy(buf[offset:], float80(ld))
		} else {
			copy(buf[offset:], float80FromFloat64(p.evalDouble(e)))
		}
		return
	case TyBool:
		if e.Ty.IsFlonum() || e.Kind == NCast && e.LHS.Ty.IsFlonum() {
			src := e
			if e.Kind == NCast {
				src = e.LHS
			}
			if p.evalDouble(src) != 0 {
				buf[offset] = 1
			}
			return
		}
		label := ""
		v := p.eval2(e, &label)
		if label != "" || v != 0 {
			buf[offset] = 1
		}
		return
	}
	if ty.Size() == 16 && ty.IsInteger() {
		// exactly, in 128 bits
		if w, ok := p.evalWide(newCast(e, ty)); ok {
			u := new(big.Int).Mod(w, new(big.Int).Lsh(big.NewInt(1), 128))
			lo := new(big.Int).And(u, new(big.Int).SetUint64(^uint64(0)))
			p.writeBuf(buf[offset:], lo.Uint64(), 8)
			p.writeBuf(buf[offset+8:], new(big.Int).Rsh(u, 64).Uint64(), 8)
			return
		}
	}
	label := ""
	v := p.eval2(e, &label)
	if label == "" {
		if ty.Size() == 16 {
			p.writeBuf(buf[offset:], uint64(v), 8)
			hi := uint64(0)
			if v < 0 && !(e.Ty.IsUnsigned() && e.Ty.Size() == 8) {
				hi = ^uint64(0)
			}
			p.writeBuf(buf[offset+8:], hi, 8)
			return
		}
		p.writeBuf(buf[offset:], uint64(v), ty.Size())
		return
	}
	if l1, l2, ok := strings.Cut(label, "\x00"); ok {
		o.Relocs = append(o.Relocs, InitReloc{Offset: offset, Label: l1, Minus: l2, Addend: v, Size: ty.Size()})
		return
	}
	if ty.Size() != 8 {
		p.errorAt(e.Tok, "initializer element is not computable at load time")
	}
	o.Relocs = append(o.Relocs, InitReloc{Offset: offset, Label: label, Addend: v})
}

// copyStructConst initializes a struct from another constant struct object
// (a compound literal at file scope)
func (p *parser) copyStructConst(o *Obj, e *Node, ty *Type, buf []byte, offset int64) {
	for e.Kind == NCast {
		e = e.LHS
	}
	if e.Kind == NVar && !e.Var.IsLocal && e.Var.InitData != nil {
		copy(buf[offset:offset+ty.Size()], e.Var.InitData)
		for _, r := range e.Var.Relocs {
			r.Offset += offset
			o.Relocs = append(o.Relocs, r)
		}
		return
	}
	p.errorAt(e.Tok, "initializer element is not constant")
}

// gvarInitializer parses the initializer of a static object
func (p *parser) gvarInitializer(o *Obj) {
	saved := p.staticInit
	p.staticInit = true
	defer func() { p.staticInit = saved }()
	init, ty := p.parseInitializer(o.Ty)
	declared := o.Ty
	o.Ty = ty
	if o.Align < ty.Align() {
		o.Align = ty.Align()
	}
	size := ty.Size()
	if size < 0 {
		p.errorAt(o.Tok, "storage size of '%s' isn't known", o.Name)
	}
	buf := make([]byte, size)
	p.writeGvarData(o, init, ty, buf, 0)
	o.InitData = buf
	if declared.IsRecord() && declared.Rec.Flexible {
		// sizeof the object stays sizeof(struct); the initialized tail is extra storage
		o.Ty = declared
	}
	o.IsTentative = false
	o.IsDefinition = true
	if ty.Const && len(o.Relocs) == 0 {
		o.IsConst = true
	}
	if ty.Kind == TyArray && ty.Base.Const && len(o.Relocs) == 0 {
		o.IsConst = true
	}
}
