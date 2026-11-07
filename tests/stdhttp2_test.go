package tests

import (
	"bufio"
	"bytes"
	"compress/gzip"
	"context"
	"crypto/tls"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"
	"math/rand"
	"mime/multipart"
	"net"
	"net/http"
	"net/http/httptest"
	"os/exec"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

var hpackNames = []string{":method", ":path", ":scheme", ":authority", ":status", "content-type", "content-length",
	"accept", "accept-encoding", "user-agent", "cookie", "set-cookie", "authorization", "cache-control", "date", "etag",
	"x-request-id", "x-custom", "x-a", "via", "location", "server", "vary", "if-none-match"}

var hpackValues = []string{"", "GET", "POST", "/", "/index.html", "https", "http", "200", "204", "404", "500",
	"text/html", "application/json", "gzip, deflate", "gzip", "*/*", "no-cache", "example.com", "localhost:8443",
	"Mon, 02 Jan 2006 15:04:05 GMT", "\x00\xff binary \x7f", "a=b; c=d"}

func randHpackString(r *rand.Rand, pool []string) string {
	switch r.Intn(4) {
	case 0:
		b := make([]byte, r.Intn(40))
		for i := range b {
			b[i] = byte(r.Intn(256))
		}
		return string(b)
	case 1:
		return strings.Repeat(string(rune('a'+r.Intn(26))), r.Intn(300))
	default:
		return pool[r.Intn(len(pool))]
	}
}

// RFC 7541 Appendix C: request examples with and without Huffman
// coding, response examples with a 256-byte table
var hpackRFC = [][]string{
	{"4096", "0", "828684410f7777772e6578616d706c652e636f6d", "828684be58086e6f2d6361636865",
		"828785bf400a637573746f6d2d6b65790c637573746f6d2d76616c7565"},
	{"4096", "0", "828684418cf1e3c2e5f23a6ba0ab90f4ff", "828684be5886a8eb10649cbf",
		"828785bf408825a849e95ba97d7f8925a849e95bb8e8b4bf"},
	{"256", "0", "4803333032580770726976617465611d4d6f6e2c203231204f637420323031332032303a31333a323120474d546e1768747470733a2f2f7777772e6578616d706c652e636f6d",
		"4803333037c1c0bf",
		"88c1611d4d6f6e2c203231204f637420323031332032303a31333a323220474d54c05a04677a69707738666f6f3d4153444a4b48514b425a584f5157454f50495541585157454f49553b206d61782d6167653d333630303b2076657273696f6e3d31"},
	{"256", "0", "488264025885aec3771a4b6196d07abe941054d444a8200595040b8166e082a62d1bff6e919d29ad171863c78f0b97c8e9ae82ae43d3",
		"4883640effc1c0bf",
		"88c16196d07abe941054d444a8200595040b8166e084a62d1bffc05a839bd9ab77ad94e7821dd7f2e6c7b335dfdfcd5b3960d5af27087f3672c1ab270fb5291f9587316065c003ed4ee5b1063d5007"},
}

func TestDiffHPACK(t *testing.T) {
	r := rand.New(rand.NewSource(7541))
	var enc []string
	for i := 0; i < 600; i++ {
		ops := []string{"hpe"}
		nf := 1 + r.Intn(40)
		for k := 0; k < nf; k++ {
			switch x := r.Intn(30); {
			case x == 0:
				ops = append(ops, fmt.Sprintf("S%d", []int{0, 1, 32, 100, 256, 4096, 8192, r.Intn(5000)}[r.Intn(8)]))
			case x == 1:
				ops = append(ops, fmt.Sprintf("L%d", []int{0, 64, 256, 4096, 16384, r.Intn(5000)}[r.Intn(6)]))
			case x < 5:
				ops = append(ops, "B")
			default:
				kind := "F"
				if r.Intn(12) == 0 {
					kind = "f"
				}
				name := randHpackString(r, hpackNames)
				if r.Intn(3) > 0 {
					name = hpackNames[r.Intn(len(hpackNames))]
				}
				ops = append(ops, kind+hex.EncodeToString([]byte(name))+":"+hex.EncodeToString([]byte(randHpackString(r, hpackValues))))
			}
		}
		ops = append(ops, "B")
		enc = append(enc, strings.Join(ops, "\t"))
	}
	want := goRef(t, enc)
	diffCompare(t, enc, diffRun(t, "hpack", enc), want)
	// decoding: Go's blocks, as they are and damaged
	var dec []string
	for _, rfc := range hpackRFC {
		dec = append(dec, "hpd\t"+strings.Join(rfc, "\t"))
	}
	for i, blocks := range want {
		bs := strings.Fields(blocks)
		max := []string{"4096", "4096", "4096", "256", "0", "100000"}[r.Intn(6)]
		maxStr := []string{"0", "0", "0", "10", "100"}[r.Intn(5)]
		dec = append(dec, "hpd\t"+max+"\t"+maxStr+"\t"+strings.Join(bs, "\t"))
		if i%2 == 0 {
			// damaged: a flipped byte, a cut, garbage at the end, random blocks, size changes
			d := append([]string{}, bs...)
			for k := range d {
				b, _ := hex.DecodeString(d[k])
				switch r.Intn(6) {
				case 0:
					if len(b) > 0 {
						b[r.Intn(len(b))] ^= byte(1 << r.Intn(8))
					}
				case 1:
					if len(b) > 0 {
						b = b[:r.Intn(len(b))]
					}
				case 2:
					b = append(b, byte(r.Intn(256)), byte(r.Intn(256)))
				case 3:
					b = make([]byte, r.Intn(30))
					r.Read(b)
				case 4:
					d = append(d[:k], append([]string{fmt.Sprintf("T%d", r.Intn(5000))}, d[k:]...)...)
					continue
				}
				d[k] = hex.EncodeToString(b)
			}
			dec = append(dec, "hpd\t4096\t0\t"+strings.Join(d, "\t"))
		}
	}
	// Huffman strings of every padding and invalid codes
	for i := 0; i < 400; i++ {
		b := make([]byte, 1+r.Intn(12))
		r.Read(b)
		if r.Intn(2) == 0 {
			b[len(b)-1] |= 0xff >> r.Intn(8)
		}
		// a literal without indexing with a new name, Huffman value
		blk := append([]byte{0x00, 0x01, 'x', 0x80 | byte(len(b))}, b...)
		dec = append(dec, "hpd\t4096\t0\t"+hex.EncodeToString(blk))
	}
	diffCompare(t, dec, diffRun(t, "hpack", dec), goRef(t, dec))
}

// startH2D runs std/tests/prog/h2d.c; returns its address and a function
// that ends it (closing stdin) and returns its stderr
func startH2D(t *testing.T, args ...string) (string, func() string) {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "h2d.c"), filepath.Join(tempDir(t), "h2d"))
	cmd := exec.Command(exe, args...)
	stdin, _ := cmd.StdinPipe()
	stdout, _ := cmd.StdoutPipe()
	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	line, err := bufio.NewReader(stdout).ReadString('\n')
	if err != nil || !strings.HasPrefix(line, "listening ") {
		cmd.Process.Kill()
		t.Fatalf("h2d did not start: %q %v\n%s", line, err, stderr.String())
	}
	var once sync.Once
	stop := func() string {
		once.Do(func() {
			stdin.Close()
			done := make(chan struct{})
			go func() { cmd.Wait(); close(done) }()
			select {
			case <-done:
			case <-time.After(10 * time.Second):
				cmd.Process.Kill()
				t.Errorf("h2d did not stop")
			}
		})
		return stderr.String()
	}
	t.Cleanup(func() { stop() })
	return strings.TrimSpace(strings.TrimPrefix(line, "listening ")), stop
}

