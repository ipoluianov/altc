package tests

import (
	"bufio"
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"io"
	"math/rand"
	"mime"
	"net/mail"
	"net/textproto"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"testing"
	"time"
)

// goTestStrings returns the string literals of a test file of Go's
// standard library (its cases become inputs here)
func goTestStrings(t *testing.T, rel string, minimum int) []string {
	t.Helper()
	f, err := parser.ParseFile(token.NewFileSet(), filepath.Join(runtime.GOROOT(), "src", rel), nil, 0)
	if err != nil {
		t.Fatal(err)
	}
	seen := map[string]bool{}
	var out []string
	ast.Inspect(f, func(n ast.Node) bool {
		if l, ok := n.(*ast.BasicLit); ok && l.Kind == token.STRING {
			if s, err := strconv.Unquote(l.Value); err == nil && !seen[s] {
				seen[s] = true
				out = append(out, s)
			}
		}
		return true
	})
	if len(out) < minimum {
		t.Fatalf("%s: only %d strings", rel, len(out))
	}
	return out
}

func mhx(s string) string { return hex.EncodeToString([]byte(s)) }
func munhx(s string) string { b, _ := hex.DecodeString(s); return string(b) }

func upperASCII(b []byte) []byte {
	out := make([]byte, len(b))
	for i, c := range b {
		if 'a' <= c && c <= 'z' {
			c -= 32
		}
		out[i] = c
	}
	return out
}

// the charset reader of the tests: "x-upper" upper-cases ASCII, "x-fail" fails
var testWordDecoder = &mime.WordDecoder{CharsetReader: func(charset string, input io.Reader) (io.Reader, error) {
	if charset == "x-upper" {
		b, _ := io.ReadAll(input)
		return bytes.NewReader(upperASCII(b)), nil
	}
	if charset == "x-fail" {
		return nil, errors.New("boom")
	}
	return nil, fmt.Errorf("unknown charset %s", charset)
}}

// ---- mime encoded-words ----

func goMimeWord(f []string) string {
	switch f[0] {
	case "enc":
		e := mime.QEncoding
		if f[1] == "b" {
			e = mime.BEncoding
		}
		return mhx(e.Encode(munhx(f[2]), munhx(f[3])))
	case "dec", "hdr":
		d := &mime.WordDecoder{}
		if f[1] == "1" {
			d = testWordDecoder
		}
		var s string
		var err error
		if f[0] == "dec" {
			s, err = d.Decode(munhx(f[2]))
		} else {
			s, err = d.DecodeHeader(munhx(f[2]))
		}
		if err != nil {
			return "err " + err.Error()
		}
		return "ok " + mhx(s)
	}
	return "?"
}

func randText(r *rand.Rand, pieces []string, max int) string {
	var b strings.Builder
	for k := r.Intn(max + 1); k > 0; k-- {
		if r.Intn(8) == 0 {
			b.WriteByte(byte(r.Intn(256)))
		} else {
			b.WriteString(pieces[r.Intn(len(pieces))])
		}
	}
	return b.String()
}

