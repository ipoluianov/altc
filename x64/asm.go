// Package x64 generates x86-64 machine code: an instruction encoder
// (this file) and the C code generator built on it.
package x64

import (
	"fmt"
	"strings"

	"github.com/ipoluianov/altc/elf"
)

// Reg is a general-purpose (0..15) or XMM (16..31) register
type Reg int8

const (
	RAX Reg = iota
	RCX
	RDX
	RBX
	RSP
	RBP
	RSI
	RDI
	R8
	R9
	R10
	R11
	R12
	R13
	R14
	R15
	XMM0
	XMM1
	XMM2
	XMM3
	XMM4
	XMM5
	XMM6
	XMM7
	XMM8
	XMM9
	XMM10
	XMM11
	XMM12
	XMM13
	XMM14
	XMM15

	NoReg Reg = -1
	RIP   Reg = -2
)

func (r Reg) num() int { return int(r) & 15 }

func (r Reg) IsXMM() bool { return r >= XMM0 && r <= XMM15 }

var regNames = [4][16]string{
	{"al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"},
	{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"},
	{"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"},
	{"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"},
}

func sizeIdx(size int) int {
	switch size {
	case 1:
		return 0
	case 2:
		return 1
	case 4:
		return 2
	}
	return 3
}

// RegName returns the AT&T name of a register of the given size
func RegName(r Reg, size int) string {
	if r.IsXMM() {
		return fmt.Sprintf("%%xmm%d", r-XMM0)
	}
	if r == RIP {
		return "%rip"
	}
	return "%" + regNames[sizeIdx(size)][r]
}

// Operand of an instruction: R, Mem or Imm
type Operand interface{ String() string }

// R is a register operand
type R struct {
	Reg  Reg
	Size int // 1, 2, 4, 8; 16 for XMM
}

func (r R) String() string { return RegName(r.Reg, r.Size) }

// Q, D, W, B make 64, 32, 16 and 8-bit register operands; X an XMM one
func Q(r Reg) R { return R{r, 8} }
func D(r Reg) R { return R{r, 4} }
func W(r Reg) R { return R{r, 2} }
func B(r Reg) R { return R{r, 1} }
func X(r Reg) R { return R{r, 16} }

// Sized returns the register as an operand of size n
func Sized(r Reg, n int) R { return R{r, n} }

// Relocation kinds of memory operands that refer to symbols
type RelKind int

const (
	RelPC      RelKind = iota // sym(%rip)
	RelGOT                    // sym@GOTPCREL(%rip)
	RelTPOFF                  // sym@tpoff(base) : absolute 32-bit TLS offset
	RelGOTTPOF                // sym@gottpoff(%rip)
	RelAbs32S                 // absolute sign-extended 32-bit address
)

// Mem is a memory operand: Seg:Disp+Sym(Base, Index, Scale)
type Mem struct {
	Base  Reg
	Index Reg
	Scale int
	Disp  int32
	Sym   string
	Rel   RelKind
	FS    bool // %fs: segment override
}

func (m Mem) String() string {
	var sb strings.Builder
	if m.FS {
		sb.WriteString("%fs:")
	}
	if m.Sym != "" {
		sb.WriteString(m.Sym)
		switch m.Rel {
		case RelGOT:
			sb.WriteString("@GOTPCREL")
		case RelTPOFF:
			sb.WriteString("@tpoff")
		case RelGOTTPOF:
			sb.WriteString("@gottpoff")
		}
		if m.Disp > 0 {
			fmt.Fprintf(&sb, "+%d", m.Disp)
		} else if m.Disp < 0 {
			fmt.Fprintf(&sb, "%d", m.Disp)
		}
	} else if m.Disp != 0 || (m.Base == NoReg && m.Index == NoReg) {
		fmt.Fprintf(&sb, "%d", m.Disp)
	}
	if m.Base != NoReg || m.Index != NoReg {
		sb.WriteString("(")
		if m.Base != NoReg {
			sb.WriteString(RegName(m.Base, 8))
		}
		if m.Index != NoReg {
			fmt.Fprintf(&sb, ",%s,%d", RegName(m.Index, 8), m.Scale)
		}
		sb.WriteString(")")
	}
	return sb.String()
}

// M makes a [base+disp] memory operand
func M(base Reg, disp int32) Mem { return Mem{Base: base, Index: NoReg, Disp: disp} }

// MI makes a [base+index*scale+disp] memory operand
func MI(base, index Reg, scale int, disp int32) Mem {
	return Mem{Base: base, Index: index, Scale: scale, Disp: disp}
}

// MSym makes a RIP-relative operand sym+disp(%rip)
func MSym(sym string, disp int32) Mem {
	return Mem{Base: RIP, Index: NoReg, Sym: sym, Disp: disp}
}

// MGot makes sym@GOTPCREL(%rip)
func MGot(sym string) Mem { return Mem{Base: RIP, Index: NoReg, Sym: sym, Rel: RelGOT} }

// Imm is an immediate operand
type Imm int64

func (i Imm) String() string { return fmt.Sprintf("$%d", int64(i)) }

// Label is a code position used by jumps
type Label int

// Reloc is a relocation in the produced code
type Reloc struct {
	Offset int64
	Type   uint32
	Sym    string
	Addend int64
}

type fixup struct {
	at    int // offset of the rel32 field
	label Label
}

// Asm accumulates machine code for one section
type Asm struct {
	Code    []byte
	Relocs  []Reloc
	labels  []int
	fixups  []fixup
	Listing *strings.Builder // when not nil, AT&T text of every instruction is appended
}

// NewLabel creates an unbound label
func (a *Asm) NewLabel() Label {
	a.labels = append(a.labels, -1)
	return Label(len(a.labels) - 1)
}

// Bind places the label at the current position
func (a *Asm) Bind(l Label) {
	a.labels[l] = len(a.Code)
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, ".L%d:\n", int(l))
	}
}

