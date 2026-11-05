package x64

import "github.com/ipoluianov/altc/cc"

// Atomic read-modify-write operations in the IR (__sync_*, __atomic_*,
// _Atomic compound assignments). opAtomic returns the old value of the
// memory; success flags and new values are computed with ordinary values.
// The locked instructions are full barriers, and opAtomic is not pure, so
// no load is reused or moved across it.

type atomicKind int64

const (
	atomicCas  atomicKind = iota // Args: addr, expected, new
	atomicExch                   // Args: addr, value
	atomicAdd                    // lock xadd; Args: addr, value
	atomicSub
	atomicAnd // cmpxchg loops
	atomicOr
	atomicXor
	atomicNand
)

var atomicNames = [...]string{"cas", "exch", "add", "sub", "and", "or", "xor", "nand"}

// atomicElem checks the element type of an atomic operation
func atomicElem(t *cc.Type) *cc.Type {
	if t.Kind != cc.TyPtr && !t.IsInteger() || t.Size() > 8 || t.Size() == 0 {
		unsupported("atomic operation on %d bytes", t.Size())
	}
	return t
}

func (b *irBuilder) atomic(k atomicKind, elem *cc.Type, args ...*irValue) *irValue {
	v := b.add(opAtomic, irTypeOf(elem), args...)
	v.AuxInt = int64(k)
	v.Signed = !unsignedType(elem)
	v.Data = []byte{byte(cSize(elem))}
	return v
}

// operand converts the value of n to the element type
func (b *irBuilder) atomicOperand(n *cc.Node, elem *cc.Type) *irValue {
	v := b.expr(n)
	if v == nil {
		unsupported("void atomic operand")
	}
	return b.convert(v, n.Ty, elem)
}

func (b *irBuilder) casExpr(n *cc.Node) *irValue {
	elem := atomicElem(n.CasAddr.Ty.Base)
	addr := b.expr(n.CasAddr)
	nv := b.atomicOperand(n.CasNew, elem)
	if n.Begin != 2 {
		exp := b.atomicOperand(n.CasOld, elem)
		old := b.atomic(atomicCas, elem, addr, exp, nv)
		if n.Begin == 1 {
			return old // __sync_val_compare_and_swap
		}
		return b.add(opEq, iI32, old, exp)
	}
	// __atomic_compare_exchange: CasOld points to the expected value,
	// which receives the current one on failure
	ep := b.expr(n.CasOld)
	exp := b.load(ep, elem)
	old := b.atomic(atomicCas, elem, addr, exp, nv)
	ok := b.add(opEq, iI32, old, exp)
	fail, done := b.f.newBlock(), b.f.newBlock()
	b.branch(ok, done, fail)
	b.seal(fail)
	b.cur = fail
	b.store(ep, old, elem)
	b.jump(done)
	b.seal(done)
	b.cur = done
	return ok
}

func (b *irBuilder) exchExpr(n *cc.Node) *irValue {
	elem := atomicElem(n.CasAddr.Ty.Base)
	addr := b.expr(n.CasAddr)
	return b.atomic(atomicExch, elem, addr, b.atomicOperand(n.RHS, elem))
}

func (b *irBuilder) fetchOpExpr(n *cc.Node) *irValue {
	pt := n.LHS.Ty
	if pt.Kind != cc.TyPtr {
		pt = cc.DecayType(pt)
	}
	elem := atomicElem(pt.Base)
	addr := b.expr(n.LHS)
	var val *irValue
	if elem.Kind == cc.TyPtr {
		// added as an integer, like gcc's builtins (the front end scales
		// the compound assignments of _Atomic pointers)
		val = b.atomicOperand(n.RHS, cc.TypeLong)
	} else {
		val = b.atomicOperand(n.RHS, elem)
	}
	k := map[int64]atomicKind{cc.FetchAdd: atomicAdd, cc.FetchSub: atomicSub, cc.FetchAnd: atomicAnd,
		cc.FetchOr: atomicOr, cc.FetchXor: atomicXor, cc.FetchNand: atomicNand}[n.Val]
	old := b.atomic(k, elem, addr, val)
	if n.Begin != 1 {
		return old
	}
	// the new value, wrapped to the element type
	t := old.Type
	var nv *irValue
	switch k {
	case atomicAdd:
		nv = b.add(opAdd, t, old, val)
	case atomicSub:
		nv = b.add(opSub, t, old, val)
	case atomicAnd:
		nv = b.add(opAnd, t, old, val)
	case atomicOr:
		nv = b.add(opOr, t, old, val)
	case atomicXor:
		nv = b.add(opXor, t, old, val)
	case atomicNand:
		nv = b.add(opNot, t, b.add(opAnd, t, old, val))
	}
	return b.narrow(nv, elem)
}

// ---- emission ----

func (e *irEmitter) atomic(v *irValue) {
	a := e.a
	sz := int(v.Data[0])
	addr := M(e.ireg(v.Args[0], R10), 0)
	switch atomicKind(v.AuxInt) {
	case atomicCas:
		nv := e.ireg(v.Args[2], R11)
		e.materialize(v.Args[1], RAX)
		a.LockCmpxchg(sz, addr, nv)
	case atomicExch:
		e.materialize(v.Args[1], RAX)
		a.Xchg(sz, addr, RAX)
	case atomicAdd, atomicSub:
		e.materialize(v.Args[1], RAX)
		if atomicKind(v.AuxInt) == atomicSub {
			a.Unary(NEG, 8, Q(RAX))
		}
		a.LockXadd(sz, addr, RAX)
	default:
		val := Sized(e.ireg(v.Args[1], R11), sz)
		loop := a.NewLabel()
		a.Mov(sz, Sized(RAX, sz), addr)
		a.Bind(loop)
		a.Mov(8, Q(RDX), Q(RAX))
		switch atomicKind(v.AuxInt) {
		case atomicAnd, atomicNand:
			a.Alu(AND, sz, Sized(RDX, sz), val)
			if atomicKind(v.AuxInt) == atomicNand {
				a.Unary(NOT, sz, Sized(RDX, sz))
			}
		case atomicOr:
			a.Alu(OR, sz, Sized(RDX, sz), val)
		case atomicXor:
			a.Alu(XOR, sz, Sized(RDX, sz), val)
		}
		a.LockCmpxchg(sz, addr, RDX)
		a.Jcc(CondNE, loop)
	}
	// the old value, extended like every value of its type
	switch {
	case sz < 4 && v.Signed:
		a.Movsx(4, RAX, sz, Sized(RAX, sz))
	case sz < 4:
		a.Movzx(4, RAX, sz, Sized(RAX, sz))
	}
	if v.needsLoc() {
		e.finish(v, RAX)
	}
}