func h2Client(p *pki) (*http.Client, *int32) {
	var conns int32
	tr := &http.Transport{TLSClientConfig: &tls.Config{RootCAs: p.pool, ServerName: "localhost"}, ForceAttemptHTTP2: true,
		DialContext: func(ctx context.Context, network, addr string) (net.Conn, error) {
			atomic.AddInt32(&conns, 1)
			return (&net.Dialer{}).DialContext(ctx, network, addr)
		}}
	return &http.Client{Transport: tr}, &conns
}

func pattern(n int) []byte {
	b := make([]byte, n)
	for i := range b {
		b[i] = byte('a' + i%26)
	}
	return b
}

// TestHTTP2ServerAgainstGo: Go's HTTP/2 client against the library's server
func TestHTTP2ServerAgainstGo(t *testing.T) {
	p := makePKI(t)
	addr, _ := startH2D(t, p.leaves["p256"][0], p.leaves["p256"][1])
	c, conns := h2Client(p)
	base := "https://" + addr
	get := func(path string) (*http.Response, string) {
		t.Helper()
		resp, err := c.Get(base + path)
		if err != nil {
			t.Fatalf("GET %s: %v", path, err)
		}
		b, err := io.ReadAll(resp.Body)
		resp.Body.Close()
		if err != nil {
			t.Fatalf("GET %s body: %v", path, err)
		}
		return resp, string(b)
	}
	resp, body := get("/x?a=1")
	if resp.Proto != "HTTP/2.0" || body != "GET /x HTTP/2.0 0\n" {
		t.Fatalf("proto %s body %q", resp.Proto, body)
	}
	if resp.Header.Get("Content-Type") != "text/plain; charset=utf-8" || resp.ContentLength != int64(len(body)) {
		t.Errorf("header %v length %d", resp.Header, resp.ContentLength)
	}
	// a big download, flushed in pieces
	resp, body = get("/big/20000000")
	if len(body) != 20000000 || body != string(pattern(20000000)) {
		t.Errorf("big: %d bytes", len(body))
	}
	// a big upload echoed as it comes: both windows at work
	up := pattern(9 << 20)
	r, err := c.Post(base+"/echo", "application/octet-stream", bytes.NewReader(up))
	if err != nil {
		t.Fatal(err)
	}
	got, err := io.ReadAll(r.Body)
	r.Body.Close()
	if err != nil || !bytes.Equal(got, up) {
		t.Errorf("echo: %d bytes, %v", len(got), err)
	}
	// uploads of every size up to a few frames
	for _, n := range []int{0, 1, 16383, 16384, 16385, 65535, 65536, 1 << 20, 3<<20 + 7} {
		r, err := c.Post(base+"/len", "text/plain", bytes.NewReader(pattern(n)))
		if err != nil {
			t.Fatal(err)
		}
		b, _ := io.ReadAll(r.Body)
		r.Body.Close()
		if want := fmt.Sprintf("POST /len HTTP/2.0 %d\n", n); string(b) != want {
			t.Errorf("post %d: %q", n, b)
		}
	}
	// statuses without bodies, HEAD
	for _, code := range []int{204, 304, 404, 500} {
		resp, body := get(fmt.Sprintf("/status/%d", code))
		want := "status body\n"
		if code == 204 || code == 304 {
			want = ""
		}
		if resp.StatusCode != code || body != want {
			t.Errorf("status %d: %d %q", code, resp.StatusCode, body)
		}
	}
	hr, err := c.Head(base + "/big/1000")
	if err != nil || hr.StatusCode != 200 {
		t.Errorf("HEAD: %v", err)
	} else {
		hr.Body.Close()
	}
	// headers: cookies come split and join again
	req, _ := http.NewRequest("GET", base+"/headers", nil)
	req.Header.Set("X-One", "1")
	req.Header.Add("X-Two", "a")
	req.Header.Add("X-Two", "b")
	req.AddCookie(&http.Cookie{Name: "c1", Value: "v1"})
	req.AddCookie(&http.Cookie{Name: "c2", Value: "v2"})
	req.Header.Set("X-Long", strings.Repeat("v", 60000))
	r, err = c.Do(req)
	if err != nil {
		t.Fatal(err)
	}
	b, _ := io.ReadAll(r.Body)
	r.Body.Close()
	for _, w := range []string{"X-One: 1\n", "X-Two: a\n", "X-Two: b\n", "Cookie: c1=v1; c2=v2\n", "Host=" + addr + "\n",
		"X-Long: " + strings.Repeat("v", 60000) + "\n"} {
		if !strings.Contains(string(b), w) {
			t.Errorf("headers: no %q in %.300q", w, b)
		}
	}
	// redirects
	resp, body = get("/redirect")
	if resp.Request.URL.Path != "/final" || body != "GET /final HTTP/2.0 0\n" {
		t.Errorf("redirect: %s %q", resp.Request.URL, body)
	}
	// concurrent requests on one connection
	start := time.Now()
	var wg sync.WaitGroup
	var bad int32
	for i := 0; i < 60; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			resp, err := c.Get(base + "/slow/300")
			if err != nil {
				atomic.AddInt32(&bad, 1)
				return
			}
			b, _ := io.ReadAll(resp.Body)
			resp.Body.Close()
			if string(b) != "slow 300\n" {
				atomic.AddInt32(&bad, 1)
			}
		}()
	}
	wg.Wait()
	if bad != 0 || time.Since(start) > 3*time.Second {
		t.Errorf("concurrent: %d bad, %v", bad, time.Since(start))
	}
	// the client cancels: the handler's context ends
	ctx, cancel := context.WithCancel(context.Background())
	req, _ = http.NewRequestWithContext(ctx, "GET", base+"/ctx", nil)
	r, err = c.Do(req)
	if err != nil {
		t.Fatal(err)
	}
	line := make([]byte, 8)
	io.ReadFull(r.Body, line)
	cancel()
	r.Body.Close()
	for i := 0; ; i++ {
		_, body = get("/cancelled")
		if body == "1\n" || i == 50 {
			break
		}
		time.Sleep(20 * time.Millisecond)
	}
	if body != "1\n" {
		t.Errorf("cancelled: %q", body)
	}
	// a handler that does not read a big body: the answer comes anyway
	r, err = c.Post(base+"/noread", "application/octet-stream", bytes.NewReader(pattern(20<<20)))
	if err != nil {
		t.Fatalf("noread: %v", err)
	}
	b, _ = io.ReadAll(r.Body)
	r.Body.Close()
	if string(b) != "not read\n" {
		t.Errorf("noread: %q", b)
	}
	if n := atomic.LoadInt32(conns); n != 1 {
		t.Errorf("%d connections, want 1", n)
	}
}

