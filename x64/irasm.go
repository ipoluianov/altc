package x64

import (
	"regexp"
	"strings"

	"github.com/ipoluianov/altc/cc"
)

// Inline assembly in the IR. Lowering evaluates the operands: input
// values, and the addresses where outputs go (a temporary for outputs
// that are SSA variables, read back afterwards). Emission stores them in
// the operand slots of the frame and hands over to the inline assembly
// engine shared with the stack-machine generator. An asm statement may
// use any register, so every value living across it is in memory.

type asmMode uint8

const (
	asmNone  asmMode = iota // an immediate operand
	asmValue                // the value is stored in the slot
	asmAddr                 // the address is stored at slot+16
	asmCopy                 // a long double input: the 16 bytes at the address go to the slot
)

type irAsmOp struct {
	slot *irSlot
	mode asmMode
	arg  int // index in Args (-1: none)
}

type irAsm struct {
	n   *cc.Node
	ops []irAsmOp
}

func (b *irBuilder) asmStmt(n *cc.Node) {
	st := n.Asm
	if len(st.Labels) > 0 {
		unsupported("asm goto")
	}
	info := &irAsm{n: n}
	var args []*irValue
	type back struct {
		v   *cc.Obj
		tmp *irSlot
		ty  *cc.Type
	}
	var writeBack []back
	if !st.Basic {
		ops, _ := b.g.asmOperands(n)
		for i, o := range ops {
			e := o.op.Expr
			ao := irAsmOp{slot: b.newSlot(32, 16), arg: -1}
			switch {
			case o.out:
				if e.Kind == cc.NMember && e.Member.Bitfield {
					unsupported("bit-field asm output")
				}
				var addr *irValue
				if e.Kind == cc.NVar && b.promoted[e.Var] {
					tmp := b.newSlot(max(e.Ty.Size(), 8), 16)
					addr = b.slotRef(tmp)
					if o.inout || matchedOutput(ops, i) {
						b.store(addr, b.readVar(e.Var), e.Ty)
					}
					writeBack = append(writeBack, back{e.Var, tmp, e.Ty})
				} else {
					addr = b.addr(e)
				}
				ao.mode, ao.arg = asmAddr, len(args)
				args = append(args, addr)
			case o.kind == akImm:
				ao.mode = asmNone
			case o.kind == akMem && o.match < 0 && isAddressable(e) && !(e.Kind == cc.NVar && b.promoted[e.Var]):
				ao.mode, ao.arg = asmAddr, len(args)
				args = append(args, b.addr(e))
			case isLD(e.Ty):
				ao.mode, ao.arg = asmCopy, len(args)
				args = append(args, b.expr(e)) // the address of the value
			default:
				var x *irValue
				if isAddrType(e.Ty) {
					x = b.addr(e)
				} else {
					x = b.expr(e)
				}
				if x == nil {
					unsupported("void asm operand")
				}
				ao.mode, ao.arg = asmValue, len(args)
				args = append(args, x)
			}
			info.ops = append(info.ops, ao)
		}
	}
	v := b.add(opAsm, iNone, args...)
	v.Asm = info
	for _, w := range writeBack {
		b.writeVar(w.v, b.load(b.slotRef(w.tmp), w.ty))
	}
}

// matchedOutput tells whether an input refers to output i ("0", "[name]")
func matchedOutput(ops []*asmOp, i int) bool {
	for _, o := range ops {
		if !o.out && o.match == i {
			return true
		}
	}
	return false
}

func (e *irEmitter) asm(v *irValue) {
	g, a := e.g, e.a
	info := v.Asm
	n := info.n
	if n.Asm.Basic {
		g.genAsm(n)
		return
	}
	st := n.Asm
	all := append(append([]*cc.AsmOperand(nil), st.Outputs...), st.Inputs...)
	for i, op := range all {
		g.asmSlots[op] = info.ops[i].slot.Offset
	}
	ops, nOut := g.asmOperands(n)
	clobbered, x87Clob := g.asmClobbers(n)
	for i, o := range ops {
		ao := info.ops[i]
		off := int32(ao.slot.Offset)
		switch ao.mode {
		case asmAddr:
			// frame and static locations are used directly, like the
			// stack-machine generator does (so "8+%1" works)
			switch x := v.Args[ao.arg]; x.Op {
			case opSlot:
				o.mem = M(RBP, int32(x.Slot.Offset))
			case opGlobal:
				o.mem = MSym(x.Sym, 0)
			default:
				a.Mov(8, M(RBP, off+16), Q(e.ireg(x, RAX)))
				o.viaAddr = true
			}
		case asmValue:
			x := v.Args[ao.arg]
			m := M(RBP, off)
			switch {
			case x.Type.inXMM():
				e.xstore(m, e.freg(x, XMM15), x.Type)
			case x.Type == iI32:
				r := e.ireg(x, R11)
				if o.ty.IsUnsigned() {
					a.Mov(4, D(R11), D(r))
				} else {
					a.Movsx(8, R11, 4, D(r))
				}
				a.Mov(8, m, Q(R11))
			default:
				a.Mov(8, m, Q(e.ireg(x, R11)))
			}
			if o.kind == akMem {
				o.mem = m
			}
		case asmCopy:
			m := M(RBP, off)
			src := e.ireg(v.Args[ao.arg], RAX)
			for k := int64(0); k < 16; k += 8 {
				a.Mov(8, Q(R11), mdisp(M(src, 0), k))
				a.Mov(8, mdisp(m, k), Q(R11))
			}
			if o.kind == akMem {
				o.mem = m
			}
		}
	}
	g.asmRun(n, ops, nOut, clobbered, x87Clob)
}

var asmNamedLabel = regexp.MustCompile(`(^|[\s;])[A-Za-z_.$][\w.$]*:`)

// asmDefinesLabels tells whether a function has asm text defining named
// labels: copies of it (inlining) would define them twice
func asmDefinesLabels(fn *cc.Obj) bool {
	found := false
	cc.Walk(fn.Body, func(n *cc.Node) {
		if n.Kind == cc.NAsm && n.Asm != nil && !strings.Contains(n.Asm.Template, "%=") &&
			asmNamedLabel.MatchString(n.Asm.Template) {
			found = true
		}
	})
	return found
}
