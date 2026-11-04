package x64

import (
	"math"

	"github.com/ipoluianov/altc/cc"
)

// Local improvements of the stack-machine code: constant casts, operands
// that need no stack round trip, values computed only to be discarded and
// arithmetic done in place on register variables.

// isIntLike reports integer and pointer types that fit a register
func isIntLike(t *cc.Type) bool {
	return t != nil && (t.Kind == cc.TyPtr || t.IsInteger() && !is128(t) && t.Size() <= 8)
}

// normInt converts v to the integer type t, extended to 64 bits like the
// value in rax
func normInt(v int64, t *cc.Type) int64 {
	if t.Kind == cc.TyBool {
		if v != 0 {
			return 1
		}
		return 0
	}
	uns := t.IsUnsigned() || t.Kind == cc.TyPtr
	switch t.Size() {
	case 1:
		if uns {
			return int64(uint8(v))
		}
		return int64(int8(v))
	case 2:
		if uns {
			return int64(uint16(v))
		}
		return int64(int16(v))
	case 4:
		if uns {
			return int64(uint32(v))
		}
		return int64(int32(v))
	}
	return v
}

// intConst evaluates an integer expression made of constants and casts
func intConst(n *cc.Node) (int64, bool) {
	if n == nil || !isIntLike(n.Ty) {
		return 0, false
	}
	switch n.Kind {
	case cc.NNum:
		return normInt(n.Val, n.Ty), true
	case cc.NCast:
		if !isIntLike(n.LHS.Ty) {
			return 0, false
		}
		v, ok := intConst(n.LHS)
		return normInt(v, n.Ty), ok
	case cc.NAdd, cc.NSub, cc.NMul:
		a, ok1 := intConst(n.LHS)
		b, ok2 := intConst(n.RHS)
		if !ok1 || !ok2 {
			return 0, false
		}
		switch n.Kind {
		case cc.NAdd:
			return normInt(a+b, n.Ty), true
		case cc.NSub:
			return normInt(a-b, n.Ty), true
		}
		return normInt(a*b, n.Ty), true
	}
	return 0, false
}

// isLeaf tells whether evaluating n has no side effects and changes no
// register but rax
func (g *gen) isLeaf(n *cc.Node) bool {
	if !isIntLike(n.Ty) {
		return false
	}
	if _, ok := intConst(n); ok {
		return true
	}
	switch n.Kind {
	case cc.NVar:
		if _, ok := g.varReg(n.Var); ok {
			return true
		}
		return isSimpleVar(n) && !g.isOver(n.Var)
	case cc.NCast:
		return n.Ty.Kind != cc.TyBool && g.isLeaf(n.LHS)
	case cc.NDeref, cc.NMember:
		_, ok := g.lvalMem(n)
		return ok
	}
	return false
}

// regVarNode returns the register of a node reading a register variable
func (g *gen) regVarNode(n *cc.Node) (Reg, bool) {
	if n.Kind != cc.NVar {
		return 0, false
	}
	return g.varReg(n.Var)
}

func commutative(k cc.NodeKind) bool {
	switch k {
	case cc.NAdd, cc.NMul, cc.NBitAnd, cc.NBitOr, cc.NBitXor, cc.NEq, cc.NNe:
		return true
	}
	return false
}

// pureDiscard strips the parts of a discarded expression that compute
// nothing observable
func pureDiscard(n *cc.Node) *cc.Node {
	for n != nil {
		switch n.Kind {
		case cc.NCast:
			if (isIntLike(n.Ty) || n.Ty.Kind == cc.TyVoid) && isIntLike(n.LHS.Ty) {
				n = n.LHS
				continue
			}
		case cc.NAdd, cc.NSub, cc.NMul, cc.NBitAnd, cc.NBitOr, cc.NBitXor:
			if _, ok := intConst(n.RHS); ok && isIntLike(n.LHS.Ty) {
				n = n.LHS
				continue
			}
		}
		return n
	}
	return n
}

