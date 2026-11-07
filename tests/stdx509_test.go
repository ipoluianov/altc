package tests

import (
	"crypto"
	"crypto/ecdsa"
	"crypto/ed25519"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/asn1"
	"encoding/hex"
	"fmt"
	mrand "math/rand"
	"net"
	"math/big"
	"net/url"
	"os"
	"strings"
	"sync"
	"testing"
	"time"
)

var testKeysOnce sync.Once
var testKeys []crypto.Signer

// keys of every kind (made once: RSA generation is slow)
func x509Keys() []crypto.Signer {
	testKeysOnce.Do(func() {
		r1024, _ := rsa.GenerateKey(rand.Reader, 1024)
		r2048, _ := rsa.GenerateKey(rand.Reader, 2048)
		p256, _ := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
		p384, _ := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
		p521, _ := ecdsa.GenerateKey(elliptic.P521(), rand.Reader)
		_, ed, _ := ed25519.GenerateKey(rand.Reader)
		testKeys = []crypto.Signer{r1024, r2048, p256, p384, p521, ed}
	})
	return testKeys
}

func goCertSummary(der []byte) string {
	c, err := x509.ParseCertificate(der)
	if err != nil {
		return "error " + err.Error()
	}
	var b strings.Builder
	fmt.Fprintf(&b, "ok|v=%d|serial=%x", c.Version, c.SerialNumber.Bytes())
	if c.SerialNumber.Sign() == 0 {
		b.WriteString("00")
	}
	fmt.Fprintf(&b, "|sig=%d|pk=%d", c.SignatureAlgorithm, c.PublicKeyAlgorithm)
	names := func(label string, n pkix.Name) {
		fmt.Fprintf(&b, "|%s=", label)
		for i, a := range n.Names {
			if i > 0 {
				b.WriteByte(';')
			}
			if s, ok := a.Value.(string); ok {
				fmt.Fprintf(&b, "%s=%x", a.Type, s)
			} else {
				fmt.Fprintf(&b, "%s=?", a.Type)
			}
		}
	}
	names("iss", c.Issuer)
	names("sub", c.Subject)
	fmt.Fprintf(&b, "|nb=%d|na=%d|key=", c.NotBefore.Unix(), c.NotAfter.Unix())
	switch k := c.PublicKey.(type) {
	case *rsa.PublicKey:
		fmt.Fprintf(&b, "%x,%d", k.N.Bytes(), k.E)
	case *ecdsa.PublicKey:
		curve := map[string]int{"P-256": 1, "P-384": 2, "P-521": 3}[k.Curve.Params().Name]
		pb, _ := k.Bytes()
		fmt.Fprintf(&b, "%d,%x", curve, pb)
	case ed25519.PublicKey:
		fmt.Fprintf(&b, "%x", []byte(k))
	}
	mpl := c.MaxPathLen
	if !c.BasicConstraintsValid {
		mpl = -1
	}
	fmt.Fprintf(&b, "|ku=%d|bc=%d,%d,%d,%d|skid=%x|akid=%x", c.KeyUsage, b2i(c.BasicConstraintsValid), b2i(c.IsCA), mpl,
		b2i(c.MaxPathLenZero), c.SubjectKeyId, c.AuthorityKeyId)
	list := func(label string, s []string) { fmt.Fprintf(&b, "|%s=%s", label, strings.Join(s, ",")) }
	hlist := func(label string, s []string) {
		var h []string
		for _, x := range s {
			h = append(h, hex.EncodeToString([]byte(x)))
		}
		list(label, h)
	}
	hlist("dns", c.DNSNames)
	hlist("em", c.EmailAddresses)
	var ips, uris, ueku, pol, uc []string
	for _, ip := range c.IPAddresses {
		ips = append(ips, hex.EncodeToString(ip))
	}
	list("ip", ips)
	for _, u := range c.URIs {
		uris = append(uris, u.String())
	}
	hlist("uri", uris)
	var ekus []string
	for _, e := range c.ExtKeyUsage {
		ekus = append(ekus, fmt.Sprint(int(e)))
	}
	list("eku", ekus)
	for _, o := range c.UnknownExtKeyUsage {
		ueku = append(ueku, o.String())
	}
	list("ueku", ueku)
	for _, o := range c.Policies {
		pol = append(pol, o.String())
	}
	list("pol", pol)
	for _, o := range c.UnhandledCriticalExtensions {
		uc = append(uc, o.String())
	}
	list("uc", uc)
	hlist("crl", c.CRLDistributionPoints)
	hlist("ocsp", c.OCSPServer)
	hlist("issurl", c.IssuingCertificateURL)
	hlist("pdns", c.PermittedDNSDomains)
	hlist("edns", c.ExcludedDNSDomains)
	nets := func(label string, n []*net.IPNet) {
		var s []string
		for _, x := range n {
			s = append(s, hex.EncodeToString(x.IP)+"/"+hex.EncodeToString(x.Mask))
		}
		list(label, s)
	}
	nets("pip", c.PermittedIPRanges)
	nets("eip", c.ExcludedIPRanges)
	hlist("pem", c.PermittedEmailAddresses)
	hlist("eem", c.ExcludedEmailAddresses)
	hlist("puri", c.PermittedURIDomains)
	hlist("euri", c.ExcludedURIDomains)
	fmt.Fprintf(&b, "|pcrit=%d|sigv=%x", b2i(c.PermittedDNSDomainsCritical), c.Signature)
	return b.String()
}

