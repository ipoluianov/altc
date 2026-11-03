package cc

import (
	"math"
	"strings"
)

// libBuiltins are __builtin_X functions that simply call the library function X
var libBuiltins = map[string]string{}

func init() {
	for _, n := range strings.Fields(`memcpy memmove memset memcmp memchr strlen strcpy strncpy strcat strncat strcmp strncmp
		strchr strrchr strstr strdup strndup printf fprintf sprintf snprintf vprintf vfprintf vsprintf vsnprintf puts putchar
		fputs fputc abort exit _exit malloc calloc realloc free abs labs llabs fabs fabsf fabsl sqrt sqrtf sqrtl sin cos tan
		exp log log2 log10 pow floor ceil round trunc fmod floorf ceilf roundf truncf powf expf logf sinf cosf copysign
		copysignf copysignl fmin fmax fminf fmaxf lround lrint nearbyint rint isdigit isalpha isspace toupper tolower
		alloca bzero strspn strcspn strpbrk stpcpy index rindex frexp ldexp modf atan atan2 asin acos sinh cosh tanh
		hypot cbrt exp2 expm1 log1p fma scalbn`) {
		libBuiltins["__builtin_"+n] = n
	}
}

var builtinNames = map[string]bool{}

func init() {
	for _, n := range strings.Fields(`__builtin_va_start __builtin_va_end __builtin_va_arg __builtin_va_copy __builtin_offsetof
		__builtin_expect __builtin_expect_with_probability __builtin_assume_aligned __builtin_alloca __builtin_alloca_with_align
		__builtin_inf __builtin_inff __builtin_infl __builtin_huge_val __builtin_huge_valf __builtin_huge_vall
		__builtin_nan __builtin_nanf __builtin_nanl __builtin_nans __builtin_nansf __builtin_nansl
		__builtin_constant_p __builtin_types_compatible_p __builtin_choose_expr __builtin_unreachable __builtin_trap
		__builtin_bswap16 __builtin_bswap32 __builtin_bswap64 __builtin_clz __builtin_clzl __builtin_clzll
		__builtin_ctz __builtin_ctzl __builtin_ctzll __builtin_popcount __builtin_popcountl __builtin_popcountll
		__builtin_ffs __builtin_ffsl __builtin_ffsll __builtin_parity __builtin_parityl __builtin_parityll
		__builtin_clrsb __builtin_clrsbl __builtin_clrsbll
		__builtin_frame_address __builtin_return_address __builtin_prefetch __builtin_object_size
		__builtin_complex __builtin_creal __builtin_cimag __builtin_crealf __builtin_cimagf __builtin_creall __builtin_cimagl __builtin_conj __builtin_conjf __builtin_conjl
		__builtin_isnan __builtin_isinf __builtin_isfinite __builtin_isnormal __builtin_signbit __builtin_signbitf
		__builtin_signbitl __builtin_isinf_sign __builtin_fpclassify
		__builtin_isgreater __builtin_isgreaterequal __builtin_isless __builtin_islessequal __builtin_islessgreater
		__builtin_isunordered __builtin_add_overflow __builtin_sub_overflow __builtin_mul_overflow
		__builtin_add_overflow_p __builtin_sub_overflow_p __builtin_mul_overflow_p
		__builtin_sadd_overflow __builtin_saddl_overflow __builtin_saddll_overflow __builtin_uadd_overflow
		__builtin_uaddl_overflow __builtin_uaddll_overflow __builtin_ssub_overflow __builtin_ssubl_overflow
		__builtin_ssubll_overflow __builtin_usub_overflow __builtin_usubl_overflow __builtin_usubll_overflow
		__builtin_smul_overflow __builtin_smull_overflow __builtin_smulll_overflow __builtin_umul_overflow
		__builtin_umull_overflow __builtin_umulll_overflow
		__builtin_classify_type __builtin_LINE __builtin_FILE __builtin_FUNCTION
		__sync_synchronize __sync_fetch_and_add __sync_fetch_and_sub __sync_fetch_and_or __sync_fetch_and_and
		__sync_fetch_and_xor __sync_fetch_and_nand __sync_add_and_fetch __sync_sub_and_fetch __sync_or_and_fetch
		__sync_and_and_fetch __sync_xor_and_fetch __sync_nand_and_fetch __sync_val_compare_and_swap
		__sync_bool_compare_and_swap __sync_lock_test_and_set __sync_lock_release
		__atomic_load_n __atomic_store_n __atomic_exchange_n __atomic_compare_exchange_n __atomic_load __atomic_store
		__atomic_exchange __atomic_compare_exchange
		__atomic_fetch_add __atomic_fetch_sub __atomic_fetch_and __atomic_fetch_or __atomic_fetch_xor __atomic_fetch_nand
		__atomic_add_fetch __atomic_sub_fetch __atomic_and_fetch __atomic_or_fetch __atomic_xor_fetch __atomic_nand_fetch
		__atomic_thread_fence __atomic_signal_fence __atomic_test_and_set __atomic_clear __atomic_always_lock_free
		__atomic_is_lock_free`) {
		builtinNames[n] = true
	}
}

