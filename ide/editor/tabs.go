package editor

import "strings"

// The editor shows a tab as spaces up to the next stop of TabSize. A file
// with tabs keeps them: TabMemory remembers its lines, so the lines the
// user didn't change are written back as they were, and the new ones are
// indented with tabs when the file was.

// TabMemory is what Untab learned about the tabs of a text
type TabMemory struct {
	orig    map[string]string // a line with tabs by the line shown
	useTabs bool              // most indented lines start with a tab
}

// ExpandLine replaces the tabs of a line with spaces up to the next stop
func ExpandLine(line string) string {
	if !strings.ContainsRune(line, '\t') {
		return line
	}
	var b strings.Builder
	col := 0
	for _, r := range line {
		if r == '\t' {
			n := TabSize - col%TabSize
			b.WriteString(strings.Repeat(" ", n))
			col += n
			continue
		}
		b.WriteRune(r)
		col++
	}
	return b.String()
}

// Untab returns the text with the tabs replaced by spaces, and the memory
// to put them back (nil when there are none)
func Untab(text string) (string, *TabMemory) {
	if !strings.ContainsRune(text, '\t') {
		return text, nil
	}
	m := &TabMemory{orig: make(map[string]string)}
	lines := strings.Split(text, "\n")
	tabbed, spaced := 0, 0
	for i, line := range lines {
		switch {
		case strings.HasPrefix(line, "\t"):
			tabbed++
		case strings.HasPrefix(line, " "):
			spaced++
		}
		if strings.ContainsRune(line, '\t') {
			e := ExpandLine(line)
			m.orig[e] = line
			lines[i] = e
		}
	}
	m.useTabs = tabbed > spaced
	return strings.Join(lines, "\n"), m
}

// Retab returns the text to write: the lines that were in the file as they
// were, the other ones indented with tabs when the file was
func (m *TabMemory) Retab(text string) string {
	if m == nil {
		return text
	}
	lines := strings.Split(text, "\n")
	for i, line := range lines {
		if orig, ok := m.orig[line]; ok {
			lines[i] = orig
			continue
		}
		if m.useTabs {
			n := 0
			for n < len(line) && line[n] == ' ' {
				n++
			}
			if tabs := n / TabSize; tabs > 0 {
				lines[i] = strings.Repeat("\t", tabs) + line[tabs*TabSize:]
			}
		}
	}
	return strings.Join(lines, "\n")
}

// VisualCol returns the column (from 0) the editor shows the byte offset
// of the line of the file at: the tabs go to their stops, a character of
// several bytes is one column
func VisualCol(line string, byteOffset int) int {
	col := 0
	for i, r := range line {
		if i >= byteOffset {
			break
		}
		if r == '\t' {
			col += TabSize - col%TabSize
		} else {
			col++
		}
	}
	if byteOffset > len(line) {
		col += byteOffset - len(line)
	}
	return col
}
