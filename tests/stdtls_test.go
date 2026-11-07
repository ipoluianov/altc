package tests

import (
	"bufio"
	"bytes"
	"crypto"
	"crypto/ecdsa"
	"crypto/ed25519"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/pem"
	"fmt"
	"io"
	"math/big"
	"net"
	"net/http"
	"net/http/httptest"
	"net/http/httptrace"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

type pki struct {
	dir     string
	caPEM   string            // the CA certificate file
	leaves  map[string][2]string // kind -> cert, key files
	clientC [2]string
	pool    *x509.CertPool
}

func writePEM(t *testing.T, path, typ string, der []byte) {
	if err := os.WriteFile(path, pem.EncodeToMemory(&pem.Block{Type: typ, Bytes: der}), 0o600); err != nil {
		t.Fatal(err)
	}
}

// makePKI: an ECDSA CA, leaves of every key kind for localhost,
// 127.0.0.1 and example.com (the "other" one only for other.example),
// and a client certificate
func makePKI(t *testing.T) *pki {
	dir := t.TempDir()
	p := &pki{dir: dir, leaves: map[string][2]string{}, pool: x509.NewCertPool()}
	caKey, _ := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	now := time.Now()
	ca := &x509.Certificate{SerialNumber: big.NewInt(1), Subject: pkix.Name{CommonName: "AltC TLS Test CA"}, NotBefore: now.Add(-time.Hour),
		NotAfter: now.Add(24 * time.Hour), IsCA: true, BasicConstraintsValid: true, KeyUsage: x509.KeyUsageCertSign}
	caDER, _ := x509.CreateCertificate(rand.Reader, ca, ca, caKey.Public(), caKey)
	caCert, _ := x509.ParseCertificate(caDER)
	p.pool.AddCert(caCert)
	p.caPEM = filepath.Join(dir, "ca.pem")
	writePEM(t, p.caPEM, "CERTIFICATE", caDER)
	keys := map[string]crypto.Signer{}
	k256, _ := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	k384, _ := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	k521, _ := ecdsa.GenerateKey(elliptic.P521(), rand.Reader)
	_, ked, _ := ed25519.GenerateKey(rand.Reader)
	krsa, _ := rsa.GenerateKey(rand.Reader, 2048)
	kother, _ := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	keys["p256"], keys["p384"], keys["p521"], keys["ed25519"], keys["rsa"], keys["other"] = k256, k384, k521, ked, krsa, kother
	serial := int64(10)
	for kind, key := range keys {
		serial++
		leaf := &x509.Certificate{SerialNumber: big.NewInt(serial), Subject: pkix.Name{CommonName: "leaf " + kind},
			NotBefore: now.Add(-time.Hour), NotAfter: now.Add(24 * time.Hour), KeyUsage: x509.KeyUsageDigitalSignature,
			ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
			DNSNames:    []string{"localhost", "example.com"}, IPAddresses: []net.IP{net.ParseIP("127.0.0.1")}}
		if kind == "other" {
			leaf.DNSNames, leaf.IPAddresses = []string{"other.example"}, nil
		}
		der, err := x509.CreateCertificate(rand.Reader, leaf, caCert, key.Public(), caKey)
		if err != nil {
			t.Fatal(err)
		}
		kd, _ := x509.MarshalPKCS8PrivateKey(key)
		files := [2]string{filepath.Join(dir, kind+".pem"), filepath.Join(dir, kind+".key")}
		writePEM(t, files[0], "CERTIFICATE", der)
		// the RSA key as PKCS #1, the P-384 one as SEC 1: all formats get read
		switch kind {
		case "rsa":
			writePEM(t, files[1], "RSA PRIVATE KEY", x509.MarshalPKCS1PrivateKey(krsa))
		case "p384":
			sec1, _ := x509.MarshalECPrivateKey(k384)
			writePEM(t, files[1], "EC PRIVATE KEY", sec1)
		default:
			writePEM(t, files[1], "PRIVATE KEY", kd)
		}
		p.leaves[kind] = files
	}
	ck, _ := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	client := &x509.Certificate{SerialNumber: big.NewInt(99), Subject: pkix.Name{CommonName: "the client"}, NotBefore: now.Add(-time.Hour),
		NotAfter: now.Add(24 * time.Hour), KeyUsage: x509.KeyUsageDigitalSignature, ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth}}
	cder, _ := x509.CreateCertificate(rand.Reader, client, caCert, ck.Public(), caKey)
	ckd, _ := x509.MarshalPKCS8PrivateKey(ck)
	p.clientC = [2]string{filepath.Join(dir, "client.pem"), filepath.Join(dir, "client.key")}
	writePEM(t, p.clientC[0], "CERTIFICATE", cder)
	writePEM(t, p.clientC[1], "PRIVATE KEY", ckd)
	return p
}

