package x64

import (
	"fmt"
	"strings"
)

// VEX-encoded instructions: AVX, AVX2, FMA, F16C, BMI1 and BMI2.
//
// Most AVX instructions are the SSE ones with a "v" prefix: the legacy
// prefix becomes VEX.pp, the opcode map VEX.mmmmm, and the destructive
// two-operand form gets a separate first source in VEX.vvvv. The table
// below is derived from the SSE table; AVX-only instructions are added
// explicitly. Registers %ymmN are the %xmmN registers with size 32.

type vexForm int

const (
	vNDS       vexForm = iota // op rm, vvvv, reg
	vNDSI                     // op $imm, rm, vvvv, reg
	v2                        // op rm, reg
	v2I                       // op $imm, rm, reg
	vMov                      // load (op) and store (store opcode) forms
	vStore                    // op reg, mem
	vShift                    // count in xmm/m128 (NDS) or $imm: op $imm, rm, vvvv(dst)
	vBlendv                   // op mask, rm, vvvv, reg (mask in imm8[7:4])
	vScalarMov                // vmovss/vmovsd: load, store, or 3-register merge
	vMovLH                    // vmovlps/vmovhps...: m64, vvvv, reg loads; reg, m64 stores
	vExtract                  // op $imm, reg(src), rm(dst)
	vMaskMov                  // load: mem, vvvv(mask), reg; store: reg, vvvv(mask), mem
	vBroadcast                // op xmm/mem, reg
	vZero                     // vzeroupper / vzeroall
)

type vexDef struct {
	pp, mm byte
	op     byte
	store  byte // vMov / vMaskMov / vScalarMov / vMovLH store opcode
	w      bool
	form   vexForm
	ext    int  // vShift: /ext of the immediate form; GP forms: /ext
	iop    byte // vShift: immediate form opcode
	l      int  // -1: from the operands; 0 or 1: fixed
	scalar bool // LIG: always L=0
	pred   bool // a comparison pseudo-instruction: ext is the predicate byte
	mixed  bool // xmm and ymm operands may be combined (conversions, inserts)
}

var vexTable = map[string]*vexDef{}

// isVec tells whether the operand is an xmm or ymm register
func (o aop) isVec() bool { return o.kind == okReg && o.reg.IsXMM() }

func (o aop) isYMM() bool { return o.isVec() && o.size == 32 }

func (o aop) isVecOrMem() bool { return o.isVec() || o.kind == okMem }

func ppOf(pfx []byte) byte {
	if len(pfx) == 0 {
		return 0
	}
	switch pfx[0] {
	case 0x66:
		return 1
	case 0xF3:
		return 2
	case 0xF2:
		return 3
	}
	return 0
}

// splitOp separates the opcode map from the opcode of an SSE definition
func splitOp(op []byte) (mm byte, code byte) {
	if len(op) == 0 {
		return 1, 0
	}
	switch op[0] {
	case 0x38:
		return 2, op[1]
	case 0x3A:
		return 3, op[1]
	}
	return 1, op[0]
}

// vexL computes VEX.L: 1 when a ymm register takes part
func vexL(d *vexDef, ops ...aop) int {
	if d.scalar {
		return 0
	}
	if d.l >= 0 {
		return d.l
	}
	for _, o := range ops {
		if o.isYMM() {
			return 1
		}
	}
	return 0
}

func (t *textAsm) encVEX(pp, mm byte, w bool, l int, vvvv int, op byte, reg int, o aop, trail int) {
	rm, lbl := t.rmOperand(o)
	t.a.encVEX(pp, mm, w, l, vvvv, op, reg, rm, trail)
	if lbl != nil {
		t.a.fixups = append(t.a.fixups, fixup{at: len(t.a.Code) - 4, label: lbl.l, trail: trail, addend: int64(o.mem.Disp)})
	}
}

