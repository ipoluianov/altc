package cc

import (
	"math"
	"math/big"
	"strings"
)

func (p *parser) expr() *Node {
	n := p.assign()
	for p.is(",") {
		t := p.next()
		n = p.newBinary(NComma, n, p.assign(), t)
	}
	addType(n)
	return n
}

// isLvalue tells whether a node designates an object
func isLvalue(n *Node) bool {
	switch n.Kind {
	case NVar, NDeref, NVLAPtr:
		return true
	case NMember:
		return true
	case NComma:
		// a compound literal: (init, anonymous variable)
		return n.RHS.Kind == NVar && n.RHS.Var.IsLocal && n.RHS.Var.Name == ""
	}
	return false
}

func (p *parser) checkLvalue(n *Node, tok *Token, what string) {
	addType(n)
	if n.Ty.Kind == TyVoid && what != "unary '&' operand" {
		p.errorAt(tok, "invalid use of void expression")
		return
	}
	if !isLvalue(n) {
		p.errorAt(tok, "lvalue required as %s", what)
	}
	if n.Ty.Const && what != "unary '&' operand" {
		name := "read-only location"
		if n.Kind == NVar {
			name = "read-only variable '" + n.Var.Name + "'"
		} else if n.Kind == NMember && n.Member.Name != "" {
			name = "read-only member '" + n.Member.Name + "'"
		}
		p.errorAt(tok, "%s of %s", what, name)
	}
}

// checkAssign reports invalid implicit conversions of assignments
func (p *parser) checkAssign(to *Type, e *Node, tok *Token, ctx string) {
	addType(e)
	from := e.Ty
	switch {
	case from.Kind == TyVoid && to.Kind != TyVoid:
		p.errorAt(tok, "void value not ignored as it ought to be")
	case to.Kind == TyPtr && from.IsInteger() && !isNullPtrConst(e):
		p.warnAt(tok, "%s makes pointer from integer without a cast", ctx)
	case to.IsInteger() && to.Kind != TyBool && decayType(from).Kind == TyPtr:
		p.warnAt(tok, "%s makes integer from pointer without a cast", ctx)
	case to.IsVector() || from.IsVector():
		if !to.IsVector() || !from.IsVector() || to.Size() != from.Size() {
			p.errorAt(tok, "incompatible types in %s ('%s' from '%s')", ctx, to, from)
		}
	case to.IsComplex() && (from.IsNumeric() || from.IsComplex()):
	case from.IsComplex() && to.IsNumeric():
	case to.IsRecord() && !(from.IsRecord() && from.Rec == to.Rec):
		p.errorAt(tok, "incompatible types in %s ('%s' from '%s')", ctx, to, from)
	case to.IsScalar() && from.IsRecord():
		p.errorAt(tok, "incompatible types in %s ('%s' from '%s')", ctx, to, from)
	case to.Kind == TyPtr && decayType(from).Kind == TyPtr:
		fb := decayType(from).Base
		tb := to.Base
		if tb.Kind != TyVoid && fb.Kind != TyVoid && !compatible(tb.Unqual(), fb.Unqual()) {
			// char* vs unsigned char* etc. are common; keep it a warning
			p.warnAt(tok, "incompatible pointer types in %s ('%s' from '%s')", ctx, to, decayType(from))
		}
	}
}

var assignOps = map[string]NodeKind{
	"+=": NAdd, "-=": NSub, "*=": NMul, "/=": NDiv, "%=": NMod, "&=": NBitAnd, "|=": NBitOr, "^=": NBitXor,
	"<<=": NShl, ">>=": NShr,
}

func (p *parser) assign() *Node {
	n := p.conditional()
	t := p.cur()
	if t.Kind != TPunct {
		return n
	}
	if t.Text == "=" {
		p.next()
		p.checkLvalue(n, t, "left operand of assignment")
		rhs := p.assign()
		addType(n)
		p.checkAssign(n.Ty, rhs, t, "assignment")
		return p.newBinary(NAssign, n, rhs, t)
	}
	if op, ok := assignOps[t.Text]; ok {
		p.next()
		p.checkLvalue(n, t, "left operand of assignment")
		rhs := p.assign()
		return p.toAssign(op, n, rhs, t)
	}
	return n
}

// arith builds a binary arithmetic node (with pointer arithmetic for + and -)
func (p *parser) arith(op NodeKind, lhs, rhs *Node, tok *Token) *Node {
	addType(lhs)
	addType(rhs)
	if lhs.Ty.IsVector() || rhs.Ty.IsVector() {
		return p.vectorBinary(op, lhs, rhs, tok)
	}
	switch op {
	case NAdd:
		return p.newAdd(lhs, rhs, tok)
	case NSub:
		return p.newSub(lhs, rhs, tok)
	}
	addType(lhs)
	addType(rhs)
	if lhs.Ty.IsComplex() || rhs.Ty.IsComplex() {
		return p.complexBinary(op, lhs, rhs, tok)
	}
	n := p.newBinary(op, lhs, rhs, tok)
	p.checkArith(n)
	return n
}

func (p *parser) checkArith(n *Node) {
	addType(n.LHS)
	addType(n.RHS)
	l, r := n.LHS.Ty, n.RHS.Ty
	switch n.Kind {
	case NMul, NDiv:
		if !l.IsNumeric() || !r.IsNumeric() {
			p.errorAt(n.Tok, "invalid operands to binary %s (have '%s' and '%s')", opName(n.Kind), l, r)
		}
	case NMod, NBitAnd, NBitOr, NBitXor, NShl, NShr:
		if !l.IsInteger() || !r.IsInteger() {
			p.errorAt(n.Tok, "invalid operands to binary %s (have '%s' and '%s')", opName(n.Kind), l, r)
		}
	}
}

