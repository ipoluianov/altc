package tests

import (
	"bytes"
	"crypto"
	"crypto/aes"
	"crypto/cipher"
	"crypto/ecdh"
	"crypto/ecdsa"
	"crypto/elliptic"
	crand "crypto/rand"
	"crypto/ed25519"
	"crypto/hkdf"
	"crypto/md5"
	"crypto/pbkdf2"
	"crypto/rsa"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/sha512"
	"encoding/hex"
	"fmt"
	"hash"
	"math/rand"
	"os/exec"
	"strings"
	"testing"
)

func randBytes(r *rand.Rand, n int) []byte {
	b := make([]byte, n)
	r.Read(b)
	return b
}

func hexs(bs ...[]byte) string {
	var s []string
	for _, b := range bs {
		s = append(s, hex.EncodeToString(b))
	}
	return strings.Join(s, "\t")
}

func TestDiffAES(t *testing.T) {
	r := rand.New(rand.NewSource(31))
	var in, want []string
	for i := 0; i < 600; i++ {
		key := randBytes(r, []int{16, 24, 32}[i%3])
		blk, _ := aes.NewCipher(key)
		data := randBytes(r, r.Intn(300))
		switch i % 5 {
		case 0:
			data = data[:len(data)/16*16]
			out := make([]byte, len(data))
			for k := 0; k < len(data); k += 16 {
				blk.Encrypt(out[k:], data[k:])
			}
			in, want = append(in, "A\t"+hexs(key, data)), append(want, hex.EncodeToString(out))
		case 1:
			iv := randBytes(r, 16)
			if i%10 == 1 {
				for k := 8; k < 16; k++ {
					iv[k] = 0xff // the counter carries across bytes
				}
			}
			out := make([]byte, len(data))
			cipher.NewCTR(blk, iv).XORKeyStream(out, data)
			in, want = append(in, "C\t"+hexs(key, iv, data)), append(want, hex.EncodeToString(out))
		case 2:
			iv := randBytes(r, 16)
			m := data[:len(data)/16*16]
			out := make([]byte, len(m))
			cipher.NewCBCEncrypter(blk, iv).CryptBlocks(out, m)
			in, want = append(in, "B\t"+hexs(key, iv, data)), append(want, hex.EncodeToString(out))
		case 3, 4:
			nn := 12
			if i%7 == 0 {
				nn = 1 + r.Intn(40)
			}
			nonce, ad := randBytes(r, nn), randBytes(r, r.Intn(70))
			g, _ := cipher.NewGCMWithNonceSize(blk, nn)
			ct := g.Seal(nil, nonce, data, ad)
			if i%5 == 3 {
				in, want = append(in, "G\t"+hexs(key, nonce, ad, data)), append(want, hex.EncodeToString(ct))
				continue
			}
			// open: valid, or with a damaged byte anywhere
			if r.Intn(2) == 0 {
				k := r.Intn(len(ct) + len(ad) + len(nonce))
				switch {
				case k < len(ct):
					ct[k] ^= 1 << r.Intn(8)
				case k < len(ct)+len(ad):
					ad[k-len(ct)] ^= 1
				default:
					nonce[k-len(ct)-len(ad)] ^= 1
				}
			}
			pt, err := g.Open(nil, nonce, ct, ad)
			w := "auth"
			if err == nil {
				w = "ok " + hex.EncodeToString(pt)
			}
			in, want = append(in, "O\t"+hexs(key, nonce, ad, ct)), append(want, w)
		}
	}
	in, want = append(in, "A\t"+hexs(randBytes(r, 17), nil)), append(want, "badkey")
	diffCompare(t, in, diffRun(t, "crypto", in), want)
}

var hashKinds = []func() hash.Hash{md5.New, sha1.New, sha256.New224, sha256.New, sha512.New384, sha512.New}

