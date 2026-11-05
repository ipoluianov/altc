package x64

import (
	"maps"
	"math"
	"math/bits"
	"sort"
)

// optimize runs the IR passes
func (f *irFunc) optimize() {
	f.cleanup()
	f.removeUnreachable()
	for i := 0; i < 2; i++ {
		f.fold()
		f.foldBranches()
		f.removeUnreachable()
		f.cleanup()
		f.analyze()
		f.gvn()
		f.licm()
	}
	f.fold()
	f.dce()
	f.simplifyCFG()
	f.analyze()
	if f.vectorize() {
		f.fold()
		f.cleanup()
		f.analyze()
		f.gvn()
		f.licm()
	}
	f.fold()
	f.dce()
	f.foldAddresses()
	f.dce()
	f.simplifyCFG()
}

// simplifyCFG bypasses empty blocks and merges straight-line chains
func (f *irFunc) simplifyCFG() {
	for changed := true; changed; {
		changed = false
		for _, x := range f.Blocks {
			if x == f.Entry || x.Kind != bJump || len(x.Phis) > 0 || len(x.Instrs) > 0 {
				continue
			}
			s := x.Succs[0]
			if s == x || len(x.Preds) == 0 {
				continue
			}
			// no second edge between the same blocks: phi operands are
			// matched to edges by their order
			dup := false
			for _, p := range x.Preds {
				n := 0
				for _, q := range p.Succs {
					if q == x || q == s {
						n++
					}
				}
				if n > 1 {
					dup = true
				}
			}
			if dup {
				continue
			}
			xi := -1
			for i, p := range s.Preds {
				if p == x {
					xi = i
					break
				}
			}
			for k, p := range x.Preds {
				for i := range p.Succs {
					if p.Succs[i] == x {
						p.Succs[i] = s
					}
				}
				if k == 0 {
					s.Preds[xi] = p
					continue
				}
				s.Preds = append(s.Preds, p)
				for _, phi := range s.Phis {
					phi.Args = append(phi.Args, phi.Args[xi])
				}
			}
			x.Preds = nil
			x.Succs = nil
			x.dbgFwd = s
			changed = true
		}
		// merge a block into its only predecessor when that has no other successor
		for _, b := range f.Blocks {
			if b.Kind != bJump || len(b.Succs) != 1 {
				continue
			}
			s := b.Succs[0]
			if s == b || s == f.Entry || len(s.Preds) != 1 || len(s.Phis) > 0 {
				continue
			}
			for _, v := range s.Instrs {
				v.Block = b
			}
			s.dbgFwd = b
			if n := len(b.Instrs); n > 0 {
				s.dbgAfter = b.Instrs[n-1]
			}
			b.Instrs = append(b.Instrs, s.Instrs...)
			b.Kind, b.Ctrl, b.Succs, b.RetVals = s.Kind, s.Ctrl, s.Succs, s.RetVals
			for _, t := range s.Succs {
				for i := range t.Preds {
					if t.Preds[i] == s {
						t.Preds[i] = b
					}
				}
			}
			s.Preds, s.Succs, s.Instrs = nil, nil, nil
			s.Kind = bJump
			changed = true
		}
		f.removeUnreachable()
	}
}

// forEachArg visits every operand slot of the function
func (f *irFunc) forEachArg(fn func(p **irValue)) {
	for _, b := range f.Blocks {
		for _, v := range b.Phis {
			for i := range v.Args {
				fn(&v.Args[i])
			}
		}
		for _, v := range b.Instrs {
			for i := range v.Args {
				fn(&v.Args[i])
			}
		}
		if b.Ctrl != nil {
			fn(&b.Ctrl)
		}
		for i := range b.RetVals {
			fn(&b.RetVals[i])
		}
	}
}

