package x64

import (
	"fmt"
	"os"
	"strings"

	"github.com/ipoluianov/altc/cc"
)

// Inlining: the body of a small (or always_inline) function defined in the
// same file is lowered in place of a call. Parameters become variables of
// the caller's SSA form, returns jump to the continuation, and labels get
// a prefix per inlined copy.

const (
	inlineMaxNodes = 40 // functions up to this size are inlined
	inlineMaxDepth = 8
)

type inlineCtx struct {
	fn     *cc.Obj
	exit   *irBlock
	result *cc.Obj // temporary holding the returned value
	prefix string
	outer  *inlineCtx
}

// inlinable tells whether calls to fn may be replaced by its body
func (g *gen) inlinable(fn *cc.Obj) bool {
	if g.noInline {
		return false
	}
	if ok, done := g.inlineOK[fn]; done {
		return ok
	}
	g.inlineOK[fn] = false // in progress: no recursion through this
	ok := g.checkInlinable(fn)
	g.inlineOK[fn] = ok
	return ok
}

func (g *gen) checkInlinable(fn *cc.Obj) bool {
	if skip := os.Getenv("ALTC_NOINLINE_FN"); skip != "" {
		for _, n := range strings.Split(skip, ",") {
			if n == fn.Name {
				return false
			}
		}
	}
	ft := fn.Ty
	if fn.Body == nil || fn.NoInline || fn.IsWeak || fn.LabelAddrs || ft.Variadic || ft.OldStyle && len(fn.Params) > 0 {
		return false
	}
	if ft.Ret.IsRecord() && !isCplx(ft.Ret) || ft.Ret.Kind == cc.TyLongDouble || isCLD(ft.Ret) {
		return false
	}
	for _, p := range fn.Params {
		if p.Ty.IsRecord() && !isCplx(p.Ty) || p.Ty.Kind == cc.TyLongDouble || isCLD(p.Ty) {
			return false
		}
	}
	if asmDefinesLabels(fn) {
		return false
	}
	// stack allocations would pile up in the caller (gcc does not inline them either)
	nodes, alloca := 0, false
	cc.Walk(fn.Body, func(n *cc.Node) {
		nodes++
		alloca = alloca || n.Kind == cc.NAlloca
	})
	if alloca {
		return false
	}
	if nodes > inlineMaxNodes && !fn.AlwaysInline {
		return false
	}
	// the body must be something the IR handles on its own
	_, err := g.lowerFunction(fn)
	return err == nil
}

// tryInline lowers a direct call by inlining the callee
func (b *irBuilder) tryInline(n *cc.Node, fn *cc.Obj) (*irValue, bool) {
	depth := 0
	for c := b.inl; c != nil; c = c.outer {
		if c.fn == fn {
			return nil, false
		}
		depth++
	}
	if fn == b.fn || depth >= inlineMaxDepth || !b.g.inlinable(fn) {
		return nil, false
	}
	// a function for more instruction set extensions stays a call
	if b.g.targetISA(fn.Target)&^b.g.isa != 0 || len(fn.TargetClones) > 0 {
		return nil, false
	}
	if only := os.Getenv("ALTC_INLINE_INTO"); only != "" && !strings.Contains(","+only+",", ","+b.fn.Name+",") {
		return nil, false
	}
	// arguments are evaluated in the caller first
	var args []*irValue
	for _, a := range n.Args {
		v := b.expr(a)
		if v == nil {
			unsupported("void argument")
		}
		args = append(args, v)
	}
	b.analyzeLocals(fn)
	if irStats {
		fmt.Fprintf(os.Stderr, "ir: inline %s into %s\n", fn.Name, b.fn.Name)
	}
	b.inlineN++
	ctx := &inlineCtx{fn: fn, exit: b.f.newBlock(), prefix: fmt.Sprintf("inl%d.", b.inlineN), outer: b.inl}
	if fn.Ty.Ret.Kind != cc.TyVoid {
		ctx.result = b.newTemp(irTypeOf(fn.Ty.Ret))
	}
	site := b.f.curSite
	var here *dbgSite
	if b.g.debug {
		here = &dbgSite{fn: fn, call: n.Tok, outer: site, vars: dbgVarSet(fn)}
		b.f.curSite = here
	}
	for i, p := range fn.Params {
		if i >= len(args) {
			break
		}
		if b.promoted[p] {
			b.writeVar(p, args[i])
		} else {
			b.store(b.slotAddr(p), args[i], p.Ty)
		}
	}
	b.inl = ctx
	b.stmt(fn.Body)
	b.jump(ctx.exit)
	if here != nil {
		here.mem = map[*cc.Obj]*irSlot{}
		for v := range here.vars {
			if s, ok := b.slots[v]; ok && !b.promoted[v] {
				here.mem[v] = s
			}
		}
	}
	b.f.curSite = site
	b.inl = ctx.outer
	b.seal(ctx.exit)
	b.cur = ctx.exit
	if ctx.result == nil {
		return nil, true
	}
	return b.readVar(ctx.result), true
}
