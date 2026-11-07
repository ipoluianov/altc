package tests

import (
	"bytes"
	"context"
	"crypto/hmac"
	"crypto/md5"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/sha512"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"hash"
	"hash/adler32"
	"hash/crc32"
	"hash/crc64"
	"hash/fnv"
	"log/slog"
	"math"
	"math/rand"
	"net"
	"net/netip"
	"net/url"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
	"time"
	"unicode"
	"unicode/utf8"
)

// The differential tests feed many inputs to a C program of
// std/tests/diff and compare its answers with those of Go's packages,
// which serve as the reference.

// diffRun builds std/tests/diff/<name>_diff.c and runs it on the input
// lines, returning its answer lines
func diffRun(t *testing.T, name string, lines []string) []string {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "diff", name+"_diff.c"), filepath.Join(tempDir(t), name+"_diff"))
	cmd := exec.Command(exe)
	cmd.Stdin = strings.NewReader(strings.Join(lines, "\n") + "\n")
	var out, errb bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &errb
	if err := cmd.Run(); err != nil {
		t.Fatalf("%s: %v\n%s", name, err, errb.String())
	}
	got := strings.Split(strings.TrimSuffix(out.String(), "\n"), "\n")
	if len(got) != len(lines) {
		t.Fatalf("%s: %d answers for %d requests", name, len(got), len(lines))
	}
	return got
}

// diffCompare reports the differing answers (at most 20)
func diffCompare(t *testing.T, in, got, want []string) {
	t.Helper()
	bad := 0
	for i := range want {
		if got[i] != want[i] {
			bad++
			if bad <= 20 {
				t.Errorf("%q:\n got %q\nwant %q", in[i], got[i], want[i])
			}
		}
	}
	if bad > 20 {
		t.Errorf("... %d differences in %d cases", bad, len(want))
	}
}

var layouts = []string{
	time.ANSIC, time.UnixDate, time.RubyDate, time.RFC822, time.RFC822Z, time.RFC850, time.RFC1123, time.RFC1123Z,
	time.RFC3339, time.RFC3339Nano, time.Kitchen, time.Stamp, time.StampMilli, time.StampMicro, time.StampNano,
	time.DateTime, time.DateOnly, time.TimeOnly, "Mon, 02 Jan 2006 15:04:05 GMT", "2006-01-02T15:04:05.000Z0700",
	"Monday January 2 3:4:5 pm -07", "002 __2 _2 06 1 01 Jan", "15h04m05s,999999", "Z070000 Z07:00:00 -070000 -07:00:00",
	"2006.01.02 at 3PM .00000 .9", "_2006 Jan_2", "Janet Monk MSTx", "x01y02z03 .0000000000", "Mon Monday MST pm PM",
	"January 2, 2006", "02/01/06 15:04", "20060102150405", "2006-002",
}

