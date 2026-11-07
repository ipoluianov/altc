package tests

import (
	"bytes"
	"encoding/hex"
	"io"
	"math/rand"
	"mime"
	"mime/multipart"
	"mime/quotedprintable"
	"net/textproto"
	"sort"
	"strings"
	"testing"
)

func goMime(f []string) string {
	unhex := func(s string) string { b, _ := hex.DecodeString(s); return string(b) }
	switch f[0] {
	case "mt":
		t, p, err := mime.ParseMediaType(unhex(f[1]))
		if err != nil {
			return "err " + err.Error() + " [" + t + "]"
		}
		var keys []string
		for k := range p {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		out := t
		for _, k := range keys {
			out += "|" + k + "=" + hex.EncodeToString([]byte(p[k]))
		}
		return out
	case "fmt":
		p := map[string]string{}
		for i := 2; i+1 < len(f); i += 2 {
			p[unhex(f[i])] = unhex(f[i+1])
		}
		s := mime.FormatMediaType(unhex(f[1]), p)
		if s == "" {
			return "invalid"
		}
		return hex.EncodeToString([]byte(s))
	case "qp":
		b, err := io.ReadAll(quotedprintable.NewReader(strings.NewReader(unhex(f[1]))))
		out := hex.EncodeToString(b)
		if err != nil {
			out += " err " + err.Error()
		}
		return out
	case "mp":
		r := multipart.NewReader(strings.NewReader(unhex(f[2])), unhex(f[1]))
		var out strings.Builder
		for i := 0; i < 100; i++ {
			p, err := r.NextPart()
			if err == io.EOF {
				out.WriteString("end")
				break
			}
			if err != nil {
				out.WriteString("err " + err.Error())
				break
			}
			var keys []string
			for k := range p.Header {
				keys = append(keys, k)
			}
			sort.Strings(keys)
			var hs []string
			for _, k := range keys {
				for _, v := range p.Header[k] {
					hs = append(hs, k+"="+v)
				}
			}
			b, rerr := io.ReadAll(p)
			out.WriteString("[" + strings.Join(hs, ",") + "|" + p.FormName() + "|" + p.FileName() + "|" + hex.EncodeToString(b))
			if rerr != nil {
				out.WriteString(" readerr")
			}
			out.WriteString("]")
		}
		return out.String()
	case "mpw":
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		pre := ""
		if err := w.SetBoundary(f[1]); err != nil {
			pre = "badboundary "
		}
		for i := 2; i < len(f); i++ {
			if f[i] == "f" && i+2 < len(f) {
				w.WriteField(unhex(f[i+1]), unhex(f[i+2]))
				i += 2
			} else if f[i] == "F" && i+3 < len(f) {
				p, _ := w.CreateFormFile(unhex(f[i+1]), unhex(f[i+2]))
				p.Write([]byte(unhex(f[i+3])))
				i += 3
			}
		}
		w.Close()
		_ = textproto.MIMEHeader{}
		if pre != "" {
			return "badboundary" // a random boundary then
		}
		return w.FormDataContentType() + " " + hex.EncodeToString(b.Bytes())
	}
	return "unknown"
}

func TestDiffMIME(t *testing.T) {
	r := rand.New(rand.NewSource(2046))
	var in []string
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	types := []string{"text/html", "TEXT/Plain", "form-data", "attachment", "multipart/form-data", "a/b/c", "/x", "x/", "",
		" text/html ", "image/svg+xml", "\u212Aa/b", "x/y z"}
	params := []string{"; charset=utf-8", "; charset=\"utf-8\"", ";name=\"a b\"", "; filename=\"C:\\\\x\\\\y.txt\"",
		"; filename*=UTF-8''%e2%82%ac%20rates", "; title*0=\"a\"; title*1=\"b\"", "; title*0*=us-ascii'en'x%20; title*1*=%41",
		"; a=1; a=2", "; a=1; A=1", ";", "; =x", "; x=", "; x", "; boundary=----abc", "; q=\"esc\\\"aped\"", "; bad=\"unterminated",
		" ; spaced = value ", "; k=v;", "; f*=bad'%zz", "; n*=utf-8''%2", "; z*1=only"}
	for i := 0; i < 1500; i++ {
		s := types[r.Intn(len(types))]
		for j := r.Intn(4); j > 0; j-- {
			s += params[r.Intn(len(params))]
		}
		in = append(in, "mt\t"+h(s))
	}
	vals := []string{"", "v", "a b", "x\"y", "€uro", "semi;colon", "tab\tx", "back\\slash", "*star", "50%"}
	for i := 0; i < 400; i++ {
		f := []string{"fmt", h(types[r.Intn(len(types))])}
		for j := r.Intn(4); j > 0; j-- {
			f = append(f, h([]string{"charset", "name", "Filename", "bad key", "q"}[r.Intn(5)]), h(vals[r.Intn(len(vals))]))
		}
		in = append(in, strings.Join(f, "\t"))
	}
	qpPieces := []string{"abc", "=3D", "=3d", "=\r\n", "=\n", "= \n", "=\t \r\n", "=4", "=zz", "=", "\r\n", "\n", "  \n", "é", "\x01",
		"x=y", "=C3=A9", " ", "\t"}
	for i := 0; i < 800; i++ {
		var s strings.Builder
		for j := r.Intn(12); j > 0; j-- {
			s.WriteString(qpPieces[r.Intn(len(qpPieces))])
		}
		in = append(in, "qp\t"+h(s.String()))
	}
	// multipart bodies: made by Go's writer, then damaged or rearranged
	for i := 0; i < 600; i++ {
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		bd := []string{"XyZ", "----WebKitFormBoundary7MA4YWxkTrZu0gW", "a b", "simple"}[r.Intn(4)]
		w.SetBoundary(bd)
		for j := r.Intn(4); j > 0; j-- {
			switch r.Intn(3) {
			case 0:
				w.WriteField([]string{"name", "a\"b", "€"}[r.Intn(3)], strings.Repeat("v", r.Intn(5000)))
			case 1:
				p, _ := w.CreateFormFile("file", []string{"a.txt", "dir/b.bin", "../../etc/passwd", "C:\\x.txt"}[r.Intn(4)])
				p.Write(randBytes(r, r.Intn(9000)))
			default:
				hh := textproto.MIMEHeader{}
				hh.Set("Content-Transfer-Encoding", "quoted-printable")
				hh.Set("Content-Disposition", `form-data; name="qp"`)
				p, _ := w.CreatePart(hh)
				p.Write([]byte("caf=C3=A9 =\r\nline\r\n"))
			}
		}
		w.Close()
		body := b.String()
		switch r.Intn(7) {
		case 1:
			body = "preamble line\r\n" + body + "epilogue"
		case 2:
			body = strings.ReplaceAll(body, "\r\n", "\n")
		case 3:
			if len(body) > 0 {
				body = body[:r.Intn(len(body))]
			}
		case 4:
			if len(body) > 0 {
				k := r.Intn(len(body))
				body = body[:k] + string(rune('A'+r.Intn(26))) + body[k+1:]
			}
		case 5:
			body = strings.Replace(body, "--"+bd+"--", "--"+bd+"--   \r\n", 1)
		case 6:
			body = strings.Replace(body, "\r\n\r\n", "\r\nBad Header Line\r\n\r\n", 1)
		}
		in = append(in, "mp\t"+h(bd)+"\t"+h(body))
	}
	for i := 0; i < 200; i++ {
		f := []string{"mpw", []string{"abc", "with space", "bad\x01", "", strings.Repeat("x", 71), "ends space ", "x:y"}[r.Intn(7)]}
		for j := r.Intn(4); j > 0; j-- {
			if r.Intn(2) == 0 {
				f = append(f, "f", h([]string{"a", "q\"uote", "nl\nx"}[r.Intn(3)]), h(vals[r.Intn(len(vals))]))
			} else {
				f = append(f, "F", h("up"), h([]string{"f.txt", "a\\b", "\"x\""}[r.Intn(3)]), h(string(randBytes(r, r.Intn(50)))))
			}
		}
		in = append(in, strings.Join(f, "\t"))
	}
	var want []string
	for _, l := range in {
		want = append(want, goMime(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "mime", in), want)
}
