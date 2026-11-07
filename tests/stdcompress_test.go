package tests

import (
	"bytes"
	"compress/flate"
	"compress/gzip"
	"compress/zlib"
	"encoding/hex"
	"errors"
	"fmt"
	"hash/crc32"
	"io"
	"math/rand"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

// compressSamples: text, binary, repetitive and random data of many sizes
func compressSamples(r *rand.Rand) [][]byte {
	var out [][]byte
	for _, n := range []int{0, 1, 2, 3, 4, 10, 100, 257, 258, 259, 1000} {
		b := make([]byte, n)
		for i := range b {
			b[i] = "ab"[r.Intn(2)]
		}
		out = append(out, b)
	}
	files, _ := filepath.Glob(filepath.Join(goRoot(), "src", "net", "http", "*.go"))
	for i, f := range files {
		if i%9 != 0 {
			continue
		}
		b, err := os.ReadFile(f)
		if err == nil {
			out = append(out, b)
		}
	}
	out = append(out, bytes.Repeat([]byte{0}, 300000), bytes.Repeat([]byte("abcdefghij"), 20000))
	for _, n := range []int{100, 5000, 70000, 200000} {
		b := make([]byte, n)
		r.Read(b)
		out = append(out, b)
	}
	// text with long-range repeats (beyond and within the window)
	words := strings.Fields("the quick brown fox jumps over lazy dog lorem ipsum dolor sit amet")
	var sb strings.Builder
	for sb.Len() < 150000 {
		sb.WriteString(words[r.Intn(len(words))])
		sb.WriteByte(" \n,."[r.Intn(4)])
	}
	out = append(out, []byte(sb.String()))
	// low-entropy bytes: many short matches
	b := make([]byte, 100000)
	for i := range b {
		b[i] = byte(r.Intn(4))
	}
	out = append(out, b)
	return out
}

func goRoot() string { return runtime.GOROOT() }

func crc32IEEE(b []byte) uint32 { return crc32.ChecksumIEEE(b) }

func errKind(err error) string {
	var ce flate.CorruptInputError
	switch {
	case err == nil:
		return "ok"
	case errors.As(err, &ce):
		return fmt.Sprintf("corrupt %d", int64(ce))
	case err == io.ErrUnexpectedEOF:
		return "eof"
	case err == gzip.ErrChecksum, err == zlib.ErrChecksum:
		return "checksum"
	case err == gzip.ErrHeader, err == zlib.ErrHeader:
		return "header"
	case err == zlib.ErrDictionary:
		return "dict"
	case err == io.EOF:
		return "empty"
	}
	return "other " + err.Error()
}

// goDecode decodes like the C program: "kind datahex"
func goDecode(op byte, data []byte) string {
	var r io.Reader
	var err error
	src := bytes.NewReader(data)
	switch op {
	case 'I':
		r = flate.NewReader(src)
	case 'G':
		r, err = gzip.NewReader(src)
	case 'Z':
		r, err = zlib.NewReader(src)
	}
	if err != nil {
		k := errKind(err)
		if op == 'G' && strings.HasPrefix(k, "corrupt") || op == 'Z' && strings.HasPrefix(k, "corrupt") {
			k = "corrupt"
		}
		return k + " "
	}
	out, err := io.ReadAll(r)
	k := errKind(err)
	if op != 'I' && strings.HasPrefix(k, "corrupt") {
		k = "corrupt"
	}
	return k + " " + hex.EncodeToString(out)
}

// mutations of a valid stream: truncations, flipped bits, changed bytes
func mutations(r *rand.Rand, s []byte, n int) [][]byte {
	var out [][]byte
	for i := 0; i < n && len(s) > 0; i++ {
		m := append([]byte(nil), s...)
		switch i % 4 {
		case 0:
			m = m[:r.Intn(len(m))]
		case 1:
			k := r.Intn(len(m))
			m[k] ^= 1 << r.Intn(8)
		case 2:
			k := r.Intn(len(m))
			m[k] = byte(r.Intn(256))
		case 3:
			// damage near the start, where the block headers are
			k := r.Intn(min(len(m), 40))
			m[k] ^= 1 << r.Intn(8)
		}
		out = append(out, m)
	}
	return out
}

func TestCompressDecodeAgainstGo(t *testing.T) {
	r := rand.New(rand.NewSource(21))
	var in, want []string
	add := func(op byte, data []byte) {
		in = append(in, string(op)+"\t"+hex.EncodeToString(data))
		want = append(want, goDecode(op, data))
	}
	for _, s := range compressSamples(r) {
		for _, level := range []int{-2, 0, 1, 2, 5, 6, 9} {
			var fb, gb, zb bytes.Buffer
			fw, _ := flate.NewWriter(&fb, level)
			gw, _ := gzip.NewWriterLevel(&gb, level)
			gw.Name, gw.Comment, gw.Extra = "file.txt", "comment é", []byte{1, 2, 3}
			zw, _ := zlib.NewWriterLevel(&zb, level)
			cut := r.Intn(len(s) + 1)
			for _, w := range []interface {
				io.Writer
				Flush() error
				Close() error
			}{fw, gw, zw} {
				w.Write(s[:cut])
				if level%2 == 0 {
					w.Flush()
				}
				w.Write(s[cut:])
				w.Close()
			}
			add('I', fb.Bytes())
			add('G', gb.Bytes())
			add('Z', zb.Bytes())
			if len(s) < 20000 {
				for _, m := range mutations(r, fb.Bytes(), 12) {
					add('I', m)
				}
				for _, m := range mutations(r, gb.Bytes(), 6) {
					add('G', m)
				}
				for _, m := range mutations(r, zb.Bytes(), 6) {
					add('Z', m)
				}
			}
		}
	}
	// gzip: multistream, empty input, trailing garbage, header CRC
	var a, b bytes.Buffer
	w1 := gzip.NewWriter(&a)
	w1.Write([]byte("first "))
	w1.Close()
	w2 := gzip.NewWriter(&b)
	w2.Write([]byte("second"))
	w2.Close()
	two := append(append([]byte(nil), a.Bytes()...), b.Bytes()...)
	add('G', two)
	add('G', nil)
	add('G', append(append([]byte(nil), a.Bytes()...), 0x1f))
	add('G', append(append([]byte(nil), a.Bytes()...), "garbage!garbage!"...))
	hcrc := []byte{0x1f, 0x8b, 8, 2 | 8, 0, 0, 0, 0, 0, 255, 'n', 0}
	sum := crc32IEEE(hcrc)
	hcrc = append(hcrc, byte(sum), byte(sum>>8))
	add('G', append(hcrc, a.Bytes()[10:]...))
	hcrc[len(hcrc)-1] ^= 1
	add('G', append(hcrc, a.Bytes()[10:]...))
	long := append([]byte{0x1f, 0x8b, 8, 8, 0, 0, 0, 0, 0, 255}, bytes.Repeat([]byte{'x'}, 600)...)
	add('G', append(long, 0))
	// zlib: dictionary required, bad headers
	var zd bytes.Buffer
	zw, _ := zlib.NewWriterLevelDict(&zd, 6, []byte("dictionary"))
	zw.Write([]byte("dictionary words"))
	zw.Close()
	add('Z', zd.Bytes())
	add('Z', []byte{0x78})
	add('Z', []byte{0x79, 0x9c})
	add('Z', []byte{0x88, 0x98})
	// hand-made DEFLATE streams: stored blocks, bad lengths, distances too far,
	// incomplete and over-subscribed codes, reserved block type
	for _, h := range []string{"010000ffff", "0100", "0105", "0105fffa68656c6c6f", "01050000ff", "07", "0300", "03",
		"6360000000", "edc1010d000000c2a0f74f6d0e37a00000000000000000000000000000000000000000000000000000008000",
		"6300800000", "05c0310100000000", "fbffffffffff", "ed9d09", "6362606164626661656367e0e4e2e6e1e5e3e7"} {
		d, _ := hex.DecodeString(h)
		add('I', d)
	}
	for i := 0; i < 3000; i++ {
		d := make([]byte, 1+r.Intn(30))
		r.Read(d)
		add('I', d)
	}
	diffCompare(t, in, diffRun(t, "compress", in), want)
}

func TestCompressEncodeAgainstGo(t *testing.T) {
	r := rand.New(rand.NewSource(22))
	var in []string
	var samples [][]byte
	var cuts []int
	var ops []byte
	var levels []int
	for _, s := range compressSamples(r) {
		for _, level := range []int{-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9} {
			for _, op := range []byte{'D', 'g', 'z'} {
				if op != 'D' && level%3 != 0 {
					continue
				}
				cut := len(s)
				if r.Intn(3) == 0 {
					cut = r.Intn(len(s) + 1)
				}
				in = append(in, fmt.Sprintf("%c\t%d\t%d\t%s", op, level, cut, hex.EncodeToString(s)))
				samples, cuts, ops, levels = append(samples, s), append(cuts, cut), append(ops, op), append(levels, level)
			}
		}
	}
	got := diffRun(t, "compress", in)
	bad := 0
	var ours, theirs [10]int
	for i, g := range got {
		data, err := hex.DecodeString(g)
		if err != nil {
			t.Errorf("%c level %d: %s", ops[i], levels[i], g)
			continue
		}
		var rd io.Reader
		switch ops[i] {
		case 'D':
			rd = flate.NewReader(bytes.NewReader(data))
		case 'g':
			gr, err := gzip.NewReader(bytes.NewReader(data))
			if err != nil {
				t.Errorf("gzip header: %v", err)
				continue
			}
			if gr.Name != "nameé.txt" || gr.Comment != "c" || string(gr.Extra) != "xy" || gr.ModTime.Unix() != 1234567890 || gr.OS != 3 {
				t.Errorf("gzip header %+v", gr.Header)
			}
			rd = gr
		case 'z':
			rd, err = zlib.NewReader(bytes.NewReader(data))
			if err != nil {
				t.Errorf("zlib header: %v", err)
				continue
			}
		}
		out, err := io.ReadAll(rd)
		if err != nil || !bytes.Equal(out, samples[i]) {
			bad++
			if bad < 10 {
				t.Errorf("%c level %d len %d cut %d: Go decodes %d bytes, %v", ops[i], levels[i], len(samples[i]), cuts[i], len(out), err)
			}
			continue
		}
		// compare the size with Go's for whole writes
		if ops[i] == 'D' && cuts[i] == len(samples[i]) && levels[i] >= 1 {
			var b bytes.Buffer
			w, _ := flate.NewWriter(&b, levels[i])
			w.Write(samples[i])
			w.Close()
			ours[levels[i]] += len(data)
			theirs[levels[i]] += b.Len()
		}
	}
	for l := 1; l <= 9; l++ {
		t.Logf("level %d: %d bytes, Go %d (%.3f)", l, ours[l], theirs[l], float64(ours[l])/float64(theirs[l]))
		if float64(ours[l]) > 1.1*float64(theirs[l]) {
			t.Errorf("level %d compresses worse than Go's by more than 10%%", l)
		}
	}
}