// cleanup resolves replaced values, drops them and removes trivial phis
func (f *irFunc) cleanup() {
	for {
		f.forEachArg(func(p **irValue) { *p = (*p).resolve() })
		changed := false
		for _, b := range f.Blocks {
			phis := b.Phis[:0]
			for _, v := range b.Phis {
				if v.fwd != nil {
					continue
				}
				if r := tryRemoveTrivialPhi(v); r != v {
					changed = true
					continue
				}
				phis = append(phis, v)
			}
			b.Phis = phis
			ins := b.Instrs[:0]
			for _, v := range b.Instrs {
				if v.fwd == nil {
					ins = append(ins, v)
				}
			}
			b.Instrs = ins
		}
		if !changed {
			f.forEachArg(func(p **irValue) { *p = (*p).resolve() })
			return
		}
	}
}

// removeUnreachable drops blocks not reachable from the entry
func (f *irFunc) removeUnreachable() {
	seen := map[*irBlock]bool{}
	var visit func(b *irBlock)
	visit = func(b *irBlock) {
		if seen[b] {
			return
		}
		seen[b] = true
		for _, s := range b.Succs {
			visit(s)
		}
	}
	visit(f.Entry)
	var keep []*irBlock
	for _, b := range f.Blocks {
		if seen[b] {
			keep = append(keep, b)
			continue
		}
		for _, s := range b.Succs {
			if seen[s] {
				s.removePred(b)
			}
		}
	}
	f.Blocks = keep
}

// removePred removes every edge from p and the matching phi operands
func (b *irBlock) removePred(p *irBlock) {
	for i := 0; i < len(b.Preds); {
		if b.Preds[i] != p {
			i++
			continue
		}
		b.Preds = append(b.Preds[:i], b.Preds[i+1:]...)
		for _, phi := range b.Phis {
			phi.Args = append(phi.Args[:i], phi.Args[i+1:]...)
		}
	}
}

// ---- dominators and loops ----

// analyze computes the reverse postorder, dominators and loop depths
func (f *irFunc) analyze() {
	var order []*irBlock
	seen := map[*irBlock]bool{}
	var visit func(b *irBlock)
	visit = func(b *irBlock) {
		seen[b] = true
		// the first successor (the taken path of a condition) ends up
		// right after the block in reverse postorder
		for i := len(b.Succs) - 1; i >= 0; i-- {
			if s := b.Succs[i]; !seen[s] {
				visit(s)
			}
		}
		order = append(order, b)
	}
	visit(f.Entry)
	for i, j := 0, len(order)-1; i < j; i, j = i+1, j-1 {
		order[i], order[j] = order[j], order[i]
	}
	for i, b := range order {
		b.rpo = i
		b.idom = nil
		b.depth = 0
	}
	f.Blocks = order
	// Cooper, Harvey, Kennedy
	f.Entry.idom = f.Entry
	for changed := true; changed; {
		changed = false
		for _, b := range order[1:] {
			var nd *irBlock
			for _, p := range b.Preds {
				if p.idom == nil {
					continue
				}
				if nd == nil {
					nd = p
					continue
				}
				nd = intersect(p, nd)
			}
			if nd != b.idom {
				b.idom = nd
				changed = true
			}
		}
	}
	// number the dominator tree for dominates()
	children := map[*irBlock][]*irBlock{}
	for _, b := range order[1:] {
		children[b.idom] = append(children[b.idom], b)
	}
	clock := 0
	var number func(b *irBlock)
	number = func(b *irBlock) {
		b.domIn = clock
		clock++
		for _, c := range children[b] {
			number(c)
		}
		b.domOut = clock
		clock++
	}
	number(f.Entry)
	// natural loops of back edges
	for _, h := range order {
		in := map[*irBlock]bool{}
		for _, p := range h.Preds {
			if !dominates(h, p) {
				continue
			}
			for _, blk := range loopBody(h, p) {
				if !in[blk] {
					in[blk] = true
					blk.depth++
				}
			}
		}
	}
}