func opName(k NodeKind) string {
	switch k {
	case NMul:
		return "*"
	case NDiv:
		return "/"
	case NMod:
		return "%"
	case NBitAnd:
		return "&"
	case NBitOr:
		return "|"
	case NBitXor:
		return "^"
	case NShl:
		return "<<"
	case NShr:
		return ">>"
	}
	return "?"
}

// toAssign converts "A op= B" into an assignment that evaluates A once
func (p *parser) toAssign(op NodeKind, lhs, rhs *Node, tok *Token) *Node {
	addType(lhs)
	addType(rhs)
	// _Atomic integers: a locked read-modify-write
	if lhs.Ty.Atomic && (lhs.Ty.IsInteger() || lhs.Ty.Kind == TyPtr && (op == NAdd || op == NSub)) &&
		(op == NAdd || op == NSub || op == NBitAnd || op == NBitOr || op == NBitXor) && !(lhs.Kind == NMember && lhs.Member.Bitfield) {
		kind := map[NodeKind]int{NAdd: FetchAdd, NSub: FetchSub, NBitAnd: FetchAnd, NBitOr: FetchOr, NBitXor: FetchXor}[op]
		addr := newUnary(NAddr, lhs, tok)
		addType(addr)
		ty := lhs.Ty.Unqual()
		var val *Node
		if ty.Kind == TyPtr {
			// in units of the pointed-to type (the builtins work on bytes)
			val = newCast(rhs, tyLong)
			if sz := ty.Base.Size(); sz > 1 {
				val = p.newBinary(NMul, val, newCast(newNum(sz, tok), tyLong), tok)
				addType(val)
			}
		} else {
			val = newCast(rhs, ty)
		}
		n := &Node{Kind: NFetchOp, LHS: addr, RHS: val, Val: int64(kind), Begin: 1, Tok: tok, Ty: ty}
		return n
	}
	// a plain variable can be repeated safely
	if lhs.Kind == NVar {
		return p.newBinary(NAssign, lhs, p.arith(op, newVarNode(lhs.Var, tok), rhs, tok), tok)
	}
	if p.fn == nil {
		p.errorAt(tok, "compound assignment outside of a function")
	}
	// a member of a plain variable: v.x = v.x op C (v keeps its address untaken)
	if lhs.Kind == NMember && isPure(lhs) {
		return p.newBinary(NAssign, lhs, p.arith(op, dupMember(lhs, tok), rhs, tok), tok)
	}
	// A.x op= C  ->  tmp = &A, (*tmp).x = (*tmp).x op C  (A.x may be a bit-field)
	if lhs.Kind == NMember {
		v := p.newLvar("", pointerTo(lhs.LHS.Ty))
		e1 := p.newBinary(NAssign, newVarNode(v, tok), newUnary(NAddr, lhs.LHS, tok), tok)
		m1 := &Node{Kind: NMember, LHS: newUnary(NDeref, newVarNode(v, tok), tok), Member: lhs.Member, Tok: tok}
		m2 := &Node{Kind: NMember, LHS: newUnary(NDeref, newVarNode(v, tok), tok), Member: lhs.Member, Tok: tok}
		e2 := p.newBinary(NAssign, m1, p.arith(op, m2, rhs, tok), tok)
		n := p.newBinary(NComma, e1, e2, tok)
		addType(n)
		return n
	}
	// A op= B  ->  tmp = &A, *tmp = *tmp op B
	v := p.newLvar("", pointerTo(lhs.Ty))
	e1 := p.newBinary(NAssign, newVarNode(v, tok), newUnary(NAddr, lhs, tok), tok)
	e2 := p.newBinary(NAssign, newUnary(NDeref, newVarNode(v, tok), tok),
		p.arith(op, newUnary(NDeref, newVarNode(v, tok), tok), rhs, tok), tok)
	n := p.newBinary(NComma, e1, e2, tok)
	addType(n)
	return n
}

func (p *parser) conditional() *Node {
	c := p.logor()
	if !p.is("?") {
		return c
	}
	t := p.next()
	if p.is(":") {
		// GNU a ?: b
		p.next()
		addType(c)
		if p.fn == nil {
			rhs := p.conditional()
			return &Node{Kind: NCond, Cond: c, Then: c, Els: rhs, Tok: t}
		}
		v := p.newLvar("", decayType(c.Ty).Unqual())
		e1 := p.newBinary(NAssign, newVarNode(v, t), c, t)
		rhs := p.conditional()
		cond := newVarNode(v, t)
		if v.Ty.IsComplex() {
			cond = p.toBool(cond)
		}
		n := &Node{Kind: NCond, Cond: cond, Then: newVarNode(v, t), Els: rhs, Tok: t}
		return p.newBinary(NComma, e1, n, t)
	}
	n := &Node{Kind: NCond, Cond: c, Tok: t}
	n.Then = p.nullPtrConst(p.expr())
	p.expect(":")
	n.Els = p.nullPtrConst(p.conditional())
	addType(n.Cond)
	if n.Cond.Ty.IsComplex() {
		n.Cond = p.toBool(n.Cond)
	}
	if !decayType(n.Cond.Ty).IsScalar() {
		p.errorAt(t, "used non-scalar value where scalar is required")
	}
	return n
}

