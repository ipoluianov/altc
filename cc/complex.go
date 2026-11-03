package cc

// Complex numbers are lowered to operations on their real and imaginary
// parts. NMakeComplex builds a value from two parts (the code generator
// gives each such node a temporary).

const NMakeComplex = NFence + 1

func zeroOf(t *Type, tok *Token) *Node {
	return &Node{Kind: NNum, Ty: t, Tok: tok}
}

func makeComplex(ct *Type, re, im *Node, tok *Token) *Node {
	return &Node{Kind: NMakeComplex, LHS: newCast(re, ct.Base), RHS: newCast(im, ct.Base), Ty: ct, Tok: tok}
}

// isPure tells whether an expression can be evaluated twice
func isPure(n *Node) bool {
	switch n.Kind {
	case NNum:
		return true
	case NVar:
		return n.Var.Ty.Kind != TyVLA
	case NCast, NNeg:
		return isPure(n.LHS)
	case NMember:
		return isPure(n.LHS)
	case NMakeComplex:
		return isPure(n.LHS) && isPure(n.RHS)
	}
	return false
}

func complexPart(e *Node, im bool, tok *Token) *Node {
	addType(e)
	m := e.Ty.Rec.Members[0]
	if im {
		m = e.Ty.Rec.Members[1]
	}
	return &Node{Kind: NMember, LHS: e, Member: m, Ty: m.Ty, Tok: tok}
}

// cparts splits an expression into its real and imaginary parts. pre is
// code that must run first (evaluating the operand once into a temporary).
func (p *parser) cparts(e *Node, tok *Token) (pre, re, im *Node) {
	addType(e)
	if !e.Ty.IsComplex() {
		base := e.Ty
		if !base.IsFlonum() {
			base = tyDouble
		}
		if isPure(e) || p.fn == nil {
			return nil, newCast(e, base), zeroOf(base, tok)
		}
		t := p.newLvar("", e.Ty.Unqual())
		pre = p.newBinary(NAssign, newVarNode(t, tok), e, tok)
		return pre, newCast(newVarNode(t, tok), base), zeroOf(base, tok)
	}
	if e.Kind == NMakeComplex && (isPure(e) || p.fn == nil) {
		return nil, e.LHS, e.RHS
	}
	if e.Kind == NCast && !e.LHS.Ty.IsComplex() && p.fn == nil {
		return nil, newCast(e.LHS, e.Ty.Base), zeroOf(e.Ty.Base, tok)
	}
	if isPure(e) && isLvalue(e) {
		return nil, complexPart(e, false, tok), complexPart(e, true, tok)
	}
	if p.fn == nil {
		if e.Kind == NCast && e.LHS.Ty.IsComplex() {
			_, r, i := p.cparts(e.LHS, tok)
			return nil, newCast(r, e.Ty.Base), newCast(i, e.Ty.Base)
		}
		p.errorAt(tok, "initializer element is not constant")
	}
	t := p.newLvar("", e.Ty.Unqual())
	pre = p.newBinary(NAssign, newVarNode(t, tok), e, tok)
	return pre, complexPart(newVarNode(t, tok), false, tok), complexPart(newVarNode(t, tok), true, tok)
}

func seq(p *parser, tok *Token, pres []*Node, v *Node) *Node {
	for i := len(pres) - 1; i >= 0; i-- {
		if pres[i] != nil {
			v = p.newBinary(NComma, pres[i], v, tok)
		}
	}
	addType(v)
	return v
}

// complexResultType is the complex type for an operation on a and b
func complexResultType(a, b *Type) *Type {
	ra, rb := a, b
	if a.IsComplex() {
		ra = a.Base
	}
	if b.IsComplex() {
		rb = b.Base
	}
	return complexOf(commonType(ra, rb))
}

