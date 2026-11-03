package editor

import (
	"image/png"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/ipoluianov/nui/ui"
	"github.com/ipoluianov/nui/ui/uitest"
)

func showEditor(t *testing.T, text string) (*Editor, *uitest.Window) {
	t.Helper()
	e := New()
	e.Load(text)
	form := ui.NewForm()
	form.SetSize(600, 400)
	form.Panel().AddWidget(0, 0, e.TextBox)
	w := uitest.Show(t, form)
	e.Focus()
	w.Settle()
	return e, w
}

func TestTypingAndIndent(t *testing.T) {
	e, w := showEditor(t, "")
	w.Type("int main(void) {\nreturn 0;\n}")
	want := "int main(void) {\n    return 0;\n}"
	if got := e.Text(); got != want {
		t.Fatalf("text:\n%s\nwant:\n%s", got, want)
	}
	if !e.Modified() {
		t.Error("not modified after typing")
	}
}

func TestEnterBetweenBraces(t *testing.T) {
	e, w := showEditor(t, "void f() {}")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 10})
	w.Type("\nx;")
	want := "void f() {\n    x;\n}"
	if got := e.Text(); got != want {
		t.Fatalf("text:\n%q\nwant:\n%q", got, want)
	}
}

func TestEnterKeepsIndent(t *testing.T) {
	e, w := showEditor(t, "        a = 1;   ")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 17})
	w.Type("\nb")
	if got := e.Text(); got != "        a = 1;\n        b" {
		t.Fatalf("text %q", got)
	}
	// In the middle of a line: the rest starts at the indent
	e.Load("    a; b;")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 6})
	w.Type("\n")
	if got := e.Text(); got != "    a;\n    b;" {
		t.Fatalf("split: %q", got)
	}
}

func TestTab(t *testing.T) {
	e, w := showEditor(t, "ab")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 1})
	w.Key(ui.KeyTab, ui.KeyModifiers{})
	if got := e.Text(); got != "a   b" {
		t.Fatalf("tab: %q", got)
	}
	if w.Focused() == nil || w.Focused().Id() != e.Id() {
		t.Fatal("Tab moved the focus")
	}
	// Indent and unindent the lines of a selection
	e.Load("a\n  b\nc")
	e.SetSelection(ui.TextPos{Line: 0, Col: 0}, ui.TextPos{Line: 1, Col: 2})
	w.Key(ui.KeyTab, ui.KeyModifiers{})
	if got := e.Text(); got != "    a\n      b\nc" {
		t.Fatalf("indent: %q", got)
	}
	w.Key(ui.KeyTab, ui.KeyModifiers{Shift: true})
	w.Key(ui.KeyTab, ui.KeyModifiers{Shift: true})
	if got := e.Text(); got != "a\nb\nc" {
		t.Fatalf("unindent: %q", got)
	}
	// Backspace in the indent goes back to the previous stop
	e.Load("      x")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 6})
	w.Key(ui.KeyBackspace, ui.KeyModifiers{})
	if got := e.Text(); got != "    x" {
		t.Fatalf("backspace: %q", got)
	}
}

func TestClosingBrace(t *testing.T) {
	e, w := showEditor(t, "if (x) {\n    y;\n    ")
	e.SetCursorPosition(ui.TextPos{Line: 2, Col: 4})
	w.Type("}")
	if got := e.Text(); got != "if (x) {\n    y;\n}" {
		t.Fatalf("text %q", got)
	}
}

func TestSmartHome(t *testing.T) {
	e, w := showEditor(t, "    abc")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 6})
	w.Key(ui.KeyHome, ui.KeyModifiers{})
	if p := e.CursorPosition(); p.Col != 4 {
		t.Fatalf("first Home: %v", p)
	}
	w.Key(ui.KeyHome, ui.KeyModifiers{})
	if p := e.CursorPosition(); p.Col != 0 {
		t.Fatalf("second Home: %v", p)
	}
	w.Key(ui.KeyEnd, ui.KeyModifiers{})
	w.Key(ui.KeyHome, ui.KeyModifiers{Shift: true})
	if e.SelectedText() != "abc" {
		t.Fatalf("Shift+Home selected %q", e.SelectedText())
	}
}

func TestUndoAndModified(t *testing.T) {
	e, w := showEditor(t, "x")
	e.SetCursorPosition(ui.TextPos{Line: 0, Col: 1})
	var changes []bool
	e.OnModifiedChanged = func(m bool) { changes = append(changes, m) }
	w.Type("\n")
	if e.Text() != "x\n" || !e.Modified() {
		t.Fatalf("typed: %q %v", e.Text(), e.Modified())
	}
	w.Shortcut("Ctrl+Z")
	if e.Text() != "x" {
		t.Fatalf("undo: %q", e.Text())
	}
	// Back at the saved text: not modified once the check runs
	w.Wait(500 * 1e6)
	if e.Modified() {
		t.Error("modified after undoing to the saved text")
	}
	if len(changes) != 2 || !changes[0] || changes[1] {
		t.Errorf("changes %v", changes)
	}
	w.Shortcut("Ctrl+Y")
	if e.Text() != "x\n" {
		t.Fatalf("redo: %q", e.Text())
	}
	e.MarkSaved()
	if e.Modified() {
		t.Error("modified after saving")
	}
}

