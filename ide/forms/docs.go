package forms

import (
	"errors"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/ide/config"
	"github.com/ipoluianov/altc/ide/editor"
	"github.com/ipoluianov/nui/ui"
)

// Document is a file open in a tab
type Document struct {
	path    string
	ed      *editor.Editor
	format  editor.FileFormat
	modTime time.Time
	size    int64
	// pendingScroll: the cursor was set before the editor had a size; it is
	// scrolled to once shown
	pendingScroll bool
}

func (d *Document) name() string { return filepath.Base(d.path) }

func (d *Document) stat() {
	if st, err := os.Stat(d.path); err == nil {
		d.modTime, d.size = st.ModTime(), st.Size()
	}
}

// DocArea is the center of the window: the tabs of the open files, the
// editor of the current one and the find bar
type DocArea struct {
	ui.Widget

	m     *MainForm
	tabs  *TabBar
	stack *ui.Panel
	empty *ui.Label
	find  *FindBar
	list  []*Document
	cur   int
	menu  *ui.ContextMenu
}

// NewDocArea makes the area without files
func NewDocArea(m *MainForm) *DocArea {
	c := &DocArea{m: m, cur: -1}
	c.InitWidget()
	c.SetPanelPadding(0)
	c.SetCellPadding(0)
	c.SetXExpandable(true)
	c.SetYExpandable(true)

	c.tabs = NewTabBar()
	c.tabs.OnSelect = c.selectIndex
	c.tabs.OnClose = func(i int) { c.closeIndex(i, nil) }
	c.menu = ui.NewContextMenu(c.tabs)
	c.menu.AddItem(T().CloseTab, func() { c.closeIndex(c.tabs.MenuTab(), nil) }).SetTextFunc(func() string { return T().CloseTab })
	c.menu.AddItem(T().CloseOthers, func() { c.closeOthers(c.tabs.MenuTab()) }).SetTextFunc(func() string { return T().CloseOthers })
	c.menu.AddItem(T().CloseAll, func() { c.closeOthers(-1) }).SetTextFunc(func() string { return T().CloseAll })
	c.menu.AddSeparator()
	c.menu.AddItem(T().CopyPath, func() {
		if i := c.tabs.MenuTab(); i >= 0 && i < len(c.list) {
			ui.ClipboardSetText(c.list[i].path)
		}
	}).SetTextFunc(func() string { return T().CopyPath })
	c.tabs.SetContextMenu(c.menu)
	c.AddWidget(0, 0, c.tabs)

	c.stack = ui.NewPanel()
	c.stack.SetPanelPadding(0)
	c.stack.SetCellPadding(0)
	c.stack.SetXExpandable(true)
	c.stack.SetYExpandable(true)
	c.empty = ui.NewLabel("")
	c.empty.SetTextAlign(ui.HAlignCenter)
	c.empty.SetXExpandable(true)
	c.empty.SetYExpandable(true)
	c.stack.AddWidget(0, 0, c.empty)
	c.AddWidget(1, 0, c.stack)

	c.find = NewFindBar(c)
	c.find.SetVisible(false)
	c.AddWidget(2, 0, c.find)
	c.applyLanguage()
	c.updateEmpty()
	return c
}

func (c *DocArea) applyLanguage() {
	s := T()
	key := func(k string) string {
		sc, err := ui.ParseShortcut(k)
		if err != nil {
			return k
		}
		return sc.String()
	}
	c.empty.SetText(s.NoOpenFiles + "\n\n" + s.EditorHints(key("Mod+O"), key("F7"), key("F5")))
	c.find.applyLanguage()
}

func (c *DocArea) updateEmpty() {
	c.empty.SetVisible(len(c.list) == 0)
	c.tabs.SetVisible(len(c.list) > 0)
}

// current returns the document of the current tab, nil if none
func (c *DocArea) current() *Document {
	if c == nil || c.cur < 0 || c.cur >= len(c.list) {
		return nil
	}
	return c.list[c.cur]
}

func (c *DocArea) indexOf(path string) int {
	for i, d := range c.list {
		if d.path == path {
			return i
		}
	}
	return -1
}

