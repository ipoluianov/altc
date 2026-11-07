package tests

// Tests of <alt/net/websocket.h>: interoperability with gorilla/websocket
// and golang.org/x/net/websocket (both run by tests/goref), a raw-frame
// conformance suite in the style of the Autobahn testsuite against the
// library's server and client, the opening handshakes, and
// permessage-deflate checked with Go's compress/flate.

import (
	"bufio"
	"bytes"
	"compress/flate"
	"crypto/sha1"
	"encoding/base64"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"
	"math/rand"
	"net"
	"net/http"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
	"unicode/utf8"
)

// ---- the programs ----

var wsExes sync.Map // name -> *wsExe

type wsExe struct {
	once sync.Once
	path string
}

// wsProg builds std/tests/prog/<name>.c once
func wsProg(t *testing.T, name string) string {
	t.Helper()
	v, _ := wsExes.LoadOrStore(name, &wsExe{})
	e := v.(*wsExe)
	e.once.Do(func() {
		e.path = buildStdTest(t, filepath.Join("..", "std", "tests", "prog", name+".c"), filepath.Join(scratch, "ws_"+name))
	})
	if e.path == "" {
		t.Fatalf("%s did not build", name)
	}
	return e.path
}

// a server process: "listening <addr>" first, then event lines
type wsServerProc struct {
	addr   string
	lines  chan string
	stderr *bytes.Buffer
}

func startWSProc(t *testing.T, exe string, args ...string) *wsServerProc {
	t.Helper()
	cmd := exec.Command(exe, args...)
	stdin, _ := cmd.StdinPipe()
	stdout, _ := cmd.StdoutPipe()
	p := &wsServerProc{lines: make(chan string, 10000), stderr: &bytes.Buffer{}}
	cmd.Stderr = p.stderr
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	sc := bufio.NewScanner(stdout)
	if !sc.Scan() || !strings.HasPrefix(sc.Text(), "listening ") {
		cmd.Process.Kill()
		t.Fatalf("%s did not start: %q\n%s", exe, sc.Text(), p.stderr)
	}
	p.addr = strings.TrimPrefix(sc.Text(), "listening ")
	go func() {
		for sc.Scan() {
			p.lines <- sc.Text()
		}
		close(p.lines)
	}()
	t.Cleanup(func() {
		stdin.Close()
		done := make(chan struct{})
		go func() { cmd.Wait(); close(done) }()
		select {
		case <-done:
		case <-time.After(5 * time.Second):
			cmd.Process.Kill()
			<-done
		}
	})
	return p
}

// next waits for an event line with the prefix (skipping others)
func (p *wsServerProc) next(t *testing.T, prefix string) string {
	t.Helper()
	timeout := time.After(10 * time.Second)
	for {
		select {
		case l, ok := <-p.lines:
			if !ok {
				t.Fatalf("server ended waiting for %q\n%s", prefix, p.stderr)
			}
			if strings.HasPrefix(l, prefix) {
				return l
			}
		case <-timeout:
			t.Fatalf("no %q from the server", prefix)
		}
	}
}

func startWSD(t *testing.T, args ...string) *wsServerProc {
	return startWSProc(t, wsProg(t, "wsd"), args...)
}

func startGoWSServer(t *testing.T, lib string, args ...string) *wsServerProc {
	return startWSProc(t, goRefBinary(t), append([]string{"ws-server", "-lib", lib}, args...)...)
}

func runLines(t *testing.T, cmd *exec.Cmd, input string) []string {
	t.Helper()
	cmd.Stdin = strings.NewReader(input)
	var out, errb bytes.Buffer
	cmd.Stdout, cmd.Stderr = &out, &errb
	done := make(chan error, 1)
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	go func() { done <- cmd.Wait() }()
	select {
	case err := <-done:
		if err != nil {
			t.Fatalf("%s: %v\n%s\n%s", cmd.Path, err, out.String(), errb.String())
		}
	case <-time.After(90 * time.Second):
		cmd.Process.Kill()
		<-done
		t.Fatalf("%s timed out\n%s\n%s", cmd.Path, out.String(), errb.String())
	}
	s := strings.TrimSuffix(out.String(), "\n")
	if s == "" {
		return nil
	}
	return strings.Split(s, "\n")
}

// runWSC runs the library's scripted client
func runWSC(t *testing.T, script ...string) []string {
	t.Helper()
	return runLines(t, exec.Command(wsProg(t, "wsc")), strings.Join(script, "\n")+"\n")
}

// runGoWSClient runs a scripted gorilla or x/net client
func runGoWSClient(t *testing.T, lib, url, script string, args ...string) []string {
	t.Helper()
	return runLines(t, exec.Command(goRefBinary(t), append([]string{"ws-client", "-lib", lib, "-url", url, "-script", script}, args...)...), "")
}

func wantLines(t *testing.T, what string, got []string, want ...string) {
	t.Helper()
	if len(got) != len(want) {
		t.Errorf("%s: %d lines, want %d:\n got %q\nwant %q", what, len(got), len(want), got, want)
		return
	}
	for i := range want {
		if !strings.HasPrefix(got[i], want[i]) {
			t.Errorf("%s: line %d:\n got %q\nwant %q", what, i, got[i], want[i])
		}
	}
}

// ---- interoperability ----

// TestWebSocketServerGorilla: gorilla's client against the library's server
func TestWebSocketServerGorilla(t *testing.T) {
	p := makePKI(t)
	for _, srvArgs := range [][]string{{}, {"-compress"}, {"-compress", "-snct", "-cnct", "-frame", "300"}} {
		name := strings.Join(srvArgs, " ")
		t.Run("args="+name, func(t *testing.T) {
			s := startWSD(t, append([]string{"-protos", "superchat,chat"}, srvArgs...)...)
			u := "ws://" + s.addr
			compressed := len(srvArgs) > 0
			for _, comp := range []bool{false, true} {
				var flags []string
				ext := ""
				if comp {
					flags = []string{"-compress"}
					if compressed {
						ext = "permessage-deflate; server_no_context_takeover; client_no_context_takeover"
					}
				}
				wantLines(t, "echo", runGoWSClient(t, "gorilla", u+"/echo", "echo", flags...), "dial ok proto= ext="+ext, "echo 20 ok")
				s.next(t, "end /echo EUNEXPECTED_EOF 1006")
				wantLines(t, "stream", runGoWSClient(t, "gorilla", u+"/echo", "stream", flags...), "dial ok", "stream 10 ok")
				s.next(t, "end /echo")
				wantLines(t, "stream path", runGoWSClient(t, "gorilla", u+"/stream", "stream", flags...), "dial ok", "stream 10 ok")
				s.next(t, "end /stream")
			}
			wantLines(t, "ping", runGoWSClient(t, "gorilla", u+"/echo", "ping"), "dial ok", "pong p1")
			s.next(t, "end /echo")
			// the client starts the close handshake: the server answers with the code
			wantLines(t, "close", runGoWSClient(t, "gorilla", u+"/echo", "close"), "dial ok", `closed 4001 ""`)
			if l := s.next(t, "end /echo"); l != "end /echo ECLOSED 4001 bye" {
				t.Errorf("server: %q", l)
			}
			// the server starts it
			wantLines(t, "server close", runGoWSClient(t, "gorilla", u+"/close?code=4002&reason=srv", "read"), "dial ok", `closed 4002 "srv"`)
			if l := s.next(t, "end /close"); l != "end /close ECLOSED 4002 " {
				t.Errorf("server: %q", l)
			}
			// the server pings: gorilla answers, the server says so
			wantLines(t, "server ping", runGoWSClient(t, "gorilla", u+"/ping", "close"), "dial ok", `closed 4001 ""`)
			wantLines(t, "subprotocols", runGoWSClient(t, "gorilla", u+"/info", "close", "-proto", "chat,superchat"),
				"dial ok proto=superchat", `closed 4001`)
			wantLines(t, "bad utf-8", runGoWSClient(t, "gorilla", u+"/echo", "badutf8"), "dial ok", "closed 1007")
			if l := s.next(t, "end /echo"); !strings.HasPrefix(l, "end /echo EILSEQ") {
				t.Errorf("server: %q", l)
			}
		})
	}
	t.Run("limit", func(t *testing.T) {
		s := startWSD(t, "-limit", "1000000")
		wantLines(t, "big", runGoWSClient(t, "gorilla", "ws://"+s.addr+"/echo", "big"), "dial ok", "closed 1009")
		if l := s.next(t, "end /echo"); !strings.HasPrefix(l, "end /echo EMSGSIZE") {
			t.Errorf("server: %q", l)
		}
	})
	t.Run("wss", func(t *testing.T) {
		s := startWSD(t, "-tls", p.leaves["p256"][0], p.leaves["p256"][1], "-compress")
		u := "wss://localhost:" + strings.Split(s.addr, ":")[1]
		wantLines(t, "echo", runGoWSClient(t, "gorilla", u+"/echo", "echo", "-roots", p.caPEM), "dial ok", "echo 20 ok")
		wantLines(t, "echo compressed", runGoWSClient(t, "gorilla", u+"/echo", "echo", "-roots", p.caPEM, "-compress"),
			"dial ok proto= ext=permessage-deflate", "echo 20 ok")
		wantLines(t, "close", runGoWSClient(t, "gorilla", u+"/echo", "close", "-roots", p.caPEM), "dial ok", `closed 4001 ""`)
	})
}

// TestWebSocketServerXNet: x/net's client against the library's server
func TestWebSocketServerXNet(t *testing.T) {
	s := startWSD(t, "-compress", "-protos", "chat")
	u := "ws://" + s.addr
	wantLines(t, "echo", runGoWSClient(t, "xnet", u+"/echo", "echo"), "dial ok", "echo 20 ok")
	if l := s.next(t, "end /echo"); l != "end /echo ECLOSED 1000 " {
		t.Errorf("server: %q", l)
	}
	wantLines(t, "info", runGoWSClient(t, "xnet", u+"/info", "read", "-proto", "chat"), "dial ok", "msg 1 proto=chat ext=")
	wantLines(t, "ping", runGoWSClient(t, "xnet", u+"/echo", "ping"), "dial ok", "msg <nil> after")
	p := makePKI(t)
	st := startWSD(t, "-tls", p.leaves["rsa"][0], p.leaves["rsa"][1])
	wantLines(t, "wss echo", runGoWSClient(t, "xnet", "wss://localhost:"+strings.Split(st.addr, ":")[1]+"/echo", "echo", "-roots", p.caPEM),
		"dial ok", "echo 20 ok")
}

