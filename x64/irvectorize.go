package x64

import (
	"fmt"
	"math"
	"os"
)

// Loop vectorization. A loop with a unit-step induction variable, loads
// and stores of consecutive elements a[i] and element-wise arithmetic gets
// a vector copy processing W bytes per iteration (16 with SSE, 32 with
// AVX/AVX2). The original loop then runs the remaining iterations, at
// least one, so values used after the loop still come from it. Integer
// reductions (sum += a[i], -=, ^=, |=, &=) keep a vector of partial
// results; floating-point reductions (+, -, *) are vectorized only with
// -ffast-math because reassociating them changes the result. Before the
// vector loop, a run-time test checks that the stored arrays do not
// overlap the others.
//
// The body may branch (if, ?:, && and || without side effects in the
// branches): it is if-converted. Comparisons become vector masks, the
// values joining at a phi are blended with the masks of the incoming
// edges, and a reduction updated on one path adds the identity on the
// others. Loads and stores must be in blocks run by every iteration.
//
// For a one-block loop, a scalar copy first runs the iterations up to the
// point where the first stored array (or the first loaded one) is aligned
// to the vector size, so its vector accesses do not cross cache lines.

type vecAccess struct {
	v     *irValue // the load or store
	base  *irValue // invariant base address
	shift int64    // index scaling: (inv + i) << shift
	off   int64    // constant byte offset
	inv   *irValue // invariant part of the index (nil: none)
}

type vecReduction struct {
	phi  *irValue
	upd  *irValue // phi op x
	x    *irValue
	op   irOp
	join *irValue // the phi choosing upd or phi when the update is conditional
	edge int      // the incoming edge of join that carries upd (the others carry phi)
}

type vecLoop struct {
	h, pre, exit *irBlock
	latch        *irBlock          // the block branching back to h or out
	blocks       []*irBlock        // the body in topological order, h first
	inBody       map[*irBlock]bool // blocks of the body
	uncond       map[*irBlock]bool // blocks run by every iteration
	order        []*irValue        // body values in order (phis of inner blocks included)
	iv, ivNext   *irValue
	cond         *irValue
	limit        *irValue // the bound the induction variable is compared with
	elem         irType   // element type of the vector values
	esize        int
	loads        []vecAccess
	stores       []vecAccess
	reductions   []vecReduction
	accesses     map[*irValue]vecAccess
	masks        map[*irValue]bool // comparisons controlling branches of the body
	skip         map[*irValue]bool // values not copied into the vector loop
	hasInt       bool              // integer element operations (256 bits need AVX2)
}

func (f *irFunc) vectorize() bool {
	if f.noVectorize {
		return false
	}
	changed := false
	for _, h := range append([]*irBlock(nil), f.Blocks...) {
		if lp := f.vectorLoop(h); lp != nil {
			f.buildVectorLoop(lp)
			changed = true
		}
	}
	if changed {
		f.cleanup()
		f.removeUnreachable()
		f.analyze()
	}
	return changed
}

// inLoop tells whether v is computed inside the loop body
func (lp *vecLoop) inLoop(v *irValue) bool { return lp.inBody[v.Block] }

// loopBody finds the blocks of the loop headed by h: an acyclic body from
// h to the latch, which branches back to h or to the exit. It fails for
// other exits, inner loops and edges back to h from other blocks.
func (lp *vecLoop) loopBody(h *irBlock) bool {
	const maxBlocks = 16
	reached := map[*irBlock]bool{h: true}
	stack := []*irBlock{h}
	for len(stack) > 0 {
		b := stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		if b == lp.latch {
			continue
		}
		if b.Kind != bJump && b.Kind != bBranch || b.Kind == bBranch && b.Succs[0] == b.Succs[1] {
			return false
		}
		for _, s := range b.Succs {
			if s == h {
				return false // a continue edge or an inner loop
			}
			if !reached[s] {
				if len(reached) == maxBlocks {
					return false
				}
				reached[s] = true
				stack = append(stack, s)
			}
		}
	}
	if !reached[lp.latch] {
		return false
	}
	// every block must lead to the latch (no other exit)
	toLatch := map[*irBlock]bool{lp.latch: true}
	for changed := true; changed; {
		changed = false
		for b := range reached {
			if toLatch[b] || b == lp.latch {
				continue
			}
			for _, s := range b.Succs {
				if toLatch[s] {
					toLatch[b], changed = true, true
					break
				}
			}
		}
	}
	for b := range reached {
		if !toLatch[b] {
			return false
		}
		if b != h {
			for _, p := range b.Preds {
				if !reached[p] {
					return false // entered from outside
				}
			}
		}
	}
	// topological order (Kahn), without the back edge
	indeg := map[*irBlock]int{}
	for b := range reached {
		if b != h {
			indeg[b] = len(b.Preds)
		}
	}
	ready := []*irBlock{h}
	for len(ready) > 0 {
		b := ready[0]
		ready = ready[1:]
		lp.blocks = append(lp.blocks, b)
		if b == lp.latch {
			continue
		}
		for _, s := range b.Succs {
			indeg[s]--
			if indeg[s] == 0 {
				ready = append(ready, s)
			}
		}
	}
	if len(lp.blocks) != len(reached) || lp.blocks[len(lp.blocks)-1] != lp.latch {
		return false // a cycle inside the body
	}
	lp.inBody = reached
	// the blocks run by every iteration: those dominating the latch
	dom := map[*irBlock]map[*irBlock]bool{}
	for _, b := range lp.blocks {
		var d map[*irBlock]bool
		if b != h {
			for _, p := range b.Preds {
				if d == nil {
					d = map[*irBlock]bool{}
					for x := range dom[p] {
						d[x] = true
					}
					continue
				}
				for x := range d {
					if !dom[p][x] {
						delete(d, x)
					}
				}
			}
		}
		if d == nil {
			d = map[*irBlock]bool{}
		}
		d[b] = true
		dom[b] = d
	}
	lp.uncond = dom[lp.latch]
	for _, b := range lp.blocks {
		if b != h {
			lp.order = append(lp.order, b.Phis...)
		}
		lp.order = append(lp.order, b.Instrs...)
	}
	return true
}

