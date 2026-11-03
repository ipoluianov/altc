package forms

import (
	"image"
	"image/color"
	"math"
	"path/filepath"
	"strings"
	"sync"

	"github.com/ipoluianov/nui/ui"
)

// The icons are drawn on a grid of 24 by 24 and scaled to the size asked:
// they stay crisp at any size and follow the theme

// Sizes of the icons
const (
	toolIconSize = 20
	menuIconSize = 16
)

type iconKey struct {
	name string
	size int
	dark bool
}

var (
	iconMtx   sync.Mutex
	iconCache = map[iconKey]image.Image{}
)

// iconInk is the color of the one-color icons in the current theme
func iconInk() color.RGBA {
	if ui.IsDarkTheme {
		return ui.ColorFromHex("#c5c5c5")
	}
	return ui.ColorFromHex("#455a64")
}

var (
	colorGreen  = ui.ColorFromHex("#3fa652")
	colorRed    = ui.ColorFromHex("#e5484d")
	colorYellow = ui.ColorFromHex("#e0a800")
	colorBlue   = ui.ColorFromHex("#3d8bf2")
	colorPurple = ui.ColorFromHex("#a46ad6")
	colorFolder = ui.ColorFromHex("#d9a53f")
)

// icon returns the icon of the name at the size, in the colors of the current theme
func icon(name string, size int) image.Image {
	key := iconKey{name, size, ui.IsDarkTheme}
	iconMtx.Lock()
	defer iconMtx.Unlock()
	if img, ok := iconCache[key]; ok {
		return img
	}
	img := drawIcon(name, size)
	iconCache[key] = img
	return img
}

func stroke(cnv *ui.Canvas, p *ui.Path, w float64, col color.Color) {
	cnv.StrokePath(p, ui.StrokeStyle{Width: w, Join: ui.LineJoinRound, Cap: ui.LineCapRound}, col)
}

func svgPath(d string) *ui.Path {
	p, err := ui.ParseSVGPath(d)
	if err != nil {
		return ui.NewPath()
	}
	return p
}

