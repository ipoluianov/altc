package x64

import (
	"fmt"
	"math/bits"
	"sort"
)

// Register allocation. Values get live ranges with holes over the linear
// block order. Phis are coalesced with their operands when the ranges do
// not interfere. Groups are then given registers in order of decreasing
// weight (uses weighted by loop depth); a call clobbers the caller-saved
// registers, so values living across calls get callee-saved registers or a
// stack slot. Spilled values are loaded into scratch registers where used.
//
// Positions: an instruction at p uses its operands at p, a call clobbers
// at p+1 and the result is defined at p+2.

type locKind uint8

const (
	locNone locKind = iota
	locReg
	locStack
)

type irLoc struct {
	kind locKind
	reg  Reg
	off  int32 // rbp offset of a stack location
	slot *irSlot
}

func (l irLoc) String() string {
	switch l.kind {
	case locReg:
		return RegName(l.reg, 8)
	case locStack:
		return fmt.Sprintf("stack(%d)", l.off)
	}
	return "-"
}

type seg struct{ from, to int }

// addSeg inserts [from, to) into a sorted list of disjoint segments
func addSeg(segs []seg, from, to int) []seg {
	segs = append(segs, seg{from, to})
	sort.Slice(segs, func(i, j int) bool { return segs[i].from < segs[j].from })
	out := segs[:0]
	for _, s := range segs {
		if n := len(out); n > 0 && s.from <= out[n-1].to {
			if s.to > out[n-1].to {
				out[n-1].to = s.to
			}
			continue
		}
		out = append(out, s)
	}
	return out
}

func overlaps(a, b []seg) bool {
	i, j := 0, 0
	for i < len(a) && j < len(b) {
		if a[i].to <= b[j].from {
			i++
		} else if b[j].to <= a[i].from {
			j++
		} else {
			return true
		}
	}
	return false
}

var (
	intCallerSaved = []Reg{RSI, RDI, R8, R9}
	intCalleeSaved = []Reg{RBX, R12, R13, R14, R15}
	floatRegs      = []Reg{XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12}
)

type allocGroup struct {
	vals   []*irValue
	segs   []seg
	float  bool
	weight float64
	loc    irLoc
}

type irAlloc struct {
	f          *irFunc
	order      []*irBlock
	segs       map[*irValue][]seg
	own        map[*irValue][]seg // the segments of a group root before coalescing
	parent     map[*irValue]*irValue
	calls      []int
	asms       []int // inline assembly may use any register
	usedCallee []Reg
}

func (v *irValue) needsLoc() bool {
	return v.Type != iNone && !v.Op.remat() && !v.fused && v.uses > 0
}

// splitCriticalEdges gives every edge from a branch into a block with
// phis a block of its own, where the phi moves go
func (f *irFunc) splitCriticalEdges() {
	for _, b := range append([]*irBlock(nil), f.Blocks...) {
		if len(b.Succs) < 2 {
			continue
		}
		for i, s := range b.Succs {
			if len(s.Phis) == 0 {
				continue
			}
			e := f.newBlock()
			e.Kind = bJump
			e.sealed = true
			e.Preds = []*irBlock{b}
			e.Succs = []*irBlock{s}
			// the k-th edge b->s is the k-th occurrence of b in s.Preds
			k := 0
			for _, q := range b.Succs[:i] {
				if q == s {
					k++
				}
			}
			b.Succs[i] = e
			for j, p := range s.Preds {
				if p == b {
					if k == 0 {
						s.Preds[j] = e
						break
					}
					k--
				}
			}
		}
	}
}

// lastCode is the last instruction of b that emits code (constants and
// addresses are materialized where they are used)
func lastCode(b *irBlock) *irValue {
	for i := len(b.Instrs) - 1; i >= 0; i-- {
		if v := b.Instrs[i]; !v.Op.remat() {
			return v
		}
	}
	return nil
}

// allocate assigns locations to the values of f
func (f *irFunc) allocate() *irAlloc {
	f.splitCriticalEdges()
	f.analyze()
	f.countUses()
	// a comparison right before the branch that uses it only sets flags
	for _, b := range f.Blocks {
		if b.Kind != bBranch {
			continue
		}
		c := b.Ctrl
		if c.setsFlags() && c.Block == b && c.uses == 1 && lastCode(b) == c {
			c.fused = true
		}
	}
	ra := &irAlloc{f: f, order: f.Blocks, segs: map[*irValue][]seg{}, own: map[*irValue][]seg{}, parent: map[*irValue]*irValue{}}
	ra.number()
	ra.liveRanges()
	ra.coalesce()
	ra.assign()
	return ra
}

func (ra *irAlloc) number() {
	pos := 0
	for _, b := range ra.order {
		b.start = pos
		pos += 4
		for _, v := range b.Instrs {
			v.pos = pos
			if v.Op == opCall {
				ra.calls = append(ra.calls, pos)
			}
			if v.Op == opAsm {
				ra.asms = append(ra.asms, pos)
			}
			pos += 4
		}
		pos += 4 // the terminator
		b.end = pos
	}
}