// LabelPos is the bound position of a label (-1 when unbound)
func (a *Asm) LabelPos(l Label) int { return a.labels[l] }

// Comment adds a line to the listing
func (a *Asm) Comment(format string, args ...any) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\t# "+format+"\n", args...)
	}
}

// Finish resolves the label references; it returns an error for unbound labels
func (a *Asm) Finish() error {
	for _, f := range a.fixups {
		pos := a.labels[f.label]
		if pos < 0 {
			return fmt.Errorf("unbound label .L%d", int(f.label))
		}
		rel := pos - (f.at + 4)
		elf.PutLE(a.Code[f.at:], uint64(int64(rel)), 4)
	}
	a.fixups = nil
	return nil
}

func (a *Asm) list(mn string, ops ...Operand) {
	if a.Listing == nil {
		return
	}
	a.Listing.WriteString("\t")
	a.Listing.WriteString(mn)
	for i := len(ops) - 1; i >= 0; i-- {
		if i == len(ops)-1 {
			a.Listing.WriteString("\t")
		} else {
			a.Listing.WriteString(", ")
		}
		a.Listing.WriteString(ops[i].String())
	}
	a.Listing.WriteString("\n")
}

func (a *Asm) b(bytes ...byte) { a.Code = append(a.Code, bytes...) }

func (a *Asm) imm(v int64, n int) {
	for i := 0; i < n; i++ {
		a.Code = append(a.Code, byte(v>>(8*i)))
	}
}

func fitsI8(v int64) bool  { return v >= -128 && v <= 127 }
func fitsI32(v int64) bool { return v >= -(1<<31) && v <= (1<<31)-1 }

// needsRex8 tells whether a byte register needs a REX prefix (spl..dil)
func needsRex8(r R) bool {
	return r.Size == 1 && !r.Reg.IsXMM() && r.Reg >= RSP && r.Reg <= RDI
}

// enc emits [pfx] [REX] op ModRM [SIB] [disp]. reg is the ModRM.reg
// value (a register number or an opcode extension); rm is R or Mem;
// trail is the number of immediate bytes following (needed for RIP
// relocations); forceRex is set for byte registers spl..dil.
func (a *Asm) enc(pfx []byte, w bool, op []byte, reg int, rm Operand, trail int, forceRex bool) {
	a.Code = append(a.Code, pfx...)
	rex := byte(0)
	if w {
		rex |= 8
	}
	if reg&8 != 0 {
		rex |= 4
	}
	var mem Mem
	isMem := false
	switch o := rm.(type) {
	case R:
		if o.Reg.num()&8 != 0 {
			rex |= 1
		}
		if needsRex8(o) {
			forceRex = true
		}
	case Mem:
		mem = o
		isMem = true
		if mem.Base >= 0 && mem.Base.num()&8 != 0 {
			rex |= 1
		}
		if mem.Index >= 0 && mem.Index.num()&8 != 0 {
			rex |= 2
		}
	default:
		panic("bad r/m operand")
	}
	if mem.FS {
		// segment override goes before the other prefixes
		a.Code = append(a.Code[:len(a.Code)-len(pfx)], 0x64)
		a.Code = append(a.Code, pfx...)
	}
	if rex != 0 || forceRex {
		a.b(0x40 | rex)
	}
	a.b(op...)
	r := byte(reg&7) << 3
	if !isMem {
		a.b(0xC0 | r | byte(rm.(R).Reg.num()&7))
		return
	}
	switch {
	case mem.Base == RIP:
		a.b(0x05 | r)
		if mem.Sym != "" {
			typ := uint32(elf.R_X86_64_PC32)
			switch mem.Rel {
			case RelGOT:
				typ = elf.R_X86_64_GOTPCREL
			case RelGOTTPOF:
				typ = elf.R_X86_64_GOTTPOFF
			}
			a.Relocs = append(a.Relocs, Reloc{Offset: int64(len(a.Code)), Type: typ, Sym: mem.Sym, Addend: int64(mem.Disp) - 4 - int64(trail)})
			a.imm(0, 4)
		} else {
			a.imm(int64(mem.Disp)-0, 4)
		}
		return
	case mem.Base == NoReg && mem.Index == NoReg:
		a.b(0x04|r, 0x25)
		a.dispSym(mem)
		return
	}
	disp := int64(mem.Disp)
	mod := byte(0)
	switch {
	case mem.Sym != "":
		mod = 2
	case mem.Base == NoReg:
		mod = 0
	case disp == 0 && mem.Base.num()&7 != 5:
		mod = 0
	case fitsI8(disp):
		mod = 1
	default:
		mod = 2
	}
	if mem.Index != NoReg || mem.Base == NoReg || mem.Base.num()&7 == 4 {
		a.b(mod<<6 | r | 4)
		ss := byte(0)
		switch mem.Scale {
		case 2:
			ss = 1
		case 4:
			ss = 2
		case 8:
			ss = 3
		}
		idx := byte(4)
		if mem.Index != NoReg {
			idx = byte(mem.Index.num() & 7)
		}
		base := byte(5)
		if mem.Base != NoReg {
			base = byte(mem.Base.num() & 7)
		}
		a.b(ss<<6 | idx<<3 | base)
		if mem.Base == NoReg {
			a.dispSym(mem)
			return
		}
	} else {
		a.b(mod<<6 | r | byte(mem.Base.num()&7))
	}
	switch mod {
	case 1:
		a.b(byte(int8(disp)))
	case 2:
		a.dispSym(mem)
	}
}