func drawIcon(name string, size int) image.Image {
	img := image.NewRGBA(image.Rect(0, 0, size, size))
	cnv := ui.NewCanvas(img)
	cnv.SetDirectTranslateAndClip(0, 0, size, size)
	cnv.ScaleBy(float64(size)/24, float64(size)/24)
	ink := iconInk()
	sw := 2.0
	if size <= 16 {
		sw = 2.4 // thicker lines stay crisp when small
	}
	switch name {
	case "folder":
		cnv.FillPath(svgPath("M2 6.5C2 5.1 3.1 4 4.5 4H9l2 2.5h8.5C20.9 6.5 22 7.6 22 9v9.5c0 1.4-1.1 2.5-2.5 2.5h-15C3.1 21 2 19.9 2 18.5z"), colorFolder)
	case "open":
		cnv.FillPath(svgPath("M2 6.5C2 5.1 3.1 4 4.5 4H9l2 2.5h7C19.4 6.5 20 7.1 20 8.5V10H7.5L4 20H4.5C3.1 20 2 19 2 17.5z"), colorFolder)
		cnv.FillPath(svgPath("M7.5 11H23l-4 10H3.5z"), colorFolder)
	case "file", "newfile", "cfile", "hfile":
		p := svgPath("M6 2.5h8l5 5v13c0 .6-.4 1-1 1H6c-.6 0-1-.4-1-1v-17c0-.6.4-1 1-1zM14 2.5V7.5h5")
		stroke(cnv, p, sw*0.75, ink)
		switch name {
		case "newfile":
			cnv.FillCircle(17, 17, 6, colorGreen)
			stroke(cnv, svgPath("M17 14v6M14 17h6"), 2, color.White)
		case "cfile":
			drawLetter(cnv, "C", colorBlue)
		case "hfile":
			drawLetter(cnv, "H", colorPurple)
		}
	case "save", "saveall":
		off := 0.0
		if name == "saveall" {
			stroke(cnv, svgPath("M7 2.5h12.5c.6 0 1 .4 1 1V17"), sw*0.75, ink)
			off = 2.5
		}
		cnv.Save()
		cnv.Translate(-off*0.6, off*0.6)
		stroke(cnv, svgPath("M4.5 5.5h11l3 3v11c0 .6-.4 1-1 1h-13c-.6 0-1-.4-1-1v-13c0-.6.4-1 1-1z"), sw*0.75, ink)
		stroke(cnv, svgPath("M8 5.5v4h6v-4M8 20.5v-5h7v5"), sw*0.6, ink)
		cnv.Restore()
	case "build":
		// A hammer
		stroke(cnv, svgPath("M4.5 19.5l10-10"), 3, ink)
		cnv.StrokePath(svgPath("M10 5l9 9"), ui.StrokeStyle{Width: 6, Join: ui.LineJoinRound}, colorYellow)
	case "run":
		cnv.FillPath(svgPath("M7 4.5v15c0 .8.9 1.3 1.5.8l11.5-7.5c.6-.4.6-1.2 0-1.6L8.5 3.7C7.9 3.2 7 3.7 7 4.5z"), colorGreen)
	case "stop":
		cnv.FillRoundedRectAA(5, 5, 14, 14, 2, colorRed)
	case "settings":
		drawGear(cnv, ink)
	case "refresh":
		stroke(cnv, svgPath("M19.5 12a7.5 7.5 0 1 1-2.2-5.3"), sw*0.8, ink)
		cnv.FillPath(svgPath("M20.5 3v6.5H14z"), ink)
	case "find":
		stroke(cnv, svgPath("M10.5 4a6.5 6.5 0 1 0 0 13a6.5 6.5 0 1 0 0-13zM15.5 15.5l5 5"), sw, ink)
	case "error":
		cnv.FillCircle(12, 12, 10, colorRed)
		stroke(cnv, svgPath("M8.5 8.5l7 7M15.5 8.5l-7 7"), 2.6, color.White)
	case "warning":
		cnv.FillPath(svgPath("M12 2.5c.5 0 .9.3 1.2.7l9 16c.5.9-.1 2-1.2 2h-18c-1.1 0-1.7-1.1-1.2-2l9-16c.3-.4.7-.7 1.2-.7z"), colorYellow)
		stroke(cnv, svgPath("M12 8.5v6"), 2.6, color.Black)
		cnv.FillCircle(12, 18, 1.5, color.Black)
	case "note":
		cnv.FillCircle(12, 12, 10, colorBlue)
		stroke(cnv, svgPath("M12 11v6"), 2.6, color.White)
		cnv.FillCircle(12, 7.5, 1.5, color.White)
	case "close":
		stroke(cnv, svgPath("M6 6l12 12M18 6L6 18"), sw, ink)
	}
	return img
}

// drawLetter puts a letter in a badge at the corner of a file icon
func drawLetter(cnv *ui.Canvas, letter string, col color.RGBA) {
	cnv.FillRoundedRectAA(9, 10, 15, 13, 3, col)
	cnv.SetColor(color.White)
	cnv.SetFontFamily(ui.FontFamilySans + ":bold")
	cnv.SetFontSize(12)
	cnv.SetHAlign(ui.HAlignCenter)
	cnv.SetVAlign(ui.VAlignCenter)
	cnv.DrawText(9, 10, 15, 13, letter)
}

func drawGear(cnv *ui.Canvas, ink color.RGBA) {
	p := ui.NewPath()
	const teeth = 8
	for i := 0; i < teeth*2; i++ {
		a0 := float64(i) * math.Pi / teeth
		a1 := float64(i+1) * math.Pi / teeth
		r := 10.0
		if i%2 == 1 {
			r = 7.5
		}
		x0, y0 := 12+r*math.Cos(a0+0.08), 12+r*math.Sin(a0+0.08)
		x1, y1 := 12+r*math.Cos(a1-0.08), 12+r*math.Sin(a1-0.08)
		if i == 0 {
			p.MoveTo(x0, y0)
		} else {
			p.LineTo(x0, y0)
		}
		p.LineTo(x1, y1)
	}
	p.Close()
	p.Circle(12, 12, 3.5)
	p.SetFillRule(ui.FillRuleEvenOdd)
	cnv.FillPath(p, ink)
}

// fileIcon returns the icon of a file by its name
func fileIcon(name string, size int) image.Image {
	switch strings.ToLower(filepath.Ext(name)) {
	case ".c":
		return icon("cfile", size)
	case ".h":
		return icon("hfile", size)
	}
	return icon("file", size)
}
