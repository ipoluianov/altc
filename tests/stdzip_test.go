package tests

import (
	"archive/zip"
	"bytes"
	"compress/flate"
	"compress/gzip"
	"crypto/sha256"
	"encoding/base64"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"math"
	"math/rand"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"testing"
	"time"
)

// The differential test of std/src/archive/zip*.c (std/tests/diff/zip_diff.c)
// against Go's archive/zip: archives written by both (byte for byte when
// stored), and dumps of archives read by both: Go-written ones, Go's test
// data, and mutations of them.

func zipTok(err error) string {
	switch {
	case err == nil:
		return "ok"
	case errors.Is(err, zip.ErrFormat):
		return "format"
	case errors.Is(err, zip.ErrAlgorithm):
		return "algorithm"
	case errors.Is(err, zip.ErrChecksum):
		return "checksum"
	case err == io.EOF:
		return "eof"
	case err == io.ErrUnexpectedEOF:
		return "unexpected-eof"
	}
	var ce flate.CorruptInputError
	if errors.As(err, &ce) {
		return "corrupt"
	}
	msg := err.Error()
	for _, s := range []string{"negative offset", "size cannot be negative", "invalid comment length", "too long",
		"write to directory"} {
		if strings.Contains(msg, s) {
			return "inval"
		}
	}
	return "other:" + msg
}

// zipUnixMode is fileModeToUnixMode of archive/zip
func zipUnixMode(mode os.FileMode) uint32 {
	var m uint32
	switch mode & os.ModeType {
	default:
		m = 0x8000
	case os.ModeDir:
		m = 0x4000
	case os.ModeSymlink:
		m = 0xa000
	case os.ModeNamedPipe:
		m = 0x1000
	case os.ModeSocket:
		m = 0xc000
	case os.ModeDevice:
		m = 0x6000
	case os.ModeDevice | os.ModeCharDevice:
		m = 0x2000
	}
	if mode&os.ModeSetuid != 0 {
		m |= 0x800
	}
	if mode&os.ModeSetgid != 0 {
		m |= 0x400
	}
	if mode&os.ModeSticky != 0 {
		m |= 0x200
	}
	return m | uint32(mode&0777)
}

// zipNano is the time as alt_wall (clamped)
func zipNano(t time.Time) int64 {
	if t.Before(time.Unix(0, math.MinInt64)) {
		return math.MinInt64
	}
	if t.After(time.Unix(0, math.MaxInt64)) {
		return math.MaxInt64
	}
	return t.UnixNano()
}

func zipReadAll(b *strings.Builder, r io.Reader) {
	h := sha256.New()
	n, err := io.Copy(h, r)
	if err == nil || errors.Is(err, zip.ErrChecksum) {
		fmt.Fprintf(b, "%d:%x:%s", n, h.Sum(nil)[:8], zipTok(err))
	} else {
		fmt.Fprintf(b, "-:-:%s", zipTok(err))
	}
}

