// The same tests as ../c/suite_test.c (TestAltcTestAgainstGo)
package suite

import (
	"os"
	"testing"
)

func TestLog(t *testing.T) {
	t.Logf("hello\nworld")
	t.Logf("trail\n")
	t.Logf("%d+%d", 2, 3)
}

func TestError(t *testing.T) {
	t.Errorf("e1")
	t.Logf("after")
}

func TestFatal(t *testing.T) {
	t.Fatalf("stop")
	t.Errorf("not reached")
}

func TestSkip(t *testing.T) {
	t.Skipf("skipping")
	t.Errorf("not reached")
}

func pass(msg string) func(t *testing.T) {
	return func(t *testing.T) {
		if msg != "" {
			t.Logf("%s", msg)
		}
	}
}

func TestNested(t *testing.T) {
	t.Run("a", func(t *testing.T) {
		t.Run("b", func(t *testing.T) { t.Errorf("deep") })
	})
	t.Run("a", pass("second a"))
	t.Run("a#01", pass("explicit"))
	t.Run("with space", pass("spaced"))
	t.Run("", pass("empty"))
	t.Run("", pass(""))
	t.Run("tab\tx", pass(""))
	t.Run("ctl\x01", pass(""))
	t.Run("uni\xc3\xa9", pass(""))
}

func TestSubFatal(t *testing.T) {
	ok := t.Run("f", func(t *testing.T) { t.Fatalf("sub fatal") })
	t.Logf("run returned %v", ok)
	ok = t.Run("g", pass(""))
	t.Logf("run returned %v", ok)
}

func TestCleanup(t *testing.T) {
	t.Cleanup(func() { t.Logf("c1") })
	t.Cleanup(func() { t.Logf("c2") })
	t.Errorf("fail")
}

func TestParallelOne(t *testing.T) {
	t.Run("p", func(t *testing.T) {
		t.Parallel()
		t.Logf("in parallel")
	})
	t.Logf("parent body done")
}

func TestTopPar(t *testing.T) {
	t.Parallel()
	t.Logf("top parallel")
}

func TestPass(t *testing.T) {}

func TestFailNow(t *testing.T) {
	t.Fail()
	t.Logf("failed: %v", t.Failed())
	t.FailNow()
}

func TestSubSkip(t *testing.T) {
	t.Run("s", func(t *testing.T) { t.SkipNow() })
	t.Logf("skipped: %v", true)
}

func TestShort(t *testing.T) {
	if testing.Short() {
		t.Skipf("short mode")
	}
	t.Logf("long mode")
}

func TestName(t *testing.T) { t.Logf("name %s", t.Name()) }

func TestEnv(t *testing.T) {
	t.Run("set", func(t *testing.T) {
		t.Setenv("ALTC_SUITE_VAR", "inner")
		t.Logf("env %s", os.Getenv("ALTC_SUITE_VAR"))
	})
	v, ok := os.LookupEnv("ALTC_SUITE_VAR")
	if !ok {
		v = "unset"
	}
	t.Logf("env after %s", v)
}