// dispSym emits a 32-bit displacement, with a relocation when it refers to a symbol
func (a *Asm) dispSym(m Mem) {
	if m.Sym != "" {
		typ := uint32(elf.R_X86_64_32S)
		if m.Rel == RelTPOFF {
			typ = elf.R_X86_64_TPOFF32
		}
		a.Relocs = append(a.Relocs, Reloc{Offset: int64(len(a.Code)), Type: typ, Sym: m.Sym, Addend: int64(m.Disp)})
		a.imm(0, 4)
		return
	}
	a.imm(int64(m.Disp), 4)
}

func sizeOf(o Operand) int {
	if r, ok := o.(R); ok {
		return r.Size
	}
	return 0
}

var sfx = map[int]string{1: "b", 2: "w", 4: "l", 8: "q"}

func opsize16(size int) []byte {
	if size == 2 {
		return []byte{0x66}
	}
	return nil
}

func rexByte8(o Operand) bool {
	if r, ok := o.(R); ok {
		return needsRex8(r)
	}
	return false
}

// ---- integer instructions ----

// AluOp selects add/or/adc/sbb/and/sub/xor/cmp
type AluOp int

const (
	ADD AluOp = iota
	OR
	ADC
	SBB
	AND
	SUB
	XOR
	CMP
)

var aluNames = [...]string{"add", "or", "adc", "sbb", "and", "sub", "xor", "cmp"}

// Alu emits "op src, dst" of the given size: dst = dst op src.
// dst is R or Mem; src is R, Mem (only when dst is R) or Imm.
func (a *Asm) Alu(op AluOp, size int, dst, src Operand) {
	a.list(aluNames[op]+sfx[size], dst, src)
	base := byte(op) * 8
	w := size == 8
	p := opsize16(size)
	switch s := src.(type) {
	case Imm:
		v := int64(s)
		acc := false
		if r, ok := dst.(R); ok && r.Reg == RAX {
			acc = true
		}
		switch {
		case size == 1 && acc:
			a.b(base + 4)
			a.imm(v, 1)
		case size == 1:
			a.enc(p, w, []byte{0x80}, int(op), dst, 1, false)
			a.imm(v, 1)
		case fitsI8(v):
			a.enc(p, w, []byte{0x83}, int(op), dst, 1, false)
			a.imm(v, 1)
		default:
			n := 4
			if size == 2 {
				n = 2
			}
			if acc {
				a.b(p...)
				if w {
					a.b(0x48)
				}
				a.b(base + 5)
			} else {
				a.enc(p, w, []byte{0x81}, int(op), dst, n, false)
			}
			a.imm(v, n)
		}
	case R:
		opc := base + 1
		if size == 1 {
			opc = base
		}
		a.enc(p, w, []byte{opc}, s.Reg.num(), dst, 0, needsRex8(s))
	case Mem:
		opc := base + 3
		if size == 1 {
			opc = base + 2
		}
		d := dst.(R)
		a.enc(p, w, []byte{opc}, d.Reg.num(), s, 0, needsRex8(d))
	}
}

// Mov emits a move of the given size between registers, memory and immediates
func (a *Asm) Mov(size int, dst, src Operand) {
	w := size == 8
	p := opsize16(size)
	switch s := src.(type) {
	case Imm:
		v := int64(s)
		if d, ok := dst.(R); ok {
			switch {
			case size == 8 && !fitsI32(v) && v >= 0 && v <= 0xFFFFFFFF:
				a.list("movl", D(d.Reg), s)
				a.movImmReg(4, d.Reg, v)
			case size == 8 && !fitsI32(v):
				a.list("movabsq", dst, src)
				a.movImmReg(8, d.Reg, v)
			case size == 8:
				a.list("movq", dst, src)
				a.enc(nil, true, []byte{0xC7}, 0, dst, 4, false)
				a.imm(v, 4)
			default:
				a.list("mov"+sfx[size], dst, src)
				a.movImmReg(size, d.Reg, v)
			}
			return
		}
		a.list("mov"+sfx[size], dst, src)
		switch size {
		case 1:
			a.enc(nil, false, []byte{0xC6}, 0, dst, 1, false)
			a.imm(v, 1)
		case 2:
			a.enc(p, false, []byte{0xC7}, 0, dst, 2, false)
			a.imm(v, 2)
		default:
			a.enc(nil, w, []byte{0xC7}, 0, dst, 4, false)
			a.imm(v, 4)
		}
	case R:
		a.list("mov"+sfx[size], dst, src)
		opc := byte(0x89)
		if size == 1 {
			opc = 0x88
		}
		a.enc(p, w, []byte{opc}, s.Reg.num(), dst, 0, needsRex8(s))
	case Mem:
		a.list("mov"+sfx[size], dst, src)
		d := dst.(R)
		opc := byte(0x8B)
		if size == 1 {
			opc = 0x8A
		}
		a.enc(p, w, []byte{opc}, d.Reg.num(), s, 0, needsRex8(d))
	}
}

