package x64

// Inline assembly: operand constraints, register assignment, template
// substitution and the hand-off to the AT&T assembler (att.go).
//
// The code generator keeps no values in registers between statements, so
// an asm statement is compiled as: evaluate every operand into a frame
// slot, load the register operands, emit the instructions, then store the
// outputs. Callee-saved registers are saved in the prologue of functions
// that contain asm statements.

import (
	"fmt"
	"sort"
	"strconv"
	"strings"

	"github.com/ipoluianov/altc/cc"
	"github.com/ipoluianov/altc/elf"
)

type asmKind int

const (
	akReg asmKind = iota
	akXMM
	akMem
	akImm
	akX87
	akFlag
)

type asmOp struct {
	op      *cc.AsmOperand
	out     bool
	inout   bool // '+': also an input
	early   bool // '&'
	kind    asmKind
	reg     Reg
	reg2    Reg // 'A' with a 16-byte value: the high half (rdx)
	fixed   bool
	match   int // input tied to this output index (-1)
	ty      *cc.Type
	slot    int64 // value slot (16 bytes); the address slot follows
	mem     Mem
	viaAddr bool // the location is reached through the address in the slot
	cond    Cond
	st      int // x87: 0 for 't' (st(0)), 1 for 'u' (st(1))
	pool    []Reg
}

var asmGPPool = []Reg{RAX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, RBX, R12, R13, R14, R15}

// calleeSaved are saved in the prologue of functions with asm statements
var calleeSaved = []Reg{RBX, R12, R13, R14, R15}

// hasAsm tells whether a function body contains asm needing the register save area
func hasAsm(body *cc.Node) bool {
	found := false
	cc.Walk(body, func(n *cc.Node) {
		if n.Kind == cc.NAsm && n.Asm != nil && !isTrivialAsm(n.Asm) {
			found = true
		}
	})
	return found
}

func isTrivialAsm(st *cc.AsmStmt) bool {
	return strings.TrimSpace(st.Template) == "" && len(st.Outputs) == 0 && len(st.Inputs) == 0 && len(st.Labels) == 0
}

// asmFrame reserves the frame slots of asm operands; it returns the new frame offset
func (g *gen) asmFrame(fn *cc.Obj, offset int64) int64 {
	g.asmSlots = map[*cc.AsmOperand]int64{}
	g.asmSaveOff = 0
	if !hasAsm(fn.Body) {
		return offset
	}
	cc.Walk(fn.Body, func(n *cc.Node) {
		if n.Kind != cc.NAsm || n.Asm == nil {
			return
		}
		for _, list := range [][]*cc.AsmOperand{n.Asm.Outputs, n.Asm.Inputs} {
			for _, o := range list {
				offset = alignTo(offset+32, 16)
				g.asmSlots[o] = -offset
			}
		}
	})
	offset = alignTo(offset+int64(8*len(calleeSaved)), 8)
	g.asmSaveOff = -offset
	return offset
}

func (g *gen) saveCalleeRegs() {
	if g.asmSaveOff == 0 {
		return
	}
	for i, r := range calleeSaved {
		g.a.SaveReg(r, int32(g.asmSaveOff)+int32(8*i))
	}
}

func (g *gen) restoreCalleeRegs() {
	if g.asmSaveOff == 0 {
		return
	}
	for i, r := range calleeSaved {
		g.a.Mov(8, Q(r), M(RBP, int32(g.asmSaveOff)+int32(8*i)))
	}
}

// ---- constraints ----

type constraint struct {
	letters string
	match   int    // matching output index (-1)
	name    string // matching [name]
	flag    string // @cc condition
	inout   bool
	early   bool
}

func parseConstraint(c string) constraint {
	k := constraint{match: -1}
	if strings.HasPrefix(c, "=@cc") || strings.HasPrefix(c, "+@cc") {
		k.flag = c[4:]
		return k
	}
	// first alternative only
	if i := strings.IndexByte(c, ','); i >= 0 {
		c = c[:i]
	}
	for i := 0; i < len(c); i++ {
		ch := c[i]
		switch {
		case ch == '=':
		case ch == '+':
			k.inout = true
		case ch == '&':
			k.early = true
		case ch == '%' || ch == '*' || ch == '?' || ch == '!' || ch == ' ':
		case ch == '#':
			i = len(c)
		case ch >= '0' && ch <= '9':
			j := i
			for j < len(c) && c[j] >= '0' && c[j] <= '9' {
				j++
			}
			k.match, _ = strconv.Atoi(c[i:j])
			i = j - 1
		case ch == '[':
			j := strings.IndexByte(c[i:], ']')
			if j > 0 {
				k.name = c[i+1 : i+j]
				i += j
			}
		case ch == 'Y' && i+1 < len(c):
			// Yz = xmm0, Yi/Yd... = SSE registers
			if c[i+1] == 'z' {
				k.letters += "z"
			} else {
				k.letters += "x"
			}
			i++
		default:
			k.letters += string(ch)
		}
	}
	return k
}

func fixedReg(ch byte) (Reg, bool) {
	switch ch {
	case 'a':
		return RAX, true
	case 'b':
		return RBX, true
	case 'c':
		return RCX, true
	case 'd':
		return RDX, true
	case 'S':
		return RSI, true
	case 'D':
		return RDI, true
	case 'z':
		return XMM0, true
	}
	return NoReg, false
}

