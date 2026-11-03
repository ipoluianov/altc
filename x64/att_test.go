package x64

import (
	"bytes"
	"debug/elf"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"

	"github.com/ipoluianov/altc/cc"
)

// attCorpus are instructions whose encoding must match GNU as byte for byte
var attCorpus = []string{
	// integer
	"movl %eax, %ebx", "movq %r8, %rax", "movb %ah, %al", "movb %al, %bh", "movb %sil, %dil", "movb %sil, (%rdi)", "addb %dil, 1(%rax)", "cmpb (%rax), %bpl", "xchgb %spl, (%rax)", "testb %sil, %al", "cmpxchgb %sil, (%rdi)", "movw %ax, %r9w",
	"movl $1, %eax", "movq $1, %rax", "movq $-1, %r10", "movq $0x123456789, %rax", "movabsq $1, %rax",
	"movb $5, %cl", "movb $5, %ah", "movw $300, %dx", "movl $7, (%rax)", "movq $-8, 8(%rsp)", "movb $1, -1(%rbp)",
	"movw $2, (%r12)", "movl (%rax), %ecx", "movq 8(%rsp), %r15", "movq %rax, (%r13)", "mov %eax, (%rax,%rcx,4)",
	"movl %eax, 16(,%rcx,8)", "movq %fs:foo@tpoff(,%rax,8), %rdx", "movq foo(,%rax,8), %rdx", "leaq foo+4(,%rcx,4), %rax", "movq %fs:0, %rax", "movq %fs:40, %rdx", "movl foo(%rip), %eax", "movq foo+8(%rip), %rax",
	"movq foo@GOTPCREL(%rip), %rax", "leaq foo(%rip), %rdi", "leaq 8(%rax,%rbx,2), %rcx", "leal (%rdi,%rsi), %eax",
	"leaw (%rax), %ax", "lea -0x10(%rbp), %rsp", "movl $foo, %eax", "movq $foo, %rax", "movabsq $foo, %rax",
	"addl $1, %eax", "addl $1000, %eax", "addq $1000, %rbx", "addb $1, %al", "addb $1, %bl", "addw $1000, %ax",
	"subq %rax, %rbx", "sub (%rdi), %r8", "andl %ecx, (%rdx)", "orb $0x80, (%rax)", "xorl %eax, %eax",
	"cmpq $0, 8(%rbp)", "cmpb %ah, %bl", "adcq $0, %rdx", "sbbl %eax, %eax", "cmpl $-129, %r11d",
	"testl %eax, %eax", "testb $1, %al", "testb $1, %bl", "testq $256, %rax", "testl $1, (%rdi)", "test %rax, (%rsp)",
	"notl %eax", "negq (%rax)", "mull %ebx", "imulq %rcx", "divq 8(%rsp)", "idivl %r9d", "incq %rax", "decb (%rdi)",
	"incw %ax", "imull %ebx, %eax", "imulq 8(%rsp), %rdx", "imull $10, %eax, %ebx", "imulq $1000, (%rax), %rcx",
	"imul $3, %eax", "shll $3, %eax", "shrq %rax", "shll $1, %eax", "sarl %cl, %edx", "rolq $7, (%rax)",
	"rorb %cl, %al", "rcll $1, %ecx", "rcrq %cl, %r8", "salq $2, %r9", "shldq $4, %rax, %rbx", "shrdl %cl, %eax, %edx",
	"shld %eax, %edx", "btl %eax, %ebx", "btq $5, %rax", "btsl $3, (%rdi)", "btrq %rcx, (%rax)", "btcw %ax, %bx",
	"bsfl %eax, %ecx", "bsrq (%rax), %rdx", "popcntq %rax, %rbx", "lzcntl %eax, %eax", "tzcntq (%rdi), %rax",
	"popcntw %ax, %bx", "crc32b %al, %eax", "crc32l %ecx, %eax", "crc32q %rcx, %rax", "crc32w (%rsi), %eax",
	"movbe (%rdi), %eax", "movbeq %rax, (%rsi)", "adcx %rax, %rbx", "adox %ecx, %edx",
	"bswap %eax", "bswapq %r12", "xchgl %eax, %ebx", "xchgq %rax, %r9", "xchg %ebx, %ecx", "xchgl %ecx, (%rdi)",
	"xchg %al, %bl", "xchgw %ax, %dx", "xchgl %eax, %eax", "lock xaddl %eax, (%rdi)", "xaddq %rcx, (%rax)",
	"lock cmpxchgq %rdx, (%rdi)", "cmpxchgb %cl, (%rax)", "lock cmpxchg16b (%rdi)", "cmpxchg8b (%rax)",
	"movzbl %al, %eax", "movzbl (%rdi), %ecx", "movzwl %ax, %eax", "movzbq %sil, %rax", "movzwq (%rax), %r8",
	"movsbl %al, %eax", "movsbq (%rax), %rax", "movswl %cx, %edx", "movslq %eax, %rax", "movslq (%rdi), %r10",
	"movsbw %al, %ax", "movzx %al, %eax", "movsx %cx, %rdx", "movzbl %ah, %eax",
	"sete %al", "setne (%rdi)", "setb %r8b", "setg %sil", "seta %ah", "cmovel %eax, %ebx", "cmovncq (%rax), %rdx",
	"cmovlw %ax, %cx", "cmovgeq %r8, %r9",
	"jmp *%rax", "jmpq *(%rax)", "call *%rbx", "callq *8(%rax)", "jmp *foo(%rip)",
	"ret", "retq", "ret $8", "pushq %rbp", "push %r12", "popq %r15", "pushq $1", "pushq $1000", "pushq (%rax)",
	"popq 8(%rsp)", "pushw %ax", "pushfq", "popfq", "pushf", "leave", "nop", "nopl (%rax)", "nopw 8(%rax)",
	"hlt", "int3", "int $0x80", "ud2", "syscall", "cpuid", "rdtsc", "rdtscp", "pause", "mfence", "lfence", "sfence",
	"clc", "stc", "cld", "std", "lahf", "sahf", "xgetbv", "cltq", "cqto", "cltd", "cwtl", "cbtw", "cwtd", "endbr64",
	"rep movsb", "rep stosq", "repne scasb", "repe cmpsb", "movsl", "stosw", "lodsq", "rep movsq", "lock incl (%rax)",
	"rdrand %eax", "rdseed %rax", "rdrand %ax", "clflush (%rdi)", "prefetcht0 (%rax)", "prefetchnta 64(%rsi)",
	"prefetchw (%rdi)", "ldmxcsr 4(%rsp)", "stmxcsr (%rsp)", "fxsave (%rax)", "enter $16, $0",
	"movb %fs:(%rax), %al", "addq %fs:0x10, %rax",
	// SSE
	"addss %xmm1, %xmm0", "addsd (%rax), %xmm9", "subps %xmm8, %xmm15", "mulpd 16(%rsp), %xmm2", "divss %xmm0, %xmm1",
	"sqrtsd %xmm1, %xmm1", "minps %xmm2, %xmm3", "maxsd %xmm4, %xmm5", "rcpps %xmm0, %xmm1", "rsqrtss %xmm1, %xmm2",
	"andps %xmm1, %xmm0", "andnpd %xmm2, %xmm3", "orps (%rax), %xmm0", "xorpd %xmm0, %xmm0", "unpcklps %xmm1, %xmm0",
	"unpckhpd %xmm1, %xmm0", "shufps $0x1b, %xmm1, %xmm0", "shufpd $1, %xmm2, %xmm3", "cmpps $1, %xmm1, %xmm0",
	"cmpsd $2, %xmm1, %xmm0", "cmpltps %xmm1, %xmm0", "cmpeqsd %xmm2, %xmm3", "ucomiss %xmm1, %xmm0",
	"ucomisd (%rax), %xmm1", "comisd %xmm1, %xmm0", "movaps %xmm1, %xmm0", "movaps (%rax), %xmm1",
	"movaps %xmm2, 16(%rsp)", "movups %xmm0, (%rdi)", "movupd (%rsi), %xmm8", "movss (%rax), %xmm0",
	"movss %xmm0, (%rax)", "movss %xmm1, %xmm0", "movsd %xmm0, 8(%rsp)", "movsd 8(%rsp), %xmm10", "movdqa %xmm0, %xmm1",
	"movdqu (%rdi), %xmm2", "movdqu %xmm2, (%rsi)", "movdqa %xmm9, (%rax)", "movlps (%rax), %xmm0",
	"movhps %xmm0, (%rax)", "movhlps %xmm1, %xmm0", "movlhps %xmm1, %xmm0", "movntdq %xmm0, (%rdi)",
	"movntps %xmm1, (%rax)", "lddqu (%rax), %xmm0", "movnti %eax, (%rdi)", "movntiq %rax, (%rdi)",
	"cvtsi2sdl %eax, %xmm0", "cvtsi2sdq %rax, %xmm1", "cvtsi2ssl (%rax), %xmm0", "cvtsi2ss %r8, %xmm9",
	"cvttsd2si %xmm0, %eax", "cvttsd2si %xmm0, %rax", "cvtsd2si (%rax), %r10", "cvttss2si %xmm1, %ecx",
	"cvtss2sd %xmm0, %xmm0", "cvtsd2ss (%rax), %xmm1", "cvtdq2ps %xmm0, %xmm1", "cvttps2dq %xmm1, %xmm2",
	"cvtps2pd %xmm0, %xmm1", "cvtpd2dq %xmm0, %xmm1", "cvtdq2pd %xmm0, %xmm1",
	"movd %eax, %xmm0", "movd %xmm0, %eax", "movd (%rax), %xmm1", "movd %xmm1, (%rax)", "movq %rax, %xmm0",
	"movq %xmm0, %rax", "movq %xmm1, %xmm0", "movq (%rax), %xmm0", "movq %xmm0, (%rax)", "movq %r9, %xmm12",
	"paddd %xmm1, %xmm0", "psubb (%rax), %xmm2", "pxor %xmm0, %xmm0", "pand %xmm9, %xmm1", "por %xmm1, %xmm8",
	"pcmpeqb %xmm1, %xmm0", "pmovmskb %xmm0, %eax", "movmskps %xmm1, %ecx", "movmskpd %xmm0, %eax",
	"punpcklbw %xmm1, %xmm0", "punpckhqdq %xmm1, %xmm0", "packuswb %xmm1, %xmm0", "pmuludq %xmm1, %xmm0",
	"pslld $4, %xmm0", "psrlq $1, %xmm9", "psraw %xmm1, %xmm0", "pslldq $8, %xmm1", "psrldq $4, %xmm2",
	"psllq (%rax), %xmm0", "pshufd $0x4e, %xmm0, %xmm1", "pshuflw $0, (%rax), %xmm1", "pshufhw $0xff, %xmm2, %xmm3",
	"pextrw $3, %xmm0, %eax", "pinsrw $2, %eax, %xmm1", "pinsrw $1, (%rax), %xmm0",
	"pshufb %xmm1, %xmm0", "palignr $4, %xmm1, %xmm0", "pabsd %xmm1, %xmm0", "ptest %xmm1, %xmm0",
	"pmovzxbw (%rax), %xmm0", "pmovsxdq %xmm1, %xmm2", "pmulld %xmm1, %xmm0", "pminud %xmm2, %xmm3",
	"pcmpgtq %xmm1, %xmm0", "pcmpeqq %xmm1, %xmm0", "roundsd $1, %xmm1, %xmm0", "roundps $4, (%rax), %xmm2",
	"blendps $5, %xmm1, %xmm0", "blendvps %xmm0, %xmm1, %xmm2", "pblendvb %xmm0, %xmm3, %xmm4",
	"pextrd $1, %xmm0, %eax", "pextrq $1, %xmm1, %rax", "pextrb $15, %xmm2, (%rdi)", "pinsrd $2, %eax, %xmm0",
	"pinsrq $1, %rax, %xmm9", "pinsrb $0, (%rsi), %xmm1", "insertps $0x10, %xmm1, %xmm0", "extractps $1, %xmm0, %eax",
	"pclmulqdq $0x11, %xmm1, %xmm0", "aesenc %xmm1, %xmm0", "aesenclast (%rax), %xmm2", "aesdec %xmm3, %xmm4",
	"aesimc %xmm1, %xmm0", "aeskeygenassist $1, %xmm0, %xmm1", "pcmpistri $0x18, (%rdi), %xmm0",
	"sha256rnds2 %xmm0, %xmm1, %xmm2", "sha1rnds4 $3, %xmm1, %xmm0", "haddps %xmm1, %xmm0", "movddup %xmm1, %xmm0",
	"dpps $0xff, %xmm1, %xmm0",
	// x87
	"flds (%rax)", "fldl 8(%rsp)", "fldt (%rdi)", "fstps (%rax)", "fstpl -8(%rbp)", "fstpt (%rsi)", "fsts (%rax)",
	"fstl (%rax)", "filds (%rax)", "fildl (%rax)", "fildll (%rax)", "fildq (%rax)", "fistps (%rax)", "fistpl (%rax)",
	"fistpll (%rax)", "fisttpl (%rax)", "fisttpll (%rax)", "fld %st(1)", "fld %st", "fstp %st(0)", "fstp %st(3)",
	"fst %st(2)", "fxch %st(1)", "fxch", "fchs", "fabs", "fsqrt", "fsin", "fcos", "fsincos", "fptan", "fpatan",
	"fyl2x", "fyl2xp1", "f2xm1", "fscale", "frndint", "fprem", "fprem1", "fld1", "fldz", "fldpi", "fldl2e",
	"fldln2", "fxam", "ftst", "fcompp", "fucompp", "fnstsw %ax", "fnstsw (%rax)", "fstsw %ax", "fnstcw (%rax)",
	"fldcw (%rax)", "fstcw 2(%rsp)", "fwait", "fninit", "fnclex",
	"fadd %st(1), %st", "fadd %st, %st(1)", "faddp %st, %st(1)", "faddp", "fsub %st(1), %st", "fsub %st, %st(2)",
	"fsubp %st, %st(1)", "fsubrp %st, %st(1)", "fsubr %st(2), %st", "fsubr %st, %st(2)", "fmul %st(3), %st",
	"fmulp %st, %st(2)", "fdiv %st(1), %st", "fdiv %st, %st(1)", "fdivp %st, %st(1)", "fdivrp %st, %st(1)",
	"fdivr %st(1), %st", "fdivr %st, %st(1)", "fsubp", "fdivp", "fsubrp", "fdivrp", "fmulp",
	"fadds (%rax)", "faddl (%rax)", "fmuls (%rax)", "fsubl (%rax)", "fsubrs (%rax)", "fdivl (%rax)", "fdivrl (%rax)",
	"fiaddl (%rax)", "fimuls (%rax)", "fcoms (%rax)", "fcompl (%rax)", "fcom %st(1)", "fcomp %st(2)", "fucom %st(1)",
	"fucomp %st(1)", "fcomi %st(1), %st", "fcomip %st(1), %st", "fucomi %st(1), %st", "fucomip %st(1), %st",
	"ffree %st(1)", "fcmovb %st(1), %st", "fcmovne %st(2), %st",
}

