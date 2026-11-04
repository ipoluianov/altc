package x64

import (
	"fmt"
	"strings"

	"github.com/ipoluianov/altc/cc"
)

// The IR backend: functions are lowered to an SSA intermediate
// representation, optimized, given registers by a priority-based
// allocator and emitted with the encoder. Functions using something the
// IR does not cover fall back to the stack-machine generator.
//
//	irlower.go   AST -> SSA (Braun et al.: variables become values directly)
//	iropt.go     folding, GVN, loop-invariant code motion, DCE, addressing
//	irregalloc.go liveness, coalescing of phis, register assignment
//	iremit.go    machine code

// irType is the machine type of a value. Integers narrower than 32 bits are
// kept in I32 values extended according to their C type.
type irType uint8

const (
	iNone irType = iota
	iI32
	iI64
	iF32
	iF64
	iV128 // a 128-bit vector (any element type)
	iV256 // a 256-bit vector (needs AVX)
)

func (t irType) isFloat() bool { return t == iF32 || t == iF64 }

func (t irType) isVec() bool { return t == iV128 || t == iV256 }

// inXMM tells whether values of the type live in vector registers
func (t irType) inXMM() bool { return t.isFloat() || t.isVec() }

func (t irType) size() int {
	switch t {
	case iI64, iF64:
		return 8
	case iV128:
		return 16
	case iV256:
		return 32
	}
	return 4
}

func (t irType) String() string {
	return [...]string{"void", "i32", "i64", "f32", "f64", "v128", "v256"}[t]
}

type irOp uint8

const (
	opInvalid   irOp = iota
	opConst          // AuxInt, or F for floats
	opParam          // AuxInt: parameter index
	opGlobal         // address of Sym
	opGlobalGOT      // address of Sym loaded from the GOT
	opTLS            // address of the thread-local Sym (Signed: defined here)
	opSlot           // address of a frame slot
	opCopy
	opPhi

	opAdd
	opSub
	opMul
	opDiv
	opUDiv
	opMod
	opUMod
	opAnd
	opOr
	opXor
	opShl
	opShr
	opSar
	opNeg
	opNot
	opMulHi // high 64 bits of a 64x64 product (Signed: signed)

	// integer comparisons: i32 0 or 1; the operand type is Args[0].Type
	opEq
	opNe
	opLt
	opLe
	opGt
	opGe
	opULt
	opULe
	opUGt
	opUGe

	opFAdd
	opFSub
	opFMul
	opFDiv
	opFNeg

	// float comparisons (false when unordered, except FNe)
	opFEq
	opFNe
	opFLt
	opFLe
	opFGt
	opFGe

	opSExt8 // within the result type
	opSExt16
	opZExt8
	opZExt16
	opSExt32 // i32 -> i64
	opZExt32
	opTrunc  // i64 -> i32
	opCvtI2F // signed integer -> float
	opCvtU2F // unsigned i64 -> float
	opCvtF2I // float -> signed integer (truncating)
	opCvtF2U // float -> unsigned i64
	opFExt   // f32 -> f64
	opFTrunc // f64 -> f32
	opFAbs
	opFBits   // the bits of a float as an integer
	opBuiltin // unary integer builtin Sym on Args[0]; AuxInt: operand size

	opLoad    // AuxInt: size; Signed: sign-extends; address in Args (see irMem)
	opStore   // Args[0]: the value
	opMemzero // AuxInt: size; Args[0]: address
	opMemcpy  // AuxInt: size; Args[0]: destination, Args[1]: source
	opCall    // Args: arguments, then the callee when indirect
	opFence
	opTrap
	opAsm        // inline assembly: Args are its operand values and addresses
	opVInsn      // vector instruction Sym on Args (AuxInt: immediate when Signed)
	opVBroadcast // scalar to all elements; Sym: element kind (b w d q ps pd)
	opNumOps
)

