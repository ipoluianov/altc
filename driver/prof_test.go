package driver

import (
	"os"
	"testing"
)

// TestProfileFile compiles $ALTC_PROF_FILE (for profiling with -cpuprofile)
func TestProfileFile(t *testing.T) {
	f := os.Getenv("ALTC_PROF_FILE")
	if f == "" {
		t.Skip("ALTC_PROF_FILE not set")
	}
	data, err := os.ReadFile(f)
	if err != nil {
		t.Fatal(err)
	}
	for i := 0; i < 5; i++ {
		u := CompileSource(f, data, &Config{Std: "gnu99", Defines: []string{"DLUA_USE_LINUX"}, NoWarnings: true})
		if u.Obj == nil {
			t.Fatal(u.Diagnostics)
		}
	}
}