func TestDiffTime(t *testing.T) {
	time.Local = time.UTC // zone abbreviations in Parse are looked up in Local
	r := rand.New(rand.NewSource(1))
	zones := []*time.Location{time.UTC, time.FixedZone("CEST", 2*3600), time.FixedZone("", -(5*3600 + 30*60)),
		time.FixedZone("XYZ", 13*3600+45*60+7), time.FixedZone("", -30), time.FixedZone("", 0), time.FixedZone("PST", -8*3600)}
	var in, want []string
	instant := func() int64 {
		switch r.Intn(5) {
		case 0:
			return r.Int63n(4e18) - 2e18 // years 1906..2033 and beyond
		case 1:
			return int64(r.Intn(400*365*86400)-200*365*86400) * 1e9 // whole seconds around 1970
		case 2:
			return time.Date(r.Intn(3000)-500, time.Month(r.Intn(12)+1), r.Intn(31)+1, r.Intn(24), r.Intn(60), r.Intn(60), 0, time.UTC).UnixNano()
		case 3:
			return time.Date(2000+r.Intn(30), 1, 1, 0, 0, 0, r.Intn(1e9), time.UTC).UnixNano()
		}
		return []int64{0, -1, 1, 951782400e9, -9223372036854775808, 9223372036854775807, 253402300799e9 / 100}[r.Intn(7)]
	}
	for i := 0; i < 6000; i++ {
		ns := instant()
		z := zones[r.Intn(len(zones))]
		layout := layouts[r.Intn(len(layouts))]
		name, off := time.Unix(0, ns).In(z).Zone()
		tm := time.Unix(0, ns).In(z)
		in = append(in, fmt.Sprintf("F\t%d\t%d\t%s\t%s", ns, off, name, layout))
		want = append(want, tm.Format(layout))
		// parse back what Go formatted
		if tm.Year() >= 0 && tm.Year() <= 9999 {
			s := tm.Format(layout)
			in = append(in, fmt.Sprintf("P\t%s\t%s", layout, s))
			want = append(want, goParse(layout, s))
		}
		in = append(in, fmt.Sprintf("C\t%d\t%d", ns, off))
		y, mo, d := tm.Date()
		want = append(want, fmt.Sprintf("%d %d %d %d %d %d %d %d %d", y, mo, d, tm.Hour(), tm.Minute(), tm.Second(), tm.Nanosecond(), tm.Weekday(), tm.YearDay()))
	}
	// malformed and edge inputs for parsing
	for _, c := range [][2]string{
		{time.RFC3339, "2024-02-30T00:00:00Z"}, {time.RFC3339, "2023-02-29T00:00:00Z"}, {time.RFC3339, "2024-02-29T23:59:59+14:00"},
		{time.RFC3339, "2024-13-01T00:00:00Z"}, {time.RFC3339, "2024-01-01T24:00:00Z"}, {time.RFC3339, "2024-01-01T00:00:00"},
		{time.RFC3339, "2024-01-01T00:00:00.123456789Z"}, {time.RFC3339, "2024-01-01T00:00:00,5Z"}, {time.RFC3339, "2024-1-01T00:00:00Z"},
		{time.RFC1123, "Mon, 02 Jan 2006 15:04:05 GMT"}, {time.RFC1123, "mon, 02 jan 2006 15:04:05 GMT"},
		{time.RFC1123, "Mon, 02 Jan 2006 15:04:05 GMT+3"}, {time.RFC1123, "Mon, 02 Jan 2006 15:04:05 X"},
		{time.RFC850, "Sunday, 06-Nov-94 08:49:37 GMT"}, {time.ANSIC, "Sun Nov  6 08:49:37 1994"}, {time.ANSIC, "Sun Nov 16 08:49:37 1994"},
		{time.Kitchen, "12:00AM"}, {time.Kitchen, "12:00PM"}, {time.Kitchen, "13:00PM"}, {time.Kitchen, "1:05pm"},
		{"2006-002", "2024-060"}, {"2006-002", "2023-366"}, {"2006-01-02 002", "2024-02-29 060"}, {"2006-01-02 002", "2024-03-01 060"},
		{time.DateOnly, "2024-01-01 "}, {time.DateOnly, ""}, {"", ""}, {"", "x"}, {time.TimeOnly, "1:2:3"}, {"15:04:05.000", "10:00:00.12"},
		{"15:04:05.999", "10:00:00"}, {"15:04:05", "10:00:00.5"}, {"-0700", "+2500"}, {"Z07:00", "Z"}, {"Z07:00", "z"},
		{"06", "68"}, {"06", "69"}, {"Jan", "jan"}, {"January", "Janua"},
	} {
		in = append(in, fmt.Sprintf("P\t%s\t%s", c[0], c[1]))
		want = append(want, goParse(c[0], c[1]))
	}
	// durations
	durs := []time.Duration{0, 1, 999, 1000, 1500, 999999, 1e6, 1500000, 1e9, 1500 * time.Millisecond, time.Minute,
		61 * time.Second, time.Hour, 90*time.Minute + 1, -5 * time.Second, -1, 1<<63 - 1, -1 << 63, 3*time.Hour + 2*time.Nanosecond}
	for i := 0; i < 2000; i++ {
		durs = append(durs, time.Duration(r.Int63n(1<<62)>>uint(r.Intn(60)))*time.Duration(1-2*r.Intn(2)))
	}
	for _, d := range durs {
		in = append(in, fmt.Sprintf("D\t%d", int64(d)))
		want = append(want, d.String())
		in = append(in, "Q\t"+d.String())
		want = append(want, goDur(d.String()))
	}
	for _, s := range []string{"", "0", "+0", "-0", "1", "s", "1x", "1.5h", ".5s", "5.s", ".s", "1h2m3s4ms5us6ns", "1µs", "1μs",
		"9223372036854775807ns", "9223372036854775808ns", "-9223372036854775808ns", "2562047h47m16.854775807s",
		"2562047h47m16.854775808s", "1.0000000000000000000001s", "100000000000000000000h", "3m-2s", "1h 2m"} {
		in = append(in, "Q\t"+s)
		want = append(want, goDur(s))
	}
	diffCompare(t, in, diffRun(t, "time", in), want)
}

func goParse(layout, value string) string {
	tm, err := time.Parse(layout, value)
	if err != nil {
		return "err"
	}
	name, off := tm.Zone()
	return fmt.Sprintf("%d %d %s", tm.UnixNano(), off, name)
}

func goDur(s string) string {
	d, err := time.ParseDuration(s)
	if err != nil {
		return "err"
	}
	return strconv.FormatInt(int64(d), 10)
}

func TestDiffUTF8(t *testing.T) {
	r := rand.New(rand.NewSource(2))
	var in, want []string
	pieces := [][]byte{{0xc2, 0x80}, {0xdf, 0xbf}, {0xe0, 0xa0, 0x80}, {0xe0, 0x9f, 0xbf}, {0xed, 0x9f, 0xbf}, {0xed, 0xa0, 0x80},
		{0xef, 0xbf, 0xbf}, {0xf0, 0x90, 0x80, 0x80}, {0xf0, 0x8f, 0xbf, 0xbf}, {0xf4, 0x8f, 0xbf, 0xbf}, {0xf4, 0x90, 0x80, 0x80},
		{0xf5}, {0xff}, {0xc0, 0x80}, {0xc1, 0xbf}, {0x80}, {0xbf}, {'a'}, {0}, {0xe2, 0x82}, {0xf0, 0x9f, 0x98}}
	for i := 0; i < 20000; i++ {
		var s []byte
		for k := r.Intn(8); k > 0; k-- {
			if r.Intn(3) == 0 {
				s = append(s, byte(r.Intn(256)))
			} else {
				s = append(s, pieces[r.Intn(len(pieces))]...)
			}
		}
		in = append(in, "S "+hex.EncodeToString(s))
		w := fmt.Sprintf("%d %d", b2i(utf8.Valid(s)), utf8.RuneCount(s))
		for j := 0; j < len(s); {
			rr, k := utf8.DecodeRune(s[j:])
			w += fmt.Sprintf(" %d:%d", rr, k)
			j += k
		}
		rr, k := utf8.DecodeLastRune(s)
		w += fmt.Sprintf(" L%d:%d F", rr, k)
		for m := 0; m <= 4 && m <= len(s); m++ {
			w += strconv.Itoa(b2i(utf8.FullRune(s[:m])))
		}
		want = append(want, w)
	}
	for _, rv := range []int32{-1, 0, 0x7f, 0x80, 0x7ff, 0x800, 0xd7ff, 0xd800, 0xdfff, 0xe000, 0xfffd, 0xffff, 0x10000, 0x10ffff, 0x110000, 1 << 30} {
		in = append(in, fmt.Sprintf("E %d", rv))
		buf := make([]byte, 4)
		k := utf8.EncodeRune(buf, rv)
		l := utf8.RuneLen(rv)
		if l < 0 {
			l = 0
		}
		want = append(want, fmt.Sprintf("%s %d %d", hex.EncodeToString(buf[:k]), l, b2i(utf8.ValidRune(rv))))
	}
	diffCompare(t, in, diffRun(t, "utf8", in), want)
}

