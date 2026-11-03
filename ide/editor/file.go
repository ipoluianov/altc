package editor

import (
	"bytes"
	"errors"
	"os"
	"strings"
	"unicode/utf8"
)

// MaxFileSize is the largest file the editor opens
const MaxFileSize = 16 << 20

// ErrBinary tells the file is not text
var ErrBinary = errors.New("not a text file")

// ErrTooLarge tells the file is larger than MaxFileSize
var ErrTooLarge = errors.New("the file is too large")

// FileFormat is how a file was written: it is written back so
type FileFormat struct {
	CRLF bool // the lines end with "\r\n"
	BOM  bool // starts with the UTF-8 byte order mark
	Tabs *TabMemory
}

var bom = []byte{0xEF, 0xBB, 0xBF}

// Decode returns the text of a file as the editor shows it, and its format
func Decode(data []byte) (string, FileFormat, error) {
	var f FileFormat
	if len(data) > MaxFileSize {
		return "", f, ErrTooLarge
	}
	if bytes.HasPrefix(data, bom) {
		f.BOM = true
		data = data[len(bom):]
	}
	head := data[:min(len(data), 8000)]
	if bytes.IndexByte(head, 0) >= 0 || !utf8.Valid(head[:max(0, len(head)-4)]) {
		return "", f, ErrBinary
	}
	text := string(data)
	if strings.Contains(text, "\r\n") {
		f.CRLF = true
	}
	text = strings.ReplaceAll(text, "\r", "")
	text, f.Tabs = Untab(text)
	return text, f, nil
}

// Encode returns the bytes of the file of the text shown
func (f FileFormat) Encode(text string) []byte {
	text = f.Tabs.Retab(text)
	if f.CRLF {
		text = strings.ReplaceAll(text, "\n", "\r\n")
	}
	if f.BOM {
		return append(append([]byte{}, bom...), text...)
	}
	return []byte(text)
}

// ReadFile reads a file for the editor
func ReadFile(path string) (string, FileFormat, error) {
	st, err := os.Stat(path)
	if err != nil {
		return "", FileFormat{}, err
	}
	if st.Size() > MaxFileSize {
		return "", FileFormat{}, ErrTooLarge
	}
	data, err := os.ReadFile(path)
	if err != nil {
		return "", FileFormat{}, err
	}
	return Decode(data)
}

// WriteFile writes the text in the format and returns the format of what
// is in the file now
func WriteFile(path, text string, f FileFormat) (FileFormat, error) {
	data := f.Encode(text)
	perm := os.FileMode(0o644)
	if st, err := os.Stat(path); err == nil {
		perm = st.Mode().Perm()
	}
	if err := os.WriteFile(path, data, perm); err != nil {
		return f, err
	}
	_, nf, err := Decode(data)
	if err != nil {
		return f, nil
	}
	return nf, nil
}

// IsCFile tells whether the file is highlighted as C
func IsCFile(path string) bool {
	for _, ext := range []string{".c", ".h", ".inc", ".def"} {
		if strings.HasSuffix(strings.ToLower(path), ext) {
			return true
		}
	}
	return false
}