// reductionUpdate matches phi op x for a reduction operation
func (f *irFunc) reductionUpdate(phi, next *irValue) (*irValue, bool) {
	switch next.Op {
	case opAdd, opXor, opOr, opAnd, opSub, opFAdd, opFSub, opFMul:
	default:
		return nil, false
	}
	float := next.Type.isFloat()
	if next.Type.isVec() || float != (next.Op == opFAdd || next.Op == opFSub || next.Op == opFMul) || float && !f.fastMath {
		return nil, false
	}
	if next.Args[0] == phi {
		return next.Args[1], next.Args[1] != phi
	}
	// phi - x only, not x - phi
	if next.Args[1] != phi || next.Op == opSub || next.Op == opFSub {
		return nil, false
	}
	return next.Args[0], true
}

// isCompare tells whether v is a comparison with a vector form
func isCompare(op irOp) bool {
	return op >= opEq && op <= opUGe || op >= opFEq && op <= opFGe
}

// vectorLoop checks that h is a vectorizable loop
func (f *irFunc) vectorLoop(h *irBlock) *vecLoop {
	if len(h.Preds) != 2 {
		return nil
	}
	lp := &vecLoop{h: h, accesses: map[*irValue]vecAccess{}, skip: map[*irValue]bool{}, masks: map[*irValue]bool{}}
	// the back edge comes from the latch, the other edge from the preheader
	pi := -1
	for i, p := range h.Preds {
		if p.Kind == bBranch && p.Succs[0] == h && p.Succs[1] != h {
			if pi >= 0 {
				return nil
			}
			pi = 1 - i
		}
	}
	if pi < 0 {
		return nil
	}
	lp.pre, lp.latch = h.Preds[pi], h.Preds[1-pi]
	lp.exit = lp.latch.Succs[1]
	if lp.pre == h || lp.pre == lp.latch || !lp.loopBody(h) || lp.inBody[lp.exit] || lp.inBody[lp.pre] {
		return nil
	}
	// the induction variable and the reductions
	for _, phi := range h.Phis {
		next := phi.Args[1-pi]
		if !lp.inLoop(next) {
			return nil
		}
		if next.Op == opAdd && next.Args[0] == phi {
			if c, ok := constOf(next.Args[1]); ok && c == 1 && lp.iv == nil && !next.Type.inXMM() && lp.uncond[next.Block] {
				lp.iv, lp.ivNext = phi, next
				continue
			}
		}
		if x, ok := f.reductionUpdate(phi, next); ok {
			lp.reductions = append(lp.reductions, vecReduction{phi: phi, upd: next, x: x, op: next.Op})
			continue
		}
		// a conditional update: join = phi(..., phi op x, ..., phi, ...)
		if next.Op == opPhi && next.Block != h {
			// one incoming edge carries the update, the others phi itself
			e := -1
			for i, a := range next.Args {
				if a != phi {
					if e >= 0 {
						e = -2
						break
					}
					e = i
				}
			}
			if e >= 0 && lp.inLoop(next.Args[e]) {
				u := next.Args[e]
				if x, ok := f.reductionUpdate(phi, u); ok {
					lp.reductions = append(lp.reductions, vecReduction{phi: phi, upd: u, x: x, op: u.Op, join: next, edge: e})
					lp.skip[next] = true
					continue
				}
			}
		}
		return nil
	}
	if lp.iv == nil {
		return nil
	}
	// the exit test: ivNext < limit (or !=), true stays in the loop
	c := lp.latch.Ctrl
	if c == nil || c.Block != lp.latch || len(c.Args) != 2 || c.Args[0] != lp.ivNext || lp.inLoop(c.Args[1]) {
		return nil
	}
	switch c.Op {
	case opLt, opULt, opNe:
	default:
		return nil
	}
	lp.cond, lp.limit = c, c.Args[1]
	lp.skip[lp.ivNext], lp.skip[c] = true, true
	for _, r := range lp.reductions {
		lp.skip[r.upd] = true
	}
	// the conditions of the branches inside the body
	for _, b := range lp.blocks {
		if b == lp.latch || b.Kind != bBranch {
			continue
		}
		if b.Ctrl == nil || b.Ctrl.Block != b || !isCompare(b.Ctrl.Op) {
			return nil
		}
		lp.masks[b.Ctrl] = true
	}
	// comparisons are only used by their branch
	for _, v := range lp.order {
		for _, a := range v.Args {
			if lp.masks[a] {
				return nil
			}
		}
	}

	// the body
	for _, v := range lp.order {
		if lp.skip[v] {
			continue
		}
		switch {
		case v.Op == opPhi:
			if len(v.Args) < 2 || len(v.Args) != len(v.Block.Preds) || !lp.setElem(v.Type) {
				return nil
			}
			for _, a := range v.Args {
				if !lp.vectorizable(a) {
					return nil
				}
			}
		case v.Op == opLoad || v.Op == opStore:
			// stores only where every iteration runs; conditional loads
			// are checked below
			if v.Mem != nil || v.Op == opStore && !lp.uncond[v.Block] {
				return nil
			}
			addr := v.Args[0]
			t := v.Type
			if v.Op == opStore {
				addr, t = v.Args[1], v.Args[0].Type
			}
			if v.vol || int(v.AuxInt) != t.size() || !lp.setElem(t) {
				return nil
			}
			acc, ok := lp.access(addr)
			if !ok || 1<<acc.shift != lp.esize {
				return nil
			}
			acc.v = v
			lp.accesses[v] = acc
			if v.Op == opLoad {
				lp.loads = append(lp.loads, acc)
			} else {
				lp.stores = append(lp.stores, acc)
				if !lp.vectorizable(v.Args[0]) {
					return nil
				}
			}
		case v.Op == opConst:
		case lp.masks[v]:
			t := v.Args[0].Type
			if !lp.setElem(t) || !lp.vectorizable(v.Args[0]) || !lp.vectorizable(v.Args[1]) {
				return nil
			}
			if t == iI64 && !f.isa.Has(ISASSE42) && !(v.Op == opEq || v.Op == opNe) || t == iI64 && !f.isa.Has(ISASSE41) {
				return nil // pcmpgtq needs SSE4.2, pcmpeqq SSE4.1
			}
		case lp.addressPart(v):
			lp.skip[v] = true
		case v.Op.pure():
			if !lp.setElem(v.Type) || vecMnemonic(v, f.isa) == "" {
				return nil
			}
			for _, a := range v.Args {
				if !lp.vectorizable(a) {
					return nil
				}
			}
		default:
			return nil
		}
	}
	for _, r := range lp.reductions {
		if !lp.setElem(r.upd.Type) || !lp.vectorizable(r.x) {
			return nil
		}
	}
	// a conditional load is run for every element only when an
	// unconditional access reads or writes the same elements: it cannot
	// fault then
	for _, l := range lp.loads {
		if lp.uncond[l.v.Block] {
			continue
		}
		safe := false
		for _, o := range append(append([]vecAccess(nil), lp.loads...), lp.stores...) {
			if lp.uncond[o.v.Block] && o.base == l.base && o.off == l.off && o.inv == l.inv && o.shift == l.shift {
				safe = true
				break
			}
		}
		if !safe {
			return nil
		}
	}
	if lp.elem == iNone || len(lp.stores) == 0 && len(lp.reductions) == 0 {
		return nil
	}
	// values used after the loop come from the original loop, which runs
	// at least once after the vector loop
	// stores through the same base must be the only accesses at their offset
	for _, s := range lp.stores {
		for _, l := range lp.loads {
			if l.base == s.base && (l.off != s.off || l.inv != s.inv) {
				return nil
			}
		}
		for _, s2 := range lp.stores {
			if s2.v != s.v && s2.base == s.base {
				return nil
			}
		}
	}
	if lp.width(f.isa) == 0 {
		return nil
	}
	return lp
}

