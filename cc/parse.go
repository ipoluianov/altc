package cc

import (
	"fmt"
	"strconv"
	"strings"
)

// varScope is what an ordinary identifier denotes in a scope
type varScope struct {
	obj     *Obj
	typedef *Type
	enumTy  *Type
	enumVal int64
}

type scope struct {
	parent *scope
	vars   map[string]*varScope
	tags   map[string]*Type
}

// declAttr collects storage classes and attributes of a declaration
type declAttr struct {
	isTypedef, isStatic, isExtern, isInline, isTLS, isConstexpr, isAuto, isRegister bool
	noreturn                                                                        bool
	align                                                                           int64
	weak, used, hidden, ctor, dtor, packed, unusedAttr                              bool
	alias, section, asmName                                                         string
	ifunc                                                                           bool
	mode                                                                            string
	vecSize                                                                         int64
	vecTok                                                                          *Token
	cleanup                                                                         *Token
	prio                                                                            int64
	gnuInline                                                                       bool
	alwaysInline, noinline                                                          bool
	target                                                                          string
	targetClones                                                                    []string
	deprecated                                                                      bool
}

// Options for the parser
type Options struct {
	Std      int   // 1989, 1999, 2011, 2017, 2023
	GNU      bool  // GNU dialect
	VecAlign int64 // largest alignment of vector types (16; 32 with AVX)
}

type parser struct {
	opt                 Options
	toks                []*Token
	pos                 int
	sc                  *scope
	prog                *Program
	fn                  *Obj
	errs                []*Error
	uniq                int
	gotos               []*Node
	staticInit          bool // parsing the initializer of an object with static storage
	labels              []*Node
	brk, cont           string
	sw                  *Node
	pack                []int64 // #pragma pack stack; last is current (0 = default)
	strs                map[string]*Obj
	weakNames           map[string]bool
	globalsByName       map[string]*Obj
	vaListTy            *Type
	inSizeof            int
	funcNames           map[string]*Obj
	cleanups            [][]cleanupEntry // per block scope: variables with __attribute__((cleanup))
	brkDepth, contDepth int              // cleanup depth of the innermost break/continue target
	hasVLA              bool             // a VLA was declared in the current block or loop body
	blocks              []*Node          // blocks being parsed (for gotos leaving VLA scopes)
}

type cleanupEntry struct {
	v  *Obj
	fn *Obj
}

// fatal stops parsing the current top-level declaration
type fatal struct{ err *Error }

func (p *parser) errorAt(t *Token, format string, args ...any) {
	panic(fatal{errAt(t, false, format, args...)})
}

func (p *parser) warnAt(t *Token, format string, args ...any) {
	p.errs = append(p.errs, errAt(t, true, format, args...))
}

// softError records an error but continues parsing
func (p *parser) softError(t *Token, format string, args ...any) {
	p.errs = append(p.errs, errAt(t, false, format, args...))
}

// Parse parses a preprocessed token stream into a program
func Parse(toks []*Token, opt Options) (*Program, []*Error) {
	p := &parser{opt: opt, toks: toks, prog: &Program{}, strs: map[string]*Obj{}, weakNames: map[string]bool{},
		globalsByName: map[string]*Obj{}, funcNames: map[string]*Obj{}}
	p.sc = &scope{vars: map[string]*varScope{}, tags: map[string]*Type{}}
	p.pack = []int64{0}
	for _, t := range toks {
		if t.Kind == TIdent && keywords[t.Text] && !keywordInMode(t.Text, opt) {
			t.notKw = true
		}
	}
	p.declareBuiltinTypes()
	p.program()
	for name := range p.weakNames {
		if o := p.globalsByName[name]; o != nil {
			o.IsWeak = true
		}
	}
	return p.prog, p.errs
}

func (p *parser) declareBuiltinTypes() {
	// typedef struct { unsigned gp_offset, fp_offset; void *overflow_arg_area, *reg_save_area; } __builtin_va_list[1];
	rec := recordType(false)
	rec.Rec.Tag = "__va_list_tag"
	rec.Rec.Members = []*Member{
		{Name: "gp_offset", Ty: tyUInt, Align: 4, Idx: 0},
		{Name: "fp_offset", Ty: tyUInt, Align: 4, Idx: 1},
		{Name: "overflow_arg_area", Ty: pointerTo(tyVoid), Align: 8, Idx: 2},
		{Name: "reg_save_area", Ty: pointerTo(tyVoid), Align: 8, Idx: 3},
	}
	layoutRecord(rec.Rec, 0)
	va := arrayOf(rec, 1)
	va.Typedef = "__builtin_va_list"
	p.vaListTy = va
	p.sc.vars["__builtin_va_list"] = &varScope{typedef: va}
	p.sc.vars["__int128_t"] = &varScope{typedef: tyInt128}
	p.sc.vars["__uint128_t"] = &varScope{typedef: tyUInt128}
	// TS 18661 floating types (glibc typedefs them itself when the compiler does not)
	for name, t := range map[string]*Type{"_Float32": tyFloat, "_Float64": tyDouble, "_Float32x": tyDouble, "_Float64x": tyLDouble} {
		p.sc.vars[name] = &varScope{typedef: t}
	}
}

// ---- tokens ----

func (p *parser) cur() *Token {
	for {
		t := p.toks[p.pos]
		if t.Kind != TPragma {
			return t
		}
		p.pragma(t)
		p.pos++
	}
}

func (p *parser) peekAt(n int) *Token {
	p.cur()
	i := p.pos
	for k := 0; ; i++ {
		if i >= len(p.toks) {
			return p.toks[len(p.toks)-1]
		}
		if p.toks[i].Kind == TPragma {
			continue
		}
		if k == n {
			return p.toks[i]
		}
		k++
	}
}

func (p *parser) next() *Token {
	t := p.cur()
	if t.Kind != TEOF {
		p.pos++
	}
	return t
}

func (p *parser) is(s string) bool {
	t := p.cur()
	return (t.Kind == TPunct || t.Kind == TIdent && !t.notKw) && t.Text == s
}

func (p *parser) consume(s string) bool {
	if p.is(s) {
		p.next()
		return true
	}
	return false
}

func (p *parser) expect(s string) *Token {
	t := p.cur()
	if !p.is(s) {
		if t.Kind == TEOF {
			p.errorAt(t, "expected '%s' at end of input", s)
		}
		p.errorAt(t, "expected '%s' before '%s'", s, t.Text)
	}
	return p.next()
}

func (p *parser) ident() *Token {
	t := p.cur()
	if t.Kind != TIdent || t.kw() {
		p.errorAt(t, "expected identifier before '%s'", t.Text)
	}
	return p.next()
}

