package editor

import "testing"

func TestUntabRetab(t *testing.T) {
	file := "int f() {\n\tif (x)\n\t\treturn 1;\n\treturn 0; \t// end\n}"
	shown, m := Untab(file)
	want := "int f() {\n    if (x)\n        return 1;\n    return 0;   // end\n}"
	if shown != want {
		t.Fatalf("untab:\n%q\nwant\n%q", shown, want)
	}
	if got := m.Retab(shown); got != file {
		t.Fatalf("retab unchanged:\n%q", got)
	}
	// A new line is indented with tabs as the file is
	edited := shown + "\n        x = 1;\n  y;"
	if got := m.Retab(edited); got != file+"\n\t\tx = 1;\n  y;" {
		t.Fatalf("retab edited:\n%q", got)
	}
	// Without tabs nothing changes
	if s, m := Untab("a\n    b"); s != "a\n    b" || m != nil || m.Retab(s) != s {
		t.Fatal("text without tabs changed")
	}
	// A file indented with spaces keeps them for the new lines
	_, m = Untab("    a;\n    b;\nx\t= 1;")
	if got := m.Retab("    a;\n        c;\nx   = 1;"); got != "    a;\n        c;\nx\t= 1;" {
		t.Fatalf("spaces file: %q", got)
	}
}

func TestVisualCol(t *testing.T) {
	tests := []struct {
		line string
		off  int
		want int
	}{
		{"abc", 2, 2},
		{"\tx", 1, 4},
		{"a\tx", 2, 4},
		{"\t\tx", 2, 8},
		{"пр x", 5, 3}, // two letters of two bytes, a space
		{"ab", 5, 5},
	}
	for _, tt := range tests {
		if got := VisualCol(tt.line, tt.off); got != tt.want {
			t.Errorf("VisualCol(%q, %d) = %d, want %d", tt.line, tt.off, got, tt.want)
		}
	}
}