// TestWebSocketClientGorilla: the library's client against gorilla's server
func TestWebSocketClientGorilla(t *testing.T) {
	s := startGoWSServer(t, "gorilla")
	u := "ws://" + s.addr
	const nct = "permessage-deflate; server_no_context_takeover; client_no_context_takeover"
	for _, opts := range []string{"", "\tcompress", "\tcompress\tnct", "\tcompress\tframe=100\tlevel=9"} {
		ext := ""
		if opts != "" {
			ext = nct
		}
		echos, n := "echos", "40"
		if strings.Contains(opts, "level=9") {
			echos, n = "echos\t1048576", "36" // our level 9 is slow on 5 MiB of words
		}
		wantLines(t, "echos"+opts, runWSC(t, "dial\t"+u+"/echo"+opts, echos, "close\t1000\tdone"),
			"dial ok status=101 proto= ext="+ext, "echos ok "+n, "close ok peer=1000 ")
		if l := s.next(t, "end /echo"); l != "end /echo 1000 done" {
			t.Errorf("gorilla: %q", l)
		}
	}
	wantLines(t, "ping", runWSC(t, "dial\t"+u+"/echo", "ping\thello", "ping\t", "close\t1001\t"), "dial ok", "pong hello", "pong ", "close ok peer=1001")
	// gorilla pings: the library answers, gorilla's pong handler sends a text
	wantLines(t, "server ping", runWSC(t, "dial\t"+u+"/ping", "recv", "close\t1000\t"),
		"dial ok", "msg text 10 706f6e673a6770696e67 ", "close ok peer=1000")
	// gorilla starts the close handshake: the library answers with its code
	wantLines(t, "server close", runWSC(t, "dial\t"+u+"/close?code=4003&reason=x", "recv"), "dial ok",
		"err ECLOSED code=4003 reason=x sent=4003")
	if l := s.next(t, "end /close"); l != "end /close 4003 " {
		t.Errorf("gorilla: %q", l)
	}
	wantLines(t, "subprotocol", runWSC(t, "dial\t"+u+"/info\tproto=superchat,chat\thdr=X-Test:42\thdr=Origin:http://example.com", "recv"),
		"dial ok status=101 proto=chat", "msg text")
	out := runWSC(t, "dial\t"+u+"/info\tproto=superchat,chat\thdr=X-Test:42\thdr=Origin:http://example.com\tcompress", "recv")
	if len(out) != 2 {
		t.Fatalf("%q", out)
	}
	msg, _ := hex.DecodeString(strings.Fields(out[1])[3])
	if want := "proto=chat ext=permessage-deflate origin=http://example.com x=42"; !strings.HasPrefix(want, string(msg)) {
		t.Errorf("info: %q", msg)
	}
	// the connection ends without a close frame
	wantLines(t, "drop", runWSC(t, "dial\t"+u+"/drop", "send\ttext\t6869", "recv"), "dial ok", "sent", "err EUNEXPECTED_EOF code=1006")
	// a refused handshake
	wantLines(t, "404", runWSC(t, "dial\t"+u+"/nothing"), "dial err EBADRESPONSE status=404 body=404 page not found|")
	p := makePKI(t)
	st := startGoWSServer(t, "gorilla", "-cert", p.leaves["ed25519"][0], "-key", p.leaves["ed25519"][1])
	wantLines(t, "wss", runWSC(t, "dial\twss://localhost:"+strings.Split(st.addr, ":")[1]+"/echo\troots="+p.caPEM+"\tcompress", "echos",
		"close\t1000\t"), "dial ok status=101 proto= ext="+nct, "echos ok 40", "close ok peer=1000")
	wantLines(t, "wss untrusted", runWSC(t, "dial\twss://localhost:"+strings.Split(st.addr, ":")[1]+"/echo"), "dial err ETLSCERT")
}

// TestWebSocketClientXNet: the library's client against x/net's server
// (which takes one frame as a message: no streamed writes here)
func TestWebSocketClientXNet(t *testing.T) {
	s := startGoWSServer(t, "xnet")
	u := "ws://" + s.addr
	var script []string
	var want []string
	script = append(script, "dial\t"+u+"/echo\tcompress\thdr=Origin:http://localhost")
	want = append(want, "dial ok status=101 proto= ext=")
	for i, n := range []int{0, 1, 125, 126, 127, 65535, 65536, 65537, 1 << 20, 6 << 20} {
		for _, typ := range []string{"text", "binary"} {
			script = append(script, fmt.Sprintf("echo\t%s\t%d\t%d", typ, n, i))
			want = append(want, "echo ok")
		}
	}
	script = append(script, "ping\txping", "close\t1000\t")
	want = append(want, "pong xping", "close ok peer=1000")
	wantLines(t, "echo", runWSC(t, script...), want...)
	wantLines(t, "info", runWSC(t, "dial\t"+u+"/info\tproto=chat\thdr=Origin:http://o.example", "recv"),
		"dial ok status=101 proto=chat", "msg text")
	p := makePKI(t)
	st := startGoWSServer(t, "xnet", "-cert", p.leaves["p384"][0], "-key", p.leaves["p384"][1])
	wantLines(t, "wss", runWSC(t, "dial\twss://localhost:"+strings.Split(st.addr, ":")[1]+"/echo\troots="+p.caPEM,
		"echo\tbinary\t3000000\t1", "echo\ttext\t70000\t2"), "dial ok", "echo ok", "echo ok")
}

// TestWebSocketSelf: the library's client and server, every compression
// setting, streamed messages, TLS
func TestWebSocketSelf(t *testing.T) {
	p := makePKI(t)
	for _, sa := range [][]string{{}, {"-compress"}, {"-compress", "-snct"}, {"-compress", "-cnct"}, {"-compress", "-snct", "-cnct", "-level", "9"}} {
		s := startWSD(t, append([]string{"-frame", "1000"}, sa...)...)
		for _, ca := range []string{"", "\tcompress", "\tcompress\tnct", "\tcompress\tframe=77"} {
			ext := ""
			if len(sa) > 0 && ca != "" {
				ext = "permessage-deflate"
				if strings.Contains(ca, "nct") || strings.Contains(strings.Join(sa, " "), "-snct") {
					ext += "; server_no_context_takeover"
				}
				if strings.Contains(ca, "nct") || strings.Contains(strings.Join(sa, " "), "-cnct") {
					ext += "; client_no_context_takeover"
				}
			}
			for _, path := range []string{"/echo", "/stream"} {
				what := fmt.Sprintf("%v%s %s", sa, ca, path)
				out := runWSC(t, "dial\tws://"+s.addr+path+ca, "echos\t65537", "close\t1000\t")
				wantLines(t, what, out, "dial ok status=101 proto= ext="+ext, "echos ok 32", "close ok peer=1000")
				if len(out) > 0 && out[0] != "dial ok status=101 proto= ext="+ext+" x-ws=" {
					t.Errorf("%s: %q", what, out[0])
				}
			}
		}
	}
	s := startWSD(t, "-tls", p.leaves["p521"][0], p.leaves["p521"][1], "-compress")
	wantLines(t, "wss", runWSC(t, "dial\twss://localhost:"+strings.Split(s.addr, ":")[1]+"/stream\troots="+p.caPEM+"\tcompress", "echos",
		"close\t1000\t"), "dial ok", "echos ok 40", "close ok peer=1000")
	// every size up to 5 MiB without compression
	s = startWSD(t)
	wantLines(t, "plain", runWSC(t, "dial\tws://"+s.addr+"/echo", "echos", "close\t1000\t"), "dial ok", "echos ok 40", "close ok peer=1000")
	// response headers, a server-initiated close
	wantLines(t, "hdr", runWSC(t, "dial\tws://"+s.addr+"/hdr", "close\t1000\t"), "dial ok status=101 proto= ext= x-ws=yes", "close ok")
	wantLines(t, "server close", runWSC(t, "dial\tws://"+s.addr+"/close?code=3999&reason=bye", "recv"), "dial ok",
		"err ECLOSED code=3999 reason=bye sent=3999")
	wantLines(t, "server ping", runWSC(t, "dial\tws://"+s.addr+"/ping", "recv", "pongs"), "dial ok", "msg text 13 706f6e673a7372762d70696e67 ", "pongs ")
}

// ---- raw frames ----

var wsKey = [4]byte{0x37, 0xfa, 0x21, 0x3d}

// wsFrameLen builds a frame; lenForm 0 is the shortest length encoding,
// 2 and 8 force the 16- and 64-bit forms
func wsFrameLen(b0 byte, payload []byte, masked bool, lenForm int) []byte {
	var b []byte
	b = append(b, b0)
	m := byte(0)
	if masked {
		m = 0x80
	}
	n := len(payload)
	switch {
	case lenForm == 8 || (lenForm == 0 && n > 65535):
		b = append(b, m|127)
		b = binary.BigEndian.AppendUint64(b, uint64(n))
	case lenForm == 2 || (lenForm == 0 && n > 125):
		b = append(b, m|126)
		b = binary.BigEndian.AppendUint16(b, uint16(n))
	default:
		b = append(b, m|byte(n))
	}
	if masked {
		b = append(b, wsKey[:]...)
		for i, c := range payload {
			b = append(b, c^wsKey[i&3])
		}
	} else {
		b = append(b, payload...)
	}
	return b
}

func wsFrame(b0 byte, payload []byte) []byte { return wsFrameLen(b0, payload, true, 0) }

func closePayload(code int, reason string) []byte {
	return append([]byte{byte(code >> 8), byte(code)}, reason...)
}

type wsGot struct {
	b0      byte
	masked  bool
	payload []byte
}

// readWSFrame reads a frame (unmasking it)
func readWSFrame(br *bufio.Reader) (wsGot, error) {
	var g wsGot
	h := make([]byte, 2)
	if _, err := io.ReadFull(br, h); err != nil {
		return g, err
	}
	g.b0 = h[0]
	g.masked = h[1]&0x80 != 0
	n := uint64(h[1] & 0x7f)
	if n == 126 {
		b := make([]byte, 2)
		if _, err := io.ReadFull(br, b); err != nil {
			return g, err
		}
		n = uint64(binary.BigEndian.Uint16(b))
	} else if n == 127 {
		b := make([]byte, 8)
		if _, err := io.ReadFull(br, b); err != nil {
			return g, err
		}
		n = binary.BigEndian.Uint64(b)
	}
	var key [4]byte
	if g.masked {
		if _, err := io.ReadFull(br, key[:]); err != nil {
			return g, err
		}
	}
	if n > 64<<20 {
		return g, fmt.Errorf("frame of %d bytes", n)
	}
	g.payload = make([]byte, n)
	if _, err := io.ReadFull(br, g.payload); err != nil {
		return g, err
	}
	if g.masked {
		for i := range g.payload {
			g.payload[i] ^= key[i&3]
		}
	}
	return g, nil
}

const wsTestKey = "dGhlIHNhbXBsZSBub25jZQ=="

