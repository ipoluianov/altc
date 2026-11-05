package tests

import (
	"os"
	"strings"
	"testing"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/elf"
	"github.com/ipoluianov/altc/x64"
)

// testConfig applies ALTC_TEST_FLAGS (for example "-march=haswell", "-O0 -g") so
// that the whole suite can run with other machine options
func testConfig(c *driver.Config) *driver.Config {
	for _, f := range strings.Fields(os.Getenv("ALTC_TEST_FLAGS")) {
		switch f {
		case "-O0":
			c.NoRegAlloc = true
			continue
		case "-g":
			c.Debug = true
			continue
		case "-static":
			continue // linkInput
		}
		if _, err := c.MachineFlag(f); err != nil {
			panic(err)
		}
	}
	return c
}

// flagsFor reads an optional "// flags: -mavx2 -mfma" line at the top of a
// test; the test is skipped when the processor lacks those extensions
func flagsFor(t *testing.T, src []byte) []string {
	for _, l := range strings.SplitN(string(src), "\n", 5) {
		if strings.HasPrefix(l, "// flags: ") {
			flags := strings.Fields(l[10:])
			var isa x64.ISA
			for _, f := range flags {
				isa.MachineFlag(f)
			}
			if isa&^x64.NativeISA() != 0 {
				t.Skip("the processor lacks extensions needed by " + strings.Join(flags, " "))
			}
			return flags
		}
	}
	return nil
}

func withFlags(c *driver.Config, flags []string) *driver.Config {
	for _, f := range flags {
		if _, err := c.MachineFlag(f); err != nil {
			panic(err)
		}
	}
	return c
}

// linkInput makes a link input honoring -static in ALTC_TEST_FLAGS
func linkInput(objs ...*elf.Object) *driver.LinkInput {
	in := &driver.LinkInput{Objects: objs}
	for _, f := range strings.Fields(os.Getenv("ALTC_TEST_FLAGS")) {
		if f == "-static" {
			in.Static = true
		}
	}
	return in
}
