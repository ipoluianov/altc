// Package elf holds the object-file model shared by the code generator and
// the linker, reads and writes ELF relocatable files, archives and shared
// libraries, and links executables for linux/amd64.
package elf

// Relocation types of x86-64 (System V ABI)
const (
	R_X86_64_NONE            = 0
	R_X86_64_64              = 1
	R_X86_64_PC32            = 2
	R_X86_64_GOT32           = 3
	R_X86_64_PLT32           = 4
	R_X86_64_COPY            = 5
	R_X86_64_GLOB_DAT        = 6
	R_X86_64_JUMP_SLOT       = 7
	R_X86_64_RELATIVE        = 8
	R_X86_64_GOTPCREL        = 9
	R_X86_64_32              = 10
	R_X86_64_32S             = 11
	R_X86_64_16              = 12
	R_X86_64_PC16            = 13
	R_X86_64_8               = 14
	R_X86_64_PC8             = 15
	R_X86_64_DTPMOD64        = 16
	R_X86_64_DTPOFF64        = 17
	R_X86_64_TPOFF64         = 18
	R_X86_64_TLSGD           = 19
	R_X86_64_TLSLD           = 20
	R_X86_64_DTPOFF32        = 21
	R_X86_64_GOTTPOFF        = 22
	R_X86_64_TPOFF32         = 23
	R_X86_64_PC64            = 24
	R_X86_64_GOTOFF64        = 25
	R_X86_64_GOTPC32         = 26
	R_X86_64_SIZE32          = 32
	R_X86_64_SIZE64          = 33
	R_X86_64_GOTPC32_TLSDESC = 34
	R_X86_64_TLSDESC_CALL    = 35
	R_X86_64_TLSDESC         = 36
	R_X86_64_IRELATIVE       = 37
	R_X86_64_GOTPCRELX       = 41
	R_X86_64_REX_GOTPCRELX   = 42
)

// SectionKind tells where the linker places a section
type SectionKind int

const (
	SecText SectionKind = iota
	SecROData
	SecData
	SecBSS
	SecTData
	SecTBSS
	SecInitArray
	SecFiniArray
	SecPreinitArray
	SecOther   // not loaded (comments, notes, debug info...)
	SecEHFrame // .eh_frame: call frame information for the unwinder
)

// Section is a chunk of code or data of one object file
type Section struct {
	Name   string
	Kind   SectionKind
	Align  int64
	Data   []byte // nil for BSS / TBSS
	Size   int64  // size of BSS / TBSS sections (len(Data) otherwise)
	Relocs []Reloc

	// set by the linker
	Addr  int64
	index int
}

// Len is the size of the section in memory
func (s *Section) Len() int64 {
	if s.Kind == SecBSS || s.Kind == SecTBSS {
		return s.Size
	}
	return int64(len(s.Data))
}

// Reloc patches Section.Data at Offset with the address of Sym (+Addend)
type Reloc struct {
	Offset int64
	Type   uint32
	Sym    *Symbol
	Addend int64
}

// Symbol binding
type Bind int

const (
	BindLocal Bind = iota
	BindGlobal
	BindWeak
)

// Symbol type
type SymType int

const (
	SymNoType SymType = iota
	SymObject
	SymFunc
	SymSection
	SymFile
	SymCommon
	SymTLS
)

// Symbol is a named location. Section == nil means undefined, except for
// common symbols (Type == SymCommon), whose Value holds the alignment.
type Symbol struct {
	Name    string
	Section *Section
	Value   int64
	Size    int64
	Bind    Bind
	Type    SymType
	Hidden  bool // visibility hidden/internal: never exported
	Abs     bool // absolute symbol (SHN_ABS)
	IFunc   bool // STT_GNU_IFUNC: the value is a resolver returning the address
}

// Defined tells whether the symbol has a definition in its object
func (s *Symbol) Defined() bool {
	return s.Section != nil || s.Abs || s.Type == SymCommon
}

// Object is one compiled translation unit (or a loaded .o file)
type Object struct {
	Name     string
	Sections []*Section
	Symbols  []*Symbol
	symMap   map[string]*Symbol
}

// NewObject creates an empty object
func NewObject(name string) *Object {
	return &Object{Name: name, symMap: map[string]*Symbol{}}
}

// AddSection appends a new section
func (o *Object) AddSection(name string, kind SectionKind, align int64) *Section {
	s := &Section{Name: name, Kind: kind, Align: align}
	o.Sections = append(o.Sections, s)
	return s
}

// Symbol returns the named global symbol, creating it undefined when missing
func (o *Object) Symbol(name string) *Symbol {
	if o.symMap == nil {
		o.symMap = map[string]*Symbol{}
	}
	if s, ok := o.symMap[name]; ok {
		return s
	}
	s := &Symbol{Name: name, Bind: BindGlobal}
	o.symMap[name] = s
	o.Symbols = append(o.Symbols, s)
	return s
}

// Lookup returns the named symbol or nil
func (o *Object) Lookup(name string) *Symbol {
	return o.symMap[name]
}

// AddLocal adds a local symbol that is not entered into the name map
// (several locals may share a name)
func (o *Object) AddLocal(s *Symbol) *Symbol {
	o.Symbols = append(o.Symbols, s)
	return s
}

// PutLE writes a little-endian value of n bytes
func PutLE(b []byte, v uint64, n int) {
	for i := 0; i < n; i++ {
		b[i] = byte(v >> (8 * i))
	}
}

// Clone makes a deep copy of the object. The linker records addresses in
// the sections and symbols it links, so an object linked more than once
// (the compiler runtime, in every link of a process such as the IDE or the
// tests, possibly at the same time) is cloned for each link.
func (o *Object) Clone() *Object {
	c := &Object{Name: o.Name, symMap: map[string]*Symbol{}}
	secs := map[*Section]*Section{}
	for _, s := range o.Sections {
		n := *s
		if s.Data != nil {
			n.Data = append([]byte(nil), s.Data...)
		}
		secs[s] = &n
		c.Sections = append(c.Sections, &n)
	}
	syms := map[*Symbol]*Symbol{}
	for _, s := range o.Symbols {
		n := *s
		if s.Section != nil {
			if ns, ok := secs[s.Section]; ok {
				n.Section = ns
			}
		}
		syms[s] = &n
		c.Symbols = append(c.Symbols, &n)
	}
	for name, s := range o.symMap {
		c.symMap[name] = syms[s]
	}
	for _, s := range c.Sections {
		rs := make([]Reloc, len(s.Relocs))
		for i, r := range s.Relocs {
			if ns, ok := syms[r.Sym]; ok {
				r.Sym = ns
			}
			rs[i] = r
		}
		s.Relocs = rs
	}
	return c
}