// nullPtrConst folds an operand of ?: that is a null pointer constant
// other than a plain 0, (void *)(integer constant expression of value 0),
// to (void *)0: the other operand then gives the type of the result
// (<tgmath.h> selects types this way: 0 ? (T *)0 : (void *)!(c))
func (p *parser) nullPtrConst(n *Node) *Node {
	addType(n)
	inner := n
	for inner.Kind == NCast && inner.Ty.Kind == TyPtr && inner.Ty.Base.Kind == TyVoid && !inner.Ty.Base.Const && !inner.Ty.Base.Volatile {
		inner = inner.LHS
	}
	if inner == n || inner.Kind == NNum || !inner.Ty.IsInteger() || !p.isConstExpr(inner) || p.eval(inner) != 0 {
		return n
	}
	return newCast(&Node{Kind: NNum, Ty: tyInt, Tok: inner.Tok}, n.Ty)
}

// toBool checks a controlling expression
func (p *parser) toBool(n *Node) *Node {
	addType(n)
	if n.Ty.IsComplex() {
		pre, r, i := p.cparts(n, n.Tok)
		return p.complexToBool(pre, r, i, n.Tok)
	}
	if !decayType(n.Ty).IsScalar() {
		p.errorAt(n.Tok, "used %s value where scalar is required", n.Ty)
	}
	return n
}

func (p *parser) logor() *Node {
	n := p.logand()
	for p.is("||") {
		t := p.next()
		n = p.newBinary(NLogOr, p.toBool(n), p.toBool(p.logand()), t)
	}
	return n
}

func (p *parser) logand() *Node {
	n := p.bitor()
	for p.is("&&") {
		t := p.next()
		n = p.newBinary(NLogAnd, p.toBool(n), p.toBool(p.bitor()), t)
	}
	return n
}

func (p *parser) bitor() *Node {
	n := p.bitxor()
	for p.is("|") {
		t := p.next()
		n = p.arith(NBitOr, n, p.bitxor(), t)
	}
	return n
}

func (p *parser) bitxor() *Node {
	n := p.bitand()
	for p.is("^") {
		t := p.next()
		n = p.arith(NBitXor, n, p.bitand(), t)
	}
	return n
}

func (p *parser) bitand() *Node {
	n := p.equality()
	for p.is("&") {
		t := p.next()
		n = p.arith(NBitAnd, n, p.equality(), t)
	}
	return n
}

func (p *parser) compare(k NodeKind, lhs, rhs *Node, tok *Token) *Node {
	addType(lhs)
	addType(rhs)
	if lhs.Ty.IsVector() || rhs.Ty.IsVector() {
		return p.vectorBinary(k, lhs, rhs, tok)
	}
	if (lhs.Ty.IsComplex() || rhs.Ty.IsComplex()) && (k == NEq || k == NNe) {
		return p.complexBinary(k, lhs, rhs, tok)
	}
	l, r := decayType(lhs.Ty), decayType(rhs.Ty)
	if !(l.IsScalar() && r.IsScalar()) {
		p.errorAt(tok, "invalid operands to comparison (have '%s' and '%s')", lhs.Ty, rhs.Ty)
	}
	if l.Kind == TyPtr && r.IsInteger() && !isNullPtrConst(rhs) || r.Kind == TyPtr && l.IsInteger() && !isNullPtrConst(lhs) {
		p.warnAt(tok, "comparison between pointer and integer")
	}
	return p.newBinary(k, lhs, rhs, tok)
}

func (p *parser) equality() *Node {
	n := p.relational()
	for {
		t := p.cur()
		switch {
		case p.is("=="):
			p.next()
			n = p.compare(NEq, n, p.relational(), t)
		case p.is("!="):
			p.next()
			n = p.compare(NNe, n, p.relational(), t)
		default:
			return n
		}
	}
}

func (p *parser) relational() *Node {
	n := p.shift()
	for {
		t := p.cur()
		switch {
		case p.is("<"):
			p.next()
			n = p.compare(NLt, n, p.shift(), t)
		case p.is("<="):
			p.next()
			n = p.compare(NLe, n, p.shift(), t)
		case p.is(">"):
			p.next()
			n = p.compare(NLt, p.shift(), n, t)
		case p.is(">="):
			p.next()
			n = p.compare(NLe, p.shift(), n, t)
		default:
			return n
		}
	}
}

func (p *parser) shift() *Node {
	n := p.add()
	for {
		t := p.cur()
		switch {
		case p.is("<<"):
			p.next()
			n = p.arith(NShl, n, p.add(), t)
		case p.is(">>"):
			p.next()
			n = p.arith(NShr, n, p.add(), t)
		default:
			return n
		}
	}
}

// elemSize returns the size of the pointed-to type as a node (VLA aware)
func (p *parser) elemSize(ptrTy *Type, tok *Token) *Node {
	base := ptrTy.Base
	if base.Kind == TyVLA {
		return newVarNode(base.VLASize, tok)
	}
	sz := base.Size()
	if base.Kind == TyVoid || base.Kind == TyFunc {
		sz = 1 // GNU: arithmetic on void* and function pointers
	}
	if sz < 0 {
		p.errorAt(tok, "arithmetic on pointer to an incomplete type")
	}
	return newLong(sz, tok)
}

