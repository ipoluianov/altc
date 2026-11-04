package elf

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"sort"
	"strings"
)

const (
	baseAddr = 0x400000
	pageSize = 0x1000
)

// Linker builds an executable from objects, archives and shared libraries
type Linker struct {
	Objects    []*Object
	Archives   []*Archive
	Lazy       []*Object // objects linked only when they define a needed symbol (runtime support)
	SharedLibs []*SharedLib
	Interp     string // dynamic loader path
	Entry      string // entry symbol; "_start" by default (generated when missing)
	StripSyms  bool   // do not write .symtab

	globals map[string]*Symbol // name -> definition (or import)
	imports map[*Symbol]*imported
	impList []*imported
	needed  []*SharedLib
	errs    []string

	// layout
	out       []*outSec
	copyBss   *Section
	shIndex   map[string]int
	got       map[*Symbol]int // GOT slot of a symbol (address)
	gotTP     map[*Symbol]int // GOT slot holding a TLS offset
	gotSize   int
	plt       map[*Symbol]int
	pltSize   int
	tlsStart  int64
	tlsEnd    int64 // aligned end used as the thread pointer offset base
	tlsAlign  int64
	dynRelocs []dynReloc

	sectionSyms map[string]int64 // linker-defined symbols
}

type imported struct {
	sym      *Symbol
	lib      *SharedLib
	dyn      *DynSym
	dynIndex int
	copyAddr int64 // address of the copy in .bss (copy relocation), 0 if none
	needCopy bool
	needPLT  bool
}

type dynReloc struct {
	off    int64 // address to patch
	typ    uint32
	imp    *imported // nil: no symbol
	addend int64
}

// outSec is an output section made by merging input sections of a kind
type outSec struct {
	name  string
	kind  SectionKind
	align int64
	secs  []*Section
	addr  int64
	off   int64
	size  int64
	data  []byte // synthesized content (dynamic sections)
	flags uint64
	typ   uint32
	link  int
	info  int
	entsz int64
}

func (l *Linker) errorf(format string, args ...any) {
	l.errs = append(l.errs, fmt.Sprintf(format, args...))
}

// LinkError carries all the undefined references and other link errors
type LinkError struct{ Messages []string }

func (e *LinkError) Error() string { return strings.Join(e.Messages, "\n") }

func (l *Linker) resolve(s *Symbol) *Symbol {
	if s.Bind == BindLocal {
		return s
	}
	if d, ok := l.globals[s.Name]; ok {
		return d
	}
	return s
}

func (l *Linker) addObject(o *Object) {
	for _, s := range o.Symbols {
		if s.Bind == BindLocal || !s.Defined() {
			continue
		}
		old, ok := l.globals[s.Name]
		if !ok {
			l.globals[s.Name] = s
			continue
		}
		switch {
		case old.Type == SymCommon && s.Type == SymCommon:
			if s.Size > old.Size {
				old.Size = s.Size
			}
			if s.Value > old.Value {
				old.Value = s.Value
			}
		case s.Type == SymCommon:
			// a definition beats a common symbol
		case old.Type == SymCommon:
			l.globals[s.Name] = s
		case old.Bind == BindWeak && s.Bind != BindWeak:
			l.globals[s.Name] = s
		case s.Bind == BindWeak:
		default:
			l.errorf("multiple definition of `%s' (%s)", s.Name, o.Name)
		}
	}
}

// undefinedNames lists the names referenced but not defined anywhere yet
func (l *Linker) undefinedNames() []string {
	seen := map[string]bool{}
	var names []string
	for _, o := range l.Objects {
		for _, s := range o.Symbols {
			if s.Bind == BindLocal || s.Defined() || seen[s.Name] {
				continue
			}
			if _, ok := l.globals[s.Name]; ok {
				continue
			}
			seen[s.Name] = true
			names = append(names, s.Name)
		}
	}
	sort.Strings(names)
	return names
}

// Link produces the executable image
func (l *Linker) Link() ([]byte, error) {
	if l.Interp == "" {
		l.Interp = "/lib64/ld-linux-x86-64.so.2"
	}
	if l.Entry == "" {
		l.Entry = "_start"
	}
	l.globals = map[string]*Symbol{}
	l.imports = map[*Symbol]*imported{}
	for _, o := range l.Objects {
		l.addObject(o)
	}
	if _, ok := l.globals[l.Entry]; !ok && l.Entry == "_start" {
		st := startObject()
		l.Objects = append(l.Objects, st)
		l.addObject(st)
	}
	// pull archive members until nothing changes
	for changed := true; changed; {
		changed = false
		for _, name := range l.undefinedNames() {
			for i, lo := range l.Lazy {
				if lo == nil {
					continue
				}
				if s := lo.Lookup(name); s != nil && s.Defined() && s.Bind != BindLocal {
					l.Lazy[i] = nil
					l.Objects = append(l.Objects, lo)
					l.addObject(lo)
					changed = true
					break
				}
			}
			if changed {
				continue
			}
			for _, ar := range l.Archives {
				obj, err := ar.Pull(name)
				if err != nil {
					return nil, err
				}
				if obj != nil {
					l.Objects = append(l.Objects, obj)
					l.addObject(obj)
					changed = true
					break
				}
			}
		}
	}
	// the rest comes from shared libraries
	usedLib := map[*SharedLib]bool{}
	for _, name := range l.undefinedNames() {
		found := false
		for _, lib := range l.SharedLibs {
			if ds, ok := lib.Syms[name]; ok {
				s := &Symbol{Name: name, Bind: BindGlobal, Type: ds.Type, Size: ds.Size}
				imp := &imported{sym: s, lib: lib, dyn: ds}
				l.globals[name] = s
				l.imports[s] = imp
				l.impList = append(l.impList, imp)
				usedLib[lib] = true
				found = true
				break
			}
		}
		if !found {
			// weak undefined references resolve to 0
			strong := l.strongRefs(name)
			if len(strong) == 0 {
				continue
			}
			if l.linkerDefined(name) {
				continue
			}
			l.errorf("undefined reference to `%s' (in %s)", name, strings.Join(strong, ", "))
		}
	}
	if len(l.errs) > 0 {
		return nil, &LinkError{l.errs}
	}
	for _, lib := range l.SharedLibs {
		if usedLib[lib] {
			l.needed = append(l.needed, lib)
		}
	}
	l.scanRelocs()
	if len(l.errs) > 0 {
		return nil, &LinkError{l.errs}
	}
	image := l.layoutAndWrite()
	if len(l.errs) > 0 {
		return nil, &LinkError{l.errs}
	}
	return image, nil
}