func (a *Asm) movImmReg(size int, r Reg, v int64) {
	if size == 2 {
		a.b(0x66)
	}
	rex := byte(0)
	if size == 8 {
		rex |= 8
	}
	if r.num()&8 != 0 {
		rex |= 1
	}
	if rex != 0 || (size == 1 && r >= RSP && r <= RDI) {
		a.b(0x40 | rex)
	}
	if size == 1 {
		a.b(0xB0 + byte(r.num()&7))
	} else {
		a.b(0xB8 + byte(r.num()&7))
	}
	a.imm(v, size)
}

// MovImm loads a 64-bit constant into a register using the shortest form
func (a *Asm) MovImm(r Reg, v int64) {
	switch {
	case v == 0:
		a.Alu(XOR, 4, D(r), D(r))
	case v > 0 && v <= 0xFFFFFFFF:
		a.list("movl", D(r), Imm(v))
		a.movImmReg(4, r, v)
	default:
		a.Mov(8, Q(r), Imm(v))
	}
}

// Movzx zero-extends a byte or word source into a 32/64-bit register
func (a *Asm) Movzx(dstSize int, dst Reg, srcSize int, src Operand) {
	a.list("movz"+sfx[srcSize]+sfx[dstSize], Sized(dst, dstSize), src)
	opc := byte(0xB6)
	if srcSize == 2 {
		opc = 0xB7
	}
	a.enc(nil, dstSize == 8, []byte{0x0F, opc}, dst.num(), src, 0, rexByte8(src))
}

// Movsx sign-extends a byte, word or dword source into a register
func (a *Asm) Movsx(dstSize int, dst Reg, srcSize int, src Operand) {
	a.list("movs"+sfx[srcSize]+sfx[dstSize], Sized(dst, dstSize), src)
	switch srcSize {
	case 1:
		a.enc(opsize16(dstSize), dstSize == 8, []byte{0x0F, 0xBE}, dst.num(), src, 0, rexByte8(src))
	case 2:
		a.enc(nil, dstSize == 8, []byte{0x0F, 0xBF}, dst.num(), src, 0, false)
	default:
		a.enc(nil, true, []byte{0x63}, dst.num(), src, 0, false)
	}
}

// Lea loads the address of a memory operand
func (a *Asm) Lea(dst Reg, m Mem) {
	a.list("leaq", Q(dst), m)
	a.enc(nil, true, []byte{0x8D}, dst.num(), m, 0, false)
}

// Test emits "test src, dst"
func (a *Asm) Test(size int, dst Operand, src Operand) {
	a.list("test"+sfx[size], dst, src)
	p := opsize16(size)
	w := size == 8
	switch s := src.(type) {
	case Imm:
		if r, ok := dst.(R); ok && r.Reg == RAX {
			a.b(p...)
			if w {
				a.b(0x48)
			}
			if size == 1 {
				a.b(0xA8)
				a.imm(int64(s), 1)
			} else {
				n := 4
				if size == 2 {
					n = 2
				}
				a.b(0xA9)
				a.imm(int64(s), n)
			}
			return
		}
		if size == 1 {
			a.enc(p, w, []byte{0xF6}, 0, dst, 1, false)
			a.imm(int64(s), 1)
		} else {
			n := 4
			if size == 2 {
				n = 2
			}
			a.enc(p, w, []byte{0xF7}, 0, dst, n, false)
			a.imm(int64(s), n)
		}
	case R:
		opc := byte(0x85)
		if size == 1 {
			opc = 0x84
		}
		a.enc(p, w, []byte{opc}, s.Reg.num(), dst, 0, needsRex8(s))
	}
}

// Unary ops in group 3 (F6/F7)
type UnOp int

const (
	NOT  UnOp = 2
	NEG  UnOp = 3
	MUL  UnOp = 4
	IMUL UnOp = 5
	DIV  UnOp = 6
	IDIV UnOp = 7
)

var unNames = map[UnOp]string{NOT: "not", NEG: "neg", MUL: "mul", IMUL: "imul", DIV: "div", IDIV: "idiv"}

// Unary emits not/neg/mul/imul/div/idiv on a register or memory operand
func (a *Asm) Unary(op UnOp, size int, dst Operand) {
	a.list(unNames[op]+sfx[size], dst)
	opc := byte(0xF7)
	if size == 1 {
		opc = 0xF6
	}
	a.enc(opsize16(size), size == 8, []byte{opc}, int(op), dst, 0, false)
}

// Imul2 emits dst = dst * src (two-operand signed multiply)
func (a *Asm) Imul2(size int, dst Reg, src Operand) {
	a.list("imul"+sfx[size], Sized(dst, size), src)
	a.enc(opsize16(size), size == 8, []byte{0x0F, 0xAF}, dst.num(), src, 0, false)
}

