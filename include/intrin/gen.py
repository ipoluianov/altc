#!/usr/bin/env python3
"""Generates the x86 intrinsic headers of include/c from compact tables.

Intrinsics are always_inline functions (the IR backend inlines them)
built on two compiler builtins:

  __builtin_altc_insn(*(T *)0, "mnemonic", operands...)
  __builtin_altc_insni(*(T *)0, "mnemonic", imm, operands...)

which emit one SSE/AVX instruction with the operands in intrinsic order
(the first is the destination-merged operand of the legacy two-operand
form) and give a result of type T. Intrinsics taking an immediate are
macros, because the immediate must be a constant expression.

usage: python3 include/intrin/gen.py   (writes include/c/*intrin.h)
"""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "c")
files = {}
cur = None


protos = None


def begin(name, guard, includes=()):
    global cur, protos
    cur = []
    protos = []
    files[name] = (guard, includes, cur, protos)


def emit(s):
    cur.append(s)


INL = "__ALTC_INL"


def fn(ret, name, params, body):
    ps = ", ".join(f"{t} {n}" for t, n in params) if params else "void"
    protos.append(f"{INL} {ret} {name}({ps});")
    emit(f"{INL} {ret} {name}({ps}) {{ {body} }}")


def V(T, mn, *args):
    if args:
        return f'__builtin_altc_insn(*({T} *)0, "{mn}", {", ".join(args)})'
    return f'__builtin_altc_insn(*({T} *)0, "{mn}")'


def VI(T, mn, imm, *args):
    return f'__builtin_altc_insni(*({T} *)0, "{mn}", {imm}, {", ".join(args)})'


def un(name, T, mn, A=None):
    fn(T, name, [(A or T, "__a")], f"return {V(T, mn, '__a')};")


def bin(name, T, mn, A=None, B=None):
    fn(T, name, [(A or T, "__a"), (B or A or T, "__b")], f"return {V(T, mn, '__a', '__b')};")


def binswap(name, T, mn):
    fn(T, name, [(T, "__a"), (T, "__b")], f"return {V(T, mn, '__b', '__a')};")


def macro(name, params, body):
    emit(f"#define {name}({', '.join(params)}) ({body})")


def unI(name, T, mn, A=None):
    macro(name, ["a", "imm"], VI(T, mn, "(imm)", f"({A or T})(a)"))


def binI(name, T, mn, A=None, B=None):
    macro(name, ["a", "b", "imm"], VI(T, mn, "(imm)", f"({A or T})(a)", f"({B or A or T})(b)"))


def toGPR(name, R, mn, A):
    fn(R, name, [(A, "__a")], f"return {V(R, mn, '__a')};")


# ---------------------------------------------------------------- common
begin("altc_intrin_common.h", "__ALTC_INTRIN_COMMON_H")
emit("""#define __ALTC_INL static __inline__ __attribute__((__always_inline__, __unused__))

typedef float __m128 __attribute__((__vector_size__(16), __may_alias__));
typedef float __m128_u __attribute__((__vector_size__(16), __may_alias__, __aligned__(1)));
typedef double __m128d __attribute__((__vector_size__(16), __may_alias__));
typedef double __m128d_u __attribute__((__vector_size__(16), __may_alias__, __aligned__(1)));
typedef long long __m128i __attribute__((__vector_size__(16), __may_alias__));
typedef long long __m128i_u __attribute__((__vector_size__(16), __may_alias__, __aligned__(1)));
typedef float __m256 __attribute__((__vector_size__(32), __may_alias__));
typedef float __m256_u __attribute__((__vector_size__(32), __may_alias__, __aligned__(1)));
typedef double __m256d __attribute__((__vector_size__(32), __may_alias__));
typedef double __m256d_u __attribute__((__vector_size__(32), __may_alias__, __aligned__(1)));
typedef long long __m256i __attribute__((__vector_size__(32), __may_alias__));
typedef long long __m256i_u __attribute__((__vector_size__(32), __may_alias__, __aligned__(1)));

typedef float __v4sf __attribute__((__vector_size__(16)));
typedef double __v2df __attribute__((__vector_size__(16)));
typedef long long __v2di __attribute__((__vector_size__(16)));
typedef unsigned long long __v2du __attribute__((__vector_size__(16)));
typedef int __v4si __attribute__((__vector_size__(16)));
typedef unsigned int __v4su __attribute__((__vector_size__(16)));
typedef short __v8hi __attribute__((__vector_size__(16)));
typedef unsigned short __v8hu __attribute__((__vector_size__(16)));
typedef char __v16qi __attribute__((__vector_size__(16)));
typedef signed char __v16qs __attribute__((__vector_size__(16)));
typedef unsigned char __v16qu __attribute__((__vector_size__(16)));
typedef float __v8sf __attribute__((__vector_size__(32)));
typedef double __v4df __attribute__((__vector_size__(32)));
typedef long long __v4di __attribute__((__vector_size__(32)));
typedef unsigned long long __v4du __attribute__((__vector_size__(32)));
typedef int __v8si __attribute__((__vector_size__(32)));
typedef unsigned int __v8su __attribute__((__vector_size__(32)));
typedef short __v16hi __attribute__((__vector_size__(32)));
typedef unsigned short __v16hu __attribute__((__vector_size__(32)));
typedef char __v32qi __attribute__((__vector_size__(32)));
typedef signed char __v32qs __attribute__((__vector_size__(32)));
typedef unsigned char __v32qu __attribute__((__vector_size__(32)));

/* unaligned scalar access */
struct __altc_u16 { short __v; } __attribute__((__packed__, __may_alias__));
struct __altc_u32 { int __v; } __attribute__((__packed__, __may_alias__));
struct __altc_u64 { long long __v; } __attribute__((__packed__, __may_alias__));
struct __altc_f32 { float __v; } __attribute__((__packed__, __may_alias__));
struct __altc_f64 { double __v; } __attribute__((__packed__, __may_alias__));

#define __ALTC_SPLAT(T, x) __builtin_altc_broadcast(*(T *)0, (x))""")

# ---------------------------------------------------------------- SSE
begin("xmmintrin.h", "_XMMINTRIN_H_INCLUDED", ["altc_intrin_common.h", "mmintrin.h", "mm_malloc.h"])
emit("""#define _MM_SHUFFLE(fp3, fp2, fp1, fp0) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))
#define _MM_HINT_T0 3
#define _MM_HINT_T1 2
#define _MM_HINT_T2 1
#define _MM_HINT_NTA 0
#define _mm_prefetch(p, i) __builtin_prefetch((const void *)(p), 0, (i))
#define _MM_EXCEPT_MASK 0x003f
#define _MM_MASK_MASK 0x1f80
#define _MM_ROUND_MASK 0x6000
#define _MM_ROUND_NEAREST 0x0000
#define _MM_ROUND_DOWN 0x2000
#define _MM_ROUND_UP 0x4000
#define _MM_ROUND_TOWARD_ZERO 0x6000
#define _MM_FLUSH_ZERO_MASK 0x8000
#define _MM_FLUSH_ZERO_ON 0x8000
#define _MM_FLUSH_ZERO_OFF 0x0000""")
T = "__m128"
for op in ["add", "sub", "mul", "div", "min", "max"]:
    bin(f"_mm_{op}_ps", T, f"{op}ps")
    bin(f"_mm_{op}_ss", T, f"{op}ss")
for op in ["sqrt", "rcp", "rsqrt"]:
    un(f"_mm_{op}_ps", T, f"{op}ps")
    un(f"_mm_{op}_ss", T, f"{op}ss")
bin("_mm_and_ps", T, "andps")
bin("_mm_andnot_ps", T, "andnps")
bin("_mm_or_ps", T, "orps")
bin("_mm_xor_ps", T, "xorps")
preds = {"eq": 0, "lt": 1, "le": 2, "unord": 3, "neq": 4, "nlt": 5, "nle": 6, "ord": 7}
for sfx in ["ps", "ss"]:
    for p, i in preds.items():
        fn(T, f"_mm_cmp{p}_{sfx}", [(T, "__a"), (T, "__b")], f"return {VI(T, 'cmp' + sfx, i, '__a', '__b')};")
    for p, q in [("gt", 1), ("ge", 2), ("ngt", 5), ("nge", 6)]:
        if sfx == "ps":
            fn(T, f"_mm_cmp{p}_ps", [(T, "__a"), (T, "__b")], f"return {VI(T, 'cmpps', q, '__b', '__a')};")
        else:
            fn(T, f"_mm_cmp{p}_ss", [(T, "__a"), (T, "__b")], f"return _mm_move_ss(__a, {VI(T, 'cmpss', q, '__b', '__a')});")
for p, op in [("eq", "=="), ("lt", "<"), ("le", "<="), ("gt", ">"), ("ge", ">="), ("neq", "!=")]:
    for c in ["comi", "ucomi"]:
        fn("int", f"_mm_{c}{p}_ss", [(T, "__a"), (T, "__b")], f"return _mm_cvtss_f32(__a) {op} _mm_cvtss_f32(__b);")
