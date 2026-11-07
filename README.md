# AltC

A C compiler for linux/amd64 written in pure Go, with a built-in
preprocessor, assembler, linker and C library, plus a small IDE. One binary
turns a folder of C (and assembler) sources into a ready-to-run executable,
the way the Go toolchain does: nothing else has to be installed, not gcc,
not binutils, not even a C library. Executables are static and run on any
x86-64 Linux kernel.

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
```

## The C library

AltC carries the sources of [musl](https://musl.libc.org) 1.2.5 (MIT
license) in the binary (`libc/`). The first link compiles them with AltC
itself, in a few seconds, into the build cache (`$ALTC_CACHE`, else
`~/.cache/altc`); later links take the library from there. The cache key
is the hash of the compiler's executable, so every build of AltC uses a
library it compiled, and older entries are pruned. Every program is
linked statically with it: `crt1.o`, `crti.o`, the objects, `libc.a`,
`crtn.o`, and a small runtime of the helpers gcc takes from libgcc
(`__int128` division, `_Complex` multiplication and division,
`__cpu_model`). `-lm`, `-lpthread`, `-lrt`, `-ldl` and the other parts of
the C library are accepted and add nothing; other `-l` libraries must be
static archives (`lib<name>.a`) in the `-L` directories. `-static` is
accepted and changes nothing.

- The headers are musl's, after AltC's own (`include/c/`: the intrinsic
  headers, `<stdatomic.h>`, `<stdckdint.h>`, `<cpuid.h>`, and
  `<stdbit.h>` and the C23 additions to `<stddef.h>`, which musl 1.2.5
  does not have). Nothing is read from `/usr/include`.
- `libc/altc/` holds AltC's additions to the library (the `<stdbit.h>`
  functions, the startup code that applies `R_X86_64_IRELATIVE`
  relocations of IFUNC symbols, which musl's static startup does not).
- `libc/patches/` holds the changes to musl (the CVE-2025-26519 fix of
  `iconv`). `go run ./libc/vendor <musl source dir>` vendors a release
  again and applies them.
- `TestLibcAgainstGCC` builds the same library with gcc and musl's flags
  and checks that programs exercising formatted I/O, libm (every function,
  three precisions, special values, rounding modes, exceptions), strings,
  wide characters, time, regex, iconv, malloc, threads, signals and
  `setjmp` print exactly the same with both.

## The standard library

Besides the C library, AltC carries a standard library in the spirit of
Go's (`std/`, headers `<alt/...>`), written for AltC in C with a classic
C API and built into the cache like the C library. A web server:

```c
#include <alt/net/http.h>

static void hello(alt_http_rw *w, alt_http_request *r, void *arg) {
	alt_http_rw_printf(w, "hello, %.*s\n", (int)r->url.path.n, r->url.path.p);
}

int main(void) {
	alt_http_mux *m;
	alt_http_mux_new(&m);
	alt_http_mux_handle_func(m, "GET /{name}", hello, 0);
	return alt_http_listen_and_serve(":8080", alt_http_mux_handler(m)) < 0;
}
```

- `<alt/task.h>`: tasks (coroutines with their own stacks) on schedulers,
  one per thread; any thread can run one (a UI thread too). Blocking calls
  of the library let the other tasks run.
- `<alt/chan.h>`: channels and `select` between tasks and threads;
  `<alt/sync.h>`: mutex, rwmutex, condition variables, once, wait groups,
  semaphores, the thread pool; `<alt/context.h>`: cancellation and
  deadlines; `<alt/timer.h>`: timers and tickers; `<alt/time.h>`:
  monotonic and calendar time, Go layouts, durations.
- `<alt/str.h>`, `<alt/arena.h>`, `<alt/io.h>` (readers, writers, bufio),
  `<alt/unicode/utf8.h>`, `<alt/errors.h>` (negative error codes).
- `<alt/net.h>`: TCP, UDP and Unix sockets with deadlines, IP addresses,
  an own DNS resolver (hosts, resolv.conf, UDP and TCP); `<alt/net/url.h>`;
  `<alt/net/http.h>`: an HTTP/1.1 server (keep-alive, chunked bodies,
  timeouts, graceful shutdown, Go 1.22 patterns in the multiplexer, a file
  server with ranges) and client (connection pool, redirects, transparent
  gzip); `<alt/mime.h>`.
- `<alt/strconv.h>`, `<alt/unicode.h>` (classes and case mapping with Go's
  tables), `<alt/log.h>` (structured logging as log/slog, text and JSON),
  `<alt/flag.h>` (command-line flags as Go's flag).
- `<alt/encoding/{hex,base64,json,pem,asn1}.h>`; `<alt/hash/{crc32,crc64,adler32,fnv}.h>`;
  `<alt/compress/{flate,gzip,zlib}.h>`.
- `<alt/crypto/...>`: `hash.h` (MD5, SHA-1, SHA-2, HMAC, random bytes),
  `kdf.h` (HKDF, PBKDF2), `aes.h` (AES-NI, CTR, CBC, GCM),
  `chacha20poly1305.h`, `ed25519.h` (Ed25519, X25519), `ecdsa.h` (P-256,
  P-384, P-521: ECDSA, ECDH), `rsa.h` (PKCS #1 v1.5, PSS), `x509.h`
  (certificates, chains, system roots, private keys), `tls.h` (TLS 1.3
  client and server). `alt_http_listen_and_serve_tls` and `https://` URLs
  in the client use it.

