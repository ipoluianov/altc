package cc

import (
	"fmt"
)

func (p *parser) function(ty *Type, name *Token, attr *declAttr, asmLabel string) {
	if asmLabel != "" {
		attr.asmName = asmLabel
	}
	// K&R parameter declarations
	if ty.OldStyle && len(ty.ParamNames) > 0 {
		ty = p.krParams(ty)
	}
	o := p.declareGlobal(name, ty, attr)
	if o.Body != nil && !o.InlineOnly {
		p.errorAt(name, "redefinition of '%s'", name.Text)
	}
	// GNU89 "extern inline" (or gnu_inline): the body only serves inlining
	o.InlineOnly = attr.isExtern && attr.isInline && (attr.gnuInline || p.opt.Std < 1999 && p.opt.GNU)
	o.Ty = ty
	o.Tok = name
	o.IsDefinition = true
	o.IsExtern = false
	if attr.isInline {
		o.IsInline = true
	}
	p.applyAttrs(o, attr)
	if attr.isStatic {
		o.IsStatic = true
	}
	p.fn = o
	o.Locals = nil
	o.Params = nil
	o.VaArea = nil
	o.Refs = nil
	o.IsInline = attr.isInline
	p.gotos, p.labels = nil, nil
	p.enterScope()
	if ty.Ret.IsRecord() && !ty.Ret.IsComplete() {
		p.errorAt(name, "return type is an incomplete type")
	}
	for i, pt := range ty.Params {
		pname := ""
		if i < len(ty.ParamNames) && ty.ParamNames[i] != nil {
			pname = ty.ParamNames[i].Text
		}
		if pt.IsRecord() && !pt.IsComplete() {
			p.errorAt(name, "parameter %d has incomplete type", i+1)
		}
		var v *Obj
		if i < len(ty.ParamObjs) {
			v = ty.ParamObjs[i]
			v.Ty, v.Align = pt, pt.Align()
			if pname != "" {
				p.pushVar(pname).obj = v
			}
			o.Locals = append(o.Locals, v)
		} else {
			v = p.newLvar(pname, pt)
		}
		if i < len(ty.ParamNames) {
			v.Tok = ty.ParamNames[i]
		}
		o.Params = append(o.Params, v)
	}
	if ty.Variadic {
		o.VaArea = p.newLvar("__va_area__", arrayOf(tyChar, 176))
		o.VaArea.Align = 16
	}
	o.AllocaBottom = p.newLvar("__alloca_size__", pointerTo(tyChar))
	var prep []*Node
	for _, pv := range o.Params {
		if e := p.vlaPrep(pv.Ty, name); e != nil {
			prep = append(prep, &Node{Kind: NExprStmt, LHS: e, Tok: name})
		}
	}
	body := p.compoundStmt()
	if len(prep) > 0 {
		for _, e := range prep {
			addType(e)
		}
		body.Body = append(prep, body.Body...)
	}
	o.Body = body
	p.resolveGotos()
	p.leaveScope()
	p.fn = nil
}

// krParams reads old-style parameter declarations before the body
func (p *parser) krParams(ty *Type) *Type {
	types := map[string]*Type{}
	for !p.is("{") {
		attr := &declAttr{}
		bt := p.declspec(attr)
		if bt == nil {
			bt = tyInt
		}
		first := true
		for !p.consume(";") {
			if !first {
				p.expect(",")
			}
			first = false
			t, n := p.declarator(bt, attr)
			if n == nil {
				p.errorAt(p.cur(), "parameter name omitted")
			}
			types[n.Text] = t
		}
	}
	nt := *ty
	nt.Params = make([]*Type, len(ty.ParamNames))
	for i, n := range ty.ParamNames {
		t, ok := types[n.Text]
		if !ok {
			t = tyInt
		}
		t = adjustParamType(t)
		// callers promote the arguments of unprototyped functions
		if t.Kind == TyFloat {
			t = tyDouble
		}
		nt.Params[i] = t
	}
	nt.OldStyle = true
	return &nt
}

