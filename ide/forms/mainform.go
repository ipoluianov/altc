// Package forms is the window of the IDE: the project tree, the editor
// tabs, the output panel, the menu, the build and the run
package forms

import (
	"context"
	"image"
	"os/exec"
	"time"

	"github.com/ipoluianov/altc/app"
	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/ide/config"
	"github.com/ipoluianov/nui/ui"
)

const (
	defaultWindowWidth  = 1200
	defaultWindowHeight = 800
	defaultTreeWidth    = 240
	defaultOutputHeight = 220
)

// MainForm is the content of the main window
type MainForm struct {
	ui.Widget

	form *ui.Form

	toolbar *ui.ToolBar
	start   *StartPage
	work    *ui.Panel // the tree, the editors and the output: shown with a project
	hsplit  *ui.Splitter
	vsplit  *ui.Splitter
	tree    *FileTree
	docs    *DocArea
	output  *OutputPanel
	status  *ui.StatusBar
	lblPos  *ui.Label

	recentMenu *ui.ContextMenu
	themeItems []*ui.ContextMenuItem
	langItems  []*ui.ContextMenuItem

	// actions with their texts, set again when the language changes
	actions []textAction
	act     struct {
		openFolder, newFile, newFolder, save, saveAll, closeTab, closeFolder, exit *ui.Action
		undo, redo, cut, copy, paste, selectAll, find, findNext, findPrev, goTo    *ui.Action
		zoomIn, zoomOut, zoomReset                                                 *ui.Action
		build, run, stop, settings                                                 *ui.Action
	}

	project *driver.Project // nil when no folder is open

	// The build and the program running
	buildCancel context.CancelFunc
	runCmd      *exec.Cmd
	runStop     func()
	diags       []driver.Diagnostic

	lastPos ui.TextPos // the cursor shown in the status bar

	treeWidth    int
	outputHeight int
}

type textAction struct {
	a    *ui.Action
	text func(s *Strings) string
}

// NewMainForm makes the content of the window and sets the form up: the
// menu, the shortcuts
func NewMainForm(form *ui.Form) *MainForm {
	c := &MainForm{form: form, treeWidth: defaultTreeWidth, outputHeight: defaultOutputHeight}
	c.InitWidget()
	c.SetPanelPadding(0)
	c.SetCellPadding(0)

	c.createActions()

	c.toolbar = c.buildToolbar()
	c.AddWidget(0, 0, c.toolbar)

	c.tree = NewFileTree(c)
	c.docs = NewDocArea(c)
	c.output = NewOutputPanel(c)

	c.vsplit = ui.NewVSplitter()
	c.vsplit.SetWidgets(c.docs, c.output)
	c.vsplit.SetSecondSize(c.outputHeight)
	c.vsplit.OnActivated.Connect(func(size int) { c.outputHeight = size })

	c.hsplit = ui.NewHSplitter()
	c.hsplit.SetWidgets(c.tree, c.vsplit)
	c.hsplit.SetFirstSize(c.treeWidth)
	c.hsplit.OnActivated.Connect(func(size int) { c.treeWidth = size })

	c.work = ui.NewPanel()
	c.work.SetPanelPadding(0)
	c.work.SetCellPadding(0)
	c.work.AddWidget(0, 0, c.hsplit)
	c.work.SetVisible(false)

	c.start = NewStartPage(c)
	c.AddWidget(1, 0, c.start)
	c.AddWidget(1, 0, c.work)

	c.status = ui.NewStatusBar()
	c.status.SetPanelPadding(4)
	c.lblPos = c.status.AddLabel("")
	c.status.SetText(T().Ready)
	c.AddWidget(2, 0, c.status)

	form.SetMenuBar(c.buildMenu())
	form.AddShortcut("Ctrl+Tab", func() { c.docs.cycle(1) })
	form.AddShortcut("Ctrl+Shift+Tab", func() { c.docs.cycle(-1) })
	form.AddShortcut("Ctrl+PageDown", func() { c.docs.cycle(1) })
	form.AddShortcut("Ctrl+PageUp", func() { c.docs.cycle(-1) })
	form.SetOnActivated(c.onActivated)
	form.OnLanguageChanged.Connect(func(string) { c.applyLanguage() })
	ui.OnThemeChanged.ConnectWhileShown(c, func(bool) { c.applyTheme() })

	// The cursor position in the status bar: the text box tells no moves
	c.AddTimer(150, c.updateCursorStatus)

	c.updateTitle()
	return c
}