func TestDiffMimeWord(t *testing.T) {
	r := rand.New(rand.NewSource(2047))
	textPieces := []string{"a", "Hello", " ", "\t", "=", "?", "_", "é", "日本語", "😀", "\xff", "\x00", "\r\n", "~", "\x7f", ".", "\"",
		"abcdefghij", "€", "\u00a0", "ü"}
	charsets := []string{"UTF-8", "utf-8", "Utf-8", "iso-8859-1", "us-ascii", "x-upper", "", "?", "windows-1252"}
	var in []string
	for i := 0; i < 6000; i++ {
		s := randText(r, textPieces, 40)
		if r.Intn(10) == 0 {
			s = strings.Repeat(textPieces[r.Intn(len(textPieces))], r.Intn(80))
		}
		in = append(in, "enc\t"+[]string{"b", "q"}[r.Intn(2)]+"\t"+mhx(charsets[r.Intn(len(charsets))])+"\t"+mhx(s))
	}
	// decoding: words built from pieces, valid and broken
	wordPieces := []string{"=?", "?=", "?", "utf-8", "UTF-8", "iso-8859-1", "ISO-8859-1", "us-ascii", "x-upper", "x-fail", "koi8-r", "q", "Q",
		"b", "B", "x", "=C3=A9", "=c3=a9", "=E9", "=e", "=G1", "=1g", "_", "a", " ", "\t", "\r\n ", "\n", "w6k=", "w6", "w6k", "====",
		"SGVsbG8=", "SGVsbG8", "\r", "\n", "!", "~", "\x01", "é", "\xff", "Zm9v", "Zm9vYg==", "Zm9vYg=", "Zm9vYmE=", "Zm9vYmE", "=\n=",
		"=3D", "=?utf-8?q?", "=?utf-8?b?", "?= =?utf-8?q?", "?=  =?UTF-8?Q?", "x y"}
	mkWord := func() string {
		switch r.Intn(4) {
		case 0:
			return randText(r, wordPieces, 8)
		case 1:
			cs := []string{"utf-8", "UTF-8", "iso-8859-1", "us-ascii", "x-upper", "x-fail", "koi8-r", "", "Utf-8"}[r.Intn(9)]
			enc := []string{"q", "Q", "b", "B", "x", "qq", ""}[r.Intn(7)]
			return "=?" + cs + "?" + enc + "?" + randText(r, wordPieces[16:], 5) + "?="
		}
		cs := []string{"utf-8", "iso-8859-1", "us-ascii", "x-upper", "x-fail", "koi8-r"}[r.Intn(6)]
		w := (mime.WordEncoder([]byte{'q', 'b'}[r.Intn(2)])).Encode(cs, randText(r, textPieces, 12))
		if r.Intn(4) == 0 && len(w) > 0 {
			i := r.Intn(len(w))
			w = w[:i] + w[i+1:]
		}
		return w
	}
	for i := 0; i < 8000; i++ {
		w := mkWord()
		in = append(in, "dec\t"+strconv.Itoa(r.Intn(2))+"\t"+mhx(w))
		var h strings.Builder
		for k := r.Intn(5); k >= 0; k-- {
			h.WriteString(mkWord())
			h.WriteString([]string{" ", "", "  ", "\t", "x", " a ", "\r\n "}[r.Intn(7)])
		}
		in = append(in, "hdr\t"+strconv.Itoa(r.Intn(2))+"\t"+mhx(h.String()))
	}
	for _, s := range goTestStrings(t, "mime/encodedword_test.go", 20) {
		for _, fl := range []string{"0", "1"} {
			in = append(in, "dec\t"+fl+"\t"+mhx(s), "hdr\t"+fl+"\t"+mhx(s))
		}
		in = append(in, "enc\tq\t"+mhx("UTF-8")+"\t"+mhx(s), "enc\tb\t"+mhx("UTF-8")+"\t"+mhx(s), "enc\tq\t"+mhx("ISO-8859-1")+"\t"+mhx(s))
	}
	var want []string
	for _, l := range in {
		want = append(want, goMimeWord(strings.Split(l, "\t")))
	}
	logOutcomes(t, in, want)
	diffCompare(t, in, diffRun(t, "mimeword", in), want)
}

// ---- textproto ----

func tpErr(err error) string {
	if err == nil {
		return ""
	}
	return err.Error()
}

func tpRest(r *textproto.Reader, continued bool) string {
	var b strings.Builder
	for i := 0; i < 50; i++ {
		var l string
		var err error
		if continued {
			l, err = r.ReadContinuedLine()
		} else {
			l, err = r.ReadLine()
		}
		if err != nil {
			b.WriteString("E:" + err.Error())
			break
		}
		b.WriteString(mhx(l) + ";")
	}
	return b.String()
}