// classify chooses how an operand is passed
func (g *gen) classify(o *asmOp, k constraint, n *cc.Node) {
	l := k.letters
	t := o.ty
	isFloat := t.Kind == cc.TyFloat || t.Kind == cc.TyDouble
	scalar := t.IsInteger() || t.Kind == cc.TyPtr || t.Kind == cc.TyBool || isFloat || t.Kind == cc.TyEnum ||
		t.Kind == cc.TyArray || t.Kind == cc.TyFunc
	regOK := scalar && (t.Size() <= 8 || t.Kind == cc.TyArray || t.Kind == cc.TyFunc || strings.Contains(l, "A") && t.Size() == 16)
	hasImm := strings.ContainsAny(l, "inIJKLMNOeEFGsZgX")
	hasMem := strings.ContainsAny(l, "mogVX<>")
	hasX87 := strings.ContainsAny(l, "tuf")
	hasXMM := strings.ContainsAny(l, "xvz")
	hasGP := strings.ContainsAny(l, "rqQRlabcdSDAUgXp")
	switch {
	case k.flag != "":
		o.kind = akFlag
		cnd, ok := condCodes[k.flag]
		if !ok {
			g.fail(n, "invalid flag output constraint '=@cc%s'", k.flag)
		}
		o.cond = cnd
		o.pool = asmGPPool
	case !o.out && o.op.IsConst && hasImm:
		o.kind = akImm
	case hasX87 && t.IsFlonum():
		o.kind = akX87
		if strings.Contains(l, "u") {
			o.st = 1
		}
	case hasXMM && (isFloat || !hasGP) && regOK:
		o.kind = akXMM
		if r, ok := fixedReg('z'); ok && strings.Contains(l, "z") {
			o.reg, o.fixed = r, true
		}
		o.pool = nil
		for i := 0; i < 16; i++ {
			o.pool = append(o.pool, XMM0+Reg(i))
		}
	case hasGP && regOK:
		o.kind = akReg
		o.pool = asmGPPool
		switch {
		case strings.Contains(l, "Q"):
			o.pool = []Reg{RAX, RBX, RCX, RDX} // registers with a high byte
		case strings.Contains(l, "R"):
			o.pool = []Reg{RAX, RBX, RCX, RDX, RSI, RDI}
		case strings.Contains(l, "U"):
			o.pool = []Reg{RAX, RCX, RDX, RSI, RDI, R8, R9, R10, R11}
		}
		for i := 0; i < len(l); i++ {
			if r, ok := fixedReg(l[i]); ok && r != XMM0 {
				o.reg, o.fixed = r, true
			}
		}
		if strings.Contains(l, "A") {
			o.reg, o.fixed = RAX, true
			if t.Size() == 16 {
				o.reg2 = RDX
			}
		}
		if n.Kind == cc.NVar && n.Var.RegName != "" && !o.fixed {
			if r, ok := gpRegs[n.Var.RegName]; ok && !r.r.IsXMM() {
				o.reg, o.fixed = r.r, true
			}
		}
	case hasMem:
		o.kind = akMem
	case hasImm && !o.out:
		g.fail(n, "impossible constraint in 'asm'")
	default:
		g.fail(n, "unsupported asm constraint '%s'", o.op.Constraint)
	}
	if o.kind == akXMM && n.Kind == cc.NVar && strings.HasPrefix(n.Var.RegName, "xmm") {
		if r, ok := gpRegs[n.Var.RegName]; ok {
			o.reg, o.fixed = r.r, true
		}
	}
}

// ---- statement ----

func (g *gen) genAsm(n *cc.Node) {
	st := n.Asm
	if st.Basic {
		if strings.TrimSpace(st.Template) == "" {
			return
		}
		g.assembleInFunc(n, st.Template, map[string]*TextLabel{})
		return
	}
	ops, nOut := g.asmOperands(n)
	clobbered, x87Clob := g.asmClobbers(n)

	// phase A: evaluate the operands into the frame
	for _, o := range ops {
		e := o.op.Expr
		switch {
		case o.out:
			g.asmLocation(n, o, e)
		case o.kind == akImm:
		case o.kind == akMem:
			if isAddressable(e) {
				g.asmLocation(n, o, e)
			} else {
				g.genExpr(e)
				g.spillValue(n, o.ty, o.slot)
				o.mem = M(RBP, int32(o.slot))
			}
		default:
			g.genExpr(e)
			g.spillValue(n, o.ty, o.slot)
		}
	}
	g.asmRun(n, ops, nOut, clobbered, x87Clob)
}

