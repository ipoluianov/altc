package x64

import (
	"encoding/binary"
	"math"
	"sort"

	"github.com/ipoluianov/altc/cc"
)

// Debug information of functions compiled by the IR backend (-g): where
// each variable lives over the code (DWARF location lists), and the code
// that came from inlined calls (DW_TAG_inlined_subroutine).
//
// Lowering records every SSA value assigned to a named variable (and the
// phis merging them). After register allocation each such value has a
// location and live segments; the emitter maps the segment positions to
// code offsets. Where the values of two assignments are live at the same
// time, the later assignment describes the variable.

// dbgSite is an inlined call
type dbgSite struct {
	fn     *cc.Obj
	call   *cc.Token // the call expression
	outer  *dbgSite
	ranges [][2]int // code offsets [lo, hi)
	vars   map[*cc.Obj]bool
	mem    map[*cc.Obj]*irSlot // its variables kept in frame slots
	locs   []dbgVarLoc         // where its variables are
}

type dbgDef struct {
	v    *cc.Obj
	val  *irValue
	half int      // a 128-bit variable: 1 for the low half, 2 for the high half
	site *dbgSite // nil: a variable of the function itself
	// where the assignment is in the program: in blk after anchor (nil:
	// at the start of blk). Optimizations may compute the value earlier
	// (hoisted out of a loop, shared with another expression); the
	// variable only takes it here.
	blk     *irBlock
	anchors []*irValue // the instructions before it, the closest last
}

// dbgPiece is a part of a location list: the variable is described by
// expr for code offsets [lo, hi)
type dbgPiece struct {
	lo, hi int
	expr   []byte
}

// dbgVarLoc is where a variable is: a frame slot (rbp offset) for the
// whole function, or a location list
type dbgVarLoc struct {
	v      *cc.Obj
	inSlot bool
	off    int64
	list   []dbgPiece
}

// irDebug is the debug information of one IR-compiled function
type irDebug struct {
	vars  []dbgVarLoc
	sites []*dbgSite
}

// dbgVarSet is the set of named parameters and locals of fn
func dbgVarSet(fn *cc.Obj) map[*cc.Obj]bool {
	m := map[*cc.Obj]bool{}
	for _, v := range append(append([]*cc.Obj(nil), fn.Params...), fn.Locals...) {
		if v.Name != "" && v.Ty != nil && v.Tok != nil && v.RegName == "" && v.Ty.Kind != cc.TyVLA {
			m[v] = true
		}
	}
	return m
}

// recordDef notes that val is the value of variable v from here on
func (b *irBuilder) recordDef(v *cc.Obj, val *irValue) {
	if b.dbgVars == nil {
		return
	}
	half := 0
	if h, ok := b.halfOf[v]; ok {
		v, half = h.v, h.half
	}
	s := b.f.curSite
	if s != nil && !s.vars[v] || s == nil && !b.dbgVars[v] {
		return
	}
	d := dbgDef{v: v, val: val, half: half, site: s}
	if val.Op == opPhi {
		d.blk = val.Block
	} else if b.cur != nil {
		d.blk = b.cur
		in := b.cur.Instrs
		d.anchors = append([]*irValue(nil), in[max(0, len(in)-8):]...)
	}
	b.f.dbgDefs = append(b.f.dbgDefs, d)
}

// bindPos is the first position where the variable holds the value of an
// assignment (-1 when the place of the assignment was optimized away)
func (e *irEmitter) bindPos(d dbgDef, inOrder map[*irBlock]bool, live map[*irValue]bool) int {
	blk := d.blk
	anchors := d.anchors
	for i := 0; blk != nil && !inOrder[blk] && i < 1000; i++ {
		if a := blk.dbgAfter; a != nil {
			anchors = append([]*irValue{a}, anchors...) // the code before the merged block
		}
		blk = blk.dbgFwd
	}
	if blk == nil || !inOrder[blk] {
		return -1
	}
	// the closest instruction before the assignment still there (others
	// were removed or moved)
	for i := len(anchors) - 1; i >= 0; i-- {
		if a := anchors[i]; live[a] && a.Block == blk {
			return a.pos + 2
		}
	}
	return blk.start
}

// memVarLocs describes variables kept in frame slots
func memVarLocs(mem map[*cc.Obj]*irSlot) []dbgVarLoc {
	var vs []*cc.Obj
	for v := range mem {
		vs = append(vs, v)
	}
	sort.Slice(vs, func(i, j int) bool {
		x, y := vs[i].Tok, vs[j].Tok
		return x.Line < y.Line || x.Line == y.Line && (x.Col < y.Col || x.Col == y.Col && vs[i].Name < vs[j].Name)
	})
	var out []dbgVarLoc
	for _, v := range vs {
		out = append(out, dbgVarLoc{v: v, inSlot: true, off: mem[v].Offset})
	}
	return out
}