// Imul3 emits dst = src * imm
func (a *Asm) Imul3(size int, dst Reg, src Operand, v int64) {
	a.list("imul"+sfx[size], Sized(dst, size), src, Imm(v))
	if fitsI8(v) {
		a.enc(opsize16(size), size == 8, []byte{0x6B}, dst.num(), src, 1, false)
		a.imm(v, 1)
		return
	}
	n := 4
	if size == 2 {
		n = 2
	}
	a.enc(opsize16(size), size == 8, []byte{0x69}, dst.num(), src, n, false)
	a.imm(v, n)
}

// Shift ops in group 2
type ShiftOp int

const (
	ROL ShiftOp = 0
	ROR ShiftOp = 1
	SHL ShiftOp = 4
	SHR ShiftOp = 5
	SAR ShiftOp = 7
)

var shiftNames = map[ShiftOp]string{ROL: "rol", ROR: "ror", SHL: "shl", SHR: "shr", SAR: "sar"}

// ShiftCL shifts dst by %cl
func (a *Asm) ShiftCL(op ShiftOp, size int, dst Operand) {
	a.list(shiftNames[op]+sfx[size], dst, B(RCX))
	opc := byte(0xD3)
	if size == 1 {
		opc = 0xD2
	}
	a.enc(opsize16(size), size == 8, []byte{opc}, int(op), dst, 0, false)
}

// ShiftImm shifts dst by a constant
func (a *Asm) ShiftImm(op ShiftOp, size int, dst Operand, n int) {
	a.list(shiftNames[op]+sfx[size], dst, Imm(n))
	opc := byte(0xC1)
	if size == 1 {
		opc = 0xC0
	}
	a.enc(opsize16(size), size == 8, []byte{opc}, int(op), dst, 1, false)
	a.imm(int64(n), 1)
}

// Cqo sign-extends rax into rdx:rax
func (a *Asm) Cqo() { a.list("cqto"); a.b(0x48, 0x99) }

// Cdq sign-extends eax into edx:eax
func (a *Asm) Cdq() { a.list("cltd"); a.b(0x99) }

// Cwd sign-extends ax into dx:ax
func (a *Asm) Cwd() { a.list("cwtd"); a.b(0x66, 0x99) }

// Condition codes
type Cond int

const (
	CondO  Cond = 0
	CondNO Cond = 1
	CondB  Cond = 2 // below (unsigned <), carry
	CondAE Cond = 3 // above or equal (unsigned >=)
	CondE  Cond = 4
	CondNE Cond = 5
	CondBE Cond = 6
	CondA  Cond = 7
	CondS  Cond = 8
	CondNS Cond = 9
	CondP  Cond = 10
	CondNP Cond = 11
	CondL  Cond = 12
	CondGE Cond = 13
	CondLE Cond = 14
	CondG  Cond = 15
)

var condNames = [...]string{"o", "no", "b", "ae", "e", "ne", "be", "a", "s", "ns", "p", "np", "l", "ge", "le", "g"}

// Setcc sets a byte register to the condition
func (a *Asm) Setcc(c Cond, dst Reg) {
	a.list("set"+condNames[c], B(dst))
	a.enc(nil, false, []byte{0x0F, 0x90 + byte(c)}, 0, B(dst), 0, false)
}

// Cmov moves when the condition holds
func (a *Asm) Cmov(c Cond, size int, dst Reg, src Operand) {
	a.list("cmov"+condNames[c]+sfx[size], Sized(dst, size), src)
	a.enc(opsize16(size), size == 8, []byte{0x0F, 0x40 + byte(c)}, dst.num(), src, 0, false)
}

// Jcc jumps to the label when the condition holds
func (a *Asm) Jcc(c Cond, l Label) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tj%s\t.L%d\n", condNames[c], int(l))
	}
	a.b(0x0F, 0x80+byte(c))
	a.fixups = append(a.fixups, fixup{len(a.Code), l})
	a.imm(0, 4)
}

// Jmp jumps to the label
func (a *Asm) Jmp(l Label) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tjmp\t.L%d\n", int(l))
	}
	a.b(0xE9)
	a.fixups = append(a.fixups, fixup{len(a.Code), l})
	a.imm(0, 4)
}

// JmpInd jumps to the address in a register or memory
func (a *Asm) JmpInd(target Operand) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tjmp\t*%s\n", target)
	}
	a.enc(nil, false, []byte{0xFF}, 4, target, 0, false)
}

// Call calls a symbol (PLT32 relocation)
func (a *Asm) Call(sym string) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tcall\t%s\n", sym)
	}
	a.b(0xE8)
	a.Relocs = append(a.Relocs, Reloc{Offset: int64(len(a.Code)), Type: elf.R_X86_64_PLT32, Sym: sym, Addend: -4})
	a.imm(0, 4)
}

// CallInd calls the address in a register or memory
func (a *Asm) CallInd(target Operand) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tcall\t*%s\n", target)
	}
	a.enc(nil, false, []byte{0xFF}, 2, target, 0, false)
}

// LabelAddr loads the address of a label into a register (lea .L(%rip))
func (a *Asm) LabelAddr(dst Reg, l Label) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tleaq\t.L%d(%%rip), %s\n", int(l), RegName(dst, 8))
	}
	a.enc(nil, true, []byte{0x8D}, dst.num(), Mem{Base: RIP, Index: NoReg}, 0, false)
	a.fixups = append(a.fixups, fixup{len(a.Code) - 4, l})
}

