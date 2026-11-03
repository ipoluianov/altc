package editor

// lineSource gives the lines of a text
type lineSource interface {
	LineCount() int
	Line(i int) string
}

// stateCache keeps the lexer state at the start of each line, so a line is
// highlighted without lexing the text above it again. An entry is checked
// against the text it was made for: after an edit only the lines that
// changed (and those whose start state changed) are lexed again. Equal
// strings shared with the text box compare by the pointer, so checking
// thousands of unchanged lines is cheap.
type stateCache struct {
	texts []string // the text of the line the entry was made for
	start []State
	end   []State
	// checked is how many entries from the top are known to be right for
	// the text of version
	checked int
	version int64
	toks    []Token // a buffer for the lexer
	runes   []rune
}

// invalidate tells the text changed: the entries are checked again
func (c *stateCache) invalidate() {
	c.version++
	c.checked = 0
}

// startState returns the state line starts in
func (c *stateCache) startState(src lineSource, line int) State {
	n := src.LineCount()
	if line <= 0 || n == 0 {
		return StateNormal
	}
	line = min(line, n)
	if len(c.texts) > n {
		c.texts, c.start, c.end = c.texts[:n], c.start[:n], c.end[:n]
		c.checked = min(c.checked, n)
	}
	for j := c.checked; j < line; j++ {
		text := src.Line(j)
		st := StateNormal
		if j > 0 {
			st = c.end[j-1]
		}
		if j < len(c.texts) && c.start[j] == st && c.texts[j] == text {
			continue
		}
		c.runes = append(c.runes[:0], []rune(text)...)
		var end State
		c.toks, end = LexRunes(c.runes, st, c.toks[:0])
		if j < len(c.texts) {
			c.texts[j], c.start[j], c.end[j] = text, st, end
		} else {
			c.texts = append(c.texts, text)
			c.start = append(c.start, st)
			c.end = append(c.end, end)
		}
	}
	c.checked = max(c.checked, line)
	return c.end[line-1]
}
