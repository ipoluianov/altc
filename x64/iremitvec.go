package x64

import (
	"fmt"
	"strings"
)

// Emission of vector values. Instructions are produced as AT&T text and
// assembled by the table-driven assembler (Asm.Text), in their legacy SSE
// form or, with AVX, their VEX form.

func vecWidth(t irType) int {
	if t == iV256 {
		return 32
	}
	return 16
}

// xname is the register name of an xmm-class value of type t
func xname(r Reg, t irType) string { return RegName(r, vecWidth(t)) }

func (e *irEmitter) text(format string, args ...any) {
	if err := e.a.Text(fmt.Sprintf(format, args...)); err != nil {
		panic(err)
	}
}

// xcopy copies an xmm-class register
func (e *irEmitter) xcopy(dst, src Reg, t irType) {
	if dst == src {
		return
	}
	if t == iV256 {
		e.a.MovapsY(dst, src)
		return
	}
	e.a.Movaps(dst, src)
}

func (e *irEmitter) xload(dst Reg, t irType, m Mem) {
	switch t {
	case iF32, iF64:
		e.a.MovF(t == iF64, X(dst), m)
	default:
		e.a.Movups(vecWidth(t), R{dst, vecWidth(t)}, m)
	}
}

func (e *irEmitter) xstore(m Mem, src Reg, t irType) {
	switch t {
	case iF32, iF64:
		e.a.MovF(t == iF64, m, X(src))
	default:
		e.a.Movups(vecWidth(t), m, R{src, vecWidth(t)})
	}
}

// vconstMem places a vector constant in the pool
func (e *irEmitter) vconstMem(v *irValue) Mem {
	return MSym(e.g.constLabel(v.Data, int64(len(v.Data))), 0)
}

func allZero(d []byte) bool {
	for _, b := range d {
		if b != 0 {
			return false
		}
	}
	return true
}

// vmaterialize puts a vector value into the register x
func (e *irEmitter) vmaterialize(v *irValue, x Reg) {
	if v.Op == opConst {
		if allZero(v.Data) {
			e.a.Xorp(false, x, X(x))
			return
		}
		e.xload(x, v.Type, e.vconstMem(v))
		return
	}
	switch v.loc.kind {
	case locReg:
		e.xcopy(x, v.loc.reg, v.Type)
	case locStack:
		e.xload(x, v.Type, e.locMem(v.loc))
	default:
		panic(fmt.Sprintf("v%d has no location", v.ID))
	}
}

// xsrc formats an xmm-class operand: a register, or memory when allowed;
// other values are loaded into scratch
func (e *irEmitter) xsrc(x *irValue, scratch Reg, memOK bool) string {
	if r, ok := e.regOf(x); ok {
		return xname(r, x.Type)
	}
	if memOK && !x.Op.remat() && x.loc.kind == locStack {
		return e.locMem(x.loc).String()
	}
	if memOK && x.Op == opConst && x.Type.isVec() && !allZero(x.Data) {
		// straight from the constant pool (aligned to the vector size)
		return e.vconstMem(x).String()
	}
	if x.Type.isVec() {
		e.vmaterialize(x, scratch)
	} else {
		e.fmaterialize(x, scratch)
	}
	return xname(scratch, x.Type)
}

// gsrc formats a general-purpose operand (in a register)
func (e *irEmitter) gsrc(x *irValue, scratch Reg) string {
	return RegName(e.ireg(x, scratch), x.Type.size())
}

var flagSuffix = map[string]Cond{
	"e": CondE, "z": CondE, "ne": CondNE, "nz": CondNE, "a": CondA, "ae": CondAE, "nc": CondAE,
	"b": CondB, "c": CondB, "be": CondBE, "p": CondP, "np": CondNP,
}