// setElem records the element type of the loop; all vector values share
// the element size
func (lp *vecLoop) setElem(t irType) bool {
	switch t {
	case iI32, iI64:
		lp.hasInt = true
	case iF32, iF64:
	default:
		return false
	}
	if lp.elem == iNone {
		lp.elem, lp.esize = t, t.size()
		return true
	}
	return t.size() == lp.esize
}

// vectorizable tells whether an operand has a vector form: computed in the
// loop by something vectorized, or invariant (broadcast)
func (lp *vecLoop) vectorizable(v *irValue) bool {
	if v == lp.iv || v == lp.ivNext {
		return false
	}
	if !lp.inLoop(v) {
		return v.Type.size() == lp.esize && (v.Type == iI32 || v.Type == iI64 || v.Type.isFloat())
	}
	for _, r := range lp.reductions {
		if v == r.phi || v == r.upd || v == r.join {
			return false
		}
	}
	if lp.masks[v] {
		return false
	}
	return !lp.skip[v] || v.Op == opConst
}

// ivIndex matches the scaled induction variable: (ext(iv) << k) or iv*s
func (lp *vecLoop) ivIndex(v *irValue) (int64, bool) {
	k, inv, ok := lp.ivIndexInv(v)
	return k, ok && inv == nil
}

// ivIndexInv also accepts ext(inv + iv) for signed indexes (no overflow,
// as C assumes), giving the invariant part
func (lp *vecLoop) ivIndexInv(v *irValue) (int64, *irValue, bool) {
	k := int64(0)
	switch v.Op {
	case opShl:
		c, ok := constOf(v.Args[1])
		if !ok || c < 0 || c > 3 {
			return 0, nil, false
		}
		k, v = c, v.Args[0]
	case opMul:
		c, ok := constOf(v.Args[1])
		if !ok || c <= 0 || c&(c-1) != 0 || c > 8 {
			return 0, nil, false
		}
		for 1<<k != c {
			k++
		}
		v = v.Args[0]
	}
	var inv *irValue
	if v.Op == opSExt32 && v.Args[0].Op == opAdd && lp.cond.Op == opLt {
		a := v.Args[0]
		for i := 0; i < 2; i++ {
			if a.Args[i] == lp.iv && !lp.inLoop(a.Args[1-i]) {
				return k, a.Args[1-i], true
			}
		}
	}
	if v.Op == opSExt32 && lp.cond.Op == opLt || v.Op == opZExt32 && lp.cond.Op == opULt {
		v = v.Args[0]
	}
	if v.Op == opAdd && v.Type == iI64 && lp.iv.Type == iI64 {
		for i := 0; i < 2; i++ {
			if v.Args[i] == lp.iv && !lp.inLoop(v.Args[1-i]) {
				return k, v.Args[1-i], true
			}
		}
	}
	if v != lp.iv {
		return 0, nil, false
	}
	return k, inv, true
}