func TestDiffKDF(t *testing.T) {
	r := rand.New(rand.NewSource(32))
	var in, want []string
	for i := 0; i < 300; i++ {
		k := r.Intn(len(hashKinds))
		salt, ikm, info := randBytes(r, r.Intn(3)*r.Intn(80)), randBytes(r, r.Intn(100)), randBytes(r, r.Intn(60))
		n := 1 + r.Intn(300)
		in = append(in, fmt.Sprintf("K\t%d\t%s\t%d", k, hexs(salt, ikm, info), n))
		key, err := hkdf.Key(hashKinds[k], ikm, salt, string(info), n)
		if err != nil {
			want = append(want, "error")
		} else {
			want = append(want, hex.EncodeToString(key))
		}
		if i%3 == 0 {
			pass, salt := randBytes(r, r.Intn(100)), randBytes(r, r.Intn(40))
			iter, n := 1+r.Intn(50), 1+r.Intn(150)
			in = append(in, fmt.Sprintf("P\t%d\t%s\t%d\t%d", k, hexs(pass, salt), iter, n))
			key, _ := pbkdf2.Key(hashKinds[k], string(pass), salt, iter, n)
			want = append(want, hex.EncodeToString(key))
		}
	}
	// beyond 255 hash lengths
	in, want = append(in, "K\t3\t\t00\t\t8161"), append(want, "error")
	diffCompare(t, in, diffRun(t, "crypto", in), want)
}

// TestDiffChaCha20Poly1305 compares with python3's cryptography package
// (Go has no ChaCha20-Poly1305 in its standard library), skipped without
// it; the C tests check the RFC 8439 vectors in any case
func TestDiffChaCha20Poly1305(t *testing.T) {
	r := rand.New(rand.NewSource(33))
	var in, script []string
	for i := 0; i < 300; i++ {
		key, ad, pt := randBytes(r, 32), randBytes(r, r.Intn(50)), randBytes(r, r.Intn(300))
		nn := 12
		if i%3 == 0 {
			nn = 24
		}
		nonce := randBytes(r, nn)
		in = append(in, "X\t"+hexs(key, nonce, ad, pt))
		script = append(script, fmt.Sprintf("seal(%q,%q,%q,%q)", hex.EncodeToString(key), hex.EncodeToString(nonce), hex.EncodeToString(ad), hex.EncodeToString(pt)))
	}
	py := `import sys
from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305
try:
    from cryptography.hazmat.primitives.ciphers.aead import XChaCha20Poly1305
except ImportError:
    XChaCha20Poly1305 = None
def seal(k, n, a, p):
    k, n, a, p = (bytes.fromhex(x) for x in (k, n, a, p))
    if len(n) == 24:
        if XChaCha20Poly1305 is None:
            print("skip"); return
        print(XChaCha20Poly1305(k).encrypt(n, p, a).hex())
    else:
        print(ChaCha20Poly1305(k).encrypt(n, p, a).hex())
` + strings.Join(script, "\n") + "\n"
	cmd := exec.Command("python3", "-I", "-")
	cmd.Stdin = strings.NewReader(py)
	out, err := cmd.Output()
	if err != nil {
		t.Skipf("python3 with cryptography: %v", err)
	}
	want := strings.Split(strings.TrimSuffix(string(out), "\n"), "\n")
	got := diffRun(t, "crypto", in)
	var in2, want2 []string
	for i := range want {
		if want[i] == "skip" {
			continue
		}
		if got[i] != want[i] {
			t.Errorf("%.80s: got %.80s want %.80s", in[i], got[i], want[i])
		}
		// open: valid and damaged
		ct, _ := hex.DecodeString(want[i])
		f := strings.Split(in[i], "\t")
		w := "ok " + f[4]
		if i%2 == 1 {
			ct[r.Intn(len(ct))] ^= 1
			w = "auth"
		}
		in2 = append(in2, "Y\t"+f[1]+"\t"+f[2]+"\t"+f[3]+"\t"+hex.EncodeToString(ct))
		want2 = append(want2, w)
	}
	diffCompare(t, in2, diffRun(t, "crypto", in2), want2)
}