// asmOperands classifies the operands of an extended asm statement
func (g *gen) asmOperands(n *cc.Node) ([]*asmOp, int) {
	st := n.Asm
	var ops []*asmOp
	outIdx := map[string]int{}
	for i, op := range st.Outputs {
		k := parseConstraint(op.Constraint)
		o := &asmOp{op: op, out: true, inout: k.inout, early: k.early, ty: op.Expr.Ty, slot: g.asmSlots[op], match: -1, reg: NoReg, reg2: NoReg}
		if op.Expr.Kind == cc.NMember && op.Expr.Member.Bitfield {
			g.fail(n, "bit-field asm output operands are not supported")
		}
		g.classify(o, k, op.Expr)
		if o.kind == akFlag && !(o.ty.IsInteger() || o.ty.Kind == cc.TyBool) {
			g.fail(n, "flag output operand must have an integer type")
		}
		ops = append(ops, o)
		if op.Name != "" {
			outIdx[op.Name] = i
		}
	}
	for _, op := range st.Inputs {
		k := parseConstraint(op.Constraint)
		t := op.Expr.Ty
		if t.Kind == cc.TyArray || t.Kind == cc.TyFunc {
			if !strings.ContainsAny(k.letters, "mo") {
				t = cc.PointerTo(t)
			}
		}
		o := &asmOp{op: op, ty: t, slot: g.asmSlots[op], match: -1, reg: NoReg, reg2: NoReg}
		if k.name != "" {
			idx, ok := outIdx[k.name]
			if !ok {
				g.fail(n, "undefined named operand '%s'", k.name)
			}
			k.match = idx
		}
		if k.match >= 0 {
			if k.match >= len(st.Outputs) {
				g.fail(n, "matching constraint references invalid operand number")
			}
			o.match = k.match
			out := ops[k.match]
			o.kind = out.kind
			o.reg, o.reg2, o.fixed, o.pool, o.st = out.reg, out.reg2, out.fixed, out.pool, out.st
		} else {
			g.classify(o, k, op.Expr)
		}
		ops = append(ops, o)
	}
	return ops, len(st.Outputs)
}

// asmClobbers reads the clobber list of an asm statement
func (g *gen) asmClobbers(n *cc.Node) (map[Reg]bool, map[int]bool) {
	st := n.Asm
	clobbered := map[Reg]bool{}
	x87Clob := map[int]bool{}
	for _, c := range st.Clobbers {
		c = strings.TrimPrefix(strings.TrimSpace(c), "%")
		switch {
		case c == "cc" || c == "memory" || c == "dirflag" || c == "fpsr" || c == "fpcr" || c == "flags" || c == "redzone":
		case c == "st":
			x87Clob[0] = true
		case strings.HasPrefix(c, "st(") && len(c) == 5:
			x87Clob[int(c[3]-'0')] = true
		case strings.HasPrefix(c, "mm") || strings.HasPrefix(c, "ymm") || strings.HasPrefix(c, "zmm") || strings.HasPrefix(c, "k"):
		default:
			r, ok := gpRegs[c]
			if !ok {
				g.fail(n, "unknown register name '%s' in 'asm'", c)
			}
			if r.r == RSP || r.r == RBP {
				continue
			}
			clobbered[r.r] = true
		}
	}
	return clobbered, x87Clob
}

