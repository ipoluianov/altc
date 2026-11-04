package elf

import (
	"encoding/binary"
	"fmt"
	"sort"
)

// The unwinder finds the frame description of an address through the
// PT_GNU_EH_FRAME segment: .eh_frame_hdr, a table of the FDEs sorted by
// the address of their code.

// ehRecords walks the CIEs and FDEs of an .eh_frame section: f gets the
// offset of each record and whether it is an FDE
func ehRecords(data []byte, f func(off int, fde bool) error) error {
	le := binary.LittleEndian
	for off := 0; off+4 <= len(data); {
		n := int(le.Uint32(data[off:]))
		if n == 0 {
			return nil // terminator
		}
		if n == 0xffffffff {
			return fmt.Errorf(".eh_frame: 64-bit DWARF records are not supported")
		}
		if off+4+n > len(data) || n < 4 {
			return fmt.Errorf(".eh_frame: truncated record at %#x", off)
		}
		if err := f(off, le.Uint32(data[off+4:]) != 0); err != nil {
			return err
		}
		off += 4 + n
	}
	return nil
}

// countFDEs counts the frame descriptions of the input sections
func countFDEs(secs []*Section) int {
	n := 0
	for _, s := range secs {
		ehRecords(s.Data, func(_ int, fde bool) error {
			if fde {
				n++
			}
			return nil
		})
	}
	return n
}

func readULEB(b []byte, i int) (uint64, int) {
	var v uint64
	for sh := 0; i < len(b); sh += 7 {
		c := b[i]
		i++
		v |= uint64(c&0x7f) << sh
		if c&0x80 == 0 {
			break
		}
	}
	return v, i
}

// fdeEncoding reads the pointer encoding of the FDEs of the CIE at off
func fdeEncoding(data []byte, off int) (byte, error) {
	i := off + 9 // length, id, version
	if i > len(data) {
		return 0, fmt.Errorf(".eh_frame: bad CIE at %#x", off)
	}
	aug := i
	for i < len(data) && data[i] != 0 {
		i++
	}
	augStr := string(data[aug:i])
	i++
	_, i = readULEB(data, i) // code alignment
	_, i = readULEB(data, i) // data alignment (sleb: only skipped)
	if data[off+8] == 1 {
		i++ // return address register
	} else {
		_, i = readULEB(data, i)
	}
	if len(augStr) == 0 || augStr[0] != 'z' {
		return 0, nil // absolute pointers
	}
	_, i = readULEB(data, i)
	for _, c := range augStr[1:] {
		switch c {
		case 'R':
			return data[i], nil
		case 'P':
			enc := data[i]
			i++
			i += encSize(enc)
		case 'L':
			i++
		case 'S', 'B':
		default:
			return 0, fmt.Errorf(".eh_frame: unknown augmentation '%s'", augStr)
		}
	}
	return 0, nil
}

func encSize(enc byte) int {
	switch enc & 0x0f {
	case 0x02, 0x0a:
		return 2
	case 0x03, 0x0b:
		return 4
	}
	return 8
}

// ehFrameHdr builds .eh_frame_hdr (at hdrAddr) for the laid out and
// relocated .eh_frame input sections in img
func ehFrameHdr(img []byte, hdrAddr int64, secs []*Section) ([]byte, error) {
	le := binary.LittleEndian
	type entry struct{ pc, fde int64 }
	var table []entry
	for _, s := range secs {
		data := img[s.Addr-baseAddr : s.Addr-baseAddr+int64(len(s.Data))]
		err := ehRecords(data, func(off int, fde bool) error {
			if !fde {
				return nil
			}
			cie := off + 4 - int(le.Uint32(data[off+4:]))
			if cie < 0 || cie >= off {
				return fmt.Errorf(".eh_frame: bad CIE pointer at %#x", off)
			}
			enc, err := fdeEncoding(data, cie)
			if err != nil {
				return err
			}
			at := off + 8
			var pc int64
			switch enc & 0x0f {
			case 0x03:
				pc = int64(le.Uint32(data[at:]))
			case 0x0b:
				pc = int64(int32(le.Uint32(data[at:])))
			case 0x00, 0x04, 0x0c:
				pc = int64(le.Uint64(data[at:]))
			default:
				return fmt.Errorf(".eh_frame: unsupported FDE encoding %#x", enc)
			}
			if enc&0x70 == 0x10 {
				pc += s.Addr + int64(at)
			}
			table = append(table, entry{pc, s.Addr + int64(off)})
			return nil
		})
		if err != nil {
			return nil, err
		}
	}
	sort.Slice(table, func(i, j int) bool { return table[i].pc < table[j].pc })
	hdr := []byte{1, 0x1b, 0x03, 0x3b} // version; pc-relative sdata4 frame pointer; udata4 count; data-relative sdata4 table
	ehAddr := secs[0].Addr
	hdr = le.AppendUint32(hdr, uint32(int32(ehAddr-(hdrAddr+4))))
	hdr = le.AppendUint32(hdr, uint32(len(table)))
	for _, e := range table {
		hdr = le.AppendUint32(hdr, uint32(int32(e.pc-hdrAddr)))
		hdr = le.AppendUint32(hdr, uint32(int32(e.fde-hdrAddr)))
	}
	return hdr, nil
}