func fnv64(b []byte) string {
	h := uint64(14695981039346656037)
	for _, c := range b {
		h = (h ^ uint64(c)) * 1099511628211
	}
	return fmt.Sprintf("%016x", h)
}

// runH2Fetch runs std/tests/prog/h2fetch.c with the commands
func runH2Fetch(t *testing.T, input string, args ...string) []string {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "h2fetch.c"), filepath.Join(tempDir(t), "h2fetch"))
	cmd := exec.Command(exe, args...)
	cmd.Stdin = strings.NewReader(input)
	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	out, err := cmd.Output()
	if err != nil {
		t.Fatalf("h2fetch: %v\n%s\n%s", err, out, stderr.String())
	}
	return strings.Split(strings.TrimSuffix(string(out), "\n"), "\n")
}

func goH2Handler(w http.ResponseWriter, r *http.Request) {
	switch {
	case r.URL.Path == "/echo":
		w.Header().Set("Content-Type", "application/octet-stream")
		w.WriteHeader(200)
		w.(http.Flusher).Flush()
		io.Copy(w, r.Body)
	case strings.HasPrefix(r.URL.Path, "/big/"):
		var n int
		fmt.Sscanf(r.URL.Path, "/big/%d", &n)
		w.Write(pattern(n))
	case strings.HasPrefix(r.URL.Path, "/slow/"):
		var ms int
		fmt.Sscanf(r.URL.Path, "/slow/%d", &ms)
		time.Sleep(time.Duration(ms) * time.Millisecond)
		fmt.Fprintf(w, "slow %d\n", ms)
	case r.URL.Path == "/gzip":
		w.Header().Set("Content-Encoding", "gzip")
		zw := gzip.NewWriter(w)
		zw.Write(pattern(100000))
		zw.Close()
	case r.URL.Path == "/redirect":
		http.Redirect(w, r, "/final", 302)
	case r.URL.Path == "/headers":
		var keys []string
		for k := range r.Header {
			keys = append(keys, k)
		}
		sort.Strings(keys)
		for _, k := range keys {
			fmt.Fprintf(w, "%s=%s\n", k, strings.Join(r.Header[k], ","))
		}
	default:
		b, _ := io.ReadAll(r.Body)
		fmt.Fprintf(w, "%s %s %s %d", r.Method, r.URL.Path, r.Proto, len(b))
	}
}