func (p *parser) resolveGotos() {
	for _, g := range p.gotos {
		found := false
		for _, l := range p.labels {
			if g.Label == l.Label {
				g.UniqueLabel = l.UniqueLabel
				found = true
				// leaving blocks that allocated VLAs: restore the outermost one's stack
				k := 0
				for k < len(g.blocks) && k < len(l.blocks) && g.blocks[k] == l.blocks[k] {
					k++
				}
				if g.Kind == NGoto {
					for _, b := range g.blocks[k:] {
						if b.StackSave != nil {
							g.StackSave = b.StackSave
							break
						}
					}
				}
				break
			}
		}
		if !found {
			p.softError(g.Tok, "label '%s' used but not defined", g.Label)
		}
	}
}

func (p *parser) compoundStmt() *Node {
	t := p.expect("{")
	n := &Node{Kind: NBlock, Tok: t}
	p.enterScope()
	outerVLA := p.hasVLA
	p.hasVLA = false
	p.cleanups = append(p.cleanups, nil)
	p.blocks = append(p.blocks, n)
	for !p.consume("}") {
		if p.cur().Kind == TEOF {
			p.errorAt(t, "expected '}' at end of input")
		}
		n.Body = append(n.Body, p.blockItem()...)
	}
	n.Body = append(n.Body, p.cleanupCalls(len(p.cleanups)-1, t)...)
	p.cleanups = p.cleanups[:len(p.cleanups)-1]
	p.blocks = p.blocks[:len(p.blocks)-1]
	if p.hasVLA && p.fn != nil {
		n.StackSave = p.newLvar("", arrayOf(pointerTo(tyVoid), 2))
	}
	p.hasVLA = outerVLA || p.hasVLA
	p.leaveScope()
	return n
}

// loopBody parses a loop body and arranges for VLA stack space to be
// released on every iteration
func (p *parser) loopBody(loop *Node) *Node {
	outer := p.hasVLA
	p.hasVLA = false
	body := p.stmt()
	if p.hasVLA && p.fn != nil {
		loop.StackSave = p.newLvar("", arrayOf(pointerTo(tyVoid), 2))
	}
	p.hasVLA = outer || p.hasVLA
	return body
}

// cleanupCalls returns the calls of the cleanup functions of the block
// scopes from depth to the innermost one, innermost first
func (p *parser) cleanupCalls(depth int, t *Token) []*Node {
	var out []*Node
	for d := len(p.cleanups) - 1; d >= depth && d >= 0; d-- {
		list := p.cleanups[d]
		for i := len(list) - 1; i >= 0; i-- {
			c := list[i]
			fn := newVarNode(c.fn, t)
			arg := newUnary(NAddr, newVarNode(c.v, t), t)
			addType(arg)
			call := &Node{Kind: NFuncall, LHS: fn, Tok: t, Args: []*Node{arg}}
			addType(fn)
			ft := fn.Ty
			if ft.Kind == TyPtr {
				ft = ft.Base
			}
			call.FuncTy = ft
			call.Ty = ft.Ret
			if len(ft.Params) > 0 {
				call.Args[0] = newCast(arg, ft.Params[0])
			}
			s := &Node{Kind: NExprStmt, LHS: call, Tok: t}
			addType(s)
			out = append(out, s)
		}
	}
	return out
}

func (p *parser) blockItem() []*Node {
	t := p.cur()
	if p.is("_Static_assert") || p.is("static_assert") {
		p.staticAssert()
		return nil
	}
	if p.is("__label__") {
		p.next()
		for !p.consume(";") {
			p.next()
		}
		return nil
	}
	if p.isTypename(t) && !p.peekAt(1).Is(":") || (t.Is("__extension__") && p.isTypename(p.peekAt(1))) {
		attr := &declAttr{}
		basety := p.declspec(attr)
		if attr.isTypedef {
			start := len(p.sc.vars)
			_ = start
			names := p.typedefDecl(basety, attr)
			var out []*Node
			for _, nm := range names {
				if e := p.vlaPrep(p.findVar(nm.Text).typedef, nm); e != nil {
					out = append(out, &Node{Kind: NExprStmt, LHS: e, Tok: nm})
				}
			}
			for _, o := range out {
				addType(o)
			}
			return out
		}
		return []*Node{p.declaration(basety, attr)}
	}
	return []*Node{p.stmt()}
}

