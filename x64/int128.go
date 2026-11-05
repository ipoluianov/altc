package x64

import "github.com/ipoluianov/altc/cc"

// 128-bit integers live in rdx:rax (rax holds the low half)

func is128(t *cc.Type) bool { return t.IsInteger() && t.Size() == 16 }

// push128 pushes rdx:rax (low half on top)
func (g *gen) push128() {
	g.a.Push(RDX)
	g.a.Push(RAX)
	g.depth += 2
}

// pop128 pops into lo:hi registers
func (g *gen) pop128(lo, hi Reg) {
	g.a.Pop(lo)
	g.a.Pop(hi)
	g.depth -= 2
}

func (g *gen) genInt128Binary(n *cc.Node) {
	uns := n.LHS.Ty.IsUnsigned()
	switch n.Kind {
	case cc.NShl, cc.NShr:
		g.genExpr(n.RHS)
		g.push()
		g.genExpr(n.LHS)
		g.pop(RCX)
		done := g.a.NewLabel()
		if n.Kind == cc.NShl {
			g.a.ShldCL(Q(RDX), RAX)
			g.a.ShiftCL(SHL, 8, Q(RAX))
			g.a.Test(1, B(RCX), Imm(64))
			g.a.Jcc(CondE, done)
			g.a.Mov(8, Q(RDX), Q(RAX))
			g.a.Alu(XOR, 4, D(RAX), D(RAX))
		} else {
			g.a.ShrdCL(Q(RAX), RDX)
			if uns {
				g.a.ShiftCL(SHR, 8, Q(RDX))
			} else {
				g.a.ShiftCL(SAR, 8, Q(RDX))
			}
			g.a.Test(1, B(RCX), Imm(64))
			g.a.Jcc(CondE, done)
			g.a.Mov(8, Q(RAX), Q(RDX))
			if uns {
				g.a.Alu(XOR, 4, D(RDX), D(RDX))
			} else {
				g.a.ShiftImm(SAR, 8, Q(RDX), 63)
			}
		}
		g.a.Bind(done)
		return
	case cc.NDiv, cc.NMod:
		// libgcc-compatible helpers from the runtime: a in rdi:rsi, b in rdx:rcx
		g.genExpr(n.RHS)
		g.push128()
		g.genExpr(n.LHS)
		g.a.Mov(8, Q(RDI), Q(RAX))
		g.a.Mov(8, Q(RSI), Q(RDX))
		g.pop128(RDX, RCX)
		name := "__divti3"
		switch {
		case n.Kind == cc.NDiv && uns:
			name = "__udivti3"
		case n.Kind == cc.NMod && uns:
			name = "__umodti3"
		case n.Kind == cc.NMod:
			name = "__modti3"
		}
		g.callAligned(name)
		return
	}
	g.genExpr(n.RHS)
	g.push128()
	g.genExpr(n.LHS)
	g.pop128(RDI, RSI) // rhs in rsi:rdi
	switch n.Kind {
	case cc.NAdd:
		g.a.Alu(ADD, 8, Q(RAX), Q(RDI))
		g.a.Alu(ADC, 8, Q(RDX), Q(RSI))
	case cc.NSub:
		g.a.Alu(SUB, 8, Q(RAX), Q(RDI))
		g.a.Alu(SBB, 8, Q(RDX), Q(RSI))
	case cc.NBitAnd:
		g.a.Alu(AND, 8, Q(RAX), Q(RDI))
		g.a.Alu(AND, 8, Q(RDX), Q(RSI))
	case cc.NBitOr:
		g.a.Alu(OR, 8, Q(RAX), Q(RDI))
		g.a.Alu(OR, 8, Q(RDX), Q(RSI))
	case cc.NBitXor:
		g.a.Alu(XOR, 8, Q(RAX), Q(RDI))
		g.a.Alu(XOR, 8, Q(RDX), Q(RSI))
	case cc.NMul:
		g.a.Mov(8, Q(R8), Q(RAX))
		g.a.Mov(8, Q(R9), Q(RDX))
		g.a.Imul2(8, R9, Q(RDI))  // a.hi * b.lo
		g.a.Mov(8, Q(R10), Q(R8)) // a.lo
		g.a.Imul2(8, R10, Q(RSI)) // a.lo * b.hi
		g.a.Mov(8, Q(RAX), Q(R8))
		g.a.Unary(MUL, 8, Q(RDI)) // rdx:rax = a.lo * b.lo
		g.a.Alu(ADD, 8, Q(RDX), Q(R9))
		g.a.Alu(ADD, 8, Q(RDX), Q(R10))
	case cc.NEq, cc.NNe:
		g.a.Alu(XOR, 8, Q(RAX), Q(RDI))
		g.a.Alu(XOR, 8, Q(RDX), Q(RSI))
		g.a.Alu(OR, 8, Q(RAX), Q(RDX))
		if n.Kind == cc.NEq {
			g.a.Setcc(CondE, RAX)
		} else {
			g.a.Setcc(CondNE, RAX)
		}
		g.a.Movzx(4, RAX, 1, B(RAX))
	case cc.NLt, cc.NLe:
		if n.Kind == cc.NLe {
			// a <= b  ==  !(b < a)
			g.a.Alu(SUB, 8, Q(RDI), Q(RAX))
			g.a.Alu(SBB, 8, Q(RSI), Q(RDX))
			if uns {
				g.a.Setcc(CondAE, RAX)
			} else {
				g.a.Setcc(CondGE, RAX)
			}
		} else {
			g.a.Alu(SUB, 8, Q(RAX), Q(RDI))
			g.a.Alu(SBB, 8, Q(RDX), Q(RSI))
			if uns {
				g.a.Setcc(CondB, RAX)
			} else {
				g.a.Setcc(CondL, RAX)
			}
		}
		g.a.Movzx(4, RAX, 1, B(RAX))
	default:
		g.fail(n, "invalid operands for a 128-bit operation")
	}
}