// TestHTTP2ClientAgainstGo: the library's client against Go's HTTP/2 server
func TestHTTP2ClientAgainstGo(t *testing.T) {
	var conns int32
	srv := httptest.NewUnstartedServer(http.HandlerFunc(goH2Handler))
	srv.EnableHTTP2 = true
	srv.Config.HTTP2 = &http.HTTP2Config{MaxConcurrentStreams: 5}
	srv.Config.ConnState = func(c net.Conn, s http.ConnState) {
		if s == http.StateNew {
			atomic.AddInt32(&conns, 1)
		}
	}
	srv.StartTLS()
	defer srv.Close()
	roots := filepath.Join(t.TempDir(), "roots.pem")
	writePEM(t, roots, "CERTIFICATE", srv.Certificate().Raw)
	u := srv.URL
	in := strings.Join([]string{
		"get\t" + u + "/a",
		"get\t" + u + "/big/7000000",
		"post\t" + u + "/echo\t5000000",
		"post\t" + u + "/len\t0",
		"post\t" + u + "/len\t70000",
		"get\t" + u + "/gzip",
		"get\t" + u + "/redirect",
		"headers\t" + u + "/headers\tX-A=1\tX-B=2\tCookie=a=b",
		"par\t40\t" + u + "/slow/200",
		"head\t" + u + "/big/100",
	}, "\n") + "\n"
	out := runH2Fetch(t, in, "roots="+roots)
	want := []string{
		fmt.Sprintf("200 HTTP/2.0 %d %s", len("GET /a HTTP/2.0 0"), fnv64([]byte("GET /a HTTP/2.0 0"))),
		fmt.Sprintf("200 HTTP/2.0 7000000 %s", fnv64(pattern(7000000))),
		fmt.Sprintf("200 HTTP/2.0 5000000 %s", fnv64(pattern(5000000))),
		fmt.Sprintf("200 HTTP/2.0 %d %s", len("POST /len HTTP/2.0 0"), fnv64([]byte("POST /len HTTP/2.0 0"))),
		fmt.Sprintf("200 HTTP/2.0 %d %s", len("POST /len HTTP/2.0 70000"), fnv64([]byte("POST /len HTTP/2.0 70000"))),
		fmt.Sprintf("200 HTTP/2.0 100000 %s", fnv64(pattern(100000))),
		fmt.Sprintf("200 HTTP/2.0 %d %s", len("GET /final HTTP/2.0 0"), fnv64([]byte("GET /final HTTP/2.0 0"))),
		"Accept-Encoding=gzip|Cookie=a=b|User-Agent=AltC-http-client/1.0|X-A=1|X-B=2|",
		"par 40",
		"200 HTTP/2.0 0 cbf29ce484222325",
	}
	if len(out) != len(want) {
		t.Fatalf("%q", out)
	}
	for i := range want {
		if !strings.HasPrefix(out[i], want[i]) {
			t.Errorf("line %d:\n got %q\nwant %q", i, out[i], want[i])
		}
	}
	// 40 requests of 200 ms, 5 at a time: about 1.6 s on one connection
	var ms int
	fmt.Sscanf(out[8], "par 40 %d", &ms)
	if ms < 1500 || ms > 4000 {
		t.Errorf("concurrency limit: %d ms", ms)
	}
	if n := atomic.LoadInt32(&conns); n != 1 {
		t.Errorf("%d connections, want 1", n)
	}
}