func (p *parser) newAdd(lhs, rhs *Node, tok *Token) *Node {
	addType(lhs)
	addType(rhs)
	if lhs.Ty.IsVector() || rhs.Ty.IsVector() {
		return p.vectorBinary(NAdd, lhs, rhs, tok)
	}
	if lhs.Ty.IsComplex() || rhs.Ty.IsComplex() {
		return p.complexBinary(NAdd, lhs, rhs, tok)
	}
	lt, rt := decayType(lhs.Ty), decayType(rhs.Ty)
	if lt.IsNumeric() && rt.IsNumeric() {
		// like gcc, even without optimization: a + -b is a - b and -a + b
		// is b - a (which gives NaNs the sign gcc's results have)
		if isFloatNeg(rhs) && lt.IsFlonum() {
			return p.newBinary(NSub, lhs, rhs.LHS, tok)
		}
		if isFloatNeg(lhs) && rt.IsFlonum() {
			return p.newBinary(NSub, rhs, lhs.LHS, tok)
		}
		return p.newBinary(NAdd, lhs, rhs, tok)
	}
	if lt.Kind == TyPtr && rt.Kind == TyPtr {
		p.errorAt(tok, "invalid operands to binary + (have '%s' and '%s')", lhs.Ty, rhs.Ty)
	}
	if lt.Kind != TyPtr {
		lhs, rhs = rhs, lhs
		lt, rt = rt, lt
	}
	if lt.Kind != TyPtr || !rt.IsInteger() {
		p.errorAt(tok, "invalid operands to binary + (have '%s' and '%s')", lhs.Ty, rhs.Ty)
	}
	rhs = p.newBinary(NMul, newCast(rhs, tyLong), p.elemSize(lt, tok), tok)
	n := p.newBinary(NAdd, newCast(lhs, lt), rhs, tok)
	n.Ty = lt
	addType(rhs)
	return n
}

func (p *parser) newSub(lhs, rhs *Node, tok *Token) *Node {
	addType(lhs)
	addType(rhs)
	if lhs.Ty.IsVector() || rhs.Ty.IsVector() {
		return p.vectorBinary(NSub, lhs, rhs, tok)
	}
	if lhs.Ty.IsComplex() || rhs.Ty.IsComplex() {
		return p.complexBinary(NSub, lhs, rhs, tok)
	}
	lt, rt := decayType(lhs.Ty), decayType(rhs.Ty)
	if lt.IsNumeric() && rt.IsNumeric() {
		// like gcc: a - -b is a + b
		if isFloatNeg(rhs) && lt.IsFlonum() {
			return p.newBinary(NAdd, lhs, rhs.LHS, tok)
		}
		return p.newBinary(NSub, lhs, rhs, tok)
	}
	if lt.Kind == TyPtr && rt.IsInteger() {
		rhs = p.newBinary(NMul, newCast(rhs, tyLong), p.elemSize(lt, tok), tok)
		addType(rhs)
		n := p.newBinary(NSub, newCast(lhs, lt), rhs, tok)
		n.Ty = lt
		return n
	}
	if lt.Kind == TyPtr && rt.Kind == TyPtr {
		if !compatible(lt.Base.Unqual(), rt.Base.Unqual()) {
			p.warnAt(tok, "subtraction of pointers to different types")
		}
		n := p.newBinary(NSub, newCast(lhs, tyLong), newCast(rhs, tyLong), tok)
		n.Ty = tyLong
		d := p.newBinary(NDiv, n, p.elemSize(lt, tok), tok)
		addType(d)
		return d
	}
	p.errorAt(tok, "invalid operands to binary - (have '%s' and '%s')", lhs.Ty, rhs.Ty)
	return nil
}

func (p *parser) add() *Node {
	n := p.mul()
	for {
		t := p.cur()
		switch {
		case p.is("+"):
			p.next()
			n = p.newAdd(n, p.mul(), t)
		case p.is("-"):
			p.next()
			n = p.newSub(n, p.mul(), t)
		default:
			return n
		}
	}
}

func (p *parser) mul() *Node {
	n := p.cast()
	for {
		t := p.cur()
		switch {
		case p.is("*"):
			p.next()
			n = p.arith(NMul, n, p.cast(), t)
		case p.is("/"):
			p.next()
			n = p.arith(NDiv, n, p.cast(), t)
		case p.is("%"):
			p.next()
			n = p.arith(NMod, n, p.cast(), t)
		default:
			return n
		}
	}
}

func (p *parser) cast() *Node {
	if p.is("(") && p.typenameAt(1) {
		start := p.pos
		t := p.next()
		ty := p.typename()
		p.expect(")")
		if p.is("{") {
			// compound literal
			p.pos = start
			return p.unary()
		}
		e := p.cast()
		addType(e)
		if ty.Kind == TyUnion && !(e.Ty.Kind == TyUnion && e.Ty.Rec == ty.Rec) {
			// GNU cast to a union: initialize the member of the operand's type
			for _, m := range ty.Rec.Members {
				if compatible(m.Ty.Unqual(), e.Ty.Unqual()) {
					if p.fn == nil {
						p.errorAt(t, "cast to union at file scope is not supported")
					}
					v := p.newLvar("", ty)
					as := p.newBinary(NAssign, &Node{Kind: NMember, LHS: newVarNode(v, t), Member: m, Tok: t}, e, t)
					return p.newBinary(NComma, p.newBinary(NComma, &Node{Kind: NMemzero, Var: v, Tok: t}, as, t), newVarNode(v, t), t)
				}
			}
			p.errorAt(t, "cast to union type from type not present in union")
		}
		if ty.Kind == TyUnion || ty.Kind == TyStruct && e.Ty.IsRecord() && e.Ty.Rec == ty.Rec {
			return e
		}
		if ty.IsVector() || e.Ty.IsVector() {
			if ty.Size() != e.Ty.Size() {
				p.errorAt(t, "cannot convert a value of type '%s' to '%s' (sizes differ)", e.Ty, ty)
			}
			if !ty.IsVector() || !e.Ty.IsVector() {
				p.errorAt(t, "conversion between a vector and a scalar is not supported")
			}
			return &Node{Kind: NCast, LHS: e, Ty: ty.Unqual(), Tok: t}
		}
		if ty.Kind != TyVoid && !ty.IsScalar() && !ty.IsComplex() {
			p.errorAt(t, "conversion to non-scalar type requested")
		}
		if ty.Kind != TyVoid && e.Ty.IsRecord() && !e.Ty.IsComplex() {
			p.errorAt(t, "conversion of a struct to '%s' requested", ty)
		}
		n := &Node{Kind: NCast, LHS: e, Ty: ty.Unqual(), Tok: t}
		if ty.Kind == TyVLA {
			n.Ty = pointerTo(ty.Base)
		}
		return n
	}
	return p.unary()
}

