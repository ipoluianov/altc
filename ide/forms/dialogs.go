package forms

import (
	"strings"
	"time"

	"github.com/ipoluianov/altc/app"
	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/nui/ui"
)

// The standards offered in the project settings ("" - the default)
var standards = []string{"", "c89", "c99", "c11", "c17", "c23", "gnu89", "gnu99", "gnu11", "gnu17", "gnu23"}

// ProjectDialog edits the altc.json of the project
type ProjectDialog struct {
	ui.DialogContent

	p *driver.Project

	name, output, libs, cflags, args       *ui.TextBox
	std                                    *ui.ComboBox
	sources, includeDirs, defines, libDirs *ui.TextBox

	// OnSaved gets the project as saved
	OnSaved func(p *driver.Project)
}

// NewProjectDialog makes the dialog with the settings of the project
func NewProjectDialog(p *driver.Project) *ProjectDialog {
	c := &ProjectDialog{p: p}
	c.InitWidget()
	s := T()

	line := func(text, hint string) *ui.TextBox {
		tb := ui.NewTextBox()
		tb.SetText(text)
		tb.SetHint(hint)
		tb.SetMinWidth(360)
		return tb
	}
	lines := func(list []string, hint string) *ui.TextBox {
		tb := ui.NewTextBox()
		tb.SetMultiline(true)
		tb.SetText(strings.Join(list, "\n"))
		tb.SetHint(hint)
		tb.SetMinWidth(360)
		return tb
	}
	c.name = line(p.Name, s.NameHint+" ("+p.ExeName()+")")
	c.name.SetName("altc-project-name")
	c.std = ui.NewComboBox()
	for _, std := range standards {
		text := std
		if std == "" {
			text = s.StdDefault
		}
		c.std.AddItem(text, std)
	}
	if c.std.FindData(p.Std) < 0 {
		c.std.AddItem(p.Std, p.Std)
	}
	c.std.SetSelectedIndex(c.std.FindData(p.Std))
	c.output = line(p.Output, "build/"+p.ExeName())
	c.sources = lines(p.Sources, s.SourcesHint)
	c.includeDirs = lines(p.IncludeDirs, s.OnePerLine)
	c.defines = lines(p.Defines, s.DefinesHint)
	c.libs = line(strings.Join(p.Libs, " "), s.LibsHint)
	c.libDirs = lines(p.LibDirs, s.OnePerLine)
	c.cflags = line(joinArgs(p.CFlags), s.CFlagsHint)
	c.args = line(joinArgs(p.Args), s.ArgsHint)
	c.args.SetName("altc-project-args")

	fields := ui.NewFormLayout()
	fields.AddRow(s.Name, c.name)
	fields.AddRow(s.Standard, c.std)
	fields.AddRow(s.Output, c.output)
	fields.AddRow(s.Sources, c.sources)
	fields.AddRow(s.IncludeDirs, c.includeDirs)
	fields.AddRow(s.Defines, c.defines)
	fields.AddRow(s.Libs, c.libs)
	fields.AddRow(s.LibDirs, c.libDirs)
	fields.AddRow(s.CFlags, c.cflags)
	fields.AddRow(s.Args, c.args)
	c.AddWidget(0, 0, fields)
	c.AddVSpacer(1, 0)

	buttons := ui.NewDialogButtonBox()
	buttons.AddOK(c.save)
	buttons.AddCancel(func() { c.Form().Close() })
	c.AddWidget(2, 0, buttons)

	c.OnDialogShow = func() {
		c.Form().SetTitle(T().SettingsTitle + " - " + p.ExeName())
		c.Form().MoveToCenterOfParent()
		c.name.Focus()
	}
	return c
}

func (c *ProjectDialog) save() {
	p := *c.p
	p.Name = strings.TrimSpace(c.name.Text())
	p.Std, _ = c.std.SelectedItemData().(string)
	p.Output = strings.TrimSpace(c.output.Text())
	p.Sources = splitLines(c.sources.Text())
	p.IncludeDirs = splitLines(c.includeDirs.Text())
	p.Defines = splitLines(c.defines.Text())
	p.Libs = strings.FieldsFunc(c.libs.Text(), func(r rune) bool { return r == ' ' || r == ',' || r == ';' })
	p.LibDirs = splitLines(c.libDirs.Text())
	p.CFlags = splitArgs(c.cflags.Text())
	p.Args = splitArgs(c.args.Text())
	if err := p.Save(); err != nil {
		ui.ShowMessageBox(c, T().Error, T().CannotSaveProj(err.Error()))
		return
	}
	c.RunInParent(func() {
		if c.OnSaved != nil {
			c.OnSaved(&p)
		}
	})
	c.Form().Close()
}

