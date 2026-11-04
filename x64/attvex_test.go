package x64

import (
	"bytes"
	"debug/elf"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"testing"
)

// vexCorpus generates operand variants for every VEX instruction of the
// table: low and high registers (VEX.R/X/B and the 2- or 3-byte prefix),
// xmm and ymm forms and memory operands
func vexCorpus() []string {
	vx := []string{"%xmm1", "%xmm9"}
	vy := []string{"%ymm2", "%ymm12"}
	mems := []string{"(%rax)", "16(%r13,%r9,4)", "sym(%rip)"}
	var lines []string
	add := func(format string, args ...any) { lines = append(lines, fmt.Sprintf(format, args...)) }
	names := make([]string, 0, len(vexTable))
	for n := range vexTable {
		if !strings.Contains(n, "$") {
			names = append(names, n)
		}
	}
	sort.Strings(names)
	for _, n := range names {
		d := vexTable[n]
		sets := [][]string{vx, vy}
		for _, regs := range sets {
			a, b := regs[0], regs[1]
			switch d.form {
			case vNDS:
				add("%s %s, %s, %s", n, a, b, a)
				add("%s %s, %s, %s", n, b, a, b)
				for _, m := range mems {
					add("%s %s, %s, %s", n, m, a, b)
				}
			case vNDSI:
				add("%s $5, %s, %s, %s", n, a, b, a)
				add("%s $0x81, %s, %s, %s", n, mems[1], a, b)
				add("%s $3, %s, %s, %s", n, mems[2], b, a)
				// inserts take an xmm source into ymm
				add("%s $1, %s, %s, %s", n, vx[1], a, b)
			case v2:
				add("%s %s, %s", n, a, b)
				add("%s %s, %s", n, b, a)
				for _, m := range mems {
					add("%s %s, %s", n, m, a)
				}
				add("%s %s, %s", n, vx[0], b)
				add("%s %s, %s", n, a, vx[1])
			case v2I:
				add("%s $7, %s, %s", n, a, b)
				add("%s $0x1b, %s, %s", n, mems[1], a)
				add("%s $1, %s, %s", n, mems[2], b)
			case vMov:
				add("%s %s, %s", n, a, b)
				add("%s %s, %s", n, b, a)
				for _, m := range mems {
					add("%s %s, %s", n, m, a)
					add("%s %s, %s", n, b, m)
				}
			case vStore:
				for _, m := range mems {
					add("%s %s, %s", n, b, m)
				}
			case vShift:
				add("%s $3, %s, %s", n, a, b)
				add("%s $9, %s, %s", n, b, a)
				add("%s %s, %s, %s", n, vx[1], a, b)
				add("%s %s, %s, %s", n, mems[1], b, a)
			case vBlendv:
				add("%s %s, %s, %s, %s", n, a, b, a, b)
				add("%s %s, %s, %s, %s", n, b, mems[1], a, b)
			case vScalarMov:
				if regs[0] == vx[0] {
					add("%s %s, %s, %s", n, a, b, a)
					add("%s %s, %s, %s", n, b, a, vx[0])
					for _, m := range mems {
						add("%s %s, %s", n, m, b)
						add("%s %s, %s", n, a, m)
					}
				}
			case vMovLH:
				if regs[0] == vx[0] {
					for _, m := range mems {
						add("%s %s, %s, %s", n, m, a, b)
						add("%s %s, %s", n, b, m)
					}
				}
			case vExtract:
				add("%s $1, %s, %s", n, a, vx[1])
				add("%s $0, %s, %s", n, b, mems[1])
			case vMaskMov:
				add("%s %s, %s, %s", n, mems[0], a, b)
				add("%s %s, %s, %s", n, a, b, mems[1])
			case vBroadcast:
				add("%s %s, %s", n, vx[1], a)
				add("%s %s, %s", n, vx[0], b)
				for _, m := range mems {
					add("%s %s, %s", n, m, b)
				}
			case vZero:
				if regs[0] == vx[0] {
					add("%s", n)
				}
			}
		}
	}
	for _, n := range []string{"vmovd", "vmovq"} {
		add("%s %%eax, %%xmm3", n)
		add("%s %%r9d, %%xmm11", n)
		add("%s %%xmm3, %%r10d", n)
		add("%s (%%rax), %%xmm12", n)
		add("%s %%xmm12, 8(%%rbx)", n)
	}
	add("vmovq %%rax, %%xmm3")
	add("vmovq %%xmm9, %%r11")
	add("vmovq %%xmm1, %%xmm9")
	for _, n := range []string{"vpmovmskb", "vmovmskps", "vmovmskpd"} {
		add("%s %%xmm3, %%eax", n)
		add("%s %%ymm11, %%r9d", n)
	}
	for _, n := range []string{"vpextrb", "vpextrw", "vpextrd", "vextractps"} {
		add("%s $1, %%xmm3, %%eax", n)
		add("%s $2, %%xmm11, (%%r9)", n)
	}
	add("vpextrw $3, %%xmm9, %%r10d")
	add("vpextrq $1, %%xmm3, %%rax")
	for _, n := range []string{"vpinsrb", "vpinsrw", "vpinsrd"} {
		add("%s $1, %%eax, %%xmm3, %%xmm4", n)
		add("%s $2, (%%r9), %%xmm11, %%xmm12", n)
	}
	add("vpinsrq $1, %%rax, %%xmm3, %%xmm14")
	add("vinsertps $0x10, %%xmm1, %%xmm9, %%xmm3")
	add("vcvtsi2sd %%eax, %%xmm1, %%xmm2")
	add("vcvtsi2sd %%r11, %%xmm9, %%xmm12")
	add("vcvtsi2ssl (%%rax), %%xmm1, %%xmm2")
	add("vcvtsi2sdq 8(%%rbx), %%xmm1, %%xmm12")
	add("vcvttsd2si %%xmm1, %%eax")
	add("vcvttss2si %%xmm9, %%r11")
	add("vcvtsd2si (%%rax), %%r9d")
	add("vcvtss2si %%xmm3, %%rcx")
	add("vpermilps $0x1b, %%xmm1, %%xmm9")
	add("vpermilpd $5, %%ymm2, %%ymm12")
	for _, l := range []string{
		"andn %eax, %ebx, %ecx", "andn (%r9), %r10, %r11", "bextr %eax, %ebx, %ecx", "bextr %r9, (%rax), %r10",
		"blsr %eax, %ebx", "blsmsk (%r9), %r10", "blsi %r11, %rax", "bzhi %eax, %ebx, %ecx", "bzhi %r9, %r10, %r11",
		"pdep %eax, %ebx, %ecx", "pext (%rax), %r9, %r10", "mulx %r11, %r9, %r8", "sarx %eax, %ebx, %ecx",
		"shlx %r9, (%rax), %r10", "shrx %rcx, %rdx, %r12", "rorx $5, %eax, %ebx", "rorx $63, (%r9), %r10",
		"andnl %eax, %ebx, %ecx", "shlxq %r9, %r10, %r11",
	} {
		lines = append(lines, l)
	}
	return lines
}

