package cc

import (
	"fmt"
	"io/fs"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

// Macro is a preprocessor macro definition
type Macro struct {
	Name     string
	FuncLike bool
	Params   []string
	VaName   string // name of the variadic parameter ("__VA_ARGS__" or a GNU named one); "" if none
	Body     []*Token
	Def      *Token
	builtin  func(pp *Preprocessor, t *Token) []*Token
	special  bool // __has_include and friends: only meaningful in #if
}

// IncludeDir is one directory of the include search path
type IncludeDir struct {
	Path     string
	Embedded fs.FS // when not nil, Path is a directory inside this FS
	System   bool
}

// PPConfig configures the preprocessor
type PPConfig struct {
	Std        int  // 1989, 1999, 2011, 2017, 2023
	GNU        bool // gnu dialect (defines linux, unix)
	QuoteDirs  []string
	Dirs       []IncludeDir // searched in order for <...> (and after the quote dirs for "...")
	Defines    []string     // "-DNAME=VAL" / "-UNAME" in command line order, without the dash letter: "DNAME=VAL"
	PreInclude []string     // -include files
	ReadFile   func(path string) ([]byte, error)
}

type cond struct {
	tok      *Token
	inElse   bool
	taken    bool // a branch has been taken
	guard    string
	guardPos int
}

type source struct {
	toks   []*Token
	pos    int
	file   *File
	dir    string // directory of the file for "..." includes
	dirIdx int    // index in Dirs where the file was found (-1: not from Dirs)
	conds  []*cond
	path   string // canonical path (for #pragma once / include guards)
	// include-guard detection
	firstDirective bool
	guardCond      *cond
	guardDone      string
}

// Preprocessor expands macros and directives of one translation unit
type Preprocessor struct {
	cfg      PPConfig
	macros   map[string]*Macro
	srcs     []*source
	pending  []*Token // expanded tokens to be read before the sources (stack: last is next)
	Errors   []*Error
	counter  int
	once     map[string]bool
	guards   map[string]string
	fileToks map[string][]*Token
	files    map[string]*File
	nextID   int
	Deps     []string // every file read
	now      time.Time
	baseFile string
	inIf     bool
}

// NewPreprocessor creates a preprocessor with the predefined macros
func NewPreprocessor(cfg PPConfig) *Preprocessor {
	if cfg.ReadFile == nil {
		cfg.ReadFile = os.ReadFile
	}
	pp := &Preprocessor{
		cfg:      cfg,
		macros:   map[string]*Macro{},
		once:     map[string]bool{},
		guards:   map[string]string{},
		fileToks: map[string][]*Token{},
		files:    map[string]*File{},
		now:      time.Now(),
	}
	pp.definePredefined()
	return pp
}

func (pp *Preprocessor) errorf(t *Token, format string, args ...any) {
	pp.Errors = append(pp.Errors, errAt(t, false, format, args...))
}

func (pp *Preprocessor) warnf(t *Token, format string, args ...any) {
	pp.Errors = append(pp.Errors, errAt(t, true, format, args...))
}

// Define defines an object-like macro from text: "NAME" or "NAME=VALUE"
// (the command-line -D form); NAME(args)=body defines a function-like one
func (pp *Preprocessor) Define(def string) {
	name, val, ok := strings.Cut(def, "=")
	if !ok {
		val = "1"
	}
	pp.defineText(name + " " + val)
}

// defineText processes the text of a #define directive
func (pp *Preprocessor) defineText(text string) {
	f := &File{Name: "<command line>", Contents: []byte(text)}
	toks, _ := Tokenize(f)
	for _, t := range toks {
		t.File = nil
	}
	pp.readDefine(toks, 0)
}

// Undef removes a macro
func (pp *Preprocessor) Undef(name string) { delete(pp.macros, name) }

// Macros returns the macro table (for tests and -dM)
func (pp *Preprocessor) Macros() map[string]*Macro { return pp.macros }

func (pp *Preprocessor) newFile(name string, data []byte) *File {
	pp.nextID++
	return &File{Name: name, Contents: data, ID: pp.nextID, Trigraphs: !pp.cfg.GNU && pp.cfg.Std < 2023}
}

func (pp *Preprocessor) definePredefined() {
	std := pp.cfg.Std
	var b strings.Builder
	def := func(name, val string) { fmt.Fprintf(&b, "#define %s %s\n", name, val) }
	def("__STDC__", "1")
	switch {
	case std >= 2023:
		def("__STDC_VERSION__", "202311L")
	case std >= 2017:
		def("__STDC_VERSION__", "201710L")
	case std >= 2011:
		def("__STDC_VERSION__", "201112L")
	case std >= 1999:
		def("__STDC_VERSION__", "199901L")
	}
	def("__STDC_HOSTED__", "1")
	def("__STDC_UTF_16__", "1")
	def("__STDC_UTF_32__", "1")
	def("__STDC_IEC_559__", "1")
	def("__STDC_IEC_60559_BFP__", "201404L")
	def("__STDC_ISO_10646__", "201706L")
	def("__STDC_NO_COMPLEX__", "1")
	if !pp.cfg.GNU {
		def("__STRICT_ANSI__", "1")
	} else {
		def("linux", "1")
		def("unix", "1")
	}
	for _, n := range []string{"__x86_64__", "__x86_64", "__amd64__", "__amd64", "__linux__", "__linux", "__gnu_linux__",
		"__unix__", "__unix", "__ELF__", "__LP64__", "_LP64", "__altc__", "__NO_INLINE__"} {
		def(n, "1")
	}
	def("__ALTC_VERSION__", "1")
	def("__CHAR_BIT__", "8")
	for _, kv := range [][2]string{{"SHORT", "2"}, {"INT", "4"}, {"LONG", "8"}, {"LONG_LONG", "8"}, {"POINTER", "8"},
		{"FLOAT", "4"}, {"DOUBLE", "8"}, {"LONG_DOUBLE", "16"}, {"SIZE_T", "8"}, {"WCHAR_T", "4"}, {"WINT_T", "4"}, {"PTRDIFF_T", "8"}} {
		def("__SIZEOF_"+kv[0]+"__", kv[1])
	}
	def("__ORDER_LITTLE_ENDIAN__", "1234")
	def("__ORDER_BIG_ENDIAN__", "4321")
	def("__ORDER_PDP_ENDIAN__", "3412")
	def("__BYTE_ORDER__", "__ORDER_LITTLE_ENDIAN__")
	def("__FLOAT_WORD_ORDER__", "__ORDER_LITTLE_ENDIAN__")
	def("__BIGGEST_ALIGNMENT__", "16")
	for _, kv := range [][2]string{
		{"SIZE", "unsigned long"}, {"PTRDIFF", "long"}, {"WCHAR", "int"}, {"WINT", "unsigned int"},
		{"INTMAX", "long"}, {"UINTMAX", "unsigned long"}, {"CHAR16", "unsigned short"}, {"CHAR32", "unsigned int"},
		{"INTPTR", "long"}, {"UINTPTR", "unsigned long"},
		{"INT8", "signed char"}, {"INT16", "short"}, {"INT32", "int"}, {"INT64", "long"},
		{"UINT8", "unsigned char"}, {"UINT16", "unsigned short"}, {"UINT32", "unsigned int"}, {"UINT64", "unsigned long"},
	} {
		def("__"+kv[0]+"_TYPE__", kv[1])
	}
	def("__SCHAR_MAX__", "0x7f")
	def("__SHRT_MAX__", "0x7fff")
	def("__INT_MAX__", "0x7fffffff")
	def("__LONG_MAX__", "0x7fffffffffffffffL")
	def("__LONG_LONG_MAX__", "0x7fffffffffffffffLL")
	def("__WCHAR_MAX__", "0x7fffffff")
	def("__WCHAR_MIN__", "(-__WCHAR_MAX__ - 1)")
	def("__SIZE_MAX__", "0xffffffffffffffffUL")
	def("__PTRDIFF_MAX__", "0x7fffffffffffffffL")
	def("__INTMAX_MAX__", "0x7fffffffffffffffL")
	def("__UINTMAX_MAX__", "0xffffffffffffffffUL")
	def("__INTPTR_MAX__", "0x7fffffffffffffffL")
	def("__UINTPTR_MAX__", "0xffffffffffffffffUL")
	for i, n := range []string{"RELAXED", "CONSUME", "ACQUIRE", "RELEASE", "ACQ_REL", "SEQ_CST"} {
		def("__ATOMIC_"+n, fmt.Sprint(i))
	}
	for _, n := range []string{"BOOL", "CHAR", "CHAR16_T", "CHAR32_T", "WCHAR_T", "SHORT", "INT", "LONG", "LLONG", "POINTER"} {
		def("__GCC_ATOMIC_"+n+"_LOCK_FREE", "2")
	}
	def("__USER_LABEL_PREFIX__", "")
	def("__REGISTER_PREFIX__", "")
	def("__VERSION__", "\"AltC\"")
	// GNU spellings of keywords that some code uses without the compiler
	def("__restrict__", "restrict")
	f := pp.newFile("<built-in>", []byte(b.String()))
	toks, _ := Tokenize(f)
	pp.runDirectivesOnly(toks)

	for _, name := range []string{"__FILE__", "__LINE__", "__COUNTER__", "__DATE__", "__TIME__", "__TIMESTAMP__",
		"__BASE_FILE__", "__INCLUDE_LEVEL__", "__FILE_NAME__"} {
		pp.macros[name] = &Macro{Name: name, builtin: builtinMacro}
	}
	for _, name := range []string{"__has_include", "__has_include_next", "__has_builtin", "__has_attribute",
		"__has_c_attribute", "__has_feature", "__has_extension", "__has_cpp_attribute", "__has_embed", "__has_declspec_attribute"} {
		pp.macros[name] = &Macro{Name: name, special: true}
	}
}

// runDirectivesOnly processes a token list that holds only directives (predefined macros)
func (pp *Preprocessor) runDirectivesOnly(toks []*Token) {
	pp.srcs = append(pp.srcs, &source{toks: toks, dirIdx: -1, file: toks[0].File})
	for {
		t := pp.raw()
		if t.Kind == TEOF {
			break
		}
	}
	pp.srcs = pp.srcs[:len(pp.srcs)-1]
}

func builtinMacro(pp *Preprocessor, t *Token) []*Token {
	loc := t.loc()
	mk := func(kind TokKind, text string) []*Token {
		n := t.copy()
		n.Kind = kind
		n.Text = text
		n.hide = nil
		return []*Token{n}
	}
	switch t.Text {
	case "__FILE__":
		name := "<unknown>"
		if loc.File != nil {
			name = loc.File.Name
		}
		return mk(TStr, strconv.Quote(name))
	case "__FILE_NAME__":
		name := "<unknown>"
		if loc.File != nil {
			name = filepath.Base(loc.File.Name)
		}
		return mk(TStr, strconv.Quote(name))
	case "__BASE_FILE__":
		return mk(TStr, strconv.Quote(pp.baseFile))
	case "__LINE__":
		return mk(TNum, strconv.Itoa(int(loc.Line)))
	case "__COUNTER__":
		pp.counter++
		return mk(TNum, strconv.Itoa(pp.counter-1))
	case "__INCLUDE_LEVEL__":
		n := 0
		for _, s := range pp.srcs {
			if s.file != nil {
				n++
			}
		}
		if n > 0 {
			n--
		}
		return mk(TNum, strconv.Itoa(n))
	case "__DATE__":
		return mk(TStr, pp.now.Format("\"Jan _2 2006\""))
	case "__TIME__":
		return mk(TStr, pp.now.Format("\"15:04:05\""))
	case "__TIMESTAMP__":
		return mk(TStr, pp.now.Format("\"Mon Jan _2 15:04:05 2006\""))
	}
	return nil
}

// ---- token sources ----

func (pp *Preprocessor) top() *source { return pp.srcs[len(pp.srcs)-1] }

// raw returns the next token without macro expansion, processing directives
func (pp *Preprocessor) raw() *Token {
	for {
		if n := len(pp.pending); n > 0 {
			t := pp.pending[n-1]
			pp.pending = pp.pending[:n-1]
			return t
		}
		s := pp.top()
		t := s.toks[s.pos]
		if t.Kind == TEOF {
			if s.file == nil || len(pp.srcs) == 1 {
				if s.file != nil {
					pp.endFile(s)
				}
				return t
			}
			pp.endFile(s)
			pp.srcs = pp.srcs[:len(pp.srcs)-1]
			continue
		}
		s.pos++
		if t.BOL && t.Kind == TPunct && t.Text == "#" && s.file != nil {
			pp.directive(s, t)
			continue
		}
		if s.file != nil {
			s.firstDirective = false
		}
		return t
	}
}

func (pp *Preprocessor) endFile(s *source) {
	for _, c := range s.conds {
		pp.errorf(c.tok, "unterminated conditional directive")
	}
	s.conds = nil
	if s.guardDone != "" && s.path != "" {
		pp.guards[s.path] = s.guardDone
	}
}

// peekRaw looks at the next token without consuming it or running directives
func (pp *Preprocessor) peekRaw() *Token {
	if n := len(pp.pending); n > 0 {
		return pp.pending[n-1]
	}
	s := pp.top()
	return s.toks[s.pos]
}

func (pp *Preprocessor) push(toks []*Token) {
	for i := len(toks) - 1; i >= 0; i-- {
		pp.pending = append(pp.pending, toks[i])
	}
}

// Next returns the next fully expanded token
func (pp *Preprocessor) Next() *Token {
	for {
		t := pp.raw()
		if t.Kind == TIdent && pp.inIf && t.Text == "defined" {
			return pp.definedOperator(t)
		}
		if t.Kind == TIdent && pp.expand(t) {
			continue
		}
		return t
	}
}

// definedOperator evaluates "defined X" that appears while expanding the
// controlling expression of #if (GCC and Clang accept it)
func (pp *Preprocessor) definedOperator(t *Token) *Token {
	n := pp.raw()
	paren := n.Is("(")
	if paren {
		n = pp.raw()
	}
	v := int64(0)
	if n.Kind == TIdent {
		if _, ok := pp.macros[n.Text]; ok {
			v = 1
		}
	} else {
		pp.errorf(t, "operator \"defined\" requires an identifier")
	}
	if paren {
		if r := pp.raw(); !r.Is(")") {
			pp.errorf(t, "missing ')' after \"defined\"")
		}
	}
	return pp.num(t, v)
}

// expandList macro-expands a token list in isolation
func (pp *Preprocessor) expandList(toks []*Token) []*Token {
	saved := pp.pending
	pp.pending = nil
	eof := &Token{Kind: TEOF}
	list := make([]*Token, len(toks)+1)
	copy(list, toks)
	list[len(toks)] = eof
	pp.srcs = append(pp.srcs, &source{toks: list, dirIdx: -1})
	var out []*Token
	for {
		t := pp.Next()
		if t == eof {
			break
		}
		out = append(out, t)
	}
	pp.srcs = pp.srcs[:len(pp.srcs)-1]
	pp.pending = saved
	return out
}

// ---- macro expansion ----

func (pp *Preprocessor) expand(t *Token) bool {
	if t.hide.contains(t.Text) {
		return false
	}
	m := pp.macros[t.Text]
	if m == nil || m.special {
		return false
	}
	if m.builtin != nil {
		res := m.builtin(pp, t)
		for _, r := range res {
			r.origin = t
		}
		pp.push(res)
		return true
	}
	if !m.FuncLike {
		hs := hsAdd(t.hide, m.Name)
		src := m.Body
		if m.hasPaste() {
			src = pp.subst(m, nil, t)
		}
		body := make([]*Token, len(src))
		for i, b := range src {
			c := b.copy()
			c.hide = hsUnion(c.hide, hs)
			c.origin = t
			c.BOL = false
			body[i] = c
		}
		if len(body) == 0 && t.Space {
			pp.spaceNext()
		}
		if len(body) > 0 {
			body[0].Space = t.Space
		}
		pp.push(body)
		return true
	}
	if !pp.peekRaw().Is("(") {
		return false
	}
	pp.raw() // (
	args, rparen, ok := pp.readArgs(m, t)
	if !ok {
		return true
	}
	hs := hsAdd(hsIntersect(t.hide, rparen.hide), m.Name)
	body := pp.subst(m, args, t)
	for _, c := range body {
		c.hide = hsUnion(c.hide, hs)
		c.origin = t
		c.BOL = false
	}
	if len(body) > 0 {
		body[0].Space = t.Space
	} else if t.Space {
		pp.spaceNext()
	}
	pp.push(body)
	return true
}

func (m *Macro) hasPaste() bool {
	for _, t := range m.Body {
		if t.Kind == TPunct && t.Text == "##" {
			return true
		}
	}
	return false
}

// spaceNext marks the token after an empty expansion as preceded by white
// space (GCC does this so that stringification keeps tokens apart)
func (pp *Preprocessor) spaceNext() {
	if n := len(pp.pending); n > 0 {
		c := pp.pending[n-1].copy()
		c.Space = true
		pp.pending[n-1] = c
		return
	}
	s := pp.top()
	t := s.toks[s.pos]
	if t.Kind == TEOF || t.BOL {
		return
	}
	c := t.copy()
	c.Space = true
	s.pos++
	pp.pending = append(pp.pending, c)
}

type macroArg struct {
	toks     []*Token
	expanded []*Token
	done     bool
}

// readArgs reads the arguments of a function-like macro call after "("
func (pp *Preprocessor) readArgs(m *Macro, name *Token) ([]*macroArg, *Token, bool) {
	var args []*macroArg
	cur := &macroArg{}
	depth := 0
	nparams := len(m.Params)
	for {
		t := pp.raw()
		if t.Kind == TEOF {
			pp.errorf(name, "unterminated argument list invoking macro \"%s\"", m.Name)
			return nil, nil, false
		}
		if depth == 0 && t.Is(")") {
			args = append(args, cur)
			// m() with no parameters takes no arguments
			if nparams == 0 && m.VaName == "" && len(args) == 1 && len(args[0].toks) == 0 {
				args = nil
			}
			want := nparams
			if m.VaName != "" {
				if len(args) == nparams {
					args = append(args, &macroArg{}) // empty variadic part
				}
				want = nparams + 1
				if len(args) < want {
					pp.errorf(name, "macro \"%s\" requires at least %d arguments, but only %d given", m.Name, nparams, len(args))
					return nil, nil, false
				}
			} else if len(args) != want {
				if !(want == 1 && len(args) == 0) {
					pp.errorf(name, "macro \"%s\" passed %d arguments, but takes %d", m.Name, len(args), want)
					return nil, nil, false
				}
				args = append(args, &macroArg{})
			}
			return args, t, true
		}
		if depth == 0 && t.Is(",") && (m.VaName == "" || len(args) < nparams) {
			args = append(args, cur)
			cur = &macroArg{}
			continue
		}
		if t.Is("(") {
			depth++
		} else if t.Is(")") {
			depth--
		}
		if len(cur.toks) == 0 {
			c := t.copy()
			c.BOL = false
			t = c
		}
		cur.toks = append(cur.toks, t)
	}
}

func (m *Macro) paramIndex(t *Token) int {
	if t.Kind != TIdent {
		return -1
	}
	for i, p := range m.Params {
		if p == t.Text {
			return i
		}
	}
	if m.VaName != "" && t.Text == m.VaName {
		return len(m.Params)
	}
	return -1
}

func (pp *Preprocessor) argExpanded(a *macroArg) []*Token {
	if !a.done {
		a.expanded = pp.expandList(a.toks)
		a.done = true
	}
	return a.expanded
}

func copyToks(toks []*Token) []*Token {
	out := make([]*Token, len(toks))
	for i, t := range toks {
		out[i] = t.copy()
	}
	return out
}

// subst replaces the parameters in the macro body
func (pp *Preprocessor) subst(m *Macro, args []*macroArg, call *Token) []*Token {
	body := m.Body
	var out []*Token
	vaEmpty := func() bool {
		if m.VaName == "" {
			return true
		}
		return len(pp.argExpanded(args[len(m.Params)])) == 0
	}
	for i := 0; i < len(body); i++ {
		t := body[i]
		// #param
		if t.Is("#") && i+1 < len(body) {
			if idx := m.paramIndex(body[i+1]); idx >= 0 {
				s := stringize(args[idx].toks, t)
				s.Space = t.Space
				out = append(out, s)
				i++
				continue
			}
		}
		// GNU: , ## __VA_ARGS__ drops the comma when the variadic part is empty
		if t.Is(",") && i+2 < len(body) && body[i+1].Is("##") && m.VaName != "" && body[i+2].Text == m.VaName {
			if len(args[len(m.Params)].toks) == 0 {
				i += 2
				continue
			}
			out = append(out, t.copy())
			out = append(out, copyToks(args[len(m.Params)].toks)...)
			i += 2
			continue
		}
		// __VA_OPT__(content)
		if t.Kind == TIdent && t.Text == "__VA_OPT__" && m.VaName != "" && i+1 < len(body) && body[i+1].Is("(") {
			depth := 0
			j := i + 1
			for ; j < len(body); j++ {
				if body[j].Is("(") {
					depth++
				} else if body[j].Is(")") {
					depth--
					if depth == 0 {
						break
					}
				}
			}
			content := body[i+2 : j]
			if !vaEmpty() {
				sub := &Macro{Name: m.Name, FuncLike: true, Params: m.Params, VaName: m.VaName, Body: content}
				r := pp.subst(sub, args, call)
				if len(r) > 0 {
					r[0].Space = t.Space
				}
				out = append(out, r...)
			}
			i = j
			continue
		}
		// lhs ## rhs
		if t.Is("##") && i+1 < len(body) {
			rhs := body[i+1]
			i++
			if idx := m.paramIndex(rhs); idx >= 0 {
				a := args[idx].toks
				if len(a) == 0 {
					continue
				}
				if len(out) == 0 {
					out = append(out, copyToks(a)...)
					continue
				}
				out[len(out)-1] = pp.paste(out[len(out)-1], a[0])
				out = append(out, copyToks(a[1:])...)
				continue
			}
			if rhs.Kind == TIdent && rhs.Text == "__VA_OPT__" && m.VaName != "" && i+1 < len(body) && body[i+1].Is("(") {
				depth := 0
				j := i + 1
				for ; j < len(body); j++ {
					if body[j].Is("(") {
						depth++
					} else if body[j].Is(")") {
						depth--
						if depth == 0 {
							break
						}
					}
				}
				var r []*Token
				if !vaEmpty() {
					sub := &Macro{Name: m.Name, FuncLike: true, Params: m.Params, VaName: m.VaName, Body: body[i+2 : j]}
					r = pp.subst(sub, args, call)
				}
				i = j
				if len(r) == 0 {
					continue
				}
				if len(out) == 0 {
					out = append(out, r...)
					continue
				}
				out[len(out)-1] = pp.paste(out[len(out)-1], r[0])
				out = append(out, r[1:]...)
				continue
			}
			if len(out) == 0 {
				out = append(out, rhs.copy())
				continue
			}
			out[len(out)-1] = pp.paste(out[len(out)-1], rhs)
			continue
		}
		if idx := m.paramIndex(t); idx >= 0 {
			// param ## ...: use the unexpanded argument
			if i+1 < len(body) && body[i+1].Is("##") {
				a := args[idx].toks
				if len(a) == 0 {
					// placemarker: the next ## pastes nothing onto the rhs
					if i+2 < len(body) {
						rhs := body[i+2]
						if idx2 := m.paramIndex(rhs); idx2 >= 0 {
							out = append(out, copyToks(args[idx2].toks)...)
						} else {
							out = append(out, rhs.copy())
						}
						i += 2
					}
					continue
				}
				c := copyToks(a)
				c[0].Space = t.Space
				out = append(out, c...)
				continue
			}
			c := copyToks(pp.argExpanded(args[idx]))
			if len(c) > 0 {
				c[0].Space = t.Space
			}
			out = append(out, c...)
			continue
		}
		out = append(out, t.copy())
	}
	return out
}

// stringize implements the # operator
func stringize(toks []*Token, at *Token) *Token {
	var sb strings.Builder
	for i, t := range toks {
		if i > 0 && t.Space {
			sb.WriteByte(' ')
		}
		if t.Kind == TStr || t.Kind == TChar {
			for _, c := range []byte(t.Text) {
				if c == '"' || c == '\\' {
					sb.WriteByte('\\')
				}
				sb.WriteByte(c)
			}
		} else {
			sb.WriteString(t.Text)
		}
	}
	n := at.copy()
	n.Kind = TStr
	n.Text = "\"" + sb.String() + "\""
	return n
}

func escapeForC(s string) string {
	var sb strings.Builder
	for i := 0; i < len(s); i++ {
		c := s[i]
		switch c {
		case '"':
			sb.WriteString("\\\"")
		case '\\':
			sb.WriteString("\\\\")
		case '\n':
			sb.WriteString("\\n")
		default:
			sb.WriteByte(c)
		}
	}
	return sb.String()
}

// paste implements the ## operator
func (pp *Preprocessor) paste(lhs, rhs *Token) *Token {
	text := lhs.Text + rhs.Text
	f := &File{Name: "<paste>", Contents: []byte(text)}
	toks, errs := Tokenize(f)
	if len(errs) > 0 || len(toks) != 2 {
		pp.errorf(lhs, "pasting \"%s\" and \"%s\" does not give a valid preprocessing token", lhs.Text, rhs.Text)
		return lhs.copy()
	}
	n := lhs.copy()
	n.Kind = toks[0].Kind
	n.Text = toks[0].Text
	return n
}

// ---- directives ----

// lineToks reads the rest of the directive line
func (pp *Preprocessor) lineToks(s *source) []*Token {
	start := s.pos
	for !s.toks[s.pos].BOL {
		s.pos++
	}
	return s.toks[start:s.pos]
}

func (pp *Preprocessor) directive(s *source, hash *Token) {
	first := s.firstDirective
	s.firstDirective = false
	t := s.toks[s.pos]
	if t.BOL {
		return // null directive
	}
	if t.Kind == TNum {
		// GNU line marker: # 123 "file"
		pp.doLine(s, pp.lineToks(s))
		return
	}
	if t.Kind != TIdent {
		pp.errorf(t, "invalid preprocessing directive")
		pp.lineToks(s)
		return
	}
	s.pos++
	switch t.Text {
	case "define":
		pp.readDefine(pp.lineToks(s), 0)
	case "undef":
		line := pp.lineToks(s)
		if len(line) == 0 || line[0].Kind != TIdent {
			pp.errorf(t, "macro name must be an identifier")
			return
		}
		delete(pp.macros, line[0].Text)
	case "include", "include_next", "import":
		pp.doInclude(s, t, pp.lineToks(s), t.Text == "include_next")
	case "embed":
		pp.doEmbed(s, t, pp.lineToks(s))
	case "if":
		line := pp.lineToks(s)
		v := pp.evalIf(t, line)
		c := &cond{tok: t, taken: v}
		s.conds = append(s.conds, c)
		if !v {
			pp.skip(s)
		}
	case "ifdef", "ifndef":
		line := pp.lineToks(s)
		if len(line) == 0 || line[0].Kind != TIdent {
			pp.errorf(t, "no macro name given in #%s directive", t.Text)
			line = []*Token{{Kind: TIdent}}
		}
		_, def := pp.macros[line[0].Text]
		v := def == (t.Text == "ifdef")
		c := &cond{tok: t, taken: v}
		s.conds = append(s.conds, c)
		if first && t.Text == "ifndef" && s.file != nil && len(s.conds) == 1 {
			c.guard = line[0].Text
			s.guardCond = c
		}
		if !v {
			pp.skip(s)
		}
	case "elif", "elifdef", "elifndef", "else":
		line := pp.lineToks(s)
		if len(s.conds) == 0 {
			pp.errorf(t, "#%s without #if", t.Text)
			return
		}
		c := s.conds[len(s.conds)-1]
		if c.inElse {
			pp.errorf(t, "#%s after #else", t.Text)
		}
		if s.guardCond == c {
			s.guardCond = nil
		}
		if t.Text == "else" {
			c.inElse = true
		}
		if c.taken {
			pp.skip(s)
			return
		}
		v := true
		switch t.Text {
		case "elif":
			v = pp.evalIf(t, line)
		case "elifdef", "elifndef":
			if len(line) == 0 || line[0].Kind != TIdent {
				pp.errorf(t, "no macro name given in #%s directive", t.Text)
				v = false
			} else {
				_, def := pp.macros[line[0].Text]
				v = def == (t.Text == "elifdef")
			}
		}
		if v {
			c.taken = true
		} else {
			pp.skip(s)
		}
	case "endif":
		pp.lineToks(s)
		if len(s.conds) == 0 {
			pp.errorf(t, "#endif without #if")
			return
		}
		c := s.conds[len(s.conds)-1]
		s.conds = s.conds[:len(s.conds)-1]
		if c == s.guardCond && s.toks[s.pos].Kind == TEOF {
			s.guardDone = c.guard
		}
	case "line":
		pp.doLine(s, pp.lineToks(s))
	case "error":
		line := pp.lineToks(s)
		pp.errorf(t, "#error %s", tokensText(line))
	case "warning":
		line := pp.lineToks(s)
		pp.warnf(t, "#warning %s", tokensText(line))
	case "pragma":
		pp.doPragma(s, t, pp.lineToks(s))
	case "ident", "sccs", "assert", "unassert":
		pp.lineToks(s)
	default:
		pp.errorf(t, "invalid preprocessing directive #%s", t.Text)
		pp.lineToks(s)
	}
}

// skip skips a false conditional group up to the matching #elif/#else/#endif
func (pp *Preprocessor) skip(s *source) {
	depth := 0
	for {
		t := s.toks[s.pos]
		if t.Kind == TEOF {
			return
		}
		if t.BOL && t.Is("#") && s.toks[s.pos+1].Kind == TIdent && !s.toks[s.pos+1].BOL {
			d := s.toks[s.pos+1].Text
			switch d {
			case "if", "ifdef", "ifndef":
				depth++
			case "endif":
				if depth == 0 {
					return
				}
				depth--
			case "elif", "elifdef", "elifndef", "else":
				if depth == 0 {
					return
				}
			}
		}
		s.pos++
	}
}

func (pp *Preprocessor) readDefine(line []*Token, _ int) {
	if len(line) == 0 || line[0].Kind != TIdent {
		var at *Token
		if len(line) > 0 {
			at = line[0]
		}
		pp.errorf(at, "macro names must be identifiers")
		return
	}
	name := line[0]
	if name.Text == "defined" {
		pp.errorf(name, "\"defined\" cannot be used as a macro name")
		return
	}
	m := &Macro{Name: name.Text, Def: name}
	rest := line[1:]
	if len(rest) > 0 && rest[0].Is("(") && !rest[0].Space {
		m.FuncLike = true
		i := 1
		for {
			if i >= len(rest) {
				pp.errorf(name, "missing ')' in macro parameter list")
				return
			}
			p := rest[i]
			if p.Is(")") && len(m.Params) == 0 && m.VaName == "" {
				i++
				break
			}
			if p.Is("...") {
				m.VaName = "__VA_ARGS__"
				i++
				if i >= len(rest) || !rest[i].Is(")") {
					pp.errorf(p, "missing ')' after \"...\"")
					return
				}
				i++
				break
			}
			if p.Kind != TIdent {
				pp.errorf(p, "expected parameter name, found \"%s\"", p.Text)
				return
			}
			i++
			if i < len(rest) && rest[i].Is("...") {
				m.VaName = p.Text
				i++
				if i >= len(rest) || !rest[i].Is(")") {
					pp.errorf(p, "missing ')' after \"...\"")
					return
				}
				i++
				break
			}
			m.Params = append(m.Params, p.Text)
			if i < len(rest) && rest[i].Is(",") {
				i++
				continue
			}
			if i < len(rest) && rest[i].Is(")") {
				i++
				break
			}
			pp.errorf(p, "expected ',' or ')' in macro parameter list")
			return
		}
		rest = rest[i:]
	}
	m.Body = rest
	if len(m.Body) > 0 {
		b := m.Body[0].copy()
		b.Space = false
		m.Body = append([]*Token{b}, m.Body[1:]...)
	}
	if old, ok := pp.macros[m.Name]; ok && old.builtin == nil && !old.special && !sameMacro(old, m) && old.Def != nil && old.Def.File != nil {
		pp.warnf(name, "\"%s\" redefined", m.Name)
	}
	pp.macros[m.Name] = m
}

func sameMacro(a, b *Macro) bool {
	if a.FuncLike != b.FuncLike || a.VaName != b.VaName || len(a.Params) != len(b.Params) || len(a.Body) != len(b.Body) {
		return false
	}
	for i := range a.Params {
		if a.Params[i] != b.Params[i] {
			return false
		}
	}
	for i := range a.Body {
		if a.Body[i].Text != b.Body[i].Text || (i > 0 && a.Body[i].Space != b.Body[i].Space) {
			return false
		}
	}
	return true
}

func (pp *Preprocessor) doLine(s *source, line []*Token) {
	line = pp.expandList(line)
	if len(line) == 0 || line[0].Kind != TNum {
		if len(line) > 0 {
			pp.errorf(line[0], "#line directive requires a positive integer argument")
		}
		return
	}
	n, err := strconv.Atoi(line[0].Text)
	if err != nil {
		pp.errorf(line[0], "invalid line number")
		return
	}
	// renumber the following tokens of this file: the line after the
	// directive gets number n
	if s.pos >= len(s.toks) {
		return
	}
	dirLine := line[0].loc().Line
	if s.pos > 0 {
		dirLine = s.toks[s.pos-1].Line
	}
	delta := int32(n) - (dirLine + 1)
	var newFile *File
	if len(line) > 1 && line[1].Kind == TStr {
		name, _ := strconv.Unquote(line[1].Text)
		newFile = &File{Name: name, Contents: s.file.Contents, ID: s.file.ID}
	}
	rest := make([]*Token, 0, len(s.toks)-s.pos)
	for _, t := range s.toks[s.pos:] {
		c := t.copy()
		c.Line += delta
		if newFile != nil && c.File == s.file {
			c.File = newFile
		}
		rest = append(rest, c)
	}
	s.toks = append(append([]*Token(nil), s.toks[:s.pos]...), rest...)
	if newFile != nil {
		s.file = newFile
	}
}

func (pp *Preprocessor) doPragma(s *source, t *Token, line []*Token) {
	if len(line) == 0 {
		return
	}
	switch {
	case line[0].Text == "once":
		if s.path != "" {
			pp.once[s.path] = true
		}
		return
	case line[0].Text == "GCC" || line[0].Text == "clang" || line[0].Text == "STDC" || line[0].Text == "message":
		if len(line) > 1 && line[0].Text == "GCC" && (line[1].Text == "error" || line[1].Text == "warning") {
			msg := tokensText(line[2:])
			if line[1].Text == "error" {
				pp.errorf(t, "%s", msg)
			} else {
				pp.warnf(t, "%s", msg)
			}
		}
		if len(line) > 1 && line[0].Text == "GCC" && line[1].Text == "poison" {
			return
		}
		return
	}
	// other pragmas (pack, weak, redefine_extname...) go to the parser
	p := t.copy()
	p.Kind = TPragma
	p.Text = tokensText(pp.expandIfNeeded(line))
	p.BOL = true
	pp.push([]*Token{p})
}

func (pp *Preprocessor) expandIfNeeded(line []*Token) []*Token {
	if len(line) > 0 && line[0].Text == "pack" {
		return pp.expandList(line)
	}
	return line
}

// handlePragmaOperator implements _Pragma("...")
func (pp *Preprocessor) handlePragmaOperator(t *Token) bool {
	if !pp.peekRaw().Is("(") {
		return false
	}
	pp.raw()
	str := pp.Next()
	if str.Kind != TStr {
		pp.errorf(str, "_Pragma takes a parenthesized string literal")
		return true
	}
	if r := pp.Next(); !r.Is(")") {
		pp.errorf(r, "expected ')'")
	}
	text := str.Text
	if strings.HasPrefix(text, "L") {
		text = text[1:]
	}
	text = text[1 : len(text)-1]
	text = strings.ReplaceAll(strings.ReplaceAll(text, "\\\"", "\""), "\\\\", "\\")
	f := &File{Name: "<_Pragma>", Contents: []byte(text)}
	toks, _ := Tokenize(f)
	for _, x := range toks {
		x.origin = t
	}
	src := &source{toks: append([]*Token{}, toks...), dirIdx: -1}
	pp.doPragma(src, t, toks[:len(toks)-1])
	return true
}

// ---- includes ----

type foundFile struct {
	path    string // display / canonical path
	data    []byte
	dirIdx  int
	dir     string
	embedFS fs.FS
}

func (pp *Preprocessor) readFrom(d IncludeDir, name string) (string, []byte, bool) {
	if d.Embedded != nil {
		p := path.Join(d.Path, name)
		data, err := fs.ReadFile(d.Embedded, p)
		if err != nil {
			return "", nil, false
		}
		return "<altc>/" + name, data, true
	}
	p := filepath.Join(d.Path, name)
	data, err := pp.cfg.ReadFile(p)
	if err != nil {
		return "", nil, false
	}
	return filepath.Clean(p), data, true
}

func (pp *Preprocessor) findInclude(s *source, name string, quoted, next bool) (*foundFile, bool) {
	if filepath.IsAbs(name) {
		data, err := pp.cfg.ReadFile(name)
		if err != nil {
			return nil, false
		}
		return &foundFile{path: filepath.Clean(name), data: data, dirIdx: -1, dir: filepath.Dir(name)}, true
	}
	start := 0
	if next {
		start = s.dirIdx + 1
		if s.dirIdx < 0 {
			start = 0
		}
	}
	if quoted && !next {
		if s.dir != "" || s.file != nil {
			dir := s.dir
			if s.embedDir() {
				// a built-in header including "x.h": look next to it in the FS
				for i, d := range pp.cfg.Dirs {
					if d.Embedded != nil {
						if p, data, ok := pp.readFrom(IncludeDir{Path: path.Join(d.Path, path.Dir(strings.TrimPrefix(s.path, "<altc>/"))), Embedded: d.Embedded}, name); ok {
							p = "<altc>/" + path.Join(path.Dir(strings.TrimPrefix(s.path, "<altc>/")), name)
							return &foundFile{path: p, data: data, dirIdx: i, dir: ""}, true
						}
					}
				}
			} else {
				p := filepath.Join(dir, name)
				if data, err := pp.cfg.ReadFile(p); err == nil {
					return &foundFile{path: filepath.Clean(p), data: data, dirIdx: s.dirIdx, dir: filepath.Dir(p)}, true
				}
			}
		}
		for _, d := range pp.cfg.QuoteDirs {
			p := filepath.Join(d, name)
			if data, err := pp.cfg.ReadFile(p); err == nil {
				return &foundFile{path: filepath.Clean(p), data: data, dirIdx: -1, dir: filepath.Dir(p)}, true
			}
		}
	}
	for i := start; i < len(pp.cfg.Dirs); i++ {
		d := pp.cfg.Dirs[i]
		if p, data, ok := pp.readFrom(d, name); ok {
			dir := ""
			if d.Embedded == nil {
				dir = filepath.Dir(p)
			}
			return &foundFile{path: p, data: data, dirIdx: i, dir: dir}, true
		}
	}
	return nil, false
}

func (s *source) embedDir() bool { return strings.HasPrefix(s.path, "<altc>/") }

// includeName extracts the header name from an #include line
func (pp *Preprocessor) includeName(line []*Token) (string, bool, bool) {
	if len(line) == 0 {
		return "", false, false
	}
	if line[0].Kind == TStr && strings.HasPrefix(line[0].Text, "\"") {
		return line[0].Text[1 : len(line[0].Text)-1], true, true
	}
	if line[0].Is("<") {
		var sb strings.Builder
		for i := 1; i < len(line); i++ {
			if line[i].Is(">") {
				return sb.String(), false, true
			}
			if line[i].Space && i > 1 {
				sb.WriteByte(' ')
			}
			sb.WriteString(line[i].Text)
		}
		return "", false, false
	}
	return "", false, false
}

func (pp *Preprocessor) doInclude(s *source, t *Token, line []*Token, next bool) {
	name, quoted, ok := pp.includeName(line)
	if !ok {
		line = pp.expandList(line)
		name, quoted, ok = pp.includeName(line)
		if !ok {
			pp.errorf(t, "#include expects \"FILENAME\" or <FILENAME>")
			return
		}
	}
	f, found := pp.findInclude(s, name, quoted, next)
	if !found {
		pp.errorf(t, "%s: No such file or directory", name)
		return
	}
	pp.pushFile(f, t)
}

func (pp *Preprocessor) pushFile(f *foundFile, from *Token) {
	if pp.once[f.path] {
		return
	}
	if g, ok := pp.guards[f.path]; ok {
		if _, def := pp.macros[g]; def {
			return
		}
	}
	if len(pp.srcs) > 200 {
		pp.errorf(from, "#include nested too deeply")
		return
	}
	toks, ok := pp.fileToks[f.path]
	if !ok {
		file := pp.newFile(f.path, f.data)
		var errs []*Error
		toks, errs = Tokenize(file)
		pp.Errors = append(pp.Errors, errs...)
		pp.fileToks[f.path] = toks
		pp.Deps = append(pp.Deps, f.path)
	}
	pp.srcs = append(pp.srcs, &source{toks: toks, file: toks[0].File, dir: f.dir, dirIdx: f.dirIdx, path: f.path, firstDirective: true})
}

// doEmbed implements C23 #embed with the limit() parameter
func (pp *Preprocessor) doEmbed(s *source, t *Token, line []*Token) {
	name, quoted, ok := pp.includeName(line)
	if !ok {
		pp.errorf(t, "#embed expects \"FILENAME\" or <FILENAME>")
		return
	}
	f, found := pp.findInclude(s, name, quoted, false)
	if !found {
		pp.errorf(t, "%s: No such file or directory", name)
		return
	}
	data := f.data
	// parameters after the name
	i := 1
	if !quoted {
		for i < len(line) && !line[i].Is(">") {
			i++
		}
		i++
	}
	var prefix, suffix, ifEmpty []*Token
	for i < len(line) {
		p := line[i]
		i++
		var arg []*Token
		if i < len(line) && line[i].Is("(") {
			depth := 0
			j := i
			for ; j < len(line); j++ {
				if line[j].Is("(") {
					depth++
				} else if line[j].Is(")") {
					depth--
					if depth == 0 {
						break
					}
				}
			}
			arg = line[i+1 : min(j, len(line))]
			i = j + 1
		}
		switch strings.Trim(p.Text, "_") {
		case "limit":
			v := pp.evalIf(p, arg)
			_ = v
			n := pp.evalInt(p, arg)
			if n >= 0 && int(n) < len(data) {
				data = data[:n]
			}
		case "prefix":
			prefix = arg
		case "suffix":
			suffix = arg
		case "if_empty":
			ifEmpty = arg
		}
	}
	var out []*Token
	if len(data) == 0 {
		out = append(out, ifEmpty...)
	} else {
		out = append(out, prefix...)
		for k, b := range data {
			n := t.copy()
			n.Kind = TNum
			n.Text = strconv.Itoa(int(b))
			n.Space = k > 0
			n.BOL = false
			out = append(out, n)
			if k < len(data)-1 {
				c := t.copy()
				c.Kind = TPunct
				c.Text = ","
				c.BOL = false
				out = append(out, c)
			}
		}
		out = append(out, suffix...)
	}
	pp.push(out)
}

// ---- running ----

// Run preprocesses the main file and returns the token stream ending in TEOF
func (pp *Preprocessor) Run(path string, data []byte) []*Token {
	pp.baseFile = path
	main := &foundFile{path: path, data: data, dirIdx: -1, dir: filepath.Dir(path)}
	for _, d := range pp.cfg.Defines {
		switch {
		case strings.HasPrefix(d, "D"):
			pp.Define(d[1:])
		case strings.HasPrefix(d, "U"):
			pp.Undef(d[1:])
		}
	}
	pp.pushFile(main, nil)
	if len(pp.srcs) == 0 {
		return []*Token{{Kind: TEOF}}
	}
	// -include files are processed before the main file
	for i := len(pp.cfg.PreInclude) - 1; i >= 0; i-- {
		name := pp.cfg.PreInclude[i]
		f, ok := pp.findInclude(pp.top(), name, true, false)
		if !ok {
			pp.errorf(nil, "%s: No such file or directory", name)
			continue
		}
		pp.pushFile(f, nil)
	}
	var out []*Token
	for {
		t := pp.Next()
		if t.Kind == TIdent && t.Text == "_Pragma" && pp.handlePragmaOperator(t) {
			continue
		}
		out = append(out, t)
		if t.Kind == TEOF {
			break
		}
	}
	return out
}