// splitLines returns the lines of the text that are not empty, trimmed
func splitLines(s string) []string {
	var out []string
	for _, l := range strings.Split(s, "\n") {
		if l = strings.TrimSpace(l); l != "" {
			out = append(out, l)
		}
	}
	return out
}

// splitArgs splits a command line into the arguments as a shell would:
// quotes ("..." and '...') keep the spaces, a backslash escapes a character
func splitArgs(s string) []string {
	var args []string
	var cur strings.Builder
	inArg := false
	var quote rune
	rs := []rune(s)
	for i := 0; i < len(rs); i++ {
		r := rs[i]
		switch {
		case quote != 0:
			if r == quote {
				quote = 0
			} else if r == '\\' && quote == '"' && i+1 < len(rs) && (rs[i+1] == '"' || rs[i+1] == '\\') {
				i++
				cur.WriteRune(rs[i])
			} else {
				cur.WriteRune(r)
			}
		case r == '"' || r == '\'':
			quote = r
			inArg = true
		case r == '\\' && i+1 < len(rs):
			i++
			cur.WriteRune(rs[i])
			inArg = true
		case r == ' ' || r == '\t' || r == '\n':
			if inArg {
				args = append(args, cur.String())
				cur.Reset()
				inArg = false
			}
		default:
			cur.WriteRune(r)
			inArg = true
		}
	}
	if inArg {
		args = append(args, cur.String())
	}
	return args
}

// joinArgs is the inverse of splitArgs
func joinArgs(args []string) string {
	parts := make([]string, len(args))
	for i, a := range args {
		if a != "" && !strings.ContainsAny(a, " \t\n\"'\\") {
			parts[i] = a
			continue
		}
		parts[i] = "'" + strings.ReplaceAll(a, "'", `'"'"'`) + "'"
	}
	return strings.Join(parts, " ")
}

// ShowProjectSettings opens the settings of the project
func (c *MainForm) ShowProjectSettings() {
	if c.project == nil {
		return
	}
	// The file may have changed since it was loaded
	p, err := driver.LoadProject(c.project.Dir)
	if err != nil {
		ui.ShowMessageBox(c, T().Error, T().CannotLoad(c.project.Dir, err.Error()))
		return
	}
	d := NewProjectDialog(p)
	d.OnSaved = func(p *driver.Project) {
		c.project = p
		c.tree.Refresh()
		c.docs.checkChangedOnDisk() // altc.json open in a tab
		c.updateTitle()
		c.status.ShowMessage(T().SettingsSaved, 3*time.Second)
	}
	c.ShowDialog(d)
}

// AboutDialog shows the name, the version and the author
type AboutDialog struct {
	ui.DialogContent
}

// NewAboutDialog makes the About dialog
func NewAboutDialog() *AboutDialog {
	c := &AboutDialog{}
	c.InitWidget()
	s := T()

	content := ui.NewVBox()
	if AppIcon != nil {
		img := ui.NewImageBox()
		img.SetImage(AppIcon)
		img.SetMinSize(64, 64)
		img.SetMaxSize(64, 64)
		row := ui.NewHBox()
		row.AddStretch()
		row.Add(img)
		row.AddStretch()
		content.Add(row)
	}
	center := func(text string) *ui.Label {
		l := ui.NewLabel(text)
		l.SetTextAlign(ui.HAlignCenter)
		content.Add(l)
		return l
	}
	center(app.DisplayName).SetFontSize(24)
	center(s.Description)
	center(s.Version + " " + app.Version)
	center(s.Author + ": " + app.Author)
	center(app.Copyright())
	center(s.License + ": " + app.License)
	center(app.Website)
	c.AddWidget(0, 0, content)
	c.AddVSpacer(1, 0)

	buttons := ui.NewHBox()
	buttons.AddStretch()
	website := ui.NewButton(s.VisitWebsite)
	website.OnClick.Connect(func(ui.Widgeter) {
		if err := app.OpenSiteURL(app.Website, "about_dialog"); err != nil {
			ui.ShowMessageBox(c, T().Error, err.Error())
		}
	})
	closeBtn := ui.NewButton(s.Close)
	closeBtn.OnClick.Connect(func(ui.Widgeter) { c.Form().Close() })
	buttons.Add(website)
	buttons.Add(closeBtn)
	c.AddWidget(2, 0, buttons)

	c.OnDialogShow = func() {
		c.Form().SetTitle(T().AboutTitle(app.DisplayName))
		c.Form().SetSize(420, 380)
		c.Form().MoveToCenterOfParent()
		c.Form().SetAcceptButton(closeBtn)
		c.Form().SetCancelButton(closeBtn)
	}
	return c
}