func (p *parser) unary() *Node {
	t := p.cur()
	if t.Kind == TPunct {
		switch t.Text {
		case "+":
			p.next()
			e := p.cast()
			addType(e)
			if e.Ty.IsComplex() || e.Ty.IsVector() {
				return e
			}
			if !e.Ty.IsNumeric() {
				p.errorAt(t, "wrong type argument to unary plus")
			}
			return newCast(e, intPromote(operandType(e)))
		case "-":
			p.next()
			e := p.cast()
			addType(e)
			if e.Ty.IsComplex() {
				return p.complexUnary(NNeg, e, t)
			}
			if e.Ty.IsVector() {
				return &Node{Kind: NNeg, LHS: e, Ty: e.Ty.Unqual(), Tok: t}
			}
			if !e.Ty.IsNumeric() {
				p.errorAt(t, "wrong type argument to unary minus")
			}
			return newUnary(NNeg, e, t)
		case "&":
			p.next()
			e := p.cast()
			addType(e)
			if e.Kind == NMember && e.Member.Bitfield {
				p.errorAt(t, "cannot take address of bit-field '%s'", e.Member.Name)
			}
			if e.Ty.Kind == TyFunc || e.Kind == NFuncall && e.Ty.IsRecord() {
				// &function and &f() (returning a struct) are fine
			} else {
				p.checkLvalue(e, t, "unary '&' operand")
			}
			if e.Kind == NVar && e.Var.IsLocal && e.Var.Name != "" && false {
				return nil
			}
			return newUnary(NAddr, e, t)
		case "*":
			p.next()
			e := p.cast()
			addType(e)
			if e.Ty.Kind == TyFunc {
				return e // *func is func
			}
			if decayType(e.Ty).Kind != TyPtr {
				p.errorAt(t, "invalid type argument of unary '*' (have '%s')", e.Ty)
			}
			return newUnary(NDeref, e, t)
		case "!":
			p.next()
			return newUnary(NNot, p.toBool(p.cast()), t)
		case "~":
			p.next()
			e := p.cast()
			addType(e)
			if e.Ty.IsComplex() {
				return p.complexUnary(NBitNot, e, t)
			}
			if e.Ty.IsVector() {
				return &Node{Kind: NBitNot, LHS: e, Ty: e.Ty.Unqual(), Tok: t}
			}
			if !e.Ty.IsInteger() {
				p.errorAt(t, "wrong type argument to bit-complement")
			}
			return newUnary(NBitNot, e, t)
		case "++", "--":
			p.next()
			e := p.unary()
			p.checkLvalue(e, t, "increment operand")
			op := NAdd
			if t.Text == "--" {
				op = NSub
			}
			return p.toAssign(op, e, newNum(1, t), t)
		case "&&":
			// GNU labels as values
			p.next()
			name := p.ident()
			n := &Node{Kind: NLabelVal, Label: name.Text, Tok: name}
			p.gotos = append(p.gotos, n)
			return n
		}
	}
	if t.Kind == TIdent && !t.notKw {
		switch t.Text {
		case "sizeof":
			p.next()
			return p.sizeofExpr(t)
		case "_Alignof", "alignof", "__alignof__", "__alignof":
			p.next()
			if p.is("(") && p.typenameAt(1) {
				p.next()
				ty := p.typename()
				p.expect(")")
				return newULong(ty.Align(), t)
			}
			e := p.unary()
			addType(e)
			if e.Kind == NVar {
				return newULong(e.Var.Align, t)
			}
			return newULong(e.Ty.Align(), t)
		case "__extension__":
			p.next()
			return p.cast()
		case "__real__", "__real":
			p.next()
			return p.realImag(p.cast(), false, t)
		case "__imag__", "__imag":
			p.next()
			return p.realImag(p.cast(), true, t)
		}
	}
	return p.postfix()
}

func (p *parser) sizeofExpr(t *Token) *Node {
	if p.is("(") && p.typenameAt(1) {
		start := p.pos
		p.next()
		ty := p.typename()
		p.expect(")")
		if p.is("{") {
			p.pos = start
		} else {
			return p.sizeofType(ty, t)
		}
	}
	p.inSizeof++
	e := p.unary()
	p.inSizeof--
	addType(e)
	if e.Kind == NMember && e.Member.Bitfield {
		p.errorAt(t, "'sizeof' applied to a bit-field")
	}
	if e.Ty.Kind == TyVLA {
		if e.Ty.VLASize == nil {
			p.errorAt(t, "sizeof of an unsized VLA")
		}
		return newVarNode(e.Ty.VLASize, t)
	}
	return p.sizeofType(e.Ty, t)
}

func (p *parser) sizeofType(ty *Type, t *Token) *Node {
	if ty.Kind == TyVLA {
		if ty.VLASize != nil {
			return newVarNode(ty.VLASize, t)
		}
		if p.fn == nil {
			p.errorAt(t, "sizeof of a variable length array at file scope")
		}
		sz := p.computeVLASize(ty, t)
		return p.newBinary(NComma, sz, newVarNode(ty.VLASize, t), t)
	}
	if ty.Kind == TyFunc || ty.Kind == TyVoid {
		return newULong(1, t) // GNU extension
	}
	sz := ty.Size()
	if sz < 0 {
		p.errorAt(t, "invalid application of 'sizeof' to incomplete type '%s'", ty)
	}
	return newULong(sz, t)
}