func (c *MainForm) action(text func(s *Strings) string, f func()) *ui.Action {
	a := ui.NewAction(text(T()), f)
	c.actions = append(c.actions, textAction{a, text})
	return a
}

func (c *MainForm) createActions() {
	hasProject := func() bool { return c.project != nil }
	hasDoc := func() bool { return c.docs.current() != nil }
	a := &c.act
	a.openFolder = c.action(func(s *Strings) string { return s.OpenFolder }, c.ShowOpenFolder).SetShortcut("Mod+O")
	a.newFile = c.action(func(s *Strings) string { return s.NewFile }, func() { c.tree.newEntry("", false) }).
		SetShortcut("Mod+N").SetEnabledFunc(hasProject)
	a.newFolder = c.action(func(s *Strings) string { return s.NewFolder }, func() { c.tree.newEntry("", true) }).
		SetEnabledFunc(hasProject)
	a.save = c.action(func(s *Strings) string { return s.Save }, func() { c.docs.save(c.docs.current()) }).
		SetShortcut("Mod+S").SetEnabledFunc(hasDoc)
	a.saveAll = c.action(func(s *Strings) string { return s.SaveAll }, func() { c.docs.saveAll() }).
		SetShortcut("Mod+Shift+S").SetEnabledFunc(func() bool { return c.docs.modifiedCount() > 0 })
	a.closeTab = c.action(func(s *Strings) string { return s.CloseTab }, func() { c.docs.closeIndex(c.docs.tabs.Current(), nil) }).
		SetShortcut("Mod+W").SetEnabledFunc(hasDoc)
	a.closeFolder = c.action(func(s *Strings) string { return s.CloseFolder }, func() { c.closeProject(nil) }).
		SetEnabledFunc(hasProject)
	a.exit = c.action(func(s *Strings) string { return s.Exit }, func() { c.form.RequestClose() })

	edit := func(f func(e *ui.TextBox)) func() {
		return func() {
			if d := c.docs.current(); d != nil {
				f(d.ed.TextBox)
				d.ed.Focus()
			}
		}
	}
	a.undo = c.action(func(s *Strings) string { return s.Undo }, edit((*ui.TextBox).Undo)).SetShortcut("Mod+Z").SetEnabledFunc(hasDoc)
	a.redo = c.action(func(s *Strings) string { return s.Redo }, edit((*ui.TextBox).Redo)).SetShortcut("Mod+Y").SetEnabledFunc(hasDoc)
	a.cut = c.action(func(s *Strings) string { return s.Cut }, edit((*ui.TextBox).Cut)).SetShortcut("Mod+X").SetEnabledFunc(hasDoc)
	a.copy = c.action(func(s *Strings) string { return s.Copy }, edit((*ui.TextBox).Copy)).SetShortcut("Mod+C").SetEnabledFunc(hasDoc)
	a.paste = c.action(func(s *Strings) string { return s.Paste }, edit((*ui.TextBox).Paste)).SetShortcut("Mod+V").SetEnabledFunc(hasDoc)
	a.selectAll = c.action(func(s *Strings) string { return s.SelectAll }, edit((*ui.TextBox).SelectAllText)).SetShortcut("Mod+A").SetEnabledFunc(hasDoc)
	a.find = c.action(func(s *Strings) string { return s.Find }, func() { c.docs.showFind() }).SetShortcut("Mod+F").SetEnabledFunc(hasDoc)
	a.findNext = c.action(func(s *Strings) string { return s.FindNext }, func() { c.docs.findNext(false) }).SetShortcut("F3").SetEnabledFunc(hasDoc)
	a.findPrev = c.action(func(s *Strings) string { return s.FindPrevious }, func() { c.docs.findNext(true) }).SetShortcut("Shift+F3").SetEnabledFunc(hasDoc)
	a.goTo = c.action(func(s *Strings) string { return s.GoToLine }, func() { c.docs.showGoTo() }).SetShortcut("Mod+G").SetEnabledFunc(hasDoc)

	a.zoomIn = c.action(func(s *Strings) string { return s.ZoomIn }, func() { c.zoom(1) }).SetShortcut("Mod+=")
	a.zoomOut = c.action(func(s *Strings) string { return s.ZoomOut }, func() { c.zoom(-1) }).SetShortcut("Mod+-")
	a.zoomReset = c.action(func(s *Strings) string { return s.ZoomReset }, func() { c.zoom(0) }).SetShortcut("Mod+0")

	a.build = c.action(func(s *Strings) string { return s.Build }, func() { c.Build(nil) }).
		SetShortcut("F7").SetEnabledFunc(func() bool { return c.project != nil && !c.busy() })
	a.run = c.action(func(s *Strings) string { return s.Run }, c.Run).
		SetShortcut("F5").SetEnabledFunc(func() bool { return c.project != nil && !c.busy() })
	a.stop = c.action(func(s *Strings) string { return s.Stop }, c.Stop).
		SetShortcut("Shift+F5").SetEnabledFunc(c.busy)
	a.settings = c.action(func(s *Strings) string { return s.Settings }, c.ShowProjectSettings).SetEnabledFunc(hasProject)

	c.setActionIcons()
}