toGPR("_mm_cvtss_si32", "int", "cvtss2si", T)
toGPR("_mm_cvt_ss2si", "int", "cvtss2si", T)
toGPR("_mm_cvttss_si32", "int", "cvttss2si", T)
toGPR("_mm_cvtt_ss2si", "int", "cvttss2si", T)
toGPR("_mm_cvtss_si64", "long long", "cvtss2si", T)
toGPR("_mm_cvtss_si64x", "long long", "cvtss2si", T)
toGPR("_mm_cvttss_si64", "long long", "cvttss2si", T)
toGPR("_mm_cvttss_si64x", "long long", "cvttss2si", T)
fn(T, "_mm_cvtsi32_ss", [(T, "__a"), ("int", "__b")], f"return {V(T, 'cvtsi2ss', '__a', '__b')};")
fn(T, "_mm_cvt_si2ss", [(T, "__a"), ("int", "__b")], f"return {V(T, 'cvtsi2ss', '__a', '__b')};")
fn(T, "_mm_cvtsi64_ss", [(T, "__a"), ("long long", "__b")], f"return {V(T, 'cvtsi2ss', '__a', '__b')};")
fn(T, "_mm_cvtsi64x_ss", [(T, "__a"), ("long long", "__b")], f"return {V(T, 'cvtsi2ss', '__a', '__b')};")
fn("float", "_mm_cvtss_f32", [(T, "__a")], f"return {V('float', '', '__a')};")
binI("_mm_shuffle_ps", T, "shufps")
bin("_mm_unpackhi_ps", T, "unpckhps")
bin("_mm_unpacklo_ps", T, "unpcklps")
bin("_mm_movehl_ps", T, "movhlps")
bin("_mm_movelh_ps", T, "movlhps")
bin("_mm_move_ss", T, "movss")
toGPR("_mm_movemask_ps", "int", "movmskps", T)
fn(T, "_mm_setzero_ps", [], f"return {V(T, 'xorps')};")
fn(T, "_mm_undefined_ps", [], f"return {V(T, 'xorps')};")
fn(T, "_mm_set1_ps", [("float", "__f")], "return (__m128)__ALTC_SPLAT(__v4sf, __f);")
fn(T, "_mm_set_ps1", [("float", "__f")], "return (__m128)__ALTC_SPLAT(__v4sf, __f);")
fn(T, "_mm_set_ss", [("float", "__f")], "return (__m128){__f, 0.0f, 0.0f, 0.0f};")
fn(T, "_mm_set_ps", [("float", "__z"), ("float", "__y"), ("float", "__x"), ("float", "__w")], "return (__m128){__w, __x, __y, __z};")
fn(T, "_mm_setr_ps", [("float", "__z"), ("float", "__y"), ("float", "__x"), ("float", "__w")], "return (__m128){__z, __y, __x, __w};")
fn(T, "_mm_load_ps", [("float const *", "__p")], "return *(const __m128 *)__p;")
fn(T, "_mm_loadu_ps", [("float const *", "__p")], "return *(const __m128_u *)__p;")
fn(T, "_mm_load_ss", [("float const *", "__p")], "return _mm_set_ss(*__p);")
fn(T, "_mm_load1_ps", [("float const *", "__p")], "return _mm_set1_ps(*__p);")
fn(T, "_mm_load_ps1", [("float const *", "__p")], "return _mm_set1_ps(*__p);")
fn(T, "_mm_loadr_ps", [("float const *", "__p")], "__m128 __v = _mm_load_ps(__p); return _mm_shuffle_ps(__v, __v, _MM_SHUFFLE(0, 1, 2, 3));")
fn(T, "_mm_loadh_pi", [(T, "__a"), ("void const *", "__p")], "__m128 __b = (__m128)(__v2di){((const struct __altc_u64 *)__p)->__v, 0}; return _mm_movelh_ps(__a, __b);")
fn(T, "_mm_loadl_pi", [(T, "__a"), ("void const *", "__p")], "__m128 __b = (__m128)(__v2di){((const struct __altc_u64 *)__p)->__v, 0}; return _mm_shuffle_ps(__b, __a, _MM_SHUFFLE(3, 2, 1, 0));")
fn("void", "_mm_store_ps", [("float *", "__p"), (T, "__a")], "*(__m128 *)__p = __a;")
fn("void", "_mm_storeu_ps", [("float *", "__p"), (T, "__a")], "*(__m128_u *)__p = __a;")
fn("void", "_mm_store_ss", [("float *", "__p"), (T, "__a")], "*__p = _mm_cvtss_f32(__a);")
fn("void", "_mm_store1_ps", [("float *", "__p"), (T, "__a")], "_mm_store_ps(__p, _mm_shuffle_ps(__a, __a, 0));")
fn("void", "_mm_store_ps1", [("float *", "__p"), (T, "__a")], "_mm_store_ps(__p, _mm_shuffle_ps(__a, __a, 0));")
fn("void", "_mm_storer_ps", [("float *", "__p"), (T, "__a")], "_mm_store_ps(__p, _mm_shuffle_ps(__a, __a, _MM_SHUFFLE(0, 1, 2, 3)));")
fn("void", "_mm_storeh_pi", [("void *", "__p"), (T, "__a")], "((struct __altc_u64 *)__p)->__v = ((__v2di)__a)[1];")
fn("void", "_mm_storel_pi", [("void *", "__p"), (T, "__a")], "((struct __altc_u64 *)__p)->__v = ((__v2di)__a)[0];")
fn("void", "_mm_stream_ps", [("float *", "__p"), (T, "__a")], "*(__m128 *)__p = __a;")
fn("void", "_mm_sfence", [], '__asm__ __volatile__("sfence" ::: "memory");')
fn("unsigned int", "_mm_getcsr", [], 'unsigned int __r; __asm__ __volatile__("stmxcsr %0" : "=m"(__r)); return __r;')
fn("void", "_mm_setcsr", [("unsigned int", "__i")], '__asm__ __volatile__("ldmxcsr %0" :: "m"(__i));')
emit("""#define _MM_GET_ROUNDING_MODE() (_mm_getcsr() & _MM_ROUND_MASK)
#define _MM_SET_ROUNDING_MODE(mode) _mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | (mode))
#define _MM_GET_FLUSH_ZERO_MODE() (_mm_getcsr() & _MM_FLUSH_ZERO_MASK)
#define _MM_SET_FLUSH_ZERO_MODE(mode) _mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO_MASK) | (mode))
#define _MM_TRANSPOSE4_PS(row0, row1, row2, row3) do { \\
    __m128 __t0 = _mm_unpacklo_ps((row0), (row1)), __t1 = _mm_unpacklo_ps((row2), (row3)); \\
    __m128 __t2 = _mm_unpackhi_ps((row0), (row1)), __t3 = _mm_unpackhi_ps((row2), (row3)); \\
    (row0) = _mm_movelh_ps(__t0, __t1); (row1) = _mm_movehl_ps(__t1, __t0); \\
    (row2) = _mm_movelh_ps(__t2, __t3); (row3) = _mm_movehl_ps(__t3, __t2); \\
  } while (0)""")

# ---------------------------------------------------------------- SSE2
begin("emmintrin.h", "_EMMINTRIN_H_INCLUDED", ["xmmintrin.h"])
D, I = "__m128d", "__m128i"
for op in ["add", "sub", "mul", "div", "min", "max"]:
    bin(f"_mm_{op}_pd", D, f"{op}pd")
    bin(f"_mm_{op}_sd", D, f"{op}sd")
un("_mm_sqrt_pd", D, "sqrtpd")
fn(D, "_mm_sqrt_sd", [(D, "__a"), (D, "__b")], f"return _mm_move_sd(__a, {V(D, 'sqrtsd', '__b')});")
for op, m in [("and", "andpd"), ("andnot", "andnpd"), ("or", "orpd"), ("xor", "xorpd")]:
    bin(f"_mm_{op}_pd", D, m)
for sfx in ["pd", "sd"]:
    for p, i in preds.items():
        fn(D, f"_mm_cmp{p}_{sfx}", [(D, "__a"), (D, "__b")], f"return {VI(D, 'cmp' + sfx, i, '__a', '__b')};")
    for p, q in [("gt", 1), ("ge", 2), ("ngt", 5), ("nge", 6)]:
        if sfx == "pd":
            fn(D, f"_mm_cmp{p}_pd", [(D, "__a"), (D, "__b")], f"return {VI(D, 'cmppd', q, '__b', '__a')};")
        else:
            fn(D, f"_mm_cmp{p}_sd", [(D, "__a"), (D, "__b")], f"return _mm_move_sd(__a, {VI(D, 'cmpsd', q, '__b', '__a')});")
for p, op in [("eq", "=="), ("lt", "<"), ("le", "<="), ("gt", ">"), ("ge", ">="), ("neq", "!=")]:
    for c in ["comi", "ucomi"]:
        fn("int", f"_mm_{c}{p}_sd", [(D, "__a"), (D, "__b")], f"return _mm_cvtsd_f64(__a) {op} _mm_cvtsd_f64(__b);")