// startTLSD runs the C server with the arguments; returns its address
func startTLSD(t *testing.T, args ...string) string {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "tlsd.c"), filepath.Join(tempDir(t), "tlsd"))
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
		t.Fatalf("tlsd did not start: %q %v\n%s", line, err, stderr.String())
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
		if s := stderr.String(); s != "" {
			t.Logf("tlsd stderr:\n%s", s)
		}
	})
	return strings.TrimSpace(strings.TrimPrefix(line, "listening "))
}

func goSuiteGroup(s tls.ConnectionState) string {
	return fmt.Sprintf("%04x %04x", s.CipherSuite, uint16(s.CurveID))
}

// TestTLSServerAgainstGo: Go's client against the library's server
func TestTLSServerAgainstGo(t *testing.T) {
	p := makePKI(t)
	for _, kind := range []string{"p256", "p384", "p521", "ed25519", "rsa"} {
		kind := kind
		t.Run(kind, func(t *testing.T) {
			addr := startTLSD(t, p.leaves[kind][0], p.leaves[kind][1], "-alpn", "h2,http/1.1")
			for _, curves := range [][]tls.CurveID{nil, {tls.X25519}, {tls.CurveP256}, {tls.CurveP384}, {tls.CurveP521}, {tls.CurveP384, tls.X25519}} {
				cfg := &tls.Config{RootCAs: p.pool, ServerName: "localhost", CurvePreferences: curves, NextProtos: []string{"http/1.1"}}
				c, err := tls.Dial("tcp", addr, cfg)
				if err != nil {
					t.Fatalf("%v: %v", curves, err)
				}
				st := c.ConnectionState()
				want := map[tls.CurveID]uint16{0: 0x1d, tls.X25519: 0x1d, tls.CurveP256: 0x17, tls.CurveP384: 0x18, tls.CurveP521: 0x19}
				g := want[0]
				if len(curves) > 0 {
					g = want[curves[0]]
				}
				if len(curves) == 2 {
					g = 0x1d // our server prefers X25519: a retry asks for it
				}
				if uint16(st.CurveID) != g || st.NegotiatedProtocol != "http/1.1" || st.Version != tls.VersionTLS13 {
					t.Errorf("%v: group %04x proto %q version %x", curves, uint16(st.CurveID), st.NegotiatedProtocol, st.Version)
				}
				if st.PeerCertificates[0].Subject.CommonName != "leaf "+kind {
					t.Errorf("certificate %q", st.PeerCertificates[0].Subject.CommonName)
				}
				fmt.Fprintf(c, "hello\nkeyupdate\nbig 300000\nafter\n")
				r := bufio.NewReader(c)
				for _, w := range []string{"hello | " + goSuiteGroup(st) + " http/1.1 localhost -", "keyupdate | ", "big", "after | "} {
					l, err := r.ReadString('\n')
					if err != nil {
						t.Fatalf("read: %v", err)
					}
					if w == "big" {
						if len(l) != 300001 || l[:3] != "abc" {
							t.Errorf("big answer of %d bytes", len(l))
						}
					} else if !strings.HasPrefix(l, w) {
						t.Errorf("got %q want %q", l, w)
					}
				}
				c.Close()
			}
		})
	}
	t.Run("sni", func(t *testing.T) {
		addr := startTLSD(t, p.leaves["other"][0], p.leaves["other"][1], p.leaves["ed25519"][0], p.leaves["ed25519"][1])
		for _, name := range []string{"localhost", "other.example"} {
			c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: name})
			if err != nil {
				t.Fatalf("%s: %v", name, err)
			}
			cn := c.ConnectionState().PeerCertificates[0].Subject.CommonName
			if (name == "localhost") != (cn == "leaf ed25519") {
				t.Errorf("%s: certificate %q", name, cn)
			}
			c.Close()
		}
	})
	t.Run("alpn", func(t *testing.T) {
		addr := startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-alpn", "h2,http/1.1")
		c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", NextProtos: []string{"h3"}})
		if err == nil {
			c.Close()
			t.Fatal("no error for an unsupported protocol")
		}
		if !strings.Contains(err.Error(), "no application protocol") {
			t.Errorf("error %v", err)
		}
	})
	t.Run("client-auth", func(t *testing.T) {
		addr := startTLSD(t, p.leaves["p256"][0], p.leaves["p256"][1], "-auth", "4", "-ca", p.caPEM)
		cc, err := tls.LoadX509KeyPair(p.clientC[0], p.clientC[1])
		if err != nil {
			t.Fatal(err)
		}
		c, err := tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost", Certificates: []tls.Certificate{cc}})
		if err != nil {
			t.Fatal(err)
		}
		fmt.Fprintf(c, "who\n")
		l, _ := bufio.NewReader(c).ReadString('\n')
		if !strings.HasSuffix(strings.TrimSpace(l), "the client") {
			t.Errorf("server saw %q", l)
		}
		c.Close()
		// without a certificate: the server refuses
		c, err = tls.Dial("tcp", addr, &tls.Config{RootCAs: p.pool, ServerName: "localhost"})
		if err == nil {
			fmt.Fprintf(c, "who\n")
			_, err = bufio.NewReader(c).ReadString('\n')
			c.Close()
		}
		if err == nil || !strings.Contains(err.Error(), "certificate required") {
			t.Errorf("without a client certificate: %v", err)
		}
	})
}