// assignRegVar stores the value of an assignment to the register r; the
// value is left in rax when want is set
func (g *gen) assignRegVar(n *cc.Node, r Reg, want bool) {
	t := n.LHS.Ty
	if r.IsXMM() {
		g.assignXMMVar(n, r)
		return
	}
	if v, ok := intConst(n.RHS); ok {
		g.a.MovImm(r, normInt(v, t))
		if want {
			g.a.Mov(8, Q(RAX), Q(r))
		}
		return
	}
	if g.inPlace(n, r, t) {
		if want {
			g.a.Mov(8, Q(RAX), Q(r))
		}
		return
	}
	g.genExpr(n.RHS)
	g.extendInto(r, t, RAX)
}

// inPlace handles v = v op x for a register variable v by applying the
// operation to its register
func (g *gen) inPlace(n *cc.Node, r Reg, t *cc.Type) bool {
	e := n.RHS
	if e.Kind == cc.NCast && getTypeID(e.Ty) == getTypeID(e.LHS.Ty) {
		e = e.LHS
	}
	sz := int(t.Size())
	if sz != 4 && sz != 8 || !isIntLike(e.Ty) || int(e.Ty.Size()) != sz || e.LHS == nil || e.RHS == nil {
		return false
	}
	if lr, ok := g.opReg(e.LHS, sz); !ok || lr != r {
		return false
	}
	var src Operand
	if c, ok := intConst(e.RHS); ok && fitsI32(c) {
		src = Imm(c)
	} else if rr, ok := g.opReg(e.RHS, sz); ok {
		src = Sized(rr, sz)
	} else if m, ok := g.memOperand(e.RHS, sz); ok && e.Kind != cc.NShl && e.Kind != cc.NShr {
		src = m
	} else {
		return false
	}
	dst := Sized(r, sz)
	switch e.Kind {
	case cc.NAdd:
		g.a.Alu(ADD, sz, dst, src)
	case cc.NSub:
		g.a.Alu(SUB, sz, dst, src)
	case cc.NBitAnd:
		g.a.Alu(AND, sz, dst, src)
	case cc.NBitOr:
		g.a.Alu(OR, sz, dst, src)
	case cc.NBitXor:
		g.a.Alu(XOR, sz, dst, src)
	case cc.NMul:
		if c, ok := src.(Imm); ok {
			g.a.Imul3(sz, r, dst, int64(c))
		} else {
			g.a.Imul2(sz, r, src)
		}
	case cc.NShl, cc.NShr:
		c, ok := src.(Imm)
		if !ok {
			return false
		}
		op := SHL
		if e.Kind == cc.NShr {
			op = SAR
			if e.Ty.IsUnsigned() {
				op = SHR
			}
		}
		g.a.ShiftImm(op, sz, dst, int(int64(c)&int64(sz*8-1)))
	default:
		return false
	}
	if sz == 4 && !t.IsUnsigned() {
		g.a.Movsx(8, r, 4, D(r))
	}
	return true
}

// compareRegVar compares a register variable with a constant or another
// register variable without moving it to rax
func (g *gen) compareRegVar(n *cc.Node) (Cond, bool) {
	lt := n.LHS.Ty
	if !isIntLike(lt) {
		return 0, false
	}
	sz := 4
	if lt.Size() == 8 {
		sz = 8
	}
	lr, ok := g.opReg(n.LHS, sz)
	if !ok {
		return 0, false
	}
	var src Operand
	if c, ok := intConst(n.RHS); ok && fitsI32(c) {
		src = Imm(c)
	} else if rr, ok := g.opReg(n.RHS, sz); ok {
		src = Sized(rr, sz)
	} else if m, ok := g.memOperand(n.RHS, sz); ok {
		src = m
	} else {
		return 0, false
	}
	g.a.Alu(CMP, sz, Sized(lr, sz), src)
	return compareCond(n.Kind, lt.IsUnsigned() || lt.Kind == cc.TyPtr), true
}

