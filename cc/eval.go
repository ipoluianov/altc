package cc

import (
	"math"
	"math/big"
	"strings"
)

// constExpr parses and evaluates an integer constant expression
func (p *parser) constExpr() int64 {
	n := p.conditional()
	addType(n)
	return p.eval(n)
}

func (p *parser) eval(n *Node) int64 { return p.eval2(n, nil) }

// isConstExpr tells whether an expression is an arithmetic constant expression
func (p *parser) isConstExpr(n *Node) bool {
	addType(n)
	switch n.Kind {
	case NAdd, NSub, NMul, NDiv, NMod, NBitAnd, NBitOr, NBitXor, NShl, NShr, NEq, NNe, NLt, NLe:
		if !p.isConstExpr(n.LHS) || !p.isConstExpr(n.RHS) {
			return false
		}
		if (n.Kind == NDiv || n.Kind == NMod) && n.RHS.Ty.IsInteger() && p.eval(n.RHS) == 0 {
			return false
		}
		return true
	case NLogAnd:
		if !p.isConstExpr(n.LHS) {
			return false
		}
		if p.eval(n.LHS) == 0 {
			return true
		}
		return p.isConstExpr(n.RHS)
	case NLogOr:
		if !p.isConstExpr(n.LHS) {
			return false
		}
		if p.eval(n.LHS) != 0 {
			return true
		}
		return p.isConstExpr(n.RHS)
	case NCond:
		if !p.isConstExpr(n.Cond) {
			return false
		}
		if p.evalCond(n.Cond) {
			return p.isConstExpr(n.Then)
		}
		return p.isConstExpr(n.Els)
	case NComma:
		return p.isConstExpr(n.RHS) && p.isConstExpr(n.LHS)
	case NNeg, NNot, NBitNot:
		return p.isConstExpr(n.LHS)
	case NCast:
		if n.Ty.Kind == TyPtr && n.LHS.Ty.IsInteger() {
			return p.isConstExpr(n.LHS)
		}
		return (n.Ty.IsScalar() || n.Ty.Kind == TyVoid) && n.LHS.Ty.IsNumeric() && p.isConstExpr(n.LHS)
	case NNum:
		return true
	case NVar:
		return n.Var.ConstExpr != nil
	case NNull:
		return true
	}
	return false
}

func (p *parser) evalCond(n *Node) bool {
	addType(n)
	if n.Ty.IsFlonum() {
		return p.evalDouble(n) != 0
	}
	return p.eval(n) != 0
}

func (p *parser) notConst(n *Node) {
	p.errorAt(n.Tok, "initializer element is not constant")
}