func (p *parser) pragma(t *Token) {
	f := strings.Fields(strings.NewReplacer("(", " ( ", ")", " ) ", ",", " , ").Replace(t.Text))
	if len(f) == 0 {
		return
	}
	switch f[0] {
	case "pack":
		args := []string{}
		for _, a := range f[1:] {
			if a != "(" && a != ")" && a != "," {
				args = append(args, a)
			}
		}
		cur := &p.pack[len(p.pack)-1]
		switch {
		case len(args) == 0:
			*cur = 0
		case args[0] == "push":
			v := *cur
			for _, a := range args[1:] {
				if n, err := strconv.Atoi(a); err == nil {
					v = int64(n)
				}
			}
			p.pack = append(p.pack, v)
		case args[0] == "pop":
			if len(p.pack) > 1 {
				p.pack = p.pack[:len(p.pack)-1]
			}
		default:
			if n, err := strconv.Atoi(args[0]); err == nil {
				*cur = int64(n)
			}
		}
	case "weak":
		if len(f) > 1 {
			p.weakNames[f[1]] = true
		}
	}
}

// ---- scopes ----

func (p *parser) enterScope() {
	p.sc = &scope{parent: p.sc, vars: map[string]*varScope{}, tags: map[string]*Type{}}
}

func (p *parser) leaveScope() { p.sc = p.sc.parent }

func (p *parser) findVar(name string) *varScope {
	for s := p.sc; s != nil; s = s.parent {
		if v, ok := s.vars[name]; ok {
			return v
		}
	}
	return nil
}

func (p *parser) findTag(name string) *Type {
	for s := p.sc; s != nil; s = s.parent {
		if t, ok := s.tags[name]; ok {
			return t
		}
	}
	return nil
}

func (p *parser) isGlobalScope() bool { return p.sc.parent == nil }

func (p *parser) pushVar(name string) *varScope {
	v := &varScope{}
	p.sc.vars[name] = v
	return v
}

func (p *parser) newLvar(name string, ty *Type) *Obj {
	o := &Obj{Name: name, Ty: ty, IsLocal: true, Align: ty.Align()}
	if name != "" {
		p.pushVar(name).obj = o
	}
	if p.fn != nil {
		p.fn.Locals = append(p.fn.Locals, o)
	}
	return o
}

func (p *parser) newGvar(name string, ty *Type) *Obj {
	o := &Obj{Name: name, AsmName: name, Ty: ty, Align: ty.Align(), IsDefinition: true}
	p.pushVar(name).obj = o
	p.prog.Globals = append(p.prog.Globals, o)
	return o
}

func (p *parser) uniqueName(prefix string) string {
	p.uniq++
	return fmt.Sprintf(".L%s.%d", prefix, p.uniq)
}

// newAnonGvar creates static storage without a C name (literals)
func (p *parser) newAnonGvar(ty *Type) *Obj {
	name := p.uniqueName("anon")
	o := &Obj{Name: name, AsmName: name, Ty: ty, Align: ty.Align(), IsDefinition: true, IsStatic: true, IsLiteral: true}
	p.prog.Globals = append(p.prog.Globals, o)
	return o
}

// ---- keywords ----

var keywords = map[string]bool{}

func init() {
	for _, k := range strings.Fields(`auto break case char const continue default do double else enum extern float for goto
		if inline int long register restrict return short signed sizeof static struct switch typedef union unsigned void
		volatile while _Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn _Static_assert _Thread_local
		alignas alignof bool constexpr false nullptr static_assert thread_local true typeof typeof_unqual _BitInt
		_Decimal32 _Decimal64 _Decimal128 __attribute__ __attribute __extension__ __inline __inline__ __restrict
		__restrict__ __const __const__ __volatile __volatile__ __signed __signed__ __typeof __typeof__ __alignof
		__alignof__ __asm __asm__ asm __int128 __thread __label__ __auto_type
		__builtin_va_arg __builtin_offsetof __builtin_types_compatible_p __builtin_choose_expr __builtin_constant_p
		__unaligned __forceinline`) {
		keywords[k] = true
	}
}

func isKeyword(s string) bool { return keywords[s] }

// keywordInMode tells whether a keyword spelling is reserved in the language mode
func keywordInMode(s string, opt Options) bool {
	switch s {
	case "bool", "true", "false", "nullptr", "constexpr", "alignas", "alignof", "static_assert", "thread_local", "typeof_unqual":
		return opt.Std >= 2023
	case "typeof":
		return opt.Std >= 2023 || opt.GNU
	case "asm":
		return opt.GNU
	case "inline", "restrict":
		return opt.Std >= 1999 || opt.GNU
	}
	return true
}

// kw tells whether the token is a keyword in this language mode
func (t *Token) kw() bool { return t.Kind == TIdent && keywords[t.Text] && !t.notKw }

var typeKeywords = map[string]bool{}

func init() {
	for _, k := range strings.Fields(`void _Bool bool char short int long float double signed unsigned struct union enum
		typedef static extern inline _Thread_local thread_local __thread register auto const volatile restrict _Atomic
		_Alignas alignas _Noreturn constexpr typeof typeof_unqual __typeof __typeof__ __attribute__ __attribute
		__extension__ __inline __inline__ __restrict __restrict__ __const __const__ __volatile __volatile__ __signed
		__signed__ _BitInt __int128 __auto_type _Complex _Decimal32 _Decimal64
		_Decimal128 _Imaginary __unaligned __forceinline`) {
		typeKeywords[k] = true
	}
}

// isTypename tells whether the current token starts a type
func (p *parser) isTypename(t *Token) bool {
	if t.Kind != TIdent {
		return t.Kind == TPunct && t.Text == "[" && p.isAttrStart(t)
	}
	if typeKeywords[t.Text] && !t.notKw {
		return true
	}
	return p.isTypedefName(t)
}

func (p *parser) isTypedefName(t *Token) bool {
	v := p.findVar(t.Text)
	return v != nil && v.typedef != nil
}

func (p *parser) isAttrStart(t *Token) bool {
	return t.Is("[") && p.peekNext(t).Is("[")
}

func (p *parser) peekNext(t *Token) *Token {
	for i := p.pos; i < len(p.toks)-1; i++ {
		if p.toks[i] == t {
			j := i + 1
			for j < len(p.toks)-1 && p.toks[j].Kind == TPragma {
				j++
			}
			return p.toks[j]
		}
	}
	return p.toks[len(p.toks)-1]
}

// ---- attributes ----