// open opens the file in a tab, or chooses its tab; nil if it can't be opened
func (c *DocArea) open(path string, focus bool) *Document {
	if i := c.indexOf(path); i >= 0 {
		c.selectIndex(i)
		if focus {
			c.list[i].ed.Focus()
		}
		return c.list[i]
	}
	text, format, err := editor.ReadFile(path)
	if err != nil {
		name := filepath.Base(path)
		msg := T().CannotOpen(path, err.Error())
		switch {
		case errors.Is(err, editor.ErrBinary):
			msg = T().NotText(name)
		case errors.Is(err, editor.ErrTooLarge):
			msg = T().TooLarge(name)
		}
		ui.ShowMessageBox(c, T().Error, msg)
		return nil
	}
	d := &Document{path: path, format: format}
	d.stat()
	d.ed = editor.New()
	d.ed.SetFontSize(float64(config.Get().EditorFontSize()))
	d.ed.SetHighlighting(editor.IsCFile(path))
	d.ed.Load(text)
	d.ed.OnModifiedChanged = func(bool) {
		c.retitle()
		c.m.updateTitle()
	}
	d.ed.OnKeyDown.Connect(func(e *ui.KeyEvent) {
		if e.Key == ui.KeyEsc && c.find.IsVisible() {
			c.hideFind()
			e.Handled = true
		}
	})
	d.ed.SetVisible(false)
	c.stack.AddWidget(0, 0, d.ed.TextBox)
	c.list = append(c.list, d)
	c.tabs.Add(d.name(), "")
	c.applyMarkers(d)
	c.retitle()
	c.updateEmpty()
	c.selectIndex(len(c.list) - 1)
	if focus {
		d.ed.Focus()
	}
	return d
}

// selectIndex shows the editor of the tab
func (c *DocArea) selectIndex(i int) {
	if i < 0 || i >= len(c.list) {
		return
	}
	if old := c.current(); old != nil && c.cur != i {
		old.ed.SetVisible(false)
	}
	c.cur = i
	d := c.list[i]
	d.ed.SetVisible(true)
	c.tabs.SetCurrent(i)
	if c.find.IsVisible() {
		d.ed.SetFind(c.find.text(), c.find.matchCase())
		c.find.updateCount()
	}
	if d.pendingScroll {
		d.pendingScroll = false
		// Once laid out: the view has its size then
		d.ed.AfterFunc(time.Millisecond, func() {
			pos := d.ed.CursorPosition()
			d.ed.GoTo(pos.Line, pos.Col)
		})
	}
	if d.ed.Form() != nil {
		d.ed.Focus()
	}
	c.m.updateTitle()
}

// cycle chooses the next tab (dir > 0) or the previous one
func (c *DocArea) cycle(dir int) {
	if n := len(c.list); n > 1 {
		c.selectIndex(((c.cur+dir)%n + n) % n)
	}
}

// retitle gives the tabs their texts: the name of the file, with its
// folder when another open file has the same name
func (c *DocArea) retitle() {
	count := map[string]int{}
	for _, d := range c.list {
		count[d.name()]++
	}
	for i, d := range c.list {
		title := d.name()
		if count[title] > 1 {
			title += " - " + filepath.Base(filepath.Dir(d.path))
		}
		c.tabs.SetTab(i, title, c.m.relPath(d.path), d.ed.Modified())
	}
}

// save writes the document to its file; false if it failed
func (c *DocArea) save(d *Document) bool {
	if d == nil {
		return false
	}
	format, err := editor.WriteFile(d.path, d.ed.Text(), d.format)
	if err != nil {
		ui.ShowMessageBox(c, T().Error, T().CannotSave(d.name(), err.Error()))
		return false
	}
	d.format = format
	d.ed.MarkSaved()
	d.stat()
	return true
}

// saveAll saves the modified documents; false if one failed
func (c *DocArea) saveAll() bool {
	for _, d := range c.list {
		if d.ed.Modified() && !c.save(d) {
			return false
		}
	}
	return true
}

func (c *DocArea) modifiedCount() int {
	if c == nil {
		return 0
	}
	n := 0
	for _, d := range c.list {
		if d.ed.Modified() {
			n++
		}
	}
	return n
}