func (l *Linker) strongRefs(name string) []string {
	var objs []string
	for _, o := range l.Objects {
		s := o.Lookup(name)
		if s != nil && !s.Defined() && s.Bind != BindWeak && l.isReferenced(o, s) {
			objs = append(objs, o.Name)
		}
	}
	return objs
}

func (l *Linker) isReferenced(o *Object, s *Symbol) bool {
	for _, sec := range o.Sections {
		for _, r := range sec.Relocs {
			if r.Sym == s {
				return true
			}
		}
	}
	return false
}

var linkerSyms = map[string]bool{
	"_GLOBAL_OFFSET_TABLE_": true, "_DYNAMIC": true, "__init_array_start": true, "__init_array_end": true,
	"__fini_array_start": true, "__fini_array_end": true, "__preinit_array_start": true, "__preinit_array_end": true,
	"_end": true, "end": true, "_edata": true, "edata": true, "__bss_start": true, "_etext": true, "etext": true,
	"__executable_start": true, "__ehdr_start": true,
}

func (l *Linker) linkerDefined(name string) bool { return linkerSyms[name] }

// isDynFunc tells whether the symbol is a function imported from a shared library
func (l *Linker) imp(s *Symbol) *imported { return l.imports[s] }

func (l *Linker) scanRelocs() {
	l.got = map[*Symbol]int{}
	l.gotTP = map[*Symbol]int{}
	l.plt = map[*Symbol]int{}
	for _, o := range l.Objects {
		for _, sec := range o.Sections {
			if sec.Kind == SecOther {
				continue
			}
			for _, r := range sec.Relocs {
				s := l.resolve(r.Sym)
				imp := l.imp(s)
				if imp == nil && s.IFunc && s.Section != nil {
					// a local IFUNC: every reference goes through a PLT entry
					// whose GOT slot the dynamic loader fills by calling the
					// resolver (R_X86_64_IRELATIVE)
					if _, ok := l.got[s]; !ok {
						l.got[s] = l.gotSize
						l.gotSize += 8
					}
					if _, ok := l.plt[s]; !ok {
						l.plt[s] = l.pltSize
						l.pltSize += 16
					}
					continue
				}
				switch r.Type {
				case R_X86_64_GOTPCREL, R_X86_64_GOTPCRELX, R_X86_64_REX_GOTPCRELX, R_X86_64_GOT32:
					if _, ok := l.got[s]; !ok {
						l.got[s] = l.gotSize
						l.gotSize += 8
					}
				case R_X86_64_GOTTPOFF:
					if _, ok := l.gotTP[s]; !ok {
						l.gotTP[s] = l.gotSize
						l.gotSize += 8
					}
				case R_X86_64_PLT32, R_X86_64_PC32, R_X86_64_32, R_X86_64_32S, R_X86_64_PC64:
					if imp != nil {
						if imp.dyn.Type == SymFunc || imp.dyn.Type == SymNoType && imp.dyn.Size == 0 {
							if _, ok := l.plt[s]; !ok {
								l.plt[s] = l.pltSize
								l.pltSize += 16
								imp.needPLT = true
								if _, ok := l.got[s]; !ok {
									l.got[s] = l.gotSize
									l.gotSize += 8
								}
							}
						} else {
							imp.needCopy = true
						}
					}
				case R_X86_64_64:
					if imp != nil && sec.Kind != SecData && sec.Kind != SecTData && sec.Kind != SecInitArray &&
						sec.Kind != SecFiniArray && sec.Kind != SecPreinitArray {
						if imp.dyn.Type == SymFunc {
							if _, ok := l.plt[s]; !ok {
								l.plt[s] = l.pltSize
								l.pltSize += 16
								imp.needPLT = true
								if _, ok := l.got[s]; !ok {
									l.got[s] = l.gotSize
									l.gotSize += 8
								}
							}
						} else {
							imp.needCopy = true
						}
					}
				case R_X86_64_TLSGD, R_X86_64_TLSLD, R_X86_64_DTPOFF32, R_X86_64_GOTPC32_TLSDESC, R_X86_64_TLSDESC_CALL:
					l.errorf("%s: TLS model of relocation type %d is not supported (compile with -fno-pic)", o.Name, r.Type)
				}
			}
		}
	}
}

func alignUp(v, a int64) int64 {
	if a <= 1 {
		return v
	}
	return (v + a - 1) / a * a
}