// setActionIcons gives the actions the icons of the current theme
func (c *MainForm) setActionIcons() {
	a := &c.act
	set := func(a *ui.Action, name string) { a.SetIcon(icon(name, toolIconSize)) }
	set(a.openFolder, "open")
	set(a.newFile, "newfile")
	set(a.save, "save")
	set(a.saveAll, "saveall")
	set(a.find, "find")
	set(a.build, "build")
	set(a.run, "run")
	set(a.stop, "stop")
	set(a.settings, "settings")
}

func (c *MainForm) buildToolbar() *ui.ToolBar {
	tb := ui.NewToolBar()
	a := &c.act
	for _, x := range []*ui.Action{a.openFolder, a.newFile, a.save, a.saveAll, nil, a.build, a.run, a.stop, nil, a.find, a.settings} {
		if x == nil {
			tb.AddSeparator()
			continue
		}
		tb.AddAction(x)
	}
	return tb
}

func (c *MainForm) buildMenu() *ui.MenuBar {
	bar := ui.NewMenuBar()
	a := &c.act
	s := T()

	file := ui.NewContextMenu(nil)
	file.AddAction(a.openFolder)
	c.recentMenu = ui.NewContextMenu(nil)
	file.AddItemWithSubmenu(s.OpenRecent, c.recentMenu).SetTextFunc(func() string { return T().OpenRecent })
	c.updateRecentMenu()
	file.AddSeparator()
	file.AddAction(a.newFile)
	file.AddAction(a.newFolder)
	file.AddAction(a.save)
	file.AddAction(a.saveAll)
	file.AddSeparator()
	file.AddAction(a.closeTab)
	file.AddAction(a.closeFolder)
	file.AddSeparator()
	file.AddAction(a.exit)
	bar.AddMenuItem(s.MenuFile, file).SetTextFunc(func() string { return T().MenuFile })

	edit := ui.NewContextMenu(nil)
	for _, x := range []*ui.Action{a.undo, a.redo, nil, a.cut, a.copy, a.paste, a.selectAll, nil, a.find, a.findNext, a.findPrev, a.goTo} {
		if x == nil {
			edit.AddSeparator()
		} else {
			edit.AddAction(x)
		}
	}
	bar.AddMenuItem(s.MenuEdit, edit).SetTextFunc(func() string { return T().MenuEdit })

	view := ui.NewContextMenu(nil)
	theme := ui.NewContextMenu(nil)
	c.themeItems = []*ui.ContextMenuItem{
		theme.AddItem(s.ThemeDark, func() { c.setTheme(config.ThemeDark) }).SetCheckable(true),
		theme.AddItem(s.ThemeLight, func() { c.setTheme(config.ThemeLight) }).SetCheckable(true),
	}
	c.themeItems[0].SetTextFunc(func() string { return T().ThemeDark })
	c.themeItems[1].SetTextFunc(func() string { return T().ThemeLight })
	theme.SetOnShow(func() {
		light := config.Get().Theme == config.ThemeLight
		c.themeItems[0].SetChecked(!light)
		c.themeItems[1].SetChecked(light)
	})
	view.AddItemWithSubmenu(s.Theme, theme).SetTextFunc(func() string { return T().Theme })
	lang := ui.NewContextMenu(nil)
	c.langItems = nil
	sys := lang.AddItem(s.SystemDefault, func() { c.setLanguage("") }).SetCheckable(true)
	sys.SetTextFunc(func() string { return T().SystemDefault })
	c.langItems = append(c.langItems, sys)
	lang.AddSeparator()
	for _, l := range languages {
		tag := l.tag
		c.langItems = append(c.langItems, lang.AddItem(l.name, func() { c.setLanguage(tag) }).SetCheckable(true))
	}
	lang.SetOnShow(func() {
		cur := config.Get().Language
		c.langItems[0].SetChecked(cur == "")
		for i, l := range languages {
			c.langItems[i+1].SetChecked(cur == l.tag)
		}
	})
	view.AddItemWithSubmenu(s.Language, lang).SetTextFunc(func() string { return T().Language })
	view.AddSeparator()
	view.AddAction(a.zoomIn)
	view.AddAction(a.zoomOut)
	view.AddAction(a.zoomReset)
	bar.AddMenuItem(s.MenuView, view).SetTextFunc(func() string { return T().MenuView })

	build := ui.NewContextMenu(nil)
	build.AddAction(a.build)
	build.AddAction(a.run)
	build.AddAction(a.stop)
	build.AddSeparator()
	build.AddAction(a.settings)
	bar.AddMenuItem(s.MenuBuild, build).SetTextFunc(func() string { return T().MenuBuild })

	help := ui.NewContextMenu(nil)
	help.AddItem(s.Website, func() { app.OpenSiteURL(app.Website, "menu") }).SetTextFunc(func() string { return T().Website })
	help.AddItem(s.About, c.ShowAbout).SetTextFunc(func() string { return T().About })
	bar.AddMenuItem(s.MenuHelp, help).SetTextFunc(func() string { return T().MenuHelp })
	return bar
}

