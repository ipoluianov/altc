package forms

import (
	"image/png"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/ide/config"
	"github.com/ipoluianov/nui/ui"
	"github.com/ipoluianov/nui/ui/uitest"
)

func TestAllTranslated(t *testing.T) {
	for lang, fields := range catalog.Missing() {
		t.Errorf("%s: not translated: %v", lang, fields)
	}
	offered := map[string]bool{}
	for _, l := range languages {
		offered[l.tag] = true
	}
	for _, lang := range catalog.Languages() {
		if !offered[lang] {
			t.Errorf("%s: not in the languages list", lang)
		}
	}
}

func TestSplitArgs(t *testing.T) {
	tests := []struct {
		in   string
		want []string
	}{
		{"", nil},
		{"a b  c", []string{"a", "b", "c"}},
		{`"a b" 'c d' e\ f`, []string{"a b", "c d", "e f"}},
		{`x "" y`, []string{"x", "", "y"}},
		{`"say \"hi\""`, []string{`say "hi"`}},
	}
	for _, tt := range tests {
		got := splitArgs(tt.in)
		if strings.Join(got, "|") != strings.Join(tt.want, "|") || len(got) != len(tt.want) {
			t.Errorf("splitArgs(%q) = %q, want %q", tt.in, got, tt.want)
		}
		if back := splitArgs(joinArgs(got)); strings.Join(back, "|") != strings.Join(got, "|") || len(back) != len(got) {
			t.Errorf("joinArgs(%q) = %q does not split back", got, joinArgs(got))
		}
	}
}

// newProject makes a project folder with a few files
func newProject(t *testing.T) string {
	t.Helper()
	dir := filepath.Join(t.TempDir(), "hello")
	files := map[string]string{
		"main.c":     "#include <stdio.h>\n#include \"util.h\"\n\nint main(void) {\n\tprintf(\"%d\\n\", twice(21));\n\treturn 0;\n}\n",
		"util.h":     "int twice(int x);\n",
		"src/util.c": "/* Doubles the number */\nint twice(int x) {\n    return x * 2;\n}\n",
		".hidden":    "x",
	}
	for name, text := range files {
		path := filepath.Join(dir, name)
		os.MkdirAll(filepath.Dir(path), 0o755)
		if err := os.WriteFile(path, []byte(text), 0o644); err != nil {
			t.Fatal(err)
		}
	}
	return dir
}

// showMain shows the main window with the settings in a temporary folder
func showMain(t *testing.T) (*MainForm, *uitest.Window) {
	t.Helper()
	config.SetDirectory(t.TempDir())
	t.Cleanup(func() { config.SetDirectory("") })
	config.Load()
	SetLanguage("en")
	form := ui.NewForm()
	m := NewMainForm(form)
	form.Panel().SetPanelPadding(0)
	form.Panel().SetCellPadding(0)
	form.Panel().AddWidget(0, 0, m)
	form.SetSize(1100, 720)
	form.OnClose = m.CanClose
	w := uitest.Show(t, form)
	t.Cleanup(func() { m.Stop() })
	return m, w
}

// waitIdle waits until the build and the program end
func waitIdle(t *testing.T, m *MainForm, w *uitest.Window) {
	t.Helper()
	for i := 0; i < 500 && m.busy(); i++ {
		w.Wait(10 * time.Millisecond)
	}
	if m.busy() {
		t.Fatal("still busy")
	}
	w.Wait(20 * time.Millisecond)
}

func logText(m *MainForm) string {
	return m.output.log.Text()
}

