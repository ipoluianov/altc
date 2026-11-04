package x64

// Assembler for AT&T syntax text: the bodies of inline asm statements and
// file-scope asm. It covers the general-purpose, SSE (up to SSE4.2, AES,
// SHA, PCLMUL) and x87 instruction sets, local and numeric labels and the
// common data and section directives.

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/ipoluianov/altc/elf"
)

// TextEnv connects assembly text with the code generator
type TextEnv struct {
	// Section returns the assembler of a section; ".text" is the code being
	// generated. flags are the ELF flags of a .section directive ("" when absent).
	Section func(name, flags string) (*Asm, error)
	// Labels are the named labels shared by all asm texts of a scope
	Labels map[string]*TextLabel
	// Symbol handles .globl/.weak/.hidden/.local/.type/.size (nil: ignored)
	Symbol func(directive string, args []string) error
	// Listing receives the text (for -S)
	Listing *strings.Builder
	// Consts are the absolute symbols defined with .set / .equ / =
	Consts map[string]int64
	// FileName, when set, prefixes errors with file:line (assembler sources)
	FileName string
}

// TextLabel is a named label of assembly text
type TextLabel struct {
	A       *Asm
	L       Label
	Defined bool
	Off     int64 // .set alias = label + Off
}

type labelRef struct {
	a *Asm
	l Label
}

type textAsm struct {
	env      *TextEnv
	a        *Asm
	secName  string
	stack    []secEntry
	prev     secEntry
	numBack  map[string]labelRef
	numFwd   map[string]labelRef
	defNames map[string]bool
	labelSec map[string]string // section of each label defined in the text
	globals  map[string]bool   // names made global by .globl/.weak

	// prefixes written by separate words (lock, rep...) just before
	pfxStart, pfxEnd int
	pfxAsm           *Asm
	secFlags         map[string]string // flags of .section directives
	cur              *instDef

	// jump relaxation
	short    map[int]bool // ordinals of jumps emitted in the short form
	trial    bool         // trial pass: record the jumps
	branchN  int
	branches []branchInfo
}

type secEntry struct {
	name string
	a    *Asm
}

// AssembleText assembles AT&T syntax text. Jumps to labels of the same
// section use the short form when the target is in range (like GNU as).
func AssembleText(env *TextEnv, text string) error {
	if env.Labels == nil {
		env.Labels = map[string]*TextLabel{}
	}
	short := relaxBranches(env, text)
	_, err := assemblePass(env, text, short, false)
	return err
}

// branchInfo describes a relaxable jump in a trial pass
type branchInfo struct {
	ord    int
	a      *Asm
	end    int // end of the instruction
	label  Label
	addend int64
}

// relaxBranches chooses the jumps that can be short: start with all of
// them short and lengthen those out of range until the layout is stable
func relaxBranches(env *TextEnv, text string) map[int]bool {
	trial := func(short map[int]bool) ([]branchInfo, bool) {
		ext := &Asm{}
		ext.NewLabel()
		secs := map[string]*Asm{}
		tenv := &TextEnv{
			Section: func(name, flags string) (*Asm, error) {
				if a, ok := secs[name]; ok {
					return a, nil
				}
				a := &Asm{}
				secs[name] = a
				return a, nil
			},
			Labels: map[string]*TextLabel{},
			Symbol: func(string, []string) error { return nil },
			Consts: env.Consts,
		}
		for name := range env.Labels {
			tenv.Labels[name] = &TextLabel{A: ext, L: 0, Defined: true}
		}
		infos, err := assemblePass(tenv, text, short, true)
		return infos, err == nil
	}
	infos, ok := trial(nil)
	if !ok || len(infos) == 0 {
		return nil
	}
	short := map[int]bool{}
	for _, b := range infos {
		short[b.ord] = true
	}
	for iter := 0; iter < 32; iter++ {
		infos, ok = trial(short)
		if !ok {
			return nil
		}
		changed := false
		for _, b := range infos {
			if !short[b.ord] {
				continue
			}
			pos := b.a.labels[b.label]
			if pos < 0 || !fitsI8(int64(pos)+b.addend-int64(b.end)) {
				delete(short, b.ord)
				changed = true
			}
		}
		if !changed {
			return short
		}
	}
	return nil
}

func assemblePass(env *TextEnv, text string, short map[int]bool, trial bool) ([]branchInfo, error) {
	t := &textAsm{env: env, numBack: map[string]labelRef{}, numFwd: map[string]labelRef{}, defNames: map[string]bool{},
		short: short, trial: trial}
	a, err := env.Section(".text", "")
	if err != nil {
		return nil, err
	}
	t.a, t.secName = a, ".text"
	t.prev = secEntry{t.secName, t.a}
	stmts := splitAsm(text)
	// pre-pass: the section of every named label, following the section directives
	sim := &secSim{cur: ".text", prev: ".text"}
	t.labelSec = map[string]string{}
	t.secFlags = map[string]string{}
	t.globals = map[string]bool{}
	for _, st := range stmts {
		s := st.text
		for {
			name, rest, ok := leadingLabel(s)
			if !ok {
				break
			}
			if !isDigits(name) {
				t.defNames[name] = true
				t.labelSec[name] = sim.cur
			}
			s = rest
		}
		s = strings.TrimSpace(s)
		sim.apply(s, t.secFlags)
		if w, rest := splitWord(s); w == ".globl" || w == ".global" || w == ".weak" {
			for _, n := range splitTop(rest) {
				t.globals[strings.TrimSpace(n)] = true
			}
		}
	}
	if env.Listing != nil {
		env.Listing.WriteString("#APP\n")
		for _, line := range strings.Split(strings.TrimRight(text, "\n"), "\n") {
			env.Listing.WriteString("\t" + strings.TrimSpace(line) + "\n")
		}
		env.Listing.WriteString("#NO_APP\n")
	}
	for _, st := range stmts {
		saved := t.a.Listing
		t.a.Listing = nil
		err := t.stmt(st.text)
		t.a.Listing = saved
		if err != nil {
			if env.FileName != "" {
				file := st.file
				if file == "" {
					file = env.FileName
				}
				return nil, fmt.Errorf("%s:%d: %v", file, st.line, err)
			}
			return nil, fmt.Errorf("%v (in '%s')", err, strings.TrimSpace(st.text))
		}
	}
	for n := range t.numFwd {
		return nil, fmt.Errorf("undefined local label '%sf'", n)
	}
	return t.branches, nil
}

// asmStmt is a statement of assembly text with its position
type asmStmt struct {
	text string
	file string
	line int
}

// splitAsm splits text into statements, removing comments; "# N file"
// line markers of preprocessed text set the positions
func splitAsm(text string) []asmStmt {
	var out []asmStmt
	var cur strings.Builder
	inStr := false
	file, line := "", 1
	stmtLine := 1
	flush := func() {
		out = append(out, asmStmt{cur.String(), file, stmtLine})
		cur.Reset()
		stmtLine = line
	}
	atBOL := true
	for i := 0; i < len(text); i++ {
		c := text[i]
		if atBOL && c == '#' {
			// line marker: # 12 "file.S"
			end := strings.IndexByte(text[i:], '\n')
			if end < 0 {
				end = len(text) - i
			}
			var n int
			var name string
			if k, _ := fmt.Sscanf(text[i:i+end], "# %d %q", &n, &name); k == 2 {
				flush()
				file, line = name, n
				stmtLine = line
				i += end
				continue
			}
		}
		if c != ' ' && c != '\t' {
			atBOL = false
		}
		switch {
		case inStr:
			cur.WriteByte(c)
			if c == '\\' && i+1 < len(text) {
				i++
				cur.WriteByte(text[i])
			} else if c == '"' {
				inStr = false
			}
		case c == '"':
			inStr = true
			cur.WriteByte(c)
		case c == '#':
			for i+1 < len(text) && text[i+1] != '\n' {
				i++
			}
		case c == '/' && i+1 < len(text) && text[i+1] == '*':
			end := strings.Index(text[i+2:], "*/")
			if end < 0 {
				end = len(text) - i - 2
			}
			line += strings.Count(text[i:i+2+end], "\n")
			i += end + 3
			cur.WriteByte(' ')
		case c == '\n':
			line++
			flush()
			atBOL = true
		case c == ';':
			flush()
		default:
			cur.WriteByte(c)
		}
	}
	flush()
	return out
}

func isIdentChar(c byte, first bool) bool {
	return c == '_' || c == '.' || c == '$' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || !first && c >= '0' && c <= '9'
}

func isDigits(s string) bool {
	if s == "" {
		return false
	}
	for i := 0; i < len(s); i++ {
		if s[i] < '0' || s[i] > '9' {
			return false
		}
	}
	return true
}

// leadingLabel splits "name: rest"
func leadingLabel(s string) (name, rest string, ok bool) {
	s = strings.TrimSpace(s)
	i := 0
	if i < len(s) && s[i] >= '0' && s[i] <= '9' {
		for i < len(s) && s[i] >= '0' && s[i] <= '9' {
			i++
		}
	} else {
		for i < len(s) && isIdentChar(s[i], i == 0) {
			i++
		}
	}
	if i == 0 || i >= len(s) || s[i] != ':' {
		return "", s, false
	}
	return s[:i], s[i+1:], true
}

func (t *textAsm) defineLabel(name string) error {
	if isDigits(name) {
		l := t.a.NewLabel()
		if r, ok := t.numFwd[name]; ok {
			if r.a != t.a {
				return fmt.Errorf("local label '%s' is referenced from another section", name)
			}
			l = r.l
			delete(t.numFwd, name)
		}
		t.a.Bind(l)
		t.numBack[name] = labelRef{t.a, l}
		return nil
	}
	tl := t.env.Labels[name]
	if tl == nil {
		tl = &TextLabel{A: t.a, L: t.a.NewLabel()}
		t.env.Labels[name] = tl
	}
	if tl.Defined {
		return fmt.Errorf("symbol '%s' is already defined", name)
	}
	if tl.A != t.a {
		return fmt.Errorf("label '%s' is referenced from another section before its definition", name)
	}
	tl.Defined = true
	t.a.Bind(tl.L)
	return nil
}

// ---- statements ----

var prefixBytes = map[string]byte{
	"lock": 0xF0, "rep": 0xF3, "repe": 0xF3, "repz": 0xF3, "repne": 0xF2, "repnz": 0xF2,
	"data16": 0x66, "addr32": 0x67, "cs": 0x2E, "ds": 0x3E, "es": 0x26, "fs": 0x64, "gs": 0x65, "ss": 0x36,
	"notrack": 0x3E, "rex64": 0x48, "rex": 0x40,
}

func (t *textAsm) stmt(s string) error {
	for {
		name, rest, ok := leadingLabel(s)
		if !ok {
			break
		}
		if err := t.defineLabel(name); err != nil {
			return err
		}
		s = rest
	}
	s = strings.TrimSpace(s)
	if s == "" {
		return nil
	}
	word, rest := splitWord(s)
	if word[0] == '.' {
		return t.directive(strings.ToLower(word), strings.TrimSpace(rest))
	}
	if i := strings.IndexByte(s, '='); i > 0 && isSymbolName(strings.TrimSpace(s[:i])) {
		return t.assign(strings.TrimSpace(s[:i]), s[i+1:])
	}
	mn := strings.ToLower(word)
	for {
		pb, ok := prefixBytes[mn]
		if !ok {
			break
		}
		if t.pfxEnd != len(t.a.Code) || t.pfxAsm != t.a {
			t.pfxStart = len(t.a.Code)
		}
		t.a.b(pb)
		t.pfxEnd, t.pfxAsm = len(t.a.Code), t.a
		if strings.TrimSpace(rest) == "" {
			return nil
		}
		word, rest = splitWord(strings.TrimSpace(rest))
		mn = strings.ToLower(word)
	}
	ops, err := t.operands(rest)
	if err != nil {
		return err
	}
	if t.pfxAsm == t.a && t.pfxEnd == len(t.a.Code) && t.pfxEnd > t.pfxStart {
		// explicit prefixes go after the operand-size prefix (GNU as order)
		start, end := t.pfxStart, t.pfxEnd
		t.pfxAsm = nil
		if err := t.instr(mn, ops); err != nil {
			return err
		}
		code := t.a.Code
		if end < len(code) && code[end] == 0x66 {
			copy(code[start+1:end+1], code[start:end])
			code[start] = 0x66
		}
		return nil
	}
	return t.instr(mn, ops)
}

func splitWord(s string) (string, string) {
	i := strings.IndexAny(s, " \t")
	if i < 0 {
		return s, ""
	}
	return s[:i], s[i+1:]
}

// splitTop splits by commas outside parentheses and strings
func splitTop(s string) []string {
	var out []string
	depth, start := 0, 0
	inStr := false
	for i := 0; i < len(s); i++ {
		c := s[i]
		switch {
		case inStr:
			if c == '\\' {
				i++
			} else if c == '"' {
				inStr = false
			}
		case c == '"':
			inStr = true
		case c == '(':
			depth++
		case c == ')':
			depth--
		case c == ',' && depth == 0:
			out = append(out, strings.TrimSpace(s[start:i]))
			start = i + 1
		}
	}
	if last := strings.TrimSpace(s[start:]); last != "" || len(out) > 0 {
		out = append(out, last)
	}
	return out
}

// ---- operands ----

type opKind int

const (
	okReg opKind = iota
	okMem
	okImm
	okST
)

type aop struct {
	kind opKind
	reg  Reg
	size int // register size (16 for XMM)
	mem  Mem
	val  xval // immediate value, or memory displacement
	ind  bool // '*' prefix of indirect jumps
	bare bool // memory without base/index: also a jump target
	st   int
}

func (o aop) isGP() bool  { return o.kind == okReg && !o.reg.IsXMM() }
func (o aop) isXMM() bool { return o.kind == okReg && o.reg.IsXMM() && o.size != 32 }
func (o aop) isRM() bool  { return o.isGP() || o.kind == okMem }

