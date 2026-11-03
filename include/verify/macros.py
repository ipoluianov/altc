#!/usr/bin/env python3
"""Compare object-like macro values between the AltC headers and glibc.

For every public header shipped in include/c, collect the object-like
macros it defines (with -D_GNU_SOURCE) both with the AltC headers and with
the system headers.  For macros defined by both, a C program printing the
value and the type of each macro is compiled twice (AltC headers, system
headers) and the outputs are diffed.  Macros that the system header
defines but AltC does not are listed as "missing" (informational).

Usage: macros.py [-v] [workdir]
"""
import os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
INC = os.path.join(os.path.dirname(HERE), 'c')
verbose = '-v' in sys.argv
args = [a for a in sys.argv[1:] if a != '-v']
WORK = args[0] if args else tempfile.mkdtemp(prefix='altc-verify-')
os.makedirs(WORK, exist_ok=True)

ALTC = ['-nostdinc', '-isystem', INC]
SKIP_NAME = re.compile(r'^(__|_[A-Z0-9_]*_(H|H_|SOURCE)$|_[A-Z]+_SOURCE$|'
                       r'_LP64$|_GNU_SOURCE$|_DEFAULT_SOURCE$|_ATFILE_SOURCE$|'
                       r'_ISOC\w+_SOURCE$|_POSIX_C_SOURCE$|_POSIX_SOURCE$|'
                       r'_XOPEN_SOURCE\w*$|_LARGEFILE\w*_SOURCE$|'
                       r'_DYNAMIC_STACK_SIZE_SOURCE$|linux$|unix$|'
                       r'_FORTIFY_SOURCE$)')

def headers():
    hs = []
    for root, dirs, files in os.walk(INC):
        dirs[:] = [d for d in dirs if d != '_altc']
        for f in files:
            if f.endswith('.h'):
                hs.append(os.path.relpath(os.path.join(root, f), INC))
    return sorted(hs)

def macros(hdr, extra):
    src = '#define _GNU_SOURCE 1\n#include <%s>\n' % hdr
    r = subprocess.run(['gcc', '-std=gnu17', '-dM', '-E', '-'] + extra,
                       input=src.encode(), capture_output=True)
    if r.returncode != 0:
        return None
    names = set()
    for line in r.stdout.decode().split('\n'):
        m = re.match(r'#define ([A-Za-z_]\w*) ', line)
        if m and not SKIP_NAME.match(m.group(1)):
            names.add(m.group(1))
    return names

PRELUDE = r'''
#include <stdio.h>
static void p_c (const char *n, char x) { printf ("%s char %d\n", n, x); }
static void p_sc (const char *n, signed char x) { printf ("%s schar %d\n", n, x); }
static void p_uc (const char *n, unsigned char x) { printf ("%s uchar %u\n", n, x); }
static void p_s (const char *n, short x) { printf ("%s short %d\n", n, x); }
static void p_us (const char *n, unsigned short x) { printf ("%s ushort %u\n", n, x); }
static void p_i (const char *n, int x) { printf ("%s int %d\n", n, x); }
static void p_u (const char *n, unsigned x) { printf ("%s uint %u\n", n, x); }
static void p_l (const char *n, long x) { printf ("%s long %ld\n", n, x); }
static void p_ul (const char *n, unsigned long x) { printf ("%s ulong %lu\n", n, x); }
static void p_ll (const char *n, long long x) { printf ("%s llong %lld\n", n, x); }
static void p_ull (const char *n, unsigned long long x) { printf ("%s ullong %llu\n", n, x); }
static void p_f (const char *n, float x) { printf ("%s float %a\n", n, x); }
static void p_d (const char *n, double x) { printf ("%s double %a\n", n, x); }
static void p_ld (const char *n, long double x) { printf ("%s ldouble %La\n", n, x); }
static void p_str (const char *n, const char *x) { printf ("%s str \"%s\"\n", n, x); }
static void p_ptr (const char *n, const void *x) { printf ("%s ptr %p\n", n, x); }
static void p_other (const char *n, ...) { printf ("%s other\n", n); }
#define P(x) _Generic ((x), char: p_c, signed char: p_sc, unsigned char: p_uc, \
  short: p_s, unsigned short: p_us, int: p_i, unsigned: p_u, long: p_l, \
  unsigned long: p_ul, long long: p_ll, unsigned long long: p_ull, \
  float: p_f, double: p_d, long double: p_ld, char *: p_str, \
  const char *: p_str, void *: p_ptr, const void *: p_ptr, \
  default: p_other) (#x, (x))
'''

def build(names, incs, extra, tag):
    """Compile a printer for NAMES; drop names that do not compile."""
    names = list(names)
    while True:
        src = '#define _GNU_SOURCE 1\n'
        src += ''.join('#include <%s>\n' % h for h in incs)
        src += PRELUDE + 'int main (void) {\n'
        first = src.count('\n') + 1
        for n in names:
            src += '  P (%s);\n' % n
        src += '  return 0;\n}\n'
        cfile = os.path.join(WORK, 'macros_%s.c' % tag)
        exe = os.path.join(WORK, 'macros_%s' % tag)
        open(cfile, 'w').write(src)
        r = subprocess.run(['gcc', '-std=gnu17', '-w', '-o', exe, cfile, '-lm']
                           + extra, capture_output=True)
        if r.returncode == 0:
            return names, exe
        bad = set()
        for m in re.finditer(r'macros_%s\.c:(\d+):' % tag, r.stderr.decode()):
            i = int(m.group(1)) - first
            if 0 <= i < len(names):
                bad.add(names[i])
        if not bad:
            sys.stderr.write(r.stderr.decode()[:3000])
            sys.exit(2)
        names = [n for n in names if n not in bad]

def main():
    hs = headers()
    common = set()
    missing = {}
    extra_only = {}
    for h in hs:
        a = macros(h, ALTC)
        s = macros(h, [])
        if a is None:
            print('ERROR: <%s> does not preprocess with AltC headers' % h)
            continue
        if s is None:
            continue
        common |= a & s
        if s - a:
            missing[h] = sorted(s - a)
        if a - s:
            extra_only[h] = sorted(a - s)
    sysnames, sysexe = build(sorted(common), hs, [], 'sys')
    altnames, altexe = build(sysnames, hs, ALTC, 'altc')
    failed = sorted(set(sysnames) - set(altnames))
    out_s = subprocess.check_output([sysexe]).decode().split('\n')
    # rebuild the system printer with exactly the same name list
    sysnames2, sysexe = build(altnames, hs, [], 'sys')
    out_s = subprocess.check_output([sysexe]).decode().split('\n')
    out_a = subprocess.check_output([altexe]).decode().split('\n')
    diffs = [(x, y) for x, y in zip(out_s, out_a) if x != y]
    print('compared %d macros (%d common names, %d not expressions)'
          % (len(altnames), len(common), len(common) - len(sysnames)))
    for n in failed:
        print('FAIL (does not compile with AltC headers): %s' % n)
    for x, y in diffs:
        print('DIFF system: %s\n     altc:   %s' % (x, y))
    if verbose:
        for h in sorted(missing):
            print('missing in <%s>: %s' % (h, ' '.join(missing[h])))
        for h in sorted(extra_only):
            print('only in AltC <%s>: %s' % (h, ' '.join(extra_only[h])))
    else:
        print('%d system macros not provided by AltC headers (use -v to list)'
              % sum(len(v) for v in missing.values()))
    return 1 if (diffs or failed) else 0

sys.exit(main())
