package x64

import (
	"encoding/binary"
	"fmt"
	"os"

	"github.com/ipoluianov/altc/cc"
	"github.com/ipoluianov/altc/elf"
)

// DWARF 4 debug information (-g): the line table of the code, the
// functions, and the types and variables gdb needs to show values.

type lineRec struct {
	pos  int
	file *cc.File
	line int32
}

// dbgFunc is a function described in .debug_info
type dbgFunc struct {
	ob         *cc.Obj
	start, end int
	frame      bool     // its locals are in the frame at their offsets (no register allocation)
	ir         *irDebug // locations of the variables of an IR-compiled function
}

// SetLine records that the code from here on comes from line of file
func (a *Asm) SetLine(f *cc.File, line int32) {
	if f == nil || line <= 0 {
		return
	}
	if n := len(a.lines); n > 0 {
		last := &a.lines[n-1]
		if last.file == f && last.line == line {
			return
		}
		if last.pos == len(a.Code) {
			last.file, last.line = f, line
			return
		}
	}
	a.lines = append(a.lines, lineRec{len(a.Code), f, line})
	if a.Listing != nil {
		// the same information for GNU as
		if a.listFiles == nil {
			a.listFiles = map[*cc.File]int{}
		}
		n, ok := a.listFiles[f]
		if !ok {
			n = len(a.listFiles) + 1
			a.listFiles[f] = n
			fmt.Fprintf(a.Listing, "\t.file %d %q\n", n, f.Name)
		}
		fmt.Fprintf(a.Listing, "\t.loc %d %d\n", n, line)
	}
}

// lineAt records the source position of tok for the code that follows
func (g *gen) lineAt(tok *cc.Token) {
	if g.debug && tok != nil {
		f, l := tok.SourcePos()
		g.a.SetLine(f, l)
	}
}

// DWARF constants
const (
	dwTagArrayType       = 0x01
	dwTagEnumType        = 0x04
	dwTagMember          = 0x0d
	dwTagPointerType     = 0x0f
	dwTagCompileUnit     = 0x11
	dwTagStructType      = 0x13
	dwTagSubroutineType  = 0x15
	dwTagTypedef         = 0x16
	dwTagUnionType       = 0x17
	dwTagFormalParameter = 0x05
	dwTagSubrangeType    = 0x21
	dwTagBaseType        = 0x24
	dwTagConstType       = 0x26
	dwTagEnumerator      = 0x28
	dwTagSubprogram      = 0x2e
	dwTagVariable        = 0x34
	dwTagVolatileType    = 0x35
	dwTagInlinedSub      = 0x1d

	dwFormAddr      = 0x01
	dwFormData1     = 0x0b
	dwFormData2     = 0x05
	dwFormData8     = 0x07
	dwFormSdata     = 0x0d
	dwFormString    = 0x08
	dwFormUdata     = 0x0f
	dwFormRef4      = 0x13
	dwFormFlag      = 0x0c
	dwFormSecOffset = 0x17
	dwFormExprloc   = 0x18
	dwFormFlagPres  = 0x19

	dwAtSibling      = 0x01
	dwAtLocation     = 0x02
	dwAtName         = 0x03
	dwAtByteSize     = 0x0b
	dwAtStmtList     = 0x10
	dwAtLowPC        = 0x11
	dwAtHighPC       = 0x12
	dwAtLanguage     = 0x13
	dwAtCompDir      = 0x1b
	dwAtConstValue   = 0x1c
	dwAtUpperBound   = 0x2f
	dwAtProducer     = 0x25
	dwAtPrototyped   = 0x27
	dwAtBitSize      = 0x0d
	dwAtDataBitOff   = 0x6b
	dwAtMemberLoc    = 0x38
	dwAtDeclFile     = 0x3a
	dwAtDeclLine     = 0x3b
	dwAtDeclaration  = 0x3c
	dwAtEncoding     = 0x3e
	dwAtExternal     = 0x3f
	dwAtFrameBase    = 0x40
	dwAtType         = 0x49
	dwAtCount        = 0x37
	dwAtTypeEncoding = 0x3e
	dwAtInline       = 0x20
	dwAtAbstractOrig = 0x31
	dwAtRanges       = 0x55
	dwAtCallFile     = 0x58
	dwAtCallLine     = 0x59
)

