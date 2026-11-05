package tests

import (
	"bytes"
	"debug/elf"
	"encoding/binary"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strings"
	"testing"

	"github.com/ipoluianov/altc/driver"
	aelf "github.com/ipoluianov/altc/elf"
)

// TestAssembleGCCOutput assembles the gcc -S -O2 output of every test
// program with the AltC assembler. The sections and relocations must match
// GNU as, and the program linked by AltC must behave like the gcc build.
func TestAssembleGCCOutput(t *testing.T) {
	gcc, err := exec.LookPath("gcc")
	if err != nil {
		t.Skip("gcc not found")
	}
	gas, err := exec.LookPath("as")
	if err != nil {
		t.Skip("as not found")
	}
	files, _ := filepath.Glob("c/*.c")
	if only := os.Getenv("ONLY"); only != "" {
		files, _ = filepath.Glob("c/" + only + ".c")
	}
	tmp := t.TempDir()
	for _, f := range files {
		f := f
		name := strings.TrimSuffix(filepath.Base(f), ".c")
		t.Run(name, func(t *testing.T) {
			t.Parallel()
			src, err := os.ReadFile(f)
			if err != nil {
				t.Fatal(err)
			}
			for _, l := range strings.SplitN(string(src), "\n", 5) {
				if strings.HasPrefix(l, "// skip: ") {
					t.Skip(strings.TrimPrefix(l, "// skip: "))
				}
				if strings.HasPrefix(l, "// skip-asm: ") {
					t.Skip(strings.TrimPrefix(l, "// skip-asm: "))
				}
			}
			std := stdFor(src)
			flags := flagsFor(t, src)
			asmFile := filepath.Join(tmp, name+".s")
			if out, err := exec.Command(gcc, append(append([]string{"-std=" + std, "-w", "-O2"}, flags...), "-S", "-o", asmFile, f)...).CombinedOutput(); err != nil {
				t.Fatalf("gcc -S: %v\n%s", err, out)
			}
			ref := filepath.Join(tmp, name+".gcc2")
			if out, err := exec.Command(gcc, append(append([]string{"-std=" + std, "-w", "-O2"}, flags...), "-o", ref, f, "-lm")...).CombinedOutput(); err != nil {
				t.Fatalf("gcc: %v\n%s", err, out)
			}
			gasObj := filepath.Join(tmp, name+".gas.o")
			if out, err := exec.Command(gas, "--64", "-o", gasObj, asmFile).CombinedOutput(); err != nil {
				t.Skipf("GNU as rejects gcc's output: %s", out)
			}

			data, _ := os.ReadFile(asmFile)
			u := driver.CompileSource(asmFile, data, &driver.Config{Std: std, NoWarnings: true})
			if u.Obj == nil {
				for _, d := range u.Diagnostics {
					t.Error(d)
				}
				t.FailNow()
			}
			compareWithGas(t, gasObj, u.Obj)

			want, wantCode := run(t, ref, tmp)
			img, diags := driver.Link(linkInput(u.Obj))
			if img == nil {
				for _, d := range diags {
					t.Error(d)
				}
				t.FailNow()
			}
			exe := filepath.Join(tmp, name+".as.altc")
			if err := os.WriteFile(exe, img, 0o755); err != nil {
				t.Fatal(err)
			}
			got, gotCode := run(t, exe, tmp)
			if got != want || gotCode != wantCode {
				t.Errorf("output differs\n--- gcc -O2 (exit %d):\n%s\n--- assembled by altc (exit %d):\n%s", wantCode, want, gotCode, got)
			}
		})
	}
}

// compareWithGas checks the loaded sections and the relocations (offset
// and type) against the object written by GNU as
func compareWithGas(t *testing.T, gasObj string, obj *aelf.Object) {
	f, err := elf.Open(gasObj)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()
	ours := map[string][]byte{}
	ourRel := map[string][]string{}
	for _, s := range obj.Sections {
		ours[s.Name] = append(ours[s.Name], s.Data...)
		for _, r := range s.Relocs {
			ourRel[s.Name] = append(ourRel[s.Name], fmt.Sprintf("%x:%d", r.Offset, r.Type))
		}
	}
	for _, s := range f.Sections {
		if (s.Type == elf.SHT_PROGBITS || s.Name == ".eh_frame") && s.Flags&elf.SHF_ALLOC != 0 && s.Size > 0 {
			want, _ := s.Data()
			if !bytes.Equal(ours[s.Name], want) {
				t.Errorf("section %s differs from GNU as", s.Name)
			}
		}
		if s.Type != elf.SHT_RELA {
			continue
		}
		target := f.Sections[s.Info]
		if target.Flags&elf.SHF_ALLOC == 0 {
			continue
		}
		raw, _ := s.Data()
		var want []string
		for i := 0; i+24 <= len(raw); i += 24 {
			off := binary.LittleEndian.Uint64(raw[i:])
			info := binary.LittleEndian.Uint64(raw[i+8:])
			want = append(want, fmt.Sprintf("%x:%d", off, uint32(info)))
		}
		got := ourRel[target.Name]
		sort.Strings(want)
		sort.Strings(got)
		if strings.Join(got, " ") != strings.Join(want, " ") {
			t.Errorf("relocations of %s differ from GNU as\n got %v\nwant %v", target.Name, got, want)
		}
	}
}
