// Command goref answers requests with the results of Go packages that
// are not in the standard library (golang.org/x/crypto), for the
// differential tests of AltC's library: one request per input line,
// fields separated by tabs, one answer line each.
package main

import (
	"bufio"
	encoding_asn1 "encoding/asn1"
	"encoding/hex"
	"fmt"
	"os"
	"strings"
	"time"

	"golang.org/x/crypto/chacha20poly1305"
	"golang.org/x/crypto/cryptobyte"
	"golang.org/x/crypto/cryptobyte/asn1"
)

func unhex(s string) []byte {
	b, _ := hex.DecodeString(s)
	return b
}

func main() {
	in := bufio.NewScanner(os.Stdin)
	in.Buffer(make([]byte, 1<<20), 64<<20)
	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()
	for in.Scan() {
		f := strings.Split(in.Text(), "\t")
		fmt.Fprintln(out, answer(f))
	}
}

func answer(f []string) string {
	switch f[0] {
	case "seal": // key nonce ad pt
		k, n, a, p := unhex(f[1]), unhex(f[2]), unhex(f[3]), unhex(f[4])
		var aead interface {
			Seal(dst, nonce, plaintext, additionalData []byte) []byte
		}
		if len(n) == 24 {
			aead, _ = chacha20poly1305.NewX(k)
		} else {
			aead, _ = chacha20poly1305.New(k)
		}
		return hex.EncodeToString(aead.Seal(nil, n, p, a))
	case "der": // kind hex: parse one value with cryptobyte
		return der(f[1], unhex(f[2]))
	}
	return "unknown"
}

func der(kind string, b []byte) string {
	s := cryptobyte.String(b)
	var ok bool
	var res string
	switch kind {
	case "elem":
		var c cryptobyte.String
		var tag asn1.Tag
		ok = s.ReadAnyASN1(&c, &tag)
		res = fmt.Sprintf("%d %x", tag, []byte(c))
	case "bool":
		var v bool
		ok = s.ReadASN1Boolean(&v)
		res = fmt.Sprint(v)
	case "int64":
		var v int64
		ok = s.ReadASN1Integer(&v)
		res = fmt.Sprint(v)
	case "uint64":
		var v uint64
		ok = s.ReadASN1Integer(&v)
		res = fmt.Sprint(v)
	case "uintbytes":
		var v []byte
		ok = s.ReadASN1Integer(&v)
		res = hex.EncodeToString(v)
	case "enum":
		var v int
		ok = s.ReadASN1Enum(&v)
		res = fmt.Sprint(v)
	case "oid":
		var oid encoding_asn1.ObjectIdentifier
		ok = s.ReadASN1ObjectIdentifier(&oid)
		res = oid.String()
	case "bits":
		var v encoding_asn1.BitString
		ok = s.ReadASN1BitString(&v)
		res = fmt.Sprintf("%d %x", v.BitLength, v.Bytes)
	case "utc", "gen":
		var t time.Time
		if kind == "utc" {
			ok = s.ReadASN1UTCTime(&t)
		} else {
			ok = s.ReadASN1GeneralizedTime(&t)
		}
		res = fmt.Sprint(t.Unix())
	}
	if !ok {
		return "fail"
	}
	return res + fmt.Sprintf(" rest=%d", len(s))
}