// computeVLASize emits code computing the byte size of a VLA type
func (p *parser) computeVLASize(ty *Type, tok *Token) *Node {
	n := &Node{Kind: NNull, Tok: tok}
	if ty.Base != nil {
		n = p.newBinary(NComma, n, p.computeVLASize(ty.Base, tok), tok)
	}
	if ty.Kind != TyVLA || ty.VLASize != nil {
		return n
	}
	var baseSz *Node
	if ty.Base.Kind == TyVLA {
		baseSz = newVarNode(ty.Base.VLASize, tok)
	} else {
		baseSz = newLong(ty.Base.Size(), tok)
	}
	ty.VLASize = p.newLvar("", tyULong)
	l := newCast(ty.VLALen, tyULong)
	expr := p.newBinary(NAssign, newVarNode(ty.VLASize, tok), p.newBinary(NMul, l, baseSz, tok), tok)
	r := p.newBinary(NComma, n, expr, tok)
	addType(r)
	return r
}

// vlaPrep computes the sizes of the variable-length array types reachable
// through pointers in ty (parameters, typedefs, pointers to VLAs); nil
// when there is nothing to compute
func (p *parser) vlaPrep(ty *Type, tok *Token) *Node {
	var n *Node
	var walk func(t *Type)
	seen := map[*Type]bool{}
	walk = func(t *Type) {
		if t == nil || seen[t] {
			return
		}
		seen[t] = true
		switch t.Kind {
		case TyPtr, TyArray:
			walk(t.Base)
		case TyVLA:
			if t.VLASize == nil {
				e := p.computeVLASize(t, tok)
				if n == nil {
					n = e
				} else {
					n = p.newBinary(NComma, n, e, tok)
				}
			}
		}
	}
	walk(ty)
	return n
}

func (p *parser) newAlloca(sz *Node) *Node {
	n := &Node{Kind: NAlloca, LHS: sz, Tok: sz.Tok, Ty: pointerTo(tyVoid)}
	addType(sz)
	return n
}