// asmRun assigns registers, loads the inputs, assembles the template and
// writes the outputs; the operands are already in their frame slots
func (g *gen) asmRun(n *cc.Node, ops []*asmOp, nOut int, clobbered map[Reg]bool, x87Clob map[int]bool) {
	st := n.Asm

	// phase B: registers
	usedOut := map[Reg]bool{}
	usedIn := map[Reg]bool{}
	take := func(o *asmOp, used ...map[Reg]bool) {
		if o.fixed {
			return
		}
		for _, r := range o.pool {
			if clobbered[r] {
				continue
			}
			free := true
			for _, u := range used {
				if u[r] {
					free = false
				}
			}
			if free {
				o.reg = r
				return
			}
		}
		g.fail(n, "'asm' operand requires impossible reload (out of registers)")
	}
	regKind := func(o *asmOp) bool { return o.kind == akReg || o.kind == akXMM || o.kind == akFlag }
	for _, o := range ops[:nOut] {
		if regKind(o) && o.fixed {
			if usedOut[o.reg] {
				g.fail(n, "two outputs in register %s", RegName(o.reg, 8))
			}
			if clobbered[o.reg] {
				g.fail(n, "'asm' operand has impossible constraints: register %s is clobbered", RegName(o.reg, 8))
			}
			usedOut[o.reg] = true
			if o.reg2 != NoReg {
				usedOut[o.reg2] = true
			}
		}
	}
	for _, o := range ops[nOut:] {
		if regKind(o) && o.fixed && o.match < 0 {
			if clobbered[o.reg] {
				g.fail(n, "'asm' operand has impossible constraints: register %s is clobbered", RegName(o.reg, 8))
			}
			usedIn[o.reg] = true
			if o.reg2 != NoReg {
				usedIn[o.reg2] = true
			}
		}
	}
	for _, o := range ops[:nOut] {
		if regKind(o) && !o.fixed {
			take(o, usedOut, usedIn)
			usedOut[o.reg] = true
		}
	}
	for _, o := range ops[nOut:] {
		if o.match >= 0 {
			out := ops[o.match]
			o.kind, o.reg, o.reg2, o.st = out.kind, out.reg, out.reg2, out.st
			if out.kind == akMem {
				o.mem, o.viaAddr = out.mem, out.viaAddr
			}
			continue
		}
		if regKind(o) && !o.fixed {
			take(o, usedOut, usedIn)
			usedIn[o.reg] = true
		}
	}
	// memory operands reached through a pointer need an address register
	for _, o := range ops {
		if o.kind == akMem && o.viaAddr && !(o.match >= 0) {
			tmp := &asmOp{pool: asmGPPool}
			take(tmp, usedOut, usedIn)
			usedIn[tmp.reg] = true
			o.reg = tmp.reg
			o.mem = M(tmp.reg, 0)
		}
	}
	for _, o := range ops[nOut:] {
		if o.match >= 0 && ops[o.match].kind == akMem {
			o.mem = ops[o.match].mem
		}
	}

	// phase C: load the inputs
	// matching inputs of memory outputs are stored into the output first
	for _, o := range ops[nOut:] {
		if o.match >= 0 && ops[o.match].kind == akMem {
			out := ops[o.match]
			dst := out.mem
			if out.viaAddr {
				g.a.Mov(8, Q(RDI), M(RBP, int32(out.slot+16)))
				dst = M(RDI, 0)
			}
			g.copyToMem(dst, M(RBP, int32(o.slot)), out.ty.Size())
		}
	}
	for _, o := range ops {
		if o.kind != akXMM {
			continue
		}
		switch {
		case o.out && o.inout:
			src := o.mem
			if o.viaAddr {
				g.a.Mov(8, Q(RAX), M(RBP, int32(o.slot+16)))
				src = M(RAX, 0)
			}
			g.loadXMM(o.reg, o.ty, src)
		case !o.out:
			g.loadXMM(o.reg, o.ty, M(RBP, int32(o.slot)))
		}
	}
	for _, o := range ops {
		if o.kind != akReg && !(o.kind == akFlag && o.inout) {
			continue
		}
		switch {
		case o.out && o.inout:
			src := o.mem
			if o.viaAddr {
				g.a.Mov(8, Q(o.reg), M(RBP, int32(o.slot+16)))
				src = M(o.reg, 0)
			}
			if o.reg2 != NoReg {
				hi := src
				hi.Disp += 8
				g.a.Mov(8, Q(o.reg2), hi)
				g.a.Mov(8, Q(o.reg), src)
			} else {
				g.loadInto(o.reg, o.ty, src)
			}
		case !o.out:
			g.a.Mov(8, Q(o.reg), M(RBP, int32(o.slot)))
			if o.reg2 != NoReg {
				g.a.Mov(8, Q(o.reg2), M(RBP, int32(o.slot+8)))
			}
		}
	}
	for _, o := range ops {
		if o.kind == akMem && o.viaAddr && o.match < 0 {
			g.a.Mov(8, Q(o.reg), M(RBP, int32(o.slot+16)))
		}
	}
	// x87 inputs: st(1) first
	var x87In []*asmOp
	for _, o := range ops {
		if o.kind == akX87 && (!o.out || o.inout) {
			x87In = append(x87In, o)
		}
	}
	sort.SliceStable(x87In, func(i, j int) bool { return x87In[i].st > x87In[j].st })
	for i, o := range x87In {
		if i > 0 && x87In[i-1].st == o.st {
			g.fail(n, "two x87 inputs in the same stack register")
		}
		if o.out {
			src := o.mem
			if o.viaAddr {
				g.a.Mov(8, Q(RAX), M(RBP, int32(o.slot+16)))
				src = M(RAX, 0)
			}
			g.fldType(o.ty, src)
		} else {
			g.fldType(o.ty, M(RBP, int32(o.slot)))
		}
	}
	if len(x87In) == 1 && x87In[0].st == 1 {
		g.fail(n, "x87 operand in st(1) without one in st(0)")
	}

	// the template
	labels := map[string]*TextLabel{}
	var labelNames []string
	for i, l := range st.Labels {
		name := fmt.Sprintf(".Laltc.goto.%d.%d", g.uniq, i)
		g.uniq++
		labels[name] = &TextLabel{A: g.a, L: g.label(l.UniqueLabel), Defined: true}
		labelNames = append(labelNames, name)
		if g.listing != nil {
			fmt.Fprintf(g.listing, "\t.set\t%s, .L%d\n", name, int(g.label(l.UniqueLabel)))
		}
	}
	uniq := g.uniq
	g.uniq++
	text, err := asmSubstitute(st, ops, labelNames, uniq)
	if err != nil {
		g.fail(n, "%v", err)
	}
	g.assembleInFunc(n, text, labels)

	// phase D: outputs
	for _, o := range ops[:nOut] {
		if o.kind == akFlag {
			g.a.Setcc(o.cond, o.reg)
			g.a.Movzx(4, o.reg, 1, B(o.reg))
		}
	}
	// x87 results: st(0) then st(1), then the inputs left on the stack
	var x87Out []*asmOp
	for _, o := range ops[:nOut] {
		if o.kind == akX87 {
			x87Out = append(x87Out, o)
		}
	}
	sort.SliceStable(x87Out, func(i, j int) bool { return x87Out[i].st < x87Out[j].st })
	for _, o := range x87Out {
		g.fstpType(o.ty, M(RBP, int32(o.slot)))
	}
	if len(x87Out) == 1 && x87Out[0].st == 1 {
		g.fail(n, "x87 output in st(1) without one in st(0)")
	}
	left := 0
	for _, o := range x87In {
		if o.out || o.match >= 0 {
			continue
		}
		if !x87Clob[o.st] {
			left++
		}
	}
	for i := 0; i < left; i++ {
		g.a.FstpSt(0)
	}
	for _, o := range ops[:nOut] {
		switch o.kind {
		case akReg, akFlag:
			g.a.Mov(8, M(RBP, int32(o.slot)), Q(o.reg))
			if o.reg2 != NoReg {
				g.a.Mov(8, M(RBP, int32(o.slot+8)), Q(o.reg2))
			}
		case akXMM:
			switch o.ty.Size() {
			case 16:
				g.a.Movups(16, M(RBP, int32(o.slot)), X(o.reg))
			case 32:
				g.a.Movups(32, M(RBP, int32(o.slot)), R{o.reg, 32})
			default:
				g.a.enc([]byte{0x66}, false, []byte{0x0F, 0xD6}, o.reg.num(), M(RBP, int32(o.slot)), 0, false) // movq
			}
		}
	}
	for _, o := range ops[:nOut] {
		if o.kind == akMem {
			continue
		}
		dst := o.mem
		if o.viaAddr {
			g.a.Mov(8, Q(RDI), M(RBP, int32(o.slot+16)))
			dst = M(RDI, 0)
		}
		size := o.ty.Size()
		if o.kind == akFlag && o.ty.Kind == cc.TyBool {
			size = 1
		}
		g.copyToMem(dst, M(RBP, int32(o.slot)), size)
	}
}

