package main

// WebSocket peers for tests/stdwebsocket_test.go: servers and scripted
// clients of github.com/gorilla/websocket and golang.org/x/net/websocket.
//
//	goref ws-server -lib gorilla|xnet [-cert f -key f]
//	goref ws-client -lib gorilla|xnet -url u -script s [-compress] [-proto a,b] [-roots f]
//
// The server prints "listening <addr>", then "end <path> <code> <reason>"
// when a connection ends (the close code it received, 1006 when none);
// it stops when stdin closes. The client prints one line per step.

import (
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"errors"
	"flag"
	"fmt"
	"io"
	"math/rand"
	"net"
	"net/http"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	gorilla "github.com/gorilla/websocket"
	xnet "golang.org/x/net/websocket"
)

var outMu sync.Mutex

func say(format string, a ...interface{}) {
	outMu.Lock()
	fmt.Printf(format+"\n", a...)
	outMu.Unlock()
}

// wsMessage: deterministic contents; text is valid UTF-8 of exactly n bytes
func wsMessage(seed int64, n int, text bool) []byte {
	r := rand.New(rand.NewSource(seed))
	b := make([]byte, 0, n)
	if !text {
		b = b[:n]
		r.Read(b)
		return b
	}
	words := []string{"web", "socket ", "frame ", "π ", "deflate ", "日本 ", "context ", "\n", "€"}
	for len(b) < n {
		w := words[r.Intn(len(words))]
		if len(b)+len(w) > n {
			for len(b) < n {
				b = append(b, 'x')
			}
			break
		}
		b = append(b, w...)
	}
	return b
}

var wsSizes = []int{0, 1, 125, 126, 127, 65535, 65536, 65537, 1 << 20, 5 << 20}

func closeOf(err error) (int, string) {
	var ce *gorilla.CloseError
	if errors.As(err, &ce) {
		return ce.Code, ce.Text
	}
	return 1006, err.Error()
}

func wsServer(args []string) {
	fs := flag.NewFlagSet("ws-server", flag.ExitOnError)
	lib := fs.String("lib", "gorilla", "")
	cert := fs.String("cert", "", "")
	key := fs.String("key", "", "")
	fs.Parse(args)
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	say("listening %s", ln.Addr())
	go func() {
		io.Copy(io.Discard, os.Stdin)
		os.Exit(0)
	}()
	mux := http.NewServeMux()
	if *lib == "gorilla" {
		gorillaHandlers(mux)
	} else {
		xnetHandlers(mux)
	}
	srv := &http.Server{Handler: mux}
	if *cert != "" {
		err = srv.ServeTLS(ln, *cert, *key)
	} else {
		err = srv.Serve(ln)
	}
	fmt.Fprintln(os.Stderr, err)
}

func gorillaHandlers(mux *http.ServeMux) {
	up := gorilla.Upgrader{EnableCompression: true, Subprotocols: []string{"chat", "superchat"}, WriteBufferSize: 512,
		CheckOrigin: func(*http.Request) bool { return true }}
	echo := func(c *gorilla.Conn, path string) {
		for {
			mt, rd, err := c.NextReader()
			if err != nil {
				code, text := closeOf(err)
				say("end %s %d %s", path, code, text)
				c.Close()
				return
			}
			w, err := c.NextWriter(mt)
			if err != nil {
				continue
			}
			io.Copy(w, rd)
			w.Close()
		}
	}
	mux.HandleFunc("/echo", func(w http.ResponseWriter, r *http.Request) {
		c, err := up.Upgrade(w, r, http.Header{"X-Gorilla": {"yes"}})
		if err != nil {
			return
		}
		echo(c, "/echo")
	})
	mux.HandleFunc("/info", func(w http.ResponseWriter, r *http.Request) {
		c, err := up.Upgrade(w, r, nil)
		if err != nil {
			return
		}
		c.WriteMessage(gorilla.TextMessage, []byte(fmt.Sprintf("proto=%s ext=%s origin=%s x=%s", c.Subprotocol(),
			r.Header.Get("Sec-Websocket-Extensions"), r.Header.Get("Origin"), r.Header.Get("X-Test"))))
		echo(c, "/info")
	})
	mux.HandleFunc("/close", func(w http.ResponseWriter, r *http.Request) {
		c, err := up.Upgrade(w, r, nil)
		if err != nil {
			return
		}
		code, _ := strconv.Atoi(r.URL.Query().Get("code"))
		c.WriteControl(gorilla.CloseMessage, gorilla.FormatCloseMessage(code, r.URL.Query().Get("reason")), time.Now().Add(time.Second))
		echo(c, "/close")
	})
	mux.HandleFunc("/ping", func(w http.ResponseWriter, r *http.Request) {
		c, err := up.Upgrade(w, r, nil)
		if err != nil {
			return
		}
		c.SetPongHandler(func(s string) error {
			return c.WriteMessage(gorilla.TextMessage, []byte("pong:"+s))
		})
		c.WriteControl(gorilla.PingMessage, []byte("gping"), time.Now().Add(time.Second))
		echo(c, "/ping")
	})
	mux.HandleFunc("/drop", func(w http.ResponseWriter, r *http.Request) {
		c, err := up.Upgrade(w, r, nil)
		if err != nil {
			return
		}
		c.ReadMessage()
		c.NetConn().Close()
	})
}