// ---- code offsets ----

// dbgEvent ties a position of the linear order to a code offset
type dbgEvent struct{ pos, pc int }

// pcAt is the code offset of the first event at or after position p
func pcAt(evs []dbgEvent, p int) int {
	i := sort.Search(len(evs), func(i int) bool { return evs[i].pos >= p })
	if i == len(evs) {
		return evs[len(evs)-1].pc
	}
	return evs[i].pc
}

// debugInfo builds the variable locations once the function is emitted
func (e *irEmitter) debugInfo() *irDebug {
	f := e.f
	d := &irDebug{sites: e.sites}
	evs := e.events
	if len(evs) == 0 {
		return d
	}
	// variables kept in memory
	d.vars = memVarLocs(f.dbgMem)
	for _, s := range e.sites {
		for ; s != nil; s = s.outer {
			if s.locs == nil {
				s.locs = append(memVarLocs(s.mem), dbgVarLoc{}) // the empty entry marks it done
			}
		}
	}
	// SSA variables: the live ranges of their values
	type cand struct {
		lo, hi int
		order  int // later assignments win
		expr   []byte
	}
	type key struct {
		v    *cc.Obj
		site *dbgSite
		half int
	}
	byVar := map[key][]cand{}
	var order []key
	inOrder := map[*irBlock]bool{}
	live := map[*irValue]bool{}
	for _, b := range e.ra.order {
		inOrder[b] = true
		for _, v := range b.Instrs {
			live[v] = true
		}
	}
	// where each assignment takes effect; it lasts until the next one of
	// the same variable (in the linear order)
	binds := make([]int, len(f.dbgDefs))
	byKey := map[key][]int{}
	for i, df := range f.dbgDefs {
		k := key{df.v, df.site, df.half}
		binds[i] = e.bindPos(df, inOrder, live)
		if _, ok := byKey[k]; !ok {
			order = append(order, k)
		}
		byKey[k] = append(byKey[k], i)
	}
	end := evs[len(evs)-1].pos + 1
	for _, k := range order {
		byVar[k] = []cand{}
		for _, i := range byKey[k] {
			df, bind := f.dbgDefs[i], binds[i]
			next := end
			for _, j := range byKey[k] {
				if bind >= 0 && binds[j] > bind && binds[j] < next {
					next = binds[j]
				}
			}
			add := func(from, to, prio int, expr []byte) {
				lo, hi := pcAt(evs, from), pcAt(evs, to)
				if lo < hi {
					byVar[k] = append(byVar[k], cand{lo, hi, prio, expr})
				}
			}
			val := df.val.resolve()
			if bind >= 0 {
				// the variable has this value (or an unknown one) until the
				// next assignment
				add(bind, next, 2*i, e.dbgConst(val))
			}
			expr := e.dbgExpr(val)
			if expr == nil {
				continue
			}
			segs, ok := e.ra.own[val]
			if !ok {
				segs = e.ra.segs[val]
			}
			for _, sg := range segs {
				from, to := sg.from, sg.to+1
				if bind >= 0 {
					from, to = max(from, bind), min(to, next)
				}
				if from < to {
					add(from, to, 2*i+1, expr)
				}
			}
		}
	}
	halfLists := map[key][2][]dbgPiece{}
	var halfOrder []key
	for _, k := range order {
		cs := byVar[k]
		// cut the code into intervals at every boundary; each takes the
		// latest assignment live there
		var cuts []int
		for _, c := range cs {
			cuts = append(cuts, c.lo, c.hi)
		}
		sort.Ints(cuts)
		var list []dbgPiece
		for k := 0; k+1 < len(cuts); k++ {
			lo, hi := cuts[k], cuts[k+1]
			if lo == hi {
				continue
			}
			best := -1
			for j, c := range cs {
				if c.lo <= lo && hi <= c.hi && (best < 0 || c.order > cs[best].order) {
					best = j
				}
			}
			if best < 0 || cs[best].expr == nil {
				continue // not known here
			}
			ex := cs[best].expr
			if n := len(list); n > 0 && list[n-1].hi == lo && string(list[n-1].expr) == string(ex) {
				list[n-1].hi = hi
				continue
			}
			list = append(list, dbgPiece{lo, hi, ex})
		}
		if k.half != 0 {
			// a half of a 128-bit variable: combined below
			w := key{k.v, k.site, 0}
			if _, ok := halfLists[w]; !ok {
				halfOrder = append(halfOrder, w)
			}
			hl := halfLists[w]
			hl[k.half-1] = list
			halfLists[w] = hl
			continue
		}
		d.addVar(k.site, dbgVarLoc{v: k.v, list: list})
	}
	for _, w := range halfOrder {
		d.addVar(w.site, dbgVarLoc{v: w.v, list: pieces(halfLists[w], byte(w.v.Ty.Size()/2))})
	}
	return d
}

