package cc

import (
	"strings"
)

// evalIf evaluates the controlling expression of #if / #elif
func (pp *Preprocessor) evalIf(t *Token, line []*Token) bool {
	return pp.evalInt(t, line) != 0
}

func (pp *Preprocessor) num(t *Token, v int64) *Token {
	n := t.copy()
	n.Kind = TNum
	if v < 0 {
		n.Text = "0"
	} else {
		n.Text = itoa(v)
	}
	return n
}

func itoa(v int64) string {
	if v == 0 {
		return "0"
	}
	var b [24]byte
	i := len(b)
	neg := v < 0
	u := uint64(v)
	if neg {
		u = uint64(-v)
	}
	for u > 0 {
		i--
		b[i] = byte('0' + u%10)
		u /= 10
	}
	if neg {
		i--
		b[i] = '-'
	}
	return string(b[i:])
}

// replaceSpecial resolves defined(X), __has_include(...) and similar operators
func (pp *Preprocessor) replaceSpecial(line []*Token) []*Token {
	var out []*Token
	for i := 0; i < len(line); i++ {
		t := line[i]
		if t.Kind != TIdent {
			out = append(out, t)
			continue
		}
		switch t.Text {
		case "defined":
			j := i + 1
			paren := j < len(line) && line[j].Is("(")
			if paren {
				j++
			}
			if j >= len(line) || line[j].Kind != TIdent {
				pp.errorf(t, "operator \"defined\" requires an identifier")
				out = append(out, pp.num(t, 0))
				continue
			}
			_, def := pp.macros[line[j].Text]
			if paren {
				j++
				if j >= len(line) || !line[j].Is(")") {
					pp.errorf(t, "missing ')' after \"defined\"")
				}
			}
			v := int64(0)
			if def {
				v = 1
			}
			out = append(out, pp.num(t, v))
			i = j
		case "__has_include", "__has_include_next", "__has_embed":
			j := i + 1
			if j >= len(line) || !line[j].Is("(") {
				pp.errorf(t, "missing '(' after \"%s\"", t.Text)
				out = append(out, pp.num(t, 0))
				continue
			}
			depth := 0
			k := j
			for ; k < len(line); k++ {
				if line[k].Is("(") {
					depth++
				} else if line[k].Is(")") {
					depth--
					if depth == 0 {
						break
					}
				}
			}
			arg := line[j+1 : min(k, len(line))]
			name, quoted, ok := pp.includeName(arg)
			if !ok {
				arg = pp.expandList(arg)
				name, quoted, ok = pp.includeName(arg)
			}
			v := int64(0)
			if ok {
				if _, found := pp.findInclude(pp.currentFileSource(), name, quoted, t.Text == "__has_include_next"); found {
					v = 1
				}
			}
			out = append(out, pp.num(t, v))
			i = k
		case "__has_builtin", "__has_attribute", "__has_c_attribute", "__has_feature", "__has_extension",
			"__has_cpp_attribute", "__has_declspec_attribute":
			j := i + 1
			if j >= len(line) || !line[j].Is("(") {
				out = append(out, pp.num(t, 0))
				continue
			}
			depth := 0
			k := j
			for ; k < len(line); k++ {
				if line[k].Is("(") {
					depth++
				} else if line[k].Is(")") {
					depth--
					if depth == 0 {
						break
					}
				}
			}
			var name strings.Builder
			for _, a := range line[j+1 : min(k, len(line))] {
				name.WriteString(a.Text)
			}
			n := name.String()
			v := int64(0)
			switch t.Text {
			case "__has_builtin":
				if HasBuiltin(n) {
					v = 1
				}
			case "__has_attribute":
				if knownAttributes[strings.Trim(strings.TrimPrefix(n, "gnu::"), "_")] {
					v = 1
				}
			case "__has_c_attribute":
				v = cAttributes[strings.TrimPrefix(strings.Trim(n, "_"), "gnu::")]
			}
			out = append(out, pp.num(t, v))
			i = k
		default:
			out = append(out, t)
		}
	}
	return out
}

