package x64

import (
	"encoding/binary"
	"fmt"
	"sort"
	"strconv"
	"strings"
	"sync/atomic"

	"github.com/ipoluianov/altc/elf"
)

// Call frame information: the .cfi_* directives of assembly text and the
// frame rules of generated functions are recorded on the Asm of their
// section, then turned into an .eh_frame section laid out like GNU as does
// (the same CIE selection, advance encodings and padding), so the unwinder
// (backtraces, C++ exceptions, pthread_cancel) can walk through the code.

type cfiKind uint8

const (
	cfiStart cfiKind = iota
	cfiEnd
	cfiOp          // DW_CFA bytes
	cfiPersonality // enc, sym
	cfiLSDA        // enc, sym
	cfiSignal      // the FDE is a signal frame
)

type cfiRec struct {
	pos    int
	kind   cfiKind
	ops    []byte
	stop   bool  // the op ends the part of an FDE a CIE may hold (remember_state, escape)
	seq    int64 // order of the .cfi_startproc in the whole unit
	simple bool  // .cfi_startproc simple: no initial instructions
	enc    byte
	sym    string
}

type cfiState struct {
	recs   []cfiRec
	cfaOff int64
	stack  []int64
	open   bool
}

var cfiSeq atomic.Int64

// DWARF register numbers of the general-purpose registers (by encoding)
var dwarfGPR = [16]byte{0, 2, 1, 3, 7, 6, 4, 5, 8, 9, 10, 11, 12, 13, 14, 15}

func dwarfReg(r Reg) byte {
	if r.IsXMM() {
		return 17 + byte(r-XMM0)
	}
	return dwarfGPR[r&15]
}

func appendULEB(b []byte, v uint64) []byte {
	for {
		c := byte(v & 0x7f)
		v >>= 7
		if v != 0 {
			b = append(b, c|0x80)
			continue
		}
		return append(b, c)
	}
}

func appendSLEB(b []byte, v int64) []byte {
	for {
		c := byte(v & 0x7f)
		v >>= 7
		if v == 0 && c&0x40 == 0 || v == -1 && c&0x40 != 0 {
			return append(b, c)
		}
		b = append(b, c|0x80)
	}
}

// ---- encodings of the rules (data alignment -8) ----

func cfaDefCFA(reg uint64, off int64) []byte {
	if off < 0 {
		return appendSLEB(appendULEB([]byte{0x12}, reg), off/-8)
	}
	return appendULEB(appendULEB([]byte{0x0c}, reg), uint64(off))
}

func cfaDefCFAOffset(off int64) []byte {
	if off < 0 {
		return appendSLEB([]byte{0x13}, off/-8)
	}
	return appendULEB([]byte{0x0e}, uint64(off))
}

func cfaDefCFARegister(reg uint64) []byte { return appendULEB([]byte{0x0d}, reg) }

func cfaOffset(reg uint64, off int64) []byte {
	scaled := off / -8
	switch {
	case scaled < 0:
		return appendSLEB(appendULEB([]byte{0x11}, reg), scaled)
	case reg < 64:
		return appendULEB([]byte{0x80 | byte(reg)}, uint64(scaled))
	}
	return appendULEB(appendULEB([]byte{0x05}, reg), uint64(scaled))
}

func cfaRestore(reg uint64) []byte {
	if reg < 64 {
		return []byte{0xc0 | byte(reg)}
	}
	return appendULEB([]byte{0x06}, reg)
}

// ---- recording ----

func (a *Asm) cfiAdd(r cfiRec) {
	r.pos = len(a.Code)
	a.cfi.recs = append(a.cfi.recs, r)
}

func (a *Asm) cfiListing(format string, args ...any) {
	if a.Listing != nil {
		fmt.Fprintf(a.Listing, "\t"+format+"\n", args...)
	}
}

