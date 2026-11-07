package tests

import (
	"bufio"
	"crypto/tls"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"strings"
	"testing"
)

var suites12ECDSA = []uint16{
	tls.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, tls.TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
	tls.TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256, tls.TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
	tls.TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
}

var suites12RSA = []uint16{
	tls.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, tls.TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
	tls.TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256, tls.TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
	tls.TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
}

func suitesFor(kind string) []uint16 {
	if kind == "rsa" {
		return suites12RSA
	}
	return suites12ECDSA
}

// talk sends lines over a Go TLS connection and checks tlsd's answers
func talk(t *testing.T, c *tls.Conn, what string) {
	t.Helper()
	st := c.ConnectionState()
	fmt.Fprintf(c, "hello\nbig 100000\nstate\n")
	r := bufio.NewReader(c)
	l, err := r.ReadString('\n')
	if err != nil || !strings.HasPrefix(l, "hello | "+goSuiteGroup(st)) {
		t.Fatalf("%s: hello answer %q %v", what, l, err)
	}
	l, err = r.ReadString('\n')
	if err != nil || len(l) != 100001 || l[:3] != "abc" {
		t.Fatalf("%s: big answer of %d bytes, %v", what, len(l), err)
	}
	l, err = r.ReadString('\n')
	resumed := 0
	if st.DidResume {
		resumed = 1
	}
	if want := fmt.Sprintf("state | %04x %d\n", st.Version, resumed); err != nil || l != want {
		t.Fatalf("%s: state %q want %q (%v)", what, l, want, err)
	}
}

// TestTLS12ServerAgainstGo: Go's TLS 1.2 client against the library's
// server, every suite of every key kind, the groups, client
// certificates, the version limits
func TestTLS12ServerAgainstGo(t *testing.T) {
	p := makePKI(t)
	for _, kind := range []string{"p256", "p384", "p521", "ed25519", "rsa"} {
		kind := kind
		t.Run(kind, func(t *testing.T) {
			addr := startTLSD(t, p.leaves[kind][0], p.leaves[kind][1], "-alpn", "h2,http/1.1")
			for _, suite := range suitesFor(kind) {
				for _, curves := range [][]tls.CurveID{nil, {tls.CurveP256}, {tls.CurveP384}, {tls.CurveP521}} {
					cfg := &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12,
						CipherSuites: []uint16{suite}, CurvePreferences: curves, NextProtos: []string{"http/1.1"}}
					c, err := tls.Dial("tcp", addr, cfg)
					what := fmt.Sprintf("%04x %v", suite, curves)
					if err != nil {
						t.Fatalf("%s: %v", what, err)
					}
					st := c.ConnectionState()
					g := tls.X25519
					if len(curves) > 0 {
						g = curves[0]
					}
					if st.Version != tls.VersionTLS12 || st.CipherSuite != suite || st.CurveID != g || st.NegotiatedProtocol != "http/1.1" ||
						st.PeerCertificates[0].Subject.CommonName != "leaf "+kind {
						t.Errorf("%s: version %x suite %04x group %v proto %q", what, st.Version, st.CipherSuite, st.CurveID, st.NegotiatedProtocol)
					}
					talk(t, c, what)
					c.Close()
				}
			}
			// Go's default suites: the server's preference (AES-GCM with AES hardware)
			c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12})
			if err != nil {
				t.Fatal(err)
			}
			if s := c.ConnectionState().CipherSuite; s != suitesFor(kind)[0] {
				t.Errorf("default suite %04x", s)
			}
			c.Close()
		})
	}
	t.Run("client-auth", func(t *testing.T) {
		addr := startTLSD(t, p.leaves["rsa"][0], p.leaves["rsa"][1], "-auth", "4", "-ca", p.caPEM)
		cc, _ := tls.LoadX509KeyPair(p.clientC[0], p.clientC[1])
		c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12,
			Certificates: []tls.Certificate{cc}})
		if err != nil {
			t.Fatal(err)
		}
		fmt.Fprintf(c, "who\n")
		l, _ := bufio.NewReader(c).ReadString('\n')
		if !strings.HasSuffix(strings.TrimSpace(l), "the client") {
			t.Errorf("server saw %q", l)
		}
		c.Close()
		c, err = tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12})
		if err == nil {
			fmt.Fprintf(c, "who\n")
			_, err = bufio.NewReader(c).ReadString('\n')
			c.Close()
		}
		if err == nil || !strings.Contains(err.Error(), "bad certificate") {
			t.Errorf("without a client certificate: %v", err)
		}
	})
	t.Run("versions", func(t *testing.T) {
		addr := startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-max", "303")
		c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost"})
		if err != nil {
			t.Fatal(err)
		}
		if v := c.ConnectionState().Version; v != tls.VersionTLS12 {
			t.Errorf("version %x with a TLS 1.2 server", v)
		}
		c.Close()
		_, err = tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MinVersion: tls.VersionTLS13})
		if err == nil || !strings.Contains(err.Error(), "protocol version not supported") {
			t.Errorf("a TLS 1.3 client against a TLS 1.2 server: %v", err)
		}
		addr = startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-min", "304")
		_, err = tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12})
		if err == nil || !strings.Contains(err.Error(), "protocol version not supported") {
			t.Errorf("a TLS 1.2 client against a TLS 1.3 server: %v", err)
		}
	})
	t.Run("suites", func(t *testing.T) {
		// only CBC configured on the server
		addr := startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-suites", "c00a,c009")
		c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12})
		if err != nil {
			t.Fatal(err)
		}
		if s := c.ConnectionState().CipherSuite; s != tls.TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA {
			t.Errorf("suite %04x", s)
		}
		talk(t, c, "cbc")
		c.Close()
		// no common suite
		_, err = tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: tls.VersionTLS12,
			CipherSuites: []uint16{tls.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256}})
		if err == nil || !strings.Contains(err.Error(), "handshake failure") {
			t.Errorf("no common suite: %v", err)
		}
	})
}

