package tests

import (
	"bytes"
	"encoding/base32"
	"encoding/binary"
	"encoding/csv"
	"encoding/hex"
	"fmt"
	"io"
	"math/rand"
	"strings"
	"testing"
)

var b32kinds = map[string]*base32.Encoding{"std": base32.StdEncoding, "hex": base32.HexEncoding,
	"rawstd": base32.StdEncoding.WithPadding(base32.NoPadding), "rawhex": base32.HexEncoding.WithPadding(base32.NoPadding)}

func goEnc2(f []string) string {
	switch f[0] {
	case "b32e":
		b, _ := hex.DecodeString(f[2])
		return b32kinds[f[1]].EncodeToString(b)
	case "b32d":
		s, _ := hex.DecodeString(f[2])
		out, err := b32kinds[f[1]].DecodeString(string(s))
		if err != nil {
			var off int64
			fmt.Sscan(strings.TrimPrefix(err.Error(), "illegal base32 data at input byte "), &off)
			return fmt.Sprintf("err %d %x", off, out)
		}
		return "ok " + hex.EncodeToString(out)
	case "uv":
		b, _ := hex.DecodeString(f[1])
		u, a := binary.Uvarint(b)
		s, c := binary.Varint(b)
		ru, err := binary.ReadUvarint(bytes.NewReader(b))
		st := "ok"
		switch {
		case err == io.EOF:
			st = "EOF"
		case err == io.ErrUnexpectedEOF:
			st = "unexpected"
		case err != nil:
			st = "overflow"
		}
		return fmt.Sprintf("%d %d %d %d %s %d", u, a, s, c, st, ru)
	case "pv":
		var v uint64
		fmt.Sscan(f[1], &v)
		return hex.EncodeToString(binary.AppendUvarint(nil, v)) + " " + hex.EncodeToString(binary.AppendVarint(nil, int64(v)))
	case "csv":
		in, _ := hex.DecodeString(f[6])
		r := csv.NewReader(bytes.NewReader(in))
		var comma, comment rune
		fmt.Sscan(f[1], &comma)
		fmt.Sscan(f[2], &comment)
		r.Comma, r.Comment = comma, comment
		fmt.Sscan(f[3], &r.FieldsPerRecord)
		r.LazyQuotes = f[4] == "1"
		r.TrimLeadingSpace = f[5] == "1"
		var out strings.Builder
		for i := 0; i < 1000; i++ {
			rec, err := r.Read()
			if err == io.EOF {
				break
			}
			if err != nil {
				msg := err.Error()
				out.WriteString("{err " + msg + "}")
			}
			if err == nil || len(rec) > 0 {
				out.WriteString("[")
				for k, x := range rec {
					if k > 0 {
						out.WriteString("|")
					}
					out.WriteString(hex.EncodeToString([]byte(x)))
				}
				out.WriteString("]")
			}
			if err != nil && strings.Contains(err.Error(), "invalid field or comment delimiter") {
				break
			}
		}
		return out.String()
	case "csvw":
		var b bytes.Buffer
		w := csv.NewWriter(&b)
		var comma rune
		fmt.Sscan(f[1], &comma)
		if comma != 0 {
			w.Comma = comma
		}
		w.UseCRLF = f[2] == "1"
		var rec []string
		for _, x := range f[3:] {
			s, _ := hex.DecodeString(x)
			rec = append(rec, string(s))
		}
		if err := w.Write(rec); err != nil {
			return "invalid"
		}
		w.Flush()
		return hex.EncodeToString(b.Bytes())
	}
	return "unknown"
}

func TestDiffBase32VarintCSV(t *testing.T) {
	r := rand.New(rand.NewSource(4648))
	var in []string
	kinds := []string{"std", "hex", "rawstd", "rawhex"}
	for i := 0; i < 600; i++ {
		b := randBytes(r, r.Intn(30))
		k := kinds[r.Intn(4)]
		in = append(in, "b32e\t"+k+"\t"+hex.EncodeToString(b))
		enc := []byte(b32kinds[k].EncodeToString(b))
		// valid, damaged, other padding, newlines inside
		switch r.Intn(5) {
		case 1:
			if len(enc) > 0 {
				enc[r.Intn(len(enc))] = "=A1!\xff\n0z"[r.Intn(8)]
			}
		case 2:
			if len(enc) > 0 {
				enc = enc[:r.Intn(len(enc))]
			}
		case 3:
			enc = append(enc, "===="[:r.Intn(4)]...)
		case 4:
			p := r.Intn(len(enc) + 1)
			enc = append(enc[:p], append([]byte("\r\n"), enc[p:]...)...)
		}
		in = append(in, "b32d\t"+kinds[r.Intn(4)]+"\t"+hex.EncodeToString(enc))
	}
	for i := 0; i < 400; i++ {
		b := randBytes(r, r.Intn(12))
		for j := range b {
			if r.Intn(3) > 0 {
				b[j] |= 0x80
			}
		}
		in = append(in, "uv\t"+hex.EncodeToString(b))
		in = append(in, fmt.Sprintf("pv\t%d", r.Uint64()>>uint(r.Intn(64))))
	}
	pieces := []string{"a", "b", ",", ",", "\"", "\"\"", " ", "\t", "\n", "\r\n", "\r", "#", ";", "é", "x y", "\"q\"", "€"}
	for i := 0; i < 1500; i++ {
		var s strings.Builder
		for j := r.Intn(30); j > 0; j-- {
			s.WriteString(pieces[r.Intn(len(pieces))])
		}
		comma := []rune{',', ',', ';', '\t', '€', '"', 0}[r.Intn(7)]
		comment := []rune{0, 0, '#', ',', ';'}[r.Intn(5)]
		fpr := []int{0, 0, -1, 2, 3}[r.Intn(5)]
		in = append(in, fmt.Sprintf("csv\t%d\t%d\t%d\t%d\t%d\t%s", comma, comment, fpr, r.Intn(2), r.Intn(2), hex.EncodeToString([]byte(s.String()))))
	}
	for i := 0; i < 500; i++ {
		f := []string{"csvw", fmt.Sprint([]rune{0, ',', ';', '\t', '€'}[r.Intn(5)]), fmt.Sprint(r.Intn(2))}
		for j := r.Intn(5); j > 0; j-- {
			var s strings.Builder
			for k := r.Intn(4); k > 0; k-- {
				s.WriteString(pieces[r.Intn(len(pieces))])
			}
			if r.Intn(10) == 0 {
				s.Reset()
				s.WriteString(`\.`)
			}
			f = append(f, hex.EncodeToString([]byte(s.String())))
		}
		in = append(in, strings.Join(f, "\t"))
	}
	var want []string
	for _, l := range in {
		want = append(want, goEnc2(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "enc2", in), want)
}