// startObject is the program entry point calling __libc_start_main(main, argc, argv, 0, 0, rtld_fini, stack_end)
func startObject() *Object {
	o := NewObject("<start>")
	text := o.AddSection(".text", SecText, 16)
	text.Data = []byte{
		0x31, 0xed, // xor %ebp,%ebp
		0x49, 0x89, 0xd1, // mov %rdx,%r9
		0x5e,             // pop %rsi
		0x48, 0x89, 0xe2, // mov %rsp,%rdx
		0x48, 0x83, 0xe4, 0xf0, // and $-16,%rsp
		0x50,             // push %rax
		0x54,             // push %rsp
		0x45, 0x31, 0xc0, // xor %r8d,%r8d
		0x31, 0xc9, // xor %ecx,%ecx
		0x48, 0x8d, 0x3d, 0, 0, 0, 0, // lea main(%rip),%rdi
		0xff, 0x15, 0, 0, 0, 0, // call *__libc_start_main@GOTPCREL(%rip)
		0xf4, // hlt
	}
	start := o.Symbol("_start")
	start.Section = text
	start.Type = SymFunc
	start.Size = int64(len(text.Data))
	text.Relocs = []Reloc{
		{Offset: 23, Type: R_X86_64_PC32, Sym: o.Symbol("main"), Addend: -4},
		{Offset: 29, Type: R_X86_64_GOTPCREL, Sym: o.Symbol("__libc_start_main"), Addend: -4},
	}
	return o
}

// symAddr is the final address of a resolved symbol
func (l *Linker) symAddr(s *Symbol) int64 {
	if imp := l.imp(s); imp != nil {
		if imp.copyAddr != 0 {
			return imp.copyAddr
		}
		if off, ok := l.plt[s]; ok {
			return l.secAddr(".plt") + int64(off)
		}
		return 0
	}
	if s.Abs {
		return s.Value
	}
	if s.IFunc {
		if off, ok := l.plt[s]; ok {
			return l.secAddr(".plt") + int64(off)
		}
	}
	if s.Section != nil {
		return s.Section.Addr + s.Value
	}
	if v, ok := l.sectionSyms[s.Name]; ok {
		return v
	}
	return 0 // weak undefined
}

func (l *Linker) secAddr(name string) int64 {
	if o := l.outByName(name); o != nil {
		return o.addr
	}
	return 0
}

func (l *Linker) outByName(name string) *outSec {
	for _, o := range l.out {
		if o.name == name {
			return o
		}
	}
	return nil
}

// Section header constants
const (
	shtProgbits  = 1
	shtSymtab    = 2
	shtStrtab    = 3
	shtRela      = 4
	shtHash      = 5
	shtDynamic   = 6
	shtNobits    = 8
	shtDynsym    = 11
	shtInitArray = 14
	shtFiniArray = 15
	shtPreinit   = 16
	shfWrite     = 1
	shfAlloc     = 2
	shfExec      = 4
	shfTLS       = 0x400
	ptLoad       = 1
	ptDynamic    = 2
	ptInterp     = 3
	ptPhdr       = 6
	ptTLS        = 7
	ptGnuStack   = 0x6474e551
	ptGnuRelro   = 0x6474e552
)

type strtab struct {
	buf bytes.Buffer
	m   map[string]int
}

func newStrtab() *strtab {
	t := &strtab{m: map[string]int{}}
	t.buf.WriteByte(0)
	return t
}

func (t *strtab) add(s string) int {
	if s == "" {
		return 0
	}
	if i, ok := t.m[s]; ok {
		return i
	}
	i := t.buf.Len()
	t.buf.WriteString(s)
	t.buf.WriteByte(0)
	t.m[s] = i
	return i
}