// xval is the value of an expression: v + sym (or + a label)
type xval struct {
	v     int64
	sym   string
	name  string // the symbol name of a named label
	lbl   *labelRef
	minus *labelRef // label difference: lbl - minus
	rel   string    // @PLT, @GOTPCREL...
}

func (x xval) isConst() bool { return x.sym == "" && x.lbl == nil }

// asmReg is a register name of assembly text
type asmReg struct {
	r    Reg
	size int
}

// gpRegs maps register names (without %) to registers and sizes
var gpRegs = map[string]asmReg{}

func init() {
	for i := 0; i < 16; i++ {
		gpRegs[regNames[3][i]] = asmReg{Reg(i), 8}
		gpRegs[regNames[2][i]] = asmReg{Reg(i), 4}
		gpRegs[regNames[1][i]] = asmReg{Reg(i), 2}
		gpRegs[regNames[0][i]] = asmReg{Reg(i), 1}
		gpRegs[fmt.Sprintf("xmm%d", i)] = asmReg{XMM0 + Reg(i), 16}
	}
	for i := 8; i < 16; i++ {
		gpRegs[fmt.Sprintf("r%dl", i)] = asmReg{Reg(i), 1}
	}
	for i, n := range []string{"ah", "ch", "dh", "bh"} {
		gpRegs[n] = asmReg{AH + Reg(i), 1}
	}
}

func (t *textAsm) operands(s string) ([]aop, error) {
	s = strings.TrimSpace(s)
	if s == "" {
		return nil, nil
	}
	var ops []aop
	for _, part := range splitTop(s) {
		o, err := t.operand(part)
		if err != nil {
			return nil, err
		}
		ops = append(ops, o)
	}
	return ops, nil
}

func (t *textAsm) operand(s string) (aop, error) {
	var o aop
	s = strings.TrimSpace(s)
	if s == "" {
		return o, fmt.Errorf("missing operand")
	}
	if s[0] == '*' {
		o.ind = true
		s = strings.TrimSpace(s[1:])
	}
	if s[0] == '$' {
		v, err := t.expr(s[1:])
		if err != nil {
			return o, err
		}
		o.kind, o.val = okImm, v
		return o, nil
	}
	seg := false
	if s[0] == '%' {
		name, rest := regToken(s[1:])
		rest = strings.TrimSpace(rest)
		if (name == "fs" || name == "gs") && strings.HasPrefix(rest, ":") {
			if name == "gs" {
				return o, fmt.Errorf("%%gs is not supported")
			}
			seg = true
			s = strings.TrimSpace(rest[1:])
		} else {
			if name == "st" {
				o.kind = okST
				if strings.HasPrefix(rest, "(") {
					end := strings.IndexByte(rest, ')')
					if end < 0 {
						return o, fmt.Errorf("bad register %%%s", s)
					}
					n, err := strconv.Atoi(strings.TrimSpace(rest[1:end]))
					if err != nil || n < 0 || n > 7 {
						return o, fmt.Errorf("bad register %%%s", s)
					}
					o.st = n
					rest = rest[end+1:]
				}
				if strings.TrimSpace(rest) != "" {
					return o, fmt.Errorf("junk after register")
				}
				return o, nil
			}
			r, ok := gpRegs[name]
			if !ok {
				return o, fmt.Errorf("bad register name '%%%s'", name)
			}
			if rest != "" {
				return o, fmt.Errorf("junk '%s' after register", rest)
			}
			o.kind, o.reg, o.size = okReg, r.r, r.size
			return o, nil
		}
	}
	// memory: disp(base,index,scale) or disp
	o.kind = okMem
	o.mem = Mem{Base: NoReg, Index: NoReg, FS: seg}
	disp := s
	if strings.HasSuffix(s, ")") {
		depth := 0
		open := -1
		for i := len(s) - 1; i >= 0; i-- {
			if s[i] == ')' {
				depth++
			} else if s[i] == '(' {
				depth--
				if depth == 0 {
					open = i
					break
				}
			}
		}
		inner := ""
		if open >= 0 {
			inner = strings.TrimSpace(s[open+1 : len(s)-1])
		}
		if open >= 0 && (inner == "" || inner[0] == '%' || inner[0] == ',') {
			disp = s[:open]
			parts := strings.Split(inner, ",")
			if len(parts) > 3 {
				return o, fmt.Errorf("bad memory operand '%s'", s)
			}
			if b := strings.TrimSpace(parts[0]); b != "" {
				r, err := addrReg(b)
				if err != nil {
					return o, err
				}
				o.mem.Base = r
			}
			if len(parts) > 1 {
				if x := strings.TrimSpace(parts[1]); x != "" {
					r, err := addrReg(x)
					if err != nil {
						return o, err
					}
					if r == RIP || r == RSP {
						return o, fmt.Errorf("bad index register")
					}
					o.mem.Index = r
				}
				o.mem.Scale = 1
				if len(parts) > 2 {
					sc, err := t.expr(parts[2])
					if err != nil || !sc.isConst() || (sc.v != 1 && sc.v != 2 && sc.v != 4 && sc.v != 8) {
						return o, fmt.Errorf("bad scale")
					}
					o.mem.Scale = int(sc.v)
				}
			}
		}
	}
	o.bare = o.mem.Base == NoReg && o.mem.Index == NoReg && !seg
	if strings.TrimSpace(disp) != "" {
		v, err := t.expr(disp)
		if err != nil {
			return o, err
		}
		o.val = v
		if !fitsI32(v.v) {
			if !o.bare || !v.isConst() {
				return o, fmt.Errorf("displacement out of range")
			}
		}
		o.mem.Disp = int32(v.v)
	}
	return o, nil
}

func regToken(s string) (string, string) {
	i := 0
	for i < len(s) && (s[i] >= 'a' && s[i] <= 'z' || s[i] >= 'A' && s[i] <= 'Z' || s[i] >= '0' && s[i] <= '9') {
		i++
	}
	return strings.ToLower(s[:i]), s[i:]
}

func addrReg(s string) (Reg, error) {
	if !strings.HasPrefix(s, "%") {
		return NoReg, fmt.Errorf("bad address register '%s'", s)
	}
	name := strings.ToLower(strings.TrimSpace(s[1:]))
	if name == "rip" {
		return RIP, nil
	}
	r, ok := gpRegs[name]
	if !ok || r.size != 8 {
		return NoReg, fmt.Errorf("bad address register '%s'", s)
	}
	return r.r, nil
}

// ---- expressions ----

type exprParser struct {
	t   *textAsm
	s   string
	pos int
}

func (t *textAsm) expr(s string) (xval, error) {
	p := &exprParser{t: t, s: s}
	v, err := p.binary(0)
	if err != nil {
		return v, err
	}
	p.skip()
	if p.pos != len(p.s) {
		return v, fmt.Errorf("junk '%s' in expression", p.s[p.pos:])
	}
	return v, nil
}

func (p *exprParser) skip() {
	for p.pos < len(p.s) && (p.s[p.pos] == ' ' || p.s[p.pos] == '\t') {
		p.pos++
	}
}

var binPrec = map[string]int{"|": 1, "^": 2, "&": 3, "+": 4, "-": 4, "*": 5, "/": 5, "%": 5, "<<": 5, ">>": 5}

func (p *exprParser) peekOp() string {
	p.skip()
	if p.pos >= len(p.s) {
		return ""
	}
	for _, op := range []string{"<<", ">>"} {
		if strings.HasPrefix(p.s[p.pos:], op) {
			return op
		}
	}
	c := p.s[p.pos : p.pos+1]
	if _, ok := binPrec[c]; ok {
		return c
	}
	return ""
}

func (p *exprParser) binary(minPrec int) (xval, error) {
	lhs, err := p.unary()
	if err != nil {
		return lhs, err
	}
	for {
		op := p.peekOp()
		prec, ok := binPrec[op]
		if !ok || prec <= minPrec {
			return lhs, nil
		}
		p.pos += len(op)
		rhs, err := p.binary(prec)
		if err != nil {
			return lhs, err
		}
		switch op {
		case "+":
			if !lhs.isConst() && !rhs.isConst() {
				return lhs, fmt.Errorf("cannot add two symbols")
			}
			if lhs.isConst() {
				rhs.v += lhs.v
				lhs = rhs
			} else {
				lhs.v += rhs.v
			}
			continue
		case "-":
			if !rhs.isConst() {
				if rhs.lbl == nil || rhs.minus != nil || lhs.lbl == nil || lhs.minus != nil {
					return lhs, fmt.Errorf("symbol differences are only supported between labels")
				}
				// fold when both labels are already placed in the same section
				if lhs.lbl.a == rhs.lbl.a && lhs.lbl.a.labels[lhs.lbl.l] >= 0 && rhs.lbl.a.labels[rhs.lbl.l] >= 0 {
					lhs = xval{v: int64(lhs.lbl.a.labels[lhs.lbl.l]-rhs.lbl.a.labels[rhs.lbl.l]) + lhs.v - rhs.v}
					continue
				}
				lhs.minus = rhs.lbl
				lhs.v -= rhs.v
				continue
			}
			lhs.v -= rhs.v
			continue
		}
		if !lhs.isConst() || !rhs.isConst() {
			return lhs, fmt.Errorf("invalid operation on a symbol")
		}
		a, b := lhs.v, rhs.v
		switch op {
		case "*":
			a *= b
		case "/", "%":
			if b == 0 {
				return lhs, fmt.Errorf("division by zero")
			}
			if op == "/" {
				a /= b
			} else {
				a %= b
			}
		case "<<":
			a <<= uint(b)
		case ">>":
			a >>= uint(b)
		case "&":
			a &= b
		case "|":
			a |= b
		case "^":
			a ^= b
		}
		lhs.v = a
	}
}

func (p *exprParser) unary() (xval, error) {
	p.skip()
	if p.pos >= len(p.s) {
		return xval{}, fmt.Errorf("missing expression")
	}
	switch c := p.s[p.pos]; c {
	case '-', '~', '+', '!':
		p.pos++
		v, err := p.unary()
		if err != nil {
			return v, err
		}
		if c == '+' {
			return v, nil
		}
		if !v.isConst() {
			return v, fmt.Errorf("invalid operation on a symbol")
		}
		switch c {
		case '-':
			v.v = -v.v
		case '~':
			v.v = ^v.v
		case '!':
			if v.v == 0 {
				v.v = 1
			} else {
				v.v = 0
			}
		}
		return v, nil
	case '(':
		p.pos++
		v, err := p.binary(0)
		if err != nil {
			return v, err
		}
		p.skip()
		if p.pos >= len(p.s) || p.s[p.pos] != ')' {
			return v, fmt.Errorf("missing ')'")
		}
		p.pos++
		return v, nil
	case '\'':
		if p.pos+1 >= len(p.s) {
			return xval{}, fmt.Errorf("bad character constant")
		}
		p.pos++
		ch := int64(p.s[p.pos])
		if ch == '\\' && p.pos+1 < len(p.s) {
			p.pos++
			ch = int64(unescape(p.s[p.pos]))
		}
		p.pos++
		if p.pos < len(p.s) && p.s[p.pos] == '\'' {
			p.pos++
		}
		return xval{v: ch}, nil
	}
	c := p.s[p.pos]
	if c >= '0' && c <= '9' {
		start := p.pos
		for p.pos < len(p.s) && (isIdentChar(p.s[p.pos], false)) {
			p.pos++
		}
		tok := p.s[start:p.pos]
		if len(tok) > 1 && (tok[len(tok)-1] == 'b' || tok[len(tok)-1] == 'f') && isDigits(tok[:len(tok)-1]) {
			return p.t.numericRef(tok[:len(tok)-1], tok[len(tok)-1] == 'f')
		}
		v, err := parseAsmInt(tok)
		if err != nil {
			return xval{}, err
		}
		return xval{v: v}, nil
	}
	if isIdentChar(c, true) {
		start := p.pos
		for p.pos < len(p.s) && isIdentChar(p.s[p.pos], false) {
			p.pos++
		}
		name := p.s[start:p.pos]
		rel := ""
		if p.pos < len(p.s) && p.s[p.pos] == '@' {
			p.pos++
			rs := p.pos
			for p.pos < len(p.s) && isIdentChar(p.s[p.pos], false) {
				p.pos++
			}
			rel = strings.ToUpper(p.s[rs:p.pos])
		}
		v := p.t.symbolRef(name)
		v.rel = rel
		return v, nil
	}
	return xval{}, fmt.Errorf("bad expression '%s'", p.s[p.pos:])
}

func unescape(c byte) byte {
	switch c {
	case 'n':
		return '\n'
	case 't':
		return '\t'
	case 'r':
		return '\r'
	case '0':
		return 0
	case 'b':
		return '\b'
	case 'f':
		return '\f'
	case 'v':
		return '\v'
	}
	return c
}

func parseAsmInt(s string) (int64, error) {
	ls := strings.ToLower(s)
	var u uint64
	var err error
	switch {
	case strings.HasPrefix(ls, "0x"):
		u, err = strconv.ParseUint(ls[2:], 16, 64)
	case strings.HasPrefix(ls, "0b"):
		u, err = strconv.ParseUint(ls[2:], 2, 64)
	case len(ls) > 1 && ls[0] == '0':
		u, err = strconv.ParseUint(ls[1:], 8, 64)
	default:
		u, err = strconv.ParseUint(ls, 10, 64)
	}
	if err != nil {
		return 0, fmt.Errorf("bad number '%s'", s)
	}
	return int64(u), nil
}

func (t *textAsm) numericRef(n string, fwd bool) (xval, error) {
	if !fwd {
		r, ok := t.numBack[n]
		if !ok {
			return xval{}, fmt.Errorf("undefined local label '%sb'", n)
		}
		return xval{lbl: &r}, nil
	}
	r, ok := t.numFwd[n]
	if !ok {
		r = labelRef{t.a, t.a.NewLabel()}
		t.numFwd[n] = r
	}
	return xval{lbl: &r}, nil
}