// TestTLS12ClientAgainstGo: the library's TLS 1.2 client against Go's server
func TestTLS12ClientAgainstGo(t *testing.T) {
	p := makePKI(t)
	for _, kind := range []string{"p256", "p384", "p521", "ed25519", "rsa"} {
		cert, err := tls.LoadX509KeyPair(p.leaves[kind][0], p.leaves[kind][1])
		if err != nil {
			t.Fatal(err)
		}
		addr := goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}, MaxVersion: tls.VersionTLS12,
			NextProtos: []string{"h2", "http/1.1"}})
		type tc struct {
			args []string
			want string
		}
		cases := []tc{{nil, fmt.Sprintf("ok %04x 001d - 1", suitesFor(kind)[0])}}
		for _, s := range suitesFor(kind) {
			cases = append(cases, tc{[]string{"-suites", fmt.Sprintf("%04x", s)}, fmt.Sprintf("ok %04x 001d - 1", s)})
		}
		for _, g := range []string{"17", "18", "19"} {
			cases = append(cases, tc{[]string{"-groups", g}, fmt.Sprintf("ok %04x 00%s - 1", suitesFor(kind)[0], g)})
		}
		cases = append(cases, tc{[]string{"-alpn", "x,http/1.1"}, fmt.Sprintf("ok %04x 001d http/1.1 1", suitesFor(kind)[0])})
		for _, c := range cases {
			args := append([]string{addr, "-name", "localhost", "-roots", p.caPEM, "-v"}, c.args...)
			out := runTLSC(t, "hello\nbig 200000\nagain\n", args...)
			if len(out) != 5 || out[0] != c.want || out[1] != "v 0303 0" || !strings.HasPrefix(out[2], "hello | ") ||
				out[3] != "big 200000 ok" || !strings.HasPrefix(out[4], "again | ") {
				t.Errorf("%s %v: %q", kind, c.args, out)
			}
		}
	}
	cert, _ := tls.LoadX509KeyPair(p.leaves["p256"][0], p.leaves["p256"][1])
	// a TLS 1.3 server, the client limited to TLS 1.2; a TLS 1.2 server, the client wanting TLS 1.3
	addr := goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}})
	out := runTLSC(t, "", addr, "-name", "localhost", "-roots", p.caPEM, "-max", "303", "-v")
	if len(out) != 2 || out[1] != "v 0303 0" {
		t.Errorf("-max 303: %q", out)
	}
	addr12 := goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}, MaxVersion: tls.VersionTLS12})
	out = runTLSC(t, "", addr12, "-name", "localhost", "-roots", p.caPEM, "-min", "304")
	if want := "error remote error: tls: protocol version not supported"; out[0] != want {
		t.Errorf("-min 304: %q want %q", out[0], want)
	}
	// client certificates in TLS 1.2
	addr = goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}, ClientAuth: tls.RequireAndVerifyClientCert,
		ClientCAs: p.pool, MaxVersion: tls.VersionTLS12})
	out = runTLSC(t, "who\n", addr, "-name", "localhost", "-roots", p.caPEM, "-cert", p.clientC[0], p.clientC[1])
	if len(out) != 2 || !strings.HasSuffix(out[1], "the client") {
		t.Errorf("client certificate: %q", out)
	}
	out = runTLSC(t, "who\n", addr, "-name", "localhost", "-roots", p.caPEM)
	if j := strings.Join(out, " "); !strings.Contains(j, "error") {
		t.Errorf("without a client certificate: %q", out)
	}
	// verification failures are the same as in TLS 1.3
	out = runTLSC(t, "", addr12, "-name", "wrong.example", "-roots", p.caPEM)
	if want := "error tls: failed to verify certificate: x509: certificate is valid for localhost, example.com, not wrong.example"; out[0] != want {
		t.Errorf("wrong name: %q", out[0])
	}
}