// stripWide removes casts between 64-bit integer and pointer types, which
// do not change a value held in a register
func stripWide(n *cc.Node) *cc.Node {
	for n.Kind == cc.NCast && isIntLike(n.Ty) && n.Ty.Size() == 8 && isIntLike(n.LHS.Ty) {
		if n.LHS.Ty.Size() != 8 && n.LHS.Kind != cc.NVar {
			break
		}
		n = n.LHS
	}
	return n
}

// wideReg returns the register holding the 64-bit value of n: a register
// variable, possibly converted to a 64-bit type (the register already
// holds the extended value)
func (g *gen) wideReg(n *cc.Node) (Reg, bool) {
	n = stripWide(n)
	if n.Kind != cc.NVar || !isIntLike(n.Ty) {
		return 0, false
	}
	return g.varReg(n.Var)
}

func addDisp(m Mem, d int64) (Mem, bool) {
	v := int64(m.Disp) + d
	if !fitsI32(v) {
		return m, false
	}
	m.Disp = int32(v)
	return m, true
}

// addrMem returns a memory operand for the address computed by the
// pointer expression n, when that takes no code
func (g *gen) addrMem(n *cc.Node) (Mem, bool) {
	if n.Ty == nil || n.Ty.Kind != cc.TyPtr && n.Ty.Kind != cc.TyArray && !(isIntLike(n.Ty) && n.Ty.Size() == 8) {
		return Mem{}, false
	}
	if r, ok := g.wideReg(n); ok {
		return M(r, 0), true
	}
	n = stripWide(n)
	switch n.Kind {
	case cc.NVar:
		// an array decaying to a pointer
		if n.Var.Ty.Kind == cc.TyArray {
			return g.lvalMem(n)
		}
	case cc.NCast:
		if n.LHS.Ty.Kind == cc.TyArray {
			return g.addrMem(n.LHS)
		}
	case cc.NAddr:
		return g.lvalMem(n.LHS)
	case cc.NAdd, cc.NSub:
		m, ok := g.addrMem(n.LHS)
		if !ok {
			return m, false
		}
		if c, ok := intConst(n.RHS); ok {
			if n.Kind == cc.NSub {
				c = -c
			}
			return addDisp(m, c)
		}
		if n.Kind != cc.NAdd || m.Index != NoReg || m.Base == RIP {
			return m, false
		}
		idx, scale := n.RHS, int64(1)
		if idx.Kind == cc.NMul {
			c, ok := intConst(idx.RHS)
			if !ok || c != 1 && c != 2 && c != 4 && c != 8 {
				return m, false
			}
			idx, scale = idx.LHS, c
		}
		r, ok := g.wideReg(idx)
		if !ok || idx.Ty.Size() != 8 && stripWide(idx) == idx {
			return m, false
		}
		m.Index, m.Scale = r, int(scale)
		return m, true
	}
	return Mem{}, false
}

// lvalMem returns a memory operand for an lvalue whose address takes no
// code
func (g *gen) lvalMem(n *cc.Node) (Mem, bool) {
	switch n.Kind {
	case cc.NDeref:
		return g.addrMem(n.LHS)
	case cc.NMember:
		if n.Member.Bitfield {
			return Mem{}, false
		}
		m, ok := g.lvalMem(n.LHS)
		if !ok {
			return m, false
		}
		return addDisp(m, n.Member.Offset)
	case cc.NVar:
		v := n.Var
		if _, ok := g.varReg(v); ok || v.Ty.Kind == cc.TyVLA || g.isOver(v) {
			return Mem{}, false
		}
		if v.IsLocal {
			return M(RBP, int32(v.Offset)), true
		}
		if !v.IsTLS && !v.IsFunction && (g.isLocalDef(v) || v.IsStatic) {
			return MSym(v.AsmName, 0), true
		}
	}
	return Mem{}, false
}