func TestAssembleTextAgainstGas(t *testing.T) {
	if _, err := exec.LookPath("as"); err != nil {
		t.Skip("GNU as not found")
	}
	dir := t.TempDir()
	var src strings.Builder
	for i, line := range attCorpus {
		fmt.Fprintf(&src, "\t.section .t%d,\"ax\",@progbits\n\t%s\n", i, line)
	}
	sfile := filepath.Join(dir, "c.s")
	ofile := filepath.Join(dir, "c.o")
	os.WriteFile(sfile, []byte(src.String()), 0o644)
	if out, err := exec.Command("as", "--64", "-o", ofile, sfile).CombinedOutput(); err != nil {
		t.Fatalf("as: %v\n%s", err, out)
	}
	f, err := elf.Open(ofile)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()
	fails := 0
	for i, line := range attCorpus {
		sec := f.Section(fmt.Sprintf(".t%d", i))
		want, _ := sec.Data()
		a := &Asm{}
		env := &TextEnv{Section: func(name, flags string) (*Asm, error) { return a, nil }}
		if err := AssembleText(env, line); err != nil {
			t.Errorf("%s: %v", line, err)
			fails++
			continue
		}
		if err := a.Finish(); err != nil {
			t.Errorf("%s: %v", line, err)
			continue
		}
		if !bytes.Equal(a.Code, want) {
			t.Errorf("%-32s got % x, want % x", line, a.Code, want)
			fails++
		}
		if fails > 40 {
			t.Fatal("too many failures")
		}
	}
}