// callAligned calls a runtime helper keeping the stack 16-byte aligned
func (g *gen) callAligned(name string) {
	g.spillXMM()
	defer g.reloadXMM()
	if g.depth%2 != 0 {
		g.a.Alu(SUB, 8, Q(RSP), Imm(8))
		g.a.Call(name)
		g.a.Alu(ADD, 8, Q(RSP), Imm(8))
		return
	}
	g.a.Call(name)
}

// extend128 widens the integer in rax (of type from) to rdx:rax
func (g *gen) extend128(from *cc.Type) {
	if from.IsUnsigned() {
		g.a.Alu(XOR, 4, D(RDX), D(RDX))
	} else {
		g.a.Cqo()
	}
}

// cast128 converts to or from 128-bit integers; it reports whether it handled the cast
func (g *gen) cast128(from, to *cc.Type) bool {
	f128, t128 := is128(from), is128(to)
	if !f128 && !t128 {
		return false
	}
	switch {
	case f128 && t128:
		return true
	case t128 && (from.IsInteger() || from.Kind == cc.TyPtr):
		g.extend128(from)
	case t128 && from.Kind == cc.TyLongDouble:
		// long double -> 128 via the runtime, exactly
		if to.IsUnsigned() {
			g.callWithLD("__fixunsxfti")
		} else {
			g.callWithLD("__fixxfti")
		}
	case t128 && from.IsFlonum():
		// float/double -> 128 via the runtime
		if from.Kind == cc.TyFloat {
			g.a.CvtFF(true, XMM0, X(XMM0))
		}
		if to.IsUnsigned() {
			g.callAligned("__fixunsdfti")
		} else {
			g.callAligned("__fixdfti")
		}
	case f128 && to.Kind == cc.TyBool:
		g.a.Alu(OR, 8, Q(RAX), Q(RDX))
		g.a.Setcc(CondNE, RAX)
		g.a.Movzx(4, RAX, 1, B(RAX))
	case f128 && (to.IsInteger() || to.Kind == cc.TyPtr):
		g.intToInt(getTypeID(to))
	case f128 && to.Kind == cc.TyLongDouble:
		// the result comes back in st(0)
		g.a.Mov(8, Q(RDI), Q(RAX))
		g.a.Mov(8, Q(RSI), Q(RDX))
		if from.IsUnsigned() {
			g.callAligned("__floatuntixf")
		} else {
			g.callAligned("__floattixf")
		}
	case f128 && to.IsFlonum():
		g.a.Mov(8, Q(RDI), Q(RAX))
		g.a.Mov(8, Q(RSI), Q(RDX))
		if from.IsUnsigned() {
			g.callAligned("__floatuntidf")
		} else {
			g.callAligned("__floattidf")
		}
		if to.Kind == cc.TyFloat {
			g.a.CvtFF(false, XMM0, X(XMM0))
		}
	}
	return true
}

// callWithLD calls a runtime function taking the long double in st(0)
// (popped) as its only argument: 16 bytes on the stack
func (g *gen) callWithLD(name string) {
	size := int64(16)
	if g.depth%2 != 0 {
		size += 8 // keep the call 16-byte aligned
	}
	g.spillXMM()
	g.a.Alu(SUB, 8, Q(RSP), Imm(size))
	g.a.Fstp(10, M(RSP, 0))
	g.a.Call(name)
	g.a.Alu(ADD, 8, Q(RSP), Imm(size))
	g.reloadXMM()
}