// directScalar tells whether values of t move between memory and
// rax/xmm0 with loadDirect and storeAt
func directScalar(t *cc.Type) bool {
	return isIntLike(t) || isSSE(t)
}

// storeAt stores rax or xmm0 to memory as type t
func (g *gen) storeAt(t *cc.Type, m Mem) {
	switch t.Kind {
	case cc.TyFloat:
		g.a.MovF(false, m, X(XMM0))
	case cc.TyDouble:
		g.a.MovF(true, m, X(XMM0))
	default:
		g.storeSized(m, RAX, t.Size())
	}
}

// floatLeaf tells whether evaluating n changes only rax and xmm0 and has
// no side effects
func (g *gen) floatLeaf(n *cc.Node) bool {
	if !isSSE(n.Ty) {
		return false
	}
	if n.Kind == cc.NNum {
		return true
	}
	if r, ok := g.regVarNode(n); ok && r.IsXMM() {
		return true
	}
	_, ok := g.lvalMem(n)
	return ok && (n.Kind == cc.NVar || n.Kind == cc.NDeref || n.Kind == cc.NMember)
}

// memOperand returns the memory operand of an integer load of size sz
func (g *gen) memOperand(n *cc.Node, sz int) (Mem, bool) {
	if sz == 8 {
		n = stripWide(n)
	}
	if !isIntLike(n.Ty) || int(n.Ty.Size()) != sz || n.Kind != cc.NDeref && n.Kind != cc.NMember && n.Kind != cc.NVar {
		return Mem{}, false
	}
	return g.lvalMem(n)
}

// floatMem returns a memory operand holding the value of a float or double
// expression: a variable, a load or a constant (placed in the pool)
func (g *gen) floatMem(n *cc.Node) *Mem {
	if !isSSE(n.Ty) {
		return nil
	}
	if f, ok := floatConst(n); ok {
		var data []byte
		if isFloat(n.Ty) {
			b := math.Float32bits(float32(f))
			data = []byte{byte(b), byte(b >> 8), byte(b >> 16), byte(b >> 24)}
		} else {
			b := math.Float64bits(f)
			for i := 0; i < 8; i++ {
				data = append(data, byte(b>>(8*i)))
			}
		}
		m := MSym(g.constLabel(data, int64(len(data))), 0)
		return &m
	}
	if n.Kind != cc.NVar && n.Kind != cc.NDeref && n.Kind != cc.NMember {
		return nil
	}
	if m, ok := g.lvalMem(n); ok {
		return &m
	}
	return nil
}

// floatSrc returns an operand holding the value of a float or double
// expression without code: an xmm register variable or memory
func (g *gen) floatSrc(n *cc.Node) Operand {
	if !isSSE(n.Ty) {
		return nil
	}
	if r, ok := g.regVarNode(n); ok && r.IsXMM() {
		return X(r)
	}
	if m := g.floatMem(n); m != nil {
		return *m
	}
	return nil
}

// assignXMMVar assigns to a float variable living in an xmm register; the
// value is also left in xmm0. v = v op x is done in place.
func (g *gen) assignXMMVar(n *cc.Node, r Reg) {
	dbl := isDouble(n.LHS.Ty)
	e := n.RHS
	if e.Kind == cc.NAdd || e.Kind == cc.NSub || e.Kind == cc.NMul || e.Kind == cc.NDiv {
		if lr, ok := g.regVarNode(e.LHS); ok && lr == r && e.Ty.Kind == n.LHS.Ty.Kind {
			if src := g.floatSrc(e.RHS); src != nil {
				op := map[cc.NodeKind]FOp{cc.NAdd: FAdd, cc.NSub: FSub, cc.NMul: FMul, cc.NDiv: FDiv}[e.Kind]
				g.a.FArith(op, dbl, r, src)
				g.a.MovF(true, X(XMM0), X(r))
				return
			}
		}
	}
	g.genExpr(n.RHS)
	g.a.MovF(true, X(r), X(XMM0))
}

func isMem(_ Mem, ok bool) bool { return ok }

