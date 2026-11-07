package tests

import (
	"bytes"
	"crypto"
	"crypto/rand"
	"crypto/rsa"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/sha512"
	"crypto/x509"
	"encoding/hex"
	"fmt"
	"hash"
	mbig "math/big"
	mrand "math/rand"
	"strings"
	"testing"
)

// the library's hash numbers and Go's
var rsaHashes = []struct {
	id int
	h  func() hash.Hash
}{{1, sha1.New}, {3, sha256.New}, {4, sha512.New384}, {5, sha512.New}}

// TestRSAKeysAndEncryption: keys made by the library pass Go's checks;
// OAEP and PKCS #1 v1.5 encryption both ways; damaged ciphertexts are
// refused as Go refuses them
func TestRSAKeysAndEncryption(t *testing.T) {
	var in []string
	for _, bits := range []int{1024, 1024, 2048, 2048, 3072, 4096} {
		in = append(in, fmt.Sprintf("gen\t%d", bits))
	}
	out := diffRun(t, "rsa", in)
	var keys []*rsa.PrivateKey
	for i, l := range out {
		f := strings.Fields(l)
		if len(f) != 2 {
			t.Fatalf("gen: %q", l)
		}
		p1, _ := hex.DecodeString(f[0])
		p8, _ := hex.DecodeString(f[1])
		k, err := x509.ParsePKCS1PrivateKey(p1)
		if err != nil {
			t.Fatalf("PKCS #1: %v", err)
		}
		k8, err := x509.ParsePKCS8PrivateKey(p8)
		if err != nil || !k.Equal(k8) {
			t.Fatalf("PKCS #8: %v", err)
		}
		if err := k.Validate(); err != nil {
			t.Errorf("Validate: %v", err)
		}
		bits := []int{1024, 1024, 2048, 2048, 3072, 4096}[i]
		if k.N.BitLen() != bits || k.E != 65537 || len(k.Primes) != 2 {
			t.Errorf("key %d: %d bits, e %d", i, k.N.BitLen(), k.E)
		}
		if !bytes.Equal(x509.MarshalPKCS1PrivateKey(k), p1) {
			t.Errorf("PKCS #1 encoding differs from Go's")
		}
		h := sha256.Sum256([]byte("message"))
		sig, err := rsa.SignPSS(rand.Reader, k, crypto.SHA256, h[:], nil)
		if err != nil || rsa.VerifyPSS(&k.PublicKey, crypto.SHA256, h[:], sig, nil) != nil {
			t.Errorf("Go cannot sign with the key: %v", err)
		}
		keys = append(keys, k)
	}
	r := mrand.New(mrand.NewSource(8017))
	in = nil
	var want []string
	var checks []func(string) error
	for _, k := range keys[:4] {
		kh := hex.EncodeToString(x509.MarshalPKCS1PrivateKey(k))
		size := k.Size()
		for _, hh := range rsaHashes {
			hl := hh.h().Size()
			if size < 2*hl+2 {
				continue
			}
			for _, mlen := range []int{0, 1, 17, size - 2*hl - 2, size - 2*hl - 1} {
				msg := randBytes(r, mlen)
				label := randBytes(r, r.Intn(3)*7)
				// ours encrypted, Go decrypts
				k, hh, msg, label := k, hh, msg, label
				in = append(in, fmt.Sprintf("oenc\t%s\t%d\t%x\t%x", kh, hh.id, label, msg))
				if mlen > size-2*hl-2 {
					want = append(want, "error too long")
					checks = append(checks, nil)
					continue
				}
				want = append(want, "")
				checks = append(checks, func(got string) error {
					ct, _ := hex.DecodeString(got)
					pt, err := rsa.DecryptOAEP(hh.h(), nil, k, ct, label)
					if err != nil || !bytes.Equal(pt, msg) {
						return fmt.Errorf("Go's DecryptOAEP: %v", err)
					}
					return nil
				})
				// Go encrypted, ours decrypts: as it is and damaged
				ct, err := rsa.EncryptOAEP(hh.h(), rand.Reader, &k.PublicKey, msg, label)
				if err != nil {
					t.Fatal(err)
				}
				for v := 0; v < 4; v++ {
					c := append([]byte{}, ct...)
					lb := label
					switch v {
					case 1:
						c[r.Intn(len(c))] ^= byte(1 << r.Intn(8))
					case 2:
						lb = append(append([]byte{}, label...), 'x')
					case 3:
						c = c[1:]
					}
					in = append(in, fmt.Sprintf("odec\t%s\t%d\t%x\t%x", kh, hh.id, lb, c))
					pt, err := rsa.DecryptOAEP(hh.h(), nil, k, c, lb)
					if err != nil {
						want = append(want, "error")
					} else {
						want = append(want, "ok "+hex.EncodeToString(pt))
					}
					checks = append(checks, nil)
				}
			}
		}
		for _, mlen := range []int{0, 1, 50, size - 11, size - 10} {
			msg := randBytes(r, mlen)
			k, msg := k, msg
			in = append(in, fmt.Sprintf("penc\t%s\t%x", kh, msg))
			if mlen > size-11 {
				want = append(want, "error too long")
				checks = append(checks, nil)
				continue
			}
			want = append(want, "")
			checks = append(checks, func(got string) error {
				ct, _ := hex.DecodeString(got)
				pt, err := rsa.DecryptPKCS1v15(nil, k, ct)
				if err != nil || !bytes.Equal(pt, msg) {
					return fmt.Errorf("Go's DecryptPKCS1v15: %v", err)
				}
				return nil
			})
			ct, _ := rsa.EncryptPKCS1v15(rand.Reader, &k.PublicKey, msg)
			for v := 0; v < 3; v++ {
				c := append([]byte{}, ct...)
				if v == 1 {
					c[r.Intn(len(c))] ^= 0x10
				}
				if v == 2 {
					// a valid block with a short padding: 0 2 <7 bytes> 0 ...
					em := make([]byte, size)
					em[1] = 2
					for j := 2; j < 9; j++ {
						em[j] = 0x55
					}
					c = new(bigInt).SetBytes(em).encrypt(&k.PublicKey)
				}
				in = append(in, fmt.Sprintf("pdec\t%s\t%x", kh, c))
				pt, err := rsa.DecryptPKCS1v15(nil, k, c)
				if err != nil {
					want = append(want, "error")
				} else {
					want = append(want, "ok "+hex.EncodeToString(pt))
				}
				checks = append(checks, nil)
			}
		}
	}
	got := diffRun(t, "rsa", in)
	if len(got) != len(in) {
		t.Fatalf("%d answers for %d requests", len(got), len(in))
	}
	bad := 0
	for i := range in {
		var err error
		if checks[i] != nil {
			err = checks[i](got[i])
		} else if got[i] != want[i] {
			err = fmt.Errorf("got %q want %q", got[i], want[i])
		}
		if err != nil {
			if bad++; bad < 10 {
				t.Errorf("%.60s: %v", in[i], err)
			}
		}
	}
}

type bigInt struct{ b []byte }

func (x *bigInt) SetBytes(b []byte) *bigInt { x.b = b; return x }

// encrypt: the raw RSA operation (x^e mod n) on the block
func (x *bigInt) encrypt(k *rsa.PublicKey) []byte {
	m := new(mbig.Int).SetBytes(x.b)
	c := new(mbig.Int).Exp(m, mbig.NewInt(int64(k.E)), k.N)
	return c.FillBytes(make([]byte, k.Size()))
}