// vexCheck rejects register sizes the instruction does not have
func vexCheck(d *vexDef, ops []aop) error {
	if d.form == vZero {
		return nil
	}
	hasX, hasY := false, false
	for _, o := range ops {
		if o.isYMM() {
			hasY = true
		} else if o.isVec() {
			hasX = true
		}
	}
	switch {
	case (d.scalar || d.l == 0) && hasY:
		return errOps()
	case d.l == 1 && !hasY:
		return errOps()
	case hasX && hasY && !d.mixed:
		return errOps()
	}
	return nil
}

func instVEX(t *textAsm, mn string, size int, ops []aop) error {
	d := vexTable[mn]
	if d == nil {
		return fmt.Errorf("unknown instruction %s", mn)
	}
	if err := vexCheck(d, ops); err != nil {
		return err
	}
	vreg := func(o aop) int { return o.reg.num() }
	switch d.form {
	case vNDS:
		if err := nops(ops, 3); err != nil {
			return err
		}
		if !ops[0].isVecOrMem() || !ops[1].isVec() || !ops[2].isVec() {
			return errOps()
		}
		if d.pred {
			t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), vreg(ops[1]), d.op, ops[2].reg.num(), ops[0], 1)
			t.a.b(byte(d.ext))
			return nil
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), vreg(ops[1]), d.op, ops[2].reg.num(), ops[0], 0)
	case vNDSI:
		if err := nops(ops, 4); err != nil {
			return err
		}
		if ops[0].kind != okImm || !ops[1].isVecOrMem() || !ops[2].isVec() || !ops[3].isVec() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops[1:]...), vreg(ops[2]), d.op, ops[3].reg.num(), ops[1], 1)
		return t.emitImm(ops[0].val, 1, false)
	case v2:
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !ops[0].isVecOrMem() || !ops[1].isVec() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), 0, d.op, ops[1].reg.num(), ops[0], 0)
	case v2I:
		if err := nops(ops, 3); err != nil {
			return err
		}
		if ops[0].kind != okImm || !ops[1].isVecOrMem() || !ops[2].isVec() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops[1:]...), 0, d.op, ops[2].reg.num(), ops[1], 1)
		return t.emitImm(ops[0].val, 1, false)
	case vMov:
		if err := nops(ops, 2); err != nil {
			return err
		}
		l := vexL(d, ops...)
		switch {
		case ops[0].isVec() && ops[1].isVec():
			// like GNU as: the store form when it allows the 2-byte prefix
			if ops[0].reg.num()&8 != 0 && ops[1].reg.num()&8 == 0 && d.mm == 1 {
				t.encVEX(d.pp, d.mm, d.w, l, 0, d.store, ops[0].reg.num(), ops[1], 0)
				return nil
			}
			t.encVEX(d.pp, d.mm, d.w, l, 0, d.op, ops[1].reg.num(), ops[0], 0)
		case ops[0].kind == okMem && ops[1].isVec():
			t.encVEX(d.pp, d.mm, d.w, l, 0, d.op, ops[1].reg.num(), ops[0], 0)
		case ops[0].isVec() && ops[1].kind == okMem:
			t.encVEX(d.pp, d.mm, d.w, l, 0, d.store, ops[0].reg.num(), ops[1], 0)
		default:
			return errOps()
		}
	case vStore:
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !ops[0].isVec() || ops[1].kind != okMem {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), 0, d.op, ops[0].reg.num(), ops[1], 0)
	case vShift:
		if len(ops) == 3 && ops[0].kind == okImm {
			if !ops[1].isVec() || !ops[2].isVec() {
				return errOps()
			}
			t.encVEX(d.pp, d.mm, d.w, vexL(d, ops[1:]...), vreg(ops[2]), d.iop, d.ext, ops[1], 1)
			return t.emitImm(ops[0].val, 1, false)
		}
		if d.op == 0 {
			return errOps()
		}
		if err := nops(ops, 3); err != nil {
			return err
		}
		if !ops[0].isVecOrMem() || !ops[1].isVec() || !ops[2].isVec() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops[1:]...), vreg(ops[1]), d.op, ops[2].reg.num(), ops[0], 0)
	case vBlendv:
		if err := nops(ops, 4); err != nil {
			return err
		}
		if !ops[0].isVec() || !ops[1].isVecOrMem() || !ops[2].isVec() || !ops[3].isVec() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), vreg(ops[2]), d.op, ops[3].reg.num(), ops[1], 1)
		t.a.b(byte(ops[0].reg.num() << 4))
	case vScalarMov:
		switch {
		case len(ops) == 2 && ops[0].kind == okMem && ops[1].isVec():
			t.encVEX(d.pp, d.mm, false, 0, 0, d.op, ops[1].reg.num(), ops[0], 0)
		case len(ops) == 2 && ops[0].isVec() && ops[1].kind == okMem:
			t.encVEX(d.pp, d.mm, false, 0, 0, d.store, ops[0].reg.num(), ops[1], 0)
		case len(ops) == 3 && ops[0].isVec() && ops[1].isVec() && ops[2].isVec():
			if ops[0].reg.num()&8 != 0 && ops[2].reg.num()&8 == 0 {
				t.encVEX(d.pp, d.mm, false, 0, vreg(ops[1]), d.store, ops[0].reg.num(), ops[2], 0)
				return nil
			}
			t.encVEX(d.pp, d.mm, false, 0, vreg(ops[1]), d.op, ops[2].reg.num(), ops[0], 0)
		default:
			return errOps()
		}
	case vMovLH:
		switch {
		case len(ops) == 3 && ops[0].kind == okMem && ops[1].isVec() && ops[2].isVec():
			t.encVEX(d.pp, d.mm, false, 0, vreg(ops[1]), d.op, ops[2].reg.num(), ops[0], 0)
		case len(ops) == 2 && ops[0].isVec() && ops[1].kind == okMem:
			t.encVEX(d.pp, d.mm, false, 0, 0, d.store, ops[0].reg.num(), ops[1], 0)
		default:
			return errOps()
		}
	case vExtract:
		if err := nops(ops, 3); err != nil {
			return err
		}
		if ops[0].kind != okImm || !ops[1].isVec() || !ops[2].isVecOrMem() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops[1:]...), 0, d.op, ops[1].reg.num(), ops[2], 1)
		return t.emitImm(ops[0].val, 1, false)
	case vMaskMov:
		if err := nops(ops, 3); err != nil {
			return err
		}
		switch {
		case ops[0].kind == okMem && ops[1].isVec() && ops[2].isVec():
			t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), vreg(ops[1]), d.op, ops[2].reg.num(), ops[0], 0)
		case ops[0].isVec() && ops[1].isVec() && ops[2].kind == okMem:
			t.encVEX(d.pp, d.mm, d.w, vexL(d, ops...), vreg(ops[1]), d.store, ops[0].reg.num(), ops[2], 0)
		default:
			return errOps()
		}
	case vBroadcast:
		if err := nops(ops, 2); err != nil {
			return err
		}
		if !ops[0].isVecOrMem() || !ops[1].isVec() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, d.w, vexL(d, ops[1]), 0, d.op, ops[1].reg.num(), ops[0], 0)
	case vZero:
		if err := nops(ops, 0); err != nil {
			return err
		}
		t.a.b(0xC5, 0xF8|byte(d.l)<<2, 0x77)
	}
	return nil
}

