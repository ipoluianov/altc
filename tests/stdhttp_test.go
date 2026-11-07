package tests

import (
	"bufio"
	"bytes"
	"compress/gzip"
	"context"
	"encoding/hex"
	"fmt"
	"io"
	"math/rand"
	"net"
	"net/http"
	"net/http/httptest"
	"net/http/httptrace"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

// startHTTPD builds std/tests/prog/httpd.c, starts it with root as the
// directory of /static/ and returns its address and a stop function
// (graceful shutdown) that reports the exit status
func startHTTPD(t *testing.T, root string) (string, func() error) {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "httpd.c"), filepath.Join(tempDir(t), "httpd"))
	cmd := exec.Command(exe, root)
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
		t.Fatalf("server did not start: %q %v\n%s", line, err, stderr.String())
	}
	addr := strings.TrimSpace(strings.TrimPrefix(line, "listening "))
	var once sync.Once
	var result error
	stop := func() error {
		once.Do(func() {
			stdin.Close()
			done := make(chan error, 1)
			go func() { done <- cmd.Wait() }()
			select {
			case err := <-done:
				if err != nil {
					result = fmt.Errorf("server exit: %v\n%s", err, stderr.String())
				}
			case <-time.After(10 * time.Second):
				cmd.Process.Kill()
				result = fmt.Errorf("server did not stop\n%s", stderr.String())
			}
		})
		return result
	}
	t.Cleanup(func() { stop() })
	return addr, stop
}

func staticRoot(t *testing.T) string {
	root := t.TempDir()
	os.WriteFile(filepath.Join(root, "a.txt"), []byte("file a\n"), 0o644)
	os.WriteFile(filepath.Join(root, "page.html"), []byte("<p>page</p>"), 0o644)
	os.WriteFile(filepath.Join(root, "noext"), []byte("<html><body>sniffed</body></html>"), 0o644)
	os.MkdirAll(filepath.Join(root, "sub", "deeper"), 0o755)
	os.WriteFile(filepath.Join(root, "sub", "index.html"), []byte("sub index"), 0o644)
	os.WriteFile(filepath.Join(root, "sub", "deeper", "x.json"), []byte(`{"x":1}`), 0o644)
	big := make([]byte, 200000)
	for i := range big {
		big[i] = byte(i % 251)
	}
	os.WriteFile(filepath.Join(root, "big.bin"), big, 0o644)
	os.WriteFile(filepath.Join(filepath.Dir(root), "secret.txt"), []byte("secret"), 0o644)
	return root
}

func get(t *testing.T, c *http.Client, url string) (*http.Response, string) {
	t.Helper()
	resp, err := c.Get(url)
	if err != nil {
		t.Fatalf("GET %s: %v", url, err)
	}
	b, err := io.ReadAll(resp.Body)
	resp.Body.Close()
	if err != nil {
		t.Fatalf("GET %s body: %v", url, err)
	}
	return resp, string(b)
}

