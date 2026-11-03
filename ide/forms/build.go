package forms

import (
	"context"
	"fmt"
	"io"
	"os/exec"
	"path/filepath"
	"runtime/debug"
	"strings"
	"sync"
	"time"

	"github.com/ipoluianov/altc/driver"
	"github.com/ipoluianov/nui/ui"
)

// busy tells whether a build or the program is running
func (c *MainForm) busy() bool {
	return c.buildCancel != nil || c.runCmd != nil
}

// relPath returns the path relative to the project folder when it is in it
func (c *MainForm) relPath(path string) string {
	if c.project == nil {
		return path
	}
	rel, err := filepath.Rel(c.project.Dir, path)
	if err != nil || strings.HasPrefix(rel, "..") {
		return path
	}
	if rel == "." {
		return filepath.Base(c.project.Dir)
	}
	return rel
}

func formatDuration(d time.Duration) string {
	switch {
	case d < time.Second:
		return d.Round(time.Millisecond).String()
	case d < time.Minute:
		return d.Round(10 * time.Millisecond).String()
	}
	return d.Round(time.Second).String()
}

// safeBuild builds the project; a panic of the compiler is an error of the
// build, not of the IDE
func safeBuild(ctx context.Context, p *driver.Project, opt driver.BuildOptions) (res *driver.BuildResult) {
	start := time.Now()
	defer func() {
		if r := recover(); r != nil {
			if opt.Log != nil {
				opt.Log(fmt.Sprintf("panic: %v\n%s", r, debug.Stack()))
			}
			res = &driver.BuildResult{Diagnostics: []driver.Diagnostic{{
				Severity: driver.SevError, Message: fmt.Sprint("internal compiler error: ", r)}}}
		}
		if res == nil {
			res = &driver.BuildResult{}
		}
		if res.Duration == 0 {
			res.Duration = time.Since(start)
		}
	}()
	return p.Build(ctx, opt)
}

// Build saves the modified files and builds the project in the background;
// then (if not nil) gets the result of a build that succeeded
func (c *MainForm) Build(then func(res *driver.BuildResult)) {
	if c.project == nil || c.busy() {
		return
	}
	if !c.docs.saveAll() {
		return
	}
	// altc.json may have been edited since the project was opened
	p, err := driver.LoadProject(c.project.Dir)
	c.output.clear()
	c.output.showOutput()
	c.diags = nil
	c.docs.applyAllMarkers()
	if err != nil {
		c.output.println(T().CannotLoad(c.project.Dir, err.Error()))
		c.status.SetText(T().CannotLoad(c.project.Dir, err.Error()))
		return
	}
	c.project = p
	c.output.println("== " + T().Building(p.ExeName()))
	c.status.SetText(T().StatusBuilding)
	ctx, cancel := context.WithCancel(context.Background())
	c.buildCancel = cancel
	log := c.output.log
	go func() {
		res := safeBuild(ctx, p, driver.BuildOptions{Log: func(line string) {
			io.WriteString(log, strings.TrimRight(line, "\n")+"\n")
		}})
		ui.Invoke(func() { c.buildDone(ctx, res, then) })
	}()
}

func (c *MainForm) buildDone(ctx context.Context, res *driver.BuildResult, then func(res *driver.BuildResult)) {
	cancelled := ctx.Err() != nil
	if c.buildCancel != nil {
		c.buildCancel()
		c.buildCancel = nil
	}
	errs, warns := 0, 0
	for _, d := range res.Diagnostics {
		c.output.println(c.diagText(d))
		switch d.Severity {
		case driver.SevError:
			errs++
		case driver.SevWarning:
			warns++
		}
	}
	c.diags = res.Diagnostics
	c.output.setDiagnostics(res.Diagnostics)
	c.docs.applyAllMarkers()
	var msg string
	switch {
	case cancelled:
		msg = T().BuildCancelled
	case res.OK:
		msg = T().BuildOK(formatDuration(res.Duration))
	default:
		msg = T().BuildFailed(errs, warns)
	}
	c.output.println("== " + msg)
	c.status.SetText(msg)
	c.tree.Refresh()
	if !res.OK {
		for _, d := range res.Diagnostics {
			if d.File != "" {
				c.output.showProblems()
				break
			}
		}
	}
	if res.OK && !cancelled && then != nil {
		then(res)
	}
}

// diagText is the diagnostic as the log shows it, with the path in the project
func (c *MainForm) diagText(d driver.Diagnostic) string {
	if d.File != "" {
		d.File = c.relPath(d.File)
	}
	return d.String()
}

// Run builds the project and runs the program, its output in the log
func (c *MainForm) Run() {
	c.Build(func(res *driver.BuildResult) { c.startProgram(res.Output) })
}

func (c *MainForm) startProgram(path string) {
	p := c.project
	if p == nil || path == "" {
		return
	}
	cmd := exec.Command(path, p.Args...)
	cmd.Dir = p.Dir
	w := &tailWriter{w: c.output.log}
	cmd.Stdout = w
	cmd.Stderr = w
	setProcessGroup(cmd)
	line := c.relPath(path)
	if len(p.Args) > 0 {
		line += " " + strings.Join(p.Args, " ")
	}
	c.output.println("== " + T().Running(line))
	if err := cmd.Start(); err != nil {
		c.output.println(T().CannotRun(err.Error()))
		c.status.SetText(T().CannotRun(err.Error()))
		return
	}
	start := time.Now()
	stopped := false
	c.runCmd = cmd
	c.runStop = func() {
		stopped = true
		killProcessGroup(cmd)
	}
	c.status.SetText(T().StatusRunning)
	go func() {
		cmd.Wait()
		w.finish()
		ui.Invoke(func() {
			c.runCmd, c.runStop = nil, nil
			msg := T().Exited(cmd.ProcessState.ExitCode(), formatDuration(time.Since(start)))
			if stopped {
				msg = T().Stopped
			}
			c.output.println("== " + msg)
			c.status.SetText(msg)
		})
	}()
}

// Stop cancels the build or kills the program
func (c *MainForm) Stop() {
	if c.buildCancel != nil {
		c.buildCancel()
	}
	if c.runStop != nil {
		c.runStop()
	}
}

// tailWriter passes the output of the program to the log and ends its last
// line when the program exits
type tailWriter struct {
	mu   sync.Mutex
	w    io.Writer
	last byte
}

func (t *tailWriter) Write(p []byte) (int, error) {
	t.mu.Lock()
	defer t.mu.Unlock()
	if len(p) > 0 {
		t.last = p[len(p)-1]
	}
	return t.w.Write(p)
}

func (t *tailWriter) finish() {
	t.mu.Lock()
	defer t.mu.Unlock()
	if t.last != 0 && t.last != '\n' {
		t.w.Write([]byte{'\n'})
	}
}
