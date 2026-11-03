package forms

import (
	"errors"
	"image"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/ipoluianov/nui/ui"
)

// FileTree shows the files of the project folder. The folders are read when
// they are expanded; Refresh brings the tree up to date with the disk
// without collapsing it.
type FileTree struct {
	ui.Widget

	m        *MainForm
	tree     *ui.TreeView
	root     string
	rootNode *ui.TreeNode
	menu     *ui.ContextMenu

	itemRename *ui.ContextMenuItem
	itemDelete *ui.ContextMenuItem
}

type treeEntry struct {
	path string
	dir  bool
}

// NewFileTree makes an empty tree
func NewFileTree(m *MainForm) *FileTree {
	c := &FileTree{m: m}
	c.InitWidget()
	c.SetPanelPadding(0)
	c.SetCellPadding(0)
	c.SetMinWidth(120)
	c.tree = ui.NewTreeView()
	c.tree.SetName("tree")
	c.tree.SetHeaderVisible(false)
	c.tree.SetXExpandable(true)
	c.tree.SetYExpandable(true)
	c.tree.OnExpanding.Connect(func(n *ui.TreeNode) {
		if n.ChildCount() == 0 {
			c.load(n)
		}
	})
	c.tree.OnNodeActivated.Connect(func(n *ui.TreeNode) {
		if e, ok := n.Data().(treeEntry); ok && !e.dir {
			m.docs.open(e.path, true)
		}
	})
	c.AddWidget(0, 0, c.tree)

	s := T()
	c.menu = ui.NewContextMenu(c.tree)
	c.menu.AddItem(s.NewFile, func() { c.newEntry("", false) }).SetTextFunc(func() string { return T().NewFile })
	c.menu.AddItem(s.NewFolder, func() { c.newEntry("", true) }).SetTextFunc(func() string { return T().NewFolder })
	c.itemRename = c.menu.AddItem(s.Rename, c.rename)
	c.itemRename.SetTextFunc(func() string { return T().Rename })
	c.itemDelete = c.menu.AddItem(s.Delete, c.delete)
	c.itemDelete.SetTextFunc(func() string { return T().Delete })
	c.menu.AddSeparator()
	c.menu.AddItem(s.CopyPath, func() {
		if e, ok := c.currentEntry(); ok {
			ui.ClipboardSetText(e.path)
		}
	}).SetTextFunc(func() string { return T().CopyPath })
	c.menu.AddItem(s.Refresh, c.Refresh).SetTextFunc(func() string { return T().Refresh })
	c.menu.SetOnShow(func() {
		e, ok := c.currentEntry()
		editable := ok && e.path != c.root
		c.itemRename.SetEnabled(editable)
		c.itemDelete.SetEnabled(editable)
	})
	c.tree.SetContextMenu(c.menu)
	return c
}

// Focus gives the focus to the tree
func (c *FileTree) Focus() { c.tree.Focus() }

func (c *FileTree) applyLanguage() {}

// applyTheme draws the icons in the colors of the theme
func (c *FileTree) applyTheme() {
	var walk func(nodes []*ui.TreeNode)
	walk = func(nodes []*ui.TreeNode) {
		for _, n := range nodes {
			if e, ok := n.Data().(treeEntry); ok {
				n.SetImage(entryIcon(e))
			}
			walk(n.Children())
		}
	}
	walk(c.tree.Nodes())
}

func entryIcon(e treeEntry) image.Image {
	if e.dir {
		return icon("folder", menuIconSize)
	}
	return fileIcon(e.path, menuIconSize)
}

// SetRoot shows the folder ("" - nothing)
func (c *FileTree) SetRoot(dir string) {
	c.tree.Clear()
	c.root = dir
	c.rootNode = nil
	if dir == "" {
		return
	}
	c.rootNode = c.addNode(nil, treeEntry{dir, true})
	c.load(c.rootNode)
	c.rootNode.Expand()
}

func (c *FileTree) addNode(parent *ui.TreeNode, e treeEntry) *ui.TreeNode {
	n := c.tree.AddNode(parent, filepath.Base(e.path))
	n.SetData(e)
	n.SetImage(entryIcon(e))
	if e.dir {
		n.SetHasChildren(true)
	}
	return n
}