func TestDiffEd25519X25519(t *testing.T) {
	r := rand.New(rand.NewSource(34))
	var in, want []string
	var sigs [][]string
	for i := 0; i < 200; i++ {
		seed, msg := randBytes(r, 32), randBytes(r, r.Intn(200))
		priv := ed25519.NewKeyFromSeed(seed)
		pub := priv.Public().(ed25519.PublicKey)
		sig := ed25519.Sign(priv, msg)
		in, want = append(in, "E\t"+hexs(seed, msg)), append(want, hex.EncodeToString(pub)+" "+hex.EncodeToString(sig))
		sigs = append(sigs, []string{hex.EncodeToString(pub), hex.EncodeToString(msg), hex.EncodeToString(sig)})
	}
	// verification: valid, damaged signatures, messages and keys, S >= L,
	// non-canonical and invalid points
	L, _ := hex.DecodeString("edd3f55c1a631258d69cf7a2def9de1400000000000000000000000000000010")
	for i, s := range sigs {
		pub, _ := hex.DecodeString(s[0])
		msg, _ := hex.DecodeString(s[1])
		sig, _ := hex.DecodeString(s[2])
		switch i % 8 {
		case 1:
			sig[r.Intn(64)] ^= 1 << r.Intn(8)
		case 2:
			if len(msg) > 0 {
				msg[r.Intn(len(msg))] ^= 1
			} else {
				msg = []byte{0}
			}
		case 3:
			pub[r.Intn(32)] ^= 1 << r.Intn(8)
		case 4:
			// S + L: the same point, a non-canonical scalar
			var c uint
			for k := 0; k < 32; k++ {
				v := uint(sig[32+k]) + uint(L[k]) + c
				sig[32+k], c = byte(v), v>>8
			}
		case 5:
			pub = randBytes(r, 32)
		case 6:
			// y = p + small: non-canonical encodings of points
			pub = []byte{0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}
			pub[0] += byte(r.Intn(18))
			if r.Intn(2) == 0 {
				pub[31] |= 0x80
			}
		case 7:
			sig[63] |= 0x20
		}
		in = append(in, "V\t"+hexs(pub, msg, sig))
		if ed25519.Verify(pub, msg, sig) {
			want = append(want, "ok")
		} else {
			want = append(want, "bad")
		}
	}
	// X25519: random points, low-order points, the base point
	low := []string{"0000000000000000000000000000000000000000000000000000000000000000",
		"0100000000000000000000000000000000000000000000000000000000000000",
		"e0eb7a7c3b41b8ae1656e3faf19fc46ada098deb9c32b1fd866205165f49b800",
		"5f9c95bca3508c24b1d0b1559c83ef5b04445cc4581c8e86d8224eddd09f1157",
		"ecffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
		"edffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
		"eeffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f"}
	for i := 0; i < 300; i++ {
		sc := randBytes(r, 32)
		pt := randBytes(r, 32)
		if i < len(low) {
			pt, _ = hex.DecodeString(low[i])
		}
		if i%5 == 0 {
			pt[31] |= 0x80 // the top bit is ignored
		}
		priv, _ := ecdh.X25519().NewPrivateKey(sc)
		pub, _ := ecdh.X25519().NewPublicKey(pt)
		out, err := priv.ECDH(pub)
		in = append(in, "W\t"+hexs(sc, pt))
		if err != nil {
			want = append(want, "low")
		} else {
			want = append(want, hex.EncodeToString(out))
		}
		in, want = append(in, "w\t"+hexs(sc)), append(want, hex.EncodeToString(priv.PublicKey().Bytes()))
	}
	diffCompare(t, in, diffRun(t, "crypto", in), want)
}