// Push pushes a 64-bit register
func (a *Asm) Push(r Reg) {
	a.list("pushq", Q(r))
	if r.num()&8 != 0 {
		a.b(0x41)
	}
	a.b(0x50 + byte(r.num()&7))
}

// Pop pops into a 64-bit register
func (a *Asm) Pop(r Reg) {
	a.list("popq", Q(r))
	if r.num()&8 != 0 {
		a.b(0x41)
	}
	a.b(0x58 + byte(r.num()&7))
}

// Ret returns from the function
func (a *Asm) Ret() { a.list("ret"); a.b(0xC3) }

// Leave restores rsp and rbp
func (a *Asm) Leave() { a.list("leave"); a.b(0xC9) }

// Hlt halts (used after calls that never return)
func (a *Asm) Hlt() { a.list("hlt"); a.b(0xF4) }

// Ud2 is an invalid instruction (__builtin_trap)
func (a *Asm) Ud2() { a.list("ud2"); a.b(0x0F, 0x0B) }

// Nop emits a one-byte nop
func (a *Asm) Nop() { a.list("nop"); a.b(0x90) }

// RepMovsb copies rcx bytes from (rsi) to (rdi)
func (a *Asm) RepMovsb() { a.list("rep movsb"); a.b(0xF3, 0xA4) }

// RepStosb fills rcx bytes at (rdi) with al
func (a *Asm) RepStosb() { a.list("rep stosb"); a.b(0xF3, 0xAA) }

// Bswap reverses the bytes of a 32/64-bit register
func (a *Asm) Bswap(size int, r Reg) {
	a.list("bswap"+sfx[size], Sized(r, size))
	rex := byte(0)
	if size == 8 {
		rex |= 8
	}
	if r.num()&8 != 0 {
		rex |= 1
	}
	if rex != 0 {
		a.b(0x40 | rex)
	}
	a.b(0x0F, 0xC8+byte(r.num()&7))
}

// Bit scan and count instructions
func (a *Asm) Bsr(size int, dst Reg, src Operand) {
	a.list("bsr"+sfx[size], Sized(dst, size), src)
	a.enc(opsize16(size), size == 8, []byte{0x0F, 0xBD}, dst.num(), src, 0, false)
}

func (a *Asm) Bsf(size int, dst Reg, src Operand) {
	a.list("bsf"+sfx[size], Sized(dst, size), src)
	a.enc(opsize16(size), size == 8, []byte{0x0F, 0xBC}, dst.num(), src, 0, false)
}

// Xchg exchanges a register with memory or a register (lock implied with memory)
func (a *Asm) Xchg(size int, dst Operand, src Reg) {
	a.list("xchg"+sfx[size], dst, Sized(src, size))
	opc := byte(0x87)
	if size == 1 {
		opc = 0x86
	}
	a.enc(opsize16(size), size == 8, []byte{opc}, src.num(), dst, 0, size == 1 && src >= RSP && src <= RDI)
}

// LockXadd atomically adds src to memory, returning the old value in src
func (a *Asm) LockXadd(size int, dst Mem, src Reg) {
	a.list("lock xadd"+sfx[size], dst, Sized(src, size))
	a.b(0xF0)
	opc := byte(0xC1)
	if size == 1 {
		opc = 0xC0
	}
	a.enc(opsize16(size), size == 8, []byte{0x0F, opc}, src.num(), dst, 0, size == 1 && src >= RSP && src <= RDI)
}

// LockCmpxchg compares rax with memory and stores src when equal
func (a *Asm) LockCmpxchg(size int, dst Mem, src Reg) {
	a.list("lock cmpxchg"+sfx[size], dst, Sized(src, size))
	a.b(0xF0)
	opc := byte(0xB1)
	if size == 1 {
		opc = 0xB0
	}
	a.enc(opsize16(size), size == 8, []byte{0x0F, opc}, src.num(), dst, 0, size == 1 && src >= RSP && src <= RDI)
}

// Mfence is a full memory barrier
func (a *Asm) Mfence() { a.list("mfence"); a.b(0x0F, 0xAE, 0xF0) }

// ---- SSE ----

// SSE op codes (second opcode byte after 0F)
const (
	sseSqrt  = 0x51
	sseAnd   = 0x54
	sseXor   = 0x57
	sseAdd   = 0x58
	sseMul   = 0x59
	sseCvt   = 0x5A
	sseSub   = 0x5C
	sseMin   = 0x5D
	sseDiv   = 0x5E
	sseMax   = 0x5F
	sseUcomi = 0x2E
	sseComi  = 0x2F
)

// FOp selects a scalar float arithmetic operation
type FOp int

const (
	FAdd  FOp = sseAdd
	FSub  FOp = sseSub
	FMul  FOp = sseMul
	FDiv  FOp = sseDiv
	FSqrt FOp = sseSqrt
	FMin  FOp = sseMin
	FMax  FOp = sseMax
)

var fopNames = map[FOp]string{FAdd: "add", FSub: "sub", FMul: "mul", FDiv: "div", FSqrt: "sqrt", FMin: "min", FMax: "max"}

func ssePfx(double bool) []byte {
	if double {
		return []byte{0xF2}
	}
	return []byte{0xF3}
}

func ssSd(double bool) string {
	if double {
		return "sd"
	}
	return "ss"
}