// debugSections builds .debug_abbrev, .debug_info and .debug_line
func (g *gen) debugSections(text *elf.Section, syms map[string]*elf.Symbol) {
	if !g.debug {
		return
	}
	o := g.obj
	le := binary.LittleEndian
	secSym := func(s *elf.Section) *elf.Symbol {
		for _, x := range o.Symbols {
			if x.Type == elf.SymSection && x.Section == s {
				return x
			}
		}
		return o.AddLocal(&elf.Symbol{Bind: elf.BindLocal, Type: elf.SymSection, Section: s})
	}
	abbrevSec := o.AddSection(".debug_abbrev", elf.SecOther, 1)
	infoSec := o.AddSection(".debug_info", elf.SecOther, 1)
	lineSec := o.AddSection(".debug_line", elf.SecOther, 1)
	textSym := secSym(text)

	// the files, numbered from 1
	fileIdx := map[*cc.File]int{}
	var files []*cc.File
	fileOf := func(f *cc.File) int {
		if f == nil {
			return 0
		}
		if i, ok := fileIdx[f]; ok {
			return i
		}
		files = append(files, f)
		fileIdx[f] = len(files)
		return len(files)
	}
	for _, r := range g.a.lines {
		fileOf(r.file)
	}
	for _, fn := range g.dbgFuncs {
		if fn.ob.Tok != nil {
			f, _ := fn.ob.Tok.SourcePos()
			fileOf(f)
		}
	}

	// .debug_line
	var names []string
	for _, f := range files {
		names = append(names, f.Name)
	}
	seq := lineSeq{sym: textSym, end: len(g.a.Code)}
	for _, r := range g.a.lines {
		seq.rows = append(seq.rows, lineRow{r.pos, fileIdx[r.file], int(r.line)})
	}
	lineSec.Data, lineSec.Relocs = lineProgram(names, []lineSeq{seq})

	// .debug_info
	d := &dwarfInfo{g: g, fileOf: fileOf}
	cwd, _ := os.Getwd()
	b := d.buf
	b = append(b, 0, 0, 0, 0)
	b = le.AppendUint16(b, 4)
	infoSec.Relocs = append(infoSec.Relocs, elf.Reloc{Offset: int64(len(b)), Type: 10, Sym: secSym(abbrevSec)}) // R_X86_64_32
	b = append(b, 0, 0, 0, 0, 8)
	b = appendULEB(b, abbrevCU)
	b = append(append(b, "AltC"...), 0)
	b = append(b, 0x1d) // DW_LANG_C11
	b = append(append(b, o.Name...), 0)
	b = append(append(b, cwd...), 0)
	infoSec.Relocs = append(infoSec.Relocs, elf.Reloc{Offset: int64(len(b)), Type: elf.R_X86_64_64, Sym: textSym})
	b = append(b, 0, 0, 0, 0, 0, 0, 0, 0)
	b = le.AppendUint64(b, uint64(len(g.a.Code)))
	infoSec.Relocs = append(infoSec.Relocs, elf.Reloc{Offset: int64(len(b)), Type: 10, Sym: secSym(lineSec)})
	b = append(b, 0, 0, 0, 0)
	d.buf = b
	d.relocs = &infoSec.Relocs
	d.textSym = textSym
	d.syms = syms
	d.unit(g)
	d.buf = append(d.buf, 0) // end of the unit's children
	le.PutUint32(d.buf, uint32(len(d.buf)-4))
	d.resolveRefs()
	for _, x := range []struct {
		name string
		data []byte
		refs []int
	}{{".debug_loc", d.loc, d.locRefs}, {".debug_ranges", d.ranges, d.rangeRefs}} {
		if len(x.refs) == 0 {
			continue
		}
		sec := o.AddSection(x.name, elf.SecOther, 1)
		sec.Data = x.data
		sym := secSym(sec)
		for _, at := range x.refs {
			infoSec.Relocs = append(infoSec.Relocs, elf.Reloc{Offset: int64(at), Type: 10, Sym: sym,
				Addend: int64(le.Uint32(d.buf[at:]))}) // R_X86_64_32
			le.PutUint32(d.buf[at:], 0)
		}
	}
	infoSec.Data = d.buf
	abbrevSec.Data = dwarfAbbrevs()
}