// eval2 evaluates an integer or address constant. When label is not nil
// the result may be an address: *label receives the symbol name and the
// returned value is the offset.
func (p *parser) eval2(n *Node, label *string) int64 {
	addType(n)
	if n.Ty.IsFlonum() {
		return int64(p.evalDouble(n))
	}
	switch n.Kind {
	case NAdd:
		return p.eval2(n.LHS, label) + p.eval(n.RHS)
	case NSub:
		if label != nil {
			// &a[5] - &a[2]
			var l1, l2 string
			a := p.eval2(n.LHS, &l1)
			b := p.eval2(n.RHS, &l2)
			if l2 != "" {
				if l1 == l2 {
					return a - b
				}
				if strings.HasPrefix(l1, ".Llabel.") && strings.HasPrefix(l2, ".Llabel.") {
					*label = l1 + "\x00" + l2
					return a - b
				}
				p.notConst(n)
			}
			*label = l1
			return a - b
		}
		return p.eval(n.LHS) - p.eval(n.RHS)
	case NMul:
		return p.eval(n.LHS) * p.eval(n.RHS)
	case NDiv, NMod:
		if label != nil && n.Kind == NDiv && n.RHS.Kind == NNum && n.RHS.Val == 1 {
			return p.eval2(n.LHS, label)
		}
		a, b := p.eval(n.LHS), p.eval(n.RHS)
		if b == 0 {
			p.errorAt(n.Tok, "division by zero in constant expression")
		}
		if n.Ty.IsUnsigned() {
			if n.Kind == NDiv {
				return int64(uint64(a) / uint64(b))
			}
			return int64(uint64(a) % uint64(b))
		}
		if b == -1 {
			if n.Kind == NDiv {
				return -a
			}
			return 0
		}
		if n.Kind == NDiv {
			return a / b
		}
		return a % b
	case NNeg:
		return -p.eval(n.LHS)
	case NBitAnd:
		return p.eval(n.LHS) & p.eval(n.RHS)
	case NBitOr:
		return p.eval(n.LHS) | p.eval(n.RHS)
	case NBitXor:
		return p.eval(n.LHS) ^ p.eval(n.RHS)
	case NShl:
		return truncInt(p.eval(n.LHS)<<uint64(p.eval(n.RHS)&63), n.Ty)
	case NShr:
		s := uint64(p.eval(n.RHS) & 63)
		if n.Ty.IsUnsigned() && n.Ty.Size() == 8 {
			return int64(uint64(p.eval(n.LHS)) >> s)
		}
		if n.Ty.IsUnsigned() {
			return int64(uint32(p.eval(n.LHS)) >> s)
		}
		return p.eval(n.LHS) >> s
	case NEq, NNe, NLt, NLe:
		if n.LHS.Ty.IsFlonum() {
			a, b := p.evalDouble(n.LHS), p.evalDouble(n.RHS)
			switch n.Kind {
			case NEq:
				return b2i(a == b)
			case NNe:
				return b2i(a != b)
			case NLt:
				return b2i(a < b)
			}
			return b2i(a <= b)
		}
		a, b := p.eval(n.LHS), p.eval(n.RHS)
		uns := n.LHS.Ty.IsUnsigned()
		switch n.Kind {
		case NEq:
			return b2i(a == b)
		case NNe:
			return b2i(a != b)
		case NLt:
			if uns {
				return b2i(uint64(a) < uint64(b))
			}
			return b2i(a < b)
		}
		if uns {
			return b2i(uint64(a) <= uint64(b))
		}
		return b2i(a <= b)
	case NCond:
		if p.evalCond(n.Cond) {
			return p.eval2(n.Then, label)
		}
		return p.eval2(n.Els, label)
	case NComma:
		return p.eval2(n.RHS, label)
	case NNot:
		return b2i(!p.evalCond(n.LHS))
	case NBitNot:
		return truncInt(^p.eval(n.LHS), n.Ty)
	case NLogAnd:
		return b2i(p.evalCond(n.LHS) && p.evalCond(n.RHS))
	case NLogOr:
		return b2i(p.evalCond(n.LHS) || p.evalCond(n.RHS))
	case NCast:
		if n.LHS.Ty.IsFlonum() {
			f := p.evalDouble(n.LHS)
			if n.Ty.Kind == TyBool {
				return b2i(f != 0)
			}
			if n.Ty.IsUnsigned() && f >= 9223372036854775808.0 {
				return int64(uint64(f))
			}
			return truncInt(int64(f), n.Ty)
		}
		v := p.eval2(n.LHS, label)
		if n.Ty.Kind == TyBool {
			if label != nil && *label != "" {
				return 1
			}
			return b2i(v != 0)
		}
		if n.Ty.IsInteger() {
			if label != nil && *label != "" && n.Ty.Size() < 8 && !strings.Contains(*label, "\x00") {
				p.notConst(n)
			}
			return truncInt(v, n.Ty)
		}
		return v
	case NAddr:
		return p.evalRval(n.LHS, label)
	case NLabelVal:
		if label == nil || p.fn == nil {
			p.errorAt(n.Tok, "label address is not a constant here")
		}
		p.fn.LabelAddrs = true
		*label = LabelSymbol(p.fn.AsmName, n.Label)
		return 0
	case NMember:
		if label == nil {
			p.notConst(n)
		}
		if n.Ty.Kind != TyArray {
			p.notConst(n)
		}
		return p.evalRval(n.LHS, label) + n.Member.Offset
	case NVar:
		if n.Var.ConstExpr != nil {
			return p.eval2(n.Var.ConstExpr, label)
		}
		if label == nil {
			p.errorAt(n.Tok, "expression is not a compile-time constant")
		}
		if n.Var.IsLocal {
			p.notConst(n)
		}
		if n.Var.Ty.Kind != TyArray && n.Var.Ty.Kind != TyFunc {
			p.notConst(n)
		}
		n.Var.IsUsed = true
		*label = n.Var.AsmName
		return 0
	case NNum:
		return n.Val
	case NDeref:
		// *&x or a[i] of an array that is itself an address
		if label != nil && n.Ty.Kind == TyArray {
			return p.eval2(n.LHS, label)
		}
	}
	p.errorAt(n.Tok, "expression is not a compile-time constant")
	return 0
}