// attributes parses any number of __attribute__((...)) and [[...]]
func (p *parser) attributes(a *declAttr) {
	for {
		t := p.cur()
		if t.Is("__attribute__") || t.Is("__attribute") {
			p.next()
			p.expect("(")
			p.expect("(")
			p.attrList(a, ")")
			p.expect(")")
			p.expect(")")
			continue
		}
		if t.Is("[") && p.peekAt(1).Is("[") {
			p.next()
			p.next()
			p.attrList(a, "]")
			p.expect("]")
			p.expect("]")
			continue
		}
		if t.Is("__declspec") {
			p.next()
			p.skipBalanced()
			continue
		}
		return
	}
}

// skipBalanced skips a parenthesized group
func (p *parser) skipBalanced() {
	p.expect("(")
	depth := 1
	for depth > 0 {
		t := p.next()
		if t.Kind == TEOF {
			p.errorAt(t, "unbalanced parentheses")
		}
		if t.Is("(") {
			depth++
		} else if t.Is(")") {
			depth--
		}
	}
}

func (p *parser) attrList(a *declAttr, end string) {
	if a == nil {
		a = &declAttr{}
	}
	for !p.is(end) {
		if p.consume(",") {
			continue
		}
		t := p.next()
		if t.Kind != TIdent {
			p.errorAt(t, "expected attribute name")
		}
		name := t.Text
		if p.consume("::") {
			name = p.next().Text // gnu::x, clang::x
		}
		name = strings.TrimSuffix(strings.TrimPrefix(name, "__"), "__")
		var args []*Token
		var argStart int
		if p.is("(") {
			argStart = p.pos
			p.skipBalanced()
			for i := argStart + 1; i < p.pos-1; i++ {
				if p.toks[i].Kind != TPragma {
					args = append(args, p.toks[i])
				}
			}
		}
		strArg := func() string {
			if len(args) > 0 && args[0].Kind == TStr {
				s := ""
				for _, x := range args {
					if x.Kind == TStr {
						_, body := literalPrefix(x.Text)
						s += body
					}
				}
				return s
			}
			return ""
		}
		switch name {
		case "aligned":
			if len(args) == 0 {
				a.align = 16
			} else {
				saved := p.pos
				p.pos = argStart + 1
				v := p.constExpr()
				p.pos = saved
				if v > a.align {
					a.align = v
				}
			}
		case "packed":
			a.packed = true
		case "weak":
			a.weak = true
		case "alias":
			a.alias = strArg()
		case "ifunc":
			a.alias = strArg()
			a.ifunc = true
		case "section":
			a.section = strArg()
		case "visibility":
			s := strArg()
			a.hidden = s == "hidden" || s == "internal"
		case "constructor", "destructor":
			if name == "constructor" {
				a.ctor = true
			} else {
				a.dtor = true
			}
			if len(args) > 0 {
				saved := p.pos
				p.pos = argStart + 1
				a.prio = p.constExpr()
				p.pos = saved
			}
		case "noreturn", "_Noreturn":
			a.noreturn = true
		case "used":
			a.used = true
		case "target":
			a.target = strings.ToLower(strArg())
		case "target_clones":
			for _, x := range args {
				if x.Kind == TStr {
					_, body := literalPrefix(x.Text)
					for _, c := range strings.Split(body, ",") {
						if c = strings.TrimSpace(strings.ToLower(c)); c != "" {
							a.targetClones = append(a.targetClones, c)
						}
					}
				}
			}
		case "always_inline":
			a.alwaysInline = true
		case "noinline", "noipa":
			a.noinline = true
		case "gnu_inline":
			a.gnuInline = true
		case "unused", "maybe_unused":
			a.unusedAttr = true
		case "deprecated":
			a.deprecated = true
		case "cleanup":
			if len(args) > 0 {
				a.cleanup = args[0]
			}
		case "mode":
			if len(args) > 0 {
				a.mode = strings.Trim(args[0].Text, "_")
			}
		case "vector_size":
			if len(args) > 0 {
				saved := p.pos
				p.pos = argStart + 1
				a.vecSize = p.constExpr()
				a.vecTok = t
				p.pos = saved
			}
		case "ext_vector_type":
			p.errorAt(t, "ext_vector_type is not supported")
		}
	}
}

// ---- declaration specifiers ----

