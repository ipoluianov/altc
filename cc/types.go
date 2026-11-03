package cc

import (
	"fmt"
	"strings"
)

// TypeKind is the kind of a C type
type TypeKind uint8

const (
	TyVoid TypeKind = iota
	TyBool
	TyChar
	TyShort
	TyInt
	TyLong
	TyLongLong
	TyInt128
	TyFloat
	TyDouble
	TyLongDouble
	TyEnum
	TyPtr
	TyFunc
	TyArray
	TyVLA
	TyStruct
	TyUnion
	TyNullptr
	TyComplex // _Complex T: Base is T; stored like struct { T re, im; }
)

// Type is a C type. Qualified versions are copies that share Rec and Base.
type Type struct {
	Kind     TypeKind
	Unsigned bool
	Plain    bool // plain char (distinct from signed char)

	Const, Volatile, Restrict, Atomic bool

	size  int64
	align int64

	Base     *Type // pointer target, array element
	ArrayLen int64 // -1: unknown ([])

	// variable-length arrays
	VLALen  *Node
	VLASize *Obj

	Rec  *Record   // struct/union
	Enum *EnumInfo // enum

	// functions
	Ret        *Type
	Params     []*Type
	Variadic   bool
	OldStyle   bool     // declared without a prototype: f()
	ParamNames []*Token // names from the declarator (nil entries for abstract ones)
	ParamObjs  []*Obj   // parameter objects created while parsing the declarator

	Typedef string // name if this type came from a typedef (for messages)
	Origin  *Type  // the unqualified type this one was copied from
	AlignAs int64  // explicit alignment from _Alignas/aligned attribute on a typedef
	Packed  bool
}

// Record is the body of a struct or union, shared by its qualified copies
type Record struct {
	Tag      string
	Union    bool
	Members  []*Member
	Complete bool
	Size     int64
	Align    int64
	Flexible bool
	Packed   bool
	Tok      *Token
}

// Member of a struct or union
type Member struct {
	Name      string
	Ty        *Type
	Tok       *Token
	Offset    int64
	Align     int64
	Idx       int
	Bitfield  bool
	BitOffset int64
	BitWidth  int64
	Anon      bool // anonymous struct/union whose members are visible in the parent
}

// EnumInfo describes an enum type
type EnumInfo struct {
	Tag      string
	Complete bool
	Fixed    *Type // C23 fixed underlying type
	Under    *Type // computed underlying type
}

var (
	tyVoid    = &Type{Kind: TyVoid, size: 1, align: 1}
	tyBool    = &Type{Kind: TyBool, size: 1, align: 1, Unsigned: true}
	tyChar    = &Type{Kind: TyChar, size: 1, align: 1, Plain: true}
	tySChar   = &Type{Kind: TyChar, size: 1, align: 1}
	tyUChar   = &Type{Kind: TyChar, size: 1, align: 1, Unsigned: true}
	tyShort   = &Type{Kind: TyShort, size: 2, align: 2}
	tyUShort  = &Type{Kind: TyShort, size: 2, align: 2, Unsigned: true}
	tyInt     = &Type{Kind: TyInt, size: 4, align: 4}
	tyUInt    = &Type{Kind: TyInt, size: 4, align: 4, Unsigned: true}
	tyLong    = &Type{Kind: TyLong, size: 8, align: 8}
	tyULong   = &Type{Kind: TyLong, size: 8, align: 8, Unsigned: true}
	tyLLong   = &Type{Kind: TyLongLong, size: 8, align: 8}
	tyULLong  = &Type{Kind: TyLongLong, size: 8, align: 8, Unsigned: true}
	tyInt128  = &Type{Kind: TyInt128, size: 16, align: 16}
	tyUInt128 = &Type{Kind: TyInt128, size: 16, align: 16, Unsigned: true}
	tyFloat   = &Type{Kind: TyFloat, size: 4, align: 4}
	tyDouble  = &Type{Kind: TyDouble, size: 8, align: 8}
	tyLDouble = &Type{Kind: TyLongDouble, size: 16, align: 16}
	tyNullptr = &Type{Kind: TyNullptr, size: 8, align: 8}
)

// Size returns sizeof the type (-1 if incomplete)
func (t *Type) Size() int64 {
	switch t.Kind {
	case TyStruct, TyUnion, TyComplex:
		if !t.Rec.Complete {
			return -1
		}
		return t.Rec.Size
	case TyArray:
		if t.ArrayLen < 0 {
			return -1
		}
		bs := t.Base.Size()
		if bs < 0 {
			return -1
		}
		return bs * t.ArrayLen
	case TyEnum:
		if t.Enum.Under != nil {
			return t.Enum.Under.size
		}
		return 4
	case TyVLA:
		return 8
	}
	return t.size
}

