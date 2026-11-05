package elf

import "bytes"

// TLS model relaxation (System V x86-64 TLS ABI, "ELF Handling For
// Thread-Local Storage"): code compiled with -fPIC reaches thread-local
// variables through __tls_get_addr (general and local dynamic models). An
// executable knows the offsets of its own TLS block, so the linker rewrites
// those sequences to local exec (variable of the executable) or initial exec
// (variable of a shared library), as GNU ld does.

var (
	// data16 lea x@tlsgd(%rip),%rdi; data16 data16 rex.W call __tls_get_addr@PLT
	tlsGDCall = []byte{0x66, 0x48, 0x8d, 0x3d}
	// lea x@tlsld(%rip),%rdi; call __tls_get_addr@PLT
	tlsLDCall = []byte{0x48, 0x8d, 0x3d}
	// mov %fs:0,%rax
	movFS0 = []byte{0x64, 0x48, 0x8b, 0x04, 0x25, 0, 0, 0, 0}
)

// relaxTLS rewrites general/local dynamic TLS accesses before relocations
// are scanned
func (l *Linker) relaxTLS() {
	for _, o := range l.Objects {
		for _, sec := range o.Sections {
			if sec.Kind != SecText {
				continue
			}
			l.relaxSectionTLS(o, sec)
		}
	}
}

func (l *Linker) relaxSectionTLS(o *Object, sec *Section) {
	has := false
	for _, r := range sec.Relocs {
		if r.Type == R_X86_64_TLSGD || r.Type == R_X86_64_TLSLD {
			has = true
			break
		}
	}
	if !has {
		return
	}
	d := sec.Data
	drop := map[int64]bool{} // offsets of the __tls_get_addr call relocations
	out := sec.Relocs[:0:0]
	isCall := func(at int64) bool {
		for _, r := range sec.Relocs {
			if r.Offset == at && (r.Type == R_X86_64_PLT32 || r.Type == R_X86_64_PC32 ||
				r.Type == R_X86_64_GOTPCREL || r.Type == R_X86_64_GOTPCRELX) && r.Sym.Name == "__tls_get_addr" {
				return true
			}
		}
		return false
	}
	for _, r := range sec.Relocs {
		switch r.Type {
		case R_X86_64_TLSGD:
			start := r.Offset - 4
			// direct call (16 bytes) or call *__tls_get_addr@GOTPCREL(%rip) (14 bytes,
			// -fno-plt)
			direct := start >= 0 && start+16 <= int64(len(d)) && bytes.Equal(d[start:start+4], tlsGDCall) &&
				bytes.Equal(d[start+8:start+12], []byte{0x66, 0x66, 0x48, 0xe8}) && isCall(start+12)
			indirect := !direct && start >= 0 && start+14 <= int64(len(d)) && bytes.Equal(d[start:start+4], tlsGDCall) &&
				bytes.Equal(d[start+8:start+10], []byte{0xff, 0x15}) && isCall(start+10)
			if !direct && !indirect {
				l.errorf("%s: unrecognized general dynamic TLS sequence for `%s'", o.Name, r.Sym.Name)
				continue
			}
			if indirect {
				l.errorf("%s: -fno-plt general dynamic TLS sequence for `%s' is not supported", o.Name, r.Sym.Name)
				continue
			}
			drop[start+12] = true
			s := l.resolve(r.Sym)
			// mov %fs:0,%rax; then lea x@tpoff(%rax),%rax (local exec) or
			// add x@gottpoff(%rip),%rax (initial exec)
			seq := append([]byte(nil), movFS0...)
			typ, add := uint32(R_X86_64_TPOFF32), int64(0)
			if l.imports[s] != nil || s.Section == nil {
				seq = append(seq, 0x48, 0x03, 0x05, 0, 0, 0, 0)
				typ, add = R_X86_64_GOTTPOFF, -4
			} else {
				seq = append(seq, 0x48, 0x8d, 0x80, 0, 0, 0, 0)
			}
			copy(d[start:], seq)
			out = append(out, Reloc{Offset: start + 12, Type: typ, Sym: r.Sym, Addend: add})
		case R_X86_64_TLSLD:
			start := r.Offset - 3
			if start < 0 || start+12 > int64(len(d)) || !bytes.Equal(d[start:start+3], tlsLDCall) || d[start+7] != 0xe8 || !isCall(start+8) {
				l.errorf("%s: unrecognized local dynamic TLS sequence", o.Name)
				continue
			}
			drop[start+8] = true
			// data16 data16 data16 mov %fs:0,%rax
			copy(d[start:], append([]byte{0x66, 0x66, 0x66}, movFS0...))
		case R_X86_64_DTPOFF32:
			// x@dtpoff(%rax) after a local dynamic sequence: an offset from
			// the thread pointer now
			r.Type = R_X86_64_TPOFF32
			out = append(out, r)
		default:
			out = append(out, r)
		}
	}
	final := out[:0]
	for _, r := range out {
		if !drop[r.Offset] || r.Sym.Name != "__tls_get_addr" {
			final = append(final, r)
		}
	}
	sec.Relocs = final
}