bin("_mm_move_sd", D, "movsd")
binI("_mm_shuffle_pd", D, "shufpd")
bin("_mm_unpackhi_pd", D, "unpckhpd")
bin("_mm_unpacklo_pd", D, "unpcklpd")
toGPR("_mm_movemask_pd", "int", "movmskpd", D)
fn("double", "_mm_cvtsd_f64", [(D, "__a")], f"return {V('double', '', '__a')};")
toGPR("_mm_cvtsd_si32", "int", "cvtsd2si", D)
toGPR("_mm_cvttsd_si32", "int", "cvttsd2si", D)
toGPR("_mm_cvtsd_si64", "long long", "cvtsd2si", D)
toGPR("_mm_cvtsd_si64x", "long long", "cvtsd2si", D)
toGPR("_mm_cvttsd_si64", "long long", "cvttsd2si", D)
toGPR("_mm_cvttsd_si64x", "long long", "cvttsd2si", D)
fn(D, "_mm_cvtsi32_sd", [(D, "__a"), ("int", "__b")], f"return {V(D, 'cvtsi2sd', '__a', '__b')};")
fn(D, "_mm_cvtsi64_sd", [(D, "__a"), ("long long", "__b")], f"return {V(D, 'cvtsi2sd', '__a', '__b')};")
fn(D, "_mm_cvtsi64x_sd", [(D, "__a"), ("long long", "__b")], f"return {V(D, 'cvtsi2sd', '__a', '__b')};")
fn("__m128", "_mm_cvtsd_ss", [("__m128", "__a"), (D, "__b")], f"return {V('__m128', 'cvtsd2ss', '__a', '__b')};")
fn(D, "_mm_cvtss_sd", [(D, "__a"), ("__m128", "__b")], f"return {V(D, 'cvtss2sd', '__a', '__b')};")
un("_mm_cvtpd_ps", "__m128", "cvtpd2ps", D)
un("_mm_cvtps_pd", D, "cvtps2pd", "__m128")
un("_mm_cvtepi32_pd", D, "cvtdq2pd", I)
un("_mm_cvtpd_epi32", I, "cvtpd2dq", D)
un("_mm_cvttpd_epi32", I, "cvttpd2dq", D)
un("_mm_cvtepi32_ps", "__m128", "cvtdq2ps", I)
un("_mm_cvtps_epi32", I, "cvtps2dq", "__m128")
un("_mm_cvttps_epi32", I, "cvttps2dq", "__m128")
fn(D, "_mm_setzero_pd", [], f"return {V(D, 'xorpd')};")
fn(D, "_mm_undefined_pd", [], f"return {V(D, 'xorpd')};")
fn(D, "_mm_set1_pd", [("double", "__f")], "return (__m128d)__ALTC_SPLAT(__v2df, __f);")
fn(D, "_mm_set_pd1", [("double", "__f")], "return (__m128d)__ALTC_SPLAT(__v2df, __f);")
fn(D, "_mm_set_sd", [("double", "__f")], "return (__m128d){__f, 0.0};")
fn(D, "_mm_set_pd", [("double", "__x"), ("double", "__y")], "return (__m128d){__y, __x};")
fn(D, "_mm_setr_pd", [("double", "__x"), ("double", "__y")], "return (__m128d){__x, __y};")
fn(D, "_mm_load_pd", [("double const *", "__p")], "return *(const __m128d *)__p;")
fn(D, "_mm_loadu_pd", [("double const *", "__p")], "return *(const __m128d_u *)__p;")
fn(D, "_mm_load_sd", [("double const *", "__p")], "return _mm_set_sd(*__p);")
fn(D, "_mm_load1_pd", [("double const *", "__p")], "return _mm_set1_pd(*__p);")
fn(D, "_mm_load_pd1", [("double const *", "__p")], "return _mm_set1_pd(*__p);")
fn(D, "_mm_loadr_pd", [("double const *", "__p")], "__m128d __v = _mm_load_pd(__p); return _mm_shuffle_pd(__v, __v, 1);")
fn(D, "_mm_loadh_pd", [(D, "__a"), ("double const *", "__p")], "return _mm_unpacklo_pd(__a, _mm_set_sd(((const struct __altc_f64 *)__p)->__v));")
fn(D, "_mm_loadl_pd", [(D, "__a"), ("double const *", "__p")], "return _mm_move_sd(__a, _mm_set_sd(((const struct __altc_f64 *)__p)->__v));")
fn("void", "_mm_store_pd", [("double *", "__p"), (D, "__a")], "*(__m128d *)__p = __a;")
fn("void", "_mm_storeu_pd", [("double *", "__p"), (D, "__a")], "*(__m128d_u *)__p = __a;")
fn("void", "_mm_store_sd", [("double *", "__p"), (D, "__a")], "*__p = _mm_cvtsd_f64(__a);")
fn("void", "_mm_storel_pd", [("double *", "__p"), (D, "__a")], "((struct __altc_f64 *)__p)->__v = _mm_cvtsd_f64(__a);")
fn("void", "_mm_storeh_pd", [("double *", "__p"), (D, "__a")], "((struct __altc_f64 *)__p)->__v = _mm_cvtsd_f64(_mm_unpackhi_pd(__a, __a));")
fn("void", "_mm_store1_pd", [("double *", "__p"), (D, "__a")], "_mm_store_pd(__p, _mm_unpacklo_pd(__a, __a));")
fn("void", "_mm_store_pd1", [("double *", "__p"), (D, "__a")], "_mm_store_pd(__p, _mm_unpacklo_pd(__a, __a));")
fn("void", "_mm_storer_pd", [("double *", "__p"), (D, "__a")], "_mm_store_pd(__p, _mm_shuffle_pd(__a, __a, 1));")
fn("void", "_mm_stream_pd", [("double *", "__p"), (D, "__a")], "*(__m128d *)__p = __a;")
# integer
for w, s in [("8", "b"), ("16", "w"), ("32", "d"), ("64", "q")]:
    bin(f"_mm_add_epi{w}", I, f"padd{s}")
    bin(f"_mm_sub_epi{w}", I, f"psub{s}")
for name, mn in [("adds_epi8", "paddsb"), ("adds_epi16", "paddsw"), ("adds_epu8", "paddusb"), ("adds_epu16", "paddusw"),
                 ("subs_epi8", "psubsb"), ("subs_epi16", "psubsw"), ("subs_epu8", "psubusb"), ("subs_epu16", "psubusw"),
                 ("avg_epu8", "pavgb"), ("avg_epu16", "pavgw"), ("madd_epi16", "pmaddwd"), ("max_epi16", "pmaxsw"),
                 ("max_epu8", "pmaxub"), ("min_epi16", "pminsw"), ("min_epu8", "pminub"), ("mulhi_epi16", "pmulhw"),
                 ("mulhi_epu16", "pmulhuw"), ("mullo_epi16", "pmullw"), ("mul_epu32", "pmuludq"), ("sad_epu8", "psadbw"),
                 ("and_si128", "pand"), ("andnot_si128", "pandn"), ("or_si128", "por"), ("xor_si128", "pxor"),
                 ("cmpeq_epi8", "pcmpeqb"), ("cmpeq_epi16", "pcmpeqw"), ("cmpeq_epi32", "pcmpeqd"),
                 ("cmpgt_epi8", "pcmpgtb"), ("cmpgt_epi16", "pcmpgtw"), ("cmpgt_epi32", "pcmpgtd"),
                 ("packs_epi16", "packsswb"), ("packs_epi32", "packssdw"), ("packus_epi16", "packuswb"),
                 ("unpackhi_epi8", "punpckhbw"), ("unpackhi_epi16", "punpckhwd"), ("unpackhi_epi32", "punpckhdq"),
                 ("unpackhi_epi64", "punpckhqdq"), ("unpacklo_epi8", "punpcklbw"), ("unpacklo_epi16", "punpcklwd"),
                 ("unpacklo_epi32", "punpckldq"), ("unpacklo_epi64", "punpcklqdq")]:
    bin("_mm_" + name, I, mn)
for name, mn in [("cmplt_epi8", "pcmpgtb"), ("cmplt_epi16", "pcmpgtw"), ("cmplt_epi32", "pcmpgtd")]:
    binswap("_mm_" + name, I, mn)
for s, m in [("16", "w"), ("32", "d"), ("64", "q")]:
    fn(I, f"_mm_slli_epi{s}", [(I, "__a"), ("int", "__n")], f"return {V(I, 'psll' + m, '__a', '__n')};")
    fn(I, f"_mm_srli_epi{s}", [(I, "__a"), ("int", "__n")], f"return {V(I, 'psrl' + m, '__a', '__n')};")
    bin(f"_mm_sll_epi{s}", I, "psll" + m)
    bin(f"_mm_srl_epi{s}", I, "psrl" + m)
for s, m in [("16", "w"), ("32", "d")]:
    fn(I, f"_mm_srai_epi{s}", [(I, "__a"), ("int", "__n")], f"return {V(I, 'psra' + m, '__a', '__n')};")
    bin(f"_mm_sra_epi{s}", I, "psra" + m)
macro("_mm_slli_si128", ["a", "imm"], VI(I, "pslldq", "(imm)", f"({I})(a)"))
macro("_mm_srli_si128", ["a", "imm"], VI(I, "psrldq", "(imm)", f"({I})(a)"))
macro("_mm_bslli_si128", ["a", "imm"], VI(I, "pslldq", "(imm)", f"({I})(a)"))
macro("_mm_bsrli_si128", ["a", "imm"], VI(I, "psrldq", "(imm)", f"({I})(a)"))
unI("_mm_shuffle_epi32", I, "pshufd")
unI("_mm_shufflehi_epi16", I, "pshufhw")
unI("_mm_shufflelo_epi16", I, "pshuflw")
macro("_mm_extract_epi16", ["a", "imm"], VI("int", "pextrw", "(imm)", f"({I})(a)"))
macro("_mm_insert_epi16", ["a", "x", "imm"], VI(I, "pinsrw", "(imm)", f"({I})(a)", "(int)(x)"))
toGPR("_mm_movemask_epi8", "int", "pmovmskb", I)
toGPR("_mm_cvtsi128_si32", "int", "movd", I)
toGPR("_mm_cvtsi128_si64", "long long", "movq", I)
toGPR("_mm_cvtsi128_si64x", "long long", "movq", I)
fn(I, "_mm_cvtsi32_si128", [("int", "__a")], f"return {V(I, 'movd', '__a')};")
fn(I, "_mm_cvtsi64_si128", [("long long", "__a")], f"return {V(I, 'movq', '__a')};")
fn(I, "_mm_cvtsi64x_si128", [("long long", "__a")], f"return {V(I, 'movq', '__a')};")
fn(I, "_mm_move_epi64", [(I, "__a")], f"return {V(I, 'movq', '__a')};")
fn(I, "_mm_setzero_si128", [], f"return {V(I, 'pxor')};")
fn(I, "_mm_undefined_si128", [], f"return {V(I, 'pxor')};")
fn(I, "_mm_set1_epi8", [("char", "__x")], "return (__m128i)__ALTC_SPLAT(__v16qi, __x);")
fn(I, "_mm_set1_epi16", [("short", "__x")], "return (__m128i)__ALTC_SPLAT(__v8hi, __x);")
fn(I, "_mm_set1_epi32", [("int", "__x")], "return (__m128i)__ALTC_SPLAT(__v4si, __x);")
fn(I, "_mm_set1_epi64x", [("long long", "__x")], "return (__m128i)__ALTC_SPLAT(__v2di, __x);")
fn(I, "_mm_set_epi64x", [("long long", "__a"), ("long long", "__b")], "return (__m128i)(__v2di){__b, __a};")
fn(I, "_mm_set_epi32", [("int", "__a"), ("int", "__b"), ("int", "__c"), ("int", "__d")], "return (__m128i)(__v4si){__d, __c, __b, __a};")
fn(I, "_mm_setr_epi32", [("int", "__a"), ("int", "__b"), ("int", "__c"), ("int", "__d")], "return (__m128i)(__v4si){__a, __b, __c, __d};")
h8 = [("short", f"__q{i}") for i in range(7, -1, -1)]
fn(I, "_mm_set_epi16", h8, "return (__m128i)(__v8hi){" + ", ".join(f"__q{i}" for i in range(8)) + "};")
fn(I, "_mm_setr_epi16", [("short", f"__q{i}") for i in range(8)], "return (__m128i)(__v8hi){" + ", ".join(f"__q{i}" for i in range(8)) + "};")
b16 = [("char", f"__q{i:02d}") for i in range(15, -1, -1)]
fn(I, "_mm_set_epi8", b16, "return (__m128i)(__v16qi){" + ", ".join(f"__q{i:02d}" for i in range(16)) + "};")
fn(I, "_mm_setr_epi8", [("char", f"__q{i:02d}") for i in range(16)], "return (__m128i)(__v16qi){" + ", ".join(f"__q{i:02d}" for i in range(16)) + "};")
fn(I, "_mm_load_si128", [(I + " const *", "__p")], "return *__p;")
fn(I, "_mm_loadu_si128", [(I + "_u const *", "__p")], "return *__p;")
fn(I, "_mm_loadl_epi64", [(I + "_u const *", "__p")], "return _mm_set_epi64x(0, ((const struct __altc_u64 *)__p)->__v);")
fn(I, "_mm_loadu_si64", [("void const *", "__p")], "return _mm_set_epi64x(0, ((const struct __altc_u64 *)__p)->__v);")
fn(I, "_mm_loadu_si32", [("void const *", "__p")], "return _mm_cvtsi32_si128(((const struct __altc_u32 *)__p)->__v);")
fn(I, "_mm_loadu_si16", [("void const *", "__p")], "return _mm_cvtsi32_si128((unsigned short)((const struct __altc_u16 *)__p)->__v);")
fn("void", "_mm_store_si128", [(I + " *", "__p"), (I, "__a")], "*__p = __a;")
fn("void", "_mm_storeu_si128", [(I + "_u *", "__p"), (I, "__a")], "*__p = __a;")
fn("void", "_mm_storel_epi64", [(I + "_u *", "__p"), (I, "__a")], "((struct __altc_u64 *)__p)->__v = _mm_cvtsi128_si64(__a);")
fn("void", "_mm_storeu_si64", [("void *", "__p"), (I, "__a")], "((struct __altc_u64 *)__p)->__v = _mm_cvtsi128_si64(__a);")
fn("void", "_mm_storeu_si32", [("void *", "__p"), (I, "__a")], "((struct __altc_u32 *)__p)->__v = _mm_cvtsi128_si32(__a);")
fn("void", "_mm_storeu_si16", [("void *", "__p"), (I, "__a")], "((struct __altc_u16 *)__p)->__v = (short)_mm_cvtsi128_si32(__a);")
fn("void", "_mm_stream_si128", [(I + " *", "__p"), (I, "__a")], "*__p = __a;")
fn("void", "_mm_stream_si32", [("int *", "__p"), ("int", "__a")], "*__p = __a;")
fn("void", "_mm_stream_si64", [("long long *", "__p"), ("long long", "__a")], "*__p = __a;")
fn("void", "_mm_maskmoveu_si128", [(I, "__a"), (I, "__m"), ("char *", "__p")], "for (int __i = 0; __i < 16; __i++) if (((__v16qi)__m)[__i] < 0) __p[__i] = ((__v16qi)__a)[__i];")
for a, b in [("ps", "__m128"), ("pd", D), ("si128", I)]:
    for c, d in [("ps", "__m128"), ("pd", D), ("si128", I)]:
        if a != c:
            fn(d, f"_mm_cast{a}_{c}", [(b, "__a")], f"return ({d})__a;")