// Align returns the alignment of the type
func (t *Type) Align() int64 {
	if t.AlignAs > 0 {
		return t.AlignAs
	}
	switch t.Kind {
	case TyStruct, TyUnion, TyComplex:
		if t.Rec.Align == 0 {
			return 1
		}
		return t.Rec.Align
	case TyArray, TyVLA:
		return t.Base.Align()
	case TyEnum:
		if t.Enum.Under != nil {
			return t.Enum.Under.align
		}
		return 4
	}
	return t.align
}

// IsInteger tells whether the type is an integer type (including bool, char, enum)
func (t *Type) IsInteger() bool {
	switch t.Kind {
	case TyBool, TyChar, TyShort, TyInt, TyLong, TyLongLong, TyInt128, TyEnum:
		return true
	}
	return false
}

func (t *Type) IsFlonum() bool {
	return t.Kind == TyFloat || t.Kind == TyDouble || t.Kind == TyLongDouble
}

func (t *Type) IsNumeric() bool { return t.IsInteger() || t.IsFlonum() }

func (t *Type) IsScalar() bool {
	return t.IsNumeric() || t.Kind == TyPtr || t.Kind == TyNullptr
}

// IsRecord tells whether the value is handled like a struct (complex
// numbers are stored and passed like struct { T re, im; })
func (t *Type) IsRecord() bool { return t.Kind == TyStruct || t.Kind == TyUnion || t.Kind == TyComplex }

// IsComplex tells whether the type is a complex floating type
func (t *Type) IsComplex() bool { return t.Kind == TyComplex }

var complexTypes = map[TypeKind]*Type{}

// complexOf returns the complex type of a real floating type
func complexOf(base *Type) *Type {
	if base.Kind == TyComplex {
		return base
	}
	if !base.IsFlonum() {
		base = tyDouble
	}
	if t, ok := complexTypes[base.Kind]; ok {
		return t
	}
	b := base.Unqual()
	sz := b.Size()
	r := &Record{Members: []*Member{
		{Name: "__re", Ty: b, Align: b.Align(), Idx: 0},
		{Name: "__im", Ty: b, Align: b.Align(), Idx: 1, Offset: sz},
	}, Size: 2 * sz, Align: b.Align(), Complete: true}
	t := &Type{Kind: TyComplex, Base: b, Rec: r}
	complexTypes[base.Kind] = t
	return t
}

func (t *Type) IsArray() bool { return t.Kind == TyArray || t.Kind == TyVLA }

// IsPtrLike tells whether the type decays to a pointer
func (t *Type) IsPtrLike() bool { return t.Kind == TyPtr || t.IsArray() }

// IsUnsigned for integer types (enums with an unsigned underlying type too)
func (t *Type) IsUnsigned() bool {
	if t.Kind == TyEnum {
		return t.Enum.Under != nil && t.Enum.Under.Unsigned
	}
	if t.Kind == TyPtr {
		return true
	}
	return t.Unsigned
}

func (t *Type) IsComplete() bool {
	if t.Kind == TyVoid {
		return false
	}
	if t.Kind == TyVLA {
		return true
	}
	return t.Size() >= 0
}

// IsQualified tells whether any qualifier is set
func (t *Type) IsQualified() bool { return t.Const || t.Volatile || t.Restrict || t.Atomic }

func (t *Type) clone() *Type {
	c := *t
	if t.Origin != nil {
		c.Origin = t.Origin
	} else {
		c.Origin = t
	}
	return &c
}

// Unqual returns the type without qualifiers
func (t *Type) Unqual() *Type {
	if !t.IsQualified() {
		return t
	}
	c := t.clone()
	c.Const, c.Volatile, c.Restrict, c.Atomic = false, false, false, false
	return c
}

// withQuals returns the type with qualifiers added
func (t *Type) withQuals(c, v, r, a bool) *Type {
	if (!c || t.Const) && (!v || t.Volatile) && (!r || t.Restrict) && (!a || t.Atomic) {
		return t
	}
	n := t.clone()
	n.Const = n.Const || c
	n.Volatile = n.Volatile || v
	n.Restrict = n.Restrict || r
	n.Atomic = n.Atomic || a
	return n
}

func pointerTo(base *Type) *Type {
	return &Type{Kind: TyPtr, size: 8, align: 8, Base: base, Unsigned: true}
}

func arrayOf(base *Type, n int64) *Type {
	return &Type{Kind: TyArray, Base: base, ArrayLen: n}
}

func vlaOf(base *Type, length *Node) *Type {
	return &Type{Kind: TyVLA, size: 8, align: 8, Base: base, VLALen: length}
}

