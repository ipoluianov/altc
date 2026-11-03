package cc

import "math/big"

// NodeKind is the kind of an AST node
type NodeKind uint8

const (
	NNull NodeKind = iota
	NAdd
	NSub
	NMul
	NDiv
	NMod
	NBitAnd
	NBitOr
	NBitXor
	NShl
	NShr
	NEq
	NNe
	NLt
	NLe
	NAssign
	NCond   // a ? b : c
	NComma  // a, b
	NMember // a.b
	NAddr   // &a
	NDeref  // *a
	NNot    // !a
	NBitNot // ~a
	NNeg    // -a
	NLogAnd
	NLogOr
	NReturn
	NIf
	NFor // for and while
	NDo
	NSwitch
	NCase
	NBlock
	NGoto
	NGotoExpr // goto *p
	NLabel
	NLabelVal // &&label
	NFuncall
	NExprStmt
	NStmtExpr
	NVar
	NNum
	NCast
	NMemzero
	NAsm
	NCas     // atomic compare and swap
	NExch    // atomic exchange
	NFetchOp // atomic fetch-and-op (Val holds the operation kind)
	NVLAPtr
	NVaStart
	NVaArg // address of the next variadic argument of type Ty.Base
	NVaCopy
	NAlloca
	NBuiltin // other builtins (Builtin holds the name)
	NFrameAddr
	NFence
)

// Node is an AST node. Expressions have Ty set after type checking.
type Node struct {
	Kind NodeKind
	Ty   *Type
	Tok  *Token

	LHS, RHS *Node

	// if / for / while / do / ?:
	Cond, Then, Els, Init, Inc *Node

	// block / statement expression
	Body []*Node

	// member access
	Member *Member

	// variable
	Var *Obj

	// constants
	Val  int64
	FVal float64
	LD   *big.Float // exact long double constant (may be nil)

	// function call
	FuncTy    *Type
	Args      []*Node
	RetBuffer *Obj

	// goto, labels, break, continue
	Label       string
	UniqueLabel string
	BrkLabel    string
	ContLabel   string

	// switch
	Cases      []*Node
	Default    *Node
	Begin, End int64 // case range

	// atomic
	CasAddr, CasOld, CasNew *Node

	Builtin string
	AsmText string
	Asm     *AsmStmt

	// blocks and loops that declare VLAs: slot for the saved stack pointer
	// (for a goto: the slot to restore before jumping out of VLA scopes)
	StackSave *Obj
	blocks    []*Node // enclosing blocks of a label or goto
}

// AsmStmt is an inline assembly statement
type AsmStmt struct {
	Template string
	Basic    bool // no operand sections: '%' is not special
	Outputs  []*AsmOperand
	Inputs   []*AsmOperand
	Clobbers []string
	Labels   []*Node // asm goto targets (their UniqueLabel is resolved like a goto)
}

// AsmOperand is an operand of an extended asm statement
type AsmOperand struct {
	Name       string // [name]
	Constraint string
	Expr       *Node
	IsConst    bool // constant value (immediate operands): Sym + Val
	Val        int64
	Sym        string
}

// InitReloc is an address stored in initialized data
type InitReloc struct {
	Offset int64
	Label  string
	Addend int64
	Minus  string // label difference: Label - Minus (both code labels of one function)
	Size   int64  // bytes to write for label differences
	Local  *Obj   // a string literal or static local (no global name)
}

// Obj is a variable or a function
type Obj struct {
	Name    string // C name
	AsmName string // symbol name (differs for static locals and __asm__ labels)
	Ty      *Type
	Tok     *Token
	IsLocal bool
	Align   int64

	// locals
	Offset  int64
	RegName string // local register variable: register asm("name")

	// globals
	IsFunction   bool
	IsDefinition bool
	IsStatic     bool
	IsExtern     bool
	IsInline     bool
	IsTLS        bool
	IsWeak       bool
	IsTentative  bool // no initializer: common/bss
	IsHidden     bool
	IsConst      bool // goes to .rodata
	IsUsed       bool // referenced (static inline functions are emitted only then)
	IsLiteral    bool // anonymous string literal / compound literal storage
	Section      string
	Alias        string
	Ctor, Dtor   bool
	Priority     int64 // constructor/destructor priority (0: none)
	InitData     []byte
	Relocs       []InitReloc

	// functions
	Params       []*Obj
	Body         *Node
	Locals       []*Obj
	VaArea       *Obj
	AllocaBottom *Obj
	StackSize    int64
	Noreturn     bool
	Refs         []*Obj // functions and globals referenced from this one
	HasFuncName  bool
	ConstExpr    *Node // value of a constexpr scalar (C23)
	LabelAddrs   bool  // labels of this function are used in static data
	InlineOnly   bool  // GNU extern inline body: never emitted, may be redefined
}

// LabelSymbol is the local symbol of a label whose address is stored in data
func LabelSymbol(fn, label string) string { return ".Llabel." + fn + "." + label }

// Program is a parsed translation unit
type Program struct {
	Globals []*Obj   // variables and functions in definition order
	TopAsm  []string // file-scope asm statements
	AsmToks []*Token // their positions (for diagnostics)
}