func goTextproto(f []string) string {
	newR := func(s string) *textproto.Reader { return textproto.NewReader(bufio.NewReader(strings.NewReader(munhx(s)))) }
	switch f[0] {
	case "rl", "rcl":
		return tpRest(newR(f[1]), f[0] == "rcl")
	case "code", "resp":
		expect, _ := strconv.Atoi(f[1])
		r := newR(f[2])
		var b strings.Builder
		for i := 0; i < 20; i++ {
			var code int
			var msg string
			var err error
			if f[0] == "code" {
				code, msg, err = r.ReadCodeLine(expect)
			} else {
				code, msg, err = r.ReadResponse(expect)
			}
			fmt.Fprintf(&b, "%d|%s|%s;", code, mhx(msg), tpErr(err))
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				break
			}
		}
		return b.String()
	case "dot":
		r := newR(f[1])
		d, err := r.ReadDotBytes()
		return mhx(string(d)) + "|" + tpErr(err) + "|" + tpRest(r, false)
	case "dotl":
		r := newR(f[1])
		lines, err := r.ReadDotLines()
		var b strings.Builder
		for _, l := range lines {
			b.WriteString(mhx(l) + ",")
		}
		return b.String() + "|" + tpErr(err) + "|" + tpRest(r, false)
	case "dotp":
		n, _ := strconv.Atoi(f[1])
		r := newR(f[2])
		d := r.DotReader()
		var got []byte
		buf := make([]byte, 3)
		for len(got) < n {
			k := min(n-len(got), 3)
			m, err := d.Read(buf[:k])
			got = append(got, buf[:m]...)
			if err != nil {
				break
			}
		}
		return mhx(string(got)) + "|" + tpRest(r, false)
	case "mh":
		r := newR(f[1])
		h, err := r.ReadMIMEHeader()
		var keys []string
		for k := range h {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		var b strings.Builder
		for i, k := range keys {
			if i > 0 {
				b.WriteString(" ")
			}
			b.WriteString(mhx(k) + "=")
			for j, v := range h[k] {
				if j > 0 {
					b.WriteString(",")
				}
				b.WriteString(mhx(v))
			}
		}
		if len(keys) > 0 {
			b.WriteString("|g:" + mhx(h.Get(strings.ToLower(keys[0]))))
		}
		return b.String() + "|" + tpErr(err) + "|" + tpRest(r, false)
	case "canon":
		return mhx(textproto.CanonicalMIMEHeaderKey(munhx(f[1])))
	case "trim":
		return mhx(textproto.TrimString(munhx(f[1])))
	case "pl", "dw":
		var out bytes.Buffer
		bw := bufio.NewWriter(&out)
		w := textproto.NewWriter(bw)
		if f[0] == "pl" {
			w.PrintfLine("%s", munhx(f[1]))
		} else {
			d := w.DotWriter()
			line := false
			for _, c := range f[1:] {
				if c == "L" {
					line = true
					continue
				}
				d.Write([]byte(munhx(c)))
			}
			if line {
				w.PrintfLine("x")
			} else {
				d.Close()
			}
		}
		return mhx(out.String())
	}
	return "?"
}

