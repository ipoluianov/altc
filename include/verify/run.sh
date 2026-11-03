#!/bin/sh
# Verification of the AltC C headers (include/c) against glibc, using GCC.
#
#   include/verify/run.sh [workdir]
#
# Steps:
#   1. every public header compiles alone and all together in C89 ... C23,
#      with and without _GNU_SOURCE, with -Wall -Wextra -pedantic and no
#      warnings (headers given with -I so warnings inside them are shown);
#   2. layout.c (sizes, alignments, member offsets, initializers, macro
#      expressions) prints identical output with AltC and system headers;
#   3. macros.py: values and types of all object-like macros match glibc;
#   4. protos.py: prototypes match glibc's and every declared function is
#      exported by libc.so.6/libm.so.6;
#   5. smoke programs build (-Wall -Wextra -pedantic) and run in all
#      language modes and produce the same output as with system headers;
#   6. mixing: AltC headers combined with glibc headers that AltC does not
#      ship (gcc -nostdinc -isystem include/c -idirafter /usr/include ...).
set -u
HERE=$(cd "$(dirname "$0")" && pwd)
INC=$(cd "$HERE/../c" && pwd)
WORK=${1:-$(mktemp -d "${TMPDIR:-/tmp}/altc-verify.XXXXXX")}
echo "work directory: $WORK"
mkdir -p "$WORK"
ALTC="-nostdinc -isystem $INC"
MIX="-nostdinc -isystem $INC -idirafter /usr/include/x86_64-linux-gnu -idirafter /usr/include"
FAIL=0
fail () { echo "FAIL: $*"; FAIL=1; }

HDRS=$(cd "$INC" && find . -name '*.h' ! -path './_altc/*' | sed 's|^\./||' | sort)

echo "== 1. header self-containment and warnings"
for std in c89 c99 c11 c17 c2x gnu89 gnu17; do
  for gnu in "" "-D_GNU_SOURCE"; do
    { for h in $HDRS; do echo "#include <$h>"; done
      echo 'int main(void){return 0;}'; } > "$WORK/all.c"
    out=$(gcc -std=$std $gnu -Wall -Wextra -pedantic -Wno-long-long \
          -nostdinc -I "$INC" -c "$WORK/all.c" -o /dev/null 2>&1)
    [ -n "$out" ] && { fail "all headers -std=$std $gnu"; echo "$out" | head -20; }
  done
done
for h in $HDRS; do
  printf '#include <%s>\n#include <%s>\nint x;\n' "$h" "$h" > "$WORK/one.c"
  for std in c89 c11 c2x; do
    out=$(gcc -std=$std -Wall -Wextra -pedantic -Wno-long-long -nostdinc \
          -I "$INC" -c "$WORK/one.c" -o /dev/null 2>&1)
    [ -n "$out" ] && { fail "<$h> alone -std=$std"; echo "$out" | head -10; }
  done
done

echo "== 2. layout"
gcc -std=gnu11 $ALTC "$HERE/layout.c" -o "$WORK/layout_altc" -lm || fail "layout build (altc)"
gcc -std=gnu11 "$HERE/layout.c" -o "$WORK/layout_sys" -lm || fail "layout build (system)"
"$WORK/layout_altc" > "$WORK/layout_altc.txt"
"$WORK/layout_sys" > "$WORK/layout_sys.txt"
if diff "$WORK/layout_sys.txt" "$WORK/layout_altc.txt" > "$WORK/layout.diff"; then
  echo "layout identical ($(wc -l < "$WORK/layout_sys.txt") lines)"
else
  fail "layout differs"; head -40 "$WORK/layout.diff"
fi

echo "== 3. macros"
python3 "$HERE/macros.py" "$WORK" || fail "macro values"

echo "== 4. prototypes"
python3 "$HERE/protos.py" "$WORK" || fail "prototypes"

echo "== 5. smoke programs"
run_smoke () { # file std extra-flags
  src=$1; std=$2; shift 2
  b=$(basename "$src" .c)
  libs="-lm"
  if ! gcc -std=$std "$@" -Wall -Wextra -pedantic -Wno-long-long $ALTC "$src" \
       -o "$WORK/$b.altc" $libs 2> "$WORK/$b.$std.warn"; then
    fail "$b -std=$std $*: build"; head -20 "$WORK/$b.$std.warn"; return
  fi
  [ -s "$WORK/$b.$std.warn" ] && { echo "warnings: $b -std=$std $*"; head -10 "$WORK/$b.$std.warn"; }
  # glibc hides POSIX/BSD interfaces in strict ISO modes; AltC never does.
  gcc -std=$std -D_DEFAULT_SOURCE "$@" -w "$src" -o "$WORK/$b.sys" $libs \
    || { fail "$b system build"; return; }
  "$WORK/$b.altc" > "$WORK/$b.altc.out" 2>&1 || fail "$b -std=$std: exit status"
  "$WORK/$b.sys" > "$WORK/$b.sys.out" 2>&1
  if ! cmp -s "$WORK/$b.altc.out" "$WORK/$b.sys.out"; then
    fail "$b -std=$std $*: output differs"
    diff "$WORK/$b.sys.out" "$WORK/$b.altc.out" | head -20
  fi
}
for std in c89 c99 c11 c17 c2x; do run_smoke "$HERE/smoke/c89.c" $std; done
for f in stdio_stdlib strings mathfp time_signal posix threads; do
  for std in c99 c11 c17 c2x gnu17; do
    run_smoke "$HERE/smoke/$f.c" $std
  done
  run_smoke "$HERE/smoke/$f.c" gnu17 -D_GNU_SOURCE
done
echo "smoke programs done"

echo "== 6. mixing with glibc headers"
if gcc -std=gnu17 -Wall $MIX "$HERE/smoke/mixing.c" -o "$WORK/mixing" -lrt; then
  "$WORK/mixing" > /dev/null || fail "mixing run"
else
  fail "mixing build"
fi
python3 "$HERE/mixall.py" "$WORK" || fail "mixing with every glibc header"

if [ $FAIL = 0 ]; then echo "ALL CHECKS PASSED"; else echo "SOME CHECKS FAILED"; fi
exit $FAIL