func funcType(ret *Type) *Type {
	return &Type{Kind: TyFunc, size: 1, align: 1, Ret: ret}
}

func enumType() *Type {
	return &Type{Kind: TyEnum, Enum: &EnumInfo{}}
}

func recordType(union bool) *Type {
	k := TyStruct
	if union {
		k = TyUnion
	}
	return &Type{Kind: k, Rec: &Record{Union: union, Align: 1}}
}

// intRank orders the integer conversion ranks
func intRank(t *Type) int {
	switch t.Kind {
	case TyBool:
		return 1
	case TyChar:
		return 2
	case TyShort:
		return 3
	case TyInt:
		return 4
	case TyLong:
		return 5
	case TyLongLong:
		return 6
	case TyInt128:
		return 7
	case TyEnum:
		return intRank(t.Enum.underlying())
	}
	return 0
}

func (e *EnumInfo) underlying() *Type {
	if e.Under != nil {
		return e.Under
	}
	return tyUInt
}

// compatible implements C type compatibility (6.2.7)
func compatible(a, b *Type) bool {
	if a == b {
		return true
	}
	if a.Origin != nil && a.Origin == b || b.Origin != nil && b.Origin == a || a.Origin != nil && a.Origin == b.Origin {
		if a.Const == b.Const && a.Volatile == b.Volatile && a.Atomic == b.Atomic {
			return true
		}
	}
	if a.Const != b.Const || a.Volatile != b.Volatile || a.Atomic != b.Atomic {
		return false
	}
	if a.Kind == TyEnum && b.IsInteger() && b.Kind != TyEnum {
		return compatible(a.Enum.underlying().withQuals(a.Const, a.Volatile, false, false), b)
	}
	if b.Kind == TyEnum && a.IsInteger() && a.Kind != TyEnum {
		return compatible(a, b.Enum.underlying().withQuals(b.Const, b.Volatile, false, false))
	}
	if a.Kind != b.Kind {
		return false
	}
	switch a.Kind {
	case TyChar:
		return a.Unsigned == b.Unsigned && a.Plain == b.Plain
	case TyShort, TyInt, TyLong, TyLongLong, TyInt128:
		return a.Unsigned == b.Unsigned
	case TyBool, TyFloat, TyDouble, TyLongDouble, TyVoid, TyNullptr:
		return true
	case TyPtr:
		return compatible(a.Base, b.Base)
	case TyFunc:
		if !compatible(a.Ret, b.Ret) {
			return false
		}
		if a.OldStyle || b.OldStyle {
			return true
		}
		if a.Variadic != b.Variadic || len(a.Params) != len(b.Params) {
			return false
		}
		for i := range a.Params {
			if !compatible(a.Params[i].Unqual(), b.Params[i].Unqual()) {
				return false
			}
		}
		return true
	case TyArray:
		if !compatible(a.Base, b.Base) {
			return false
		}
		return a.ArrayLen < 0 || b.ArrayLen < 0 || a.ArrayLen == b.ArrayLen
	case TyVLA:
		return compatible(a.Base, b.Base)
	case TyStruct, TyUnion:
		return a.Rec == b.Rec
	case TyComplex:
		return a.Base.Kind == b.Base.Kind
	case TyEnum:
		return a.Enum == b.Enum
	}
	return false
}

// String renders the type in C syntax (for messages)
func (t *Type) String() string {
	return typeString(t, "")
}

func typeString(t *Type, inner string) string {
	q := ""
	if t.Const {
		q += "const "
	}
	if t.Volatile {
		q += "volatile "
	}
	if t.Typedef != "" && t.Kind != TyPtr && t.Kind != TyFunc && t.Kind != TyArray {
		return strings.TrimSpace(q + t.Typedef + " " + inner)
	}
	switch t.Kind {
	case TyPtr:
		s := "*"
		if t.Const {
			s += "const "
		}
		if t.Base.Kind == TyFunc || t.Base.Kind == TyArray {
			return typeString(t.Base, "("+s+inner+")")
		}
		return typeString(t.Base, s+inner)
	case TyArray:
		if t.ArrayLen < 0 {
			return typeString(t.Base, inner+"[]")
		}
		return typeString(t.Base, fmt.Sprintf("%s[%d]", inner, t.ArrayLen))
	case TyVLA:
		return typeString(t.Base, inner+"[*]")
	case TyFunc:
		var ps []string
		for _, p := range t.Params {
			ps = append(ps, p.String())
		}
		if t.Variadic {
			ps = append(ps, "...")
		}
		if len(ps) == 0 && !t.OldStyle {
			ps = append(ps, "void")
		}
		return typeString(t.Ret, inner+"("+strings.Join(ps, ", ")+")")
	}
	var base string
	switch t.Kind {
	case TyVoid:
		base = "void"
	case TyBool:
		base = "_Bool"
	case TyChar:
		switch {
		case t.Plain:
			base = "char"
		case t.Unsigned:
			base = "unsigned char"
		default:
			base = "signed char"
		}
	case TyShort:
		base = "short"
	case TyInt:
		base = "int"
	case TyLong:
		base = "long"
	case TyLongLong:
		base = "long long"
	case TyInt128:
		base = "__int128"
	case TyFloat:
		base = "float"
	case TyDouble:
		base = "double"
	case TyLongDouble:
		base = "long double"
	case TyComplex:
		base = "_Complex " + typeString(t.Base, "")
	case TyNullptr:
		base = "nullptr_t"
	case TyEnum:
		base = "enum " + t.Enum.Tag
	case TyStruct:
		base = "struct " + t.Rec.Tag
	case TyUnion:
		base = "union " + t.Rec.Tag
	}
	if t.Unsigned && t.Kind != TyChar && t.Kind != TyBool && t.Kind != TyPtr {
		base = "unsigned " + base
	}
	return strings.TrimSpace(q + base + " " + inner)
}