func (p *parser) declspec(attr *declAttr) *Type {
	const (
		VOID     = 1 << 0
		BOOL     = 1 << 2
		CHAR     = 1 << 4
		SHORT    = 1 << 6
		INT      = 1 << 8
		LONG     = 1 << 10
		FLOAT    = 1 << 12
		DOUBLE   = 1 << 14
		OTHER    = 1 << 16
		SIGNED   = 1 << 17
		UNSIGNED = 1 << 18
		INT128   = 1 << 19
		COMPLEX  = 1 << 20
	)
	ty := tyInt
	counter := 0
	var isConst, isVolatile, isRestrict, isAtomic bool
	start := p.cur()
	for {
		t := p.cur()
		if t.Is("[") && p.peekAt(1).Is("[") || t.Is("__attribute__") || t.Is("__attribute") || t.Is("__declspec") {
			p.attributes(attr)
			continue
		}
		if !p.isTypename(t) {
			break
		}
		name := t.Text
		if t.notKw {
			name = "\x00" + name // a typedef name spelled like a keyword of another mode
		}
		// storage classes
		switch name {
		case "typedef", "static", "extern", "inline", "__inline", "__inline__", "_Thread_local", "thread_local",
			"__thread", "register", "_Noreturn", "constexpr", "__forceinline":
			if attr == nil {
				p.errorAt(t, "storage class specifier is not allowed in this context")
			}
			p.next()
			switch name {
			case "typedef":
				attr.isTypedef = true
			case "static":
				attr.isStatic = true
			case "extern":
				attr.isExtern = true
			case "inline", "__inline", "__inline__", "__forceinline":
				attr.isInline = true
			case "_Thread_local", "thread_local", "__thread":
				attr.isTLS = true
			case "register":
				attr.isRegister = true
			case "_Noreturn":
				attr.noreturn = true
			case "constexpr":
				attr.isConstexpr = true
				isConst = true
			}
			if attr.isTypedef && (attr.isStatic || attr.isExtern || attr.isInline || attr.isTLS) {
				p.errorAt(t, "typedef may not be used together with static, extern, inline or _Thread_local")
			}
			continue
		case "auto", "__auto_type":
			p.next()
			if attr != nil {
				attr.isAuto = true
			}
			continue
		case "const", "__const", "__const__":
			p.next()
			isConst = true
			continue
		case "volatile", "__volatile", "__volatile__":
			p.next()
			isVolatile = true
			continue
		case "restrict", "__restrict", "__restrict__":
			p.next()
			isRestrict = true
			continue
		case "__extension__", "__unaligned":
			p.next()
			continue
		case "_Atomic":
			p.next()
			if p.is("(") {
				p.next()
				ty = p.typename()
				p.expect(")")
				counter += OTHER
				isAtomic = true
				continue
			}
			isAtomic = true
			continue
		case "_Alignas", "alignas":
			if attr == nil {
				p.errorAt(t, "_Alignas is not allowed in this context")
			}
			p.next()
			p.expect("(")
			var al int64
			if p.isTypename(p.cur()) {
				al = p.typename().Align()
			} else {
				al = p.constExpr()
			}
			if al > attr.align {
				attr.align = al
			}
			p.expect(")")
			continue
		case "_Imaginary":
			p.errorAt(t, "imaginary types are not supported")
		case "_Decimal32", "_Decimal64", "_Decimal128":
			p.errorAt(t, "decimal floating types are not supported")
		}
		// user types
		isKw := typeKeywords[name] && !t.notKw
		if name == "struct" || name == "union" || name == "enum" || isKw && (name == "typeof" || name == "typeof_unqual") ||
			name == "__typeof" || name == "__typeof__" || name == "_BitInt" || (p.isTypedefName(t) && !isKw) {
			if counter != 0 {
				if p.isTypedefName(t) && !isKw {
					break // a typedef name after a type is a declarator
				}
				p.errorAt(t, "two or more data types in declaration specifiers")
			}
			p.next()
			switch name {
			case "struct", "union":
				ty = p.structDecl(name == "union")
			case "enum":
				ty = p.enumSpecifier()
			case "typeof", "__typeof", "__typeof__", "typeof_unqual":
				ty = p.typeofSpecifier()
				if name == "typeof_unqual" {
					ty = ty.Unqual()
				}
			case "_BitInt":
				p.expect("(")
				n := p.constExpr()
				p.expect(")")
				ty = bitIntType(n)
				if counter&UNSIGNED != 0 {
					ty = unsignedOf(ty)
				}
			default:
				ty = p.findVar(t.Text).typedef
			}
			counter += OTHER
			continue
		}
		p.next()
		switch name {
		case "void":
			counter += VOID
		case "_Bool", "bool":
			counter += BOOL
		case "char":
			counter += CHAR
		case "short":
			counter += SHORT
		case "int":
			counter += INT
		case "long":
			counter += LONG
		case "float":
			counter += FLOAT
		case "double":
			counter += DOUBLE
		case "signed", "__signed", "__signed__":
			counter |= SIGNED
		case "unsigned":
			counter |= UNSIGNED
		case "__int128":
			counter += INT128
		case "_Complex":
			counter |= COMPLEX
		default:
			p.errorAt(t, "unexpected '%s' in declaration specifiers", name)
		}
		if counter&COMPLEX != 0 {
			switch counter &^ COMPLEX {
			case 0, DOUBLE:
				ty = complexOf(tyDouble)
			case FLOAT:
				ty = complexOf(tyFloat)
			case LONG + DOUBLE:
				ty = complexOf(tyLDouble)
			case LONG:
				// _Complex long double: checked when the specifiers end
			default:
				p.errorAt(t, "complex integer types are not supported")
			}
			continue
		}
		switch counter {
		case VOID:
			ty = tyVoid
		case BOOL:
			ty = tyBool
		case CHAR:
			ty = tyChar
		case SIGNED + CHAR:
			ty = tySChar
		case UNSIGNED + CHAR:
			ty = tyUChar
		case SHORT, SHORT + INT, SIGNED + SHORT, SIGNED + SHORT + INT:
			ty = tyShort
		case UNSIGNED + SHORT, UNSIGNED + SHORT + INT:
			ty = tyUShort
		case INT, SIGNED, SIGNED + INT:
			ty = tyInt
		case UNSIGNED, UNSIGNED + INT:
			ty = tyUInt
		case LONG, LONG + INT, SIGNED + LONG, SIGNED + LONG + INT:
			ty = tyLong
		case LONG + LONG, LONG + LONG + INT, SIGNED + LONG + LONG, SIGNED + LONG + LONG + INT:
			ty = tyLLong
		case UNSIGNED + LONG, UNSIGNED + LONG + INT:
			ty = tyULong
		case UNSIGNED + LONG + LONG, UNSIGNED + LONG + LONG + INT:
			ty = tyULLong
		case INT128, SIGNED + INT128:
			ty = tyInt128
		case UNSIGNED + INT128:
			ty = tyUInt128
		case FLOAT:
			ty = tyFloat
		case DOUBLE:
			ty = tyDouble
		case LONG + DOUBLE:
			ty = tyLDouble
		default:
			p.errorAt(t, "invalid combination of type specifiers")
		}
	}
	if counter == COMPLEX+LONG {
		p.errorAt(start, "complex integer types are not supported")
	}
	if counter == 0 && attr != nil && attr.isAuto && p.opt.Std >= 2023 || counter == 0 && attr != nil && attr.isAuto && start.Text == "__auto_type" {
		ty = nil // type inferred from the initializer
		return nil
	}
	if counter == 0 && attr != nil && attr.isAuto {
		ty = tyInt // C89 implicit int: auto x = 1;
	}
	if counter == 0 && p.opt.Std >= 1999 && !(attr != nil && (attr.isTypedef || attr.isStatic || attr.isExtern || attr.isRegister || attr.isConstexpr || attr.isInline || attr.isAuto)) && !isConst && !isVolatile {
		if start.Kind == TIdent && !start.kw() {
			p.errorAt(start, "unknown type name '%s'", start.Text)
		}
		p.warnAt(start, "type defaults to 'int' in declaration")
	}
	if attr != nil && attr.mode != "" {
		ty = modeType(ty, attr.mode)
		attr.mode = ""
	}
	if attr != nil && attr.vecSize > 0 {
		ty = p.applyVectorSize(ty, attr.vecSize, attr.vecTok)
		attr.vecSize = 0
	}
	return ty.withQuals(isConst, isVolatile, isRestrict, isAtomic)
}

func unsignedOf(t *Type) *Type {
	switch t.Kind {
	case TyChar:
		return tyUChar
	case TyShort:
		return tyUShort
	case TyInt:
		return tyUInt
	case TyLong:
		return tyULong
	case TyLongLong:
		return tyULLong
	case TyInt128:
		return tyUInt128
	}
	return t
}

