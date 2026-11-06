package libc

import (
	"slices"
	"strings"
	"testing"
)

func TestSources(t *testing.T) {
	src := Sources()
	if len(src) < 1000 {
		t.Fatalf("only %d sources", len(src))
	}
	for _, f := range []string{
		Root + "/src/string/x86_64/memcpy.s",   // replaces string/memcpy.c
		Root + "/src/math/x86_64/sqrt.c",       // replaces math/sqrt.c
		Root + "/src/malloc/mallocng/malloc.c", // the malloc in use
		Root + "/src/stdio/printf.c",
		Root + "/src/internal/syscall_ret.c",
	} {
		if !slices.Contains(src, f) {
			t.Errorf("%s missing", f)
		}
	}
	for _, f := range src {
		if f == Root+"/src/string/memcpy.c" || f == Root+"/src/math/sqrt.c" || strings.Contains(f, "oldmalloc") {
			t.Errorf("%s should not be built", f)
		}
		if _, err := FS.Open(f); err != nil {
			t.Error(err)
		}
	}
	if Version() != "1.2.5" {
		t.Errorf("version %q", Version())
	}
	t.Logf("%d sources", len(src))
}