func (d *irDebug) addVar(site *dbgSite, l dbgVarLoc) {
	if site == nil {
		d.vars = append(d.vars, l)
	} else if site.locs != nil {
		site.locs = append(site.locs, l)
	}
}

// pieces combines the location lists of the halves of a 128-bit or
// complex variable: DW_OP_piece size after each (an empty piece: that half
// is not known there)
func pieces(h [2][]dbgPiece, size byte) []dbgPiece {
	var cuts []int
	for _, l := range h {
		for _, p := range l {
			cuts = append(cuts, p.lo, p.hi)
		}
	}
	sort.Ints(cuts)
	at := func(l []dbgPiece, lo, hi int) []byte {
		for _, p := range l {
			if p.lo <= lo && hi <= p.hi {
				return p.expr
			}
		}
		return nil
	}
	var list []dbgPiece
	for i := 0; i+1 < len(cuts); i++ {
		lo, hi := cuts[i], cuts[i+1]
		if lo == hi {
			continue
		}
		a, b := at(h[0], lo, hi), at(h[1], lo, hi)
		if a == nil && b == nil {
			continue
		}
		ex := append(append(append([]byte(nil), a...), 0x93, size), b...) // DW_OP_piece size
		ex = append(ex, 0x93, size)
		if n := len(list); n > 0 && list[n-1].hi == lo && string(list[n-1].expr) == string(ex) {
			list[n-1].hi = hi
			continue
		}
		list = append(list, dbgPiece{lo, hi, ex})
	}
	return list
}

// dbgConst describes a constant value (nil for other values)
func (e *irEmitter) dbgConst(v *irValue) []byte {
	if v.Op != opConst {
		return nil
	}
	switch v.Type {
	case iI32, iI64:
		return append(appendSLEB([]byte{0x11}, v.AuxInt), 0x9f) // DW_OP_consts; DW_OP_stack_value
	case iF32:
		b := math.Float32bits(float32(v.F))
		return []byte{0x9e, 4, byte(b), byte(b >> 8), byte(b >> 16), byte(b >> 24)} // DW_OP_implicit_value
	case iF64:
		return binary.LittleEndian.AppendUint64([]byte{0x9e, 8}, math.Float64bits(v.F))
	}
	return nil
}

// dbgExpr is the DWARF location expression of a value's location
func (e *irEmitter) dbgExpr(v *irValue) []byte {
	if v.Op.remat() || v.Type.isVec() {
		return nil
	}
	l := v.loc
	switch l.kind {
	case locReg:
		n := int(dwarfReg(l.reg))
		if n < 32 {
			return []byte{byte(0x50 + n)} // DW_OP_reg<n>
		}
		return appendULEB([]byte{0x90}, uint64(n)) // DW_OP_regx
	case locStack:
		off := int64(l.off)
		if l.slot != nil {
			off = l.slot.Offset
		}
		return appendSLEB([]byte{0x91}, off-16) // DW_OP_fbreg: the CFA is rbp+16
	}
	return nil
}

// noteSite extends the code ranges of an inlined call (and its callers)
func noteSite(s *dbgSite, lo, hi int) {
	for ; s != nil; s = s.outer {
		if n := len(s.ranges); n > 0 && s.ranges[n-1][1] == lo {
			s.ranges[n-1][1] = hi
			continue
		}
		s.ranges = append(s.ranges, [2]int{lo, hi})
	}
}

// seen tells whether the inlined call s (or a call it is inlined into) is
// already listed
func (e *irEmitter) seen(s *dbgSite) bool {
	for _, x := range e.sites {
		if x == s {
			return true
		}
	}
	return false
}

// ---- DWARF ----

