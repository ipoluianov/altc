package tests

import (
	"bufio"
	"crypto/tls"
	"errors"
	"fmt"
	"net"
	"net/smtp"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
)

// A scripted SMTP server: it answers each command with the next reply
// queued for its verb (a default otherwise) and records what it
// receives. Replies are sent as they are (several lines, malformed
// ones); "CLOSE" closes the connection instead.
type smtpScript struct {
	greet   string              // default "220 mx.example ESMTP ready\r\n"
	replies map[string][]string // verb ("EHLO", "MAIL"...), "+" for AUTH continuations, "." after the data
}

var smtpDefaults = map[string]string{"STARTTLS": "220 2.0.0 go ahead\r\n", "DATA": "354 end with .\r\n", "QUIT": "221 2.0.0 bye\r\n",
	"AUTH": "235 2.7.0 accepted\r\n", "+": "235 2.7.0 accepted\r\n"}

type smtpServer struct {
	ln   net.Listener
	mu   sync.Mutex
	log  []string
	done chan struct{}
}

func startSMTPServer(t *testing.T, sc smtpScript, cfg *tls.Config) *smtpServer {
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	s := &smtpServer{ln: ln, done: make(chan struct{})}
	t.Cleanup(func() { ln.Close() })
	go func() {
		defer close(s.done)
		queues := map[string][]string{}
		for k, v := range sc.replies {
			queues[k] = append([]string(nil), v...)
		}
		// one connection per run (SendMail and the client make one)
		conn, err := ln.Accept()
		if err != nil {
			return
		}
		defer conn.Close()
		conn.SetDeadline(time.Now().Add(20 * time.Second))
		s.serve(conn, sc, queues, cfg)
	}()
	return s
}

func (s *smtpServer) record(l string) {
	s.mu.Lock()
	s.log = append(s.log, l)
	s.mu.Unlock()
}

func (s *smtpServer) serve(conn net.Conn, sc smtpScript, queues map[string][]string, cfg *tls.Config) {
	greet := sc.greet
	if greet == "" {
		greet = "220 mx.example ESMTP ready\r\n"
	}
	if greet == "CLOSE" {
		return
	}
	conn.Write([]byte(greet))
	br := bufio.NewReader(conn)
	cont := false
	for {
		line, err := br.ReadString('\n')
		if err != nil {
			if line != "" {
				s.record(fmt.Sprintf("C: %q", line))
			}
			s.record("EOF")
			return
		}
		s.record(fmt.Sprintf("C: %q", line))
		verb := strings.ToUpper(strings.TrimRight(line, "\r\n"))
		if i := strings.IndexAny(verb, " :"); i >= 0 {
			verb = verb[:i]
		}
		if cont {
			verb = "+"
		}
		reply := ""
		if q := queues[verb]; len(q) > 0 {
			reply, queues[verb] = q[0], q[1:]
		} else if d, ok := smtpDefaults[verb]; ok {
			reply = d
		} else {
			reply = "250 2.0.0 ok\r\n"
		}
		if reply == "CLOSE" {
			s.record("closed")
			return
		}
		conn.Write([]byte(reply))
		cont = strings.HasPrefix(reply, "334")
		switch {
		case verb == "DATA" && strings.HasPrefix(reply, "354"):
			var data strings.Builder
			for {
				l, err := br.ReadString('\n')
				if err != nil {
					s.record(fmt.Sprintf("D: %q EOF", data.String()+l))
					return
				}
				if l == ".\r\n" {
					break
				}
				data.WriteString(l)
			}
			s.record(fmt.Sprintf("D: %q", data.String()))
			r := "250 2.0.0 queued\r\n"
			if q := queues["."]; len(q) > 0 {
				r, queues["."] = q[0], q[1:]
			}
			conn.Write([]byte(r))
		case verb == "STARTTLS" && strings.HasPrefix(reply, "220"):
			tc := tls.Server(conn, cfg)
			if err := tc.Handshake(); err != nil {
				s.record("tls handshake failed")
				return
			}
			s.record("tls")
			conn = tc
			br = bufio.NewReader(conn)
		case verb == "QUIT" && strings.HasPrefix(reply, "221"):
			// wait for the client to close
			b := make([]byte, 1)
			if _, err := br.Read(b); err != nil {
				s.record("EOF")
			}
			return
		}
	}
}