// CFIStartProc opens the frame description of a function at this point
func (a *Asm) CFIStartProc() {
	a.cfiListing(".cfi_startproc")
	a.cfi.open, a.cfi.cfaOff, a.cfi.stack = true, 8, nil
	a.cfiAdd(cfiRec{kind: cfiStart, seq: cfiSeq.Add(1)})
}

// CFIEndProc closes it
func (a *Asm) CFIEndProc() {
	a.cfiListing(".cfi_endproc")
	a.cfi.open = false
	a.cfiAdd(cfiRec{kind: cfiEnd})
}

func (a *Asm) cfiOp(ops []byte, stop bool) {
	if a.cfi.open {
		a.cfiAdd(cfiRec{kind: cfiOp, ops: ops, stop: stop})
	}
}

// CFIDefCFAOffset: the CFA is now off bytes above the CFA register
func (a *Asm) CFIDefCFAOffset(off int64) {
	a.cfiListing(".cfi_def_cfa_offset %d", off)
	a.cfi.cfaOff = off
	a.cfiOp(cfaDefCFAOffset(off), false)
}

// CFIDefCFARegister: the CFA is now computed from r
func (a *Asm) CFIDefCFARegister(r Reg) {
	a.cfiListing(".cfi_def_cfa_register %d", dwarfReg(r))
	a.cfiOp(cfaDefCFARegister(uint64(dwarfReg(r))), false)
}

// CFIDefCFA: the CFA is r + off
func (a *Asm) CFIDefCFA(r Reg, off int64) {
	a.cfiListing(".cfi_def_cfa %d, %d", dwarfReg(r), off)
	a.cfi.cfaOff = off
	a.cfiOp(cfaDefCFA(uint64(dwarfReg(r)), off), false)
}

// CFIOffset: r is saved at CFA + off
func (a *Asm) CFIOffset(r Reg, off int64) {
	a.cfiListing(".cfi_offset %d, %d", dwarfReg(r), off)
	a.cfiOp(cfaOffset(uint64(dwarfReg(r)), off), false)
}

// CFIRememberState and CFIRestoreState bracket an epilogue followed by
// more code of the function
func (a *Asm) CFIRememberState() {
	a.cfiListing(".cfi_remember_state")
	a.cfi.stack = append(a.cfi.stack, a.cfi.cfaOff)
	a.cfiOp([]byte{0x0a}, true)
}

func (a *Asm) CFIRestoreState() {
	a.cfiListing(".cfi_restore_state")
	if n := len(a.cfi.stack); n > 0 {
		a.cfi.cfaOff, a.cfi.stack = a.cfi.stack[n-1], a.cfi.stack[:n-1]
	}
	a.cfiOp([]byte{0x0b}, false)
}

// FramePrologue emits push %rbp; mov %rsp, %rbp with their frame rules
func (a *Asm) FramePrologue() {
	a.Push(RBP)
	a.CFIDefCFAOffset(16)
	a.CFIOffset(RBP, -16)
	a.Mov(8, Q(RBP), Q(RSP))
	a.CFIDefCFARegister(RBP)
}

// SaveReg stores a callee-saved register at off(%rbp) and records where
func (a *Asm) SaveReg(r Reg, off int32) {
	a.Mov(8, M(RBP, off), Q(r))
	a.CFIOffset(r, int64(off)-16)
}

// FrameReturn emits leave; ret; the rules of the frame go on for the code
// after it when more follows in the function
func (a *Asm) FrameReturn(more bool) {
	if more {
		a.CFIRememberState()
	}
	a.Leave()
	a.CFIDefCFA(RSP, 8)
	a.Ret()
	if more {
		a.CFIRestoreState()
	}
}

// ---- .cfi_* directives of assembly text ----