// ---- general-purpose VEX instructions (BMI1, BMI2) ----

type gpVexForm int

const (
	gNDS  gpVexForm = iota // andn, pdep, pext, mulx: rm, vvvv, reg
	gRMV                   // bextr, bzhi, sarx, shlx, shrx: vvvv, rm, reg (AT&T: src2, rm, dst)
	gExt                   // blsr, blsmsk, blsi: rm, vvvv(dst) with /ext
	gRorx                  // rorx $imm, rm, reg
)

type gpVexDef struct {
	pp, mm byte
	op     byte
	ext    int
	form   gpVexForm
}

func instGPVEX(t *textAsm, mn string, size int, ops []aop, d gpVexDef) error {
	var dst aop
	switch d.form {
	case gRorx:
		if err := nops(ops, 3); err != nil {
			return err
		}
		dst = ops[2]
		if ops[0].kind != okImm || !ops[1].isRM() || !dst.isGP() || dst.size < 4 {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, dst.size == 8, 0, 0, d.op, dst.reg.num(), ops[1], 1)
		return t.emitImm(ops[0].val, 1, false)
	case gExt:
		if err := nops(ops, 2); err != nil {
			return err
		}
		dst = ops[1]
		if !ops[0].isRM() || !dst.isGP() || dst.size < 4 {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, dst.size == 8, 0, dst.reg.num(), d.op, d.ext, ops[0], 0)
		return nil
	}
	if err := nops(ops, 3); err != nil {
		return err
	}
	dst = ops[2]
	if !dst.isGP() || dst.size < 4 {
		return errOps()
	}
	w := dst.size == 8
	switch d.form {
	case gNDS:
		if !ops[0].isRM() || !ops[1].isGP() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, w, 0, ops[1].reg.num(), d.op, dst.reg.num(), ops[0], 0)
	case gRMV:
		if !ops[0].isGP() || !ops[1].isRM() {
			return errOps()
		}
		t.encVEX(d.pp, d.mm, w, 0, ops[0].reg.num(), d.op, dst.reg.num(), ops[1], 0)
	}
	return nil
}