// updateRecentMenu fills the Open Recent menu
func (c *MainForm) updateRecentMenu() {
	m := c.recentMenu
	if m == nil {
		return
	}
	m.RemoveAllItems()
	recent := config.Get().Recent
	if len(recent) == 0 {
		m.AddItem(T().NoRecent, nil).SetEnabled(false)
		return
	}
	for _, dir := range recent {
		dir := dir
		m.AddItem(dir, func() { c.OpenProject(dir) })
	}
	m.AddSeparator()
	m.AddItem(T().ClearRecent, func() {
		config.Update(func(s *config.Settings) { s.Recent = nil })
		c.updateRecentMenu()
		c.start.update()
	})
}

func (c *MainForm) applyLanguage() {
	s := T()
	for _, ta := range c.actions {
		ta.a.SetText(ta.text(s))
	}
	c.updateRecentMenu()
	c.start.update()
	c.docs.applyLanguage()
	c.output.applyLanguage()
	c.tree.applyLanguage()
	if c.busy() {
		c.status.SetText(s.StatusBuilding)
	} else {
		c.status.SetText(s.Ready)
	}
	c.lastPos = ui.TextPos{Line: -1}
	c.updateCursorStatus()
	c.updateTitle()
}

func (c *MainForm) applyTheme() {
	c.setActionIcons()
	c.tree.applyTheme()
	c.output.applyTheme()
	c.start.update()
}

func (c *MainForm) setTheme(theme string) {
	config.Update(func(s *config.Settings) { s.Theme = theme })
	ApplyTheme(theme)
}