// goTLSServer: Go's server answering as tlsd does
func goTLSServer(t *testing.T, cfg *tls.Config) string {
	ln, err := tls.Listen("tcp", "127.0.0.1:0", cfg)
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
				tc := c.(*tls.Conn)
				if err := tc.Handshake(); err != nil {
					return
				}
				st := tc.ConnectionState()
				r := bufio.NewReader(c)
				for {
					l, err := r.ReadString('\n')
					if err != nil {
						return
					}
					l = strings.TrimSuffix(l, "\n")
					var n int
					if _, err := fmt.Sscanf(l, "big %d", &n); err == nil {
						b := make([]byte, n+1)
						for i := 0; i < n; i++ {
							b[i] = byte('a' + i%26)
						}
						b[n] = '\n'
						c.Write(b)
						continue
					}
					cn := "-"
					if len(st.PeerCertificates) > 0 {
						cn = st.PeerCertificates[0].Subject.CommonName
					}
					proto := st.NegotiatedProtocol
					if proto == "" {
						proto = "-"
					}
					fmt.Fprintf(c, "%s | %s %s %s %s\n", l, goSuiteGroup(st), proto, st.ServerName, cn)
				}
			}(c)
		}
	}()
	return ln.Addr().String()
}

func runTLSC(t *testing.T, input string, args ...string) []string {
	t.Helper()
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "tlsc.c"), filepath.Join(tempDir(t), "tlsc"))
	cmd := exec.Command(exe, args...)
	cmd.Stdin = strings.NewReader(input)
	out, err := cmd.Output()
	if err != nil {
		t.Fatalf("tlsc %v: %v\n%s", args, err, out)
	}
	return strings.Split(strings.TrimSuffix(string(out), "\n"), "\n")
}