// ---- the table ----

func init() {
	for i := 0; i < 16; i++ {
		gpRegs[fmt.Sprintf("ymm%d", i)] = asmReg{XMM0 + Reg(i), 32}
	}
	add := func(n string, d *vexDef) {
		vexTable[n] = d
		insts[n] = def(instVEX, false, 0)
	}
	// unary or two-operand SSE instructions keep two operands under VEX
	twoOp := map[string]bool{
		"sqrtps": true, "sqrtpd": true, "rcpps": true, "rsqrtps": true, "ucomiss": true, "ucomisd": true,
		"comiss": true, "comisd": true, "cvtdq2ps": true, "cvtps2dq": true, "cvttps2dq": true, "cvtdq2pd": true,
		"cvtpd2dq": true, "cvttpd2dq": true, "cvtps2pd": true, "cvtpd2ps": true, "movddup": true,
		"movshdup": true, "movsldup": true, "pshufd": true, "pshufhw": true, "pshuflw": true, "ptest": true,
		"pabsb": true, "pabsw": true, "pabsd": true, "phminposuw": true, "aesimc": true,
		"aeskeygenassist": true, "pcmpestri": true, "pcmpestrm": true, "pcmpistri": true, "pcmpistrm": true,
		"lddqu": true, "movntdqa": true, "roundps": true, "roundpd": true,
		"pmovsxbw": true, "pmovsxbd": true, "pmovsxbq": true, "pmovsxwd": true, "pmovsxwq": true, "pmovsxdq": true,
		"pmovzxbw": true, "pmovzxbd": true, "pmovzxbq": true, "pmovzxwd": true, "pmovzxwq": true, "pmovzxdq": true,
	}
	xmmOnly := map[string]bool{"aesimc": true,
		"aeskeygenassist": true, "phminposuw": true, "pcmpestri": true, "pcmpestrm": true,
		"pcmpistri": true, "pcmpistrm": true, "movhlps": true, "movlhps": true, "insertps": true}
	mixedSizes := map[string]bool{"cvtps2pd": true, "cvtdq2pd": true, "cvtpd2ps": true, "cvtpd2dq": true, "cvttpd2dq": true,
		"pmovsxbw": true, "pmovsxbd": true, "pmovsxbq": true, "pmovsxwd": true, "pmovsxwq": true, "pmovsxdq": true,
		"pmovzxbw": true, "pmovzxbd": true, "pmovzxbq": true, "pmovzxwd": true, "pmovzxwq": true, "pmovzxdq": true}
	noVEX := map[string]bool{"sha1nexte": true, "sha1msg1": true, "sha1msg2": true, "sha256rnds2": true,
		"sha256msg1": true, "sha256msg2": true, "sha1rnds4": true, "movsd": false}
	for n, sd := range sseTable {
		if noVEX[n] {
			continue
		}
		mm, op := splitOp(sd.op)
		d := &vexDef{pp: ppOf(sd.pfx), mm: mm, op: op, l: -1}
		last := n[len(n)-2:]
		// scalar floating-point instructions ignore VEX.L (packed integer
		// ones like pminsd start with p)
		d.scalar = (last == "ss" || last == "sd") && !strings.HasPrefix(n, "p")
		switch sd.kind {
		case sseRM:
			switch {
			case n == "pblendvb" || n == "blendvps" || n == "blendvpd":
				d.mm, d.form = 3, vBlendv
				d.op = map[string]byte{"pblendvb": 0x4C, "blendvps": 0x4A, "blendvpd": 0x4B}[n]
			case twoOp[n]:
				d.form = v2
			case n == "movhlps" || n == "movlhps":
				d.form = vNDS
			default:
				d.form = vNDS
			}
		case sseRMI:
			d.form = vNDSI
			if twoOp[n] {
				d.form = v2I
			}
		case sseMov:
			d.store = sd.store
			switch n {
			case "movss", "movsd":
				d.form = vScalarMov
			case "movlps", "movlpd", "movhps", "movhpd":
				d.form = vMovLH
			default:
				d.form = vMov
			}
		case sseStore:
			d.form = vStore
		case sseShift:
			d.form = vShift
			d.iop, d.ext = sd.iop, sd.ext
			if sd.op == nil {
				d.op = 0
			}
		}
		if n == "ucomiss" || n == "ucomisd" || n == "comiss" || n == "comisd" {
			d.scalar = true
		}
		if xmmOnly[n] {
			d.l = 0
		}
		d.mixed = mixedSizes[n] || strings.HasPrefix(n, "cvt") || sd.kind == sseShift
		add("v"+n, d)
	}
	// SSE comparison pseudo-instructions: 32 predicates under AVX
	preds := []string{"eq", "lt", "le", "unord", "neq", "nlt", "nle", "ord", "eq_uq", "nge", "ngt", "false",
		"neq_oq", "ge", "gt", "true", "eq_os", "lt_oq", "le_oq", "unord_s", "neq_us", "nlt_uq", "nle_uq", "ord_s",
		"eq_us", "nge_uq", "ngt_uq", "false_os", "neq_os", "ge_oq", "gt_oq", "true_us"}
	for _, f := range []struct {
		sfx string
		pp  byte
	}{{"ps", 0}, {"pd", 1}, {"ss", 2}, {"sd", 3}} {
		for i, p := range preds {
			add("vcmp"+p+f.sfx, &vexDef{pp: f.pp, mm: 1, op: 0xC2, l: -1, scalar: f.sfx == "ss" || f.sfx == "sd", form: vNDS, ext: i, pred: true})
		}
	}
	// AVX-only instructions
	m := func(n string, pp, mm, op byte, form vexForm, w bool, l int) {
		d := &vexDef{pp: pp, mm: mm, op: op, form: form, w: w, l: l}
		switch form {
		case vBroadcast, vExtract:
			d.mixed = true
		}
		if n == "vinsertf128" || n == "vinserti128" || n == "vcvtph2ps" {
			d.mixed = true
		}
		add(n, d)
	}
	m("vbroadcastss", 1, 2, 0x18, vBroadcast, false, -1)
	m("vbroadcastsd", 1, 2, 0x19, vBroadcast, false, 1)
	m("vbroadcastf128", 1, 2, 0x1A, vBroadcast, false, 1)
	m("vbroadcasti128", 1, 2, 0x5A, vBroadcast, false, 1)
	m("vpbroadcastb", 1, 2, 0x78, vBroadcast, false, -1)
	m("vpbroadcastw", 1, 2, 0x79, vBroadcast, false, -1)
	m("vpbroadcastd", 1, 2, 0x58, vBroadcast, false, -1)
	m("vpbroadcastq", 1, 2, 0x59, vBroadcast, false, -1)
	m("vextractf128", 1, 3, 0x19, vExtract, false, 1)
	m("vextracti128", 1, 3, 0x39, vExtract, false, 1)
	m("vinsertf128", 1, 3, 0x18, vNDSI, false, 1)
	m("vinserti128", 1, 3, 0x38, vNDSI, false, 1)
	m("vperm2f128", 1, 3, 0x06, vNDSI, false, 1)
	m("vperm2i128", 1, 3, 0x46, vNDSI, false, 1)
	m("vpermq", 1, 3, 0x00, v2I, true, 1)
	m("vpermpd", 1, 3, 0x01, v2I, true, 1)
	m("vpermd", 1, 2, 0x36, vNDS, false, 1)
	m("vpermps", 1, 2, 0x16, vNDS, false, 1)
	m("vpermilps", 1, 2, 0x0C, vNDS, false, -1)
	m("vpermilpd", 1, 2, 0x0D, vNDS, false, -1)
	m("vpblendd", 1, 3, 0x02, vNDSI, false, -1)
	m("vpsllvd", 1, 2, 0x47, vNDS, false, -1)
	m("vpsllvq", 1, 2, 0x47, vNDS, true, -1)
	m("vpsrlvd", 1, 2, 0x45, vNDS, false, -1)
	m("vpsrlvq", 1, 2, 0x45, vNDS, true, -1)
	m("vpsravd", 1, 2, 0x46, vNDS, false, -1)
	m("vtestps", 1, 2, 0x0E, v2, false, -1)
	m("vtestpd", 1, 2, 0x0F, v2, false, -1)
	m("vcvtph2ps", 1, 2, 0x13, v2, false, -1)
	m("vcvtps2ph", 1, 3, 0x1D, vExtract, false, -1)
	vexTable["vmaskmovps"] = &vexDef{pp: 1, mm: 2, op: 0x2C, store: 0x2E, form: vMaskMov, l: -1}
	vexTable["vmaskmovpd"] = &vexDef{pp: 1, mm: 2, op: 0x2D, store: 0x2F, form: vMaskMov, l: -1}
	vexTable["vpmaskmovd"] = &vexDef{pp: 1, mm: 2, op: 0x8C, store: 0x8E, form: vMaskMov, l: -1}
	vexTable["vpmaskmovq"] = &vexDef{pp: 1, mm: 2, op: 0x8C, store: 0x8E, form: vMaskMov, l: -1, w: true}
	for _, n := range []string{"vmaskmovps", "vmaskmovpd", "vpmaskmovd", "vpmaskmovq"} {
		insts[n] = def(instVEX, false, 0)
	}
	add("vzeroupper", &vexDef{form: vZero, l: 0})
	add("vzeroall", &vexDef{form: vZero, l: 1})
	// permilps/pd with an immediate use another opcode
	insts["vpermilps"] = def(func(t *textAsm, mn string, size int, ops []aop) error {
		if len(ops) == 3 && ops[0].kind == okImm {
			vexTable["vpermilps$i"] = &vexDef{pp: 1, mm: 3, op: 0x04, form: v2I, l: -1}
			return instVEX(t, "vpermilps$i", size, ops)
		}
		return instVEX(t, mn, size, ops)
	}, false, 0)
	insts["vpermilpd"] = def(func(t *textAsm, mn string, size int, ops []aop) error {
		if len(ops) == 3 && ops[0].kind == okImm {
			vexTable["vpermilpd$i"] = &vexDef{pp: 1, mm: 3, op: 0x05, form: v2I, l: -1}
			return instVEX(t, "vpermilpd$i", size, ops)
		}
		return instVEX(t, mn, size, ops)
	}, false, 0)
	// AVX2 shifts by a vector count reuse the SSE opcodes; the immediate
	// forms are in the vShift entries
	// FMA
	for i, kind := range []string{"fmaddsub", "fmsubadd", "fmadd", "fmsub", "fnmadd", "fnmsub"} {
		base := byte(0x96 + i)
		if i >= 2 {
			base = byte(0x98 + 2*(i-2))
		}
		for j, order := range []string{"132", "213", "231"} {
			op := base + byte(j*0x10)
			add("v"+kind+order+"ps", &vexDef{pp: 1, mm: 2, op: op, form: vNDS, l: -1})
			add("v"+kind+order+"pd", &vexDef{pp: 1, mm: 2, op: op, form: vNDS, l: -1, w: true})
			if i >= 2 {
				add("v"+kind+order+"ss", &vexDef{pp: 1, mm: 2, op: op + 1, form: vNDS, scalar: true})
				add("v"+kind+order+"sd", &vexDef{pp: 1, mm: 2, op: op + 1, form: vNDS, scalar: true, w: true})
			}
		}
	}
	// moves between general-purpose and vector registers, inserts and
	// extracts, conversions with integer operands, mask moves
	insts["vmovd"] = def(instVMovdq, false, 0)
	insts["vmovq"] = def(instVMovdq, false, 0)
	for n, op := range map[string]byte{"vpmovmskb": 0xD7, "vmovmskps": 0x50, "vmovmskpd": 0x50} {
		pp := byte(1)
		if n == "vmovmskps" {
			pp = 0
		}
		insts[n] = def(func(t *textAsm, mn string, size int, ops []aop) error {
			if err := nops(ops, 2); err != nil {
				return err
			}
			if !ops[0].isVec() || !ops[1].isGP() || ops[1].size < 4 {
				return errOps()
			}
			t.encVEX(pp, 1, false, vexL(&vexDef{l: -1}, ops[0]), 0, op, ops[1].reg.num(), ops[0], 0)
			return nil
		}, false, 0)
	}
	for n, x := range map[string]struct {
		mm, op byte
		w      bool
	}{"vpextrb": {3, 0x14, false}, "vpextrw": {3, 0x15, false}, "vpextrd": {3, 0x16, false}, "vpextrq": {3, 0x16, true}, "vextractps": {3, 0x17, false}} {
		insts[n] = def(func(t *textAsm, mn string, size int, ops []aop) error {
			if err := nops(ops, 3); err != nil {
				return err
			}
			if ops[0].kind != okImm || !ops[1].isVec() || !ops[2].isRM() {
				return errOps()
			}
			if mn == "vpextrw" && ops[2].isGP() {
				t.encVEX(1, 1, false, 0, 0, 0xC5, ops[2].reg.num(), ops[1], 1)
				return t.emitImm(ops[0].val, 1, false)
			}
			t.encVEX(1, x.mm, x.w, 0, 0, x.op, ops[1].reg.num(), ops[2], 1)
			return t.emitImm(ops[0].val, 1, false)
		}, false, 0)
	}
	for n, x := range map[string]struct {
		mm, op byte
		w      bool
	}{"vpinsrb": {3, 0x20, false}, "vpinsrw": {1, 0xC4, false}, "vpinsrd": {3, 0x22, false}, "vpinsrq": {3, 0x22, true}, "vinsertps": {3, 0x21, false}} {
		insts[n] = def(func(t *textAsm, mn string, size int, ops []aop) error {
			if err := nops(ops, 4); err != nil {
				return err
			}
			if ops[0].kind != okImm || !(ops[1].isRM() || ops[1].isVec()) || !ops[2].isVec() || !ops[3].isVec() {
				return errOps()
			}
			t.encVEX(1, x.mm, x.w, 0, ops[2].reg.num(), x.op, ops[3].reg.num(), ops[1], 1)
			return t.emitImm(ops[0].val, 1, false)
		}, false, 0)
	}
	for n, x := range map[string]struct{ pp, op byte }{"vcvtsi2ss": {2, 0x2A}, "vcvtsi2sd": {3, 0x2A}} {
		insts[n] = def(func(t *textAsm, mn string, size int, ops []aop) error {
			if err := nops(ops, 3); err != nil {
				return err
			}
			if !ops[0].isRM() || !ops[1].isVec() || !ops[2].isVec() {
				return errOps()
			}
			if size == 0 {
				size = 4
				if ops[0].isGP() {
					size = ops[0].size
				}
			}
			t.encVEX(x.pp, 1, size == 8, 0, ops[1].reg.num(), x.op, ops[2].reg.num(), ops[0], 0)
			return nil
		}, true, 0)
	}
	for n, x := range map[string]struct{ pp, op byte }{"vcvttss2si": {2, 0x2C}, "vcvttsd2si": {3, 0x2C}, "vcvtss2si": {2, 0x2D}, "vcvtsd2si": {3, 0x2D}} {
		insts[n] = def(func(t *textAsm, mn string, size int, ops []aop) error {
			if err := nops(ops, 2); err != nil {
				return err
			}
			if !ops[0].isVecOrMem() || !ops[1].isGP() || ops[1].size < 4 {
				return errOps()
			}
			t.encVEX(x.pp, 1, ops[1].size == 8, 0, 0, x.op, ops[1].reg.num(), ops[0], 0)
			return nil
		}, true, 0)
	}
	// BMI1 / BMI2
	gp := func(n string, d gpVexDef) {
		insts[n] = def(func(t *textAsm, mn string, size int, ops []aop) error {
			return instGPVEX(t, mn, size, ops, d)
		}, true, 0)
	}
	gp("andn", gpVexDef{0, 2, 0xF2, 0, gNDS})
	gp("bextr", gpVexDef{0, 2, 0xF7, 0, gRMV})
	gp("blsr", gpVexDef{0, 2, 0xF3, 1, gExt})
	gp("blsmsk", gpVexDef{0, 2, 0xF3, 2, gExt})
	gp("blsi", gpVexDef{0, 2, 0xF3, 3, gExt})
	gp("bzhi", gpVexDef{0, 2, 0xF5, 0, gRMV})
	gp("pdep", gpVexDef{3, 2, 0xF5, 0, gNDS})
	gp("pext", gpVexDef{2, 2, 0xF5, 0, gNDS})
	gp("mulx", gpVexDef{3, 2, 0xF6, 0, gNDS})
	gp("sarx", gpVexDef{2, 2, 0xF7, 0, gRMV})
	gp("shlx", gpVexDef{1, 2, 0xF7, 0, gRMV})
	gp("shrx", gpVexDef{3, 2, 0xF7, 0, gRMV})
	gp("rorx", gpVexDef{3, 3, 0xF0, 0, gRorx})
}