const (
	abbrevCU = iota + 1
	abbrevSubprogram
	abbrevSubprogramVars
	abbrevVariable
	abbrevParam
	abbrevGlobal
	abbrevBaseType
	abbrevPointer
	abbrevVoidPointer
	abbrevConst
	abbrevVolatile
	abbrevTypedef
	abbrevStruct
	abbrevStructDecl
	abbrevUnion
	abbrevMember
	abbrevBitMember
	abbrevArray
	abbrevSubrange
	abbrevSubrangeUnknown
	abbrevEnum
	abbrevEnumerator
	abbrevFuncType
	abbrevFuncTypeParam
	abbrevUnspecified
	abbrevVariableLoc // location lists
	abbrevParamLoc
	abbrevVariableNoLoc // optimized out
	abbrevParamNoLoc
	abbrevInlined // inlined calls
	abbrevInlinedLeaf
	abbrevAbstract // the abstract instance of an inlined function
)

func dwarfAbbrevs() []byte {
	var b []byte
	add := func(code int, tag int, children bool, attrs ...int) {
		b = appendULEB(b, uint64(code))
		b = appendULEB(b, uint64(tag))
		if children {
			b = append(b, 1)
		} else {
			b = append(b, 0)
		}
		for _, a := range attrs {
			b = appendULEB(b, uint64(a))
		}
		b = append(b, 0, 0)
	}
	add(abbrevCU, dwTagCompileUnit, true, dwAtProducer, dwFormString, dwAtLanguage, dwFormData1, dwAtName, dwFormString,
		dwAtCompDir, dwFormString, dwAtLowPC, dwFormAddr, dwAtHighPC, dwFormData8, dwAtStmtList, dwFormSecOffset)
	sub := []int{dwAtExternal, dwFormFlag, dwAtName, dwFormString, dwAtDeclFile, dwFormUdata, dwAtDeclLine, dwFormUdata,
		dwAtPrototyped, dwFormFlag, dwAtType, dwFormRef4, dwAtLowPC, dwFormAddr, dwAtHighPC, dwFormData8, dwAtFrameBase, dwFormExprloc}
	add(abbrevSubprogram, dwTagSubprogram, false, sub...)
	add(abbrevSubprogramVars, dwTagSubprogram, true, sub...)
	add(abbrevVariable, dwTagVariable, false, dwAtName, dwFormString, dwAtDeclFile, dwFormUdata, dwAtDeclLine, dwFormUdata,
		dwAtType, dwFormRef4, dwAtLocation, dwFormExprloc)
	add(abbrevParam, dwTagFormalParameter, false, dwAtName, dwFormString, dwAtDeclFile, dwFormUdata, dwAtDeclLine, dwFormUdata,
		dwAtType, dwFormRef4, dwAtLocation, dwFormExprloc)
	add(abbrevGlobal, dwTagVariable, false, dwAtName, dwFormString, dwAtDeclFile, dwFormUdata, dwAtDeclLine, dwFormUdata,
		dwAtType, dwFormRef4, dwAtExternal, dwFormFlag, dwAtLocation, dwFormExprloc)
	add(abbrevBaseType, dwTagBaseType, false, dwAtByteSize, dwFormData1, dwAtEncoding, dwFormData1, dwAtName, dwFormString)
	add(abbrevPointer, dwTagPointerType, false, dwAtByteSize, dwFormData1, dwAtType, dwFormRef4)
	add(abbrevVoidPointer, dwTagPointerType, false, dwAtByteSize, dwFormData1)
	add(abbrevConst, dwTagConstType, false, dwAtType, dwFormRef4)
	add(abbrevVolatile, dwTagVolatileType, false, dwAtType, dwFormRef4)
	add(abbrevTypedef, dwTagTypedef, false, dwAtName, dwFormString, dwAtType, dwFormRef4)
	add(abbrevStruct, dwTagStructType, true, dwAtName, dwFormString, dwAtByteSize, dwFormUdata)
	add(abbrevStructDecl, dwTagStructType, false, dwAtName, dwFormString, dwAtDeclaration, dwFormFlagPres)
	add(abbrevUnion, dwTagUnionType, true, dwAtName, dwFormString, dwAtByteSize, dwFormUdata)
	add(abbrevMember, dwTagMember, false, dwAtName, dwFormString, dwAtType, dwFormRef4, dwAtMemberLoc, dwFormUdata)
	add(abbrevBitMember, dwTagMember, false, dwAtName, dwFormString, dwAtType, dwFormRef4, dwAtByteSize, dwFormUdata,
		dwAtBitSize, dwFormUdata, dwAtDataBitOff, dwFormUdata)
	add(abbrevArray, dwTagArrayType, true, dwAtType, dwFormRef4)
	add(abbrevSubrange, dwTagSubrangeType, false, dwAtCount, dwFormUdata)
	add(abbrevSubrangeUnknown, dwTagSubrangeType, false)
	add(abbrevEnum, dwTagEnumType, true, dwAtName, dwFormString, dwAtByteSize, dwFormData1, dwAtType, dwFormRef4)
	add(abbrevEnumerator, dwTagEnumerator, false, dwAtName, dwFormString, dwAtConstValue, dwFormSdata)
	add(abbrevFuncType, dwTagSubroutineType, true, dwAtPrototyped, dwFormFlag, dwAtType, dwFormRef4)
	add(abbrevFuncTypeParam, dwTagFormalParameter, false, dwAtType, dwFormRef4)
	add(abbrevUnspecified, 0x3b, false, dwAtName, dwFormString) // DW_TAG_unspecified_type
	varAttrs := []int{dwAtName, dwFormString, dwAtDeclFile, dwFormUdata, dwAtDeclLine, dwFormUdata, dwAtType, dwFormRef4}
	add(abbrevVariableLoc, dwTagVariable, false, append(varAttrs, dwAtLocation, dwFormSecOffset)...)
	add(abbrevParamLoc, dwTagFormalParameter, false, append(varAttrs, dwAtLocation, dwFormSecOffset)...)
	add(abbrevVariableNoLoc, dwTagVariable, false, varAttrs...)
	add(abbrevParamNoLoc, dwTagFormalParameter, false, varAttrs...)
	inl := []int{dwAtAbstractOrig, dwFormRef4, dwAtRanges, dwFormSecOffset, dwAtCallFile, dwFormUdata, dwAtCallLine, dwFormUdata}
	add(abbrevInlined, dwTagInlinedSub, true, inl...)
	add(abbrevInlinedLeaf, dwTagInlinedSub, false, inl...)
	add(abbrevAbstract, dwTagSubprogram, false, dwAtName, dwFormString, dwAtDeclFile, dwFormUdata, dwAtDeclLine, dwFormUdata,
		dwAtPrototyped, dwFormFlag, dwAtType, dwFormRef4, dwAtInline, dwFormData1)
	return append(b, 0)
}