fn("void", "_mm_lfence", [], '__asm__ __volatile__("lfence" ::: "memory");')
fn("void", "_mm_mfence", [], "__sync_synchronize();")
fn("void", "_mm_pause", [], '__asm__ __volatile__("pause");')
fn("void", "_mm_clflush", [("void const *", "__p")], '__asm__ __volatile__("clflush %0" :: "m"(*(const char *)__p));')

# ---------------------------------------------------------------- SSE3
begin("pmmintrin.h", "_PMMINTRIN_H_INCLUDED", ["emmintrin.h"])
for n, m, T in [("addsub_ps", "addsubps", "__m128"), ("addsub_pd", "addsubpd", D), ("hadd_ps", "haddps", "__m128"),
                ("hadd_pd", "haddpd", D), ("hsub_ps", "hsubps", "__m128"), ("hsub_pd", "hsubpd", D)]:
    bin("_mm_" + n, T, m)
un("_mm_movehdup_ps", "__m128", "movshdup")
un("_mm_moveldup_ps", "__m128", "movsldup")
un("_mm_movedup_pd", D, "movddup")
fn(I, "_mm_lddqu_si128", [(I + " const *", "__p")], "return *(const __m128i_u *)__p;")
fn(D, "_mm_loaddup_pd", [("double const *", "__p")], "return _mm_set1_pd(*__p);")

# ---------------------------------------------------------------- SSSE3
begin("tmmintrin.h", "_TMMINTRIN_H_INCLUDED", ["pmmintrin.h"])
for n, m in [("abs_epi8", "pabsb"), ("abs_epi16", "pabsw"), ("abs_epi32", "pabsd")]:
    un("_mm_" + n, I, m)
for n, m in [("hadd_epi16", "phaddw"), ("hadd_epi32", "phaddd"), ("hadds_epi16", "phaddsw"), ("hsub_epi16", "phsubw"),
             ("hsub_epi32", "phsubd"), ("hsubs_epi16", "phsubsw"), ("maddubs_epi16", "pmaddubsw"),
             ("mulhrs_epi16", "pmulhrsw"), ("shuffle_epi8", "pshufb"), ("sign_epi8", "psignb"),
             ("sign_epi16", "psignw"), ("sign_epi32", "psignd")]:
    bin("_mm_" + n, I, m)
binI("_mm_alignr_epi8", I, "palignr")

# ---------------------------------------------------------------- SSE4.1
begin("smmintrin.h", "_SMMINTRIN_H_INCLUDED", ["tmmintrin.h"])
emit("""#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF 0x01
#define _MM_FROUND_TO_POS_INF 0x02
#define _MM_FROUND_TO_ZERO 0x03
#define _MM_FROUND_CUR_DIRECTION 0x04
#define _MM_FROUND_RAISE_EXC 0x00
#define _MM_FROUND_NO_EXC 0x08
#define _MM_FROUND_NINT (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_FLOOR (_MM_FROUND_TO_NEG_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_CEIL (_MM_FROUND_TO_POS_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_TRUNC (_MM_FROUND_TO_ZERO | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_RINT (_MM_FROUND_CUR_DIRECTION | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_NEARBYINT (_MM_FROUND_CUR_DIRECTION | _MM_FROUND_NO_EXC)""")
binI("_mm_blend_ps", "__m128", "blendps")
binI("_mm_blend_pd", D, "blendpd")
binI("_mm_blend_epi16", I, "pblendw")
for n, m, T in [("blendv_ps", "blendvps", "__m128"), ("blendv_pd", "blendvpd", D), ("blendv_epi8", "pblendvb", I)]:
    fn(T, "_mm_" + n, [(T, "__a"), (T, "__b"), (T, "__m")], f"return {V(T, m, '__a', '__b', '__m')};")
unI("_mm_round_ps", "__m128", "roundps")
unI("_mm_round_pd", D, "roundpd")
binI("_mm_round_ss", "__m128", "roundss")
binI("_mm_round_sd", D, "roundsd")
emit("#define _mm_floor_ps(a) _mm_round_ps((a), _MM_FROUND_FLOOR)")
emit("#define _mm_ceil_ps(a) _mm_round_ps((a), _MM_FROUND_CEIL)")
emit("#define _mm_floor_pd(a) _mm_round_pd((a), _MM_FROUND_FLOOR)")
emit("#define _mm_ceil_pd(a) _mm_round_pd((a), _MM_FROUND_CEIL)")
emit("#define _mm_floor_ss(a, b) _mm_round_ss((a), (b), _MM_FROUND_FLOOR)")
emit("#define _mm_ceil_ss(a, b) _mm_round_ss((a), (b), _MM_FROUND_CEIL)")
emit("#define _mm_floor_sd(a, b) _mm_round_sd((a), (b), _MM_FROUND_FLOOR)")
emit("#define _mm_ceil_sd(a, b) _mm_round_sd((a), (b), _MM_FROUND_CEIL)")
for n, m in [("cmpeq_epi64", "pcmpeqq"), ("max_epi8", "pmaxsb"), ("max_epi32", "pmaxsd"), ("max_epu16", "pmaxuw"),
             ("max_epu32", "pmaxud"), ("min_epi8", "pminsb"), ("min_epi32", "pminsd"), ("min_epu16", "pminuw"),
             ("min_epu32", "pminud"), ("mul_epi32", "pmuldq"), ("mullo_epi32", "pmulld"), ("packus_epi32", "packusdw")]:
    bin("_mm_" + n, I, m)
for f, t in [("epi8_epi16", "bw"), ("epi8_epi32", "bd"), ("epi8_epi64", "bq"), ("epi16_epi32", "wd"), ("epi16_epi64", "wq"), ("epi32_epi64", "dq")]:
    un(f"_mm_cvt{f}", I, "pmovsx" + t)
    un(f"_mm_cvt{f.replace('epi', 'epu', 1)}", I, "pmovzx" + t)
binI("_mm_dp_ps", "__m128", "dpps")
binI("_mm_dp_pd", D, "dppd")
binI("_mm_mpsadbw_epu8", I, "mpsadbw")
macro("_mm_extract_epi8", ["a", "imm"], "(unsigned char)" + VI("int", "pextrb", "(imm)", f"({I})(a)"))
macro("_mm_extract_epi32", ["a", "imm"], VI("int", "pextrd", "(imm)", f"({I})(a)"))
macro("_mm_extract_epi64", ["a", "imm"], VI("long long", "pextrq", "(imm)", f"({I})(a)"))
macro("_mm_extract_ps", ["a", "imm"], VI("int", "extractps", "(imm)", "(__m128)(a)"))
macro("_mm_insert_epi8", ["a", "x", "imm"], VI(I, "pinsrb", "(imm)", f"({I})(a)", "(int)(x)"))
macro("_mm_insert_epi32", ["a", "x", "imm"], VI(I, "pinsrd", "(imm)", f"({I})(a)", "(int)(x)"))
macro("_mm_insert_epi64", ["a", "x", "imm"], VI(I, "pinsrq", "(imm)", f"({I})(a)", "(long long)(x)"))
binI("_mm_insert_ps", "__m128", "insertps")
un("_mm_minpos_epu16", I, "phminposuw")
fn(I, "_mm_stream_load_si128", [(I + " *", "__p")], "return *__p;")
fn("int", "_mm_testz_si128", [(I, "__a"), (I, "__b")], f"return {V('int', 'ptest:e', '__a', '__b')};")
fn("int", "_mm_testc_si128", [(I, "__a"), (I, "__b")], f"return {V('int', 'ptest:c', '__a', '__b')};")
fn("int", "_mm_testnzc_si128", [(I, "__a"), (I, "__b")], f"return {V('int', 'ptest:a', '__a', '__b')};")
emit("#define _mm_test_all_zeros(m, v) _mm_testz_si128((m), (v))")
emit("#define _mm_test_all_ones(v) _mm_testc_si128((v), _mm_cmpeq_epi32((v), (v)))")
emit("#define _mm_test_mix_ones_zeros(m, v) _mm_testnzc_si128((m), (v))")