// declaration parses a block-scope declaration
func (p *parser) declaration(basety *Type, attr *declAttr) *Node {
	blk := &Node{Kind: NBlock, Tok: p.cur()}
	first := true
	for !p.consume(";") {
		if !first {
			p.expect(",")
		}
		first = false
		a := *attr
		if basety == nil {
			// C23 auto / __auto_type: the type comes from the initializer
			name := p.ident()
			p.attributes(&a)
			p.expect("=")
			init := p.assign()
			addType(init)
			ty := decayType(init.Ty).Unqual()
			v := p.newLvar(name.Text, ty)
			v.Tok = name
			if a.isStatic {
				p.errorAt(name, "static 'auto' declarations are not supported")
			}
			blk.Body = append(blk.Body, &Node{Kind: NExprStmt, Tok: name, LHS: p.newBinary(NAssign, newVarNode(v, name), init, name)})
			continue
		}
		ty, name := p.declarator(basety, &a)
		p.attributes(&a)
		a.asmName = p.asmLabel()
		p.attributes(&a)
		if name == nil {
			p.errorAt(p.cur(), "variable name omitted")
		}
		if a.mode != "" {
			ty = modeType(ty, a.mode)
		}
		if ty.Kind == TyVoid {
			p.errorAt(name, "variable '%s' declared void", name.Text)
		}
		if ty.Kind == TyFunc || a.isExtern {
			o := p.declareGlobal(name, ty, &a)
			p.applyAttrs(o, &a)
			if a.isTLS {
				o.IsTLS = true
			}
			if ty.Kind != TyFunc && p.is("=") {
				p.errorAt(name, "'%s' has both 'extern' and initializer", name.Text)
			}
			continue
		}
		if a.isStatic {
			// static local: a global with a unique symbol name
			uname := fmt.Sprintf("%s.%d", name.Text, p.nextUniq())
			o := &Obj{Name: name.Text, AsmName: uname, Ty: ty, Tok: name, Align: ty.Align(), IsStatic: true, IsDefinition: true}
			p.applyAttrs(o, &a)
			if a.asmName == "" {
				o.AsmName = uname
			}
			o.IsTLS = a.isTLS
			p.prog.Globals = append(p.prog.Globals, o)
			p.pushVar(name.Text).obj = o
			if ty.Kind == TyVLA {
				p.errorAt(name, "static variable length array")
			}
			if p.consume("=") {
				p.gvarInitializer(o)
			} else {
				o.IsTentative = false
				o.InitData = nil
			}
			if a.isConstexpr {
				o.IsConst = true
			}
			continue
		}
		if a.isTLS {
			p.errorAt(name, "'_Thread_local' used for a local variable")
		}
		if ty.Kind == TyVLA {
			if p.is("=") {
				p.errorAt(name, "variable-sized object may not be initialized")
			}
			sz := p.computeVLASize(ty, name)
			p.hasVLA = true
			v := p.newLvar(name.Text, ty)
			v.Tok = name
			expr := p.newBinary(NAssign, &Node{Kind: NVLAPtr, Var: v, Tok: name, Ty: pointerTo(ty.Base)}, p.newAlloca(newVarNode(ty.VLASize, name)), name)
			blk.Body = append(blk.Body, &Node{Kind: NExprStmt, LHS: p.newBinary(NComma, sz, expr, name), Tok: name})
			continue
		}
		if e := p.vlaPrep(ty, name); e != nil {
			blk.Body = append(blk.Body, &Node{Kind: NExprStmt, LHS: e, Tok: name})
		}
		v := p.newLvar(name.Text, ty)
		v.Tok = name
		if a.align > 0 {
			v.Align = a.align
		}
		if a.cleanup != nil && len(p.cleanups) > 0 {
			fv := p.findVar(a.cleanup.Text)
			if fv == nil || fv.obj == nil || !fv.obj.IsFunction {
				p.errorAt(a.cleanup, "cleanup argument not a function")
			}
			p.markUsed(fv.obj)
			top := len(p.cleanups) - 1
			p.cleanups[top] = append(p.cleanups[top], cleanupEntry{v, fv.obj})
		}
		if p.consume("=") {
			init := p.lvarInitializer(v)
			blk.Body = append(blk.Body, &Node{Kind: NExprStmt, LHS: init, Tok: name})
			if a.isConstexpr && v.Ty.IsScalar() {
				// remember the value so it can be used in constant expressions
				v.ConstExpr = p.constexprValue(init)
			}
		}
		if !v.Ty.IsComplete() {
			p.errorAt(name, "storage size of '%s' isn't known", name.Text)
		}
	}
	addType(blk)
	return blk
}

func (p *parser) constexprValue(init *Node) *Node {
	// init is (memzero, var = value) chain; find the assignment value
	var val *Node
	var walk func(n *Node)
	walk = func(n *Node) {
		if n == nil {
			return
		}
		if n.Kind == NAssign {
			val = n.RHS
		}
		walk(n.LHS)
		walk(n.RHS)
	}
	walk(init)
	if val != nil && p.isConstExpr(val) {
		return val
	}
	return nil
}

func (p *parser) nextUniq() int {
	p.uniq++
	return p.uniq
}

func (p *parser) newLabel() string {
	return p.uniqueName("L")
}