// access matches base + (index << k) + off with an invariant base
func (lp *vecLoop) access(addr *irValue) (vecAccess, bool) {
	var acc vecAccess
	for addr.Op == opAdd {
		if c, ok := constOf(addr.Args[1]); ok {
			acc.off += c
			addr = addr.Args[0]
			continue
		}
		break
	}
	if addr.Op != opAdd {
		return acc, false
	}
	for i := 0; i < 2; i++ {
		base, idx := addr.Args[i], addr.Args[1-i]
		if lp.inLoop(base) {
			continue
		}
		if k, inv, ok := lp.ivIndexInv(idx); ok {
			acc.base, acc.shift, acc.inv = base, k, inv
			return acc, true
		}
	}
	return acc, false
}

// addressPart tells whether v only computes addresses of matched accesses
// (index scaling, extensions of the induction variable, base + index)
func (lp *vecLoop) addressPart(v *irValue) bool {
	switch v.Op {
	case opSExt32, opZExt32:
		_, _, ok := lp.ivIndexInv(v)
		return ok
	case opShl, opMul:
		_, _, ok := lp.ivIndexInv(v)
		return ok
	case opAdd:
		if _, _, ok := lp.ivIndexInv(v); ok {
			return true
		}
		if v.Args[0] == lp.iv || v.Args[1] == lp.iv {
			// inv + iv inside an index
			return v.Type == lp.iv.Type && (!lp.inLoop(v.Args[0]) || !lp.inLoop(v.Args[1]))
		}
		if _, _, ok := lp.ivIndexInv(v.Args[0]); ok {
			return !lp.inLoop(v.Args[1])
		}
		if _, _, ok := lp.ivIndexInv(v.Args[1]); ok {
			return !lp.inLoop(v.Args[0])
		}
		if c, ok := constOf(v.Args[1]); ok {
			_ = c
			return lp.addressPart(v.Args[0])
		}
	}
	return false
}

// width is the vector size in bytes for the loop (0: not vectorized)
func (lp *vecLoop) width(isa ISA) int {
	if isa.Has(ISAAVX2) || isa.Has(ISAAVX) && !lp.hasInt {
		return 32
	}
	return 16
}

// vecMnemonic names the instruction of an element-wise operation
func vecMnemonic(v *irValue, isa ISA) string {
	t := v.Type
	if t.isFloat() {
		sfx := "ps"
		if t == iF64 {
			sfx = "pd"
		}
		switch v.Op {
		case opFAdd:
			return "add" + sfx
		case opFSub:
			return "sub" + sfx
		case opFMul:
			return "mul" + sfx
		case opFDiv:
			return "div" + sfx
		case opFNeg, opFAbs:
			return "xor" + sfx
		case opCvtI2F:
			if t == iF32 && v.Args[0].Type == iI32 {
				return "cvtdq2ps"
			}
		}
		return ""
	}
	sfx := "d"
	if t == iI64 {
		sfx = "q"
	}
	switch v.Op {
	case opAdd:
		return "padd" + sfx
	case opSub:
		return "psub" + sfx
	case opMul:
		if t == iI32 && (isa.Has(ISASSE41) || isa.Has(ISAAVX)) {
			return "pmulld"
		}
	case opAnd:
		return "pand"
	case opOr:
		return "por"
	case opXor:
		return "pxor"
	case opNeg:
		return "psub" + sfx
	case opNot:
		return "pxor"
	case opShl, opShr:
		if _, ok := constOf(v.Args[1]); ok {
			if v.Op == opShl {
				return "psll" + sfx
			}
			return "psrl" + sfx
		}
	case opSar:
		if _, ok := constOf(v.Args[1]); ok && t == iI32 {
			return "psrad"
		}
	case opCvtF2I:
		if t == iI32 && v.Args[0].Type == iF32 {
			return "cvttps2dq"
		}
	}
	return ""
}

// ---- building the vector loop ----

type vecBuilder struct {
	f      *irFunc
	b      *irBlock
	lp     *vecLoop
	vt     irType
	vf     int64
	pre    *irBlock // where invariants are broadcast
	vm     map[*irValue]*irValue
	bmasks map[*irBlock]*irValue // masks of the conditional blocks
}

func (vb *vecBuilder) add(b *irBlock, op irOp, t irType, args ...*irValue) *irValue {
	v := vb.f.newValue(b, op, t, args...)
	b.Instrs = append(b.Instrs, v)
	return v
}

func (vb *vecBuilder) iconst(b *irBlock, t irType, c int64) *irValue {
	v := vb.add(b, opConst, t)
	v.AuxInt = wrap(t, c)
	return v
}

func (vb *vecBuilder) vinsn(b *irBlock, t irType, mn string, args ...*irValue) *irValue {
	v := vb.add(b, opVInsn, t, args...)
	v.Sym = mn
	return v
}

func (vb *vecBuilder) vinsnImm(b *irBlock, t irType, mn string, imm int64, args ...*irValue) *irValue {
	v := vb.vinsn(b, t, mn, args...)
	v.Signed, v.AuxInt = true, imm
	return v
}

func (vb *vecBuilder) vconst(b *irBlock, data []byte) *irValue {
	v := vb.add(b, opConst, vb.vt)
	v.Data = data
	return v
}

func (vb *vecBuilder) elemKind() string {
	switch vb.lp.elem {
	case iF32:
		return "ps"
	case iF64:
		return "pd"
	case iI64:
		return "q"
	}
	return "d"
}