func TestOpenEditSave(t *testing.T) {
	m, w := showMain(t)
	if !m.start.IsVisible() || m.work.IsVisible() {
		t.Fatal("the start page is not shown without a project")
	}
	dir := newProject(t)
	m.OpenProject(dir)
	w.Settle()
	if m.project == nil || !m.work.IsVisible() || m.start.IsVisible() {
		t.Fatal("project not opened")
	}
	if !strings.Contains(w.Title(), "hello") {
		t.Errorf("title %q", w.Title())
	}
	// The tree: the folders first, no hidden files
	var names []string
	for _, n := range m.tree.rootNode.Children() {
		names = append(names, n.Text(0))
	}
	if strings.Join(names, ",") != "src,main.c,util.h" {
		t.Errorf("tree %v", names)
	}

	main := filepath.Join(dir, "main.c")
	d := m.docs.open(main, true)
	if d == nil {
		t.Fatal("not opened")
	}
	// The tab of main.c shows the tab characters as spaces
	if !strings.Contains(d.ed.Text(), "\n    printf") {
		t.Fatalf("tabs not expanded: %q", d.ed.Text())
	}
	d.ed.GoTo(5, 13)
	w.Type("\n")
	w.Type("puts(\"x\");")
	if !d.ed.Modified() || !m.docs.tabs.IsModified(0) || !strings.Contains(w.Title(), "main.c*") {
		t.Fatalf("not shown modified: %q", w.Title())
	}
	w.Shortcut("Ctrl+S")
	data, _ := os.ReadFile(main)
	if !strings.Contains(string(data), "\treturn 0;\n\tputs(\"x\");\n") {
		t.Fatalf("saved:\n%s", data)
	}
	if d.ed.Modified() || m.docs.tabs.IsModified(0) {
		t.Error("modified after saving")
	}

	// A second file; Ctrl+W closes it, Ctrl+Tab cycles
	m.docs.open(filepath.Join(dir, "src", "util.c"), true)
	if len(m.docs.list) != 2 || m.docs.current().name() != "util.c" {
		t.Fatal("second tab")
	}
	w.Shortcut("Ctrl+Tab")
	if m.docs.current().name() != "main.c" {
		t.Errorf("Ctrl+Tab: %s", m.docs.current().name())
	}
	w.Shortcut("Ctrl+W")
	if len(m.docs.list) != 1 || m.docs.current().name() != "util.c" {
		t.Errorf("Ctrl+W left %d tabs", len(m.docs.list))
	}

	// Closing a modified tab asks
	w.Type("// more\n")
	w.Shortcut("Ctrl+W")
	q := w.Dialog()
	if q == nil {
		t.Fatal("no question about the changes")
	}
	q.Click(q.Find("No"))
	w.Settle()
	if len(m.docs.list) != 0 {
		t.Fatal("not closed after No")
	}
	if data, _ := os.ReadFile(filepath.Join(dir, "src", "util.c")); strings.Contains(string(data), "more") {
		t.Error("saved after No")
	}
}

func TestProjectStateRestored(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	m.docs.open(filepath.Join(dir, "util.h"), true)
	m.docs.open(filepath.Join(dir, "main.c"), true).ed.GoTo(3, 2)
	m.closeProject(nil)
	w.Settle()
	if m.project != nil || !m.start.IsVisible() {
		t.Fatal("not closed")
	}
	st := config.Get().Project(dir)
	if st == nil || len(st.OpenFiles) != 2 || filepath.Base(st.ActiveFile) != "main.c" {
		t.Fatalf("state %+v", st)
	}
	if r := config.Get().Recent; len(r) != 1 || r[0] != dir {
		t.Fatalf("recent %v", r)
	}
	m.RestoreProject(dir)
	w.Settle()
	if len(m.docs.list) != 2 || m.docs.current().name() != "main.c" {
		t.Fatalf("tabs not restored: %d", len(m.docs.list))
	}
	if p := m.docs.current().ed.CursorPosition(); p.Line != 3 || p.Col != 2 {
		t.Errorf("cursor %v", p)
	}
}

func TestBuildShowsDiagnostics(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	d := m.docs.open(filepath.Join(dir, "main.c"), true)
	w.Type("x")
	w.Key(ui.KeyF7, ui.KeyModifiers{})
	waitIdle(t, m, w)
	if d.ed.Modified() {
		t.Error("not saved before the build")
	}
	if !strings.Contains(logText(m), "Building hello") || !strings.Contains(logText(m), "== Build") {
		t.Errorf("log:\n%s", logText(m))
	}

	// Diagnostics as the compiler gives them: the columns count the tab as a byte
	main := filepath.Join(dir, "main.c")
	m.buildDone(t.Context(), &driver.BuildResult{Diagnostics: []driver.Diagnostic{
		{File: main, Line: 5, Col: 2, Severity: driver.SevError, Message: "implicit declaration of function 'printf'"},
		{File: main, Line: 6, Col: 9, Severity: driver.SevWarning, Message: "something"},
		{Severity: driver.SevError, Message: "linker: undefined twice"},
	}}, nil)
	if n := m.output.problems.Model().RowCount(); n != 3 {
		t.Fatalf("%d problems", n)
	}
	if ms := d.ed.MarkersAt(4); len(ms) != 1 || ms[0].Col != 4 {
		t.Errorf("markers of line 5: %+v", ms)
	}
	if !strings.Contains(logText(m), "main.c:5:2: error: implicit") {
		t.Errorf("log:\n%s", logText(m))
	}
	m.docs.closeAll()
	m.output.openProblem(1)
	cur := m.docs.current()
	if cur == nil || cur.name() != "main.c" {
		t.Fatal("the file of the problem not opened")
	}
	if p := cur.ed.CursorPosition(); p.Line != 5 || p.Col != 11 {
		t.Errorf("cursor at %v", p)
	}
	if ms := cur.ed.MarkersAt(5); len(ms) != 1 || !ms[0].Warning {
		t.Errorf("markers in the reopened file: %+v", ms)
	}
}