// TestAssembleTextLabels checks local labels, numeric labels and directives
func TestAssembleTextLabels(t *testing.T) {
	a := &Asm{}
	env := &TextEnv{Section: func(name, flags string) (*Asm, error) { return a, nil }}
	text := "1: dec %ecx\n jnz 1b\n jmp 2f\n .byte 1, 2\n2: .long 0x01020304\n loop 1b; .asciz \"a\\n\"\nlbl: lea lbl(%rip), %rax"
	if err := AssembleText(env, text); err != nil {
		t.Fatal(err)
	}
	if err := a.Finish(); err != nil {
		t.Fatal(err)
	}
	want := []byte{0xff, 0xc9, 0x75, 0xfc, 0xeb, 0x02, 1, 2, 4, 3, 2, 1, 0xe2, 0xf2, 'a', '\n', 0,
		0x48, 0x8d, 0x05, 0xf9, 0xff, 0xff, 0xff}
	if !bytes.Equal(a.Code, want) {
		t.Fatalf("got % x\nwant % x", a.Code, want)
	}
}

// TestAssembleFilesAgainstGas assembles every .s file of $ASM_COMPARE_DIR
// (recursively; only paths containing $ASM_COMPARE_MATCH) with GNU as and with AssembleText and compares the
// section contents. It is skipped when the variable is not set.
func TestAssembleFilesAgainstGas(t *testing.T) {
	root := os.Getenv("ASM_COMPARE_DIR")
	if root == "" {
		t.Skip("ASM_COMPARE_DIR not set")
	}
	var files []string
	filepath.Walk(root, func(p string, info os.FileInfo, err error) error {
		if err == nil && !info.IsDir() && strings.HasSuffix(p, ".s") && strings.Contains(p, os.Getenv("ASM_COMPARE_MATCH")) {
			files = append(files, p)
		}
		return nil
	})
	dir := t.TempDir()
	for _, file := range files {
		src, _ := os.ReadFile(file)
		ofile := filepath.Join(dir, "x.o")
		if out, err := exec.Command("as", "--64", "-o", ofile, file).CombinedOutput(); err != nil {
			t.Logf("%s: GNU as failed: %s", file, out)
			continue
		}
		f, err := elf.Open(ofile)
		if err != nil {
			t.Fatal(err)
		}
		res, err := Generate(&cc.Program{TopAsm: []string{string(src)}}, file, false)
		if err != nil {
			t.Errorf("%s: %v", file, err)
			f.Close()
			continue
		}
		for _, s := range f.Sections {
			if s.Type != elf.SHT_PROGBITS || s.Size == 0 || s.Flags&elf.SHF_ALLOC == 0 {
				continue
			}
			want, _ := s.Data()
			var got []byte
			for _, os := range res.Obj.Sections {
				if os.Name == s.Name {
					got = append(got, os.Data...)
				}
			}
			if !bytes.Equal(got, want) {
				t.Errorf("%s: section %s differs\n got % x\nwant % x", file, s.Name, got, want)
			}
		}
		f.Close()
	}
	t.Logf("%d files", len(files))
}