// dwarfInfo writes the DIEs of a unit; type references are resolved at
// the end, after the type DIEs
type dwarfInfo struct {
	g       *gen
	buf     []byte
	relocs  *[]elf.Reloc
	textSym *elf.Symbol
	syms    map[string]*elf.Symbol
	fileOf  func(*cc.File) int
	keys    map[string]int // DIE offset of each type (by its structure)
	pending []*cc.Type
	pkeys   map[string]bool
	refs    []dwRef
	recIDs  map[*cc.Record]int

	loc       []byte // .debug_loc
	ranges    []byte // .debug_ranges
	locRefs   []int  // offsets in buf of 4-byte references into .debug_loc
	rangeRefs []int  // ... into .debug_ranges
	inlined   []*cc.Obj
	inlSeen   map[*cc.Obj]bool
}

type dwRef struct {
	at  int
	key string
}

func (d *dwarfInfo) str(s string) { d.buf = append(append(d.buf, s...), 0) }

func (d *dwarfInfo) uleb(v uint64) { d.buf = appendULEB(d.buf, v) }

// typeKey identifies a type by its structure
func (d *dwarfInfo) typeKey(t *cc.Type) string {
	if t == nil {
		return "v"
	}
	q := ""
	if t.Const {
		q += "c"
	}
	if t.Volatile {
		q += "V"
	}
	switch t.Kind {
	case cc.TyPtr:
		return q + "*" + d.typeKey(t.Base)
	case cc.TyArray, cc.TyVLA, cc.TyVector:
		n := t.ArrayLen
		if t.Kind == cc.TyVLA {
			n = -1
		}
		return q + "[" + itoa(n) + "]" + d.typeKey(t.Base)
	case cc.TyStruct, cc.TyUnion:
		return q + "s" + itoa(int64(d.recID(t.Rec)))
	case cc.TyFunc:
		k := q + "f(" + d.typeKey(t.Ret)
		for _, p := range t.Params {
			k += "," + d.typeKey(p)
		}
		return k + ")"
	case cc.TyEnum:
		return q + "e" + itoa(t.Size())
	case cc.TyComplex:
		return q + "C" + d.typeKey(t.Base)
	}
	u := "s"
	if t.Unsigned {
		u = "u"
	}
	if t.Kind == cc.TyChar && t.Plain {
		u = "p"
	}
	return q + "b" + itoa(int64(t.Kind)) + u
}