// askSaveAll asks whether to save the modified documents, then calls then
// unless cancelled (or a save failed)
func (c *DocArea) askSaveAll(then func()) {
	n := c.modifiedCount()
	if n == 0 {
		then()
		return
	}
	text := T().SaveChangesN(n)
	if n == 1 {
		for _, d := range c.list {
			if d.ed.Modified() {
				text = T().SaveChangesTo(d.name())
			}
		}
	}
	ui.ShowQuestionMessageBoxYesNoCancel(c, T().UnsavedTitle, text,
		func() {
			if c.saveAll() {
				then()
			}
		},
		then, nil)
}

// closeIndex closes the tab, asking about its changes first; then is
// called once it is closed
func (c *DocArea) closeIndex(i int, then func()) {
	if i < 0 || i >= len(c.list) {
		return
	}
	d := c.list[i]
	done := func() {
		c.remove(d)
		if then != nil {
			then()
		}
	}
	if !d.ed.Modified() {
		done()
		return
	}
	c.selectIndex(i)
	ui.ShowQuestionMessageBoxYesNoCancel(c, T().UnsavedTitle, T().SaveChangesTo(d.name()),
		func() {
			if c.save(d) {
				done()
			}
		},
		done, nil)
}

// closeOthers closes all the tabs but keep (-1: all of them)
func (c *DocArea) closeOthers(keep int) {
	var keepDoc *Document
	if keep >= 0 && keep < len(c.list) {
		keepDoc = c.list[keep]
	}
	var next func()
	next = func() {
		for i, d := range c.list {
			if d != keepDoc {
				c.closeIndex(i, next)
				return
			}
		}
	}
	next()
}

// remove closes the tab of the document without asking
func (c *DocArea) remove(d *Document) {
	i := -1
	for j, x := range c.list {
		if x == d {
			i = j
		}
	}
	if i < 0 {
		return
	}
	wasCurrent := i == c.cur
	c.stack.RemoveWidget(d.ed.TextBox)
	c.list = append(c.list[:i], c.list[i+1:]...)
	c.tabs.Remove(i)
	switch {
	case len(c.list) == 0:
		c.cur = -1
	case wasCurrent:
		c.cur = -1
		c.selectIndex(min(i, len(c.list)-1))
	case c.cur > i:
		c.cur--
	}
	c.retitle()
	c.updateEmpty()
	if len(c.list) == 0 {
		c.hideFind()
	}
	c.m.updateTitle()
}

// closeAll closes all the tabs without asking
func (c *DocArea) closeAll() {
	for len(c.list) > 0 {
		c.remove(c.list[len(c.list)-1])
	}
}

func (c *DocArea) setFontSize(size float64) {
	for _, d := range c.list {
		d.ed.SetFontSize(size)
	}
}

// checkChangedOnDisk reloads the files changed by other programs; a
// modified one is reloaded if the user agrees
func (c *DocArea) checkChangedOnDisk() {
	for _, d := range c.list {
		st, err := os.Stat(d.path)
		if err != nil || (st.ModTime().Equal(d.modTime) && st.Size() == d.size) {
			continue
		}
		d.modTime, d.size = st.ModTime(), st.Size()
		if !d.ed.Modified() {
			c.reload(d)
			continue
		}
		d := d
		ui.ShowQuestionMessageBoxYesNo(c, T().ChangedOnDisk, T().ReloadAsk(d.name()), func() { c.reload(d) }, nil)
	}
}

func (c *DocArea) reload(d *Document) {
	text, format, err := editor.ReadFile(d.path)
	if err != nil {
		return
	}
	if text == d.ed.Text() {
		d.format = format
		d.ed.MarkSaved()
		d.stat()
		return
	}
	pos := d.ed.CursorPosition()
	scroll := d.ed.ScrollY()
	d.format = format
	d.ed.Load(text)
	d.ed.SetCursorPosition(pos)
	d.ed.SetScrollY(scroll)
	d.stat()
}

// renamed follows a file or a folder renamed in the tree
func (c *DocArea) renamed(oldPath, newPath string) {
	for _, d := range c.list {
		switch {
		case d.path == oldPath:
			d.path = newPath
		case strings.HasPrefix(d.path, oldPath+string(filepath.Separator)):
			d.path = newPath + d.path[len(oldPath):]
		default:
			continue
		}
		d.ed.SetHighlighting(editor.IsCFile(d.path))
	}
	c.retitle()
	c.m.updateTitle()
}

