package tests

import (
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"testing"
)

// goRef runs tests/goref, a separate module (golang.org/x/crypto from the
// module cache; nothing is downloaded), on the request lines and returns
// its answers. The test is skipped when it cannot be built.
var goRefOnce sync.Once
var goRefExe string
var goRefErr string

func goRef(t *testing.T, lines []string) []string {
	t.Helper()
	goRefOnce.Do(func() {
		exe := filepath.Join(scratch, "goref")
		cmd := exec.Command("go", "build", "-o", exe, ".")
		cmd.Dir = "goref"
		cmd.Env = append(os.Environ(), "GOPROXY=off", "GOFLAGS=-mod=mod")
		if out, err := cmd.CombinedOutput(); err != nil {
			goRefErr = string(out)
			return
		}
		goRefExe = exe
	})
	if goRefExe == "" {
		t.Skipf("tests/goref does not build (golang.org/x/crypto not in the module cache?):\n%s", goRefErr)
	}
	cmd := exec.Command(goRefExe)
	cmd.Stdin = strings.NewReader(strings.Join(lines, "\n") + "\n")
	var out, errb bytes.Buffer
	cmd.Stdout, cmd.Stderr = &out, &errb
	if err := cmd.Run(); err != nil {
		t.Fatalf("goref: %v\n%s", err, errb.String())
	}
	return strings.Split(strings.TrimSuffix(out.String(), "\n"), "\n")
}