func (c *MainForm) setLanguage(lang string) {
	config.Update(func(s *config.Settings) { s.Language = lang })
	SetLanguage(lang)
}

// zoom changes the font size of the editors by a step; 0 resets it
func (c *MainForm) zoom(step int) {
	var size int
	config.Update(func(s *config.Settings) {
		if step == 0 {
			s.FontSize = 0
		} else {
			s.FontSize = max(config.MinFontSize, min(s.EditorFontSize()+step, config.MaxFontSize))
		}
		size = s.EditorFontSize()
	})
	c.docs.setFontSize(float64(size))
}

// ApplyTheme switches the application to the theme of the settings
func ApplyTheme(theme string) {
	if theme == config.ThemeLight {
		ui.ApplyLightTheme()
	} else {
		ui.ApplyDarkTheme()
	}
}

// updateTitle shows the file and the project in the title of the window
func (c *MainForm) updateTitle() {
	title := app.DisplayName
	if c.project != nil {
		title = c.project.ExeName() + " - " + title
		if d := c.docs.current(); d != nil {
			mark := ""
			if d.ed.Modified() {
				mark = "*"
			}
			title = d.name() + mark + " - " + title
		}
	}
	c.form.SetTitle(title)
}

func (c *MainForm) updateCursorStatus() {
	d := c.docs.current()
	if d == nil {
		if c.lastPos.Line != -2 {
			c.lastPos = ui.TextPos{Line: -2}
			c.lblPos.SetText("")
		}
		return
	}
	pos := d.ed.CursorPosition()
	if pos == c.lastPos {
		return
	}
	c.lastPos = pos
	c.lblPos.SetText(T().LineCol(pos.Line+1, pos.Col+1))
	// The diagnostic of the line, if any
	if ms := d.ed.MarkersAt(pos.Line); len(ms) > 0 && !c.busy() {
		c.status.ShowMessage(ms[0].Message, 10*time.Second)
	}
}

// RestoreWindowState applies the saved window layout before the form is
// shown; returns whether the window should be maximized once shown
func (c *MainForm) RestoreWindowState() (maximized bool) {
	c.form.SetSize(defaultWindowWidth, defaultWindowHeight)
	state, ok := config.LoadWindowState()
	if !ok {
		return false
	}
	c.form.SetSize(state.Width, state.Height)
	if state.X != 0 || state.Y != 0 {
		c.form.Move(state.X, state.Y)
	}
	if state.TreeWidth > 0 {
		c.treeWidth = state.TreeWidth
		c.hsplit.SetFirstSize(c.treeWidth)
	}
	if state.OutputHeight > 0 {
		c.outputHeight = state.OutputHeight
		c.vsplit.SetSecondSize(c.outputHeight)
	}
	return state.Maximized
}

// SaveWindowState remembers the window layout for the next start
func (c *MainForm) SaveWindowState() {
	state, _ := config.LoadWindowState()
	state.Maximized = c.form.IsMaximized()
	if !state.Maximized {
		state.X, state.Y = c.form.Position()
		state.Width, state.Height = c.form.Size()
	}
	state.TreeWidth = c.treeWidth
	state.OutputHeight = c.outputHeight
	config.SaveWindowState(state)
}

// CanClose is the OnClose of the window: asks about the unsaved files, then
// stops the program and remembers the state
func (c *MainForm) CanClose() bool {
	if c.docs.modifiedCount() == 0 {
		c.shutdown()
		return true
	}
	c.docs.askSaveAll(func() {
		c.shutdown()
		c.form.Close()
	})
	return false
}

func (c *MainForm) shutdown() {
	c.Stop()
	c.saveProjectState()
	c.SaveWindowState()
}

func (c *MainForm) onActivated() {
	if c.project == nil {
		return
	}
	c.docs.checkChangedOnDisk()
	c.tree.Refresh()
}

// ShowAbout opens the About dialog
func (c *MainForm) ShowAbout() {
	c.ShowDialog(NewAboutDialog())
}

// AppIcon is the icon of the application, set by Run
var AppIcon image.Image
