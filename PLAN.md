# AltC — plan

A C compiler in pure Go (no cgo, no external as/ld) plus a simple IDE.
Goal: `altc <folder>` → a ready-to-run ELF executable for linux/amd64, with
distribution like Go's: one binary, nothing else installed (no gcc, no
binutils, no C library), static executables.

## Architecture

```
.c sources ──► cc/      tokenizer, preprocessor, parser, types, AST
                │
                ▼
               x64/     code generator (SysV ABI) + x86-64 assembler (encoding)
.s/.S sources ─►        AT&T assembler (inline asm, file-scope asm, .s files)
                │
                ▼
               elf/     object model, ELF .o writer/reader, .a archives, linker
                │
                ▼
         static ELF64 executable (ET_EXEC at 0x400000)
```

* `libc/` — the C library: musl 1.2.5 sources (embedded, x86_64 only,
  headers installed into `musl/include`), AltC's additions (`altc/`) and
  patches (`patches/`). Compiled by AltC on first use into the build cache,
  keyed by the hash of the compiler executable.
* `include/` — the compiler's own headers (intrinsics, `<stdatomic.h>`...),
  searched before musl's. Nothing is read from the system.
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
23. Distribution like Go: musl 1.2.5 embedded and compiled by AltC on
    first use into the build cache (pruned to the last builds); every
    executable static; glibc, the system headers and libraries, libgcc
    and shared libraries dropped. AltC's startup applies IRELATIVE
    relocations (IFUNC, target_clones); `<stdbit.h>` functions and the C23
    `<stddef.h>` additions come from AltC. Tests compare with gcc using
    the same headers and library; `TestLibcAgainstGCC` compares the
    library with gcc's build of it.
24. Next steps of the distribution work:
    - a build cache for project objects (incremental builds, like
      `go build`);
    - the compiler running on other hosts (Windows, macOS) and producing
      Linux executables, then other targets;
    - musl's libc-test suite against AltC's build of the library;
    - `backtrace()` (`<execinfo.h>`) from `.eh_frame`, which musl lacks;
    - dropping the dynamic linking code of `elf/` (no longer used) or
      keeping it for targets that need system libraries.

## Standard library