func TestHTTPServer(t *testing.T) {
	root := staticRoot(t)
	addr, stop := startHTTPD(t, root)
	base := "http://" + addr
	noRedirect := &http.Client{CheckRedirect: func(*http.Request, []*http.Request) error { return http.ErrUseLastResponse }}
	c := &http.Client{}

	t.Run("routes", func(t *testing.T) {
		cases := []struct {
			method, path string
			status       int
			body         string
			header       map[string]string
		}{
			{"GET", "/hello", 200, "hello, world\n", map[string]string{"Content-Type": "text/plain; charset=utf-8", "Content-Length": "13"}},
			{"HEAD", "/hello", 200, "", map[string]string{"Content-Length": "13"}},
			{"POST", "/hello", 405, "Method Not Allowed\n", map[string]string{"Allow": "GET, HEAD"}},
			{"GET", "/items/42", 200, "item 42 pattern GET /items/{id}", nil},
			{"GET", "/items/a%2Fb", 200, "item a/b pattern GET /items/{id}", nil},
			{"POST", "/items/7/edit", 200, "edit 7", nil},
			{"GET", "/items/7/edit", 405, "Method Not Allowed\n", map[string]string{"Allow": "POST"}},
			{"GET", "/files/a/b/c.txt", 200, "path [a/b/c.txt]", nil},
			{"GET", "/files/", 200, "path []", nil},
			{"GET", "/", 200, "root", nil},
			{"GET", "/nothing", 404, "404 page not found\n", map[string]string{"X-Content-Type-Options": "nosniff"}},
			{"GET", "/status/201", 201, "body", nil},
			{"GET", "/status/204", 204, "", nil},
			{"GET", "/status/304", 304, "", nil},
			{"GET", "/status/599", 599, "body", nil},
			{"GET", "/json", 200, `{"ok":true}`, map[string]string{"Content-Type": "application/json"}},
			{"GET", "/html", 200, "<!DOCTYPE html><html><body>hi</body></html>", map[string]string{"Content-Type": "text/html; charset=utf-8"}},
			{"GET", "/empty", 200, "", map[string]string{"Content-Length": "0"}},
			{"GET", "/declared", 200, "0123456789", map[string]string{"Content-Length": "10"}},
			{"GET", "/dir", 307, "<a href=\"/dir/\">Temporary Redirect</a>.\n\n", map[string]string{"Location": "/dir/"}},
			{"GET", "/a/../hello", 307, "", map[string]string{"Location": "/hello"}},
			{"GET", "/hello//", 307, "", nil},
			{"GET", "/redirect", 302, "<a href=\"/hello\">Found</a>.\n\n", map[string]string{"Location": "/hello"}},
			{"GET", "/cookie", 200, "set", map[string]string{"Set-Cookie": "session=abc123; Path=/; Max-Age=60; HttpOnly; SameSite=Lax"}},
			{"OPTIONS", "/hello", 405, "Method Not Allowed\n", nil},
		}
		for _, tc := range cases {
			req, _ := http.NewRequest(tc.method, base+tc.path, nil)
			resp, err := noRedirect.Do(req)
			if err != nil {
				t.Errorf("%s %s: %v", tc.method, tc.path, err)
				continue
			}
			b, _ := io.ReadAll(resp.Body)
			resp.Body.Close()
			if resp.StatusCode != tc.status || (tc.body != "" || tc.method == "HEAD") && string(b) != tc.body {
				t.Errorf("%s %s: %d %q, want %d %q", tc.method, tc.path, resp.StatusCode, b, tc.status, tc.body)
			}
			for k, v := range tc.header {
				if got := resp.Header.Get(k); got != v {
					t.Errorf("%s %s: %s %q, want %q", tc.method, tc.path, k, got, v)
				}
			}
			if resp.Header.Get("Date") == "" {
				t.Errorf("%s %s: no Date", tc.method, tc.path)
			}
		}
	})

	t.Run("bodies", func(t *testing.T) {
		for _, n := range []int{0, 1, 100, 2047, 2048, 2049, 10000, 1 << 20, 5 << 20} {
			body := bytes.Repeat([]byte("xyz\x00\xff"), n/5+1)[:n]
			// with a length and chunked (no length)
			for _, chunked := range []bool{false, true} {
				var r io.Reader = bytes.NewReader(body)
				if chunked {
					r = io.MultiReader(bytes.NewReader(body)) // hides the length
				}
				req, _ := http.NewRequest("PUT", base+"/echo", r)
				if chunked && n == 0 {
					req.Body = io.NopCloser(strings.NewReader(""))
					req.ContentLength = -1
				}
				resp, err := c.Do(req)
				if err != nil {
					t.Fatalf("%d %v: %v", n, chunked, err)
				}
				got, err := io.ReadAll(resp.Body)
				resp.Body.Close()
				if err != nil || !bytes.Equal(got, body) || resp.Header.Get("X-Len") != fmt.Sprint(n) {
					t.Errorf("echo %d chunked=%v: %d bytes, %v, X-Len %s", n, chunked, len(got), err, resp.Header.Get("X-Len"))
				}
				wantChunkedResp := n > 2048
				if (len(resp.TransferEncoding) > 0) != wantChunkedResp {
					t.Errorf("echo %d: response transfer encoding %v", n, resp.TransferEncoding)
				}
			}
		}
		// big chunked responses
		for _, n := range []int{0, 2048, 2049, 123457, 3 << 20} {
			resp, b := get(t, c, fmt.Sprintf("%s/big?n=%d", base, n))
			ok := len(b) == n
			for i := 0; ok && i < n; i++ {
				ok = b[i] == byte('a'+i%26)
			}
			if !ok || resp.StatusCode != 200 {
				t.Errorf("big %d: %d bytes", n, len(b))
			}
		}
		// trailers in a chunked request
		pr, pw := io.Pipe()
		req, _ := http.NewRequest("POST", base+"/echo", pr)
		req.Trailer = http.Header{"X-Checksum": nil}
		go func() {
			pw.Write([]byte("part1"))
			req.Trailer.Set("X-Checksum", "abc")
			pw.Close()
		}()
		resp, err := c.Do(req)
		if err != nil {
			t.Fatal(err)
		}
		b, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		if string(b) != "part1" || resp.Header.Get("X-Trailer") != "abc" || resp.Header.Get("X-Chunked") != "1" {
			t.Errorf("trailer: %q %q", b, resp.Header)
		}
	})

	t.Run("forms_headers_streaming", func(t *testing.T) {
		resp, err := c.PostForm(base+"/form?q=1", map[string][]string{"name": {"Ann Lee"}, "x": {"a&b=c"}})
		if err != nil {
			t.Fatal(err)
		}
		b, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		if string(b) != "name=Ann Lee\nx=a&b=c\nq=1\n" {
			t.Errorf("form: %q", b)
		}
		req, _ := http.NewRequest("GET", base+"/headers?z=1", nil)
		req.Header.Set("x-custom-thing", "v1")
		req.Header.Add("X-Multi", "a")
		req.Header.Add("X-Multi", "b")
		req.AddCookie(&http.Cookie{Name: "session", Value: "s1"})
		resp, err = c.Do(req)
		if err != nil {
			t.Fatal(err)
		}
		b, _ = io.ReadAll(resp.Body)
		resp.Body.Close()
		for _, w := range []string{"X-Custom-Thing: v1\r\n", "X-Multi: a\r\nX-Multi: b\r\n", "GET /headers?z=1 host=" + addr, "cookie=s1"} {
			if !strings.Contains(string(b), w) {
				t.Errorf("headers: %q lacks %q", b, w)
			}
		}
		// streaming: the events arrive while the handler still runs
		resp, err = c.Get(base + "/stream")
		if err != nil {
			t.Fatal(err)
		}
		br := bufio.NewReader(resp.Body)
		t0 := time.Now()
		first, _ := br.ReadString('\n')
		if first != "event 0\n" || time.Since(t0) > 40*time.Millisecond {
			t.Errorf("stream: %q after %v", first, time.Since(t0))
		}
		rest, _ := io.ReadAll(br)
		resp.Body.Close()
		if string(rest) != "event 1\nevent 2\nevent 3\nevent 4\n" {
			t.Errorf("stream rest %q", rest)
		}
	})

	t.Run("keepalive", func(t *testing.T) {
		var reused, fresh int32
		ctx := httptrace.WithClientTrace(context.Background(), &httptrace.ClientTrace{GotConn: func(i httptrace.GotConnInfo) {
			if i.Reused {
				atomic.AddInt32(&reused, 1)
			} else {
				atomic.AddInt32(&fresh, 1)
			}
		}})
		tr := &http.Transport{}
		cl := &http.Client{Transport: tr}
		for i := 0; i < 50; i++ {
			req, _ := http.NewRequestWithContext(ctx, "GET", base+"/hello", nil)
			resp, err := cl.Do(req)
			if err != nil {
				t.Fatal(err)
			}
			io.Copy(io.Discard, resp.Body)
			resp.Body.Close()
		}
		if fresh != 1 || reused != 49 {
			t.Errorf("connections: %d new, %d reused", fresh, reused)
		}
		tr.CloseIdleConnections()
	})

	t.Run("concurrency", func(t *testing.T) {
		tr := &http.Transport{MaxIdleConnsPerHost: 300, MaxConnsPerHost: 0}
		cl := &http.Client{Transport: tr}
		var wg sync.WaitGroup
		var failed int32
		t0 := time.Now()
		for i := 0; i < 300; i++ {
			wg.Add(1)
			go func() {
				defer wg.Done()
				resp, err := cl.Get(base + "/sleep?ms=200")
				if err != nil {
					atomic.AddInt32(&failed, 1)
					return
				}
				b, _ := io.ReadAll(resp.Body)
				resp.Body.Close()
				if string(b) != "slept 200" {
					atomic.AddInt32(&failed, 1)
				}
			}()
		}
		wg.Wait()
		took := time.Since(t0)
		if failed > 0 || took > 3*time.Second {
			t.Errorf("300 concurrent 200 ms requests: %d failed, took %v", failed, took)
		}
		tr.CloseIdleConnections()
	})

	t.Run("files", func(t *testing.T) {
		resp, b := get(t, c, base+"/static/a.txt")
		if b != "file a\n" || resp.Header.Get("Content-Type") != "text/plain; charset=utf-8" || resp.Header.Get("Last-Modified") == "" {
			t.Errorf("a.txt: %q %v", b, resp.Header)
		}
		lm := resp.Header.Get("Last-Modified")
		resp, b = get(t, c, base+"/static/page.html")
		if b != "<p>page</p>" || resp.Header.Get("Content-Type") != "text/html; charset=utf-8" {
			t.Errorf("page.html: %q %v", b, resp.Header)
		}
		resp, _ = get(t, c, base+"/static/noext")
		if resp.Header.Get("Content-Type") != "text/html; charset=utf-8" {
			t.Errorf("sniffed: %v", resp.Header)
		}
		resp, b = get(t, c, base+"/static/sub/")
		if b != "sub index" {
			t.Errorf("index: %q", b)
		}
		resp, b = get(t, noRedirect, base+"/static/sub")
		if resp.StatusCode != 301 || resp.Header.Get("Location") != "sub/" {
			t.Errorf("dir redirect: %d %v", resp.StatusCode, resp.Header)
		}
		resp, b = get(t, noRedirect, base+"/static/sub/index.html")
		if resp.StatusCode != 301 || resp.Header.Get("Location") != "./" {
			t.Errorf("index redirect: %d %v", resp.StatusCode, resp.Header)
		}
		resp, b = get(t, c, base+"/static/")
		for _, w := range []string{"<pre>\n", `<a href="a.txt">a.txt</a>`, `<a href="sub/">sub/</a>`} {
			if !strings.Contains(b, w) {
				t.Errorf("listing lacks %q:\n%s", w, b)
			}
		}
		resp, b = get(t, c, base+"/static/sub/deeper/x.json")
		if b != `{"x":1}` || resp.Header.Get("Content-Type") != "application/json" {
			t.Errorf("json: %q %v", b, resp.Header)
		}
		resp, _ = get(t, c, base+"/static/missing")
		if resp.StatusCode != 404 {
			t.Errorf("missing: %d", resp.StatusCode)
		}
		// no way out of the root
		for _, p := range []string{"/static/../secret.txt", "/static/%2e%2e/secret.txt", "/static/sub/../../secret.txt"} {
			conn, err := net.Dial("tcp", addr)
			if err != nil {
				t.Fatal(err)
			}
			fmt.Fprintf(conn, "GET %s HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n", p)
			all, _ := io.ReadAll(conn)
			conn.Close()
			if strings.Contains(string(all), "secret") && !strings.Contains(string(all), "Location") {
				t.Errorf("%s escaped the root:\n%s", p, all)
			}
		}
		// conditional and range requests
		req, _ := http.NewRequest("GET", base+"/static/a.txt", nil)
		req.Header.Set("If-Modified-Since", lm)
		resp, _ = c.Do(req)
		resp.Body.Close()
		if resp.StatusCode != 304 {
			t.Errorf("If-Modified-Since: %d", resp.StatusCode)
		}
		for _, rc := range []struct {
			rng, want string
			status    int
		}{{"bytes=0-3", "file", 206}, {"bytes=5-", "a\n", 206}, {"bytes=-2", "a\n", 206}, {"bytes=100-", "", 416}, {"bytes=x", "file a\n", 200}} {
			req, _ := http.NewRequest("GET", base+"/static/a.txt", nil)
			req.Header.Set("Range", rc.rng)
			resp, err := c.Do(req)
			if err != nil {
				t.Fatal(err)
			}
			b, _ := io.ReadAll(resp.Body)
			resp.Body.Close()
			if resp.StatusCode != rc.status || (rc.status != 416 && string(b) != rc.want) {
				t.Errorf("Range %s: %d %q", rc.rng, resp.StatusCode, b)
			}
		}
		resp, b = get(t, c, base+"/static/big.bin")
		want, _ := os.ReadFile(filepath.Join(root, "big.bin"))
		if b != string(want) || resp.ContentLength != int64(len(want)) {
			t.Errorf("big.bin: %d bytes", len(b))
		}
	})

	t.Run("raw", func(t *testing.T) {
		// requests Go's client cannot send; the answers are Go's server's
		cases := []struct{ req, want string }{
			{"GET /hello HTTP/1.0\r\n\r\n", "HTTP/1.0 200 OK"},
			{"GET /hello HTTP/1.0\r\nConnection: keep-alive\r\n\r\n", "Connection: keep-alive"},
			{"GET /hello HTTP/1.1\r\n\r\n", "HTTP/1.1 400 Bad Request"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nHost: b\r\n\r\n", "HTTP/1.1 400 Bad Request"},
			{"GET /hello HTTP/2.0\r\nHost: a\r\n\r\n", "HTTP/1.1 505 HTTP Version Not Supported"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: gzip\r\n\r\n", "HTTP/1.1 501 Not Implemented"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\nContent-Length: 2\r\n\r\nxy", "HTTP/1.1 400 Bad Request"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nX: " + strings.Repeat("a", 40000) + "\r\n\r\n", "HTTP/1.1 431 Request Header Fields Too Large"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nBad Name: x\r\n\r\n", "HTTP/1.1 400 Bad Request"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nX: a\x01b\r\n\r\n", "HTTP/1.1 400 Bad Request"},
			{"G@T /hello HTTP/1.1\r\nHost: a\r\n\r\n", "HTTP/1.1 400 Bad Request"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nExpect: something\r\n\r\n", "HTTP/1.1 417 Expectation Failed"},
			{"OPTIONS * HTTP/1.1\r\nHost: a\r\n\r\n", "HTTP/1.1 200 OK"},
			{"GET /hello HTTP/1.1\r\nHost: a\r\nX: folded\r\n  value\r\nConnection: close\r\n\r\n", "HTTP/1.1 200 OK"},
			{"POST /echo HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n", "X-Len: 5"},
			{"POST /echo HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n5\nhello\r\n0\r\n\r\n", "HTTP/1.1 400 Bad Request"},
			{"POST /echo HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\nContent-Length: 3\r\n\r\n2\r\nhi\r\n0\r\n\r\n", "X-Len: 2"},
			{"GET http://other/hello HTTP/1.1\r\nHost: a\r\n\r\n", "HTTP/1.1 200 OK"},
		}
		for _, tc := range cases {
			conn, err := net.Dial("tcp", addr)
			if err != nil {
				t.Fatal(err)
			}
			conn.SetDeadline(time.Now().Add(5 * time.Second))
			io.WriteString(conn, tc.req)
			if cw, ok := conn.(*net.TCPConn); ok && strings.HasPrefix(tc.req, "GET /hello HTTP/1.0") {
				cw.CloseWrite()
			}
			br := bufio.NewReader(conn)
			var head strings.Builder
			for {
				l, err := br.ReadString('\n')
				head.WriteString(l)
				if err != nil || l == "\r\n" {
					break
				}
			}
			conn.Close()
			if !strings.Contains(head.String(), tc.want) {
				t.Errorf("%q:\n%s\nwant %q", tc.req[:min(len(tc.req), 70)], head.String(), tc.want)
			}
		}
		// pipelined requests on one connection
		conn, _ := net.Dial("tcp", addr)
		conn.SetDeadline(time.Now().Add(5 * time.Second))
		io.WriteString(conn, strings.Repeat("GET /hello HTTP/1.1\r\nHost: a\r\n\r\n", 10)+"GET /hello HTTP/1.1\r\nHost: a\r\nConnection: close\r\n\r\n")
		all, _ := io.ReadAll(conn)
		conn.Close()
		if n := strings.Count(string(all), "hello, world\n"); n != 11 {
			t.Errorf("pipelining: %d responses", n)
		}
		// Expect: 100-continue
		conn, _ = net.Dial("tcp", addr)
		conn.SetDeadline(time.Now().Add(5 * time.Second))
		io.WriteString(conn, "POST /echo HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\nExpect: 100-continue\r\n\r\n")
		br := bufio.NewReader(conn)
		l, _ := br.ReadString('\n')
		if l != "HTTP/1.1 100 Continue\r\n" {
			t.Errorf("100-continue: %q", l)
		}
		br.ReadString('\n')
		io.WriteString(conn, "data")
		l, _ = br.ReadString('\n')
		if l != "HTTP/1.1 200 OK\r\n" {
			t.Errorf("after continue: %q", l)
		}
		conn.Close()
		// hijacking
		resp, b := get(t, c, base+"/hijack")
		if resp.StatusCode != 200 || b != "hijacked" {
			t.Errorf("hijack: %d %q", resp.StatusCode, b)
		}
	})

	t.Run("tools", func(t *testing.T) {
		if curl, err := exec.LookPath("curl"); err == nil {
			out, err := exec.Command(curl, "-s", "-i", "--http1.1", base+"/items/9").CombinedOutput()
			if err != nil || !strings.Contains(string(out), "item 9") || !strings.Contains(string(out), "HTTP/1.1 200 OK") {
				t.Errorf("curl: %v\n%s", err, out)
			}
			out, err = exec.Command(curl, "-s", "--data-binary", "@-", base+"/echo").CombinedOutput()
			_ = out
			if err != nil {
				t.Errorf("curl post: %v", err)
			}
		}
		if py, err := exec.LookPath("python3"); err == nil {
			script := `import http.client,sys
c=http.client.HTTPConnection(sys.argv[1])
for i in range(3):
    c.request("POST","/echo",body=b"py"*1000,headers={"X-A":"1"})
    r=c.getresponse(); d=r.read()
    assert r.status==200 and d==b"py"*1000, (r.status,len(d))
c.request("GET","/big?n=50000"); r=c.getresponse(); assert len(r.read())==50000
print("ok")`
			out, err := exec.Command(py, "-c", script, addr).CombinedOutput()
			if err != nil || strings.TrimSpace(string(out)) != "ok" {
				t.Errorf("python: %v\n%s", err, out)
			}
		}
	})

	// a request in flight while the server shuts down still completes
	done := make(chan string, 1)
	go func() {
		resp, err := http.Get(base + "/sleep?ms=300")
		if err != nil {
			done <- err.Error()
			return
		}
		b, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		done <- string(b)
	}()
	time.Sleep(100 * time.Millisecond)
	if err := stop(); err != nil {
		t.Error(err)
	}
	if got := <-done; got != "slept 300" {
		t.Errorf("in-flight request during shutdown: %q", got)
	}
}

// startProg builds and starts a server program of std/tests/prog
func startProg(t *testing.T, name string, args ...string) string {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", name+".c"), filepath.Join(tempDir(t), name))
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
		t.Fatalf("%s did not start: %q %v\n%s", name, line, err, stderr.String())
	}
	t.Cleanup(func() {
		stdin.Close()
		done := make(chan struct{})
		go func() { cmd.Wait(); close(done) }()
		select {
		case <-done:
		case <-time.After(5 * time.Second):
			cmd.Process.Kill()
		}
	})
	return strings.TrimSpace(strings.TrimPrefix(line, "listening "))
}

func goDump(w http.ResponseWriter, r *http.Request) {
	var b strings.Builder
	chunked := len(r.TransferEncoding) > 0 && r.TransferEncoding[0] == "chunked"
	fmt.Fprintf(&b, "%s|%s|%s|%s|%d|%d|%d|", r.Method, r.RequestURI, r.Proto, r.Host, r.ContentLength, b2i(chunked), b2i(r.Close))
	var hb bytes.Buffer
	h := r.Header.Clone()
	h.Write(&hb)
	b.WriteString(hex.EncodeToString(hb.Bytes()) + "|")
	body, err := io.ReadAll(io.LimitReader(r.Body, 1<<20))
	if err != nil {
		b.WriteString("bodyerr")
	} else {
		b.WriteString(hex.EncodeToString(body))
	}
	b.WriteString("|")
	var tb bytes.Buffer
	r.Trailer.Write(&tb)
	b.WriteString(hex.EncodeToString(tb.Bytes()))
	w.Header().Set("Content-Type", "text/plain")
	io.WriteString(w, b.String())
}

// exchange sends raw bytes and returns the status code and body of the
// first response (or "closed")
func exchange(addr string, raw []byte) string {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return "dial: " + err.Error()
	}
	defer conn.Close()
	conn.SetDeadline(time.Now().Add(400 * time.Millisecond))
	conn.Write(raw)
	resp, err := http.ReadResponse(bufio.NewReader(conn), nil)
	if err != nil {
		return "closed"
	}
	defer resp.Body.Close()
	b, _ := io.ReadAll(resp.Body)
	if resp.StatusCode != 200 {
		return fmt.Sprint(resp.StatusCode)
	}
	return "200 " + string(b)
}

// TestHTTPParseAgainstGo sends the same raw requests to Go's server and
// to AltC's and compares what each parsed (or the status of the error)
func TestHTTPParseAgainstGo(t *testing.T) {
	altc := startProg(t, "dumpd")
	gl, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	gs := &http.Server{Handler: http.HandlerFunc(goDump), MaxHeaderBytes: 8192, ErrorLog: nil}
	go gs.Serve(gl)
	defer gs.Close()
	goAddr := gl.Addr().String()

	r := rand.New(rand.NewSource(5))
	var inputs [][]byte
	add := func(s string) { inputs = append(inputs, []byte(s)) }
	for _, s := range []string{
		"GET / HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET /x?y=1 HTTP/1.1\r\nHost: a:8080\r\nAccept: */*\r\n\r\n",
		"GET / HTTP/1.0\r\n\r\n",
		"GET / HTTP/1.0\r\nConnection: Keep-Alive\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nConnection: close, x\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: 5\r\n\r\nhello",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: 05\r\n\r\nhello",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: +5\r\n\r\nhello",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: 5\r\nContent-Length: 5\r\n\r\nhello",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: 5, 5\r\n\r\nhello",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length:\r\n\r\nhello",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: 99999999999999999999\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nContent-Length: 10\r\n\r\nshort",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: Chunked\r\n\r\n3;ext=1\r\nabc\r\n0\r\nX-T: 1\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\nTrailer: X-T\r\n\r\n3\r\nabc\r\n0\r\nX-T: 1\r\nY: 2\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\nA\r\n0123456789\r\n0\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n3 \r\nabc\r\n0\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\nzz\r\nabc\r\n0\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabcXX0\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n10000000000000000\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked, identity\r\n\r\n",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
		"POST /p HTTP/1.0\r\nTransfer-Encoding: chunked\r\nContent-Length: 3\r\n\r\nabc",
		"POST /p HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\nContent-Length: 100\r\n\r\n1\r\na\r\n0\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nX: a\r\n b\r\n\tc\r\n\r\n",
		"GET / HTTP/1.1\r\n Host: a\r\n\r\n",
		"GET / HTTP/1.1\nHost: a\n\n",
		"GET / HTTP/1.1\r\nHost: a\r\nX:\r\nY: \t v \t\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nx-lower-case: v\r\nMIXED-case: v\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nX: \x80\xff\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nX: \x7f\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nNo-Colon\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\n: empty\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a b\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: [::1]:80\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\x00b\r\n\r\n",
		"GET http://h.com/p?q HTTP/1.1\r\nHost: other\r\n\r\n",
		"GET http://h.com HTTP/1.1\r\nHost: other\r\n\r\n",
		"CONNECT h.com:443 HTTP/1.1\r\nHost: h.com:443\r\n\r\n",
		"CONNECT /path HTTP/1.1\r\nHost: a\r\n\r\n",
		"OPTIONS * HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET * HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET rel HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET  / HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET / HTTP/1.1 \r\nHost: a\r\n\r\n",
		"GET / http/1.1\r\nHost: a\r\n\r\n",
		"GET / HTTP/1.2\r\nHost: a\r\n\r\n",
		"GET / HTTP/0.9\r\nHost: a\r\n\r\n",
		"GET / HTTP/11.1\r\nHost: a\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\n",
		"GET /%zz HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET /a b HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET /\x01 HTTP/1.1\r\nHost: a\r\n\r\n",
		"get / HTTP/1.1\r\nHost: a\r\n\r\n",
		"X-CUSTOM / HTTP/1.1\r\nHost: a\r\n\r\n",
		"GE(T / HTTP/1.1\r\nHost: a\r\n\r\n",
		"\r\nGET / HTTP/1.1\r\nHost: a\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nX: " + strings.Repeat("v", 9000) + "\r\n\r\n",
		"GET / HTTP/1.1\r\nHost: a\r\nX: " + strings.Repeat("v", 4000) + "\r\n\r\n",
	} {
		add(s)
	}
	// random headers and bodies built from pieces
	names := []string{"Host", "Content-Length", "Transfer-Encoding", "Connection", "X", "x-y", "Accept", "Bad Name", "a_b", "Ä", "Cookie"}
	values := []string{"a", "", " 5 ", "5", "0", "chunked", "close", "keep-alive", "v\tw", "\x01", "\xff", "1,2", "  lead", "trail  "}
	for i := 0; i < 1500; i++ {
		var b strings.Builder
		b.WriteString([]string{"GET", "POST", "PUT", "HEAD", "DELETE"}[r.Intn(5)])
		b.WriteString(" ")
		b.WriteString([]string{"/", "/a?b=c", "/x%20y", "http://h/z", "/../a", "//x"}[r.Intn(6)])
		b.WriteString([]string{" HTTP/1.1", " HTTP/1.0", " HTTP/1.1"}[r.Intn(3)])
		eol := "\r\n"
		if r.Intn(10) == 0 {
			eol = "\n"
		}
		b.WriteString(eol)
		hasHost := false
		for k := r.Intn(5); k >= 0; k-- {
			n := names[r.Intn(len(names))]
			if n == "Host" {
				hasHost = true
			}
			b.WriteString(n + ": " + values[r.Intn(len(values))] + eol)
		}
		if !hasHost && r.Intn(4) > 0 {
			b.WriteString("Host: h" + eol)
		}
		b.WriteString(eol)
		switch r.Intn(4) {
		case 0:
			b.WriteString("hello")
		case 1:
			fmt.Fprintf(&b, "%x\r\n%s\r\n0\r\n\r\n", 3, "abc")
		case 2:
			b.WriteString("5\r\nab")
		}
		add(b.String())
	}
	bad := 0
	for _, in := range inputs {
		g, a := exchange(goAddr, in), exchange(altc, in)
		if g != a {
			bad++
			if bad <= 25 {
				t.Errorf("%q:\n  go   %s\n  altc %s", in, decodeDump(g), decodeDump(a))
			}
		}
	}
	if bad > 25 {
		t.Errorf("%d of %d requests differ", bad, len(inputs))
	}
}

// decodeDump makes the hex fields of a dump readable
func decodeDump(s string) string {
	parts := strings.Split(s, "|")
	for i, p := range parts {
		if i >= 7 {
			if b, err := hex.DecodeString(p); err == nil {
				parts[i] = fmt.Sprintf("%q", b)
			}
		}
	}
	return strings.Join(parts, "|")
}

var muxPatterns = []string{
	"/", "GET /{$}", "/a/b", "/a/b/", "GET /a/{x}", "GET /a/{x}/c", "POST /a/{x}/c", "/files/{rest...}", "GET /users/{id}/posts/{pid}",
	"GET /users/{id}/{$}", "host.example/h", "host.example/", "GET host.example/{y}", "/x%2Fy", "/esc/{v}", "GET /tree/",
	"GET /tree/leaf", "DELETE /items/{id}", "PUT /items/{id}", "HEAD /head-only", "/both/{a}/{b...}", "/z/", "/z/{q}",
	// conflicting or invalid ones
	"GET /a/{y}", "/files/{other...}", "GET /a/b/{c}/{c}", "GET /{x...}/y", "GET a", "GET /a/{x}/c ", "/a/{$}x", "GET /{bad name}",
	"GET /users/{id}/posts/{pid}", "POST /a/{z}/c", "/{x}/b", "GET /dup/{a}/{a...}", "BAD METHOD /m", "OPTIONS /o/{w}",
}

func TestMuxAgainstGo(t *testing.T) {
	// the C side
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "muxd.c"), filepath.Join(tempDir(t), "muxd"))
	cmd := exec.Command(exe)
	stdin, _ := cmd.StdinPipe()
	stdout, _ := cmd.StdoutPipe()
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	defer func() { stdin.Close(); cmd.Wait() }()
	io.WriteString(stdin, strings.Join(muxPatterns, "\n")+"\n\n")
	br := bufio.NewReader(stdout)
	// the Go side
	gm := http.NewServeMux()
	for _, p := range muxPatterns {
		line, _ := br.ReadString('\n')
		got := strings.TrimSpace(line)
		want := "ok"
		func() {
			defer func() {
				if r := recover(); r != nil {
					want = "invalid"
					if strings.Contains(fmt.Sprint(r), "conflicts with") {
						want = "conflict"
					}
				}
			}()
			pat := p
			gm.HandleFunc(pat, func(w http.ResponseWriter, r *http.Request) {
				var b strings.Builder
				b.WriteString(r.Pattern + "|")
				for _, name := range wildcardNames(r.Pattern) {
					fmt.Fprintf(&b, "%s=%s;", name, r.PathValue(name))
				}
				io.WriteString(w, b.String())
			})
		}()
		if got != want {
			t.Errorf("pattern %q: altc %s, go %s", p, got, want)
		}
	}
	line, _ := br.ReadString('\n')
	altc := strings.TrimSpace(strings.TrimPrefix(line, "listening "))
	gl, _ := net.Listen("tcp", "127.0.0.1:0")
	gs := &http.Server{Handler: gm}
	go gs.Serve(gl)
	defer gs.Close()

	r := rand.New(rand.NewSource(6))
	segs := []string{"a", "b", "c", "files", "users", "posts", "tree", "leaf", "items", "head-only", "both", "z", "x%2Fy", "esc", "h",
		"", ".", "..", "%2E", "%2F", "x y", "%41", "q", "1", "{$}"}
	methods := []string{"GET", "HEAD", "POST", "PUT", "DELETE", "OPTIONS", "PATCH"}
	hosts := []string{"other", "host.example", "host.example:80", "HOST.example"}
	var reqs []string
	for i := 0; i < 3000; i++ {
		var p strings.Builder
		for k := r.Intn(5); k >= 0; k-- {
			p.WriteString("/" + segs[r.Intn(len(segs))])
		}
		if r.Intn(4) == 0 {
			p.WriteString("/")
		}
		q := ""
		if r.Intn(5) == 0 {
			q = "?k=v"
		}
		reqs = append(reqs, fmt.Sprintf("%s %s%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", methods[r.Intn(len(methods))],
			strings.ReplaceAll(p.String(), " ", "%20"), q, hosts[r.Intn(len(hosts))]))
	}
	bad := 0
	for _, req := range reqs {
		g, a := muxExchange(gl.Addr().String(), req), muxExchange(altc, req)
		if g != a {
			bad++
			if bad <= 20 {
				t.Errorf("%q:\n  go   %s\n  altc %s", strings.SplitN(req, "\r\n", 3)[:2], g, a)
			}
		}
	}
	if bad > 20 {
		t.Errorf("%d of %d requests differ", bad, len(reqs))
	}
}