func intersect(a, b *irBlock) *irBlock {
	for a != b {
		for a.rpo > b.rpo {
			a = a.idom
		}
		for b.rpo > a.rpo {
			b = b.idom
		}
	}
	return a
}

// dominates uses the numbering of the dominator tree made by analyze
func dominates(a, b *irBlock) bool {
	return a.domIn <= b.domIn && b.domOut <= a.domOut
}

// loopBody returns the blocks of the natural loop of the back edge latch->h
func loopBody(h, latch *irBlock) []*irBlock {
	in := map[*irBlock]bool{h: true}
	body := []*irBlock{h}
	stack := []*irBlock{latch}
	for len(stack) > 0 {
		b := stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		if in[b] {
			continue
		}
		in[b] = true
		body = append(body, b)
		stack = append(stack, b.Preds...)
	}
	return body
}

// ---- constant folding and simplification ----

func (f *irFunc) replace(v, w *irValue) {
	if v != w {
		v.fwd = w
	}
}

func constOf(v *irValue) (int64, bool) {
	if v.Op == opConst && (v.Type == iI32 || v.Type == iI64) {
		return v.AuxInt, true
	}
	return 0, false
}

func fconstOf(v *irValue) (float64, bool) {
	if v.Op == opConst && v.Type.isFloat() {
		return v.F, true
	}
	return 0, false
}

// wrap normalizes an integer to the value type
func wrap(t irType, x int64) int64 {
	if t == iI32 {
		return int64(int32(x))
	}
	return x
}

func (f *irFunc) setConst(v *irValue, c int64) {
	v.Op = opConst
	v.AuxInt = wrap(v.Type, c)
	v.Args = nil
}

func (f *irFunc) setFConst(v *irValue, c float64) {
	if v.Type == iF32 {
		c = float64(float32(c))
	}
	v.Op = opConst
	v.F = c
	v.Args = nil
}

func (f *irFunc) fold() {
	for _, b := range f.Blocks {
		// foldValue may insert instructions before the current one
		for i := 0; i < len(b.Instrs); i++ {
			v := b.Instrs[i]
			for i, a := range v.Args {
				v.Args[i] = a.resolve()
			}
			f.foldValue(v)
		}
	}
	f.cleanup()
}

// before inserts a new instruction in front of v
func (f *irFunc) before(v *irValue, op irOp, t irType, args ...*irValue) *irValue {
	w := f.newValue(v.Block, op, t, args...)
	b := v.Block
	for i, x := range b.Instrs {
		if x == v {
			b.Instrs = append(b.Instrs[:i], append([]*irValue{w}, b.Instrs[i:]...)...)
			return w
		}
	}
	panic("before: value not in its block")
}

func (f *irFunc) constBefore(v *irValue, t irType, c int64) *irValue {
	k := f.before(v, opConst, t)
	k.AuxInt = wrap(t, c)
	return k
}

// divByConst replaces a division or remainder by a constant (not a power
// of two, at least 3) with multiplications: 32-bit operands only.
// q = mulhi(x, ceil(2^64/c)) is exact for |x| < 2^32; signed quotients
// of negative x are one too small and get corrected.
func (f *irFunc) divByConst(v *irValue, c int64) bool {
	x := v.Args[0]
	if v.Type != iI32 || c < 3 || c&(c-1) == 0 || c > math.MaxInt32 {
		return false
	}
	signed := v.Op == opDiv || v.Op == opMod
	m := ^uint64(0)/uint64(c) + 1 // ceil(2^64/c) as c is not a power of two
	var x64 *irValue
	if signed {
		x64 = f.before(v, opSExt32, iI64, x)
	} else {
		x64 = f.before(v, opZExt32, iI64, x)
	}
	q := f.before(v, opMulHi, iI64, x64, f.constBefore(v, iI64, int64(m)))
	q.Signed = signed
	if signed {
		neg := f.before(v, opShr, iI64, x64, f.constBefore(v, iI64, 63))
		q = f.before(v, opAdd, iI64, q, neg)
	}
	q32 := f.before(v, opTrunc, iI32, q)
	if v.Op == opDiv || v.Op == opUDiv {
		f.replace(v, q32)
		return true
	}
	prod := f.before(v, opMul, iI32, q32, f.constBefore(v, iI32, c))
	v.Op = opSub
	v.Args = []*irValue{x, prod}
	return true
}