type xnetMsg struct {
	typ  byte
	data []byte
}

var xnetRaw = xnet.Codec{
	Marshal: func(v interface{}) ([]byte, byte, error) {
		m := v.(xnetMsg)
		return m.data, m.typ, nil
	},
	Unmarshal: func(d []byte, t byte, v interface{}) error {
		m := v.(*xnetMsg)
		m.typ, m.data = t, d
		return nil
	},
}

func xnetHandlers(mux *http.ServeMux) {
	accept := func(*xnet.Config, *http.Request) error { return nil }
	mux.Handle("/echo", xnet.Server{Handshake: accept, Handler: func(ws *xnet.Conn) {
		for {
			var m xnetMsg
			if err := xnetRaw.Receive(ws, &m); err != nil {
				say("end /echo %v", err)
				ws.Close() // x/net does not answer a close frame itself
				return
			}
			if err := xnetRaw.Send(ws, m); err != nil {
				say("end /echo send %v", err)
				return
			}
		}
	}})
	mux.Handle("/info", xnet.Server{Handshake: accept, Handler: func(ws *xnet.Conn) {
		xnet.Message.Send(ws, fmt.Sprintf("proto=%s origin=%s", strings.Join(ws.Config().Protocol, ","), ws.Request().Header.Get("Origin")))
		var m xnetMsg
		xnetRaw.Receive(ws, &m)
	}})
}