// splat broadcasts an invariant scalar (in the vector preheader)
func (vb *vecBuilder) splat(x *irValue) *irValue {
	if v, ok := vb.vm[x]; ok {
		return v
	}
	var v *irValue
	if x.Op == opConst {
		elem := make([]byte, vb.lp.esize)
		bitsv := uint64(x.AuxInt)
		if x.Type == iF32 {
			bitsv = uint64(math.Float32bits(float32(x.F)))
		} else if x.Type == iF64 {
			bitsv = math.Float64bits(x.F)
		}
		for i := range elem {
			elem[i] = byte(bitsv >> (8 * i))
		}
		v = vb.vconst(vb.pre, splat(vb.vt, elem))
	} else {
		v = vb.add(vb.pre, opVBroadcast, vb.vt, x)
		v.Sym = vb.elemKind()
	}
	vb.vm[x] = v
	return v
}

func (vb *vecBuilder) operand(x *irValue) *irValue {
	if v, ok := vb.vm[x]; ok {
		return v
	}
	if !vb.lp.inLoop(x) || x.Op == opConst {
		return vb.splat(x)
	}
	panic("vectorize: operand without a vector form")
}

// address computes base + (i << shift) + off for the vector index vi
func (vb *vecBuilder) address(acc vecAccess, vi *irValue) *irValue {
	b := vb.b
	idx := vi
	if vi.Type == iI32 {
		if vb.lp.cond.Op == opULt {
			idx = vb.add(b, opZExt32, iI64, vi)
		} else {
			idx = vb.add(b, opSExt32, iI64, vi)
		}
	}
	if acc.inv != nil {
		idx = vb.add(b, opAdd, iI64, idx, vb.lp.extInv(vb, b, acc.inv))
	}
	if acc.shift > 0 {
		idx = vb.add(b, opShl, iI64, idx, vb.iconst(b, iI64, acc.shift))
	}
	a := vb.add(b, opAdd, iI64, acc.base, idx)
	if acc.off != 0 {
		a = vb.add(b, opAdd, iI64, a, vb.iconst(b, iI64, acc.off))
	}
	return a
}

func (vb *vecBuilder) vectorOp(v *irValue) *irValue {
	b := vb.b
	mn := vecMnemonic(v, vb.f.isa)
	switch v.Op {
	case opFNeg, opFAbs:
		sign := make([]byte, vb.lp.esize)
		for i := range sign {
			sign[i] = 0
		}
		sign[len(sign)-1] = 0x80
		if v.Op == opFAbs {
			for i := range sign {
				sign[i] = 0xFF
			}
			sign[len(sign)-1] = 0x7F
			mn = "and" + mn[3:]
		}
		return vb.vinsn(b, vb.vt, mn, vb.operand(v.Args[0]), vb.vconst(vb.pre, splat(vb.vt, sign)))
	case opNeg:
		return vb.vinsn(b, vb.vt, mn, vb.vconst(vb.pre, make([]byte, vb.vt.size())), vb.operand(v.Args[0]))
	case opNot:
		ones := make([]byte, vb.vt.size())
		for i := range ones {
			ones[i] = 0xFF
		}
		return vb.vinsn(b, vb.vt, mn, vb.operand(v.Args[0]), vb.vconst(vb.pre, ones))
	case opShl, opShr, opSar:
		c, _ := constOf(v.Args[1])
		return vb.vinsnImm(b, vb.vt, mn, c&int64(vb.lp.esize*8-1), vb.operand(v.Args[0]))
	case opCvtI2F, opCvtF2I:
		return vb.vinsn(b, vb.vt, mn, vb.operand(v.Args[0]))
	}
	return vb.vinsn(b, vb.vt, mn, vb.operand(v.Args[0]), vb.operand(v.Args[1]))
}

// combineOp is the operation joining partial results of a reduction:
// differences are accumulated as negative sums
func combineOp(op irOp) irOp {
	switch op {
	case opSub:
		return opAdd
	case opFSub:
		return opFAdd
	}
	return op
}

// redMnemonic names the vector instruction of a reduction operation
func (vb *vecBuilder) redMnemonic(op irOp) string {
	k := vb.elemKind()
	switch op {
	case opAdd:
		return "padd" + k
	case opSub:
		return "psub" + k
	case opXor:
		return "pxor"
	case opOr:
		return "por"
	case opAnd:
		return "pand"
	case opFAdd:
		return "add" + k
	case opFSub:
		return "sub" + k
	case opFMul:
		return "mul" + k
	}
	panic("vectorize: reduction operation")
}

// redIdentity is the initial vector of partial results
func (vb *vecBuilder) redIdentity(op irOp) []byte {
	d := make([]byte, vb.vt.size())
	switch {
	case op == opAnd:
		for k := range d {
			d[k] = 0xFF
		}
	case op == opFMul && vb.lp.elem == iF32:
		d = splat(vb.vt, []byte{0, 0, 0x80, 0x3F})
	case op == opFMul:
		d = splat(vb.vt, []byte{0, 0, 0, 0, 0, 0, 0xF0, 0x3F})
	}
	return d
}

// reduceVector combines the lanes of a vector of partial results
func (vb *vecBuilder) reduceVector(b *irBlock, acc *irValue, op irOp) *irValue {
	mn := vb.redMnemonic(combineOp(op))
	v := acc
	if vb.vt == iV256 {
		ext := "extracti128"
		if vb.lp.elem.isFloat() {
			ext = "extractf128"
		}
		hi := vb.vinsnImm(b, iV128, ext, 1, v)
		lo := vb.vinsn(b, iV128, "", v)
		v = vb.vinsn(b, iV128, mn, lo, hi)
	}
	v = vb.vinsn(b, iV128, mn, v, vb.vinsnImm(b, iV128, "pshufd", 0x4E, v))
	switch vb.lp.elem {
	case iF32:
		v = vb.vinsn(b, iV128, mn, v, vb.vinsnImm(b, iV128, "pshufd", 0xB1, v))
		return vb.vinsn(b, iF32, "", v)
	case iF64:
		return vb.vinsn(b, iF64, "", v)
	case iI32:
		v = vb.vinsn(b, iV128, mn, v, vb.vinsnImm(b, iV128, "pshufd", 0xB1, v))
		return vb.vinsn(b, iI32, "movd", v)
	}
	return vb.vinsn(b, iI64, "movq", v)
}

