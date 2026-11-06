package driver

import (
	"testing"
	"time"
)

// TestCompileLibc compiles the whole C library
func TestCompileLibc(t *testing.T) {
	if testing.Short() {
		t.Skip("short")
	}
	start := time.Now()
	files, err := compileLibc()
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("libc.a: %d bytes in %v", len(files["libc.a"]), time.Since(start))
}