// TestHTTP2Cleartext: h2c with prior knowledge both ways
func TestHTTP2Cleartext(t *testing.T) {
	addr, _ := startH2D(t, "-", "-", "-h2c")
	var p http.Protocols
	p.SetUnencryptedHTTP2(true)
	c := &http.Client{Transport: &http.Transport{Protocols: &p}}
	resp, err := c.Get("http://" + addr + "/q")
	if err != nil {
		t.Fatal(err)
	}
	b, _ := io.ReadAll(resp.Body)
	resp.Body.Close()
	if resp.Proto != "HTTP/2.0" || string(b) != "GET /q HTTP/2.0 0\n" {
		t.Errorf("Go h2c client: %s %q", resp.Proto, b)
	}
	// HTTP/1.1 still works on the same server
	resp, err = http.Get("http://" + addr + "/q")
	if err != nil {
		t.Fatal(err)
	}
	b, _ = io.ReadAll(resp.Body)
	resp.Body.Close()
	if string(b) != "GET /q HTTP/1.1 0\n" {
		t.Errorf("HTTP/1.1 on an h2c server: %q", b)
	}
	// the library's h2c client against Go's server
	gs := &http.Server{Handler: http.HandlerFunc(goH2Handler), Protocols: &p}
	ln, _ := net.Listen("tcp", "127.0.0.1:0")
	go gs.Serve(ln)
	defer gs.Close()
	out := runH2Fetch(t, "get\thttp://"+ln.Addr().String()+"/z\npost\thttp://"+ln.Addr().String()+"/echo\t300000\n", "h2c")
	if len(out) != 2 || !strings.HasPrefix(out[0], "200 HTTP/2.0 ") || out[1] != fmt.Sprintf("200 HTTP/2.0 300000 %s", fnv64(pattern(300000))) {
		t.Errorf("h2c client: %q", out)
	}
	// and against its own server
	out = runH2Fetch(t, "get\thttp://"+addr+"/self\n", "h2c")
	if len(out) != 1 || out[0] != fmt.Sprintf("200 HTTP/2.0 %d %s", len("GET /self HTTP/2.0 0\n"), fnv64([]byte("GET /self HTTP/2.0 0\n"))) {
		t.Errorf("self: %q", out)
	}
}

// TestHTTP2Shutdown: a graceful shutdown lets the running request
// finish (GOAWAY), new connections are refused
func TestHTTP2Shutdown(t *testing.T) {
	p := makePKI(t)
	addr, stop := startH2D(t, p.leaves["p256"][0], p.leaves["p256"][1])
	c, _ := h2Client(p)
	base := "https://" + addr
	done := make(chan string)
	go func() {
		resp, err := c.Get(base + "/slow/700")
		if err != nil {
			done <- err.Error()
			return
		}
		b, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		done <- string(b)
	}()
	time.Sleep(150 * time.Millisecond)
	resp, err := c.Get(base + "/shutdown")
	if err != nil {
		t.Fatal(err)
	}
	resp.Body.Close()
	if got := <-done; got != "slow 700\n" {
		t.Errorf("the request in flight: %q", got)
	}
	time.Sleep(200 * time.Millisecond)
	c2, _ := h2Client(p)
	if _, err := c2.Get(base + "/after"); err == nil {
		t.Errorf("a request after the shutdown succeeded")
	}
	if s := stop(); !strings.Contains(s, "serve: ") {
		t.Errorf("stderr %q", s)
	}
}

// TestHTTP2Self: the library's client and server: everything over one
// connection, the client limited by the server's concurrent streams
func TestHTTP2Self(t *testing.T) {
	p := makePKI(t)
	addr, _ := startH2D(t, p.leaves["ed25519"][0], p.leaves["ed25519"][1], "-streams", "4")
	u := "https://localhost:" + strings.Split(addr, ":")[1]
	out := runH2Fetch(t, strings.Join([]string{
		"get\t" + u + "/x",
		"post\t" + u + "/echo\t12000000",
		"get\t" + u + "/big/3000000",
		"par\t20\t" + u + "/slow/100",
		"cancel\t" + u + "/ctx",
		"get\t" + u + "/after-cancel",
	}, "\n")+"\n", "roots="+p.caPEM)
	want := []string{
		fmt.Sprintf("200 HTTP/2.0 %d %s", len("GET /x HTTP/2.0 0\n"), fnv64([]byte("GET /x HTTP/2.0 0\n"))),
		fmt.Sprintf("200 HTTP/2.0 12000000 %s", fnv64(pattern(12000000))),
		fmt.Sprintf("200 HTTP/2.0 3000000 %s", fnv64(pattern(3000000))),
		"par 20",
		"cancelled 200",
		fmt.Sprintf("200 HTTP/2.0 %d", len("GET /after-cancel HTTP/2.0 0\n")),
	}
	if len(out) != len(want) {
		t.Fatalf("%q", out)
	}
	for i := range want {
		if !strings.HasPrefix(out[i], want[i]) {
			t.Errorf("line %d: %q want %q", i, out[i], want[i])
		}
	}
	var ms int
	fmt.Sscanf(out[3], "par 20 %d", &ms)
	if ms < 450 || ms > 3000 {
		t.Errorf("20 requests of 100 ms, 4 at a time: %d ms", ms)
	}
}

