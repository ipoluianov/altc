package driver

import (
	"encoding/json"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

// ProjectFileName is the optional project description in the project folder
const ProjectFileName = "altc.json"

// Project is a folder with C sources that is built into one executable.
// Without altc.json every *.c file under Dir is compiled.
type Project struct {
	Dir         string   `json:"-"`                     // absolute path of the project folder
	Name        string   `json:"name,omitempty"`        // executable name; default: folder name
	Sources     []string `json:"sources,omitempty"`     // files or folders relative to Dir; empty: all *.c
	Exclude     []string `json:"exclude,omitempty"`     // files or folders relative to Dir to skip
	IncludeDirs []string `json:"includeDirs,omitempty"` // relative to Dir or absolute
	Defines     []string `json:"defines,omitempty"`     // "NAME" or "NAME=VALUE"
	Libs        []string `json:"libs,omitempty"`        // "m", "pthread" ... (-l)
	LibDirs     []string `json:"libDirs,omitempty"`     // -L
	Std         string   `json:"std,omitempty"`         // "c17" by default
	Output      string   `json:"output,omitempty"`      // relative to Dir; default: "build/<Name>"
	CFlags      []string `json:"cflags,omitempty"`      // extra command-line flags
	Args        []string `json:"args,omitempty"`        // arguments for "run"
}

// LoadProject reads dir/altc.json when it exists; otherwise returns defaults
func LoadProject(dir string) (*Project, error) {
	abs, err := filepath.Abs(dir)
	if err != nil {
		return nil, err
	}
	st, err := os.Stat(abs)
	if err != nil {
		return nil, err
	}
	if !st.IsDir() {
		return nil, fmt.Errorf("%s is not a directory", abs)
	}
	p := &Project{Dir: abs}
	data, err := os.ReadFile(filepath.Join(abs, ProjectFileName))
	if err == nil {
		if err := json.Unmarshal(data, p); err != nil {
			return nil, fmt.Errorf("%s: %v", ProjectFileName, err)
		}
	} else if !os.IsNotExist(err) {
		return nil, err
	}
	p.Dir = abs
	return p, nil
}

// Save writes the project description to Dir/altc.json
func (p *Project) Save() error {
	data, err := json.MarshalIndent(p, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(filepath.Join(p.Dir, ProjectFileName), append(data, '\n'), 0o644)
}

// ExeName is the name of the produced executable
func (p *Project) ExeName() string {
	if p.Name != "" {
		return p.Name
	}
	return filepath.Base(p.Dir)
}

// OutputPath is the absolute path of the produced executable
func (p *Project) OutputPath() string {
	out := p.Output
	if out == "" {
		out = filepath.Join("build", p.ExeName())
	}
	if filepath.IsAbs(out) {
		return out
	}
	return filepath.Join(p.Dir, out)
}

func (p *Project) abs(path string) string {
	if filepath.IsAbs(path) {
		return filepath.Clean(path)
	}
	return filepath.Join(p.Dir, path)
}

// skipDir tells folders that never hold project sources
func skipDir(name string) bool {
	return strings.HasPrefix(name, ".") || name == "build" || name == "node_modules"
}

// SourceFiles returns the absolute paths of the .c files to compile, sorted
func (p *Project) SourceFiles() ([]string, error) {
	excluded := map[string]bool{}
	for _, e := range p.Exclude {
		excluded[p.abs(e)] = true
	}
	roots := p.Sources
	if len(roots) == 0 {
		roots = []string{"."}
	}
	seen := map[string]bool{}
	var files []string
	for _, r := range roots {
		root := p.abs(r)
		st, err := os.Stat(root)
		if err != nil {
			return nil, err
		}
		if !st.IsDir() {
			if !excluded[root] && !seen[root] {
				seen[root] = true
				files = append(files, root)
			}
			continue
		}
		err = filepath.WalkDir(root, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return err
			}
			if excluded[path] {
				if d.IsDir() {
					return filepath.SkipDir
				}
				return nil
			}
			if d.IsDir() {
				if path != root && skipDir(d.Name()) {
					return filepath.SkipDir
				}
				return nil
			}
			if strings.HasSuffix(d.Name(), ".c") && !seen[path] {
				seen[path] = true
				files = append(files, path)
			}
			return nil
		})
		if err != nil {
			return nil, err
		}
	}
	sort.Strings(files)
	return files, nil
}

// IncludePaths are the -I folders: the project folder, its "include"
// subfolder when present, then the configured ones
func (p *Project) IncludePaths() []string {
	paths := []string{p.Dir}
	if st, err := os.Stat(filepath.Join(p.Dir, "include")); err == nil && st.IsDir() {
		paths = append(paths, filepath.Join(p.Dir, "include"))
	}
	for _, d := range p.IncludeDirs {
		paths = append(paths, p.abs(d))
	}
	return paths
}