func TestVEXAgainstGas(t *testing.T) {
	if _, err := exec.LookPath("as"); err != nil {
		t.Skip("GNU as not found")
	}
	lines := vexCorpus()
	dir := t.TempDir()
	sfile := filepath.Join(dir, "v.s")
	ofile := filepath.Join(dir, "v.o")
	errLine := regexp.MustCompile(`v\.s:(\d+): Error`)
	// drop the lines GNU as rejects (operand combinations that do not exist)
	var rejected []string
	for round := 0; round < 3; round++ {
		var src strings.Builder
		for i, line := range lines {
			fmt.Fprintf(&src, "\t.section .t%d,\"ax\",@progbits\n\t%s\n", i, line)
		}
		os.WriteFile(sfile, []byte(src.String()), 0o644)
		out, err := exec.Command("as", "--64", "-o", ofile, sfile).CombinedOutput()
		if err == nil {
			break
		}
		bad := map[int]bool{}
		for _, m := range errLine.FindAllStringSubmatch(string(out), -1) {
			n, _ := strconv.Atoi(m[1])
			bad[(n-2)/2] = true
		}
		if len(bad) == 0 {
			t.Fatalf("as: %v\n%s", err, out)
		}
		var keep []string
		for i, l := range lines {
			if bad[i] {
				rejected = append(rejected, l)
			} else {
				keep = append(keep, l)
			}
		}
		lines = keep
	}
	f, err := elf.Open(ofile)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()
	fails := 0
	for i, line := range lines {
		want, _ := f.Section(fmt.Sprintf(".t%d", i)).Data()
		if len(want) > 0 && want[0] == 0x62 {
			continue // GNU as chose an AVX-512 (EVEX) form
		}
		a := &Asm{}
		env := &TextEnv{Section: func(name, flags string) (*Asm, error) { return a, nil }}
		err := AssembleText(env, line)
		if err == nil {
			err = a.Finish()
		}
		switch {
		case err != nil:
			t.Errorf("%-44s %v", line, err)
			fails++
		case !bytes.Equal(a.Code, want):
			t.Errorf("%-44s got % x, want % x", line, a.Code, want)
			fails++
		}
		if fails > 60 {
			t.Fatal("too many failures")
		}
	}
	// what GNU as rejects must not be accepted silently either
	accepted := 0
	for _, line := range rejected {
		a := &Asm{}
		env := &TextEnv{Section: func(name, flags string) (*Asm, error) { return a, nil }}
		if AssembleText(env, line) == nil {
			accepted++
			if testing.Verbose() {
				t.Logf("accepted although GNU as rejects it: %s", line)
			}
		}
	}
	t.Logf("%d instructions compared, %d variants rejected by GNU as (%d of them accepted here)", len(lines), len(rejected), accepted)
}