AltC will ship a standard library for C programs that covers what Go's
standard library covers for Go programs: strings and bytes, strconv,
unicode/utf8, bufio, encoding (json, base64, hex, binary, csv, pem, asn1),
hash, compress, archive, net, net/http, net/url, mime, crypto, time,
math/big, regexp, log, flag, os/exec, path, sort, containers, context,
sync. What the C library already provides (fmt, os, math, syscall) is
not repeated; what is tied to the Go language (reflect, unsafe, runtime,
go/*, testing, plugin, embed) has no counterpart.

- Written by AltC's authors, in C, compiled by AltC like the C library
  (on first use, into the build cache; only what a program uses is
  linked). No third-party sources are vendored for it.
- Implementations follow the standards (RFCs, FIPS, Unicode) and are
  checked against their test vectors and, in tests only, against external
  tools as references.
- Go's standard library (BSD-3-Clause) may be consulted and translated:
  files derived from it keep the Go authors' copyright and refer to
  `LICENSE-Go`. Other code may be consulted only under permissive licenses
  (MIT, BSD, ISC, Apache-2.0, CC0), never GPL/LGPL.
- The API is classic C (functions, structures, return codes, explicit
  ownership of memory), designed anew rather than copied from Go: C has
  no garbage collector, interfaces or goroutines.
- musl stays the C library for now; AltC may patch it (`libc/patches`).

### Concurrency

Programs are network services and UI programs alike, so there are two
kinds of execution:

- OS threads (pthreads), as now: computation, blocking calls of other
  code, background work of UI programs.
- Tasks: lightweight coroutines with their own stacks, always bound to
  the thread that runs them (never migrated: `__thread` variables, `errno`
  and pthread mutexes stay correct). Any thread may run a scheduler: one
  per core for a network service, the main thread of a UI program (the
  display connection is one more file descriptor its poller waits on, so
  UI events, sockets and timers share one loop, and tasks of the UI thread
  touch the UI without locks).
- The scheduler's waiting is an abstraction of its own: epoll first,
  later IOCP/MsgWaitForMultipleObjects on Windows and kqueue/the Cocoa run
  loop on macOS.
- The blocking API (network, HTTP...) is the same in both: called from a
  thread it waits (poll, futex), called from a task it yields.

Communication and synchronization, built on one internal waiter that
parks a task in its scheduler or puts a thread to sleep on a futex; a
wakeup from another thread goes through the target scheduler's lock-free
queue and an eventfd. Every primitive therefore works between tasks,
between tasks and threads and between threads:

- channels (elements copied by value, buffered or not, close, deadlines)
  and `select` over several of them;
- mutex, rwmutex, condition variable, once, wait group, semaphore (a task
  waiting blocks only itself; pthread mutexes keep working but block the
  whole thread);
- timers (sleep, one-shot and periodic timers usable in `select`);
- contexts: cancellation and deadlines inherited by children, taken by
  every blocking network and HTTP operation;
- running a function on another thread's scheduler (for example on the
  UI thread);
- a thread pool for computation and blocking work.

Atomics are `<stdatomic.h>`. The scheduler saves and restores `errno`
when it switches tasks, so plain libc calls (-1 and `errno`) are reliable
inside tasks too.

### Errors

- Functions return `int` (`ssize_t` for counts): `>= 0` is success or a
  count, `< 0` is the negated error code. No hidden state (`errno` would
  be overwritten by other tasks of the thread while a task waits).
- Created objects come back through an out parameter:
  `int alt_listen(alt_listener **out, const char *addr, ...)`.
- System errors are negated `errno` values (`-ETIMEDOUT`, `-ECONNRESET`,
  `-ENOMEM`); on other platforms the library maps native codes to them.
- Errors of a domain have codes of their own above the `errno` range, one
  range per package (`ALT_EHTTP_*`, `ALT_ETLS_*`, `ALT_EJSON_*`);
  `alt_strerror(code)` describes any code.
- Details live in the structures of the API concerned (the offset of a
  JSON syntax error, a TLS alert), never in global state.
- Programming errors (NULL where not allowed, double close, use after
  free that is detected) abort with a message, like a Go panic. Running
  out of memory is an ordinary `-ENOMEM`.

### Time

- `alt_time`: an instant of the monotonic clock in nanoseconds (`int64`,
  about 292 years of range); unaffected by clock changes, used for every
  deadline and timeout.
- `alt_dur`: a duration in nanoseconds (`int64`), with `ALT_US`, `ALT_MS`,
  `ALT_SEC`, `ALT_MIN`, `ALT_HOUR` (`5 * ALT_SEC`).
- Deadlines are absolute (retries share one deadline):
  `alt_deadline(dur)` (now + dur, saturating), `ALT_FOREVER`, `ALT_NOWAIT`
  (any instant in the past means "do not wait").
- `alt_wall`: calendar time, nanoseconds since the Unix epoch in UTC, for
  timestamps, logs, HTTP dates; formatting and parsing (RFC 3339, RFC
  1123, layouts) and time zones (the system tzdata, the `TZ` variable) in
  the time package. The two kinds are never mixed.
- Simple waits (channels, sync, sleep, select) take an `alt_time`
  deadline; network and HTTP operations take an `alt_ctx *` (deadline and
  cancellation, NULL for none); connections also have their own read and
  write deadlines, as in Go.

### API conventions

The aim is Go's capabilities with the means of C, choosing the best
solution for C rather than copying Go's shapes.

- Names: `alt_<package>_<name>` for functions and types
  (`alt_http_server`, `alt_json_parse`), `ALT_<PACKAGE>_<NAME>` for
  constants; core packages drop the package part (`alt_chan_send`,
  `alt_mutex_lock`, `alt_deadline`). Where two Go packages share a last
  element the C name says which (`alt_cryptorand_*`, `alt_rand_*`).
- Headers follow Go's import paths under `alt/`: `<alt/net/http.h>`,
  `<alt/encoding/json.h>`, `<alt/crypto/sha256.h>`, `<alt/sync.h>`.
- Byte strings: `alt_str` (pointer and length) for data that is not
  NUL-terminated or may contain NUL (HTTP headers, JSON strings, binary
  data), so nothing calls strlen twice and slices of a buffer cost
  nothing; functions taking plain C strings say so in their names
  (`_cstr`). `alt_buf` is a growable byte buffer (strings.Builder,
  bytes.Buffer).
- Memory without a garbage collector: whoever creates an object frees it
  (`alt_x_new`/`alt_x_free`); arenas (`alt_arena`) give short-lived work a
  single lifetime: an HTTP request and everything allocated for it (parsed
  headers, JSON values, the response) is freed at once when the handler
  returns. Allocation failures return `-ENOMEM`.
- Interfaces become small structures of function pointers with a context
  (`alt_reader`, `alt_writer`, `alt_handler`), so streams compose as
  io.Reader/io.Writer do (gzip over a connection, JSON into a buffer).
- Generic containers (map, slice helpers, heap, list, sort) work on
  elements of a given size with hash/compare callbacks, and typed wrappers
  are generated by macros (`ALT_MAP(name, key, value)`) for type safety
  and inlining.
- No hidden global state: no allocation or thread creation behind the
  caller's back except where documented (the scheduler's poller, the
  thread pool).

### Layout and build

- `std/include/alt/...` (headers, searched after the compiler's and
  before musl's), `std/src/<package>/` (sources), `std/tests/` (C tests
  with expected outputs, test vectors).
- Built like the C library: on first use, by AltC, into the build cache,
  as an archive linked into every program (only the members a program
  uses end up in it).
- Platform-specific parts (poller, context switch, sockets) are separated
  from the portable code from the start (`_linux_amd64.c`, as Go's file
  suffixes).

### Order of work

Each stage ends with something usable and tested.

1. Core: errors (`alt_strerror`), time (`alt_time`, `alt_wall`, clocks),
   `alt_str`/`alt_buf`, arenas, unicode/utf8; the context switch
   (assembler), tasks, schedulers, the poller (epoll), the waiter,
   `errno` saving.
2. Concurrency: channels and select, sync, timers, contexts, the thread
   pool, running on another thread's scheduler.
3. io and bufio (readers, writers, buffered streams), strconv additions,
   strings/bytes functions, containers and sort.
4. net: TCP, UDP, Unix sockets, listeners and connections with deadlines,
   address parsing (netip); an own DNS resolver (RFC 1035, `/etc/hosts`,
   `/etc/resolv.conf`) that waits in the poller instead of blocking.
5. net/url (RFC 3986), mime, net/http: an HTTP/1.1 server and client (RFC
   9110/9112: keep-alive, chunked, limits, graceful shutdown), a request
   multiplexer with Go 1.22 patterns (`GET /items/{id}`), a file server
   (sendfile, ranges, conditional requests), test helpers. A web server
   without TLS is usable here.
6. encoding: hex, base64, base32, binary, json (DOM and streaming, RFC
   8259), csv, pem; log/slog-style structured logging; flag.
7. hash (crc32, crc64, adler32, fnv) and compress (flate RFC 1951, gzip
   RFC 1952, zlib RFC 1950); archive (tar, zip); HTTP gzip.
8. crypto: rand (getrandom), subtle (constant time), sha1/sha256/sha512,
   hmac, hkdf, pbkdf2, aes (AES-NI with a constant-time fallback) with GCM
   and CTR, chacha20-poly1305, x25519, ed25519, ecdsa/ecdh P-256/P-384,
   rsa (PSS, PKCS#1 v1.5); checked against NIST CAVP, RFC and Wycheproof
   vectors.
9. ASN.1 DER, x509 (RFC 5280: parsing, chain verification, host names),
   root certificates (the system's), TLS 1.3 client and server (RFC
   8446), then TLS 1.2 for older servers; HTTPS and HTTP/2 after that.
10. The rest of Go's set: regexp (RE2's linear-time algorithm), math/big,
    text/template and html/template (contextual escaping), os/exec,
    path/filepath, websocket-level helpers, profiling (pprof-like
    sampling).

Progress (2026-11-07): stages 1, 2 and 4 are done; 3 partly (io, bufio,
strings, strconv with Go's ParseInt/ParseUint/Quote, unicode with Go's
tables; containers and sort to come); 5 nearly (server and client over
HTTP and HTTPS, transparent gzip as Go's Transport, mux, files, url,
mime; multipart, byte ranges and client-side cookies to come); 6 partly
(hex, base64, json, pem, asn1/DER, log/slog-style logging, flag; base32,
binary, csv to come); 7 partly (crc32, crc64, adler32, fnv, flate, gzip,
zlib; archive to come); 8 done for what TLS needs (md5, sha1, sha2, hmac,
hkdf, pbkdf2, rand, AES with AES-NI, CTR, CBC, GCM, ChaCha20-Poly1305,
XChaCha20, X25519, Ed25519, ECDSA and ECDH on P-256/P-384/P-521, RSA
PKCS #1 v1.5 and PSS; RSA key generation and OAEP to come); 9 mostly
(X.509 parsing as Go's ParseCertificate, chain verification as Go's
Verify without policy validation, system roots, PEM, PKCS #8/#1/SEC 1
keys, TLS 1.3 client and server with HRR, ALPN, SNI, client
certificates, KeyUpdate; TLS 1.2, session resumption and HTTP/2 to
come). Each is checked against Go's package by differential tests; TLS
against Go's crypto/tls both ways and against public servers.

Toolchain counterparts of the Go commands come alongside: `altc test`
(test functions in `*_test.c`, like `go test`), later formatting and
static checks.

### Testing

- Every package has C tests with expected results; parsers and decoders
  (HTTP, JSON, DNS, X.509, TLS records, flate) get fuzzers.
- Standards' own vectors (RFC appendices, NIST CAVP, Unicode data,
  Wycheproof) are test data.
- Interoperability against external tools, used only as references and
  skipped without them: curl and python3 against the HTTP server and
  client, openssl s_server/s_client for TLS, gzip/zlib output, dig-like
  answers for DNS.

## CLI

```
altc build [dir]           build a project (current dir by default)
altc run [dir] [-- args]   build and run
altc file.c ... -o prog    gcc-like: -c -S -E -I -D -U -L -l -std= -o -w -O0 -g
                           -march= -m<ext> -ffast-math -static
altc ide [dir]             open the IDE (same as no arguments)
```