func TestDiffASN1(t *testing.T) {
	r := rand.New(rand.NewSource(35))
	var in []string
	add := func(kind string, b []byte) { in = append(in, "der\t"+kind+"\t"+hex.EncodeToString(b)) }
	kinds := []string{"elem", "bool", "int64", "uint64", "uintbytes", "enum", "oid", "bits", "utc", "gen"}
	tags := map[string]byte{"bool": 1, "int64": 2, "uint64": 2, "uintbytes": 2, "enum": 10, "oid": 6, "bits": 3, "utc": 23, "gen": 24}
	seeds := map[string][]string{
		"int64":  {"00", "7f", "80", "ff", "0080", "00ff", "ff7f", "ff80", "0000", "ffff", "7fffffffffffffff", "8000000000000000", "00ffffffffffffffff", ""},
		"bool":   {"00", "ff", "01", "", "0000"},
		"oid":    {"2a864886f70d", "55", "8180", "2b0601040182371402", "7f", "ffffffff7f", "80", "2a80", "8fffffff7f", "88808080807f"},
		"bits":   {"00", "07", "0080", "0780", "0781", "08ff", "01fe", "01ff", "00ffff"},
		"utc":    {"3235303130313030303030305a", "3439313233313233353935395a", "3530303130313030303030305a", "393130313031303030305a", "3730303130313030303030302b30313030", "37303031303130303030303030", "3730303133323030303030305a"},
		"gen":    {"32303235303130313030303030305a", "39393939313233313233353935395a", "32303234303232393030303030305a",
			"32303233303232393030303030305a", "32303235303130313030303030302b32343030", "32303235303130313030303030302b30303630",
			"32303235303130313030303030302d30313330", "32303235303130313234303030305a", "32303235303130313030363030305a", "31393530303130313030303030305a", "32303235303130313030303030302e355a", "323032353031303130303030302b30303030", "33303030313233313233353935395a"},
		"enum":   {"00", "7fffffff", "0080000000", "ff7fffffff", "ff"},
	}
	seeds["uint64"], seeds["uintbytes"] = seeds["int64"], seeds["int64"]
	for _, k := range kinds {
		tag := tags[k]
		for _, sd := range seeds[k] {
			c, _ := hex.DecodeString(sd)
			el := append([]byte{tag, byte(len(c))}, c...)
			add(k, el)
			add(k, append(el, 0x05, 0x00))                                    // with something after
			add(k, append([]byte{tag, 0x81, byte(len(c))}, c...))             // non-minimal length
			add(k, append([]byte{tag ^ 0x20, byte(len(c))}, c...))            // another tag
			if len(c) > 0 {
				add(k, el[:len(el)-1]) // truncated
			}
		}
	}
	// lengths in every form, random elements and their mutations
	for _, h := range []string{"0400", "047f", "0480", "048100", "04817f", "048180", "04820080", "048200ff", "04830000ff",
		"0484000000ff", "048500000000ff", "1f0100", "3f8101", "300030", "a003020101"} {
		b, _ := hex.DecodeString(h)
		for len(b) < 300 {
			b = append(b, 0x41)
		}
		add("elem", b)
	}
	for i := 0; i < 2000; i++ {
		k := kinds[r.Intn(len(kinds))]
		n := r.Intn(12)
		b := append([]byte{tags[k], byte(n)}, randBytes(r, n)...)
		if k == "elem" || r.Intn(10) == 0 {
			b[0] = byte(r.Intn(256))
		}
		if r.Intn(8) == 0 {
			b[1] = byte(r.Intn(256))
		}
		add(k, b)
	}
	diffCompare(t, in, diffRun(t, "asn1", in), goRef(t, in))
}

