//go:build unix

package forms

import (
	"os/exec"
	"syscall"
)

// setProcessGroup starts the program in a group of its own, so Stop kills
// its children too
func setProcessGroup(cmd *exec.Cmd) {
	cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
}

func killProcessGroup(cmd *exec.Cmd) {
	if cmd.Process == nil {
		return
	}
	if err := syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL); err != nil {
		cmd.Process.Kill()
	}
}
