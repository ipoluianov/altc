package tests

import (
	"bufio"
	"bytes"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	htmltemplate "html/template"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"testing"
	"text/template"
)

// The differential tests of text/template and html/template: Go executes
// on the value json.Unmarshal gives into an any, which is what the C side
// gets as an alt_json tree.

type tmplCase struct {
	html  bool
	opts  string   // ";"-separated: mk=..., ld=..., rd=...
	texts []string // parsed one after the other
	data  string   // JSON, "-" for nil
	name  string   // the template to execute, "" for the first
}

func (c tmplCase) line() string {
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	kind := "text"
	if c.html {
		kind = "html"
	}
	d := "-"
	if c.data != "-" {
		d = h(c.data)
	}
	return kind + "\t" + h(c.opts) + "\t" + h(strings.Join(c.texts, "\x00")) + "\t" + d + "\t" + h(c.name)
}

func tmplEsc(s string) string {
	return strings.NewReplacer(`\`, `\\`, "\n", `\n`, "\r", `\r`).Replace(s)
}

// jsonify turns a function's argument into what the C side gets: JSON
func jsonify(v any) any {
	b, err := json.Marshal(v)
	if err != nil {
		return nil
	}
	var r any
	json.Unmarshal(b, &r)
	return r
}

func tmplFuncs() map[string]any {
	safe := func(f func(string) any) func(any) (any, error) {
		return func(a any) (any, error) {
			s, ok := jsonify(a).(string)
			if !ok {
				return nil, errors.New("safe: not a string")
			}
			return f(s), nil
		}
	}
	return map[string]any{
		"add": func(a, b any) (any, error) {
			x, ok1 := jsonify(a).(float64)
			y, ok2 := jsonify(b).(float64)
			if !ok1 || !ok2 {
				return nil, errors.New("add: bad arg")
			}
			return x + y, nil
		},
		"echo":  func(a any) (any, error) { return jsonify(a), nil },
		"first": func(a any, rest ...any) (any, error) { return jsonify(a), nil },
		"fail":  func(args ...any) (any, error) { return nil, errors.New("boom") },
		"nilf":  func() (any, error) { return nil, nil },
		"join": func(args ...any) (any, error) {
			var s []string
			for _, a := range args {
				x, ok := jsonify(a).(string)
				if !ok {
					return nil, errors.New("join: not a string")
				}
				s = append(s, x)
			}
			return strings.Join(s, ","), nil
		},
		"list": func(args ...any) (any, error) {
			r := []any{}
			for _, a := range args {
				r = append(r, jsonify(a))
			}
			return r, nil
		},
		"dict": func(args ...any) (any, error) {
			m := map[string]any{}
			for i := 0; i+1 < len(args); i += 2 {
				k, ok := jsonify(args[i]).(string)
				if !ok {
					return nil, errors.New("dict: key not a string")
				}
				m[k] = jsonify(args[i+1])
			}
			return m, nil
		},
		"safeCSS":    safe(func(s string) any { return htmltemplate.CSS(s) }),
		"safeHTML":   safe(func(s string) any { return htmltemplate.HTML(s) }),
		"safeAttr":   safe(func(s string) any { return htmltemplate.HTMLAttr(s) }),
		"safeJS":     safe(func(s string) any { return htmltemplate.JS(s) }),
		"safeJSStr":  safe(func(s string) any { return htmltemplate.JSStr(s) }),
		"safeURL":    safe(func(s string) any { return htmltemplate.URL(s) }),
		"safeSrcset": safe(func(s string) any { return htmltemplate.Srcset(s) }),
	}
}

func goTemplate(c tmplCase) (res string) {
	defer func() {
		if r := recover(); r != nil {
			res = fmt.Sprintf("PANIC: %v", r)
		}
	}()
	var data any
	if c.data != "-" {
		if err := json.Unmarshal([]byte(c.data), &data); err != nil {
			data = nil
		}
	}
	var ld, rd, mk string
	for _, o := range strings.Split(c.opts, ";") {
		switch {
		case strings.HasPrefix(o, "mk="):
			mk = o[3:]
		case strings.HasPrefix(o, "ld="):
			ld = o[3:]
		case strings.HasPrefix(o, "rd="):
			rd = o[3:]
		}
	}
	var out bytes.Buffer
	var err error
	if c.html {
		t := htmltemplate.New("t").Funcs(htmltemplate.FuncMap(tmplFuncs()))
		if mk != "" {
			t.Option("missingkey=" + mk)
		}
		if ld != "" || rd != "" {
			t.Delims(ld, rd)
		}
		for _, text := range c.texts {
			if _, err = t.Parse(text); err != nil {
				return "P:" + tmplEsc(err.Error())
			}
		}
		if c.name != "" {
			err = t.ExecuteTemplate(&out, c.name, data)
		} else {
			err = t.Execute(&out, data)
		}
	} else {
		t := template.New("t").Funcs(template.FuncMap(tmplFuncs()))
		if mk != "" {
			t.Option("missingkey=" + mk)
		}
		if ld != "" || rd != "" {
			t.Delims(ld, rd)
		}
		for _, text := range c.texts {
			if _, err = t.Parse(text); err != nil {
				return "P:" + tmplEsc(err.Error())
			}
		}
		if c.name != "" {
			err = t.ExecuteTemplate(&out, c.name, data)
		} else {
			err = t.Execute(&out, data)
		}
	}
	res = "O:" + tmplEsc(out.String())
	if err != nil {
		res += " E:" + tmplEsc(err.Error())
	}
	return res
}

func runTmplCases(t *testing.T, cases []tmplCase) {
	t.Helper()
	var in, want []string
	var kept []tmplCase
	skipped := 0
	for _, c := range cases {
		w := goTemplate(c)
		// the C side limits the nesting of templates to 1000 (Go: 100000)
		if strings.Contains(w, "exceeded maximum template depth (100000)") {
			skipped++
			continue
		}
		kept = append(kept, c)
		in = append(in, c.line())
		want = append(want, w)
	}
	cases = kept
	got := diffRun(t, "template", in)
	bad := 0
	for i := range want {
		if got[i] != want[i] {
			bad++
			if bad <= 25 {
				c := cases[i]
				t.Errorf("html=%v opts=%q texts=%q data=%s name=%q:\n got %q\nwant %q", c.html, c.opts, c.texts, c.data, c.name, got[i], want[i])
			}
		}
	}
	if bad > 25 {
		t.Errorf("... %d differences in %d cases", bad, len(want))
	}
	t.Logf("%d cases (%d skipped for the depth limit)", len(want), skipped)
}

// a data value with something of every kind
const tmplData = `{"a":1,"b":"two","c":[1,2,3],"d":{"x":"X","y":{"z":"deep"}},"e":null,"f":false,"t":true,"z":0,"s":"",` +
	`"arr":[],"obj":{},"n":-2.5,"big":1e21,"small":1e-7,"html":"<a href='x'>&</a>","strs":["b","a","c"],` +
	`"m":{"k2":"v2","k1":"v1","k3":"v3"},"nested":[{"n":"one","v":[1]},{"n":"two","v":[]}],"u":"héllo, 世界","q":"\"quoted\"\n"}`

func hand(texts ...string) tmplCase { return tmplCase{texts: texts, data: tmplData} }

// data for the templates of Go's own tests (after exec_test.go's tVal
// and escape_test.go's data)
const tmplGoData = `{"True":true,"I":17,"U16":16,"X":"x","S":"xyz","U":{"V":"v"},"V0":{"j":6666},"V1":{"j":7777},` +
	`"W0":{"k":888},"W1":{"k":999},"SI":[3,4,5],"SICap":[0,0,0,0,0],"AI":[3,4,5],"PAI":[3,4,5],"SB":[true,false],` +
	`"MSI":{"one":1,"two":2,"three":3},"MSIone":{"one":1},"MXI":{"one":1},"MII":{"1":1},` +
	`"SMSI":[{"one":1,"two":2},{"eleven":11,"twelve":12}],"Empty0":null,"Empty1":3,"Empty2":"empty2","Empty3":[7,8],` +
	`"Empty4":{"V":"UinEmpty"},"NonEmptyInterface":{"X":"x"},"Str":"foozle","Err":"erroozle","PI":23,"PS":"a string",` +
	`"PSI":[21,22,23],"Tmpl":"test template","MSIEmpty":{},"SIEmpty":[],"F":false,"T":true,"C":"<Cincinnati>",` +
	`"G":"<Goodbye>","H":"<Hello>","A":["<a>","<b>"],"E":[],"N":42,"Z":null,"W":"&iexcl;<b class=\"foo\">Hello</b>",` +
	`"Items":[1,2],"V":"v\"'<>&","FloatZero":0,"ComplexZero":0,"UPI":23,"Ptr":"x"}`

// loadTmplCorpus reads one Go-quoted template text per line
func loadTmplCorpus(t *testing.T, name string) []string {
	f, err := os.Open("testdata/template/" + name)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()
	var out []string
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1<<20), 1<<20)
	for sc.Scan() {
		s, err := strconv.Unquote(sc.Text())
		if err != nil {
			t.Fatalf("%s: %v", sc.Text(), err)
		}
		out = append(out, s)
	}
	return out
}

func TestDiffTemplate(t *testing.T) {
	var cases []tmplCase
	for _, s := range tmplHandCases {
		cases = append(cases, hand(s))
	}
	for _, s := range tmplHandCases {
		cases = append(cases, tmplCase{texts: []string{s}, data: "-"}, tmplCase{opts: "mk=error", texts: []string{s}, data: tmplData},
			tmplCase{opts: "mk=zero", texts: []string{s}, data: tmplData})
	}
	cases = append(cases, tmplMoreCases()...)
	for _, s := range loadTmplCorpus(t, "go_text_cases.txt") {
		for _, d := range []string{tmplGoData, "-", `"str"`, `[1,"a",null]`} {
			cases = append(cases, tmplCase{texts: []string{s}, data: d})
		}
	}
	cases = append(cases, tmplRandomCases(rand.New(rand.NewSource(7001)), 12000)...)
	cases = append(cases, tmplSoupCases(rand.New(rand.NewSource(7003)), 6000)...)
	runTmplCases(t, cases)
}

var tmplHandCases = []string{
	"", "hello", "{{.a}}", "{{.b}}", "{{.c}}", "{{.d}}", "{{.d.x}}", "{{.d.y.z}}", "{{.e}}", "{{.f}}", "{{.t}}", "{{.nope}}",
	"{{.nope.x}}", "{{.e.x}}", "{{.a.x}}", "{{.}}", "{{.n}}", "{{.big}}", "{{.small}}", "{{.html}}", "{{.u}}", "{{.q}}",
	"{{1}}", "{{1.5}}", "{{1e3}}", "{{0x10}}", "{{'a'}}", "{{1+2i}}", "{{\"s\"}}", "{{`raw`}}", "{{true}}", "{{nil}}",
	"{{range .c}}{{.}},{{end}}", "{{range $i, $e := .c}}{{$i}}={{$e}};{{end}}", "{{range .m}}{{.}}{{end}}",
	"{{range $k, $v := .m}}{{$k}}:{{$v}} {{end}}", "{{range .arr}}x{{else}}empty{{end}}", "{{range .nope}}x{{else}}none{{end}}",
	"{{range 3}}{{.}}{{end}}", "{{range $i := 3}}{{$i}}{{end}}", "{{range $i, $j := 3}}{{end}}", "{{range .a}}{{end}}",
	"{{range .b}}{{end}}", "{{range .c}}{{if eq . 2.0}}{{break}}{{end}}{{.}}{{end}}", "{{range .c}}{{if eq . 2.0}}{{continue}}{{end}}{{.}}{{end}}",
	"{{if .t}}yes{{else}}no{{end}}", "{{if .f}}yes{{else}}no{{end}}", "{{if .z}}y{{else if .s}}s{{else}}n{{end}}",
	"{{with .d}}{{.x}}{{end}}", "{{with .nope}}x{{else}}no{{end}}", "{{with .e}}x{{else with .d}}{{.x}}{{end}}",
	"{{$x := 1}}{{$x}}{{$x = 2}}{{$x}}", "{{$x := .d}}{{$x.y.z}}", "{{$}}", "{{$.a}}", "{{$nope}}",
	"{{define \"a\"}}A{{.}}{{end}}{{template \"a\" .b}}", "{{template \"nope\"}}", "{{block \"b\" .a}}B{{.}}{{end}}",
	"{{- 1 -}} {{- 2}}", "a  {{- 1 -}}  b", "{{/* comment */}}x", "{{- /* c */ -}} x", "{{/* unclosed",
	"{{len .c}}", "{{len .m}}", "{{len .b}}", "{{len .a}}", "{{len .e}}", "{{len .nope}}", "{{len nil}}",
	"{{index .c 1}}", "{{index .c 5}}", "{{index .c 3}}", "{{index .m \"k1\"}}", "{{index .m \"zz\"}}", "{{index .d \"y\" \"z\"}}",
	"{{index .b 0}}", "{{index .c .a}}", "{{index nil 1}}", "{{index .e 1}}", "{{index .a 1}}", "{{index .m 1}}", "{{index .m nil}}",
	"{{slice .c 1}}", "{{slice .c 1 2}}", "{{slice .c 1 2 3}}", "{{slice .c 0 4}}", "{{slice .c 0 5}}", "{{slice .b 1}}",
	"{{slice .b 1 2 3}}", "{{slice .c 2 1}}", "{{slice .a}}", "{{slice nil}}", "{{slice .c 1 2 3 4}}", "{{slice .c .a}}", "{{slice .b .e}}", "{{slice .c (index .c 0)}}", "{{slice .b (len .b)}}",
	"{{eq 1 1}}", "{{eq 1 2}}", "{{eq .a 1.0}}", "{{eq .a 1}}", "{{eq .b \"two\"}}", "{{eq .c .c}}", "{{eq .m .c}}",
	"{{eq .e nil}}", "{{eq .nope .e}}", "{{eq 1 2 3 1}}", "{{eq 1}}", "{{ne 1 2}}", "{{lt 1 2}}", "{{lt .a 2.0}}", "{{lt .a 2}}",
	"{{lt \"a\" \"b\"}}", "{{lt true false}}", "{{le 2 2}}", "{{gt 3 2}}", "{{ge 1 2}}", "{{lt .c 1}}", "{{lt nil 1}}",
	"{{eq (index \"abc\" 0) 97}}", "{{lt (index \"abc\" 0) 98}}", "{{index \"abc\" 1}}",
	"{{and 1 0}}", "{{and 1 2}}", "{{or 0 \"\"}}", "{{or 0 3}}", "{{and}}", "{{not 0}}", "{{not .c}}", "{{or .nope .b}}",
	"{{print 1 2}}", "{{print \"a\" \"b\"}}", "{{print 1 \"a\" 2}}", "{{print .c .m}}", "{{println 1 2}}", "{{print nil}}",
	"{{printf \"%d\" 3}}", "{{printf \"%d\" .a}}", "{{printf \"%v\" .a}}", "{{printf \"%5.2f\" .n}}", "{{printf \"%x\" .b}}",
	"{{printf \"%q\" .b}}", "{{printf \"%s\" .c}}", "{{printf \"%v\" .m}}", "{{printf \"%#v\" .m}}", "{{printf \"%+v\" .d}}",
	"{{printf \"%T\" .a}}", "{{printf \"%T\" .c}}", "{{printf \"%T\" .e}}", "{{printf \"%T\" 1}}", "{{printf \"%d %d\" 1}}",
	"{{printf \"%d\" 1 2}}", "{{printf \"%!\"}}", "{{printf \"%\"}}", "{{printf \"%z\" 1}}", "{{printf \"%c\" 65}}",
	"{{printf \"%U\" 0x1F600}}", "{{printf \"%#U\" 'x'}}", "{{printf \"%08.3f\" 3.14159}}", "{{printf \"%-8d|\" 42}}",
	"{{printf \"%+d\" 5}}", "{{printf \"%e\" 1234.5678}}", "{{printf \"%g\" 1e21}}", "{{printf \"%x\" 255}}", "{{printf \"%X\" -255}}",
	"{{printf \"%o %O %b\" 8 8 5}}", "{{printf \"%#x\" 255}}", "{{printf \"%x\" 1.5}}", "{{printf \"%b\" 1.0}}", "{{printf \"%t\" true}}",
	"{{printf \"%*d\" 5 3}}", "{{printf \"%-*d|\" 5 3}}", "{{printf \"%.*f\" 2 3.14159}}", "{{printf \"%[2]d %[1]d\" 1 2}}",
	"{{printf \"%[3]d\" 1 2}}", "{{printf \"%d\" nil}}", "{{printf \"%s\" nil}}", "{{printf \"%v\" .e}}", "{{printf \"%d\" .c}}",
	"{{printf \"%v\" (list 1 nil \"x\")}}", "{{printf \"%#v\" (list 1 nil \"x\")}}", "{{printf \"%x\" \"hello\"}}", "{{printf \"% x\" \"hi\"}}",
	"{{printf \"%#q\" \"back`q\"}}", "{{printf \"%#q\" \"plain\"}}", "{{printf \"%+q\" .u}}", "{{printf \"%q\" 'x'}}", "{{printf \"%5s|%-5s|\" \"a\" \"b\"}}",
	"{{printf \"%.2s\" .u}}", "{{printf \"%v\" 1+2i}}", "{{printf \"%.1f\" 1+2i}}", "{{printf \"%5t\" true}}", "{{printf \"%05d\" -3}}",
	"{{html .html}}", "{{js .html}}", "{{urlquery .html}}", "{{html 1 2}}", "{{html nil}}", "{{js .u}}", "{{urlquery .u \"x y\"}}",
	"{{.b | printf \"%s!\"}}", "{{.a | add 1}}", "{{add 1 2}}", "{{add 1}}", "{{add 1 \"x\"}}", "{{echo .d}}", "{{(echo .d).x}}",
	"{{fail}}", "{{nilf}}", "{{join \"a\" \"b\"}}", "{{join}}", "{{list 1 2}}", "{{dict \"a\" 1}}", "{{(dict \"a\" .d).a.x}}",
	"{{first}}", "{{first 1 2 3}}", "{{call .a}}", "{{call nil}}", "{{call}}", "{{nope}}", "{{.a 1}}", "{{1 2}}", "{{. 1}}",
	"{{.a | .b}}", "{{1 | 2}}", "{{(1)}}", "{{(.d).x}}", "{{(.d.y).z}}", "{{(nil).x}}", "{{\"x\".y}}", "{{$x := 1 | 2}}",
	"{{if}}", "{{range}}", "{{end}}", "{{else}}", "{{if 1}}", "{{if 1}}{{else}}{{else}}{{end}}", "{{with $x := 1}}{{$x}}{{end}}{{$x}}",
	"{{break}}", "{{range .c}}{{break 1}}{{end}}", "{{.a.}}", "{{.a b}}", "{{$x}}", "{{$x := }}", "{{(}}", "{{)}}", "{{\"x}}",
	"{{`x}}", "{{'x}}", "{{'ab'}}", "{{''}}", "{{3k}}", "{{0x}}", "{{1e}}", "{{99999999999999999999}}", "{{-0}}", "{{+5}}",
	"{{0b101}}", "{{0o17}}", "{{017}}", "{{1_000}}", "{{0x1p4}}", "{{1i}}", "{{.5}}", "{{-.5}}", "{{#}}", "{{.a#}}", "{{@}}",
	"{{define \"x\"}}a{{end}}{{define \"x\"}}b{{end}}", "{{define \"x\"}}{{end}}{{define \"x\"}}b{{end}}{{template \"x\"}}",
	"{{define x}}{{end}}", "{{define \"x\" 1}}{{end}}", "{{template x}}", "{{template \"x\" 1 2}}", "{{block \"x\"}}{{end}}",
	"line1\n{{.nope.x.y}}\n{{.a.b}}", "a\nb\n  {{index .c 9}}", "{{if .t}}\n{{end", "{{range .c}}\n{{.x}}{{end}}",
	"{{.d.y.z.w}}", "{{$.d.y}}", "{{$v := .c}}{{range $v}}{{.}}{{end}}", "{{range $i, $x := .nested}}{{$i}}{{$x.n}}{{len $x.v}}{{end}}",
	"{{with .d}}{{$.a}}{{.x}}{{end}}", "{{range .c}}{{$}}{{end}}", "{{0x1F11+2i}}", "{{0b1+1i}}", "{{1_0+2i}}", "{{1e400+1i}}",
	"{{1+.i}}", "{{0x1p2+1i}}", "{{1__0+1i}}", "{{0o7+1i}}", "{{1+0x2i}}", "{{-1-1i}}", "{{1e+2i}}", "{{09+1i}}", "{{1e400}}",
	"{{0x1p-1074}}", "{{-9223372036854775808}}", "{{9223372036854775808}}", "{{18446744073709551616}}", "{{-0x8000000000000000}}",
	"{{1.0}}", "{{1e0}}", "{{'\\x41'}}", "{{'\\u00e9'}}", "{{'\\377'}}", "{{'\\400'}}", "{{'é'}}", "{{'\\''}}", "{{'\\\"'}}",
	"{{\"\\'\"}}", "{{\"\\x\"}}", "{{\"\\u00e9\\U0001F600\\n\\t\\\\\"}}", "{{\"\xff\"}}", "{{`a\rb`}}", "{{.d.x | len}}", "{{len .d.x | printf \"%03d\"}}",
}

// cases with several texts, other delimiters, options and named templates
func tmplMoreCases() []tmplCase {
	d := tmplData
	return []tmplCase{
		{texts: []string{`{{define "a"}}A{{end}}`, `{{template "a"}}`}, data: d},
		{texts: []string{`{{define "a"}}A{{end}}x`, `{{define "a"}}B{{end}}`}, data: d, name: "a"},
		{texts: []string{`{{define "a"}}A{{end}}`, `   `}, data: d},
		{texts: []string{`main`, `{{define "a"}}A{{end}}`}, data: d},
		{texts: []string{`main`, ` `}, data: d},
		{texts: []string{`{{define "a"}}A{{.}}{{end}}`}, data: d, name: "a"},
		{texts: []string{`{{define "a"}}A{{.b}}{{end}}`}, data: d, name: "b"},
		{texts: []string{`{{define "a"}}{{.x.y}}{{end}}{{template "a" .}}`}, data: d},
		{texts: []string{"x\n{{define \"a\"}}\n\n{{.x.y}}{{end}}{{template \"a\" .d}}"}, data: d},
		{texts: []string{`{{block "a" .}}[{{.b}}]{{end}}`, `{{define "a"}}({{.a}}){{end}}`}, data: d},
		{texts: []string{`{{define "t"}}T{{end}}`}, data: d},
		{texts: []string{`{{define "t"}}T{{end}}rest`}, data: d},
		{texts: []string{`{{template "a"}}`}, data: d},
		{texts: []string{`{{define "a"}}{{template "b"}}{{end}}{{define "b"}}B{{.}}{{end}}{{template "a" 5}}`}, data: d},
		{texts: []string{`{{define "a"}}{{.}}{{end}}{{range .c}}{{template "a" .}}{{end}}`}, data: d},
		{texts: []string{`{{define "%d"}}{{.nope.x}}{{end}}{{template "%d" .}}`}, data: d},
		{opts: "ld=[[;rd=]]", texts: []string{`[[.a]] {{.a}} [[- .b -]] ]]`}, data: d},
		{opts: "ld=<<;rd=>>", texts: []string{`<<if .t>>y<<end>> <</* c */>> <<.x>`}, data: d},
		{opts: "ld=(;rd=)", texts: []string{`(.a) ((.b))`}, data: d},
		{opts: "ld=|;rd=|", texts: []string{`|.a| |len .c|`}, data: d},
		{opts: "ld={;rd=}", texts: []string{`{.a}{.d}`}, data: d},
		{opts: "ld=[[", texts: []string{`[[.a}}`}, data: d},
		{opts: "rd=]]", texts: []string{`{{.a]]`}, data: d},
		{opts: "mk=error", texts: []string{`{{.nope}}`}, data: d},
		{opts: "mk=error", texts: []string{`{{.d.nope}}`}, data: d},
		{opts: "mk=error", texts: []string{`{{.a}}`}, data: "-"},
		{opts: "mk=zero", texts: []string{`{{.nope}} {{printf "%v" .nope}} {{.d.nope.x}}`}, data: d},
		{opts: "mk=invalid", texts: []string{`{{.nope}}`}, data: d},
		{opts: "mk=default", texts: []string{`{{.nope}}`}, data: d},
		{texts: []string{`{{.}}`}, data: `"just a string"`},
		{texts: []string{`{{.}} {{len .}} {{index . 0}}`}, data: `[1, 2]`},
		{texts: []string{`{{.}} {{.x}}`}, data: `3`},
		{texts: []string{`{{.}} {{.x}}`}, data: `null`},
		{texts: []string{`{{range .}}{{.}}{{end}}`}, data: `{"b":1,"a":2,"c":{"x":1}}`},
		{texts: []string{`{{range $k, $v := .}}{{$k}}={{$v}};{{end}}`}, data: `{"b":1,"a":2,"a":3}`},
		{texts: []string{`{{.a}} {{len .}}`}, data: `{"a":1,"a":2}`},
		{texts: []string{`{{.}}`}, data: `{"a":[1,{"b":null}],"c":"d"}`},
		{texts: []string{`{{printf "%v|%+v|%#v|%s|%d|%q|%x" . . . . . . .}}`}, data: `{"a":[1,{"b":null}],"c":"d","e":true}`},
		{texts: []string{`{{printf "%v %d %s %x %X %o %e %E %f %F %g %G %b %c %q %U %t %T %p" . . . . . . . . . . . . . . . . . .}}`}, data: `12.5`},
		{texts: []string{`{{printf "%v %d %s %x %q %T %8.3v|%-8v|" . . . . . . . .}}`}, data: `"str"`},
		{texts: []string{`{{printf "%v %d %s %t %T %q" . . . . . .}}`}, data: `true`},
		{texts: []string{`{{printf "%v %5.1f %x %e %g" . . . . .}}`}, data: `[1.5, -2, 1e100, 1e-100, 123456789]`},
		{texts: []string{`{{range .}}{{printf "%v|%.3g|%10.4e|%-+8.2f|%08.2f|% .1f|%#g|%#.3x\n" . . . . . . . .}}{{end}}`},
			data: `[0, 1, -1, 0.5, 1e6, 1e21, 1e20, 123456.789, 1e-5, 1e-4, 3.14159, -0.0, 100, 255, 1.7976931348623157e308, 5e-324]`},
		{texts: []string{`{{range .}}{{.}} {{print .}} {{println .}}{{end}}`},
			data: `[0, 1, -1, 0.5, 1e6, 1e21, 1e20, 123456.789, 1e-5, 1e-4, 3.14159, 100, 255, 1.7976931348623157e308, 5e-324, 2e-7, 999999, 1234567]`},
		{texts: []string{`{{range .}}{{printf "%d|%x|%q|%c|%U|%5d|%-5d|%05d|%+d|% d|%#o|%b" . . . . . . . . . . . .}}{{end}}`},
			data: `[0]`},
		{texts: []string{`{{range $i, $_ := .}}{{printf "%d|%x|%X|%q|%c|%U|%#U|%5d|%-5d|%05d|%+d|% d|%#o|%O|%b|%08b|%.3d|%8.3d" $i $i $i $i $i $i $i $i $i $i $i $i $i $i $i $i $i $i}}{{end}}`},
			data: `[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]`},
		{texts: []string{`{{printf "%d %d %d %c %q %U" -9223372036854775807 9223372036854775807 -1 -1 -1 -1}}`}, data: d},
		{texts: []string{`{{printf "%c%c%c %q %q %+q %#q %x % X %#x" 0xD800 0x110000 0x1F600 0x7f 0xFFFD 0x1F600 0x1F600 "\xff" "ab" "ab"}}`}, data: d},
		{texts: []string{`{{printf "%s|%q|%x|%v|%+q" . . . . .}}`}, data: `"\u0000\u0007\t\u001f\u007f\u0080\u00a0\u200b\ufeff\ud7ff\ue000 é 世 😀"`},
		{texts: []string{`{{printf "%5.2q|%.0s|%10.3x|%-10x|%010s|%-010s|%#5q" . . . . . . .}}`}, data: `"hello world"`},
		{texts: []string{`{{printf "%v %v %v" 1+2i -1.5i 0i}} {{1+2i}} {{eq 1i 1i}} {{lt 1i 2i}}`}, data: d},
		{texts: []string{`{{printf "%[1]*[2]d|%[3]*.[2]*[1]f|%[0]d|%[x]d|%[2]*d|%.[2]d|%[5]d" 8 2 6.0 4}}`}, data: d},
		{texts: []string{`{{printf "%*d|%.*d|%*d" "x" 1 "y" 2 1.5 3}}`}, data: d},
		{texts: []string{`{{printf "%-*d|%*d|%.*d" -5 1 10000000 2 -1 3}}`}, data: d},
		{texts: []string{`{{printf "%[2]d %d %d" 1 2 3}} {{printf "%[1]d" 1 2}} {{printf "%3" 1}} {{printf "%-" 1}} {{printf "%.d" 0}}`}, data: d},
		{texts: []string{`{{printf "%w %v" 1 2}} {{printf "%!d %%d %🙂"}} {{printf "%é" 1}}`}, data: d},
		{texts: []string{`{{printf "%v" nil}} {{printf "%T" nil}} {{printf "%x" nil}} {{printf "%v %T" .e .e}} {{printf "" nil}}`}, data: d},
		{texts: []string{`{{printf 1}}`}, data: d},
		{texts: []string{`{{printf .a}}`}, data: d},
		{texts: []string{`{{printf .e}}`}, data: d},
		{texts: []string{`{{printf .nope}}`}, data: d},
		{texts: []string{`{{printf nil}}`}, data: d},
		{texts: []string{`{{printf (safeHTML "x%d") 1}}`}, data: d},
		{texts: []string{`{{printf "%T %v %s" (safeHTML "<b>") (safeJS "x") (safeURL "u")}} {{safeCSS "c"}}`}, data: d},
		{texts: []string{`{{html (safeHTML "<b>")}} {{len (safeHTML "<b>")}} {{eq (safeHTML "a") "a"}} {{index (safeHTML "ab") 1}}`}, data: d},
		{texts: []string{`{{print 1 2 "a" "b" 3 nil 4 .c .e "x" .nope}}`}, data: d},
		{texts: []string{`{{println}}{{print}}{{printf ""}}`}, data: d},
		{texts: []string{`{{html "a" 1 nil .e .nope}} {{js 1 "<" .c}} {{urlquery "a b&c=d/é" 2}}`}, data: d},
		{texts: []string{`{{js "  \u0000\u001f\ufeff\U0001F600 é \\ ' \" = & < >"}}`}, data: d},
		{texts: []string{`{{html "\u0000<>&'\"é"}}`}, data: d},
		{texts: []string{`{{urlquery "~-_.!*'();:@&=+$,/?#[] \u0000é"}}`}, data: d},
	}
}

// ---- random templates ----

type tmplGen struct {
	r    *rand.Rand
	vars []string
	loop int
	defs []string
}

func (g *tmplGen) pick(s ...string) string { return s[g.r.Intn(len(s))] }

func (g *tmplGen) data(depth int) string {
	r := g.r
	switch k := r.Intn(10); {
	case depth > 2 || k < 4:
		return g.pick("0", "1", "-1", "2.5", "1e6", "1e21", "1e-5", "3", "100", "0.1", "-0", "123456789", `""`, `"a"`, `"<b>"`,
			`"x y"`, `"é"`, `"\"q\""`, `"&"`, "true", "false", "null")
	case k < 7:
		var items []string
		for i := r.Intn(5); i > 0; i-- {
			items = append(items, g.data(depth+1))
		}
		return "[" + strings.Join(items, ",") + "]"
	default:
		var items []string
		for _, key := range []string{"a", "b", "c", "x", "y", "Z"} {
			if r.Intn(2) == 0 {
				items = append(items, strconv.Quote(key)+":"+g.data(depth+1))
			}
		}
		return "{" + strings.Join(items, ",") + "}"
	}
}

func (g *tmplGen) operand(depth int) string {
	r := g.r
	switch k := r.Intn(20); {
	case k < 6:
		f := g.pick(".a", ".b", ".c", ".x", ".y", ".Z", ".a.b", ".x.y", ".c.a", ".nope", ".a.x.y")
		return f
	case k < 8:
		if len(g.vars) > 0 && r.Intn(4) != 0 {
			v := g.vars[r.Intn(len(g.vars))]
			if r.Intn(3) == 0 {
				return v + g.pick(".a", ".b", ".x")
			}
			return v
		}
		return g.pick("$", "$.a", "$.c", "$x")
	case k < 9:
		return "."
	case k < 13:
		return g.pick("0", "1", "2", "-1", "3", "2.5", "1e3", "0x1f", "'a'", "1i", "1+2i", `"s"`, `""`, `"a b"`, "`r`", "true", "false", "nil", `"<x>"`, `"%d"`)
	case k < 16 && depth < 3:
		return "(" + g.pipe(depth+1) + ")"
	default:
		if depth < 3 {
			return "(" + g.call(depth+1) + ")" + g.pick("", "", ".a", ".x")
		}
		return "1"
	}
}

func (g *tmplGen) call(depth int) string {
	r := g.r
	fn := g.pick("len", "index", "slice", "eq", "ne", "lt", "le", "gt", "ge", "and", "or", "not", "print", "printf", "println",
		"html", "js", "urlquery", "add", "echo", "list", "dict", "join", "first", "call")
	n := r.Intn(4)
	switch fn {
	case "len", "not", "echo":
		n = 1 - r.Intn(10)/9
	case "ne", "lt", "le", "gt", "ge", "add":
		n = 2 - r.Intn(10)/9
	case "printf":
		f := g.pick(`"%v"`, `"%d"`, `"%s"`, `"%q"`, `"%x"`, `"%5.2f"`, `"%T"`, `"%v %v"`, `"%+v|%#v"`, `"%t"`, `"%c"`, `"%-4d|"`, `"%e"`, `"%g"`, `"%08.3f"`, `"%U"`, `"x"`, `"%"`)
		s := fn + " " + f
		for i := r.Intn(3); i > 0; i-- {
			s += " " + g.operand(depth)
		}
		return s
	}
	s := fn
	for i := 0; i < n; i++ {
		s += " " + g.operand(depth)
	}
	return s
}

func (g *tmplGen) pipe(depth int) string {
	r := g.r
	var s string
	if r.Intn(2) == 0 {
		s = g.operand(depth)
	} else {
		s = g.call(depth)
	}
	for r.Intn(4) == 0 {
		s += " | " + g.pick("len", "print", "printf \"%v\"", "html", "js", "urlquery", "not", "eq 1", "index .c", "slice", "echo", "and 1", "or 0", "add 1")
	}
	return s
}

func (g *tmplGen) action(depth int) string {
	r := g.r
	l, rd := "{{", "}}"
	if r.Intn(6) == 0 {
		l = "{{- "
	}
	if r.Intn(6) == 0 {
		rd = " -}}"
	}
	switch k := r.Intn(22); {
	case k < 8 || depth > 2:
		return l + g.pipe(0) + rd
	case k < 10:
		return l + "if " + g.pipe(0) + rd + g.body(depth+1) + g.pick("", "{{else}}"+g.body(depth+1), "{{else if "+g.pipe(0)+"}}"+g.body(depth+1)) + "{{end}}"
	case k < 12:
		return l + "with " + g.pipe(0) + rd + g.body(depth+1) + g.pick("", "{{else}}"+g.body(depth+1), "{{else with "+g.pipe(0)+"}}"+g.body(depth+1)) + "{{end}}"
	case k < 15:
		decl := g.pick("", "", "$e := ", "$i, $e := ", "$k, $v := ")
		nv := len(g.vars)
		if strings.Contains(decl, "$e") {
			g.vars = append(g.vars, "$e")
		}
		if strings.Contains(decl, "$i") {
			g.vars = append(g.vars, "$i")
		}
		if strings.Contains(decl, "$k") {
			g.vars = append(g.vars, "$k", "$v")
		}
		g.loop++
		s := l + "range " + decl + g.pipe(0) + rd + g.body(depth+1)
		g.loop--
		s += g.pick("", "{{else}}"+g.body(depth+1)) + "{{end}}"
		g.vars = g.vars[:nv]
		return s
	case k < 16 && g.loop > 0:
		return g.pick("{{break}}", "{{continue}}")
	case k < 17:
		g.vars = append(g.vars, "$x")
		return l + "$x := " + g.pipe(0) + rd
	case k < 18:
		return l + "/* " + g.pick("c", "", "x y") + " */" + rd
	case k < 19 && len(g.defs) > 0:
		return "{{template " + strconv.Quote(g.defs[r.Intn(len(g.defs))]) + g.pick("", " .", " .a", " $") + "}}"
	case k < 20 && len(g.vars) > 0:
		return l + g.vars[r.Intn(len(g.vars))] + " = " + g.pipe(0) + rd
	default:
		return l + g.pipe(0) + rd
	}
}

func (g *tmplGen) body(depth int) string {
	var s strings.Builder
	for i := g.r.Intn(4); i > 0; i-- {
		if g.r.Intn(3) == 0 {
			s.WriteString(g.pick("a", " ", "\n", "<", "x y", "  \n ", "é"))
		} else {
			s.WriteString(g.action(depth))
		}
	}
	return s.String()
}

func (g *tmplGen) template() string {
	g.vars = nil
	g.defs = nil
	var s strings.Builder
	if g.r.Intn(4) == 0 {
		name := g.pick("d1", "d2")
		g.defs = append(g.defs, name)
		s.WriteString("{{define " + strconv.Quote(name) + "}}" + g.body(1) + "{{end}}")
		g.vars = nil
	}
	s.WriteString(g.body(0))
	return s.String()
}

func tmplMutate(r *rand.Rand, s string) string {
	b := []byte(s)
	for k := r.Intn(3) + 1; k > 0 && len(b) > 0; k-- {
		i := r.Intn(len(b))
		switch r.Intn(3) {
		case 0:
			b = append(b[:i], b[i+1:]...)
		case 1:
			c := "{}()|$.\"`'-:=, 1ae\n"[r.Intn(19)]
			b = append(b[:i], append([]byte{c}, b[i:]...)...)
		default:
			b[i] = "{}()|$.\"`'-:=, 1ae\n"[r.Intn(19)]
		}
	}
	return string(b)
}

func tmplRandomCases(r *rand.Rand, n int) []tmplCase {
	g := &tmplGen{r: r}
	var cases []tmplCase
	for i := 0; i < n; i++ {
		text := g.template()
		if r.Intn(5) == 0 {
			text = tmplMutate(r, text)
		}
		c := tmplCase{texts: []string{text}, data: g.data(0)}
		switch r.Intn(10) {
		case 0:
			c.opts = "mk=error"
		case 1:
			c.opts = "mk=zero"
		case 2:
			c.data = "-"
		}
		cases = append(cases, c)
	}
	return cases
}

// ---- html/template ----

const tmplHTMLData = `{"a":"<b>\"x\" & 'y'</b>","u":"http://example.com/a b?c=d&e#f","j":"javascript:alert(1)","n":42,"f":-1.5,` +
	`"t":true,"z":null,"arr":[1,"two",null,{"k":"v"}],"obj":{"k":"</script>","l":[1,2]},"s":"","css":"red; background: url(x)",` +
	`"id":"main","ws":"a b\tc\nd","uni":"é\u2028\u2029\u0000世","sl":"/","q":"'\"` + "`" + `","dl":"${x}","cm":"*/<!--"}`

var tmplHTMLHand = []string{
	"{{.a}}", "<p>{{.a}}</p>", "<a href=\"{{.u}}\">x</a>", "<a href='{{.u}}'>", "<a href={{.u}}>", "<a href=\"/x?q={{.a}}\">",
	"<a href=\"{{.j}}\">", "<a href=\"#{{.a}}\">", "<a title=\"{{.a}}\">", "<a title='{{.a}}'>", "<a title={{.a}}>", "<a title={{.s}}>",
	"<a onclick=\"f({{.a}})\">", "<a onclick='f({{.n}}, {{.arr}}, {{.obj}})'>", "<a onclick=\"x='{{.a}}'\">",
	"<script>var x = {{.a}};</script>", "<script>var x = '{{.a}}';</script>", "<script>var x = \"{{.uni}}\";</script>",
	"<script>var x = {{.obj}}; var y = {{.arr}}; var z = {{.z}}; var t = {{.t}}; var f = {{.f}};</script>",
	"<script>var r = /{{.a}}/;</script>", "<script>var r = /x{{.s}}/;</script>", "<script>var s = `{{.dl}}`;</script>",
	"<script>var s = `a${ {{.a}} }b`;</script>", "<script>// {{.a}}\n</script>", "<script>/* {{.a}} */</script>",
	"<script>x = 1 / {{.n}} / 2</script>", "<script>x = a / {{.n}}</script>", "<script>x = ( / {{.n}}</script>",
	"<style>p { color: {{.css}} }</style>", "<style>p { color: \"{{.a}}\" }</style>", "<style>p { background: url({{.u}}) }</style>",
	"<style>p { background: url('{{.u}}') }</style>", "<style>/* {{.a}} */</style>", "<p style=\"color: {{.css}}\">",
	"<p style=\"color: {{.id}}\">", "<p style=\"background: url({{.u}})\">", "<p style='x: \"{{.a}}\"'>",
	"<img src=\"{{.u}}\">", "<img srcset=\"{{.u}} 1x, {{.u}} 2x\">", "<img srcset={{.u}}>", "<img src=\"data:{{.a}}\">",
	"<textarea>{{.a}}</textarea>", "<title>{{.a}}</title>", "<!-- {{.a}} -->", "<p {{.id}}>", "<p {{.a}}>", "<p {{.s}}>",
	"<p {{.id}}=\"x\">", "<p data-{{.id}}=\"x\">", "<input value=\"{{.a}}\" {{if .t}}checked{{end}}>", "<a href=\"{{if .t}}/a{{else}}/b{{end}}\">",
	"<a href=\"{{if .t}}/a?{{end}}{{.a}}\">", "{{if .t}}<a{{else}}<b{{end}}>", "{{if .t}}<a href=\"{{end}}x\">",
	"<a href=\"x{{.a}}\"{{.a}}>", "{{range .arr}}<li>{{.}}</li>{{end}}", "{{range .arr}}<a href=\"{{.}}\">{{end}}",
	"<script>{{range .arr}}{{.}},{{end}}</script>", "{{with .obj}}<b>{{.k}}</b>{{end}}", "<a href=\"{{.u | html}}\">",
	"{{.a | html}}", "{{html .a}}", "{{html .a .n}}", "{{.u | urlquery}}", "<a href=\"/?q={{.a | urlquery}}\">", "{{.a | html | print}}",
	"<a title={{.a | html}}>", "{{.a | printf \"%s\"}}", "{{printf \"<%s>\" .a}}", "{{.z}}", "{{.nope}}", "<a href=\"{{.nope}}\">",
	"<script>{{.nope}}</script>", "<a href=\"{{.n}}\">", "{{.n}} {{.f}} {{.t}} {{.arr}} {{.obj}}", "<p>{{safeHTML \"<b>ok</b>\"}}</p>",
	"<a title=\"{{safeHTML \"<b>x</b>\"}}\">", "<a href=\"{{safeURL \"javascript:ok()\"}}\">", "<script>x = {{safeJS \"a+b\"}}</script>",
	"<script>x = \"{{safeJSStr \"a\\\"b\"}}\"</script>", "<script>x = {{safeJSStr \"a\\\"b\"}}</script>", "<p style=\"{{safeCSS \"color: red\"}}\">",
	"<p {{safeAttr \"onclick=x\"}}>", "<img srcset=\"{{safeSrcset \"a.png 1x, b.png 2x\"}}\">", "<img srcset=\"{{safeURL \"a,b\"}}\">",
	"<a href=\"{{safeHTML \"x y\"}}\">", "<p title={{safeHTML \"<i>a b</i>\"}}>", "<textarea>{{safeHTML \"<b>x</b>\"}}</textarea>",
	"<a href='{{.ws}}'>", "<a title={{.ws}}>", "<a href=\"{{.uni}}\">", "<a title=\"{{.uni}}\">", "<script>'{{.uni}}'</script>",
	"<script>`{{.q}}`</script>", "<script>'{{.cm}}'</script>", "<script>{{.cm}}</script>", "<p>{{.cm}}</p>",
	"<a href=\"{{.a}}", "<a href=", "<script>", "<a", "<!--", "<p title=\"{{.a}}", "<script>var x = '{{.a}}", "<a \"x\">",
	"<a x\"y>", "<a b=c\"d\">", "<script>var x = /[{{.a}}/</script>", "<script>var x = 'a\\", "<a href=\"{{.u}}{{.u}}\">",
	"<a href=\"{{if .t}}{{.u}}?{{end}}{{.u}}\">", "{{define \"x\"}}<b>{{.}}</b>{{end}}{{template \"x\" .a}}",
	"{{define \"x\"}}{{.}}{{end}}<a href=\"{{template \"x\" .u}}\">{{template \"x\" .a}}", "{{define \"x\"}}<a href=\"{{end}}{{template \"x\"}}\">",
	"{{template \"nope\"}}", "<a href=\"{{.u}}\"{{range .arr}}{{.}}{{end}}>", "<script>{{range .arr}}{{break}}{{end}}</script>",
	"{{range .arr}}{{if .}}<a href=\"{{break}}{{end}}x{{end}}", "{{range .arr}}<a{{continue}}{{end}}", "<a href=\"{{.u}}\">{{.a}}</a><script>f({{.a}})</script>",
	"<button onclick=\"alert({{.a}})\">x</button>", "<div onmouseover='x({{.a}})'>", "<a href=\"javascript:{{.a}}\">",
	"<a xlink:href=\"{{.u}}\">", "<a xmlns:foo=\"{{.u}}\">", "<a data-src=\"{{.u}}\">", "<a data-onclick=\"{{.a}}\">",
	"<script type=\"text/template\">{{.a}}</script>", "<script type=\"text/javascript\">{{.a}}</script>", "<script type=\"module\">{{.a}}</script>",
	"<meta http-equiv=\"refresh\" content=\"0; url={{.u}}\">", "<meta content=\"{{.a}}\">", "<meta content=\"url={{.j}}\">",
	"<a style=\"x: url('{{.u}}')\">", "<style>@import '{{.u}}';</style>", "<style>a{b: '{{.a}}'}</style>", "<style>a{b:{{.n}}px}</style>",
	"<style>p{b:{{.f}}}</style>", "<style>{{.css}}</style>", "<!DOCTYPE html><p>{{.a}}", "a < b {{.a}} c > d", "<p>a<b</p>",
	"<script>if (a < b) { x = {{.a}} }</script>", "<script>a = \"</script>\"</script>", "<script>a = '<!--'; b = {{.a}}</script>",
	"<script>a = 1;<!-- comment\nb = {{.a}}</script>", "<script>x = {}/{{.n}}</script>", "<script>x = a.b/{{.n}}</script>",
	"<script>x = return /{{.a}}/</script>", "<script>x = typeof /{{.a}}/</script>", "<script>x++/{{.n}}</script>", "<script>x+/{{.a}}/</script>",
	"<script>{{.a}}</script><p>{{.a}}</p>", "<script>`${ {{.n}} + `${ {{.a}} }` }`</script>", "<script>`${ { a: {{.n}} } }`</script>",
	"<script>`a${b}c{{.a}}`</script>", "<script>#!x {{.a}}\ny</script>", "<script>x = 1 --> {{.a}}\ny</script>",
	"<a onclick=\"&quot;{{.a}}&quot;\">", "<a onclick=\"x = &#39;{{.a}}\">", "<a href=\"&#x6a;avascript:{{.a}}\">", "<a title=\"&amp;{{.a}}\">",
	"<a onclick='f(&quot;'{{.a}}>", "<a href=\"x&amp;y={{.a}}\">", "<p title=x{{.a}}y>", "<input type=text value={{.a}}>",
	"{{.a}}<br/>{{.a}}<br>", "<svg><a xlink:href=\"{{.u}}\"></a></svg>", "<iframe srcdoc=\"{{.a}}\">", "<form action=\"{{.u}}\">",
	"<a href=\"{{.sl}}{{.a}}\">", "<a href=\"?{{.a}}\">", "<a href=\"x#{{.a}}\">", "<a href=\"%{{.a}}\">", "<p class=\"{{.a}} {{.id}}\">",
	"{{/* c */}}<p>{{.a}}</p>", "{{- .a -}} <b>", "<a href='{{- .u -}}'>", "{{$x := .a}}<p>{{$x}}</p>", "{{with $x := .u}}<a href=\"{{$x}}\">{{end}}",
	"<script>var a = {{.a}}{{.n}};</script>", "<script>var a = [{{range .arr}}{{.}},{{end}}];</script>", "<a href=\"{{range .arr}}{{.}}{{end}}\">",
	"<a href=\"/{{range .arr}}{{.}}?{{end}}\">", "{{if .t}}<script>{{else}}<style>{{end}}", "<p {{if .t}}a=\"{{.a}}\"{{end}}>",
	"{{html}}", "{{urlquery}}", "{{.a | urlquery | html}}", "{{html .a | urlquery}}", "<a title={{html .a}}>", "<a href=\"{{urlquery .a}}\">",
}

func TestDiffTemplateHTML(t *testing.T) {
	var cases []tmplCase
	for _, s := range tmplHTMLHand {
		cases = append(cases, tmplCase{html: true, texts: []string{s}, data: tmplHTMLData})
		cases = append(cases, tmplCase{html: true, texts: []string{s}, data: "-"})
	}
	// the text/template cases through html/template
	for _, s := range tmplHandCases {
		cases = append(cases, tmplCase{html: true, texts: []string{s}, data: tmplData})
	}
	for _, s := range loadTmplCorpus(t, "go_html_cases.txt") {
		cases = append(cases, tmplCase{html: true, texts: []string{s}, data: tmplGoData}, tmplCase{html: true, texts: []string{s}, data: tmplHTMLData})
	}
	for _, s := range loadTmplCorpus(t, "go_text_cases.txt") {
		cases = append(cases, tmplCase{html: true, texts: []string{s}, data: tmplGoData})
	}
	cases = append(cases, tmplHTMLMoreCases()...)
	cases = append(cases, tmplHTMLRandomCases(rand.New(rand.NewSource(7002)), 10000)...)
	runTmplCases(t, cases)
}

func tmplHTMLMoreCases() []tmplCase {
	d := tmplHTMLData
	cs := []tmplCase{
		{html: true, texts: []string{`{{define "a"}}<b>{{.}}</b>{{end}}`, `{{template "a" .a}}`}, data: d},
		{html: true, texts: []string{`{{define "a"}}{{.}}{{end}}<a href="{{template "a" .u}}">`}, data: d, name: "a"},
		{html: true, texts: []string{`{{define "a"}}<a href="{{.}}">{{end}}`}, data: d, name: "a"},
		{html: true, texts: []string{`{{define "a"}}<a href="{{end}}`}, data: d, name: "a"},
		{html: true, texts: []string{`x`}, data: d, name: "nope"},
		{html: true, texts: []string{`{{define "a"}}A{{end}}`}, data: d},
		{html: true, texts: []string{`{{define "a"}}{{template "a" .}}{{end}}{{template "a"}}`}, data: d},
		{html: true, texts: []string{`{{define "a"}}<a{{template "a"}}{{end}}{{template "a"}}`}, data: d},
		{html: true, texts: []string{`{{define "a"}}{{if .}}<a href="{{template "a"}}">{{end}}{{end}}{{template "a" .t}}`}, data: d},
		{html: true, texts: []string{`{{define "a"}}{{.}}{{end}}<script>{{template "a" .a}}</script><p>{{template "a" .a}}</p>`}, data: d},
		{html: true, opts: "ld=[[;rd=]]", texts: []string{`<a href="[[.u]]">[[.a]]</a>`}, data: d},
		{html: true, opts: "mk=error", texts: []string{`<p>{{.nope}}</p>`}, data: d},
		{html: true, opts: "mk=zero", texts: []string{`<p>{{.nope}}</p>`}, data: d},
		{html: true, texts: []string{`{{block "b" .}}<i>{{.a}}</i>{{end}}`}, data: d},
		{html: true, texts: []string{`{{define "t"}}<a{{end}}`}, data: "-", name: "t"},
	}
	// after escape_test.go's TestEscapeSet: sets of templates run from "main"
	tree := `{"Children":[{"X":"foo"},{"X":"<bar>"},{"Children":[{"X":"baz"}]}]}`
	for _, set := range [][]string{
		{"main", ``},
		{"main", `Hello, {{template "helper"}}!`, "helper", `{{"<World>"}}`},
		{"main", `<a onclick='a = {{template "helper"}};'>`, "helper", `{{"<a>"}}<b`},
		{"main", `{{range .Children}}{{template "main" .}}{{else}}{{.X}} {{end}}`},
		{"main", `{{template "helper" .}}`, "helper", `{{if .Children}}<ul>{{range .Children}}<li>{{template "main" .}}</li>{{end}}</ul>{{else}}{{.X}}{{end}}`},
		{"main", `<blockquote>{{range .Children}}{{template "helper" .}}{{end}}</blockquote>`, "helper", `{{if .Children}}{{template "main" .}}{{else}}{{.X}}<br>{{end}}`},
		{"main", `<button onclick="title='{{template "helper"}}'; ...">{{template "helper"}}</button>`, "helper", `{{11}} of {{"<100>"}}`},
		{"main", `<script>var x={{template "helper"}}/{{"42"}};</script>`, "helper", "{{126}}"},
		{"main", `<script>var x=[{{template "countdown" 4}}];</script>`, "countdown", `{{.}}{{if .}},{{template "countdown" add . -1}}{{end}}`},
		{"main", `<a href="/foo{{template "helper" .}}">`, "helper", `{{if .Children}}{{range .Children}}{{template "helper" .}}{{end}}{{else}}?x={{.X}}{{end}}`},
		{"main", `<a href="{{template "u" .}}">{{template "u" .}}`, "u", `{{range .Children}}{{.X}}{{end}}`},
		{"main", `<script>{{template "j" .}}</script><style>{{template "j" .}}</style><p title="{{template "j" .}}">`, "j", `{{range .Children}}{{.X}}{{end}}`},
		{"main", `<a href="?{{template "loop" .}}">`, "loop", `{{range .Children}}{{template "loop" .}}&{{end}}{{.X}}`},
		{"main", `{{template "a" .}}`, "a", `<a href="{{template "b" .}}`, "b", `{{.X}}">`},
		{"main", `<textarea>{{template "x"}}</textarea><title>{{template "x"}}</title>`, "x", `<b>{{"<i>"}}</b>`},
		{"main", "<script>`${ {{template \"x\"}} }`</script>", "x", `{{"a"}}`},
		{"main", `{{template "x" .}}{{template "x" .}}`, "x", `<a href="{{.X}}">`},
	} {
		src := ""
		for i := 0; i < len(set); i += 2 {
			src += fmt.Sprintf("{{define %q}}%s{{end}} ", set[i], set[i+1])
		}
		cs = append(cs, tmplCase{html: true, texts: []string{src}, data: tree, name: "main"}, tmplCase{texts: []string{src}, data: tree, name: "main"})
	}
	return cs
}

func tmplHTMLRandomCases(r *rand.Rand, n int) []tmplCase {
	pick := func(s ...string) string { return s[r.Intn(len(s))] }
	act := func() string {
		a := pick(".a", ".u", ".j", ".n", ".f", ".t", ".z", ".arr", ".obj", ".s", ".css", ".id", ".ws", ".uni", ".sl", ".q", ".dl", ".cm",
			".nope", `"lit<>"`, "1", "(index .arr 1)", "(safeHTML .a)", "(safeURL .u)", "(safeJS .a)", "(safeCSS .css)", "(safeAttr .id)",
			"(safeJSStr .a)", "(safeSrcset .u)", ".obj.k")
		switch r.Intn(8) {
		case 0:
			a += " | html"
		case 1:
			a += " | urlquery"
		case 2:
			a = "printf \"%v\" " + a
		}
		return "{{" + a + "}}"
	}
	var piece func(depth int) string
	piece = func(depth int) string {
		switch k := r.Intn(30); {
		case k < 6:
			return act()
		case k < 8:
			return pick("<p>", "</p>", "<a ", "<b>", "</a>", ">", "\"", "'", " ", "=", "x", "<br/>", "&amp;", "&lt;", "&#39;", "<!--", "-->", "\n")
		case k < 10:
			return "<a " + pick("href", "title", "onclick", "style", "src", "srcset", "data-x", "class", "xlink:href", "value", "id", "on", "type") +
				"=" + pick("\"", "'", "") + pick("", "x", "/p?q=", "http://h/", "javascript:", "a: ", "f(", "&quot;") + act() + pick("", "y", "?z", ")") + pick("\"", "'", "") + ">"
		case k < 12:
			return "<script>" + pick("var x = ", "f(", "x = '", "x = \"", "x = /", "x = `", "// ", "/* ", "x = `${", "if (a) {") +
				act() + pick("", ";", "'", "\"", "/", "`", " */", "}`", "}", ")") + "</script>"
		case k < 13:
			return "<style>" + pick("p { color: ", "p { b: url(", "p { b: '", "/* ", "a{b:\"") + act() + pick("}", ")", "'", "*/", "\"}") + "</style>"
		case k < 14:
			return "<" + pick("textarea", "title") + ">" + act() + "</" + pick("textarea", "title") + ">"
		case k < 15:
			return "<!-- " + act() + " -->"
		case k < 16:
			return "<p " + act() + ">"
		case k < 19 && depth < 3:
			return "{{if " + pick(".t", ".z", ".n", ".s") + "}}" + piece(depth+1) + pick("", "{{else}}"+piece(depth+1)) + "{{end}}"
		case k < 21 && depth < 3:
			return "{{range " + pick(".arr", ".obj", ".s", ".z") + "}}" + piece(depth+1) + pick("", "{{break}}", "{{continue}}") + piece(depth+1) + "{{end}}"
		case k < 22 && depth < 3:
			return "{{with " + pick(".obj", ".z", ".a") + "}}" + piece(depth+1) + "{{end}}"
		default:
			return act()
		}
	}
	var cases []tmplCase
	for i := 0; i < n; i++ {
		var s strings.Builder
		defs := ""
		if r.Intn(5) == 0 {
			defs = `{{define "d"}}` + piece(1) + piece(1) + `{{end}}`
		}
		for k := r.Intn(5) + 1; k > 0; k-- {
			if defs != "" && r.Intn(4) == 0 {
				s.WriteString(`{{template "d" ` + pick(".", ".a", ".obj", ".u") + `}}`)
			} else {
				s.WriteString(piece(0))
			}
		}
		text := defs + s.String()
		if r.Intn(10) == 0 {
			text = tmplMutate(r, text)
		}
		cases = append(cases, tmplCase{html: true, texts: []string{text}, data: tmplHTMLData})
	}
	return cases
}

// tmplSoupCases strings tokens together at random: mostly invalid
// templates, for the lexer's and the parser's errors
func tmplSoupCases(r *rand.Rand, n int) []tmplCase {
	toks := []string{"{{", "}}", "{{-", "-}}", "{{- ", " -}}", " ", "  ", "\n", "\t", ".", ".a", ".b.c", "$", "$x", "$x :=", "$x =",
		"$i, $e :=", ",", "|", "(", ")", "\"s\"", "\"", "`r`", "`", "'a'", "'", "'\\n'", "1", "-1", "1.5", "0x1F", "1e3", "1+2i", "2i",
		"nil", "true", "if", "else", "end", "range", "with", "define", "template", "block", "break", "continue", "/*", "*/",
		"{{/*", "*/}}", "len", "index", "print", "printf", "and", "or", "not", "eq", "a", "text", "<", "\"d\"", "#", "@", ":", "=",
		"\u00e9", "\xff"}
	var cases []tmplCase
	for i := 0; i < n; i++ {
		var s strings.Builder
		for k := r.Intn(14) + 1; k > 0; k-- {
			s.WriteString(toks[r.Intn(len(toks))])
		}
		text := s.String()
		text = strings.NewReplacer("\\n", "\n", "\\t", "\t", "\\u00e9", "\u00e9", "\\xff", "\xff").Replace(text)
		cases = append(cases, tmplCase{texts: []string{text}, data: `{"a":1,"b":{"c":[1,2]}}`, html: r.Intn(4) == 0})
	}
	return cases
}