func isAddressable(e *cc.Node) bool {
	switch e.Kind {
	case cc.NVar, cc.NDeref, cc.NMember:
		return !(e.Kind == cc.NMember && e.Member.Bitfield)
	case cc.NComma:
		return isAddressable(e.RHS)
	case cc.NCast:
		return e.Ty.IsRecord() && isAddressable(e.LHS)
	}
	return e.Ty != nil && (e.Ty.IsRecord() || e.Ty.Kind == cc.TyArray)
}

// asmLocation computes where an lvalue operand lives: directly addressable
// (locals, globals of this unit) or through an address saved in the slot
func (g *gen) asmLocation(n *cc.Node, o *asmOp, e *cc.Node) {
	if e.Kind == cc.NVar {
		v := e.Var
		switch {
		case v.IsLocal && !g.isOver(v) && v.Ty.Kind != cc.TyVLA:
			o.mem = M(RBP, int32(v.Offset))
			return
		case !v.IsLocal && !v.IsTLS && (g.isLocalDef(v) || v.IsStatic):
			o.mem = MSym(v.AsmName, 0)
			return
		}
	}
	g.genAddr(e)
	g.a.Mov(8, M(RBP, int32(o.slot+16)), Q(RAX))
	o.viaAddr = true
}

// spillValue stores the value just computed into a frame slot
func (g *gen) spillValue(n *cc.Node, t *cc.Type, off int64) {
	m := M(RBP, int32(off))
	switch {
	case t.Kind == cc.TyFloat:
		g.a.MovF(false, m, X(XMM0))
	case t.Kind == cc.TyDouble:
		g.a.MovF(true, m, X(XMM0))
	case t.Kind == cc.TyLongDouble:
		g.a.Fstp(10, m)
	case is128(t):
		g.a.Mov(8, m, Q(RAX))
		m.Disp += 8
		g.a.Mov(8, m, Q(RDX))
	case t.IsRecord():
		if t.Size() > 16 {
			g.fail(n, "asm operand of type '%s' is too large", t)
		}
		g.a.Mov(8, Q(RSI), Q(RAX))
		g.a.Lea(RDI, m)
		g.copyMem(RDI, RSI, t.Size())
	default:
		g.a.Mov(8, m, Q(RAX))
	}
}

func (g *gen) loadXMM(r Reg, t *cc.Type, m Mem) {
	switch {
	case t.Kind == cc.TyFloat || t.Size() == 4:
		g.a.MovF(false, X(r), m)
	case t.Size() == 16:
		g.a.Movups(16, X(r), m)
	case t.Size() == 32:
		g.a.Movups(32, R{r, 32}, m)
	default:
		g.a.enc([]byte{0xF3}, false, []byte{0x0F, 0x7E}, r.num(), m, 0, false) // movq
	}
}

func (g *gen) fldType(t *cc.Type, m Mem) {
	switch t.Kind {
	case cc.TyFloat:
		g.a.Fld(4, m)
	case cc.TyDouble:
		g.a.Fld(8, m)
	default:
		g.a.Fld(10, m)
	}
}

func (g *gen) fstpType(t *cc.Type, m Mem) {
	switch t.Kind {
	case cc.TyFloat:
		g.a.Fstp(4, m)
	case cc.TyDouble:
		g.a.Fstp(8, m)
	default:
		g.a.Fstp(10, m)
	}
}

// copyToMem copies n bytes from a frame slot to memory using rax
func (g *gen) copyToMem(dst, src Mem, n int64) {
	for i := int64(0); i < n; {
		c := int64(8)
		for c > n-i {
			c /= 2
		}
		s, d := src, dst
		s.Disp += int32(i)
		d.Disp += int32(i)
		g.a.Mov(int(c), Sized(RAX, int(c)), s)
		g.a.Mov(int(c), d, Sized(RAX, int(c)))
		i += c
	}
}

// assembleInFunc assembles text into the current function
func (g *gen) assembleInFunc(n *cc.Node, text string, labels map[string]*TextLabel) {
	env := &TextEnv{Section: g.asmSectionFor, Labels: labels, Listing: g.listing}
	if err := AssembleText(env, text); err != nil {
		g.fail(n, "%v", err)
	}
}

// ---- template substitution ----