func (d *dwarfInfo) recID(r *cc.Record) int {
	if id, ok := d.recIDs[r]; ok {
		return id
	}
	d.recIDs[r] = len(d.recIDs) + 1
	return d.recIDs[r]
}

func itoa(v int64) string {
	if v < 0 {
		return "-" + itoa(-v)
	}
	if v < 10 {
		return string(rune('0' + v))
	}
	return itoa(v/10) + string(rune('0'+v%10))
}

// ref writes a reference to the DIE of t
func (d *dwarfInfo) ref(t *cc.Type) {
	k := d.typeKey(t)
	if !d.pkeys[k] {
		d.pkeys[k] = true
		d.pending = append(d.pending, t)
	}
	d.refs = append(d.refs, dwRef{len(d.buf), k})
	d.buf = append(d.buf, 0, 0, 0, 0)
}

// addr writes an address relocated against sym + addend
func (d *dwarfInfo) addr(sym *elf.Symbol, addend int64) {
	*d.relocs = append(*d.relocs, elf.Reloc{Offset: int64(len(d.buf)), Type: elf.R_X86_64_64, Sym: sym, Addend: addend})
	d.buf = append(d.buf, 0, 0, 0, 0, 0, 0, 0, 0)
}

func (d *dwarfInfo) declPos(tok *cc.Token) {
	var f *cc.File
	var l int32
	if tok != nil {
		f, l = tok.SourcePos()
	}
	d.uleb(uint64(d.fileOf(f)))
	d.uleb(uint64(max(l, 0)))
}

// unit writes the functions, the global variables and the types
func (d *dwarfInfo) unit(g *gen) {
	d.keys, d.pkeys, d.recIDs = map[string]int{}, map[string]bool{}, map[*cc.Record]int{}
	d.inlSeen = map[*cc.Obj]bool{}
	for _, fn := range g.dbgFuncs {
		ob := fn.ob
		if fn.ir != nil {
			d.irSubprogram(fn)
			continue
		}
		var vars []*cc.Obj
		if fn.frame {
			for _, v := range append(append([]*cc.Obj(nil), ob.Params...), ob.Locals...) {
				if v.Name != "" && v.Ty != nil && v.Ty.Kind != cc.TyVLA && v.RegName == "" && v.Tok != nil {
					vars = append(vars, v)
				}
			}
		}
		code := abbrevSubprogram
		if len(vars) > 0 {
			code = abbrevSubprogramVars
		}
		d.uleb(uint64(code))
		if ob.IsStatic {
			d.buf = append(d.buf, 0)
		} else {
			d.buf = append(d.buf, 1)
		}
		d.str(ob.Name)
		d.declPos(ob.Tok)
		d.buf = append(d.buf, 1) // prototyped
		d.ref(ob.Ty.Ret)
		d.addr(d.textSym, int64(fn.start))
		d.buf = binary.LittleEndian.AppendUint64(d.buf, uint64(fn.end-fn.start))
		d.buf = append(d.buf, 1, 0x9c) // frame base: DW_OP_call_frame_cfa
		if len(vars) == 0 {
			continue
		}
		params := map[*cc.Obj]bool{}
		for _, p := range ob.Params {
			params[p] = true
		}
		seen := map[*cc.Obj]bool{}
		for _, v := range vars {
			if seen[v] {
				continue
			}
			seen[v] = true
			if params[v] {
				d.uleb(abbrevParam)
			} else {
				d.uleb(abbrevVariable)
			}
			d.str(v.Name)
			d.declPos(v.Tok)
			d.ref(v.Ty)
			// rbp + Offset = CFA - 16 + Offset
			loc := appendSLEB([]byte{0x91}, v.Offset-16) // DW_OP_fbreg
			d.uleb(uint64(len(loc)))
			d.buf = append(d.buf, loc...)
		}
		d.buf = append(d.buf, 0)
	}
	// the abstract instances of inlined functions
	for _, fn := range d.inlined {
		d.keys["inl:"+fn.AsmName] = len(d.buf)
		d.uleb(abbrevAbstract)
		d.str(fn.Name)
		d.declPos(fn.Tok)
		d.buf = append(d.buf, 1)
		d.ref(fn.Ty.Ret)
		d.buf = append(d.buf, 3) // DW_INL_declared_inlined
	}
	// global variables
	for _, ob := range g.prog.Globals {
		if ob.IsFunction || g.defined[ob.AsmName] != ob || ob.IsTLS || ob.IsLiteral || ob.Name == "" || ob.Ty == nil {
			continue
		}
		s := d.syms[ob.AsmName]
		if s == nil || s.Section == nil && s.Type != elf.SymCommon {
			continue
		}
		d.uleb(abbrevGlobal)
		d.str(ob.Name)
		d.declPos(ob.Tok)
		d.ref(ob.Ty)
		if ob.IsStatic {
			d.buf = append(d.buf, 0)
		} else {
			d.buf = append(d.buf, 1)
		}
		d.buf = append(d.buf, 9, 0x03) // DW_OP_addr
		d.addr(s, 0)
	}
	for len(d.pending) > 0 {
		t := d.pending[0]
		d.pending = d.pending[1:]
		d.typeDIE(t)
	}
}

