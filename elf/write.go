package elf

import (
	"bytes"
	"encoding/binary"
)

// WriteObject serializes an object as an ELF64 relocatable file (.o)
func WriteObject(o *Object) []byte {
	le := binary.LittleEndian
	shstr := newStrtab()
	strt := newStrtab()

	type shdr struct {
		name         int
		typ          uint32
		flags        uint64
		off, size    int64
		link, info   uint32
		align, entsz int64
		data         []byte
	}
	shdrs := []shdr{{}}
	secIdx := map[*Section]int{}
	for _, s := range o.Sections {
		if s.Len() == 0 && len(s.Relocs) == 0 {
			continue
		}
		h := shdr{name: shstr.add(s.Name), align: max(s.Align, 1)}
		switch s.Kind {
		case SecText:
			h.typ, h.flags = shtProgbits, shfAlloc|shfExec
		case SecROData:
			h.typ, h.flags = shtProgbits, shfAlloc
		case SecData:
			h.typ, h.flags = shtProgbits, shfAlloc|shfWrite
		case SecBSS:
			h.typ, h.flags = shtNobits, shfAlloc|shfWrite
		case SecTData:
			h.typ, h.flags = shtProgbits, shfAlloc|shfWrite|shfTLS
		case SecTBSS:
			h.typ, h.flags = shtNobits, shfAlloc|shfWrite|shfTLS
		case SecInitArray:
			h.typ, h.flags, h.entsz = shtInitArray, shfAlloc|shfWrite, 8
		case SecFiniArray:
			h.typ, h.flags, h.entsz = shtFiniArray, shfAlloc|shfWrite, 8
		case SecPreinitArray:
			h.typ, h.flags, h.entsz = shtPreinit, shfAlloc|shfWrite, 8
		case SecEHFrame:
			h.typ, h.flags = 0x70000001, shfAlloc // SHT_X86_64_UNWIND
		default:
			h.typ = shtProgbits
		}
		h.size = s.Len()
		h.data = s.Data
		secIdx[s] = len(shdrs)
		shdrs = append(shdrs, h)
	}
	// symbols: locals first
	var syms bytes.Buffer
	syms.Write(make([]byte, 24))
	symIdx := map[*Symbol]int{}
	n := 1
	writeSym := func(s *Symbol) {
		e := make([]byte, 24)
		name := s.Name
		le.PutUint32(e[0:], uint32(strt.add(name)))
		typ := byte(0)
		switch s.Type {
		case SymFunc:
			typ = 2
			if s.IFunc {
				typ = 10 // STT_GNU_IFUNC
			}
		case SymObject, SymCommon:
			typ = 1
		case SymTLS:
			typ = 6
		case SymSection:
			typ = 3
		}
		bind := byte(0)
		switch s.Bind {
		case BindGlobal:
			bind = 1
		case BindWeak:
			bind = 2
		}
		e[4] = bind<<4 | typ
		if s.Hidden {
			e[5] = 2 // STV_HIDDEN
		}
		shn := uint16(0)
		switch {
		case s.Type == SymCommon:
			shn = 0xfff2
		case s.Abs:
			shn = 0xfff1
		case s.Section != nil:
			shn = uint16(secIdx[s.Section])
		}
		le.PutUint16(e[6:], shn)
		le.PutUint64(e[8:], uint64(s.Value))
		le.PutUint64(e[16:], uint64(s.Size))
		syms.Write(e)
		symIdx[s] = n
		n++
	}
	for _, s := range o.Symbols {
		if s.Bind == BindLocal {
			writeSym(s)
		}
	}
	nlocal := n
	for _, s := range o.Symbols {
		if s.Bind != BindLocal {
			writeSym(s)
		}
	}
	symtabIdx := len(shdrs) + 0
	// relocation sections
	var relas []shdr
	for _, s := range o.Sections {
		if len(s.Relocs) == 0 || secIdx[s] == 0 {
			continue
		}
		var b bytes.Buffer
		for _, r := range s.Relocs {
			e := make([]byte, 24)
			le.PutUint64(e[0:], uint64(r.Offset))
			si := symIdx[r.Sym]
			le.PutUint64(e[8:], uint64(si)<<32|uint64(r.Type))
			le.PutUint64(e[16:], uint64(r.Addend))
			b.Write(e)
		}
		relas = append(relas, shdr{name: shstr.add(".rela" + s.Name), typ: shtRela, flags: 0x40, size: int64(b.Len()),
			info: uint32(secIdx[s]), align: 8, entsz: 24, data: b.Bytes()})
	}
	symtabIdx = len(shdrs) + len(relas)
	for i := range relas {
		relas[i].link = uint32(symtabIdx)
	}
	shdrs = append(shdrs, relas...)
	shdrs = append(shdrs, shdr{name: shstr.add(".symtab"), typ: shtSymtab, size: int64(syms.Len()), link: uint32(symtabIdx + 1),
		info: uint32(nlocal), align: 8, entsz: 24, data: syms.Bytes()})
	shdrs = append(shdrs, shdr{name: shstr.add(".strtab"), typ: shtStrtab, size: int64(strt.buf.Len()), align: 1, data: strt.buf.Bytes()})
	shdrs = append(shdrs, shdr{name: shstr.add(".note.GNU-stack"), typ: shtProgbits, align: 1})
	shstrIdx := len(shdrs)
	shdrs = append(shdrs, shdr{name: shstr.add(".shstrtab"), typ: shtStrtab, align: 1})
	shdrs[shstrIdx].data = shstr.buf.Bytes()
	shdrs[shstrIdx].size = int64(shstr.buf.Len())

	out := make([]byte, 64)
	for i := range shdrs {
		h := &shdrs[i]
		if h.typ == shtNobits || len(h.data) == 0 {
			h.off = int64(len(out))
			continue
		}
		off := alignTo64(int64(len(out)), h.align)
		out = append(out, make([]byte, off-int64(len(out)))...)
		h.off = off
		out = append(out, h.data...)
	}
	shoff := alignTo64(int64(len(out)), 8)
	out = append(out, make([]byte, shoff-int64(len(out)))...)
	for _, h := range shdrs {
		e := make([]byte, 64)
		le.PutUint32(e[0:], uint32(h.name))
		le.PutUint32(e[4:], h.typ)
		le.PutUint64(e[8:], h.flags)
		le.PutUint64(e[24:], uint64(h.off))
		le.PutUint64(e[32:], uint64(h.size))
		le.PutUint32(e[40:], h.link)
		le.PutUint32(e[44:], h.info)
		le.PutUint64(e[48:], uint64(h.align))
		le.PutUint64(e[56:], uint64(h.entsz))
		out = append(out, e...)
	}
	h := out[:64]
	copy(h, []byte{0x7f, 'E', 'L', 'F', 2, 1, 1, 0})
	le.PutUint16(h[16:], 1) // ET_REL
	le.PutUint16(h[18:], 62)
	le.PutUint32(h[20:], 1)
	le.PutUint64(h[40:], uint64(shoff))
	le.PutUint16(h[52:], 64)
	le.PutUint16(h[58:], 64)
	le.PutUint16(h[60:], uint16(len(shdrs)))
	le.PutUint16(h[62:], uint16(shstrIdx))
	return out
}

func alignTo64(v, a int64) int64 {
	if a <= 1 {
		return v
	}
	return (v + a - 1) / a * a
}