func alignTo(n, a int64) int64 {
	if a <= 1 {
		return n
	}
	return (n + a - 1) / a * a
}

// layoutRecord computes member offsets, the size and the alignment
// (System V x86-64 rules, GCC-compatible bit-fields). maxAlign is the
// #pragma pack value (0: none).
func layoutRecord(r *Record, maxAlign int64) {
	if r.Union {
		size := int64(0)
		align := int64(1)
		for _, m := range r.Members {
			al := m.Align
			if r.Packed {
				al = 1
			} else if maxAlign > 0 && al > maxAlign {
				al = maxAlign
			}
			m.Offset = 0
			if m.Bitfield {
				m.BitOffset = 0
				sz := (m.BitWidth + 7) / 8
				if sz > size {
					size = sz
				}
				if m.Name != "" && al > align {
					align = al
				}
				continue
			}
			if al > align {
				align = al
			}
			if s := m.Ty.Size(); s > size {
				size = s
			}
		}
		if r.Align > align {
			align = r.Align
		}
		r.Align = align
		r.Size = alignTo(size, align)
		r.Complete = true
		return
	}
	bits := int64(0)
	align := int64(1)
	for i, m := range r.Members {
		al := m.Align
		if r.Packed {
			al = 1
		} else if maxAlign > 0 && al > maxAlign {
			al = maxAlign
		}
		if m.Bitfield && m.BitWidth == 0 {
			sz := m.Ty.Size() * 8
			bits = alignTo(bits, sz)
			continue
		}
		if m.Bitfield {
			sz := m.Ty.Size()
			if r.Packed {
				m.Offset = bits / 8
				m.BitOffset = bits % 8
				// a packed bit-field may need a wider load; keep it inside its storage unit
				for m.BitOffset+m.BitWidth > sz*8 {
					m.Offset--
					m.BitOffset += 8
				}
				if m.Offset < 0 {
					m.Offset = 0
				}
			} else {
				unit := sz * 8
				if maxAlign > 0 && al < sz {
					unit = al * 8
				}
				if bits/unit != (bits+m.BitWidth-1)/unit {
					bits = alignTo(bits, unit)
				}
				m.Offset = (bits / 8) / sz * sz
				if maxAlign > 0 && al < sz {
					m.Offset = (bits / 8) / al * al
				}
				m.BitOffset = bits - m.Offset*8
			}
			bits += m.BitWidth
			if m.Name != "" && al > align {
				align = al
			}
			continue
		}
		bits = alignTo(bits, al*8)
		m.Offset = bits / 8
		sz := m.Ty.Size()
		if sz < 0 {
			// flexible array member
			if i == len(r.Members)-1 && m.Ty.Kind == TyArray {
				sz = 0
				r.Flexible = true
			} else {
				sz = 0
			}
		}
		bits += sz * 8
		if al > align {
			align = al
		}
	}
	if r.Align > align {
		align = r.Align
	}
	r.Align = align
	r.Size = alignTo((bits+7)/8, align)
	r.Complete = true
}

// findMember looks up a member by name, descending into anonymous members.
// It returns the access path (anonymous members first).
func findMember(r *Record, name string) []*Member {
	for _, m := range r.Members {
		if m.Name == name {
			return []*Member{m}
		}
		if m.Anon && m.Ty.IsRecord() {
			if p := findMember(m.Ty.Rec, name); p != nil {
				return append([]*Member{m}, p...)
			}
		}
	}
	return nil
}
