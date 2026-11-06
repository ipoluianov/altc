package elf

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strings"
)

// ArchiveMember is an object file stored in an archive
type ArchiveMember struct {
	Name string // file name in the archive (any length)
	Data []byte // the ELF object
}

// WriteArchive builds a GNU ar archive with a symbol index of the global
// and weak definitions (and common symbols) of the members, as ar rcs does
func WriteArchive(members []ArchiveMember) ([]byte, error) {
	type entry struct {
		name   string
		member int
	}
	var syms []entry
	for i, m := range members {
		obj, err := ReadObject(m.Name, m.Data)
		if err != nil {
			return nil, err
		}
		for _, s := range obj.Symbols {
			if s.Bind != BindLocal && s.Defined() && s.Name != "" {
				syms = append(syms, entry{s.Name, i})
			}
		}
	}
	// long names go to the "//" member, referenced as /offset
	var names bytes.Buffer
	hdrNames := make([]string, len(members))
	for i, m := range members {
		n := m.Name + "/"
		if len(n) > 16 || strings.Contains(m.Name, "/") {
			n = fmt.Sprintf("/%d", names.Len())
			names.WriteString(m.Name + "/\n")
		}
		hdrNames[i] = n
	}
	// the index: count, member offsets, names (big-endian 32 bits)
	var strtab bytes.Buffer
	for _, s := range syms {
		strtab.WriteString(s.name)
		strtab.WriteByte(0)
	}
	indexSize := 4 + 4*len(syms) + strtab.Len()
	pad := func(n int) int { return n + n&1 }
	pos := 8 + 60 + pad(indexSize)
	if names.Len() > 0 {
		pos += 60 + pad(names.Len())
	}
	offsets := make([]int, len(members))
	for i, m := range members {
		offsets[i] = pos
		pos += 60 + pad(len(m.Data))
	}
	if pos > 1<<32-1 {
		return nil, fmt.Errorf("archive too large")
	}
	var out bytes.Buffer
	out.WriteString("!<arch>\n")
	member := func(name string, data []byte) {
		fmt.Fprintf(&out, "%-16s%-12d%-6d%-6d%-8o%-10d`\n", name, 0, 0, 0, 0o644, len(data))
		out.Write(data)
		if len(data)%2 == 1 {
			out.WriteByte('\n')
		}
	}
	index := make([]byte, 4+4*len(syms), indexSize)
	binary.BigEndian.PutUint32(index, uint32(len(syms)))
	for i, s := range syms {
		binary.BigEndian.PutUint32(index[4+4*i:], uint32(offsets[s.member]))
	}
	member("/", append(index, strtab.Bytes()...))
	if names.Len() > 0 {
		member("//", names.Bytes())
	}
	for i, m := range members {
		member(hdrNames[i], m.Data)
	}
	return out.Bytes(), nil
}