// randomCert makes a certificate with a random selection of Go's fields
func randomCert(r *mrand.Rand) []byte {
	keys := x509Keys()
	key, issuer := keys[r.Intn(len(keys))], keys[r.Intn(len(keys))]
	words := []string{"Example", "Тест", "a.b", "x y", "Ünïcode", "plain"}
	w := func() string { return words[r.Intn(len(words))] }
	tmpl := &x509.Certificate{
		SerialNumber: newSerial(r),
		Subject: pkix.Name{CommonName: w(), Organization: []string{w()}, Country: []string{"DE"},
			OrganizationalUnit: []string{w(), w()}, Locality: []string{w()}},
		NotBefore: time.Unix(r.Int63n(4e9), 0).UTC(),
		NotAfter:  time.Unix(r.Int63n(4e9)+4e9, 0).UTC(),
	}
	if r.Intn(5) == 0 {
		tmpl.NotAfter = time.Date(9999, 12, 31, 23, 59, 59, 0, time.UTC)
	}
	if r.Intn(2) == 0 {
		tmpl.DNSNames = []string{"example.com", "*.example.org", "xn--e1afmkfd.xn--p1ai"}[:1+r.Intn(3)]
	}
	if r.Intn(3) == 0 {
		tmpl.IPAddresses = []net.IP{net.ParseIP("127.0.0.1"), net.ParseIP("2001:db8::1")}
	}
	if r.Intn(3) == 0 {
		tmpl.EmailAddresses = []string{"a@example.com"}
	}
	if r.Intn(3) == 0 {
		u, _ := url.Parse("https://example.com/path?q=1")
		tmpl.URIs = []*url.URL{u}
	}
	if r.Intn(2) == 0 {
		tmpl.BasicConstraintsValid = true
		tmpl.IsCA = r.Intn(2) == 0
		if tmpl.IsCA {
			tmpl.MaxPathLen = r.Intn(3) - 1
			tmpl.MaxPathLenZero = tmpl.MaxPathLen == 0
		}
	}
	tmpl.KeyUsage = x509.KeyUsage(r.Intn(512))
	if r.Intn(2) == 0 {
		tmpl.ExtKeyUsage = []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth, x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageCodeSigning}[:1+r.Intn(3)]
		tmpl.UnknownExtKeyUsage = []asn1.ObjectIdentifier{{1, 2, 3, 4}}
	}
	if r.Intn(3) == 0 {
		tmpl.PermittedDNSDomains = []string{".example.com", "example.net"}
		tmpl.ExcludedDNSDomains = []string{"bad.example.com"}
		_, n1, _ := net.ParseCIDR("10.0.0.0/8")
		_, n2, _ := net.ParseCIDR("2001:db8::/32")
		tmpl.PermittedIPRanges = []*net.IPNet{n1, n2}
		tmpl.PermittedEmailAddresses = []string{"example.com", "a@b.example"}
		tmpl.ExcludedURIDomains = []string{".bad.example"}
		tmpl.PermittedDNSDomainsCritical = r.Intn(2) == 0
	}
	if r.Intn(3) == 0 {
		tmpl.OCSPServer = []string{"http://ocsp.example.com"}
		tmpl.IssuingCertificateURL = []string{"http://ca.example.com/ca.crt"}
		tmpl.CRLDistributionPoints = []string{"http://crl.example.com/ca.crl"}
	}
	if r.Intn(3) == 0 {
		tmpl.Policies = []x509.OID{mustOID("1.2.3.4.5"), mustOID("2.23.140.1.2.1")}
	}
	if r.Intn(4) == 0 {
		tmpl.ExtraExtensions = []pkix.Extension{{Id: asn1.ObjectIdentifier{1, 2, 3, 99}, Critical: r.Intn(2) == 0, Value: []byte{5, 0}}}
	}
	if r.Intn(4) == 0 {
		tmpl.SubjectKeyId = []byte{1, 2, 3, 4}
	}
	parent := tmpl
	if r.Intn(2) == 0 {
		parent = &x509.Certificate{SerialNumber: newSerial(r), Subject: pkix.Name{CommonName: "Issuer CA"}, SubjectKeyId: []byte{9, 9}}
	}
	if rk, ok := issuer.(*rsa.PrivateKey); ok && r.Intn(2) == 0 {
		_ = rk
		tmpl.SignatureAlgorithm = x509.SHA256WithRSAPSS + x509.SignatureAlgorithm(r.Intn(3))
	}
	der, err := x509.CreateCertificate(rand.Reader, tmpl, parent, key.Public(), issuer)
	if err != nil {
		// combinations Go refuses to make (a PSS signature with a non-RSA key...)
		tmpl.SignatureAlgorithm = 0
		if der, err = x509.CreateCertificate(rand.Reader, tmpl, parent, key.Public(), issuer); err != nil {
			panic(err)
		}
	}
	return der
}