// unqualified copies t without its qualifiers
func unqualified(t *cc.Type) *cc.Type {
	c := *t
	c.Const, c.Volatile, c.Restrict, c.Atomic = false, false, false, false
	return &c
}

func (d *dwarfInfo) typeDIE(t *cc.Type) {
	d.keys[d.typeKey(t)] = len(d.buf)
	if t == nil || t.Kind == cc.TyVoid && !t.Const && !t.Volatile {
		d.uleb(abbrevUnspecified)
		d.str("void")
		return
	}
	switch {
	case t.Const:
		c := unqualified(t)
		c.Volatile = t.Volatile
		d.uleb(abbrevConst)
		d.ref(c)
		return
	case t.Volatile:
		d.uleb(abbrevVolatile)
		d.ref(unqualified(t))
		return
	}
	base := func(enc byte, name string) {
		d.uleb(abbrevBaseType)
		d.buf = append(d.buf, byte(t.Size()), enc)
		d.str(name)
	}
	intName := func(s string) string {
		if t.Unsigned {
			return "unsigned " + s
		}
		return s
	}
	intEnc := byte(0x05)
	if t.Unsigned {
		intEnc = 0x07
	}
	switch t.Kind {
	case cc.TyBool:
		base(0x02, "_Bool")
	case cc.TyChar:
		switch {
		case t.Plain:
			base(0x06, "char")
		case t.Unsigned:
			base(0x08, "unsigned char")
		default:
			base(0x06, "signed char")
		}
	case cc.TyShort:
		base(intEnc, intName("short"))
	case cc.TyInt:
		base(intEnc, intName("int"))
	case cc.TyLong:
		base(intEnc, intName("long"))
	case cc.TyLongLong:
		base(intEnc, intName("long long"))
	case cc.TyInt128:
		base(intEnc, intName("__int128"))
	case cc.TyEnum:
		enc := byte(0x05)
		if t.Unsigned {
			enc = 0x07
		}
		name := "int"
		if t.Size() == 8 {
			name = "long"
		}
		base(enc, name)
	case cc.TyFloat:
		base(0x04, "float")
	case cc.TyDouble:
		base(0x04, "double")
	case cc.TyLongDouble:
		base(0x04, "long double")
	case cc.TyComplex:
		base(0x03, "complex "+map[cc.TypeKind]string{cc.TyFloat: "float", cc.TyDouble: "double", cc.TyLongDouble: "long double"}[t.Base.Kind])
	case cc.TyNullptr:
		d.uleb(abbrevVoidPointer)
		d.buf = append(d.buf, 8)
	case cc.TyPtr:
		if t.Base == nil || t.Base.Kind == cc.TyVoid && !t.Base.Const && !t.Base.Volatile {
			d.uleb(abbrevVoidPointer)
			d.buf = append(d.buf, 8)
			return
		}
		d.uleb(abbrevPointer)
		d.buf = append(d.buf, 8)
		d.ref(t.Base)
	case cc.TyArray, cc.TyVLA, cc.TyVector:
		d.uleb(abbrevArray)
		d.ref(t.Base)
		if t.Kind != cc.TyVLA && t.ArrayLen >= 0 {
			d.uleb(abbrevSubrange)
			d.uleb(uint64(t.ArrayLen))
		} else {
			d.uleb(abbrevSubrangeUnknown)
		}
		d.buf = append(d.buf, 0)
	case cc.TyStruct, cc.TyUnion:
		r := t.Rec
		if r == nil || !r.Complete {
			d.uleb(abbrevStructDecl)
			if r != nil {
				d.str(r.Tag)
			} else {
				d.str("")
			}
			return
		}
		if t.Kind == cc.TyUnion {
			d.uleb(abbrevUnion)
		} else {
			d.uleb(abbrevStruct)
		}
		d.str(r.Tag)
		d.uleb(uint64(t.Size()))
		for _, m := range r.Members {
			if m.Bitfield {
				d.uleb(abbrevBitMember)
				d.str(m.Name)
				d.ref(m.Ty)
				d.uleb(uint64(m.Ty.Size()))
				d.uleb(uint64(m.BitWidth))
				d.uleb(uint64(m.Offset*8 + m.BitOffset))
				continue
			}
			d.uleb(abbrevMember)
			d.str(m.Name)
			d.ref(m.Ty)
			d.uleb(uint64(m.Offset))
		}
		d.buf = append(d.buf, 0)
	case cc.TyFunc:
		d.uleb(abbrevFuncType)
		if t.OldStyle {
			d.buf = append(d.buf, 0)
		} else {
			d.buf = append(d.buf, 1)
		}
		d.ref(t.Ret)
		for _, p := range t.Params {
			d.uleb(abbrevFuncTypeParam)
			d.ref(p)
		}
		d.buf = append(d.buf, 0)
	default:
		d.uleb(abbrevUnspecified)
		d.str("void")
	}
}

