package tests

import (
	"encoding/hex"
	"fmt"
	"math/rand"
	"regexp"
	"strings"
	"testing"
)

func goRegexp(f []string) string {
	unhex := func(s string) string { b, _ := hex.DecodeString(s); return string(b) }
	if f[0] == "q" {
		return hex.EncodeToString([]byte(regexp.QuoteMeta(unhex(f[1]))))
	}
	pat := unhex(f[1])
	var re *regexp.Regexp
	var err error
	if f[0] == "p" {
		re, err = regexp.CompilePOSIX(pat)
	} else {
		re, err = regexp.Compile(pat)
	}
	if err != nil {
		return "err " + err.Error()
	}
	switch f[0] {
	case "c", "p":
		return fmt.Sprintf("ok %d %s,", re.NumSubexp(), strings.Join(re.SubexpNames(), ","))
	case "a":
		if len(f) > 3 && f[3] == "L" {
			re.Longest()
		}
		text := unhex(f[2])
		var out strings.Builder
		for _, m := range re.FindAllStringSubmatchIndex(text, -1) {
			out.WriteString("[")
			for i, x := range m {
				if i > 0 {
					out.WriteString(" ")
				}
				fmt.Fprint(&out, x)
			}
			out.WriteString("]")
		}
		fmt.Fprintf(&out, " %d", b2i(re.MatchString(text)))
		return out.String()
	case "r":
		return hex.EncodeToString([]byte(re.ReplaceAllString(unhex(f[2]), unhex(f[3]))))
	case "l":
		return hex.EncodeToString([]byte(re.ReplaceAllLiteralString(unhex(f[2]), unhex(f[3]))))
	case "s":
		var n int
		fmt.Sscan(f[3], &n)
		parts := re.Split(unhex(f[2]), n)
		var out strings.Builder
		fmt.Fprintf(&out, "%d:", len(parts))
		for _, p := range parts {
			out.WriteString("|" + hex.EncodeToString([]byte(p)))
		}
		return out.String()
	}
	return "unknown"
}

// randPattern: a random expression from pieces that cover the syntax
func randPattern(r *rand.Rand, depth int) string {
	atoms := []string{"a", "b", "c", "ab", ".", "\\d", "\\w", "\\s", "\\W", "[a-c]", "[^ab]", "[[:alpha:]]", "[[:^digit:]x]",
		"\\pL", "\\p{Greek}", "\\PN", "\\p{^Lu}", "é", "Σ", "k", "\\x41", "\\x{263a}", "\\n", "[\\d_-]", "\\.", "\\b", "\\B", "^", "$",
		"\\A", "\\z", "(?i)a", "(?i:k)", "(?s:.)", "(?m:^)", "(?m:$)", "\\Qa.b\\E", "[é-ö]", "(?i)[k-l]", "x*?", "\\p{Han}",
		"[\\p{Lu}0-9]", "\\pN"}
	if depth > 3 {
		return atoms[r.Intn(len(atoms))]
	}
	var s string
	switch r.Intn(9) {
	case 0:
		s = randPattern(r, depth+1) + randPattern(r, depth+1)
	case 1:
		s = randPattern(r, depth+1) + "|" + randPattern(r, depth+1)
	case 2:
		s = "(" + randPattern(r, depth+1) + ")"
	case 3:
		s = "(?:" + randPattern(r, depth+1) + ")"
	case 4:
		s = "(?P<n" + fmt.Sprint(r.Intn(3)) + ">" + randPattern(r, depth+1) + ")"
	default:
		s = atoms[r.Intn(len(atoms))]
	}
	if r.Intn(3) == 0 {
		s = "(" + s + ")" + []string{"*", "+", "?", "*?", "+?", "??", "{2}", "{1,3}", "{0,}", "{2,}?", "{0,2}"}[r.Intn(11)]
	}
	return s
}

func TestDiffRegexp(t *testing.T) {
	r := rand.New(rand.NewSource(1950))
	h := func(s string) string { return hex.EncodeToString([]byte(s)) }
	var in []string
	bad := []string{"(", ")", "a)", "(a", "[a", "[]", "[^]", "[z-a]", "a**", "a++", "a*+", "*", "+a", "?", "{1}", "a{1001}", "a{2,1}",
		"x{1000}{1000}", "\\", "a\\", "\\8", "\\Ca", "\\xZ", "\\x{110000}", "\\x{}", "(?P<>a)", "(?P<a-b>x)", "(?<n", "(?P<n>a)(?P<n>b)",
		"(?z)", "(?i-)", "(?-)", "(?", "[[:foo:]]", "\\p{Foo}", "\\p{", "\\pX", "[a-\\d]", "\\Q", "(?i", "\xff", "[\xff]",
		"(?P=n)", "a{,3}", "a{3,", "x{2}{3}", "(a{999}){999}", strings.Repeat("(", 1001) + strings.Repeat(")", 1001), "(?<name>a)",
		"\\p{greek}", "\\p{L u}", "\\p{Uppercase_Letter}", "\\p{Assigned}", "\\p{ASCII}", "\\p{Any}", "\\p{latin}", "\\p{Lc}",
		"[a-]", "[-a]", "a{1,1001}", "\\z\\Z", "(?U)a*", "a|*", "()", "(|)", "a{0}", "x**?"}
	for _, p := range bad {
		in = append(in, "c\t"+h(p), "p\t"+h(p))
	}
	textPieces := []string{"a", "b", "c", "ab", "abc", " ", "\n", "1", "42", "_", "é", "Σσς", "K", "k", "K", "x", "a.b", "日本",
		"\xff", "Ab", "ö", "-", "\t"}
	for i := 0; i < 3000; i++ {
		p := randPattern(r, 0)
		var s strings.Builder
		for k := r.Intn(10); k > 0; k-- {
			s.WriteString(textPieces[r.Intn(len(textPieces))])
		}
		mode := ""
		if r.Intn(5) == 0 {
			mode = "\tL"
		}
		in = append(in, "c\t"+h(p), "a\t"+h(p)+"\t"+h(s.String())+mode)
		if i%5 == 0 {
			repl := []string{"<$0>", "$1", "${1}x", "$1x", "$$", "$n0", "${n1}", "$", "${", "${1", "$9", "$01", "-"}[r.Intn(13)]
			in = append(in, "r\t"+h(p)+"\t"+h(s.String())+"\t"+h(repl), "l\t"+h(p)+"\t"+h(s.String())+"\t"+h(repl),
				"s\t"+h(p)+"\t"+h(s.String())+"\t"+fmt.Sprint(r.Intn(5)-1))
		}
	}
	for _, q := range []string{"", "a.b", "[x]*", "\\$^{}|()+?", "é"} {
		in = append(in, "q\t"+h(q))
	}
	var want []string
	for _, l := range in {
		want = append(want, goRegexp(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "regexp", in), want)
}
