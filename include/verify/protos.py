#!/usr/bin/env python3
"""Check function prototypes and object declarations of the AltC headers
against glibc.

For every public header H shipped in include/c that also exists in the
system include directories:
  1. glibc's <H> is preprocessed (with -D_GNU_SOURCE and __GNUC__
     undefined, so glibc's sys/cdefs.h strips attributes and asm labels)
     and all "extern" declarations are extracted;
  2. a translation unit is built that includes the AltC <H> followed by
     glibc's declarations of every name the AltC header also declares;
  3. GCC compiles it against the AltC headers: any "conflicting types"
     error is a prototype mismatch.
Additionally every function/object declared by the AltC headers is checked
against the dynamic symbol tables of libc.so.6 / libm.so.6 (default symbol
versions only) so that nothing is declared that cannot be linked.

Usage: protos.py [-v] [workdir]
"""
import os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
INC = os.path.join(os.path.dirname(HERE), 'c')
verbose = '-v' in sys.argv
args = [a for a in sys.argv[1:] if a != '-v']
WORK = args[0] if args else tempfile.mkdtemp(prefix='altc-verify-')
os.makedirs(WORK, exist_ok=True)
ALTC = ['-nostdinc', '-isystem', INC]

# Functions that glibc provides only in libc_nonshared.a (linked through the
# libc.so linker script), not in libc.so.6.
NONSHARED = {'atexit', 'at_quick_exit', 'pthread_atfork',
             # declared, then defined as a macro expanding to a builtin
             'alloca'}

def dynsyms(lib):
    out = subprocess.check_output(['nm', '-D', '--defined-only', lib]).decode()
    s = set()
    for line in out.split('\n'):
        p = line.split()
        if len(p) == 3 and '@@' in p[2]:
            s.add(p[2].split('@')[0])
    return s

def find_lib(name):
    for d in ('/lib/x86_64-linux-gnu', '/usr/lib/x86_64-linux-gnu', '/lib64',
              '/usr/lib64'):
        p = os.path.join(d, name)
        if os.path.exists(p):
            return p
    raise SystemExit('cannot find ' + name)

def headers():
    hs = []
    for root, dirs, files in os.walk(INC):
        dirs[:] = [d for d in dirs if d != '_altc']
        for f in files:
            if f.endswith('.h'):
                hs.append(os.path.relpath(os.path.join(root, f), INC))
    return sorted(hs)

# (compiler mode, prelude) combinations that are checked.
CONFIGS = [('-std=gnu17', '#define _GNU_SOURCE 1\n'),
           ('-std=gnu17', ''),
           ('-std=c99', ''),
           ('-std=c2x', '#define _GNU_SOURCE 1\n')]

def decls(hdr, extra, cfg):
    """Return {name: declaration} for extern declarations seen when
    preprocessing <hdr> (all included files)."""
    src = cfg[1] + '#include <%s>\n' % hdr
    r = subprocess.run(['gcc', '-E', '-P', cfg[0], '-U__GNUC__',
                        '-U__GNUC_MINOR__', '-'] + extra,
                       input=src.encode(), capture_output=True)
    if r.returncode != 0:
        return None
    text = r.stdout.decode()
    res = {}
    buf = ''
    depth = 0
    for ch in text:
        buf += ch
        if ch in '({':
            depth += 1
        elif ch in ')}':
            depth -= 1
        elif ch == ';' and depth == 0:
            d = re.sub(r'\s+', ' ', buf).strip()
            buf = ''
            # drop a leading struct/typedef body that ended right before
            d = re.sub(r'^.*\}\s*', '', d) if d.count('}') and not d.startswith('extern') else d
            if not d.startswith('extern') or '"C' in d:
                continue
            d = re.sub(r'__attribute__\s*\(\(.*?\)\)', '', d)
            m = re.search(r'(\w+)\s*\(', d)
            if m and m.group(1) not in ('__attribute__',):
                name = m.group(1)
            else:
                m = re.match(r'extern [^;(]*?\b(\w+)\s*(\[[^\]]*\])?\s*;$', d)
                if not m:
                    continue
                name = m.group(1)
            res[name] = d
    return res

def main():
    syms = dynsyms(find_lib('libc.so.6')) | dynsyms(find_lib('libm.so.6'))
    problems = 0
    unexported = {}
    for cfg in CONFIGS:
        print('== mode: %s %s' % (cfg[0], cfg[1].strip()))
        problems += check(cfg, syms, unexported)
    for n in sorted(unexported):
        print('NOT EXPORTED by libc.so.6/libm.so.6: %s (declared in <%s>)'
              % (n, unexported[n]))
        problems += 1
    return 1 if problems else 0

def check(cfg, syms, unexported):
    problems = 0
    for h in headers():
        ours = decls(h, ALTC, cfg)
        if ours is None:
            print('ERROR: <%s> does not preprocess with AltC headers' % h)
            problems += 1
            continue
        for n in ours:
            if n not in syms and n not in NONSHARED and not n.startswith('__altc'):
                unexported.setdefault(n, h)
        theirs = decls(h, [], cfg)
        if theirs is None:
            continue
        common = sorted(set(ours) & set(theirs))
        lines = [theirs[n] for n in common]
        while True:
            src = cfg[1] + '#include <%s>\n' % h
            first = src.count('\n') + 1
            src += '\n'.join(lines) + '\nint __altc_dummy;\n'
            cfile = os.path.join(WORK, 'protos.c')
            open(cfile, 'w').write(src)
            r = subprocess.run(['gcc', cfg[0], '-fsyntax-only', '-w',
                                cfile] + ALTC, capture_output=True)
            if r.returncode == 0:
                break
            err = r.stderr.decode()
            bad = set()
            for m in re.finditer(r'protos\.c:(\d+):\d+: error: (.*)', err):
                i = int(m.group(1)) - first
                if 0 <= i < len(lines):
                    bad.add(i)
                    msg = m.group(2)
                    if 'conflicting types' in msg or 'redeclar' in msg or \
                       'redefinition' in msg:
                        print('MISMATCH <%s>: %s\n   glibc: %s'
                              % (h, msg, lines[i]))
                        problems += 1
                    elif verbose:
                        print('unchecked <%s>: %s\n   glibc: %s'
                              % (h, msg, lines[i]))
            if not bad:
                sys.stderr.write(err[:3000])
                return 2
            lines = [l for i, l in enumerate(lines) if i not in bad]
        missing = sorted(n for n in set(theirs) - set(ours)
                         if n in syms and not n.startswith('_'))
        if verbose or missing:
          print('<%s>: %d declarations checked%s' % (
            h, len(lines),
            (', %d glibc functions not declared' % len(missing))
            if missing else ''))
        if verbose and missing:
            print('   not declared: ' + ' '.join(missing))
    return problems

sys.exit(main())