func b2i(b bool) int {
	if b {
		return 1
	}
	return 0
}

func TestDiffNet(t *testing.T) {
	r := rand.New(rand.NewSource(3))
	var in, want []string
	ips := []string{"1.2.3.4", "0.0.0.0", "255.255.255.255", "256.0.0.1", "1.2.3", "01.2.3.4", "1.2.3.04", "1..2.3", "::", "::1",
		"::ffff:1.2.3.4", "::ffff:01.2.3.4", "::1.2.3.4", "1:2:3:4:5:6:1.2.3.4", "1:2:3:4:5:1.2.3.4", "::ffff:1.2.3.4%z",
		"fe80::1%eth0", "fe80::1%", "fe80::1%25", "2001:db8::", "2001:DB8::A", ":::", "1:::2", "1::2::3", ":1", "1:", "1:2:3:4:5:6:7:8",
		"1:2:3:4:5:6:7:8:9", "1:2:3:4:5:6:7::", "::1:2:3:4:5:6:7", "::1:2:3:4:5:6:7:8", "00001::", "0001::", "x", "", "1.2.3.4%eth0",
		"%eth0", "1.2.3.4.5", "10.1.2.3", "172.16.0.1", "172.32.0.1", "192.168.0.1", "fc00::1", "fe00::1", "127.0.0.1", "127.255.255.255",
		"::ffff:127.0.0.1", "1:0:0:2:0:0:0:3", "0:0:1:0:0:1:0:0", "1:0:0:0:1:0:0:0", "0:0:0:0:0:0:0:1", "1:2:3:4:5:6:0:0"}
	for i := 0; i < 5000; i++ {
		if r.Intn(2) == 0 {
			b := make([]byte, 16)
			for k := range b {
				if r.Intn(3) > 0 {
					b[k] = byte(r.Intn(256))
				}
			}
			if r.Intn(4) == 0 {
				copy(b, []byte{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff})
			}
			a := netip.AddrFrom16([16]byte(b))
			ips = append(ips, a.String())
			// some uppercase, leading zeros and full forms
			if r.Intn(3) == 0 {
				ips = append(ips, strings.ToUpper(a.StringExpanded()))
			}
		} else {
			ips = append(ips, fmt.Sprintf("%d.%d.%d.%d", r.Intn(300), r.Intn(256), r.Intn(256), r.Intn(256)))
		}
	}
	for _, s := range ips {
		in = append(in, "I\t"+s)
		a, err := netip.ParseAddr(s)
		if err != nil {
			want = append(want, "err")
			continue
		}
		u := a.Unmap()
		want = append(want, fmt.Sprintf("%s %d%d%d", a.String(), b2i(u.IsLoopback()), b2i(a.IsPrivate()), b2i(a.IsUnspecified())))
	}
	for _, s := range []string{"a:1", "[::1]:2", "[::1]", "::1:2", "[a]b:c", "a]:1", "[a:1", "[a]:1]", ":80", "[]:80", "a:", "a:b:c",
		"[fe80::1%eth0]:8080", "host.example.com:http", "", "1.2.3.4:56789", "[[a]]:1", "a[b]:1"} {
		in = append(in, "S\t"+s)
		h, p, err := net.SplitHostPort(s)
		if err != nil {
			want = append(want, "err")
		} else {
			want = append(want, h+"|"+p)
		}
	}
	for _, hp := range [][2]string{{"a", "1"}, {"::1", "2"}, {"", "80"}, {"fe80::1%eth0", "x"}} {
		in = append(in, "J\t"+hp[0]+"\t"+hp[1])
		want = append(want, net.JoinHostPort(hp[0], hp[1]))
	}
	diffCompare(t, in, diffRun(t, "net", in), want)
}

func hx(s string) string { return hex.EncodeToString([]byte(s)) + "|" }

func goURLFields(u *url.URL) string {
	var w strings.Builder
	w.WriteString(hx(u.Scheme) + hx(u.Opaque))
	pw, hasPw := "", false
	user := ""
	if u.User != nil {
		user = u.User.Username()
		pw, hasPw = u.User.Password()
	}
	fmt.Fprintf(&w, "%d%d|", b2i(u.User != nil), b2i(hasPw))
	w.WriteString(hx(user) + hx(pw) + hx(u.Host) + hx(u.Path) + hx(u.RawPath) + hx(u.RawQuery) + hx(u.Fragment) + hx(u.RawFragment))
	fmt.Fprintf(&w, "%d%d|", b2i(u.ForceQuery), b2i(u.OmitHost))
	w.WriteString(hx(u.String()) + hx(u.EscapedPath()) + hx(u.RequestURI()) + hx(u.Hostname()) + hx(u.Port()))
	return w.String()
}

