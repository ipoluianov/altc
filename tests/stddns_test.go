package tests

import (
	"encoding/binary"
	"fmt"
	"net"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

// TestResolver runs AltC's resolver against a DNS server written here
// (UDP and TCP), whose answers cover CNAME chains, compression,
// truncation, failures, spoofed IDs and malformed messages.

type dnsQuestion struct {
	id    uint16
	name  string // as asked (case kept)
	qtype uint16
	raw   []byte // the question section
}

func parseQuery(b []byte) (q dnsQuestion, ok bool) {
	if len(b) < 12 {
		return q, false
	}
	q.id = binary.BigEndian.Uint16(b)
	off := 12
	var labels []string
	for off < len(b) && b[off] != 0 {
		l := int(b[off])
		if off+1+l > len(b) {
			return q, false
		}
		labels = append(labels, string(b[off+1:off+1+l]))
		off += 1 + l
	}
	if off+5 > len(b) {
		return q, false
	}
	off++
	q.qtype = binary.BigEndian.Uint16(b[off:])
	q.raw = b[12 : off+4]
	q.name = strings.Join(labels, ".")
	return q, true
}

func encName(n string) []byte {
	var out []byte
	for _, l := range strings.Split(strings.TrimSuffix(n, "."), ".") {
		out = append(out, byte(len(l)))
		out = append(out, l...)
	}
	return append(out, 0)
}

type rr struct {
	name  []byte // encoded owner (may be a pointer)
	typ   uint16
	rdata []byte
}

func msg(q dnsQuestion, id uint16, flags uint16, rrs []rr) []byte {
	b := make([]byte, 12)
	binary.BigEndian.PutUint16(b, id)
	binary.BigEndian.PutUint16(b[2:], 0x8000|0x0100|0x0080|flags)
	binary.BigEndian.PutUint16(b[4:], 1)
	binary.BigEndian.PutUint16(b[6:], uint16(len(rrs)))
	b = append(b, q.raw...)
	for _, r := range rrs {
		b = append(b, r.name...)
		var h [10]byte
		binary.BigEndian.PutUint16(h[0:], r.typ)
		binary.BigEndian.PutUint16(h[2:], 1)
		binary.BigEndian.PutUint32(h[4:], 60)
		binary.BigEndian.PutUint16(h[8:], uint16(len(r.rdata)))
		b = append(b, h[:]...)
		b = append(b, r.rdata...)
	}
	return b
}

var qptr = []byte{0xC0, 12} // the question's name

func ip4(s string) []byte { return net.ParseIP(s).To4() }
func ip6(s string) []byte { return net.ParseIP(s).To16() }

// answer gives the reply to a query; tcp tells over which transport
func answer(q dnsQuestion, tcp bool, server int) [][]byte {
	name := strings.ToLower(q.name)
	const A, AAAA, CNAME = 1, 28, 5
	one := func(flags uint16, rrs []rr) [][]byte { return [][]byte{msg(q, q.id, flags, rrs)} }
	switch name {
	case "a.test":
		if q.qtype == A {
			return one(0, []rr{{qptr, A, ip4("1.2.3.4")}, {qptr, A, ip4("1.2.3.5")}})
		}
		return one(0, []rr{{qptr, AAAA, ip6("2001:db8::1")}})
	case "alias.test":
		// alias -> b.test -> c.test (compressed through earlier names)
		b := encName("b.test")
		c := encName("c.test")
		if q.qtype == A {
			return one(0, []rr{{encName("other.test"), A, ip4("9.9.9.9")}, {qptr, CNAME, b}, {b, CNAME, c}, {c, A, ip4("5.6.7.8")}})
		}
		return one(0, []rr{{qptr, CNAME, b}, {b, CNAME, c}})
	case "big.test":
		if !tcp {
			return one(0x0200, nil) // truncated
		}
		if q.qtype != A {
			return one(0, nil)
		}
		var rrs []rr
		for i := 1; i <= 20; i++ {
			rrs = append(rrs, rr{qptr, A, ip4(fmt.Sprintf("10.0.0.%d", i))})
		}
		return one(0, rrs)
	case "missing.test":
		return one(3, nil) // NXDOMAIN
	case "empty.test":
		return one(0, nil)
	case "host.corp.test":
		if q.qtype == A {
			return one(0, []rr{{qptr, A, ip4("7.7.7.7")}})
		}
		return one(0, nil)
	case "flaky.test":
		if server == 0 {
			return one(2, nil) // SERVFAIL from the first server
		}
		if q.qtype == A {
			return one(0, []rr{{qptr, A, ip4("8.8.4.4")}})
		}
		return one(0, nil)
	case "spoof.test":
		if q.qtype != A {
			return one(0, nil)
		}
		// a reply with another ID first, then the real one
		return [][]byte{msg(q, q.id+1, 0, []rr{{qptr, A, ip4("6.6.6.6")}}), msg(q, q.id, 0, []rr{{qptr, A, ip4("4.4.4.4")}})}
	case "loop.test":
		// a compression pointer to itself, then silence
		m := msg(q, q.id, 0, nil)
		binary.BigEndian.PutUint16(m[6:], 1)
		m = append(m, 0xC0, byte(len(m)))
		return [][]byte{m}
	case "slow.test":
		return nil
	case "upper.test":
		if q.qtype == A {
			// the answer's owner in other case, compressed to the question
			return one(0, []rr{{encName("UPPER.TEST"), A, ip4("3.3.3.3")}})
		}
		return one(0, nil)
	}
	return one(3, nil)
}

func startDNS(t *testing.T, server int) string {
	pc, err := net.ListenPacket("udp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	addr := pc.LocalAddr().String()
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { pc.Close(); ln.Close() })
	go func() {
		buf := make([]byte, 1500)
		for {
			n, from, err := pc.ReadFrom(buf)
			if err != nil {
				return
			}
			q, ok := parseQuery(buf[:n])
			if !ok {
				continue
			}
			for _, m := range answer(q, false, server) {
				pc.WriteTo(m, from)
			}
		}
	}()
	go func() {
		for {
			c, err := ln.Accept()
			if err != nil {
				return
			}
			go func(c net.Conn) {
				defer c.Close()
				var l [2]byte
				if _, err := c.Read(l[:]); err != nil {
					return
				}
				b := make([]byte, binary.BigEndian.Uint16(l[:]))
				if _, err := c.Read(b); err != nil {
					return
				}
				q, _ := parseQuery(b)
				for _, m := range answer(q, true, server) {
					binary.BigEndian.PutUint16(l[:], uint16(len(m)))
					c.Write(append(l[:], m...))
				}
			}(c)
		}
	}()
	return addr
}

func TestResolver(t *testing.T) {
	s0, s1 := startDNS(t, 0), startDNS(t, 1)
	cases := []struct{ line, want string }{
		{"a.test\t\t1\t1000\t1", "1.2.3.4 1.2.3.5 2001:db8::1"},
		{"a.test.\t\t1\t1000\t1", "1.2.3.4 1.2.3.5 2001:db8::1"},
		{"A.TEST\t\t1\t1000\t1", "1.2.3.4 1.2.3.5 2001:db8::1"},
		{"alias.test\t\t1\t1000\t1", "5.6.7.8"},
		{"big.test\t\t1\t2000\t1", "10.0.0.1 10.0.0.2 10.0.0.3 10.0.0.4 10.0.0.5 10.0.0.6 10.0.0.7 10.0.0.8 10.0.0.9 10.0.0.10 10.0.0.11 10.0.0.12 10.0.0.13 10.0.0.14 10.0.0.15 10.0.0.16 10.0.0.17 10.0.0.18 10.0.0.19 10.0.0.20"},
		{"missing.test\t\t1\t1000\t1", "err no such host"},
		{"empty.test\t\t1\t1000\t1", "err no such host"},
		{"host\tcorp.test\t1\t1000\t1", "7.7.7.7"},
		{"flaky.test\t\t1\t1000\t1", "8.8.4.4"},
		{"spoof.test\t\t1\t1000\t1", "4.4.4.4"},
		{"upper.test\t\t1\t1000\t1", "3.3.3.3"},
		{"loop.test\t\t1\t200\t1", "err Operation timed out"},
		{"slow.test\t\t1\t200\t1", "err Operation timed out"},
		{"bad..name\t\t1\t200\t1", "err no such host"},
		{"localhost\t\t1\t200\t1", "127.0.0.1 ::1"},
		{"x.localhost\t\t1\t200\t1", "127.0.0.1 ::1"},
		{"8.8.8.8\t\t1\t200\t1", "8.8.8.8"},
	}
	var in, want []string
	for _, c := range cases {
		in = append(in, c.line)
		want = append(want, c.want)
	}
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "diff", "dns_diff.c"), filepath.Join(tempDir(t), "dns_diff"))
	cmd := exec.Command(exe, s0, s1)
	cmd.Stdin = strings.NewReader(strings.Join(in, "\n") + "\n")
	out, err := cmd.CombinedOutput()
	if err != nil {
		t.Fatalf("%v\n%s", err, out)
	}
	got := strings.Split(strings.TrimSuffix(string(out), "\n"), "\n")
	if len(got) != len(want) {
		t.Fatalf("%d answers:\n%s", len(got), out)
	}
	diffCompare(t, in, got, want)
}
