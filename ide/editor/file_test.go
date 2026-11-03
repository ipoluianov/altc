package editor

import (
	"os"
	"path/filepath"
	"testing"
)

func TestFileRoundTrip(t *testing.T) {
	dir := t.TempDir()
	for _, content := range []string{
		"int x;\n",
		"int x;\r\nint y;\r\n",
		"\xEF\xBB\xBFint x;\n",
		"void f() {\n\treturn;\n}\n",
		"",
	} {
		path := filepath.Join(dir, "a.c")
		os.WriteFile(path, []byte(content), 0o644)
		text, f, err := ReadFile(path)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := WriteFile(path, text, f); err != nil {
			t.Fatal(err)
		}
		got, _ := os.ReadFile(path)
		if string(got) != content {
			t.Errorf("round trip of %q gave %q", content, got)
		}
	}
	if _, _, err := Decode([]byte("ab\x00cd")); err != ErrBinary {
		t.Errorf("binary: %v", err)
	}
}