// resolveRefs fills the type references with the DIE offsets
func (d *dwarfInfo) resolveRefs() {
	for _, r := range d.refs {
		binary.LittleEndian.PutUint32(d.buf[r.at:], uint32(d.keys[r.key]))
	}
}

type lineRow struct{ pos, file, line int }

// lineSeq is the line information of the code of one section
type lineSeq struct {
	sym  *elf.Symbol // the section
	end  int
	rows []lineRow
}

// lineProgram encodes a DWARF 4 line table; files are numbered from 1
func lineProgram(files []string, seqs []lineSeq) ([]byte, []elf.Reloc) {
	le := binary.LittleEndian
	var relocs []elf.Reloc
	var ln []byte
	ln = append(ln, 0, 0, 0, 0) // unit length
	ln = le.AppendUint16(ln, 4) // version
	ln = append(ln, 0, 0, 0, 0) // header length
	hdrStart := len(ln)
	ln = append(ln, 1, 1, 1, 0xfb, 14, 13) // min instruction length, max ops, default is_stmt, line base -5, line range, opcode base
	ln = append(ln, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1)
	ln = append(ln, 0) // no include directories
	for _, f := range files {
		ln = append(ln, f...)
		ln = append(ln, 0, 0, 0, 0) // directory 0 (the compilation directory), no time, no size
	}
	ln = append(ln, 0)
	le.PutUint32(ln[6:], uint32(len(ln)-hdrStart))
	for _, sq := range seqs {
		if len(sq.rows) == 0 {
			continue
		}
		ln = append(ln, 0, 9, 2) // DW_LNE_set_address
		relocs = append(relocs, elf.Reloc{Offset: int64(len(ln)), Type: elf.R_X86_64_64, Sym: sq.sym, Addend: int64(sq.rows[0].pos)})
		ln = append(ln, 0, 0, 0, 0, 0, 0, 0, 0)
		pos, file, line := sq.rows[0].pos, 1, 1
		for _, r := range sq.rows {
			if r.file != file {
				ln = appendULEB(append(ln, 4), uint64(r.file)) // DW_LNS_set_file
				file = r.file
			}
			if d := r.pos - pos; d > 0 {
				ln = appendULEB(append(ln, 2), uint64(d)) // DW_LNS_advance_pc
				pos = r.pos
			}
			if d := r.line - line; d != 0 {
				ln = appendSLEB(append(ln, 3), int64(d)) // DW_LNS_advance_line
				line = r.line
			}
			ln = append(ln, 1) // DW_LNS_copy
		}
		if d := sq.end - pos; d > 0 {
			ln = appendULEB(append(ln, 2), uint64(d))
		}
		ln = append(ln, 0, 1, 1) // DW_LNE_end_sequence
	}
	le.PutUint32(ln, uint32(len(ln)-4))
	return ln, relocs
}