// extInv widens the invariant part of an index like the index itself
func (lp *vecLoop) extInv(vb *vecBuilder, b *irBlock, inv *irValue) *irValue {
	if inv.Type == iI64 {
		return inv
	}
	return vb.add(b, opSExt32, iI64, inv)
}

func (f *irFunc) buildVectorLoop(lp *vecLoop) {
	if irStats {
		fmt.Fprintf(os.Stderr, "ir: %s: loop vectorized (%d-byte vectors)\n", f.Name, lp.width(f.isa))
	}
	h := lp.h
	pi := 0
	if h.Preds[0] == h {
		pi = 1
	}
	w := lp.width(f.isa)
	vt := iV128
	if w == 32 {
		vt = iV256
	}
	vf := int64(w / lp.esize)
	ivt := lp.iv.Type
	vb := &vecBuilder{f: f, lp: lp, vt: vt, vf: vf, vm: map[*irValue]*irValue{}}

	check := f.newBlock()
	vpre := f.newBlock()
	vbody := f.newBlock()
	middle := f.newBlock()
	rem := f.newBlock()
	vpre.isPreHead = true

	// pre -> check instead of pre -> h
	for i, s := range lp.pre.Succs {
		if s == h {
			lp.pre.Succs[i] = check
			break
		}
	}
	check.Preds = []*irBlock{lp.pre}
	h.Preds[pi] = rem
	rem.Succs = []*irBlock{h}
	rem.Kind = bJump

	// check: the vector iterations, leaving at least one for the loop
	start := lp.iv.Args[pi]
	ext := func(b *irBlock, x *irValue) *irValue {
		if x.Type == iI64 {
			return x
		}
		if lp.cond.Op == opULt {
			return vb.add(b, opZExt32, iI64, x)
		}
		return vb.add(b, opSExt32, iI64, x)
	}
	s64, n64 := ext(check, start), ext(check, lp.limit)
	lo := func(a vecAccess, x *irValue) *irValue {
		if a.inv != nil {
			x = vb.add(check, opAdd, iI64, x, lp.extInv(vb, check, a.inv))
		}
		idx := vb.add(check, opShl, iI64, x, vb.iconst(check, iI64, a.shift))
		v := vb.add(check, opAdd, iI64, a.base, idx)
		return vb.add(check, opAdd, iI64, v, vb.iconst(check, iI64, a.off))
	}
	cnt := vb.add(check, opSub, iI64, n64, s64)
	cnt1 := vb.add(check, opSub, iI64, cnt, vb.iconst(check, iI64, 1))
	// the peeled iterations: elements up to the next vector boundary
	pa := lp.loads
	if len(lp.stores) > 0 {
		pa = lp.stores
	}
	peel := vb.iconst(check, iI64, 0) // no arrays: nothing to align
	if len(pa) > 0 && len(lp.blocks) == 1 {
		neg := vb.add(check, opSub, iI64, peel, lo(pa[0], s64))
		bytes := vb.add(check, opAnd, iI64, neg, vb.iconst(check, iI64, int64(w-1)))
		peel = vb.add(check, opShr, iI64, bytes, vb.iconst(check, iI64, pa[0].shift))
		cnt1 = vb.add(check, opSub, iI64, cnt1, peel)
	}
	viters := vb.add(check, opAnd, iI64, cnt1, vb.iconst(check, iI64, -vf))
	ok := vb.add(check, opLt, iI32, vb.iconst(check, iI64, 0), viters)
	// the arrays written must not overlap the others
	for _, s := range lp.stores {
		others := append(append([]vecAccess(nil), lp.loads...), lp.stores...)
		for _, o := range others {
			if o.v == s.v || o.base == s.base && o.off == s.off && o.inv == s.inv {
				continue
			}
			slo, shi := lo(s, s64), lo(s, n64)
			olo, ohi := lo(o, s64), lo(o, n64)
			before := vb.add(check, opULe, iI32, shi, olo)
			after := vb.add(check, opULe, iI32, ohi, slo)
			sep := vb.add(check, opOr, iI32, before, after)
			ok = vb.add(check, opAnd, iI32, ok, sep)
		}
	}
	check.Kind = bBranch
	check.Ctrl = vb.add(check, opNe, iI32, ok, vb.iconst(check, iI32, 0))
	vstart64 := vb.add(check, opAdd, iI64, s64, peel)
	vecEnd64 := vb.add(check, opAdd, iI64, vstart64, viters)
	vstart, vecEnd := vstart64, vecEnd64
	if ivt == iI32 {
		vstart = vb.add(check, opTrunc, iI32, vstart64)
		vecEnd = vb.add(check, opTrunc, iI32, vecEnd64)
	}

	// the peeled iterations
	pguard := f.newBlock()
	check.Succs = []*irBlock{pguard, rem}
	pjoin := pguard
	var redInit []*irValue
	if len(lp.blocks) == 1 {
		pjoin = f.peelLoop(lp, vb, pguard, check, peel, vstart)
		redInit = pjoin.Phis
	} else {
		pguard.Preds = []*irBlock{check}
		for _, r := range lp.reductions {
			redInit = append(redInit, r.phi.Args[pi])
		}
	}

	// the vector preheader and loop
	vpre.Preds = []*irBlock{pjoin}
	pjoin.Kind, pjoin.Succs = bJump, []*irBlock{vpre}
	vpre.Kind, vpre.Succs = bJump, []*irBlock{vbody}
	vb.pre = vpre
	vbody.Preds = []*irBlock{vpre, vbody}
	vbody.Succs = []*irBlock{vbody, middle}
	vbody.Kind = bBranch
	vb.b = vbody

	vi := f.newValue(vbody, opPhi, ivt)
	vbody.Phis = append(vbody.Phis, vi)
	var vaccs []*irValue
	for _, r := range lp.reductions {
		phi := f.newValue(vbody, opPhi, vt)
		vbody.Phis = append(vbody.Phis, phi)
		vaccs = append(vaccs, phi)
		vb.vm[r.phi] = phi
	}
	for _, v := range lp.order {
		if lp.skip[v] || v.Op == opConst {
			continue
		}
		switch {
		case v.Op == opPhi:
			vb.vm[v] = vb.blend(v)
			continue
		case lp.masks[v]:
			vb.vm[v] = vb.compare(v)
			continue
		}
		switch v.Op {
		case opLoad:
			acc := lp.accesses[v]
			l := vb.add(vbody, opLoad, vt, vb.address(acc, vi))
			l.AuxInt = int64(w)
			vb.vm[v] = l
		case opStore:
			acc := lp.accesses[v]
			s := vb.add(vbody, opStore, iNone, vb.operand(v.Args[0]), vb.address(acc, vi))
			s.AuxInt = int64(w)
		default:
			vb.vm[v] = vb.vectorOp(v)
		}
	}
	var vupd []*irValue
	for i, r := range lp.reductions {
		x := vb.operand(r.x)
		if r.join != nil {
			// the identity on the paths that do not update
			j := r.join.Block
			x = vb.selectMask(vb.edgeMask(j.Preds[r.edge], j), x, vb.vconst(vpre, vb.redIdentity(r.op)))
		}
		vupd = append(vupd, vb.vinsn(vbody, vt, vb.redMnemonic(r.op), vaccs[i], x))
	}
	vnext := vb.add(vbody, opAdd, ivt, vi, vb.iconst(vbody, ivt, vf))
	vbody.Ctrl = vb.add(vbody, opNe, iI32, vnext, vecEnd)
	vi.Args = []*irValue{vstart, vnext}
	for i, r := range lp.reductions {
		vaccs[i].Args = []*irValue{vb.vconst(vpre, vb.redIdentity(r.op)), vupd[i]}
	}

	// middle: the reductions combined with the initial values
	middle.Preds = []*irBlock{vbody}
	middle.Kind, middle.Succs = bJump, []*irBlock{rem}
	var reduced []*irValue
	for i, r := range lp.reductions {
		s := vb.reduceVector(middle, vupd[i], r.op)
		reduced = append(reduced, vb.add(middle, combineOp(r.op), r.phi.Type, redInit[i], s))
	}

	// the remainder starts where the vector loop stopped
	rem.Preds = []*irBlock{check, middle}
	ivStart := f.newValue(rem, opPhi, ivt)
	ivStart.Args = []*irValue{start, vecEnd}
	rem.Phis = append(rem.Phis, ivStart)
	lp.iv.Args[pi] = ivStart
	for i, r := range lp.reductions {
		p := f.newValue(rem, opPhi, r.phi.Type)
		p.Args = []*irValue{r.phi.Args[pi], reduced[i]}
		rem.Phis = append(rem.Phis, p)
		r.phi.Args[pi] = p
	}
}