func (t *textAsm) symbolRef(name string) xval {
	if name == "." {
		l := t.a.NewLabel()
		t.a.labels[l] = len(t.a.Code)
		return xval{lbl: &labelRef{t.a, l}}
	}
	if v, ok := t.env.Consts[name]; ok {
		return xval{v: v}
	}
	if t.globals[name] {
		// global symbols are referenced through relocations (they may be
		// preempted), like GNU as does
		return xval{sym: name}
	}
	if tl := t.env.Labels[name]; tl != nil {
		return xval{lbl: &labelRef{tl.A, tl.L}, v: tl.Off, name: name}
	}
	if t.defNames[name] {
		a := t.a
		if sec := t.labelSec[name]; sec != "" && sec != t.secName {
			if sa, err := t.env.Section(sec, t.secFlags[sec]); err == nil {
				a = sa
			}
		}
		tl := &TextLabel{A: a, L: a.NewLabel()}
		t.env.Labels[name] = tl
		return xval{lbl: &labelRef{tl.A, tl.L}}
	}
	return xval{sym: name}
}

// ---- encoding helpers ----

// rmOperand converts an operand for enc; labels are resolved through
// fixups (RIP-relative) or label symbols
func (t *textAsm) rmOperand(o aop) (Operand, *labelRef) {
	if o.kind == okReg {
		return R{o.reg, o.size}, nil
	}
	m := o.mem
	switch {
	case o.val.lbl != nil && m.Base == RIP && o.val.lbl.a == t.a:
		return m, o.val.lbl
	case o.val.lbl != nil:
		m.Sym = o.val.lbl.a.LabelSymbol(o.val.lbl.l)
	case o.val.sym != "":
		m.Sym = o.val.sym
	}
	if m.Sym != "" {
		switch o.val.rel {
		case "GOTPCREL":
			m.Rel = RelGOT
		case "TPOFF":
			m.Rel = RelTPOFF
		case "GOTTPOFF":
			m.Rel = RelGOTTPOF
		}
	}
	return m, nil
}

// encRM emits an instruction with a ModRM operand followed by trail bytes
func (t *textAsm) encRM(pfx []byte, w bool, op []byte, reg int, o aop, trail int) {
	rm, lbl := t.rmOperand(o)
	force := reg&rexByteFlag != 0
	reg &^= rexByteFlag
	if r, ok := rm.(R); ok && r.Size == 1 && r.Reg >= RSP && r.Reg <= RDI {
		force = true
	}
	start := len(t.a.Code)
	nrel := len(t.a.Relocs)
	t.a.enc(pfx, w, op, reg, rm, trail, force)
	if m, ok := rm.(Mem); ok && m.Rel == RelGOT && len(t.a.Relocs) == nrel+1 && len(op) == 1 {
		// relaxable GOT loads, like GNU as: mov, call, jmp, test and binops
		switch op[0] {
		case 0x8B, 0xFF, 0x85, 0x03, 0x0B, 0x13, 0x1B, 0x23, 0x2B, 0x33, 0x3B:
			r := &t.a.Relocs[nrel]
			r.Type = elf.R_X86_64_GOTPCRELX
			if i := start + len(pfx); i < len(t.a.Code) && t.a.Code[i]&0xF0 == 0x40 {
				r.Type = elf.R_X86_64_REX_GOTPCRELX
			}
		}
	}
	if lbl != nil {
		t.a.fixups = append(t.a.fixups, fixup{at: len(t.a.Code) - 4, label: lbl.l, trail: trail, addend: int64(o.mem.Disp)})
	}
}

// emitImm writes an immediate of n bytes (with a relocation for symbols)
func (t *textAsm) emitImm(v xval, n int, signed bool) error {
	if v.isConst() {
		t.a.imm(v.v, n)
		return nil
	}
	if v.minus != nil {
		switch {
		case v.lbl.a == v.minus.a:
			t.a.fixups = append(t.a.fixups, fixup{at: len(t.a.Code), size: n, diff: true, la: v.lbl.a, label: v.lbl.l, minus: v.minus.l, addend: v.v})
			t.a.imm(0, n)
			return nil
		case v.minus.a == t.a && (n == 4 || n == 8):
			// sym - . : PC-relative
			typ := uint32(elf.R_X86_64_PC32)
			if n == 8 {
				typ = elf.R_X86_64_PC64
			}
			pos := t.a.labels[v.minus.l]
			sym := v.lbl.a.LabelSymbol(v.lbl.l)
			t.a.Relocs = append(t.a.Relocs, Reloc{Offset: int64(len(t.a.Code)), Type: typ, Sym: sym, Addend: v.v + int64(len(t.a.Code)-pos)})
			t.a.imm(0, n)
			return nil
		}
		return fmt.Errorf("can't resolve a difference of labels in different sections")
	}
	sym := v.sym
	if v.lbl != nil {
		sym = v.lbl.a.LabelSymbol(v.lbl.l)
	}
	var typ uint32
	switch n {
	case 1:
		typ = elf.R_X86_64_8
	case 2:
		typ = elf.R_X86_64_16
	case 4:
		typ = elf.R_X86_64_32
		if signed {
			typ = elf.R_X86_64_32S
		}
	case 8:
		typ = elf.R_X86_64_64
	}
	switch v.rel {
	case "":
	case "TPOFF":
		if n != 4 {
			return fmt.Errorf("bad @tpoff immediate")
		}
		typ = elf.R_X86_64_TPOFF32
	default:
		return fmt.Errorf("relocation @%s is not supported in an immediate", v.rel)
	}
	t.a.Relocs = append(t.a.Relocs, Reloc{Offset: int64(len(t.a.Code)), Type: typ, Sym: sym, Addend: v.v})
	t.a.imm(0, n)
	return nil
}

// rexByteFlag marks a ModRM.reg value holding %spl..%dil (needs a REX prefix)
const rexByteFlag = 0x100

// regField gives the ModRM.reg value for a register operand (AH..BH -> 4..7)
func regField(o aop) int {
	if o.reg >= AH && o.reg <= BH {
		return int(o.reg - AH + 4)
	}
	if o.size == 1 && o.reg >= RSP && o.reg <= RDI {
		return o.reg.num() | rexByteFlag
	}
	return o.reg.num()
}

// highConflict reports AH..BH used together with a REX prefix
func highConflict(w bool, ops ...aop) error {
	high, rex := false, w
	for _, o := range ops {
		if o.kind == okReg {
			if o.reg >= AH && o.reg <= BH {
				high = true
			} else if o.reg.num()&8 != 0 || o.size == 1 && o.reg >= RSP && o.reg <= RDI {
				rex = true
			}
		}
		if o.kind == okMem && (o.mem.Base >= R8 && o.mem.Base <= R15 || o.mem.Index >= R8 && o.mem.Index <= R15) {
			rex = true
		}
	}
	if high && rex {
		return fmt.Errorf("can't encode a high byte register in an instruction requiring REX prefix")
	}
	return nil
}

// ---- instruction table ----

type instFn func(t *textAsm, mn string, size int, ops []aop) error

type instDef struct {
	fn     instFn
	suffix bool // b/w/l/q suffixes select the operand size
	arg    int
	op     []byte
	pfx    []byte
}

var insts map[string]*instDef

func (t *textAsm) instr(mn string, ops []aop) error {
	d := insts[mn]
	size := 0
	if d == nil && len(mn) > 1 {
		if base := insts[mn[:len(mn)-1]]; base != nil && base.suffix {
			switch mn[len(mn)-1] {
			case 'b':
				d, size = base, 1
			case 'w':
				d, size = base, 2
			case 'l':
				d, size = base, 4
			case 'q':
				d, size = base, 8
			}
		}
	}
	if d == nil {
		return fmt.Errorf("unknown instruction '%s'", mn)
	}
	t.cur = d
	return d.fn(t, mn, size, ops)
}

// opSize determines the operand size from the suffix or the registers
func opSize(size int, ops ...aop) (int, error) {
	if size != 0 {
		return size, nil
	}
	for _, o := range ops {
		if o.isGP() {
			return o.size, nil
		}
	}
	return 0, fmt.Errorf("no instruction mnemonic suffix given and no register operands; can't size the instruction")
}

func checkSize(size int, ops ...aop) error {
	for _, o := range ops {
		if o.isGP() && o.size != size {
			return fmt.Errorf("operand size mismatch")
		}
	}
	return nil
}

func nops(ops []aop, n ...int) error {
	for _, k := range n {
		if len(ops) == k {
			return nil
		}
	}
	return fmt.Errorf("wrong number of operands")
}

func errOps() error { return fmt.Errorf("invalid operands") }

// ALU: add or adc sbb and sub xor cmp
func instAlu(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	size, err := opSize(size, dst, src)
	if err != nil {
		return err
	}
	if err := checkSize(size, src, dst); err != nil {
		return err
	}
	if err := highConflict(size == 8, src, dst); err != nil {
		return err
	}
	op := t.cur.arg
	base := byte(op * 8)
	p := opsize16(size)
	w := size == 8
	switch {
	case src.kind == okImm && dst.isRM():
		v := src.val
		acc := dst.kind == okReg && dst.reg == RAX
		switch {
		case size == 1 && acc:
			t.a.b(base + 4)
			return t.emitImm(v, 1, true)
		case size == 1:
			t.encRM(p, w, []byte{0x80}, op, dst, 1)
			return t.emitImm(v, 1, true)
		case v.isConst() && fitsI8(v.v):
			t.encRM(p, w, []byte{0x83}, op, dst, 1)
			return t.emitImm(v, 1, true)
		default:
			n := 4
			if size == 2 {
				n = 2
			}
			if acc {
				t.a.b(p...)
				if w {
					t.a.b(0x48)
				}
				t.a.b(base + 5)
			} else {
				t.encRM(p, w, []byte{0x81}, op, dst, n)
			}
			return t.emitImm(v, n, true)
		}
	case src.isGP() && dst.isRM():
		opc := base + 1
		if size == 1 {
			opc = base
		}
		t.encRM(p, w, []byte{opc}, regField(src), dst, 0)
	case src.kind == okMem && dst.isGP():
		opc := base + 3
		if size == 1 {
			opc = base + 2
		}
		t.encRM(p, w, []byte{opc}, regField(dst), src, 0)
	default:
		return errOps()
	}
	return nil
}

func instMov(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if src.isXMM() || dst.isXMM() {
		if size == 4 {
			return instMovdq(t, "movd", 0, ops)
		}
		return instMovdq(t, "movq", 0, ops)
	}
	size, err := opSize(size, dst, src)
	if err != nil {
		return err
	}
	if err := checkSize(size, src, dst); err != nil {
		return err
	}
	if err := highConflict(size == 8, src, dst); err != nil {
		return err
	}
	p := opsize16(size)
	w := size == 8
	movabs := strings.HasPrefix(mn, "movabs")
	switch {
	case src.kind == okImm && dst.isGP():
		r := dst.reg
		v := src.val
		if size == 8 && !movabs && (!v.isConst() || fitsI32(v.v)) {
			t.encRM(nil, true, []byte{0xC7}, 0, dst, 4)
			return t.emitImm(v, 4, true)
		}
		if movabs && size != 8 {
			return errOps()
		}
		t.a.b(p...)
		rex := byte(0)
		if w {
			rex |= 8
		}
		if r.num()&8 != 0 && r < AH {
			rex |= 1
		}
		if rex != 0 || size == 1 && r >= RSP && r <= RDI {
			t.a.b(0x40 | rex)
		}
		num := byte(r.num() & 7)
		if r >= AH && r <= BH {
			num = byte(r - AH + 4)
		}
		if size == 1 {
			t.a.b(0xB0 + num)
		} else {
			t.a.b(0xB8 + num)
		}
		return t.emitImm(v, size, false)
	case src.kind == okImm && dst.kind == okMem:
		n := size
		if n == 8 {
			n = 4
		}
		opc := byte(0xC7)
		if size == 1 {
			opc = 0xC6
		}
		t.encRM(p, w, []byte{opc}, 0, dst, n)
		return t.emitImm(src.val, n, true)
	case src.isGP() && dst.isRM():
		opc := byte(0x89)
		if size == 1 {
			opc = 0x88
		}
		t.encRM(p, w, []byte{opc}, regField(src), dst, 0)
	case src.kind == okMem && dst.isGP():
		opc := byte(0x8B)
		if size == 1 {
			opc = 0x8A
		}
		t.encRM(p, w, []byte{opc}, regField(dst), src, 0)
	default:
		return errOps()
	}
	return nil
}

// movzx/movsx: arg = 0 zero, 1 sign; sizes from the mnemonic or operands
func instMovx(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !dst.isGP() || !src.isRM() {
		return errOps()
	}
	sz := sfxSize(mn)
	ssz, dsz := sz[0], sz[1]
	if ssz == 0 {
		if !src.isGP() {
			return fmt.Errorf("ambiguous source size")
		}
		ssz = src.size
	}
	if dsz == 0 {
		dsz = dst.size
	}
	if dst.size != dsz || src.isGP() && src.size != ssz {
		return fmt.Errorf("operand size mismatch")
	}
	if err := highConflict(dsz == 8, src, dst); err != nil {
		return err
	}
	sign := t.cur.arg == 1
	switch {
	case ssz == 4 && sign && dsz == 8:
		t.encRM(nil, true, []byte{0x63}, regField(dst), src, 0)
	case ssz == 1 || ssz == 2:
		opc := byte(0xB6)
		if sign {
			opc = 0xBE
		}
		if ssz == 2 {
			opc++
		}
		t.encRM(opsize16(dsz), dsz == 8, []byte{0x0F, opc}, regField(dst), src, 0)
	default:
		return errOps()
	}
	return nil
}