var dwarfRegNames = func() map[string]uint64 {
	m := map[string]uint64{"rip": 16, "eflags": 49, "rflags": 49, "es": 50, "cs": 51, "ss": 52, "ds": 53, "fs": 54, "gs": 55,
		"fs.base": 58, "gs.base": 59, "tr": 62, "ldtr": 63, "mxcsr": 64, "fcw": 65, "fsw": 66}
	for i, n := range []string{"rax", "rdx", "rcx", "rbx", "rsi", "rdi", "rbp", "rsp"} {
		m[n] = uint64(i)
	}
	for i := 8; i < 16; i++ {
		m[fmt.Sprintf("r%d", i)] = uint64(i)
	}
	for i := 0; i < 16; i++ {
		m[fmt.Sprintf("xmm%d", i)] = uint64(17 + i)
	}
	for i := 0; i < 8; i++ {
		m[fmt.Sprintf("st(%d)", i)] = uint64(33 + i)
		m[fmt.Sprintf("st%d", i)] = uint64(33 + i)
		m[fmt.Sprintf("mm%d", i)] = uint64(41 + i)
	}
	return m
}()

func cfiRegArg(s string) (uint64, error) {
	s = strings.TrimSpace(s)
	if n, err := strconv.ParseUint(s, 0, 32); err == nil {
		return n, nil
	}
	if r, ok := dwarfRegNames[strings.ToLower(strings.TrimPrefix(s, "%"))]; ok {
		return r, nil
	}
	return 0, fmt.Errorf("bad register expression '%s'", s)
}

// CFIDirective records a .cfi_* directive of assembly text; num evaluates
// constant expressions
func (a *Asm) CFIDirective(dir string, args []string, num func(string) (int64, error)) error {
	st := &a.cfi
	reg := func(i int) (uint64, error) {
		if i >= len(args) {
			return 0, fmt.Errorf("%s: missing operand", dir)
		}
		return cfiRegArg(args[i])
	}
	val := func(i int) (int64, error) {
		if i >= len(args) {
			return 0, fmt.Errorf("%s: missing operand", dir)
		}
		return num(strings.TrimSpace(args[i]))
	}
	if dir == ".cfi_startproc" {
		if st.open {
			return fmt.Errorf("previous CFI entry not closed (missing .cfi_endproc)")
		}
		st.open, st.cfaOff, st.stack = true, 8, nil
		a.cfiAdd(cfiRec{kind: cfiStart, seq: cfiSeq.Add(1), simple: len(args) > 0 && strings.TrimSpace(args[0]) == "simple"})
		return nil
	}
	switch dir {
	case ".cfi_sections", ".cfi_label":
		return nil
	}
	if !st.open {
		return fmt.Errorf("CFI instruction used without previous .cfi_startproc")
	}
	op := func(b []byte, stop bool) error {
		a.cfiAdd(cfiRec{kind: cfiOp, ops: b, stop: stop})
		return nil
	}
	switch dir {
	case ".cfi_endproc":
		st.open = false
		a.cfiAdd(cfiRec{kind: cfiEnd})
		return nil
	case ".cfi_def_cfa":
		r, err := reg(0)
		if err != nil {
			return err
		}
		off, err := val(1)
		if err != nil {
			return err
		}
		st.cfaOff = off
		return op(cfaDefCFA(r, off), false)
	case ".cfi_def_cfa_offset", ".cfi_adjust_cfa_offset":
		off, err := val(0)
		if err != nil {
			return err
		}
		if dir == ".cfi_adjust_cfa_offset" {
			off += st.cfaOff
		}
		st.cfaOff = off
		return op(cfaDefCFAOffset(off), false)
	case ".cfi_def_cfa_register":
		r, err := reg(0)
		if err != nil {
			return err
		}
		return op(cfaDefCFARegister(r), false)
	case ".cfi_offset", ".cfi_rel_offset", ".cfi_val_offset":
		r, err := reg(0)
		if err != nil {
			return err
		}
		off, err := val(1)
		if err != nil {
			return err
		}
		if dir == ".cfi_rel_offset" {
			off -= st.cfaOff
		}
		if dir == ".cfi_val_offset" {
			if off/-8 < 0 {
				return op(appendSLEB(appendULEB([]byte{0x15}, r), off/-8), false)
			}
			return op(appendULEB(appendULEB([]byte{0x14}, r), uint64(off/-8)), false)
		}
		return op(cfaOffset(r, off), false)
	case ".cfi_restore":
		var b []byte
		for i := range args {
			r, err := reg(i)
			if err != nil {
				return err
			}
			b = append(b, cfaRestore(r)...)
		}
		return op(b, false)
	case ".cfi_undefined", ".cfi_same_value":
		code := byte(0x07)
		if dir == ".cfi_same_value" {
			code = 0x08
		}
		var b []byte
		for i := range args {
			r, err := reg(i)
			if err != nil {
				return err
			}
			b = appendULEB(append(b, code), r)
		}
		return op(b, false)
	case ".cfi_register":
		r1, err := reg(0)
		if err != nil {
			return err
		}
		r2, err := reg(1)
		if err != nil {
			return err
		}
		return op(appendULEB(appendULEB([]byte{0x09}, r1), r2), false)
	case ".cfi_remember_state":
		st.stack = append(st.stack, st.cfaOff)
		return op([]byte{0x0a}, true)
	case ".cfi_restore_state":
		if n := len(st.stack); n > 0 {
			st.cfaOff, st.stack = st.stack[n-1], st.stack[:n-1]
		}
		return op([]byte{0x0b}, false)
	case ".cfi_escape":
		var b []byte
		for i := range args {
			v, err := val(i)
			if err != nil {
				return err
			}
			b = append(b, byte(v))
		}
		return op(b, true)
	case ".cfi_signal_frame":
		a.cfiAdd(cfiRec{kind: cfiSignal})
		return nil
	case ".cfi_personality", ".cfi_lsda":
		enc, err := val(0)
		if err != nil {
			return err
		}
		r := cfiRec{kind: cfiPersonality, enc: byte(enc)}
		if dir == ".cfi_lsda" {
			r.kind = cfiLSDA
		}
		if enc != 0xff {
			if len(args) < 2 {
				return fmt.Errorf("%s: missing symbol", dir)
			}
			switch enc & 0x7f {
			case 0x00, 0x03, 0x0b, 0x13, 0x1b:
			default:
				return fmt.Errorf("%s: unsupported encoding %#x", dir, enc)
			}
			r.sym = strings.TrimSpace(args[1])
		}
		a.cfiAdd(r)
		return nil
	case ".cfi_return_column", ".cfi_window_save", ".cfi_val_encoded_addr", ".cfi_negate_ra_state":
		return fmt.Errorf("%s is not supported", dir)
	}
	return fmt.Errorf("unknown directive '%s'", dir)
}

