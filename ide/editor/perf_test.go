package editor

import (
	"fmt"
	"strings"
	"testing"
	"time"
)

// TestLargeFile types into a file of 10 000 lines: a key with the frame
// painted after it must stay fast
func TestLargeFile(t *testing.T) {
	if testing.Short() {
		t.Skip("short")
	}
	var b strings.Builder
	for i := 0; i < 10000; i++ {
		fmt.Fprintf(&b, "    int value%d = compute(%d, \"text\"); // comment\n", i, i)
	}
	e, w := showEditor(t, b.String())
	e.GoTo(9000, 4)
	w.Settle()
	w.Screenshot()
	start := time.Now()
	const keys = 50
	for i := 0; i < keys; i++ {
		w.Type("x")
		w.Screenshot() // a frame
	}
	per := time.Since(start) / keys
	t.Logf("%v per key with a frame", per)
	if per > 100*time.Millisecond {
		t.Errorf("typing is slow: %v per key", per)
	}
	// The block comment opened at the top changes the highlighting at the bottom
	e.GoTo(0, 0)
	e.TextBox.InsertText("/*")
	if st := e.states.startState(e.TextBox, 9999); st != StateBlockComment {
		t.Errorf("state at the end %d", st)
	}
}