// sfxSize decodes movzbl-style suffixes: source and destination sizes
func sfxSize(mn string) [2]int {
	codes := map[byte]int{'b': 1, 'w': 2, 'l': 4, 'q': 8}
	for _, p := range []string{"movz", "movs"} {
		if strings.HasPrefix(mn, p) && len(mn) == 6 {
			return [2]int{codes[mn[4]], codes[mn[5]]}
		}
	}
	switch mn {
	case "movsxd", "movslq":
		return [2]int{4, 8}
	}
	if strings.HasPrefix(mn, "movzx") || strings.HasPrefix(mn, "movsx") {
		if len(mn) == 6 {
			return [2]int{codes[mn[5]], 0}
		}
	}
	return [2]int{0, 0}
}

func instLea(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	if ops[0].kind != okMem || !ops[1].isGP() || ops[1].size == 1 {
		return errOps()
	}
	size, _ = opSize(size, ops[1])
	t.encRM(opsize16(size), size == 8, []byte{0x8D}, regField(ops[1]), ops[0], 0)
	return nil
}

func instTest(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if src.kind == okMem && dst.isGP() {
		src, dst = dst, src
	}
	size, err := opSize(size, dst, src)
	if err != nil {
		return err
	}
	if err := checkSize(size, src, dst); err != nil {
		return err
	}
	p := opsize16(size)
	w := size == 8
	n := 4
	if size == 2 {
		n = 2
	} else if size == 1 {
		n = 1
	}
	switch {
	case src.kind == okImm && dst.isRM():
		if dst.kind == okReg && dst.reg == RAX {
			t.a.b(p...)
			if w {
				t.a.b(0x48)
			}
			if size == 1 {
				t.a.b(0xA8)
			} else {
				t.a.b(0xA9)
			}
			return t.emitImm(src.val, n, true)
		}
		opc := byte(0xF7)
		if size == 1 {
			opc = 0xF6
		}
		t.encRM(p, w, []byte{opc}, 0, dst, n)
		return t.emitImm(src.val, n, true)
	case src.isGP() && dst.isRM():
		opc := byte(0x85)
		if size == 1 {
			opc = 0x84
		}
		t.encRM(p, w, []byte{opc}, regField(src), dst, 0)
		return nil
	}
	return errOps()
}

// group 3 unary (not neg mul div idiv) and inc/dec
func instUnary(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if !ops[0].isRM() {
		return errOps()
	}
	size, err := opSize(size, ops[0])
	if err != nil {
		return err
	}
	opc := t.cur.op[0]
	if size != 1 {
		opc++
	}
	t.encRM(opsize16(size), size == 8, []byte{opc}, t.cur.arg, ops[0], 0)
	return nil
}

func instImul(t *textAsm, mn string, size int, ops []aop) error {
	switch len(ops) {
	case 1:
		t.cur = &instDef{arg: 5, op: []byte{0xF6}}
		return instUnary(t, mn, size, ops)
	case 2:
		if ops[0].kind == okImm {
			return instImul(t, mn, size, []aop{ops[0], ops[1], ops[1]})
		}
		if !ops[0].isRM() || !ops[1].isGP() {
			return errOps()
		}
		size, _ := opSize(size, ops[1])
		if err := checkSize(size, ops...); err != nil {
			return err
		}
		t.encRM(opsize16(size), size == 8, []byte{0x0F, 0xAF}, regField(ops[1]), ops[0], 0)
		return nil
	case 3:
		if ops[0].kind != okImm || !ops[1].isRM() || !ops[2].isGP() {
			return errOps()
		}
		size, _ := opSize(size, ops[2])
		if err := checkSize(size, ops[1:]...); err != nil {
			return err
		}
		v := ops[0].val
		if v.isConst() && fitsI8(v.v) {
			t.encRM(opsize16(size), size == 8, []byte{0x6B}, regField(ops[2]), ops[1], 1)
			return t.emitImm(v, 1, true)
		}
		n := 4
		if size == 2 {
			n = 2
		}
		t.encRM(opsize16(size), size == 8, []byte{0x69}, regField(ops[2]), ops[1], n)
		return t.emitImm(v, n, true)
	}
	return errOps()
}

// shifts and rotates: arg is the group 2 extension
func instShift(t *textAsm, mn string, size int, ops []aop) error {
	var cnt *aop
	var dst aop
	switch len(ops) {
	case 1:
		dst = ops[0]
	case 2:
		cnt, dst = &ops[0], ops[1]
	default:
		return errOps()
	}
	if !dst.isRM() {
		return errOps()
	}
	size, err := opSize(size, dst)
	if err != nil {
		return err
	}
	p := opsize16(size)
	w := size == 8
	b := byte(0)
	if size == 1 {
		b = 1
	}
	ext := t.cur.arg
	switch {
	case cnt == nil || cnt.kind == okImm && cnt.val.isConst() && cnt.val.v == 1:
		t.encRM(p, w, []byte{0xD1 - b}, ext, dst, 0)
	case cnt.kind == okImm:
		t.encRM(p, w, []byte{0xC1 - b}, ext, dst, 1)
		return t.emitImm(cnt.val, 1, false)
	case cnt.kind == okReg && cnt.reg == RCX && cnt.size == 1:
		t.encRM(p, w, []byte{0xD3 - b}, ext, dst, 0)
	default:
		return errOps()
	}
	return nil
}

// shld/shrd: op is the imm form opcode (cl form is op+1)
func instShxd(t *textAsm, mn string, size int, ops []aop) error {
	if len(ops) == 2 {
		ops = []aop{{kind: okReg, reg: RCX, size: 1}, ops[0], ops[1]}
	}
	if err := nops(ops, 3); err != nil {
		return err
	}
	cnt, src, dst := ops[0], ops[1], ops[2]
	if !src.isGP() || !dst.isRM() {
		return errOps()
	}
	size, _ = opSize(size, src)
	opc := t.cur.op[0]
	switch {
	case cnt.kind == okImm:
		t.encRM(opsize16(size), size == 8, []byte{0x0F, opc}, src.reg.num(), dst, 1)
		return t.emitImm(cnt.val, 1, false)
	case cnt.kind == okReg && cnt.reg == RCX && cnt.size == 1:
		t.encRM(opsize16(size), size == 8, []byte{0x0F, opc + 1}, src.reg.num(), dst, 0)
		return nil
	}
	return errOps()
}

// bt/bts/btr/btc: arg = group 8 extension (4..7)
func instBt(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !dst.isRM() {
		return errOps()
	}
	size, err := opSize(size, dst, src)
	if err != nil {
		return err
	}
	ext := t.cur.arg
	if src.kind == okImm {
		t.encRM(opsize16(size), size == 8, []byte{0x0F, 0xBA}, ext, dst, 1)
		return t.emitImm(src.val, 1, false)
	}
	if !src.isGP() {
		return errOps()
	}
	opc := []byte{0xA3, 0xAB, 0xB3, 0xBB}[ext-4]
	t.encRM(opsize16(size), size == 8, []byte{0x0F, opc}, src.reg.num(), dst, 0)
	return nil
}

// "op r/m, reg" with prefix bytes and opcode from the table (bsf, popcnt, crc32...)
func instRegRM(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !src.isRM() || !dst.isGP() {
		return errOps()
	}
	size, _ = opSize(size, dst)
	if err := checkSize(size, src, dst); err != nil {
		return err
	}
	pfx := append(opsize16(size), t.cur.pfx...)
	t.encRM(pfx, size == 8, t.cur.op, dst.reg.num(), src, 0)
	return nil
}

func instCrc32(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !src.isRM() || !dst.isGP() || dst.size < 4 {
		return errOps()
	}
	ssz, err := opSize(size, src)
	if err != nil {
		return err
	}
	opc := byte(0xF1)
	if ssz == 1 {
		opc = 0xF0
	}
	pfx := append(opsize16(ssz), 0xF2)
	t.encRM(pfx, dst.size == 8, []byte{0x0F, 0x38, opc}, dst.reg.num(), src, 0)
	return nil
}

func instMovbe(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	switch {
	case src.kind == okMem && dst.isGP():
		t.encRM(opsize16(dst.size), dst.size == 8, []byte{0x0F, 0x38, 0xF0}, dst.reg.num(), src, 0)
	case src.isGP() && dst.kind == okMem:
		t.encRM(opsize16(src.size), src.size == 8, []byte{0x0F, 0x38, 0xF1}, src.reg.num(), dst, 0)
	default:
		return errOps()
	}
	return nil
}

func instBswap(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	r := ops[0]
	if !r.isGP() || r.size < 4 {
		return errOps()
	}
	rex := byte(0)
	if r.size == 8 {
		rex |= 8
	}
	if r.reg.num()&8 != 0 {
		rex |= 1
	}
	if rex != 0 {
		t.a.b(0x40 | rex)
	}
	t.a.b(0x0F, 0xC8+byte(r.reg.num()&7))
	return nil
}

func instXchg(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	a, b := ops[0], ops[1]
	if a.kind == okMem {
		a, b = b, a
	}
	if !a.isGP() || !b.isRM() {
		return errOps()
	}
	size, _ = opSize(size, a)
	if err := checkSize(size, a, b); err != nil {
		return err
	}
	if size > 1 && b.isGP() && (a.reg == RAX || b.reg == RAX) && !(size == 4 && a.reg == RAX && b.reg == RAX) {
		other := b.reg
		if b.reg == RAX {
			other = a.reg
		}
		t.a.b(opsize16(size)...)
		rex := byte(0)
		if size == 8 {
			rex |= 8
		}
		if other.num()&8 != 0 {
			rex |= 1
		}
		if rex != 0 {
			t.a.b(0x40 | rex)
		}
		t.a.b(0x90 + byte(other.num()&7))
		return nil
	}
	opc := byte(0x87)
	if size == 1 {
		opc = 0x86
	}
	t.encRM(opsize16(size), size == 8, []byte{opc}, regField(a), b, 0)
	return nil
}

// xadd/cmpxchg: "op reg, r/m"; op is the byte form opcode
func instXadd(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !src.isGP() || !dst.isRM() {
		return errOps()
	}
	size, _ = opSize(size, src)
	if err := checkSize(size, src, dst); err != nil {
		return err
	}
	opc := t.cur.op[0]
	if size != 1 {
		opc++
	}
	t.encRM(opsize16(size), size == 8, []byte{0x0F, opc}, regField(src), dst, 0)
	return nil
}

// instructions with one memory operand and a fixed encoding: op /arg
func instMemOnly(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if ops[0].kind != okMem {
		return errOps()
	}
	w := false
	if t.cur.pfx != nil && t.cur.pfx[0] == 0x48 {
		w = true
		t.encRM(t.cur.pfx[1:], w, t.cur.op, t.cur.arg, ops[0], 0)
		return nil
	}
	t.encRM(t.cur.pfx, w, t.cur.op, t.cur.arg, ops[0], 0)
	return nil
}

// rdrand/rdseed: op /arg with a register
func instRegOnly(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if !ops[0].isGP() || ops[0].size == 1 {
		return errOps()
	}
	t.encRM(opsize16(ops[0].size), ops[0].size == 8, t.cur.op, t.cur.arg, ops[0], 0)
	return nil
}

func instSetcc(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if !ops[0].isRM() || ops[0].isGP() && ops[0].size != 1 {
		return errOps()
	}
	t.encRM(nil, false, []byte{0x0F, 0x90 + byte(t.cur.arg)}, 0, ops[0], 0)
	return nil
}

func instCmov(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	if !ops[0].isRM() || !ops[1].isGP() || ops[1].size == 1 {
		return errOps()
	}
	size, _ = opSize(size, ops[1])
	t.encRM(opsize16(size), size == 8, []byte{0x0F, 0x40 + byte(t.cur.arg)}, ops[1].reg.num(), ops[0], 0)
	return nil
}

// branch emits a rel32 (or rel8) jump to a label or symbol
func (t *textAsm) branch(op []byte, target aop, short bool) error {
	return t.branchRelax(op, 0, target, short)
}

// branchRelax emits a jump; shortOp (0: none) is the rel8 opcode used
// when the relaxation chose this jump
func (t *textAsm) branchRelax(op []byte, shortOp byte, target aop, short bool) error {
	if target.kind != okMem || !target.bare || target.ind {
		return errOps()
	}
	v := target.val
	if shortOp != 0 && v.lbl != nil && v.lbl.a == t.a {
		ord := t.branchN
		t.branchN++
		if t.short[ord] {
			t.a.b(shortOp)
			t.a.fixups = append(t.a.fixups, fixup{at: len(t.a.Code), label: v.lbl.l, size: 1, addend: v.v})
			t.a.imm(0, 1)
		} else {
			t.a.b(op...)
			t.a.fixups = append(t.a.fixups, fixup{at: len(t.a.Code), label: v.lbl.l, size: 4, addend: v.v})
			t.a.imm(0, 4)
		}
		if t.trial {
			t.branches = append(t.branches, branchInfo{ord, t.a, len(t.a.Code), v.lbl.l, v.v})
		}
		return nil
	}
	t.a.b(op...)
	switch {
	case v.lbl != nil && v.lbl.a == t.a:
		n := 4
		if short {
			n = 1
		}
		t.a.fixups = append(t.a.fixups, fixup{at: len(t.a.Code), label: v.lbl.l, size: n, addend: v.v})
		t.a.imm(0, n)
	case short:
		return fmt.Errorf("short jump to a symbol")
	case v.lbl != nil:
		t.a.Relocs = append(t.a.Relocs, Reloc{Offset: int64(len(t.a.Code)), Type: elf.R_X86_64_PC32, Sym: v.lbl.a.LabelSymbol(v.lbl.l), Addend: v.v - 4})
		t.a.imm(0, 4)
	case v.sym != "":
		typ := uint32(elf.R_X86_64_PLT32)
		if v.rel != "" && v.rel != "PLT" {
			return fmt.Errorf("bad relocation @%s in a jump", v.rel)
		}
		t.a.Relocs = append(t.a.Relocs, Reloc{Offset: int64(len(t.a.Code)), Type: typ, Sym: v.sym, Addend: v.v - 4})
		t.a.imm(0, 4)
	default:
		return fmt.Errorf("jumps to absolute addresses are not supported")
	}
	return nil
}