func (l *Linker) layoutAndWrite() []byte {
	le := binary.LittleEndian
	// ---- dynamic symbol table ----
	dynstr := newStrtab()
	for _, lib := range l.needed {
		dynstr.add(lib.Soname)
	}
	var dynsym bytes.Buffer
	dynsym.Write(make([]byte, 24))
	sort.Slice(l.impList, func(i, j int) bool { return l.impList[i].sym.Name < l.impList[j].sym.Name })
	for i, imp := range l.impList {
		imp.dynIndex = i + 1
		dynstr.add(imp.sym.Name)
	}
	nsyms := len(l.impList) + 1
	// hash table: one bucket chaining all symbols
	hash := make([]byte, 8+4+4*nsyms)
	le.PutUint32(hash[0:], 1)
	le.PutUint32(hash[4:], uint32(nsyms))
	if nsyms > 1 {
		le.PutUint32(hash[8:], 1)
	}
	for i := 1; i < nsyms; i++ {
		next := uint32(i + 1)
		if i == nsyms-1 {
			next = 0
		}
		le.PutUint32(hash[12+4*i:], next)
	}

	// ---- group input sections ----
	type group struct {
		name  string
		kind  SectionKind
		flags uint64
		typ   uint32
	}
	groups := []group{
		{".rodata", SecROData, shfAlloc, shtProgbits},
		{".text", SecText, shfAlloc | shfExec, shtProgbits},
		{".tdata", SecTData, shfAlloc | shfWrite | shfTLS, shtProgbits},
		{".tbss", SecTBSS, shfAlloc | shfWrite | shfTLS, shtNobits},
		{".preinit_array", SecPreinitArray, shfAlloc | shfWrite, shtPreinit},
		{".init_array", SecInitArray, shfAlloc | shfWrite, shtInitArray},
		{".fini_array", SecFiniArray, shfAlloc | shfWrite, shtFiniArray},
		{".data", SecData, shfAlloc | shfWrite, shtProgbits},
		{".bss", SecBSS, shfAlloc | shfWrite, shtNobits},
	}
	inputs := map[SectionKind][]*Section{}
	for _, o := range l.Objects {
		for _, s := range o.Sections {
			if s.Kind != SecOther {
				inputs[s.Kind] = append(inputs[s.Kind], s)
			}
		}
	}
	// prioritized init/fini entries first, in priority order (like SORT_BY_INIT_PRIORITY)
	for _, k := range []SectionKind{SecInitArray, SecFiniArray, SecPreinitArray} {
		sort.SliceStable(inputs[k], func(i, j int) bool { return initPriority(inputs[k][i].Name) < initPriority(inputs[k][j].Name) })
	}
	// common symbols go to .bss
	var commons []*Symbol
	for _, s := range l.globals {
		if s.Type == SymCommon {
			commons = append(commons, s)
		}
	}
	sort.Slice(commons, func(i, j int) bool { return commons[i].Name < commons[j].Name })
	{
		cb := &Section{Name: ".bss.common", Kind: SecBSS, Align: 16}
		for _, s := range commons {
			al := s.Value
			if al < 1 {
				al = 1
			}
			off := alignUp(cb.Size, al)
			cb.Size = off + s.Size
			s.Section = cb
			s.Value = off
			s.Type = SymObject
		}
		// copy relocations also live in .bss
		for _, imp := range l.impList {
			if imp.needCopy {
				off := alignUp(cb.Size, 16)
				cb.Size = off + imp.dyn.Size
				imp.copyAddr = off // fixed up after layout
			}
		}
		inputs[SecBSS] = append(inputs[SecBSS], cb)
		l.copyBss = cb
	}

	// ---- assign addresses ----
	// segment 1 (R): headers, .interp, .hash, .dynsym, .dynstr, .rela.dyn, .rodata
	// segment 2 (RX): .text, .plt
	// segment 3 (RW): .tdata, .tbss, init/fini arrays, .dynamic, .got, .data, .bss
	nPhdr := 9
	hdrSize := int64(64 + 56*nPhdr)
	pos := hdrSize // file offset == vaddr - baseAddr

	add := func(o outSec) *outSec {
		p := &o
		l.out = append(l.out, p)
		return p
	}
	place := func(o *outSec) {
		pos = alignUp(pos, o.align)
		o.off = pos
		o.addr = baseAddr + pos
		if o.typ != shtNobits {
			pos += o.size
		}
	}
	placeInputs := func(o *outSec) {
		o.size = 0
		for _, s := range o.secs {
			al := s.Align
			if al < 1 {
				al = 1
			}
			if al > o.align {
				o.align = al
			}
		}
		pos = alignUp(pos, o.align)
		o.off = pos
		o.addr = baseAddr + pos
		cur := int64(0)
		for _, s := range o.secs {
			al := s.Align
			if al < 1 {
				al = 1
			}
			cur = alignUp(cur, al)
			s.Addr = o.addr + cur
			cur += s.Len()
		}
		o.size = cur
		if o.typ != shtNobits {
			pos += cur
		}
	}
	// precompute the dynamic relocation count to size .rela.dyn
	l.out = nil
	interp := add(outSec{name: ".interp", align: 1, flags: shfAlloc, typ: shtProgbits, data: append([]byte(l.Interp), 0)})
	interp.size = int64(len(interp.data))
	place(interp)
	hashSec := add(outSec{name: ".hash", align: 8, flags: shfAlloc, typ: shtHash, data: hash, entsz: 4})
	hashSec.size = int64(len(hash))
	place(hashSec)
	dynsymSec := add(outSec{name: ".dynsym", align: 8, flags: shfAlloc, typ: shtDynsym, entsz: 24, info: 1})
	dynsymSec.size = int64(24 * nsyms)
	place(dynsymSec)
	dynstrSec := add(outSec{name: ".dynstr", align: 1, flags: shfAlloc, typ: shtStrtab})
	dynstrSec.size = int64(dynstr.buf.Len())
	place(dynstrSec)
	nRela := l.countDynRelocs()
	relaSec := add(outSec{name: ".rela.dyn", align: 8, flags: shfAlloc, typ: shtRela, entsz: 24})
	relaSec.size = int64(24 * nRela)
	place(relaSec)
	ro := add(outSec{name: ".rodata", kind: SecROData, align: 16, flags: shfAlloc, typ: shtProgbits, secs: inputs[SecROData]})
	placeInputs(ro)
	roEnd := pos

	// RX segment
	pos = alignUp(pos, pageSize)
	rxStart := pos
	text := add(outSec{name: ".text", kind: SecText, align: 16, flags: shfAlloc | shfExec, typ: shtProgbits, secs: inputs[SecText]})
	placeInputs(text)
	pltSec := add(outSec{name: ".plt", align: 16, flags: shfAlloc | shfExec, typ: shtProgbits, entsz: 16})
	pltSec.size = int64(l.pltSize)
	place(pltSec)
	rxEnd := pos

	// RW segment
	pos = alignUp(pos, pageSize)
	rwStart := pos
	var tdata, tbss *outSec
	l.tlsAlign = 1
	for _, g := range groups[2:] {
		o := add(outSec{name: g.name, kind: g.kind, align: 1, flags: g.flags, typ: g.typ, secs: inputs[g.kind]})
		if g.kind == SecInitArray || g.kind == SecFiniArray || g.kind == SecPreinitArray {
			o.entsz = 8
		}
		if g.kind == SecData {
			// .dynamic and .got come before .data
			l.out = l.out[:len(l.out)-1]
			dyn := add(outSec{name: ".dynamic", align: 8, flags: shfAlloc | shfWrite, typ: shtDynamic, entsz: 16})
			dyn.size = int64(16 * l.dynamicCount())
			place(dyn)
			got := add(outSec{name: ".got", align: 8, flags: shfAlloc | shfWrite, typ: shtProgbits, entsz: 8})
			got.size = int64(l.gotSize)
			place(got)
			l.out = append(l.out, o)
		}
		if g.kind == SecTBSS {
			// .tbss takes no space in the image but needs its TLS offsets
			for _, s := range o.secs {
				if s.Align > o.align {
					o.align = s.Align
				}
			}
			start := alignUp(pos, o.align)
			o.off = start
			o.addr = baseAddr + start
			cur := int64(0)
			for _, s := range o.secs {
				cur = alignUp(cur, max(s.Align, 1))
				s.Addr = o.addr + cur
				cur += s.Len()
			}
			o.size = cur
			tbss = o
			continue
		}
		placeInputs(o)
		if g.kind == SecTData {
			tdata = o
		}
	}
	rwFileEnd := pos
	bss := l.outByName(".bss")
	rwMemEnd := bss.addr + bss.size
	// TLS block
	if tdata.size > 0 || tbss.size > 0 {
		l.tlsStart = tdata.addr
		l.tlsAlign = max(tdata.align, tbss.align, 1)
		end := tbss.addr + tbss.size
		if tbss.size == 0 {
			end = tdata.addr + tdata.size
		}
		l.tlsEnd = l.tlsStart + alignUp(end-l.tlsStart, l.tlsAlign)
	}
	for _, imp := range l.impList {
		if imp.needCopy {
			imp.copyAddr += l.copyBss.Addr
		}
	}
	// linker-defined symbols
	ia, fa, pa := l.outByName(".init_array"), l.outByName(".fini_array"), l.outByName(".preinit_array")
	gotSec, dynSec := l.outByName(".got"), l.outByName(".dynamic")
	l.sectionSyms = map[string]int64{
		"_GLOBAL_OFFSET_TABLE_": gotSec.addr, "_DYNAMIC": dynSec.addr,
		"__init_array_start": ia.addr, "__init_array_end": ia.addr + ia.size,
		"__fini_array_start": fa.addr, "__fini_array_end": fa.addr + fa.size,
		"__preinit_array_start": pa.addr, "__preinit_array_end": pa.addr + pa.size,
		"_end": rwMemEnd, "end": rwMemEnd, "_edata": baseAddr + rwFileEnd, "edata": baseAddr + rwFileEnd,
		"__bss_start": bss.addr, "_etext": baseAddr + rxEnd, "etext": baseAddr + rxEnd,
		"__executable_start": baseAddr, "__ehdr_start": baseAddr,
	}

	// ---- build the image ----
	img := make([]byte, rwFileEnd)
	for _, o := range l.out {
		if o.typ == shtNobits {
			continue
		}
		for _, s := range o.secs {
			copy(img[s.Addr-baseAddr:], s.Data)
		}
	}
	// .plt stubs: jmp *GOT(%rip); padding
	for s, off := range l.plt {
		at := pltSec.off + int64(off)
		p := img[at:]
		p[0], p[1] = 0xff, 0x25
		gotAddr := gotSec.addr + int64(l.got[s])
		le.PutUint32(p[2:], uint32(gotAddr-(pltSec.addr+int64(off)+6)))
		for i := 6; i < 16; i++ {
			p[i] = 0xcc
		}
	}
	// GOT contents
	for s, off := range l.got {
		at := gotSec.addr + int64(off)
		if imp := l.imp(s); imp != nil && imp.copyAddr == 0 {
			l.dynRelocs = append(l.dynRelocs, dynReloc{off: at, typ: R_X86_64_GLOB_DAT, imp: imp})
			continue
		}
		if s.IFunc && l.imp(s) == nil && s.Section != nil {
			l.dynRelocs = append(l.dynRelocs, dynReloc{off: at, typ: R_X86_64_IRELATIVE, addend: s.Section.Addr + s.Value})
			continue
		}
		le.PutUint64(img[at-baseAddr:], uint64(l.symAddr(s)))
	}
	for s, off := range l.gotTP {
		at := gotSec.addr + int64(off)
		if imp := l.imp(s); imp != nil {
			l.dynRelocs = append(l.dynRelocs, dynReloc{off: at, typ: R_X86_64_TPOFF64, imp: imp})
			continue
		}
		le.PutUint64(img[at-baseAddr:], uint64(l.symAddr(s)-l.tlsEnd))
	}
	// copy relocations
	for _, imp := range l.impList {
		if imp.needCopy {
			l.dynRelocs = append(l.dynRelocs, dynReloc{off: imp.copyAddr, typ: R_X86_64_COPY, imp: imp})
		}
	}
	// apply relocations
	for _, o := range l.Objects {
		for _, sec := range o.Sections {
			if sec.Kind == SecOther || sec.Kind == SecBSS || sec.Kind == SecTBSS {
				continue
			}
			for _, r := range sec.Relocs {
				l.applyReloc(img, o, sec, r)
			}
		}
	}
	if len(l.dynRelocs) != nRela {
		l.errorf("internal error: %d dynamic relocations, %d reserved", len(l.dynRelocs), nRela)
		return nil
	}

	// .dynsym
	symtabData := dynsym.Bytes()
	for _, imp := range l.impList {
		e := make([]byte, 24)
		le.PutUint32(e[0:], uint32(dynstr.add(imp.sym.Name)))
		typ := byte(0) // STT_NOTYPE
		switch imp.dyn.Type {
		case SymFunc:
			typ = 2
		case SymObject:
			typ = 1
		case SymTLS:
			typ = 6
		}
		e[4] = 1<<4 | typ // STB_GLOBAL
		if imp.copyAddr != 0 {
			// the copy in .bss is the definition everybody uses
			le.PutUint16(e[6:], uint16(l.shIndexOf(".bss")))
			le.PutUint64(e[8:], uint64(imp.copyAddr))
			le.PutUint64(e[16:], uint64(imp.dyn.Size))
		}
		symtabData = append(symtabData, e...)
	}
	copy(img[dynsymSec.off:], symtabData)
	copy(img[dynstrSec.off:], dynstr.buf.Bytes())
	copy(img[interp.off:], interp.data)
	copy(img[hashSec.off:], hash)
	// .rela.dyn
	sort.Slice(l.dynRelocs, func(i, j int) bool { return l.dynRelocs[i].off < l.dynRelocs[j].off })
	for i, r := range l.dynRelocs {
		e := img[relaSec.off+int64(24*i):]
		le.PutUint64(e[0:], uint64(r.off))
		si := 0
		if r.imp != nil {
			si = r.imp.dynIndex
		}
		le.PutUint64(e[8:], uint64(si)<<32|uint64(r.typ))
		le.PutUint64(e[16:], uint64(r.addend))
	}
	// .dynamic
	var dyn []uint64
	for _, lib := range l.needed {
		dyn = append(dyn, 1, uint64(dynstr.add(lib.Soname))) // DT_NEEDED
	}
	dyn = append(dyn,
		4, uint64(hashSec.addr), // DT_HASH
		5, uint64(dynstrSec.addr), // DT_STRTAB
		6, uint64(dynsymSec.addr), // DT_SYMTAB
		10, uint64(dynstr.buf.Len()), // DT_STRSZ
		11, 24, // DT_SYMENT
		7, uint64(relaSec.addr), // DT_RELA
		8, uint64(relaSec.size), // DT_RELASZ
		9, 24, // DT_RELAENT
	)
	if ia.size > 0 {
		dyn = append(dyn, 25, uint64(ia.addr), 27, uint64(ia.size))
	}
	if fa.size > 0 {
		dyn = append(dyn, 26, uint64(fa.addr), 28, uint64(fa.size))
	}
	if pa.size > 0 {
		dyn = append(dyn, 32, uint64(pa.addr), 33, uint64(pa.size))
	}
	dyn = append(dyn,
		30, 8, // DT_FLAGS = DF_BIND_NOW
		0x6ffffffb, 1, // DT_FLAGS_1 = DF_1_NOW
		21, 0, // DT_DEBUG
		0, 0) // DT_NULL
	for i, v := range dyn {
		le.PutUint64(img[dynSec.off+int64(8*i):], v)
	}

	// ---- program headers ----
	entry := l.globals[l.Entry]
	if entry == nil {
		l.errorf("entry symbol %s not found", l.Entry)
		return nil
	}
	type phdr struct {
		typ, flags                       uint32
		off, vaddr, filesz, memsz, align int64
	}
	phdrs := []phdr{
		{ptPhdr, 4, 64, baseAddr + 64, 56 * int64(nPhdr), 56 * int64(nPhdr), 8},
		{ptInterp, 4, interp.off, interp.addr, interp.size, interp.size, 1},
		{ptLoad, 4, 0, baseAddr, roEnd, roEnd, pageSize},
		{ptLoad, 5, rxStart, baseAddr + rxStart, rxEnd - rxStart, rxEnd - rxStart, pageSize},
		{ptLoad, 6, rwStart, baseAddr + rwStart, rwFileEnd - rwStart, rwMemEnd - (baseAddr + rwStart), pageSize},
		{ptDynamic, 6, dynSec.off, dynSec.addr, dynSec.size, dynSec.size, 8},
		{ptGnuStack, 6, 0, 0, 0, 0, 16},
	}
	if l.tlsEnd != 0 {
		phdrs = append(phdrs, phdr{ptTLS, 4, tdata.off, tdata.addr, tdata.size, l.tlsEnd - l.tlsStart, l.tlsAlign})
	}
	for len(phdrs) < nPhdr {
		phdrs = append(phdrs, phdr{typ: 0}) // PT_NULL
	}
	for i, p := range phdrs {
		e := img[64+56*i:]
		le.PutUint32(e[0:], p.typ)
		le.PutUint32(e[4:], p.flags)
		le.PutUint64(e[8:], uint64(p.off))
		le.PutUint64(e[16:], uint64(p.vaddr))
		le.PutUint64(e[24:], uint64(p.vaddr))
		le.PutUint64(e[32:], uint64(p.filesz))
		le.PutUint64(e[40:], uint64(p.memsz))
		le.PutUint64(e[48:], uint64(p.align))
	}

	// ---- section headers, .symtab ----
	shstr := newStrtab()
	type shdr struct {
		name            int
		typ             uint32
		flags           uint64
		addr, off, size int64
		link, info      uint32
		align, entsz    int64
	}
	shdrs := []shdr{{}}
	secIndex := map[string]int{}
	for _, o := range l.out {
		if o.size == 0 && o.name != ".bss" && o.name != ".dynamic" && o.name != ".dynsym" && o.name != ".dynstr" {
			continue
		}
		secIndex[o.name] = len(shdrs)
		shdrs = append(shdrs, shdr{name: shstr.add(o.name), typ: o.typ, flags: o.flags, addr: o.addr, off: o.off,
			size: o.size, align: max(o.align, 1), entsz: o.entsz})
	}
	fix := func(name, link string, info int) {
		if i, ok := secIndex[name]; ok {
			shdrs[i].link = uint32(secIndex[link])
			if info >= 0 {
				shdrs[i].info = uint32(info)
			}
		}
	}
	fix(".dynsym", ".dynstr", 1)
	fix(".hash", ".dynsym", -1)
	fix(".rela.dyn", ".dynsym", -1)
	fix(".dynamic", ".dynstr", -1)
	l.shIndex = secIndex
	// patch copy-relocation symbol section indexes now that they are known
	if bi, ok := secIndex[".bss"]; ok {
		for _, imp := range l.impList {
			if imp.copyAddr != 0 {
				le.PutUint16(img[dynsymSec.off+int64(24*imp.dynIndex)+6:], uint16(bi))
			}
		}
	}
	out := img
	if !l.StripSyms {
		strt := newStrtab()
		var syms bytes.Buffer
		syms.Write(make([]byte, 24))
		nlocal := 1
		writeSym := func(s *Symbol, local bool) {
			e := make([]byte, 24)
			le.PutUint32(e[0:], uint32(strt.add(s.Name)))
			typ := byte(0)
			switch s.Type {
			case SymFunc:
				typ = 2
			case SymObject:
				typ = 1
			case SymTLS:
				typ = 6
			}
			bind := byte(1)
			if local {
				bind = 0
			} else if s.Bind == BindWeak {
				bind = 2
			}
			e[4] = bind<<4 | typ
			shn := 0xfff1 // SHN_ABS
			val := l.symAddr(s)
			if s.Section != nil {
				shn = l.shIndexForKind(s.Section.Kind)
				if s.Type == SymTLS {
					val = s.Section.Addr + s.Value - l.tlsStart
				}
			}
			le.PutUint16(e[6:], uint16(shn))
			le.PutUint64(e[8:], uint64(val))
			le.PutUint64(e[16:], uint64(s.Size))
			syms.Write(e)
		}
		for _, o := range l.Objects {
			for _, s := range o.Symbols {
				if s.Bind == BindLocal && s.Section != nil && s.Name != "" && s.Type != SymSection && s.Type != SymFile &&
					!strings.HasPrefix(s.Name, ".L") {
					writeSym(s, true)
					nlocal++
				}
			}
		}
		var gnames []string
		for n, s := range l.globals {
			if l.imp(s) == nil && s.Section != nil {
				gnames = append(gnames, n)
			}
		}
		sort.Strings(gnames)
		for _, n := range gnames {
			writeSym(l.globals[n], false)
		}
		off := alignUp(int64(len(out)), 8)
		out = append(out, make([]byte, off-int64(len(out)))...)
		symIdx := len(shdrs)
		shdrs = append(shdrs, shdr{name: shstr.add(".symtab"), typ: shtSymtab, off: off, size: int64(syms.Len()),
			link: uint32(symIdx + 1), info: uint32(nlocal), align: 8, entsz: 24})
		out = append(out, syms.Bytes()...)
		shdrs = append(shdrs, shdr{name: shstr.add(".strtab"), typ: shtStrtab, off: int64(len(out)), size: int64(strt.buf.Len()), align: 1})
		out = append(out, strt.buf.Bytes()...)
	}
	shstrIdx := len(shdrs)
	shstrName := shstr.add(".shstrtab")
	shdrs = append(shdrs, shdr{name: shstrName, typ: shtStrtab, off: int64(len(out)), size: 0, align: 1})
	out = append(out, shstr.buf.Bytes()...)
	shdrs[shstrIdx].size = int64(shstr.buf.Len())
	shoff := alignUp(int64(len(out)), 8)
	out = append(out, make([]byte, shoff-int64(len(out)))...)
	for _, s := range shdrs {
		e := make([]byte, 64)
		le.PutUint32(e[0:], uint32(s.name))
		le.PutUint32(e[4:], s.typ)
		le.PutUint64(e[8:], s.flags)
		le.PutUint64(e[16:], uint64(s.addr))
		le.PutUint64(e[24:], uint64(s.off))
		le.PutUint64(e[32:], uint64(s.size))
		le.PutUint32(e[40:], s.link)
		le.PutUint32(e[44:], s.info)
		le.PutUint64(e[48:], uint64(s.align))
		le.PutUint64(e[56:], uint64(s.entsz))
		out = append(out, e...)
	}

	// ---- ELF header ----
	h := out[:64]
	copy(h, []byte{0x7f, 'E', 'L', 'F', 2, 1, 1, 0})
	le.PutUint16(h[16:], 2)  // ET_EXEC
	le.PutUint16(h[18:], 62) // EM_X86_64
	le.PutUint32(h[20:], 1)
	le.PutUint64(h[24:], uint64(l.symAddr(entry)))
	le.PutUint64(h[32:], 64)
	le.PutUint64(h[40:], uint64(shoff))
	le.PutUint16(h[52:], 64)
	le.PutUint16(h[54:], 56)
	le.PutUint16(h[56:], uint16(nPhdr))
	le.PutUint16(h[58:], 64)
	le.PutUint16(h[60:], uint16(len(shdrs)))
	le.PutUint16(h[62:], uint16(shstrIdx))
	return out
}

