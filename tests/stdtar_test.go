package tests

import (
	"archive/tar"
	"bytes"
	"compress/bzip2"
	"encoding/base64"
	"encoding/hex"
	"fmt"
	"hash/fnv"
	"io"
	"math"
	"math/rand"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"testing"
	"time"
)

// TestDiffTar compares std/tests/diff/tar_diff.c (std/src/archive) with
// Go's archive/tar: archives written from random headers must be the
// same bytes with the same errors, and the readers must agree on every
// field, the data and the errors of Go-written archives, Go's testdata,
// handmade archives (PAX, GNU, STAR, sparse formats) and mutations of
// all of them.

type tarEntry struct {
	h    tar.Header
	data []byte
}

func tarTime(t time.Time) string {
	if t == (time.Time{}) {
		return "-"
	}
	return fmt.Sprintf("%d:%d", t.Unix(), t.Nanosecond())
}

func tarEncode(es []tarEntry) string {
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	var line strings.Builder
	line.WriteString("W")
	for _, e := range es {
		x := e.h
		var pax []string
		for k, v := range x.PAXRecords {
			pax = append(pax, h(k)+":"+h(v))
		}
		sort.Strings(pax)
		fmt.Fprintf(&line, "\t%d,%s,%s,%d,%d,%d,%d,%s,%s,%s,%s,%s,%d,%d,%d,%s,%s", x.Typeflag, h(x.Name), h(x.Linkname),
			x.Size, x.Mode, x.Uid, x.Gid, h(x.Uname), h(x.Gname), tarTime(x.ModTime), tarTime(x.AccessTime), tarTime(x.ChangeTime),
			x.Devmajor, x.Devminor, int(x.Format), hex.EncodeToString(e.data), strings.Join(pax, ";"))
	}
	return line.String()
}

// goTarWrite returns the errors and the archive as tar_diff.c does
func goTarWrite(es []tarEntry) (string, []byte) {
	var buf bytes.Buffer
	var errs strings.Builder
	tw := tar.NewWriter(&buf)
	for i, e := range es {
		h := e.h
		if err := tw.WriteHeader(&h); err != nil {
			fmt.Fprintf(&errs, "h%d:%s;", i, err)
		}
		if len(e.data) > 0 {
			if _, err := tw.Write(e.data); err != nil {
				fmt.Fprintf(&errs, "d%d:%s;", i, err)
			}
		}
	}
	if err := tw.Close(); err != nil {
		fmt.Fprintf(&errs, "c:%s;", err)
	}
	return errs.String(), buf.Bytes()
}

