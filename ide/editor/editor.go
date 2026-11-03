package editor

import (
	"strings"
	"time"
	"unicode"
	"unicode/utf8"

	"github.com/ipoluianov/nui/ui"
)

// TabSize is the indent of a level; Tab inserts spaces up to the next stop
const TabSize = 4

// Marker is a diagnostic shown in the editor: line and column from 0
type Marker struct {
	Line, Col int
	Warning   bool // an error otherwise
	Message   string
}

// Editor is a multiline text box set up for C: syntax highlighting, the
// line numbers, the current line, the diagnostics, the matches of a search,
// auto-indent. The box itself is TextBox, the widget to add to a layout.
type Editor struct {
	*ui.TextBox

	// OnModifiedChanged is called when the text gets different from the
	// saved one or the same again
	OnModifiedChanged func(modified bool)

	states    stateCache
	markers   map[int][]Marker // by line
	find      []rune           // the text searched, lowered unless findCase
	findCase  bool
	highlight bool // C highlighting; off for other files

	saved    string // the text as loaded or saved
	modified bool
	loading  bool
	recheck  *ui.Timer

	// buffers of the highlighter
	runes []rune
	toks  []Token
	fg    []Kind
	bg    []uint8
}

// The backgrounds of the highlighter, by priority
const (
	bgNone uint8 = iota
	bgFind
	bgWarning
	bgError
)

// New makes an empty editor
func New() *Editor {
	e := &Editor{TextBox: ui.NewTextBox(), highlight: true}
	tb := e.TextBox
	tb.SetMultiline(true)
	tb.SetFontFamily(ui.FontFamilyMono)
	tb.SetFontSize(14)
	tb.SetLineNumbers(true)
	tb.SetXExpandable(true)
	tb.SetYExpandable(true)
	tb.SetHighlighter(e.highlightLine)
	tb.SetOnTextChanged(e.textChanged)
	tb.SetOnPaint(e.paint)
	tb.OnKeyDown.Connect(e.keyDown)
	tb.OnChar.Connect(e.char)
	// Comparing the whole text with the saved one is put off while typing
	e.recheck = tb.NewTimer(400*time.Millisecond, e.checkModified)
	e.recheck.SetSingleShot(true)
	return e
}

// SetHighlighting turns the C highlighting on or off
func (e *Editor) SetHighlighting(on bool) {
	e.highlight = on
	e.TextBox.Update()
}

// Load shows the text as unmodified; the undo history starts anew
func (e *Editor) Load(text string) {
	e.loading = true
	e.TextBox.SetText(text)
	e.loading = false
	e.saved = e.TextBox.Text()
	e.states.invalidate()
	e.setModified(false)
}

// MarkSaved tells the text is what is in the file now
func (e *Editor) MarkSaved() {
	e.saved = e.TextBox.Text()
	e.setModified(false)
}

// Modified tells whether the text differs from the loaded or saved one
func (e *Editor) Modified() bool {
	return e.modified
}

func (e *Editor) textChanged() {
	e.states.invalidate()
	if e.loading {
		return
	}
	e.setModified(true)
	e.recheck.Start()
}

func (e *Editor) checkModified() {
	e.setModified(e.TextBox.Text() != e.saved)
}

func (e *Editor) setModified(m bool) {
	if e.modified == m {
		return
	}
	e.modified = m
	if e.OnModifiedChanged != nil {
		e.OnModifiedChanged(m)
	}
}

// SetMarkers shows the diagnostics of the file
func (e *Editor) SetMarkers(markers []Marker) {
	e.markers = nil
	for _, m := range markers {
		if e.markers == nil {
			e.markers = make(map[int][]Marker)
		}
		e.markers[m.Line] = append(e.markers[m.Line], m)
	}
	e.TextBox.Update()
}

// MarkersAt returns the diagnostics of the line
func (e *Editor) MarkersAt(line int) []Marker {
	return e.markers[line]
}