func instJcc(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	return t.branchRelax([]byte{0x0F, 0x80 + byte(t.cur.arg)}, 0x70+byte(t.cur.arg), ops[0], false)
}

func instJmpCall(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	o := ops[0]
	isCall := t.cur.arg == 2
	if o.ind || o.kind == okReg {
		if o.kind == okReg && (o.reg.IsXMM() || o.size != 8) {
			return errOps()
		}
		t.encRM(nil, false, []byte{0xFF}, t.cur.arg, o, 0)
		return nil
	}
	if isCall {
		return t.branch([]byte{0xE8}, o, false)
	}
	return t.branchRelax([]byte{0xE9}, 0xEB, o, false)
}

func instShortJump(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	return t.branch(t.cur.op, ops[0], true)
}

func instRet(t *textAsm, mn string, size int, ops []aop) error {
	if len(ops) == 0 {
		t.a.b(0xC3)
		return nil
	}
	if len(ops) != 1 || ops[0].kind != okImm {
		return errOps()
	}
	t.a.b(0xC2)
	return t.emitImm(ops[0].val, 2, false)
}

func instPush(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	o := ops[0]
	pop := t.cur.arg == 1
	if size == 0 {
		size = 8
		if o.isGP() {
			size = o.size
		}
	}
	if size != 8 && size != 2 {
		return fmt.Errorf("invalid operand size for push/pop")
	}
	p := opsize16(size)
	switch {
	case o.isGP():
		if o.size != size {
			return errOps()
		}
		t.a.b(p...)
		if o.reg.num()&8 != 0 {
			t.a.b(0x41)
		}
		if pop {
			t.a.b(0x58 + byte(o.reg.num()&7))
		} else {
			t.a.b(0x50 + byte(o.reg.num()&7))
		}
	case o.kind == okMem:
		if pop {
			t.encRM(p, false, []byte{0x8F}, 0, o, 0)
		} else {
			t.encRM(p, false, []byte{0xFF}, 6, o, 0)
		}
	case o.kind == okImm && !pop:
		t.a.b(p...)
		if o.val.isConst() && fitsI8(o.val.v) {
			t.a.b(0x6A)
			return t.emitImm(o.val, 1, true)
		}
		t.a.b(0x68)
		if size == 2 {
			return t.emitImm(o.val, 2, true)
		}
		return t.emitImm(o.val, 4, true)
	default:
		return errOps()
	}
	return nil
}

// fixed byte sequences without operands (op), optionally sized string ops (arg=1)
func instFixed(t *textAsm, mn string, size int, ops []aop) error {
	if t.cur.arg == 1 {
		// string instruction: operands are implicit
		if size == 0 {
			return fmt.Errorf("string instruction needs a size suffix")
		}
		t.a.b(opsize16(size)...)
		if size == 8 {
			t.a.b(0x48)
		}
		opc := t.cur.op[0]
		if size != 1 {
			opc++
		}
		t.a.b(opc)
		return nil
	}
	if len(ops) != 0 {
		return errOps()
	}
	t.a.b(t.cur.op...)
	return nil
}

func instNop(t *textAsm, mn string, size int, ops []aop) error {
	if len(ops) == 0 {
		t.a.b(0x90)
		return nil
	}
	if len(ops) != 1 || !ops[0].isRM() {
		return errOps()
	}
	size, err := opSize(size, ops[0])
	if err != nil {
		return err
	}
	t.encRM(opsize16(size), size == 8, []byte{0x0F, 0x1F}, 0, ops[0], 0)
	return nil
}

func instInt(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if ops[0].kind != okImm {
		return errOps()
	}
	t.a.b(0xCD)
	return t.emitImm(ops[0].val, 1, false)
}

func instEnter(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	t.a.b(0xC8)
	if err := t.emitImm(ops[0].val, 2, false); err != nil {
		return err
	}
	return t.emitImm(ops[1].val, 1, false)
}

// ---- SSE ----

const (
	sseRM    = iota // op xmm/m, xmm
	sseRMI          // op $imm, xmm/m, xmm
	sseMov          // load op / store op+arg2
	sseStore        // store only: op xmm, m
	sseShift        // psllw-style: register form and imm form
)

type sseDef struct {
	pfx   []byte
	op    []byte // after 0F
	kind  int
	store byte // store opcode (sseMov)
	ext   int  // imm-shift: opcode and extension
	iop   byte
}

var sseTable = map[string]sseDef{}

func instSSE(t *textAsm, mn string, size int, ops []aop) error {
	d := sseTable[mn]
	op := append([]byte{0x0F}, d.op...)
	switch d.kind {
	case sseRM:
		if len(ops) == 3 && ops[0].isXMM() && ops[0].reg == XMM0 {
			ops = ops[1:] // blendv: implicit %xmm0
		}
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !(ops[0].isXMM() || ops[0].kind == okMem) || !ops[1].isXMM() {
			return errOps()
		}
		t.encRM(d.pfx, false, op, ops[1].reg.num(), ops[0], 0)
	case sseRMI:
		if err := nops(ops, 3); err != nil {
			return err
		}
		if ops[0].kind != okImm || !(ops[1].isXMM() || ops[1].kind == okMem) || !ops[2].isXMM() {
			return errOps()
		}
		t.encRM(d.pfx, false, op, ops[2].reg.num(), ops[1], 1)
		return t.emitImm(ops[0].val, 1, false)
	case sseMov:
		if err := nops(ops, 2); err != nil {
			return err
		}
		switch {
		case ops[1].isXMM() && (ops[0].isXMM() || ops[0].kind == okMem):
			t.encRM(d.pfx, false, op, ops[1].reg.num(), ops[0], 0)
		case ops[0].isXMM() && ops[1].kind == okMem:
			sop := append([]byte{0x0F}, d.op[:len(d.op)-1]...)
			sop = append(sop, d.store)
			t.encRM(d.pfx, false, sop, ops[0].reg.num(), ops[1], 0)
		default:
			return errOps()
		}
	case sseStore:
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !ops[0].isXMM() || ops[1].kind != okMem {
			return errOps()
		}
		t.encRM(d.pfx, false, op, ops[0].reg.num(), ops[1], 0)
	case sseShift:
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !ops[1].isXMM() {
			return errOps()
		}
		if ops[0].kind == okImm {
			t.encRM(d.pfx, false, []byte{0x0F, d.iop}, d.ext, ops[1], 1)
			return t.emitImm(ops[0].val, 1, false)
		}
		if d.op == nil {
			return errOps()
		}
		t.encRM(d.pfx, false, op, ops[1].reg.num(), ops[0], 0)
	}
	return nil
}

// cvtsi2ss/sd: int r/m -> xmm (suffix l/q selects the source size)
func instCvtFromInt(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !src.isRM() || !dst.isXMM() {
		return errOps()
	}
	if size == 0 {
		size = 4
		if src.isGP() {
			size = src.size
		}
	}
	if size != 4 && size != 8 {
		return errOps()
	}
	t.encRM(t.cur.pfx, size == 8, t.cur.op, dst.reg.num(), src, 0)
	return nil
}

// cvt(t)ss2si etc: xmm/m -> int register
func instCvtToInt(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	if !(src.isXMM() || src.kind == okMem) || !dst.isGP() || dst.size < 4 {
		return errOps()
	}
	t.encRM(t.cur.pfx, dst.size == 8, t.cur.op, dst.reg.num(), src, 0)
	return nil
}

// pmovmskb/movmskps/movmskpd: xmm -> int register
func instMovmsk(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	if !ops[0].isXMM() || !ops[1].isGP() || ops[1].size < 4 {
		return errOps()
	}
	t.encRM(t.cur.pfx, false, t.cur.op, ops[1].reg.num(), ops[0], 0)
	return nil
}

func instMovdq(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	q := strings.HasPrefix(mn, "movq")
	switch {
	case dst.isXMM() && src.isGP():
		t.encRM([]byte{0x66}, src.size == 8, []byte{0x0F, 0x6E}, dst.reg.num(), src, 0)
	case src.isXMM() && dst.isGP():
		t.encRM([]byte{0x66}, dst.size == 8, []byte{0x0F, 0x7E}, src.reg.num(), dst, 0)
	case q && dst.isXMM() && (src.isXMM() || src.kind == okMem):
		t.encRM([]byte{0xF3}, false, []byte{0x0F, 0x7E}, dst.reg.num(), src, 0)
	case q && src.isXMM() && dst.kind == okMem:
		t.encRM([]byte{0x66}, false, []byte{0x0F, 0xD6}, src.reg.num(), dst, 0)
	case !q && dst.isXMM() && src.kind == okMem:
		t.encRM([]byte{0x66}, false, []byte{0x0F, 0x6E}, dst.reg.num(), src, 0)
	case !q && src.isXMM() && dst.kind == okMem:
		t.encRM([]byte{0x66}, false, []byte{0x0F, 0x7E}, src.reg.num(), dst, 0)
	default:
		return errOps()
	}
	return nil
}

// pextr{b,w,d,q}, extractps: $imm, xmm, r/m  (op: 66 0F 3A xx, reg = xmm)
func instPextr(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 3); err != nil {
		return err
	}
	if ops[0].kind != okImm || !ops[1].isXMM() || !ops[2].isRM() {
		return errOps()
	}
	if mn == "pextrw" && ops[2].isGP() {
		// legacy form 66 0F C5 /r: reg = destination
		t.encRM([]byte{0x66}, false, []byte{0x0F, 0xC5}, ops[2].reg.num(), ops[1], 1)
		return t.emitImm(ops[0].val, 1, false)
	}
	t.encRM([]byte{0x66}, mn == "pextrq", t.cur.op, ops[1].reg.num(), ops[2], 1)
	return t.emitImm(ops[0].val, 1, false)
}

// pinsr{b,w,d,q}, insertps: $imm, r/m, xmm
func instPinsr(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 3); err != nil {
		return err
	}
	if ops[0].kind != okImm || !(ops[1].isRM() || ops[1].isXMM()) || !ops[2].isXMM() {
		return errOps()
	}
	t.encRM([]byte{0x66}, mn == "pinsrq", t.cur.op, ops[2].reg.num(), ops[1], 1)
	return t.emitImm(ops[0].val, 1, false)
}

// cmpeqps-style pseudo instructions
func instCmpPseudo(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	if !(ops[0].isXMM() || ops[0].kind == okMem) || !ops[1].isXMM() {
		return errOps()
	}
	t.encRM(t.cur.pfx, false, []byte{0x0F, 0xC2}, ops[1].reg.num(), ops[0], 1)
	t.a.b(byte(t.cur.arg))
	return nil
}

// ---- x87 ----

// x87 memory operand: op /arg
func instFMem(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if ops[0].kind != okMem {
		return errOps()
	}
	t.encRM(nil, false, t.cur.op, t.cur.arg, ops[0], 0)
	return nil
}

func stIndex(o aop) (int, bool) {
	if o.kind == okST {
		return o.st, true
	}
	return 0, false
}

// fld/fst/fstp/fxch/ffree/fucom...: op[0], op[1]+i with %st(i); with no
// operand i = arg
func instFSt(t *textAsm, mn string, size int, ops []aop) error {
	i := t.cur.arg
	switch len(ops) {
	case 0:
	case 1:
		k, ok := stIndex(ops[0])
		if !ok {
			return errOps()
		}
		i = k
	case 2:
		// fcomi %st(i), %st
		k, ok := stIndex(ops[0])
		if j, ok2 := stIndex(ops[1]); !ok || !ok2 || j != 0 {
			return errOps()
		}
		i = k
	default:
		return errOps()
	}
	t.a.b(t.cur.op[0], t.cur.op[1]+byte(i))
	return nil
}

// fadd/fsub/fsubr/fmul/fdiv/fdivr (arg = /digit); suffix s/l handled by entries
func instFArith(t *textAsm, mn string, size int, ops []aop) error {
	ext := t.cur.arg
	pop := t.cur.op != nil // "p" variants
	switch len(ops) {
	case 0:
		// faddp: st(1) op= st, pop
		t.a.b(0xDE, 0xC0+byte(ext<<3)+1)
		return nil
	case 1:
		if pop {
			i, ok := stIndex(ops[0])
			if !ok {
				return errOps()
			}
			t.a.b(0xDE, 0xC0+byte(ext<<3)+byte(i))
			return nil
		}
		i, ok := stIndex(ops[0])
		if !ok {
			return errOps()
		}
		t.a.b(0xD8, 0xC0+byte(ext<<3)+byte(i))
		return nil
	case 2:
		i, ok1 := stIndex(ops[0])
		j, ok2 := stIndex(ops[1])
		if !ok1 || !ok2 {
			return errOps()
		}
		switch {
		case j == 0 && !pop:
			t.a.b(0xD8, 0xC0+byte(ext<<3)+byte(i))
		case i == 0:
			opc := byte(0xDC)
			if pop {
				opc = 0xDE
			}
			t.a.b(opc, 0xC0+byte(ext<<3)+byte(j))
		default:
			return errOps()
		}
		return nil
	}
	return errOps()
}

func instFnstsw(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 0, 1); err != nil {
		return err
	}
	if t.cur.arg == 1 {
		t.a.b(0x9B)
	}
	if len(ops) == 0 || ops[0].isGP() && ops[0].reg == RAX && ops[0].size == 2 {
		t.a.b(0xDF, 0xE0)
		return nil
	}
	if ops[0].kind != okMem {
		return errOps()
	}
	t.encRM(nil, false, []byte{0xDD}, 7, ops[0], 0)
	return nil
}

