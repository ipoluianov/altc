# AltC — plan

A C compiler in pure Go (no cgo, no external as/ld) plus a simple IDE.
Goal: `altc <folder>` → a ready-to-run ELF executable for linux/amd64.

## Architecture

```
.c sources ──► cc/      tokenizer, preprocessor, parser, types, AST
                │
                ▼
               x64/     code generator (SysV ABI) + x86-64 assembler (encoding)
.s/.S sources ─►        AT&T assembler (inline asm, file-scope asm, .s files)
                │
                ▼
               elf/     object model, ELF .o writer/reader, .a archives,
                │       .so dynamic symbol reader, linker
                ▼
         ELF64 executable (ET_EXEC), dynamically linked with libc.so.6 / libm.so.6
```

* `include/` — built-in standard headers (embedded), layout-compatible with
  glibc x86-64. System `/usr/include` is not required.
* libc is the system `libc.so.6` (present on every glibc Linux). The linker
  generates `_start` itself (calls `__libc_start_main`).
* `driver/` — build pipeline: files, project folder (`altc.json` optional),
  parallel compilation, diagnostics.
* `ide/` — a simple IDE on nui: file tree, editor tabs, syntax highlighting,
  output panel, F7 build, F5 run, jump to error.

## Language

Standards: `-std=c89/c90/c99/c11/c17/c23` (and gnu variants). A superset is
accepted: the full preprocessor (variadic macros, `#`/`##`, `__has_include`,
`_Pragma`, `#include_next`, `#elifdef`, `#embed`), all types (x87
`long double`, bit-fields, VLA, flexible array members), designated
initializers, compound literals, `_Generic`, `_Static_assert`, `_Alignas`,
`_Alignof`, `_Noreturn`, `_Thread_local`, `__attribute__` and `[[...]]`
attributes, C23 (`bool/true/false`, `nullptr`, `typeof`, `0b`, digit
separators, `static_assert`, `constexpr`, `auto`), GNU extensions (statement
expressions, `__typeof__`, `&&label`, `case a ... b`, `__builtin_*`).

## Stages

1. x86-64 assembler (encoding) + tests against GNU as.
2. ELF: object model, .o writer, linker (PLT/GOT, .dynamic, TLS, init_array).
3. Tokenizer + preprocessor.
4. Built-in headers.
5. Parser + types + constant expressions.
6. Code generator (SysV ABI: structs by value, varargs, float/double/long double).
7. Driver and CLI (gcc-compatible flags + folder mode).
8. Tests: C programs with output compared to gcc; large projects (Lua, sqlite).
9. IDE.
10. GNU extended inline asm; AT&T assembler for `.s`/`.S` files (verified
    byte for byte against GNU as on gcc's output of the test programs).
11. Register variables (callee-saved registers for integer locals whose
    address is not taken, xmm8-xmm15 for float/double ones), temporaries in
    spare callee-saved registers, local fast paths of the stack machine.
12. SSA IR backend: folding, GVN, LICM, DCE, loop rotation, addressing
    modes, magic division, priority register allocation with phi
    coalescing; fallback to the stack machine for uncovered functions.
13. Processor extensions: -march/-m flags, VEX (AVX/AVX2/FMA/BMI) in the
    assembler and for scalar SSE, GNU vector types, intrinsic headers,
    inlining, inline asm in the IR, target/target_clones,
    __builtin_cpu_supports, loop vectorization.
14. GCC interop: __GNUC__ 14.2, libgcc's processor model (__cpu_model),
    IFUNC symbols and the ifunc attribute (R_X86_64_IRELATIVE).
15. Vectorizer: -ffast-math float reductions, subtraction reductions,
    if-conversion of loops with branches (masks and blends).
16. Unwinding and debugging: .eh_frame from the code generator and from
    .cfi_* directives (identical to GNU as), .eh_frame_hdr and
    PT_GNU_EH_FRAME in the linker, DWARF 4 line tables, functions,
    variables and types (-g), .debug_* sections kept by the linker,
    .loc/.file/.uleb128/.sleb128 in the assembler.
17. Static linking (`-static`): crt objects, libc.a/libgcc.a/libgcc_eh.a,
    .rela.iplt for IFUNCs, .init/.fini ordering, __start_/__stop_ section
    symbols, STB_GNU_UNIQUE; TLS GD/LD relaxation for -fPIC objects.
18. long double (x87 values in memory) and atomics in the IR.
19. DWARF for optimized code: location lists of SSA variables (bound to
    the assignment points), constants, inlined subroutines with their
    variables (.debug_loc, .debug_ranges).
20. __int128 (pairs of 64-bit halves) and VLAs/alloca in the IR; exact
    128-bit constant initializers.
21. _Complex in the IR (pairs of parts; long double parts in memory,
    st(0)/st(1) results); inline multiplication; libgcc-exact runtime
    division; GNU as-compatible jump relaxation.
22. Later: AVX-512 (needs a machine or emulator that can run it to test
    the generated code).

## CLI

```
altc build [dir]           build a project (current dir by default)
altc run [dir] [-- args]   build and run
altc file.c ... -o prog    gcc-like: -c -S -E -I -D -U -L -l -std= -o -w -O0 -g
                           -march= -m<ext> -ffast-math -static
altc ide [dir]             open the IDE (same as no arguments)
```