func TestRunAndStop(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	m.project.Args = []string{"one", "two words"}
	script := filepath.Join(dir, "prog.sh")
	os.WriteFile(script, []byte("#!/bin/sh\necho \"args: $1|$2\"\necho oops >&2\nprintf tail\nexit 3\n"), 0o755)
	m.startProgram(script)
	waitIdle(t, m, w)
	log := logText(m)
	for _, want := range []string{"args: one|two words", "oops", "tail\n", "exited with code 3"} {
		if !strings.Contains(log, want) {
			t.Errorf("no %q in the log:\n%s", want, log)
		}
	}

	os.WriteFile(script, []byte("#!/bin/sh\nsleep 30 &\nwait\n"), 0o755)
	m.startProgram(script)
	if !m.busy() {
		t.Fatal("not running")
	}
	start := time.Now()
	w.Key(ui.KeyF5, ui.KeyModifiers{Shift: true})
	waitIdle(t, m, w)
	if time.Since(start) > 5*time.Second || !strings.Contains(logText(m), "Process stopped") {
		t.Errorf("not stopped:\n%s", logText(m))
	}
}

func TestTreeNewRenameDelete(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	w.Settle()
	m.tree.reveal(filepath.Join(dir, "src"), true)
	m.tree.newEntry("", false)
	q := w.Dialog()
	if q == nil {
		t.Fatal("no dialog")
	}
	q.Type("new.c\n")
	w.Settle()
	path := filepath.Join(dir, "src", "new.c")
	if _, err := os.Stat(path); err != nil {
		t.Fatal("not created:", err)
	}
	if d := m.docs.current(); d == nil || d.path != path {
		t.Fatal("the new file not opened")
	}
	// An existing name is refused: the dialog stays
	m.tree.reveal(path, true)
	m.tree.rename()
	q = w.Dialog()
	q.Shortcut("Ctrl+A")
	q.Type("util.c\n")
	w.Settle()
	if !w.Blocked() {
		t.Fatal("renamed over an existing file")
	}
	w.Dialog().Key(ui.KeyEsc, ui.KeyModifiers{})
	w.Settle()
	// Rename the folder: the tab follows
	m.tree.reveal(filepath.Join(dir, "src"), true)
	m.tree.rename()
	q = w.Dialog()
	q.Shortcut("Ctrl+A")
	q.Type("lib\n")
	w.Settle()
	newPath := filepath.Join(dir, "lib", "new.c")
	if _, err := os.Stat(newPath); err != nil {
		t.Fatal("not renamed:", err)
	}
	if m.docs.current().path != newPath {
		t.Errorf("tab path %s", m.docs.current().path)
	}
	// Delete the folder: its tab closes
	m.tree.reveal(filepath.Join(dir, "lib"), true)
	m.tree.delete()
	q = w.Dialog()
	q.Click(q.Find("Yes"))
	w.Settle()
	if _, err := os.Stat(filepath.Join(dir, "lib")); !os.IsNotExist(err) {
		t.Fatal("not deleted")
	}
	if len(m.docs.list) != 0 {
		t.Error("the tab of the deleted file is open")
	}
}

func TestFindBar(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	d := m.docs.open(filepath.Join(dir, "main.c"), true)
	w.Shortcut("Ctrl+F")
	if !m.docs.find.IsVisible() {
		t.Fatal("find bar hidden")
	}
	w.Type("int")
	if got := m.docs.find.lblCount.Text(); got != "2 matches" {
		t.Errorf("count %q", got)
	}
	w.Type("\n")
	if from, _ := d.ed.Selection(); from.Line != 3 {
		t.Errorf("first match at %v", from)
	}
	w.Key(ui.KeyEsc, ui.KeyModifiers{})
	if m.docs.find.IsVisible() {
		t.Error("Esc did not close the find bar")
	}
	w.Key(ui.KeyF3, ui.KeyModifiers{})
	if from, _ := d.ed.Selection(); from.Line != 4 {
		t.Errorf("F3: %v", from)
	}
}