func (p *parser) evalRval(n *Node, label *string) int64 {
	switch n.Kind {
	case NVar:
		if n.Var.IsLocal || label == nil {
			p.notConst(n)
		}
		n.Var.IsUsed = true
		*label = n.Var.AsmName
		return 0
	case NDeref:
		return p.eval2(n.LHS, label)
	case NMember:
		return p.evalRval(n.LHS, label) + n.Member.Offset
	case NComma:
		return p.evalRval(n.RHS, label)
	}
	p.notConst(n)
	return 0
}

// evalDouble evaluates a floating constant expression
func (p *parser) evalDouble(n *Node) float64 {
	addType(n)
	if n.Ty.IsInteger() {
		v := p.eval(n)
		if n.Ty.IsUnsigned() && n.Ty.Size() == 8 {
			return float64(uint64(v))
		}
		return float64(v)
	}
	switch n.Kind {
	case NAdd:
		return p.roundTo(n.Ty, canonNaN(p.evalDouble(n.LHS)+p.evalDouble(n.RHS)))
	case NSub:
		return p.roundTo(n.Ty, canonNaN(p.evalDouble(n.LHS)-p.evalDouble(n.RHS)))
	case NMul:
		return p.roundTo(n.Ty, canonNaN(p.evalDouble(n.LHS)*p.evalDouble(n.RHS)))
	case NDiv:
		return p.roundTo(n.Ty, canonNaN(p.evalDouble(n.LHS)/p.evalDouble(n.RHS)))
	case NNeg:
		return -p.evalDouble(n.LHS)
	case NCond:
		if p.evalCond(n.Cond) {
			return p.evalDouble(n.Then)
		}
		return p.evalDouble(n.Els)
	case NComma:
		return p.evalDouble(n.RHS)
	case NCast:
		if n.LHS.Ty.IsFlonum() {
			return p.roundTo(n.Ty, p.evalDouble(n.LHS))
		}
		v := p.eval(n.LHS)
		if n.LHS.Ty.IsUnsigned() && n.LHS.Ty.Size() == 8 {
			return p.roundTo(n.Ty, float64(uint64(v)))
		}
		return p.roundTo(n.Ty, float64(v))
	case NNum:
		return n.FVal
	case NVar:
		if n.Var.ConstExpr != nil {
			return p.evalDouble(n.Var.ConstExpr)
		}
	}
	p.errorAt(n.Tok, "initializer element is not constant")
	return 0
}

// canonNaN makes NaN results of constant folding positive, like GCC
func canonNaN(f float64) float64 {
	if math.IsNaN(f) {
		return math.NaN()
	}
	return f
}

func (p *parser) roundTo(t *Type, f float64) float64 {
	if t.Kind == TyFloat {
		return float64(float32(f))
	}
	return f
}

// evalLD returns the exact long double value when it is known
func (p *parser) evalLD(n *Node) *big.Float {
	switch n.Kind {
	case NNum:
		if n.LD != nil {
			return n.LD
		}
		if n.Ty.IsInteger() {
			v := n.Val
			if n.Ty.IsUnsigned() {
				return new(big.Float).SetPrec(64).SetUint64(uint64(v))
			}
			return new(big.Float).SetPrec(64).SetInt64(v)
		}
	case NNeg:
		if v := p.evalLD(n.LHS); v != nil {
			return new(big.Float).SetPrec(64).Neg(v)
		}
	case NAdd, NSub, NMul, NDiv:
		a, b := p.evalLD(n.LHS), p.evalLD(n.RHS)
		if a != nil && b != nil && !a.IsInf() && !b.IsInf() {
			r := new(big.Float).SetPrec(64)
			switch n.Kind {
			case NAdd:
				return r.Add(a, b)
			case NSub:
				return r.Sub(a, b)
			case NMul:
				return r.Mul(a, b)
			default:
				if b.Sign() != 0 {
					return r.Quo(a, b)
				}
			}
		}
	case NCast:
		if n.LHS.Ty.IsInteger() && p.isConstExpr(n.LHS) {
			v := p.eval(n.LHS)
			if n.LHS.Ty.IsUnsigned() {
				return new(big.Float).SetPrec(64).SetUint64(uint64(v))
			}
			return new(big.Float).SetPrec(64).SetInt64(v)
		}
		if n.LHS.Ty.Kind == TyLongDouble {
			return p.evalLD(n.LHS)
		}
	}
	f := p.evalDouble(n)
	if math.IsInf(f, 0) || math.IsNaN(f) {
		return nil
	}
	return new(big.Float).SetPrec(64).SetFloat64(f)
}