// HasBuiltin tells whether __has_builtin(name) is true
func HasBuiltin(name string) bool {
	return builtinNames[name] || libBuiltins[name] != ""
}

// Fetch-op kinds for NFetchOp (Val)
const (
	FetchAdd = iota
	FetchSub
	FetchAnd
	FetchOr
	FetchXor
	FetchNand
)

// fetch op flags (stored in Begin): 1 = return the new value
func (p *parser) args(n int) []*Node {
	p.expect("(")
	var a []*Node
	for !p.is(")") {
		if len(a) > 0 {
			p.expect(",")
		}
		e := p.assign()
		addType(e)
		a = append(a, e)
	}
	p.expect(")")
	if n >= 0 && len(a) != n {
		p.errorAt(p.cur(), "wrong number of arguments to builtin (expected %d, have %d)", n, len(a))
	}
	return a
}

func (p *parser) builtinCall(name string, t *Token, args []*Node, ty *Type) *Node {
	return &Node{Kind: NBuiltin, Builtin: name, Args: args, Tok: t, Ty: ty}
}

// libCall calls a library function, declaring it implicitly when needed
func (p *parser) libCall(name string, t *Token) *Node {
	v := p.findVar(name)
	var o *Obj
	if v != nil && v.obj != nil && v.obj.IsFunction {
		o = v.obj
	} else if g := p.globalsByName[name]; g != nil && g.IsFunction {
		o = g
	} else {
		ft := funcType(libReturnType(name))
		ft.OldStyle = true
		gs := p.sc
		for gs.parent != nil {
			gs = gs.parent
		}
		o = &Obj{Name: name, AsmName: name, Ty: ft, Tok: t, IsFunction: true}
		p.globalsByName[name] = o
		p.prog.Globals = append(p.prog.Globals, o)
	}
	p.markUsed(o)
	p.expect("(")
	return p.funcall(newVarNode(o, t), t)
}

func libReturnType(name string) *Type {
	switch name {
	case "memcpy", "memmove", "memset", "memchr", "strcpy", "strncpy", "strcat", "strncat", "strchr", "strrchr",
		"strstr", "strdup", "strndup", "malloc", "calloc", "realloc", "alloca", "strpbrk", "stpcpy", "index", "rindex":
		return pointerTo(tyVoid)
	case "strlen", "strspn", "strcspn":
		return tyULong
	case "labs", "lround", "lrint":
		return tyLong
	case "llabs":
		return tyLLong
	case "abort", "exit", "_exit", "free", "bzero":
		return tyVoid
	case "fabsf", "sqrtf", "floorf", "ceilf", "roundf", "truncf", "powf", "expf", "logf", "sinf", "cosf", "copysignf",
		"fminf", "fmaxf":
		return tyFloat
	case "fabsl", "sqrtl", "copysignl":
		return tyLDouble
	case "fabs", "sqrt", "sin", "cos", "tan", "exp", "log", "log2", "log10", "pow", "floor", "ceil", "round", "trunc",
		"fmod", "copysign", "fmin", "fmax", "nearbyint", "rint", "frexp", "ldexp", "modf", "atan", "atan2", "asin",
		"acos", "sinh", "cosh", "tanh", "hypot", "cbrt", "exp2", "expm1", "log1p", "fma", "scalbn":
		return tyDouble
	}
	return tyInt
}

