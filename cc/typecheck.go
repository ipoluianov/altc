package cc

// ---- node constructors ----

func newNode(k NodeKind, tok *Token) *Node { return &Node{Kind: k, Tok: tok} }

func newNum(v int64, tok *Token) *Node { return &Node{Kind: NNum, Val: v, Tok: tok, Ty: tyInt} }

func newLong(v int64, tok *Token) *Node { return &Node{Kind: NNum, Val: v, Tok: tok, Ty: tyLong} }

func newULong(v int64, tok *Token) *Node { return &Node{Kind: NNum, Val: v, Tok: tok, Ty: tyULong} }

func newVarNode(o *Obj, tok *Token) *Node {
	return &Node{Kind: NVar, Var: o, Tok: tok}
}

func newUnary(k NodeKind, lhs *Node, tok *Token) *Node {
	return &Node{Kind: k, LHS: lhs, Tok: tok}
}

// newCast converts an expression to a type
func newCast(e *Node, ty *Type) *Node {
	addType(e)
	if e.Ty == ty {
		return e
	}
	return &Node{Kind: NCast, Tok: e.Tok, LHS: e, Ty: ty}
}

func (p *parser) newBinary(k NodeKind, lhs, rhs *Node, tok *Token) *Node {
	n := &Node{Kind: k, LHS: lhs, RHS: rhs, Tok: tok}
	return n
}

// decayType returns the type an expression of type t has after array and
// function decay
func decayType(t *Type) *Type {
	switch t.Kind {
	case TyArray, TyVLA:
		return pointerTo(t.Base)
	case TyFunc:
		return pointerTo(t)
	}
	return t
}

// intPromote applies the integer promotions to a type
func intPromote(t *Type) *Type {
	if t.Kind == TyEnum {
		t = t.Enum.underlying()
	}
	if t.IsInteger() && intRank(t) < intRank(tyInt) {
		return tyInt
	}
	return t.Unqual()
}

// commonType implements the usual arithmetic conversions
func commonType(a, b *Type) *Type {
	if a.Kind == TyEnum {
		a = a.Enum.underlying()
	}
	if b.Kind == TyEnum {
		b = b.Enum.underlying()
	}
	if a.Kind == TyLongDouble || b.Kind == TyLongDouble {
		return tyLDouble
	}
	if a.Kind == TyDouble || b.Kind == TyDouble {
		return tyDouble
	}
	if a.Kind == TyFloat || b.Kind == TyFloat {
		return tyFloat
	}
	a = intPromote(a)
	b = intPromote(b)
	if a.Unsigned == b.Unsigned {
		if intRank(a) >= intRank(b) {
			return a
		}
		return b
	}
	u, s := a, b
	if !a.Unsigned {
		u, s = b, a
	}
	if intRank(u) >= intRank(s) {
		return u
	}
	if s.Size() > u.Size() {
		return s
	}
	return unsignedOf(s)
}

// operandType is the type of an operand for conversions: bit-fields
// narrower than int promote to int
func operandType(n *Node) *Type {
	if n.Kind == NMember && n.Member.Bitfield && n.Ty.IsInteger() && intRank(n.Ty) <= intRank(tyInt) {
		if n.Member.BitWidth < 32 {
			return tyInt
		}
	}
	return n.Ty
}

func usualArith(n *Node) {
	ty := commonType(operandType(n.LHS), operandType(n.RHS))
	n.LHS = newCast(n.LHS, ty)
	n.RHS = newCast(n.RHS, ty)
}

func isNullPtrConst(n *Node) bool {
	if n.Ty != nil && n.Ty.Kind == TyNullptr {
		return true
	}
	for n.Kind == NCast && n.Ty.Kind == TyPtr && n.Ty.Base.Kind == TyVoid {
		n = n.LHS
	}
	if n.Kind == NCast && n.Ty.IsInteger() {
		n = n.LHS
	}
	return n.Kind == NNum && n.Ty != nil && n.Ty.IsInteger() && n.Val == 0
}

// memberType is the type of a member access, after bit-field promotion
func memberType(m *Member) *Type {
	return m.Ty
}