func TestDiffURL(t *testing.T) {
	r := rand.New(rand.NewSource(4))
	urls := []string{
		"http://example.com", "http://example.com/", "https://user:pass@example.com:8080/a/b?c=d&e=f#frag",
		"http://[::1]:80/x", "http://[fe80::1%25eth0]:8080/", "http://[fe80::1%eth0]/", "http://[1.2.3.4]/", "http://[::1/",
		"mailto:joe@example.com", "news:comp.lang.go", "file:///etc/passwd", "file://localhost/etc", "//example.com/path",
		"/just/a/path", "relative/path", "rel:colon", "./rel:colon", "?query", "#frag", "", "*", "http://a b.com/", "http://a%20b.com/",
		"http://%41.com/", "http://x.com/%2F%2f/path", "http://x.com/a%2Fb?x%2Fy", "http://x.com/ä?ä#ä", "http://x.com/%zz",
		"http://x.com:port/", "http://x.com:80:90/", "ftp://x.com:80:90/", "http://x.com:/", "http://@x.com/", "http://u@x.com/",
		"http://u:@x.com/", "http://u:p@ss@x.com/", "http://u%40:p%3A@x.com/", "http://u<>@x.com/", "HTTP://X.COM/Path",
		"http://x.com/a/../b/./c", "http://x.com?", "http://x.com/?#", "http://x.com/#a#b", "http://x.com/#%41%2F", "http:x",
		"http:/x", "http:///x", "http:////x", "scheme://", "1http://x", "+x:y", "a+b.c-d:e", "http://x.com\x7f/", "http://\x01/",
		"http://x.com/ ", "http://x.com/?a=1;b=2", "javascript:alert(1)", "//", "///", "////x", "http://x.com/%", "http://x.com/%2",
		"http://[::1]:8a/", "http://[::1]x/", "http://x.com/a%25b", "http://x.com/!$&'()*+,;=:@", "http://x.com/[]", "http://x.com/{}",
		"http://x.com/a?b=c?d", "http://x.com/a??", "http://x.com/a?b?", "s://h/p?q#f", "http://x.com%2F/", "http://xn--n3h.com/",
		"http://x.com/%e2%82%ac", "http://[::ffff:1.2.3.4]/", "http://[2001:db8::1%25en0%25x]/", "http://[::1%25%41]/",
	}
	alphabet := "ab:/?#[]@!$&'()*+,;=%2Fx-._~ \"<>\\^`{|}é01"
	for i := 0; i < 3000; i++ {
		var b strings.Builder
		if r.Intn(2) == 0 {
			b.WriteString([]string{"http://", "https://", "//", "/", "s:", ""}[r.Intn(6)])
		}
		for k := r.Intn(25); k > 0; k-- {
			b.WriteString(string([]rune(alphabet)[r.Intn(len([]rune(alphabet)))]))
		}
		urls = append(urls, b.String())
	}
	var in, want []string
	for _, s := range urls {
		in = append(in, "P\t"+hex.EncodeToString([]byte(s)))
		if u, err := url.Parse(s); err != nil {
			want = append(want, "err")
		} else {
			want = append(want, goURLFields(u))
		}
		in = append(in, "R\t"+hex.EncodeToString([]byte(s)))
		if u, err := url.ParseRequestURI(s); err != nil {
			want = append(want, "err")
		} else {
			want = append(want, goURLFields(u))
		}
	}
	bases := []string{"http://a/b/c/d;p?q", "http://x.com", "http://x.com/dir/", "mailto:x", "http://u:p@h:1/a/b?x#y", "/rel/base", "http://a/b/c/."}
	refs := []string{"g:h", "g", "./g", "g/", "/g", "//g", "?y", "g?y", "#s", "g#s", "g?y#s", ";x", "g;x", "", ".", "./", "..",
		"../", "../g", "../..", "../../", "../../g", "../../../g", "/./g", "/../g", "g.", ".g", "g..", "..g", "./../g", "./g/.",
		"g/./h", "g/../h", "g;x=1/./y", "g;x=1/../y", "g?y/./x", "g#s/../x", "http:g", "//x.com/a/../b", "a%2Fb", "%2E%2E/x"}
	for _, bs := range bases {
		for _, rs := range refs {
			in = append(in, "X\t"+hex.EncodeToString([]byte(bs))+"\t"+hex.EncodeToString([]byte(rs)))
			bu, e1 := url.Parse(bs)
			ru, e2 := url.Parse(rs)
			if e1 != nil || e2 != nil {
				want = append(want, "err")
			} else {
				want = append(want, hex.EncodeToString([]byte(bu.ResolveReference(ru).String())))
			}
		}
	}
	queries := []string{"a=1&b=2", "b=2&a=1&a=0", "x", "x=", "=y", "&&a=1&&", "a=1;b=2", "a=%zz&b=2", "a+b=c+d", "%41=%42", "k=v=w",
		"a=1&a=2&a=3", "z=1&y=2&x=3", "é=ü", "a=%", "a=%2", ""}
	for i := 0; i < 500; i++ {
		var b strings.Builder
		for k := r.Intn(12); k > 0; k-- {
			b.WriteByte("ab=&%+;2Fé"[r.Intn(10)])
		}
		queries = append(queries, b.String())
	}
	for _, q := range queries {
		in = append(in, "Q\t"+hex.EncodeToString([]byte(q)))
		v, err := url.ParseQuery(q)
		w := hx(v.Encode()) + strconv.Itoa(b2i(err != nil)) + "|"
		// the first key in order of appearance and its first value
		first := ""
		for _, pair := range strings.Split(q, "&") {
			if pair == "" || strings.Contains(pair, ";") {
				continue
			}
			k, _, _ := strings.Cut(pair, "=")
			uk, e := url.QueryUnescape(k)
			if e != nil {
				continue
			}
			if _, ok := v[uk]; ok {
				first = hx(v.Get(uk))
				break
			}
		}
		want = append(want, w+first)
	}
	escapes := []string{"", "a b", "a+b", "a/b?c", "%41", "%zz", "%", "100%", "é", "~-._", "!*'();:@&=+$,/?#[]", "\x00\x7f\xff"}
	for _, s := range escapes {
		in = append(in, "E\t"+hex.EncodeToString([]byte(s)))
		w := hx(url.QueryEscape(s)) + hx(url.PathEscape(s))
		if u, err := url.QueryUnescape(s); err != nil {
			w += "err|"
		} else {
			w += hx(u)
		}
		if u, err := url.PathUnescape(s); err != nil {
			w += "err|"
		} else {
			w += hx(u)
		}
		want = append(want, w)
	}
	diffCompare(t, in, diffRun(t, "url", in), want)
}