func signedOf(t *Type) *Type {
	switch t.Kind {
	case TyChar:
		return tySChar
	case TyShort:
		return tyShort
	case TyInt:
		return tyInt
	case TyLong:
		return tyLong
	case TyLongLong:
		return tyLLong
	case TyInt128:
		return tyInt128
	}
	return t
}

func bitIntType(n int64) *Type {
	switch {
	case n <= 8:
		return tySChar
	case n <= 16:
		return tyShort
	case n <= 32:
		return tyInt
	case n <= 64:
		return tyLong
	}
	return tyInt128
}

func modeType(t *Type, mode string) *Type {
	var r *Type
	switch mode {
	case "QI":
		r = tySChar
	case "HI":
		r = tyShort
	case "SI":
		r = tyInt
	case "DI", "word", "pointer":
		r = tyLong
	case "TI":
		r = tyInt128
	case "SF":
		return tyFloat
	case "DF":
		return tyDouble
	case "XF":
		return tyLDouble
	default:
		return t
	}
	if t.IsUnsigned() {
		r = unsignedOf(r)
	}
	return r
}

func (p *parser) typeofSpecifier() *Type {
	p.expect("(")
	var ty *Type
	if p.isTypename(p.cur()) {
		ty = p.typename()
	} else {
		p.inSizeof++
		n := p.expr()
		p.inSizeof--
		addType(n)
		ty = n.Ty
	}
	p.expect(")")
	return ty
}

// ---- declarators ----

func (p *parser) pointers(ty *Type) *Type {
	for p.consume("*") {
		ty = pointerTo(ty)
		for {
			t := p.cur()
			switch t.Text {
			case "const", "__const", "__const__":
				ty = ty.withQuals(true, false, false, false)
			case "volatile", "__volatile", "__volatile__":
				ty = ty.withQuals(false, true, false, false)
			case "restrict", "__restrict", "__restrict__":
				ty = ty.withQuals(false, false, true, false)
			case "_Atomic":
				ty = ty.withQuals(false, false, false, true)
			case "__attribute__", "__attribute", "__unaligned", "__extension__":
				if t.Text == "__unaligned" || t.Text == "__extension__" {
					p.next()
					continue
				}
				p.attributes(nil)
				continue
			default:
				goto nextStar
			}
			p.next()
		}
	nextStar:
	}
	return ty
}

// declarator parses a (possibly abstract) declarator. It returns the
// type and the name token (nil for abstract declarators).
func (p *parser) declarator(ty *Type, attr *declAttr) (*Type, *Token) {
	ty = p.pointers(ty)
	if p.is("(") && !p.isTypename(p.peekAt(1)) && !p.peekAt(1).Is(")") && !p.peekAt(1).Is("...") {
		start := p.pos
		p.next()
		p.attributes(attr)
		p.declarator(&Type{Kind: TyInt}, nil)
		p.expect(")")
		ty = p.typeSuffix(ty)
		end := p.pos
		p.pos = start + 1
		p.attributes(attr)
		ty, name := p.declarator(ty, attr)
		p.expect(")")
		p.pos = end
		return ty, name
	}
	var name *Token
	if t := p.cur(); t.Kind == TIdent && !t.kw() {
		name = p.next()
	}
	if attr != nil {
		p.attributes(attr)
	} else {
		p.attributes(&declAttr{})
	}
	ty = p.typeSuffix(ty)
	return ty, name
}

func (p *parser) typeSuffix(ty *Type) *Type {
	if p.is("(") {
		p.next()
		return p.funcParams(ty)
	}
	if p.is("[") && !p.peekAt(1).Is("[") {
		p.next()
		return p.arrayDims(ty)
	}
	return ty
}

func (p *parser) arrayDims(ty *Type) *Type {
	// [static qualifiers N], [*]
	for {
		t := p.cur()
		if t.Is("static") || t.Is("const") || t.Is("volatile") || t.Is("restrict") || t.Is("__restrict") || t.Is("_Atomic") {
			p.next()
			continue
		}
		break
	}
	if p.consume("]") {
		ty = p.typeSuffix(ty)
		return arrayOf(ty, -1)
	}
	if p.is("*") && p.peekAt(1).Is("]") {
		p.next()
		p.next()
		ty = p.typeSuffix(ty)
		return vlaOf(ty, &Node{Kind: NNum, Val: 1, Ty: tyLong})
	}
	tok := p.cur()
	n := p.conditional()
	p.expect("]")
	ty = p.typeSuffix(ty)
	addType(n)
	if ty.Kind == TyVLA || !p.isConstExpr(n) {
		if p.isGlobalScope() && p.fn == nil {
			p.errorAt(tok, "variable length array declaration not allowed at file scope")
		}
		return vlaOf(ty, n)
	}
	v := p.eval(n)
	if v < 0 {
		p.errorAt(tok, "size of array is negative")
	}
	return arrayOf(ty, v)
}

func (p *parser) funcParams(ret *Type) *Type {
	ft := funcType(ret)
	if p.is("void") && p.peekAt(1).Is(")") {
		p.next()
		p.next()
		return p.typeSuffixAfterFunc(ft)
	}
	if p.consume(")") {
		ft.OldStyle = p.opt.Std < 2023
		return p.typeSuffixAfterFunc(ft)
	}
	// K&R identifier list
	if t := p.cur(); t.Kind == TIdent && !p.isTypename(t) && (p.peekAt(1).Is(",") || p.peekAt(1).Is(")")) {
		ft.OldStyle = true
		for {
			name := p.ident()
			ft.ParamNames = append(ft.ParamNames, name)
			ft.Params = append(ft.Params, nil)
			if p.consume(")") {
				break
			}
			p.expect(",")
		}
		return ft
	}
	p.enterScope() // parameter scope for struct tags declared in params
	for !p.is(")") {
		if len(ft.Params) > 0 {
			p.expect(",")
		}
		if p.consume("...") {
			ft.Variadic = true
			break
		}
		pa := &declAttr{}
		bt := p.declspec(pa)
		if bt == nil {
			p.errorAt(p.cur(), "'auto' parameter type")
		}
		pt, name := p.declarator(bt, pa)
		pt = adjustParamType(pt)
		if pa.align > 0 {
			pt = pt.clone()
			pt.AlignAs = pa.align
		}
		ft.Params = append(ft.Params, pt)
		ft.ParamNames = append(ft.ParamNames, name)
		// later parameters may refer to this one (int n, int a[n])
		po := &Obj{Ty: pt, IsLocal: true, Align: pt.Align(), Tok: name}
		if name != nil {
			po.Name = name.Text
			p.pushVar(name.Text).obj = po
		}
		ft.ParamObjs = append(ft.ParamObjs, po)
	}
	p.leaveScope()
	p.expect(")")
	return p.typeSuffixAfterFunc(ft)
}