// peelLoop builds the scalar iterations before the vector loop: guard
// skips them when peel is 0, else a copy of the loop runs from the start
// to end. It returns the block after them, whose phis are the values of the
// reductions.
func (f *irFunc) peelLoop(lp *vecLoop, vb *vecBuilder, guard, check *irBlock, peel, end *irValue) *irBlock {
	h := lp.h
	pi := 0
	if h.Preds[0] == h {
		pi = 1
	}
	body := f.newBlock()
	join := f.newBlock()
	guard.Preds = []*irBlock{check}
	guard.Kind = bBranch
	guard.Ctrl = vb.add(guard, opNe, iI32, peel, vb.iconst(guard, iI64, 0))
	guard.Succs = []*irBlock{body, join}
	body.Preds = []*irBlock{guard, body}
	body.Kind = bBranch
	body.Succs = []*irBlock{body, join}
	join.Preds = []*irBlock{guard, body}

	m := map[*irValue]*irValue{}
	var phis []*irValue
	for _, phi := range h.Phis {
		np := f.newValue(body, opPhi, phi.Type)
		body.Phis = append(body.Phis, np)
		m[phi] = np
		phis = append(phis, phi)
	}
	arg := func(x *irValue) *irValue {
		if y, ok := m[x]; ok {
			return y
		}
		return x
	}
	for _, v := range h.Instrs {
		if v == lp.cond {
			continue
		}
		nv := f.newValue(body, v.Op, v.Type)
		nv.AuxInt, nv.F, nv.Sym, nv.Data, nv.Signed, nv.vol = v.AuxInt, v.F, v.Sym, v.Data, v.Signed, v.vol
		for _, a := range v.Args {
			nv.Args = append(nv.Args, arg(a))
		}
		body.Instrs = append(body.Instrs, nv)
		m[v] = nv
	}
	for _, phi := range phis {
		m[phi].Args = []*irValue{phi.Args[pi], arg(phi.Args[1-pi])}
	}
	body.Ctrl = vb.add(body, opNe, iI32, m[lp.ivNext], end)
	// the reductions after the peeled iterations
	for _, r := range lp.reductions {
		p := f.newValue(join, opPhi, r.phi.Type)
		p.Args = []*irValue{r.phi.Args[pi], m[r.upd]}
		join.Phis = append(join.Phis, p)
	}
	return join
}