func (p *parser) stmt() *Node {
	t := p.cur()
	if t.Kind == TIdent {
		text := t.Text
		if t.notKw {
			text = ""
		}
		switch text {
		case "return":
			p.next()
			n := &Node{Kind: NReturn, Tok: t}
			if p.consume(";") {
				if p.fn != nil && p.fn.Ty.Ret.Kind != TyVoid {
					p.warnAt(t, "'return' with no value, in function returning non-void")
				}
				return p.withCleanups(n, nil)
			}
			e := p.expr()
			p.expect(";")
			addType(e)
			ret := p.fn.Ty.Ret
			if ret.Kind == TyVoid {
				if e.Ty.Kind != TyVoid {
					p.warnAt(t, "'return' with a value, in function returning void")
				}
				n.LHS = e
				return p.withCleanups(n, nil)
			}
			if !ret.IsRecord() || ret.IsComplex() {
				p.checkAssign(ret, e, t, "return")
				e = newCast(e, ret)
			} else if !compatible(ret.Unqual(), e.Ty.Unqual()) {
				p.errorAt(t, "incompatible types when returning type '%s' but '%s' was expected", e.Ty, ret)
			}
			n.LHS = e
			return p.withCleanups(n, ret)
		case "if":
			p.next()
			n := &Node{Kind: NIf, Tok: t}
			p.expect("(")
			n.Cond = p.toBool(p.expr())
			p.expect(")")
			n.Then = p.stmt()
			if p.consume("else") {
				n.Els = p.stmt()
			}
			return n
		case "switch":
			p.next()
			n := &Node{Kind: NSwitch, Tok: t}
			p.expect("(")
			c := p.expr()
			addType(c)
			if !c.Ty.IsInteger() {
				p.errorAt(t, "switch quantity not an integer")
			}
			n.Cond = newCast(c, intPromote(c.Ty))
			p.expect(")")
			sw, brk, bd := p.sw, p.brk, p.brkDepth
			p.sw = n
			n.BrkLabel = p.newLabel()
			p.brk = n.BrkLabel
			p.brkDepth = len(p.cleanups)
			n.Then = p.stmt()
			p.sw, p.brk, p.brkDepth = sw, brk, bd
			return n
		case "case":
			p.next()
			if p.sw == nil {
				p.errorAt(t, "case label not within a switch statement")
			}
			begin := p.constExpr()
			end := begin
			if p.consume("...") {
				end = p.constExpr()
				if end < begin {
					p.warnAt(t, "empty range specified")
				}
			}
			p.expect(":")
			n := &Node{Kind: NCase, Tok: t, Label: p.newLabel()}
			ct := p.sw.Cond.Ty
			n.Begin = truncInt(begin, ct)
			n.End = truncInt(end, ct)
			for _, c := range p.sw.Cases {
				if n.Begin <= c.End && c.Begin <= n.End {
					p.softError(t, "duplicate case value")
					break
				}
			}
			n.LHS = p.labeledBody()
			p.sw.Cases = append(p.sw.Cases, n)
			return n
		case "default":
			if !p.peekAt(1).Is(":") {
				break
			}
			p.next()
			if p.sw == nil {
				p.errorAt(t, "'default' label not within a switch statement")
			}
			p.expect(":")
			n := &Node{Kind: NCase, Tok: t, Label: p.newLabel()}
			if p.sw.Default != nil {
				p.softError(t, "multiple default labels in one switch")
			}
			n.LHS = p.labeledBody()
			p.sw.Default = n
			return n
		case "for":
			p.next()
			n := &Node{Kind: NFor, Tok: t}
			p.expect("(")
			p.enterScope()
			brk, cont, bd, cd := p.brk, p.cont, p.brkDepth, p.contDepth
			n.BrkLabel = p.newLabel()
			n.ContLabel = p.newLabel()
			p.brk, p.cont = n.BrkLabel, n.ContLabel
			p.brkDepth, p.contDepth = len(p.cleanups), len(p.cleanups)
			if p.isTypename(p.cur()) {
				attr := &declAttr{}
				bt := p.declspec(attr)
				n.Init = p.declaration(bt, attr)
			} else if !p.consume(";") {
				n.Init = &Node{Kind: NExprStmt, LHS: p.expr(), Tok: t}
				p.expect(";")
			}
			if !p.is(";") {
				n.Cond = p.toBool(p.expr())
			}
			p.expect(";")
			if !p.is(")") {
				n.Inc = p.expr()
			}
			p.expect(")")
			n.Then = p.loopBody(n)
			p.leaveScope()
			p.brk, p.cont, p.brkDepth, p.contDepth = brk, cont, bd, cd
			return n
		case "while":
			p.next()
			n := &Node{Kind: NFor, Tok: t}
			p.expect("(")
			n.Cond = p.toBool(p.expr())
			p.expect(")")
			brk, cont, bd, cd := p.brk, p.cont, p.brkDepth, p.contDepth
			n.BrkLabel = p.newLabel()
			n.ContLabel = p.newLabel()
			p.brk, p.cont = n.BrkLabel, n.ContLabel
			p.brkDepth, p.contDepth = len(p.cleanups), len(p.cleanups)
			n.Then = p.loopBody(n)
			p.brk, p.cont, p.brkDepth, p.contDepth = brk, cont, bd, cd
			return n
		case "do":
			p.next()
			n := &Node{Kind: NDo, Tok: t}
			brk, cont, bd, cd := p.brk, p.cont, p.brkDepth, p.contDepth
			n.BrkLabel = p.newLabel()
			n.ContLabel = p.newLabel()
			p.brk, p.cont = n.BrkLabel, n.ContLabel
			p.brkDepth, p.contDepth = len(p.cleanups), len(p.cleanups)
			n.Then = p.loopBody(n)
			p.brk, p.cont, p.brkDepth, p.contDepth = brk, cont, bd, cd
			p.expect("while")
			p.expect("(")
			n.Cond = p.toBool(p.expr())
			p.expect(")")
			p.expect(";")
			return n
		case "goto":
			p.next()
			if p.consume("*") {
				n := &Node{Kind: NGotoExpr, Tok: t, LHS: p.expr()}
				p.expect(";")
				return n
			}
			name := p.ident()
			n := &Node{Kind: NGoto, Tok: name, Label: name.Text, blocks: append([]*Node(nil), p.blocks...)}
			p.gotos = append(p.gotos, n)
			p.expect(";")
			return n
		case "break":
			p.next()
			if p.brk == "" {
				p.errorAt(t, "break statement not within loop or switch")
			}
			p.expect(";")
			return p.jumpWithCleanups(&Node{Kind: NGoto, Tok: t, UniqueLabel: p.brk}, p.brkDepth)
		case "continue":
			p.next()
			if p.cont == "" {
				p.errorAt(t, "continue statement not within a loop")
			}
			p.expect(";")
			return p.jumpWithCleanups(&Node{Kind: NGoto, Tok: t, UniqueLabel: p.cont}, p.contDepth)
		case "asm", "__asm__", "__asm":
			return p.asmStmt()
		}
		if !t.kw() && p.peekAt(1).Is(":") && !p.peekAt(2).Is(":") {
			p.next()
			p.next()
			p.attributes(&declAttr{})
			n := &Node{Kind: NLabel, Tok: t, Label: t.Text, UniqueLabel: p.newLabel(), blocks: append([]*Node(nil), p.blocks...)}
			for _, l := range p.labels {
				if l.Label == t.Text {
					p.softError(t, "duplicate label '%s'", t.Text)
				}
			}
			n.LHS = p.labeledBody()
			p.labels = append(p.labels, n)
			return n
		}
	}
	if p.is("{") {
		return p.compoundStmt()
	}
	if p.is("[") && p.peekAt(1).Is("[") {
		p.attributes(&declAttr{})
		return p.stmt()
	}
	if p.consume(";") {
		return &Node{Kind: NBlock, Tok: t}
	}
	n := &Node{Kind: NExprStmt, Tok: t, LHS: p.expr()}
	p.expect(";")
	return n
}