// TestTLSResumption: tickets in both versions, both directions
func TestTLSResumption(t *testing.T) {
	p := makePKI(t)
	cert, _ := tls.LoadX509KeyPair(p.leaves["p256"][0], p.leaves["p256"][1])
	for _, v := range []uint16{tls.VersionTLS12, tls.VersionTLS13} {
		vs := fmt.Sprintf("%04x", v)
		t.Run("go-client-"+vs, func(t *testing.T) {
			addr := startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-alpn", "http/1.1")
			cache := tls.NewLRUClientSessionCache(4)
			for i := 0; i < 3; i++ {
				c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: v,
					ClientSessionCache: cache, NextProtos: []string{"http/1.1"}})
				if err != nil {
					t.Fatal(err)
				}
				st := c.ConnectionState()
				if st.DidResume != (i > 0) || st.Version != v || len(st.PeerCertificates) == 0 || st.NegotiatedProtocol != "http/1.1" {
					t.Errorf("connection %d: resumed %v version %x certs %d proto %q", i, st.DidResume, st.Version,
						len(st.PeerCertificates), st.NegotiatedProtocol)
				}
				talk(t, c, fmt.Sprintf("connection %d", i))
				c.Close()
			}
		})
		t.Run("go-client-auth-"+vs, func(t *testing.T) {
			// the client certificate comes back with the session
			addr := startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-auth", "4", "-ca", p.caPEM)
			cc, _ := tls.LoadX509KeyPair(p.clientC[0], p.clientC[1])
			cache := tls.NewLRUClientSessionCache(4)
			for i := 0; i < 2; i++ {
				c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", MaxVersion: v,
					ClientSessionCache: cache, Certificates: []tls.Certificate{cc}})
				if err != nil {
					t.Fatal(err)
				}
				fmt.Fprintf(c, "who\nstate\n")
				r := bufio.NewReader(c)
				l1, _ := r.ReadString('\n')
				l2, _ := r.ReadString('\n')
				if !strings.HasSuffix(strings.TrimSpace(l1), "the client") || l2 != fmt.Sprintf("state | %04x %d\n", v, i) {
					t.Errorf("connection %d: %q %q", i, l1, l2)
				}
				c.Close()
			}
		})
		t.Run("client-go-"+vs, func(t *testing.T) {
			addr := goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}, MaxVersion: v})
			out := runTLSC(t, "hello\n", addr, "-name", "localhost", "-roots", p.caPEM, "-conns", "3", "-v")
			want := []string{"ok", "v " + vs + " 0", "ok", "v " + vs + " 1", "ok", "v " + vs + " 1", "hello | "}
			if len(out) != len(want) {
				t.Fatalf("%q", out)
			}
			for i, w := range want {
				if !strings.HasPrefix(out[i], w) {
					t.Errorf("line %d: %q want %q", i, out[i], w)
				}
			}
			// the resumed connections still report the verified chain
			if out[4] != out[0] {
				t.Errorf("resumed state %q, first %q", out[4], out[0])
			}
		})
		t.Run("self-"+vs, func(t *testing.T) {
			addr := startTLSD(t, p.leaves["ed25519"][0], p.leaves["ed25519"][1])
			out := runTLSC(t, "state\n", addr, "-name", "localhost", "-roots", p.caPEM, "-conns", "2", "-v", "-max", vs)
			if len(out) != 5 || out[1] != "v "+vs+" 0" || out[3] != "v "+vs+" 1" || out[4] != "state | "+vs+" 1" {
				t.Errorf("%q", out)
			}
			// tickets off: full handshakes
			addr = startTLSD(t, p.leaves["ed25519"][0], p.leaves["ed25519"][1], "-notickets")
			out = runTLSC(t, "", addr, "-name", "localhost", "-roots", p.caPEM, "-conns", "2", "-v", "-max", vs)
			if len(out) != 4 || out[3] != "v "+vs+" 0" {
				t.Errorf("without tickets: %q", out)
			}
		})
	}
}