// vmovd / vmovq
func instVMovdq(t *textAsm, mn string, size int, ops []aop) error {
	if err := nops(ops, 2); err != nil {
		return err
	}
	src, dst := ops[0], ops[1]
	q := mn == "vmovq"
	switch {
	case dst.isVec() && src.isGP():
		t.encVEX(1, 1, src.size == 8, 0, 0, 0x6E, dst.reg.num(), src, 0)
	case src.isVec() && dst.isGP():
		t.encVEX(1, 1, dst.size == 8, 0, 0, 0x7E, src.reg.num(), dst, 0)
	case q && dst.isVec() && src.isVecOrMem():
		t.encVEX(2, 1, false, 0, 0, 0x7E, dst.reg.num(), src, 0)
	case q && src.isVec() && dst.kind == okMem:
		t.encVEX(1, 1, false, 0, 0, 0xD6, src.reg.num(), dst, 0)
	case !q && dst.isVec() && src.kind == okMem:
		t.encVEX(1, 1, false, 0, 0, 0x6E, dst.reg.num(), src, 0)
	case !q && src.isVec() && dst.kind == okMem:
		t.encVEX(1, 1, false, 0, 0, 0x7E, src.reg.num(), dst, 0)
	default:
		return errOps()
	}
	return nil
}

// Text assembles one AT&T instruction (no labels or jumps) into a; the
// code generator uses it for vector instructions
func (a *Asm) Text(line string) error {
	env := &TextEnv{Section: func(string, string) (*Asm, error) { return a, nil }}
	t := &textAsm{env: env, a: a, secName: ".text", numBack: map[string]labelRef{}, numFwd: map[string]labelRef{},
		defNames: map[string]bool{}, labelSec: map[string]string{}, globals: map[string]bool{}, secFlags: map[string]string{}}
	t.prev = secEntry{t.secName, a}
	if a.Listing != nil {
		a.Listing.WriteString("\t" + line + "\n")
	}
	saved := a.Listing
	a.Listing = nil
	err := t.stmt(line)
	a.Listing = saved
	if err != nil {
		return fmt.Errorf("%s: %v", line, err)
	}
	return nil
}