// irSubprogram describes a function compiled by the IR backend: its
// variables with location lists and its inlined calls
func (d *dwarfInfo) irSubprogram(fn dbgFunc) {
	ob, info := fn.ob, fn.ir
	le := binary.LittleEndian
	// parameters first, in their order, then the locals
	rank := map[*cc.Obj]int{}
	for i, v := range append(append([]*cc.Obj(nil), ob.Params...), ob.Locals...) {
		if _, ok := rank[v]; !ok {
			rank[v] = i
		}
	}
	vars := append([]dbgVarLoc(nil), info.vars...)
	sort.SliceStable(vars, func(i, j int) bool { return rank[vars[i].v] < rank[vars[j].v] })
	params := map[*cc.Obj]bool{}
	for _, p := range ob.Params {
		params[p] = true
	}
	// the inlined calls as a tree
	children := map[*dbgSite][]*dbgSite{}
	var roots []*dbgSite
	seen := map[*dbgSite]bool{}
	var addSite func(s *dbgSite)
	addSite = func(s *dbgSite) {
		if seen[s] {
			return
		}
		seen[s] = true
		if s.outer == nil {
			roots = append(roots, s)
		} else {
			addSite(s.outer)
			children[s.outer] = append(children[s.outer], s)
		}
	}
	for _, s := range info.sites {
		addSite(s)
	}

	code := abbrevSubprogram
	if len(vars) > 0 || len(roots) > 0 {
		code = abbrevSubprogramVars
	}
	d.uleb(uint64(code))
	if ob.IsStatic {
		d.buf = append(d.buf, 0)
	} else {
		d.buf = append(d.buf, 1)
	}
	d.str(ob.Name)
	d.declPos(ob.Tok)
	d.buf = append(d.buf, 1) // prototyped
	d.ref(ob.Ty.Ret)
	d.addr(d.textSym, int64(fn.start))
	d.buf = le.AppendUint64(d.buf, uint64(fn.end-fn.start))
	d.buf = append(d.buf, 1, 0x9c) // frame base: DW_OP_call_frame_cfa
	if code == abbrevSubprogram {
		return
	}
	for _, v := range vars {
		d.varDIE(v, params[v.v])
	}
	var site func(s *dbgSite)
	site = func(s *dbgSite) {
		kids := children[s]
		var svars []dbgVarLoc
		for _, v := range s.locs {
			if v.v != nil {
				svars = append(svars, v)
			}
		}
		if len(kids) > 0 || len(svars) > 0 {
			d.uleb(abbrevInlined)
		} else {
			d.uleb(abbrevInlinedLeaf)
		}
		if !d.inlSeen[s.fn] {
			d.inlSeen[s.fn] = true
			d.inlined = append(d.inlined, s.fn)
		}
		d.refs = append(d.refs, dwRef{len(d.buf), "inl:" + s.fn.AsmName})
		d.buf = append(d.buf, 0, 0, 0, 0)
		d.rangeRefs = append(d.rangeRefs, len(d.buf))
		d.buf = le.AppendUint32(d.buf, uint32(len(d.ranges)))
		for _, r := range s.ranges {
			d.ranges = le.AppendUint64(d.ranges, uint64(r[0]))
			d.ranges = le.AppendUint64(d.ranges, uint64(r[1]))
		}
		d.ranges = append(d.ranges, make([]byte, 16)...)
		d.declPos(s.call)
		srank := map[*cc.Obj]int{}
		sparams := map[*cc.Obj]bool{}
		for i, v := range append(append([]*cc.Obj(nil), s.fn.Params...), s.fn.Locals...) {
			if _, ok := srank[v]; !ok {
				srank[v] = i
			}
		}
		for _, p := range s.fn.Params {
			sparams[p] = true
		}
		sort.SliceStable(svars, func(i, j int) bool { return srank[svars[i].v] < srank[svars[j].v] })
		for _, v := range svars {
			d.varDIE(v, sparams[v.v])
		}
		for _, k := range kids {
			site(k)
		}
		if len(kids) > 0 || len(svars) > 0 {
			d.buf = append(d.buf, 0)
		}
	}
	for _, s := range roots {
		site(s)
	}
	d.buf = append(d.buf, 0)
}

// varDIE writes a variable (or parameter) with its location
func (d *dwarfInfo) varDIE(v dbgVarLoc, isParam bool) {
	le := binary.LittleEndian
	switch {
	case v.inSlot:
		if isParam {
			d.uleb(abbrevParam)
		} else {
			d.uleb(abbrevVariable)
		}
	case len(v.list) > 0:
		if isParam {
			d.uleb(abbrevParamLoc)
		} else {
			d.uleb(abbrevVariableLoc)
		}
	default:
		if isParam {
			d.uleb(abbrevParamNoLoc)
		} else {
			d.uleb(abbrevVariableNoLoc)
		}
	}
	d.str(v.v.Name)
	d.declPos(v.v.Tok)
	d.ref(v.v.Ty)
	switch {
	case v.inSlot:
		loc := appendSLEB([]byte{0x91}, v.off-16) // DW_OP_fbreg
		d.uleb(uint64(len(loc)))
		d.buf = append(d.buf, loc...)
	case len(v.list) > 0:
		// entries are offsets from the unit's base address (the start
		// of the code section)
		d.locRefs = append(d.locRefs, len(d.buf))
		d.buf = le.AppendUint32(d.buf, uint32(len(d.loc)))
		for _, p := range v.list {
			d.loc = le.AppendUint64(d.loc, uint64(p.lo))
			d.loc = le.AppendUint64(d.loc, uint64(p.hi))
			d.loc = le.AppendUint16(d.loc, uint16(len(p.expr)))
			d.loc = append(d.loc, p.expr...)
		}
		d.loc = append(d.loc, make([]byte, 16)...)
	}
}