// SetFontSize changes the size of the text
func (e *Editor) SetFontSize(size float64) {
	e.TextBox.SetFontSize(size)
}

// GoTo puts the cursor at the line and the column (from 0) and scrolls to
// it, to the middle of the view when it was out of it
func (e *Editor) GoTo(line, col int) {
	tb := e.TextBox
	line = max(0, min(line, tb.LineCount()-1))
	col = max(0, min(col, utf8.RuneCountInString(tb.Line(line))))
	tb.SetCursorPosition(ui.TextPos{Line: line, Col: col})
	e.scrollToLine(line)
}

func (e *Editor) scrollToLine(line int) {
	tb := e.TextBox
	if tb.Form() == nil {
		return
	}
	lh := tb.OneLineHeight()
	y := 1 + line*lh
	vh := tb.ViewportHeight()
	if y < tb.ScrollY() || y+lh > tb.ScrollY()+vh {
		tb.SetScrollY(max(0, y-vh/2))
	}
	tb.ScrollToCursor()
}

// SetFind highlights the matches of the text ("" - none)
func (e *Editor) SetFind(text string, matchCase bool) {
	e.find = foldRunes([]rune(text), matchCase)
	e.findCase = matchCase
	e.TextBox.Update()
}

func foldRunes(rs []rune, matchCase bool) []rune {
	if matchCase {
		return rs
	}
	out := make([]rune, len(rs))
	for i, r := range rs {
		out[i] = unicode.ToLower(r)
	}
	return out
}

// indexRunes returns the index of p in s from the index from, -1 if none
func indexRunes(s, p []rune, from int) int {
	if len(p) == 0 {
		return -1
	}
	for i := max(0, from); i+len(p) <= len(s); i++ {
		if s[i] == p[0] && equalRunes(s[i:i+len(p)], p) {
			return i
		}
	}
	return -1
}

// lastIndexRunes returns the last index of p in s that ends before the index before
func lastIndexRunes(s, p []rune, before int) int {
	if len(p) == 0 {
		return -1
	}
	for i := min(before, len(s)) - len(p); i >= 0; i-- {
		if s[i] == p[0] && equalRunes(s[i:i+len(p)], p) {
			return i
		}
	}
	return -1
}

func equalRunes(a, b []rune) bool {
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}

// FindNext selects the next match of the text after the cursor (before it
// when backward), going around the end; false if there is none. Text in
// several lines is not found.
func (e *Editor) FindNext(text string, matchCase, backward bool) bool {
	e.SetFind(text, matchCase)
	p := e.find
	if len(p) == 0 {
		return false
	}
	tb := e.TextBox
	from, to := tb.Selection()
	n := tb.LineCount()
	for k := 0; k <= n; k++ {
		var line int
		if backward {
			line = ((from.Line-k)%n + n) % n
		} else {
			line = (to.Line + k) % n
		}
		s := foldRunes([]rune(tb.Line(line)), matchCase)
		idx := -1
		switch {
		case backward && k == 0:
			idx = lastIndexRunes(s, p, from.Col)
		case backward && k == n:
			idx = lastIndexRunes(s, p, len(s))
			if idx >= 0 && line == from.Line && idx < from.Col {
				idx = -1
			}
		case backward:
			idx = lastIndexRunes(s, p, len(s))
		case k == 0:
			idx = indexRunes(s, p, to.Col)
		case k == n:
			idx = indexRunes(s, p, 0)
			if idx >= to.Col {
				idx = -1
			}
		default:
			idx = indexRunes(s, p, 0)
		}
		if idx >= 0 {
			tb.SetSelection(ui.TextPos{Line: line, Col: idx}, ui.TextPos{Line: line, Col: idx + len(p)})
			e.scrollToLine(line)
			return true
		}
	}
	return false
}

// CountMatches returns how many times the searched text is in the editor
func (e *Editor) CountMatches() int {
	if len(e.find) == 0 {
		return 0
	}
	tb := e.TextBox
	count := 0
	for i := 0; i < tb.LineCount(); i++ {
		s := foldRunes([]rune(tb.Line(i)), e.findCase)
		for j := indexRunes(s, e.find, 0); j >= 0; j = indexRunes(s, e.find, j+len(e.find)) {
			count++
		}
	}
	return count
}