func (pp *Preprocessor) currentFileSource() *source {
	for i := len(pp.srcs) - 1; i >= 0; i-- {
		if pp.srcs[i].file != nil {
			return pp.srcs[i]
		}
	}
	return pp.top()
}

var knownAttributes = map[string]bool{
	"aligned": true, "packed": true, "noreturn": true, "unused": true, "used": true, "always_inline": true,
	"noinline": true, "format": true, "deprecated": true, "weak": true, "alias": true, "ifunc": true, "visibility": true,
	"constructor": true, "destructor": true, "cold": true, "hot": true, "pure": true, "const": true,
	"malloc": true, "nonnull": true, "warn_unused_result": true, "sentinel": true, "section": true,
	"fallthrough": true, "may_alias": true, "returns_nonnull": true, "nothrow": true, "leaf": true,
	"format_arg": true, "alloc_size": true, "noclone": true, "no_instrument_function": true,
	"artificial": true, "gnu_inline": true, "flatten": true, "maybe_unused": true, "nodiscard": true,
	"cleanup": true, "transparent_union": false,
}

var cAttributes = map[string]int64{
	"deprecated": 201904, "fallthrough": 201904, "maybe_unused": 201904, "nodiscard": 202003,
	"noreturn": 202202, "_Noreturn": 202202,
}

// evalInt evaluates a preprocessor constant expression
func (pp *Preprocessor) evalInt(t *Token, line []*Token) int64 {
	line = pp.replaceSpecial(line)
	pp.inIf = true
	line = pp.expandList(line)
	pp.inIf = false
	line = pp.replaceSpecial(line)
	toks := make([]*Token, 0, len(line))
	for _, x := range line {
		if x.Kind == TIdent {
			v := int64(0)
			if x.Text == "true" {
				v = 1
			}
			x = pp.num(x, v)
		}
		toks = append(toks, x)
	}
	if len(toks) == 0 {
		pp.errorf(t, "#%s with no expression", t.Text)
		return 0
	}
	e := &ppExpr{pp: pp, toks: toks, at: t}
	v := e.cond()
	if e.pos < len(e.toks) && !e.failed {
		pp.errorf(e.toks[e.pos], "missing binary operator before token \"%s\"", e.toks[e.pos].Text)
	}
	return v.v
}

type ppVal struct {
	v int64
	u bool
}

type ppExpr struct {
	pp     *Preprocessor
	toks   []*Token
	pos    int
	at     *Token
	failed bool
	skip   int // > 0 while evaluating a branch that is not taken (no division errors)
}

func (e *ppExpr) peek() *Token {
	if e.pos < len(e.toks) {
		return e.toks[e.pos]
	}
	return nil
}

func (e *ppExpr) accept(s string) bool {
	if t := e.peek(); t != nil && t.Kind == TPunct && t.Text == s {
		e.pos++
		return true
	}
	return false
}

func (e *ppExpr) fail(t *Token, msg string) ppVal {
	if !e.failed {
		if t == nil {
			t = e.at
		}
		e.pp.errorf(t, "%s", msg)
	}
	e.failed = true
	e.pos = len(e.toks)
	return ppVal{}
}

func arith(a, b ppVal) bool { return a.u || b.u }

func (e *ppExpr) cond() ppVal {
	c := e.binary(0)
	if e.accept("?") {
		if c.v == 0 {
			e.skip++
		}
		a := e.cond()
		if c.v == 0 {
			e.skip--
		}
		if !e.accept(":") {
			return e.fail(e.peek(), "expected ':' in preprocessor expression")
		}
		if c.v != 0 {
			e.skip++
		}
		b := e.cond()
		if c.v != 0 {
			e.skip--
		}
		u := arith(a, b)
		if c.v != 0 {
			return ppVal{a.v, u}
		}
		return ppVal{b.v, u}
	}
	if e.accept(",") {
		return e.cond()
	}
	return c
}

var ppPrec = map[string]int{
	"||": 1, "&&": 2, "|": 3, "^": 4, "&": 5, "==": 6, "!=": 6,
	"<": 7, ">": 7, "<=": 7, ">=": 7, "<<": 8, ">>": 8, "+": 9, "-": 9, "*": 10, "/": 10, "%": 10,
}