// typeSuffixAfterFunc handles attributes after a function declarator
func (p *parser) typeSuffixAfterFunc(ft *Type) *Type {
	if ft.Ret.Kind == TyArray {
		p.errorAt(p.cur(), "function cannot return an array")
	}
	return ft
}

// adjustParamType converts array and function parameters to pointers
func adjustParamType(t *Type) *Type {
	switch t.Kind {
	case TyArray, TyVLA:
		r := pointerTo(t.Base)
		return r.withQuals(t.Const, t.Volatile, t.Restrict, false)
	case TyFunc:
		return pointerTo(t)
	}
	return t
}

// abstractDeclarator parses a declarator without a name (casts, sizeof)
func (p *parser) abstractDeclarator(ty *Type) *Type {
	t, name := p.declarator(ty, nil)
	if name != nil {
		p.errorAt(name, "unexpected identifier in type name")
	}
	return t
}

// typename parses a type name
func (p *parser) typename() *Type {
	ty := p.declspec(nil)
	if ty == nil {
		p.errorAt(p.cur(), "expected type")
	}
	return p.abstractDeclarator(ty)
}

// ---- struct / union / enum ----

func (p *parser) structDecl(union bool) *Type {
	attr := &declAttr{}
	p.attributes(attr)
	var tag *Token
	if t := p.cur(); t.Kind == TIdent && !t.kw() {
		tag = p.next()
	}
	p.attributes(attr)
	if tag != nil && !p.is("{") {
		if t := p.findTag(tag.Text); t != nil && !(p.is(";") && p.sc.tags[tag.Text] == nil) {
			if t.IsRecord() && (t.Kind == TyUnion) != union {
				p.errorAt(tag, "'%s' defined as wrong kind of tag", tag.Text)
			}
			return t
		}
		ty := recordType(union)
		ty.Rec.Tag = tag.Text
		ty.Rec.Tok = tag
		p.sc.tags[tag.Text] = ty
		return ty
	}
	p.expect("{")
	var ty *Type
	if tag != nil {
		if old, ok := p.sc.tags[tag.Text]; ok && old.IsRecord() {
			if old.Rec.Complete {
				p.errorAt(tag, "redefinition of '%s %s'", map[bool]string{false: "struct", true: "union"}[union], tag.Text)
			}
			ty = old
		}
	}
	if ty == nil {
		ty = recordType(union)
		if tag != nil {
			ty.Rec.Tag = tag.Text
			ty.Rec.Tok = tag
			p.sc.tags[tag.Text] = ty
		}
	}
	p.structMembers(ty)
	p.attributes(attr)
	if attr.packed {
		ty.Rec.Packed = true
	}
	if attr.align > ty.Rec.Align {
		ty.Rec.Align = attr.align
	}
	layoutRecord(ty.Rec, p.pack[len(p.pack)-1])
	return ty
}

func (p *parser) structMembers(ty *Type) {
	r := ty.Rec
	idx := 0
	for !p.consume("}") {
		if p.is("_Static_assert") || p.is("static_assert") {
			p.staticAssert()
			continue
		}
		if p.consume(";") {
			continue
		}
		attr := &declAttr{}
		bt := p.declspec(attr)
		if bt == nil {
			p.errorAt(p.cur(), "member type required")
		}
		// anonymous struct/union member
		if bt.IsRecord() && p.consume(";") {
			m := &Member{Ty: bt, Tok: p.cur(), Idx: idx, Align: bt.Align(), Anon: true}
			if attr.align > m.Align {
				m.Align = attr.align
			}
			r.Members = append(r.Members, m)
			idx++
			continue
		}
		first := true
		for !p.consume(";") {
			if !first {
				p.expect(",")
			}
			first = false
			mattr := &declAttr{align: attr.align, packed: attr.packed}
			var mt *Type
			var name *Token
			if p.is(":") {
				mt = bt
			} else {
				mt, name = p.declarator(bt, mattr)
			}
			m := &Member{Ty: mt, Tok: name, Idx: idx, Align: mt.Align()}
			if name != nil {
				m.Name = name.Text
			}
			if mattr.mode != "" {
				m.Ty = modeType(m.Ty, mattr.mode)
				m.Align = m.Ty.Align()
			}
			if mattr.vecSize > 0 {
				m.Ty = p.applyVectorSize(m.Ty, mattr.vecSize, mattr.vecTok)
				m.Align = m.Ty.Align()
			}
			if p.consume(":") {
				w := p.constExpr()
				if !m.Ty.IsInteger() {
					p.errorAt(name, "bit-field has invalid type")
				}
				if w < 0 || w > m.Ty.Size()*8 {
					p.errorAt(p.cur(), "width of bit-field exceeds its type")
				}
				m.Bitfield = true
				m.BitWidth = w
			}
			p.attributes(mattr)
			if mattr.align > m.Align {
				m.Align = mattr.align
			}
			if mattr.packed {
				m.Align = 1
			}
			if mt.Kind == TyFunc {
				p.errorAt(name, "field declared as a function")
			}
			r.Members = append(r.Members, m)
			idx++
		}
	}
	// a flexible array member must be last
	for i, m := range r.Members {
		if m.Ty.Kind == TyArray && m.Ty.ArrayLen < 0 && i != len(r.Members)-1 {
			p.errorAt(m.Tok, "flexible array member not at end of struct")
		}
		if m.Ty.Kind != TyArray && !m.Ty.IsComplete() && m.Ty.Kind != TyVLA {
			p.errorAt(m.Tok, "field '%s' has incomplete type", m.Name)
		}
	}
}