// initPriority extracts NNNNN from .init_array.NNNNN (.ctors are not used)
func initPriority(name string) int {
	if i := strings.LastIndexByte(name, '.'); i > 0 {
		n := 0
		for _, c := range name[i+1:] {
			if c < '0' || c > '9' {
				return 65536
			}
			n = n*10 + int(c-'0')
		}
		if i+1 < len(name) {
			return n
		}
	}
	return 65536
}

func (l *Linker) shIndexOf(name string) int {
	if l.shIndex == nil {
		return 0
	}
	return l.shIndex[name]
}

func (l *Linker) shIndexForKind(k SectionKind) int {
	names := map[SectionKind]string{SecText: ".text", SecROData: ".rodata", SecData: ".data", SecBSS: ".bss",
		SecTData: ".tdata", SecTBSS: ".tbss", SecInitArray: ".init_array", SecFiniArray: ".fini_array", SecPreinitArray: ".preinit_array"}
	return l.shIndexOf(names[k])
}

// dynamicCount is the number of entries in .dynamic
func (l *Linker) dynamicCount() int {
	// needed + fixed entries + flags/flags_1/debug/null + room for the
	// init/fini/preinit arrays (unused slots stay DT_NULL)
	return len(l.needed) + 8 + 4 + 6
}

// countDynRelocs predicts the number of dynamic relocations
func (l *Linker) countDynRelocs() int {
	n := 0
	for s := range l.got {
		if imp := l.imp(s); imp != nil && !imp.needCopy || imp == nil && s.IFunc && s.Section != nil {
			n++
		}
	}
	for s := range l.gotTP {
		if l.imp(s) != nil {
			n++
		}
	}
	for _, imp := range l.impList {
		if imp.needCopy {
			n++
		}
	}
	for _, o := range l.Objects {
		for _, sec := range o.Sections {
			if sec.Kind == SecOther {
				continue
			}
			for _, r := range sec.Relocs {
				if r.Type == R_X86_64_64 {
					if imp := l.imp(l.resolve(r.Sym)); imp != nil && !imp.needCopy && !imp.needPLTOnly(sec) {
						n++
					}
				}
			}
		}
	}
	return n
}