func TestDiffTextproto(t *testing.T) {
	r := rand.New(rand.NewSource(5321))
	hdrPieces := []string{"Key: value\r\n", "key: v\n", "My-Key: Value 1\r\n", "content-TYPE: text/plain\r\n", " continued\r\n", "\tcont\n",
		"  \r\n", "\r\n", "\n", "bad key: v\r\n", "Key:\x01v\r\n", ":v\r\n", "Key v\r\n", "\r", "Ke\x80y: v\r\n", "Key: v\x80\r\n",
		"Key: a\rb\r\n", "X-Ünicode: 1\r\n", "a-b-c: 1\r\n", "A_B: 2\r\n", "Foo:bar\r\n", "Foo :bar\r\n", "Foo\t: bar\r\n", "-: x\r\n",
		"Key: \t value \t \r\n", "Host: example.com\r\n", "\r\n\r\n", "body", "Set-Cookie: a=b\r\n", "k:\r\n", " leading: x\r\n",
		"Key: v\r\n  ", "x: y\r\r\n", "\x00: v\r\n"}
	linePieces := []string{"a", "line", "\r\n", "\n", "\r", " ", "\t", "  x\r\n", "\tcont\n", ".", "..", ".\r\n", ".\n", "..x\r\n", "\x00", "é",
		"250-first\r\n", "250 last\r\n", "250-a\n", "251 b\r\n", "220 ready\r\n", "550 no\r\n", "25", "abc ", "099 x\r\n", "+20 x\r\n",
		"-20 x\r\n", "2500 x\r\n", "250x\r\n", "250\r\n", "250 \r\n", "250-\r\n", "200-a\r\n", "200 b\r\n", "123-\r\n", "500 \r\n", "\xff"}
	long := strings.Repeat("y", 5000)
	var in []string
	for i := 0; i < 4000; i++ {
		h := randText(r, hdrPieces, 10)
		if r.Intn(20) == 0 {
			h = "Long: " + long + "\r\n" + h
		}
		in = append(in, "mh\t"+mhx(h))
		l := randText(r, linePieces, 14)
		if r.Intn(25) == 0 {
			l = long + "\r\n" + l + long[:4095] + "\r\n" + long[:4094] + "\r\r\n"
		}
		for _, op := range []string{"rl", "rcl", "dot", "dotl"} {
			in = append(in, op+"\t"+mhx(l))
		}
		in = append(in, "rcl\t"+mhx(h))
		in = append(in, fmt.Sprintf("dotp\t%d\t%s", r.Intn(12), mhx(l)))
		ex := []int{0, 0, 2, 25, 250, 220, 5, 55, 550, 1000, -1}[r.Intn(11)]
		in = append(in, fmt.Sprintf("code\t%d\t%s", ex, mhx(l)), fmt.Sprintf("resp\t%d\t%s", ex, mhx(l)))
		k := randText(r, []string{"a", "B", "-", "x-y", "Content-type", " ", "é", "_", "1", "\x7f", ":", "ACCEPT", "e", "\t"}, 6)
		in = append(in, "canon\t"+mhx(k), "trim\t"+mhx(randText(r, []string{" ", "\t", "\n", "\r", "a b", "\v", "x"}, 6)))
		pl := strings.ReplaceAll(randText(r, linePieces, 6), "\x00", "")
		in = append(in, "pl\t"+mhx(pl))
		dw := "dw"
		for c := r.Intn(4); c >= 0; c-- {
			dw += "\t" + mhx(randText(r, linePieces, 6))
		}
		if r.Intn(4) == 0 {
			dw += "\tL"
		}
		in = append(in, dw)
	}
	in = append(in, "dw", "dw\t", "dw\tL", "rl\t", "mh\t", "dot\t", "dotl\t")
	for _, s := range goTestStrings(t, "net/textproto/reader_test.go", 20) {
		for _, op := range []string{"rl", "rcl", "dot", "dotl", "mh", "canon"} {
			in = append(in, op+"\t"+mhx(s))
		}
		in = append(in, "code\t0\t"+mhx(s), "resp\t0\t"+mhx(s), "resp\t250\t"+mhx(s), "code\t2\t"+mhx(s))
	}
	for _, s := range goTestStrings(t, "net/textproto/writer_test.go", 5) {
		in = append(in, "dw\t"+mhx(s))
	}
	var want []string
	for _, l := range in {
		want = append(want, goTextproto(strings.Split(l, "\t")))
	}
	logOutcomes(t, in, want)
	diffCompare(t, in, diffRun(t, "textproto", in), want)
}

// ---- mail ----

func goAddrStr(a *mail.Address) string {
	return mhx(a.Name) + "|" + mhx(a.Address) + "|" + mhx(a.String())
}

func goAddrList(l []*mail.Address, err error) string {
	if err != nil {
		return "err " + err.Error()
	}
	var b strings.Builder
	b.WriteString("ok")
	for i, a := range l {
		if i == 0 {
			b.WriteString(" ")
		} else {
			b.WriteString(";")
		}
		b.WriteString(goAddrStr(a))
	}
	return b.String()
}

func goDate(tm time.Time, err error) string {
	if err != nil {
		return "err " + err.Error()
	}
	name, off := tm.Zone()
	return fmt.Sprintf("ok %d %d %d %s", tm.Unix(), tm.Nanosecond(), off, name)
}