type bitset []uint64

func newBitset(n int) bitset       { return make(bitset, (n+63)/64) }
func (s bitset) set(i int)         { s[i/64] |= 1 << (i % 64) }
func (s bitset) clear(i int)       { s[i/64] &^= 1 << (i % 64) }
func (s bitset) has(i int) bool    { return s[i/64]&(1<<(i%64)) != 0 }
func (s bitset) copyFrom(o bitset) { copy(s, o) }
func (s bitset) equal(o bitset) bool {
	for i := range s {
		if s[i] != o[i] {
			return false
		}
	}
	return true
}

func (ra *irAlloc) liveRanges() {
	// dense numbers for the values that need a location
	var vals []*irValue
	idx := map[*irValue]int{}
	number := func(v *irValue) {
		if v.needsLoc() {
			if _, ok := idx[v]; !ok {
				idx[v] = len(vals)
				vals = append(vals, v)
			}
		}
	}
	for _, b := range ra.order {
		for _, v := range b.Phis {
			number(v)
		}
		for _, v := range b.Instrs {
			number(v)
		}
	}
	n := len(vals)
	nb := len(ra.order)
	bi := map[*irBlock]int{}
	for i, b := range ra.order {
		bi[b] = i
	}
	uses := make([]bitset, nb)
	defs := make([]bitset, nb)
	phiUse := make([]bitset, nb) // operands of successor phis on edges from the block
	for i, b := range ra.order {
		u, d, pu := newBitset(n), newBitset(n), newBitset(n)
		for _, v := range b.Phis {
			if k, ok := idx[v]; ok {
				d.set(k)
			}
		}
		use := func(a *irValue) {
			if k, ok := idx[a]; ok && !d.has(k) {
				u.set(k)
			}
		}
		for _, v := range b.Instrs {
			for _, a := range v.Args {
				use(a)
			}
			if k, ok := idx[v]; ok {
				d.set(k)
			}
		}
		if b.Ctrl != nil {
			use(b.Ctrl)
		}
		for _, c := range b.RetVals {
			use(c)
		}
		for _, s := range b.Succs {
			for j, p := range s.Preds {
				if p != b {
					continue
				}
				for _, phi := range s.Phis {
					if k, ok := idx[phi.Args[j]]; ok {
						pu.set(k)
					}
				}
			}
		}
		uses[i], defs[i], phiUse[i] = u, d, pu
	}
	phiDefs := make([]bitset, nb)
	for i, b := range ra.order {
		pd := newBitset(n)
		for _, v := range b.Phis {
			if k, ok := idx[v]; ok {
				pd.set(k)
			}
		}
		phiDefs[i] = pd
	}
	in := make([]bitset, nb)
	out := make([]bitset, nb)
	for i := range ra.order {
		in[i], out[i] = newBitset(n), newBitset(n)
	}
	tmp := newBitset(n)
	for changed := true; changed; {
		changed = false
		for i := nb - 1; i >= 0; i-- {
			b := ra.order[i]
			o := out[i]
			copy(o, phiUse[i])
			for _, s := range b.Succs {
				si, pd := in[bi[s]], phiDefs[bi[s]]
				for k := range o {
					o[k] |= si[k] &^ pd[k]
				}
			}
			u, d := uses[i], defs[i]
			for k := range tmp {
				tmp[k] = u[k] | o[k]&^d[k]
			}
			if !tmp.equal(in[i]) {
				copy(in[i], tmp)
				changed = true
			}
		}
	}
	liveEnd := map[*irValue]int{}
	for i, b := range ra.order {
		clear(liveEnd)
		for w, word := range out[i] {
			for word != 0 {
				t := bits.TrailingZeros64(word)
				word &= word - 1
				liveEnd[vals[w*64+t]] = b.end
			}
		}
		for _, c := range append([]*irValue{b.Ctrl}, b.RetVals...) {
			if c != nil && c.needsLoc() {
				if _, ok := liveEnd[c]; !ok {
					liveEnd[c] = b.end - 3
				}
			}
		}
		for i := len(b.Instrs) - 1; i >= 0; i-- {
			v := b.Instrs[i]
			if v.needsLoc() {
				def := v.pos + 2
				if v.Op == opParam {
					def = b.start
				}
				if e, ok := liveEnd[v]; ok {
					ra.segs[v] = addSeg(ra.segs[v], def, e)
					delete(liveEnd, v)
				} else {
					ra.segs[v] = addSeg(ra.segs[v], def, def+1)
				}
			}
			for _, a := range v.Args {
				if !a.needsLoc() {
					continue
				}
				if _, ok := liveEnd[a]; !ok {
					liveEnd[a] = v.pos + 1
				}
			}
		}
		for _, phi := range b.Phis {
			if !phi.needsLoc() {
				continue
			}
			if e, ok := liveEnd[phi]; ok {
				ra.segs[phi] = addSeg(ra.segs[phi], b.start, e)
				delete(liveEnd, phi)
			} else {
				ra.segs[phi] = addSeg(ra.segs[phi], b.start, b.start+1)
			}
		}
		for v, e := range liveEnd {
			ra.segs[v] = addSeg(ra.segs[v], b.start, e)
		}
	}
}