// divByPow2 handles signed division and remainder by 2^k
func (f *irFunc) divByPow2(v *irValue, c int64) bool {
	if c < 2 || c&(c-1) != 0 || (v.Op != opDiv && v.Op != opMod) {
		return false
	}
	x := v.Args[0]
	t := v.Type
	bitsN := int64(t.size() * 8)
	k := int64(bits.TrailingZeros64(uint64(c)))
	// bias = (x >> (n-1)) >>> (n-k): c-1 for negative x, else 0
	sign := f.before(v, opSar, t, x, f.constBefore(v, t, bitsN-1))
	bias := f.before(v, opShr, t, sign, f.constBefore(v, t, bitsN-k))
	adj := f.before(v, opAdd, t, x, bias)
	if v.Op == opDiv {
		v.Op = opSar
		v.Args = []*irValue{adj, f.constBefore(v, t, k)}
		return true
	}
	// x - ((x + bias) & -c)
	low := f.before(v, opAnd, t, adj, f.constBefore(v, t, -c))
	v.Op = opSub
	v.Args = []*irValue{x, low}
	return true
}

func boolInt(b bool) int64 {
	if b {
		return 1
	}
	return 0
}

func (f *irFunc) foldValue(v *irValue) {
	if v.fwd != nil || !v.Op.pure() || v.Op == opConst || len(v.Args) == 0 {
		return
	}
	if v.Op == op128 {
		f.fold128(v)
		return
	}
	a := v.Args[0]
	ca, aok := constOf(a)
	if len(v.Args) == 1 {
		if aok {
			switch v.Op {
			case opNeg:
				f.setConst(v, -ca)
			case opNot:
				f.setConst(v, ^ca)
			case opSExt8:
				f.setConst(v, int64(int8(ca)))
			case opSExt16:
				f.setConst(v, int64(int16(ca)))
			case opZExt8:
				f.setConst(v, int64(uint8(ca)))
			case opZExt16:
				f.setConst(v, int64(uint16(ca)))
			case opSExt32:
				f.setConst(v, int64(int32(ca)))
			case opZExt32:
				f.setConst(v, int64(uint32(ca)))
			case opTrunc:
				f.setConst(v, int64(int32(ca)))
			case opCvtI2F:
				if a.Type == iI32 {
					ca = int64(int32(ca))
				}
				if v.Type == iF32 {
					f.setFConst(v, float64(float32(ca)))
				} else {
					f.setFConst(v, float64(ca))
				}
			case opCvtU2F:
				if v.Type == iF32 {
					f.setFConst(v, float64(float32(uint64(ca))))
				} else {
					f.setFConst(v, float64(uint64(ca)))
				}
			}
			return
		}
		if fa, ok := fconstOf(a); ok {
			switch v.Op {
			case opFNeg:
				f.setFConst(v, -fa)
			case opFExt, opFTrunc:
				f.setFConst(v, fa)
			case opCvtF2I:
				// only exact, in-range values: the rest is left to the CPU
				if fa == math.Trunc(fa) && math.Abs(fa) < 1<<31 {
					f.setConst(v, int64(fa))
				}
			}
			return
		}
		// an extension of an already extended value
		switch v.Op {
		case opSExt8, opZExt8, opSExt16, opZExt16:
			if a.Op == v.Op {
				f.replace(v, a)
			}
		case opTrunc:
			if a.Op == opSExt32 || a.Op == opZExt32 {
				f.replace(v, a.Args[0])
			}
		}
		return
	}
	c := v.Args[1]
	cc, cok := constOf(c)
	t := v.Type
	if v.Op.isCompare() {
		t = a.Type
	}
	if aok && cok {
		ua, uc := uint64(ca), uint64(cc)
		if t == iI32 {
			ua, uc = uint64(uint32(ca)), uint64(uint32(cc))
		}
		switch v.Op {
		case opAdd:
			f.setConst(v, ca+cc)
		case opSub:
			f.setConst(v, ca-cc)
		case opMul:
			f.setConst(v, ca*cc)
		case opAnd:
			f.setConst(v, ca&cc)
		case opOr:
			f.setConst(v, ca|cc)
		case opXor:
			f.setConst(v, ca^cc)
		case opShl:
			f.setConst(v, ca<<(uint64(cc)&uint64(t.size()*8-1)))
		case opShr:
			f.setConst(v, int64(ua>>(uint64(cc)&uint64(t.size()*8-1))))
		case opSar:
			f.setConst(v, ca>>(uint64(cc)&uint64(t.size()*8-1)))
		case opDiv, opMod:
			if cc == 0 || cc == -1 {
				return
			}
			if v.Op == opDiv {
				f.setConst(v, ca/cc)
			} else {
				f.setConst(v, ca%cc)
			}
		case opUDiv, opUMod:
			if uc == 0 {
				return
			}
			if v.Op == opUDiv {
				f.setConst(v, int64(ua/uc))
			} else {
				f.setConst(v, int64(ua%uc))
			}
		case opEq:
			f.setConst(v, boolInt(ca == cc))
		case opNe:
			f.setConst(v, boolInt(ca != cc))
		case opLt:
			f.setConst(v, boolInt(ca < cc))
		case opLe:
			f.setConst(v, boolInt(ca <= cc))
		case opGt:
			f.setConst(v, boolInt(ca > cc))
		case opGe:
			f.setConst(v, boolInt(ca >= cc))
		case opULt:
			f.setConst(v, boolInt(ua < uc))
		case opULe:
			f.setConst(v, boolInt(ua <= uc))
		case opUGt:
			f.setConst(v, boolInt(ua > uc))
		case opUGe:
			f.setConst(v, boolInt(ua >= uc))
		}
		return
	}
	fa, faok := fconstOf(a)
	fc, fcok := fconstOf(c)
	if faok && fcok {
		switch v.Op {
		case opFAdd:
			f.setFConst(v, fa+fc)
		case opFSub:
			f.setFConst(v, fa-fc)
		case opFMul:
			f.setFConst(v, fa*fc)
		case opFDiv:
			f.setFConst(v, fa/fc)
		}
		return
	}
	// constants go to the right of commutative operations
	if aok && !cok && v.Op.commutative() {
		v.Args[0], v.Args[1] = c, a
		a, c = c, a
		ca, cc = cc, ca
		aok, cok = false, true
	}
	if !cok {
		if a == c {
			switch v.Op {
			case opSub, opXor:
				f.setConst(v, 0)
			case opAnd, opOr:
				f.replace(v, a)
			case opEq, opLe, opGe, opULe, opUGe:
				f.setConst(v, 1)
			case opNe, opLt, opGt, opULt, opUGt:
				f.setConst(v, 0)
			}
		}
		return
	}
	switch v.Op {
	case opAdd, opSub, opOr, opXor, opShl, opShr, opSar:
		if cc == 0 {
			f.replace(v, a)
			return
		}
		if v.Op == opSub && cc != math.MinInt64 {
			v.Op = opAdd
			v.Args[1] = f.newConstLike(v, -cc)
			cc = wrap(t, -cc)
		}
		// (x + c1) + c2
		if v.Op == opAdd && a.Op == opAdd {
			if c1, ok := constOf(a.Args[1]); ok {
				v.Args[0] = a.Args[0]
				v.Args[1] = f.newConstLike(v, wrap(t, c1+cc))
			}
		}
	case opMul:
		switch {
		case cc == 0:
			f.setConst(v, 0)
		case cc == 1:
			f.replace(v, a)
		case cc > 0 && cc&(cc-1) == 0:
			v.Op = opShl
			v.Args[1] = f.newConstLike(v, int64(bits.TrailingZeros64(uint64(cc))))
		}
	case opAnd:
		if cc == 0 {
			f.setConst(v, 0)
		} else if cc == -1 || t == iI32 && uint32(cc) == math.MaxUint32 {
			f.replace(v, a)
		}
	case opUDiv:
		if t == iI32 {
			cc = int64(uint32(cc))
		}
		if cc > 0 && cc&(cc-1) == 0 {
			v.Op = opShr
			v.Args[1] = f.newConstLike(v, int64(bits.TrailingZeros64(uint64(cc))))
			return
		}
		f.divByConst(v, cc)
	case opUMod:
		if t == iI32 {
			cc = int64(uint32(cc))
		}
		if cc > 0 && cc&(cc-1) == 0 {
			v.Op = opAnd
			v.Args[1] = f.newConstLike(v, cc-1)
			return
		}
		f.divByConst(v, cc)
	case opDiv:
		if cc == 1 {
			f.replace(v, a)
			return
		}
		if !f.divByPow2(v, cc) {
			f.divByConst(v, cc)
		}
	case opMod:
		if !f.divByPow2(v, cc) {
			f.divByConst(v, cc)
		}
	case opNe:
		// (cmp) != 0 is the comparison itself
		if cc == 0 && a.Op.isCompare() {
			f.replace(v, a)
		}
	case opEq:
		if cc == 0 && a.Op.isCompare() && !a.Type.isFloat() {
			if inv, ok := invertCompare(a.Op); ok {
				v.Op = inv
				v.Args = []*irValue{a.Args[0], a.Args[1]}
			}
		}
	}
}

