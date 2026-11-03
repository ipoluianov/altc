// Package cc is the C front end: tokenizer, preprocessor, parser and
// type checker. It produces a typed AST for the code generator.
package cc

import (
	"fmt"
	"strings"
)

// TokKind is the class of a token
type TokKind uint8

const (
	TIdent  TokKind = iota // identifier or keyword
	TPunct                 // punctuator
	TNum                   // preprocessing number
	TChar                  // character constant, possibly prefixed
	TStr                   // string literal, possibly prefixed
	TOther                 // stray character
	TPragma                // #pragma passed to the parser (Text holds the pragma line)
	TEOF
)

// File is a source file
type File struct {
	Name      string // path as shown in diagnostics
	Contents  []byte
	ID        int
	Trigraphs bool // replace ??= and friends (strict ISO modes before C23)
}

// Token is a preprocessing token
type Token struct {
	Kind  TokKind
	Text  string // spelling
	File  *File
	Line  int32
	Col   int32
	BOL   bool // first token on a line
	Space bool // preceded by white space

	hide   *hideSet // macros that must not be expanded again (Prosser's algorithm)
	origin *Token   // the macro invocation this token came from
	// set by the parser on conversion
	noExpand bool // identifier that was a disabled macro when seen
	notKw    bool // a keyword spelling that is an ordinary identifier in this language mode
}

func (t *Token) String() string { return t.Text }

// Is tells whether the token is the given punctuator or identifier
func (t *Token) Is(s string) bool {
	return (t.Kind == TPunct || t.Kind == TIdent) && t.Text == s
}

// Pos returns "file:line:col"
func (t *Token) Pos() string {
	if t.File == nil {
		return "<command line>"
	}
	return fmt.Sprintf("%s:%d:%d", t.File.Name, t.Line, t.Col)
}

// loc returns the token whose position should be reported: the place in
// the source file where the outermost macro was invoked
func (t *Token) loc() *Token {
	for t.origin != nil {
		t = t.origin
	}
	return t
}

func (t *Token) copy() *Token {
	c := *t
	return &c
}

// hideSet is an immutable set of macro names
type hideSet struct {
	names []string
}

func (h *hideSet) contains(name string) bool {
	if h == nil {
		return false
	}
	for _, n := range h.names {
		if n == name {
			return true
		}
	}
	return false
}

func hsUnion(a, b *hideSet) *hideSet {
	if a == nil {
		return b
	}
	if b == nil {
		return a
	}
	n := &hideSet{names: append([]string(nil), a.names...)}
	for _, x := range b.names {
		if !a.contains(x) {
			n.names = append(n.names, x)
		}
	}
	return n
}

func hsAdd(a *hideSet, name string) *hideSet {
	if a.contains(name) {
		return a
	}
	n := &hideSet{names: make([]string, 0, 1+lenHS(a))}
	if a != nil {
		n.names = append(n.names, a.names...)
	}
	n.names = append(n.names, name)
	return n
}

func lenHS(a *hideSet) int {
	if a == nil {
		return 0
	}
	return len(a.names)
}

func hsIntersect(a, b *hideSet) *hideSet {
	if a == nil || b == nil {
		return nil
	}
	var n []string
	for _, x := range a.names {
		if b.contains(x) {
			n = append(n, x)
		}
	}
	if n == nil {
		return nil
	}
	return &hideSet{names: n}
}

// Error is a diagnostic produced while compiling
type Error struct {
	File    string
	Line    int
	Col     int
	Warning bool
	Msg     string
}

func (e *Error) Error() string {
	kind := "error"
	if e.Warning {
		kind = "warning"
	}
	if e.File == "" {
		return kind + ": " + e.Msg
	}
	return fmt.Sprintf("%s:%d:%d: %s: %s", e.File, e.Line, e.Col, kind, e.Msg)
}

func errAt(t *Token, warning bool, format string, args ...any) *Error {
	e := &Error{Warning: warning, Msg: fmt.Sprintf(format, args...)}
	if t != nil {
		l := t.loc()
		if l.File != nil {
			e.File = l.File.Name
			e.Line = int(l.Line)
			e.Col = int(l.Col)
		}
	}
	return e
}

// tokensText joins token spellings the way they would be printed
func tokensText(toks []*Token) string {
	var sb strings.Builder
	for i, t := range toks {
		if t.Kind == TEOF {
			break
		}
		if i > 0 && t.Space {
			sb.WriteByte(' ')
		}
		sb.WriteString(t.Text)
	}
	return sb.String()
}