func instFstcw(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 1); err != nil {
		return err
	}
	if ops[0].kind != okMem {
		return errOps()
	}
	t.a.b(0x9B)
	t.encRM(nil, false, t.cur.op, t.cur.arg, ops[0], 0)
	return nil
}

// ---- table construction ----

func def(fn instFn, suffix bool, arg int, op ...byte) *instDef {
	return &instDef{fn: fn, suffix: suffix, arg: arg, op: op}
}

func init() {
	insts = map[string]*instDef{}
	for i, n := range aluNames {
		insts[n] = def(instAlu, true, i)
	}
	insts["mov"] = def(instMov, true, 0)
	insts["movabs"] = def(instMov, true, 0)
	for _, n := range []string{"movzbw", "movzbl", "movzbq", "movzwl", "movzwq", "movzx", "movzxb", "movzxw"} {
		insts[n] = def(instMovx, false, 0)
	}
	for _, n := range []string{"movsbw", "movsbl", "movsbq", "movswl", "movswq", "movslq", "movsxd", "movsx", "movsxb", "movsxw"} {
		insts[n] = def(instMovx, false, 1)
	}
	insts["lea"] = def(instLea, true, 0)
	insts["test"] = def(instTest, true, 0)
	for _, u := range []struct {
		n   string
		ext int
		op  byte
	}{{"not", 2, 0xF6}, {"neg", 3, 0xF6}, {"mul", 4, 0xF6}, {"div", 6, 0xF6}, {"idiv", 7, 0xF6}, {"inc", 0, 0xFE}, {"dec", 1, 0xFE}} {
		insts[u.n] = def(instUnary, true, u.ext, u.op)
	}
	insts["imul"] = def(instImul, true, 0)
	for _, s := range []struct {
		n   string
		ext int
	}{{"rol", 0}, {"ror", 1}, {"rcl", 2}, {"rcr", 3}, {"shl", 4}, {"sal", 4}, {"shr", 5}, {"sar", 7}} {
		insts[s.n] = def(instShift, true, s.ext)
	}
	insts["shld"] = def(instShxd, true, 0, 0xA4)
	insts["shrd"] = def(instShxd, true, 0, 0xAC)
	for i, n := range []string{"bt", "bts", "btr", "btc"} {
		insts[n] = def(instBt, true, 4+i)
	}
	rr := func(n string, pfx []byte, op ...byte) {
		d := def(instRegRM, true, 0, op...)
		d.pfx = pfx
		insts[n] = d
	}
	rr("bsf", nil, 0x0F, 0xBC)
	rr("bsr", nil, 0x0F, 0xBD)
	rr("popcnt", []byte{0xF3}, 0x0F, 0xB8)
	rr("lzcnt", []byte{0xF3}, 0x0F, 0xBD)
	rr("tzcnt", []byte{0xF3}, 0x0F, 0xBC)
	rr("adcx", []byte{0x66}, 0x0F, 0x38, 0xF6)
	rr("adox", []byte{0xF3}, 0x0F, 0x38, 0xF6)
	insts["crc32"] = def(instCrc32, true, 0)
	insts["movbe"] = def(instMovbe, true, 0)
	insts["bswap"] = def(instBswap, true, 0)
	insts["xchg"] = def(instXchg, true, 0)
	insts["xadd"] = def(instXadd, true, 0, 0xC0)
	insts["cmpxchg"] = def(instXadd, true, 0, 0xB0)
	mo := func(n string, pfx []byte, ext int, op ...byte) {
		d := def(instMemOnly, false, ext, op...)
		d.pfx = pfx
		insts[n] = d
	}
	mo("cmpxchg8b", nil, 1, 0x0F, 0xC7)
	mo("cmpxchg16b", []byte{0x48}, 1, 0x0F, 0xC7)
	mo("clflush", nil, 7, 0x0F, 0xAE)
	mo("clflushopt", []byte{0x66}, 7, 0x0F, 0xAE)
	mo("clwb", []byte{0x66}, 6, 0x0F, 0xAE)
	mo("prefetchnta", nil, 0, 0x0F, 0x18)
	mo("prefetcht0", nil, 1, 0x0F, 0x18)
	mo("prefetcht1", nil, 2, 0x0F, 0x18)
	mo("prefetcht2", nil, 3, 0x0F, 0x18)
	mo("prefetchw", nil, 1, 0x0F, 0x0D)
	mo("prefetch", nil, 0, 0x0F, 0x0D)
	mo("ldmxcsr", nil, 2, 0x0F, 0xAE)
	mo("stmxcsr", nil, 3, 0x0F, 0xAE)
	mo("fxsave", nil, 0, 0x0F, 0xAE)
	mo("fxrstor", nil, 1, 0x0F, 0xAE)
	mo("fxsave64", []byte{0x48}, 0, 0x0F, 0xAE)
	mo("fxrstor64", []byte{0x48}, 1, 0x0F, 0xAE)
	mo("xsave", nil, 4, 0x0F, 0xAE)
	mo("xrstor", nil, 5, 0x0F, 0xAE)
	mo("xsave64", []byte{0x48}, 4, 0x0F, 0xAE)
	mo("xrstor64", []byte{0x48}, 5, 0x0F, 0xAE)
	insts["rdrand"] = def(instRegOnly, true, 6, 0x0F, 0xC7)
	insts["rdseed"] = def(instRegOnly, true, 7, 0x0F, 0xC7)
	for cn, c := range condCodes {
		insts["set"+cn] = def(instSetcc, false, int(c))
		insts["cmov"+cn] = def(instCmov, true, int(c))
		insts["j"+cn] = def(instJcc, false, int(c))
	}
	insts["jmp"] = def(instJmpCall, true, 4)
	insts["call"] = def(instJmpCall, true, 2)
	insts["jrcxz"] = def(instShortJump, false, 0, 0xE3)
	insts["jecxz"] = def(instShortJump, false, 0, 0x67, 0xE3)
	insts["loop"] = def(instShortJump, false, 0, 0xE2)
	insts["loope"] = def(instShortJump, false, 0, 0xE1)
	insts["loopz"] = def(instShortJump, false, 0, 0xE1)
	insts["loopne"] = def(instShortJump, false, 0, 0xE0)
	insts["loopnz"] = def(instShortJump, false, 0, 0xE0)
	insts["ret"] = def(instRet, true, 0)
	insts["push"] = def(instPush, true, 0)
	insts["pop"] = def(instPush, true, 1)
	insts["nop"] = def(instNop, true, 0)
	insts["int"] = def(instInt, false, 0)
	insts["enter"] = def(instEnter, true, 0)
	fixed := map[string][]byte{
		"leave": {0xC9}, "hlt": {0xF4}, "int3": {0xCC}, "into": {0xCE}, "ud2": {0x0F, 0x0B}, "syscall": {0x0F, 0x05},
		"sysenter": {0x0F, 0x34}, "cpuid": {0x0F, 0xA2}, "rdtsc": {0x0F, 0x31}, "rdtscp": {0x0F, 0x01, 0xF9},
		"rdpmc": {0x0F, 0x33}, "pause": {0xF3, 0x90}, "mfence": {0x0F, 0xAE, 0xF0}, "lfence": {0x0F, 0xAE, 0xE8},
		"sfence": {0x0F, 0xAE, 0xF8}, "clc": {0xF8}, "stc": {0xF9}, "cmc": {0xF5}, "cld": {0xFC}, "std": {0xFD},
		"cli": {0xFA}, "sti": {0xFB}, "lahf": {0x9F}, "sahf": {0x9E}, "xgetbv": {0x0F, 0x01, 0xD0},
		"xsetbv": {0x0F, 0x01, 0xD1}, "pushf": {0x9C}, "pushfq": {0x9C}, "popf": {0x9D}, "popfq": {0x9D},
		"pushfw": {0x66, 0x9C}, "popfw": {0x66, 0x9D},
		"cbtw": {0x66, 0x98}, "cbw": {0x66, 0x98}, "cwtl": {0x98}, "cwde": {0x98}, "cltq": {0x48, 0x98}, "cdqe": {0x48, 0x98},
		"cwtd": {0x66, 0x99}, "cwd": {0x66, 0x99}, "cltd": {0x99}, "cdq": {0x99}, "cqto": {0x48, 0x99}, "cqo": {0x48, 0x99},
		"endbr64": {0xF3, 0x0F, 0x1E, 0xFA}, "wbinvd": {0x0F, 0x09}, "xlat": {0xD7}, "xlatb": {0xD7},
		"emms": {0x0F, 0x77}, "rsm": {0x0F, 0xAA}, "iret": {0xCF}, "iretq": {0x48, 0xCF}, "sysret": {0x0F, 0x07},
		"swapgs": {0x0F, 0x01, 0xF8}, "monitor": {0x0F, 0x01, 0xC8}, "mwait": {0x0F, 0x01, 0xC9},
		"xend": {0x0F, 0x01, 0xD5}, "xtest": {0x0F, 0x01, 0xD6},
		"leaveq": {0xC9}, "retq": {0xC3}, "serialize": {0x0F, 0x01, 0xE8},
		// x87 without operands
		"fchs": {0xD9, 0xE0}, "fabs": {0xD9, 0xE1}, "ftst": {0xD9, 0xE4}, "fxam": {0xD9, 0xE5}, "fld1": {0xD9, 0xE8},
		"fldl2t": {0xD9, 0xE9}, "fldl2e": {0xD9, 0xEA}, "fldpi": {0xD9, 0xEB}, "fldlg2": {0xD9, 0xEC},
		"fldln2": {0xD9, 0xED}, "fldz": {0xD9, 0xEE}, "f2xm1": {0xD9, 0xF0}, "fyl2x": {0xD9, 0xF1},
		"fptan": {0xD9, 0xF2}, "fpatan": {0xD9, 0xF3}, "fxtract": {0xD9, 0xF4}, "fprem1": {0xD9, 0xF5},
		"fdecstp": {0xD9, 0xF6}, "fincstp": {0xD9, 0xF7}, "fprem": {0xD9, 0xF8}, "fyl2xp1": {0xD9, 0xF9},
		"fsqrt": {0xD9, 0xFA}, "fsincos": {0xD9, 0xFB}, "frndint": {0xD9, 0xFC}, "fscale": {0xD9, 0xFD},
		"fsin": {0xD9, 0xFE}, "fcos": {0xD9, 0xFF}, "fnop": {0xD9, 0xD0}, "fcompp": {0xDE, 0xD9},
		"fucompp": {0xDA, 0xE9}, "fwait": {0x9B}, "wait": {0x9B}, "fninit": {0xDB, 0xE3}, "finit": {0x9B, 0xDB, 0xE3},
		"fnclex": {0xDB, 0xE2}, "fclex": {0x9B, 0xDB, 0xE2},
	}
	for n, b := range fixed {
		insts[n] = def(instFixed, false, 0, b...)
	}
	for n, op := range map[string]byte{"movs": 0xA4, "stos": 0xAA, "lods": 0xAC, "scas": 0xAE, "cmps": 0xA6} {
		d := def(instFixed, true, 1, op)
		insts[n] = d
	}
	// movsd/cmpsd with no operands are string instructions; with operands SSE
	insts["movsd"] = def(func(t *textAsm, mn string, size int, ops []aop) error {
		if len(ops) == 0 {
			t.a.b(0xA5)
			return nil
		}
		return instSSE(t, "movsd", size, ops)
	}, false, 0)
	insts["cmpsd"] = def(func(t *textAsm, mn string, size int, ops []aop) error {
		if len(ops) == 0 {
			t.a.b(0xA7)
			return nil
		}
		return instSSE(t, "cmpsd", size, ops)
	}, false, 0)

	// SSE
	type ps struct {
		sfx string
		pfx []byte
	}
	four := []ps{{"ps", nil}, {"pd", []byte{0x66}}, {"ss", []byte{0xF3}}, {"sd", []byte{0xF2}}}
	add := func(n string, d sseDef) {
		sseTable[n] = d
		if n == "movsd" || n == "cmpsd" {
			return
		}
		insts[n] = def(instSSE, false, 0)
	}
	for _, a := range []struct {
		n  string
		op byte
	}{{"add", 0x58}, {"mul", 0x59}, {"sub", 0x5C}, {"min", 0x5D}, {"div", 0x5E}, {"max", 0x5F}, {"sqrt", 0x51}} {
		for _, f := range four {
			add(a.n+f.sfx, sseDef{pfx: f.pfx, op: []byte{a.op}})
		}
	}
	for _, f := range four {
		add("cmp"+f.sfx, sseDef{pfx: f.pfx, op: []byte{0xC2}, kind: sseRMI})
		for i, c := range []string{"eq", "lt", "le", "unord", "neq", "nlt", "nle", "ord"} {
			d := def(instCmpPseudo, false, i)
			d.pfx = f.pfx
			insts["cmp"+c+f.sfx] = d
		}
	}
	add("rcpps", sseDef{op: []byte{0x53}})
	add("rcpss", sseDef{pfx: []byte{0xF3}, op: []byte{0x53}})
	add("rsqrtps", sseDef{op: []byte{0x52}})
	add("rsqrtss", sseDef{pfx: []byte{0xF3}, op: []byte{0x52}})
	for _, l := range []struct {
		n  string
		op byte
	}{{"and", 0x54}, {"andn", 0x55}, {"or", 0x56}, {"xor", 0x57}, {"unpckl", 0x14}, {"unpckh", 0x15}} {
		add(l.n+"ps", sseDef{op: []byte{l.op}})
		add(l.n+"pd", sseDef{pfx: []byte{0x66}, op: []byte{l.op}})
	}
	add("shufps", sseDef{op: []byte{0xC6}, kind: sseRMI})
	add("shufpd", sseDef{pfx: []byte{0x66}, op: []byte{0xC6}, kind: sseRMI})
	add("ucomiss", sseDef{op: []byte{0x2E}})
	add("ucomisd", sseDef{pfx: []byte{0x66}, op: []byte{0x2E}})
	add("comiss", sseDef{op: []byte{0x2F}})
	add("comisd", sseDef{pfx: []byte{0x66}, op: []byte{0x2F}})
	add("movaps", sseDef{op: []byte{0x28}, kind: sseMov, store: 0x29})
	add("movapd", sseDef{pfx: []byte{0x66}, op: []byte{0x28}, kind: sseMov, store: 0x29})
	add("movups", sseDef{op: []byte{0x10}, kind: sseMov, store: 0x11})
	add("movupd", sseDef{pfx: []byte{0x66}, op: []byte{0x10}, kind: sseMov, store: 0x11})
	add("movss", sseDef{pfx: []byte{0xF3}, op: []byte{0x10}, kind: sseMov, store: 0x11})
	add("movsd", sseDef{pfx: []byte{0xF2}, op: []byte{0x10}, kind: sseMov, store: 0x11})
	add("movdqa", sseDef{pfx: []byte{0x66}, op: []byte{0x6F}, kind: sseMov, store: 0x7F})
	add("movdqu", sseDef{pfx: []byte{0xF3}, op: []byte{0x6F}, kind: sseMov, store: 0x7F})
	add("movlps", sseDef{op: []byte{0x12}, kind: sseMov, store: 0x13})
	add("movlpd", sseDef{pfx: []byte{0x66}, op: []byte{0x12}, kind: sseMov, store: 0x13})
	add("movhps", sseDef{op: []byte{0x16}, kind: sseMov, store: 0x17})
	add("movhpd", sseDef{pfx: []byte{0x66}, op: []byte{0x16}, kind: sseMov, store: 0x17})
	add("movhlps", sseDef{op: []byte{0x12}})
	add("movlhps", sseDef{op: []byte{0x16}})
	add("movntps", sseDef{op: []byte{0x2B}, kind: sseStore})
	add("movntpd", sseDef{pfx: []byte{0x66}, op: []byte{0x2B}, kind: sseStore})
	add("movntdq", sseDef{pfx: []byte{0x66}, op: []byte{0xE7}, kind: sseStore})
	add("movntdqa", sseDef{pfx: []byte{0x66}, op: []byte{0x38, 0x2A}})
	add("lddqu", sseDef{pfx: []byte{0xF2}, op: []byte{0xF0}})
	for n, d := range map[string]sseDef{
		"cvtss2sd": {pfx: []byte{0xF3}, op: []byte{0x5A}}, "cvtsd2ss": {pfx: []byte{0xF2}, op: []byte{0x5A}},
		"cvtps2pd": {op: []byte{0x5A}}, "cvtpd2ps": {pfx: []byte{0x66}, op: []byte{0x5A}},
		"cvtdq2ps": {op: []byte{0x5B}}, "cvtps2dq": {pfx: []byte{0x66}, op: []byte{0x5B}},
		"cvttps2dq": {pfx: []byte{0xF3}, op: []byte{0x5B}}, "cvtdq2pd": {pfx: []byte{0xF3}, op: []byte{0xE6}},
		"cvtpd2dq": {pfx: []byte{0xF2}, op: []byte{0xE6}}, "cvttpd2dq": {pfx: []byte{0x66}, op: []byte{0xE6}},
		"haddps": {pfx: []byte{0xF2}, op: []byte{0x7C}}, "haddpd": {pfx: []byte{0x66}, op: []byte{0x7C}},
		"hsubps": {pfx: []byte{0xF2}, op: []byte{0x7D}}, "hsubpd": {pfx: []byte{0x66}, op: []byte{0x7D}},
		"addsubps": {pfx: []byte{0xF2}, op: []byte{0xD0}}, "addsubpd": {pfx: []byte{0x66}, op: []byte{0xD0}},
		"movddup": {pfx: []byte{0xF2}, op: []byte{0x12}}, "movshdup": {pfx: []byte{0xF3}, op: []byte{0x16}},
		"movsldup": {pfx: []byte{0xF3}, op: []byte{0x12}},
		"pshufd":   {pfx: []byte{0x66}, op: []byte{0x70}, kind: sseRMI}, "pshuflw": {pfx: []byte{0xF2}, op: []byte{0x70}, kind: sseRMI},
		"pshufhw": {pfx: []byte{0xF3}, op: []byte{0x70}, kind: sseRMI},
	} {
		add(n, d)
	}
	for n, op := range map[string]byte{
		"punpcklbw": 0x60, "punpcklwd": 0x61, "punpckldq": 0x62, "packsswb": 0x63, "pcmpgtb": 0x64, "pcmpgtw": 0x65,
		"pcmpgtd": 0x66, "packuswb": 0x67, "punpckhbw": 0x68, "punpckhwd": 0x69, "punpckhdq": 0x6A, "packssdw": 0x6B,
		"punpcklqdq": 0x6C, "punpckhqdq": 0x6D, "pcmpeqb": 0x74, "pcmpeqw": 0x75, "pcmpeqd": 0x76, "paddq": 0xD4,
		"pmullw": 0xD5, "psubusb": 0xD8, "psubusw": 0xD9, "pminub": 0xDA, "pand": 0xDB, "paddusb": 0xDC,
		"paddusw": 0xDD, "pmaxub": 0xDE, "pandn": 0xDF, "pavgb": 0xE0, "pavgw": 0xE3, "pmulhuw": 0xE4,
		"pmulhw": 0xE5, "psubsb": 0xE8, "psubsw": 0xE9, "pminsw": 0xEA, "por": 0xEB, "paddsb": 0xEC,
		"paddsw": 0xED, "pmaxsw": 0xEE, "pxor": 0xEF, "pmuludq": 0xF4, "pmaddwd": 0xF5, "psadbw": 0xF6,
		"psubb": 0xF8, "psubw": 0xF9, "psubd": 0xFA, "psubq": 0xFB, "paddb": 0xFC, "paddw": 0xFD, "paddd": 0xFE,
	} {
		add(n, sseDef{pfx: []byte{0x66}, op: []byte{op}})
	}
	for n, s := range map[string]struct {
		op, iop byte
		ext     int
	}{
		"psrlw": {0xD1, 0x71, 2}, "psraw": {0xE1, 0x71, 4}, "psllw": {0xF1, 0x71, 6},
		"psrld": {0xD2, 0x72, 2}, "psrad": {0xE2, 0x72, 4}, "pslld": {0xF2, 0x72, 6},
		"psrlq": {0xD3, 0x73, 2}, "psllq": {0xF3, 0x73, 6}, "psrldq": {0, 0x73, 3}, "pslldq": {0, 0x73, 7},
	} {
		d := sseDef{pfx: []byte{0x66}, kind: sseShift, iop: s.iop, ext: s.ext}
		if s.op != 0 {
			d.op = []byte{s.op}
		}
		add(n, d)
	}
	for n, op := range map[string]byte{
		"pshufb": 0x00, "phaddw": 0x01, "phaddd": 0x02, "phaddsw": 0x03, "pmaddubsw": 0x04, "phsubw": 0x05,
		"phsubd": 0x06, "phsubsw": 0x07, "psignb": 0x08, "psignw": 0x09, "psignd": 0x0A, "pmulhrsw": 0x0B,
		"pblendvb": 0x10, "blendvps": 0x14, "blendvpd": 0x15, "ptest": 0x17, "pabsb": 0x1C, "pabsw": 0x1D, "pabsd": 0x1E,
		"pmovsxbw": 0x20, "pmovsxbd": 0x21, "pmovsxbq": 0x22, "pmovsxwd": 0x23, "pmovsxwq": 0x24, "pmovsxdq": 0x25,
		"pmuldq": 0x28, "pcmpeqq": 0x29, "packusdw": 0x2B, "pmovzxbw": 0x30, "pmovzxbd": 0x31, "pmovzxbq": 0x32,
		"pmovzxwd": 0x33, "pmovzxwq": 0x34, "pmovzxdq": 0x35, "pcmpgtq": 0x37, "pminsb": 0x38, "pminsd": 0x39,
		"pminuw": 0x3A, "pminud": 0x3B, "pmaxsb": 0x3C, "pmaxsd": 0x3D, "pmaxuw": 0x3E, "pmaxud": 0x3F,
		"pmulld": 0x40, "phminposuw": 0x41, "aesimc": 0xDB, "aesenc": 0xDC, "aesenclast": 0xDD, "aesdec": 0xDE,
		"aesdeclast": 0xDF,
	} {
		add(n, sseDef{pfx: []byte{0x66}, op: []byte{0x38, op}})
	}
	for n, op := range map[string]byte{"sha1nexte": 0xC8, "sha1msg1": 0xC9, "sha1msg2": 0xCA, "sha256rnds2": 0xCB, "sha256msg1": 0xCC, "sha256msg2": 0xCD} {
		add(n, sseDef{op: []byte{0x38, op}})
	}
	add("sha1rnds4", sseDef{op: []byte{0x3A, 0xCC}, kind: sseRMI})
	for n, op := range map[string]byte{
		"roundps": 0x08, "roundpd": 0x09, "roundss": 0x0A, "roundsd": 0x0B, "blendps": 0x0C, "blendpd": 0x0D,
		"pblendw": 0x0E, "palignr": 0x0F, "dpps": 0x40, "dppd": 0x41, "mpsadbw": 0x42, "pclmulqdq": 0x44,
		"pcmpestrm": 0x60, "pcmpestri": 0x61, "pcmpistrm": 0x62, "pcmpistri": 0x63, "aeskeygenassist": 0xDF,
	} {
		add(n, sseDef{pfx: []byte{0x66}, op: []byte{0x3A, op}, kind: sseRMI})
	}
	cvt := func(n string, fn instFn, pfx byte, op byte) {
		d := def(fn, true, 0, 0x0F, op)
		d.pfx = []byte{pfx}
		insts[n] = d
	}
	cvt("cvtsi2ss", instCvtFromInt, 0xF3, 0x2A)
	cvt("cvtsi2sd", instCvtFromInt, 0xF2, 0x2A)
	cvt("cvttss2si", instCvtToInt, 0xF3, 0x2C)
	cvt("cvttsd2si", instCvtToInt, 0xF2, 0x2C)
	cvt("cvtss2si", instCvtToInt, 0xF3, 0x2D)
	cvt("cvtsd2si", instCvtToInt, 0xF2, 0x2D)
	insts["movd"] = def(instMovdq, false, 0)
	insts["movq"] = def(func(t *textAsm, mn string, size int, ops []aop) error {
		if len(ops) == 2 && (ops[0].isXMM() || ops[1].isXMM()) {
			return instMovdq(t, mn, size, ops)
		}
		return instMov(t, "mov", 8, ops)
	}, false, 0)
	mm := func(n string, pfx []byte, op byte) {
		d := def(instMovmsk, false, 0, 0x0F, op)
		d.pfx = pfx
		insts[n] = d
	}
	mm("pmovmskb", []byte{0x66}, 0xD7)
	mm("movmskps", nil, 0x50)
	mm("movmskpd", []byte{0x66}, 0x50)
	insts["pextrb"] = def(instPextr, false, 0, 0x0F, 0x3A, 0x14)
	insts["pextrw"] = def(instPextr, false, 0, 0x0F, 0x3A, 0x15)
	insts["pextrd"] = def(instPextr, false, 0, 0x0F, 0x3A, 0x16)
	insts["pextrq"] = def(instPextr, false, 0, 0x0F, 0x3A, 0x16)
	insts["extractps"] = def(instPextr, false, 0, 0x0F, 0x3A, 0x17)
	insts["pinsrb"] = def(instPinsr, false, 0, 0x0F, 0x3A, 0x20)
	insts["pinsrw"] = def(instPinsr, false, 0, 0x0F, 0xC4)
	insts["pinsrd"] = def(instPinsr, false, 0, 0x0F, 0x3A, 0x22)
	insts["pinsrq"] = def(instPinsr, false, 0, 0x0F, 0x3A, 0x22)
	insts["insertps"] = def(instPinsr, false, 0, 0x0F, 0x3A, 0x21)
	insts["movnti"] = def(func(t *textAsm, mn string, size int, ops []aop) error {
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !ops[0].isGP() || ops[1].kind != okMem || ops[0].size < 4 {
			return errOps()
		}
		t.encRM(nil, ops[0].size == 8, []byte{0x0F, 0xC3}, ops[0].reg.num(), ops[1], 0)
		return nil
	}, true, 0)

	// x87 memory forms
	fm := func(n string, ext int, op ...byte) { insts[n] = def(instFMem, false, ext, op...) }
	fm("flds", 0, 0xD9)
	fm("fldl", 0, 0xDD)
	fm("fldt", 5, 0xDB)
	fm("fsts", 2, 0xD9)
	fm("fstl", 2, 0xDD)
	fm("fstps", 3, 0xD9)
	fm("fstpl", 3, 0xDD)
	fm("fstpt", 7, 0xDB)
	fm("filds", 0, 0xDF)
	fm("fildl", 0, 0xDB)
	fm("fildll", 5, 0xDF)
	fm("fildq", 5, 0xDF)
	fm("fists", 2, 0xDF)
	fm("fistl", 2, 0xDB)
	fm("fistps", 3, 0xDF)
	fm("fistpl", 3, 0xDB)
	fm("fistpll", 7, 0xDF)
	fm("fistpq", 7, 0xDF)
	fm("fisttps", 1, 0xDF)
	fm("fisttpl", 1, 0xDB)
	fm("fisttpll", 1, 0xDD)
	fm("fisttpq", 1, 0xDD)
	fm("fldcw", 5, 0xD9)
	fm("fnstcw", 7, 0xD9)
	fm("fldenv", 4, 0xD9)
	fm("fnstenv", 6, 0xD9)
	fm("frstor", 4, 0xDD)
	fm("fnsave", 6, 0xDD)
	fm("fbld", 4, 0xDF)
	fm("fbstp", 6, 0xDF)
	insts["fstcw"] = def(instFstcw, false, 7, 0xD9)
	insts["fstenv"] = def(instFstcw, false, 6, 0xD9)
	insts["fsave"] = def(instFstcw, false, 6, 0xDD)
	insts["fnstsw"] = def(instFnstsw, false, 0)
	insts["fstsw"] = def(instFnstsw, false, 1)
	for i, n := range []string{"add", "mul", "com", "comp", "sub", "subr", "div", "divr"} {
		fm("f"+n+"s", i, 0xD8)
		fm("f"+n+"l", i, 0xDC)
		fm("fi"+n+"s", i, 0xDE)
		fm("fi"+n+"l", i, 0xDA)
		if n != "com" && n != "comp" {
			insts["f"+n] = def(instFArith, false, i)
			insts["f"+n+"p"] = &instDef{fn: instFArith, arg: i, op: []byte{1}}
		}
	}
	fs := func(n string, def0 int, op ...byte) { insts[n] = def(instFSt, false, def0, op...) }
	fs("fld", 0, 0xD9, 0xC0)
	fs("fst", 0, 0xDD, 0xD0)
	fs("fstp", 0, 0xDD, 0xD8)
	fs("fxch", 1, 0xD9, 0xC8)
	fs("ffree", 0, 0xDD, 0xC0)
	fs("fcom", 1, 0xD8, 0xD0)
	fs("fcomp", 1, 0xD8, 0xD8)
	fs("fucom", 1, 0xDD, 0xE0)
	fs("fucomp", 1, 0xDD, 0xE8)
	fs("fcomi", 1, 0xDB, 0xF0)
	fs("fcomip", 1, 0xDF, 0xF0)
	fs("fucomi", 1, 0xDB, 0xE8)
	fs("fucomip", 1, 0xDF, 0xE8)
	fs("fcmovb", 1, 0xDA, 0xC0)
	fs("fcmove", 1, 0xDA, 0xC8)
	fs("fcmovbe", 1, 0xDA, 0xD0)
	fs("fcmovu", 1, 0xDA, 0xD8)
	fs("fcmovnb", 1, 0xDB, 0xC0)
	fs("fcmovne", 1, 0xDB, 0xC8)
	fs("fcmovnbe", 1, 0xDB, 0xD0)
	fs("fcmovnu", 1, 0xDB, 0xD8)
}