func TestExternalChange(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	path := filepath.Join(dir, "util.h")
	d := m.docs.open(path, true)
	os.WriteFile(path, []byte("int twice(int x);\nint thrice(int x);\n"), 0o644)
	os.Chtimes(path, time.Now().Add(time.Minute), time.Now().Add(time.Minute))
	m.onActivated()
	w.Settle()
	if !strings.Contains(d.ed.Text(), "thrice") {
		t.Error("not reloaded")
	}
}

func TestLanguage(t *testing.T) {
	m, w := showMain(t)
	SetLanguage("ru")
	defer SetLanguage("en")
	w.Settle()
	if m.act.build.Text() != ru.Build {
		t.Errorf("action text %q", m.act.build.Text())
	}
}

// TestScreenshots writes pictures of the window in both themes when
// ALTC_SCREENSHOTS is a directory
func TestScreenshots(t *testing.T) {
	out := os.Getenv("ALTC_SCREENSHOTS")
	if out == "" {
		t.Skip("ALTC_SCREENSHOTS not set")
	}
	save := func(w *uitest.Window, name string) {
		f, err := os.Create(filepath.Join(out, name))
		if err != nil {
			t.Fatal(err)
		}
		png.Encode(f, w.Screenshot())
		f.Close()
	}
	for _, dark := range []bool{false, true} {
		suffix := "light"
		if dark {
			suffix = "dark"
		}
		m, w := showMain(t)
		if dark {
			ui.ApplyDarkTheme()
		} else {
			ui.ApplyLightTheme()
		}
		dir := newProject(t)
		config.Update(func(s *config.Settings) { s.AddRecent(dir); s.AddRecent("/home/user/projects/lua") })
		m.start.update()
		w.Settle()
		save(w, "start-"+suffix+".png")

		m.OpenProject(dir)
		m.docs.open(filepath.Join(dir, "util.h"), false)
		m.docs.open(filepath.Join(dir, "main.c"), true)
		m.tree.reveal(filepath.Join(dir, "src", "util.c"), false)
		w.Key(ui.KeyF7, ui.KeyModifiers{})
		waitIdle(t, m, w)
		main := filepath.Join(dir, "main.c")
		m.buildDone(t.Context(), &driver.BuildResult{Diagnostics: []driver.Diagnostic{
			{File: main, Line: 5, Col: 9, Severity: driver.SevError, Message: "implicit declaration of function 'twice'"},
			{File: main, Line: 4, Col: 5, Severity: driver.SevWarning, Message: "unused parameter"},
		}}, nil)
		m.output.showOutput()
		m.docs.current().ed.GoTo(5, 10)
		w.Settle()
		save(w, "main-"+suffix+".png")
		w.Shortcut("Ctrl+F")
		w.Type("int")
		m.output.showProblems()
		save(w, "find-"+suffix+".png")
	}
	ui.ApplyLightTheme()
}

func TestProjectSettings(t *testing.T) {
	m, w := showMain(t)
	dir := newProject(t)
	m.OpenProject(dir)
	m.ShowProjectSettings()
	q := w.Dialog()
	if q == nil {
		t.Fatal("no dialog")
	}
	d := q.Find("altc-project-name")
	q.Click(d)
	q.Type("greeter")
	m2 := q.Find("altc-project-args")
	q.Click(m2)
	q.Type(`--name "Big World"`)
	q.Click(q.Find("OK"))
	w.Settle()
	if w.Blocked() {
		t.Fatal("dialog still open")
	}
	p, err := driver.LoadProject(dir)
	if err != nil {
		t.Fatal(err)
	}
	if p.Name != "greeter" || len(p.Args) != 2 || p.Args[1] != "Big World" {
		t.Fatalf("saved %+v", p)
	}
	if m.project.ExeName() != "greeter" || !strings.Contains(w.Title(), "greeter") {
		t.Errorf("project not updated: %q", w.Title())
	}
	if out := os.Getenv("ALTC_SCREENSHOTS"); out != "" {
		m.ShowProjectSettings()
		save := func(img *uitest.Window, name string) {
			f, _ := os.Create(filepath.Join(out, name))
			png.Encode(f, img.Screenshot())
			f.Close()
		}
		save(w.Dialog(), "settings.png")
		w.Dialog().Key(ui.KeyEsc, ui.KeyModifiers{})
		w.Settle()
		m.ShowAbout()
		save(w.Dialog(), "about.png")
	}
}