var opNames = [...]string{
	"invalid", "const", "param", "global", "globalgot", "tls", "slot", "copy", "phi",
	"add", "sub", "mul", "div", "udiv", "mod", "umod", "and", "or", "xor", "shl", "shr", "sar", "neg", "not", "mulhi",
	"eq", "ne", "lt", "le", "gt", "ge", "ult", "ule", "ugt", "uge",
	"fadd", "fsub", "fmul", "fdiv", "fneg",
	"feq", "fne", "flt", "fle", "fgt", "fge",
	"sext8", "sext16", "zext8", "zext16", "sext32", "zext32", "trunc",
	"cvti2f", "cvtu2f", "cvtf2i", "cvtf2u", "fext", "ftrunc", "fabs", "fbits", "builtin",
	"load", "store", "memzero", "memcpy", "call", "fence", "trap", "asm", "vinsn", "vbroadcast",
}

// pure operations depend only on their arguments
func (op irOp) pure() bool {
	return op >= opConst && op <= opBuiltin && op != opParam && op != opPhi || op == opVInsn || op == opVBroadcast
}

// traps tells whether an operation may fault (it is not hoisted)
func (op irOp) traps() bool {
	return op == opDiv || op == opUDiv || op == opMod || op == opUMod
}

func (op irOp) commutative() bool {
	switch op {
	case opAdd, opMul, opAnd, opOr, opXor, opEq, opNe, opFAdd, opFMul, opFEq, opFNe:
		return true
	}
	return false
}

func (op irOp) isCompare() bool {
	return op >= opEq && op <= opUGe || op >= opFEq && op <= opFGe
}

// remat values are recomputed where they are used instead of living in a
// register
func (op irOp) remat() bool {
	return op == opConst || op == opGlobal || op == opSlot
}

// irMem is the folded address of a load or store: base + index*scale +
// disp, where the base is a value, a frame slot or a symbol
type irMem struct {
	HasBase  bool // a value in Args
	HasIndex bool // a value in Args after the base
	Scale    int
	Disp     int64
	Slot     *irSlot
	Sym      string
}

// irCall describes a call: Args[:NArgs] go to Locs, then comes the
// callee of an indirect call
type irCall struct {
	Direct    string // symbol of a direct call
	Variadic  bool   // al holds the number of vector registers
	NArgs     int
	Locs      []callLoc
	NFP       int   // vector registers used
	StackSize int64 // bytes of stack arguments
	RetSlot   *irSlot
	RetChunks []irChunk // a struct result in registers, stored to RetSlot
}

// callLoc is where an argument goes: a register, or the stack (a struct
// copied there when Copy > 0; Args holds its address then)
type callLoc struct {
	Reg   Reg
	Stack int64
	Copy  int64
}

type irValue struct {
	ID     int
	Op     irOp
	Type   irType
	Args   []*irValue
	AuxInt int64
	F      float64
	Sym    string
	Slot   *irSlot
	Mem    *irMem // loads and stores after address folding
	Data   []byte // the bytes of a vector constant
	Asm    *irAsm // inline assembly
	Signed bool
	Call   *irCall
	Block  *irBlock

	tok   *cc.Token // the source position (debug line tables)
	fwd   *irValue  // replacement of a removed value
	uses  int
	fused bool // a comparison that only sets flags for its branch
	vol   bool // a volatile load
	loc   irLoc
	pos   int // position in the linear order
}

type blockKind uint8

const (
	bJump blockKind = iota
	bBranch
	bRet
	bNone // not terminated yet / unreachable end
)

type irBlock struct {
	ID      int
	Phis    []*irValue
	Instrs  []*irValue
	Kind    blockKind
	Ctrl    *irValue   // branch condition or returned value
	RetVals []*irValue // eightbytes of a struct returned in registers
	Succs   []*irBlock
	Preds   []*irBlock

	sealed     bool
	incomplete map[*cc.Obj]*irValue
	defs       map[*cc.Obj]*irValue

	label     Label
	depth     int // loop depth
	idom      *irBlock
	domIn     int // dominator tree numbering
	domOut    int
	rpo       int
	start     int // positions in the linear order
	end       int
	isPreHead bool
}

// irSlot is a frame area: a local in memory or a spill slot. Incoming
// stack arguments have fixed positive offsets.
type irSlot struct {
	Size, Align int64
	Offset      int64
	Fixed       bool
	Name        string
}