// FArith emits addss/addsd etc: dst = dst op src
func (a *Asm) FArith(op FOp, double bool, dst Reg, src Operand) {
	a.list(fopNames[op]+ssSd(double), X(dst), src)
	a.enc(ssePfx(double), false, []byte{0x0F, byte(op)}, dst.num(), src, 0, false)
}

// MovF loads or stores a float/double: movss/movsd
func (a *Asm) MovF(double bool, dst, src Operand) {
	a.list("mov"+ssSd(double), dst, src)
	if d, ok := dst.(R); ok {
		a.enc(ssePfx(double), false, []byte{0x0F, 0x10}, d.Reg.num(), src, 0, false)
		return
	}
	s := src.(R)
	a.enc(ssePfx(double), false, []byte{0x0F, 0x11}, s.Reg.num(), dst, 0, false)
}

// Movaps copies an XMM register
func (a *Asm) Movaps(dst, src Reg) {
	a.list("movaps", X(dst), X(src))
	a.enc(nil, false, []byte{0x0F, 0x28}, dst.num(), X(src), 0, false)
}

// MovqToX moves a 64-bit (or 32-bit) general register into an XMM register
func (a *Asm) MovqToX(size int, dst Reg, src Reg) {
	if size == 8 {
		a.list("movq", X(dst), Q(src))
	} else {
		a.list("movd", X(dst), D(src))
	}
	a.enc([]byte{0x66}, size == 8, []byte{0x0F, 0x6E}, dst.num(), Sized(src, size), 0, false)
}

// MovqFromX moves the low 64 (or 32) bits of an XMM register into a general register
func (a *Asm) MovqFromX(size int, dst Reg, src Reg) {
	if size == 8 {
		a.list("movq", Q(dst), X(src))
	} else {
		a.list("movd", D(dst), X(src))
	}
	a.enc([]byte{0x66}, size == 8, []byte{0x0F, 0x7E}, src.num(), Sized(dst, size), 0, false)
}

// Xorp clears or flips bits of an XMM register (xorps/xorpd)
func (a *Asm) Xorp(double bool, dst Reg, src Operand) {
	if double {
		a.list("xorpd", X(dst), src)
		a.enc([]byte{0x66}, false, []byte{0x0F, sseXor}, dst.num(), src, 0, false)
		return
	}
	a.list("xorps", X(dst), src)
	a.enc(nil, false, []byte{0x0F, sseXor}, dst.num(), src, 0, false)
}

// Andp masks an XMM register (andps/andpd)
func (a *Asm) Andp(double bool, dst Reg, src Operand) {
	if double {
		a.list("andpd", X(dst), src)
		a.enc([]byte{0x66}, false, []byte{0x0F, sseAnd}, dst.num(), src, 0, false)
		return
	}
	a.list("andps", X(dst), src)
	a.enc(nil, false, []byte{0x0F, sseAnd}, dst.num(), src, 0, false)
}

// Ucomi compares two scalars and sets flags (ucomiss/ucomisd)
func (a *Asm) Ucomi(double bool, x Reg, y Operand) {
	if double {
		a.list("ucomisd", X(x), y)
		a.enc([]byte{0x66}, false, []byte{0x0F, sseUcomi}, x.num(), y, 0, false)
		return
	}
	a.list("ucomiss", X(x), y)
	a.enc(nil, false, []byte{0x0F, sseUcomi}, x.num(), y, 0, false)
}

// CvtIntToF converts a signed 32/64-bit integer to float/double
func (a *Asm) CvtIntToF(double bool, dst Reg, srcSize int, src Operand) {
	a.list("cvtsi2"+ssSd(double)+sfx[srcSize], X(dst), src)
	a.enc(ssePfx(double), srcSize == 8, []byte{0x0F, 0x2A}, dst.num(), src, 0, false)
}

// CvtFToInt converts float/double to a signed integer with truncation
func (a *Asm) CvtFToInt(double bool, dstSize int, dst Reg, src Operand) {
	a.list("cvtt"+ssSd(double)+"2si"+sfx[dstSize], Sized(dst, dstSize), src)
	a.enc(ssePfx(double), dstSize == 8, []byte{0x0F, 0x2C}, dst.num(), src, 0, false)
}

// CvtFF converts between float and double; toDouble selects cvtss2sd
func (a *Asm) CvtFF(toDouble bool, dst Reg, src Operand) {
	if toDouble {
		a.list("cvtss2sd", X(dst), src)
		a.enc([]byte{0xF3}, false, []byte{0x0F, sseCvt}, dst.num(), src, 0, false)
		return
	}
	a.list("cvtsd2ss", X(dst), src)
	a.enc([]byte{0xF2}, false, []byte{0x0F, sseCvt}, dst.num(), src, 0, false)
}

// ---- x87 ----

// Fld loads a float (4), double (8) or long double (10) from memory
func (a *Asm) Fld(size int, m Mem) {
	switch size {
	case 4:
		a.list("flds", m)
		a.enc(nil, false, []byte{0xD9}, 0, m, 0, false)
	case 8:
		a.list("fldl", m)
		a.enc(nil, false, []byte{0xDD}, 0, m, 0, false)
	default:
		a.list("fldt", m)
		a.enc(nil, false, []byte{0xDB}, 5, m, 0, false)
	}
}

