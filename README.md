# AltC

A C compiler for linux/amd64 written in pure Go, with a built-in
preprocessor, assembler and linker, plus a small IDE. One binary turns a
folder of C (and assembler) sources into a ready-to-run executable: no gcc,
binutils or libc development packages are needed, only the system
`libc.so.6`.

## Quick start

```sh
go build -o altc .
./altc init hello        # altc.json + main.c
./altc run hello         # build and run
./altc build myproject   # build a folder into myproject/build/<name>
./altc                   # open the IDE (or: altc ide <folder>)
```

gcc-style use works too:

```sh
altc -std=c11 -O2 -Wall -Iinclude a.c b.c -o prog -lm
altc -c file.c           # ELF object, linkable by GNU ld as well
altc -S file.c           # GNU assembler listing
altc -E file.c           # preprocessed output
altc -c start.S          # assembler sources: .s, or .S with the C preprocessor
altc -march=native f.c   # use the extensions of this processor (AVX2...)
altc -static f.c -o f    # self-contained executable (libc.a, no dynamic loader)
```

## Processor extensions and SIMD

Like gcc, AltC generates code for the x86-64 baseline (SSE2) unless asked
for more:

- `-march=x86-64-v2|x86-64-v3|haswell|skylake|znver3|native|...` and
  `-m<feature>` / `-mno-<feature>` (`sse4.2`, `popcnt`, `avx`, `avx2`,
  `fma`, `bmi`, `bmi2`, `lzcnt`, `f16c`, `aes`, `pclmul`, `sha`...) enable
  extensions and define the same macros as gcc (`__SSE4_2__`, `__AVX2__`...).
  With AVX all SSE instructions use their VEX forms; `popcnt`, `lzcnt` and
  `tzcnt` replace the generic sequences when enabled. In `altc.json` the
  options go in `cflags`.
- GNU vector types (`__attribute__((vector_size(16)))`) with element-wise
  operators, comparisons, `v[i]`, initializers and casts.
- `<immintrin.h>` and the other intrinsic headers (`xmmintrin.h` to
  `avx2intrin.h`, `fmaintrin.h`, `f16cintrin.h`, `wmmintrin.h`,
  `bmi2intrin.h`...), `<cpuid.h>`, `<mm_malloc.h>`; SSE to SSE4.2, AVX,
  AVX2, FMA, F16C, AES, PCLMUL, BMI1/2 intrinsics are inlined.
- `__builtin_cpu_supports("avx2")`, `__builtin_cpu_is`, `__builtin_cpu_init`
  read libgcc's processor model (`__cpu_model`, `__cpu_features2`) like gcc
  does; AltC's runtime provides it, so objects of both compilers link with
  either linker. `__attribute__((target("avx2")))` compiles one function
  for more extensions; `__attribute__((target_clones("avx2", "default")))`
  compiles a version per target and picks one at startup.
- `__attribute__((ifunc("resolver")))` and IFUNC symbols
  (`.type f, @gnu_indirect_function`): the linker calls the resolver at
  startup (`R_X86_64_IRELATIVE`), so gcc's `target_clones` output links too.
- AltC predefines `__GNUC__` 14, `__GNUC_MINOR__` 2 and the other macros
  of gcc 14.2, so code that checks for GNU C takes its GNU paths.
- Loops over arrays (`c[i] = a[i] * k + b[i]`, `sum += a[i]`, `s -= a[i]`)
  are vectorized automatically, with a run-time check that the arrays do
  not overlap; a few scalar iterations first align the stored array to the
  vector size, and loop heads start on 16-byte boundaries. Loops with
  conditions inside (`c[i] = a[i] > 0 ? a[i] : 0`, `if (a[i] > k) s +=
  a[i]`, `&&`, `||`, `if`/`else`) are if-converted: comparisons become
  masks and the results are blended. Floating-point reductions (`+`, `-`,
  `*`) are vectorized only with `-ffast-math` (or `-fassociative-math`,
  `-Ofast`), which also defines `__FAST_MATH__`.
- Unwind tables: every function gets call frame information in
  `.eh_frame` (and the linker builds `.eh_frame_hdr`), so `backtrace()`,
  gdb, `pthread_cancel` cleanups and C++ exceptions work through AltC
  code. The assembler turns `.cfi_*` directives into `.eh_frame` byte for
  byte like GNU as; programs mixing C++ and AltC objects link with
  `-lstdc++` (libgcc_s is added when needed).