// needPLTOnly tells whether an absolute reference from a read-only section
// is satisfied by the PLT address instead of a dynamic relocation
func (imp *imported) needPLTOnly(sec *Section) bool {
	writable := sec.Kind == SecData || sec.Kind == SecTData || sec.Kind == SecInitArray ||
		sec.Kind == SecFiniArray || sec.Kind == SecPreinitArray
	return !writable && imp.dyn.Type == SymFunc
}

func (l *Linker) applyReloc(img []byte, o *Object, sec *Section, r Reloc) {
	le := binary.LittleEndian
	s := l.resolve(r.Sym)
	P := sec.Addr + r.Offset
	at := P - baseAddr
	A := r.Addend
	imp := l.imp(s)
	var S int64
	if s.Type == SymSection && s.Section != nil {
		S = s.Section.Addr
	} else {
		S = l.symAddr(s)
	}
	gotAddr := l.secAddr(".got")
	put32 := func(v int64, signed bool) {
		if signed && (v < -(1<<31) || v > (1<<31)-1) || !signed && (v < 0 || v > 0xFFFFFFFF) {
			l.errorf("%s: relocation %d against `%s' out of range", o.Name, r.Type, s.Name)
			return
		}
		le.PutUint32(img[at:], uint32(v))
	}
	switch r.Type {
	case R_X86_64_64:
		if imp != nil && imp.copyAddr == 0 && !imp.needPLTOnly(sec) {
			l.dynRelocs = append(l.dynRelocs, dynReloc{off: P, typ: R_X86_64_64, imp: imp, addend: A})
			le.PutUint64(img[at:], 0)
			return
		}
		le.PutUint64(img[at:], uint64(S+A))
	case R_X86_64_PC32, R_X86_64_PLT32:
		put32(S+A-P, true)
	case R_X86_64_PC64:
		le.PutUint64(img[at:], uint64(S+A-P))
	case R_X86_64_32:
		put32(S+A, false)
	case R_X86_64_32S:
		put32(S+A, true)
	case R_X86_64_16:
		le.PutUint16(img[at:], uint16(S+A))
	case R_X86_64_PC16:
		le.PutUint16(img[at:], uint16(S+A-P))
	case R_X86_64_8:
		img[at] = byte(S + A)
	case R_X86_64_PC8:
		img[at] = byte(S + A - P)
	case R_X86_64_GOTPCREL, R_X86_64_GOTPCRELX, R_X86_64_REX_GOTPCRELX:
		put32(gotAddr+int64(l.got[s])+A-P, true)
	case R_X86_64_GOT32:
		put32(int64(l.got[s])+A, true)
	case R_X86_64_GOTPC32:
		put32(gotAddr+A-P, true)
	case R_X86_64_GOTOFF64:
		le.PutUint64(img[at:], uint64(S+A-gotAddr))
	case R_X86_64_GOTTPOFF:
		put32(gotAddr+int64(l.gotTP[s])+A-P, true)
	case R_X86_64_TPOFF32:
		put32(S+A-l.tlsEnd, true)
	case R_X86_64_TPOFF64:
		le.PutUint64(img[at:], uint64(S+A-l.tlsEnd))
	case R_X86_64_DTPOFF64:
		le.PutUint64(img[at:], uint64(S+A-l.tlsStart))
	case R_X86_64_SIZE32:
		put32(s.Size+A, false)
	case R_X86_64_SIZE64:
		le.PutUint64(img[at:], uint64(s.Size+A))
	default:
		l.errorf("%s: unsupported relocation type %d against `%s'", o.Name, r.Type, s.Name)
	}
}
