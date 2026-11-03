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
```

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

Not supported yet: vector types and intrinsics headers, debug information
and unwind tables, AVX (VEX-encoded) instructions; the generated code is
not optimized.

## Assembler

The built-in AT&T assembler handles inline asm, file-scope asm and `.s`/`.S`
files: general-purpose, x87, SSE up to SSE4.2, AES, SHA and PCLMUL
instructions, short/long jump relaxation, numeric and named labels, label
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
  libgcc helpers for `__int128` come from a small runtime compiled on the
  fly.
- Objects from gcc (`-fno-pic` or PIE) and static archives can be linked in.

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

## License

MIT