// addType computes the type of an expression tree
func addType(n *Node) {
	if n == nil || n.Ty != nil {
		return
	}
	addType(n.LHS)
	addType(n.RHS)
	addType(n.Cond)
	addType(n.Then)
	addType(n.Els)
	addType(n.Init)
	addType(n.Inc)
	for _, b := range n.Body {
		addType(b)
	}
	for _, a := range n.Args {
		addType(a)
	}
	switch n.Kind {
	case NNum:
		n.Ty = tyInt
	case NAdd, NSub, NMul, NDiv, NMod, NBitAnd, NBitOr, NBitXor:
		usualArith(n)
		n.Ty = n.LHS.Ty
	case NNeg:
		ty := intPromote(operandType(n.LHS))
		if n.LHS.Ty.IsFlonum() {
			ty = n.LHS.Ty.Unqual()
		}
		n.LHS = newCast(n.LHS, ty)
		n.Ty = ty
	case NAssign:
		if n.LHS.Ty.Kind == TyArray {
			panic(fatal{errAt(n.Tok, false, "assignment to expression with array type")})
		}
		if !n.LHS.Ty.IsRecord() || n.LHS.Ty.IsComplex() {
			n.RHS = newCast(n.RHS, n.LHS.Ty.Unqual())
		}
		n.Ty = n.LHS.Ty.Unqual()
	case NEq, NNe, NLt, NLe:
		if n.LHS.Ty.IsNumeric() && n.RHS.Ty.IsNumeric() {
			usualArith(n)
		} else {
			if n.LHS.Ty.IsArray() || n.LHS.Ty.Kind == TyFunc {
				n.LHS = newCast(n.LHS, decayType(n.LHS.Ty))
			}
			if n.RHS.Ty.IsArray() || n.RHS.Ty.Kind == TyFunc {
				n.RHS = newCast(n.RHS, decayType(n.RHS.Ty))
			}
		}
		n.Ty = tyInt
	case NFuncall:
		if n.Ty == nil {
			n.Ty = tyLong
		}
	case NNot, NLogOr, NLogAnd:
		n.Ty = tyInt
	case NBitNot:
		ty := intPromote(operandType(n.LHS))
		n.LHS = newCast(n.LHS, ty)
		n.Ty = ty
	case NShl, NShr:
		lt := intPromote(operandType(n.LHS))
		n.LHS = newCast(n.LHS, lt)
		n.RHS = newCast(n.RHS, intPromote(operandType(n.RHS)))
		n.Ty = lt
	case NVar:
		n.Ty = n.Var.Ty
	case NVLAPtr:
		n.Ty = pointerTo(n.Var.Ty.Base)
	case NCond:
		lt, rt := n.Then.Ty, n.Els.Ty
		switch {
		case lt.Kind == TyVoid || rt.Kind == TyVoid:
			n.Ty = tyVoid
		case lt.IsComplex() || rt.IsComplex():
			ty := complexResultType(lt, rt)
			n.Then = newCast(n.Then, ty)
			n.Els = newCast(n.Els, ty)
			n.Ty = ty
		case lt.IsNumeric() && rt.IsNumeric():
			ty := commonType(operandType(n.Then), operandType(n.Els))
			n.Then = newCast(n.Then, ty)
			n.Els = newCast(n.Els, ty)
			n.Ty = ty
		case lt.IsRecord() || rt.IsRecord():
			n.Ty = lt
		default:
			ld, rd := decayType(lt), decayType(rt)
			switch {
			case ld.Kind == TyPtr && isNullPtrConst(n.Els):
				n.Ty = ld
			case rd.Kind == TyPtr && isNullPtrConst(n.Then):
				n.Ty = rd
			case ld.Kind == TyPtr && rd.Kind == TyPtr:
				if ld.Base.Kind == TyVoid || rd.Base.Kind == TyVoid {
					n.Ty = pointerTo(tyVoid.withQuals(ld.Base.Const || rd.Base.Const, ld.Base.Volatile || rd.Base.Volatile, false, false))
				} else {
					n.Ty = pointerTo(ld.Base.withQuals(rd.Base.Const, rd.Base.Volatile, false, false))
				}
			case ld.Kind == TyPtr:
				n.Ty = ld
			case rd.Kind == TyPtr:
				n.Ty = rd
			case ld.Kind == TyNullptr && rd.Kind == TyNullptr:
				n.Ty = tyNullptr
			default:
				n.Ty = ld
			}
			n.Then = newCast(n.Then, n.Ty)
			n.Els = newCast(n.Els, n.Ty)
		}
	case NComma:
		n.Ty = n.RHS.Ty
		// the comma result is not an lvalue: arrays decay (but compound
		// literals, built as (init, var), keep their type)
		if n.Ty != nil && (n.Ty.IsArray() || n.Ty.Kind == TyFunc) && !(n.RHS.Kind == NVar && n.RHS.Var.IsLocal && n.RHS.Var.Name == "") {
			n.Ty = decayType(n.Ty)
		}
	case NMember:
		n.Ty = memberType(n.Member)
	case NAddr:
		n.Ty = pointerTo(n.LHS.Ty)
	case NDeref:
		t := n.LHS.Ty
		if t.Kind != TyPtr && !t.IsArray() {
			if t.Kind == TyFunc {
				n.Ty = t
				return
			}
			panic(fatal{errAt(n.Tok, false, "invalid pointer dereference")})
		}
		// *p of a void *p is an expression of type void, as in gcc (used
		// in __typeof__ by <tgmath.h>); using its value is an error
		n.Ty = t.Base
	case NStmtExpr:
		n.Ty = tyVoid
		if len(n.Body) > 0 {
			last := n.Body[len(n.Body)-1]
			if last.Kind == NExprStmt {
				n.Ty = last.LHS.Ty
			}
		}
	case NLabelVal:
		n.Ty = pointerTo(tyVoid)
	case NCas:
		n.Ty = tyBool
	case NExch:
		n.Ty = n.CasAddr.Ty.Base
	case NMemzero, NExprStmt, NReturn, NIf, NFor, NDo, NSwitch, NCase, NBlock, NGoto, NGotoExpr, NLabel, NAsm:
		n.Ty = tyVoid // statements
	}
}