// TestTLSClientAgainstGo: the library's client against Go's server
func TestTLSClientAgainstGo(t *testing.T) {
	p := makePKI(t)
	for _, kind := range []string{"p256", "p384", "p521", "ed25519", "rsa"} {
		cert, err := tls.LoadX509KeyPair(p.leaves[kind][0], p.leaves[kind][1])
		if err != nil {
			t.Fatal(err)
		}
		addr := goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}, NextProtos: []string{"h2", "http/1.1"}})
		cases := []struct {
			args []string
			want string
		}{
			{nil, "ok 1301 001d - 1"},
			{[]string{"-suites", "1302"}, "ok 1302 001d - 1"},
			{[]string{"-suites", "1303"}, "ok 1303 001d - 1"},
			{[]string{"-groups", "17"}, "ok 1301 0017 - 1"},
			{[]string{"-groups", "18"}, "ok 1301 0018 - 1"},
			{[]string{"-groups", "19"}, "ok 1301 0019 - 1"},
			{[]string{"-groups", "17,1d"}, "ok 1301 0017 - 1"}, // Go takes the share it got (no retry)
			{[]string{"-alpn", "x,http/1.1"}, "ok 1301 001d http/1.1 1"},
			{[]string{"-keyupdate"}, "ok 1301 001d - 1"},
		}
		for _, c := range cases {
			args := append([]string{addr, "-name", "localhost", "-roots", p.caPEM}, c.args...)
			out := runTLSC(t, "hello\nbig 200000\nagain\n", args...)
			if len(out) != 4 || out[0] != c.want || !strings.HasPrefix(out[1], "hello | ") || out[2] != "big 200000 ok" ||
				!strings.HasPrefix(out[3], "again | ") {
				t.Errorf("%s %v: %q", kind, c.args, out)
			}
		}
	}
	cert, _ := tls.LoadX509KeyPair(p.leaves["p256"][0], p.leaves["p256"][1])
	addr := goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}})
	for _, c := range []struct {
		args []string
		want string
	}{
		{[]string{"-name", "wrong.example", "-roots", p.caPEM}, "error tls: failed to verify certificate: x509: certificate is valid for localhost, example.com, not wrong.example"},
		{[]string{"-name", "localhost"}, "error tls: failed to verify certificate: x509: certificate signed by unknown authority"},
		{[]string{"-name", "wrong.example", "-insecure"}, "ok 1301 001d - 0"},
	} {
		out := runTLSC(t, "", append([]string{addr}, c.args...)...)
		if out[0] != c.want {
			t.Errorf("%v:\n got %q\nwant %q", c.args, out[0], c.want)
		}
	}
	// a server asking for client certificates
	addr = goTLSServer(t, &tls.Config{Certificates: []tls.Certificate{cert}, ClientAuth: tls.RequireAndVerifyClientCert, ClientCAs: p.pool})
	out := runTLSC(t, "who\n", addr, "-name", "localhost", "-roots", p.caPEM, "-cert", p.clientC[0], p.clientC[1])
	if len(out) != 2 || !strings.HasSuffix(out[1], "the client") {
		t.Errorf("client certificate: %q", out)
	}
	out = runTLSC(t, "who\n", addr, "-name", "localhost", "-roots", p.caPEM)
	if j := strings.Join(out, " "); len(out) < 2 || !strings.Contains(j, "certificate required") && !strings.Contains(j, "error") {
		t.Errorf("without a client certificate: %q", out)
	}
	_ = io.EOF
}