// invertCompare returns the negated comparison (integers only: float
// comparisons are not symmetric under negation because of NaN)
func invertCompare(op irOp) (irOp, bool) {
	switch op {
	case opEq:
		return opNe, true
	case opNe:
		return opEq, true
	case opLt:
		return opGe, true
	case opLe:
		return opGt, true
	case opGt:
		return opLe, true
	case opGe:
		return opLt, true
	case opULt:
		return opUGe, true
	case opULe:
		return opUGt, true
	case opUGt:
		return opULe, true
	case opUGe:
		return opULt, true
	}
	return op, false
}

// newConstLike makes a constant of the type of the second operand of v
// (constants are rematerialized where used, so the entry block holds it)
func (f *irFunc) newConstLike(v *irValue, c int64) *irValue {
	t := v.Args[1].Type
	k := f.newValue(f.Entry, opConst, t)
	k.AuxInt = wrap(t, c)
	f.Entry.Instrs = append(f.Entry.Instrs, k)
	return k
}

// foldBranches turns branches on constants into jumps
func (f *irFunc) foldBranches() {
	for _, b := range f.Blocks {
		if b.Kind != bBranch {
			continue
		}
		c, ok := constOf(b.Ctrl.resolve())
		if !ok {
			if b.Succs[0] == b.Succs[1] && len(b.Succs[0].Phis) == 0 {
				s := b.Succs[0]
				s.removeOnePred(b)
				b.Kind = bJump
				b.Ctrl = nil
				b.Succs = b.Succs[:1]
			}
			continue
		}
		keep, drop := b.Succs[0], b.Succs[1]
		if c == 0 {
			keep, drop = drop, keep
		}
		b.Kind = bJump
		b.Ctrl = nil
		b.Succs = []*irBlock{keep}
		if keep != drop {
			drop.removePred(b)
		} else {
			keep.removeOnePred(b)
		}
	}
}