// readDir returns the entries of the folder: the folders first, by name;
// hidden ones are left out
func readDir(dir string) []treeEntry {
	items, err := os.ReadDir(dir)
	if err != nil {
		return nil
	}
	var list []treeEntry
	for _, it := range items {
		if strings.HasPrefix(it.Name(), ".") {
			continue
		}
		isDir := it.IsDir()
		if it.Type()&os.ModeSymlink != 0 {
			if st, err := os.Stat(filepath.Join(dir, it.Name())); err == nil {
				isDir = st.IsDir()
			}
		}
		list = append(list, treeEntry{filepath.Join(dir, it.Name()), isDir})
	}
	sort.Slice(list, func(i, j int) bool {
		if list[i].dir != list[j].dir {
			return list[i].dir
		}
		a, b := strings.ToLower(filepath.Base(list[i].path)), strings.ToLower(filepath.Base(list[j].path))
		if a != b {
			return a < b
		}
		return list[i].path < list[j].path
	})
	return list
}

func (c *FileTree) load(n *ui.TreeNode) {
	e, ok := n.Data().(treeEntry)
	if !ok || !e.dir {
		return
	}
	for _, child := range readDir(e.path) {
		c.addNode(n, child)
	}
}

// Refresh brings the tree up to date with the disk, keeping the folders
// expanded and the chosen node
func (c *FileTree) Refresh() {
	if c.rootNode == nil {
		return
	}
	if _, err := os.Stat(c.root); err != nil {
		return
	}
	cur, hasCur := c.currentEntry()
	if c.sync(c.rootNode) && hasCur {
		c.reveal(cur.path, false)
	}
}

// sync updates the children of the loaded folder node; true if something changed
func (c *FileTree) sync(n *ui.TreeNode) bool {
	e := n.Data().(treeEntry)
	want := readDir(e.path)
	have := n.Children()
	same := len(want) == len(have)
	for i := 0; same && i < len(want); i++ {
		he, _ := have[i].Data().(treeEntry)
		same = he == want[i]
	}
	if same {
		changed := false
		for _, child := range have {
			if ce := child.Data().(treeEntry); ce.dir && child.ChildCount() > 0 {
				changed = c.sync(child) || changed
			}
		}
		return changed
	}
	// The folder changed: its children anew, with the folders that were
	// expanded expanded again
	expanded := map[string]bool{}
	var collect func(nodes []*ui.TreeNode)
	collect = func(nodes []*ui.TreeNode) {
		for _, x := range nodes {
			if x.IsExpanded() {
				expanded[x.Data().(treeEntry).path] = true
			}
			collect(x.Children())
		}
	}
	collect(have)
	n.RemoveChildren()
	var add func(parent *ui.TreeNode, entries []treeEntry)
	add = func(parent *ui.TreeNode, entries []treeEntry) {
		for _, ce := range entries {
			child := c.addNode(parent, ce)
			if ce.dir && expanded[ce.path] {
				add(child, readDir(ce.path))
				child.Expand()
			}
		}
	}
	add(n, want)
	return true
}

// reveal expands the folders down to the path and chooses its node
func (c *FileTree) reveal(path string, focus bool) {
	if c.rootNode == nil {
		return
	}
	rel, err := filepath.Rel(c.root, path)
	if err != nil || strings.HasPrefix(rel, "..") {
		return
	}
	n := c.rootNode
	if rel != "." {
		for _, part := range strings.Split(rel, string(filepath.Separator)) {
			if n.ChildCount() == 0 {
				c.load(n)
			}
			n.Expand()
			var next *ui.TreeNode
			for _, ch := range n.Children() {
				if filepath.Base(ch.Data().(treeEntry).path) == part {
					next = ch
				}
			}
			if next == nil {
				break
			}
			n = next
		}
	}
	c.tree.SetCurrentNode(n)
	if focus {
		c.tree.Focus()
	}
}

