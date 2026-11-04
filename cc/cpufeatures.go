package cc

// CPUFeatures lists the processor features known to __builtin_cpu_supports
// with their bit numbers in libgcc's processor model (gcc's enum
// processor_features): bits 0-31 are in __cpu_model.__cpu_features[0], the
// others in __cpu_features2[(bit-32)/32]. Code compiled by AltC and by gcc
// shares that model, so objects of both work with either runtime.
var CPUFeatures = []struct {
	Name string
	Bit  int
}{
	{"cmov", 0}, {"mmx", 1}, {"popcnt", 2}, {"sse", 3}, {"sse2", 4}, {"sse3", 5}, {"ssse3", 6}, {"sse4.1", 7},
	{"sse4.2", 8}, {"avx", 9}, {"avx2", 10}, {"sse4a", 11}, {"fma4", 12}, {"xop", 13}, {"fma", 14}, {"avx512f", 15},
	{"bmi", 16}, {"bmi2", 17}, {"aes", 18}, {"pclmul", 19}, {"avx512vl", 20}, {"avx512bw", 21}, {"avx512dq", 22},
	{"avx512cd", 23}, {"avx512vbmi", 26}, {"avx512ifma", 27}, {"avx512vpopcntdq", 30}, {"avx512vbmi2", 31},
	{"gfni", 32}, {"vpclmulqdq", 33}, {"avx512vnni", 34}, {"avx512bitalg", 35}, {"adx", 40}, {"abm", 41},
	{"f16c", 49}, {"lzcnt", 57}, {"movbe", 58}, {"rdrnd", 69}, {"rdseed", 70}, {"sha", 74}, {"vaes", 78},
	{"x86-64", 95}, {"x86-64-v2", 96}, {"x86-64-v3", 97}, {"x86-64-v4", 98},
}

// CPUFeatureBit finds the bit of a feature name
func CPUFeatureBit(name string) (int, bool) {
	for _, f := range CPUFeatures {
		if f.Name == name {
			return f.Bit, true
		}
	}
	return 0, false
}

// CPU vendors in __cpu_model.__cpu_vendor
var cpuVendors = map[string]int64{"intel": 1, "amd": 2}

// cpuModelVar declares one of libgcc's processor model variables under an
// internal name: an array of unsigned ints, the layout of both
func (p *parser) cpuModelVar(asm string, tok *Token) *Obj {
	name := "__altc_ref" + asm
	o := p.globalsByName[name]
	if o == nil {
		o = &Obj{Name: name, AsmName: asm, Ty: arrayOf(tyUInt, 4), Tok: tok, IsExtern: true}
		p.globalsByName[name] = o
		p.prog.Globals = append(p.prog.Globals, o)
	}
	p.markUsed(o)
	return o
}

// cpuWord reads word i of a processor model variable
func (p *parser) cpuWord(asm string, i int64, tok *Token) *Node {
	v := newVarNode(p.cpuModelVar(asm, tok), tok)
	return newUnary(NDeref, p.newAdd(v, newNum(i, tok), tok), tok)
}

// cpuBuiltin lowers __builtin_cpu_supports, __builtin_cpu_is and
// __builtin_cpu_init like gcc does: they read libgcc's processor model,
// filled by its constructor __cpu_indicator_init
func (p *parser) cpuBuiltin(name string, t *Token) *Node {
	if name == "__builtin_cpu_init" {
		p.args(0)
		o := p.globalsByName["__cpu_indicator_init"]
		if o == nil {
			o = &Obj{Name: "__cpu_indicator_init", AsmName: "__cpu_indicator_init", Ty: funcType(tyInt), Tok: t, IsFunction: true}
			p.globalsByName[o.Name] = o
			p.prog.Globals = append(p.prog.Globals, o)
		}
		p.markUsed(o)
		fn := newVarNode(o, t)
		addType(fn)
		call := &Node{Kind: NFuncall, LHS: fn, FuncTy: o.Ty, Tok: t, Ty: tyInt}
		return newCast(call, tyVoid)
	}
	p.expect("(")
	st := p.next()
	if st.Kind != TStr {
		p.errorAt(st, "the argument of '%s' must be a string literal", name)
		p.expect(")")
		return newNum(0, t)
	}
	p.expect(")")
	_, arg := literalPrefix(st.Text)
	if name == "__builtin_cpu_is" {
		vendor, ok := cpuVendors[arg]
		if !ok {
			// processor types and subtypes are not modelled
			return newNum(0, t)
		}
		n := p.newBinary(NEq, p.cpuWord("__cpu_model", 0, t), newNum(vendor, t), t)
		addType(n)
		return n
	}
	bit, ok := CPUFeatureBit(arg)
	if !ok {
		p.errorAt(st, "parameter to builtin not valid: %s", arg)
		return newNum(0, t)
	}
	var w *Node
	if bit < 32 {
		w = p.cpuWord("__cpu_model", 3, t)
	} else {
		w = p.cpuWord("__cpu_features2", int64(bit-32)/32, t)
		bit = (bit - 32) % 32
	}
	var n *Node
	if bit == 31 {
		// gcc gives 1, not the sign bit
		n = p.newBinary(NShr, w, newNum(31, t), t)
	} else {
		n = p.newBinary(NBitAnd, w, &Node{Kind: NNum, Val: 1 << bit, Tok: t, Ty: tyUInt}, t)
	}
	n = newCast(n, tyInt)
	addType(n)
	return n
}