func (p *parser) enumSpecifier() *Type {
	attr := &declAttr{}
	p.attributes(attr)
	var tag *Token
	if t := p.cur(); t.Kind == TIdent && !t.kw() {
		tag = p.next()
	}
	p.attributes(attr)
	var fixed *Type
	if p.is(":") && !p.peekAt(1).Is(":") {
		p.next()
		fixed = p.declspec(nil)
		fixed = p.abstractDeclarator(fixed).Unqual()
		if !fixed.IsInteger() {
			p.errorAt(p.cur(), "invalid underlying type for enum")
		}
	}
	if tag != nil && !p.is("{") {
		if t := p.findTag(tag.Text); t != nil {
			if t.Kind != TyEnum {
				p.errorAt(tag, "'%s' defined as wrong kind of tag", tag.Text)
			}
			return t
		}
		ty := enumType()
		ty.Enum.Tag = tag.Text
		ty.Enum.Fixed = fixed
		ty.Enum.Under = fixed
		p.sc.tags[tag.Text] = ty
		return ty
	}
	p.expect("{")
	ty := enumType()
	if tag != nil {
		if old, ok := p.sc.tags[tag.Text]; ok && old.Kind == TyEnum && !old.Enum.Complete {
			ty = old
		} else {
			ty.Enum.Tag = tag.Text
			p.sc.tags[tag.Text] = ty
		}
	}
	ty.Enum.Fixed = fixed
	val := int64(0)
	first := true
	var minV, maxV int64
	var consts []*varScope
	for !p.consume("}") {
		if !first {
			p.expect(",")
			if p.consume("}") {
				break
			}
		}
		first = false
		name := p.ident()
		p.attributes(&declAttr{})
		if p.consume("=") {
			val = p.constExpr()
		}
		vs := p.pushVar(name.Text)
		vs.enumTy = ty
		vs.enumVal = val
		consts = append(consts, vs)
		if len(consts) == 1 || val < minV {
			minV = val
		}
		if len(consts) == 1 || val > maxV {
			maxV = val
		}
		val++
	}
	p.attributes(attr)
	switch {
	case fixed != nil:
		ty.Enum.Under = fixed
	case minV >= 0 && maxV <= 0xFFFFFFFF && !(attr.packed):
		if maxV <= 0x7FFFFFFF {
			ty.Enum.Under = tyUInt
		} else {
			ty.Enum.Under = tyUInt
		}
	case minV >= -0x80000000 && maxV <= 0x7FFFFFFF && !attr.packed:
		ty.Enum.Under = tyInt
	case attr.packed:
		switch {
		case minV >= 0 && maxV <= 0xFF:
			ty.Enum.Under = tyUChar
		case minV >= -128 && maxV <= 127:
			ty.Enum.Under = tySChar
		case minV >= 0 && maxV <= 0xFFFF:
			ty.Enum.Under = tyUShort
		case minV >= -32768 && maxV <= 32767:
			ty.Enum.Under = tyShort
		default:
			ty.Enum.Under = tyInt
		}
	case minV >= 0:
		ty.Enum.Under = tyULong
	default:
		ty.Enum.Under = tyLong
	}
	ty.Enum.Complete = true
	return ty
}

// enumConstType is the type of an enumeration constant
func enumConstType(e *varScope) *Type {
	if e.enumTy.Enum.Fixed != nil {
		return e.enumTy
	}
	v := e.enumVal
	if v >= -0x80000000 && v <= 0x7FFFFFFF {
		return tyInt
	}
	return e.enumTy.Enum.underlying()
}

// ---- top level ----

func (p *parser) program() {
	for p.cur().Kind != TEOF {
		start := p.pos
		func() {
			defer func() {
				if r := recover(); r != nil {
					f, ok := r.(fatal)
					if !ok {
						panic(r)
					}
					p.errs = append(p.errs, f.err)
					p.fn = nil
					for p.sc.parent != nil {
						p.leaveScope()
					}
					p.recover(start)
				}
			}()
			p.topLevel()
		}()
		if len(p.errorsOnly()) >= 20 {
			p.errs = append(p.errs, &Error{Msg: "too many errors, stopping"})
			return
		}
	}
}

func (p *parser) errorsOnly() []*Error {
	var r []*Error
	for _, e := range p.errs {
		if !e.Warning {
			r = append(r, e)
		}
	}
	return r
}

// recover skips to the end of the broken top-level declaration
func (p *parser) recover(start int) {
	if p.pos <= start {
		p.pos = start + 1
	}
	// find the end: a ';' or '}' at brace depth 0, counting from the declaration start
	depth := 0
	for i := start; i < len(p.toks); i++ {
		t := p.toks[i]
		if t.Kind == TEOF {
			p.pos = i
			return
		}
		if t.Kind != TPunct {
			continue
		}
		switch t.Text {
		case "{":
			depth++
		case "}":
			depth--
			if depth <= 0 {
				if i+1 >= p.pos {
					p.pos = i + 1
					if p.toks[p.pos].Is(";") {
						p.pos++
					}
					return
				}
				depth = 0
			}
		case ";":
			if depth == 0 && i+1 >= p.pos {
				p.pos = i + 1
				return
			}
		}
	}
}

func (p *parser) topLevel() {
	if p.consume(";") {
		return
	}
	if p.is("_Static_assert") || p.is("static_assert") {
		p.staticAssert()
		return
	}
	if p.is("asm") || p.is("__asm__") || p.is("__asm") {
		t := p.next()
		p.expect("(")
		text := p.asmString()
		p.expect(")")
		p.expect(";")
		p.prog.TopAsm = append(p.prog.TopAsm, text)
		p.prog.AsmToks = append(p.prog.AsmToks, t)
		return
	}
	attr := &declAttr{}
	basety := p.declspec(attr)
	if basety == nil {
		basety = tyInt
	}
	if attr.isTypedef {
		p.typedefDecl(basety, attr)
		return
	}
	if p.consume(";") {
		return
	}
	// function definition?
	save := p.pos
	ty, name := p.declarator(basety, attr)
	if ty.Kind == TyFunc && name != nil {
		p.attributes(attr)
		asmLabel := p.asmLabel()
		p.attributes(attr)
		if p.is("{") || (ty.OldStyle && p.isTypename(p.cur())) {
			p.function(ty, name, attr, asmLabel)
			return
		}
	}
	p.pos = save
	p.globalDecl(basety, attr)
}

func (p *parser) asmLabel() string {
	if p.is("asm") || p.is("__asm__") || p.is("__asm") {
		p.next()
		p.expect("(")
		s := ""
		for p.cur().Kind == TStr {
			_, body := literalPrefix(p.next().Text)
			s += body
		}
		p.expect(")")
		return s
	}
	return ""
}

func (p *parser) typedefDecl(basety *Type, attr *declAttr) []*Token {
	var names []*Token
	first := true
	for !p.consume(";") {
		if !first {
			p.expect(",")
		}
		first = false
		a := &declAttr{align: attr.align, mode: attr.mode}
		ty, name := p.declarator(basety, a)
		p.attributes(a)
		if name == nil {
			p.errorAt(p.cur(), "typedef name omitted")
		}
		if a.mode != "" {
			ty = modeType(ty, a.mode)
		}
		if a.vecSize > 0 {
			ty = p.applyVectorSize(ty, a.vecSize, a.vecTok)
		}
		if a.align > 0 || a.packed {
			ty = ty.clone()
			ty.Origin = nil
			if a.align > 0 {
				ty.AlignAs = a.align
			}
		}
		if ty.Typedef == "" && !ty.IsRecord() && ty.Kind != TyEnum {
			c := ty.clone()
			c.Typedef = name.Text
			ty = c
		}
		if old, ok := p.sc.vars[name.Text]; ok && old.typedef != nil && !compatible(old.typedef, ty) {
			p.errorAt(name, "conflicting types for '%s'", name.Text)
		}
		p.pushVar(name.Text).typedef = ty
		names = append(names, name)
	}
	return names
}