# ---------------------------------------------------------------- SSE4.2
begin("nmmintrin.h", "_NMMINTRIN_H_INCLUDED", ["smmintrin.h", "popcntintrin.h"])
bin("_mm_cmpgt_epi64", I, "pcmpgtq")
emit("""__ALTC_INL unsigned int __altc_crc32(unsigned int __c, unsigned long long __v, int __n) {
  for (int __i = 0; __i < __n; __i++) {
    __c ^= (unsigned char)(__v >> (8 * __i));
    for (int __k = 0; __k < 8; __k++) __c = (__c >> 1) ^ (0x82F63B78u & -(__c & 1));
  }
  return __c;
}
__ALTC_INL unsigned int _mm_crc32_u8(unsigned int __c, unsigned char __v) { return __altc_crc32(__c, __v, 1); }
__ALTC_INL unsigned int _mm_crc32_u16(unsigned int __c, unsigned short __v) { return __altc_crc32(__c, __v, 2); }
__ALTC_INL unsigned int _mm_crc32_u32(unsigned int __c, unsigned int __v) { return __altc_crc32(__c, __v, 4); }
__ALTC_INL unsigned long long _mm_crc32_u64(unsigned long long __c, unsigned long long __v) { return __altc_crc32((unsigned int)__c, __v, 8); }""")

# ---------------------------------------------------------------- AES, PCLMUL
begin("wmmintrin.h", "_WMMINTRIN_H_INCLUDED", ["emmintrin.h"])
for n in ["aesenc", "aesenclast", "aesdec", "aesdeclast"]:
    bin(f"_mm_{n}_si128", I, n)
un("_mm_aesimc_si128", I, "aesimc")
unI("_mm_aeskeygenassist_si128", I, "aeskeygenassist")
binI("_mm_clmulepi64_si128", I, "pclmulqdq")

# ---------------------------------------------------------------- AVX
begin("avxintrin.h", "_AVXINTRIN_H_INCLUDED", ["nmmintrin.h"])
emit("""#define _CMP_EQ_OQ 0x00
#define _CMP_LT_OS 0x01
#define _CMP_LE_OS 0x02
#define _CMP_UNORD_Q 0x03
#define _CMP_NEQ_UQ 0x04
#define _CMP_NLT_US 0x05
#define _CMP_NLE_US 0x06
#define _CMP_ORD_Q 0x07
#define _CMP_EQ_UQ 0x08
#define _CMP_NGE_US 0x09
#define _CMP_NGT_US 0x0a
#define _CMP_FALSE_OQ 0x0b
#define _CMP_NEQ_OQ 0x0c
#define _CMP_GE_OS 0x0d
#define _CMP_GT_OS 0x0e
#define _CMP_TRUE_UQ 0x0f
#define _CMP_EQ_OS 0x10
#define _CMP_LT_OQ 0x11
#define _CMP_LE_OQ 0x12
#define _CMP_UNORD_S 0x13
#define _CMP_NEQ_US 0x14
#define _CMP_NLT_UQ 0x15
#define _CMP_NLE_UQ 0x16
#define _CMP_ORD_S 0x17
#define _CMP_EQ_US 0x18
#define _CMP_NGE_UQ 0x19
#define _CMP_NGT_UQ 0x1a
#define _CMP_FALSE_OS 0x1b
#define _CMP_NEQ_OS 0x1c
#define _CMP_GE_OQ 0x1d
#define _CMP_GT_OQ 0x1e
#define _CMP_TRUE_US 0x1f""")
S, SD, SI = "__m256", "__m256d", "__m256i"
for op in ["add", "sub", "mul", "div", "min", "max", "addsub", "hadd", "hsub"]:
    bin(f"_mm256_{op}_ps", S, f"{op}ps")
    bin(f"_mm256_{op}_pd", SD, f"{op}pd")
for op, m in [("and", "and"), ("andnot", "andn"), ("or", "or"), ("xor", "xor")]:
    bin(f"_mm256_{op}_ps", S, m + "ps")
    bin(f"_mm256_{op}_pd", SD, m + "pd")
for op in ["sqrt", "rcp", "rsqrt"]:
    un(f"_mm256_{op}_ps", S, f"{op}ps")
un("_mm256_sqrt_pd", SD, "sqrtpd")
binI("_mm256_blend_ps", S, "blendps")
binI("_mm256_blend_pd", SD, "blendpd")
fn(S, "_mm256_blendv_ps", [(S, "__a"), (S, "__b"), (S, "__m")], f"return {V(S, 'blendvps', '__a', '__b', '__m')};")
fn(SD, "_mm256_blendv_pd", [(SD, "__a"), (SD, "__b"), (SD, "__m")], f"return {V(SD, 'blendvpd', '__a', '__b', '__m')};")
binI("_mm256_dp_ps", S, "dpps")
binI("_mm256_shuffle_ps", S, "shufps")
binI("_mm256_shuffle_pd", SD, "shufpd")
for t, T in [("ps", S), ("pd", SD)]:
    bin(f"_mm256_unpackhi_{t}", T, f"unpckh{t}")
    bin(f"_mm256_unpacklo_{t}", T, f"unpckl{t}")
    binI(f"_mm256_cmp_{t}", T, f"cmp{t}")
    toGPR(f"_mm256_movemask_{t}", "int", f"movmsk{t}", T)
    unI(f"_mm256_round_{t}", T, f"round{t}")
    emit(f"#define _mm256_floor_{t}(a) _mm256_round_{t}((a), _MM_FROUND_FLOOR)")
    emit(f"#define _mm256_ceil_{t}(a) _mm256_round_{t}((a), _MM_FROUND_CEIL)")
    fn("int", f"_mm256_testz_{t}", [(T, "__a"), (T, "__b")], f"return {V('int', 'test' + t + ':e', '__a', '__b')};")
    fn("int", f"_mm256_testc_{t}", [(T, "__a"), (T, "__b")], f"return {V('int', 'test' + t + ':c', '__a', '__b')};")
    fn("int", f"_mm256_testnzc_{t}", [(T, "__a"), (T, "__b")], f"return {V('int', 'test' + t + ':a', '__a', '__b')};")
    fn("int", f"_mm_testz_{t}", [(f"__m128{'d' if t == 'pd' else ''}", "__a"), (f"__m128{'d' if t == 'pd' else ''}", "__b")], f"return {V('int', 'test' + t + ':e', '__a', '__b')};")
binI("_mm_cmp_ps", "__m128", "cmpps")
binI("_mm_cmp_pd", D, "cmppd")
binI("_mm_cmp_ss", "__m128", "cmpss")
binI("_mm_cmp_sd", D, "cmpsd")
fn("int", "_mm256_testz_si256", [(SI, "__a"), (SI, "__b")], f"return {V('int', 'ptest:e', '__a', '__b')};")
fn("int", "_mm256_testc_si256", [(SI, "__a"), (SI, "__b")], f"return {V('int', 'ptest:c', '__a', '__b')};")
fn("int", "_mm256_testnzc_si256", [(SI, "__a"), (SI, "__b")], f"return {V('int', 'ptest:a', '__a', '__b')};")
un("_mm256_cvtepi32_pd", SD, "cvtdq2pd", I)
un("_mm256_cvtepi32_ps", S, "cvtdq2ps", SI)
un("_mm256_cvtpd_ps", "__m128", "cvtpd2ps", SD)
un("_mm256_cvtps_epi32", SI, "cvtps2dq", S)
un("_mm256_cvtps_pd", SD, "cvtps2pd", "__m128")
un("_mm256_cvttpd_epi32", I, "cvttpd2dq", SD)
un("_mm256_cvtpd_epi32", I, "cvtpd2dq", SD)
un("_mm256_cvttps_epi32", SI, "cvttps2dq", S)
fn("float", "_mm256_cvtss_f32", [(S, "__a")], f"return {V('float', '', '__a')};")
fn("double", "_mm256_cvtsd_f64", [(SD, "__a")], f"return {V('double', '', '__a')};")
fn("int", "_mm256_cvtsi256_si32", [(SI, "__a")], "return _mm_cvtsi128_si32(_mm256_castsi256_si128(__a));")
macro("_mm256_extractf128_ps", ["a", "imm"], VI("__m128", "extractf128", "(imm)", f"({S})(a)"))
macro("_mm256_extractf128_pd", ["a", "imm"], VI(D, "extractf128", "(imm)", f"({SD})(a)"))
macro("_mm256_extractf128_si256", ["a", "imm"], VI(I, "extractf128", "(imm)", f"({SI})(a)"))
macro("_mm256_insertf128_ps", ["a", "b", "imm"], VI(S, "insertf128", "(imm)", f"({S})(a)", "(__m128)(b)"))
macro("_mm256_insertf128_pd", ["a", "b", "imm"], VI(SD, "insertf128", "(imm)", f"({SD})(a)", f"({D})(b)"))
macro("_mm256_insertf128_si256", ["a", "b", "imm"], VI(SI, "insertf128", "(imm)", f"({SI})(a)", f"({I})(b)"))
binI("_mm256_permute2f128_ps", S, "perm2f128")
binI("_mm256_permute2f128_pd", SD, "perm2f128")
binI("_mm256_permute2f128_si256", SI, "perm2f128")
unI("_mm256_permute_ps", S, "permilps")
unI("_mm256_permute_pd", SD, "permilpd")
unI("_mm_permute_ps", "__m128", "permilps")
unI("_mm_permute_pd", D, "permilpd")
bin("_mm256_permutevar_ps", S, "permilps", S, SI)
bin("_mm256_permutevar_pd", SD, "permilpd", SD, SI)
bin("_mm_permutevar_ps", "__m128", "permilps", "__m128", I)
bin("_mm_permutevar_pd", D, "permilpd", D, I)
un("_mm256_movehdup_ps", S, "movshdup")
un("_mm256_moveldup_ps", S, "movsldup")
un("_mm256_movedup_pd", SD, "movddup")
for t, T, U in [("ps", S, "__m256_u"), ("pd", SD, "__m256d_u")]:
    e = "float" if t == "ps" else "double"
    fn(T, f"_mm256_load_{t}", [(f"{e} const *", "__p")], f"return *(const {T} *)__p;")
    fn(T, f"_mm256_loadu_{t}", [(f"{e} const *", "__p")], f"return *(const {U} *)__p;")
    fn("void", f"_mm256_store_{t}", [(f"{e} *", "__p"), (T, "__a")], f"*({T} *)__p = __a;")
    fn("void", f"_mm256_storeu_{t}", [(f"{e} *", "__p"), (T, "__a")], f"*({U} *)__p = __a;")
    fn("void", f"_mm256_stream_{t}", [(f"{e} *", "__p"), (T, "__a")], f"*({T} *)__p = __a;")