func goTarRead(a []byte) string {
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	tm := func(t time.Time) string {
		if t.IsZero() {
			return " z"
		}
		return fmt.Sprintf(" %d.%d", t.Unix(), t.Nanosecond())
	}
	var out strings.Builder
	tr := tar.NewReader(struct{ io.Reader }{bytes.NewReader(a)})
	buf := make([]byte, 4096)
	for count := 0; count < 2000; count++ {
		x, err := tr.Next()
		if err != nil {
			out.WriteString("E:" + err.Error())
			break
		}
		name := h(x.Name)
		if x.Typeflag == 'g' && len(x.PAXRecords) == 0 {
			// a bad record: Go may or may not have applied a path record
			// before it, depending on the map order
			name = "?"
		}
		fmt.Fprintf(&out, "{%d %s %s %d %d %d %d %s %s%s%s%s %d %d %d [", x.Typeflag, name, h(x.Linkname), x.Size, x.Mode,
			x.Uid, x.Gid, h(x.Uname), h(x.Gname), tm(x.ModTime), tm(x.AccessTime), tm(x.ChangeTime), x.Devmajor, x.Devminor, int(x.Format))
		var keys []string
		for k := range x.PAXRecords {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		for _, k := range keys {
			out.WriteString(h(k) + "=" + h(x.PAXRecords[k]) + ";")
		}
		out.WriteString("]}")
		fh := fnv.New64a()
		total, limit := 0, 1<<20
		rerr := ""
		for total < limit {
			want := min(limit-total, len(buf))
			n, err := tr.Read(buf[:want])
			fh.Write(buf[:n])
			total += n
			if err == io.EOF {
				break
			}
			if err != nil {
				rerr = err.Error()
				break
			}
			if n == 0 {
				break
			}
		}
		fmt.Fprintf(&out, "d%d:%016x:%s;", total, fh.Sum64(), rerr)
	}
	return out.String()
}

// random headers

type tarGen struct{ r *rand.Rand }

func (g tarGen) pick(xs ...string) string { return xs[g.r.Intn(len(xs))] }

func (g tarGen) component() string {
	switch g.r.Intn(10) {
	case 0:
		return strings.Repeat("x", 1+g.r.Intn(120))
	case 1:
		return strings.Repeat("é", 1+g.r.Intn(60))
	case 2:
		return g.pick("..", ".", "", "日本語", "a b", "\x7f", "\xff\xfe", "A=B", "PaxHeaders.0")
	}
	return g.pick("a", "bb", "file.txt", "dir", "src", "x.tar.gz", "usr", "local", "lib")[:]
}

func (g tarGen) name() string {
	var parts []string
	n := 1 + g.r.Intn(4)
	if g.r.Intn(4) == 0 {
		n += g.r.Intn(12)
	}
	for i := 0; i < n; i++ {
		parts = append(parts, g.component())
	}
	s := strings.Join(parts, "/")
	switch g.r.Intn(8) {
	case 0:
		s = "/" + s
	case 1:
		s += "/"
	case 2:
		s += "//"
	}
	if g.r.Intn(20) == 0 {
		// lengths around the USTAR limits
		s = strings.Repeat("p", 50+g.r.Intn(110)) + "/" + strings.Repeat("n", 1+g.r.Intn(105))
	}
	return s
}

func (g tarGen) num(bits ...int64) int64 {
	switch g.r.Intn(8) {
	case 0:
		return bits[g.r.Intn(len(bits))]
	case 1:
		return -g.r.Int63n(1 << uint(g.r.Intn(62)+1))
	case 2:
		return g.r.Int63n(1 << uint(g.r.Intn(62)+1))
	}
	return g.r.Int63n(5000)
}

func (g tarGen) tm(mtime bool) time.Time {
	switch g.r.Intn(9) {
	case 0, 1:
		if !mtime || g.r.Intn(2) == 0 {
			return time.Time{}
		}
	case 2:
		return time.Unix(g.r.Int63n(1<<33)-1<<32, 0)
	case 3:
		return time.Unix(g.r.Int63n(1<<40)-1<<39, int64(g.r.Intn(1e9)))
	case 4:
		return time.Unix([]int64{0, -1, 1<<33 - 1, 1 << 33, -62135596800, 253402300799, 1 << 40, -1 << 40, 1<<62 - 1, -1 << 62}[g.r.Intn(10)],
			[]int64{0, 0, 1, 499999999, 500000000, 999999999}[g.r.Intn(6)])
	}
	return time.Unix(1600000000+g.r.Int63n(1e8), int64(g.r.Intn(1e9))*int64(g.r.Intn(2)))
}

func (g tarGen) str(long int) string {
	switch g.r.Intn(6) {
	case 0:
		return ""
	case 1:
		return strings.Repeat("u", long-2+g.r.Intn(5))
	case 2:
		return g.pick("ünï", "root\x80", "日本")
	}
	return g.pick("root", "wheel", "user", "nobody", "gopher")
}

func (g tarGen) header(i int) tarEntry {
	var e tarEntry
	h := &e.h
	flags := []byte{'0', '0', '0', '0', 0, '1', '2', '3', '4', '5', '6', '7', 'x', 'g', 'S', 'L', 'K', 'A', 'V', 'D'}
	h.Typeflag = flags[g.r.Intn(len(flags))]
	h.Name = g.name()
	if g.r.Intn(3) == 0 {
		h.Linkname = g.name()
	}
	h.Mode = []int64{0o644, 0o755, 0o4777, 0o7777, 0o100644}[g.r.Intn(5)]
	if g.r.Intn(6) == 0 {
		h.Mode = g.num(1<<21-1, 1<<21, 1<<56, -1, math.MaxInt64, math.MinInt64)
	}
	h.Uid = int(g.num(1<<21-1, 1<<21, 1<<56, -1, 1<<56-1, -1<<56, math.MaxInt64))
	h.Gid = int(g.num(1<<21-1, 1<<21, -1<<56-1, math.MinInt64))
	h.Uname, h.Gname = g.str(32), g.str(32)
	h.ModTime, h.AccessTime, h.ChangeTime = g.tm(true), g.tm(false), g.tm(false)
	if g.r.Intn(4) == 0 {
		h.Devmajor, h.Devminor = g.num(1<<21-1, 1<<21, -1, 1<<56), g.num(1<<21, -1<<56)
	}
	h.Format = []tar.Format{0, 0, 0, tar.FormatUSTAR, tar.FormatPAX, tar.FormatGNU, tar.FormatPAX | tar.FormatGNU,
		tar.FormatUSTAR | tar.FormatPAX, tar.FormatUSTAR | tar.FormatGNU, tar.FormatUSTAR | tar.FormatPAX | tar.FormatGNU}[g.r.Intn(10)]
	if g.r.Intn(3) == 0 {
		h.PAXRecords = map[string]string{}
		for k := g.r.Intn(4); k >= 0; k-- {
			key := g.pick("GOLANG.pkg", "SCHILY.xattr.user.a", "comment", "path", "mtime", "size", "uid", "linkpath",
				"GNU.sparse.major", "charset", "é.key", "uname")
			val := g.pick("v", "", "1", "0", strings.Repeat("long value ", 20), "ü", "a\nb", "x\x00y")
			if key == "path" && g.r.Intn(2) == 0 {
				val = h.Name
			}
			if key == "mtime" && g.r.Intn(2) == 0 {
				val = strconv.FormatInt(h.ModTime.Unix(), 10)
			}
			if !(val == "x\x00y" && (key == "path" || key == "linkpath" || key == "uname")) {
				h.PAXRecords[key] = val
			}
		}
		if g.r.Intn(50) == 0 {
			h.PAXRecords[strings.Repeat("K", 300000)] = strings.Repeat("v", 800000) // more than 1 MiB of records
		}
	}
	// at most one invalid string: Go reports one of several at random
	switch g.r.Intn(40) {
	case 0:
		h.Name += "\x00z"
	case 1:
		h.Uname = "a\x00"
	case 2:
		if h.PAXRecords == nil {
			h.PAXRecords = map[string]string{}
		}
		h.PAXRecords[g.pick("a=b", "", "k\x00")] = "v"
	}
	if h.Typeflag == 'g' && g.r.Intn(2) == 0 {
		*h = tar.Header{Typeflag: 'g', Name: h.Name, PAXRecords: h.PAXRecords, Format: h.Format}
		if g.r.Intn(2) == 0 {
			h.Name = ""
		}
	}
	switch g.r.Intn(12) {
	case 0:
		h.Size = g.num(1<<33-1, 1<<33, -1, math.MaxInt64, 1<<40)
	case 1:
		h.Size = int64(g.r.Intn(3000))
		e.data = make([]byte, h.Size+int64(g.r.Intn(5))-2)
	default:
		h.Size = int64(g.r.Intn(1500))
		if g.r.Intn(10) == 0 {
			h.Size = 0
		}
		e.data = make([]byte, h.Size)
	}
	g.r.Read(e.data)
	return e
}

func tarGoTestdata(t *testing.T) map[string][]byte {
	dir := filepath.Join(runtime.GOROOT(), "src", "archive", "tar", "testdata")
	files, _ := filepath.Glob(filepath.Join(dir, "*"))
	out := map[string][]byte{}
	for _, f := range files {
		b, err := os.ReadFile(f)
		if err != nil {
			t.Fatal(err)
		}
		switch {
		case strings.HasSuffix(f, ".tar"):
		case strings.HasSuffix(f, ".base64"):
			b, err = base64.StdEncoding.DecodeString(string(b))
		case strings.HasSuffix(f, ".bz2"):
			b, err = io.ReadAll(bzip2.NewReader(bytes.NewReader(b)))
		default:
			continue
		}
		if err != nil {
			t.Fatal(f, err)
		}
		out[filepath.Base(f)] = b
	}
	if len(out) < 30 {
		t.Fatalf("only %d testdata archives in %s", len(out), dir)
	}
	return out
}

// handmade archives

type rawHdr struct {
	name, link, uname, gname, magic, version, prefix string
	flag                                             byte
	size, mode, mtime                                string
	extra                                            map[int][]byte // raw bytes at offsets
	badSum                                           bool
}

func octal(n int64, w int) string { return fmt.Sprintf("%0*o", w-1, n) }

func (h rawHdr) block() []byte {
	b := make([]byte, 512)
	copy(b[0:100], h.name)
	copy(b[100:108], h.mode)
	copy(b[108:116], octal(0, 8))
	copy(b[116:124], octal(0, 8))
	copy(b[124:136], h.size)
	copy(b[136:148], h.mtime)
	b[156] = h.flag
	copy(b[157:257], h.link)
	copy(b[257:263], h.magic)
	copy(b[263:265], h.version)
	copy(b[265:297], h.uname)
	copy(b[297:329], h.gname)
	copy(b[345:500], h.prefix)
	for off, v := range h.extra {
		copy(b[off:], v)
	}
	tarChecksum(b)
	if h.badSum {
		b[150]++
	}
	return b
}

func tarChecksum(b []byte) {
	copy(b[148:156], "        ")
	sum := 0
	for _, c := range b[:512] {
		sum += int(c)
	}
	copy(b[148:156], fmt.Sprintf("%06o\x00 ", sum))
}

func pad512(b []byte) []byte { return append(b, make([]byte, -len(b)&511)...) }

func paxData(kv ...string) []byte {
	var s strings.Builder
	for i := 0; i+1 < len(kv); i += 2 {
		rec := kv[i] + "=" + kv[i+1] + "\n"
		n := len(rec) + 1
		for len(strconv.Itoa(n))+1+len(rec) != n {
			n = len(strconv.Itoa(n)) + 1 + len(rec)
		}
		fmt.Fprintf(&s, "%d %s", n, rec)
	}
	return []byte(s.String())
}

func tarArchive(parts ...[]byte) []byte {
	var b []byte
	for _, p := range parts {
		b = append(b, pad512(p)...)
	}
	return append(b, make([]byte, 1024)...)
}

func ustarHdr(name string, flag byte, size int) rawHdr {
	return rawHdr{name: name, flag: flag, size: octal(int64(size), 12), mode: octal(0o644, 8), mtime: octal(1234567890, 12),
		magic: "ustar\x00", version: "00", uname: "u", gname: "g"}
}

func paxEntry(flag byte, data []byte) [][]byte {
	return [][]byte{ustarHdr("PaxHeaders.0/f", flag, len(data)).block(), data}
}

func tarHandmade(r *rand.Rand) [][]byte {
	var out [][]byte
	add := func(parts ...[]byte) { out = append(out, tarArchive(parts...)) }
	addp := func(pre [][]byte, parts ...[]byte) { add(append(append([][]byte{}, pre...), parts...)...) }
	file := func(name string, data string) [][]byte {
		return [][]byte{ustarHdr(name, '0', len(data)).block(), []byte(data)}
	}
	cat := func(xs ...[][]byte) [][]byte {
		var o [][]byte
		for _, x := range xs {
			o = append(o, x...)
		}
		return o
	}
	// PAX records of every kind, good and bad
	recs := [][]string{
		{"path", "long/" + strings.Repeat("n", 200)}, {"path", ""}, {"linkpath", "target"}, {"uname", "ü"}, {"gname", ""},
		{"uid", "123456789012"}, {"uid", "x"}, {"gid", "-5"}, {"gid", "99999999999999999999"},
		{"size", "10"}, {"size", "-1"}, {"size", "0"}, {"size", "abc"},
		{"mtime", "1.5"}, {"mtime", "-1.5"}, {"mtime", "-0.000000001"}, {"mtime", "123.1234567891234"}, {"mtime", "1."},
		{"mtime", ".5"}, {"mtime", "1.x"}, {"mtime", "+5.25"}, {"atime", "9223372036854775807.999999999"}, {"ctime", "-9223372036854775808.5"},
		{"atime", "9223372036854775808"}, {"SCHILY.xattr.user.k", "v"}, {"GOLANG.x", "y\x00z"}, {"comment", ""},
		{"GNU.sparse.name", "sparse-name"}, {"GNU.sparse.size", "100"},
	}
	for _, kv := range recs {
		addp(cat(paxEntry('x', paxData(kv...)), file("file", "0123456789")))
		addp(cat(paxEntry('g', paxData(kv...)), file("file", "abc")))
	}
	for i := 0; i < 60; i++ {
		var kv []string
		for k := r.Intn(4); k >= 0; k-- {
			kv = append(kv, recs[r.Intn(len(recs))]...)
		}
		addp(cat(paxEntry('x', paxData(kv...)), file("f", "0123456789")))
	}
	// malformed records
	for _, d := range []string{"", "5 a=b\n", "6 a=b\n", "7 a=b\n", "x a=b\n", "10 a=b\n", "6 ab\n\n", "4 a\n", "05 =b\n",
		"6 a=b\r", "-6 a=b\n", "+7 a=b\n", "7 a=\x00\n", "8 path=\x00\n", "13 a=b\n6 c=d\n", "6 a=b", "9 k\x00e=v\n",
		"1000 a=b\n", "11 path=a\n", "11 path=b\n11 path=c\n"} {
		addp(cat(paxEntry('x', []byte(d)), file("f", "x")))
	}
	// several meta headers in a row, a meta header at the end
	addp(cat(paxEntry('x', paxData("path", "first")), paxEntry('x', paxData("uid", "7")), file("f", "")))
	add(paxEntry('x', paxData("path", "p"))...)
	add(ustarHdr("x", 'L', 3).block(), []byte("abc"))
	// GNU long names
	gnu := func(flag byte, data string) [][]byte {
		h := ustarHdr("././@LongLink", flag, len(data))
		h.magic, h.version = "ustar ", " \x00"
		return [][]byte{h.block(), []byte(data)}
	}
	gfile := func(name string, flag byte, size int) rawHdr {
		h := ustarHdr(name, flag, size)
		h.magic, h.version = "ustar ", " \x00"
		return h
	}
	long := strings.Repeat("L", 300)
	addp(cat(gnu('L', long+"\x00junk"), gnu('K', "link\x00"), [][]byte{gfile("short", '2', 0).block()}))
	addp(cat(gnu('L', ""), [][]byte{gfile("short", '0', 0).block()}))
	addp(cat(gnu('L', long), gnu('L', "second"), [][]byte{gfile("short", 0, 0).block()}))
	// GNU atime and ctime, base-256 numbers
	g1 := gfile("times", '0', 0)
	g1.extra = map[int][]byte{345: []byte(octal(1500000000, 12)), 357: []byte(octal(1600000000, 12))}
	g2 := gfile("badtimes", '0', 0)
	g2.extra = map[int][]byte{345: []byte("garbage"), 357: []byte(octal(5, 12)), 500: []byte("pre")}
	g2.prefix = "gnuprefix"
	g3 := gfile("b256", '0', 0)
	g3.extra = map[int][]byte{108: {0x80, 0, 0, 0, 0, 0, 0x12, 0x34}, 116: {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe},
		136: {0x80, 0, 0, 0, 0, 0, 0, 0x01, 0, 0, 0, 0}, 329: {0x80, 1, 2, 3, 4, 5, 6, 7}}
	g4 := gfile("b256-overflow", '0', 0)
	g4.extra = map[int][]byte{136: {0x80, 0, 0, 0x80, 0, 0, 0, 0, 0, 0, 0, 0}}
	g5 := gfile("b256-size", '0', 0)
	g5.size = string([]byte{0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5})
	g6 := gfile("negsize", '0', 0)
	g6.size = string([]byte{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff})
	for _, h := range []rawHdr{g1, g2, g3, g4, g5, g6} {
		add(h.block(), []byte("hello"))
	}
	// odd numeric fields
	for _, f := range []string{"", "  12 ", "\x0012", "12\x0034", "0000000012\x00", "89", "-1", "+1", "0x1", " 1 2", "777777777777"} {
		h := ustarHdr("num", '0', 0)
		h.mode = f
		add(h.block())
		h = ustarHdr("num", '0', 0)
		h.size = f
		add(h.block(), []byte("data-data-data"))
	}
	// V7, STAR, USTAR prefix, a bad checksum, a signed checksum, non-ASCII
	v7 := ustarHdr("v7file", 0, 3)
	v7.magic, v7.version, v7.uname, v7.gname = "", "", "", ""
	add(v7.block(), []byte("abc"))
	v7d := v7
	v7d.name = "v7dir/"
	add(v7d.block())
	star := ustarHdr("starfile", '0', 2)
	star.extra = map[int][]byte{345: []byte("starprefix"), 476: []byte(octal(1000, 12)), 488: []byte(octal(2000, 12)), 508: []byte("tar\x00")}
	add(star.block(), []byte("hi"))
	pre := ustarHdr("name", '0', 0)
	pre.prefix = "some/prefix"
	add(pre.block())
	bad := ustarHdr("bad", '0', 0)
	bad.badSum = true
	add(bad.block())
	add(ustarHdr("ok", '0', 0).block(), make([]byte, 512), ustarHdr("after-zero", '0', 0).block())
	sig := ustarHdr("signed\xff\xfe", '0', 0)
	b := sig.block()
	sum := 0
	for i, c := range b {
		if i >= 148 && i < 156 {
			c = ' '
		}
		sum += int(int8(c))
	}
	copy(b[148:156], fmt.Sprintf("%06o\x00 ", sum))
	add(b)
	nul := ustarHdr("nonnul", '0', 0)
	nul.mode = "00006440"
	add(nul.block())

	// sparse files: old GNU format with extended headers
	oldSparse := func(entries [][2]int64, realSize int64, data []byte, ext bool) [][]byte {
		h := gfile("sparse", 'S', len(data))
		h.extra = map[int][]byte{483: []byte(octal(realSize, 12))}
		var blocks [][]byte
		for i, e := range entries {
			if i < 4 {
				h.extra[386+i*24] = []byte(octal(e[0], 12))
				h.extra[386+i*24+12] = []byte(octal(e[1], 12))
			}
		}
		rest := entries[min(4, len(entries)):]
		if len(rest) > 0 || ext {
			h.extra[482] = []byte{1}
		}
		blocks = append(blocks, h.block())
		for len(rest) > 0 || ext {
			ext = false
			blk := make([]byte, 512)
			for i := 0; i < 21 && len(rest) > 0; i++ {
				copy(blk[i*24:], octal(rest[0][0], 12))
				copy(blk[i*24+12:], octal(rest[0][1], 12))
				rest = rest[1:]
			}
			if len(rest) > 0 {
				blk[504] = 1
			}
			blocks = append(blocks, blk)
		}
		return append(blocks, data)
	}
	sparseCases := []struct {
		e    [][2]int64
		size int64
	}{
		{[][2]int64{{0, 5}}, 5}, {[][2]int64{{2, 3}, {10, 2}}, 20}, {nil, 10}, {[][2]int64{{0, 0}}, 0},
		{[][2]int64{{5, 5}, {0, 2}}, 20}, {[][2]int64{{0, 30}}, 20}, {[][2]int64{{-1, 2}}, 20},
		{[][2]int64{{0, 1}, {2, 1}, {4, 1}, {6, 1}, {8, 1}, {10, 1}, {12, 1}}, 14},
	}
	var many [][2]int64
	for i := int64(0); i < 60; i++ {
		many = append(many, [2]int64{i * 10, 3})
	}
	sparseCases = append(sparseCases, struct {
		e    [][2]int64
		size int64
	}{many, 700})
	for _, c := range sparseCases {
		n := int64(0)
		for _, e := range c.e {
			n += e[1]
		}
		for _, delta := range []int64{0, -1, 1} {
			if n+delta < 0 {
				continue
			}
			data := bytes.Repeat([]byte("D"), int(n+delta))
			add(oldSparse(c.e, c.size, data, false)...)
		}
	}
	add(oldSparse([][2]int64{{0, 1}}, 4, []byte("x"), true)...)
	// PAX sparse 0.0, 0.1 and 1.0
	sp1 := func(m string, data string, size string, kv ...string) [][]byte {
		recs := append([]string{"GNU.sparse.major", "1", "GNU.sparse.minor", "0", "GNU.sparse.name", "real/name", "GNU.sparse.realsize", size}, kv...)
		body := append(pad512([]byte(m)), data...)
		return cat(paxEntry('x', paxData(recs...)), [][]byte{ustarHdr("GNUSparseFile.0/name", '0', len(body)).block(), body})
	}
	add(sp1("2\n0\n3\n10\n2\n", "abcde", "20")...)
	add(sp1("2\n0\n3\n10\n2\n", "abcd", "20")...)
	add(sp1("2\n0\n3\n10\n2\n", "abcdef", "20")...)
	add(sp1("0\n", "", "7")...)
	add(sp1("1\n0\n", "", "7")...)
	add(sp1("x\n", "", "7")...)
	add(sp1("-1\n", "", "7")...)
	add(sp1("1\n5\n9\n", "123456789", "10")...)
	add(sp1("1\n5\n", "", "10", "GNU.sparse.size", "x")...)
	big := "200\n"
	for i := 0; i < 200; i++ {
		big += fmt.Sprintf("%d\n1\n", i*2)
	}
	add(sp1(big, strings.Repeat("z", 200), "400")...)
	add(sp1("1\n0\n2\n", "ab", "60000000000")...)
	sp0 := func(kv ...string) [][]byte {
		return cat(paxEntry('x', paxData(kv...)), [][]byte{ustarHdr("sp0", '0', 5).block(), []byte("ABCDE")})
	}
	add(sp0("GNU.sparse.numblocks", "2", "GNU.sparse.offset", "0", "GNU.sparse.numbytes", "2", "GNU.sparse.offset", "8",
		"GNU.sparse.numbytes", "3", "GNU.sparse.size", "11")...)
	add(sp0("GNU.sparse.numblocks", "1", "GNU.sparse.numbytes", "2", "GNU.sparse.offset", "0", "GNU.sparse.size", "11")...)
	add(sp0("GNU.sparse.numblocks", "1", "GNU.sparse.offset", "0,1", "GNU.sparse.numbytes", "5", "GNU.sparse.size", "11")...)
	add(sp0("GNU.sparse.numblocks", "2", "GNU.sparse.map", "0,2,8,3", "GNU.sparse.size", "11")...)
	add(sp0("GNU.sparse.major", "0", "GNU.sparse.minor", "1", "GNU.sparse.numblocks", "2", "GNU.sparse.map", "0,2,8,3", "GNU.sparse.realsize", "11")...)
	add(sp0("GNU.sparse.major", "0", "GNU.sparse.minor", "1", "GNU.sparse.numblocks", "3", "GNU.sparse.map", "0,2,8,3", "GNU.sparse.realsize", "11")...)
	add(sp0("GNU.sparse.major", "0", "GNU.sparse.minor", "1", "GNU.sparse.numblocks", "1", "GNU.sparse.map", "0,x", "GNU.sparse.realsize", "11")...)
	add(sp0("GNU.sparse.major", "0", "GNU.sparse.minor", "0", "GNU.sparse.map", "0,5")...)
	add(sp0("GNU.sparse.major", "2", "GNU.sparse.minor", "0", "GNU.sparse.map", "0,5")...)
	add(sp0("GNU.sparse.map", "", "GNU.sparse.numblocks", "0")...)
	add(sp0("GNU.sparse.numblocks", "0", "GNU.sparse.map", "0,5", "GNU.sparse.size", "5", "GNU.sparse.offset", "0", "GNU.sparse.numbytes", "5")...)
	hdrOnly := cat(paxEntry('x', paxData("GNU.sparse.map", "0,1", "GNU.sparse.numblocks", "1")), [][]byte{ustarHdr("d", '5', 0).block()})
	add(hdrOnly...)
	// truncations inside every kind of entry
	add([]byte("short"))
	out = append(out, []byte{}, make([]byte, 512), make([]byte, 700), make([]byte, 1024))
	return out
}

func tarMutate(r *rand.Rand, a []byte) []byte {
	b := append([]byte{}, a...)
	if len(b) == 0 {
		return b
	}
	switch r.Intn(5) {
	case 0: // truncate
		return b[:r.Intn(len(b))]
	case 1, 2: // change bytes of a header and fix its checksum
		if len(b) < 512 {
			return b
		}
		blk := r.Intn(len(b) / 512)
		h := b[blk*512 : blk*512+512]
		for k := 1 + r.Intn(3); k > 0; k-- {
			i := r.Intn(512)
			switch r.Intn(4) {
			case 0:
				h[i] = byte(r.Intn(256))
			case 1:
				h[i] = "0123456789 \x00/x.=\n"[r.Intn(17)]
			case 2:
				h[i] = 0
			default:
				h[i] ^= 1 << uint(r.Intn(8))
			}
		}
		if r.Intn(5) != 0 {
			tarChecksum(h)
		}
	default: // change data bytes (PAX records, sparse maps, names)
		for k := 1 + r.Intn(4); k > 0; k-- {
			i := r.Intn(len(b))
			b[i] = "0123456789 \x00/x.=\n,"[r.Intn(18)]
		}
	}
	return b
}

func TestDiffTar(t *testing.T) {
	time.Local = time.UTC // header errors show times in the local zone
	seed := int64(1017)
	if s, err := strconv.ParseInt(os.Getenv("TAR_SEED"), 10, 64); err == nil {
		seed = s // other random cases
	}
	r := rand.New(rand.NewSource(seed))
	g := tarGen{r}
	var in, want []string
	var archives [][]byte
	// writing
	nwrite := 0
	for i := 0; i < 4000; i++ {
		var es []tarEntry
		for k := 1 + r.Intn(3); k > 0; k-- {
			es = append(es, g.header(i))
		}
		in = append(in, tarEncode(es))
		errs, a := goTarWrite(es)
		want = append(want, errs+"|"+hex.EncodeToString(a))
		nwrite++
		if len(a) > 1024 && len(a) < 1<<20 {
			archives = append(archives, a)
		}
	}
	// reading
	td := tarGoTestdata(t)
	var names []string
	for n := range td {
		names = append(names, n)
	}
	sort.Strings(names)
	var reads [][]byte
	for _, n := range names {
		reads = append(reads, td[n])
	}
	hand := tarHandmade(r)
	reads = append(reads, hand...)
	reads = append(reads, archives...)
	var small [][]byte // inputs to mutate
	for _, a := range append(append([][]byte{}, hand...), archives...) {
		if len(a) < 64<<10 {
			small = append(small, a)
		}
	}
	for _, n := range names {
		if len(td[n]) < 64<<10 {
			small = append(small, td[n])
		}
	}
	for i := 0; i < 12000; i++ {
		a := small[r.Intn(len(small))]
		for k := 1 + r.Intn(2); k > 0; k-- {
			a = tarMutate(r, a)
		}
		reads = append(reads, a)
	}
	for _, a := range reads {
		in = append(in, "R\t"+hex.EncodeToString(a))
		want = append(want, goTarRead(a))
	}
	t.Logf("%d archives written, %d read (%d testdata, %d handmade)", nwrite, len(reads), len(names), len(hand))
	// what the cases cover
	classes := []string{"cannot encode header: Format specifies", "cannot encode header: USTAR", "cannot encode header: GNU",
		"cannot encode header: PAX", "invalid PAX record", "trailing slash", "cannot manually encode", "only PAXRecords",
		"negative size", "missed writing", "write too long", "header field too long", "invalid tar header", "unexpected EOF",
		"sparse map too long", "non-existent data", "unreferenced data"}
	kinds := map[string]int{}
	entries := 0
	for i, w := range want {
		pre := "read"
		if strings.HasPrefix(in[i], "W") {
			pre = "write"
			for name, x := range map[string]string{"PAX headers": "PaxHeaders.0", "GNU long names": "././@LongLink",
				"global headers": "GlobalHead.0.0", "GNU headers": "ustar  \x00", "USTAR headers": "ustar\x0000"} {
				if n := strings.Count(w, hex.EncodeToString([]byte(x))); n > 0 {
					kinds["write: "+name] += n
				}
			}
		} else {
			entries += strings.Count(w, "{")
			if strings.HasSuffix(w, "E:EOF") {
				kinds["read: archive ends normally"]++
			}
		}
		for _, c := range classes {
			if n := strings.Count(w, c); n > 0 {
				kinds[pre+": "+c] += n
			}
		}
	}
	var ks []string
	for k, n := range kinds {
		ks = append(ks, fmt.Sprintf("%6d %s", n, k))
	}
	sort.Strings(ks)
	t.Logf("%d entries read; outcomes:\n%s", entries, strings.Join(ks, "\n"))
	if f := os.Getenv("TARDUMP"); f != "" { // the requests, to run tar_diff.c under sanitizers by hand
		os.WriteFile(f, []byte(strings.Join(in, "\n")+"\n"), 0o644)
	}
	got := diffRun(t, "tar", in)
	short := func(s string) string {
		if len(s) > 600 {
			return s[:600] + "..."
		}
		return s
	}
	bad := 0
	for i := range want {
		if got[i] != want[i] {
			bad++
			if bad <= 15 {
				k := 0
				for k < len(got[i]) && k < len(want[i]) && got[i][k] == want[i][k] {
					k++
				}
				t.Errorf("%s:\n got %s\nwant %s\nfirst difference at %d: got %q want %q", short(in[i]), short(got[i]), short(want[i]),
					k, got[i][max(0, k-100):min(len(got[i]), k+100)], want[i][max(0, k-100):min(len(want[i]), k+100)])
			}
		}
	}
	if bad > 0 {
		t.Errorf("%d differences in %d cases", bad, len(want))
	}
}