func asmSubstitute(st *cc.AsmStmt, ops []*asmOp, labels []string, uniq int) (string, error) {
	s := st.Template
	var b strings.Builder
	nOps := len(ops)
	dialect := 0 // 0: outside {}, 1: in the AT&T alternative, 2: skipping others
	for i := 0; i < len(s); i++ {
		c := s[i]
		switch {
		case c == '{' && dialect == 0:
			dialect = 1
			continue
		case c == '|' && dialect == 1:
			dialect = 2
			continue
		case c == '}' && dialect != 0:
			dialect = 0
			continue
		case dialect == 2:
			continue
		}
		if c != '%' {
			b.WriteByte(c)
			continue
		}
		i++
		if i >= len(s) {
			return "", fmt.Errorf("invalid 'asm': %% at end of template")
		}
		c = s[i]
		switch c {
		case '%', '{', '|', '}':
			b.WriteByte(c)
			continue
		case '=':
			b.WriteString(strconv.Itoa(uniq))
			continue
		}
		mod := byte(0)
		if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') && i+1 < len(s) && (s[i+1] >= '0' && s[i+1] <= '9' || s[i+1] == '[') {
			mod = c
			i++
			c = s[i]
		}
		idx := -1
		switch {
		case c >= '0' && c <= '9':
			j := i
			for j < len(s) && s[j] >= '0' && s[j] <= '9' {
				j++
			}
			idx, _ = strconv.Atoi(s[i:j])
			i = j - 1
		case c == '[':
			j := strings.IndexByte(s[i:], ']')
			if j < 0 {
				return "", fmt.Errorf("invalid 'asm': missing ']'")
			}
			name := s[i+1 : i+j]
			i += j
			for k, o := range ops {
				if o.op.Name == name {
					idx = k
				}
			}
			if idx < 0 {
				for k, l := range st.Labels {
					if l.Label == name {
						idx = nOps + k
					}
				}
			}
			if idx < 0 {
				return "", fmt.Errorf("undefined named operand '%s'", name)
			}
		default:
			return "", fmt.Errorf("invalid 'asm': operand number missing after %%-letter")
		}
		if idx >= nOps {
			k := idx - nOps
			if k >= len(labels) {
				return "", fmt.Errorf("invalid 'asm': operand number out of range")
			}
			b.WriteString(labels[k])
			continue
		}
		text, err := formatAsmOperand(ops[idx], mod)
		if err != nil {
			return "", err
		}
		b.WriteString(text)
	}
	return b.String(), nil
}

func formatAsmOperand(o *asmOp, mod byte) (string, error) {
	size := int(o.ty.Size())
	if o.ty.Kind == cc.TyArray || o.ty.Kind == cc.TyFunc {
		size = 8
	}
	if mod == 'z' {
		switch size {
		case 1:
			return "b", nil
		case 2:
			return "w", nil
		case 4:
			return "l", nil
		case 8:
			return "q", nil
		}
		return "", fmt.Errorf("invalid operand size for operand code 'z'")
	}
	switch o.kind {
	case akReg, akFlag:
		if o.kind == akFlag {
			return "", fmt.Errorf("flag output operands cannot be used in the template")
		}
		switch mod {
		case 'b':
			size = 1
		case 'w':
			size = 2
		case 'k':
			size = 4
		case 'q':
			size = 8
		case 'h':
			if o.reg > RBX {
				return "", fmt.Errorf("invalid operand for code 'h'")
			}
			return RegName(AH+o.reg, 1), nil
		case 'H':
			return "", fmt.Errorf("invalid operand for code 'H'")
		case 'a':
			return "(" + RegName(o.reg, 8) + ")", nil
		}
		if size > 8 {
			size = 8
		}
		if size != 1 && size != 2 && size != 4 && size != 8 {
			size = 8
		}
		return RegName(o.reg, size), nil
	case akXMM:
		if o.ty != nil && o.ty.Size() == 32 && mod != 'x' {
			return RegName(o.reg, 32), nil
		}
		return RegName(o.reg, 16), nil
	case akX87:
		if o.st == 0 {
			return "%st", nil
		}
		return "%st(1)", nil
	case akMem:
		m := o.mem
		if mod == 'H' {
			m.Disp += 8
		}
		return fmtMemATT(m), nil
	case akImm:
		v := o.op.Val
		sym := o.op.Sym
		val := ""
		switch {
		case sym != "" && v > 0:
			val = fmt.Sprintf("%s+%d", sym, v)
		case sym != "" && v < 0:
			val = fmt.Sprintf("%s%d", sym, v)
		case sym != "":
			val = sym
		default:
			if mod == 'n' {
				v = -v
			}
			val = strconv.FormatInt(v, 10)
		}
		switch mod {
		case 'c', 'P', 'p', 'n', 'a':
			return val, nil
		}
		return "$" + val, nil
	}
	return "", fmt.Errorf("bad asm operand")
}

// fmtMemATT prints a memory operand in AT&T syntax
func fmtMemATT(m Mem) string {
	var sb strings.Builder
	if m.FS {
		sb.WriteString("%fs:")
	}
	if m.Sym != "" {
		sb.WriteString(m.Sym)
		if m.Disp > 0 {
			fmt.Fprintf(&sb, "+%d", m.Disp)
		} else if m.Disp < 0 {
			fmt.Fprintf(&sb, "%d", m.Disp)
		}
	} else if m.Disp != 0 || m.Base == NoReg && m.Index == NoReg {
		fmt.Fprintf(&sb, "%d", m.Disp)
	}
	if m.Base != NoReg || m.Index != NoReg {
		sb.WriteString("(")
		switch {
		case m.Base == RIP:
			sb.WriteString("%rip")
		case m.Base != NoReg:
			sb.WriteString(RegName(m.Base, 8))
		}
		if m.Index != NoReg {
			fmt.Fprintf(&sb, ",%s,%d", RegName(m.Index, 8), m.Scale)
		}
		sb.WriteString(")")
	}
	return sb.String()
}

