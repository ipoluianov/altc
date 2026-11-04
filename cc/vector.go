package cc

// GNU vector types: T __attribute__((vector_size(N))). A vector holds N /
// sizeof(T) elements; the arithmetic, bitwise, shift and comparison
// operators work element-wise, a scalar operand is broadcast, v[i]
// accesses an element, and casts between vectors of the same size
// reinterpret the bits.

// IsVector tells whether the type is a GNU vector type
func (t *Type) IsVector() bool { return t != nil && t.Kind == TyVector }

// VectorOf makes the vector type of size bytes with elements of type base
func VectorOf(base *Type, size int64) *Type {
	return vectorOf(base, size, 16)
}

func vectorOf(base *Type, size, maxAlign int64) *Type {
	al := min(size, max(maxAlign, 16))
	return &Type{Kind: TyVector, Base: base.Unqual(), size: size, align: al, ArrayLen: size / base.Size()}
}

// applyVectorSize turns a declared type into a vector type
func (p *parser) applyVectorSize(ty *Type, size int64, tok *Token) *Type {
	if size <= 0 {
		return ty
	}
	if ty.Kind == TyEnum {
		ty = ty.Enum.underlying()
	}
	bs := ty.Size()
	if !(ty.IsInteger() && ty.Kind != TyBool && !(ty.Kind == TyInt128) || ty.Kind == TyFloat || ty.Kind == TyDouble) {
		p.errorAt(tok, "invalid vector type for attribute 'vector_size'")
		return ty
	}
	if size%bs != 0 || size&(size-1) != 0 {
		p.errorAt(tok, "vector size %d is not a power of two multiple of the element size", size)
		return ty
	}
	v := vectorOf(ty, size, p.opt.VecAlign)
	v.Const, v.Volatile = ty.Const, ty.Volatile
	return v
}

// vecCmpType is the result type of a vector comparison: signed integers
// of the element size
func vecCmpType(t *Type) *Type {
	var e *Type
	switch t.Base.Size() {
	case 1:
		e = tySChar
	case 2:
		e = tyShort
	case 4:
		e = tyInt
	default:
		e = tyLong
	}
	return VectorOf(e, t.Size())
}

// toVector converts an operand to the vector type t: vectors of the same
// size are reinterpreted, scalars are converted to the element type and
// broadcast
func (p *parser) toVector(e *Node, t *Type, tok *Token) *Node {
	addType(e)
	if e.Ty.IsVector() {
		if e.Ty.Size() != t.Size() {
			p.errorAt(tok, "vector operands of different sizes ('%s' and '%s')", e.Ty, t)
		}
		if sameVector(e.Ty, t) {
			return e
		}
		return &Node{Kind: NCast, LHS: e, Ty: t, Tok: tok}
	}
	if !e.Ty.IsNumeric() {
		p.errorAt(tok, "invalid operand of type '%s' for a vector operation", e.Ty)
	}
	return &Node{Kind: NCast, LHS: newCast(e, t.Base), Ty: t, Tok: tok}
}

func sameVector(a, b *Type) bool {
	return a.Size() == b.Size() && a.Base.Kind == b.Base.Kind && a.Base.Size() == b.Base.Size() && a.Base.IsUnsigned() == b.Base.IsUnsigned()
}

// vectorBinary builds an element-wise binary operation
func (p *parser) vectorBinary(op NodeKind, lhs, rhs *Node, tok *Token) *Node {
	addType(lhs)
	addType(rhs)
	t := lhs.Ty
	if !t.IsVector() {
		t = rhs.Ty
	}
	t = t.Unqual()
	isFloat := t.Base.IsFlonum()
	switch op {
	case NMod, NShl, NShr, NBitAnd, NBitOr, NBitXor:
		if isFloat && (op == NMod || op == NShl || op == NShr) {
			p.errorAt(tok, "invalid operands to binary %s (have '%s' and '%s')", opName(op), lhs.Ty, rhs.Ty)
		}
	}
	lhs = p.toVector(lhs, t, tok)
	rhs = p.toVector(rhs, t, tok)
	rt := t
	switch op {
	case NEq, NNe, NLt, NLe:
		rt = vecCmpType(t)
	}
	return &Node{Kind: op, LHS: lhs, RHS: rhs, Ty: rt, Tok: tok}
}

// vectorIndex builds v[i]: the element of the vector as an lvalue
func (p *parser) vectorIndex(v, idx *Node, tok *Token) *Node {
	addType(v)
	base := v.Ty.Base
	if !isLvalueNode(v) {
		if p.fn == nil {
			p.errorAt(tok, "vector subscript of a temporary at file scope")
		}
		tmp := p.newLvar("", v.Ty.Unqual())
		set := p.newBinary(NAssign, newVarNode(tmp, tok), v, tok)
		elem := p.vectorIndex(newVarNode(tmp, tok), idx, tok)
		n := p.newBinary(NComma, set, elem, tok)
		addType(n)
		return n
	}
	addr := newUnary(NAddr, v, tok)
	addType(addr)
	ptr := &Node{Kind: NCast, LHS: addr, Ty: pointerTo(base), Tok: tok}
	return newUnary(NDeref, p.newAdd(ptr, idx, tok), tok)
}

func isLvalueNode(n *Node) bool {
	switch n.Kind {
	case NVar, NDeref:
		return true
	case NMember:
		return isLvalueNode(n.LHS) || n.LHS.Kind == NDeref
	}
	return false
}