// removeOnePred removes one edge from p
func (b *irBlock) removeOnePred(p *irBlock) {
	for i, q := range b.Preds {
		if q == p {
			b.Preds = append(b.Preds[:i], b.Preds[i+1:]...)
			for _, phi := range b.Phis {
				phi.Args = append(phi.Args[:i], phi.Args[i+1:]...)
			}
			return
		}
	}
}

// ---- global value numbering ----

type gvnKey struct {
	op     irOp
	t      irType
	aux    int64
	fbits  uint64
	sym    string
	slot   *irSlot
	signed bool
	data   string
	a0, a1 int
}

func (f *irFunc) gvn() {
	children := map[*irBlock][]*irBlock{}
	for _, b := range f.Blocks {
		if b != f.Entry && b.idom != nil {
			children[b.idom] = append(children[b.idom], b)
		}
	}
	table := map[gvnKey]*irValue{}
	// loads are reused along a chain of blocks with single predecessors
	// while nothing writes memory
	var walk func(b *irBlock, loads map[gvnKey]*irValue)
	walk = func(b *irBlock, loads map[gvnKey]*irValue) {
		var added []gvnKey
		for _, v := range b.Instrs {
			for i, a := range v.Args {
				v.Args[i] = a.resolve()
			}
			if v.Op == opLoad && !v.vol && len(v.Args) == 1 {
				k := gvnKey{op: opLoad, t: v.Type, aux: v.AuxInt, signed: v.Signed, a0: v.Args[0].ID, a1: -1}
				if w, ok := loads[k]; ok {
					v.fwd = w
				} else {
					loads[k] = v
				}
				continue
			}
			if !v.Op.pure() {
				if v.Op != opLoad {
					clear(loads)
				}
				continue
			}
			if len(v.Args) > 2 {
				continue
			}
			k := gvnKey{op: v.Op, t: v.Type, aux: v.AuxInt, fbits: math.Float64bits(v.F), sym: v.Sym, slot: v.Slot, signed: v.Signed, data: string(v.Data), a0: -1, a1: -1}
			if len(v.Args) > 0 {
				k.a0 = v.Args[0].ID
			}
			if len(v.Args) > 1 {
				k.a1 = v.Args[1].ID
				if v.Op.commutative() && k.a1 < k.a0 {
					k.a0, k.a1 = k.a1, k.a0
				}
			}
			if w, ok := table[k]; ok {
				v.fwd = w
				continue
			}
			table[k] = v
			added = append(added, k)
		}
		for _, c := range children[b] {
			if len(c.Preds) == 1 && c.Preds[0] == b {
				walk(c, maps.Clone(loads))
			} else {
				walk(c, map[gvnKey]*irValue{})
			}
		}
		for _, k := range added {
			delete(table, k)
		}
	}
	walk(f.Entry, map[gvnKey]*irValue{})
	f.cleanup()
}