fn(SI, "_mm256_load_si256", [(SI + " const *", "__p")], "return *__p;")
fn(SI, "_mm256_loadu_si256", [(SI + "_u const *", "__p")], "return *__p;")
fn(SI, "_mm256_lddqu_si256", [(SI + " const *", "__p")], "return *(const __m256i_u *)__p;")
fn("void", "_mm256_store_si256", [(SI + " *", "__p"), (SI, "__a")], "*__p = __a;")
fn("void", "_mm256_storeu_si256", [(SI + "_u *", "__p"), (SI, "__a")], "*__p = __a;")
fn("void", "_mm256_stream_si256", [(SI + " *", "__p"), (SI, "__a")], "*__p = __a;")
fn(S, "_mm256_maskload_ps", [("float const *", "__p"), (SI, "__m")], f"return {V(S, 'maskmovps', '(long long)__p', '__m')};")
fn(SD, "_mm256_maskload_pd", [("double const *", "__p"), (SI, "__m")], f"return {V(SD, 'maskmovpd', '(long long)__p', '__m')};")
fn("__m128", "_mm_maskload_ps", [("float const *", "__p"), (I, "__m")], f"return {V('__m128', 'maskmovps', '(long long)__p', '__m')};")
fn(D, "_mm_maskload_pd", [("double const *", "__p"), (I, "__m")], f"return {V(D, 'maskmovpd', '(long long)__p', '__m')};")
fn(S, "_mm256_broadcast_ss", [("float const *", "__p")], "return (__m256)__ALTC_SPLAT(__v8sf, *__p);")
fn("__m128", "_mm_broadcast_ss", [("float const *", "__p")], "return (__m128)__ALTC_SPLAT(__v4sf, *__p);")
fn(SD, "_mm256_broadcast_sd", [("double const *", "__p")], "return (__m256d)__ALTC_SPLAT(__v4df, *__p);")
fn(S, "_mm256_broadcast_ps", [("__m128 const *", "__p")], "__m128 __x = *(const __m128_u *)__p; return _mm256_insertf128_ps(_mm256_castps128_ps256(__x), __x, 1);")
fn(SD, "_mm256_broadcast_pd", [(D + " const *", "__p")], "__m128d __x = *(const __m128d_u *)__p; return _mm256_insertf128_pd(_mm256_castpd128_pd256(__x), __x, 1);")
for t, T, e, v in [("ps", S, "float", "__v8sf"), ("pd", SD, "double", "__v4df")]:
    fn(T, f"_mm256_setzero_{t}", [], f"return {V(T, 'xorps')};")
    fn(T, f"_mm256_undefined_{t}", [], f"return {V(T, 'xorps')};")
    fn(T, f"_mm256_set1_{t}", [(e, "__f")], f"return ({T})__ALTC_SPLAT({v}, __f);")
n8 = 8 if t == "ps" else 4
fn(S, "_mm256_set_ps", [("float", f"__e{i}") for i in range(7, -1, -1)], "return (__m256){" + ", ".join(f"__e{i}" for i in range(8)) + "};")
fn(S, "_mm256_setr_ps", [("float", f"__e{i}") for i in range(8)], "return (__m256){" + ", ".join(f"__e{i}" for i in range(8)) + "};")
fn(SD, "_mm256_set_pd", [("double", f"__e{i}") for i in range(3, -1, -1)], "return (__m256d){__e0, __e1, __e2, __e3};")
fn(SD, "_mm256_setr_pd", [("double", f"__e{i}") for i in range(4)], "return (__m256d){__e0, __e1, __e2, __e3};")
fn(SI, "_mm256_setzero_si256", [], f"return {V(SI, 'xorps')};")
fn(SI, "_mm256_undefined_si256", [], f"return {V(SI, 'xorps')};")
fn(SI, "_mm256_set1_epi8", [("char", "__x")], "return (__m256i)__ALTC_SPLAT(__v32qi, __x);")
fn(SI, "_mm256_set1_epi16", [("short", "__x")], "return (__m256i)__ALTC_SPLAT(__v16hi, __x);")
fn(SI, "_mm256_set1_epi32", [("int", "__x")], "return (__m256i)__ALTC_SPLAT(__v8si, __x);")
fn(SI, "_mm256_set1_epi64x", [("long long", "__x")], "return (__m256i)__ALTC_SPLAT(__v4di, __x);")
fn(SI, "_mm256_set_epi32", [("int", f"__e{i}") for i in range(7, -1, -1)], "return (__m256i)(__v8si){" + ", ".join(f"__e{i}" for i in range(8)) + "};")
fn(SI, "_mm256_setr_epi32", [("int", f"__e{i}") for i in range(8)], "return (__m256i)(__v8si){" + ", ".join(f"__e{i}" for i in range(8)) + "};")
fn(SI, "_mm256_set_epi64x", [("long long", f"__e{i}") for i in range(3, -1, -1)], "return (__m256i)(__v4di){__e0, __e1, __e2, __e3};")
fn(SI, "_mm256_setr_epi64x", [("long long", f"__e{i}") for i in range(4)], "return (__m256i)(__v4di){__e0, __e1, __e2, __e3};")
fn(SI, "_mm256_set_epi16", [("short", f"__e{i:02d}") for i in range(15, -1, -1)], "return (__m256i)(__v16hi){" + ", ".join(f"__e{i:02d}" for i in range(16)) + "};")
fn(SI, "_mm256_setr_epi16", [("short", f"__e{i:02d}") for i in range(16)], "return (__m256i)(__v16hi){" + ", ".join(f"__e{i:02d}" for i in range(16)) + "};")
fn(SI, "_mm256_set_epi8", [("char", f"__e{i:02d}") for i in range(31, -1, -1)], "return (__m256i)(__v32qi){" + ", ".join(f"__e{i:02d}" for i in range(32)) + "};")
fn(SI, "_mm256_setr_epi8", [("char", f"__e{i:02d}") for i in range(32)], "return (__m256i)(__v32qi){" + ", ".join(f"__e{i:02d}" for i in range(32)) + "};")
# casts
types = [("ps", S, "__m128"), ("pd", SD, D), ("si256", SI, I)]
for a, A, _ in types:
    for b, B, _ in types:
        if a != b:
            fn(B, f"_mm256_cast{a}_{b}", [(A, "__a")], f"return ({B})__a;")
for a, A, H in types:
    h = {"ps": "ps", "pd": "pd", "si256": "si128"}[a]
    fn(H, f"_mm256_cast{a}256_{h}128" if a != "si256" else "_mm256_castsi256_si128", [(A, "__a")], f"return {V(H, '', '__a')};")
    fn(A, f"_mm256_cast{h}128_{a}256" if a != "si256" else "_mm256_castsi128_si256", [(H, "__a")], f"return {V(A, '', '__a')};")
    fn(A, f"_mm256_zext{h}128_{a}256" if a != "si256" else "_mm256_zextsi128_si256", [(H, "__a")], f"return {V(A, '', '__a')};")
fn(S, "_mm256_set_m128", [("__m128", "__h"), ("__m128", "__l")], "return _mm256_insertf128_ps(_mm256_castps128_ps256(__l), __h, 1);")
fn(SD, "_mm256_set_m128d", [(D, "__h"), (D, "__l")], "return _mm256_insertf128_pd(_mm256_castpd128_pd256(__l), __h, 1);")
fn(SI, "_mm256_set_m128i", [(I, "__h"), (I, "__l")], "return _mm256_insertf128_si256(_mm256_castsi128_si256(__l), __h, 1);")
fn(S, "_mm256_setr_m128", [("__m128", "__l"), ("__m128", "__h")], "return _mm256_set_m128(__h, __l);")
fn(SD, "_mm256_setr_m128d", [(D, "__l"), (D, "__h")], "return _mm256_set_m128d(__h, __l);")
fn(SI, "_mm256_setr_m128i", [(I, "__l"), (I, "__h")], "return _mm256_set_m128i(__h, __l);")
fn(S, "_mm256_loadu2_m128", [("float const *", "__ph"), ("float const *", "__pl")], "return _mm256_set_m128(_mm_loadu_ps(__ph), _mm_loadu_ps(__pl));")
fn("void", "_mm256_storeu2_m128", [("float *", "__ph"), ("float *", "__pl"), (S, "__a")], "_mm_storeu_ps(__pl, _mm256_castps256_ps128(__a)); _mm_storeu_ps(__ph, _mm256_extractf128_ps(__a, 1));")
fn("void", "_mm256_zeroupper", [], "")
fn("void", "_mm256_zeroall", [], "")
macro("_mm256_extract_epi32", ["a", "i"], "_mm_extract_epi32(_mm256_extractf128_si256((a), (i) >> 2), (i) & 3)")
macro("_mm256_extract_epi64", ["a", "i"], "_mm_extract_epi64(_mm256_extractf128_si256((a), (i) >> 1), (i) & 1)")
macro("_mm256_extract_epi16", ["a", "i"], "_mm_extract_epi16(_mm256_extractf128_si256((a), (i) >> 3), (i) & 7)")
macro("_mm256_extract_epi8", ["a", "i"], "_mm_extract_epi8(_mm256_extractf128_si256((a), (i) >> 4), (i) & 15)")

# ---------------------------------------------------------------- AVX2
begin("avx2intrin.h", "_AVX2INTRIN_H_INCLUDED", ["avxintrin.h"])
for w, s in [("8", "b"), ("16", "w"), ("32", "d"), ("64", "q")]:
    bin(f"_mm256_add_epi{w}", SI, f"padd{s}")
    bin(f"_mm256_sub_epi{w}", SI, f"psub{s}")
    bin(f"_mm256_cmpeq_epi{w}", SI, f"pcmpeq{s}")
    bin(f"_mm256_cmpgt_epi{w}", SI, f"pcmpgt{s}")