func TestDiffECDSA(t *testing.T) {
	r := rand.New(rand.NewSource(36))
	curves := []elliptic.Curve{elliptic.P256(), elliptic.P384(), elliptic.P521()}
	ecdhCurves := []ecdh.Curve{ecdh.P256(), ecdh.P384(), ecdh.P521()}
	hashes := []crypto.Hash{crypto.SHA1, crypto.SHA256, crypto.SHA384, crypto.SHA512}
	var in, want []string
	type sigReq struct {
		ci   int
		pub  []byte
		hash []byte
	}
	var ours []sigReq
	for i := 0; i < 90; i++ {
		ci := i % 3
		key, _ := ecdsa.GenerateKey(curves[ci], crand.Reader)
		priv, _ := key.Bytes()
		pub, _ := key.PublicKey.Bytes()
		in, want = append(in, fmt.Sprintf("U\t%d\t%s", ci+1, hexs(priv))), append(want, hex.EncodeToString(pub))
		// Go signs, we verify (also damaged)
		h := hashes[r.Intn(len(hashes))].New()
		h.Write(randBytes(r, 50))
		digest := h.Sum(nil)
		if i%10 == 0 {
			digest = randBytes(r, 1+r.Intn(100)) // any length: truncated or short
		}
		sig, _ := ecdsa.SignASN1(crand.Reader, key, digest)
		for k := 0; k < 4; k++ {
			s, d, p := append([]byte(nil), sig...), append([]byte(nil), digest...), append([]byte(nil), pub...)
			switch k {
			case 1:
				s[r.Intn(len(s))] ^= 1 << r.Intn(8)
			case 2:
				d[r.Intn(len(d))] ^= 1
			case 3:
				p[1+r.Intn(len(p)-1)] ^= 1
			}
			in = append(in, fmt.Sprintf("Q\t%d\t%s", ci+1, hexs(p, d, s)))
			pk, err := ecdsa.ParseUncompressedPublicKey(curves[ci], p)
			switch {
			case err != nil:
				want = append(want, "key")
			case ecdsa.VerifyASN1(pk, d, s):
				want = append(want, "ok")
			default:
				want = append(want, "bad")
			}
		}
		// we sign: checked by Go below
		in, want = append(in, fmt.Sprintf("S\t%d\t%s", ci+1, hexs(priv, digest))), append(want, "")
		ours = append(ours, sigReq{ci, pub, digest})
		// ECDH both ways
		peer, _ := ecdhCurves[ci].GenerateKey(crand.Reader)
		mine, _ := ecdhCurves[ci].NewPrivateKey(priv)
		secret, _ := mine.ECDH(peer.PublicKey())
		in, want = append(in, fmt.Sprintf("D\t%d\t%s", ci+1, hexs(priv, peer.PublicKey().Bytes()))), append(want, hex.EncodeToString(secret))
		bad := append([]byte(nil), peer.PublicKey().Bytes()...)
		bad[len(bad)-1] ^= 1
		in, want = append(in, fmt.Sprintf("D\t%d\t%s", ci+1, hexs(priv, bad))), append(want, "key")
	}
	// scalars out of range
	in, want = append(in, "U\t1\t"+strings.Repeat("ff", 32)), append(want, "key")
	in, want = append(in, "U\t1\t"+strings.Repeat("00", 32)), append(want, "key")
	got := diffRun(t, "crypto", in)
	k := 0
	for i := range in {
		if in[i][0] != 'S' {
			if got[i] != want[i] {
				t.Errorf("%.60s: got %.80s want %.80s", in[i], got[i], want[i])
			}
			continue
		}
		rq := ours[k]
		k++
		sig, err := hex.DecodeString(got[i])
		pk, _ := ecdsa.ParseUncompressedPublicKey(curves[rq.ci], rq.pub)
		if err != nil || !ecdsa.VerifyASN1(pk, rq.hash, sig) {
			t.Errorf("Go does not verify our signature %q", got[i])
		}
	}
}