type irFunc struct {
	Name    string
	Blocks  []*irBlock
	Entry   *irBlock
	Slots   []*irSlot
	Params  []*irValue // in parameter order; nil for unused ones
	PLocs   []irParamLoc
	RetType irType
	VaSlot  *irSlot // register save area of a variadic function

	isa         ISA       // instruction set extensions for the function
	noVectorize bool      // no loop vectorization
	fastMath    bool      // floating-point reductions may be reassociated
	curTok      *cc.Token // the statement being lowered (the position of new values)
	nextID      int
	nextBlk     int
}

// irParamLoc is where the ABI passes a parameter
type irParamLoc struct {
	Reg   Reg   // NoReg when on the stack
	Stack int64 // offset from rbp of a stack parameter
	Type  irType
}

func (f *irFunc) newBlock() *irBlock {
	b := &irBlock{ID: f.nextBlk, Kind: bNone, defs: map[*cc.Obj]*irValue{}}
	f.nextBlk++
	f.Blocks = append(f.Blocks, b)
	return b
}

func (f *irFunc) newValue(b *irBlock, op irOp, t irType, args ...*irValue) *irValue {
	v := &irValue{ID: f.nextID, Op: op, Type: t, Args: args, Block: b, tok: f.curTok}
	f.nextID++
	return v
}

func addEdge(from, to *irBlock) {
	from.Succs = append(from.Succs, to)
	to.Preds = append(to.Preds, from)
}

// resolve follows the replacements of removed phis
func (v *irValue) resolve() *irValue {
	for v.fwd != nil {
		v = v.fwd
	}
	return v
}

func (v *irValue) isConst() bool { return v.Op == opConst }

// ---- printing (for debugging) ----

func (v *irValue) String() string {
	if v == nil {
		return "<nil>"
	}
	return fmt.Sprintf("v%d", v.ID)
}

func (v *irValue) longString() string {
	var sb strings.Builder
	fmt.Fprintf(&sb, "v%d = %s.%s", v.ID, opNames[v.Op], v.Type)
	for _, a := range v.Args {
		fmt.Fprintf(&sb, " %s", a.resolve())
	}
	switch v.Op {
	case opConst:
		if v.Type.isFloat() {
			fmt.Fprintf(&sb, " %g", v.F)
		} else {
			fmt.Fprintf(&sb, " %d", v.AuxInt)
		}
	case opParam, opLoad, opMemzero, opMemcpy:
		fmt.Fprintf(&sb, " [%d]", v.AuxInt)
	case opGlobal, opGlobalGOT:
		fmt.Fprintf(&sb, " %s", v.Sym)
	case opSlot:
		fmt.Fprintf(&sb, " %s", v.Slot.Name)
	case opCall:
		if v.Call.Direct != "" {
			fmt.Fprintf(&sb, " %s", v.Call.Direct)
		}
	}
	if m := v.Mem; m != nil {
		fmt.Fprintf(&sb, " mem{base=%v idx=%v*%d disp=%d sym=%s slot=%v}", m.HasBase, m.HasIndex, m.Scale, m.Disp, m.Sym, m.Slot != nil)
	}
	if v.loc.kind != locNone {
		fmt.Fprintf(&sb, " @%s", v.loc)
	}
	return sb.String()
}

func (f *irFunc) String() string {
	var sb strings.Builder
	fmt.Fprintf(&sb, "func %s\n", f.Name)
	for _, b := range f.Blocks {
		fmt.Fprintf(&sb, "b%d: (preds", b.ID)
		for _, p := range b.Preds {
			fmt.Fprintf(&sb, " b%d", p.ID)
		}
		fmt.Fprintf(&sb, ") depth=%d\n", b.depth)
		for _, v := range b.Phis {
			fmt.Fprintf(&sb, "  %s\n", v.longString())
		}
		for _, v := range b.Instrs {
			fmt.Fprintf(&sb, "  %s\n", v.longString())
		}
		switch b.Kind {
		case bJump:
			fmt.Fprintf(&sb, "  jump b%d\n", b.Succs[0].ID)
		case bBranch:
			fmt.Fprintf(&sb, "  branch %s b%d b%d\n", b.Ctrl.resolve(), b.Succs[0].ID, b.Succs[1].ID)
		case bRet:
			fmt.Fprintf(&sb, "  ret %v\n", b.Ctrl)
		default:
			fmt.Fprintf(&sb, "  <none>\n")
		}
	}
	return sb.String()
}