func TestDiffStrconv(t *testing.T) {
	r := rand.New(rand.NewSource(7))
	var in, want []string
	var vals []float64
	for _, v := range []float64{0, math.Copysign(0, -1), 1, -1, 0.1, 0.5, 1.5, 2.5, 100, 1e21, 1e20, 123456789, 1e-7, 1e-6, 5e-324,
		math.MaxFloat64, math.SmallestNonzeroFloat64, 2.2250738585072014e-308, math.Inf(1), math.Inf(-1), math.NaN(), 1.0 / 3,
		0.3, 9007199254740993, 4.35, 2.675, 1.005, 999999.5, 0.000001234, 12345678901234567890} {
		vals = append(vals, v)
	}
	for i := 0; i < 1500; i++ {
		switch r.Intn(4) {
		case 0:
			vals = append(vals, math.Float64frombits(r.Uint64()))
		case 1:
			vals = append(vals, float64(r.Int63n(1e6))/float64(int64(1)<<uint(r.Intn(40))))
		case 2:
			vals = append(vals, math.Ldexp(1, r.Intn(2098)-1074)) // powers of two
		default:
			vals = append(vals, r.NormFloat64()*math.Pow(10, float64(r.Intn(40)-20)))
		}
	}
	for _, v := range vals {
		bits := fmt.Sprintf("%016x", math.Float64bits(v))
		for _, f := range []byte{'e', 'E', 'f', 'g', 'G'} {
			for _, p := range []int{-1, 0, 1, 3, 6, 17, 20} {
				if f == 'f' && math.Abs(v) > 1e30 && p > 3 {
					continue
				}
				in = append(in, fmt.Sprintf("F\t%s\t%c\t%d", bits, f, p))
				want = append(want, strconv.FormatFloat(v, f, p, 64))
			}
		}
	}
	parses := []string{"1", "-1", "+1", "1.5", ".5", "5.", "1e10", "1E-10", "1e", "e1", "1e+", "0x1p-2", "0x1.8p1", "0X1P0", "0x1",
		"0x_1p0", "0x1_0p0", "1_0", "inf", "-Inf", "+INFINITY", "nan", "NaN", "-nan", "1e400", "-1e400", "1e-400", "4.9e-324",
		"2.4703282292062327e-324", "2.4703282292062328e-324", "1.7976931348623157e308", "1.7976931348623159e308", " 1", "1 ", "",
		"-", ".", "1..2", "0.0000000000000000000000000000000000000001", "123456789012345678901234567890", "0x.8p1", "0xp1", "1e1_0"}
	for i := 0; i < 500; i++ {
		v := math.Float64frombits(r.Uint64())
		parses = append(parses, strconv.FormatFloat(v, 'g', r.Intn(20)-1, 64))
	}
	for _, s := range parses {
		in = append(in, "P\t"+s)
		v, err := strconv.ParseFloat(s, 64)
		switch {
		case err != nil && errors.Is(err, strconv.ErrSyntax):
			want = append(want, "syntax")
		default:
			b := math.Float64bits(v)
			if math.IsNaN(v) {
				b = 0x7ff8000000000001
			}
			st := "ok"
			if err != nil {
				st = "range"
			}
			want = append(want, fmt.Sprintf("%016x %s", b, st))
		}
	}
	ints := []string{"", "0", "-0", "+0", "1", "-1", "+", "-", "0x", "0x1f", "0X1F", "0b101", "0o17", "017", "08", "0_1", "1_000",
		"_1", "1_", "1__0", "0x_1", "0b_1", "0_x1", "9223372036854775807", "9223372036854775808", "-9223372036854775808",
		"-9223372036854775809", "18446744073709551615", "18446744073709551616", "99999999999999999999x", "z", "Zz", "1e3",
		" 1", "1 ", "0xffffffffffffffff", "0x10000000000000000", "127", "128", "-128", "-129", "255", "256", "0b", "0o", "00", "0_"}
	for i := 0; i < 200; i++ {
		b := make([]byte, 1+r.Intn(8))
		for j := range b {
			b[j] = "0123456789abcxoXB_-+z"[r.Intn(21)]
		}
		ints = append(ints, string(b))
	}
	code := func(err error) string {
		var ne *strconv.NumError
		if err == nil {
			return "ok"
		} else if errors.As(err, &ne) && ne.Err == strconv.ErrSyntax {
			return "syntax"
		} else if errors.As(err, &ne) && ne.Err == strconv.ErrRange {
			return "range"
		}
		return "inval"
	}
	for _, s := range ints {
		for _, base := range []int{0, 2, 8, 10, 16, 36, 1, 37} {
			for _, bits := range []int{0, 8, 16, 32, 64, 65} {
				in = append(in, fmt.Sprintf("I\t%s\t%d\t%d", s, base, bits))
				v, err := strconv.ParseInt(s, base, bits)
				want = append(want, fmt.Sprintf("%d %s", v, code(err)))
				in = append(in, fmt.Sprintf("N\t%s\t%d\t%d", s, base, bits))
				u, err := strconv.ParseUint(s, base, bits)
				want = append(want, fmt.Sprintf("%d %s", u, code(err)))
			}
		}
	}
	strs := []string{"", "abc", "a\"b", "a\\b", "\n\t\r\a\b\f\v", "\x00\x01\x7f", "é", "日本", "😀", "\xff\xfe", " ", "a\x80b"}
	for i := 0; i < 300; i++ {
		b := make([]byte, r.Intn(10))
		r.Read(b)
		strs = append(strs, string(b))
	}
	for _, s := range strs {
		for m, q := range []func(string) string{strconv.Quote, strconv.QuoteToASCII, strconv.QuoteToGraphic} {
			in = append(in, fmt.Sprintf("Q\t%s\t%d", hex.EncodeToString([]byte(s)), m))
			want = append(want, q(s))
		}
		q := strconv.QuoteToASCII(s)
		for _, u := range []string{q, strconv.Quote(s), "`" + strings.ReplaceAll(s, "`", "") + "`", "'a'", "'ab'", `"\x41\101é\U0001F600"`,
			`"bad\q"`, `"\400"`, `"\ud800"`, `"unterminated`, `'\''`, `"'"`, `'"'`} {
			in = append(in, "U\t"+hex.EncodeToString([]byte(u)))
			if v, err := strconv.Unquote(u); err != nil {
				want = append(want, "err")
			} else {
				want = append(want, hex.EncodeToString([]byte(v)))
			}
		}
	}
	diffCompare(t, in, diffRun(t, "strconv", in), want)
}