func (s *smtpServer) transcript() []string {
	s.ln.Close() // a run that never connected ends here
	select {
	case <-s.done:
	case <-time.After(20 * time.Second):
		return []string{"server timed out"}
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	return append([]string(nil), s.log...)
}

// goErrText: Go's error text without the addresses of network errors
func goErrText(err error) string {
	var oe *net.OpError
	if errors.As(err, &oe) && oe.Err != nil {
		return oe.Err.Error()
	}
	return err.Error()
}

func goSMTPResult(op string, err error) string {
	if err != nil {
		return op + ": err " + goErrText(err)
	}
	return op + ": ok"
}

func parseAuthSpec(spec string) smtp.Auth {
	p := strings.Split(spec, ":")
	for i := 1; i < len(p); i++ {
		if p[i] == "-" {
			p[i] = ""
		} else {
			p[i] = munhx(p[i])
		}
	}
	switch {
	case p[0] == "plain" && len(p) == 5:
		return smtp.PlainAuth(p[1], p[2], p[3], p[4])
	case p[0] == "cram" && len(p) == 3:
		return smtp.CRAMMD5Auth(p[1], p[2])
	}
	return nil
}

// runGoSMTP runs the operations with Go's net/smtp
func runGoSMTP(addr string, ops []string, roots *tls.Config) []string {
	var out []string
	var c *smtp.Client
	for _, l := range ops {
		f := strings.Split(l, "\t")
		op := f[0]
		arg := func(i int) string { return munhx(f[i]) }
		var err error
		switch op {
		case "dial", "connect":
			if op == "dial" {
				c, err = smtp.Dial(addr)
			} else {
				var conn net.Conn
				conn, err = net.Dial("tcp", addr)
				if err == nil {
					c, err = smtp.NewClient(conn, arg(1))
				}
			}
			if err != nil {
				c = nil
			}
			out = append(out, goSMTPResult(op, err))
			continue
		case "sendmail":
			var a smtp.Auth
			if f[1] != "none" {
				a = parseAuthSpec(f[1])
			}
			var to []string
			for _, x := range strings.Split(f[3], ",") {
				if x == "-" {
					to = append(to, "")
				} else {
					to = append(to, munhx(x))
				}
			}
			out = append(out, goSMTPResult("sendmail", smtp.SendMail(addr, a, arg(2), to, []byte(arg(4)))))
			continue
		}
		if c == nil {
			out = append(out, op+": no client")
			continue
		}
		switch op {
		case "hello":
			err = c.Hello(arg(1))
		case "ext":
			ok, p := c.Extension(arg(1))
			out = append(out, fmt.Sprintf("ext: %v %s", ok, p))
			continue
		case "starttls":
			cfg := roots.Clone()
			cfg.ServerName = arg(1)
			err = c.StartTLS(cfg)
		case "tlsstate":
			st, ok := c.TLSConnectionState()
			out = append(out, fmt.Sprintf("tlsstate: %v %s", ok, st.ServerName))
			continue
		case "plain":
			err = c.Auth(smtp.PlainAuth(arg(1), arg(2), arg(3), arg(4)))
		case "cram":
			err = c.Auth(smtp.CRAMMD5Auth(arg(1), arg(2)))
		case "mail":
			err = c.Mail(arg(1))
		case "rcpt":
			err = c.Rcpt(arg(1))
		case "vrfy":
			err = c.Verify(arg(1))
		case "data":
			w, err := c.Data()
			out = append(out, goSMTPResult("data", err))
			if err == nil {
				if _, err := w.Write([]byte(arg(1))); err != nil {
					out = append(out, "write: err "+goErrText(err))
				}
				out = append(out, goSMTPResult("dataclose", w.Close()))
			}
			continue
		case "rset":
			err = c.Reset()
		case "noop":
			err = c.Noop()
		case "quit":
			err = c.Quit()
		case "close":
			err = c.Close()
			c = nil
			if err != nil {
				out = append(out, "close: err")
			} else {
				out = append(out, "close: ok")
			}
			continue
		default:
			out = append(out, op+": unknown")
			continue
		}
		out = append(out, goSMTPResult(op, err))
	}
	if c != nil {
		c.Close()
	}
	return out
}

func runCSMTP(t *testing.T, exe, addr, ca string, ops []string) []string {
	cmd := exec.Command(exe, addr, ca)
	cmd.Stdin = strings.NewReader(strings.Join(ops, "\n") + "\n")
	b, err := cmd.Output()
	if err != nil {
		t.Errorf("smtpc: %v\n%s", err, b)
	}
	return strings.Split(strings.TrimSuffix(string(b), "\n"), "\n")
}

type smtpCase struct {
	name   string
	script smtpScript
	ops    []string
}

// op builds an operation line: strings are hex-encoded
func op(name string, args ...string) string {
	l := name
	for _, a := range args {
		l += "\t" + mhx(a)
	}
	return l
}

func sendmailOp(auth, from string, to []string, msg string) string {
	var tos []string
	for _, x := range to {
		if x == "" {
			tos = append(tos, "-")
		} else {
			tos = append(tos, mhx(x))
		}
	}
	return "sendmail\t" + auth + "\t" + mhx(from) + "\t" + strings.Join(tos, ",") + "\t" + mhx(msg)
}

func authSpec(kind string, parts ...string) string {
	s := kind
	for _, p := range parts {
		if p == "" {
			s += ":-"
		} else {
			s += ":" + mhx(p)
		}
	}
	return s
}

func TestDiffSmtp(t *testing.T) {
	p := makePKI(t)
	cert, err := tls.LoadX509KeyPair(p.leaves["p256"][0], p.leaves["p256"][1])
	if err != nil {
		t.Fatal(err)
	}
	srvCfg := &tls.Config{Certificates: []tls.Certificate{cert}}
	roots := &tls.Config{RootCAs: p.pool}
	exe := buildStdTest(t, filepath.Join("..", "std", "tests", "prog", "smtpc.c"), filepath.Join(tempDir(t), "smtpc"))

	ehloFull := "250-mx.example greets localhost\r\n250-PIPELINING\r\n250-SIZE 35882577\r\n250-8BITMIME\r\n250-SMTPUTF8\r\n" +
		"250-AUTH PLAIN CRAM-MD5 LOGIN\r\n250-STARTTLS\r\n250 HELP\r\n"
	ehloPlain := "250-mx.example\r\n250-AUTH PLAIN\r\n250 8BITMIME\r\n"
	ehloNoAuth := "250-mx.example\r\n250 SIZE 100\r\n"
	ehloTLS := "250-mx.example\r\n250-STARTTLS\r\n250 AUTH PLAIN CRAM-MD5\r\n"
	body := "From: a@example.com\r\nTo: b@example.com\r\nSubject: test\r\n\r\nline 1\r\n.leading dot\r\n..two\r\nbare\nlf\r\n.\r\nend"
	cases := []smtpCase{
		{"basic", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}}},
			[]string{"dial", op("ext", "8bitmime"), op("ext", "AUTH"), op("ext", "size"), op("ext", "nope"), op("ext", "help"),
				op("mail", "a@example.com"), op("rcpt", "b@example.com"), op("rcpt", "c@example.com"), op("data", body), "quit", "close"}},
		{"hello", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}}},
			[]string{"dial", op("hello", "client.example"), op("hello", "again"), "noop", "rset", op("vrfy", "x@y"), "quit"}},
		{"hello-crlf", smtpScript{}, []string{"dial", op("hello", "a\r\nb"), op("hello", "ok.example"), "quit"}},
		{"helo-fallback", smtpScript{replies: map[string][]string{"EHLO": {"502 5.5.2 not implemented\r\n"}}},
			[]string{"dial", op("ext", "8BITMIME"), op("mail", "a@b"), op("rcpt", "c@d"), op("plain", "", "u", "p", "127.0.0.1"), "quit"}},
		{"hello-fails", smtpScript{replies: map[string][]string{"EHLO": {"502 no\r\n"}, "HELO": {"554-go\r\n554 away\r\n"}}},
			[]string{"dial", op("mail", "a@b"), "noop", op("ext", "SIZE"), op("rcpt", "x@y"), "quit", "close"}},
		{"plain-localhost", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}}},
			[]string{"dial", op("plain", "", "user", "pass", "127.0.0.1"), op("mail", "a@b"), "quit"}},
		{"plain-identity", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}}},
			[]string{op("connect", "localhost"), op("plain", "id", "user", "pass", "localhost"), "quit"}},
		{"plain-unencrypted", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}}},
			[]string{op("connect", "mail.example.com"), op("plain", "", "u", "p", "mail.example.com"), op("mail", "a@b"), "close"}},
		{"plain-wrong-host", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}}},
			[]string{"dial", op("plain", "", "u", "p", "other.example"), "noop"}},
		{"plain-challenge", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}, "AUTH": {"334 \r\n"}}},
			[]string{"dial", op("plain", "", "u", "p", "127.0.0.1"), "noop"}},
		{"plain-rejected", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}, "AUTH": {"535 5.7.8 bad credentials\r\n"}}},
			[]string{"dial", op("plain", "", "u", "p", "127.0.0.1"), op("mail", "a@b"), "quit"}},
		{"cram", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}, "AUTH": {"334 PDQxOTI5NDIzNDEuMTI4Mjg0NzJAc291cmNlZm91ci5hbmRyZXcuY211LmVkdT4=\r\n"}}},
			[]string{"dial", op("cram", "tim", "tanstaaftanstaaf"), op("mail", "tim@example.com"), "quit"}},
		{"cram-two-rounds", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}, "AUTH": {"334 Y2hhbGxlbmdl\r\n"},
			"+": {"334 bW9yZQ==\r\n", "235 ok\r\n"}}},
			[]string{"dial", op("cram", "u", "s"), "quit"}},
		{"cram-bad-base64", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}, "AUTH": {"334 not*base64\r\n"}, "+": {"501 cancelled\r\n"}}},
			[]string{"dial", op("cram", "u", "s"), "noop"}},
		{"cram-bad-base64-2", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}, "AUTH": {"334 YWJj=\r\n"}}},
			[]string{"dial", op("cram", "u", "s")}},
		{"cram-rejected", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}, "AUTH": {"334 Y2hhbGxlbmdl\r\n"},
			"+": {"535-5.7.8 Username and Password\r\n535 5.7.8 not accepted\r\n"}}},
			[]string{"dial", op("cram", "u", "s"), "noop"}},
		{"auth-odd-code", smtpScript{replies: map[string][]string{"EHLO": {ehloFull}, "AUTH": {"432 4.7.12 password transition needed\r\n"}}},
			[]string{"dial", op("cram", "u", "s"), "quit"}},
		{"starttls", smtpScript{replies: map[string][]string{"EHLO": {ehloTLS, "250-mx.example\r\n250-AUTH PLAIN\r\n250 SMTPUTF8\r\n"}}},
			[]string{"dial", "tlsstate", op("ext", "STARTTLS"), op("starttls", "example.com"), "tlsstate", op("ext", "STARTTLS"),
				op("ext", "SMTPUTF8"), op("plain", "", "user", "secret", "127.0.0.1"), op("mail", "ü@example.com"),
				op("rcpt", "b@example.com"), op("data", body), "quit"}},
		{"starttls-plain-remote", smtpScript{replies: map[string][]string{"EHLO": {ehloTLS, ehloPlain}}},
			[]string{op("connect", "example.com"), op("plain", "", "u", "p", "example.com"), op("starttls", "example.com"),
				op("plain", "", "u", "p", "example.com"), "quit"}},
		{"starttls-refused", smtpScript{replies: map[string][]string{"EHLO": {ehloTLS}, "STARTTLS": {"454 4.7.0 TLS not available\r\n"}}},
			[]string{"dial", op("starttls", "example.com"), "tlsstate", "noop", "quit"}},
		{"starttls-bad-name", smtpScript{replies: map[string][]string{"EHLO": {ehloTLS}}},
			[]string{"dial", op("starttls", "other.example"), "noop"}},
		{"rcpt-codes", smtpScript{replies: map[string][]string{"EHLO": {ehloNoAuth},
			"RCPT": {"251 2.1.5 forwarded\r\n", "252 2.1.5 cannot verify\r\n", "550 5.1.1 no such user\r\n", "450-4.2.1 try\r\n450 4.2.1 later\r\n"}}},
			[]string{"dial", op("mail", "a@b"), op("rcpt", "1@x"), op("rcpt", "2@x"), op("rcpt", "3@x"), op("rcpt", "4@x"),
				op("rcpt", "a\nb"), op("mail", "x\ry"), op("vrfy", "a\r\n"), "quit"}},
		{"data-errors", smtpScript{replies: map[string][]string{"DATA": {"554 5.5.1 no valid recipients\r\n"}, ".": {"552 5.3.4 too big\r\n"}}},
			[]string{"dial", op("mail", "a@b"), op("data", "x"), op("data", ""), op("data", "abc\r\n"), op("data", "abc\n"),
				op("data", ".\n."), "quit"}},
		{"malformed", smtpScript{replies: map[string][]string{"MAIL": {"hello\r\n", "25x ok\r\n", "2500 ok\r\n", "250-a\r\nb\r\n250 c\r\n",
			"250-multi\r\n251 other\r\n250 end\r\n"}, "NOOP": {"099 x\r\n"}}},
			[]string{"dial", op("mail", "1@x"), op("mail", "2@x"), op("mail", "3@x"), op("mail", "4@x"), op("mail", "5@x"), "noop", "quit"}},
		{"verify", smtpScript{replies: map[string][]string{"VRFY": {"252 2.1.5 cannot VRFY\r\n", "250 <x@y>\r\n", "550 no\r\n"}}},
			[]string{"dial", op("vrfy", "x"), op("vrfy", "x@y"), op("vrfy", "z"), "quit"}},
		{"greeting-error", smtpScript{greet: "554 5.3.2 go away\r\n"}, []string{"dial", "noop"}},
		{"greeting-multiline", smtpScript{greet: "220-mx.example\r\n220-welcome\r\n220 ready\r\n"}, []string{"dial", "noop", "quit"}},
		{"greeting-close", smtpScript{greet: "CLOSE"}, []string{"dial"}},
		{"server-closes", smtpScript{replies: map[string][]string{"MAIL": {"CLOSE"}}},
			[]string{"dial", op("mail", "a@b"), op("rcpt", "c@d"), "noop", "quit", "close"}},
		{"close-in-ehlo", smtpScript{replies: map[string][]string{"EHLO": {"CLOSE"}}},
			[]string{"dial", "noop", op("mail", "a@b")}},
		{"quit-then", smtpScript{}, []string{"dial", "noop", "quit", "noop", "quit", "close"}},
		{"quit-refused", smtpScript{replies: map[string][]string{"QUIT": {"500 no\r\n"}}}, []string{"dial", "quit", "noop", "quit"}},
		{"ext-dups", smtpScript{replies: map[string][]string{"EHLO": {"250-mx\r\n250-SIZE 1\r\n250-SIZE 2\r\n250-AUTH\r\n250-x-Odd  two spaces\r\n250 AUTH LOGIN\r\n"}}},
			[]string{"dial", op("ext", "size"), op("ext", "auth"), op("ext", "X-ODD"), op("ext", "x-odd"), op("ext", ""),
				op("cram", "u", "s"), "quit"}},
		{"ehlo-oneline", smtpScript{replies: map[string][]string{"EHLO": {"250 mx.example\r\n"}}},
			[]string{"dial", op("ext", "mx.example"), op("mail", "a@b"), "quit"}},
		{"sendmail", smtpScript{replies: map[string][]string{"EHLO": {ehloPlain}}},
			[]string{sendmailOp(authSpec("plain", "", "user", "pw", "127.0.0.1"), "a@example.com", []string{"b@example.com", "c@example.com"}, body)}},
		{"sendmail-cram", smtpScript{replies: map[string][]string{"EHLO": {ehloFull[:0] + "250-mx\r\n250 AUTH CRAM-MD5\r\n"}, "AUTH": {"334 Y2hhbGxlbmdl\r\n"}}},
			[]string{sendmailOp(authSpec("cram", "u", "s"), "a@b", []string{"c@d"}, "hi")}},
		{"sendmail-noauth", smtpScript{replies: map[string][]string{"EHLO": {ehloNoAuth}}},
			[]string{sendmailOp(authSpec("cram", "u", "s"), "a@b", []string{"c@d"}, "hi")}},
		{"sendmail-helo", smtpScript{replies: map[string][]string{"EHLO": {"500 what\r\n"}}},
			[]string{sendmailOp(authSpec("cram", "u", "s"), "a@b", []string{"c@d"}, "hi\n")}},
		{"sendmail-rcpt", smtpScript{replies: map[string][]string{"RCPT": {"250 ok\r\n", "550 no\r\n"}}},
			[]string{sendmailOp("none", "a@b", []string{"c@d", "e@f", "g@h"}, "hi")}},
		{"sendmail-badline", smtpScript{}, []string{sendmailOp("none", "a@b", []string{"c\nd"}, "hi"), sendmailOp("none", "a\r", []string{"x"}, "")}},
		{"sendmail-data", smtpScript{replies: map[string][]string{".": {"451 local error\r\n"}}},
			[]string{sendmailOp("none", "a@b", []string{"c@d"}, ".x\r\n..y\n")}},
		{"sendmail-starttls-untrusted", smtpScript{replies: map[string][]string{"EHLO": {ehloTLS}}},
			[]string{sendmailOp("none", "a@b", []string{"c@d"}, "hi")}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			gs := startSMTPServer(t, tc.script, srvCfg)
			gout := runGoSMTP(gs.ln.Addr().String(), tc.ops, roots)
			gtr := gs.transcript()
			cs := startSMTPServer(t, tc.script, srvCfg)
			cout := runCSMTP(t, exe, cs.ln.Addr().String(), p.caPEM, tc.ops)
			ctr := cs.transcript()
			if strings.Join(gout, "\n") != strings.Join(cout, "\n") {
				t.Errorf("results differ:\n got %q\nwant %q", cout, gout)
			}
			if strings.Join(gtr, "\n") != strings.Join(ctr, "\n") {
				t.Errorf("transcripts differ:\n got %q\nwant %q", ctr, gtr)
			}
			if testing.Verbose() {
				t.Logf("results %q\ntranscript %q", gout, gtr)
			}
		})
	}
}