// highlightLine gives the TextBox the styled parts of a line
func (e *Editor) highlightLine(line int, text string) []ui.TextSpan {
	ms := e.markers[line]
	if !e.highlight && len(e.find) == 0 && len(ms) == 0 {
		return nil
	}
	e.runes = append(e.runes[:0], []rune(text)...)
	rs := e.runes
	n := len(rs)
	if n == 0 {
		return nil
	}
	e.fg = resize(e.fg, n)
	e.bg = resize(e.bg, n)
	fg, bg := e.fg, e.bg
	if e.highlight {
		st := e.states.startState(e.TextBox, line)
		e.toks, _ = LexRunes(rs, st, e.toks[:0])
		for _, t := range e.toks {
			for i := t.Start; i < t.End; i++ {
				fg[i] = t.Kind
			}
		}
	}
	if len(e.find) > 0 {
		s := foldRunes(rs, e.findCase)
		for j := indexRunes(s, e.find, 0); j >= 0; j = indexRunes(s, e.find, j+len(e.find)) {
			for i := j; i < j+len(e.find); i++ {
				bg[i] = bgFind
			}
		}
	}
	for _, m := range ms {
		s, end := tokenAt(rs, m.Col)
		mark := bgError
		if m.Warning {
			mark = bgWarning
		}
		for i := s; i < end; i++ {
			bg[i] = max(bg[i], mark)
		}
	}

	c := CurrentColors()
	var spans []ui.TextSpan
	for start := 0; start < n; {
		end := start + 1
		for end < n && fg[end] == fg[start] && bg[end] == bg[start] {
			end++
		}
		if fg[start] != KindPlain || bg[start] != bgNone {
			span := ui.TextSpan{Start: start, End: end}
			if fg[start] != KindPlain {
				span.Color = c.Tokens[fg[start]]
			}
			switch bg[start] {
			case bgFind:
				span.Background = c.FindMatch
			case bgWarning:
				span.Background = c.WarningToken
			case bgError:
				span.Background = c.ErrorToken
			}
			spans = append(spans, span)
		}
		start = end
	}
	return spans
}

func resize[T Kind | uint8](s []T, n int) []T {
	if cap(s) < n {
		s = make([]T, n)
	}
	s = s[:n]
	clear(s)
	return s
}

// tokenAt returns the runes a diagnostic at the column marks: the word or
// the number there, or one character; the last one past the end
func tokenAt(rs []rune, col int) (int, int) {
	n := len(rs)
	if n == 0 {
		return 0, 0
	}
	if col >= n {
		return n - 1, n
	}
	col = max(0, col)
	if !isIdentChar(rs[col]) {
		return col, col + 1
	}
	end := col
	for end < n && isIdentChar(rs[end]) {
		end++
	}
	return col, end
}

// paint draws the current line and the lines of the diagnostics under the
// text, and the marks of the diagnostics in the gutter over it
func (e *Editor) paint(cnv *ui.Canvas) {
	tb := e.TextBox
	c := CurrentColors()
	lh := tb.OneLineHeight()
	sx, sy := tb.ScrollX(), tb.ScrollY()
	vw, vh := tb.ViewportWidth(), tb.ViewportHeight()
	first, last := sy/max(1, lh)-1, (sy+vh)/max(1, lh)+1
	lineY := func(line int) int { return 1 + line*lh }

	for line, ms := range e.markers {
		if line >= first && line <= last {
			col := c.ErrorLine
			if !hasError(ms) {
				col = c.WarningLine
			}
			cnv.FillRect(sx, lineY(line), vw, lh, col)
		}
	}
	if !tb.HasSelection() && tb.IsFocused() {
		cnv.FillRect(sx, lineY(tb.CursorPosition().Line), vw, lh, c.CurrentLine)
	}
	tb.Draw(cnv, tb.InnerWidth(), tb.InnerHeight())
	for line, ms := range e.markers {
		if line >= first && line <= last {
			col := c.ErrorMark
			if !hasError(ms) {
				col = c.WarningMark
			}
			cnv.FillRect(sx, lineY(line), 4, lh, col)
		}
	}
}