// ---- sections and symbols of assembly text ----

type asmSection struct {
	name  string
	flags string
	a     *Asm
}

func (g *gen) asmSectionFor(name, flags string) (*Asm, error) {
	if name == ".text" {
		return g.a, nil
	}
	for _, s := range g.asmSecs {
		if s.name == name {
			return s.a, nil
		}
	}
	s := &asmSection{name: name, flags: flags, a: &Asm{SymPrefix: fmt.Sprintf("s%d", len(g.asmSecs))}}
	g.asmSecs = append(g.asmSecs, s)
	return s.a, nil
}

// asmSym collects what assembly directives say about a symbol
type asmSym struct {
	attrs  map[string]bool // .globl .weak .hidden .local ...
	typ    string          // "function", "object", "tls_object"
	size   int64
	common bool // .comm (global common symbol)
	csize  int64
	calign int64
}

func (g *gen) asmSymbol(name string) *asmSym {
	if g.topSyms == nil {
		g.topSyms = map[string]*asmSym{}
	}
	s := g.topSyms[name]
	if s == nil {
		s = &asmSym{attrs: map[string]bool{}}
		g.topSyms[name] = s
	}
	return s
}

// symbolDirective handles the symbol directives of file-scope assembly
func (g *gen) symbolDirective(dir string, args []string) error {
	switch dir {
	case ".type":
		if len(args) == 2 {
			t := strings.TrimLeft(strings.TrimSpace(args[1]), "@%\"")
			g.asmSymbol(strings.TrimSpace(args[0])).typ = strings.Trim(t, "\"")
		}
	case ".size":
		v, _ := strconv.ParseInt(args[1], 10, 64)
		g.asmSymbol(strings.TrimSpace(args[0])).size = v
	case ".comm", ".lcomm":
		name := args[0]
		size, _ := strconv.ParseInt(args[1], 10, 64)
		align := int64(1)
		if len(args) > 2 {
			align, _ = strconv.ParseInt(args[2], 10, 64)
		} else if dir == ".comm" {
			// GNU as: the largest power of two not above the size, at most 16
			for align < 16 && align*2 <= size {
				align *= 2
			}
		}
		sym := g.asmSymbol(name)
		if dir == ".lcomm" || sym.attrs[".local"] {
			a, _ := g.asmSectionFor(".bss", "")
			if align > a.MaxAlign {
				a.MaxAlign = align
			}
			for int64(len(a.Code))%max(align, 1) != 0 {
				a.Code = append(a.Code, 0)
			}
			l := a.NewLabel()
			a.labels[l] = len(a.Code)
			a.Code = append(a.Code, make([]byte, size)...)
			g.topLabels[name] = &TextLabel{A: a, L: l, Defined: true}
			if sym.typ == "" {
				sym.typ = "object"
			}
			sym.size = size
			return nil
		}
		sym.common, sym.csize, sym.calign = true, size, align
	default:
		for _, a := range args {
			g.asmSymbol(strings.TrimSpace(a)).attrs[dir] = true
		}
	}
	return nil
}

// topLevelAsm assembles the file-scope asm statements
func (g *gen) topLevelAsm() error {
	if len(g.prog.TopAsm) == 0 {
		return nil
	}
	g.topLabels = map[string]*TextLabel{}
	env := &TextEnv{Section: g.asmSectionFor, Labels: g.topLabels, Listing: g.listing, Symbol: g.symbolDirective, CFI: true}
	if len(g.prog.AsmToks) == 0 {
		env.FileName = g.obj.Name // an assembler source file
	}
	defer func() { g.asmFiles = env.Files }()
	for i, text := range g.prog.TopAsm {
		if err := AssembleText(env, text); err != nil {
			if env.FileName != "" {
				return err
			}
			pos := ""
			if i < len(g.prog.AsmToks) && g.prog.AsmToks[i] != nil {
				pos = g.prog.AsmToks[i].Pos() + ": "
			}
			return fmt.Errorf("%serror: %v", pos, err)
		}
	}
	return nil
}