func (c *FileTree) currentEntry() (treeEntry, bool) {
	n := c.tree.CurrentNode()
	if n == nil {
		return treeEntry{}, false
	}
	e, ok := n.Data().(treeEntry)
	return e, ok
}

// targetDir is the folder new files go to: the chosen folder, or the
// folder of the chosen file, or the project folder
func (c *FileTree) targetDir() string {
	e, ok := c.currentEntry()
	switch {
	case !ok:
		return c.root
	case e.dir:
		return e.path
	}
	return filepath.Dir(e.path)
}

// checkName tells what is wrong with a name typed for a new file or a
// rename; a name of a new entry may have folders ("src/a.c")
func checkName(dir, name string, withFolders bool) error {
	name = strings.TrimSpace(name)
	if name == "" {
		return errors.New(T().NameEmpty)
	}
	parts := strings.Split(filepath.ToSlash(name), "/")
	if filepath.IsAbs(name) || strings.Contains(name, "\\") || (!withFolders && len(parts) > 1) {
		return errors.New(T().NameInvalid)
	}
	for _, p := range parts {
		if p == "" || p == "." || p == ".." {
			return errors.New(T().NameInvalid)
		}
	}
	if _, err := os.Lstat(filepath.Join(dir, name)); err == nil {
		return errors.New(T().AlreadyExists(name))
	}
	return nil
}

// newEntry asks for the name of a new file (or a folder) in dir ("" - the
// folder chosen in the tree) and makes it
func (c *FileTree) newEntry(dir string, folder bool) {
	if c.root == "" {
		return
	}
	if dir == "" {
		dir = c.targetDir()
	}
	title := T().NewFileTitle
	if folder {
		title = T().NewFolderTitle
	}
	ui.ShowInputDialog(c, ui.InputDialog{
		Title:    title,
		Prompt:   T().NamePrompt(c.m.relPath(dir)),
		Validate: func(text string) error { return checkName(dir, text, true) },
	}, func(name string) {
		path := filepath.Join(dir, strings.TrimSpace(name))
		var err error
		if folder {
			err = os.MkdirAll(path, 0o755)
		} else if err = os.MkdirAll(filepath.Dir(path), 0o755); err == nil {
			var f *os.File
			if f, err = os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0o644); err == nil {
				f.Close()
			}
		}
		if err != nil {
			ui.ShowMessageBox(c, T().Error, err.Error())
			return
		}
		c.Refresh()
		c.reveal(path, folder)
		if !folder {
			c.m.docs.open(path, true)
		}
	})
}

func (c *FileTree) rename() {
	e, ok := c.currentEntry()
	if !ok || e.path == c.root {
		return
	}
	dir := filepath.Dir(e.path)
	ui.ShowInputDialog(c, ui.InputDialog{
		Title:  T().RenameTitle,
		Prompt: T().RenamePrompt,
		Value:  filepath.Base(e.path),
		Validate: func(text string) error {
			if strings.TrimSpace(text) == filepath.Base(e.path) {
				return nil
			}
			return checkName(dir, text, false)
		},
	}, func(name string) {
		newPath := filepath.Join(dir, strings.TrimSpace(name))
		if newPath == e.path {
			return
		}
		if err := os.Rename(e.path, newPath); err != nil {
			ui.ShowMessageBox(c, T().Error, err.Error())
			return
		}
		c.m.docs.renamed(e.path, newPath)
		c.Refresh()
		c.reveal(newPath, true)
	})
}

func (c *FileTree) delete() {
	e, ok := c.currentEntry()
	if !ok || e.path == c.root {
		return
	}
	name := c.m.relPath(e.path)
	ask := T().DeleteFileAsk(name)
	if e.dir {
		ask = T().DeleteFolderAsk(name)
	}
	ui.ShowQuestionMessageBoxYesNo(c, T().DeleteTitle, ask, func() {
		var err error
		if e.dir {
			err = os.RemoveAll(e.path)
		} else {
			err = os.Remove(e.path)
		}
		if err != nil {
			ui.ShowMessageBox(c, T().Error, err.Error())
		}
		c.m.docs.deleted(e.path)
		c.Refresh()
	}, nil)
}