Its tests (`TestStd*`, `TestDiff*`, `TestHTTP*`, `TestMuxAgainstGo`,
`TestCompress*`, `TestX509*`, `TestTLS*`, `TestHTTPS`, `TestResolver`)
check it against Go's packages as references (and `tests/goref`, a
separate module with golang.org/x/crypto from the module cache, for what
Go's standard library lacks; those tests are skipped without it): the same inputs to Go and to AltC, the outputs compared (time
layouts, URLs, IP addresses, numbers, JSON, all Unicode code points, log
records, flag parsing and usage texts, hashes, decompression of valid and
damaged streams with the same errors at the same offsets, HTTP request
parsing at the server, the client against Go's client, multiplexer
decisions, ciphers and signatures, certificate parsing and chain
verification); data compressed by AltC is decompressed by Go, signatures
made by AltC are verified by Go, the TLS client and server talk to Go's;
the C tests also run under AddressSanitizer, UndefinedBehaviorSanitizer
and ThreadSanitizer (with gcc), and gcc -Wall -Wextra checks the sources.

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
  `.eh_frame` (and the linker builds `.eh_frame_hdr`), for debuggers and
  profilers. The assembler turns `.cfi_*` directives into `.eh_frame`
  byte for byte like GNU as.
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
      elf/  objects, archives, linker ─► static ELF executable
      libc/ musl sources, compiled on first use into the build cache
```

- The linker lays out TLS, init/fini arrays (by priority), IFUNC slots
  (`.rela.iplt`) and `.eh_frame_hdr`, and keeps the `.debug_*` sections.
- Objects from gcc (`-fno-pic`, PIE or `-fPIC`) and static archives can be
  linked in. General and local dynamic TLS accesses of `-fPIC` code are
  relaxed to local exec, as GNU ld does. Weak undefined references do not
  pull archive members.
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
go test ./tests -run TestLibcAgainstGCC       # the C library vs gcc's build
go test ./x64 -run 'Encoding|AssembleText'    # encoder vs GNU as
ASM_COMPARE_DIR=dir ASM_COMPARE_MATCH=/x86_64/ go test ./x64 -run AssembleFiles
```

The comparison tests use gcc and GNU as only as a reference and are
skipped without them. gcc compiles with the built-in library's headers
(`-nostdinc`, gcc's own headers, then musl's), and AltC's linker links its
objects with the built-in library, so both programs use the same C
library. `ALTC_TEST_KEEP=dir` keeps the files of the tests.

Environment variables for working on the IR backend:

```sh
ALTC_NOIR=1          # use the stack-machine generator for every function
ALTC_IR_STATS=1      # report which functions use the IR and why others do not
ALTC_IR_DUMP=name    # print the IR of a function after allocation (* for all)
ALTC_IR_STRICT=1     # fail on internal IR errors instead of falling back
ALTC_NOINLINE=1      # no inlining (ALTC_NOINLINE_FN=f,g: not these functions)
ALTC_NOVECTORIZE=1   # no loop vectorization
ALTC_TEST_FLAGS=-march=haswell go test ./tests   # the suite with other flags
ALTC_CACHE=dir       # the build cache (the compiled C library)
```

A test can start with `// flags: -mavx2 -mfma` (compiled with these flags
by both compilers, skipped on processors without them) and
`// skip-asm: reason` (not part of TestAssembleGCCOutput).

## License

MIT
