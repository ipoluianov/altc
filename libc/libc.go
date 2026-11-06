// Package libc holds the C library built into the compiler: the musl
// sources (MIT license, see musl/COPYRIGHT) for x86_64, vendored by
// ./vendor with the patches in patches/, and AltC's additions in altc/
// (what musl does not have, compiled like programs are). The driver
// compiles them on first use into the build cache; programs link with the
// result statically and need nothing on the system but the kernel.
package libc

import (
	"embed"
	"io/fs"
	"path"
	"sort"
	"strings"
)

//go:embed all:musl all:altc
var FS embed.FS

const (
	// Root is the musl tree inside FS
	Root = "musl"
	// Include holds the public headers (<stdio.h>...) inside FS
	Include = "musl/include"
	// Arch is the architecture directory name of the sources
	Arch = "x86_64"
)

// Version is the musl release
func Version() string {
	data, _ := FS.ReadFile(Root + "/VERSION")
	return strings.TrimSpace(string(data))
}

// InternalDirs are the include directories of the library's own sources,
// before Include, in the order of musl's Makefile
var InternalDirs = []string{Root + "/arch/" + Arch, Root + "/arch/generic", Root + "/src/include", Root + "/src/internal"}

// Defines are the macros the library is compiled with
var Defines = []string{"_XOPEN_SOURCE=700"}

// Sources are the files of libc.a (paths inside FS), as musl's Makefile
// selects them: the .c files of every src/* directory and of
// src/malloc/mallocng, an architecture file (.c, .s, .S in the x86_64
// subdirectory) replacing the generic file of the same name
func Sources() []string {
	dirs := []string{Root + "/src/malloc/mallocng"}
	ents, _ := fs.ReadDir(FS, Root+"/src")
	for _, e := range ents {
		if e.IsDir() {
			dirs = append(dirs, Root+"/src/"+e.Name())
		}
	}
	var out []string
	for _, d := range dirs {
		archFiles := map[string]string{}
		if ents, err := fs.ReadDir(FS, d+"/"+Arch); err == nil {
			for _, e := range ents {
				switch path.Ext(e.Name()) {
				case ".c", ".s", ".S":
					archFiles[strings.TrimSuffix(e.Name(), path.Ext(e.Name()))] = d + "/" + Arch + "/" + e.Name()
				}
			}
		}
		ents, _ := fs.ReadDir(FS, d)
		for _, e := range ents {
			if e.IsDir() || path.Ext(e.Name()) != ".c" {
				continue
			}
			if _, replaced := archFiles[strings.TrimSuffix(e.Name(), ".c")]; !replaced {
				out = append(out, d+"/"+e.Name())
			}
		}
		for _, f := range archFiles {
			out = append(out, f)
		}
	}
	sort.Strings(out)
	return out
}

// Extra are AltC's additions to the library (paths inside FS), compiled
// with the headers programs see
func Extra() []string {
	ents, _ := fs.ReadDir(FS, "altc")
	var out []string
	for _, e := range ents {
		if path.Ext(e.Name()) == ".c" {
			out = append(out, "altc/"+e.Name())
		}
	}
	return out
}

// StartFiles are the startup objects of a static executable, in link
// order: crt1 (_start), crti and AltC's start go first, crtn last
var StartFiles = struct{ Crt1, Crti, Start, Crtn string }{
	Root + "/crt/crt1.c", Root + "/crt/" + Arch + "/crti.s", "altc/crt/start.c", Root + "/crt/" + Arch + "/crtn.s",
}