// argLoader returns the code that puts the value of a call argument
// straight into its register, for arguments that need no evaluation:
// constants, register variables, loads from simple addresses and such
// addresses themselves
func (g *gen) argLoader(a *cc.Node) func(Reg) {
	t := a.Ty
	if isSSE(t) {
		if a.Kind == cc.NNum {
			return nil // a constant pool entry is made only when used
		}
		src := g.floatSrc(a)
		if src == nil {
			return nil
		}
		dbl := isDouble(t)
		return func(r Reg) { g.a.MovF(dbl, X(r), src) }
	}
	if !isIntLike(t) {
		return nil
	}
	if v, ok := intConst(a); ok {
		return func(r Reg) {
			if v == 0 {
				g.a.Alu(XOR, 4, D(r), D(r))
			} else {
				g.a.MovImm(r, v)
			}
		}
	}
	if t.Size() == 8 {
		if src, ok := g.wideReg(a); ok {
			return func(r Reg) { g.a.Mov(8, Q(r), Q(src)) }
		}
	}
	if src, ok := g.regVarNode(a); ok {
		return func(r Reg) { g.a.Mov(8, Q(r), Q(src)) }
	}
	if a.Kind == cc.NAddr {
		if m, ok := g.lvalMem(a.LHS); ok {
			return func(r Reg) { g.a.Lea(r, m) }
		}
		return nil
	}
	if a.Kind == cc.NVar || a.Kind == cc.NDeref || a.Kind == cc.NMember {
		if m, ok := g.lvalMem(a); ok {
			return func(r Reg) { g.loadInto(r, t, m) }
		}
	}
	return nil
}

// opReg returns the register variable holding the value of an operand
// of an operation of size sz
func (g *gen) opReg(n *cc.Node, sz int) (Reg, bool) {
	if !isIntLike(n.Ty) {
		return 0, false
	}
	if sz == 8 && n.Ty.Size() == 8 {
		return g.wideReg(n)
	}
	if int(n.Ty.Size()) < sz {
		return 0, false
	}
	return g.regVarNode(n)
}

// storeConst stores a constant assigned to a simple lvalue when the value
// of the assignment is not used
func (g *gen) storeConst(n *cc.Node) bool {
	lhs := n.LHS
	if !isIntLike(lhs.Ty) {
		return false
	}
	v, ok := intConst(n.RHS)
	if !ok {
		return false
	}
	sz := int(lhs.Ty.Size())
	switch sz {
	case 1:
		v = int64(int8(v))
	case 2:
		v = int64(int16(v))
	case 4:
		v = int64(int32(v))
	default:
		if !fitsI32(v) {
			return false
		}
	}
	m, ok := g.lvalMem(lhs)
	if !ok {
		return false
	}
	g.a.Mov(sz, m, Imm(v))
	return true
}

func isReg(_ Reg, ok bool) bool { return ok }

// floatConst evaluates a float or double constant, possibly converted
// from an integer or another floating type
func floatConst(n *cc.Node) (float64, bool) {
	if !isSSE(n.Ty) {
		return 0, false
	}
	switch n.Kind {
	case cc.NNum:
		if isFloat(n.Ty) {
			return float64(float32(n.FVal)), true
		}
		return n.FVal, true
	case cc.NCast:
		var f float64
		if v, ok := intConst(n.LHS); ok {
			// rounded once, straight to the target type
			uns := n.LHS.Ty.IsUnsigned() && n.LHS.Ty.Size() == 8
			switch {
			case isFloat(n.Ty) && uns:
				return float64(float32(uint64(v))), true
			case isFloat(n.Ty):
				return float64(float32(v)), true
			case uns:
				return float64(uint64(v)), true
			}
			return float64(v), true
		} else if c, ok := floatConst(n.LHS); ok {
			f = c
		} else {
			return 0, false
		}
		if isFloat(n.Ty) {
			f = float64(float32(f))
		}
		return f, true
	}
	return 0, false
}