func hasError(ms []Marker) bool {
	for _, m := range ms {
		if !m.Warning {
			return true
		}
	}
	return false
}

// Editing keys

func noMods(m ui.KeyModifiers) bool {
	return !m.Shift && !m.Ctrl && !m.Alt && !m.Cmd
}

func (e *Editor) keyDown(ev *ui.KeyEvent) {
	tb := e.TextBox
	if tb.ReadOnly() {
		return
	}
	m := ev.Mods
	switch {
	case ev.Key == ui.KeyTab && noMods(m):
		from, to := tb.Selection()
		if from.Line != to.Line {
			e.indentLines(1)
		} else {
			n := TabSize - from.Col%TabSize
			tb.InsertText(strings.Repeat(" ", n))
		}
		ev.Handled = true
	case ev.Key == ui.KeyTab && m.Shift && !m.Ctrl && !m.Alt && !m.Cmd:
		e.indentLines(-1)
		ev.Handled = true
	case ev.Key == ui.KeyEnter && (noMods(m) || (m.Shift && !m.Ctrl && !m.Alt && !m.Cmd)):
		e.newLine()
		ev.Handled = true
	case ev.Key == ui.KeyHome && !m.Ctrl && !m.Alt && !m.Cmd:
		e.smartHome(m.Shift)
		ev.Handled = true
	case ev.Key == ui.KeyBackspace && noMods(m) && !tb.HasSelection():
		// In the indent Backspace goes back to the previous stop
		cur := tb.CursorPosition()
		rs := []rune(tb.Line(cur.Line))
		if cur.Col >= 2 && allSpaces(rs[:cur.Col]) {
			n := cur.Col % TabSize
			if n == 0 {
				n = TabSize
			}
			tb.SetSelection(ui.TextPos{Line: cur.Line, Col: cur.Col - n}, cur)
			tb.InsertText("")
			ev.Handled = true
		}
	}
}

func (e *Editor) char(ev *ui.CharEvent) {
	tb := e.TextBox
	if tb.ReadOnly() || ev.Char != '}' || tb.HasSelection() {
		return
	}
	// A closing brace alone on its line goes back a level
	cur := tb.CursorPosition()
	rs := []rune(tb.Line(cur.Line))
	if cur.Col == 0 || !allSpaces(rs[:cur.Col]) {
		return
	}
	indent := max(0, (cur.Col-1)/TabSize*TabSize)
	tb.SetSelection(ui.TextPos{Line: cur.Line, Col: 0}, cur)
	tb.InsertText(strings.Repeat(" ", indent) + "}")
	ev.Handled = true
}

func allSpaces(rs []rune) bool {
	for _, r := range rs {
		if r != ' ' && r != '\t' {
			return false
		}
	}
	return true
}

func leadingSpaces(rs []rune) int {
	n := 0
	for n < len(rs) && (rs[n] == ' ' || rs[n] == '\t') {
		n++
	}
	return n
}

