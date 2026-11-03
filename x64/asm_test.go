package x64

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

// TestEncodingAgainstGas assembles the listing with GNU as and compares
// the bytes with our encoder's output (skipped when as is missing).
func TestEncodingAgainstGas(t *testing.T) {
	if _, err := exec.LookPath("as"); err != nil {
		t.Skip("GNU as not found")
	}
	var a Asm
	a.Listing = &strings.Builder{}
	regs := []Reg{RAX, RCX, RSP, RBP, RSI, RDI, R8, R12, R13, R15}
	mems := []Mem{
		M(RBP, -8), M(RSP, 0), M(RSP, 16), M(R12, 0), M(R13, 0), M(RBP, 0), M(RAX, 1000),
		MI(RAX, RCX, 8, 0), MI(R8, R9, 4, -100), MI(RBP, R12, 1, 3), MI(R13, RAX, 2, 0),
		{Base: NoReg, Index: RCX, Scale: 4, Disp: 16},
		{Base: NoReg, Index: NoReg, Disp: 0x100},
	}
	for _, size := range []int{1, 2, 4, 8} {
		for _, r := range regs {
			for _, r2 := range []Reg{RAX, RSI, R9, R14} {
				a.Mov(size, Sized(r, size), Sized(r2, size))
				a.Alu(ADD, size, Sized(r, size), Sized(r2, size))
				a.Alu(CMP, size, Sized(r2, size), Sized(r, size))
				a.Test(size, Sized(r, size), Sized(r2, size))
			}
			for _, m := range mems {
				a.Mov(size, Sized(r, size), m)
				a.Mov(size, m, Sized(r, size))
				a.Alu(SUB, size, m, Sized(r, size))
				a.Alu(XOR, size, Sized(r, size), m)
			}
			for _, v := range []int64{0, 1, -1, 127, -128, 200, 30000} {
				if size == 1 && (v > 127 || v < -128) {
					continue
				}
				a.Alu(AND, size, Sized(r, size), Imm(v))
				a.Alu(CMP, size, M(RBP, -16), Imm(v))
				a.Mov(size, M(RBP, -16), Imm(v))
				a.Mov(size, Sized(r, size), Imm(v))
				a.Test(size, Sized(r, size), Imm(v&0x7f))
			}
			a.Unary(NEG, size, Sized(r, size))
			a.Unary(IDIV, size, M(RBP, -24))
			a.ShiftCL(SAR, size, Sized(r, size))
			a.ShiftImm(SHL, size, Sized(r, size), 3)
			if size >= 2 {
				a.Imul2(size, r, Sized(RDX, size))
				a.Imul2(size, r, M(RBP, 8))
				a.Imul3(size, r, Sized(R11, size), 10)
				a.Imul3(size, r, Sized(R11, size), 1000)
				a.Cmov(CondL, size, r, Sized(RBX, size))
				a.Bsr(size, r, Sized(RBX, size))
			}
			if size == 4 || size == 8 {
				a.Bswap(size, r)
				a.Movzx(size, r, 1, B(RSI))
				a.Movzx(size, r, 2, M(RAX, 2))
				a.Movsx(size, r, 1, B(RDI))
				a.Movsx(size, r, 2, W(R10))
				a.Movsx(size, r, 1, M(RBP, -1))
			}
			if size == 8 {
				a.Movsx(8, r, 4, D(RCX))
				a.Movsx(8, r, 4, M(RBP, -4))
				a.Lea(r, M(RBP, -32))
				a.Lea(r, MI(RAX, RCX, 8, 64))
				a.Push(r)
				a.Pop(r)
				a.Mov(8, Q(r), Imm(0x123456789))
				a.Mov(8, Q(r), Imm(0xFFFFFFFF))
				a.Mov(8, Q(r), Imm(-5))
				a.CallInd(Q(r))
				a.JmpInd(Q(r))
				a.CallInd(M(r, 8))
			}
			if size == 2 {
				a.Movsx(2, r, 1, B(RAX))
			}
		}
	}
	for _, r := range regs {
		a.Setcc(CondE, r)
		a.Setcc(CondG, r)
		a.Setcc(CondNP, r)
	}
	a.Cqo()
	a.Cdq()
	a.Cwd()
	a.Ret()
	a.Leave()
	a.RepMovsb()
	a.RepStosb()
	a.Mfence()
	a.Ud2()
	a.ShldCL(Q(RDX), RAX)
	a.ShrdCL(Q(RAX), RDX)
	a.ShldCL(Q(R9), R10)
	x := []Reg{XMM0, XMM1, XMM7, XMM8, XMM15}
	for _, d := range []bool{false, true} {
		for _, r := range x {
			for _, r2 := range x {
				a.FArith(FAdd, d, r, X(r2))
				a.FArith(FDiv, d, r, X(r2))
				a.Ucomi(d, r, X(r2))
				a.Xorp(d, r, X(r2))
			}
			a.FArith(FMul, d, r, M(RBP, -8))
			a.FArith(FSqrt, d, r, M(R12, 0))
			a.MovF(d, X(r), M(RBP, -16))
			a.MovF(d, M(RSP, 8), X(r))
			a.MovF(d, X(r), X(XMM3))
			a.Movaps(r, XMM9)
			a.CvtIntToF(d, r, 4, D(RAX))
			a.CvtIntToF(d, r, 8, Q(R10))
			a.CvtIntToF(d, r, 8, M(RBP, -8))
			a.CvtFToInt(d, 4, RCX, X(r))
			a.CvtFToInt(d, 8, R11, X(r))
			a.CvtFF(d, r, X(XMM2))
			a.MovqToX(8, r, R9)
			a.MovqToX(4, r, RDX)
			a.MovqFromX(8, R12, r)
			a.MovqFromX(4, RAX, r)
		}
	}
	for _, sz := range []int{4, 8, 10} {
		a.Fld(sz, M(RBP, -16))
		a.Fstp(sz, M(R12, 0))
	}
	for _, sz := range []int{2, 4, 8} {
		a.Fild(sz, M(RSP, 0))
		a.Fistp(sz, M(RBP, -8))
	}
	a.Fnstcw(M(RBP, -2))
	a.Fldcw(M(RBP, -4))
	a.Faddp()
	a.Fmulp()
	a.Fsubp()
	a.Fsubrp()
	a.Fdivp()
	a.Fdivrp()
	a.Fchs()
	a.Fabs()
	a.Fldz()
	a.Fld1()
	a.FldSt(1)
	a.FstpSt(1)
	a.Fxch(1)
	a.Fucomip(1)
	a.Mov(8, Q(RAX), Mem{FS: true, Base: NoReg, Index: NoReg})
	a.Xchg(8, M(RDI, 0), RAX)
	a.Xchg(1, M(RDI, 0), RSI)
	a.LockXadd(4, M(RDI, 0), RCX)
	a.LockCmpxchg(8, M(RDI, 8), RDX)
	a.LockCmpxchg(1, M(RDI, 8), RDI)

	dir := t.TempDir()
	src := filepath.Join(dir, "t.s")
	obj := filepath.Join(dir, "t.o")
	bin := filepath.Join(dir, "t.bin")
	if err := os.WriteFile(src, []byte(".text\n"+a.Listing.String()), 0o644); err != nil {
		t.Fatal(err)
	}
	if out, err := exec.Command("as", "-o", obj, src).CombinedOutput(); err != nil {
		t.Fatalf("as: %v\n%s", err, out)
	}
	if out, err := exec.Command("objcopy", "-O", "binary", "-j", ".text", obj, bin).CombinedOutput(); err != nil {
		t.Fatalf("objcopy: %v\n%s", err, out)
	}
	want, _ := os.ReadFile(bin)
	if !bytes.Equal(want, a.Code) {
		// find the first differing instruction by re-assembling line by line
		lines := strings.Split(strings.TrimSpace(a.Listing.String()), "\n")
		for i, l := range lines {
			var one Asm
			_ = one
			os.WriteFile(src, []byte(".text\n"+strings.Join(lines[:i+1], "\n")+"\n"), 0o644)
			exec.Command("as", "-o", obj, src).Run()
			exec.Command("objcopy", "-O", "binary", "-j", ".text", obj, bin).Run()
			got, _ := os.ReadFile(bin)
			if len(got) > len(a.Code) || !bytes.Equal(got, a.Code[:len(got)]) {
				start := 0
				if i > 0 {
					os.WriteFile(src, []byte(".text\n"+strings.Join(lines[:i], "\n")+"\n"), 0o644)
					exec.Command("as", "-o", obj, src).Run()
					exec.Command("objcopy", "-O", "binary", "-j", ".text", obj, bin).Run()
					prev, _ := os.ReadFile(bin)
					start = len(prev)
				}
				end := min(len(got), len(a.Code))
				t.Fatalf("mismatch at line %d %q:\n gas: % x\n our: % x", i+1, l, got[start:], a.Code[start:min(end+4, len(a.Code))])
			}
		}
		t.Fatalf("mismatch (lengths gas %d, ours %d)", len(want), len(a.Code))
	}
	fmt.Println("encoded bytes:", len(a.Code))
}