func TestFind(t *testing.T) {
	e, w := showEditor(t, "foo bar\nBar baz\nbar")
	_ = w
	if !e.FindNext("bar", false, false) {
		t.Fatal("not found")
	}
	if from, to := e.Selection(); from != (ui.TextPos{Line: 0, Col: 4}) || to.Col != 7 {
		t.Fatalf("first: %v %v", from, to)
	}
	e.FindNext("bar", false, false)
	if from, _ := e.Selection(); from != (ui.TextPos{Line: 1, Col: 0}) {
		t.Fatalf("second: %v", from)
	}
	e.FindNext("bar", true, false) // match case: the third
	if from, _ := e.Selection(); from != (ui.TextPos{Line: 2, Col: 0}) {
		t.Fatalf("match case: %v", from)
	}
	e.FindNext("bar", true, false) // around the end
	if from, _ := e.Selection(); from != (ui.TextPos{Line: 0, Col: 4}) {
		t.Fatalf("wrapped: %v", from)
	}
	e.FindNext("bar", false, true) // backward, around the start
	if from, _ := e.Selection(); from != (ui.TextPos{Line: 2, Col: 0}) {
		t.Fatalf("backward: %v", from)
	}
	if n := e.CountMatches(); n != 3 { // the last search ignores the case
		t.Fatalf("matches %d", n)
	}
	if e.FindNext("nothing", false, false) {
		t.Fatal("found what is not there")
	}
	e.SetFind("", false)
}

func TestGoTo(t *testing.T) {
	var lines []string
	for i := 0; i < 500; i++ {
		lines = append(lines, "line")
	}
	e, _ := showEditor(t, strings.Join(lines, "\n"))
	e.GoTo(400, 2)
	if p := e.CursorPosition(); p.Line != 400 || p.Col != 2 {
		t.Fatalf("cursor %v", p)
	}
	lh := e.OneLineHeight()
	if y := 400 * lh; y < e.ScrollY() || y > e.ScrollY()+e.ViewportHeight() {
		t.Fatalf("line 400 at %d not in the view from %d", y, e.ScrollY())
	}
	e.GoTo(10000, 100) // clamped
	if p := e.CursorPosition(); p.Line != 499 || p.Col != 4 {
		t.Fatalf("clamped %v", p)
	}
}

func TestHighlightSpans(t *testing.T) {
	e, _ := showEditor(t, "/* a\nb */ int x = 5;")
	spans := e.highlightLine(1, e.Line(1))
	kinds := map[string]bool{}
	rs := []rune(e.Line(1))
	for _, s := range spans {
		kinds[string(rs[s.Start:s.End])] = true
	}
	for _, want := range []string{"b */", "int", "5"} {
		if !kinds[want] {
			t.Errorf("no span %q in %v", want, spans)
		}
	}
	// A marker over a keyword keeps its color and gets a background
	e.SetMarkers([]Marker{{Line: 1, Col: 5, Message: "bad"}})
	spans = e.highlightLine(1, e.Line(1))
	found := false
	for _, s := range spans {
		if string(rs[s.Start:s.End]) == "int" && s.Background != nil && s.Color != nil {
			found = true
		}
	}
	if !found {
		t.Errorf("marker span: %v", spans)
	}
}

// TestScreenshot writes pictures of the editor in both themes when
// ALTC_SCREENSHOTS is a directory
func TestScreenshot(t *testing.T) {
	dir := os.Getenv("ALTC_SCREENSHOTS")
	if dir == "" {
		t.Skip("ALTC_SCREENSHOTS not set")
	}
	text := "#include <stdio.h>\n\n/* A comment\n   in two lines */\nint main(int argc, char **argv) {\n\tint n = 42; // tab\n    printf(\"%d %c\\n\", n, 'x');\n    return undefined_var;\n}\n"
	for _, dark := range []bool{false, true} {
		if dark {
			ui.ApplyDarkTheme()
		} else {
			ui.ApplyLightTheme()
		}
		e, w := showEditor(t, text)
		e.SetMarkers([]Marker{{Line: 7, Col: 11, Message: "undeclared"}, {Line: 5, Col: 5, Warning: true}})
		e.SetFind("n", true)
		e.GoTo(6, 4)
		name := "editor-light.png"
		if dark {
			name = "editor-dark.png"
		}
		f, err := os.Create(filepath.Join(dir, name))
		if err != nil {
			t.Fatal(err)
		}
		png.Encode(f, w.Screenshot())
		f.Close()
	}
	ui.ApplyLightTheme()
}