// finishAsm lays out the sections created by assembly text and defines
// the label symbols; it returns relocations to convert with the code's
func (g *gen) finishAsm(text *elf.Section, syms map[string]*elf.Symbol) ([]pendingRelocs, error) {
	secOf := map[*Asm]*elf.Section{g.a: text}
	g.secOf = secOf
	var pend []pendingRelocs
	for _, s := range g.asmSecs {
		if err := s.a.Finish(); err != nil {
			return nil, err
		}
		kind := elf.SecData
		switch {
		case !strings.Contains(s.flags, "a") && (strings.HasPrefix(s.name, ".debug") || strings.HasPrefix(s.name, ".comment") ||
			strings.HasPrefix(s.name, ".note") || strings.HasPrefix(s.name, ".gnu.lto")):
			kind = elf.SecOther // not loaded: debug information, comments, notes
		case strings.HasPrefix(s.name, ".text") || strings.Contains(s.flags, "x") || s.name == ".init" || s.name == ".fini":
			// code; GNU as knows .init and .fini by name
			kind = elf.SecText
		case strings.HasPrefix(s.name, ".rodata"):
			kind = elf.SecROData
		case strings.HasPrefix(s.name, ".bss"):
			kind = elf.SecBSS
		case strings.HasPrefix(s.name, ".tdata"):
			kind = elf.SecTData
		case strings.HasPrefix(s.name, ".tbss"):
			kind = elf.SecTBSS
		case strings.HasPrefix(s.name, ".init_array"):
			kind = elf.SecInitArray
		case strings.HasPrefix(s.name, ".fini_array"):
			kind = elf.SecFiniArray
		case s.flags != "" && !strings.Contains(s.flags, "a"):
			kind = elf.SecOther
		case s.flags != "" && !strings.Contains(s.flags, "w"):
			kind = elf.SecROData
		}
		sec := g.obj.AddSection(s.name, kind, max(s.a.MaxAlign, 1))
		if kind == elf.SecBSS || kind == elf.SecTBSS {
			sec.Size = int64(len(s.a.Code))
		} else {
			sec.Data = s.a.Code
		}
		secOf[s.a] = sec
		pend = append(pend, pendingRelocs{sec, s.a.Relocs})
	}
	for a, sec := range secOf {
		for _, ls := range a.LabelSyms {
			typ := elf.SymNoType
			if sec.Kind == elf.SecTData || sec.Kind == elf.SecTBSS {
				typ = elf.SymTLS
			}
			syms[ls.Name] = g.obj.AddLocal(&elf.Symbol{Name: ls.Name, Bind: elf.BindLocal, Section: sec, Value: int64(a.LabelPos(ls.Label)), Type: typ})
		}
	}
	// symbols defined by file-scope asm
	relocNames := map[string]bool{}
	for _, pr := range pend {
		for _, r := range pr.relocs {
			relocNames[r.Sym] = true
		}
	}
	for _, r := range g.a.Relocs {
		relocNames[r.Sym] = true
	}
	names := make([]string, 0, len(g.topLabels))
	for name := range g.topLabels {
		names = append(names, name)
	}
	sort.Strings(names)
	for _, name := range names {
		tl := g.topLabels[name]
		if !tl.Defined {
			continue
		}
		if strings.HasPrefix(name, ".L") {
			// local labels get a symbol only when relocations refer to them by name
			// (a .set alias used before its definition)
			if _, used := relocNames[name]; !used {
				continue
			}
		}
		info := g.topSyms[name]
		if info == nil {
			info = &asmSym{attrs: map[string]bool{}}
		}
		var s *elf.Symbol
		global := info.attrs[".globl"] || info.attrs[".global"] || info.attrs[".weak"]
		if global || info.attrs[".hidden"] || info.attrs[".protected"] || info.attrs[".internal"] {
			s = g.obj.Symbol(name)
			s.Bind = elf.BindGlobal
			if info.attrs[".weak"] {
				s.Bind = elf.BindWeak
			}
			s.Hidden = info.attrs[".hidden"] || info.attrs[".internal"]
		} else if prev, ok := syms[name]; ok && prev.Section == nil {
			s = prev
			s.Bind = elf.BindLocal
		} else {
			s = g.obj.AddLocal(&elf.Symbol{Name: name, Bind: elf.BindLocal})
		}
		if s.Section != nil {
			return nil, fmt.Errorf("symbol '%s' is already defined", name)
		}
		s.Section = secOf[tl.A]
		s.Value = int64(tl.A.LabelPos(tl.L)) + tl.Off
		s.Size = info.size
		switch info.typ {
		case "function", "gnu_indirect_function", "STT_GNU_IFUNC":
			s.Type = elf.SymFunc
			s.IFunc = info.typ != "function"
			if s.Size == 0 && tl.Off == 0 {
				// a .set alias of a function (an ifunc and its resolver) gets its size
				for other, ol := range g.topLabels {
					if oi := g.topSyms[other]; other != name && ol.A == tl.A && ol.L == tl.L && ol.Off == 0 && oi != nil && oi.size > 0 {
						s.Size = oi.size
						break
					}
				}
			}
		case "object":
			s.Type = elf.SymObject
		case "tls_object":
			s.Type = elf.SymTLS
		}
		if k := s.Section.Kind; k == elf.SecTData || k == elf.SecTBSS {
			s.Type = elf.SymTLS
		}
		syms[name] = s
	}
	// common symbols and attributes of undefined symbols
	cnames := make([]string, 0, len(g.topSyms))
	for name := range g.topSyms {
		cnames = append(cnames, name)
	}
	sort.Strings(cnames)
	for _, name := range cnames {
		info := g.topSyms[name]
		if tl := g.topLabels[name]; tl != nil && tl.Defined {
			continue
		}
		switch {
		case info.common:
			s := g.obj.Symbol(name)
			if s.Section != nil {
				continue // defined by C code
			}
			s.Type = elf.SymCommon
			s.Value = max(info.calign, 1)
			s.Size = info.csize
			syms[name] = s
		case info.attrs[".weak"]:
			s := g.obj.Symbol(name)
			if s.Section == nil {
				s.Bind = elf.BindWeak
				syms[name] = s
			}
		}
	}
	return pend, nil
}

type pendingRelocs struct {
	sec    *elf.Section
	relocs []Reloc
}