// TestTLSSelf: the library's client against its own server (ChaCha20
// suites on both sides, client certificates, retries)
func TestTLSSelf(t *testing.T) {
	p := makePKI(t)
	addr := startTLSD(t, p.leaves["ed25519"][0], p.leaves["ed25519"][1], "-auth", "4", "-ca", p.caPEM, "-alpn", "h2")
	for _, c := range []struct {
		args []string
		want string
	}{
		{[]string{"-suites", "1303"}, "ok 1303 001d h2 1"},
		{[]string{"-suites", "1302,1301"}, "ok 1301 001d h2 1"}, /* the server's preference */
		{[]string{"-suites", "1302"}, "ok 1302 001d h2 1"},
		{[]string{"-groups", "18,1d"}, "ok 1301 001d h2 1"},
		{[]string{"-groups", "19", "-keyupdate"}, "ok 1301 0019 h2 1"},
	} {
		args := append([]string{addr, "-name", "localhost", "-roots", p.caPEM, "-alpn", "h2", "-cert", p.clientC[0], p.clientC[1]}, c.args...)
		out := runTLSC(t, "hello\nbig 1000000\nkeyupdate\nend\n", args...)
		if len(out) != 5 || out[0] != c.want || !strings.HasSuffix(out[1], "the client") || out[2] != "big 1000000 ok" ||
			!strings.HasPrefix(out[4], "end | ") {
			t.Errorf("%v: %q", c.args, out)
		}
	}
}

// TestHTTPS: Go's client against the library's HTTPS server, the
// library's client against Go's
func TestHTTPS(t *testing.T) {
	p := makePKI(t)
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "httpsd.c"), filepath.Join(tempDir(t), "httpsd"))
	cmd := exec.Command(exe, p.leaves["p256"][0], p.leaves["p256"][1])
	stdin, _ := cmd.StdinPipe()
	stdout, _ := cmd.StdoutPipe()
	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	defer func() { stdin.Close(); cmd.Wait() }()
	line, err := bufio.NewReader(stdout).ReadString('\n')
	if err != nil {
		t.Fatalf("httpsd: %v %s", err, stderr.String())
	}
	addr := strings.TrimSpace(strings.TrimPrefix(line, "listening "))
	tr := &http.Transport{TLSClientConfig: &tls.Config{RootCAs: p.pool, ServerName: "localhost"}}
	client := &http.Client{Transport: tr}
	var conns int
	trace := &httptrace.ClientTrace{GotConn: func(i httptrace.GotConnInfo) {
		if !i.Reused {
			conns++
		}
	}}
	for i := 0; i < 5; i++ {
		body := strings.Repeat("x", i*10000)
		req, _ := http.NewRequest("POST", "https://"+addr+"/p/"+fmt.Sprint(i), strings.NewReader(body))
		req = req.WithContext(httptrace.WithClientTrace(req.Context(), trace))
		resp, err := client.Do(req)
		if err != nil {
			t.Fatalf("POST: %v\n%s", err, stderr.String())
		}
		b, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		want := fmt.Sprintf("POST /p/%d HTTP/1.1 1301 - %d\n", i, len(body)) // no ALPN from Go without HTTP/2
		if string(b) != want || resp.TLS == nil {
			t.Errorf("got %q want %q", b, want)
		}
	}
	if conns != 1 {
		t.Errorf("%d connections for 5 requests (keep-alive over TLS)", conns)
	}
	// the library's client against Go's HTTPS server
	srv := httptest.NewTLSServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "%s %s %s %v", r.Method, r.URL.Path, r.Proto, r.TLS != nil)
	}))
	defer srv.Close()
	roots := filepath.Join(t.TempDir(), "roots.pem")
	writePEM(t, roots, "CERTIFICATE", srv.Certificate().Raw)
	fexe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "fetch.c"), filepath.Join(tempDir(t), "fetch"))
	fc := exec.Command(fexe, "gzip", "roots="+roots)
	u := strings.Replace(srv.URL, "127.0.0.1", "127.0.0.1", 1)
	fc.Stdin = strings.NewReader("GET\t" + u + "/a\nGET\t" + u + "/b\n")
	out, err := fc.Output()
	if err != nil {
		t.Fatalf("fetch: %v", err)
	}
	lines := strings.Split(strings.TrimSpace(string(out)), "\n")
	for i, path := range []string{"/a", "/b"} {
		want := fmt.Sprintf("200|0|%d||%x", len("GET "+path+" HTTP/1.1 true"), "GET "+path+" HTTP/1.1 true")
		if i >= len(lines) || lines[i] != want {
			t.Errorf("fetch %s: %q want %q", path, lines, want)
		}
	}
}