func floatConst(f float64, ty *Type, t *Token) *Node {
	return &Node{Kind: NNum, FVal: f, Ty: ty, Tok: t}
}

// builtin parses a call of a compiler builtin; nil if t is not one
func (p *parser) builtin(t *Token) *Node {
	name := t.Text
	if !builtinNames[name] && libBuiltins[name] == "" {
		return nil
	}
	if lib := libBuiltins[name]; lib != "" && !builtinNames[name] {
		p.next()
		if lib == "alloca" {
			a := p.args(1)
			return p.newAlloca(newCast(a[0], tyULong))
		}
		return p.libCall(lib, t)
	}
	p.next()
	switch name {
	case "__builtin_complex":
		a := p.args(2)
		if !a[0].Ty.IsFlonum() {
			p.errorAt(t, "__builtin_complex operands must be floating")
		}
		return makeComplex(complexOf(a[0].Ty), a[0], a[1], t)
	case "__builtin_creal", "__builtin_crealf", "__builtin_creall", "__builtin_cimag", "__builtin_cimagf", "__builtin_cimagl":
		a := p.args(1)
		return p.realImag(a[0], strings.Contains(name, "cimag"), t)
	case "__builtin_conj", "__builtin_conjf", "__builtin_conjl":
		a := p.args(1)
		if !a[0].Ty.IsComplex() {
			return newCast(a[0], complexOf(a[0].Ty))
		}
		return p.complexUnary(NBitNot, a[0], t)
	case "__builtin_va_start":
		a := p.args(-1)
		if len(a) < 1 {
			p.errorAt(t, "too few arguments to va_start")
		}
		if p.fn == nil || !p.fn.Ty.Variadic {
			p.errorAt(t, "'va_start' used in function with fixed arguments")
		}
		return &Node{Kind: NVaStart, LHS: a[0], Tok: t, Ty: tyVoid}
	case "__builtin_va_end":
		a := p.args(1)
		return newCast(a[0], tyVoid)
	case "__builtin_va_copy":
		a := p.args(2)
		return &Node{Kind: NVaCopy, LHS: a[0], RHS: a[1], Tok: t, Ty: tyVoid}
	case "__builtin_va_arg":
		p.expect("(")
		ap := p.assign()
		p.expect(",")
		ty := p.typename()
		p.expect(")")
		addType(ap)
		n := &Node{Kind: NVaArg, LHS: ap, Tok: t, Ty: pointerTo(ty)}
		return newUnary(NDeref, n, t)
	case "__builtin_offsetof":
		p.expect("(")
		ty := p.typename()
		p.expect(",")
		off := int64(0)
		var dyn *Node // non-constant index parts (GNU extension)
		cur := ty
		first := true
		for !p.is(")") {
			if !first || p.is(".") {
				if p.consume("[") {
					ie := p.expr()
					p.expect("]")
					if !cur.IsArray() {
						p.errorAt(t, "subscript of non-array in offsetof")
					}
					if p.isConstExpr(ie) {
						off += p.eval(ie) * cur.Base.Size()
					} else {
						part := p.newBinary(NMul, newCast(ie, tyULong), newULong(cur.Base.Size(), t), t)
						if dyn == nil {
							dyn = part
						} else {
							dyn = p.newBinary(NAdd, dyn, part, t)
						}
					}
					cur = cur.Base
					continue
				}
				p.expect(".")
			}
			first = false
			name := p.ident()
			if !cur.IsRecord() {
				p.errorAt(name, "offsetof of a member of a non-struct")
			}
			path := findMember(cur.Rec, name.Text)
			if path == nil {
				p.errorAt(name, "'%s' has no member named '%s'", cur, name.Text)
			}
			for _, m := range path {
				off += m.Offset
				cur = m.Ty
			}
		}
		p.expect(")")
		if dyn != nil {
			n := p.newBinary(NAdd, dyn, newULong(off, t), t)
			addType(n)
			return n
		}
		return newULong(off, t)
	case "__builtin_expect", "__builtin_expect_with_probability":
		a := p.args(-1)
		if len(a) < 2 {
			p.errorAt(t, "too few arguments to __builtin_expect")
		}
		return newCast(a[0], tyLong)
	case "__builtin_assume_aligned":
		a := p.args(-1)
		return newCast(a[0], pointerTo(tyVoid))
	case "__builtin_alloca", "__builtin_alloca_with_align":
		a := p.args(-1)
		return p.newAlloca(newCast(a[0], tyULong))
	case "__builtin_inf", "__builtin_huge_val":
		p.args(0)
		return floatConst(math.Inf(1), tyDouble, t)
	case "__builtin_inff", "__builtin_huge_valf":
		p.args(0)
		return floatConst(math.Inf(1), tyFloat, t)
	case "__builtin_infl", "__builtin_huge_vall":
		p.args(0)
		return floatConst(math.Inf(1), tyLDouble, t)
	case "__builtin_nan", "__builtin_nans":
		p.args(1)
		return floatConst(math.NaN(), tyDouble, t)
	case "__builtin_nanf", "__builtin_nansf":
		p.args(1)
		return floatConst(math.NaN(), tyFloat, t)
	case "__builtin_nanl", "__builtin_nansl":
		p.args(1)
		return floatConst(math.NaN(), tyLDouble, t)
	case "__builtin_constant_p":
		p.expect("(")
		e := p.assign()
		p.expect(")")
		addType(e)
		if p.isConstExpr(e) {
			return newNum(1, t)
		}
		return newNum(0, t)
	case "__builtin_types_compatible_p":
		p.expect("(")
		a := p.typename()
		p.expect(",")
		b := p.typename()
		p.expect(")")
		if compatible(a.Unqual(), b.Unqual()) {
			return newNum(1, t)
		}
		return newNum(0, t)
	case "__builtin_choose_expr":
		p.expect("(")
		c := p.constExpr()
		p.expect(",")
		a := p.assign()
		p.expect(",")
		b := p.assign()
		p.expect(")")
		if c != 0 {
			return a
		}
		return b
	case "__builtin_unreachable", "__builtin_trap":
		p.args(0)
		return p.builtinCall("trap", t, nil, tyVoid)
	case "__builtin_bswap16":
		a := p.args(1)
		return p.builtinCall("bswap16", t, []*Node{newCast(a[0], tyUShort)}, tyUShort)
	case "__builtin_bswap32":
		a := p.args(1)
		return p.builtinCall("bswap32", t, []*Node{newCast(a[0], tyUInt)}, tyUInt)
	case "__builtin_bswap64":
		a := p.args(1)
		return p.builtinCall("bswap64", t, []*Node{newCast(a[0], tyULong)}, tyULong)
	case "__builtin_clz", "__builtin_ctz", "__builtin_popcount", "__builtin_ffs", "__builtin_parity", "__builtin_clrsb",
		"__builtin_clzl", "__builtin_ctzl", "__builtin_popcountl", "__builtin_ffsl", "__builtin_parityl", "__builtin_clrsbl",
		"__builtin_clzll", "__builtin_ctzll", "__builtin_popcountll", "__builtin_ffsll", "__builtin_parityll", "__builtin_clrsbll":
		a := p.args(1)
		base := strings.TrimPrefix(name, "__builtin_")
		at := tyUInt
		if strings.HasSuffix(base, "l") && base != "clrsbl" || base == "clrsbl" {
			at = tyULong
		}
		base = strings.TrimRight(base, "l")
		if base == "ffs" || base == "clrsb" {
			if at == tyULong {
				at = tyLong
			} else {
				at = tyInt
			}
		}
		return p.builtinCall(base, t, []*Node{newCast(a[0], at)}, tyInt)
	case "__builtin_frame_address":
		p.args(1)
		return p.builtinCall("frame_address", t, nil, pointerTo(tyVoid))
	case "__builtin_return_address":
		p.args(1)
		return p.builtinCall("return_address", t, nil, pointerTo(tyVoid))
	case "__builtin_prefetch":
		a := p.args(-1)
		return newCast(a[0], tyVoid)
	case "__builtin_object_size":
		a := p.args(2)
		if p.eval(a[1])&2 != 0 {
			return newULong(0, t)
		}
		return newULong(-1, t)
	case "__builtin_classify_type":
		a := p.args(1)
		ty := a[0].Ty
		switch {
		case ty.Kind == TyVoid:
			return newNum(0, t)
		case ty.IsInteger():
			return newNum(1, t)
		case ty.IsFlonum():
			return newNum(8, t)
		case decayType(ty).Kind == TyPtr:
			return newNum(5, t)
		case ty.Kind == TyStruct:
			return newNum(12, t)
		case ty.Kind == TyUnion:
			return newNum(13, t)
		}
		return newNum(-1, t)
	case "__builtin_LINE":
		p.args(0)
		return newNum(int64(t.loc().Line), t)
	case "__builtin_FILE":
		p.args(0)
		f := ""
		if t.loc().File != nil {
			f = t.loc().File.Name
		}
		return p.makeString(f, t)
	case "__builtin_FUNCTION":
		p.args(0)
		return p.funcNameVar(t)
	case "__builtin_isnan", "__builtin_isinf", "__builtin_isfinite", "__builtin_isnormal", "__builtin_signbit",
		"__builtin_signbitf", "__builtin_signbitl", "__builtin_isinf_sign":
		a := p.args(1)
		x := a[0]
		if x.Ty.IsInteger() {
			x = newCast(x, tyDouble)
		}
		return p.builtinCall(strings.TrimPrefix(name, "__builtin_"), t, []*Node{x}, tyInt)
	case "__builtin_fpclassify":
		a := p.args(6)
		x := a[5]
		if x.Ty.IsInteger() {
			x = newCast(x, tyDouble)
		}
		args := []*Node{newCast(a[0], tyInt), newCast(a[1], tyInt), newCast(a[2], tyInt), newCast(a[3], tyInt), newCast(a[4], tyInt), x}
		return p.builtinCall("fpclassify", t, args, tyInt)
	case "__builtin_isgreater", "__builtin_isgreaterequal", "__builtin_isless", "__builtin_islessequal",
		"__builtin_islessgreater", "__builtin_isunordered":
		a := p.args(2)
		ct := commonType(a[0].Ty, a[1].Ty)
		x, y := newCast(a[0], ct), newCast(a[1], ct)
		switch name {
		case "__builtin_isgreater":
			return p.newBinary(NLt, y, x, t)
		case "__builtin_isgreaterequal":
			return p.newBinary(NLe, y, x, t)
		case "__builtin_isless":
			return p.newBinary(NLt, x, y, t)
		case "__builtin_islessequal":
			return p.newBinary(NLe, x, y, t)
		case "__builtin_islessgreater":
			return p.newBinary(NLogOr, p.newBinary(NLt, x, y, t), p.newBinary(NLt, newCast(a[1], ct), newCast(a[0], ct), t), t)
		}
		return p.builtinCall("isunordered", t, []*Node{x, y}, tyInt)
	case "__builtin_add_overflow", "__builtin_sub_overflow", "__builtin_mul_overflow":
		a := p.args(3)
		op := strings.TrimSuffix(strings.TrimPrefix(name, "__builtin_"), "_overflow")
		if decayType(a[2].Ty).Kind != TyPtr || !a[2].Ty.Base.IsInteger() {
			p.errorAt(t, "argument 3 of '%s' must be a pointer to an integer", name)
		}
		return p.builtinCall(op+"_overflow", t, a, tyBool)
	case "__builtin_add_overflow_p", "__builtin_sub_overflow_p", "__builtin_mul_overflow_p":
		a := p.args(3)
		op := strings.TrimSuffix(strings.TrimPrefix(name, "__builtin_"), "_overflow_p")
		// the third argument only gives the type
		v := p.newLvar("", a[2].Ty.Unqual())
		args := []*Node{a[0], a[1], newUnary(NAddr, newVarNode(v, t), t)}
		addType(args[2])
		return p.builtinCall(op+"_overflow", t, args, tyBool)
	}
	if strings.HasSuffix(name, "_overflow") && strings.HasPrefix(name, "__builtin_") {
		// typed variants: __builtin_sadd_overflow etc.
		a := p.args(3)
		base := strings.TrimSuffix(strings.TrimPrefix(name, "__builtin_"), "_overflow")
		signed := base[0] == 's'
		op := base[1:4] // add, sub, mul
		var ty *Type
		switch base[4:] {
		case "ll":
			ty = tyLLong
		case "l":
			ty = tyLong
		default:
			ty = tyInt
		}
		base = op
		if !signed {
			ty = unsignedOf(ty)
		}
		return p.builtinCall(base+"_overflow", t, []*Node{newCast(a[0], ty), newCast(a[1], ty), a[2]}, tyBool)
	}
	if strings.HasPrefix(name, "__sync_") || strings.HasPrefix(name, "__atomic_") {
		return p.atomicBuiltin(name, t)
	}
	p.errorAt(t, "builtin '%s' is not supported", name)
	return nil
}