// vecCommutative instructions give the same result with swapped operands
var vecCommutative = map[string]bool{
	"addps": true, "addpd": true, "mulps": true, "mulpd": true, "andps": true, "andpd": true, "orps": true,
	"orpd": true, "xorps": true, "xorpd": true, "paddb": true, "paddw": true, "paddd": true, "paddq": true,
	"pmullw": true, "pmulld": true, "pand": true, "por": true, "pxor": true, "pcmpeqb": true, "pcmpeqw": true,
	"pcmpeqd": true, "pcmpeqq": true, "pavgb": true, "pavgw": true, "pmaxsw": true, "pmaxub": true,
	"pminsw": true, "pminub": true, "pmaxsd": true, "pminsd": true, "pmaxud": true, "pminud": true,
	"pmuludq": true, "pmuldq": true, "pmulhw": true, "pmulhuw": true, "paddsb": true, "paddsw": true,
	"paddusb": true, "paddusw": true,
}

// immShifts are the shifts whose immediate form takes one vector operand
var immShifts = map[string]bool{
	"psllw": true, "pslld": true, "psllq": true, "psrlw": true, "psrld": true, "psrlq": true,
	"psraw": true, "psrad": true, "pslldq": true, "psrldq": true,
}

func (e *irEmitter) vinsn(v *irValue) {
	a := e.a
	mn, suffix, _ := strings.Cut(v.Sym, ":")
	avx := a.AVX
	vmn := mn
	if avx {
		vmn = "v" + mn
	}
	if !avx && mn != "" && insts[mn] == nil {
		panic(fmt.Sprintf("%s needs AVX (-mavx)", mn))
	}
	imm := ""
	if v.Signed {
		imm = fmt.Sprintf("$%d, ", v.AuxInt)
	}
	args := v.Args
	res := v.Type
	nx := 0
	for _, x := range args {
		if x.Type.inXMM() {
			nx++
		}
	}
	form := vNDS
	if d := vexTable["v"+mn]; d != nil {
		form = d.form
	}
	if v.Signed && (mn == "permilps" || mn == "permilpd") {
		form = v2I // the immediate form
	}

	// flags: ptest, ucomiss... then setcc
	if suffix != "" {
		c, ok := flagSuffix[suffix]
		if !ok || len(args) != 2 {
			panic("bad flag instruction " + v.Sym)
		}
		x := e.xsrc(args[0], XMM14, false)
		y := e.xsrc(args[1], XMM15, true)
		e.text("%s %s, %s", vmn, y, x)
		a.Setcc(c, RAX)
		if v.needsLoc() {
			d := e.dst(v, R11)
			a.Movzx(4, d, 1, B(RAX))
			e.finish(v, d)
		}
		return
	}

	// general-purpose operands only (BMI): op argN, ..., arg1, dst
	if !res.inXMM() && nx == 0 {
		d := e.dst(v, R11)
		var ops []string
		scratch := []Reg{RAX, R10, RCX}
		for i := len(args) - 1; i >= 0; i-- {
			ops = append(ops, e.gsrc(args[i], scratch[i]))
		}
		e.text("%s %s%s, %s", mn, imm, strings.Join(ops, ", "), RegName(d, res.size()))
		e.finish(v, d)
		return
	}

	// masked loads: (pointer, mask)
	if strings.Contains(mn, "maskmov") && len(args) == 2 && !args[0].Type.inXMM() {
		p := e.ireg(args[0], RAX)
		d := e.dst(v, XMM15)
		e.text("v%s (%s), %s, %s", mn, RegName(p, 8), e.xsrc(args[1], XMM14, false), xname(d, res))
		e.finish(v, d)
		return
	}

	// a general-purpose result: movmsk, pextr, movd/movq, cvt..2si
	if !res.inXMM() {
		d := e.dst(v, R11)
		e.text("%s %s%s, %s", vmn, imm, e.xsrc(args[0], XMM14, false), RegName(d, res.size()))
		e.finish(v, d)
		return
	}

	d := e.dst(v, XMM15)
	switch {
	case mn == "":
		// a reinterpretation (float <-> vector)
		x := args[0]
		if x.Type.isVec() {
			e.vmaterialize(x, d)
		} else {
			e.fmaterialize(x, d)
		}
	case len(args) == 0:
		// zero and all-ones idioms: op d, d
		if avx {
			e.text("%s %s, %s, %s", vmn, xname(d, res), xname(d, res), xname(d, res))
		} else {
			e.text("%s %s, %s", vmn, xname(d, res), xname(d, res))
		}
	case nx == 0:
		// from a general-purpose register: movd, movq
		e.text("%s %s, %s", vmn, e.gsrc(args[0], R10), xname(d, res))
	case len(args) == 1 && immShifts[mn] && v.Signed:
		if avx {
			e.text("%s %s%s, %s", vmn, imm, e.xsrc(args[0], XMM14, false), xname(d, res))
		} else {
			e.vmaterialize(args[0], d)
			e.text("%s %s%s", mn, imm, xname(d, res))
		}
	case len(args) == 1:
		x := args[0]
		if form == vNDS || form == vNDSI {
			// scalar operations merging into the source: sqrtss...
			if avx {
				xs := e.xsrc(x, XMM14, false)
				e.text("%s %s%s, %s, %s", vmn, imm, xs, xs, xname(d, res))
			} else {
				e.vmovIn(x, d)
				e.text("%s %s%s, %s", mn, imm, xname(d, res), xname(d, res))
			}
			break
		}
		// extracts take their source in the ModRM.reg field: a register
		e.text("%s %s%s, %s", vmn, imm, e.xsrc(x, XMM14, form != vExtract), xname(d, res))
	case len(args) == 2:
		x, y := args[0], args[1]
		regOnly := mn == "movss" || mn == "movsd" || !y.Type.inXMM()
		if avx {
			var ys string
			if y.Type.inXMM() {
				ys = e.xsrc(y, XMM15, !regOnly)
			} else {
				ys = e.gsrc(y, R10)
			}
			xs := e.xsrc(x, XMM14, false)
			if form == v2 || form == v2I {
				e.text("%s %s%s, %s", vmn, imm, ys, xname(d, res))
			} else {
				e.text("%s %s%s, %s, %s", vmn, imm, ys, xs, xname(d, res))
			}
			break
		}
		if r, ok := e.regOf(y); ok && r == d && x != y {
			if vecCommutative[mn] && !v.Signed {
				x, y = y, x
			} else {
				d = XMM15
			}
		}
		e.vmovIn(x, d)
		var ys string
		if y.Type.inXMM() {
			ys = e.xsrc(y, XMM14, !regOnly)
		} else {
			ys = e.gsrc(y, R10)
		}
		e.text("%s %s%s, %s", mn, imm, ys, xname(d, res))
	case len(args) == 3 && (strings.HasPrefix(mn, "fm") || strings.HasPrefix(mn, "fnm")):
		// fused multiply-add on (a, b, c): d = a; vfmadd213 c, b, d
		x, y, z := args[0], args[1], args[2]
		ry, oky := e.regOf(y)
		rz, okz := e.regOf(z)
		if oky && ry == d && x != y || okz && rz == d && x != z {
			d = XMM15
		}
		e.vmovIn(x, d)
		ys := e.xsrc(y, XMM14, false)
		zs := e.xsrc(z, XMM13, true)
		e.text("%s %s, %s, %s", vmn, zs, ys, xname(d, res))
	case len(args) == 3:
		// blendv (a, b, mask)
		x, y, m := args[0], args[1], args[2]
		if avx {
			ms := e.xsrc(m, XMM13, false)
			ys := e.xsrc(y, XMM15, true)
			xs := e.xsrc(x, XMM14, false)
			e.text("%s %s, %s, %s, %s", vmn, ms, ys, xs, xname(d, res))
			break
		}
		// the legacy form takes the mask in %xmm0
		a.Movaps(XMM14, XMM0)
		e.vmovInFrom(x, XMM15, XMM14)
		e.vmovInFrom(m, XMM0, XMM14)
		ys := e.xsrcFrom(y, XMM14)
		e.text("%s %%xmm0, %s, %%xmm15", mn, ys)
		a.Movaps(XMM0, XMM14)
		d = XMM15
	default:
		panic(fmt.Sprintf("cannot emit %s with %d operands", v.Sym, len(args)))
	}
	e.finish(v, d)
}

