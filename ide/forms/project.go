package forms

import (
	"os"
	"path/filepath"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/altc/ide/config"
	"github.com/ipoluianov/nui/ui"
)

// ShowOpenFolder asks for a project folder and opens it
func (c *MainForm) ShowOpenFolder() {
	opts := ui.SelectDirectoryDialogOptions{Title: plain(T().OpenFolder)}
	if c.project != nil {
		opts.DefaultDirectory = filepath.Dir(c.project.Dir)
	}
	c.form.ShowSelectDirectoryDialog(opts, func(path string, err error) {
		if err != nil || path == "" {
			return
		}
		c.OpenProject(path)
	})
}

// OpenProject opens the folder as the project, after the unsaved files of
// the open one are saved or dropped
func (c *MainForm) OpenProject(dir string) {
	abs, err := filepath.Abs(dir)
	if err == nil {
		dir = abs
	}
	if c.project != nil && c.project.Dir == dir {
		return
	}
	if st, err := os.Stat(dir); err != nil || !st.IsDir() {
		ui.ShowMessageBox(c, T().Error, T().FolderNotFound(dir))
		config.Update(func(s *config.Settings) { s.RemoveRecent(dir) })
		c.updateRecentMenu()
		c.start.update()
		return
	}
	c.closeProject(func() {
		p, err := driver.LoadProject(dir)
		if err != nil {
			ui.ShowMessageBox(c, T().Error, T().CannotLoad(dir, err.Error()))
			return
		}
		c.setProject(p)
	})
}

// RestoreProject opens the project of the last run, if it is still there
func (c *MainForm) RestoreProject(dir string) {
	if dir == "" {
		dir = config.Get().LastProject
		if dir == "" {
			return
		}
		if st, err := os.Stat(dir); err != nil || !st.IsDir() {
			return
		}
	}
	c.OpenProject(dir)
}

func (c *MainForm) setProject(p *driver.Project) {
	c.project = p
	config.Update(func(s *config.Settings) {
		s.LastProject = p.Dir
		s.AddRecent(p.Dir)
	})
	c.updateRecentMenu()
	c.tree.SetRoot(p.Dir)
	c.output.clear()
	c.diags = nil
	c.start.SetVisible(false)
	c.work.SetVisible(true)
	// The tabs of last time
	if st := config.Get().Project(p.Dir); st != nil {
		for _, path := range st.OpenFiles {
			if _, err := os.Stat(path); err != nil {
				continue
			}
			if d := c.docs.open(path, false); d != nil {
				if cur, ok := st.Cursors[path]; ok {
					d.ed.SetCursorPosition(ui.TextPos{Line: cur[0], Col: cur[1]})
					d.pendingScroll = true
				}
			}
		}
		if st.ActiveFile != "" {
			c.docs.open(st.ActiveFile, false)
		}
	}
	c.status.SetText(T().Ready)
	c.updateTitle()
	if d := c.docs.current(); d != nil {
		d.ed.Focus()
	} else {
		c.tree.Focus()
	}
}

// closeProject closes the folder after its unsaved files are saved or
// dropped, then calls then (if not nil); nothing happens when cancelled
func (c *MainForm) closeProject(then func()) {
	if c.project == nil {
		if then != nil {
			then()
		}
		return
	}
	c.docs.askSaveAll(func() {
		c.Stop()
		c.saveProjectState()
		c.docs.closeAll()
		c.project = nil
		c.diags = nil
		c.tree.SetRoot("")
		c.output.clear()
		config.Update(func(s *config.Settings) { s.LastProject = "" })
		c.work.SetVisible(false)
		c.start.update()
		c.start.SetVisible(true)
		c.updateTitle()
		if then != nil {
			then()
		}
	})
}

// saveProjectState remembers the open tabs of the project
func (c *MainForm) saveProjectState() {
	if c.project == nil {
		return
	}
	st := &config.ProjectState{Cursors: map[string][2]int{}}
	for _, d := range c.docs.list {
		st.OpenFiles = append(st.OpenFiles, d.path)
		pos := d.ed.CursorPosition()
		st.Cursors[d.path] = [2]int{pos.Line, pos.Col}
	}
	if d := c.docs.current(); d != nil {
		st.ActiveFile = d.path
	}
	dir := c.project.Dir
	config.Update(func(s *config.Settings) { s.SetProject(dir, st) })
}