// Fstp stores st(0) as float (4), double (8) or long double (10) and pops
func (a *Asm) Fstp(size int, m Mem) {
	switch size {
	case 4:
		a.list("fstps", m)
		a.enc(nil, false, []byte{0xD9}, 3, m, 0, false)
	case 8:
		a.list("fstpl", m)
		a.enc(nil, false, []byte{0xDD}, 3, m, 0, false)
	default:
		a.list("fstpt", m)
		a.enc(nil, false, []byte{0xDB}, 7, m, 0, false)
	}
}

// Fild loads a signed integer of 2, 4 or 8 bytes
func (a *Asm) Fild(size int, m Mem) {
	switch size {
	case 2:
		a.list("filds", m)
		a.enc(nil, false, []byte{0xDF}, 0, m, 0, false)
	case 4:
		a.list("fildl", m)
		a.enc(nil, false, []byte{0xDB}, 0, m, 0, false)
	default:
		a.list("fildll", m)
		a.enc(nil, false, []byte{0xDF}, 5, m, 0, false)
	}
}

// Fistp stores st(0) as a signed integer of 2, 4 or 8 bytes and pops
func (a *Asm) Fistp(size int, m Mem) {
	switch size {
	case 2:
		a.list("fistps", m)
		a.enc(nil, false, []byte{0xDF}, 3, m, 0, false)
	case 4:
		a.list("fistpl", m)
		a.enc(nil, false, []byte{0xDB}, 3, m, 0, false)
	default:
		a.list("fistpll", m)
		a.enc(nil, false, []byte{0xDF}, 7, m, 0, false)
	}
}

// Fnstcw stores the FPU control word
func (a *Asm) Fnstcw(m Mem) {
	a.list("fnstcw", m)
	a.enc(nil, false, []byte{0xD9}, 7, m, 0, false)
}

// Fldcw loads the FPU control word
func (a *Asm) Fldcw(m Mem) {
	a.list("fldcw", m)
	a.enc(nil, false, []byte{0xD9}, 5, m, 0, false)
}

// X87 register-stack operations without operands
func (a *Asm) Faddp()  { a.list("faddp"); a.b(0xDE, 0xC1) }  // st1 = st1+st0, pop
func (a *Asm) Fmulp()  { a.list("fmulp"); a.b(0xDE, 0xC9) }  // st1 = st1*st0, pop
func (a *Asm) Fsubp()  { a.list("fsubrp"); a.b(0xDE, 0xE9) } // st1 = st1-st0, pop (AT&T calls it fsubrp)
func (a *Asm) Fsubrp() { a.list("fsubp"); a.b(0xDE, 0xE1) }  // st1 = st0-st1, pop
func (a *Asm) Fdivp()  { a.list("fdivrp"); a.b(0xDE, 0xF9) } // st1 = st1/st0, pop (AT&T calls it fdivrp)
func (a *Asm) Fdivrp() { a.list("fdivp"); a.b(0xDE, 0xF1) }  // st1 = st0/st1, pop
func (a *Asm) Fchs()   { a.list("fchs"); a.b(0xD9, 0xE0) }
func (a *Asm) Fabs()   { a.list("fabs"); a.b(0xD9, 0xE1) }
func (a *Asm) Fldz()   { a.list("fldz"); a.b(0xD9, 0xEE) }
func (a *Asm) Fld1()   { a.list("fld1"); a.b(0xD9, 0xE8) }

// FldSt pushes a copy of st(i)
func (a *Asm) FldSt(i int) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tfld\t%%st(%d)\n", i)
	}
	a.b(0xD9, 0xC0+byte(i))
}

// FstpSt stores st(0) into st(i) and pops
func (a *Asm) FstpSt(i int) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tfstp\t%%st(%d)\n", i)
	}
	a.b(0xDD, 0xD8+byte(i))
}

// Fxch exchanges st(0) and st(i)
func (a *Asm) Fxch(i int) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tfxch\t%%st(%d)\n", i)
	}
	a.b(0xD9, 0xC8+byte(i))
}

// Fucomip compares st(0) with st(i), sets EFLAGS and pops
func (a *Asm) Fucomip(i int) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tfucomip\t%%st(%d), %%st\n", i)
	}
	a.b(0xDF, 0xE8+byte(i))
}

// ---- data directives for listings ----

// Raw appends raw bytes (used for padding)
func (a *Asm) Raw(bs ...byte) {
	if a.Listing != nil && len(bs) > 0 {
		fmt.Fprintf(a.Listing, "\t.byte\t%d", bs[0])
		for _, b := range bs[1:] {
			fmt.Fprintf(a.Listing, ",%d", b)
		}
		a.Listing.WriteString("\n")
	}
	a.b(bs...)
}

// ShldCL shifts dst left by %cl, filling from src (shld %cl, src, dst)
func (a *Asm) ShldCL(dst Operand, src Reg) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tshldq\t%%cl, %s, %s\n", Q(src), dst)
	}
	a.enc(nil, true, []byte{0x0F, 0xA5}, src.num(), dst, 0, false)
}

// ShrdCL shifts dst right by %cl, filling from src (shrd %cl, src, dst)
func (a *Asm) ShrdCL(dst Operand, src Reg) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\tshrdq\t%%cl, %s, %s\n", Q(src), dst)
	}
	a.enc(nil, true, []byte{0x0F, 0xAD}, src.num(), dst, 0, false)
}
