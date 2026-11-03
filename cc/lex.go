package cc

import (
	"bytes"
)

// mark maps an offset of the cleaned source back to the original position
type mark struct {
	off  int
	line int32
	col  int32
}

// cleanSource removes the BOM, carriage returns before newlines and
// backslash-newline line splices. It returns the text and position marks.
func cleanSource(in []byte) ([]byte, []mark) {
	in = bytes.TrimPrefix(in, []byte{0xEF, 0xBB, 0xBF})
	out := make([]byte, 0, len(in)+1)
	marks := []mark{{0, 1, 1}}
	line, col := int32(1), int32(1)
	for i := 0; i < len(in); i++ {
		c := in[i]
		if c == '\r' && i+1 < len(in) && in[i+1] == '\n' {
			continue
		}
		if c == '\\' {
			j := i + 1
			if j < len(in) && in[j] == '\n' {
				i = j
				line++
				col = 1
				marks = append(marks, mark{len(out), line, col})
				continue
			}
			if j+1 < len(in) && in[j] == '\r' && in[j+1] == '\n' {
				i = j + 1
				line++
				col = 1
				marks = append(marks, mark{len(out), line, col})
				continue
			}
		}
		out = append(out, c)
		if c == '\n' {
			line++
			col = 1
			marks = append(marks, mark{len(out), line, col})
		} else {
			col++
		}
	}
	if len(out) > 0 && out[len(out)-1] != '\n' {
		out = append(out, '\n')
	}
	return out, marks
}

var trigraphs = map[byte]byte{'=': '#', '(': '[', '/': '\\', ')': ']', '\'': '^', '<': '{', '!': '|', '>': '}', '-': '~'}

// replaceTrigraphs maps the nine ??x sequences to their characters
func replaceTrigraphs(in []byte) []byte {
	if bytes.Index(in, []byte("??")) < 0 {
		return in
	}
	out := make([]byte, 0, len(in))
	for i := 0; i < len(in); i++ {
		if in[i] == '?' && i+2 < len(in) && in[i+1] == '?' {
			if c, ok := trigraphs[in[i+2]]; ok {
				out = append(out, c)
				i += 2
				continue
			}
		}
		out = append(out, in[i])
	}
	return out
}

type lexer struct {
	file  *File
	src   []byte
	marks []mark
	mi    int
	errs  []*Error
	toks  []*Token
}

func (lx *lexer) pos(off int) (int32, int32) {
	for lx.mi+1 < len(lx.marks) && lx.marks[lx.mi+1].off <= off {
		lx.mi++
	}
	m := lx.marks[lx.mi]
	return m.line, m.col + int32(off-m.off)
}

func (lx *lexer) errorAt(off int, msg string) {
	l, c := lx.pos(off)
	lx.errs = append(lx.errs, &Error{File: lx.file.Name, Line: int(l), Col: int(c), Msg: msg})
}

func isIdentStart(c byte) bool {
	return c == '_' || c == '$' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= 0x80
}

func isIdentChar(c byte) bool {
	return isIdentStart(c) || c >= '0' && c <= '9'
}

func isDigit(c byte) bool { return c >= '0' && c <= '9' }

var puncts3 = []string{"<<=", ">>=", "..."}
var puncts2 = []string{"==", "!=", "<=", ">=", "->", "+=", "-=", "*=", "/=", "++", "--", "%=", "&=", "|=", "^=", "&&", "||", "<<", ">>", "##", "::"}

// Tokenize splits a source file into preprocessing tokens; the last token is TEOF
func Tokenize(f *File) ([]*Token, []*Error) {
	data := f.Contents
	if f.Trigraphs {
		data = replaceTrigraphs(data)
	}
	src, marks := cleanSource(data)
	lx := &lexer{file: f, src: src, marks: marks}
	lx.run()
	return lx.toks, lx.errs
}

func (lx *lexer) add(kind TokKind, start, end int, bol, space bool) *Token {
	l, c := lx.pos(start)
	t := &Token{Kind: kind, Text: string(lx.src[start:end]), File: lx.file, Line: l, Col: c, BOL: bol, Space: space}
	lx.toks = append(lx.toks, t)
	return t
}