// structRef builds a member access, descending through anonymous members
func (p *parser) structRef(n *Node, name *Token) *Node {
	addType(n)
	ty := n.Ty
	if !ty.IsRecord() {
		p.errorAt(name, "request for member '%s' in something not a structure or union", name.Text)
	}
	if !ty.Rec.Complete {
		p.errorAt(name, "invalid use of incomplete type '%s'", ty)
	}
	path := findMember(ty.Rec, name.Text)
	if path == nil {
		p.errorAt(name, "'%s' has no member named '%s'", ty, name.Text)
	}
	for _, m := range path {
		mt := m.Ty
		if ty.Const || ty.Volatile {
			mt = mt.withQuals(ty.Const, ty.Volatile, false, false)
		}
		n = &Node{Kind: NMember, LHS: n, Member: m, Tok: name, Ty: mt}
		ty = mt
	}
	return n
}

func (p *parser) postfix() *Node {
	t := p.cur()
	var n *Node
	if p.is("(") && p.typenameAt(1) {
		// compound literal
		p.next()
		ty := p.typename()
		p.expect(")")
		if !p.is("{") {
			p.errorAt(p.cur(), "expected '{'")
		}
		if p.fn == nil || p.isGlobalScope() && p.fn == nil {
			o := p.newAnonGvar(ty)
			o.IsLiteral = false
			p.gvarInitializer(o)
			n = newVarNode(o, t)
		} else {
			v := p.newLvar("", ty)
			init := p.lvarInitializer(v)
			n = p.newBinary(NComma, init, newVarNode(v, t), t)
		}
	} else {
		n = p.primary()
	}
	for {
		t := p.cur()
		switch {
		case p.is("("):
			p.next()
			n = p.funcall(n, t)
		case p.is("["):
			p.next()
			idx := p.expr()
			p.expect("]")
			addType(n)
			addType(idx)
			if n.Ty.IsVector() {
				n = p.vectorIndex(n, idx, t)
				continue
			}
			if decayType(n.Ty).Kind != TyPtr && decayType(idx.Ty).Kind != TyPtr {
				p.errorAt(t, "subscripted value is neither array nor pointer")
			}
			n = newUnary(NDeref, p.newAdd(n, idx, t), t)
		case p.is("."):
			p.next()
			n = p.structRef(n, p.ident())
		case p.is("->"):
			p.next()
			addType(n)
			if dt := decayType(n.Ty); dt.Kind != TyPtr || !dt.Base.IsRecord() {
				p.errorAt(t, "invalid type argument of '->' (have '%s')", n.Ty)
			}
			n = p.structRef(newUnary(NDeref, n, t), p.ident())
		case p.is("++"), p.is("--"):
			p.next()
			p.checkLvalue(n, t, "increment operand")
			n = p.postIncDec(n, t, t.Text == "++")
		default:
			return n
		}
	}
}

// postIncDec converts a++ into (typeof a)((a += 1) - 1)
func (p *parser) postIncDec(n *Node, t *Token, inc bool) *Node {
	addType(n)
	op, back := NAdd, NSub
	if !inc {
		op, back = NSub, NAdd
	}
	if n.Ty.Kind == TyBool {
		// bool: save the old value
		v := p.newLvar("", tyBool)
		ptr := p.newLvar("", pointerTo(n.Ty))
		e1 := p.newBinary(NAssign, newVarNode(ptr, t), newUnary(NAddr, n, t), t)
		e2 := p.newBinary(NAssign, newVarNode(v, t), newUnary(NDeref, newVarNode(ptr, t), t), t)
		e3 := p.newBinary(NAssign, newUnary(NDeref, newVarNode(ptr, t), t),
			p.arith(op, newUnary(NDeref, newVarNode(ptr, t), t), newNum(1, t), t), t)
		r := p.newBinary(NComma, p.newBinary(NComma, e1, e2, t), p.newBinary(NComma, e3, newVarNode(v, t), t), t)
		addType(r)
		return r
	}
	a := p.toAssign(op, n, newNum(1, t), t)
	return newCast(p.arith(back, a, newNum(1, t), t), n.Ty.Unqual())
}

func (p *parser) funcall(fn *Node, tok *Token) *Node {
	addType(fn)
	ft := fn.Ty
	if ft.Kind == TyPtr {
		ft = ft.Base
	}
	if ft.Kind != TyFunc {
		p.errorAt(fn.Tok, "called object is not a function or function pointer")
	}
	n := &Node{Kind: NFuncall, LHS: fn, FuncTy: ft, Tok: tok, Ty: ft.Ret.Unqual()}
	i := 0
	for !p.is(")") {
		if i > 0 {
			p.expect(",")
		}
		arg := p.assign()
		addType(arg)
		if i < len(ft.Params) && ft.Params[i] != nil {
			pt := ft.Params[i]
			if !pt.IsRecord() || pt.IsComplex() {
				p.checkAssign(pt, arg, arg.Tok, "passing argument")
				arg = newCast(arg, pt.Unqual())
			} else if !arg.Ty.IsRecord() || arg.Ty.Rec != pt.Rec {
				p.errorAt(arg.Tok, "incompatible type for argument %d", i+1)
			}
		} else {
			if !ft.Variadic && !ft.OldStyle && len(ft.Params) > 0 || !ft.Variadic && !ft.OldStyle && len(ft.Params) == 0 {
				p.errorAt(arg.Tok, "too many arguments to function")
			}
			// default argument promotions
			switch {
			case arg.Ty.Kind == TyFloat:
				arg = newCast(arg, tyDouble)
			case arg.Ty.IsInteger():
				arg = newCast(arg, intPromote(operandType(arg)))
			case arg.Ty.IsArray() || arg.Ty.Kind == TyFunc:
				arg = newCast(arg, decayType(arg.Ty))
			}
		}
		n.Args = append(n.Args, arg)
		i++
	}
	if i < len(ft.Params) && !ft.OldStyle {
		p.errorAt(tok, "too few arguments to function")
	}
	p.expect(")")
	if b := p.complexLibBuiltin(n); b != nil {
		return b
	}
	if ft.Ret.IsRecord() {
		if !ft.Ret.IsComplete() {
			p.errorAt(tok, "calling a function with incomplete return type")
		}
		if p.fn != nil {
			n.RetBuffer = p.newLvar("", ft.Ret)
		}
	}
	return n
}

