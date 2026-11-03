// Package config keeps the IDE settings between runs in ~/.altbins/.altc:
// the theme, the language, the last and the recent projects, the tabs
// open in each project, the window layout
package config

import (
	"encoding/json"
	"os"
	"path/filepath"
	"sync"
)

// MaxRecent is how many recent projects are remembered
const MaxRecent = 10

// Font sizes of the editor
const (
	DefaultFontSize = 14
	MinFontSize     = 8
	MaxFontSize     = 32
)

// The color themes (Settings.Theme)
const (
	ThemeDark  = ""
	ThemeLight = "light"
)

// ProjectState is what was open in a project when it was closed
type ProjectState struct {
	OpenFiles  []string `json:",omitempty"` // absolute paths, in the order of the tabs
	ActiveFile string   `json:",omitempty"`
	// Cursor positions of the open files (line, column from 0)
	Cursors map[string][2]int `json:",omitempty"`
}

// Settings are the options of the IDE
type Settings struct {
	// Color theme: "light"; "" - the dark one
	Theme string `json:",omitempty"`
	// Language of the interface as a tag like "ru"; "" - the system's
	Language string `json:",omitempty"`
	// Font size of the editor in pixels; 0 - DefaultFontSize
	FontSize int `json:",omitempty"`

	LastProject string   `json:",omitempty"`
	Recent      []string `json:",omitempty"` // the last opened first

	Projects map[string]*ProjectState `json:",omitempty"` // by the project folder
}

// EditorFontSize returns the font size of the editor within the limits
func (s Settings) EditorFontSize() int {
	if s.FontSize == 0 {
		return DefaultFontSize
	}
	return max(MinFontSize, min(s.FontSize, MaxFontSize))
}

// AddRecent puts the project folder first in the recent ones
func (s *Settings) AddRecent(dir string) {
	list := []string{dir}
	for _, d := range s.Recent {
		if d != dir {
			list = append(list, d)
		}
	}
	if len(list) > MaxRecent {
		list = list[:MaxRecent]
	}
	s.Recent = list
}

// RemoveRecent forgets the project folder (e.g. it no longer exists)
func (s *Settings) RemoveRecent(dir string) {
	var list []string
	for _, d := range s.Recent {
		if d != dir {
			list = append(list, d)
		}
	}
	s.Recent = list
	delete(s.Projects, dir)
}

// Project returns the saved state of the project folder, nil if none
func (s Settings) Project(dir string) *ProjectState {
	return s.Projects[dir]
}

// SetProject saves the state of the project folder. Only the recent
// projects are kept, so the file doesn't grow forever.
func (s *Settings) SetProject(dir string, st *ProjectState) {
	if s.Projects == nil {
		s.Projects = make(map[string]*ProjectState)
	}
	s.Projects[dir] = st
	keep := map[string]bool{dir: true}
	for _, d := range s.Recent {
		keep[d] = true
	}
	for d := range s.Projects {
		if !keep[d] {
			delete(s.Projects, d)
		}
	}
}

var (
	mtx      sync.Mutex
	settings Settings
	// dirOverride replaces the directory in tests
	dirOverride string
)

// ConfigDirectory returns ~/.altbins/.altc
func ConfigDirectory() string {
	if dirOverride != "" {
		return dirOverride
	}
	homeDir, err := os.UserHomeDir()
	if err != nil {
		homeDir = "."
	}
	return filepath.Join(homeDir, ".altbins", ".altc")
}

// SetDirectory keeps the files in dir instead of ~/.altbins/.altc (tests)
func SetDirectory(dir string) {
	dirOverride = dir
}

func settingsPath() string {
	return filepath.Join(ConfigDirectory(), "settings.json")
}

// Load reads the settings file; missing values get the defaults
func Load() {
	var s Settings
	if bs, err := os.ReadFile(settingsPath()); err == nil {
		_ = json.Unmarshal(bs, &s)
	}
	mtx.Lock()
	settings = s
	mtx.Unlock()
}

// Get returns a copy of the current settings
func Get() Settings {
	mtx.Lock()
	defer mtx.Unlock()
	return settings
}

// Update changes the settings by f and saves them
func Update(f func(s *Settings)) error {
	mtx.Lock()
	f(&settings)
	bs, err := json.MarshalIndent(settings, "", "  ")
	mtx.Unlock()
	if err != nil {
		return err
	}
	return writeFile(settingsPath(), bs)
}

// writeFile writes to a temporary file and renames it over the target, so a
// crash leaves either the old file or the new one, never a torn one
func writeFile(fullPath string, bs []byte) error {
	if err := os.MkdirAll(filepath.Dir(fullPath), 0o755); err != nil {
		return err
	}
	tmpPath := fullPath + ".tmp"
	if err := os.WriteFile(tmpPath, bs, 0o644); err != nil {
		os.Remove(tmpPath)
		return err
	}
	if err := os.Rename(tmpPath, fullPath); err != nil {
		os.Remove(tmpPath)
		return err
	}
	return nil
}