func wsClient(args []string) {
	fs := flag.NewFlagSet("ws-client", flag.ExitOnError)
	lib := fs.String("lib", "gorilla", "")
	url := fs.String("url", "", "")
	script := fs.String("script", "echo", "")
	compress := fs.Bool("compress", false, "")
	proto := fs.String("proto", "", "")
	roots := fs.String("roots", "", "")
	fs.Parse(args)
	var tc *tls.Config
	if *roots != "" {
		pem, err := os.ReadFile(*roots)
		if err != nil {
			say("roots: %v", err)
			return
		}
		pool := x509.NewCertPool()
		pool.AppendCertsFromPEM(pem)
		tc = &tls.Config{RootCAs: pool, ServerName: "localhost"}
	}
	var protos []string
	if *proto != "" {
		protos = strings.Split(*proto, ",")
	}
	if *lib == "xnet" {
		xnetClient(*url, *script, protos, tc)
		return
	}
	d := gorilla.Dialer{EnableCompression: *compress, Subprotocols: protos, TLSClientConfig: tc, HandshakeTimeout: 10 * time.Second,
		WriteBufferSize: 1000}
	c, resp, err := d.Dial(*url, http.Header{"X-Test": {"42"}})
	if err != nil {
		st := 0
		if resp != nil {
			st = resp.StatusCode
		}
		say("dial error %d %v", st, err)
		return
	}
	defer c.Close()
	say("dial ok proto=%s ext=%s", c.Subprotocol(), resp.Header.Get("Sec-Websocket-Extensions"))
	c.SetReadDeadline(time.Now().Add(20 * time.Second))
	readEcho := func(want []byte, mt int) bool {
		gt, got, err := c.ReadMessage()
		if err != nil {
			code, text := closeOf(err)
			say("read error %d %s", code, text)
			return false
		}
		if gt != mt || !bytes.Equal(got, want) {
			say("mismatch type %d/%d len %d/%d", gt, mt, len(got), len(want))
			return false
		}
		return true
	}
	switch *script {
	case "echo":
		n := 0
		for i, size := range wsSizes {
			for _, text := range []bool{true, false} {
				mt := gorilla.BinaryMessage
				if text {
					mt = gorilla.TextMessage
				}
				m := wsMessage(int64(i*2+n), size, text)
				if err := c.WriteMessage(mt, m); err != nil {
					say("write %v", err)
					return
				}
				if !readEcho(m, mt) {
					return
				}
				n++
			}
		}
		say("echo %d ok", n)
	case "stream":
		r := rand.New(rand.NewSource(1))
		n := 0
		for i, size := range wsSizes {
			m := wsMessage(int64(100+i), size, i%2 == 0)
			mt := gorilla.BinaryMessage
			if i%2 == 0 {
				mt = gorilla.TextMessage
			}
			w, err := c.NextWriter(mt)
			if err != nil {
				say("next writer %v", err)
				return
			}
			for off := 0; off < len(m); {
				k := 1 + r.Intn(3000)
				if off+k > len(m) {
					k = len(m) - off
				}
				w.Write(m[off : off+k])
				off += k
			}
			w.Close()
			if !readEcho(m, mt) {
				return
			}
			n++
		}
		say("stream %d ok", n)
	case "ping":
		pongs := make(chan string, 4)
		c.SetPongHandler(func(s string) error { pongs <- s; return nil })
		c.WriteControl(gorilla.PingMessage, []byte("p1"), time.Now().Add(time.Second))
		c.WriteMessage(gorilla.TextMessage, []byte("after-ping"))
		if readEcho([]byte("after-ping"), gorilla.TextMessage) {
			select {
			case p := <-pongs:
				say("pong %s", p)
			default:
				say("no pong")
			}
		}
	case "close":
		c.WriteControl(gorilla.CloseMessage, gorilla.FormatCloseMessage(4001, "bye"), time.Now().Add(time.Second))
		for {
			if _, _, err := c.ReadMessage(); err != nil {
				code, text := closeOf(err)
				say("closed %d %q", code, text)
				break
			}
		}
	case "read":
		// whatever the server sends first, then its close
		for {
			mt, m, err := c.ReadMessage()
			if err != nil {
				code, text := closeOf(err)
				say("closed %d %q", code, text)
				return
			}
			say("msg %d %s", mt, m)
		}
	case "badutf8":
		c.WriteMessage(gorilla.TextMessage, []byte("ok \xff\xfe"))
		_, _, err := c.ReadMessage()
		code, _ := closeOf(err)
		say("closed %d", code)
	case "big":
		c.WriteMessage(gorilla.BinaryMessage, make([]byte, 2<<20))
		_, _, err := c.ReadMessage()
		code, _ := closeOf(err)
		say("closed %d", code)
	}
}

func xnetClient(url, script string, protos []string, tc *tls.Config) {
	host := strings.SplitN(strings.SplitN(url, "//", 2)[1], "/", 2)[0]
	origin := "http://" + host
	cfg, err := xnet.NewConfig(url, origin)
	if err != nil {
		say("config %v", err)
		return
	}
	cfg.Protocol = protos
	cfg.TlsConfig = tc
	ws, err := xnet.DialConfig(cfg)
	if err != nil {
		say("dial error %v", err)
		return
	}
	say("dial ok")
	ws.SetDeadline(time.Now().Add(20 * time.Second))
	switch script {
	case "echo":
		n := 0
		for i, size := range wsSizes {
			for _, text := range []bool{true, false} {
				typ := byte(xnet.BinaryFrame)
				if text {
					typ = xnet.TextFrame
				}
				m := wsMessage(int64(i*2+n), size, text)
				if err := xnetRaw.Send(ws, xnetMsg{typ, m}); err != nil {
					say("send %v", err)
					return
				}
				var got xnetMsg
				if err := xnetRaw.Receive(ws, &got); err != nil {
					say("receive %v", err)
					return
				}
				if got.typ != typ || !bytes.Equal(got.data, m) {
					say("mismatch type %d len %d/%d", got.typ, len(got.data), len(m))
					return
				}
				n++
			}
		}
		say("echo %d ok", n)
		ws.Close()
	case "read":
		var got xnetMsg
		if err := xnetRaw.Receive(ws, &got); err != nil {
			say("receive %v", err)
			return
		}
		say("msg %d %s", got.typ, got.data)
		ws.Close()
	case "ping":
		// a ping as a frame of its own; the server answers with a pong,
		// which x/net drops; then an echo
		ws.PayloadType = xnet.PingFrame
		ws.Write([]byte("xp"))
		ws.PayloadType = xnet.TextFrame
		xnetRaw.Send(ws, xnetMsg{xnet.TextFrame, []byte("after")})
		var got xnetMsg
		err := xnetRaw.Receive(ws, &got)
		say("msg %v %s", err, got.data)
		ws.Close()
	}
}
