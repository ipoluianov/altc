package elf

import (
	"bytes"
	goelf "debug/elf"
	"encoding/binary"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

// sectionKindOf maps an input ELF section to where it goes in the output
func sectionKindOf(s *goelf.Section) SectionKind {
	if s.Flags&goelf.SHF_ALLOC == 0 {
		return SecOther
	}
	switch s.Type {
	case goelf.SHT_INIT_ARRAY:
		return SecInitArray
	case goelf.SHT_FINI_ARRAY:
		return SecFiniArray
	case goelf.SHT_PREINIT_ARRAY:
		return SecPreinitArray
	case goelf.SHT_NOTE:
		return SecOther
	case goelf.SectionType(0x70000001):
		return SecOther // .eh_frame: no unwinding support needed for C
	}
	if s.Name == ".eh_frame" {
		return SecOther
	}
	if s.Flags&goelf.SHF_TLS != 0 {
		if s.Type == goelf.SHT_NOBITS {
			return SecTBSS
		}
		return SecTData
	}
	if s.Type == goelf.SHT_NOBITS {
		return SecBSS
	}
	if s.Flags&goelf.SHF_EXECINSTR != 0 {
		return SecText
	}
	if s.Flags&goelf.SHF_WRITE != 0 {
		return SecData
	}
	return SecROData
}

// ReadObject loads an ELF relocatable file (.o)
func ReadObject(name string, data []byte) (*Object, error) {
	f, err := goelf.NewFile(bytes.NewReader(data))
	if err != nil {
		return nil, fmt.Errorf("%s: %v", name, err)
	}
	defer f.Close()
	if f.Type != goelf.ET_REL {
		return nil, fmt.Errorf("%s: not a relocatable object", name)
	}
	if f.Machine != goelf.EM_X86_64 || f.Class != goelf.ELFCLASS64 {
		return nil, fmt.Errorf("%s: not an x86-64 object", name)
	}
	obj := NewObject(name)
	secs := make([]*Section, len(f.Sections))
	for i, s := range f.Sections {
		if s.Type != goelf.SHT_PROGBITS && s.Type != goelf.SHT_NOBITS && s.Type != goelf.SHT_INIT_ARRAY &&
			s.Type != goelf.SHT_FINI_ARRAY && s.Type != goelf.SHT_PREINIT_ARRAY && s.Type != goelf.SectionType(0x70000001) {
			continue
		}
		kind := sectionKindOf(s)
		if kind == SecOther {
			continue
		}
		sec := obj.AddSection(s.Name, kind, int64(s.Addralign))
		if sec.Align == 0 {
			sec.Align = 1
		}
		if s.Type == goelf.SHT_NOBITS {
			sec.Size = int64(s.Size)
		} else {
			d, err := s.Data()
			if err != nil {
				return nil, fmt.Errorf("%s: section %s: %v", name, s.Name, err)
			}
			sec.Data = append([]byte(nil), d...)
			sec.Size = int64(len(d))
		}
		secs[i] = sec
	}
	// symbols (index 0 is the null symbol, which debug/elf drops)
	esyms, err := f.Symbols()
	if err != nil && err != goelf.ErrNoSymbols {
		return nil, fmt.Errorf("%s: %v", name, err)
	}
	syms := make([]*Symbol, len(esyms)+1)
	for i, es := range esyms {
		s := &Symbol{Name: es.Name, Value: int64(es.Value), Size: int64(es.Size)}
		switch goelf.ST_BIND(es.Info) {
		case goelf.STB_GLOBAL:
			s.Bind = BindGlobal
		case goelf.STB_WEAK:
			s.Bind = BindWeak
		default:
			s.Bind = BindLocal
		}
		switch goelf.ST_TYPE(es.Info) {
		case goelf.STT_FUNC, goelf.STT_GNU_IFUNC:
			s.Type = SymFunc
		case goelf.STT_OBJECT:
			s.Type = SymObject
		case goelf.STT_TLS:
			s.Type = SymTLS
		case goelf.STT_SECTION:
			s.Type = SymSection
		case goelf.STT_FILE:
			s.Type = SymFile
		}
		vis := goelf.ST_VISIBILITY(es.Other)
		s.Hidden = vis == goelf.STV_HIDDEN || vis == goelf.STV_INTERNAL
		switch es.Section {
		case goelf.SHN_UNDEF:
		case goelf.SHN_ABS:
			s.Abs = true
		case goelf.SHN_COMMON:
			s.Type = SymCommon
			s.Value = int64(es.Value) // alignment
		default:
			if int(es.Section) < len(secs) {
				s.Section = secs[es.Section]
			}
			if s.Section == nil && s.Bind != BindLocal {
				// defined in a section we dropped (e.g. debug info)
				s.Abs = true
			}
		}
		if s.Bind == BindLocal {
			obj.AddLocal(s)
		} else {
			if old := obj.Lookup(s.Name); old != nil {
				*old = *s
				s = old
			} else {
				obj.symMap[s.Name] = s
				obj.Symbols = append(obj.Symbols, s)
			}
		}
		syms[i+1] = s
	}
	// relocations
	for _, rs := range f.Sections {
		if rs.Type != goelf.SHT_RELA {
			if rs.Type == goelf.SHT_REL && int(rs.Info) < len(secs) && secs[rs.Info] != nil {
				return nil, fmt.Errorf("%s: REL relocations are not supported", name)
			}
			continue
		}
		if int(rs.Info) >= len(secs) || secs[rs.Info] == nil {
			continue
		}
		target := secs[rs.Info]
		d, err := rs.Data()
		if err != nil {
			return nil, err
		}
		for off := 0; off+24 <= len(d); off += 24 {
			roff := binary.LittleEndian.Uint64(d[off:])
			info := binary.LittleEndian.Uint64(d[off+8:])
			add := int64(binary.LittleEndian.Uint64(d[off+16:]))
			si := int(info >> 32)
			typ := uint32(info)
			if typ == R_X86_64_NONE {
				continue
			}
			var sym *Symbol
			if si > 0 && si < len(syms) {
				sym = syms[si]
			}
			if sym == nil {
				return nil, fmt.Errorf("%s: bad relocation symbol %d", name, si)
			}
			if sym.Type == SymSection && sym.Section == nil {
				continue // relocation against a dropped section
			}
			target.Relocs = append(target.Relocs, Reloc{Offset: int64(roff), Type: typ, Sym: sym, Addend: add})
		}
	}
	return obj, nil
}

// Archive is a static library (.a) whose members are loaded on demand
type Archive struct {
	Name    string
	data    []byte
	index   map[string]int64 // symbol -> member header offset
	members []int64          // member header offsets (when no index)
	loaded  map[int64]bool
	strtab  []byte
}

// ReadArchive parses an ar archive
func ReadArchive(name string, data []byte) (*Archive, error) {
	if !bytes.HasPrefix(data, []byte("!<arch>\n")) {
		return nil, fmt.Errorf("%s: not an archive", name)
	}
	ar := &Archive{Name: name, data: data, index: map[string]int64{}, loaded: map[int64]bool{}}
	pos := int64(8)
	for pos+60 <= int64(len(data)) {
		hdr := data[pos : pos+60]
		mname := strings.TrimRight(string(hdr[0:16]), " ")
		size, err := strconv.ParseInt(strings.TrimSpace(string(hdr[48:58])), 10, 64)
		if err != nil {
			return nil, fmt.Errorf("%s: bad member header", name)
		}
		body := pos + 60
		if body+size > int64(len(data)) {
			return nil, fmt.Errorf("%s: truncated", name)
		}
		switch mname {
		case "/", "/SYM64/":
			b := data[body : body+size]
			w := 4
			if mname == "/SYM64/" {
				w = 8
			}
			if len(b) >= w {
				var n int
				if w == 4 {
					n = int(binary.BigEndian.Uint32(b))
				} else {
					n = int(binary.BigEndian.Uint64(b))
				}
				offs := b[w:]
				names := b[w+n*w:]
				for i := 0; i < n && (i+1)*w <= len(offs); i++ {
					var o int64
					if w == 4 {
						o = int64(binary.BigEndian.Uint32(offs[i*w:]))
					} else {
						o = int64(binary.BigEndian.Uint64(offs[i*w:]))
					}
					end := bytes.IndexByte(names, 0)
					if end < 0 {
						break
					}
					s := string(names[:end])
					names = names[end+1:]
					if _, ok := ar.index[s]; !ok {
						ar.index[s] = o
					}
				}
			}
		case "//":
			ar.strtab = data[body : body+size]
		default:
			ar.members = append(ar.members, pos)
		}
		pos = body + size
		if pos%2 == 1 {
			pos++
		}
	}
	if len(ar.index) == 0 {
		// no symbol index: build one from the members
		for _, m := range ar.members {
			obj, err := ar.member(m)
			if err != nil || obj == nil {
				continue
			}
			for _, s := range obj.Symbols {
				if s.Bind != BindLocal && s.Defined() {
					if _, ok := ar.index[s.Name]; !ok {
						ar.index[s.Name] = m
					}
				}
			}
		}
	}
	return ar, nil
}

func (ar *Archive) memberName(hdr []byte) string {
	n := strings.TrimRight(string(hdr[0:16]), " ")
	if strings.HasPrefix(n, "/") && len(n) > 1 && ar.strtab != nil {
		if off, err := strconv.Atoi(n[1:]); err == nil && off < len(ar.strtab) {
			s := ar.strtab[off:]
			if e := bytes.Index(s, []byte("/\n")); e >= 0 {
				return string(s[:e])
			}
		}
	}
	return strings.TrimSuffix(n, "/")
}

func (ar *Archive) member(pos int64) (*Object, error) {
	hdr := ar.data[pos : pos+60]
	size, _ := strconv.ParseInt(strings.TrimSpace(string(hdr[48:58])), 10, 64)
	body := ar.data[pos+60 : pos+60+size]
	if !bytes.HasPrefix(body, []byte("\x7fELF")) {
		return nil, nil
	}
	return ReadObject(ar.Name+"("+ar.memberName(hdr)+")", body)
}

// Pull returns the member that defines sym when it was not loaded yet
func (ar *Archive) Pull(sym string) (*Object, error) {
	pos, ok := ar.index[sym]
	if !ok || ar.loaded[pos] {
		return nil, nil
	}
	ar.loaded[pos] = true
	return ar.member(pos)
}

// DynSym is a symbol exported by a shared library
type DynSym struct {
	Name string
	Type SymType
	Size int64
}

// SharedLib is a shared library the output links against
type SharedLib struct {
	Path   string
	Soname string
	Syms   map[string]*DynSym
}

// ReadSharedLib loads the exported symbols of a .so file
func ReadSharedLib(path string) (*SharedLib, error) {
	f, err := goelf.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	if f.Type != goelf.ET_DYN || f.Machine != goelf.EM_X86_64 {
		return nil, fmt.Errorf("%s: not an x86-64 shared library", path)
	}
	lib := &SharedLib{Path: path, Soname: filepath.Base(path), Syms: map[string]*DynSym{}}
	if names, err := f.DynString(goelf.DT_SONAME); err == nil && len(names) > 0 {
		lib.Soname = names[0]
	}
	syms, err := f.DynamicSymbols()
	if err != nil {
		return nil, fmt.Errorf("%s: %v", path, err)
	}
	for _, s := range syms {
		if s.Section == goelf.SHN_UNDEF {
			continue
		}
		bind := goelf.ST_BIND(s.Info)
		if bind != goelf.STB_GLOBAL && bind != goelf.STB_WEAK {
			continue
		}
		t := SymNoType
		switch goelf.ST_TYPE(s.Info) {
		case goelf.STT_FUNC, goelf.STT_GNU_IFUNC:
			t = SymFunc
		case goelf.STT_OBJECT:
			t = SymObject
		case goelf.STT_TLS:
			t = SymTLS
		}
		if old, ok := lib.Syms[s.Name]; ok && old.Size >= int64(s.Size) {
			continue
		}
		lib.Syms[s.Name] = &DynSym{Name: s.Name, Type: t, Size: int64(s.Size)}
	}
	return lib, nil
}

// IsLinkerScript tells whether a .so path is a GNU ld text script (like libc.so)
func IsLinkerScript(path string) bool {
	f, err := os.Open(path)
	if err != nil {
		return false
	}
	defer f.Close()
	var b [4]byte
	if _, err := io.ReadFull(f, b[:]); err != nil {
		return true
	}
	return string(b[:]) != "\x7fELF"
}

// ParseLinkerScript extracts the file names from GROUP/INPUT commands of a
// GNU ld script (AS_NEEDED entries are skipped)
func ParseLinkerScript(path string) ([]string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	// remove comments
	s := string(data)
	for {
		i := strings.Index(s, "/*")
		if i < 0 {
			break
		}
		j := strings.Index(s[i:], "*/")
		if j < 0 {
			s = s[:i]
			break
		}
		s = s[:i] + " " + s[i+j+2:]
	}
	s = strings.NewReplacer("(", " ( ", ")", " ) ", ",", " ").Replace(s)
	toks := strings.Fields(s)
	var files []string
	depth := 0
	inGroup := false
	asNeeded := -1
	for i := 0; i < len(toks); i++ {
		t := toks[i]
		switch {
		case (t == "GROUP" || t == "INPUT") && depth == 0:
			inGroup = true
		case t == "AS_NEEDED":
			asNeeded = depth + 1
		case t == "(":
			depth++
		case t == ")":
			if depth == asNeeded {
				asNeeded = -1
			}
			depth--
			if depth == 0 {
				inGroup = false
			}
		default:
			if inGroup && depth > 0 && asNeeded < 0 {
				files = append(files, t)
			}
		}
	}
	return files, nil
}