// ---- conformance: raw frames against the library's server (h2c) ----

type rawFrame struct {
	typ, flags byte
	id         uint32
	payload    []byte
}

type rawConn struct {
	t *testing.T
	c net.Conn
	r *bufio.Reader
}

func (rc *rawConn) write(typ, flags byte, id uint32, p []byte) {
	h := []byte{byte(len(p) >> 16), byte(len(p) >> 8), byte(len(p)), typ, flags, byte(id >> 24), byte(id >> 16), byte(id >> 8), byte(id)}
	rc.c.Write(append(h, p...))
}

func (rc *rawConn) read() (rawFrame, error) {
	rc.c.SetReadDeadline(time.Now().Add(3 * time.Second))
	h := make([]byte, 9)
	if _, err := io.ReadFull(rc.r, h); err != nil {
		return rawFrame{}, err
	}
	n := int(h[0])<<16 | int(h[1])<<8 | int(h[2])
	p := make([]byte, n)
	if _, err := io.ReadFull(rc.r, p); err != nil {
		return rawFrame{}, err
	}
	return rawFrame{h[3], h[4], uint32(h[5])<<24&0x7f000000 | uint32(h[6])<<16 | uint32(h[7])<<8 | uint32(h[8]), p}, nil
}

// waitFor reads frames until one of type typ (for stream id, unless typ is GOAWAY): its payload, or nil at EOF
func (rc *rawConn) waitFor(typ byte, id uint32) *rawFrame {
	for {
		f, err := rc.read()
		if err != nil {
			return nil
		}
		if f.typ == typ && (typ == 7 || f.id == id) {
			return &f
		}
	}
}

// literal fields without indexing, no Huffman (names and values under 127 bytes)
func hpackLiteral(kv ...string) []byte {
	var b []byte
	for i := 0; i+1 < len(kv); i += 2 {
		b = append(b, 0, byte(len(kv[i])))
		b = append(b, kv[i]...)
		b = append(b, byte(len(kv[i+1])))
		b = append(b, kv[i+1]...)
	}
	return b
}

func dialRaw(t *testing.T, addr string, settings []byte) *rawConn {
	c, err := net.Dial("tcp", addr)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { c.Close() })
	rc := &rawConn{t, c, bufio.NewReader(c)}
	c.Write([]byte("PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"))
	rc.write(4, 0, 0, settings)
	return rc
}

var getReq = hpackLiteral(":method", "GET", ":scheme", "http", ":path", "/", ":authority", "x")