// ---- building .eh_frame ----

// cfiFDE is the frame description of one function
type cfiFDE struct {
	a          *Asm
	seq        int64
	start, end int
	insns      []cfiRec // the rules with the advances between them
	signal     bool
	persEnc    byte
	pers       string
	lsdaEnc    byte
	lsda       string
}

// EHReloc is a relocation of .eh_frame: to the code of an Asm at Off, or
// to a symbol
type EHReloc struct {
	At     int // offset in .eh_frame (a 4-byte PC-relative field, or 8 bytes absolute when Abs)
	A      *Asm
	Off    int64
	Sym    string
	Abs    bool // an absolute address of Size bytes
	Size   int
	Addend int64
}

// collectFDEs gathers the frame descriptions recorded on the Asms
func collectFDEs(asms []*Asm) ([]*cfiFDE, error) {
	var fdes []*cfiFDE
	for _, a := range asms {
		var cur *cfiFDE
		last := 0
		for _, r := range a.cfi.recs {
			switch r.kind {
			case cfiStart:
				cur = &cfiFDE{a: a, seq: r.seq, start: r.pos, persEnc: 0xff, lsdaEnc: 0xff}
				last = r.pos
				if !r.simple {
					// the rules at the entry: CFA = rsp + 8, return address at CFA - 8
					cur.insns = append(cur.insns, cfiRec{kind: cfiOp, ops: cfaDefCFA(7, 8)}, cfiRec{kind: cfiOp, ops: cfaOffset(16, -8)})
				}
				continue
			case cfiEnd:
				if cur != nil {
					cur.end = r.pos
					fdes = append(fdes, cur)
				}
				cur = nil
				continue
			}
			if cur == nil {
				continue
			}
			switch r.kind {
			case cfiSignal:
				cur.signal = true
			case cfiPersonality:
				cur.persEnc, cur.pers = r.enc, r.sym
			case cfiLSDA:
				cur.lsdaEnc, cur.lsda = r.enc, r.sym
			case cfiOp:
				if r.pos != last {
					cur.insns = append(cur.insns, cfiRec{kind: cfiOp, ops: advanceLoc(r.pos - last), stop: true})
					last = r.pos
				}
				cur.insns = append(cur.insns, r)
			}
		}
		if cur != nil {
			return nil, fmt.Errorf("open CFI at end of file; missing .cfi_endproc directive")
		}
	}
	sort.SliceStable(fdes, func(i, j int) bool { return fdes[i].seq < fdes[j].seq })
	return fdes, nil
}

