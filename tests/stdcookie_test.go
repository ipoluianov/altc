package tests

import (
	"encoding/hex"
	"fmt"
	"math/rand"
	"net/http"
	"net/http/cookiejar"
	"net/http/httptest"
	"net/url"
	"strings"
	"testing"
)

func goCookie(f []string) string {
	switch f[0] {
	case "parse":
		b, _ := hex.DecodeString(f[1])
		c, err := http.ParseSetCookie(string(b))
		if err != nil {
			return "err " + err.Error()
		}
		hasExp, exp := 0, int64(0)
		if !c.Expires.IsZero() {
			hasExp, exp = 1, c.Expires.Unix()
		}
		return fmt.Sprintf("%s|%s|%d|%s|%s|%d|%d|%d|%d|%d|%d", c.Name, c.Value, b2i(c.Quoted), c.Path, c.Domain, c.MaxAge,
			b2i(c.Secure), b2i(c.HttpOnly), int(c.SameSite), hasExp, exp)
	case "jar":
		j, _ := cookiejar.New(nil)
		var out strings.Builder
		for _, op := range f[1:] {
			parts := strings.SplitN(op, " ", 3)
			if len(parts) < 2 {
				continue
			}
			u, err := url.Parse(parts[1])
			if err != nil {
				out.WriteString("[badurl]")
				continue
			}
			if parts[0] == "S" && len(parts) == 3 {
				h := http.Header{}
				for _, x := range strings.Split(parts[2], ",") {
					b, _ := hex.DecodeString(x)
					h.Add("Set-Cookie", string(b))
				}
				j.SetCookies(u, (&http.Response{Header: h}).Cookies())
			} else if parts[0] == "G" {
				req := &http.Request{Header: http.Header{}}
				for _, c := range j.Cookies(u) {
					req.AddCookie(c)
				}
				out.WriteString("[" + req.Header.Get("Cookie") + "]")
			}
		}
		return out.String()
	}
	return "unknown"
}

func TestDiffCookies(t *testing.T) {
	r := rand.New(rand.NewSource(6265))
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	var in []string
	names := []string{"a", "sid", "x y", "", "b", "Name", "ä"}
	values := []string{"1", "v", "\"quoted\"", "a b", "c,d", "x\"y", "", "\"", "semi;", "back\\slash"}
	attrs := []string{"Path=/", "Path=/a", "path=/a/b/", "Path=rel", "Domain=example.com", "domain=.Example.COM", "Domain=sub.example.com",
		"Domain=other.org", "Domain=..x", "Domain=example.com.", "Domain=127.0.0.1", "Domain=localhost", "Domain=xn--bcher-kva.example",
		"Secure", "HttpOnly", "SameSite=Lax", "samesite=STRICT", "SameSite=None", "SameSite=bogus", "SameSite", "Max-Age=100",
		"Max-Age=0", "Max-Age=-1", "Max-Age=007", "Max-Age=abc", "Max-Age=+5", "Expires=Wed, 21 Oct 2099 07:28:00 GMT",
		"Expires=Wed, 21-Oct-2099 07:28:00 GMT", "Expires=Thu, 01 Jan 1970 00:00:00 GMT", "Expires=bogus", "Partitioned",
		"Unknown=1", "  ", "", "Path=\"/q\"", "Ẃeird=1"}
	mk := func() string {
		s := names[r.Intn(len(names))] + "=" + values[r.Intn(len(values))]
		if r.Intn(20) == 0 {
			s = "no-equals"
		}
		for j := r.Intn(5); j > 0; j-- {
			s += "; " + attrs[r.Intn(len(attrs))]
		}
		if r.Intn(10) == 0 {
			s = "  " + s + " ;"
		}
		return s
	}
	for i := 0; i < 1500; i++ {
		in = append(in, "parse\t"+h(mk()))
	}
	urls := []string{"http://example.com/", "https://example.com/a/b/c", "http://sub.example.com/a", "https://www.sub.example.com/",
		"http://other.org/", "http://127.0.0.1:8080/x", "http://localhost/", "http://[::1]:80/", "https://EXAMPLE.com./a/",
		"http://bücher.example/", "ftp://example.com/", "http://example.com/a", "http://example.com/a/", "http://example.com/ab"}
	for i := 0; i < 500; i++ {
		ops := []string{"jar"}
		for k := r.Intn(12); k > 0; k-- {
			if r.Intn(2) == 0 {
				var cs []string
				for m := 1 + r.Intn(3); m > 0; m-- {
					cs = append(cs, h(mk()))
				}
				ops = append(ops, "S "+urls[r.Intn(len(urls))]+" "+strings.Join(cs, ","))
			} else {
				ops = append(ops, "G "+urls[r.Intn(len(urls))])
			}
		}
		for _, u := range urls[:6] {
			ops = append(ops, "G "+u)
		}
		in = append(in, strings.Join(ops, "\t"))
	}
	var want []string
	for _, l := range in {
		want = append(want, goCookie(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "cookie", in), want)
}

// TestClientCookieJar: the library's client with a jar against Go's
// server: a login sets a cookie and redirects, later requests send it
func TestClientCookieJar(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		switch r.URL.Path {
		case "/login":
			http.SetCookie(w, &http.Cookie{Name: "session", Value: "s3cr3t", Path: "/", HttpOnly: true})
			http.SetCookie(w, &http.Cookie{Name: "scoped", Value: "1", Path: "/private"})
			http.Redirect(w, r, "/me", http.StatusFound)
		case "/logout":
			http.SetCookie(w, &http.Cookie{Name: "session", Value: "", Path: "/", MaxAge: -1})
		default:
			fmt.Fprintf(w, "%s cookie=%q", r.URL.Path, r.Header.Get("Cookie"))
		}
	}))
	defer srv.Close()
	u := srv.URL
	out := runH2Fetch(t, strings.Join([]string{
		"headers\t" + u + "/me",
		"headers\t" + u + "/login",
		"headers\t" + u + "/private/x",
		"headers\t" + u + "/logout",
		"headers\t" + u + "/me",
	}, "\n")+"\n", "jar")
	want := []string{`/me cookie=""`, `/me cookie="session=s3cr3t"`, `/private/x cookie="scoped=1; session=s3cr3t"`, ``, `/me cookie=""`}
	if len(out) != len(want) {
		t.Fatalf("%q", out)
	}
	for i := range want {
		if out[i] != want[i] {
			t.Errorf("%d: %q want %q", i, out[i], want[i])
		}
	}
}