func TestHTTP2Conformance(t *testing.T) {
	addr, _ := startH2D(t, "-", "-", "-h2c", "-streams", "3")
	goaway := func(name string, code uint32, send func(rc *rawConn)) {
		t.Run(name, func(t *testing.T) {
			rc := dialRaw(t, addr, nil)
			send(rc)
			f := rc.waitFor(7, 0)
			if f == nil {
				t.Fatalf("no GOAWAY")
			}
			if got := binary.BigEndian.Uint32(f.payload[4:]); got != code {
				t.Errorf("GOAWAY code %d, want %d (%q)", got, code, f.payload[8:])
			}
		})
	}
	rst := func(name string, id, code uint32, send func(rc *rawConn)) {
		t.Run(name, func(t *testing.T) {
			rc := dialRaw(t, addr, nil)
			send(rc)
			f := rc.waitFor(3, id)
			if f == nil {
				t.Fatalf("no RST_STREAM")
			}
			if got := binary.BigEndian.Uint32(f.payload); got != code {
				t.Errorf("RST_STREAM code %d, want %d", got, code)
			}
			// the connection lives on
			rc.write(6, 0, 0, []byte("12345678"))
			if p := rc.waitFor(6, 0); p == nil || p.flags != 1 || string(p.payload) != "12345678" {
				t.Errorf("no PING answer after the reset")
			}
		})
	}
	const (
		protocol, flow, frameSize, closed, refused, compression = 1, 3, 6, 5, 7, 9
	)
	t.Run("get", func(t *testing.T) {
		rc := dialRaw(t, addr, nil)
		rc.write(1, 5, 1, getReq)
		h := rc.waitFor(1, 1)
		if h == nil || len(h.payload) == 0 || h.payload[0] != 0x88 {
			t.Fatalf("no :status 200 (%v)", h)
		}
		d := rc.waitFor(0, 1)
		if d == nil || string(d.payload) != "GET / HTTP/2.0 0\n" || d.flags&1 == 0 {
			t.Errorf("DATA %v", d)
		}
	})
	t.Run("ping", func(t *testing.T) {
		rc := dialRaw(t, addr, nil)
		rc.write(0xee, 0, 0, []byte("unknown types are ignored"))
		rc.write(6, 0, 0, []byte("abcdefgh"))
		if p := rc.waitFor(6, 0); p == nil || p.flags != 1 || string(p.payload) != "abcdefgh" {
			t.Errorf("PING ack %v", p)
		}
	})
	t.Run("bad-preface", func(t *testing.T) {
		c, _ := net.Dial("tcp", addr)
		defer c.Close()
		c.Write([]byte("PRI * HTTP/2.0\r\n\r\nXX\r\n\r\n"))
		rc := &rawConn{t, c, bufio.NewReader(c)}
		f := rc.waitFor(7, 0)
		if f == nil || binary.BigEndian.Uint32(f.payload[4:]) != protocol {
			t.Errorf("GOAWAY %v", f)
		}
	})
	goaway("data-stream-0", protocol, func(rc *rawConn) { rc.write(0, 0, 0, []byte("x")) })
	goaway("headers-even-stream", protocol, func(rc *rawConn) { rc.write(1, 5, 2, getReq) })
	goaway("settings-length", frameSize, func(rc *rawConn) { rc.write(4, 0, 0, []byte{0, 1, 0}) })
	goaway("settings-window-too-big", flow, func(rc *rawConn) { rc.write(4, 0, 0, []byte{0, 4, 0x80, 0, 0, 0}) })
	goaway("settings-frame-too-small", protocol, func(rc *rawConn) { rc.write(4, 0, 0, []byte{0, 5, 0, 0, 0x3f, 0xff}) })
	goaway("settings-ack-payload", frameSize, func(rc *rawConn) { rc.write(4, 1, 0, []byte{0, 1, 0, 0, 0, 0}) })
	goaway("settings-on-stream", protocol, func(rc *rawConn) { rc.write(4, 0, 1, nil) })
	goaway("ping-length", frameSize, func(rc *rawConn) { rc.write(6, 0, 0, []byte("1234567")) })
	goaway("ping-on-stream", protocol, func(rc *rawConn) { rc.write(6, 0, 1, []byte("12345678")) })
	goaway("window-update-zero", protocol, func(rc *rawConn) { rc.write(8, 0, 0, []byte{0, 0, 0, 0}) })
	goaway("window-overflow", flow, func(rc *rawConn) { rc.write(8, 0, 0, []byte{0x7f, 0xff, 0xff, 0xff}) })
	goaway("continuation-interrupted", protocol, func(rc *rawConn) {
		rc.write(1, 1, 1, getReq[:5])
		rc.write(6, 0, 0, []byte("12345678"))
	})
	goaway("continuation-alone", protocol, func(rc *rawConn) { rc.write(9, 4, 1, getReq) })
	goaway("frame-too-large", frameSize, func(rc *rawConn) { rc.write(0, 0, 1, make([]byte, 1<<20+1)) })
	goaway("bad-hpack", compression, func(rc *rawConn) { rc.write(1, 5, 1, []byte{0xff, 0xff, 0xff, 0xff, 0x0f}) })
	goaway("push-promise", protocol, func(rc *rawConn) { rc.write(5, 4, 1, []byte{0, 0, 0, 2}) })
	goaway("rst-idle", protocol, func(rc *rawConn) { rc.write(3, 0, 9, []byte{0, 0, 0, 8}) })
	goaway("data-idle", protocol, func(rc *rawConn) { rc.write(0, 0, 11, []byte("x")) })
	goaway("padding-too-long", protocol, func(rc *rawConn) {
		rc.write(1, 4, 1, hpackLiteral(":method", "POST", ":scheme", "http", ":path", "/", ":authority", "x"))
		rc.write(0, 8, 1, []byte{5, 'a', 'b'})
	})
	goaway("headers-reused-stream", closed, func(rc *rawConn) {
		rc.write(1, 5, 1, getReq)
		rc.waitFor(0, 1)
		rc.write(1, 5, 1, getReq)
	})
	rst("uppercase-name", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, append(append([]byte{}, getReq...), hpackLiteral("X-Upper", "v")...))
	})
	rst("no-path", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, hpackLiteral(":method", "GET", ":scheme", "http", ":authority", "x"))
	})
	rst("connection-header", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, append(append([]byte{}, getReq...), hpackLiteral("connection", "close")...))
	})
	rst("te-gzip", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, append(append([]byte{}, getReq...), hpackLiteral("te", "gzip")...))
	})
	rst("pseudo-after-regular", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, hpackLiteral(":method", "GET", "x-a", "b", ":scheme", "http", ":path", "/", ":authority", "x"))
	})
	rst("unknown-pseudo", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, append(hpackLiteral(":status", "200"), getReq...))
	})
	rst("content-length-mismatch", 1, protocol, func(rc *rawConn) {
		rc.write(1, 4, 1, hpackLiteral(":method", "POST", ":scheme", "http", ":path", "/", ":authority", "x", "content-length", "10"))
		rc.write(0, 1, 1, []byte("12345"))
	})
	rst("data-after-end", 1, closed, func(rc *rawConn) {
		rc.write(1, 4, 1, hpackLiteral(":method", "POST", ":scheme", "http", ":path", "/slow/300", ":authority", "x"))
		rc.write(0, 1, 1, []byte("ab"))
		rc.write(0, 0, 1, []byte("cd"))
	})
	rst("stream-window-update-zero", 1, protocol, func(rc *rawConn) {
		rc.write(1, 5, 1, hpackLiteral(":method", "GET", ":scheme", "http", ":path", "/slow/300", ":authority", "x"))
		rc.write(8, 0, 1, []byte{0, 0, 0, 0})
	})
	t.Run("refused-streams", func(t *testing.T) {
		rc := dialRaw(t, addr, nil)
		slow := hpackLiteral(":method", "GET", ":scheme", "http", ":path", "/slow/500", ":authority", "x")
		for id := uint32(1); id <= 7; id += 2 {
			rc.write(1, 5, id, slow)
		}
		f := rc.waitFor(3, 7)
		if f == nil || binary.BigEndian.Uint32(f.payload) != refused {
			t.Fatalf("the 4th stream: %v", f)
		}
		got := map[uint32]string{}
		for len(got) < 3 {
			f, err := rc.read()
			if err != nil {
				t.Fatalf("%v after %v", err, got)
			}
			if f.typ == 0 && len(f.payload) > 0 {
				got[f.id] += string(f.payload)
			}
		}
		for id := uint32(1); id <= 5; id += 2 {
			if got[id] != "slow 500\n" {
				t.Errorf("stream %d: %q", id, got[id])
			}
		}
	})
	t.Run("flow-control", func(t *testing.T) {
		// a window of 100 bytes: the server sends 100, then waits for WINDOW_UPDATE
		rc := dialRaw(t, addr, []byte{0, 4, 0, 0, 0, 100})
		rc.write(1, 5, 1, hpackLiteral(":method", "GET", ":scheme", "http", ":path", "/big/250", ":authority", "x"))
		got := 0
		for got < 100 {
			d := rc.waitFor(0, 1)
			if d == nil {
				t.Fatal("no data")
			}
			got += len(d.payload)
		}
		if got != 100 {
			t.Fatalf("%d bytes within a window of 100", got)
		}
		rc.c.SetReadDeadline(time.Now().Add(300 * time.Millisecond))
		if f, err := rc.read(); err == nil && f.typ == 0 && len(f.payload) > 0 {
			t.Fatalf("data beyond the window")
		}
		rc.write(8, 0, 1, []byte{0, 0, 1, 0})
		for got < 250 {
			d := rc.waitFor(0, 1)
			if d == nil {
				t.Fatal("no data after WINDOW_UPDATE")
			}
			got += len(d.payload)
		}
	})
}