- `-g`: DWARF 4 debug information — the line table, the functions, and at
  `-O0` the parameters, locals, globals and their types (structures,
  bit-fields, arrays, pointers), so gdb can set breakpoints, step, show
  values and backtraces. Optimized code is described too: parameters and
  locals get location lists (registers, spill slots or constants over the
  code where they hold the variable's value, from the register
  allocator's live ranges, starting where the assignment is in the
  program), and inlined calls are `DW_TAG_inlined_subroutine`s with their
  own variables, so `bt` shows them and `break` on an inlined function
  stops in every copy. `-g` does not change the generated code. The linker keeps the `.debug_*` sections of
  all objects (gcc's DWARF 5 too), the assembler understands `.file`,
  `.loc`, `.uleb128` and `.sleb128`, and `-S -g` emits `.file`/`.loc`.

## Project folder

Without `altc.json` every `*.c`, `*.s` and `*.S` file under the folder is
compiled (hidden folders and `build/` are skipped), and the folder and its `include/`
subfolder are on the include path. `altc.json` can set:

```json
{
  "name": "app",
  "std": "c17",
  "sources": ["src", "main.c"],
  "exclude": ["src/experimental"],
  "includeDirs": ["third_party/include"],
  "defines": ["DEBUG", "VERSION=3"],
  "libs": ["z"],
  "libDirs": ["/opt/lib"],
  "cflags": ["-w"],
  "static": false,
  "output": "bin/app",
  "args": ["--verbose"]
}
```

## Language support

- Standards: C89/C90, C99, C11, C17, C23 and the GNU dialects (`-std=gnu17`
  is the default).
- Full preprocessor: variadic macros, `__VA_OPT__`, `#`/`##`,
  `__has_include`, `__has_builtin`, `#include_next`, `#embed`, `_Pragma`,
  `#pragma once`, `#pragma pack`.
- All integer and floating types, including `long double` (x87),
  `__int128` and `_Complex` (with `<complex.h>`); bit-fields; VLAs; flexible array members; designated
  initializers; compound literals; `_Generic`; `_Static_assert`;
  `_Alignas`/`_Alignof`; `_Thread_local`; `_Atomic` with `<stdatomic.h>`;
  K&R function definitions.
- C23: `bool`/`true`/`false`, `nullptr`, `typeof`, `constexpr`, `auto`,
  `[[attributes]]`, binary literals, digit separators, `static_assert`
  without a message, `#elifdef`, `<stdckdint.h>`.
- GNU extensions: statement expressions, labels as values (`&&label`,
  `goto *p`), case ranges, `__attribute__` (aligned, packed, cleanup,
  constructor/destructor with priorities, weak, alias, section, visibility,
  mode...), `__builtin_*` (va_*, offsetof, overflow checks, bit counting,
  bswap, classification, `__sync_*`, `__atomic_*`...).
- Inline assembly: basic and extended `asm` with all common x86 constraints
  (`r`, `q`, `Q`, `a`...`D`, `A`, `m`, `g`, `i`/`n`, `x`, `t`/`u`, matching
  and named operands, `&`, `+`, flag outputs `=@cc<cond>`), operand
  modifiers (`%b0`, `%k1`, `%h2`, `%c3`, `%l[label]`, `%=`...), clobbers,
  `asm goto`, local register variables (`register long r asm("r10")`) and
  file-scope `asm`.

Not supported yet: AVX-512, MMX
(`__m64`), `__builtin_cpu_is` with processor names (only `"intel"` and
`"amd"` are known; the others give 0).

## Assembler

The built-in AT&T assembler handles inline asm, file-scope asm and `.s`/`.S`
files: general-purpose, x87, SSE up to SSE4.2, AVX, AVX2, FMA, F16C, BMI1/2,
AES, SHA and PCLMUL instructions, short/long jump relaxation, numeric and named labels, label
differences, sections (`.section`, `.pushsection`...), data and symbol
directives (`.globl`, `.type`, `.size`, `.comm`, `.set`...). On gcc's
`-S` output it produces the same bytes and relocations as GNU as.

## How it works

```
.c ─► cc/   tokenizer, preprocessor, parser, type checker
       │
       ▼
      x64/  code generator (System V ABI) and x86-64 encoder
       │
       ▼
      elf/  objects, archives, shared libraries, linker ─► ELF executable
```

- `include/c/` holds the standard headers built into the binary. They are
  layout-compatible with glibc, and missing ones fall back to
  `/usr/include` (mixing with glibc's own headers is supported).
- Programs are linked dynamically against `libc.so.6` and `libm.so.6`. The
  linker generates `_start`, PLT/GOT, TLS, init/fini arrays and `.dynamic`
  itself. What glibc keeps in `libc_nonshared.a` (`atexit`...) and the
  libgcc helpers for `__int128` and `_Complex` come from a small runtime
  compiled on the fly.
- `-static` (or `"static": true` in `altc.json`) links like `gcc -static`:
  `crt1.o`, `crti.o`, `crtbeginT.o`, the objects, `libc.a`, `libm.a`,
  `libgcc.a`, `libgcc_eh.a`, `crtend.o`, `crtn.o` (the `libc6-dev` and
  gcc packages). The linker then writes no `.interp`/`.dynamic`; IFUNCs
  (glibc's `memcpy`, `strlen`...) get `R_X86_64_IRELATIVE` entries in
  `.rela.iplt` that libc applies at startup. C++ (`-lstdc++`) and
  `pthread_cancel` work statically too.
- Objects from gcc (`-fno-pic`, PIE or `-fPIC`) and static archives can be
  linked in. General and local dynamic TLS accesses of `-fPIC` code are
  relaxed to local exec (or initial exec for a variable of a shared
  library), as GNU ld does. Weak undefined references do not pull archive
  members.
- Functions are compiled through an SSA intermediate representation
  (`x64/ir*.go`): lowering builds SSA directly from the AST (locals whose
  address is never taken become values), then constant folding and
  algebraic simplification, global value numbering (with reuse of loads
  while memory is unchanged), loop-invariant code motion, dead code
  elimination, loop rotation, x86 addressing modes, division by constants
  as multiplications, CFG cleanup, inlining of small and always_inline
  functions, and loop vectorization. Registers are assigned by priority
  (uses weighted by loop depth) over live ranges with holes; phis are
  coalesced with their operands; values living across calls get
  callee-saved registers or stack slots.
- `long double` values live in memory in the IR: an expression evaluates
  to the address of its 80-bit value, and each operation is a short x87
  sequence on memory operands (the x87 stack is empty between values).
  Atomic builtins and `_Atomic` operations become locked instructions
  (`lock xadd`, `lock cmpxchg` loops, `xchg`); stores to `_Atomic`
  objects are followed by `mfence`, like gcc.
- `__int128` values are pairs of 64-bit halves in the IR (two SSA
  variables per variable, two registers per argument): `add`/`adc`,
  `sub`/`sbb`, one `mul` for a full 64x64 product, `shld`/`shrd` with
  `cmov` for variable shifts, `cmp`/`sbb` comparisons fused with branches;
  division and float conversions call the libgcc functions. In `-g` output
  the halves of a variable are described as two DWARF pieces. VLAs and
  `alloca` move the stack pointer below the frame (the outgoing argument
  area moves with it); VLA scopes save and restore it, also on `goto` out
  of them and on every loop iteration.
- `_Complex` values are pairs of their parts in the IR, like `__int128`:
  `float` and `double` parts in xmm registers (a `float _Complex` argument
  packed in one register), `long double` parts as x87 values in memory,
  returned in `st(0)` and `st(1)`. Multiplication is inline, with the
  libgcc helper (C11 Annex G) only when both parts come out NaN, as gcc
  does; division calls the helpers, whose runtime versions follow libgcc
  bit for bit. `creal`, `cimag` and `conj` are expanded in place.
- Functions the IR does not cover (structs returned in x87 registers,
  asm goto, K&R definitions, computed goto, `setjmp`) use the older
  stack-machine generator, which keeps the most used locals in callee-saved
  registers. The two mix freely within a file.
- The result is typically close to gcc `-O2` on scalar code, and close to
  `-O3` on simple array loops. `-O0` uses the stack machine without
  register allocation (functions with vector values still go through the
  IR, which alone compiles them); other `-O` levels are accepted and
  ignored. `ALTC_TEST_FLAGS` runs the test suite with other flags, such as
  `-O0 -g` or `-O2 -mavx2`.

## Tests

```sh
go test ./...                                 # everything
go test ./tests -run TestCompareWithGCC       # C programs vs gcc output
ABI_SEEDS=40 go test ./tests -run TestABI     # cross-compiler ABI calls
go test ./tests -run TestAssembleGCCOutput    # gcc -S output vs GNU as, run
go test ./x64 -run 'Encoding|AssembleText'    # encoder vs GNU as
ASM_COMPARE_DIR=dir ASM_COMPARE_MATCH=/x86_64/ go test ./x64 -run AssembleFiles
include/verify/run.sh                         # header layout vs glibc
```

The comparison tests need gcc and GNU as only as a reference.

Environment variables for working on the IR backend:

```sh
ALTC_NOIR=1          # use the stack-machine generator for every function
ALTC_IR_STATS=1      # report which functions use the IR and why others do not
ALTC_IR_DUMP=name    # print the IR of a function after allocation (* for all)
ALTC_IR_STRICT=1     # fail on internal IR errors instead of falling back
ALTC_NOINLINE=1      # no inlining (ALTC_NOINLINE_FN=f,g: not these functions)
ALTC_NOVECTORIZE=1   # no loop vectorization
ALTC_TEST_FLAGS=-march=haswell go test ./tests   # the suite with other flags
```

A test can start with `// flags: -mavx2 -mfma` (compiled with these flags
by both compilers, skipped on processors without them) and
`// skip-asm: reason` (not part of TestAssembleGCCOutput).

## License

MIT