func TestDiffEncoding(t *testing.T) {
	r := rand.New(rand.NewSource(8))
	docs := []string{`null`, `true`, `false`, `0`, `-0`, `1.0`, `1e2`, `-1.5e-10`, `1e400`, `123456789012345678901234567890`,
		`0.1`, `1E+3`, `"a"`, `""`, `"é😀"`, `"\ud800"`, `"\ud800A"`, `"\udc00"`, `"<>&"`, `" "`,
		"\"\xff\xfe\"", "\"\x7f\"", `"\/\b\f\n\r\t\\\""`, `[]`, `{}`, `[1,2,3]`, `{"b":1,"a":2}`, `{"a":1,"a":2}`, ` { "k" : [ null ] } `,
		`[1,]`, `{"a":1,}`, `[01]`, `[1.]`, `[.1]`, `[-]`, `[1e]`, `["\x"]`, "[\"\x01\"]", `{"a"}`, `{a:1}`, `[1 2]`, `tru`, `nul`,
		`[true false]`, `"unterminated`, ``, ` `, `1 2`, `[[[[]]]]`, `{"":""}`, `[1e-400]`, `[-1e-400]`, `[1.7976931348623157e308]`,
		strings.Repeat("[", 10001) + strings.Repeat("]", 10001), strings.Repeat("[", 9999) + strings.Repeat("]", 9999)}
	parts := []string{`{`, `}`, `[`, `]`, `,`, `:`, `"k"`, `"v"`, `1`, `-2.5e3`, `true`, `null`, ` `, `"A"`, `"é"`}
	for i := 0; i < 3000; i++ {
		var b strings.Builder
		for k := r.Intn(12); k > 0; k-- {
			b.WriteString(parts[r.Intn(len(parts))])
		}
		docs = append(docs, b.String())
	}
	// generated valid documents
	var gen func(d int) any
	gen = func(d int) any {
		switch k := r.Intn(7); {
		case d > 3 || k == 0:
			return r.NormFloat64() * math.Pow(10, float64(r.Intn(30)-15))
		case k == 1:
			b := make([]byte, r.Intn(8))
			r.Read(b)
			return string(b)
		case k == 2:
			return r.Intn(2) == 0
		case k == 3:
			return nil
		case k == 4:
			var a []any
			for n := r.Intn(4); n > 0; n-- {
				a = append(a, gen(d+1))
			}
			return a
		default:
			m := map[string]any{}
			for n := r.Intn(4); n > 0; n-- {
				m[fmt.Sprint("k", r.Intn(5), "<&>")] = gen(d + 1)
			}
			return m
		}
	}
	for i := 0; i < 1000; i++ {
		b, _ := json.Marshal(gen(0))
		docs = append(docs, string(b))
		ib, _ := json.MarshalIndent(gen(0), "", " ")
		docs = append(docs, string(ib))
	}
	var in, want []string
	for _, d := range docs {
		var v any
		err := json.Unmarshal([]byte(d), &v)
		for _, op := range []string{"J", "I"} {
			in = append(in, op+"\t"+hex.EncodeToString([]byte(d)))
			if err != nil {
				want = append(want, "err")
				continue
			}
			var out []byte
			if op == "J" {
				out, err = json.Marshal(v)
			} else {
				out, err = json.MarshalIndent(v, "", "  ")
			}
			if err != nil {
				want = append(want, "err")
			} else {
				want = append(want, hex.EncodeToString(out))
			}
		}
	}
	encs := map[string]*base64.Encoding{"std": base64.StdEncoding, "url": base64.URLEncoding, "rawstd": base64.RawStdEncoding,
		"rawurl": base64.RawURLEncoding}
	for i := 0; i < 400; i++ {
		data := make([]byte, r.Intn(20))
		r.Read(data)
		for name, e := range encs {
			in = append(in, "E\t"+name+"\t"+hex.EncodeToString(data))
			want = append(want, e.EncodeToString(data))
			s := e.EncodeToString(data)
			cands := []string{s, s + "=", strings.TrimRight(s, "="), s + "\n", "\r\n" + s, s + "A", "!" + s, s[:len(s)/2] + "\n" + s[len(s)/2:]}
			for _, c := range cands {
				in = append(in, "D\t"+name+"\t"+hex.EncodeToString([]byte(c)))
				if d, err := e.DecodeString(c); err != nil {
					want = append(want, "err")
				} else {
					want = append(want, hex.EncodeToString(d))
				}
			}
		}
	}
	for _, s := range []string{"", "00", "0a", "0A", "abc", "zz", "123456", "12345g", "g1"} {
		in = append(in, "X\t"+hex.EncodeToString([]byte(s)))
		if d, err := hex.DecodeString(s); err != nil {
			want = append(want, "err")
		} else {
			want = append(want, hex.EncodeToString(d))
		}
		in = append(in, "H\t"+hex.EncodeToString([]byte(s)))
		want = append(want, hex.EncodeToString([]byte(s)))
	}
	diffCompare(t, in, diffRun(t, "encoding", in), want)
}

