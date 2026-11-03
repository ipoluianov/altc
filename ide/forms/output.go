package forms

import (
	"fmt"
	"strings"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/ide/editor"
	"github.com/ipoluianov/nui/ui"
)

// OutputPanel is the bottom of the window: the log of the build and of the
// program run, and the list of the diagnostics of the last build
type OutputPanel struct {
	ui.Widget

	m        *MainForm
	tabs     *ui.TabWidget
	log      *ui.LogView
	problems *ui.ListView
	model    *problemsModel
}

// problemsModel lists the diagnostics
type problemsModel struct {
	ui.ModelSignal
	m     *MainForm
	diags []driver.Diagnostic
}

func (p *problemsModel) RowCount() int    { return len(p.diags) }
func (p *problemsModel) ColumnCount() int { return 1 }

func (p *problemsModel) Text(row, col int) string {
	d := p.diags[row]
	place := ""
	switch {
	case d.File == "":
	case d.Line == 0:
		place = p.m.relPath(d.File)
	case d.Col == 0:
		place = fmt.Sprintf("%s:%d", p.m.relPath(d.File), d.Line)
	default:
		place = fmt.Sprintf("%s:%d:%d", p.m.relPath(d.File), d.Line, d.Col)
	}
	if place == "" {
		return d.Message
	}
	return place + "  " + d.Message
}

func (p *problemsModel) Style(row, col int) ui.ItemStyle {
	name := "error"
	switch p.diags[row].Severity {
	case driver.SevWarning:
		name = "warning"
	case driver.SevNote:
		name = "note"
	}
	return ui.ItemStyle{Icon: icon(name, menuIconSize)}
}

// NewOutputPanel makes the empty panel
func NewOutputPanel(m *MainForm) *OutputPanel {
	c := &OutputPanel{m: m}
	c.InitWidget()
	c.SetPanelPadding(0)
	c.SetCellPadding(0)
	c.SetMinHeight(80)

	c.log = ui.NewLogView()
	c.log.SetName("log")
	c.log.SetXExpandable(true)
	c.log.SetYExpandable(true)
	c.log.SetHighlighter(c.highlightLog)

	c.model = &problemsModel{m: m}
	c.problems = ui.NewListView()
	c.problems.SetName("problems")
	c.problems.SetModel(c.model)
	c.problems.SetXExpandable(true)
	c.problems.SetYExpandable(true)
	c.problems.OnActivated.Connect(c.openProblem)

	c.tabs = ui.NewTabWidget()
	c.tabs.AddPage(T().OutputTab, c.log)
	c.tabs.AddPage(T().ProblemsTab, c.problems)
	c.AddWidget(0, 0, c.tabs)
	c.applyLanguage()
	return c
}

func (c *OutputPanel) applyLanguage() {
	s := T()
	c.tabs.SetPageName(0, s.OutputTab)
	if n := len(c.model.diags); n > 0 {
		c.tabs.SetPageName(1, s.ProblemsN(n))
	} else {
		c.tabs.SetPageName(1, s.ProblemsTab)
	}
	c.problems.SetEmptyText(s.NoProblems)
}

func (c *OutputPanel) applyTheme() {
	c.problems.Refresh()
}

// highlightLog colors the diagnostics and the results in the log
func (c *OutputPanel) highlightLog(line int, text string) []ui.TextSpan {
	col := editor.CurrentColors()
	n := len([]rune(text))
	switch {
	case strings.Contains(text, "error:") || strings.HasPrefix(text, "error"):
		return []ui.TextSpan{{Start: 0, End: n, Color: col.ErrorMark}}
	case strings.Contains(text, "warning:"):
		return []ui.TextSpan{{Start: 0, End: n, Color: col.WarningMark}}
	case strings.HasPrefix(text, "== "):
		return []ui.TextSpan{{Start: 0, End: n, Color: ui.CurrentPalette().Highlight, Bold: true}}
	}
	return nil
}

// clear empties the log and the diagnostics
func (c *OutputPanel) clear() {
	c.log.Clear()
	c.setDiagnostics(nil)
}

// println adds a line to the log; on the UI thread
func (c *OutputPanel) println(s string) {
	c.log.AppendLine(s)
}

// showOutput brings the log to the front
func (c *OutputPanel) showOutput() {
	c.tabs.SetCurrentIndex(0)
}

// showProblems brings the list of the diagnostics to the front
func (c *OutputPanel) showProblems() {
	c.tabs.SetCurrentIndex(1)
}

func (c *OutputPanel) setDiagnostics(diags []driver.Diagnostic) {
	c.model.diags = diags
	c.model.Changed()
	c.applyLanguage()
}

func (c *OutputPanel) openProblem(row int) {
	if row < 0 || row >= len(c.model.diags) {
		return
	}
	d := c.model.diags[row]
	if d.File == "" {
		return
	}
	c.m.docs.openAt(d.File, d.Line, d.Col)
}