func TestDiffRSA(t *testing.T) {
	r := rand.New(rand.NewSource(37))
	var keys []*rsa.PrivateKey
	for _, bits := range []int{1024, 2048, 3072} {
		k, _ := rsa.GenerateKey(crand.Reader, bits)
		keys = append(keys, k)
	}
	kinds := []crypto.Hash{crypto.MD5, crypto.SHA1, crypto.SHA224, crypto.SHA256, crypto.SHA384, crypto.SHA512}
	var in, want []string
	type req struct {
		key  *rsa.PrivateKey
		h    crypto.Hash
		dig  []byte
		pss  bool
		salt int
	}
	var ours []req
	for i := 0; i < 120; i++ {
		key := keys[i%3]
		ki := r.Intn(len(kinds))
		if i%2 == 1 && ki == 0 {
			ki = 1 // PSS: no MD5
		}
		if i%3 == 0 && ki == 5 {
			ki = 4 // 1024 bits hold no PSS with SHA-512 and its length of salt
		}
		h := kinds[ki].New()
		h.Write(randBytes(r, 40))
		dig := h.Sum(nil)
		n := key.N.Bytes()
		pub := fmt.Sprintf("%d\t%x\t%d", ki, n, key.E)
		if i%2 == 0 {
			sig, _ := rsa.SignPKCS1v15(crand.Reader, key, kinds[ki], dig)
			for m := 0; m < 3; m++ {
				s, d := append([]byte(nil), sig...), append([]byte(nil), dig...)
				if m == 1 {
					s[r.Intn(len(s))] ^= 1
				} else if m == 2 {
					d[0] ^= 1
				}
				in = append(in, fmt.Sprintf("r\t%s\t%x\t%x", pub, d, s))
				if rsa.VerifyPKCS1v15(&key.PublicKey, kinds[ki], d, s) == nil {
					want = append(want, "ok")
				} else {
					want = append(want, "bad")
				}
			}
		} else {
			salt := []int{rsa.PSSSaltLengthEqualsHash, rsa.PSSSaltLengthAuto, 0, 7}[r.Intn(4)]
			sig, err := rsa.SignPSS(crand.Reader, key, kinds[ki], dig, &rsa.PSSOptions{SaltLength: salt})
			if err != nil {
				t.Fatal(err)
			}
			for m := 0; m < 3; m++ {
				s := append([]byte(nil), sig...)
				if m == 1 {
					s[r.Intn(len(s))] ^= 1
				}
				vsalt := []int{rsa.PSSSaltLengthAuto, kinds[ki].Size(), 7}[m]
				in = append(in, fmt.Sprintf("p\t%s\t%x\t%x\t%d", pub, dig, s, map[int]int{rsa.PSSSaltLengthAuto: -1}[vsalt]+vsalt*b2i(vsalt >= 0)))
				if rsa.VerifyPSS(&key.PublicKey, kinds[ki], dig, s, &rsa.PSSOptions{SaltLength: vsalt}) == nil {
					want = append(want, "ok")
				} else {
					want = append(want, "bad")
				}
			}
		}
		// we sign: Go verifies
		p, q := key.Primes[0], key.Primes[1]
		priv := fmt.Sprintf("%x\t%x\t%x\t%x\t%x\t%x", key.D.Bytes(), p.Bytes(), q.Bytes(), key.Precomputed.Dp.Bytes(),
			key.Precomputed.Dq.Bytes(), key.Precomputed.Qinv.Bytes())
		pss := i%2 == 1
		if pss {
			in = append(in, fmt.Sprintf("t\t%s\t%s\t%x\t-1", pub, priv, dig))
		} else {
			in = append(in, fmt.Sprintf("s\t%s\t%s\t%x", pub, priv, dig))
		}
		want = append(want, "")
		ours = append(ours, req{key, kinds[ki], dig, pss, 0})
	}
	got := diffRun(t, "crypto", in)
	k := 0
	for i := range in {
		if in[i][0] != 's' && in[i][0] != 't' {
			if got[i] != want[i] {
				t.Errorf("%.40s: got %s want %s", in[i], got[i], want[i])
			}
			continue
		}
		rq := ours[k]
		k++
		sig, _ := hex.DecodeString(got[i])
		var err error
		if rq.pss {
			err = rsa.VerifyPSS(&rq.key.PublicKey, rq.h, rq.dig, sig, &rsa.PSSOptions{SaltLength: rsa.PSSSaltLengthEqualsHash})
		} else {
			err = rsa.VerifyPKCS1v15(&rq.key.PublicKey, rq.h, rq.dig, sig)
			// PKCS #1 v1.5 is deterministic: the same as Go's
			want, _ := rsa.SignPKCS1v15(nil, rq.key, rq.h, rq.dig)
			if !bytes.Equal(want, sig) {
				t.Errorf("PKCS #1 v1.5 signature differs from Go's")
			}
		}
		if err != nil {
			t.Errorf("Go does not verify our signature: %v (%.40s)", err, got[i])
		}
	}
}