func mustOID(s string) x509.OID {
	o, err := x509.ParseOID(s)
	if err != nil {
		panic(err)
	}
	return o
}

func newSerial(r *mrand.Rand) *big.Int {
	return new(big.Int).SetBytes(randBytes(r, 1+r.Intn(20)))
}

// normalized error texts: some of Go's carry details the C code leaves out
func x509Norm(s string) string {
	for _, p := range []string{"error x509: cannot parse URI", "error x509: invalid constraint value", "error x509: IP constraint contained invalid mask"} {
		if strings.HasPrefix(s, p) {
			return p
		}
	}
	return s
}

func TestX509ParseAgainstGo(t *testing.T) {
	r := mrand.New(mrand.NewSource(41))
	var in, want []string
	for i := 0; i < 150; i++ {
		der := randomCert(r)
		in, want = append(in, hex.EncodeToString(der)), append(want, goCertSummary(der))
		for k := 0; k < 25; k++ {
			m := append([]byte(nil), der...)
			switch k % 3 {
			case 0:
				m[r.Intn(len(m))] ^= 1 << r.Intn(8)
			case 1:
				m[r.Intn(len(m))] = byte(r.Intn(256))
			case 2:
				m = m[:r.Intn(len(m))]
			}
			in, want = append(in, hex.EncodeToString(m)), append(want, goCertSummary(m))
		}
	}
	got := diffRun(t, "x509", in)
	bad := 0
	for i := range got {
		g, w := x509Norm(got[i]), x509Norm(want[i])
		if g == w {
			continue
		}
		bad++
		if bad > 15 {
			continue
		}
		// the first differing field
		gf, wf := strings.Split(g, "|"), strings.Split(w, "|")
		k := 0
		for k < len(gf) && k < len(wf) && gf[k] == wf[k] {
			k++
		}
		at := func(f []string) string {
			if k < len(f) {
				return f[k]
			}
			return ""
		}
		t.Errorf("cert %d: got %.200q want %.200q", i, at(gf), at(wf))
		if out := os.Getenv("X509_OUT"); out != "" {
			os.WriteFile(fmt.Sprintf("%s.%d.der", out, bad), mustHex(in[i]), 0o644)
		}
	}
	if bad > 0 {
		t.Errorf("%d differences in %d", bad, len(got))
	}
}


