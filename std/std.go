// Package std holds AltC's standard library for C programs: what Go's
// standard library offers Go programs, written in C with a classic C API
// (see PLAN.md, "Standard library"). Headers are under include/alt
// (<alt/net/http.h>), sources under src/<package>. The driver compiles the
// sources on first use into the build cache (libalt.a); programs link
// with it statically, and only the parts they use end up in them.
package std

import (
	"embed"
	"io/fs"
	"path"
	"sort"
	"strings"
)

//go:embed all:include all:src
var FS embed.FS

// Include is the header directory inside FS
const Include = "include"

// Sources are the C and assembler files of the library (paths in FS).
// Platform files carry the platform in their name (x_linux_amd64.c,
// x_linux.c, x_amd64.s); this build is for linux/amd64.
func Sources() []string {
	var out []string
	fs.WalkDir(FS, "src", func(p string, d fs.DirEntry, err error) error {
		if err != nil || d.IsDir() {
			return err
		}
		switch path.Ext(p) {
		case ".c", ".s", ".S":
		default:
			return nil
		}
		if !forPlatform(path.Base(p), "linux", "amd64") {
			return nil
		}
		out = append(out, p)
		return nil
	})
	sort.Strings(out)
	return out
}

var knownOS = map[string]bool{"linux": true, "windows": true, "darwin": true, "freebsd": true}
var knownArch = map[string]bool{"amd64": true, "arm64": true}

// forPlatform applies Go's file name rule: name_os_arch.ext, name_os.ext
// and name_arch.ext are only for that platform
func forPlatform(name, goos, goarch string) bool {
	parts := strings.Split(strings.TrimSuffix(name, path.Ext(name)), "_")
	n := len(parts)
	if n >= 3 && knownOS[parts[n-2]] && knownArch[parts[n-1]] {
		return parts[n-2] == goos && parts[n-1] == goarch
	}
	if n >= 2 && knownOS[parts[n-1]] {
		return parts[n-1] == goos
	}
	if n >= 2 && knownArch[parts[n-1]] {
		return parts[n-1] == goarch
	}
	return true
}
