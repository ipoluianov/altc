// Package ide is the simple development environment of AltC: a project
// folder in a tree, the files in tabs of a code editor, F7 builds, F5 runs
package ide

import (
	"bytes"
	_ "embed"
	"image"
	"image/png"

	"github.com/ipoluianov/altc/ide/config"
	"github.com/ipoluianov/altc/ide/forms"
	"github.com/ipoluianov/nui/ui"
)

// icon.png is rendered from icon.svg by render-icon.sh (the SVG is for
// the desktop files of Linux)
//
//go:embed icon.png
var iconPNG []byte

// IconPNG returns the application icon as a PNG file
func IconPNG() []byte { return iconPNG }

func appIcon() image.Image {
	img, err := png.Decode(bytes.NewReader(iconPNG))
	if err != nil {
		return nil
	}
	return img
}

// Run opens the IDE window and returns when it is closed; dir is the
// project folder to open ("" for the one of the last run)
func Run(dir string) {
	config.Load()
	settings := config.Get()
	forms.SetLanguage(settings.Language)
	forms.ApplyTheme(settings.Theme)
	forms.AppIcon = appIcon()
	ui.SetAppIcon(forms.AppIcon)

	form := ui.NewForm()
	main := forms.NewMainForm(form)
	form.Panel().SetPanelPadding(0)
	form.Panel().SetCellPadding(0)
	form.Panel().AddWidget(0, 0, main)
	maximized := main.RestoreWindowState()
	form.OnClose = main.CanClose
	form.Show()
	form.Invoke(func() {
		if maximized {
			form.Maximize()
		}
		main.RestoreProject(dir)
	})
	form.Exec()
}