// zipDump is what zip_diff.c prints for an archive; structural leaves
// out what depends on the compressor (compressed sizes, offsets, raw data)
func zipDump(ra io.ReaderAt, size int64, sparse, structural bool) (s string) {
	defer func() {
		if p := recover(); p != nil {
			s = fmt.Sprint("panic ", p)
		}
	}()
	r, err := zip.NewReader(ra, size)
	if err != nil && err != zip.ErrInsecurePath {
		return "err " + zipTok(err)
	}
	insecure := false
	for _, f := range r.File {
		if f.Name != "" && (!filepath.IsLocal(f.Name) || strings.Contains(f.Name, `\`)) {
			insecure = true
		}
	}
	var b strings.Builder
	fmt.Fprintf(&b, "ok|%x|%d|%d", r.Comment, len(r.File), b2i(insecure))
	for _, f := range r.File {
		h := &f.FileHeader
		zone := "U"
		if h.Modified.Location() != time.UTC {
			_, off := h.Modified.Zone()
			zone = strconv.Itoa(off)
		}
		fmt.Fprintf(&b, "|%x,%x,%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,", h.Name, h.Comment, b2i(h.NonUTF8), h.CreatorVersion, h.ReaderVersion,
			h.Flags, h.Method, zipNano(h.Modified), zone, h.ModifiedTime, h.ModifiedDate, h.CRC32)
		if structural {
			fmt.Fprintf(&b, "-,%d,-,%d,", h.UncompressedSize, h.UncompressedSize64)
		} else {
			fmt.Fprintf(&b, "%d,%d,%d,%d,", h.CompressedSize, h.UncompressedSize, h.CompressedSize64, h.UncompressedSize64)
		}
		fmt.Fprintf(&b, "%x,%d,%o,", h.Extra, h.ExternalAttrs, zipUnixMode(h.Mode()))
		if off, err := f.DataOffset(); err != nil {
			b.WriteString("e:" + zipTok(err))
		} else if structural {
			b.WriteString("-")
		} else {
			fmt.Fprint(&b, off)
		}
		b.WriteString(",")
		big := sparse && (h.UncompressedSize64 > 1<<24 || h.CompressedSize64 > 1<<24)
		if rc, err := f.Open(); err != nil {
			b.WriteString("e:" + zipTok(err))
		} else if big {
			b.WriteString("skip")
		} else {
			zipReadAll(&b, rc)
		}
		b.WriteString(",")
		if rc, err := f.OpenRaw(); err != nil {
			b.WriteString("e:" + zipTok(err))
		} else if big {
			b.WriteString("skip")
		} else if structural {
			b.WriteString("-")
		} else {
			zipReadAll(&b, rc)
		}
	}
	return b.String()
}

// zipEntry is one entry of a writer test: kind 'H' CreateHeader, 'C'
// Create (deflate), 'R' CreateRaw
type zipEntry struct {
	kind   byte
	h      zip.FileHeader
	modSet bool
	modNs  int64
	modOff int
	utc    bool
	data   []byte
	chunk  int
}

func (e *zipEntry) header() zip.FileHeader {
	h := e.h
	h.Extra = append([]byte(nil), e.h.Extra...)
	h.CompressedSize = uint32(h.CompressedSize64)
	h.UncompressedSize = uint32(h.UncompressedSize64)
	if e.modSet {
		if e.utc {
			h.Modified = time.Unix(0, e.modNs).UTC()
		} else {
			h.Modified = time.Unix(0, e.modNs).In(time.FixedZone("", e.modOff))
		}
	}
	return h
}

func (e *zipEntry) encode() string {
	h := &e.h
	off := e.modOff
	if e.utc {
		off = 0
	}
	return fmt.Sprintf("%c,%x,%x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%x,%d,%x,%d", e.kind, h.Name, h.Comment, b2i(h.NonUTF8),
		h.CreatorVersion, h.ReaderVersion, h.Flags, h.Method, b2i(e.modSet), e.modNs, off, h.ModifiedTime, h.ModifiedDate, h.CRC32,
		h.CompressedSize64, h.UncompressedSize64, h.Extra, h.ExternalAttrs, e.data, e.chunk)
}

// deflated: whether the entry's data goes through the compressor
func (e *zipEntry) deflated() bool {
	if strings.HasSuffix(e.h.Name, "/") {
		return false
	}
	return e.kind == 'C' || e.kind == 'H' && e.h.Method == zip.Deflate
}

func zipGoWrite(offset int64, comment *string, entries []zipEntry) ([]byte, error) {
	var buf bytes.Buffer
	w := zip.NewWriter(&buf)
	w.SetOffset(offset)
	if comment != nil {
		if err := w.SetComment(*comment); err != nil {
			return nil, err
		}
	}
	for i := range entries {
		e := &entries[i]
		h := e.header()
		var fw io.Writer
		var err error
		switch e.kind {
		case 'C':
			fw, err = w.Create(h.Name)
		case 'R':
			fw, err = w.CreateRaw(&h)
		default:
			fw, err = w.CreateHeader(&h)
		}
		if err != nil {
			return nil, err
		}
		if len(e.data) > 0 {
			if _, err := fw.Write(e.data); err != nil {
				return nil, err
			}
		}
	}
	if err := w.Close(); err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

type zipGen struct{ r *rand.Rand }

var zipPieces = []string{"a", "b.txt", "dir/", "é", "日本", "\xff", `\`, "~", "}", " ", "\x01", "..", "/", "x", "\xc3", "\U0001F600",
	"\xed\xa0\x80", "C:", "README", ".", "\x7f", "ü"}

func (g zipGen) str(max int) string {
	var s strings.Builder
	for k := g.r.Intn(max + 1); k > 0; k-- {
		s.WriteString(zipPieces[g.r.Intn(len(zipPieces))])
	}
	return s.String()
}

func (g zipGen) u16() uint16 { return uint16(g.r.Intn(65536)) }

func (g zipGen) pick(v ...uint64) uint64 { return v[g.r.Intn(len(v))] }

func (g zipGen) data() []byte {
	var n int
	switch g.r.Intn(10) {
	case 0, 1, 2:
		n = 0
	case 3, 4, 5:
		n = g.r.Intn(100) + 1
	case 6, 7, 8:
		n = g.r.Intn(3000)
	default:
		if g.r.Intn(4) == 0 {
			n = 60000 + g.r.Intn(90000)
		} else {
			n = g.r.Intn(20000)
		}
	}
	b := make([]byte, n)
	if g.r.Intn(2) == 0 {
		g.r.Read(b)
	} else {
		words := []string{"zip ", "archive ", "deflate\n", "store ", "header, "}
		for i := 0; i < n; {
			i += copy(b[i:], words[g.r.Intn(len(words))])
		}
	}
	return b
}

func (g zipGen) extra() []byte {
	var e []byte
	le := binary.LittleEndian
	field := func(tag uint16, body []byte) {
		e = le.AppendUint16(e, tag)
		e = le.AppendUint16(e, uint16(len(body)))
		e = append(e, body...)
	}
	rnd := func(n int) []byte { b := make([]byte, n); g.r.Read(b); return b }
	for k := g.r.Intn(4); k > 0; k-- {
		switch g.r.Intn(9) {
		case 0:
			body := []byte{byte(g.r.Intn(8))}
			body = le.AppendUint32(body, g.r.Uint32())
			body = append(body, rnd(int(g.pick(0, 0, 4, 8, 3)))...)
			if g.r.Intn(5) == 0 {
				body = body[:g.r.Intn(len(body))]
			}
			field(0x5455, body)
		case 1, 2:
			body := rnd(4)
			for a := g.r.Intn(3); a >= 0; a-- {
				size := uint16(24)
				if g.r.Intn(6) == 0 {
					size = uint16(g.r.Intn(30))
				}
				tag := uint16(1)
				if g.r.Intn(6) == 0 {
					tag = uint16(g.r.Intn(4))
				}
				body = le.AppendUint16(body, tag)
				body = le.AppendUint16(body, size)
				for i := 0; i < int(size); i += 8 {
					var ts uint64
					switch g.r.Intn(4) {
					case 0:
						ts = g.r.Uint64()
					case 1:
						ts = uint64(int64(g.r.Intn(1<<30)) * 1e7)
					default:
						ts = uint64(116444736000000000 + g.r.Int63n(5e17) - 2e17)
					}
					body = le.AppendUint64(body, ts)
				}
			}
			if g.r.Intn(6) == 0 {
				body = body[:g.r.Intn(len(body))]
			}
			field(0x000a, body)
		case 3:
			tag := uint16(0x000d)
			if g.r.Intn(2) == 0 {
				tag = 0x5855
			}
			field(tag, rnd(int(g.pick(8, 8, 12, 4, 7))))
		case 4:
			field(0x0001, rnd(int(g.pick(0, 8, 16, 24, 4))))
		case 5:
			field(g.u16(), rnd(g.r.Intn(20)))
		case 6:
			e = append(e, rnd(g.r.Intn(5)+1)...)
		case 7:
			field(0x5455, []byte{1, 0, 0, 0, 0})
		default:
			field(0xcafe, nil)
		}
	}
	return e
}

func (g zipGen) entry(allowErr bool) zipEntry {
	r := g.r
	e := zipEntry{kind: "HHHHCR"[r.Intn(6)]}
	h := &e.h
	h.Name = g.str(5)
	if r.Intn(5) == 0 {
		h.Name += "/"
	}
	h.Comment = g.str(2)
	h.NonUTF8 = r.Intn(10) == 0
	h.CreatorVersion = uint16(g.pick(0, 0x0314, 0x0b14, 0x1314, uint64(g.u16())))
	h.ReaderVersion = uint16(g.pick(0, 20, 45, uint64(g.u16())))
	h.Flags = uint16(g.pick(0, 0, 0x800, 0x8, 0x808, 0x1, uint64(g.u16())))
	h.Method = uint16(g.pick(0, 0, 0, 8, 8))
	if allowErr && r.Intn(40) == 0 {
		h.Method = uint16(g.pick(12, 14, 99))
	}
	h.ModifiedTime, h.ModifiedDate = g.u16(), g.u16()
	if r.Intn(4) == 0 {
		h.ModifiedTime, h.ModifiedDate = 0, 0
	}
	if r.Intn(2) == 0 {
		e.modSet = true
		var t time.Time
		switch r.Intn(6) {
		case 0:
			t = time.Unix(0, 0)
		case 1:
			t = time.Date(1950+r.Intn(30), time.Month(r.Intn(12)+1), r.Intn(28)+1, r.Intn(24), r.Intn(60), r.Intn(60), 0, time.UTC)
		case 2:
			t = time.Date(2108+r.Intn(100), time.Month(r.Intn(12)+1), r.Intn(28)+1, r.Intn(24), r.Intn(60), r.Intn(60), 0, time.UTC)
		case 3:
			t = time.Date(int(g.pick(1979, 1980, 2107, 2108)), time.Month(g.pick(1, 12)), int(g.pick(1, 31)), int(g.pick(0, 23)), int(g.pick(0, 59)), int(g.pick(0, 59)), 0, time.UTC)
		default:
			t = time.Date(1980+r.Intn(127), time.Month(r.Intn(12)+1), r.Intn(31)+1, r.Intn(24), r.Intn(60), r.Intn(60), 0, time.UTC)
		}
		if r.Intn(2) == 0 {
			t = t.Add(time.Duration(r.Intn(1e9)))
		}
		e.modNs = t.UnixNano()
		offs := []int{0, 3600, -7200, 19800, -18000, 50400, -43200, 1234}
		e.modOff = offs[r.Intn(len(offs))]
		e.utc = e.modOff == 0 && r.Intn(2) == 0
	}
	h.CRC32 = uint32(g.pick(0, uint64(r.Uint32())))
	if r.Intn(2) == 0 {
		h.Extra = g.extra()
	}
	h.ExternalAttrs = uint32(g.pick(0, 0x81a40000, 0x41ed0010, 0xa1ff0000, 0x21, 1, uint64(r.Uint32())))
	if !strings.HasSuffix(h.Name, "/") || (allowErr && r.Intn(20) == 0) {
		e.data = g.data()
	}
	if e.kind == 'R' {
		h.Method = uint16(g.pick(0, 8, 8, 12))
		if h.Method == 8 && len(e.data) > 0 {
			var c bytes.Buffer
			fw, _ := flate.NewWriter(&c, r.Intn(10))
			fw.Write(e.data)
			fw.Close()
			h.UncompressedSize64 = uint64(len(e.data))
			e.data = c.Bytes()
		} else {
			h.UncompressedSize64 = uint64(len(e.data))
		}
		h.CompressedSize64 = uint64(len(e.data))
		if r.Intn(3) == 0 {
			h.CRC32 = 0
		}
		switch r.Intn(8) {
		case 0:
			h.UncompressedSize64 = g.pick(0, 1<<32-1, 1<<32, 5<<30, uint64(r.Uint32()))
		case 1:
			h.CompressedSize64 = g.pick(1<<32-1, 1<<32, 5<<30)
		}
	}
	if allowErr && r.Intn(200) == 0 {
		h.Name = strings.Repeat("n", 65530+r.Intn(10))
	}
	if allowErr && r.Intn(200) == 0 {
		h.Extra = make([]byte, 65530+r.Intn(10))
	}
	e.chunk = r.Intn(len(e.data)+1) + 1
	return e
}

func zipMutate(r *rand.Rand, b []byte) []byte {
	c := append([]byte(nil), b...)
	if len(c) == 0 {
		return c
	}
	switch r.Intn(7) {
	case 0:
		for k := r.Intn(3) + 1; k > 0; k-- {
			c[r.Intn(len(c))] ^= byte(1 << r.Intn(8))
		}
	case 1:
		for k := r.Intn(3) + 1; k > 0; k-- {
			c[len(c)-1-r.Intn(len(c)/4+1)] = byte(r.Intn(256))
		}
	case 2:
		return c[:r.Intn(len(c))]
	case 3, 4:
		// a field near a signature
		var sigs []int
		for i := 0; i+4 <= len(c); i++ {
			if c[i] == 'P' && c[i+1] == 'K' && c[i+2] < 9 && c[i+3] < 9 {
				sigs = append(sigs, i)
			}
		}
		if len(sigs) == 0 {
			return c
		}
		p := sigs[r.Intn(len(sigs))] + r.Intn(56)
		vals := []uint64{0, 0xffff, 0xffffffff, 0xffffffffffffffff, uint64(r.Intn(100)), uint64(r.Intn(len(c) + 10)), 1 << 63, r.Uint64()}
		v := vals[r.Intn(len(vals))]
		for k := int(g8(r)); k > 0 && p < len(c); k-- {
			c[p] = byte(v)
			v >>= 8
			p++
		}
	case 5:
		p := r.Intn(len(c) + 1)
		ins := make([]byte, r.Intn(30)+1)
		r.Read(ins)
		c = append(c[:p], append(ins, c[p:]...)...)
	default:
		p := r.Intn(len(c))
		q := p + r.Intn(len(c)-p) + 1
		c = append(c[:p], c[q:]...)
	}
	return c
}

func g8(r *rand.Rand) int { return []int{1, 2, 4, 8}[r.Intn(4)] }

type zipSparse struct {
	size  int64
	spans []zipSpan
}

type zipSpan struct {
	off  int64
	data []byte
}

func (f *zipSparse) ReadAt(p []byte, off int64) (int, error) {
	if off < 0 {
		return 0, errors.New("sparse: negative offset")
	}
	if off >= f.size {
		return 0, io.EOF
	}
	end := min(off+int64(len(p)), f.size)
	n := int(end - off)
	clear(p[:n])
	for _, s := range f.spans {
		s0, s1 := s.off, s.off+int64(len(s.data))
		if s1 <= off || s0 >= end {
			continue
		}
		from, to := max(s0, off), min(s1, end)
		copy(p[from-off:to-off], s.data[from-s0:to-s0])
	}
	if n < len(p) {
		return n, io.EOF
	}
	return n, nil
}

// zipReadSparse reads Go's .zsparse test files (gzip of: size, span count,
// then per span offset, length and data, little endian)
func zipReadSparse(path string) (*zipSparse, error) {
	raw, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	zr, err := gzip.NewReader(bytes.NewReader(raw))
	if err != nil {
		return nil, err
	}
	b, err := io.ReadAll(zr)
	if err != nil {
		return nil, err
	}
	le := binary.LittleEndian
	f := &zipSparse{size: int64(le.Uint64(b))}
	n := le.Uint32(b[8:])
	b = b[12:]
	for i := uint32(0); i < n; i++ {
		off, l := int64(le.Uint64(b)), le.Uint32(b[8:])
		f.spans = append(f.spans, zipSpan{off, b[12 : 12+l]})
		b = b[12+l:]
	}
	return f, nil
}

func TestDiffZip(t *testing.T) {
	seed := int64(2026)
	if s, err := strconv.ParseInt(os.Getenv("ZIPSEED"), 10, 64); err == nil {
		seed = s // other random cases
	}
	r := rand.New(rand.NewSource(seed))
	g := zipGen{r}
	var in, want []string
	transform := map[int]bool{} // answers to read back with Go (deflated archives)
	var archives [][]byte       // valid archives for the reader tests
	stored, deflated, errs := 0, 0, 0

	// writers
	for i := 0; i < 4000; i++ {
		var offset int64
		allowErr := r.Intn(4) == 0
		switch r.Intn(6) {
		case 0:
			offset = int64(r.Intn(100000))
		case 1:
			offset = int64(g.pick(1<<32-1, 1<<32-40, 1<<32, 5<<30))
		}
		var comment *string
		if r.Intn(2) == 0 {
			s := g.str(4)
			if allowErr && r.Intn(30) == 0 {
				s = strings.Repeat("c", 65530+r.Intn(10))
			}
			comment = &s
		}
		var entries []zipEntry
		for k := r.Intn(6); k > 0; k-- {
			entries = append(entries, g.entry(allowErr))
		}
		def := false
		for j := range entries {
			def = def || entries[j].deflated()
		}
		if def && offset > 1<<31 {
			offset = int64(r.Intn(1000))
		}
		line := fmt.Sprintf("W\t%d\t", offset)
		if comment == nil {
			line += "!"
		} else {
			line += hex.EncodeToString([]byte(*comment))
		}
		for j := range entries {
			line += "\t" + entries[j].encode()
		}
		out, err := zipGoWrite(offset, comment, entries)
		in = append(in, line)
		switch {
		case err != nil:
			want = append(want, "err "+zipTok(err))
			errs++
		case def:
			want = append(want, zipDump(bytes.NewReader(out), int64(len(out)), false, true))
			transform[len(in)-1] = true
			deflated++
		default:
			want = append(want, "ok "+hex.EncodeToString(out))
			stored++
		}
		if err == nil {
			archives = append(archives, out)
		}
	}
	// prefixes: a correct SetOffset, and none (a positive base offset)
	for i := 0; i < 200; i++ {
		pre := make([]byte, r.Intn(3000)+1)
		r.Read(pre)
		var entries []zipEntry
		for k := r.Intn(4) + 1; k > 0; k-- {
			entries = append(entries, g.entry(false))
		}
		off := int64(len(pre))
		if r.Intn(2) == 0 {
			off = 0
		}
		out, err := zipGoWrite(off, nil, entries)
		if err == nil {
			archives = append(archives, append(pre, out...))
		}
	}
	// Go's test data
	tdir := filepath.Join(runtime.GOROOT(), "src", "archive", "zip", "testdata")
	tfiles, _ := filepath.Glob(filepath.Join(tdir, "*"))
	var testdata [][]byte
	for _, p := range tfiles {
		b, err := os.ReadFile(p)
		if err != nil {
			continue // directories
		}
		if strings.HasSuffix(p, ".base64") {
			b, _ = base64.StdEncoding.DecodeString(string(b))
		}
		testdata = append(testdata, b)
	}
	if len(testdata) < 30 {
		t.Fatalf("only %d test data files in %s", len(testdata), tdir)
	}
	archives = append(archives, testdata...)
	// readers: every archive, and mutations
	nread := 0
	addRead := func(b []byte) {
		in = append(in, "R\t"+hex.EncodeToString(b))
		want = append(want, zipDump(bytes.NewReader(b), int64(len(b)), false, false))
		nread++
	}
	for _, b := range archives {
		if len(b) < 1<<20 {
			addRead(b)
		}
	}
	nmut := 0
	for i := 0; i < 30000; i++ {
		var b []byte
		if r.Intn(3) == 0 {
			b = testdata[r.Intn(len(testdata))]
		} else {
			b = archives[r.Intn(len(archives))]
		}
		if len(b) > 32768 {
			continue
		}
		b = zipMutate(r, b)
		if r.Intn(4) == 0 {
			b = zipMutate(r, b)
		}
		addRead(b)
		nmut++
	}
	// Zip64 archives of several GiB (sparse)
	sfiles, _ := filepath.Glob(filepath.Join(tdir, "zip64", "*.zsparse"))
	if len(sfiles) == 0 {
		t.Fatal("no sparse test data")
	}
	for _, p := range sfiles {
		sp, err := zipReadSparse(p)
		if err != nil {
			t.Fatal(err)
		}
		var spans []string
		for _, s := range sp.spans {
			spans = append(spans, fmt.Sprintf("%d:%x", s.off, s.data))
		}
		in = append(in, fmt.Sprintf("S\t%d\t%s", sp.size, strings.Join(spans, ";")))
		want = append(want, zipDump(sp, sp.size, true, false))
	}
	// Copy
	ncopy := 0
	for i := 0; i < 600; i++ {
		b := archives[r.Intn(len(archives))]
		if i < len(testdata) {
			b = testdata[i]
		}
		if len(b) > 200000 {
			continue
		}
		in = append(in, "C\t"+hex.EncodeToString(b))
		want = append(want, zipGoCopy(b))
		ncopy++
	}
	// many entries (Zip64 end records from 65535 on)
	for _, n := range []int{65534, 65535, 65536} {
		in = append(in, fmt.Sprintf("M\t%d", n))
		var buf bytes.Buffer
		w := zip.NewWriter(&buf)
		for i := 0; i < n; i++ {
			w.CreateHeader(&zip.FileHeader{Name: fmt.Sprintf("f%d", i)})
		}
		w.Close()
		zr, err := zip.NewReader(bytes.NewReader(buf.Bytes()), int64(buf.Len()))
		if err != nil {
			t.Fatal(err)
		}
		want = append(want, fmt.Sprintf("%x %d", sha256.Sum256(buf.Bytes()), len(zr.File)))
	}

	got := diffRun(t, "zip", in)
	for i := range got {
		if transform[i] {
			if hx, ok := strings.CutPrefix(got[i], "ok "); ok {
				b, _ := hex.DecodeString(hx)
				got[i] = zipDump(bytes.NewReader(b), int64(len(b)), false, true)
			}
		}
	}
	zipCompare(t, in, got, want)
	t.Logf("%d stored writes (byte for byte), %d deflated, %d writer errors; %d reads (%d mutated), %d sparse, %d copies",
		stored, deflated, errs, nread, nmut, len(sfiles), ncopy)
}

func zipGoCopy(b []byte) string {
	zr, err := zip.NewReader(bytes.NewReader(b), int64(len(b)))
	if err != nil && err != zip.ErrInsecurePath {
		return "err " + zipTok(err)
	}
	var buf bytes.Buffer
	w := zip.NewWriter(&buf)
	for _, f := range zr.File {
		if err := w.Copy(f); err != nil {
			return "err " + zipTok(err)
		}
	}
	if err := w.Close(); err != nil {
		return "err " + zipTok(err)
	}
	return "ok " + hex.EncodeToString(buf.Bytes())
}

// zipCompare is diffCompare for long lines: it shows where they differ
func zipCompare(t *testing.T, in, got, want []string) {
	t.Helper()
	cut := func(s string, from int) string {
		from = max(0, from)
		if from > len(s) {
			from = len(s)
		}
		end := min(len(s), from+240)
		return s[from:end]
	}
	bad := 0
	for i := range want {
		if got[i] == want[i] {
			continue
		}
		bad++
		if bad > 20 {
			continue
		}
		d := 0
		for d < len(got[i]) && d < len(want[i]) && got[i][d] == want[i][d] {
			d++
		}
		t.Errorf("%q:\n at %d\n got %q\nwant %q", cut(in[i], 0), d, cut(got[i], d-80), cut(want[i], d-80))
	}
	if bad > 20 {
		t.Errorf("... %d differences in %d cases", bad, len(want))
	}
}
