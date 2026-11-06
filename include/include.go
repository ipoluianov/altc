// Package include holds the headers that come with the compiler rather
// than with the C library (package libc): intrinsics, <stdatomic.h>,
// <stdckdint.h>, <stdbit.h>, <cpuid.h>. They are searched before the C
// library's headers.
package include

import "embed"

//go:embed all:c
var FS embed.FS

// Root is the directory inside FS that is searched for <...> headers
const Root = "c"
