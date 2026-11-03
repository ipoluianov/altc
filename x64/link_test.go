package x64

import (
	"os"
	"os/exec"
	"path/filepath"
	"testing"

	"github.com/ipoluianov/altc/elf"
)

// TestLinkHello links hand-written code against the system libc and runs it
func TestLinkHello(t *testing.T) {
	libc, err := elf.ReadSharedLib("/lib/x86_64-linux-gnu/libc.so.6")
	if err != nil {
		t.Skip(err)
	}
	o := elf.NewObject("hello.o")
	ro := o.AddSection(".rodata", elf.SecROData, 1)
	ro.Data = []byte("hello %s %d\x00world\x00")
	str := o.AddLocal(&elf.Symbol{Name: ".LC0", Section: ro})
	data := o.AddSection(".data", elf.SecData, 8)
	data.Data = make([]byte, 8)
	cnt := o.Symbol("counter")
	cnt.Section, cnt.Type, cnt.Size = data, elf.SymObject, 8
	tdata := o.AddSection(".tbss", elf.SecTBSS, 8)
	tdata.Size = 8
	tv := o.Symbol("tlsvar")
	tv.Section, tv.Type, tv.Size = tdata, elf.SymTLS, 8

	var a Asm
	a.Push(RBP)
	a.Mov(8, Q(RBP), Q(RSP))
	// tlsvar = 5 (local exec)
	a.Mov(8, Q(RAX), Mem{FS: true, Base: NoReg, Index: NoReg})
	a.Mov(8, Mem{Base: RAX, Index: NoReg, Sym: "tlsvar", Rel: RelTPOFF}, Imm(5))
	a.Lea(RDI, MSym(".LC0", 0))
	a.Lea(RSI, MSym(".LC0", 12))
	a.Mov(8, Q(RDX), Mem{FS: true, Base: NoReg, Index: NoReg})
	a.Mov(8, Q(RDX), Mem{Base: RDX, Index: NoReg, Sym: "tlsvar", Rel: RelTPOFF})
	a.Alu(ADD, 8, MSym("counter", 0), Imm(37))
	a.Alu(ADD, 8, Q(RDX), MSym("counter", 0))
	a.Alu(XOR, 4, D(RAX), D(RAX))
	a.Call("printf")
	a.Mov(8, Q(RAX), MGot("stdout"))
	a.Mov(8, Q(RDI), M(RAX, 0))
	a.Call("fflush")
	a.Mov(4, D(RAX), Imm(3))
	a.Leave()
	a.Ret()
	text := o.AddSection(".text", elf.SecText, 16)
	text.Data = a.Code
	main := o.Symbol("main")
	main.Section, main.Type = text, elf.SymFunc
	for _, r := range a.Relocs {
		var s *elf.Symbol
		if r.Sym == ".LC0" {
			s = str
		} else {
			s = o.Symbol(r.Sym)
		}
		text.Relocs = append(text.Relocs, elf.Reloc{Offset: r.Offset, Type: r.Type, Sym: s, Addend: r.Addend})
	}
	ln := &elf.Linker{Objects: []*elf.Object{o}, SharedLibs: []*elf.SharedLib{libc}}
	img, err := ln.Link()
	if err != nil {
		t.Fatal(err)
	}
	exe := filepath.Join(t.TempDir(), "hello")
	if err := os.WriteFile(exe, img, 0o755); err != nil {
		t.Fatal(err)
	}
	out, err := exec.Command(exe).CombinedOutput()
	if ee, ok := err.(*exec.ExitError); !ok || ee.ExitCode() != 3 {
		t.Fatalf("run: %v\n%s", err, out)
	}
	if string(out) != "hello world 42" {
		t.Fatalf("output %q", out)
	}
}