for n, m in [("adds_epi8", "paddsb"), ("adds_epi16", "paddsw"), ("adds_epu8", "paddusb"), ("adds_epu16", "paddusw"),
             ("subs_epi8", "psubsb"), ("subs_epi16", "psubsw"), ("subs_epu8", "psubusb"), ("subs_epu16", "psubusw"),
             ("avg_epu8", "pavgb"), ("avg_epu16", "pavgw"), ("madd_epi16", "pmaddwd"), ("maddubs_epi16", "pmaddubsw"),
             ("max_epi8", "pmaxsb"), ("max_epi16", "pmaxsw"), ("max_epi32", "pmaxsd"), ("max_epu8", "pmaxub"),
             ("max_epu16", "pmaxuw"), ("max_epu32", "pmaxud"), ("min_epi8", "pminsb"), ("min_epi16", "pminsw"),
             ("min_epi32", "pminsd"), ("min_epu8", "pminub"), ("min_epu16", "pminuw"), ("min_epu32", "pminud"),
             ("mul_epi32", "pmuldq"), ("mul_epu32", "pmuludq"), ("mulhi_epi16", "pmulhw"), ("mulhi_epu16", "pmulhuw"),
             ("mulhrs_epi16", "pmulhrsw"), ("mullo_epi16", "pmullw"), ("mullo_epi32", "pmulld"), ("sad_epu8", "psadbw"),
             ("and_si256", "pand"), ("andnot_si256", "pandn"), ("or_si256", "por"), ("xor_si256", "pxor"),
             ("packs_epi16", "packsswb"), ("packs_epi32", "packssdw"), ("packus_epi16", "packuswb"),
             ("packus_epi32", "packusdw"), ("shuffle_epi8", "pshufb"), ("sign_epi8", "psignb"),
             ("sign_epi16", "psignw"), ("sign_epi32", "psignd"), ("hadd_epi16", "phaddw"), ("hadd_epi32", "phaddd"),
             ("hadds_epi16", "phaddsw"), ("hsub_epi16", "phsubw"), ("hsub_epi32", "phsubd"), ("hsubs_epi16", "phsubsw"),
             ("unpackhi_epi8", "punpckhbw"), ("unpackhi_epi16", "punpckhwd"), ("unpackhi_epi32", "punpckhdq"),
             ("unpackhi_epi64", "punpckhqdq"), ("unpacklo_epi8", "punpcklbw"), ("unpacklo_epi16", "punpcklwd"),
             ("unpacklo_epi32", "punpckldq"), ("unpacklo_epi64", "punpcklqdq"), ("sllv_epi32", "psllvd"),
             ("sllv_epi64", "psllvq"), ("srlv_epi32", "psrlvd"), ("srlv_epi64", "psrlvq"), ("srav_epi32", "psravd")]:
    bin("_mm256_" + n, SI, m)
for n, m in [("sllv_epi32", "psllvd"), ("sllv_epi64", "psllvq"), ("srlv_epi32", "psrlvd"), ("srlv_epi64", "psrlvq"), ("srav_epi32", "psravd")]:
    bin("_mm_" + n, I, m)
for n, m in [("abs_epi8", "pabsb"), ("abs_epi16", "pabsw"), ("abs_epi32", "pabsd")]:
    un("_mm256_" + n, SI, m)
binI("_mm256_alignr_epi8", SI, "palignr")
binI("_mm256_blend_epi16", SI, "pblendw")
binI("_mm256_blend_epi32", SI, "pblendd")
binI("_mm_blend_epi32", I, "pblendd")
fn(SI, "_mm256_blendv_epi8", [(SI, "__a"), (SI, "__b"), (SI, "__m")], f"return {V(SI, 'pblendvb', '__a', '__b', '__m')};")
binI("_mm256_mpsadbw_epu8", SI, "mpsadbw")
toGPR("_mm256_movemask_epi8", "int", "pmovmskb", SI)
for s, m in [("16", "w"), ("32", "d"), ("64", "q")]:
    fn(SI, f"_mm256_slli_epi{s}", [(SI, "__a"), ("int", "__n")], f"return {V(SI, 'psll' + m, '__a', '__n')};")
    fn(SI, f"_mm256_srli_epi{s}", [(SI, "__a"), ("int", "__n")], f"return {V(SI, 'psrl' + m, '__a', '__n')};")
    bin(f"_mm256_sll_epi{s}", SI, "psll" + m, SI, I)
    bin(f"_mm256_srl_epi{s}", SI, "psrl" + m, SI, I)
for s, m in [("16", "w"), ("32", "d")]:
    fn(SI, f"_mm256_srai_epi{s}", [(SI, "__a"), ("int", "__n")], f"return {V(SI, 'psra' + m, '__a', '__n')};")
    bin(f"_mm256_sra_epi{s}", SI, "psra" + m, SI, I)
macro("_mm256_slli_si256", ["a", "imm"], VI(SI, "pslldq", "(imm)", f"({SI})(a)"))
macro("_mm256_srli_si256", ["a", "imm"], VI(SI, "psrldq", "(imm)", f"({SI})(a)"))
macro("_mm256_bslli_epi128", ["a", "imm"], VI(SI, "pslldq", "(imm)", f"({SI})(a)"))
macro("_mm256_bsrli_epi128", ["a", "imm"], VI(SI, "psrldq", "(imm)", f"({SI})(a)"))
unI("_mm256_shuffle_epi32", SI, "pshufd")
unI("_mm256_shufflehi_epi16", SI, "pshufhw")
unI("_mm256_shufflelo_epi16", SI, "pshuflw")
for f, t in [("epi8_epi16", "bw"), ("epi8_epi32", "bd"), ("epi8_epi64", "bq"), ("epi16_epi32", "wd"), ("epi16_epi64", "wq"), ("epi32_epi64", "dq")]:
    un(f"_mm256_cvt{f}", SI, "pmovsx" + t, I)
    un(f"_mm256_cvt{f.replace('epi', 'epu', 1)}", SI, "pmovzx" + t, I)
macro("_mm256_extracti128_si256", ["a", "imm"], VI(I, "extracti128", "(imm)", f"({SI})(a)"))
macro("_mm256_inserti128_si256", ["a", "b", "imm"], VI(SI, "inserti128", "(imm)", f"({SI})(a)", f"({I})(b)"))
binI("_mm256_permute2x128_si256", SI, "perm2i128")
unI("_mm256_permute4x64_epi64", SI, "permq")
unI("_mm256_permute4x64_pd", SD, "permpd")
fn(SI, "_mm256_permutevar8x32_epi32", [(SI, "__a"), (SI, "__i")], f"return {V(SI, 'permd', '__i', '__a')};")
fn(S, "_mm256_permutevar8x32_ps", [(S, "__a"), (SI, "__i")], f"return {V(S, 'permps', '__i', '__a')};")
for n, m, T, A in [("broadcastb_epi8", "pbroadcastb", SI, I), ("broadcastw_epi16", "pbroadcastw", SI, I),
                   ("broadcastd_epi32", "pbroadcastd", SI, I), ("broadcastq_epi64", "pbroadcastq", SI, I),
                   ("broadcastss_ps", "broadcastss", S, "__m128"), ("broadcastsd_pd", "broadcastsd", SD, D)]:
    un("_mm256_" + n, T, m, A)
for n, m, T, A in [("broadcastb_epi8", "pbroadcastb", I, I), ("broadcastw_epi16", "pbroadcastw", I, I),
                   ("broadcastd_epi32", "pbroadcastd", I, I), ("broadcastq_epi64", "pbroadcastq", I, I),
                   ("broadcastss_ps", "broadcastss", "__m128", "__m128")]:
    un("_mm_" + n, T, m, A)
fn(D, "_mm_broadcastsd_pd", [(D, "__a")], "return _mm_unpacklo_pd(__a, __a);")
fn(SI, "_mm256_broadcastsi128_si256", [(I, "__a")], "return _mm256_inserti128_si256(_mm256_castsi128_si256(__a), __a, 1);")
fn(SI, "_mm_broadcastsi128_si256", [(I, "__a")], "return _mm256_inserti128_si256(_mm256_castsi128_si256(__a), __a, 1);")
fn(SI, "_mm256_maskload_epi32", [("int const *", "__p"), (SI, "__m")], f"return {V(SI, 'pmaskmovd', '(long long)__p', '__m')};")
fn(SI, "_mm256_maskload_epi64", [("long long const *", "__p"), (SI, "__m")], f"return {V(SI, 'pmaskmovq', '(long long)__p', '__m')};")
fn(I, "_mm_maskload_epi32", [("int const *", "__p"), (I, "__m")], f"return {V(I, 'pmaskmovd', '(long long)__p', '__m')};")
fn(I, "_mm_maskload_epi64", [("long long const *", "__p"), (I, "__m")], f"return {V(I, 'pmaskmovq', '(long long)__p', '__m')};")
fn(SI, "_mm256_stream_load_si256", [(SI + " const *", "__p")], "return *__p;")

# ---------------------------------------------------------------- FMA
begin("fmaintrin.h", "_FMAINTRIN_H_INCLUDED", ["avxintrin.h"])
for op in ["fmadd", "fmsub", "fnmadd", "fnmsub", "fmaddsub", "fmsubadd"]:
    for t, T in [("ps", "__m128"), ("pd", D)]:
        fn(T, f"_mm_{op}_{t}", [(T, "__a"), (T, "__b"), (T, "__c")], f"return {V(T, op + '213' + t, '__a', '__b', '__c')};")
    for t, T in [("ps", S), ("pd", SD)]:
        fn(T, f"_mm256_{op}_{t}", [(T, "__a"), (T, "__b"), (T, "__c")], f"return {V(T, op + '213' + t, '__a', '__b', '__c')};")
    if op in ("fmadd", "fmsub", "fnmadd", "fnmsub"):
        for t, T in [("ss", "__m128"), ("sd", D)]:
            fn(T, f"_mm_{op}_{t}", [(T, "__a"), (T, "__b"), (T, "__c")], f"return {V(T, op + '213' + t, '__a', '__b', '__c')};")

# ---------------------------------------------------------------- F16C
begin("f16cintrin.h", "_F16CINTRIN_H_INCLUDED", ["avxintrin.h"])
un("_mm_cvtph_ps", "__m128", "cvtph2ps", I)
un("_mm256_cvtph_ps", S, "cvtph2ps", I)
macro("_mm_cvtps_ph", ["a", "imm"], VI(I, "cvtps2ph", "(imm)", "(__m128)(a)"))
macro("_mm256_cvtps_ph", ["a", "imm"], VI(I, "cvtps2ph", "(imm)", f"({S})(a)"))
emit("__ALTC_INL float _cvtsh_ss(unsigned short __h) { return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(__h))); }")
emit("#define _cvtss_sh(f, imm) ((unsigned short)_mm_cvtsi128_si32(_mm_cvtps_ph(_mm_set_ss(f), (imm))))")

