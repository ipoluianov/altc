package editor

import (
	"image/color"

	"github.com/ipoluianov/nui/ui"
)

// Colors are the colors of the editor in a theme
type Colors struct {
	Tokens      [KindNumber + 1]color.RGBA // by Kind; KindPlain is not used
	CurrentLine color.RGBA
	FindMatch   color.RGBA
	// The background of the token a diagnostic points at, of its line and
	// the mark in the gutter, by severity
	ErrorToken, WarningToken color.RGBA
	ErrorLine, WarningLine   color.RGBA
	ErrorMark, WarningMark   color.RGBA
}

var darkColors = Colors{
	Tokens: [...]color.RGBA{
		KindKeyword: ui.ColorFromHex("#569cd6"),
		KindType:    ui.ColorFromHex("#4ec9b0"),
		KindPreproc: ui.ColorFromHex("#c586c0"),
		KindString:  ui.ColorFromHex("#ce9178"),
		KindChar:    ui.ColorFromHex("#d7ba7d"),
		KindComment: ui.ColorFromHex("#6a9955"),
		KindNumber:  ui.ColorFromHex("#b5cea8"),
	},
	CurrentLine:  color.RGBA{255, 255, 255, 14},
	FindMatch:    color.RGBA{230, 180, 40, 110},
	ErrorToken:   color.RGBA{240, 60, 60, 120},
	WarningToken: color.RGBA{230, 170, 30, 110},
	ErrorLine:    color.RGBA{240, 60, 60, 36},
	WarningLine:  color.RGBA{230, 170, 30, 30},
	ErrorMark:    ui.ColorFromHex("#f14c4c"),
	WarningMark:  ui.ColorFromHex("#cca700"),
}

var lightColors = Colors{
	Tokens: [...]color.RGBA{
		KindKeyword: ui.ColorFromHex("#0000ff"),
		KindType:    ui.ColorFromHex("#267f99"),
		KindPreproc: ui.ColorFromHex("#af00db"),
		KindString:  ui.ColorFromHex("#a31515"),
		KindChar:    ui.ColorFromHex("#b35900"),
		KindComment: ui.ColorFromHex("#008000"),
		KindNumber:  ui.ColorFromHex("#098658"),
	},
	CurrentLine:  color.RGBA{0, 0, 0, 12},
	FindMatch:    color.RGBA{250, 200, 40, 140},
	ErrorToken:   color.RGBA{230, 40, 40, 90},
	WarningToken: color.RGBA{230, 160, 0, 100},
	ErrorLine:    color.RGBA{230, 40, 40, 26},
	WarningLine:  color.RGBA{230, 160, 0, 30},
	ErrorMark:    ui.ColorFromHex("#e51400"),
	WarningMark:  ui.ColorFromHex("#bf8803"),
}

// CurrentColors returns the colors of the current theme
func CurrentColors() *Colors {
	if ui.IsDarkTheme {
		return &darkColors
	}
	return &lightColors
}
