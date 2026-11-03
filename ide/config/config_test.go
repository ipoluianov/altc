package config

import (
	"fmt"
	"testing"
)

func TestRecent(t *testing.T) {
	var s Settings
	for i := 0; i < 15; i++ {
		s.AddRecent(fmt.Sprint("/p", i))
	}
	if len(s.Recent) != MaxRecent || s.Recent[0] != "/p14" {
		t.Fatalf("recent: %v", s.Recent)
	}
	s.AddRecent("/p10")
	if s.Recent[0] != "/p10" || s.Recent[1] != "/p14" || len(s.Recent) != MaxRecent {
		t.Fatalf("moved to the front: %v", s.Recent)
	}
	s.RemoveRecent("/p10")
	if s.Recent[0] != "/p14" || len(s.Recent) != MaxRecent-1 {
		t.Fatalf("removed: %v", s.Recent)
	}
}

func TestSaveLoad(t *testing.T) {
	SetDirectory(t.TempDir())
	defer SetDirectory("")
	Load()
	err := Update(func(s *Settings) {
		s.Theme = ThemeLight
		s.AddRecent("/a")
		s.SetProject("/a", &ProjectState{OpenFiles: []string{"/a/main.c"}, ActiveFile: "/a/main.c"})
		s.SetProject("/gone", &ProjectState{}) // not recent: dropped by the next SetProject
		s.SetProject("/a", &ProjectState{OpenFiles: []string{"/a/x.c"}})
	})
	if err != nil {
		t.Fatal(err)
	}
	Load()
	s := Get()
	if s.Theme != ThemeLight || s.LastProject != "" || len(s.Recent) != 1 {
		t.Fatalf("loaded %+v", s)
	}
	if p := s.Project("/a"); p == nil || p.OpenFiles[0] != "/a/x.c" {
		t.Fatalf("project state %+v", p)
	}
	if s.Project("/gone") != nil {
		t.Fatal("state of a project not in the recent ones kept")
	}
	if err := SaveWindowState(WindowState{Width: 800, Height: 600, TreeWidth: 200}); err != nil {
		t.Fatal(err)
	}
	if w, ok := LoadWindowState(); !ok || w.TreeWidth != 200 {
		t.Fatalf("window state %+v %v", w, ok)
	}
}