func wsAccept(key string) string {
	h := sha1.Sum([]byte(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
	return base64.StdEncoding.EncodeToString(h[:])
}

// wsRawDial makes the opening handshake by hand
func wsRawDial(t *testing.T, addr, path, extra string) (net.Conn, *bufio.Reader, *http.Response) {
	t.Helper()
	c, err := net.Dial("tcp", addr)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Fprintf(c, "GET %s HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\n"+
		"Sec-WebSocket-Version: 13\r\n%s\r\n", path, addr, wsTestKey, extra)
	br := bufio.NewReader(c)
	resp, err := http.ReadResponse(br, nil)
	if err != nil {
		c.Close()
		t.Fatal(err)
	}
	if resp.StatusCode != 101 || resp.Header.Get("Sec-Websocket-Accept") != wsAccept(wsTestKey) {
		c.Close()
		t.Fatalf("handshake: %d %v", resp.StatusCode, resp.Header)
	}
	return c, br, resp
}

// an expected frame from the server: an opcode with a payload, or a
// close frame with a code (-1: without a code)
type wsWant struct {
	op      byte
	payload []byte
	code    int
}

type wsCase struct {
	name  string
	send  []byte
	split int // 0: one write; n: writes of n bytes
	want  []wsWant
	path  string
}

func echoOf(op byte, p []byte) wsWant { return wsWant{op: op, payload: p} }
func closeWant(code int) wsWant       { return wsWant{op: 8, code: code} }
func cat(frames ...[]byte) []byte     { return bytes.Join(frames, nil) }

// runWSCase sends the frames to a server and checks its answer: the
// frames wanted, then (after a close frame) the end of the connection.
// A case that does not end with a close frame gets a normal close.
func runWSCase(t *testing.T, addr string, c wsCase, extra string) {
	t.Helper()
	path := c.path
	if path == "" {
		path = "/echo"
	}
	conn, br, _ := wsRawDial(t, addr, path, extra)
	defer conn.Close()
	want := c.want
	send := c.send
	if len(want) == 0 || want[len(want)-1].op != 8 {
		send = cat(send, wsFrame(0x88, closePayload(1000, "")))
		want = append(append([]wsWant{}, want...), closeWant(1000))
	}
	go func() {
		if c.split == 0 {
			conn.Write(send)
			return
		}
		for i := 0; i < len(send); i += c.split {
			j := i + c.split
			if j > len(send) {
				j = len(send)
			}
			if _, err := conn.Write(send[i:j]); err != nil {
				return
			}
		}
	}()
	conn.SetReadDeadline(time.Now().Add(10 * time.Second))
	for i, w := range want {
		g, err := readWSFrame(br)
		if err != nil {
			t.Errorf("%s: frame %d: %v", c.name, i, err)
			return
		}
		if g.masked {
			t.Errorf("%s: masked frame from the server", c.name)
		}
		if g.b0 != 0x80|w.op {
			t.Errorf("%s: frame %d: %#x, want %#x (payload %.40q)", c.name, i, g.b0, 0x80|w.op, g.payload)
			return
		}
		if w.op == 8 {
			code := -1
			if len(g.payload) >= 2 {
				code = int(binary.BigEndian.Uint16(g.payload))
			}
			if code != w.code {
				t.Errorf("%s: close code %d (%q), want %d", c.name, code, g.payload, w.code)
			}
			continue
		}
		if !bytes.Equal(g.payload, w.payload) {
			t.Errorf("%s: frame %d: payload %.60q, want %.60q", c.name, i, g.payload, w.payload)
		}
	}
	// the server closes the connection after the close frames
	if _, err := br.ReadByte(); err == nil {
		t.Errorf("%s: more data after the close frame", c.name)
	}
}

var wsValidUTF8 = []string{"", "Hello-µ@ßöäüàá-UTF-8!!", "κόσμε", "\xf0\x90\x80\x80", "\xf4\x8f\xbf\xbf", "\xef\xbf\xbf", "\xed\x9f\xbf",
	"\xee\x80\x80", "\xef\xbf\xbd", "\x00", "\x7f", "\xc2\x80", "\xdf\xbf", "\xe0\xa0\x80", "\xf0\x9f\x98\x80 emoji"}

var wsInvalidUTF8 = []string{"\x80", "\xbf", "\xc0\xaf", "\xc1\xbf", "\xe0\x80\xaf", "\xe0\x9f\xbf", "\xf0\x80\x80\xaf", "\xf0\x8f\xbf\xbf",
	"\xed\xa0\x80", "\xed\xad\xbf", "\xed\xbf\xbf", "\xf4\x90\x80\x80", "\xf5\x80\x80\x80", "\xf8\x88\x80\x80\x80", "\xfe", "\xff",
	"\xce", "\xe2\x82", "\xf0\x9f\x98", "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce\xb5\xed\xa0\x80edited", "a\xc3(b"}

// wsConformanceCases: the rules of RFC 6455 for a server without extensions
func wsConformanceCases() []wsCase {
	var cs []wsCase
	add := func(c wsCase) { cs = append(cs, c) }
	r := rand.New(rand.NewSource(6455))
	rb := func(n int) []byte { b := make([]byte, n); r.Read(b); return b }
	// echo of every length encoding
	for _, n := range []int{0, 1, 125, 126, 127, 128, 1000, 65535, 65536, 65537, 300000} {
		txt := []byte(strings.Repeat("*", n))
		add(wsCase{name: fmt.Sprintf("text %d", n), send: wsFrame(0x81, txt), want: []wsWant{echoOf(1, txt)}})
		bin := rb(n)
		add(wsCase{name: fmt.Sprintf("binary %d", n), send: wsFrame(0x82, bin), want: []wsWant{echoOf(2, bin)}})
	}
	// fragmentation
	add(wsCase{name: "two fragments", send: cat(wsFrame(0x01, []byte("frag")), wsFrame(0x80, []byte("ment"))),
		want: []wsWant{echoOf(1, []byte("fragment"))}})
	add(wsCase{name: "many fragments", send: cat(wsFrame(0x02, []byte("a")), wsFrame(0x00, []byte("b")), wsFrame(0x00, []byte("c")),
		wsFrame(0x00, nil), wsFrame(0x80, []byte("d"))), want: []wsWant{echoOf(2, []byte("abcd"))}})
	add(wsCase{name: "empty fragments", send: cat(wsFrame(0x01, nil), wsFrame(0x00, nil), wsFrame(0x80, nil)), want: []wsWant{echoOf(1, []byte{})}})
	var many []byte
	for i := 0; i < 1000; i++ {
		many = append(many, wsFrame(0x00, []byte{byte('a' + i%26)})...)
	}
	manyWant := []byte(strings.Repeat("abcdefghijklmnopqrstuvwxyz", 40)[:1000])
	add(wsCase{name: "1000 fragments", send: cat(wsFrame(0x01, nil), many, wsFrame(0x80, nil)), want: []wsWant{echoOf(1, manyWant)}})
	add(wsCase{name: "continuation without start", send: wsFrame(0x80, []byte("x")), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "continuation non-final without start", send: wsFrame(0x00, []byte("x")), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "text within a fragmented text", send: cat(wsFrame(0x01, []byte("a")), wsFrame(0x81, []byte("b"))),
		want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "binary within a fragmented text", send: cat(wsFrame(0x01, []byte("a")), wsFrame(0x02, []byte("b"))),
		want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "continuation after a whole message", send: cat(wsFrame(0x81, []byte("a")), wsFrame(0x80, []byte("b"))),
		want: []wsWant{echoOf(1, []byte("a")), closeWant(1002)}})
	// control frames
	for _, n := range []int{0, 1, 2, 64, 125} {
		pl := rb(n)
		add(wsCase{name: fmt.Sprintf("ping %d", n), send: wsFrame(0x89, pl), want: []wsWant{echoOf(10, pl)}})
	}
	add(wsCase{name: "ping 126", send: wsFrame(0x89, rb(126)), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "ping 125 in a 16-bit length", send: wsFrameLen(0x89, rb(125), true, 2), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "pong 126", send: wsFrame(0x8a, rb(126)), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "unsolicited pong", send: cat(wsFrame(0x8a, []byte("u")), wsFrame(0x81, []byte("t"))), want: []wsWant{echoOf(1, []byte("t"))}})
	var pings []byte
	var pongs []wsWant
	for i := 0; i < 10; i++ {
		pings = append(pings, wsFrame(0x89, []byte{byte(i)})...)
		pongs = append(pongs, echoOf(10, []byte{byte(i)}))
	}
	add(wsCase{name: "10 pings", send: pings, want: pongs})
	add(wsCase{name: "pings between fragments", send: cat(wsFrame(0x01, []byte("Hel")), wsFrame(0x89, []byte("p1")), wsFrame(0x00, []byte("lo ")),
		wsFrame(0x8a, []byte("x")), wsFrame(0x89, []byte("p2")), wsFrame(0x80, []byte("world"))),
		want: []wsWant{echoOf(10, []byte("p1")), echoOf(10, []byte("p2")), echoOf(1, []byte("Hello world"))}})
	add(wsCase{name: "fragmented ping", send: cat(wsFrame(0x09, []byte("a")), wsFrame(0x80, []byte("b"))), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "fragmented pong", send: wsFrame(0x0a, []byte("a")), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "fragmented close", send: wsFrame(0x08, closePayload(1000, "")), want: []wsWant{closeWant(1002)}})
	// reserved bits and opcodes
	for _, rsv := range []byte{0x40, 0x20, 0x10, 0x70} {
		add(wsCase{name: fmt.Sprintf("rsv %#x on text", rsv), send: wsFrame(0x81|rsv, []byte("x")), want: []wsWant{closeWant(1002)}})
		add(wsCase{name: fmt.Sprintf("rsv %#x on ping", rsv), send: wsFrame(0x89|rsv, []byte("x")), want: []wsWant{closeWant(1002)}})
	}
	add(wsCase{name: "rsv on binary after a good text", send: cat(wsFrame(0x81, []byte("ok")), wsFrame(0xa2, []byte("x"))),
		want: []wsWant{echoOf(1, []byte("ok")), closeWant(1002)}})
	add(wsCase{name: "rsv on a continuation", send: cat(wsFrame(0x01, []byte("a")), wsFrame(0xc0, []byte("b"))), want: []wsWant{closeWant(1002)}})
	for _, op := range []byte{3, 4, 5, 6, 7, 11, 12, 13, 14, 15} {
		add(wsCase{name: fmt.Sprintf("opcode %d", op), send: wsFrame(0x80|op, []byte("x")), want: []wsWant{closeWant(1002)}})
	}
	add(wsCase{name: "opcode 5 after a good text", send: cat(wsFrame(0x81, []byte("ok")), wsFrame(0x85, nil)),
		want: []wsWant{echoOf(1, []byte("ok")), closeWant(1002)}})
	// masking and lengths
	add(wsCase{name: "unmasked text", send: wsFrameLen(0x81, []byte("x"), false, 0), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "unmasked ping", send: wsFrameLen(0x89, []byte("x"), false, 0), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "16-bit length of 5", send: wsFrameLen(0x81, []byte("hello"), true, 2), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "16-bit length of 125", send: wsFrameLen(0x82, rb(125), true, 2), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "64-bit length of 200", send: wsFrameLen(0x82, rb(200), true, 8), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "64-bit length of 65535", send: wsFrameLen(0x82, rb(65535), true, 8), want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "64-bit length of 65536", send: wsFrameLen(0x82, bytes.Repeat([]byte{7}, 65536), true, 8),
		want: []wsWant{echoOf(2, bytes.Repeat([]byte{7}, 65536))}})
	add(wsCase{name: "64-bit length with the msb", send: []byte{0x82, 0xff, 0x80, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4}, want: []wsWant{closeWant(1002)}})
	add(wsCase{name: "64-bit length of 2^62 (beyond the limit)", send: []byte{0x82, 0xff, 0x40, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4},
		want: []wsWant{closeWant(1009)}})
	add(wsCase{name: "64-bit length of 2^40 in a fragment", send: cat(wsFrame(0x02, []byte("x")), []byte{0x80, 0xff, 0, 0, 1, 0, 0, 0, 0, 0, 1, 2, 3, 4}),
		want: []wsWant{closeWant(1009)}})
	// UTF-8
	for i, s := range wsValidUTF8 {
		add(wsCase{name: fmt.Sprintf("valid utf-8 %d", i), send: wsFrame(0x81, []byte(s)), want: []wsWant{echoOf(1, []byte(s))}})
	}
	for i, s := range wsInvalidUTF8 {
		add(wsCase{name: fmt.Sprintf("invalid utf-8 %d %q", i, s), send: wsFrame(0x81, []byte(s)), want: []wsWant{closeWant(1007)}})
	}
	// an invalid byte at every position of a text
	base := []byte("Hello-µ@ßöäüàá-UTF-8!!")
	for i := 0; i <= len(base); i++ {
		bad := append(append(append([]byte{}, base[:i]...), 0xff), base[i:]...)
		add(wsCase{name: fmt.Sprintf("0xff at %d", i), send: wsFrame(0x81, bad), want: []wsWant{closeWant(1007)}})
	}
	// a valid text cut into two fragments at every position (inside runes too)
	kosme := []byte("κόσμε-€𝄞")
	for i := 0; i <= len(kosme); i++ {
		add(wsCase{name: fmt.Sprintf("utf-8 fragmented at %d", i), send: cat(wsFrame(0x01, kosme[:i]), wsFrame(0x80, kosme[i:])),
			want: []wsWant{echoOf(1, kosme)}})
	}
	// an invalid sequence across fragments; the failure comes before the message ends
	add(wsCase{name: "invalid utf-8 across fragments", send: cat(wsFrame(0x01, []byte("\xce")), wsFrame(0x80, []byte("("))),
		want: []wsWant{closeWant(1007)}})
	add(wsCase{name: "invalid utf-8 in the first fragment (fail fast)", send: wsFrame(0x01, []byte("ok\xed\xa0\x80")), want: []wsWant{closeWant(1007)}})
	add(wsCase{name: "truncated rune at the end of a fragmented text", send: cat(wsFrame(0x01, []byte("a")), wsFrame(0x80, []byte("\xe2\x82"))),
		want: []wsWant{closeWant(1007)}})
	add(wsCase{name: "invalid utf-8 in binary is fine", send: wsFrame(0x82, []byte("\xff\xfe")), want: []wsWant{echoOf(2, []byte("\xff\xfe"))}})
	// close frames
	add(wsCase{name: "close without payload", send: wsFrame(0x88, nil), want: []wsWant{closeWant(-1)}})
	add(wsCase{name: "close of 1 byte", send: wsFrame(0x88, []byte{3}), want: []wsWant{closeWant(1002)}})
	for _, code := range []int{1000, 1001, 1002, 1003, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 3000, 3999, 4000, 4999} {
		add(wsCase{name: fmt.Sprintf("close %d", code), send: wsFrame(0x88, closePayload(code, "")), want: []wsWant{closeWant(code)}})
	}
	for _, code := range []int{0, 999, 1004, 1005, 1006, 1015, 1016, 1100, 2000, 2999, 5000, 65535} {
		add(wsCase{name: fmt.Sprintf("close %d (invalid)", code), send: wsFrame(0x88, closePayload(code, "")), want: []wsWant{closeWant(1002)}})
	}
	add(wsCase{name: "close with a reason", send: wsFrame(0x88, closePayload(1000, "κόσμε")), want: []wsWant{closeWant(1000)}})
	add(wsCase{name: "close with the longest reason", send: wsFrame(0x88, closePayload(1000, strings.Repeat("r", 123))), want: []wsWant{closeWant(1000)}})
	add(wsCase{name: "close with an invalid utf-8 reason", send: wsFrame(0x88, closePayload(1000, "a\xffb")), want: []wsWant{closeWant(1007)}})
	add(wsCase{name: "frames after close are ignored", send: cat(wsFrame(0x88, closePayload(1000, "")), wsFrame(0x81, []byte("late")),
		wsFrame(0x89, nil)), want: []wsWant{closeWant(1000)}})
	add(wsCase{name: "close within a fragmented message", send: cat(wsFrame(0x01, []byte("a")), wsFrame(0x88, closePayload(1001, ""))),
		want: []wsWant{closeWant(1001)}})
	// split into single bytes and other pieces
	var cs2 []wsCase
	for _, c := range cs {
		if len(c.send) < 2000 {
			c1 := c
			c1.name += " (bytewise)"
			c1.split = 1
			cs2 = append(cs2, c1)
		}
	}
	cs = append(cs, cs2...)
	big := rb(200000)
	cs = append(cs, wsCase{name: "200000 bytes in writes of 1001", send: wsFrame(0x82, big), split: 1001, want: []wsWant{echoOf(2, big)}})
	return cs
}

// TestWebSocketConformance: hand-crafted frames against the library's server
func TestWebSocketConformance(t *testing.T) {
	s := startWSD(t)
	cases := wsConformanceCases()
	t.Logf("%d cases", len(cases))
	sem := make(chan struct{}, 8)
	var wg sync.WaitGroup
	for _, c := range cases {
		c := c
		wg.Add(1)
		sem <- struct{}{}
		go func() {
			defer wg.Done()
			defer func() { <-sem }()
			runWSCase(t, s.addr, c, "")
		}()
	}
	wg.Wait()
	// the read limit
	sl := startWSD(t, "-limit", "1000")
	for _, c := range []wsCase{
		{name: "1000 bytes", send: wsFrame(0x82, make([]byte, 1000)), want: []wsWant{echoOf(2, make([]byte, 1000))}},
		{name: "1001 bytes", send: wsFrame(0x82, make([]byte, 1001)), want: []wsWant{closeWant(1009)}},
		{name: "1001 bytes in fragments", send: cat(wsFrame(0x02, make([]byte, 600)), wsFrame(0x80, make([]byte, 401))), want: []wsWant{closeWant(1009)}},
		{name: "1000 bytes in fragments", send: cat(wsFrame(0x01, []byte(strings.Repeat("a", 600))), wsFrame(0x80, []byte(strings.Repeat("a", 400)))),
			want: []wsWant{echoOf(1, []byte(strings.Repeat("a", 1000)))}},
	} {
		runWSCase(t, sl.addr, c, "")
	}
}

// ---- opening handshakes ----

type wsHdr [][2]string

// wsRawRequest sends a handshake request and returns the response and its body
func wsRawRequest(t *testing.T, addr, method, path, proto string, h wsHdr) (*http.Response, string) {
	t.Helper()
	c, err := net.Dial("tcp", addr)
	if err != nil {
		t.Fatal(err)
	}
	defer c.Close()
	var b strings.Builder
	fmt.Fprintf(&b, "%s %s %s\r\nHost: %s\r\n", method, path, proto, addr)
	for _, kv := range h {
		fmt.Fprintf(&b, "%s: %s\r\n", kv[0], kv[1])
	}
	b.WriteString("\r\n")
	c.Write([]byte(b.String()))
	c.SetReadDeadline(time.Now().Add(10 * time.Second))
	br := bufio.NewReader(c)
	resp, err := http.ReadResponse(br, nil)
	if err != nil {
		t.Fatalf("%s %s: %v", method, path, err)
	}
	body := ""
	if resp.StatusCode != 101 {
		bb, _ := io.ReadAll(resp.Body)
		body = string(bb)
	}
	return resp, body
}

func wsStdHdr(extra ...[2]string) wsHdr {
	h := wsHdr{{"Upgrade", "websocket"}, {"Connection", "Upgrade"}, {"Sec-WebSocket-Key", wsTestKey}, {"Sec-WebSocket-Version", "13"}}
	return append(h, extra...)
}

// without replaces or drops (value "") a header of the standard ones
func wsHdrWith(key, value string, extra ...[2]string) wsHdr {
	var h wsHdr
	for _, kv := range wsStdHdr() {
		if strings.EqualFold(kv[0], key) {
			if value != "" {
				h = append(h, [2]string{key, value})
			}
			continue
		}
		h = append(h, kv)
	}
	return append(h, extra...)
}

// TestWebSocketHandshakeServer: the server's checks of the request and its answers
func TestWebSocketHandshakeServer(t *testing.T) {
	s := startWSD(t, "-protos", "b,a")
	type hc struct {
		name           string
		method, proto  string
		h              wsHdr
		status         int
		protocol, path string
	}
	cases := []hc{
		{name: "valid", h: wsStdHdr(), status: 101},
		{name: "connection list", h: wsHdrWith("Connection", "keep-alive, Upgrade"), status: 101},
		{name: "connection case", h: wsHdrWith("Connection", "UPGRADE"), status: 101},
		{name: "connection in two lines", h: wsHdrWith("Connection", "keep-alive", [2]string{"Connection", "upgrade"}), status: 101},
		{name: "upgrade case", h: wsHdrWith("Upgrade", "WebSocket"), status: 101},
		{name: "upgrade list", h: wsHdrWith("Upgrade", "foo/2, websocket"), status: 101},
		{name: "no connection", h: wsHdrWith("Connection", ""), status: 400},
		{name: "connection keep-alive", h: wsHdrWith("Connection", "keep-alive"), status: 400},
		{name: "connection upgraded", h: wsHdrWith("Connection", "upgraded"), status: 400},
		{name: "no upgrade", h: wsHdrWith("Upgrade", ""), status: 400},
		{name: "upgrade h2c", h: wsHdrWith("Upgrade", "h2c"), status: 400},
		{name: "upgrade websocket2", h: wsHdrWith("Upgrade", "websocket2"), status: 400},
		{name: "POST", method: "POST", h: wsStdHdr(), status: 405},
		{name: "PUT", method: "PUT", h: wsStdHdr(), status: 405},
		{name: "HTTP/1.0", proto: "HTTP/1.0", h: wsStdHdr(), status: 400},
		{name: "version 8", h: wsHdrWith("Sec-WebSocket-Version", "8"), status: 426},
		{name: "version 14", h: wsHdrWith("Sec-WebSocket-Version", "14"), status: 426},
		{name: "no version", h: wsHdrWith("Sec-WebSocket-Version", ""), status: 426},
		{name: "version list", h: wsHdrWith("Sec-WebSocket-Version", "13, 8"), status: 101},
		{name: "no key", h: wsHdrWith("Sec-WebSocket-Key", ""), status: 400},
		{name: "short key", h: wsHdrWith("Sec-WebSocket-Key", "abc"), status: 400},
		{name: "15-byte key", h: wsHdrWith("Sec-WebSocket-Key", base64.StdEncoding.EncodeToString(make([]byte, 15))), status: 400},
		{name: "17-byte key", h: wsHdrWith("Sec-WebSocket-Key", base64.StdEncoding.EncodeToString(make([]byte, 17))), status: 400},
		{name: "unpadded key", h: wsHdrWith("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ"), status: 400},
		{name: "key not base64", h: wsHdrWith("Sec-WebSocket-Key", "!!!!!!!!!!!!!!!!!!!!!!=="), status: 400},
		{name: "foreign origin", h: wsStdHdr([2]string{"Origin", "http://evil.example"}), status: 403},
		{name: "null origin", h: wsStdHdr([2]string{"Origin", "null"}), status: 403},
		{name: "same origin", h: wsStdHdr([2]string{"Origin", "http://" + s.addr}), status: 101},
		{name: "same origin https", h: wsStdHdr([2]string{"Origin", "https://" + s.addr}), status: 101},
		{name: "subprotocol by server preference", h: wsStdHdr([2]string{"Sec-WebSocket-Protocol", "a, b"}), status: 101, protocol: "b"},
		{name: "one subprotocol", h: wsStdHdr([2]string{"Sec-WebSocket-Protocol", "a"}), status: 101, protocol: "a"},
		{name: "unknown subprotocol", h: wsStdHdr([2]string{"Sec-WebSocket-Protocol", "c"}), status: 101},
		{name: "subprotocols in two lines", h: wsStdHdr([2]string{"Sec-WebSocket-Protocol", "c"}, [2]string{"Sec-WebSocket-Protocol", "a"}),
			status: 101, protocol: "a"},
		{name: "compression not enabled", h: wsStdHdr([2]string{"Sec-WebSocket-Extensions", "permessage-deflate"}), status: 101},
	}
	for _, c := range cases {
		method, proto := c.method, c.proto
		if method == "" {
			method = "GET"
		}
		if proto == "" {
			proto = "HTTP/1.1"
		}
		resp, body := wsRawRequest(t, s.addr, method, "/echo", proto, c.h)
		if resp.StatusCode != c.status {
			t.Errorf("%s: status %d, want %d (%q)", c.name, resp.StatusCode, c.status, body)
			continue
		}
		if c.status == 101 {
			if resp.Header.Get("Upgrade") != "websocket" || resp.Header.Get("Connection") != "Upgrade" ||
				resp.Header.Get("Sec-WebSocket-Accept") != "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=" {
				t.Errorf("%s: %v", c.name, resp.Header)
			}
			if got := resp.Header.Get("Sec-WebSocket-Protocol"); got != c.protocol {
				t.Errorf("%s: protocol %q, want %q", c.name, got, c.protocol)
			}
			if _, ok := resp.Header["Sec-Websocket-Extensions"]; ok {
				t.Errorf("%s: extensions %v", c.name, resp.Header)
			}
			continue
		}
		// as gorilla: the status text and the version supported
		if want := http.StatusText(c.status) + "\n"; body != want {
			t.Errorf("%s: body %q, want %q", c.name, body, want)
		}
		if resp.Header.Get("Sec-WebSocket-Version") != "13" {
			t.Errorf("%s: no Sec-WebSocket-Version: %v", c.name, resp.Header)
		}
	}
	// the origin check can be replaced
	sa := startWSD(t, "-anyorigin")
	if resp, _ := wsRawRequest(t, sa.addr, "GET", "/echo", "HTTP/1.1", wsStdHdr([2]string{"Origin", "http://evil.example"})); resp.StatusCode != 101 {
		t.Errorf("any origin: %d", resp.StatusCode)
	}
	// alt_ws_is_upgrade
	if _, body := wsRawRequest(t, s.addr, "GET", "/upgrade", "HTTP/1.1", nil); body != "upgrade=0\n" {
		t.Errorf("is_upgrade: %q", body)
	}
	if _, body := wsRawRequest(t, s.addr, "GET", "/upgrade", "HTTP/1.1", wsHdrWith("Connection", "keep-alive, upgrade")); body != "upgrade=1\n" {
		t.Errorf("is_upgrade: %q", body)
	}
}

// TestWebSocketCompressionNegotiation: the server's answers to offers of permessage-deflate
func TestWebSocketCompressionNegotiation(t *testing.T) {
	const pmd = "permessage-deflate"
	cases := []struct{ args, offer, want string }{
		{"", pmd, pmd},
		{"", "PerMessage-Deflate", pmd},
		{"", pmd + "; client_max_window_bits", pmd},
		{"", pmd + "; client_max_window_bits=10", pmd},
		{"", pmd + `; client_max_window_bits="10"`, pmd},
		{"", pmd + "; client_max_window_bits=7", ""},
		{"", pmd + "; client_max_window_bits=16", ""},
		{"", pmd + "; client_max_window_bits=010", ""},
		{"", pmd + "; server_max_window_bits=15", pmd + "; server_max_window_bits=15"},
		{"", pmd + "; server_max_window_bits=10", ""},
		{"", pmd + "; server_max_window_bits", ""},
		{"", pmd + "; server_max_window_bits=10, " + pmd, pmd},
		{"", pmd + "; server_max_window_bits=8, " + pmd + "; client_no_context_takeover", pmd + "; client_no_context_takeover"},
		{"", pmd + "; server_no_context_takeover", pmd + "; server_no_context_takeover"},
		{"", pmd + "; client_no_context_takeover", pmd + "; client_no_context_takeover"},
		{"", "x-webkit-deflate-frame, " + pmd + "; client_no_context_takeover; server_no_context_takeover",
			pmd + "; server_no_context_takeover; client_no_context_takeover"},
		{"", "x-webkit-deflate-frame", ""},
		{"", pmd + "; unknown", ""},
		{"", pmd + "; server_no_context_takeover; server_no_context_takeover", ""},
		{"", pmd + "; server_no_context_takeover=1", ""},
		{"", pmd + ";", ""},
		{"", pmd + "; a=\"unterminated", ""},
		{"", ", ," + pmd, pmd},
		{"-snct", pmd, pmd + "; server_no_context_takeover"},
		{"-cnct", pmd, pmd + "; client_no_context_takeover"},
		{"-snct -cnct", pmd + "; server_max_window_bits=15", pmd + "; server_no_context_takeover; client_no_context_takeover; server_max_window_bits=15"},
	}
	servers := map[string]*wsServerProc{}
	for _, c := range cases {
		s := servers[c.args]
		if s == nil {
			s = startWSD(t, append([]string{"-compress"}, strings.Fields(c.args)...)...)
			servers[c.args] = s
		}
		resp, _ := wsRawRequest(t, s.addr, "GET", "/echo", "HTTP/1.1", wsStdHdr([2]string{"Sec-WebSocket-Extensions", c.offer}))
		if resp.StatusCode != 101 {
			t.Errorf("%q: %d", c.offer, resp.StatusCode)
		}
		if got := resp.Header.Get("Sec-WebSocket-Extensions"); got != c.want {
			t.Errorf("%s %q: %q, want %q", c.args, c.offer, got, c.want)
		}
	}
}

// ---- permessage-deflate with Go's compress/flate ----

var wsDeflateTail = []byte{0, 0, 0xff, 0xff}

// Go's side of a compression context
type goDeflater struct {
	takeover bool
	buf      bytes.Buffer
	w        *flate.Writer
}

func (d *goDeflater) compress(p []byte) []byte {
	if d.w == nil || !d.takeover {
		d.w, _ = flate.NewWriter(&d.buf, 6)
	}
	d.buf.Reset()
	d.w.Write(p)
	d.w.Flush()
	out := d.buf.Bytes()
	if !bytes.HasSuffix(out, wsDeflateTail) {
		panic("no sync flush tail")
	}
	return append([]byte{}, out[:len(out)-4]...)
}

type goInflater struct {
	takeover bool
	hist     []byte
}

func (f *goInflater) decompress(p []byte) ([]byte, error) {
	src := io.MultiReader(bytes.NewReader(p), bytes.NewReader([]byte{0, 0, 0xff, 0xff, 1, 0, 0, 0xff, 0xff}))
	var r io.ReadCloser
	if f.takeover && len(f.hist) > 0 {
		r = flate.NewReaderDict(src, f.hist)
	} else {
		r = flate.NewReader(src)
	}
	out, err := io.ReadAll(r)
	if f.takeover {
		f.hist = append(f.hist, out...)
		if len(f.hist) > 32768 {
			f.hist = f.hist[len(f.hist)-32768:]
		}
	}
	return out, err
}

// readMessage reads a message (its frames) from the peer
func readWSMessage(br *bufio.Reader) (op byte, rsv1 bool, payload []byte, err error) {
	for {
		g, err := readWSFrame(br)
		if err != nil {
			return 0, false, nil, err
		}
		if g.b0&0x0f >= 8 {
			if g.b0&0x0f == 8 {
				return 8, false, g.payload, nil
			}
			continue // pings, pongs
		}
		if g.b0&0x0f != 0 {
			op, rsv1 = g.b0&0x0f, g.b0&0x40 != 0
		} else if g.b0&0x40 != 0 {
			return 0, false, nil, fmt.Errorf("RSV1 on a continuation")
		}
		payload = append(payload, g.payload...)
		if g.b0&0x80 != 0 {
			return op, rsv1, payload, nil
		}
	}
}

func wsTestMessages() [][]byte {
	r := rand.New(rand.NewSource(7692))
	var ms [][]byte
	for i, n := range []int{0, 1, 100, 5000, 40000, 100000, 3000, 3000, 70000, 1 << 20} {
		var m []byte
		switch i % 3 {
		case 0:
			m = bytes.Repeat([]byte("the same words again "), n/21+1)[:n]
		case 1:
			m = make([]byte, n)
			r.Read(m)
		default:
			m = []byte(strings.Repeat("κόσμε "+fmt.Sprint(i), n/12+1))
			for !utf8.Valid(m[:n]) {
				n--
			}
			m = m[:n]
		}
		ms = append(ms, m)
	}
	// the same message twice: with context takeover the second is tiny
	rep := make([]byte, 20000)
	r.Read(rep)
	ms = append(ms, rep, rep)
	return ms
}

// TestWebSocketCompressionServer: the server's permessage-deflate against
// Go's compress/flate in every context takeover setting, fragmented
// compressed messages, and the failures
func TestWebSocketCompressionServer(t *testing.T) {
	const pmd = "permessage-deflate"
	for _, mode := range []struct{ args, offer string }{
		{"", pmd},
		{"", pmd + "; server_no_context_takeover"},
		{"", pmd + "; client_no_context_takeover"},
		{"", pmd + "; server_no_context_takeover; client_no_context_takeover"},
		{"-snct", pmd},
		{"-cnct", pmd},
		{"-level 9", pmd},
	} {
		s := startWSD(t, append([]string{"-compress"}, strings.Fields(mode.args)...)...)
		conn, br, resp := wsRawDial(t, s.addr, "/echo", "Sec-WebSocket-Extensions: "+mode.offer+"\r\n")
		ext := resp.Header.Get("Sec-WebSocket-Extensions")
		srvTakeover := !strings.Contains(ext, "server_no_context_takeover")
		cliTakeover := !strings.Contains(ext, "client_no_context_takeover")
		def := &goDeflater{takeover: cliTakeover}
		inf := &goInflater{takeover: srvTakeover}
		conn.SetDeadline(time.Now().Add(30 * time.Second))
		var sizes []int
		for i, m := range wsTestMessages() {
			op := byte(2)
			if utf8.Valid(m) && i%2 == 0 {
				op = 1
			}
			var send []byte
			switch i % 4 {
			case 3: // not compressed: allowed on a compressed connection, outside the context
				send = wsFrame(0x80|op, m)
			case 2: // compressed, in three fragments
				z := def.compress(m)
				a, b := len(z)/3, 2*len(z)/3
				send = cat(wsFrame(0x40|op, z[:a]), wsFrame(0x89, []byte("ping")), wsFrame(0x00, z[a:b]), wsFrame(0x80, z[b:]))
			default:
				send = wsFrame(0xc0|op, def.compress(m))
			}
			go conn.Write(send)
			gop, rsv1, z, err := readWSMessage(br)
			if err != nil {
				t.Fatalf("%v message %d: %v", mode, i, err)
			}
			if gop != op || !rsv1 {
				t.Fatalf("%v message %d: op %d rsv1 %v", mode, i, gop, rsv1)
			}
			got, err := inf.decompress(z)
			if err != nil || !bytes.Equal(got, m) {
				t.Fatalf("%v message %d: %d bytes, want %d: %v", mode, i, len(got), len(m), err)
			}
			sizes = append(sizes, len(z))
		}
		// the repeated message: smaller with the server's context
		n := len(sizes)
		if srvTakeover && sizes[n-1]*4 > sizes[n-2] {
			t.Errorf("%v: context takeover does not show: %d then %d bytes", mode, sizes[n-2], sizes[n-1])
		}
		if !srvTakeover && sizes[n-1] != sizes[n-2] {
			t.Errorf("%v: no context takeover, yet %d then %d bytes", mode, sizes[n-2], sizes[n-1])
		}
		conn.Write(wsFrame(0x88, closePayload(1000, "")))
		if op, _, p, err := readWSMessage(br); err != nil || op != 8 || binary.BigEndian.Uint16(p) != 1000 {
			t.Errorf("%v: close %v %d %q", mode, err, op, p)
		}
		conn.Close()
	}

	// failures on a compressed connection
	s := startWSD(t, "-compress", "-limit", "100000")
	ext := "Sec-WebSocket-Extensions: permessage-deflate\r\n"
	z := func(p []byte) []byte { return (&goDeflater{}).compress(p) }
	var closed bytes.Buffer
	fw, _ := flate.NewWriter(&closed, 5)
	fw.Write([]byte("a stream ended by its writer"))
	fw.Close()
	cases := []wsCase{
		{name: "compressed text", send: wsFrame(0xc1, z([]byte("Hello"))), want: nil},
		{name: "rsv1 on ping", send: wsFrame(0xc9, []byte("x")), want: []wsWant{closeWant(1002)}},
		{name: "rsv1 on close", send: wsFrame(0xc8, closePayload(1000, "")), want: []wsWant{closeWant(1002)}},
		{name: "rsv1 on a continuation", send: cat(wsFrame(0x41, z([]byte("ab"))), wsFrame(0xc0, nil)), want: []wsWant{closeWant(1002)}},
		{name: "rsv2", send: wsFrame(0xa1, []byte("x")), want: []wsWant{closeWant(1002)}},
		{name: "corrupt data", send: wsFrame(0xc2, []byte{0xff, 0xff, 0xff, 0xff}), want: []wsWant{closeWant(1007)}},
		{name: "invalid utf-8 inside", send: wsFrame(0xc1, z([]byte("ok \xed\xa0\x80"))), want: []wsWant{closeWant(1007)}},
		{name: "a bomb beyond the limit", send: wsFrame(0xc2, z(make([]byte, 1<<20))), want: []wsWant{closeWant(1009)}},
		{name: "fragmented bomb", send: cat(wsFrame(0x42, z(make([]byte, 1<<20))[:10]), wsFrame(0x80, z(make([]byte, 1<<20))[10:])),
			want: []wsWant{closeWant(1009)}},
	}
	for _, c := range cases {
		if c.want == nil {
			// the echo is compressed: read it by hand
			conn, br, _ := wsRawDial(t, s.addr, "/echo", ext)
			conn.Write(c.send)
			conn.SetDeadline(time.Now().Add(10 * time.Second))
			_, rsv1, p, err := readWSMessage(br)
			got, err2 := (&goInflater{}).decompress(p)
			if err != nil || err2 != nil || !rsv1 || string(got) != "Hello" {
				t.Errorf("%s: %v %v %q", c.name, err, err2, got)
			}
			conn.Close()
			continue
		}
		runWSCase(t, s.addr, c, ext)
		c.split = 1
		c.name += " (bytewise)"
		if len(c.send) < 3000 {
			runWSCase(t, s.addr, c, ext)
		}
	}
	// a message whose data ends the deflate stream itself (BFINAL)
	conn, br, _ := wsRawDial(t, s.addr, "/echo", ext)
	conn.Write(wsFrame(0xc1, closed.Bytes()))
	conn.SetDeadline(time.Now().Add(10 * time.Second))
	_, _, p, err := readWSMessage(br)
	got, _ := (&goInflater{}).decompress(p)
	if err != nil || string(got) != "a stream ended by its writer" {
		t.Errorf("BFINAL: %v %q", err, got)
	}
	conn.Close()
}

// ---- a raw server for the library's client ----

type wsClientCase struct {
	name     string
	opts     string // dial options
	respHdr  string // extra response header lines
	frames   []byte
	split    int
	script   []string // after the dial; "readall" by default
	out      []string // wanted output after "dial ok"
	back     []wsWant // frames the client sends back (masked)
	noAnswer bool     // the server sends nothing back after its frames and closes
}

// runWSClientCase serves one connection by hand and runs the client
func runWSClientCase(t *testing.T, c wsClientCase) {
	t.Helper()
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer ln.Close()
	type result struct {
		back []wsGot
		err  error
	}
	res := make(chan result, 1)
	go func() {
		var r result
		defer func() { res <- r }()
		conn, err := ln.Accept()
		if err != nil {
			r.err = err
			return
		}
		defer conn.Close()
		conn.SetDeadline(time.Now().Add(20 * time.Second))
		br := bufio.NewReader(conn)
		req, err := http.ReadRequest(br)
		if err != nil {
			r.err = err
			return
		}
		fmt.Fprintf(conn, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n%s\r\n",
			wsAccept(req.Header.Get("Sec-WebSocket-Key")), c.respHdr)
		if c.split == 0 {
			conn.Write(c.frames)
		} else {
			for i := 0; i < len(c.frames); i += c.split {
				j := min(i+c.split, len(c.frames))
				conn.Write(c.frames[i:j])
				time.Sleep(time.Millisecond)
			}
		}
		if c.noAnswer {
			return
		}
		for {
			g, err := readWSFrame(br)
			if err != nil {
				if len(c.back) > 0 {
					r.err = err
				}
				return
			}
			r.back = append(r.back, g)
			if g.b0&0x0f == 8 {
				return // the server closes the connection first
			}
		}
	}()
	script := append([]string{"dial\tws://" + ln.Addr().String() + "/" + c.opts}, c.script...)
	if len(c.script) == 0 {
		script = append(script, "readall")
	}
	out := runWSC(t, script...)
	wantLines(t, c.name, out, append([]string{"dial ok"}, c.out...)...)
	r := <-res
	if r.err != nil {
		t.Errorf("%s: server: %v", c.name, r.err)
	}
	if len(r.back) != len(c.back) {
		t.Errorf("%s: %d frames back, want %d: %v", c.name, len(r.back), len(c.back), r.back)
		return
	}
	for i, w := range c.back {
		g := r.back[i]
		if !g.masked {
			t.Errorf("%s: unmasked frame from the client", c.name)
		}
		if g.b0 != 0x80|w.op {
			t.Errorf("%s: frame %d: %#x, want %#x", c.name, i, g.b0, 0x80|w.op)
			continue
		}
		if w.op == 8 {
			code := -1
			if len(g.payload) >= 2 {
				code = int(binary.BigEndian.Uint16(g.payload))
			}
			if code != w.code {
				t.Errorf("%s: close code %d (%q), want %d", c.name, code, g.payload, w.code)
			}
		} else if !bytes.Equal(g.payload, w.payload) {
			t.Errorf("%s: frame %d payload %q, want %q", c.name, i, g.payload, w.payload)
		}
	}
}

func srvFrame(b0 byte, p []byte) []byte { return wsFrameLen(b0, p, false, 0) }

func fnvHex(b []byte) string { return fnv64(b) }

func msgLine(op byte, p []byte) string {
	kind := "binary"
	if op == 1 {
		kind = "text"
	}
	pre := p
	if len(pre) > 32 {
		pre = pre[:32]
	}
	return fmt.Sprintf("msg %s %d %x %s", kind, len(p), pre, fnvHex(p))
}

func errLine(name string, code int, reason string, sent int) string {
	return fmt.Sprintf("err %s code=%d reason=%s sent=%d", name, code, reason, sent)
}

// TestWebSocketClientConformance: hand-crafted frames from a server to
// the library's client
func TestWebSocketClientConformance(t *testing.T) {
	close1000 := srvFrame(0x88, closePayload(1000, ""))
	ok := errLine("ECLOSED", 1000, "", 1000)
	proto := func(name string, frames []byte) wsClientCase {
		return wsClientCase{name: name, frames: frames, out: []string{errLine("EPROTO", 0, "", 1002)}, back: []wsWant{closeWant(1002)}}
	}
	r := rand.New(rand.NewSource(1))
	big := make([]byte, 70000)
	r.Read(big)
	cases := []wsClientCase{
		{name: "text", frames: cat(srvFrame(0x81, []byte("hi")), close1000), out: []string{msgLine(1, []byte("hi")), ok}, back: []wsWant{closeWant(1000)}},
		{name: "empty text", frames: cat(srvFrame(0x81, nil), close1000), out: []string{msgLine(1, nil), ok}, back: []wsWant{closeWant(1000)}},
		{name: "binary 70000", frames: cat(srvFrame(0x82, big), close1000), out: []string{msgLine(2, big), ok}, back: []wsWant{closeWant(1000)}},
		{name: "binary 70000 in a 64-bit length", frames: cat(wsFrameLen(0x82, big, false, 8), close1000), out: []string{msgLine(2, big), ok},
			back: []wsWant{closeWant(1000)}},
		{name: "fragments with a ping", frames: cat(srvFrame(0x01, []byte("frag")), srvFrame(0x89, []byte("abc")), srvFrame(0x80, []byte("ment")),
			close1000), out: []string{msgLine(1, []byte("fragment")), ok}, back: []wsWant{echoOf(10, []byte("abc")), closeWant(1000)}},
		{name: "ping", frames: cat(srvFrame(0x89, []byte("abc")), close1000), out: []string{ok}, back: []wsWant{echoOf(10, []byte("abc")), closeWant(1000)}},
		{name: "empty ping", frames: cat(srvFrame(0x89, nil), close1000), out: []string{ok}, back: []wsWant{echoOf(10, nil), closeWant(1000)}},
		{name: "pong", frames: cat(srvFrame(0x8a, []byte("u")), close1000), out: []string{ok}, back: []wsWant{closeWant(1000)}},
		{name: "bytewise", split: 1, frames: cat(srvFrame(0x01, []byte("by")), srvFrame(0x89, []byte("p")), srvFrame(0x80, []byte("tes")),
			srvFrame(0x88, closePayload(4999, "end"))), out: []string{msgLine(1, []byte("bytes")), errLine("ECLOSED", 4999, "end", 4999)},
			back: []wsWant{echoOf(10, []byte("p")), closeWant(4999)}},
		{name: "close without code", frames: srvFrame(0x88, nil), out: []string{errLine("ECLOSED", 1005, "", 1005)}, back: []wsWant{closeWant(-1)}},
		{name: "close 3000 with reason", frames: srvFrame(0x88, closePayload(3000, "κόσμε")), out: []string{errLine("ECLOSED", 3000, "κόσμε", 3000)},
			back: []wsWant{closeWant(3000)}},
		proto("masked frame", wsFrameLen(0x81, []byte("x"), true, 0)),
		proto("rsv1 without extension", srvFrame(0xc1, []byte("x"))),
		proto("rsv3", srvFrame(0x91, []byte("x"))),
		proto("opcode 3", srvFrame(0x83, nil)),
		proto("opcode 11", srvFrame(0x8b, nil)),
		proto("fragmented ping", srvFrame(0x09, nil)),
		proto("ping 126", srvFrame(0x89, make([]byte, 126))),
		proto("continuation first", srvFrame(0x80, []byte("x"))),
		proto("new message in a fragmented one", cat(srvFrame(0x01, []byte("a")), srvFrame(0x82, []byte("b")))),
		proto("16-bit length of 3", wsFrameLen(0x81, []byte("abc"), false, 2)),
		proto("64-bit length of 300", wsFrameLen(0x82, make([]byte, 300), false, 8)),
		proto("64-bit length with the msb", []byte{0x82, 127, 0x80, 0, 0, 0, 0, 0, 0, 0}),
		proto("close of 1 byte", srvFrame(0x88, []byte{1})),
		proto("close 1005", srvFrame(0x88, closePayload(1005, ""))),
		proto("close 1006", srvFrame(0x88, closePayload(1006, ""))),
		proto("close 2999", srvFrame(0x88, closePayload(2999, ""))),
		proto("close 5000", srvFrame(0x88, closePayload(5000, ""))),
		{name: "invalid utf-8", frames: srvFrame(0x81, []byte("a\xffb")), out: []string{errLine("EILSEQ", 0, "", 1007)}, back: []wsWant{closeWant(1007)}},
		{name: "invalid utf-8 across fragments", frames: cat(srvFrame(0x01, []byte("\xe2\x82")), srvFrame(0x80, []byte("("))),
			out: []string{errLine("EILSEQ", 0, "", 1007)}, back: []wsWant{closeWant(1007)}},
		{name: "invalid utf-8 in the close reason", frames: srvFrame(0x88, closePayload(1000, "\xc0\xaf")), out: []string{errLine("EILSEQ", 0, "", 1007)},
			back: []wsWant{closeWant(1007)}},
		{name: "read limit", opts: "\tlimit=10", frames: srvFrame(0x82, make([]byte, 11)), out: []string{errLine("EMSGSIZE", 0, "", 1009)},
			back: []wsWant{closeWant(1009)}},
		{name: "read limit fragmented", opts: "\tlimit=10", frames: cat(srvFrame(0x02, make([]byte, 6)), srvFrame(0x80, make([]byte, 5))),
			out: []string{errLine("EMSGSIZE", 0, "", 1009)}, back: []wsWant{closeWant(1009)}},
		{name: "within the read limit", opts: "\tlimit=10", frames: cat(srvFrame(0x82, make([]byte, 10)), close1000),
			out: []string{msgLine(2, make([]byte, 10)), ok}, back: []wsWant{closeWant(1000)}},
		{name: "the server goes away", frames: srvFrame(0x81, []byte("last")), noAnswer: true,
			out: []string{msgLine(1, []byte("last")), errLine("EUNEXPECTED_EOF", 1006, "", 0)}},
		{name: "the server goes away in a frame", frames: srvFrame(0x81, []byte("cut"))[:3], noAnswer: true,
			out: []string{errLine("EUNEXPECTED_EOF", 1006, "", 0)}},
		{name: "a read deadline", frames: nil, noAnswer: false, script: []string{"deadline\t100", "recv", "free"},
			out: []string{errLine("ETIMEDOUT", 0, "", 0)}},
		// the client starts the close handshake: its frame, then the server's
		{name: "client close", frames: nil, script: []string{"writeclose\t4000\tbye"}, out: []string{"writeclose ok"}, back: []wsWant{closeWant(4000)}},
		{name: "client sends", script: []string{"send\ttext\t6869", "send\tbinary\t00ff", "send\tping\t70", "send\tpong\t", "writeclose\t1000\t"},
			out:  []string{"sent", "sent", "sent", "sent", "writeclose ok"},
			back: []wsWant{echoOf(1, []byte("hi")), echoOf(2, []byte{0, 0xff}), echoOf(9, []byte("p")), echoOf(10, nil), closeWant(1000)}},
	}
	// an invalid byte at every position of a text from the server
	base := []byte("µ@ßöäü-€𝄞")
	for i := 0; i <= len(base); i++ {
		bad := append(append(append([]byte{}, base[:i]...), 0xc0), base[i:]...)
		cases = append(cases, wsClientCase{name: fmt.Sprintf("0xc0 at %d", i), frames: srvFrame(0x81, bad),
			out: []string{errLine("EILSEQ", 0, "", 1007)}, back: []wsWant{closeWant(1007)}})
	}
	t.Logf("%d cases", len(cases))
	var wg sync.WaitGroup
	sem := make(chan struct{}, 8)
	for _, c := range cases {
		c := c
		wg.Add(1)
		sem <- struct{}{}
		go func() {
			defer wg.Done()
			defer func() { <-sem }()
			runWSClientCase(t, c)
		}()
	}
	wg.Wait()
}

// TestWebSocketCompressionClient: the client's permessage-deflate against
// Go's compress/flate, in every context takeover setting
func TestWebSocketCompressionClient(t *testing.T) {
	const pmd = "permessage-deflate"
	for _, m := range []struct{ opts, resp string }{
		{"\tcompress", pmd},
		{"\tcompress", pmd + "; server_no_context_takeover"},
		{"\tcompress", pmd + "; client_no_context_takeover"},
		{"\tcompress\tnct", pmd + "; server_no_context_takeover; client_no_context_takeover"},
		{"\tcompress\tnct", pmd + "; server_no_context_takeover"}, // the client resets anyway: it said so
		{"\tcompress", pmd + "; server_max_window_bits=10"},
		{"\tcompress\tlevel=9\tframe=50", pmd},
	} {
		ln, err := net.Listen("tcp", "127.0.0.1:0")
		if err != nil {
			t.Fatal(err)
		}
		msgs := wsTestMessages()
		srvTakeover := !strings.Contains(m.resp, "server_no_context_takeover")
		cliTakeover := !strings.Contains(m.resp, "client_no_context_takeover") && !strings.Contains(m.opts, "nct")
		errc := make(chan error, 1)
		go func() {
			errc <- func() error {
				conn, err := ln.Accept()
				if err != nil {
					return err
				}
				defer conn.Close()
				conn.SetDeadline(time.Now().Add(60 * time.Second))
				br := bufio.NewReader(conn)
				req, err := http.ReadRequest(br)
				if err != nil {
					return err
				}
				offer := pmd
				if strings.Contains(m.opts, "nct") {
					offer += "; server_no_context_takeover; client_no_context_takeover"
				}
				if got := req.Header.Get("Sec-WebSocket-Extensions"); got != offer {
					return fmt.Errorf("offer %q, want %q", got, offer)
				}
				fmt.Fprintf(conn, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n"+
					"Sec-WebSocket-Extensions: %s\r\n\r\n", wsAccept(req.Header.Get("Sec-WebSocket-Key")), m.resp)
				def := &goDeflater{takeover: srvTakeover}
				inf := &goInflater{takeover: cliTakeover}
				var sizes []int
				for i, msg := range msgs {
					// the client's message, compressed by it
					op, rsv1, z, err := readWSMessage(br)
					if err != nil {
						return fmt.Errorf("message %d: %v", i, err)
					}
					got, err := inf.decompress(z)
					if !rsv1 || op != 2 || err != nil || !bytes.Equal(got, msg) {
						return fmt.Errorf("message %d: op %d rsv1 %v %d bytes, want %d: %v", i, op, rsv1, len(got), len(msg), err)
					}
					sizes = append(sizes, len(z))
					// ours, compressed by Go (fragmented now and then)
					zz := def.compress(msg)
					if i%3 == 1 && len(zz) > 2 {
						conn.Write(cat(srvFrame(0x42, zz[:2]), srvFrame(0x80, zz[2:])))
					} else {
						conn.Write(srvFrame(0xc2, zz))
					}
				}
				n := len(sizes)
				if cliTakeover && sizes[n-1]*4 > sizes[n-2] {
					return fmt.Errorf("context takeover does not show: %d then %d bytes", sizes[n-2], sizes[n-1])
				}
				if !cliTakeover && sizes[n-1] != sizes[n-2] {
					return fmt.Errorf("no context takeover, yet %d then %d bytes", sizes[n-2], sizes[n-1])
				}
				conn.Write(srvFrame(0x88, closePayload(1000, "")))
				readWSFrame(br)
				return nil
			}()
		}()
		script := []string{"dial\tws://" + ln.Addr().String() + "/" + m.opts}
		var want []string
		want = append(want, "dial ok status=101 proto= ext="+m.resp)
		for _, msg := range msgs {
			script = append(script, "send\tbinary\t"+hex.EncodeToString(msg), "recv")
			want = append(want, "sent", msgLine(2, msg))
		}
		script = append(script, "recv")
		want = append(want, errLine("ECLOSED", 1000, "", 1000))
		wantLines(t, m.resp, runWSC(t, script...), want...)
		if err := <-errc; err != nil {
			t.Errorf("%s%s: %v", m.resp, m.opts, err)
		}
		ln.Close()
	}
}

// TestWebSocketHandshakeClient: the request the client sends, and its
// checks of the response
func TestWebSocketHandshakeClient(t *testing.T) {
	type result struct {
		req *http.Request
		err error
	}
	// serve answers one connection with the response returned by answer
	serve := func(answer func(req *http.Request) string) (string, chan result) {
		ln, err := net.Listen("tcp", "127.0.0.1:0")
		if err != nil {
			t.Fatal(err)
		}
		ch := make(chan result, 1)
		go func() {
			defer ln.Close()
			conn, err := ln.Accept()
			if err != nil {
				ch <- result{err: err}
				return
			}
			defer conn.Close()
			conn.SetDeadline(time.Now().Add(10 * time.Second))
			br := bufio.NewReader(conn)
			req, err := http.ReadRequest(br)
			if err != nil {
				ch <- result{err: err}
				return
			}
			if resp := answer(req); resp != "" {
				conn.Write([]byte(resp))
				io.Copy(io.Discard, br)
			} else {
				time.Sleep(time.Second)
			}
			ch <- result{req: req}
		}()
		return ln.Addr().String(), ch
	}
	good := func(extra string) func(*http.Request) string {
		return func(req *http.Request) string {
			return "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " +
				wsAccept(req.Header.Get("Sec-WebSocket-Key")) + "\r\n" + extra + "\r\n"
		}
	}
	// the request
	var keys []string
	for i := 0; i < 2; i++ {
		addr, ch := serve(good("Sec-WebSocket-Protocol: b\r\n"))
		out := runWSC(t, "dial\tws://"+addr+"/p/a%20th?q=1&r=2\tproto=a,b\tcompress\tnct\thdr=X-Test:42\thdr=Cookie:c=1", "free")
		wantLines(t, "request", out, "dial ok status=101 proto=b ext=")
		r := <-ch
		if r.err != nil {
			t.Fatal(r.err)
		}
		req := r.req
		key, err := base64.StdEncoding.DecodeString(req.Header.Get("Sec-WebSocket-Key"))
		if req.Method != "GET" || req.RequestURI != "/p/a%20th?q=1&r=2" || req.Proto != "HTTP/1.1" || req.Host != addr || err != nil || len(key) != 16 ||
			req.Header.Get("Upgrade") != "websocket" || req.Header.Get("Connection") != "Upgrade" ||
			req.Header.Get("Sec-WebSocket-Version") != "13" || req.Header.Get("Sec-WebSocket-Protocol") != "a, b" ||
			req.Header.Get("Sec-WebSocket-Extensions") != "permessage-deflate; server_no_context_takeover; client_no_context_takeover" ||
			req.Header.Get("X-Test") != "42" || req.Header.Get("Cookie") != "c=1" {
			t.Errorf("request: %s %s %s %s %v", req.Method, req.RequestURI, req.Proto, req.Host, req.Header)
		}
		keys = append(keys, req.Header.Get("Sec-WebSocket-Key"))
	}
	if keys[0] == keys[1] {
		t.Errorf("the same key twice: %s", keys[0])
	}
	// Host replaced, no extras
	addr, ch := serve(good(""))
	wantLines(t, "host", runWSC(t, "dial\tws://"+addr+"\thdr=Host:example.com", "free"), "dial ok status=101 proto= ext=")
	if r := <-ch; r.err != nil || r.req.Host != "example.com" || r.req.RequestURI != "/" || r.req.Header.Get("Sec-WebSocket-Extensions") != "" ||
		r.req.Header.Get("Sec-WebSocket-Protocol") != "" {
		t.Errorf("host: %v %+v", r.err, r.req)
	}
	// responses refused
	bad := []struct {
		name, opts string
		answer     func(*http.Request) string
		want       string
	}{
		{"wrong accept", "", func(*http.Request) string {
			return "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + wsAccept("x") + "\r\n\r\n"
		}, "dial err EBADRESPONSE status=101"},
		{"no accept", "", func(*http.Request) string {
			return "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n\r\n"
		}, "dial err EBADRESPONSE status=101"},
		{"no upgrade", "", func(req *http.Request) string {
			return "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + wsAccept(req.Header.Get("Sec-WebSocket-Key")) + "\r\n\r\n"
		}, "dial err EBADRESPONSE status=101"},
		{"upgrade h2c", "", func(req *http.Request) string {
			return "HTTP/1.1 101 Switching Protocols\r\nUpgrade: h2c\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " +
				wsAccept(req.Header.Get("Sec-WebSocket-Key")) + "\r\n\r\n"
		}, "dial err EBADRESPONSE status=101"},
		{"no connection", "", func(req *http.Request) string {
			return "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nSec-WebSocket-Accept: " + wsAccept(req.Header.Get("Sec-WebSocket-Key")) + "\r\n\r\n"
		}, "dial err EBADRESPONSE status=101"},
		{"status 200", "", func(req *http.Request) string {
			return "HTTP/1.1 200 OK\r\nContent-Length: 4\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " +
				wsAccept(req.Header.Get("Sec-WebSocket-Key")) + "\r\n\r\nnope"
		}, "dial err EBADRESPONSE status=200 body=nope"},
		{"redirect", "", func(*http.Request) string {
			return "HTTP/1.1 302 Found\r\nLocation: /elsewhere\r\nContent-Length: 0\r\n\r\n"
		}, "dial err EBADRESPONSE status=302 body="},
		{"unexpected subprotocol", "", good("Sec-WebSocket-Protocol: chat\r\n"), "dial err EBADRESPONSE status=101"},
		{"subprotocol not offered", "\tproto=a,b", good("Sec-WebSocket-Protocol: c\r\n"), "dial err EBADRESPONSE status=101"},
		{"extension not offered", "", good("Sec-WebSocket-Extensions: permessage-deflate\r\n"), "dial err EBADRESPONSE status=101"},
		{"unknown extension", "\tcompress", good("Sec-WebSocket-Extensions: x-foo\r\n"), "dial err EBADRESPONSE status=101"},
		{"client_max_window_bits not offered", "\tcompress", good("Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits=10\r\n"),
			"dial err EBADRESPONSE status=101"},
		{"server_max_window_bits=16", "\tcompress", good("Sec-WebSocket-Extensions: permessage-deflate; server_max_window_bits=16\r\n"),
			"dial err EBADRESPONSE status=101"},
		{"unknown parameter", "\tcompress", good("Sec-WebSocket-Extensions: permessage-deflate; foo\r\n"), "dial err EBADRESPONSE status=101"},
		{"twice", "\tcompress", good("Sec-WebSocket-Extensions: permessage-deflate, permessage-deflate\r\n"), "dial err EBADRESPONSE status=101"},
		{"malformed", "\tcompress", good("Sec-WebSocket-Extensions: permessage-deflate;\r\n"), "dial err EBADRESPONSE status=101"},
		{"parameter with a value", "\tcompress", good("Sec-WebSocket-Extensions: permessage-deflate; server_no_context_takeover=1\r\n"),
			"dial err EBADRESPONSE status=101"},
		{"duplicate parameter", "\tcompress",
			good("Sec-WebSocket-Extensions: permessage-deflate; client_no_context_takeover; client_no_context_takeover\r\n"),
			"dial err EBADRESPONSE status=101"},
		{"timeout", "\ttimeout=300", func(*http.Request) string { return "" }, "dial err ETIMEDOUT"},
		{"reserved header", "\thdr=Sec-WebSocket-Key:x", nil, "dial err EINVAL"},
	}
	for _, b := range bad {
		if b.answer == nil {
			wantLines(t, b.name, runWSC(t, "dial\tws://127.0.0.1:1/"+b.opts), b.want)
			continue
		}
		addr, ch := serve(b.answer)
		wantLines(t, b.name, runWSC(t, "dial\tws://"+addr+"/"+b.opts), b.want)
		<-ch
	}
	// accepted variants
	for _, g := range []struct{ name, opts, hdr, want string }{
		{"server_max_window_bits", "\tcompress", "Sec-WebSocket-Extensions: permessage-deflate; server_max_window_bits=10\r\n",
			"dial ok status=101 proto= ext=permessage-deflate; server_max_window_bits=10"},
		{"quoted value", "\tcompress", "Sec-WebSocket-Extensions: permessage-deflate; server_max_window_bits=\"12\"\r\n",
			"dial ok status=101 proto= ext=permessage-deflate; server_max_window_bits=\"12\""},
		{"no extension", "\tcompress", "", "dial ok status=101 proto= ext="},
		{"subprotocol", "\tproto=x,y", "Sec-WebSocket-Protocol: y\r\n", "dial ok status=101 proto=y ext="},
		{"tokens in lists", "", "Connection: keep-alive, upgrade\r\n", "dial ok"},
	} {
		addr, ch := serve(good(g.hdr))
		wantLines(t, g.name, runWSC(t, "dial\tws://"+addr+"/"+g.opts, "free"), g.want)
		<-ch
	}
	wantLines(t, "refused", runWSC(t, "dial\tws://127.0.0.1:1/"), "dial err ECONNREFUSED")
	wantLines(t, "scheme", runWSC(t, "dial\thttp://127.0.0.1:1/"), "dial err ESCHEME")
}