var condCodes = map[string]Cond{
	"o": CondO, "no": CondNO, "b": CondB, "c": CondB, "nae": CondB, "ae": CondAE, "nb": CondAE, "nc": CondAE,
	"e": CondE, "z": CondE, "ne": CondNE, "nz": CondNE, "be": CondBE, "na": CondBE, "a": CondA, "nbe": CondA,
	"s": CondS, "ns": CondNS, "p": CondP, "pe": CondP, "np": CondNP, "po": CondNP, "l": CondL, "nge": CondL,
	"ge": CondGE, "nl": CondGE, "le": CondLE, "ng": CondLE, "g": CondG, "nle": CondG,
}

// ---- directives ----

func (t *textAsm) directive(name, rest string) error {
	switch name {
	case ".byte", ".1byte":
		return t.dataDir(rest, 1)
	case ".word", ".short", ".value", ".2byte", ".hword":
		return t.dataDir(rest, 2)
	case ".long", ".int", ".4byte":
		return t.dataDir(rest, 4)
	case ".quad", ".8byte":
		return t.dataDir(rest, 8)
	case ".ascii", ".asciz", ".string":
		for _, part := range splitTop(rest) {
			s, err := unquoteAsm(part)
			if err != nil {
				return err
			}
			t.a.b([]byte(s)...)
			if name != ".ascii" {
				t.a.b(0)
			}
		}
		return nil
	case ".zero", ".skip", ".space":
		args := splitTop(rest)
		if len(args) == 0 || len(args) > 2 {
			return fmt.Errorf("bad %s", name)
		}
		n, err := t.constExpr(args[0])
		if err != nil {
			return err
		}
		fill := int64(0)
		if len(args) == 2 {
			if fill, err = t.constExpr(args[1]); err != nil {
				return err
			}
		}
		for i := int64(0); i < n; i++ {
			t.a.b(byte(fill))
		}
		return nil
	case ".p2align", ".balign", ".align":
		args := splitTop(rest)
		if len(args) == 0 {
			return fmt.Errorf("bad %s", name)
		}
		n, err := t.constExpr(args[0])
		if err != nil {
			return err
		}
		if name == ".p2align" {
			n = 1 << uint(n)
		}
		fill := int64(-1)
		if len(args) > 1 && strings.TrimSpace(args[1]) != "" {
			if fill, err = t.constExpr(args[1]); err != nil {
				return err
			}
		}
		maxSkip := int64(-1)
		if len(args) > 2 && strings.TrimSpace(args[2]) != "" {
			if maxSkip, err = t.constExpr(args[2]); err != nil {
				return err
			}
		}
		if n <= 1 {
			return nil
		}
		if n&(n-1) != 0 {
			return fmt.Errorf("alignment not a power of 2")
		}
		if n > t.a.MaxAlign {
			t.a.MaxAlign = n
		}
		pad := (n - int64(len(t.a.Code))%n) % n
		if maxSkip >= 0 && pad > maxSkip {
			return nil
		}
		switch {
		case fill >= 0:
			for i := int64(0); i < pad; i++ {
				t.a.b(byte(fill))
			}
		case t.secName == ".text" || strings.HasPrefix(t.secName, ".text."):
			t.a.Nops(int(pad))
		default:
			t.a.b(make([]byte, pad)...)
		}
		return nil
	case ".text", ".data", ".bss":
		return t.switchSection(name, "", false)
	case ".section", ".pushsection":
		args := splitTop(rest)
		if len(args) == 0 {
			return fmt.Errorf("missing section name")
		}
		flags := ""
		if len(args) > 1 {
			flags = strings.Trim(args[1], "\"")
		}
		return t.switchSection(strings.Trim(args[0], "\""), flags, name == ".pushsection")
	case ".popsection":
		if len(t.stack) == 0 {
			return fmt.Errorf(".popsection without .pushsection")
		}
		e := t.stack[len(t.stack)-1]
		t.stack = t.stack[:len(t.stack)-1]
		t.prev = secEntry{t.secName, t.a}
		t.secName, t.a = e.name, e.a
		return nil
	case ".previous":
		cur := secEntry{t.secName, t.a}
		t.secName, t.a = t.prev.name, t.prev.a
		t.prev = cur
		return nil
	case ".set", ".equ", ".equiv":
		args := splitTop(rest)
		if len(args) != 2 {
			return fmt.Errorf("bad %s", name)
		}
		return t.assign(strings.TrimSpace(args[0]), args[1])
	case ".size":
		args := splitTop(rest)
		if len(args) != 2 || t.env.Symbol == nil {
			return nil
		}
		// the size of a global function is a difference of labels of this
		// text, not of relocatable symbols
		saved := t.globals
		t.globals = nil
		v, err := t.expr(args[1])
		t.globals = saved
		if err != nil || !v.isConst() {
			return nil // sizes that are not constant are ignored
		}
		return t.env.Symbol(name, []string{args[0], strconv.FormatInt(v.v, 10)})
	case ".comm", ".lcomm":
		args := splitTop(rest)
		if len(args) < 2 {
			return fmt.Errorf("bad %s", name)
		}
		out := []string{strings.TrimSpace(args[0])}
		for _, a := range args[1:] {
			v, err := t.constExpr(a)
			if err != nil {
				return err
			}
			out = append(out, strconv.FormatInt(v, 10))
		}
		if t.env.Symbol == nil {
			return fmt.Errorf("%s is not allowed here", name)
		}
		return t.env.Symbol(name, out)
	case ".globl", ".global", ".weak", ".hidden", ".local", ".type", ".protected", ".internal":
		if t.env.Symbol != nil {
			return t.env.Symbol(name, splitTop(rest))
		}
		return nil
	case ".att_syntax", ".code64", ".file", ".loc", ".ident", ".cfi_startproc", ".cfi_endproc",
		".cfi_def_cfa_offset", ".cfi_offset", ".cfi_def_cfa_register", ".cfi_def_cfa", ".cfi_adjust_cfa_offset",
		".cfi_rel_offset", ".cfi_restore", ".cfi_remember_state", ".cfi_restore_state", ".cfi_signal_frame",
		".cfi_undefined", ".cfi_same_value", ".cfi_register", ".cfi_escape", ".cfi_sections", ".cfi_personality",
		".cfi_lsda", ".nops", ".symver", ".note", ".addrsig", ".addrsig_sym":
		if name == ".nops" {
			n, err := t.constExpr(rest)
			if err != nil {
				return err
			}
			for i := int64(0); i < n; i++ {
				t.a.b(0x90)
			}
		}
		if name == ".symver" {
			return fmt.Errorf(".symver is not supported")
		}
		return nil
	case ".intel_syntax":
		return fmt.Errorf("Intel syntax is not supported")
	}
	return fmt.Errorf("unknown directive '%s'", name)
}