func goMail(f []string) string {
	switch f[0] {
	case "addr", "list":
		p := &mail.AddressParser{}
		if f[1] == "1" {
			p.WordDecoder = testWordDecoder
		}
		if f[0] == "list" {
			return goAddrList(p.ParseList(munhx(f[2])))
		}
		a, err := p.Parse(munhx(f[2]))
		if err != nil {
			return "err " + err.Error()
		}
		out := "ok " + goAddrStr(a)
		b, err := p.Parse(a.String())
		if err != nil {
			return out + "|err " + err.Error()
		}
		return out + "|ok " + mhx(b.Name) + "|" + mhx(b.Address)
	case "date":
		return goDate(mail.ParseDate(munhx(f[1])))
	case "fmt":
		return mhx((&mail.Address{Name: munhx(f[1]), Address: munhx(f[2])}).String())
	case "msg":
		m, err := mail.ReadMessage(strings.NewReader(munhx(f[1])))
		if err != nil {
			return "err " + err.Error()
		}
		var keys []string
		for k := range m.Header {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		var b strings.Builder
		b.WriteString("ok ")
		for i, k := range keys {
			if i > 0 {
				b.WriteString(" ")
			}
			b.WriteString(mhx(k) + "=")
			for j, v := range m.Header[k] {
				if j > 0 {
					b.WriteString(",")
				}
				b.WriteString(mhx(v))
			}
		}
		body, _ := io.ReadAll(m.Body)
		b.WriteString("|" + mhx(string(body)))
		b.WriteString("|date:" + goDate(m.Header.Date()))
		b.WriteString("|from:" + goAddrList(m.Header.AddressList("From")))
		b.WriteString("|to:" + goAddrList(m.Header.AddressList("to")))
		return b.String()
	}
	return "?"
}

func TestDiffMail(t *testing.T) {
	time.Local = time.UTC // zone abbreviations are looked up in Local
	r := rand.New(rand.NewSource(5322))
	pieces := []string{"a", "b.c", "john", "Doe", "\"q s\"", "\"\\\"x\"", "\"\"", "\"", "@", "<", ">", "(c)", "(nested (x) y)", "(", ")",
		",", ";", ":", "[1.2.3.4]", "[IPv6:::1]", "[IPv6:1.2.3.4]", "[::ffff:1.2.3.4]", "[x]", "[300.1.1.1]", "[fe80::1%eth0]", "[", "]",
		"=?utf-8?q?J=C3=B6rg?=", "=?iso-8859-1?q?caf=e9?=", "=?utf-8?b?w6k=?=", "=?x-unknown?q?a?=", "=?x-upper?q?abc?=", "=?x-fail?q?a?=",
		"=?utf-8?q?bad=?=", "=?US-ASCII?Q?Keith_Moore?=", " ", "  ", "\t", "é", "日本", "\xff", "Name", "group", "..", ".", "\\", "\x01",
		"example.com", "a.b@c.d", "x@y", "!#$%&'*+-/=?^_`{|}~", "\"a\\\tb\"", "\"\\\x01\"", "\r\n", "\x7f", "<a@b>", "(\\))", "(\\"}
	structured := func() string {
		local := []string{"john", "j.doe", "\"john doe\"", "\"a@b\"", "\"\"", ".x", "x.", "a..b", "é", "\"\\\\\"", "a+b", "日本"}[r.Intn(12)]
		dom := []string{"example.com", "[1.2.3.4]", "[IPv6:::1]", "ex..com", "x", "[1.2.3]", "é.com", ".com"}[r.Intn(8)]
		spec := local + "@" + dom
		switch r.Intn(7) {
		case 0:
			return spec
		case 1:
			return spec + " (" + []string{"Name", "=?utf-8?q?J=C3=B6rg?= x", "a (b) c", "=?x-fail?q?a?=", "", "unclosed"}[r.Intn(6)] + ")"
		case 2:
			name := []string{"John Doe", "\"John Doe\"", "=?utf-8?q?J=C3=B6rg?= =?utf-8?q?_Doe?=", "Jörg", "\"a\\\"b\"", "x.y",
				"=?utf-8?b?w6k=?= Dupont", "Dr. Who", "a (comment) b", "=?x-upper?q?abc?=", "", "\"\""}[r.Intn(12)]
			return name + " <" + spec + ">"
		case 3:
			return "group: " + spec + ", B <" + spec + ">;"
		case 4:
			return "empty:;"
		case 5:
			return "<" + spec + ">"
		}
		return spec + ", " + spec
	}
	var in []string
	for i := 0; i < 6000; i++ {
		var s string
		if r.Intn(2) == 0 {
			s = randText(r, pieces, 10)
		} else {
			s = structured()
			if r.Intn(3) == 0 {
				s += []string{", ", ",", " , ", ",,"}[r.Intn(4)] + structured()
			}
			if r.Intn(4) == 0 && len(s) > 0 {
				k := r.Intn(len(s))
				s = s[:k] + pieces[r.Intn(len(pieces))] + s[k:]
			}
		}
		fl := strconv.Itoa(r.Intn(2))
		in = append(in, "addr\t"+fl+"\t"+mhx(s), "list\t"+fl+"\t"+mhx(s))
	}
	// valid addresses: Address.String of random names and addresses, lists of them
	validLocal := []string{"john", "j.doe", "john doe", "a@b", "a\\b", "x\"y", "é", "日本", "a+b", "!#$%&'*+-/=?^_`{|}~", "o'neil", "a..b"}
	validDom := []string{"example.com", "[1.2.3.4]", "[IPv6:::1]", "[IPv6:2001:db8::1]", "x", "é.com", "a-b.c-d.org"}
	validName := []string{"", "John Doe", "Jörg Doe", "a \"q\" b", "Dr. Who", "日本語の名前", "x(y)z", "a\\b", strings.Repeat("ü", 40),
		"Ünïcødé, Ltd.", "tab\there", "=?utf-8?q?x?=", "Mr. O'Neil"}
	for i := 0; i < 3000; i++ {
		var parts []string
		for k := r.Intn(3); k >= 0; k-- {
			a := &mail.Address{Name: validName[r.Intn(len(validName))],
				Address: validLocal[r.Intn(len(validLocal))] + "@" + validDom[r.Intn(len(validDom))]}
			parts = append(parts, a.String())
		}
		s := parts[0]
		in = append(in, "addr\t"+strconv.Itoa(r.Intn(2))+"\t"+mhx(s))
		l := strings.Join(parts, []string{", ", ",", " ,, "}[r.Intn(3)])
		if r.Intn(3) == 0 {
			l = "grp: " + l + "; , x@y (c)"
		}
		in = append(in, "list\t"+strconv.Itoa(r.Intn(2))+"\t"+mhx(l))
	}
	// Address.String of arbitrary names and addresses
	namePieces := []string{"John", " ", "Doe", "\"", "\\", "é", "日本語", "\xff", ",", ".", "(", ")", "<", ">", "@", ":", ";", "\t", "\x01",
		"\x7f", "=?", "?=", "_", "a", strings.Repeat("ü", 30), strings.Repeat("x", 50), "'", "#", "~"}
	for i := 0; i < 4000; i++ {
		name := randText(r, namePieces, 8)
		addr := randText(r, []string{"a", "b", ".", "@", "\"", " ", "é", "\\", "x.y", "(", "\xff", "example.com"}, 6)
		in = append(in, "fmt\t"+mhx(name)+"\t"+mhx(addr))
	}
	// dates: Go formats them, then mutations
	dateLayouts := []string{time.RFC1123Z, time.RFC1123, time.RFC822, time.RFC822Z, "Mon, 2 Jan 2006 15:04:05 -0700 (MST)", "2 Jan 06 15:04 -0700",
		"Mon, 02 Jan 2006 15:04 MST", "02 Jan 2006 15:04:05 UT", "Mon, 2 Jan 2006 15:04:05 GMT", "Monday, 02 Jan 2006 15:04:05 -0700",
		time.ANSIC, "2 Jan 2006 15:04:05 MST", "Mon, 02 Jan 2006 15:04:05 -07:00"}
	zones := []*time.Location{time.UTC, time.FixedZone("PST", -8*3600), time.FixedZone("", 5*3600+30*60), time.FixedZone("CEST", 7200),
		time.FixedZone("MDT", -6*3600)}
	datePieces := []string{" ", "(comment)", "(", ")", "\r\n", "\r", "\n", "+", "-", "T", "x", "(a (b) c)", "  ", "0", "\t", "GMT", "+0000"}
	for i := 0; i < 5000; i++ {
		tm := time.Unix(r.Int63n(9e9)-2e9, 0).In(zones[r.Intn(len(zones))])
		s := tm.Format(dateLayouts[r.Intn(len(dateLayouts))])
		switch r.Intn(6) {
		case 0:
			k := r.Intn(len(s) + 1)
			s = s[:k] + datePieces[r.Intn(len(datePieces))] + s[k:]
		case 1:
			if len(s) > 0 {
				k := r.Intn(len(s))
				s = s[:k] + s[k+1:]
			}
		case 2:
			s = datePieces[r.Intn(len(datePieces))] + s + datePieces[r.Intn(len(datePieces))]
		}
		in = append(in, "date\t"+mhx(s))
	}
	// messages
	hdrs := []string{"From: John Doe <jdoe@machine.example>\r\n", "To: Mary Smith <mary@example.net>, a@b\r\n", "to: x@y\n",
		"Subject: Saying Hello\r\n", "Date: Fri, 21 Nov 1997 09:55:06 -0600\r\n", "Date: bad\r\n", "Message-ID: <1234@local.machine.example>\r\n",
		" folded\r\n", "\tfolded\n", "X: \r\n", ": empty key\r\n", "bad line\r\n", "Key With Space: v\r\n", "k\x01: v\r\n", "From:\r\n",
		"FROM: =?utf-8?q?J=C3=B6rg?= <j@x>\r\n", "Received: from x\r\n", "Received: from y\r\n", "From ", "\xff: x\r\n", "a:b\r\n"}
	for i := 0; i < 3000; i++ {
		var m strings.Builder
		if r.Intn(10) == 0 {
			m.WriteString(" leading\r\n")
		}
		for k := r.Intn(6); k > 0; k-- {
			m.WriteString(hdrs[r.Intn(len(hdrs))])
		}
		if r.Intn(5) > 0 {
			m.WriteString([]string{"\r\n", "\n"}[r.Intn(2)])
			m.WriteString(randText(r, []string{"body", "\r\n", ".", "From: x\r\n"}, 5))
		}
		in = append(in, "msg\t"+mhx(m.String()))
	}
	for _, s := range goTestStrings(t, "net/mail/message_test.go", 20) {
		in = append(in, "addr\t0\t"+mhx(s), "list\t0\t"+mhx(s), "addr\t1\t"+mhx(s), "date\t"+mhx(s), "msg\t"+mhx(s), "fmt\t"+mhx(s)+"\t"+mhx("a@b"),
			"fmt\t"+mhx("N")+"\t"+mhx(s))
	}
	var want []string
	for _, l := range in {
		want = append(want, goMail(strings.Split(l, "\t")))
	}
	logOutcomes(t, in, want)
	diffCompare(t, in, diffRun(t, "mail", in), want)
}

// logOutcomes logs how many answers of each request kind succeeded
func logOutcomes(t *testing.T, in, want []string) {
	ok, all := map[string]int{}, map[string]int{}
	for i, l := range in {
		op, _, _ := strings.Cut(l, "\t")
		all[op]++
		if strings.HasPrefix(want[i], "ok") {
			ok[op]++
		}
	}
	var ops []string
	for op := range all {
		ops = append(ops, op)
	}
	sort.Strings(ops)
	for _, op := range ops {
		t.Logf("%s: %d cases, %d ok", op, all[op], ok[op])
	}
}