// TestMultipartForms: Go's client sends forms with files to the
// library's server over HTTP/1.1 and HTTP/2
func TestMultipartForms(t *testing.T) {
	p := makePKI(t)
	addr, _ := startH2D(t, p.leaves["p256"][0], p.leaves["p256"][1])
	h2, _ := h2Client(p)
	h1 := &http.Client{Transport: &http.Transport{TLSClientConfig: &tls.Config{RootCAs: p.pool, ServerName: "localhost"}}}
	r := rand.New(rand.NewSource(7578))
	for i, c := range []*http.Client{h1, h2} {
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		w.WriteField("title", "hello world")
		w.WriteField("lang", "Русский")
		var want []string
		want = append(want, "v title=hello world", "v lang=Русский")
		for k, size := range []int{0, 10, 70000, 3 << 20} {
			data := randBytes(r, size)
			name := fmt.Sprintf("dir/file%d.bin", k)
			fw, _ := w.CreateFormFile("upload", name)
			fw.Write(data)
			want = append(want, fmt.Sprintf("f upload file%d.bin %d %s application/octet-stream", k, size, fnv64(data)))
		}
		w.Close()
		resp, err := c.Post("https://"+addr+"/form", w.FormDataContentType(), &b)
		if err != nil {
			t.Fatal(err)
		}
		body, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		if want := []string{"HTTP/1.1", "HTTP/2.0"}[i]; resp.Proto != want {
			t.Errorf("proto %s", resp.Proto)
		}
		got := strings.Split(strings.TrimSpace(string(body)), "\n")
		if strings.Join(got, "\n") != strings.Join(want, "\n") {
			t.Errorf("%s: got\n%s\nwant\n%s", resp.Proto, body, strings.Join(want, "\n"))
		}
	}
	// not multipart: 400
	resp, err := h2.Post("https://"+addr+"/form", "text/plain", strings.NewReader("x"))
	if err != nil {
		t.Fatal(err)
	}
	resp.Body.Close()
	if resp.StatusCode != 400 {
		t.Errorf("status %d for a non-multipart body", resp.StatusCode)
	}
}
