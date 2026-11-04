package x64

import (
	"sort"

	"github.com/ipoluianov/altc/cc"
)

// Register allocation of local variables.
//
// Integer and pointer locals whose address is never taken live in the
// callee-saved registers rbx and r12-r15 instead of the frame. The
// registers keep the value extended to 64 bits, like rax does, so a read
// is a single register move. Calls preserve these registers, so nothing
// has to be saved around them; the prologue saves the registers the
// function uses and the epilogue restores them.

// returnsTwice lists functions after which callee-saved registers may hold
// stale values
var returnsTwice = map[string]bool{
	"setjmp": true, "_setjmp": true, "sigsetjmp": true, "__sigsetjmp": true,
	"savectx": true, "vfork": true, "getcontext": true, "__builtin_setjmp": true,
}

// regCandidate tells whether a local can live in a register at all
func regCandidate(fn, v *cc.Obj) bool {
	t := v.Ty
	if !v.IsLocal || v.RegName != "" || t.Volatile || t.Atomic {
		return false
	}
	if v == fn.VaArea || v == fn.AllocaBottom || v.Align > 8 {
		return false
	}
	switch {
	case t.Kind == cc.TyPtr:
	case t.IsInteger() && !is128(t) && t.Size() <= 8:
	case isSSE(t):
	default:
		return false
	}
	return true
}

// allocRegVars chooses the locals of fn that live in registers
func (g *gen) allocRegVars(fn *cc.Obj) {
	g.regVars = map[*cc.Obj]Reg{}
	g.regSaved = nil
	g.tempRegs = nil
	g.xmmVars = nil
	g.tempTop, g.tempUsed = 0, 0
	if g.noRegAlloc || fn.Body == nil || hasAsm(fn.Body) {
		return
	}
	excluded := map[*cc.Obj]bool{}
	weight := map[*cc.Obj]int{}
	disable := false
	calls := 0 // calls weighted like the uses

	// addrOf marks the variables whose address an lvalue expression takes
	var addrOf func(n *cc.Node)
	addrOf = func(n *cc.Node) {
		if n == nil {
			return
		}
		switch n.Kind {
		case cc.NVar:
			excluded[n.Var] = true
		case cc.NComma:
			addrOf(n.RHS)
		case cc.NMember, cc.NCast:
			addrOf(n.LHS)
		case cc.NCond:
			addrOf(n.Then)
			addrOf(n.Els)
		case cc.NAssign:
			addrOf(n.LHS)
		}
	}

	var walk func(n *cc.Node, w int)
	walk = func(n *cc.Node, w int) {
		if n == nil {
			return
		}
		switch n.Kind {
		case cc.NVar:
			weight[n.Var] += w
		case cc.NAddr:
			addrOf(n.LHS)
		case cc.NFuncall:
			calls += w
			if f := n.LHS; f != nil && f.Kind == cc.NVar && returnsTwice[f.Var.Name] {
				disable = true
			}
		case cc.NFor, cc.NDo:
			inner := min(w*8, 1<<20)
			walk(n.Init, w)
			walk(n.Cond, inner)
			walk(n.Inc, inner)
			walk(n.Then, inner)
			walk(n.Els, w)
			walk(n.LHS, w)
			walk(n.RHS, w)
			for _, b := range n.Body {
				walk(b, inner)
			}
			return
		}
		walk(n.LHS, w)
		walk(n.RHS, w)
		walk(n.Cond, w)
		walk(n.Then, w)
		walk(n.Els, w)
		walk(n.Init, w)
		walk(n.Inc, w)
		walk(n.CasAddr, w)
		walk(n.CasOld, w)
		walk(n.CasNew, w)
		for _, b := range n.Body {
			walk(b, w)
		}
		for _, a := range n.Args {
			walk(a, w)
		}
	}
	walk(fn.Body, 1)
	if disable {
		return
	}
	var cands, fcands []*cc.Obj
	for _, v := range fn.Locals {
		if weight[v] > 0 && !excluded[v] && regCandidate(fn, v) {
			if isSSE(v.Ty) {
				// xmm registers are saved around every call: worth it
				// only for variables used more often than calls happen
				if weight[v] > 2*calls {
					fcands = append(fcands, v)
				}
				continue
			}
			cands = append(cands, v)
		}
	}
	sort.SliceStable(fcands, func(i, j int) bool { return weight[fcands[i]] > weight[fcands[j]] })
	g.xmmVars = nil
	for i, v := range fcands {
		if i == 8 {
			break
		}
		g.regVars[v] = XMM8 + Reg(i)
		g.xmmVars = append(g.xmmVars, v)
	}
	// the most used variables first; the order of Locals breaks ties so
	// that the output is deterministic
	sort.SliceStable(cands, func(i, j int) bool { return weight[cands[i]] > weight[cands[j]] })
	for i, v := range cands {
		if i == len(calleeSaved) {
			break
		}
		g.regVars[v] = calleeSaved[i]
		g.regSaved = append(g.regSaved, calleeSaved[i])
	}
	// the rest serve as temporaries
	free := calleeSaved[len(g.regSaved):]
	if g.tempLimit >= 0 && g.tempLimit < len(free) {
		free = free[:g.tempLimit]
	}
	g.tempRegs = free
	g.regSaved = append(g.regSaved, free...)
}