func (ra *irAlloc) find(v *irValue) *irValue {
	for {
		p, ok := ra.parent[v]
		if !ok || p == v {
			return v
		}
		if gp, ok := ra.parent[p]; ok && gp != p {
			ra.parent[v] = gp
		}
		v = p
	}
}

// coalesce merges phis with their operands when their ranges are disjoint
func (ra *irAlloc) coalesce() {
	for _, b := range ra.order {
		for _, phi := range b.Phis {
			if !phi.needsLoc() {
				continue
			}
			for _, a := range phi.Args {
				if !a.needsLoc() || a.Type.inXMM() != phi.Type.inXMM() {
					continue
				}
				ra1, rb := ra.find(phi), ra.find(a)
				if ra1 == rb || overlaps(ra.segs[ra1], ra.segs[rb]) {
					continue
				}
				merged := ra.segs[ra1]
				if _, ok := ra.own[ra1]; !ok {
					ra.own[ra1] = merged
				}
				for _, s := range ra.segs[rb] {
					merged = addSeg(merged, s.from, s.to)
				}
				ra.parent[rb] = ra1
				ra.segs[ra1] = merged
			}
		}
	}
}

func (ra *irAlloc) assign() {
	groups := map[*irValue]*allocGroup{}
	var list []*allocGroup
	for _, b := range ra.order {
		for _, vs := range [][]*irValue{b.Phis, b.Instrs} {
			for _, v := range vs {
				if !v.needsLoc() {
					continue
				}
				root := ra.find(v)
				g := groups[root]
				if g == nil {
					g = &allocGroup{segs: ra.segs[root], float: v.Type.inXMM()}
					groups[root] = g
					list = append(list, g)
				}
				g.vals = append(g.vals, v)
				w := 1.0
				for d := 0; d < min(b.depth, 6); d++ {
					w *= 8
				}
				g.weight += w * float64(v.uses+1)
			}
		}
	}
	sort.SliceStable(list, func(i, j int) bool {
		if list[i].weight != list[j].weight {
			return list[i].weight > list[j].weight
		}
		return list[i].segs[0].from < list[j].segs[0].from
	})
	regSegs := map[Reg][]seg{}
	for _, p := range ra.calls {
		for _, r := range intCallerSaved {
			regSegs[r] = append(regSegs[r], seg{p + 1, p + 2})
		}
		for _, r := range floatRegs {
			regSegs[r] = append(regSegs[r], seg{p + 1, p + 2})
		}
	}
	for _, p := range ra.asms {
		for _, rs := range [][]Reg{intCallerSaved, intCalleeSaved, floatRegs} {
			for _, r := range rs {
				regSegs[r] = append(regSegs[r], seg{p + 1, p + 2})
			}
		}
	}
	for r, segs := range regSegs {
		sort.Slice(segs, func(i, j int) bool { return segs[i].from < segs[j].from })
		regSegs[r] = segs
	}
	usedCallee := map[Reg]bool{}
	if len(ra.asms) > 0 {
		for _, r := range intCalleeSaved {
			usedCallee[r] = true
		}
	}
	for _, g := range list {
		cands := floatRegs
		if !g.float {
			cands = append(append([]Reg(nil), intCallerSaved...), intCalleeSaved...)
		}
		g.loc = irLoc{}
		for _, r := range cands {
			if overlaps(g.segs, regSegs[r]) {
				continue
			}
			g.loc = irLoc{kind: locReg, reg: r}
			for _, s := range g.segs {
				regSegs[r] = addSeg(regSegs[r], s.from, s.to)
			}
			if !g.float && isCalleeSaved(r) {
				usedCallee[r] = true
			}
			break
		}
		if g.loc.kind == locNone {
			size := int64(8)
			for _, v := range g.vals {
				size = max(size, int64(v.Type.size()))
			}
			s := &irSlot{Size: size, Align: min(size, 16), Name: "spill"}
			ra.f.Slots = append(ra.f.Slots, s)
			g.loc = irLoc{kind: locStack, slot: s}
		}
		for _, v := range g.vals {
			v.loc = g.loc
		}
	}
	for _, r := range intCalleeSaved {
		if usedCallee[r] {
			ra.usedCallee = append(ra.usedCallee, r)
		}
	}
}

func isCalleeSaved(r Reg) bool {
	for _, c := range intCalleeSaved {
		if c == r {
			return true
		}
	}
	return false
}
