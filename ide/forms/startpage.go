package forms

import (
	"path/filepath"

	"github.com/ipoluianov/altc/app"
	"github.com/ipoluianov/altc/ide/config"
	"github.com/ipoluianov/nui/ui"
)

// StartPage is shown while no folder is open: the button to open one and
// the recent ones
type StartPage struct {
	ui.Widget

	m       *MainForm
	content *ui.VBox
}

// NewStartPage makes the start page
func NewStartPage(m *MainForm) *StartPage {
	c := &StartPage{m: m}
	c.InitWidget()
	c.SetXExpandable(true)
	c.SetYExpandable(true)
	c.update()
	return c
}

// update builds the page again: the recent folders, the language
func (c *StartPage) update() {
	c.RemoveAllWidgets()
	s := T()
	c.content = ui.NewVBox()
	c.content.SetMinWidth(460)

	if AppIcon != nil {
		img := ui.NewImageBox()
		img.SetImage(AppIcon)
		img.SetMinSize(72, 72)
		img.SetMaxSize(72, 72)
		c.content.Add(img)
		c.content.AddSpacing(8)
	}
	title := ui.NewLabel(app.DisplayName)
	title.SetFontSize(30)
	c.content.Add(title)
	sub := ui.NewLabel(s.Description)
	sub.SetForegroundColor(ui.CurrentPalette().PlaceholderText)
	c.content.Add(sub)
	c.content.AddSpacing(20)

	open := ui.NewButton(s.StartOpen)
	open.SetName("startOpen")
	open.SetRole("primary")
	open.OnClick.Connect(func(ui.Widgeter) { c.m.ShowOpenFolder() })
	row := ui.NewHBox()
	row.Add(open)
	row.AddStretch()
	c.content.Add(row)
	c.content.AddSpacing(8)
	hint := ui.NewLabel(s.StartHint)
	hint.SetForegroundColor(ui.CurrentPalette().PlaceholderText)
	c.content.Add(hint)
	c.content.AddSpacing(24)

	recentTitle := ui.NewLabel(s.StartRecent)
	recentTitle.SetFontBold(true)
	c.content.Add(recentTitle)
	c.content.AddSpacing(4)
	recent := config.Get().Recent
	if len(recent) == 0 {
		none := ui.NewLabel(s.StartNoRecent)
		none.SetForegroundColor(ui.CurrentPalette().PlaceholderText)
		c.content.Add(none)
	}
	for _, dir := range recent {
		dir := dir
		row := ui.NewHBox()
		link := ui.NewLink(filepath.Base(dir), func() { c.m.OpenProject(dir) })
		row.Add(link)
		row.AddSpacing(10)
		path := ui.NewLabel(dir)
		path.SetForegroundColor(ui.CurrentPalette().PlaceholderText)
		row.Add(path)
		row.AddStretch()
		c.content.Add(row)
	}

	// In the middle of the page
	c.AddHSpacer(0, 0)
	c.AddVSpacer(0, 1)
	c.AddWidget(1, 1, c.content)
	c.AddVSpacer(2, 1)
	c.AddHSpacer(1, 2)
}