func TestDiffHash(t *testing.T) {
	r := rand.New(rand.NewSource(9))
	hashes := []func() hash.Hash{md5.New, sha1.New, sha256.New224, sha256.New, sha512.New384, sha512.New, sha512.New512_256}
	var in, want []string
	lens := []int{0, 1, 3, 55, 56, 57, 63, 64, 65, 111, 112, 113, 119, 120, 127, 128, 129, 1000, 10000}
	for i := 0; i < 60; i++ {
		lens = append(lens, r.Intn(3000))
	}
	for _, n := range lens {
		data := make([]byte, n)
		r.Read(data)
		key := make([]byte, r.Intn(200))
		r.Read(key)
		for k, h := range hashes {
			for _, op := range []string{"H", "S"} {
				in = append(in, fmt.Sprintf("%s\t%d\t%s", op, k, hex.EncodeToString(data)))
				hh := h()
				hh.Write(data)
				want = append(want, hex.EncodeToString(hh.Sum(nil)))
			}
			if k == 0 {
				cut := r.Intn(n + 1)
				in = append(in, fmt.Sprintf("C\t%d\t%s", cut, hex.EncodeToString(data)))
				c := fmt.Sprintf("%08x %08x %08x %08x %016x %016x ", crc32.ChecksumIEEE(data),
					crc32.Checksum(data, crc32.MakeTable(crc32.Castagnoli)), crc32.Checksum(data, crc32.MakeTable(crc32.Koopman)),
					adler32.Checksum(data), crc64.Checksum(data, crc64.MakeTable(crc64.ISO)), crc64.Checksum(data, crc64.MakeTable(crc64.ECMA)))
				var hs []uint64
				for _, h := range []hash.Hash{fnv.New32(), fnv.New32a(), fnv.New64(), fnv.New64a()} {
					h.Write(data)
					switch h := h.(type) {
					case hash.Hash32:
						hs = append(hs, uint64(h.Sum32()))
					case hash.Hash64:
						hs = append(hs, h.Sum64())
					}
				}
				want = append(want, c+fmt.Sprintf("%08x %08x %016x %016x", hs[0], hs[1], hs[2], hs[3]))
			}
			in = append(in, fmt.Sprintf("M\t%d\t%s\t%s", k, hex.EncodeToString(key), hex.EncodeToString(data)))
			m := hmac.New(h, key)
			m.Write(data)
			want = append(want, hex.EncodeToString(m.Sum(nil)))
		}
	}
	diffCompare(t, in, diffRun(t, "hash", in), want)
}

func TestDiffUnicode(t *testing.T) {
	var in, want []string
	fns := []func(rune) bool{unicode.IsLetter, unicode.IsDigit, unicode.IsNumber, unicode.IsUpper, unicode.IsLower,
		unicode.IsTitle, unicode.IsMark, unicode.IsPunct, unicode.IsSymbol, unicode.IsControl, unicode.IsSpace,
		unicode.IsPrint, unicode.IsGraphic}
	names := "LdnulTmpsc_PG"
	for r := rune(-2); r <= unicode.MaxRune+2; r++ {
		in = append(in, strconv.Itoa(int(r)))
		var s []byte
		for i, f := range fns {
			if f(r) {
				s = append(s, names[i])
			}
		}
		want = append(want, fmt.Sprintf("%s %d %d %d %d %d", s, unicode.ToUpper(r), unicode.ToLower(r), unicode.ToTitle(r),
			unicode.SimpleFold(r), b2i(unicode.Is(unicode.Zs, r))))
	}
	diffCompare(t, in, diffRun(t, "unicode", in), want)
}