// ---- primary ----

func (p *parser) primary() *Node {
	t := p.cur()
	switch t.Kind {
	case TPunct:
		if t.Text == "(" && p.peekAt(1).Is("{") {
			// GNU statement expression
			p.next()
			if p.fn == nil {
				p.errorAt(t, "statement expression outside of a function")
			}
			blk := p.compoundStmt()
			p.expect(")")
			n := &Node{Kind: NStmtExpr, Body: blk.Body, Tok: t}
			addType(n)
			return n
		}
		if t.Text == "(" {
			p.next()
			n := p.expr()
			p.expect(")")
			return n
		}
	case TNum:
		p.next()
		return p.numberNode(t)
	case TChar:
		p.next()
		v, prefix, err := charValue(t.Text)
		if err != nil {
			p.errorAt(t, "%v", err)
		}
		n := &Node{Kind: NNum, Val: v, Tok: t, Ty: tyInt}
		switch prefix {
		case "u8":
			if p.opt.Std >= 2023 {
				n.Ty = tyUChar
			} else {
				n.Ty = tyChar
			}
		case "u":
			n.Ty = tyUShort
		case "U":
			n.Ty = tyUInt
		}
		return n
	case TStr:
		return p.stringLiteral()
	case TIdent:
		return p.identExpr()
	case TEOF:
		p.errorAt(t, "expected expression at end of input")
	}
	p.errorAt(t, "expected expression before '%s'", t.Text)
	return nil
}

func (p *parser) numberNode(t *Token) *Node {
	nl, err := parseNumber(t.Text)
	if err != nil {
		p.errorAt(t, "%v", err)
	}
	if nl.imag {
		nl.imag = false
		v := p.numNode(nl, t)
		base := v.Ty
		if !base.IsFlonum() {
			v = &Node{Kind: NNum, FVal: float64(v.Val), Ty: tyDouble, Tok: t}
			base = tyDouble
		}
		ct := complexOf(base)
		return makeComplex(ct, zeroOf(base, t), v, t)
	}
	return p.numNode(nl, t)
}

func (p *parser) numNode(nl numLit, t *Token) *Node {
	if nl.isFloat {
		n := &Node{Kind: NNum, FVal: nl.f, Tok: t, Ty: tyDouble}
		switch nl.fsuffix {
		case 'f':
			n.Ty = tyFloat
			n.FVal = float64(float32(nl.f))
		case 'l':
			n.Ty = tyLDouble
			n.LD = nl.ld
		}
		return n
	}
	if nl.overflow {
		p.errorAt(t, "integer constant is too large for its type")
	}
	v := nl.i
	var ty *Type
	switch {
	case nl.sizeT:
		ty = tyULong
		if !nl.unsigned {
			ty = tyLong
		}
	case nl.unsigned:
		switch {
		case nl.longs == 0 && v <= math.MaxUint32:
			ty = tyUInt
		case nl.longs == 2:
			ty = tyULLong
		default:
			ty = tyULong
		}
	case nl.decimal:
		switch {
		case nl.longs == 0 && v <= math.MaxInt32:
			ty = tyInt
		case nl.longs <= 1 && v <= math.MaxInt64:
			ty = tyLong
		case nl.longs == 2 && v <= math.MaxInt64:
			ty = tyLLong
		default:
			p.warnAt(t, "integer constant is so large that it is unsigned")
			ty = tyULong
		}
	default:
		switch {
		case nl.longs == 0 && v <= math.MaxInt32:
			ty = tyInt
		case nl.longs == 0 && v <= math.MaxUint32:
			ty = tyUInt
		case nl.longs <= 1 && v <= math.MaxInt64:
			ty = tyLong
		case nl.longs <= 1:
			ty = tyULong
		case v <= math.MaxInt64:
			ty = tyLLong
		default:
			ty = tyULLong
		}
	}
	return &Node{Kind: NNum, Val: int64(v), Tok: t, Ty: ty}
}

func (p *parser) stringLiteral() *Node {
	t := p.cur()
	prefix := ""
	var parts []string
	for p.cur().Kind == TStr {
		x := p.next()
		pf, body := literalPrefix(x.Text)
		if pf != "" {
			if prefix != "" && prefix != pf && !(prefix == "u8" || pf == "u8") {
				p.errorAt(x, "unsupported non-standard concatenation of string literals")
			}
			if prefix == "" || prefix == "u8" {
				prefix = pf
			}
		}
		parts = append(parts, body)
	}
	var elemTy *Type
	wide, utf16 := false, false
	switch prefix {
	case "", "u8":
		elemTy = tyChar
		if prefix == "u8" && p.opt.Std >= 2023 {
			elemTy = tyUChar
		}
	case "L":
		elemTy, wide = tyInt, true
	case "u":
		elemTy, wide, utf16 = tyUShort, true, true
	case "U":
		elemTy, wide = tyUInt, true
	}
	var vals []uint32
	for _, part := range parts {
		v, err := decodeLiteral(part, wide, utf16)
		if err != nil {
			p.errorAt(t, "%v", err)
		}
		vals = append(vals, v...)
	}
	vals = append(vals, 0)
	sz := elemTy.Size()
	data := make([]byte, int64(len(vals))*sz)
	for i, v := range vals {
		for b := int64(0); b < sz; b++ {
			data[int64(i)*sz+b] = byte(v >> (8 * b))
		}
	}
	ty := arrayOf(elemTy, int64(len(vals)))
	key := prefix + "\x00" + string(data)
	o, ok := p.strs[key]
	if !ok {
		o = p.newAnonGvar(ty)
		o.InitData = data
		o.IsConst = true
		p.strs[key] = o
	}
	return newVarNode(o, t)
}