func mustHex(s string) []byte {
	b, _ := hex.DecodeString(s)
	return b
}

type testCA struct {
	cert *x509.Certificate
	der  []byte
	key  crypto.Signer
}

// issue makes a certificate from tmpl signed by parent (self-signed when nil)
func issue(t *testing.T, tmpl *x509.Certificate, key crypto.Signer, parent *testCA) *testCA {
	t.Helper()
	p, pk := tmpl, key
	if parent != nil {
		p, pk = parent.cert, parent.key
	}
	der, err := x509.CreateCertificate(rand.Reader, tmpl, p, key.Public(), pk)
	if err != nil {
		t.Fatalf("%s: %v", tmpl.Subject.CommonName, err)
	}
	c, err := x509.ParseCertificate(der)
	if err != nil {
		t.Fatal(err)
	}
	return &testCA{c, der, key}
}

func TestX509VerifyAgainstGo(t *testing.T) {
	r := mrand.New(mrand.NewSource(43))
	keys := x509Keys()
	now := time.Date(2026, 6, 1, 12, 0, 0, 0, time.UTC)
	nb, na := now.Add(-24*time.Hour), now.Add(365*24*time.Hour)
	serial := int64(1)
	tmpl := func(cn string, ca bool) *x509.Certificate {
		serial++
		c := &x509.Certificate{SerialNumber: big.NewInt(serial), Subject: pkix.Name{CommonName: cn}, NotBefore: nb, NotAfter: na,
			BasicConstraintsValid: ca, IsCA: ca, KeyUsage: x509.KeyUsageDigitalSignature}
		if ca {
			c.KeyUsage |= x509.KeyUsageCertSign
		}
		return c
	}
	key := func() crypto.Signer { return keys[r.Intn(len(keys))] }
	root := issue(t, tmpl("Root A", true), keys[2], nil)
	root2 := issue(t, tmpl("Root B", true), keys[5], nil)
	// the same subject as Root A with another key: a cross-signing candidate
	rootA2t := tmpl("Root A", true)
	rootA2 := issue(t, rootA2t, keys[1], nil)
	var inters []*testCA
	interT := []func(*x509.Certificate){
		func(c *x509.Certificate) {},
		func(c *x509.Certificate) { c.MaxPathLen, c.MaxPathLenZero = 0, true },
		func(c *x509.Certificate) { c.ExtKeyUsage = []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth} },
		func(c *x509.Certificate) { c.ExtKeyUsage = []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth} },
		func(c *x509.Certificate) {
			c.PermittedDNSDomains = []string{".example.com", "example.com"}
			c.ExcludedDNSDomains = []string{"bad.example.com"}
			_, n, _ := net.ParseCIDR("127.0.0.0/8")
			c.PermittedIPRanges = []*net.IPNet{n}
			c.PermittedEmailAddresses = []string{"example.com"}
		},
		func(c *x509.Certificate) { c.NotAfter = now.Add(-time.Hour) },
		func(c *x509.Certificate) { c.KeyUsage = x509.KeyUsageDigitalSignature },
		func(c *x509.Certificate) { c.IsCA = false; c.KeyUsage = x509.KeyUsageDigitalSignature | x509.KeyUsageCertSign },
		func(c *x509.Certificate) { c.ExcludedDNSDomains = []string{"example.org"} },
	}
	for i, f := range interT {
		c := tmpl(fmt.Sprintf("Inter %d", i), true)
		f(c)
		parent := root
		if i%3 == 2 {
			parent = root2
		}
		inters = append(inters, issue(t, c, key(), parent))
	}
	// an intermediate below an intermediate, one signed by the other Root A
	deep := issue(t, tmpl("Inter deep", true), key(), inters[0])
	cross := issue(t, tmpl("Inter cross", true), key(), rootA2)
	inters = append(inters, deep, cross)
	hosts := []string{"", "www.example.com", "a.b.example.com", "bad.example.com", "127.0.0.1", "example.org", "WWW.EXAMPLE.COM.",
		"10.0.0.1", "::1", "[::1]", "other.net", "xn--e1afmkfd.xn--p1ai"}
	var in, want []string
	for i := 0; i < 300; i++ {
		lt := tmpl(fmt.Sprintf("Leaf %d", i), false)
		switch r.Intn(5) {
		case 0:
			lt.DNSNames = []string{"www.example.com", "*.example.com"}
		case 1:
			lt.DNSNames = []string{"bad.example.com", "example.org"}
		case 2:
			lt.IPAddresses = []net.IP{net.ParseIP("127.0.0.1"), net.ParseIP("::1")}
		case 3:
			lt.DNSNames = []string{"*.b.example.com", "xn--e1afmkfd.xn--p1ai"}
			lt.EmailAddresses = []string{"user@example.com"}
		}
		if r.Intn(4) == 0 {
			lt.ExtKeyUsage = []x509.ExtKeyUsage{[]x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth, x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageAny}[r.Intn(3)]}
		}
		if r.Intn(10) == 0 {
			lt.NotBefore = now.Add(time.Hour)
		}
		var parent *testCA
		switch r.Intn(4) {
		case 0:
			parent = root
		case 1:
			parent = root2
		default:
			parent = inters[r.Intn(len(inters))]
		}
		leaf := issue(t, lt, key(), parent)
		// pools: some roots, some intermediates
		rootSet := []*testCA{root, root2, rootA2}[:1+r.Intn(3)]
		var interSet []*testCA
		for _, x := range inters {
			if r.Intn(3) > 0 {
				interSet = append(interSet, x)
			}
		}
		host := hosts[r.Intn(len(hosts))]
		var eku []x509.ExtKeyUsage
		switch r.Intn(4) {
		case 1:
			eku = []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth}
		case 2:
			eku = []x509.ExtKeyUsage{x509.ExtKeyUsageAny}
		case 3:
			eku = []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth, x509.ExtKeyUsageClientAuth}
		}
		rp, ip := x509.NewCertPool(), x509.NewCertPool()
		var rh, ih, es []string
		for _, x := range rootSet {
			rp.AddCert(x.cert)
			rh = append(rh, hex.EncodeToString(x.der))
		}
		for _, x := range interSet {
			ip.AddCert(x.cert)
			ih = append(ih, hex.EncodeToString(x.der))
		}
		for _, e := range eku {
			es = append(es, fmt.Sprint(int(e)))
		}
		in = append(in, fmt.Sprintf("%d\t%s\t%s\t%s\t%s\t%s", now.Unix(), host, strings.Join(es, ","), strings.Join(rh, ","),
			strings.Join(ih, ","), hex.EncodeToString(leaf.der)))
		chains, err := leaf.cert.Verify(x509.VerifyOptions{DNSName: host, Roots: rp, Intermediates: ip, CurrentTime: now, KeyUsages: eku})
		if err != nil {
			want = append(want, "error "+err.Error())
			continue
		}
		var cs []string
		for _, ch := range chains {
			var names []string
			for _, c := range ch {
				names = append(names, c.Subject.CommonName)
			}
			cs = append(cs, strings.Join(names, ">"))
		}
		want = append(want, "chains:"+strings.Join(cs, ";"))
	}
	got := diffRun(t, "x509v", in)
	bad := 0
	for i := range got {
		if got[i] != want[i] {
			bad++
			if bad <= 15 {
				f := strings.Split(in[i], "\t")
				t.Errorf("host %q eku %q:\n got %s\nwant %s", f[1], f[2], got[i], want[i])
			}
		}
	}
	if bad > 0 {
		t.Errorf("%d of %d differ", bad, len(got))
	}
}
