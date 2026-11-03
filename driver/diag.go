package driver

import (
	"fmt"
	"time"
)

// Severity of a diagnostic message
type Severity int

const (
	SevError Severity = iota
	SevWarning
	SevNote
)

func (s Severity) String() string {
	switch s {
	case SevWarning:
		return "warning"
	case SevNote:
		return "note"
	}
	return "error"
}

// Diagnostic is one compiler or linker message. File is absolute (or
// "<altc>/name.h" for built-in headers); Line and Col start at 1 and are 0
// when unknown (e.g. linker errors). Col counts bytes of the line, like gcc
// (a tab or a multi-byte UTF-8 character advances it by its byte length).
type Diagnostic struct {
	File     string
	Line     int
	Col      int
	Severity Severity
	Message  string
}

// String formats the message like gcc: "file:line:col: error: text"
func (d Diagnostic) String() string {
	switch {
	case d.File == "":
		return fmt.Sprintf("%s: %s", d.Severity, d.Message)
	case d.Line == 0:
		return fmt.Sprintf("%s: %s: %s", d.File, d.Severity, d.Message)
	case d.Col == 0:
		return fmt.Sprintf("%s:%d: %s: %s", d.File, d.Line, d.Severity, d.Message)
	}
	return fmt.Sprintf("%s:%d:%d: %s: %s", d.File, d.Line, d.Col, d.Severity, d.Message)
}

// BuildOptions control one build
type BuildOptions struct {
	Jobs int               // parallel compilations; 0: number of CPUs
	Log  func(line string) // progress lines ("compiling x.c", "linking ..."); may be nil; called from any goroutine
}

// BuildResult is what a build produced
type BuildResult struct {
	OK          bool
	Output      string // absolute path of the executable (when OK)
	Diagnostics []Diagnostic
	Duration    time.Duration
}

// HasErrors tells whether any diagnostic is an error
func (r *BuildResult) HasErrors() bool {
	for _, d := range r.Diagnostics {
		if d.Severity == SevError {
			return true
		}
	}
	return false
}