// ---- if-conversion ----

// Masks are vectors of all-ones or all-zero elements; nil stands for all
// ones (a block run by every iteration). The bitwise operations use the ps
// forms, which AVX has for 256 bits without AVX2.

func (vb *vecBuilder) mand(x, y *irValue) *irValue {
	switch {
	case x == nil:
		return y
	case y == nil:
		return x
	}
	return vb.vinsn(vb.b, vb.vt, "andps", x, y)
}

// mandn is ~x & y
func (vb *vecBuilder) mandn(x, y *irValue) *irValue {
	if y == nil {
		ones := make([]byte, vb.vt.size())
		for i := range ones {
			ones[i] = 0xFF
		}
		return vb.vinsn(vb.b, vb.vt, "xorps", x, vb.vconst(vb.pre, ones))
	}
	return vb.vinsn(vb.b, vb.vt, "andnps", x, y)
}

func (vb *vecBuilder) mor(x, y *irValue) *irValue {
	if x == nil || y == nil {
		return nil
	}
	return vb.vinsn(vb.b, vb.vt, "orps", x, y)
}

// selectMask is m ? x : y element-wise
func (vb *vecBuilder) selectMask(m, x, y *irValue) *irValue {
	switch {
	case m == nil:
		return x
	case isZeroVec(y):
		return vb.mand(m, x)
	case isZeroVec(x):
		return vb.mandn(m, y)
	}
	return vb.mor(vb.mand(m, x), vb.mandn(m, y))
}

func isZeroVec(v *irValue) bool {
	if v.Op != opConst || v.Data == nil {
		return false
	}
	for _, b := range v.Data {
		if b != 0 {
			return false
		}
	}
	return true
}

// blockMask is the mask of the iterations running block b
func (vb *vecBuilder) blockMask(b *irBlock) *irValue {
	if vb.lp.uncond[b] {
		return nil
	}
	if m, ok := vb.bmasks[b]; ok {
		return m
	}
	var m *irValue
	for i, p := range b.Preds {
		e := vb.edgeMask(p, b)
		if i == 0 {
			m = e
		} else {
			m = vb.mor(m, e)
		}
	}
	if vb.bmasks == nil {
		vb.bmasks = map[*irBlock]*irValue{}
	}
	vb.bmasks[b] = m
	return m
}

// edgeMask is the mask of the iterations going from p to b
func (vb *vecBuilder) edgeMask(p, b *irBlock) *irValue {
	m := vb.blockMask(p)
	if p.Kind != bBranch {
		return m
	}
	c := vb.vm[p.Ctrl]
	if p.Succs[0] == b {
		return vb.mand(m, c)
	}
	return vb.mandn(c, m)
}

// blend converts a phi of the body into selections by the edge masks
func (vb *vecBuilder) blend(phi *irValue) *irValue {
	b := phi.Block
	n := len(phi.Args)
	v := vb.operand(phi.Args[n-1])
	for i := n - 2; i >= 0; i-- {
		v = vb.selectMask(vb.edgeMask(b.Preds[i], b), vb.operand(phi.Args[i]), v)
	}
	return v
}

// compare builds the mask of a comparison
func (vb *vecBuilder) compare(c *irValue) *irValue {
	b := vb.b
	t := c.Args[0].Type
	x, y := vb.operand(c.Args[0]), vb.operand(c.Args[1])
	if t.isFloat() {
		k := "ps"
		if t == iF64 {
			k = "pd"
		}
		// cmpps predicates: eq 0, lt 1, le 2, neq (unordered true) 4
		switch c.Op {
		case opFEq:
			return vb.vinsnImm(b, vb.vt, "cmp"+k, 0, x, y)
		case opFNe:
			return vb.vinsnImm(b, vb.vt, "cmp"+k, 4, x, y)
		case opFLt:
			return vb.vinsnImm(b, vb.vt, "cmp"+k, 1, x, y)
		case opFLe:
			return vb.vinsnImm(b, vb.vt, "cmp"+k, 2, x, y)
		case opFGt:
			return vb.vinsnImm(b, vb.vt, "cmp"+k, 1, y, x)
		case opFGe:
			return vb.vinsnImm(b, vb.vt, "cmp"+k, 2, y, x)
		}
		panic("vectorize: comparison")
	}
	k := "d"
	if t == iI64 {
		k = "q"
	}
	switch c.Op {
	case opEq:
		return vb.vinsn(b, vb.vt, "pcmpeq"+k, x, y)
	case opNe:
		return vb.mandn(vb.vinsn(b, vb.vt, "pcmpeq"+k, x, y), nil)
	}
	switch c.Op {
	case opULt, opULe, opUGt, opUGe:
		// flip the sign bits: unsigned order becomes signed order
		sign := make([]byte, t.size())
		sign[len(sign)-1] = 0x80
		sv := vb.vconst(vb.pre, splat(vb.vt, sign))
		x = vb.vinsn(b, vb.vt, "pxor", x, sv)
		y = vb.vinsn(b, vb.vt, "pxor", y, sv)
	}
	gt := func(p, q *irValue) *irValue { return vb.vinsn(b, vb.vt, "pcmpgt"+k, p, q) }
	switch c.Op {
	case opGt, opUGt:
		return gt(x, y)
	case opLt, opULt:
		return gt(y, x)
	case opLe, opULe:
		return vb.mandn(gt(x, y), nil)
	case opGe, opUGe:
		return vb.mandn(gt(y, x), nil)
	}
	panic("vectorize: comparison")
}