func TestDiffLog(t *testing.T) {
	r := rand.New(rand.NewSource(11))
	strs := []string{"", "a", "a b", "x=y", `q"q`, `back\slash`, "new\nline", "tab\t", "é", "日本", " ", " ", "​",
		"\xff", "<&>", "\x7f", "\x01\x1f", "ok_value-1.2", "\U0001F600", "\ufeff"}
	for i := 0; i < 100; i++ {
		rs := make([]rune, r.Intn(5))
		for j := range rs {
			rs[j] = rune(r.Intn(0x3000))
		}
		strs = append(strs, string(rs))
	}
	pick := func() string { return strs[r.Intn(len(strs))] }
	levels := []int{-8, -5, -4, -3, -1, 0, 1, 3, 4, 5, 8, 9, 12}
	floats := []float64{0, 1, -1.5, 1e21, 1e-7, 123456789, 0.1, math.MaxFloat64, 5e-324}
	var in, want []string
	for i := 0; i < 2000; i++ {
		format := i % 2
		level := levels[r.Intn(len(levels))]
		msg := pick()
		line := fmt.Sprintf("%d\t%d\t%s", format, level, hex.EncodeToString([]byte(msg)))
		var attrs []any
		for k := r.Intn(5); k > 0; k-- {
			key := pick()
			if key == "" {
				key = "k"
			}
			var kind, val string
			var a slog.Attr
			switch r.Intn(6) {
			case 0:
				s := pick()
				kind, val, a = "s", hex.EncodeToString([]byte(s)), slog.String(key, s)
			case 1:
				v := r.Int63() - r.Int63()
				kind, val, a = "i", strconv.FormatInt(v, 10), slog.Int64(key, v)
			case 2:
				v := r.Uint64()
				kind, val, a = "u", strconv.FormatUint(v, 10), slog.Uint64(key, v)
			case 3:
				v := floats[r.Intn(len(floats))]
				kind, val, a = "f", strconv.FormatFloat(v, 'g', -1, 64), slog.Float64(key, v)
			case 4:
				v := r.Intn(2) == 1
				kind, val, a = "b", strconv.Itoa(b2i(v)), slog.Bool(key, v)
			case 5:
				v := time.Duration(r.Int63n(1e13))
				kind, val, a = "d", strconv.FormatInt(int64(v), 10), slog.Duration(key, v)
			}
			line += "\t" + hex.EncodeToString([]byte(key)) + "\t" + kind + "\t" + val
			attrs = append(attrs, a)
		}
		in = append(in, line)
		var buf bytes.Buffer
		opts := &slog.HandlerOptions{Level: slog.Level(-20), ReplaceAttr: func(groups []string, a slog.Attr) slog.Attr {
			if a.Key == slog.TimeKey && len(groups) == 0 {
				return slog.Attr{}
			}
			return a
		}}
		var h slog.Handler = slog.NewTextHandler(&buf, opts)
		if format == 1 {
			h = slog.NewJSONHandler(&buf, opts)
		}
		slog.New(h).Log(context.Background(), slog.Level(level), msg, attrs...)
		want = append(want, strings.TrimSuffix(buf.String(), "\n"))
	}
	diffCompare(t, in, diffRun(t, "log", in), want)
}

func goFlags(args []string) string {
	var out, fnlog strings.Builder
	fs := flag.NewFlagSet("prog", flag.ContinueOnError)
	fs.SetOutput(&out)
	v := fs.Bool("v", false, "verbose")
	tt := fs.Bool("t", true, "on by default")
	x := fs.Bool("x", false, "")
	n := fs.Int64("n", 3, "number of `items`")
	zero := fs.Int64("zero", 0, "zero int")
	ln := fs.Int64("long-name-here", -7, "one ` quote")
	u := fs.Uint64("u", 0, "unsigned")
	f := fs.Float64("f", 0.5, "a float")
	s := fs.String("s", "x y", "a string\nsecond line")
	e := fs.String("e", "", "empty")
	d := fs.Duration("d", time.Second, "duration")
	dz := fs.Duration("dz", 0, "zero dur")
	fn := func(p string) func(string) error {
		return func(v string) error {
			fmt.Fprintf(&fnlog, "%s%s,", p, v)
			if v == "bad" {
				return errors.New("Invalid argument")
			}
			return nil
		}
	}
	fs.Func("fn", "func flag", fn("fn:"))
	fs.BoolFunc("bf", "bool func", fn("bf:"))
	err := fs.Parse(args)
	rc, msg := "ok", ""
	if err == flag.ErrHelp {
		rc, msg = "help", "flag: help requested"
	} else if err != nil {
		rc, msg = "parse", err.Error()
	}
	b2 := func(b bool) int { return b2i(b) }
	r := fmt.Sprintf("%s|%s|%d %d %d %d %d %d %d %s ", rc, msg, b2(*v), b2(*tt), b2(*x), *n, *zero, *ln, *u,
		strconv.FormatFloat(*f, 'g', 17, 64))
	r += fmt.Sprintf("%s %s %d %d %s|", *s, *e, int64(*d), int64(*dz), fnlog.String())
	for _, a := range fs.Args() {
		r += a + ","
	}
	r += fmt.Sprintf("|%d:", fs.NFlag())
	fs.Visit(func(f *flag.Flag) { r += f.Name + "," })
	return r + "|1 1|" + hex.EncodeToString([]byte(out.String()))
}

func TestDiffFlag(t *testing.T) {
	cases := [][]string{
		{}, {"-v"}, {"--v"}, {"-v=false"}, {"-v=0"}, {"-v=maybe"}, {"-t=false", "rest"}, {"-v", "false"},
		{"-n", "42"}, {"-n=0x1F"}, {"-n=0b11", "-n", "0o17"}, {"-n", "1_000"}, {"-n=_1"}, {"-n"}, {"-n=abc"},
		{"-n", "99999999999999999999"}, {"-u=-1"}, {"-u", "18446744073709551615"}, {"-f", "1e400"}, {"-f=-2.5e-3"},
		{"-f=x"}, {"-s", "hello world"}, {"-s="}, {"-s=a=b"}, {"-e", "-v"}, {"-d", "1h2m"}, {"-d=bad"}, {"-d=-1.5s"},
		{"-dz", "0"}, {"-fn", "a", "-fn=b"}, {"-fn=bad"}, {"-bf"}, {"-bf=xyz"}, {"-bf=bad"},
		{"-h"}, {"-help"}, {"--help"}, {"-unknown"}, {"--unknown=1"}, {"---v"}, {"-=v"}, {"--=x"}, {"-"},
		{"--", "-v"}, {"-v", "--", "--", "-x"}, {"a", "-v"}, {"-v", "a", "-x"}, {"-x", "-t=1", "-v=T", "-n=-5", "z"},
		{"-long-name-here", "8"}, {"-zero=1"}, {"-n=1", "-n=2"}, {"-x=true", "-x=FALSE"}, {"-xv"}, {"-n=", "1"},
	}
	var in, want []string
	for _, c := range cases {
		var h []string
		for _, a := range c {
			h = append(h, hex.EncodeToString([]byte(a)))
		}
		in = append(in, strings.Join(h, "\t"))
		want = append(want, goFlags(c))
	}
	diffCompare(t, in, diffRun(t, "flag", in), want)
}