// labeledBody parses the statement after a label; C23 allows a label
// before a declaration or at the end of a block
func (p *parser) labeledBody() *Node {
	if p.is("}") {
		return &Node{Kind: NBlock, Tok: p.cur()}
	}
	if t := p.cur(); p.isTypename(t) && !p.peekAt(1).Is(":") {
		items := p.blockItem()
		return &Node{Kind: NBlock, Tok: t, Body: items}
	}
	return p.stmt()
}

func (p *parser) asmStmt() *Node {
	t := p.next()
	for p.is("volatile") || p.is("__volatile__") || p.is("__volatile") || p.is("inline") || p.is("goto") {
		p.next()
	}
	p.expect("(")
	text := ""
	for p.cur().Kind == TStr {
		_, body := literalPrefix(p.next().Text)
		text += body
	}
	hasOperands := false
	depth := 0
	for !(depth == 0 && p.is(")")) {
		x := p.next()
		if x.Kind == TEOF {
			p.errorAt(t, "unterminated asm statement")
		}
		if x.Is("(") {
			depth++
		} else if x.Is(")") {
			depth--
		}
		if x.Kind != TPunct || x.Text != ":" {
			hasOperands = true
		}
	}
	p.expect(")")
	p.expect(";")
	n := &Node{Kind: NAsm, Tok: t, AsmText: text}
	if text == "" || isBarrierAsm(text) {
		return n // compiler barrier: nothing to do
	}
	_ = hasOperands
	p.errorAt(t, "inline assembly is not supported")
	return n
}