// stripVersions forwards a connection, removing supported_versions from
// the ClientHello: a downgrade attack the canaries must reveal
func stripVersions(t *testing.T, target string) string {
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { ln.Close() })
	go func() {
		for {
			c, err := ln.Accept()
			if err != nil {
				return
			}
			go func(c net.Conn) {
				defer c.Close()
				s, err := net.Dial("tcp", target)
				if err != nil {
					return
				}
				defer s.Close()
				hdr := make([]byte, 5)
				if _, err := io.ReadFull(c, hdr); err != nil {
					return
				}
				body := make([]byte, binary.BigEndian.Uint16(hdr[3:]))
				if _, err := io.ReadFull(c, body); err != nil {
					return
				}
				// handshake header, version, random, session id, suites, compression, extensions
				m := body[4:]
				off := 2 + 32
				off += 1 + int(m[off])
				off += 2 + int(binary.BigEndian.Uint16(m[off:]))
				off += 1 + int(m[off])
				exts := m[off+2:]
				var keep []byte
				for len(exts) >= 4 {
					typ, n := binary.BigEndian.Uint16(exts), int(binary.BigEndian.Uint16(exts[2:]))
					if typ != 43 {
						keep = append(keep, exts[:4+n]...)
					}
					exts = exts[4+n:]
				}
				nm := append(append(append([]byte{}, m[:off]...), byte(len(keep)>>8), byte(len(keep))), keep...)
				hs := append([]byte{1, byte(len(nm) >> 16), byte(len(nm) >> 8), byte(len(nm))}, nm...)
				rec := append([]byte{22, 3, 1, byte(len(hs) >> 8), byte(len(hs))}, hs...)
				s.Write(rec)
				go io.Copy(c, s)
				io.Copy(s, c)
			}(c)
		}
	}()
	return ln.Addr().String()
}

func TestTLSDowngradeProtection(t *testing.T) {
	p := makePKI(t)
	cert, _ := tls.LoadX509KeyPair(p.leaves["p256"][0], p.leaves["p256"][1])
	// the library's client through the attacker to Go's server
	addr := stripVersions(t, goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}}))
	out := runTLSC(t, "", addr, "-name", "localhost", "-roots", p.caPEM)
	if want := "error tls: downgrade attempt detected, possibly due to a MitM attack or a broken middlebox"; out[0] != want {
		t.Errorf("client: %q", out[0])
	}
	// Go's client through the attacker to the library's server: its canary is set
	addr = stripVersions(t, startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1]))
	_, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost"})
	if err == nil || !strings.Contains(err.Error(), "downgrade attempt detected") {
		t.Errorf("server: %v", err)
	}
}