// ---- loop-invariant code motion ----

func (f *irFunc) licm() {
	for _, h := range f.Blocks {
		var pre *irBlock
		var latches []*irBlock
		for _, p := range h.Preds {
			if dominates(h, p) {
				latches = append(latches, p)
			} else if pre == nil {
				pre = p
			} else {
				pre = nil
				break
			}
		}
		if len(latches) == 0 || pre == nil || len(pre.Succs) != 1 || !pre.isPreHead {
			continue
		}
		in := map[*irBlock]bool{}
		var body []*irBlock
		for _, l := range latches {
			for _, b := range loopBody(h, l) {
				if !in[b] {
					in[b] = true
					body = append(body, b)
				}
			}
		}
		sort.Slice(body, func(i, j int) bool { return body[i].rpo < body[j].rpo })
		for _, b := range body {
			ins := b.Instrs[:0]
			for _, v := range b.Instrs {
				if v.Op.pure() && !v.Op.traps() && f.invariant(v, in) {
					v.Block = pre
					v.tok = nil // the line table keeps the code before (no jump back into the loop's lines)
					pre.Instrs = append(pre.Instrs, v)
					continue
				}
				ins = append(ins, v)
			}
			b.Instrs = ins
		}
	}
}

func (f *irFunc) invariant(v *irValue, in map[*irBlock]bool) bool {
	for _, a := range v.Args {
		if in[a.Block] {
			return false
		}
	}
	return true
}

