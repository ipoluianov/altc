package tests

import (
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/asn1"
	"encoding/hex"
	"fmt"
	"math/big"
	mrand "math/rand"
	"strings"
	"testing"
	"time"
)

// TestX509PoliciesAgainstGo verifies random chains with certificate
// policies, policy mappings, policy constraints and inhibitAnyPolicy
// (with and without the user's acceptable policies) as Go's Verify does.
func TestX509PoliciesAgainstGo(t *testing.T) {
	r := mrand.New(mrand.NewSource(71))
	keys := x509Keys()
	now := time.Date(2026, 6, 1, 12, 0, 0, 0, time.UTC)
	nb, na := now.Add(-24*time.Hour), now.Add(365*24*time.Hour)
	mustOID := func(arcs ...uint64) x509.OID {
		o, err := x509.OIDFromInts(arcs)
		if err != nil {
			t.Fatal(err)
		}
		return o
	}
	any := mustOID(2, 5, 29, 32, 0)
	pool := []x509.OID{any, mustOID(1, 2, 3, 1), mustOID(1, 2, 3, 2), mustOID(1, 2, 3, 3), mustOID(1, 2, 3, 4294967296)}
	der := func(o x509.OID) []byte {
		b, _ := o.MarshalBinary()
		return b
	}
	tlv := func(tag byte, content []byte) []byte {
		if len(content) >= 128 {
			t.Fatal("long content")
		}
		return append([]byte{tag, byte(len(content))}, content...)
	}
	// a weighted pick: mostly the non-any policies
	pick := func() x509.OID {
		if r.Intn(5) == 0 {
			return any
		}
		return pool[1+r.Intn(len(pool)-1)]
	}
	policies := func() []x509.OID {
		var ps []x509.OID
		seen := map[string]bool{}
		for n := r.Intn(4); len(ps) < n; {
			p := pick()
			if !seen[p.String()] {
				seen[p.String()] = true
				ps = append(ps, p)
			}
		}
		return ps
	}
	serial := int64(1000)
	extras := func(c *x509.Certificate, leaf bool) {
		c.Policies = policies()
		if r.Intn(5) == 0 && len(c.Policies) == 0 {
			c.Policies = []x509.OID{any}
		}
		if !leaf && r.Intn(3) == 0 {
			var maps []byte
			for k := 1 + r.Intn(3); k > 0; k-- {
				is, su := pick(), pick()
				if r.Intn(4) != 0 {
					// mostly valid mappings (anyPolicy may not be mapped)
					for is.Equal(any) {
						is = pick()
					}
					for su.Equal(any) {
						su = pick()
					}
				}
				maps = append(maps, tlv(0x30, append(tlv(0x06, der(is)), tlv(0x06, der(su))...))...)
			}
			c.ExtraExtensions = append(c.ExtraExtensions, pkix.Extension{Id: asn1.ObjectIdentifier{2, 5, 29, 33}, Critical: true, Value: tlv(0x30, maps)})
		}
		if r.Intn(3) == 0 {
			var pc []byte
			if r.Intn(2) == 0 {
				pc = append(pc, tlv(0x80, []byte{byte(r.Intn(3))})...)
			}
			if r.Intn(2) == 0 {
				pc = append(pc, tlv(0x81, []byte{byte(r.Intn(3))})...)
			}
			c.ExtraExtensions = append(c.ExtraExtensions, pkix.Extension{Id: asn1.ObjectIdentifier{2, 5, 29, 36}, Critical: true, Value: tlv(0x30, pc)})
		}
		if !leaf && r.Intn(4) == 0 {
			c.ExtraExtensions = append(c.ExtraExtensions, pkix.Extension{Id: asn1.ObjectIdentifier{2, 5, 29, 54}, Critical: true,
				Value: tlv(0x02, []byte{byte(r.Intn(3))})})
		}
	}
	tmpl := func(cn string, ca bool) *x509.Certificate {
		serial++
		c := &x509.Certificate{SerialNumber: big.NewInt(serial), Subject: pkix.Name{CommonName: cn}, NotBefore: nb, NotAfter: na,
			BasicConstraintsValid: ca, IsCA: ca, KeyUsage: x509.KeyUsageDigitalSignature}
		if ca {
			c.KeyUsage |= x509.KeyUsageCertSign
		}
		return c
	}
	userSets := [][]x509.OID{nil, {pool[1]}, {pool[2], pool[3]}, {any}, {any, pool[1]}, {pool[4]}}
	var in, want []string
	counts := map[string]int{}
	for i := 0; i < 1500; i++ {
		rt := tmpl(fmt.Sprintf("Root %d", i), true)
		if r.Intn(2) == 0 {
			rt.Policies = policies()
		}
		root := issue(t, rt, keys[r.Intn(len(keys))], nil)
		parent := root
		var inters []*testCA
		for d := r.Intn(4); d > 0; d-- {
			name := fmt.Sprintf("Inter %d.%d", i, d)
			// a self-issued intermediate: the same name as its issuer
			if r.Intn(6) == 0 {
				name = parent.cert.Subject.CommonName
			}
			ct := tmpl(name, true)
			extras(ct, false)
			ic := issue(t, ct, keys[r.Intn(len(keys))], parent)
			inters = append(inters, ic)
			parent = ic
		}
		lt := tmpl(fmt.Sprintf("Leaf %d", i), false)
		extras(lt, true)
		leaf := issue(t, lt, keys[r.Intn(len(keys))], parent)
		user := userSets[r.Intn(len(userSets))]
		rp, ip := x509.NewCertPool(), x509.NewCertPool()
		rp.AddCert(root.cert)
		var ih, ph []string
		for _, x := range inters {
			ip.AddCert(x.cert)
			ih = append(ih, hex.EncodeToString(x.der))
		}
		for _, p := range user {
			ph = append(ph, hex.EncodeToString(der(p)))
		}
		in = append(in, fmt.Sprintf("%d\t\t\t%s\t%s\t%s\t%s", now.Unix(), hex.EncodeToString(root.der), strings.Join(ih, ","),
			hex.EncodeToString(leaf.der), strings.Join(ph, ",")))
		chains, err := leaf.cert.Verify(x509.VerifyOptions{Roots: rp, Intermediates: ip, CurrentTime: now, CertificatePolicies: user})
		if err != nil {
			want = append(want, "error "+err.Error())
			counts["error"]++
			counts[err.Error()]++
			continue
		}
		counts["ok"]++
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
	t.Logf("Go: %v", counts)
	if counts["ok"] < 300 || counts["error"] < 300 {
		t.Errorf("unbalanced cases: %v", counts)
	}
	got := diffRun(t, "x509v", in)
	bad := 0
	for i := range got {
		if got[i] != want[i] {
			bad++
			if bad <= 15 {
				t.Errorf("case %d:\n got %s\nwant %s", i, got[i], want[i])
			}
		}
	}
	if bad > 0 {
		t.Errorf("%d of %d differ", bad, len(got))
	}
}