// withCleanups runs the active cleanups before a return; the value is
// computed first into a temporary
func (p *parser) withCleanups(ret *Node, retTy *Type) *Node {
	calls := p.cleanupCalls(0, ret.Tok)
	if len(calls) == 0 {
		return ret
	}
	blk := &Node{Kind: NBlock, Tok: ret.Tok}
	if ret.LHS != nil && retTy != nil {
		tmp := p.newLvar("", ret.LHS.Ty)
		as := p.newBinary(NAssign, newVarNode(tmp, ret.Tok), ret.LHS, ret.Tok)
		addType(as)
		blk.Body = append(blk.Body, &Node{Kind: NExprStmt, LHS: as, Tok: ret.Tok})
		ret.LHS = newVarNode(tmp, ret.Tok)
		addType(ret.LHS)
	} else if ret.LHS != nil {
		blk.Body = append(blk.Body, &Node{Kind: NExprStmt, LHS: ret.LHS, Tok: ret.Tok})
		ret.LHS = nil
	}
	blk.Body = append(blk.Body, calls...)
	blk.Body = append(blk.Body, ret)
	addType(blk)
	return blk
}

func (p *parser) jumpWithCleanups(j *Node, depth int) *Node {
	calls := p.cleanupCalls(depth, j.Tok)
	if len(calls) == 0 {
		return j
	}
	blk := &Node{Kind: NBlock, Tok: j.Tok, Body: append(calls, j)}
	addType(blk)
	return blk
}

func isBarrierAsm(s string) bool {
	switch s {
	case "", " ", "nop", "mfence", "lfence", "sfence", "pause", "rep; nop", "rep nop":
		return true
	}
	return false
}

// truncInt converts a value to the representation of an integer type
func truncInt(v int64, t *Type) int64 {
	switch t.Size() {
	case 1:
		if t.IsUnsigned() {
			return int64(uint8(v))
		}
		return int64(int8(v))
	case 2:
		if t.IsUnsigned() {
			return int64(uint16(v))
		}
		return int64(int16(v))
	case 4:
		if t.IsUnsigned() {
			return int64(uint32(v))
		}
		return int64(int32(v))
	}
	return v
}
