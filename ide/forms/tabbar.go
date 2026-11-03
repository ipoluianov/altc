package forms

import (
	"image/color"

	"github.com/ipoluianov/nui/ui"
)

// TabBar is the row of the tabs of the open files: a click chooses one,
// the cross (or the middle button) closes it, a modified file shows a dot
// in place of the cross
type TabBar struct {
	ui.Widget

	tabs    []tabItem
	current int
	hover   int // the tab under the mouse, -1 if none
	onClose bool
	offset  int // the scroll of the row, in pixels
	menuTab int // the tab of the last right click

	OnSelect func(i int)
	OnClose  func(i int)
}

type tabItem struct {
	title    string
	tooltip  string
	modified bool
	x, w     int // where it was painted, without the offset
}

const (
	tabPadding = 10
	tabCloseW  = 18
)

// NewTabBar makes an empty row of tabs
func NewTabBar() *TabBar {
	c := &TabBar{current: -1, hover: -1, menuTab: -1}
	c.InitWidget()
	c.SetXExpandable(true)
	c.SetYExpandable(false)
	h := ui.ThemeControlHeight() + 4
	c.SetMinHeight(h)
	c.SetMaxHeight(h)
	c.SetOnPaint(c.paint)
	c.SetOnMouseDown(c.mouseDown)
	c.SetOnMouseMove(c.mouseMove)
	c.SetOnMouseLeave(func() {
		c.hover = -1
		c.Update()
	})
	c.SetOnMouseWheel(func(dx, dy int) bool {
		c.offset = max(0, c.offset-dy-dx)
		c.Update()
		return true
	})
	return c
}

// Count returns the number of the tabs
func (c *TabBar) Count() int { return len(c.tabs) }

// Current returns the index of the chosen tab, -1 if none
func (c *TabBar) Current() int { return c.current }

// MenuTab returns the tab the context menu was opened on, -1 if none
func (c *TabBar) MenuTab() int { return c.menuTab }

// Add adds a tab at the end
func (c *TabBar) Add(title, tooltip string) {
	c.tabs = append(c.tabs, tabItem{title: title, tooltip: tooltip})
	c.Update()
}

// Remove removes the tab
func (c *TabBar) Remove(i int) {
	if i < 0 || i >= len(c.tabs) {
		return
	}
	c.tabs = append(c.tabs[:i], c.tabs[i+1:]...)
	if c.current >= len(c.tabs) {
		c.current = len(c.tabs) - 1
	} else if c.current > i {
		c.current--
	}
	c.hover = -1
	c.Update()
}

// SetCurrent chooses the tab, without calling OnSelect
func (c *TabBar) SetCurrent(i int) {
	if i < -1 || i >= len(c.tabs) {
		return
	}
	c.current = i
	c.Update()
}

// SetTab changes the texts and the mark of the tab
func (c *TabBar) SetTab(i int, title, tooltip string, modified bool) {
	if i < 0 || i >= len(c.tabs) {
		return
	}
	t := &c.tabs[i]
	t.title, t.tooltip, t.modified = title, tooltip, modified
	c.Update()
}

// Title returns the text of the tab
func (c *TabBar) Title(i int) string {
	if i < 0 || i >= len(c.tabs) {
		return ""
	}
	return c.tabs[i].title
}

// IsModified tells whether the tab shows the modified mark
func (c *TabBar) IsModified(i int) bool {
	return i >= 0 && i < len(c.tabs) && c.tabs[i].modified
}

func (c *TabBar) layoutTabs() int {
	x := 0
	for i := range c.tabs {
		t := &c.tabs[i]
		w, _, _ := ui.MeasureText(ui.ThemeFontFamily(), ui.ThemeFontSize(), t.title)
		t.x, t.w = x, w+tabPadding*2+tabCloseW
		x += t.w
	}
	return x
}

