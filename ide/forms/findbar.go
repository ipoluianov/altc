package forms

import (
	"github.com/ipoluianov/nui/ui"
)

// FindBar is the line under the editor to search the current file: Enter
// finds the next match, Shift+Enter the previous one, Esc closes it
type FindBar struct {
	ui.Widget

	area     *DocArea
	lbl      *ui.Label
	txt      *ui.TextBox
	chkCase  *ui.Checkbox
	btnPrev  *ui.Button
	btnNext  *ui.Button
	lblCount *ui.Label
	btnClose *ui.ToolButton
}

// NewFindBar makes the find bar of the area
func NewFindBar(area *DocArea) *FindBar {
	c := &FindBar{area: area}
	c.InitWidget()
	c.SetPanelPadding(4)
	c.SetCellPadding(4)
	c.SetYExpandable(false)

	c.lbl = ui.NewLabel("")
	c.txt = ui.NewTextBox()
	c.txt.SetName("find")
	c.txt.SetMinWidth(260)
	c.txt.SetMaxWidth(400)
	c.txt.SetOnTextChanged(func() { area.findChanged() })
	c.txt.OnSubmit.Connect(func(string) { area.findNext(false) })
	c.txt.OnKeyDown.Connect(func(e *ui.KeyEvent) {
		switch {
		case e.Key == ui.KeyEsc:
			area.hideFind()
			e.Handled = true
		case e.Key == ui.KeyEnter && e.Mods.Shift:
			area.findNext(true)
			e.Handled = true
		}
	})
	c.chkCase = ui.NewCheckbox("")
	c.chkCase.OnCheckedChanged.Connect(func(bool) { area.findChanged() })
	c.btnPrev = ui.NewButton("")
	c.btnPrev.OnClick.Connect(func(ui.Widgeter) { area.findNext(true) })
	c.btnNext = ui.NewButton("")
	c.btnNext.OnClick.Connect(func(ui.Widgeter) { area.findNext(false) })
	c.lblCount = ui.NewLabel("")
	c.lblCount.SetMinWidth(110)
	c.btnClose = ui.NewToolButton(icon("close", menuIconSize), "", area.hideFind)
	c.btnClose.SetFlat(true)
	c.btnClose.SetButtonSize(24, 24)

	c.AddWidget(0, 0, c.lbl)
	c.AddWidget(0, 1, c.txt)
	c.AddWidget(0, 2, c.chkCase)
	c.AddWidget(0, 3, c.btnPrev)
	c.AddWidget(0, 4, c.btnNext)
	c.AddWidget(0, 5, c.lblCount)
	c.AddHSpacer(0, 6)
	c.AddWidget(0, 7, c.btnClose)
	ui.OnThemeChanged.ConnectWhileShown(c, func(bool) { c.btnClose.SetImage(icon("close", menuIconSize)) })
	return c
}

func (c *FindBar) applyLanguage() {
	s := T()
	c.lbl.SetText(s.FindLabel)
	c.chkCase.SetText(s.MatchCase)
	c.btnPrev.SetText(s.Previous)
	c.btnNext.SetText(s.Next)
	c.updateCount()
}

func (c *FindBar) text() string     { return c.txt.Text() }
func (c *FindBar) matchCase() bool  { return c.chkCase.Checked() }
func (c *FindBar) setText(s string) { c.txt.SetText(s) }

func (c *FindBar) focus() {
	c.txt.Focus()
	c.txt.SelectAllText()
}

// updateCount shows how many matches the current file has
func (c *FindBar) updateCount() {
	d := c.area.current()
	if d == nil || c.text() == "" || !c.IsVisible() {
		c.lblCount.SetText("")
		return
	}
	if n := d.ed.CountMatches(); n > 0 {
		c.lblCount.SetText(T().Matches(n))
	} else {
		c.lblCount.SetText(T().NoMatches)
	}
}