# ---------------------------------------------------------------- BMI, BMI2, LZCNT, POPCNT
begin("bmiintrin.h", "_BMIINTRIN_H_INCLUDED", ["altc_intrin_common.h"])
for b, t in [("32", "unsigned int"), ("64", "unsigned long long")]:
    fn(t, f"_andn_u{b}", [(t, "__a"), (t, "__b")], "return ~__a & __b;")
    fn(t, f"__andn_u{b}", [(t, "__a"), (t, "__b")], "return ~__a & __b;")
    fn(t, f"_blsi_u{b}", [(t, "__a")], "return __a & -__a;")
    fn(t, f"__blsi_u{b}", [(t, "__a")], "return __a & -__a;")
    fn(t, f"_blsmsk_u{b}", [(t, "__a")], "return __a ^ (__a - 1);")
    fn(t, f"__blsmsk_u{b}", [(t, "__a")], "return __a ^ (__a - 1);")
    fn(t, f"_blsr_u{b}", [(t, "__a")], "return __a & (__a - 1);")
    fn(t, f"__blsr_u{b}", [(t, "__a")], "return __a & (__a - 1);")
    n = b
    fn(t, f"_bextr_u{b}", [(t, "__x"), ("unsigned int", "__s"), ("unsigned int", "__l")],
       f"return __l == 0 || __s >= {n} ? 0 : (__x >> __s) & (__l >= {n} ? ({t})-1 : ((({t})1 << __l) - 1));")
    fn(t, f"__bextr_u{b}", [(t, "__x"), ("unsigned int", "__c")], f"return _bextr_u{b}(__x, __c & 0xff, (__c >> 8) & 0xff);")
    cz = "__builtin_ctz" if b == "32" else "__builtin_ctzll"
    fn(t, f"_tzcnt_u{b}", [(t, "__a")], f"return __a ? {cz}(__a) : {n};")
    fn(t, f"__tzcnt_u{b}", [(t, "__a")], f"return __a ? {cz}(__a) : {n};")
emit("#define _tzcnt_u16(a) ((unsigned short)((a) ? __builtin_ctz((unsigned short)(a)) : 16))")
begin("lzcntintrin.h", "_LZCNTINTRIN_H_INCLUDED", ["altc_intrin_common.h"])
fn("unsigned int", "_lzcnt_u32", [("unsigned int", "__a")], "return __a ? __builtin_clz(__a) : 32;")
fn("unsigned int", "__lzcnt32", [("unsigned int", "__a")], "return __a ? __builtin_clz(__a) : 32;")
fn("unsigned long long", "_lzcnt_u64", [("unsigned long long", "__a")], "return __a ? __builtin_clzll(__a) : 64;")
fn("unsigned long long", "__lzcnt64", [("unsigned long long", "__a")], "return __a ? __builtin_clzll(__a) : 64;")
fn("unsigned short", "__lzcnt16", [("unsigned short", "__a")], "return __a ? __builtin_clz(__a) - 16 : 16;")
begin("bmi2intrin.h", "_BMI2INTRIN_H_INCLUDED", ["altc_intrin_common.h"])
for b, t in [("32", "unsigned int"), ("64", "unsigned long long")]:
    fn(t, f"_bzhi_u{b}", [(t, "__x"), ("unsigned int", "__i")], f"return {V(t, 'bzhi', '__x', '(' + t + ')__i')};")
    fn(t, f"_pdep_u{b}", [(t, "__x"), (t, "__m")], f"return {V(t, 'pdep', '__x', '__m')};")
    fn(t, f"_pext_u{b}", [(t, "__x"), (t, "__m")], f"return {V(t, 'pext', '__x', '__m')};")
fn("unsigned int", "_mulx_u32", [("unsigned int", "__x"), ("unsigned int", "__y"), ("unsigned int *", "__p")],
   "unsigned long long __r = (unsigned long long)__x * __y; *__p = (unsigned int)(__r >> 32); return (unsigned int)__r;")
fn("unsigned long long", "_mulx_u64", [("unsigned long long", "__x"), ("unsigned long long", "__y"), ("unsigned long long *", "__p")],
   "unsigned __int128 __r = (unsigned __int128)__x * __y; *__p = (unsigned long long)(__r >> 64); return (unsigned long long)__r;")
begin("popcntintrin.h", "_POPCNTINTRIN_H_INCLUDED", ["altc_intrin_common.h"])
fn("int", "_mm_popcnt_u32", [("unsigned int", "__x")], "return __builtin_popcount(__x);")
fn("long long", "_mm_popcnt_u64", [("unsigned long long", "__x")], "return __builtin_popcountll(__x);")

# ---------------------------------------------------------------- umbrella headers
begin("immintrin.h", "_IMMINTRIN_H_INCLUDED", ["avx2intrin.h", "fmaintrin.h", "f16cintrin.h", "wmmintrin.h", "bmiintrin.h", "bmi2intrin.h", "lzcntintrin.h", "popcntintrin.h", "ia32intrin.h"])
begin("x86intrin.h", "_X86INTRIN_H_INCLUDED", ["immintrin.h"])
begin("x86gprintrin.h", "_X86GPRINTRIN_H_INCLUDED", ["bmiintrin.h", "bmi2intrin.h", "lzcntintrin.h", "popcntintrin.h", "ia32intrin.h"])
begin("ia32intrin.h", "_IA32INTRIN_H_INCLUDED", ["altc_intrin_common.h"])
emit("""__ALTC_INL int __bsfd(int __x) { return __builtin_ctz(__x); }
__ALTC_INL int __bsrd(int __x) { return __builtin_clz(__x) ^ 31; }
__ALTC_INL int __bswapd(int __x) { return __builtin_bswap32(__x); }
__ALTC_INL long long __bsfq(long long __x) { return __builtin_ctzll(__x); }
__ALTC_INL long long __bsrq(long long __x) { return __builtin_clzll(__x) ^ 63; }
__ALTC_INL long long __bswapq(long long __x) { return __builtin_bswap64(__x); }
__ALTC_INL int __popcntd(unsigned int __x) { return __builtin_popcount(__x); }
__ALTC_INL long long __popcntq(unsigned long long __x) { return __builtin_popcountll(__x); }
__ALTC_INL unsigned int __rold(unsigned int __x, int __c) { __c &= 31; return (__x << __c) | (__x >> ((32 - __c) & 31)); }
__ALTC_INL unsigned int __rord(unsigned int __x, int __c) { __c &= 31; return (__x >> __c) | (__x << ((32 - __c) & 31)); }
__ALTC_INL unsigned long long __rolq(unsigned long long __x, int __c) { __c &= 63; return (__x << __c) | (__x >> ((64 - __c) & 63)); }
__ALTC_INL unsigned long long __rorq(unsigned long long __x, int __c) { __c &= 63; return (__x >> __c) | (__x << ((64 - __c) & 63)); }
__ALTC_INL unsigned char __rolb(unsigned char __x, int __c) { __c &= 7; return (unsigned char)((__x << __c) | (__x >> ((8 - __c) & 7))); }
__ALTC_INL unsigned char __rorb(unsigned char __x, int __c) { __c &= 7; return (unsigned char)((__x >> __c) | (__x << ((8 - __c) & 7))); }
__ALTC_INL unsigned short __rolw(unsigned short __x, int __c) { __c &= 15; return (unsigned short)((__x << __c) | (__x >> ((16 - __c) & 15))); }
__ALTC_INL unsigned short __rorw(unsigned short __x, int __c) { __c &= 15; return (unsigned short)((__x >> __c) | (__x << ((16 - __c) & 15))); }
static __inline__ unsigned long long __rdtsc(void) { unsigned int __lo, __hi; __asm__ __volatile__("rdtsc" : "=a"(__lo), "=d"(__hi)); return ((unsigned long long)__hi << 32) | __lo; }
static __inline__ unsigned long long __rdtscp(unsigned int *__aux) { unsigned int __lo, __hi; __asm__ __volatile__("rdtscp" : "=a"(__lo), "=d"(__hi), "=c"(*__aux)); return ((unsigned long long)__hi << 32) | __lo; }
#define _bit_scan_forward(a) __bsfd(a)
#define _bit_scan_reverse(a) __bsrd(a)
#define _bswap(a) __bswapd(a)
#define _bswap64(a) __bswapq(a)
#define _popcnt32(a) __popcntd(a)
#define _popcnt64(a) __popcntq(a)
#define _rotl(a, b) __rold((a), (b))
#define _rotr(a, b) __rord((a), (b))
#define _rotwl(a, b) __rolw((a), (b))
#define _rotwr(a, b) __rorw((a), (b))
#define _lrotl(a, b) __rolq((a), (b))
#define _lrotr(a, b) __rorq((a), (b))
#define _rdtsc() __rdtsc()
#define _rdtscp(a) __rdtscp(a)""")
begin("mm_malloc.h", "_MM_MALLOC_H_INCLUDED")
emit("""#include <stdlib.h>
static __inline__ void *_mm_malloc(size_t __size, size_t __align) {
  void *__p;
  if (__align == 1) return malloc(__size);
  if (__align == 2 || (sizeof(void *) == 8 && __align == 4)) __align = sizeof(void *);
  if (posix_memalign(&__p, __align, __size) == 0) return __p;
  return (void *)0;
}
static __inline__ void _mm_free(void *__p) { free(__p); }""")
begin("mmintrin.h", "_MMINTRIN_H_INCLUDED", ["altc_intrin_common.h"])
emit("/* MMX intrinsics are not supported; the type is (for pointer casts like\n   (__m64 *)p in SSE code) */\ntypedef int __m64 __attribute__((__vector_size__(8), __may_alias__));\n__ALTC_INL void _mm_empty(void) {}")

# ---------------------------------------------------------------- write
for name, (guard, includes, body, protos) in files.items():
    with open(os.path.join(OUT, name), "w") as f:
        f.write(f"/* Generated by include/intrin/gen.py: x86 intrinsics for AltC. Do not edit. */\n")
        f.write(f"#ifndef {guard}\n#define {guard}\n")
        for inc in includes:
            f.write(f"#include <{inc}>\n")
        if protos:
            f.write("\n".join(protos) + "\n\n")
        f.write("\n".join(body) + "\n")
        f.write(f"#endif\n")
print("wrote", len(files), "headers")