func (t *textAsm) switchSection(name, flags string, push bool) error {
	if flags == "" {
		flags = t.secFlags[name]
	}
	a, err := t.env.Section(name, flags)
	if err != nil {
		return err
	}
	if push {
		t.stack = append(t.stack, secEntry{t.secName, t.a})
	}
	t.prev = secEntry{t.secName, t.a}
	t.secName, t.a = name, a
	return nil
}

func (t *textAsm) constExpr(s string) (int64, error) {
	v, err := t.expr(s)
	if err != nil {
		return 0, err
	}
	if !v.isConst() {
		return 0, fmt.Errorf("constant expression expected")
	}
	return v.v, nil
}

func (t *textAsm) dataDir(rest string, n int) error {
	for _, part := range splitTop(rest) {
		v, err := t.expr(part)
		if err != nil {
			return err
		}
		if err := t.emitImm(v, n, false); err != nil {
			return err
		}
	}
	return nil
}

func unquoteAsm(s string) (string, error) {
	s = strings.TrimSpace(s)
	if len(s) < 2 || s[0] != '"' || s[len(s)-1] != '"' {
		return "", fmt.Errorf("string expected")
	}
	s = s[1 : len(s)-1]
	var b strings.Builder
	for i := 0; i < len(s); i++ {
		c := s[i]
		if c != '\\' || i+1 >= len(s) {
			b.WriteByte(c)
			continue
		}
		i++
		c = s[i]
		switch {
		case c >= '0' && c <= '7':
			v := 0
			for k := 0; k < 3 && i < len(s) && s[i] >= '0' && s[i] <= '7'; k++ {
				v = v*8 + int(s[i]-'0')
				i++
			}
			i--
			b.WriteByte(byte(v))
		case c == 'x':
			v := 0
			i++
			for i < len(s) && strings.IndexByte("0123456789abcdefABCDEF", s[i]) >= 0 {
				d, _ := strconv.ParseUint(s[i:i+1], 16, 8)
				v = v*16 + int(d)
				i++
			}
			i--
			b.WriteByte(byte(v))
		default:
			b.WriteByte(unescape(c))
		}
	}
	return b.String(), nil
}

func isSymbolName(s string) bool {
	if s == "" || !isIdentChar(s[0], true) {
		return false
	}
	for i := 1; i < len(s); i++ {
		if !isIdentChar(s[i], false) {
			return false
		}
	}
	return true
}

// assign handles .set name, expr: a constant or an alias of a label
func (t *textAsm) assign(name, expr string) error {
	v, err := t.expr(expr)
	if err != nil {
		return err
	}
	if v.sym != "" && v.rel == "" && t.globals[v.sym] {
		// alias of a global label of this text
		saved := t.globals[v.sym]
		delete(t.globals, v.sym)
		lv := t.symbolRef(v.sym)
		t.globals[v.sym] = saved
		if lv.lbl != nil {
			lv.v += v.v
			v = lv
		}
	}
	switch {
	case v.isConst():
		if t.env.Consts == nil {
			t.env.Consts = map[string]int64{}
		}
		t.env.Consts[name] = v.v
	case v.lbl != nil && v.minus == nil:
		if old := t.env.Labels[name]; old != nil && old.Defined {
			return fmt.Errorf("symbol '%s' is already defined", name)
		}
		if old := t.env.Labels[name]; old != nil {
			// referenced before: make the earlier label an alias too
			return fmt.Errorf("symbol '%s' is used before its .set", name)
		}
		t.env.Labels[name] = &TextLabel{A: v.lbl.a, L: v.lbl.l, Defined: true, Off: v.v}
	default:
		return fmt.Errorf("unsupported value for symbol '%s'", name)
	}
	return nil
}

// secSim follows the section directives in the pre-pass
type secSim struct {
	cur, prev string
	stack     []string
}

func (s *secSim) apply(stmt string, flags map[string]string) {
	if stmt == "" || stmt[0] != '.' {
		return
	}
	word, rest := splitWord(stmt)
	word = strings.ToLower(word)
	switch word {
	case ".text", ".data", ".bss":
		s.prev, s.cur = s.cur, word
	case ".section", ".pushsection":
		args := splitTop(rest)
		if len(args) == 0 {
			return
		}
		name := strings.Trim(args[0], "\"")
		if len(args) > 1 && flags[name] == "" {
			flags[name] = strings.Trim(args[1], "\"")
		}
		if word == ".pushsection" {
			s.stack = append(s.stack, s.cur)
		}
		s.prev, s.cur = s.cur, name
	case ".popsection":
		if len(s.stack) > 0 {
			s.prev, s.cur = s.cur, s.stack[len(s.stack)-1]
			s.stack = s.stack[:len(s.stack)-1]
		}
	case ".previous":
		s.prev, s.cur = s.cur, s.prev
	}
}
