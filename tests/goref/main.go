// Command goref answers requests with the results of Go packages that
// are not in the standard library (golang.org/x/crypto), for the
// differential tests of AltC's library: one request per input line,
// fields separated by tabs, one answer line each.
package main

import (
	"bufio"
	"bytes"
	encoding_asn1 "encoding/asn1"
	"encoding/hex"
	"fmt"
	"os"
	"strings"
	"time"

	"golang.org/x/crypto/chacha20poly1305"
	"golang.org/x/crypto/cryptobyte"
	"golang.org/x/crypto/cryptobyte/asn1"
	"golang.org/x/net/http2/hpack"
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
	case "hpe": // ops: encode with one encoder
		return hpackEncode(f[1:])
	case "hpd": // max-table max-string blocks...: decode with one decoder
		return hpackDecode(f[1:])
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

// hpackEncode: ops "S<size>" (SetMaxDynamicTableSize), "L<size>"
// (SetMaxDynamicTableSizeLimit), "F<name hex>:<value hex>" or
// "f..." (sensitive), "B" (ends a block); the blocks in hex
func hpackEncode(ops []string) string {
	var buf bytes.Buffer
	e := hpack.NewEncoder(&buf)
	var blocks []string
	for _, op := range ops {
		switch op[0] {
		case 'S', 'L':
			var n uint32
			fmt.Sscanf(op[1:], "%d", &n)
			if op[0] == 'S' {
				e.SetMaxDynamicTableSize(n)
			} else {
				e.SetMaxDynamicTableSizeLimit(n)
			}
		case 'F', 'f':
			nv := strings.SplitN(op[1:], ":", 2)
			e.WriteField(hpack.HeaderField{Name: string(unhex(nv[0])), Value: string(unhex(nv[1])), Sensitive: op[0] == 'f'})
		case 'B':
			blocks = append(blocks, hex.EncodeToString(buf.Bytes()))
			buf.Reset()
		}
	}
	return strings.Join(blocks, " ")
}

// hpackDecode: each block's fields ("ok" name:value[:s] in hex) or the error, which ends it
func hpackDecode(f []string) string {
	var max, maxStr uint32
	fmt.Sscanf(f[0], "%d", &max)
	fmt.Sscanf(f[1], "%d", &maxStr)
	d := hpack.NewDecoder(max, nil)
	d.SetMaxStringLength(int(maxStr))
	var res []string
	for _, b := range f[2:] {
		if strings.HasPrefix(b, "T") { // a table size change by the decoder's side
			var n uint32
			fmt.Sscanf(b[1:], "%d", &n)
			d.SetAllowedMaxDynamicTableSize(n)
			continue
		}
		hf, err := d.DecodeFull(unhex(b))
		if err != nil {
			res = append(res, "err "+err.Error())
			break
		}
		var fs []string
		for _, x := range hf {
			s := hex.EncodeToString([]byte(x.Name)) + ":" + hex.EncodeToString([]byte(x.Value))
			if x.Sensitive {
				s += ":s"
			}
			fs = append(fs, s)
		}
		res = append(res, "ok "+strings.Join(fs, ","))
	}
	return strings.Join(res, " | ")
}