// deleted closes the files deleted in the tree, without asking
func (c *DocArea) deleted(path string) {
	for i := len(c.list) - 1; i >= 0; i-- {
		d := c.list[i]
		if d.path == path || strings.HasPrefix(d.path, path+string(filepath.Separator)) {
			c.remove(d)
		}
	}
}

// applyMarkers shows the diagnostics of the last build in the editor of the document
func (c *DocArea) applyMarkers(d *Document) {
	var ms []editor.Marker
	var lines []string // of the file, for the columns: they count bytes
	for _, diag := range c.m.diags {
		if diag.Line <= 0 || filepath.Clean(diag.File) != d.path {
			continue
		}
		if lines == nil {
			data, _ := os.ReadFile(d.path)
			lines = strings.Split(string(data), "\n")
		}
		col := 0
		if diag.Col > 0 && diag.Line <= len(lines) {
			col = editor.VisualCol(lines[diag.Line-1], diag.Col-1)
		}
		ms = append(ms, editor.Marker{Line: diag.Line - 1, Col: col, Warning: diag.Severity != driver.SevError, Message: diag.Message})
	}
	d.ed.SetMarkers(ms)
}

func (c *DocArea) applyAllMarkers() {
	for _, d := range c.list {
		c.applyMarkers(d)
	}
}

// Find and go to line

func (c *DocArea) showFind() {
	d := c.current()
	if d == nil {
		return
	}
	if sel := d.ed.SelectedText(); sel != "" && !strings.Contains(sel, "\n") {
		c.find.setText(sel)
	}
	c.find.SetVisible(true)
	d.ed.SetFind(c.find.text(), c.find.matchCase())
	c.find.updateCount()
	c.find.focus()
}

func (c *DocArea) hideFind() {
	c.find.SetVisible(false)
	for _, d := range c.list {
		d.ed.SetFind("", false)
	}
	if d := c.current(); d != nil {
		d.ed.Focus()
	}
}

// findNext selects the next match of the find bar (the previous one when back)
func (c *DocArea) findNext(back bool) {
	d := c.current()
	if d == nil {
		return
	}
	if c.find.text() == "" {
		c.showFind()
		return
	}
	if !d.ed.FindNext(c.find.text(), c.find.matchCase(), back) {
		c.m.status.ShowMessage(T().NoMatches, 3*time.Second)
	}
	c.find.updateCount()
}

// findChanged follows the text or the options of the find bar
func (c *DocArea) findChanged() {
	d := c.current()
	if d == nil {
		return
	}
	d.ed.SetFind(c.find.text(), c.find.matchCase())
	c.find.updateCount()
}

func (c *DocArea) showGoTo() {
	d := c.current()
	if d == nil {
		return
	}
	lines := d.ed.LineCount()
	pos := d.ed.CursorPosition()
	parse := func(text string) (line, col int, err error) {
		parts := strings.SplitN(strings.TrimSpace(text), ":", 2)
		line, err = strconv.Atoi(strings.TrimSpace(parts[0]))
		if err != nil || line < 1 {
			return 0, 0, errors.New(T().GoToBad)
		}
		col = 1
		if len(parts) == 2 {
			if col, err = strconv.Atoi(strings.TrimSpace(parts[1])); err != nil || col < 1 {
				return 0, 0, errors.New(T().GoToBad)
			}
		}
		return line, col, nil
	}
	ui.ShowInputDialog(c, ui.InputDialog{
		Title:  T().GoToTitle,
		Prompt: T().GoToPrompt(lines),
		Value:  strconv.Itoa(pos.Line + 1),
		Validate: func(text string) error {
			_, _, err := parse(text)
			return err
		},
	}, func(text string) {
		line, col, _ := parse(text)
		d.ed.GoTo(line-1, col-1)
		d.ed.Focus()
	})
}

// openAt opens the file and puts the cursor at the line and the column (from 1)
func (c *DocArea) openAt(path string, line, col int) {
	d := c.open(path, true)
	if d == nil || line <= 0 {
		return
	}
	vcol := 0
	if col > 0 {
		data, _ := os.ReadFile(path)
		lines := strings.Split(string(data), "\n")
		if line <= len(lines) {
			vcol = editor.VisualCol(lines[line-1], col-1)
		}
	}
	d.ed.GoTo(line-1, vcol)
	d.ed.Focus()
}