func (p *parser) staticAssert() {
	t := p.next()
	p.expect("(")
	v := p.constExpr()
	msg := ""
	if p.consume(",") {
		for p.cur().Kind == TStr {
			_, body := literalPrefix(p.next().Text)
			msg += body
		}
	}
	p.expect(")")
	p.expect(";")
	if v == 0 {
		if msg == "" {
			p.softError(t, "static assertion failed")
		} else {
			p.softError(t, "static assertion failed: \"%s\"", msg)
		}
	}
}

// declareGlobal finds or creates the global object for a declaration
func (p *parser) declareGlobal(name *Token, ty *Type, attr *declAttr) *Obj {
	if v := p.findVar(name.Text); v != nil && v.obj != nil && !v.obj.IsLocal {
		o := v.obj
		if o.IsFunction != (ty.Kind == TyFunc) || !compatible(o.Ty, ty) {
			if !(o.Ty.Kind == TyArray && ty.Kind == TyArray && compatible(o.Ty.Base, ty.Base)) {
				p.errorAt(name, "conflicting types for '%s'", name.Text)
			}
		}
		// complete an array type: extern int a[]; int a[10];
		if o.Ty.Kind == TyArray && o.Ty.ArrayLen < 0 && ty.Kind == TyArray && ty.ArrayLen >= 0 {
			o.Ty = ty
		}
		// a prototype after an old-style declaration
		if o.IsFunction && o.Ty.OldStyle && !ty.OldStyle {
			o.Ty = ty
		}
		if v != p.sc.vars[name.Text] && p.sc.parent != nil {
			p.sc.vars[name.Text] = v
		}
		if attr.isStatic {
			o.IsStatic = true
		}
		return o
	}
	o := &Obj{Name: name.Text, AsmName: name.Text, Ty: ty, Tok: name, Align: ty.Align(), IsFunction: ty.Kind == TyFunc}
	o.IsStatic = attr.isStatic
	o.IsExtern = attr.isExtern
	if p.sc.parent == nil {
		p.pushVar(name.Text).obj = o
	} else {
		// block-scope extern: visible here, but the object is global
		p.pushVar(name.Text).obj = o
		gs := p.sc
		for gs.parent != nil {
			gs = gs.parent
		}
		if _, ok := gs.vars[name.Text]; !ok {
			gs.vars[name.Text] = &varScope{obj: o}
		}
	}
	if old := p.globalsByName[name.Text]; old != nil && old != o {
		// declared in an inner scope earlier
		return old
	}
	p.globalsByName[name.Text] = o
	p.prog.Globals = append(p.prog.Globals, o)
	return o
}

func (p *parser) applyAttrs(o *Obj, a *declAttr) {
	if a.weak {
		o.IsWeak = true
	}
	if a.alias != "" {
		o.Alias = a.alias
		o.IFunc = a.ifunc
	}
	if a.section != "" {
		o.Section = a.section
	}
	if a.hidden {
		o.IsHidden = true
	}
	if a.ctor {
		o.Ctor = true
	}
	if a.prio > 0 {
		o.Priority = a.prio
	}
	if a.dtor {
		o.Dtor = true
	}
	if a.noreturn {
		o.Noreturn = true
	}
	if a.alwaysInline {
		o.AlwaysInline = true
	}
	if a.target != "" {
		o.Target = a.target
	}
	if len(a.targetClones) > 0 {
		o.TargetClones = a.targetClones
	}
	if a.noinline {
		o.NoInline = true
	}
	if a.used {
		o.IsUsed = true
	}
	if a.asmName != "" {
		o.AsmName = a.asmName
	}
	if a.align > o.Align {
		o.Align = a.align
	}
}

func (p *parser) globalDecl(basety *Type, attr *declAttr) {
	first := true
	for !p.consume(";") {
		if !first {
			p.expect(",")
		}
		first = false
		a := *attr
		ty, name := p.declarator(basety, &a)
		if ty == nil {
			p.errorAt(p.cur(), "missing type")
		}
		p.attributes(&a)
		a.asmName = p.asmLabel()
		p.attributes(&a)
		if name == nil {
			p.errorAt(p.cur(), "variable name omitted")
		}
		if a.mode != "" {
			ty = modeType(ty, a.mode)
		}
		if a.vecSize > 0 {
			ty = p.applyVectorSize(ty, a.vecSize, a.vecTok)
		}
		if ty.Kind == TyFunc {
			o := p.declareGlobal(name, ty, &a)
			if a.isInline {
				o.IsInline = true
			}
			p.applyAttrs(o, &a)
			continue
		}
		if ty.Kind == TyVoid {
			p.errorAt(name, "variable '%s' declared void", name.Text)
		}
		o := p.declareGlobal(name, ty, &a)
		p.applyAttrs(o, &a)
		if a.isTLS {
			o.IsTLS = true
		}
		if ty.Kind == TyVLA {
			p.errorAt(name, "variable length array at file scope")
		}
		if p.consume("=") {
			if o.IsDefinition && !o.IsTentative {
				p.errorAt(name, "redefinition of '%s'", name.Text)
			}
			o.IsExtern = false
			o.IsDefinition = true
			o.IsTentative = false
			if a.isConstexpr && ty.IsScalar() {
				save := p.pos
				e := p.assign()
				addType(e)
				if p.isConstExpr(e) {
					o.ConstExpr = newCast(e, ty.Unqual())
				} else {
					p.softError(name, "constexpr initializer for '%s' is not a constant expression", name.Text)
				}
				p.pos = save
			}
			if basety == nil || a.isAuto && ty.Kind == TyInt && p.opt.Std >= 2023 {
				// handled by declspec returning nil (auto)
			}
			p.gvarInitializer(o)
			if a.isConstexpr {
				o.IsConst = true
			}
			continue
		}
		if basety == nil {
			p.errorAt(name, "'auto' requires an initializer")
		}
		if !a.isExtern && !o.IsDefinition {
			o.IsDefinition = true
			o.IsTentative = true
		}
	}
}