// newLine breaks the line at the cursor, keeping its indent; after "{" the
// new line goes a level deeper
func (e *Editor) newLine() {
	tb := e.TextBox
	from, to := tb.Selection()
	rs := []rune(tb.Line(from.Line))
	before := rs[:min(from.Col, len(rs))]
	indent := string(before[:leadingSpaces(before)])
	trimmed := strings.TrimRight(string(before), " \t")
	if !allSpaces(before) {
		// Spaces left at the end of the line are dropped
		if n := utf8.RuneCountInString(trimmed); n < from.Col {
			tb.SetSelection(ui.TextPos{Line: from.Line, Col: n}, to)
		}
	}
	afterRs := []rune(tb.Line(to.Line))
	after := strings.TrimLeft(string(afterRs[min(to.Col, len(afterRs)):]), " \t")
	if strings.HasSuffix(trimmed, "{") {
		inner := indent + strings.Repeat(" ", TabSize)
		if strings.HasPrefix(after, "}") {
			// Between the braces: the cursor on a line of its own
			sel, _ := tb.Selection()
			tb.SetSelection(sel, positionAfterSpaces(tb, to))
			tb.InsertText("\n" + inner + "\n" + indent)
			line := sel.Line + 1
			tb.SetCursorPosition(ui.TextPos{Line: line, Col: utf8.RuneCountInString(inner)})
			e.scrollToLine(line)
			return
		}
		indent = inner
	}
	if after != "" && len(afterRs) > to.Col {
		// The text moved to the new line starts at the indent
		sel, _ := tb.Selection()
		tb.SetSelection(sel, positionAfterSpaces(tb, to))
	}
	tb.InsertText("\n" + indent)
	tb.ScrollToCursor()
}

// positionAfterSpaces returns the position after the spaces from p in its line
func positionAfterSpaces(tb *ui.TextBox, p ui.TextPos) ui.TextPos {
	rs := []rune(tb.Line(p.Line))
	for p.Col < len(rs) && (rs[p.Col] == ' ' || rs[p.Col] == '\t') {
		p.Col++
	}
	return p
}

// smartHome goes to the first character of the line that is not a space,
// or to the start of the line when it is there
func (e *Editor) smartHome(extend bool) {
	tb := e.TextBox
	cur := tb.CursorPosition()
	rs := []rune(tb.Line(cur.Line))
	target := leadingSpaces(rs)
	if cur.Col == target {
		target = 0
	}
	pos := ui.TextPos{Line: cur.Line, Col: target}
	if !extend {
		tb.SetCursorPosition(pos)
		return
	}
	anchor := cur
	if tb.HasSelection() {
		from, to := tb.Selection()
		anchor = from
		if from == cur {
			anchor = to
		}
	}
	tb.SetSelection(anchor, pos)
}

// indentLines moves the lines of the selection (or the cursor's) a level
// right (dir > 0) or left
func (e *Editor) indentLines(dir int) {
	tb := e.TextBox
	from, to := tb.Selection()
	cur := tb.CursorPosition()
	first, last := from.Line, to.Line
	if last > first && to.Col == 0 {
		last--
	}
	var b strings.Builder
	changed := false
	shift := make([]int, last-first+1) // how the start of each line moved
	for i := first; i <= last; i++ {
		line := tb.Line(i)
		if i > first {
			b.WriteByte('\n')
		}
		rs := []rune(line)
		if dir > 0 {
			if len(strings.TrimSpace(line)) > 0 || first == last {
				b.WriteString(strings.Repeat(" ", TabSize))
				shift[i-first] = TabSize
				changed = true
			}
			b.WriteString(line)
			continue
		}
		n := 0
		for n < TabSize && n < len(rs) && rs[n] == ' ' {
			n++
		}
		if n == 0 && len(rs) > 0 && rs[0] == '\t' {
			n = 1
		}
		if n > 0 {
			changed = true
		}
		shift[i-first] = -n
		b.WriteString(string(rs[n:]))
	}
	if !changed {
		return
	}
	lastLen := utf8.RuneCountInString(tb.Line(last))
	tb.SetSelection(ui.TextPos{Line: first, Col: 0}, ui.TextPos{Line: last, Col: lastLen})
	tb.InsertText(b.String())
	// The selection (or the cursor) stays on the same text
	moved := func(p ui.TextPos) ui.TextPos {
		if p.Line < first || p.Line > last {
			return p
		}
		p.Col = max(0, p.Col+shift[p.Line-first])
		return p
	}
	if from == to {
		tb.SetCursorPosition(moved(cur))
		return
	}
	anchor := from
	if cur == from {
		anchor = to
	}
	tb.SetSelection(moved(anchor), moved(cur))
}
