package editor

import (
	"fmt"
	"strings"
	"testing"
)

// show prints the tokens of a line as "kind:text" pairs
func show(line string, toks []Token) string {
	rs := []rune(line)
	var parts []string
	for _, t := range toks {
		parts = append(parts, fmt.Sprintf("%s:%s", t.Kind, string(rs[t.Start:t.End])))
	}
	return strings.Join(parts, " ")
}

func TestLex(t *testing.T) {
	tests := []struct {
		line  string
		start State
		want  string
		end   State
	}{
		{`int main(void) {`, StateNormal, `type:int type:void`, StateNormal},
		{`  return 0x1Fu + 1.5e-3f; // done`, StateNormal, `keyword:return number:0x1Fu number:1.5e-3f comment:// done`, StateNormal},
		{`#include <stdio.h>`, StateNormal, `preproc:#include string:<stdio.h>`, StateNormal},
		{`  # define X(a) "a\"b" 'c'`, StateNormal, `preproc:# define string:"a\"b" char:'c'`, StateNormal},
		{`#if defined(X) && Y`, StateNormal, `preproc:#if preproc:defined`, StateNormal},
		{`#define LONG 1 \`, StateNormal, `preproc:#define number:1`, StatePreproc},
		{`   + 2`, StatePreproc, `number:2`, StateNormal},
		{`x = 1; /* start`, StateNormal, `number:1 comment:/* start`, StateBlockComment},
		{`still comment`, StateBlockComment, `comment:still comment`, StateBlockComment},
		{`end */ static int y;`, StateBlockComment, `comment:end */ keyword:static type:int`, StateNormal},
		{`a /* x */ b /* y */`, StateNormal, `comment:/* x */ comment:/* y */`, StateNormal},
		{`s = L"wide" u8"x" U'c';`, StateNormal, `string:L"wide" string:u8"x" char:U'c'`, StateNormal},
		{`"unterminated`, StateNormal, `string:"unterminated`, StateNormal},
		{`"go on \`, StateNormal, `string:"go on \`, StateString},
		{`here" int`, StateString, `string:here" type:int`, StateNormal},
		{`// line \`, StateNormal, `comment:// line \`, StateLineComment},
		{`continued`, StateLineComment, `comment:continued`, StateNormal},
		{`size_t n = 1'000'000; bool b = true;`, StateNormal, `type:size_t number:1'000'000 type:bool keyword:true`, StateNormal},
		{`x.y = .5;`, StateNormal, `number:.5`, StateNormal},
		{`идент = 1;`, StateNormal, `number:1`, StateNormal},
		{``, StateBlockComment, ``, StateBlockComment},
	}
	for _, tt := range tests {
		toks, end := Lex(tt.line, tt.start)
		if got := show(tt.line, toks); got != tt.want || end != tt.end {
			t.Errorf("Lex(%q, %d) = %q, %d; want %q, %d", tt.line, tt.start, got, end, tt.want, tt.end)
		}
	}
}

type lines []string

func (l lines) LineCount() int    { return len(l) }
func (l lines) Line(i int) string { return l[i] }

func TestStateCache(t *testing.T) {
	text := lines{"int a;", "/* one", "two", "three */", "int b;", "/* x */"}
	var c stateCache
	want := []State{StateNormal, StateNormal, StateBlockComment, StateBlockComment, StateNormal, StateNormal}
	for i := len(text) - 1; i >= 0; i-- {
		if got := c.startState(text, i); got != want[i] {
			t.Errorf("line %d: %d, want %d", i, got, want[i])
		}
	}
	// The comment closed earlier: the lines after it change their state
	text[1] = "/* one */"
	c.invalidate()
	if got := c.startState(text, 3); got != StateNormal {
		t.Errorf("after the edit: line 3 starts in %d", got)
	}
	// Lines removed
	text = text[:2]
	c.invalidate()
	if got := c.startState(text, 5); got != StateNormal {
		t.Errorf("past the end: %d", got)
	}
}

func BenchmarkStateCache10k(b *testing.B) {
	var text lines
	for i := 0; i < 10000; i++ {
		text = append(text, fmt.Sprintf("    int value%d = compute(%d, \"text\"); /* comment */", i, i))
	}
	var c stateCache
	c.startState(text, 9999)
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		// An edit of the first line: the rest is checked, not lexed
		text[0] = fmt.Sprint("int x", i, ";")
		c.invalidate()
		c.startState(text, 9999)
	}
}
