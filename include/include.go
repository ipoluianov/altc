// Package include holds the C standard headers built into the compiler.
// They are glibc-compatible for linux/amd64, so programs link against the
// system libc.so.6 without needing /usr/include.
package include

import "embed"

//go:embed all:c
var FS embed.FS

// Root is the directory inside FS that is searched for <...> headers
const Root = "c"