func advanceLoc(d int) []byte {
	switch {
	case d < 64:
		return []byte{0x40 | byte(d)}
	case d < 256:
		return []byte{0x02, byte(d)}
	case d < 65536:
		return binary.LittleEndian.AppendUint16([]byte{0x03}, uint16(d))
	}
	return binary.LittleEndian.AppendUint32([]byte{0x04}, uint32(d))
}

type cieEntry struct {
	off     int
	insns   []cfiRec
	signal  bool
	persEnc byte
	pers    string
	lsdaEnc byte
}

func ptrSize(enc byte) int {
	switch enc & 0x0f {
	case 0x00, 0x04, 0x0c:
		return 8
	}
	return 4
}

// BuildEHFrame lays out the .eh_frame section of the frame descriptions
// recorded on the Asms (nil when there is none)
func BuildEHFrame(asms []*Asm) ([]byte, []EHReloc, error) {
	fdes, err := collectFDEs(asms)
	if err != nil || len(fdes) == 0 {
		return nil, nil, err
	}
	var out []byte
	var rels []EHReloc
	le := binary.LittleEndian
	// records are 4-byte aligned and the last one ends the section on an
	// 8-byte boundary, like GNU as makes them
	pad := func(start int, align int) {
		for len(out)%align != 0 {
			out = append(out, 0) // DW_CFA_nop
		}
		le.PutUint32(out[start:], uint32(len(out)-start-4))
	}
	pointer := func(enc byte, sym string) {
		rels = append(rels, EHReloc{At: len(out), Sym: sym, Abs: enc&0x70 != 0x10, Size: ptrSize(enc)})
		out = append(out, make([]byte, ptrSize(enc))...)
	}
	var cies []*cieEntry
	for fi, f := range fdes {
		// the CIE holding the leading rules of the FDE (as GNU as chooses it)
		var cie *cieEntry
		first := 0
		for _, c := range cies {
			if c.signal != f.signal || c.persEnc != f.persEnc || c.pers != f.pers || c.lsdaEnc != f.lsdaEnc {
				continue
			}
			if len(f.insns) < len(c.insns) {
				continue
			}
			ok := true
			for i, x := range c.insns {
				if f.insns[i].stop || string(f.insns[i].ops) != string(x.ops) {
					ok = false
					break
				}
			}
			// the CIE rules only need to begin the FDE's (GNU as does so)
			if ok {
				cie, first = c, len(c.insns)
				break
			}
		}
		if cie == nil {
			n := 0
			for n < len(f.insns) && !f.insns[n].stop {
				n++
			}
			cie = &cieEntry{off: len(out), insns: f.insns[:n], signal: f.signal, persEnc: f.persEnc, pers: f.pers, lsdaEnc: f.lsdaEnc}
			cies = append(cies, cie)
			first = n
			start := len(out)
			out = append(out, 0, 0, 0, 0, 0, 0, 0, 0, 1) // length, CIE id, version
			aug := "z"
			if f.persEnc != 0xff {
				aug += "P"
			}
			if f.lsdaEnc != 0xff {
				aug += "L"
			}
			aug += "R"
			if f.signal {
				aug += "S"
			}
			out = append(out, aug...)
			out = append(out, 0, 1, 0x78, 16) // code align 1, data align -8, return address column
			augLen := 1
			if f.persEnc != 0xff {
				augLen += 1 + ptrSize(f.persEnc)
			}
			if f.lsdaEnc != 0xff {
				augLen++
			}
			out = appendULEB(out, uint64(augLen))
			if f.persEnc != 0xff {
				out = append(out, f.persEnc)
				pointer(f.persEnc, f.pers)
			}
			if f.lsdaEnc != 0xff {
				out = append(out, f.lsdaEnc)
			}
			out = append(out, 0x1b) // FDE addresses: pc-relative, signed 4 bytes
			for _, x := range cie.insns {
				out = append(out, x.ops...)
			}
			pad(start, 4)
		}
		start := len(out)
		out = append(out, 0, 0, 0, 0)
		out = le.AppendUint32(out, uint32(len(out)-cie.off))
		rels = append(rels, EHReloc{At: len(out), A: f.a, Off: int64(f.start)})
		out = append(out, 0, 0, 0, 0)
		out = le.AppendUint32(out, uint32(f.end-f.start))
		if f.lsdaEnc != 0xff {
			out = appendULEB(out, uint64(ptrSize(f.lsdaEnc)))
			pointer(f.lsdaEnc, f.lsda)
		} else {
			out = append(out, 0)
		}
		for _, x := range f.insns[first:] {
			out = append(out, x.ops...)
		}
		if fi == len(fdes)-1 {
			pad(start, 8)
		} else {
			pad(start, 4)
		}
	}
	return out, rels, nil
}