func wildcardNames(pattern string) []string {
	var names []string
	for {
		i := strings.IndexByte(pattern, '{')
		if i < 0 {
			return names
		}
		j := strings.IndexByte(pattern[i:], '}')
		if j < 0 {
			return names
		}
		n := strings.TrimSuffix(pattern[i+1:i+j], "...")
		if n != "$" {
			names = append(names, n)
		}
		pattern = pattern[i+j+1:]
	}
}

func muxExchange(addr, raw string) string {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return err.Error()
	}
	defer conn.Close()
	conn.SetDeadline(time.Now().Add(2 * time.Second))
	io.WriteString(conn, raw)
	method := strings.SplitN(raw, " ", 2)[0]
	resp, err := http.ReadResponse(bufio.NewReader(conn), &http.Request{Method: method})
	if err != nil {
		return "error " + err.Error()
	}
	b, _ := io.ReadAll(resp.Body)
	resp.Body.Close()
	return fmt.Sprintf("%d loc=%q allow=%q %q", resp.StatusCode, resp.Header.Get("Location"), resp.Header.Get("Allow"), b)
}

// TestHTTPClientAgainstGo: the library's client and Go's against the same
// Go server: transparent gzip as Go's Transport does it, chunked bodies,
// redirects, HEAD and bodiless statuses
func TestHTTPClientAgainstGo(t *testing.T) {
	big := bytes.Repeat([]byte("compressible text "), 5000)
	gz := func(b []byte) []byte {
		var buf bytes.Buffer
		w := gzip.NewWriter(&buf)
		w.Write(b)
		w.Close()
		return buf.Bytes()
	}
	mux := http.NewServeMux()
	mux.HandleFunc("/gzip", func(w http.ResponseWriter, r *http.Request) {
		if !strings.Contains(r.Header.Get("Accept-Encoding"), "gzip") {
			w.Write(big)
			return
		}
		w.Header().Set("Content-Encoding", "gzip")
		w.Write(gz(big))
	})
	mux.HandleFunc("/always-gzip", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Encoding", "gzip")
		w.Write(gz([]byte("always")))
	})
	mux.HandleFunc("/bad-gzip", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Encoding", "gzip")
		b := gz([]byte("broken body"))
		b[len(b)-5] ^= 0xff
		w.Write(b)
	})
	mux.HandleFunc("/empty-gzip", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Encoding", "gzip")
		w.WriteHeader(204)
	})
	mux.HandleFunc("/ae", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "ae=%q range=%q", r.Header.Get("Accept-Encoding"), r.Header.Get("Range"))
	})
	mux.HandleFunc("/chunked", func(w http.ResponseWriter, r *http.Request) {
		for i := 0; i < 50; i++ {
			fmt.Fprintf(w, "chunk %d\n", i)
			w.(http.Flusher).Flush()
		}
	})
	mux.HandleFunc("/redirect", func(w http.ResponseWriter, r *http.Request) { http.Redirect(w, r, "/gzip", http.StatusFound) })
	mux.HandleFunc("/status/", func(w http.ResponseWriter, r *http.Request) {
		var code int
		fmt.Sscanf(strings.TrimPrefix(r.URL.Path, "/status/"), "%d", &code)
		w.WriteHeader(code)
		if code != 204 && code != 304 {
			w.Write([]byte("status body"))
		}
	})
	srv := httptest.NewServer(mux)
	defer srv.Close()
	reqs := [][]string{
		{"GET", "/gzip"}, {"GET", "/always-gzip"}, {"GET", "/bad-gzip"}, {"GET", "/empty-gzip"}, {"GET", "/ae"},
		{"GET", "/ae", "Range: bytes=0-1"}, {"GET", "/ae", "Accept-Encoding: br"}, {"GET", "/gzip", "Accept-Encoding: gzip"},
		{"HEAD", "/gzip"}, {"GET", "/chunked"}, {"GET", "/redirect"}, {"GET", "/status/404"}, {"GET", "/status/204"},
		{"GET", "/status/304"}, {"POST", "/ae"}, {"GET", "/gzip"},
	}
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "fetch.c"), filepath.Join(tempDir(t), "fetch"))
	for _, mode := range []string{"gzip", "nogzip"} {
		var lines, want []string
		client := &http.Client{Transport: &http.Transport{DisableCompression: mode == "nogzip"}}
		for _, r := range reqs {
			lines = append(lines, strings.Join(append([]string{r[0], srv.URL + r[1]}, r[2:]...), "\t"))
			req, _ := http.NewRequest(r[0], srv.URL+r[1], nil)
			for _, h := range r[2:] {
				k, v, _ := strings.Cut(h, ":")
				req.Header.Set(k, strings.TrimSpace(v))
			}
			resp, err := client.Do(req)
			if err != nil {
				want = append(want, "error "+err.Error())
				continue
			}
			b, err := io.ReadAll(resp.Body)
			resp.Body.Close()
			w := fmt.Sprintf("%d|%d|%d|%s|", resp.StatusCode, b2i(resp.Uncompressed), resp.ContentLength, resp.Header.Get("Content-Encoding"))
			if err != nil {
				w += "body error"
			} else {
				w += hex.EncodeToString(b)
			}
			want = append(want, w)
		}
		cmd := exec.Command(exe, mode)
		cmd.Stdin = strings.NewReader(strings.Join(lines, "\n") + "\n")
		out, err := cmd.Output()
		if err != nil {
			t.Fatalf("fetch: %v", err)
		}
		got := strings.Split(strings.TrimSuffix(string(out), "\n"), "\n")
		if len(got) != len(want) {
			t.Fatalf("%d answers for %d", len(got), len(want))
		}
		for i := range want {
			g := got[i]
			if strings.HasPrefix(g, "200|1|-1||body error") && strings.HasSuffix(want[i], "body error") {
				continue // the error texts differ
			}
			if g != want[i] {
				t.Errorf("%s %v:\n got %.300s\nwant %.300s", mode, reqs[i], g, want[i])
			}
		}
	}
}