// complexBinary lowers +, -, *, /, == and != with a complex operand
func (p *parser) complexBinary(op NodeKind, a, b *Node, tok *Token) *Node {
	addType(a)
	addType(b)
	if !(a.Ty.IsNumeric() || a.Ty.IsComplex()) || !(b.Ty.IsNumeric() || b.Ty.IsComplex()) {
		p.errorAt(tok, "invalid operands to binary operator (have '%s' and '%s')", a.Ty, b.Ty)
	}
	ct := complexResultType(a.Ty, b.Ty)
	bt := ct.Base
	pa, ar, ai := p.cparts(a, tok)
	pb, br, bi := p.cparts(b, tok)
	ar, ai, br, bi = newCast(ar, bt), newCast(ai, bt), newCast(br, bt), newCast(bi, bt)
	bin := func(k NodeKind, x, y *Node) *Node {
		n := p.newBinary(k, x, y, tok)
		addType(n)
		return n
	}
	var res *Node
	switch op {
	case NAdd:
		res = makeComplex(ct, bin(NAdd, ar, br), bin(NAdd, ai, bi), tok)
	case NSub:
		res = makeComplex(ct, bin(NSub, ar, br), bin(NSub, ai, bi), tok)
	case NMul:
		switch {
		case !b.Ty.IsComplex():
			res = makeComplex(ct, bin(NMul, ar, br), bin(NMul, ai, br), tok)
		case !a.Ty.IsComplex():
			res = makeComplex(ct, bin(NMul, ar, br), bin(NMul, ar, bi), tok)
		case p.fn == nil:
			res = makeComplex(ct, bin(NSub, bin(NMul, ar, br), bin(NMul, ai, bi)),
				bin(NAdd, bin(NMul, ar, bi), bin(NMul, ai, br)), tok)
		default:
			// libgcc-compatible helper: recovers infinities (C11 Annex G)
			res = p.complexHelper("mul", ct, ar, ai, br, bi, tok)
		}
	case NDiv:
		if !b.Ty.IsComplex() {
			res = makeComplex(ct, bin(NDiv, ar, br), bin(NDiv, ai, br), tok)
			break
		}
		if p.fn == nil {
			p.errorAt(tok, "complex division is not a constant expression")
		}
		res = p.complexHelper("div", ct, ar, ai, br, bi, tok)
	case NEq, NNe:
		eq := bin(NLogAnd, bin(NEq, ar, br), bin(NEq, ai, bi))
		if op == NNe {
			eq = newUnary(NNot, eq, tok)
			addType(eq)
		}
		res = eq
	default:
		p.errorAt(tok, "invalid operands to binary operator (have '%s' and '%s')", a.Ty, b.Ty)
	}
	return seq(p, tok, []*Node{pa, pb}, res)
}

// complexHelper calls a libgcc-compatible helper of the runtime
// (__mulsc3, __divdc3...)
func (p *parser) complexHelper(op string, ct *Type, ar, ai, br, bi *Node, tok *Token) *Node {
	name := "__" + op + "dc3"
	switch ct.Base.Kind {
	case TyFloat:
		name = "__" + op + "sc3"
	case TyLongDouble:
		name = "__" + op + "xc3"
	}
	o := p.globalsByName[name]
	if o == nil {
		ft := funcType(ct)
		ft.Params = []*Type{ct.Base, ct.Base, ct.Base, ct.Base}
		o = &Obj{Name: name, AsmName: name, Ty: ft, Tok: tok, IsFunction: true}
		p.globalsByName[name] = o
		p.prog.Globals = append(p.prog.Globals, o)
	}
	p.markUsed(o)
	fn := newVarNode(o, tok)
	addType(fn)
	call := &Node{Kind: NFuncall, LHS: fn, FuncTy: o.Ty, Tok: tok, Ty: ct, Args: []*Node{ar, ai, br, bi}}
	call.RetBuffer = p.newLvar("", ct)
	return call
}

// complexUnary lowers -z, ~z (conjugate), !z
func (p *parser) complexUnary(k NodeKind, e *Node, tok *Token) *Node {
	pe, r, i := p.cparts(e, tok)
	ct := e.Ty
	var res *Node
	switch k {
	case NNeg:
		res = makeComplex(ct, newUnary(NNeg, r, tok), newUnary(NNeg, i, tok), tok)
	case NBitNot:
		res = makeComplex(ct, r, newUnary(NNeg, i, tok), tok)
	case NNot:
		res = newUnary(NNot, p.complexToBool(nil, r, i, tok), tok)
	}
	return seq(p, tok, []*Node{pe}, res)
}

func (p *parser) complexToBool(pre, r, i *Node, tok *Token) *Node {
	zr := zeroOf(r.Ty, tok)
	zi := zeroOf(i.Ty, tok)
	n := p.newBinary(NLogOr, p.newBinary(NNe, r, zr, tok), p.newBinary(NNe, i, zi, tok), tok)
	return seq(p, tok, []*Node{pre}, n)
}

// realImag implements __real__ and __imag__
func (p *parser) realImag(e *Node, imag bool, tok *Token) *Node {
	addType(e)
	if !e.Ty.IsComplex() {
		if imag {
			return seq(p, tok, nil, p.newBinary(NComma, e, zeroOf(e.Ty.Unqual(), tok), tok))
		}
		return e
	}
	if isLvalue(e) {
		return complexPart(e, imag, tok)
	}
	pe, r, i := p.cparts(e, tok)
	if imag {
		return seq(p, tok, []*Node{pe}, i)
	}
	return seq(p, tok, []*Node{pe}, r)
}

// evalComplex evaluates a constant complex initializer
func (p *parser) evalComplex(e *Node) (re, im *Node) {
	_, r, i := p.cparts(e, e.Tok)
	return r, i
}