// ehFrame adds the .eh_frame section of the unit
func (g *gen) ehFrame(syms map[string]*elf.Symbol) error {
	asms := []*Asm{g.a}
	for _, s := range g.asmSecs {
		asms = append(asms, s.a)
	}
	data, rels, err := BuildEHFrame(asms)
	if err != nil || data == nil {
		return err
	}
	o := g.obj
	sec := o.AddSection(".eh_frame", elf.SecEHFrame, 8)
	sec.Data = data
	secSyms := map[*elf.Section]*elf.Symbol{}
	for _, s := range o.Symbols {
		if s.Type == elf.SymSection && s.Section != nil {
			secSyms[s.Section] = s
		}
	}
	for _, r := range rels {
		var s *elf.Symbol
		addend := r.Addend
		if r.A != nil {
			target := g.secOf[r.A]
			if target == nil {
				return fmt.Errorf("frame description of code outside the sections")
			}
			if s = secSyms[target]; s == nil {
				s = o.AddLocal(&elf.Symbol{Bind: elf.BindLocal, Type: elf.SymSection, Section: target})
				secSyms[target] = s
			}
			addend += r.Off
		} else {
			s = syms[r.Sym]
			if s == nil {
				if tl := g.topLabels[r.Sym]; tl != nil && tl.Defined && g.secOf[tl.A] != nil {
					s = o.AddLocal(&elf.Symbol{Name: r.Sym, Bind: elf.BindLocal, Section: g.secOf[tl.A], Value: int64(tl.A.LabelPos(tl.L)) + tl.Off})
				} else {
					s = o.Symbol(r.Sym)
				}
				syms[r.Sym] = s
			}
		}
		typ := uint32(elf.R_X86_64_PC32)
		switch {
		case r.Abs && r.Size == 4:
			typ = 10 // R_X86_64_32
		case r.Abs:
			typ = elf.R_X86_64_64
		}
		sec.Relocs = append(sec.Relocs, elf.Reloc{Offset: int64(r.At), Type: typ, Sym: s, Addend: addend})
	}
	return nil
}