func fetchKind(s string) int {
	switch s {
	case "add":
		return FetchAdd
	case "sub":
		return FetchSub
	case "and":
		return FetchAnd
	case "or":
		return FetchOr
	case "xor":
		return FetchXor
	}
	return FetchNand
}

func (p *parser) atomicPtr(a *Node, t *Token) *Type {
	pt := decayType(a.Ty)
	if pt.Kind != TyPtr || !(pt.Base.IsScalar()) {
		p.errorAt(t, "operand of atomic builtin must be a pointer to an integer or pointer")
	}
	return pt.Base.Unqual()
}

func (p *parser) atomicBuiltin(name string, t *Token) *Node {
	switch name {
	case "__sync_synchronize", "__atomic_thread_fence", "__atomic_signal_fence":
		p.args(-1)
		return &Node{Kind: NFence, Tok: t, Ty: tyVoid}
	case "__atomic_always_lock_free", "__atomic_is_lock_free":
		a := p.args(2)
		sz := p.eval(a[0])
		return newNum(b2i(sz == 1 || sz == 2 || sz == 4 || sz == 8), t)
	}
	if strings.HasPrefix(name, "__sync_fetch_and_") || strings.HasPrefix(name, "__atomic_fetch_") {
		op := strings.TrimPrefix(strings.TrimPrefix(name, "__sync_fetch_and_"), "__atomic_fetch_")
		a := p.args(-1)
		ty := p.atomicPtr(a[0], t)
		return &Node{Kind: NFetchOp, LHS: a[0], RHS: newCast(a[1], ty), Val: int64(fetchKind(op)), Tok: t, Ty: ty}
	}
	if strings.HasSuffix(name, "_and_fetch") && strings.HasPrefix(name, "__sync_") ||
		strings.HasPrefix(name, "__atomic_") && strings.HasSuffix(name, "_fetch") {
		op := strings.TrimSuffix(strings.TrimSuffix(strings.TrimPrefix(strings.TrimPrefix(name, "__sync_"), "__atomic_"), "_and_fetch"), "_fetch")
		a := p.args(-1)
		ty := p.atomicPtr(a[0], t)
		return &Node{Kind: NFetchOp, LHS: a[0], RHS: newCast(a[1], ty), Val: int64(fetchKind(op)), Begin: 1, Tok: t, Ty: ty}
	}
	switch name {
	case "__sync_val_compare_and_swap", "__sync_bool_compare_and_swap":
		a := p.args(3)
		ty := p.atomicPtr(a[0], t)
		n := &Node{Kind: NCas, CasAddr: a[0], CasOld: newCast(a[1], ty), CasNew: newCast(a[2], ty), Tok: t, Ty: tyBool}
		if name == "__sync_val_compare_and_swap" {
			n.Begin = 1 // return the old value
			n.Ty = ty
		}
		return n
	case "__atomic_compare_exchange_n", "__atomic_compare_exchange":
		a := p.args(6)
		ty := p.atomicPtr(a[0], t)
		desired := a[2]
		if name == "__atomic_compare_exchange" {
			desired = newUnary(NDeref, a[2], t)
		}
		// expected is a pointer: on failure it receives the current value
		return &Node{Kind: NCas, CasAddr: a[0], CasOld: a[1], CasNew: newCast(desired, ty), Begin: 2, Tok: t, Ty: tyBool}
	case "__sync_lock_test_and_set", "__atomic_exchange_n", "__atomic_test_and_set":
		a := p.args(-1)
		ty := p.atomicPtr(a[0], t)
		val := newNum(1, t)
		if name != "__atomic_test_and_set" {
			val = a[1]
		}
		n := &Node{Kind: NExch, CasAddr: a[0], RHS: newCast(val, ty), Tok: t, Ty: ty}
		if name == "__atomic_test_and_set" {
			n.Ty = tyBool
		}
		return n
	case "__atomic_exchange":
		a := p.args(4)
		ty := p.atomicPtr(a[0], t)
		ex := &Node{Kind: NExch, CasAddr: a[0], RHS: newCast(newUnary(NDeref, a[1], t), ty), Tok: t, Ty: ty}
		return p.newBinary(NAssign, newUnary(NDeref, a[2], t), ex, t)
	case "__sync_lock_release", "__atomic_clear":
		a := p.args(-1)
		ty := p.atomicPtr(a[0], t)
		ex := &Node{Kind: NExch, CasAddr: a[0], RHS: newCast(newNum(0, t), ty), Tok: t, Ty: ty}
		return newCast(ex, tyVoid)
	case "__atomic_load_n":
		a := p.args(2)
		p.atomicPtr(a[0], t)
		n := newUnary(NDeref, a[0], t)
		return p.newBinary(NComma, &Node{Kind: NFence, Tok: t, Ty: tyVoid}, n, t)
	case "__atomic_load":
		a := p.args(3)
		p.atomicPtr(a[0], t)
		return p.newBinary(NAssign, newUnary(NDeref, a[1], t), newUnary(NDeref, a[0], t), t)
	case "__atomic_store_n", "__atomic_store":
		a := p.args(3)
		ty := p.atomicPtr(a[0], t)
		val := a[1]
		if name == "__atomic_store" {
			val = newUnary(NDeref, a[1], t)
		}
		ex := &Node{Kind: NExch, CasAddr: a[0], RHS: newCast(val, ty), Tok: t, Ty: ty}
		return newCast(ex, tyVoid)
	}
	p.errorAt(t, "builtin '%s' is not supported", name)
	return nil
}
