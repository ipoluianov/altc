#!/usr/bin/env python3
"""Mixing test: combine the AltC headers with every glibc header that AltC
does not ship.

The compiler falls back to /usr/include for headers it does not provide, so
glibc headers end up mixed with AltC's.  For every glibc header H (taken
from `dpkg -L libc6-dev`, or a scan of /usr/include) that is not shipped by
AltC, four translation units are compiled with

  gcc -nostdinc -isystem include/c -idirafter /usr/include/<multiarch>
      -idirafter /usr/include

(with and without _GNU_SOURCE; H before and after a set of common AltC
headers).  A combination counts as a failure only if the same translation
unit compiles with the plain system headers.

Usage: mixall.py [-v] [workdir]
"""
import concurrent.futures, os, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
INC = os.path.join(os.path.dirname(HERE), 'c')
verbose = '-v' in sys.argv
args = [a for a in sys.argv[1:] if a != '-v']
WORK = args[0] if args else tempfile.mkdtemp(prefix='altc-verify-')
os.makedirs(WORK, exist_ok=True)
MULTI = '/usr/include/x86_64-linux-gnu'
MIX = ['-nostdinc', '-isystem', INC, '-idirafter', MULTI,
       '-idirafter', '/usr/include']
COMMON = ['stdio.h', 'stdlib.h', 'string.h', 'unistd.h', 'sys/types.h',
          'signal.h', 'time.h', 'sys/time.h', 'sys/socket.h',
          'netinet/in.h', 'pthread.h', 'fcntl.h', 'sys/stat.h', 'errno.h',
          'stdint.h', 'wchar.h', 'limits.h', 'sys/uio.h', 'sched.h']

def glibc_headers():
    names = set()
    try:
        out = subprocess.check_output(['dpkg', '-L', 'libc6-dev'],
                                      stderr=subprocess.DEVNULL).decode()
        for p in out.split('\n'):
            for base in (MULTI + '/', '/usr/include/'):
                if p.startswith(base) and p.endswith('.h'):
                    names.add(p[len(base):])
                    break
    except (OSError, subprocess.CalledProcessError):
        for base in ('/usr/include', MULTI):
            for root, dirs, files in os.walk(base):
                for f in files:
                    if f.endswith('.h'):
                        names.add(os.path.relpath(os.path.join(root, f), base))
    skip_dirs = ('bits/', 'gnu/', 'linux/', 'asm', 'x86_64-linux-gnu/',
                 'c++/', 'rpcsvc/', 'protocols/')
    return sorted(n for n in names
                  if not n.startswith(skip_dirs)
                  and not os.path.exists(os.path.join(INC, n)))

def compile_ok(src, flags):
    with tempfile.NamedTemporaryFile('w', suffix='.c', dir=WORK,
                                     delete=False) as f:
        f.write(src)
        name = f.name
    try:
        r = subprocess.run(['gcc', '-std=gnu17', '-fsyntax-only', name]
                           + flags, capture_output=True)
        return r.returncode == 0, r.stderr.decode()
    finally:
        os.unlink(name)

def test(h):
    results = []
    for gnu in ('#define _GNU_SOURCE 1\n', ''):
        common = ''.join('#include <%s>\n' % c for c in COMMON)
        for order in ('first', 'last'):
            src = gnu + ('#include <%s>\n' % h + common if order == 'first'
                         else common + '#include <%s>\n' % h)
            src += 'int __altc_mix_dummy;\n'
            base_ok, _ = compile_ok(src, [])
            if not base_ok:
                continue
            ok, err = compile_ok(src, MIX)
            if not ok:
                results.append((gnu.strip() or 'default', order, err))
    return h, results

def main():
    hs = glibc_headers()
    failures = 0
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as ex:
        for h, res in ex.map(test, hs):
            for mode, order, err in res:
                failures += 1
                print('MIX FAIL <%s> (%s, %s common headers)' % (
                    h, mode, 'before' if order == 'first' else 'after'))
                if verbose:
                    print('\n'.join(err.split('\n')[:8]))
    print('mixing: %d glibc headers tested, %d failing combinations'
          % (len(hs), failures))
    return 1 if failures else 0

sys.exit(main())