// asmLines turns the .loc directives of assembly text into the line
// table, in the .debug_line section the text refers to
func (g *gen) asmLines() {
	o := g.obj
	var seqs []lineSeq
	asms := []*Asm{g.a}
	for _, s := range g.asmSecs {
		asms = append(asms, s.a)
	}
	for _, a := range asms {
		if len(a.locs) == 0 || g.secOf[a] == nil {
			continue
		}
		sec := g.secOf[a]
		var sym *elf.Symbol
		for _, x := range o.Symbols {
			if x.Type == elf.SymSection && x.Section == sec {
				sym = x
			}
		}
		if sym == nil {
			sym = o.AddLocal(&elf.Symbol{Bind: elf.BindLocal, Type: elf.SymSection, Section: sec})
		}
		sq := lineSeq{sym: sym, end: len(a.Code)}
		for _, l := range a.locs {
			sq.rows = append(sq.rows, lineRow{l.pos, l.file, l.line})
		}
		seqs = append(seqs, sq)
	}
	if len(seqs) == 0 {
		return
	}
	n := 0
	for k := range g.asmFiles {
		n = max(n, k)
	}
	files := make([]string, n)
	for k, v := range g.asmFiles {
		if k >= 1 {
			files[k-1] = v
		}
	}
	data, relocs := lineProgram(files, seqs)
	var sec *elf.Section
	for _, s := range o.Sections {
		if s.Name == ".debug_line" {
			sec = s
		}
	}
	if sec == nil {
		sec = o.AddSection(".debug_line", elf.SecOther, 1)
	}
	// the text's .debug_line holds only the label of the table: the table
	// goes first
	for i := range sec.Relocs {
		sec.Relocs[i].Offset += int64(len(data))
	}
	sec.Data = append(data, sec.Data...)
	sec.Relocs = append(relocs, sec.Relocs...)
	for _, s := range o.Sections {
		if s.Name == ".debug_info" {
			return
		}
	}
	// no unit refers to the table: a minimal one for the code of the first
	// section, as GNU as makes
	le := binary.LittleEndian
	secSym := func(s *elf.Section) *elf.Symbol {
		for _, x := range o.Symbols {
			if x.Type == elf.SymSection && x.Section == s {
				return x
			}
		}
		return o.AddLocal(&elf.Symbol{Bind: elf.BindLocal, Type: elf.SymSection, Section: s})
	}
	abbrev := o.AddSection(".debug_abbrev", elf.SecOther, 1)
	abbrev.Data = []byte{1, dwTagCompileUnit, 0, dwAtStmtList, dwFormSecOffset, dwAtLowPC, dwFormAddr, dwAtHighPC, dwFormData8,
		dwAtName, dwFormString, dwAtProducer, dwFormString, 0, 0, 0}
	info := o.AddSection(".debug_info", elf.SecOther, 1)
	b := []byte{0, 0, 0, 0, 4, 0}
	info.Relocs = append(info.Relocs, elf.Reloc{Offset: int64(len(b)), Type: 10, Sym: secSym(abbrev)})
	b = append(b, 0, 0, 0, 0, 8, 1)
	info.Relocs = append(info.Relocs, elf.Reloc{Offset: int64(len(b)), Type: 10, Sym: secSym(sec)})
	b = append(b, 0, 0, 0, 0)
	info.Relocs = append(info.Relocs, elf.Reloc{Offset: int64(len(b)), Type: elf.R_X86_64_64, Sym: seqs[0].sym})
	b = append(b, 0, 0, 0, 0, 0, 0, 0, 0)
	b = le.AppendUint64(b, uint64(seqs[0].end))
	b = append(append(b, o.Name...), 0)
	b = append(append(b, "AltC assembler"...), 0)
	le.PutUint32(b, uint32(len(b)-4))
	info.Data = b
}

// lineMark saves the state of the line table
type lineMark struct {
	n    int
	last lineRec
}

func (a *Asm) lineMark() lineMark {
	m := lineMark{n: len(a.lines)}
	if m.n > 0 {
		m.last = a.lines[m.n-1]
	}
	return m
}

// lineRestore drops the line changes made since m (only when no code was
// emitted since)
func (a *Asm) lineRestore(m lineMark) {
	if len(a.lines) == m.n && (m.n == 0 || a.lines[m.n-1] == m.last) {
		return
	}
	a.lines = a.lines[:m.n]
	if m.n > 0 {
		a.lines[m.n-1] = m.last
	}
}