// ---- dead code ----

func (f *irFunc) countUses() {
	for _, b := range f.Blocks {
		for _, v := range b.Phis {
			v.uses = 0
		}
		for _, v := range b.Instrs {
			v.uses = 0
		}
	}
	f.forEachArg(func(p **irValue) { (*p).uses++ })
}

func (f *irFunc) dce() {
	for {
		f.countUses()
		changed := false
		for _, b := range f.Blocks {
			phis := b.Phis[:0]
			for _, v := range b.Phis {
				if v.uses == 0 || v.uses == countSelf(v) {
					changed = true
					for _, a := range v.Args {
						a.uses--
					}
					continue
				}
				phis = append(phis, v)
			}
			b.Phis = phis
			ins := make([]*irValue, 0, len(b.Instrs))
			for i := len(b.Instrs) - 1; i >= 0; i-- {
				v := b.Instrs[i]
				if v.Op.pure() && v.uses == 0 {
					changed = true
					for _, a := range v.Args {
						a.uses--
					}
					continue
				}
				ins = append(ins, v)
			}
			for i, j := 0, len(ins)-1; i < j; i, j = i+1, j-1 {
				ins[i], ins[j] = ins[j], ins[i]
			}
			b.Instrs = ins
		}
		if !changed {
			return
		}
	}
}

// countSelf counts the uses of a phi by itself
func countSelf(phi *irValue) int {
	n := 0
	for _, a := range phi.Args {
		if a == phi {
			n++
		}
	}
	return n
}

// ---- addressing modes ----

// foldAddresses turns the address operands of loads and stores into
// x86 addressing modes
func (f *irFunc) foldAddresses() {
	for _, b := range f.Blocks {
		for _, v := range b.Instrs {
			switch v.Op {
			case opLoad:
				m, args := foldAddr(v.Args[0])
				v.Mem = m
				v.Args = args
			case opStore:
				m, args := foldAddr(v.Args[1])
				v.Mem = m
				v.Args = append([]*irValue{v.Args[0]}, args...)
			}
		}
	}
}

func foldAddr(a *irValue) (*irMem, []*irValue) {
	m := &irMem{Scale: 1}
	for {
		if a.Op == opAdd {
			if c, ok := constOf(a.Args[1]); ok && fitsI32(m.Disp+c) {
				m.Disp += c
				a = a.Args[0]
				continue
			}
		}
		break
	}
	switch a.Op {
	case opSlot:
		m.Slot = a.Slot
		return m, nil
	case opGlobal:
		m.Sym = a.Sym
		return m, nil
	case opAdd:
		base, idx := a.Args[0], a.Args[1]
		scale := 1
		if s, x, ok := scaled(idx); ok {
			idx, scale = x, s
		} else if s, x, ok := scaled(base); ok {
			base, idx, scale = idx, x, s
		}
		m.HasIndex = true
		m.Scale = scale
		if base.Op == opSlot {
			m.Slot = base.Slot
			return m, []*irValue{idx}
		}
		m.HasBase = true
		return m, []*irValue{base, idx}
	}
	m.HasBase = true
	return m, []*irValue{a}
}

// scaled matches x*1, x*2, x*4, x*8 (as multiplications or shifts)
func scaled(v *irValue) (int, *irValue, bool) {
	if v.Op != opShl && v.Op != opMul {
		return 0, nil, false
	}
	c, ok := constOf(v.Args[1])
	if !ok {
		return 0, nil, false
	}
	if v.Op == opShl {
		if c < 0 || c > 3 {
			return 0, nil, false
		}
		return 1 << c, v.Args[0], true
	}
	if c == 1 || c == 2 || c == 4 || c == 8 {
		return int(c), v.Args[0], true
	}
	return 0, nil, false
}