func (lx *lexer) run() {
	src := lx.src
	asm := lx.file.Asm
	n := len(src)
	bol, space := true, false
	i := 0
	for i < n {
		c := src[i]
		switch {
		case c == '\n':
			i++
			bol, space = true, false
			continue
		case c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r':
			i++
			space = true
			continue
		case c == '/' && i+1 < n && src[i+1] == '/':
			for i < n && src[i] != '\n' {
				i++
			}
			space = true
			continue
		case c == '/' && i+1 < n && src[i+1] == '*':
			end := bytes.Index(src[i+2:], []byte("*/"))
			if end < 0 {
				lx.errorAt(i, "unterminated comment")
				i = n
				continue
			}
			i += 2 + end + 2
			space = true
			continue
		}
		start := i
		switch {
		case isDigit(c) || c == '.' && i+1 < n && isDigit(src[i+1]):
			i++
			for i < n {
				d := src[i]
				if (d == '+' || d == '-') && (src[i-1] == 'e' || src[i-1] == 'E' || src[i-1] == 'p' || src[i-1] == 'P') {
					i++
				} else if d == '\'' && i+1 < n && isIdentChar(src[i+1]) {
					i += 2
				} else if isIdentChar(d) || d == '.' {
					i++
				} else {
					break
				}
			}
			lx.add(TNum, start, i, bol, space)
		case isIdentStart(c) && !(asm && c == '$'):
			for i < n && isIdentChar(src[i]) && !(asm && src[i] == '$') {
				i++
			}
			word := string(src[start:i])
			if i < n && (src[i] == '"' || src[i] == '\'') && (word == "L" || word == "u" || word == "U" || word == "u8") {
				q := src[i]
				end := lx.scanQuoted(i, q)
				kind := TStr
				if q == '\'' {
					kind = TChar
				}
				lx.add(kind, start, end, bol, space)
				i = end
			} else {
				lx.add(TIdent, start, i, bol, space)
			}
		case c == '"' || c == '\'':
			end := lx.scanQuoted(i, c)
			kind := TStr
			if c == '\'' {
				kind = TChar
			}
			lx.add(kind, start, end, bol, space)
			i = end
		default:
			kind := TPunct
			text := ""
			l := 1
			if i+3 < n && string(src[i:i+4]) == "%:%:" {
				text, l = "##", 4
			} else {
				for _, p := range puncts3 {
					if i+3 <= n && string(src[i:i+3]) == p {
						text, l = p, 3
						break
					}
				}
				if text == "" {
					for _, p := range puncts2 {
						if i+2 <= n && string(src[i:i+2]) == p {
							text, l = p, 2
							break
						}
					}
				}
				if text == "" && i+2 <= n {
					switch string(src[i : i+2]) {
					case "<:":
						text, l = "[", 2
					case ":>":
						text, l = "]", 2
					case "<%":
						text, l = "{", 2
					case "%>":
						text, l = "}", 2
					case "%:":
						text, l = "#", 2
					}
				}
				if text == "" {
					if bytes.IndexByte([]byte("+-*/%&|^~!=<>?:;,.()[]{}#"), c) >= 0 {
						text = string(c)
					} else {
						kind = TOther
						text = string(c)
					}
				}
			}
			t := lx.add(kind, start, start+l, bol, space)
			t.Text = text
			i += l
		}
		bol, space = false, false
	}
	l, c := lx.pos(n)
	lx.toks = append(lx.toks, &Token{Kind: TEOF, File: lx.file, Line: l, Col: c, BOL: true})
}

// scanQuoted returns the end offset of a string or character literal starting at i
func (lx *lexer) scanQuoted(i int, q byte) int {
	src := lx.src
	j := i + 1
	for j < len(src) && src[j] != q {
		if src[j] == '\n' {
			if lx.file.Asm {
				return i + 1
			}
			if q == '"' {
				lx.errorAt(i, "missing terminating \" character")
			} else {
				lx.errorAt(i, "missing terminating ' character")
			}
			return j
		}
		if src[j] == '\\' && j+1 < len(src) && src[j+1] != '\n' {
			j++
		}
		j++
	}
	if j >= len(src) {
		if lx.file.Asm {
			return i + 1
		}
		lx.errorAt(i, "unterminated literal")
		return j
	}
	return j + 1
}