// vmovIn copies an xmm-class value into the register d
func (e *irEmitter) vmovIn(x *irValue, d Reg) {
	if x.Type.isVec() {
		e.vmaterialize(x, d)
	} else {
		e.fmaterialize(x, d)
	}
}

// vmovInFrom copies x into d, reading %xmm0 from saved when x lived there
// (blendv saves %xmm0 before using it for the mask)
func (e *irEmitter) vmovInFrom(x *irValue, d, saved Reg) {
	if r, ok := e.regOf(x); ok && r == XMM0 {
		e.a.Movaps(d, saved)
		return
	}
	e.vmovIn(x, d)
}

func (e *irEmitter) xsrcFrom(x *irValue, saved Reg) string {
	if r, ok := e.regOf(x); ok && r == XMM0 {
		return xname(saved, x.Type)
	}
	if r, ok := e.regOf(x); ok {
		return xname(r, x.Type)
	}
	if !x.Op.remat() && x.loc.kind == locStack {
		return e.locMem(x.loc).String()
	}
	e.vmovIn(x, XMM13)
	return xname(XMM13, x.Type)
}

// vbroadcast fills every element with a scalar
func (e *irEmitter) vbroadcast(v *irValue) {
	a := e.a
	t := v.Type
	x := v.Args[0]
	d := e.dst(v, XMM15)
	dn := xname(d, t)
	d128 := xname(d, iV128)
	avx, avx2 := e.g.isa.Has(ISAAVX), e.g.isa.Has(ISAAVX2)
	upper := func() {
		if t == iV256 {
			e.text("vinsertf128 $1, %s, %s, %s", d128, dn, dn)
		}
	}
	switch k := v.Sym; k {
	case "ps", "pd":
		xs := e.xsrc(x, XMM14, false)
		switch {
		case avx2 && (k == "ps" || t == iV256):
			e.text("vbroadcasts%s %s, %s", k[1:], xs, dn)
		case avx && k == "ps":
			e.text("vshufps $0, %s, %s, %s", xs, xs, d128)
			upper()
		case avx:
			e.text("vmovddup %s, %s", xs, d128)
			upper()
		case k == "ps":
			e.fmaterialize(x, d)
			e.text("shufps $0, %s, %s", d128, d128)
		default:
			e.fmaterialize(x, d)
			e.text("unpcklpd %s, %s", d128, d128)
		}
	default:
		r := e.ireg(x, R10)
		mv := "movd"
		if k == "q" {
			mv = "movq"
		}
		if avx2 {
			e.text("v%s %s, %s", mv, RegName(r, x.Type.size()), d128)
			e.text("vpbroadcast%s %s, %s", k, d128, dn)
			break
		}
		p := ""
		if avx {
			p = "v"
		}
		e.text("%s%s %s, %s", p, mv, RegName(r, x.Type.size()), d128)
		three := func(op, arg string) {
			if avx {
				e.text("v%s %s%s, %s, %s", op, arg, d128, d128, d128)
			} else {
				e.text("%s %s%s, %s", op, arg, d128, d128)
			}
		}
		two := func(op, arg string) {
			e.text("%s%s %s%s, %s", p, op, arg, d128, d128)
		}
		switch k {
		case "b":
			three("punpcklbw", "")
			two("pshuflw", "$0, ")
			two("pshufd", "$0, ")
		case "w":
			two("pshuflw", "$0, ")
			two("pshufd", "$0, ")
		case "d":
			two("pshufd", "$0, ")
		case "q":
			three("punpcklqdq", "")
		}
		upper()
	}
	_ = a
	e.finish(v, d)
}