func b2i(b bool) int64 {
	if b {
		return 1
	}
	return 0
}

func (e *ppExpr) binary(minPrec int) ppVal {
	lhs := e.unary()
	for {
		t := e.peek()
		if t == nil || t.Kind != TPunct {
			return lhs
		}
		prec, ok := ppPrec[t.Text]
		if !ok || prec <= minPrec {
			return lhs
		}
		e.pos++
		op := t.Text
		short := (op == "&&" && lhs.v == 0) || (op == "||" && lhs.v != 0)
		if short {
			e.skip++
		}
		rhs := e.binary(prec)
		if short {
			e.skip--
		}
		u := arith(lhs, rhs)
		a, b := lhs.v, rhs.v
		var r int64
		switch op {
		case "||":
			r, u = b2i(a != 0 || b != 0), false
		case "&&":
			r, u = b2i(a != 0 && b != 0), false
		case "|":
			r = a | b
		case "^":
			r = a ^ b
		case "&":
			r = a & b
		case "==":
			r, u = b2i(a == b), false
		case "!=":
			r, u = b2i(a != b), false
		case "<":
			if u {
				r = b2i(uint64(a) < uint64(b))
			} else {
				r = b2i(a < b)
			}
			u = false
		case ">":
			if u {
				r = b2i(uint64(a) > uint64(b))
			} else {
				r = b2i(a > b)
			}
			u = false
		case "<=":
			if u {
				r = b2i(uint64(a) <= uint64(b))
			} else {
				r = b2i(a <= b)
			}
			u = false
		case ">=":
			if u {
				r = b2i(uint64(a) >= uint64(b))
			} else {
				r = b2i(a >= b)
			}
			u = false
		case "<<":
			r = a << uint64(b&63)
			u = lhs.u
		case ">>":
			if lhs.u {
				r = int64(uint64(a) >> uint64(b&63))
			} else {
				r = a >> uint64(b&63)
			}
			u = lhs.u
		case "+":
			r = a + b
		case "-":
			r = a - b
		case "*":
			r = a * b
		case "/", "%":
			if b == 0 {
				if e.skip == 0 {
					return e.fail(t, "division by zero in #if")
				}
				r = 0
			} else if u {
				if op == "/" {
					r = int64(uint64(a) / uint64(b))
				} else {
					r = int64(uint64(a) % uint64(b))
				}
			} else if op == "/" {
				if b == -1 {
					r = -a
				} else {
					r = a / b
				}
			} else if b == -1 {
				r = 0
			} else {
				r = a % b
			}
		}
		lhs = ppVal{r, u}
	}
}

func (e *ppExpr) unary() ppVal {
	t := e.peek()
	if t == nil {
		return e.fail(e.at, "#if with no expression")
	}
	if t.Kind == TPunct {
		switch t.Text {
		case "+":
			e.pos++
			return e.unary()
		case "-":
			e.pos++
			v := e.unary()
			return ppVal{-v.v, v.u}
		case "~":
			e.pos++
			v := e.unary()
			return ppVal{^v.v, v.u}
		case "!":
			e.pos++
			v := e.unary()
			return ppVal{b2i(v.v == 0), false}
		case "(":
			e.pos++
			v := e.cond()
			if !e.accept(")") {
				return e.fail(e.peek(), "missing ')' in expression")
			}
			return v
		}
	}
	e.pos++
	switch t.Kind {
	case TNum:
		n, err := parseNumber(t.Text)
		if err != nil {
			return e.fail(t, err.Error())
		}
		if n.isFloat {
			return e.fail(t, "floating constant in preprocessor expression")
		}
		return ppVal{int64(n.i), n.unsigned || n.i > 1<<63-1}
	case TChar:
		v, _, err := charValue(t.Text)
		if err != nil {
			return e.fail(t, err.Error())
		}
		return ppVal{v, false}
	}
	return e.fail(t, "token \""+t.Text+"\" is not valid in preprocessor expressions")
}