// funcNameVar returns the static array for __func__
func (p *parser) funcNameVar(t *Token) *Node {
	if p.fn == nil {
		p.warnAt(t, "'%s' is not defined outside of function scope", t.Text)
		return p.makeString("", t)
	}
	return p.makeString(p.fn.Name, t)
}

func (p *parser) makeString(s string, t *Token) *Node {
	data := append([]byte(s), 0)
	key := "\x00" + string(data)
	o, ok := p.strs[key]
	if !ok {
		o = p.newAnonGvar(arrayOf(tyChar, int64(len(data))))
		o.InitData = data
		o.IsConst = true
		p.strs[key] = o
	}
	return newVarNode(o, t)
}

func (p *parser) identExpr() *Node {
	t := p.cur()
	switch t.Text {
	case "true", "false":
		if p.opt.Std >= 2023 || p.findVar(t.Text) == nil {
			if p.findVar(t.Text) == nil {
				p.next()
				v := int64(0)
				if t.Text == "true" {
					v = 1
				}
				ty := tyInt
				if p.opt.Std >= 2023 {
					ty = tyBool
				}
				return &Node{Kind: NNum, Val: v, Tok: t, Ty: ty}
			}
		}
	case "nullptr":
		if p.findVar(t.Text) == nil {
			p.next()
			return &Node{Kind: NNum, Val: 0, Tok: t, Ty: tyNullptr}
		}
	case "__func__", "__FUNCTION__", "__PRETTY_FUNCTION__":
		if p.findVar(t.Text) == nil {
			p.next()
			return p.funcNameVar(t)
		}
	case "_Generic":
		p.next()
		return p.genericSelection(t)
	}
	if n := p.builtin(t); n != nil {
		return n
	}
	if t.kw() {
		p.errorAt(t, "expected expression before '%s'", t.Text)
	}
	p.next()
	v := p.findVar(t.Text)
	if v != nil {
		if v.obj != nil {
			o := v.obj
			if o.IsFunction || !o.IsLocal {
				p.markUsed(o)
			}
			if o.ConstExpr != nil && p.inSizeof == 0 {
				// constexpr object: keep the variable (it is an lvalue) but remember the value
			}
			return newVarNode(o, t)
		}
		if v.enumTy != nil {
			return &Node{Kind: NNum, Val: v.enumVal, Tok: t, Ty: enumConstType(v)}
		}
		p.errorAt(t, "unexpected type name '%s'", t.Text)
	}
	if p.is("(") {
		// implicit function declaration (C89)
		if p.opt.Std >= 1999 {
			p.warnAt(t, "implicit declaration of function '%s'", t.Text)
		}
		ft := funcType(tyInt)
		ft.OldStyle = true
		o := p.declareGlobal(t, ft, &declAttr{})
		p.markUsed(o)
		return newVarNode(o, t)
	}
	p.errorAt(t, "'%s' undeclared", t.Text)
	return nil
}

func (p *parser) markUsed(o *Obj) {
	if p.fn != nil {
		if p.fn != o {
			p.fn.Refs = append(p.fn.Refs, o)
		}
	} else {
		o.IsUsed = true
	}
}

func (p *parser) genericSelection(t *Token) *Node {
	p.expect("(")
	var ctrlTy *Type
	if p.isTypename(p.cur()) {
		ctrlTy = p.typename() // C2y: a type name as the controlling operand
	} else {
		p.inSizeof++
		c := p.assign()
		p.inSizeof--
		addType(c)
		ctrlTy = decayType(c.Ty).Unqual()
		if c.Kind == NMember && c.Member.Bitfield {
			ctrlTy = c.Ty.Unqual()
		}
	}
	var result, def *Node
	for p.consume(",") {
		if p.consume("default") {
			p.expect(":")
			e := p.assign()
			if def != nil {
				p.errorAt(e.Tok, "duplicate default generic association")
			}
			def = e
			continue
		}
		ty := p.typename()
		p.expect(":")
		e := p.assign()
		if result == nil && compatible(ctrlTy, ty.Unqual()) {
			result = e
		}
	}
	p.expect(")")
	if result == nil {
		result = def
	}
	if result == nil {
		p.errorAt(t, "'_Generic' selector of type '%s' is not compatible with any association", ctrlTy)
	}
	return result
}

// ---- floating constants helpers ----

func ldFromFloat(f float64) *big.Float {
	if math.IsInf(f, 0) || math.IsNaN(f) {
		return nil
	}
	return new(big.Float).SetPrec(64).SetFloat64(f)
}

var _ = strings.TrimSpace

// isFloatNeg tells whether n negates a real floating-point value
func isFloatNeg(n *Node) bool {
	return n.Kind == NNeg && n.Ty.IsFlonum() && n.LHS.Ty != nil && n.LHS.Ty.IsFlonum()
}