// saveTemp moves rax to a free temporary register
func (g *gen) saveTemp() (Reg, bool) {
	if g.tempTop >= len(g.tempRegs) {
		return 0, false
	}
	r := g.tempRegs[g.tempTop]
	g.tempTop++
	g.tempUsed = max(g.tempUsed, g.tempTop)
	g.a.Mov(8, Q(r), Q(RAX))
	return r, true
}

// saveTempF moves xmm0 to a free temporary register
func (g *gen) saveTempF() (Reg, bool) {
	if g.tempTop >= len(g.tempRegs) {
		return 0, false
	}
	r := g.tempRegs[g.tempTop]
	g.tempTop++
	g.tempUsed = max(g.tempUsed, g.tempTop)
	g.a.MovqFromX(8, r, XMM0)
	return r, true
}

// spillXMM saves the xmm register variables before a call
func (g *gen) spillXMM() {
	for _, v := range g.xmmVars {
		g.a.MovF(isDouble(v.Ty), M(RBP, int32(v.Offset)), X(g.regVars[v]))
	}
}

// reloadXMM restores the xmm register variables after a call
func (g *gen) reloadXMM() {
	for _, v := range g.xmmVars {
		g.a.MovF(isDouble(v.Ty), X(g.regVars[v]), M(RBP, int32(v.Offset)))
	}
}

// freeTemp releases the most recent temporary register
func (g *gen) freeTemp() { g.tempTop-- }

// varReg returns the register of a local living in one
func (g *gen) varReg(v *cc.Obj) (Reg, bool) {
	r, ok := g.regVars[v]
	return r, ok
}

// regVarOperand returns the register operand of a node that reads a
// register variable
func (g *gen) regVarOperand(n *cc.Node, sz int) (Operand, bool) {
	if n.Kind != cc.NVar {
		return nil, false
	}
	r, ok := g.regVars[n.Var]
	if !ok {
		return nil, false
	}
	return Sized(r, sz), true
}

// extendInto copies the integer in src to dst extended to 64 bits as the
// type requires
func (g *gen) extendInto(dst Reg, t *cc.Type, src Reg) {
	uns := t.IsUnsigned() || t.Kind == cc.TyPtr
	switch t.Size() {
	case 1:
		if uns {
			g.a.Movzx(4, dst, 1, B(src))
		} else {
			g.a.Movsx(8, dst, 1, B(src))
		}
	case 2:
		if uns {
			g.a.Movzx(4, dst, 2, Sized(src, 2))
		} else {
			g.a.Movsx(8, dst, 2, Sized(src, 2))
		}
	case 4:
		if uns {
			g.a.Mov(4, D(dst), D(src))
		} else {
			g.a.Movsx(8, dst, 4, D(src))
		}
	default:
		if dst != src {
			g.a.Mov(8, Q(dst), Q(src))
		}
	}
}

// regFrame reserves the save area of the registers holding variables
func (g *gen) regFrame(offset int64) int64 {
	g.regSaveOff = 0
	if len(g.regSaved) == 0 {
		return offset
	}
	offset = alignTo(offset+int64(8*len(g.regSaved)), 8)
	g.regSaveOff = -offset
	return offset
}

func (g *gen) saveVarRegs() {
	for i, r := range g.regSaved {
		g.a.Mov(8, M(RBP, int32(g.regSaveOff)+int32(8*i)), Q(r))
	}
}

func (g *gen) restoreVarRegs() {
	for i, r := range g.regSaved {
		g.a.Mov(8, Q(r), M(RBP, int32(g.regSaveOff)+int32(8*i)))
	}
}
