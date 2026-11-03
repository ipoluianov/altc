package main

import (
	"os"

	"github.com/ipoluianov/altc/cli"
	"github.com/ipoluianov/altc/ide"
)

func main() {
	args := os.Args[1:]
	// No arguments, or "ide [dir]": the development environment
	if len(args) == 0 || args[0] == "ide" {
		dir := ""
		if len(args) > 1 {
			dir = args[1]
		}
		ide.Run(dir)
		return
	}
	os.Exit(cli.Main(args))
}