func (c *TabBar) paint(cnv *ui.Canvas) {
	p := ui.CurrentPalette()
	W, H := c.Width(), c.Height()
	cnv.FillRect(0, 0, W, H, p.Window)
	total := c.layoutTabs()
	// The current tab is kept in the view
	if c.current >= 0 {
		t := c.tabs[c.current]
		if t.x < c.offset {
			c.offset = t.x
		} else if t.x+t.w > c.offset+W {
			c.offset = t.x + t.w - W
		}
	}
	c.offset = max(0, min(c.offset, total-W))
	cnv.SetFontFamily(ui.ThemeFontFamily())
	cnv.SetFontSize(ui.ThemeFontSize())
	cnv.SetVAlign(ui.VAlignCenter)
	for i, t := range c.tabs {
		x := t.x - c.offset
		if x+t.w < 0 || x > W {
			continue
		}
		textColor := p.PlaceholderText
		switch {
		case i == c.current:
			cnv.FillRect(x, 0, t.w, H, p.Base)
			cnv.FillRect(x, 0, t.w, 2, p.Highlight)
			textColor = p.Text
		case i == c.hover:
			cnv.FillRect(x, 0, t.w, H, blend(p.Window, p.Text, 0.06))
		}
		cnv.FillRect(x+t.w-1, 4, 1, H-8, p.Divider)
		cnv.SetColor(textColor)
		cnv.SetHAlign(ui.HAlignLeft)
		cnv.DrawText(x+tabPadding, 0, t.w-tabPadding-tabCloseW, H, t.title)
		// The cross, or the dot of a modified file
		cx := x + t.w - tabCloseW - tabPadding/2
		cy := H / 2
		hoverClose := i == c.hover && c.onClose
		if hoverClose {
			cnv.FillRect(cx, cy-tabCloseW/2+1, tabCloseW, tabCloseW-2, blend(p.Window, p.Text, 0.15))
		}
		switch {
		case t.modified && !hoverClose && i != c.hover:
			cnv.FillCircle(float64(cx)+tabCloseW/2, float64(cy), 4, textColor)
		case i == c.current || i == c.hover:
			ink := textColor
			m := 5
			cnv.DrawLine(cx+m, cy-tabCloseW/2+m, cx+tabCloseW-m, cy+tabCloseW/2-m, 1, ink)
			cnv.DrawLine(cx+tabCloseW-m, cy-tabCloseW/2+m, cx+m, cy+tabCloseW/2-m, 1, ink)
		case t.modified:
			cnv.FillCircle(float64(cx)+tabCloseW/2, float64(cy), 4, textColor)
		}
	}
	cnv.FillRect(0, H-1, W, 1, p.Divider)
}

// tabAt returns the tab at x and whether x is on its cross
func (c *TabBar) tabAt(x int) (int, bool) {
	x += c.offset
	for i, t := range c.tabs {
		if x >= t.x && x < t.x+t.w {
			cx := t.x + t.w - tabCloseW - tabPadding/2
			return i, x >= cx && x < cx+tabCloseW
		}
	}
	return -1, false
}

func (c *TabBar) mouseDown(button ui.MouseButton, x, y int, mods ui.KeyModifiers) bool {
	i, onClose := c.tabAt(x)
	if i < 0 {
		return false
	}
	switch button {
	case ui.MouseButtonLeft:
		if onClose {
			if c.OnClose != nil {
				c.OnClose(i)
			}
			return true
		}
		if c.OnSelect != nil {
			c.OnSelect(i)
		}
		return true
	case ui.MouseButtonMiddle:
		if c.OnClose != nil {
			c.OnClose(i)
		}
		return true
	case ui.MouseButtonRight:
		c.menuTab = i
		return false // the context menu opens
	}
	return false
}

func (c *TabBar) mouseMove(x, y int, mods ui.KeyModifiers) bool {
	i, onClose := c.tabAt(x)
	if i != c.hover || onClose != c.onClose {
		c.hover, c.onClose = i, onClose
		tip := ""
		if i >= 0 {
			tip = c.tabs[i].tooltip
		}
		c.SetTooltip(tip)
		c.Update()
	}
	return false
}

// blend mixes b into a by k (0..1)
func blend(a, b color.RGBA, k float64) color.RGBA {
	mix := func(x, y uint8) uint8 { return uint8(float64(x)*(1-k) + float64(y)*k) }
	return color.RGBA{mix(a.R, b.R), mix(a.G, b.G), mix(a.B, b.B), 255}
}
