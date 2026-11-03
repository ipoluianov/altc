// Package editor is the code editor of the IDE: a multiline text box with
// C syntax highlighting, the current line, markers of the diagnostics,
// search, auto-indent
package editor

import "strings"

// Kind is the kind of a highlighted token
type Kind uint8

const (
	KindPlain Kind = iota
	KindKeyword
	KindType
	KindPreproc
	KindString
	KindChar
	KindComment
	KindNumber
)

func (k Kind) String() string {
	return [...]string{"plain", "keyword", "type", "preproc", "string", "char", "comment", "number"}[k]
}

// State is the lexer state at the start of a line: what a previous line
// left open
type State uint8

const (
	StateNormal       State = iota
	StateBlockComment       // inside /* ... */
	StateLineComment        // a // comment continued by a backslash
	StatePreproc            // a directive continued by a backslash
	StateString             // a string continued by a backslash
)

// Token is a highlighted part of a line: the runes [Start, End)
type Token struct {
	Start, End int
	Kind       Kind
}

var keywords = setOf(`auto break case const continue default do else enum extern for goto if inline
	register restrict return sizeof static struct switch typedef union volatile while
	_Alignas _Alignof _Atomic _Generic _Noreturn _Static_assert _Thread_local
	alignas alignof constexpr false true nullptr static_assert thread_local typeof typeof_unqual
	__attribute__ __asm__ asm __typeof__ __inline __inline__ __restrict __restrict__ __extension__
	__volatile__ __const __signed__ __builtin_va_arg __builtin_offsetof __label__`)

var types = setOf(`void char short int long float double signed unsigned bool _Bool _Complex _Imaginary
	_BitInt _Decimal32 _Decimal64 _Decimal128 __int128 __int128_t __uint128_t
	size_t ssize_t ptrdiff_t intptr_t uintptr_t intmax_t uintmax_t off_t wchar_t wint_t
	char8_t char16_t char32_t max_align_t nullptr_t
	int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t uint64_t
	int_least8_t int_least16_t int_least32_t int_least64_t
	uint_least8_t uint_least16_t uint_least32_t uint_least64_t
	int_fast8_t int_fast16_t int_fast32_t int_fast64_t
	uint_fast8_t uint_fast16_t uint_fast32_t uint_fast64_t
	FILE va_list __builtin_va_list time_t clock_t pid_t jmp_buf sig_atomic_t fpos_t div_t ldiv_t lldiv_t
	mbstate_t locale_t pthread_t pthread_mutex_t`)

func setOf(s string) map[string]bool {
	m := make(map[string]bool)
	for _, w := range strings.Fields(s) {
		m[w] = true
	}
	return m
}

func isIdentStart(r rune) bool {
	return r == '_' || r == '$' || (r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z') || r >= 0x80
}

func isIdentChar(r rune) bool {
	return isIdentStart(r) || (r >= '0' && r <= '9')
}

func isDigit(r rune) bool { return r >= '0' && r <= '9' }

// endsWithBackslash tells whether the line goes on in the next one
func endsWithBackslash(rs []rune) bool {
	n := len(rs)
	for n > 0 && (rs[n-1] == ' ' || rs[n-1] == '\t' || rs[n-1] == '\r') {
		n--
	}
	return n > 0 && rs[n-1] == '\\'
}

// Lex splits a line of C into the highlighted tokens (the plain text is
// left out) and returns the state the next line starts in
func Lex(line string, start State) ([]Token, State) {
	return LexRunes([]rune(line), start, nil)
}

// LexRunes is Lex for a line of runes, appending the tokens to toks
func LexRunes(rs []rune, start State, toks []Token) ([]Token, State) {
	n := len(rs)
	i := 0
	add := func(s, e int, k Kind) {
		if e > s {
			toks = append(toks, Token{s, e, k})
		}
	}
	cont := endsWithBackslash(rs)
	preproc := false

	switch start {
	case StateBlockComment:
		j := closeComment(rs, 0)
		if j < 0 {
			add(0, n, KindComment)
			return toks, StateBlockComment
		}
		add(0, j, KindComment)
		i = j
	case StateLineComment:
		add(0, n, KindComment)
		if cont {
			return toks, StateLineComment
		}
		return toks, StateNormal
	case StateString:
		j, closed := closeQuote(rs, 0, '"')
		add(0, j, KindString)
		if !closed {
			if cont {
				return toks, StateString
			}
			return toks, StateNormal
		}
		i = j
	case StatePreproc:
		preproc = true
	}

	// A directive: # and its name
	if start != StatePreproc && start != StateBlockComment {
		j := i
		for j < n && (rs[j] == ' ' || rs[j] == '\t') {
			j++
		}
		if j < n && rs[j] == '#' {
			preproc = true
			k := j + 1
			for k < n && (rs[k] == ' ' || rs[k] == '\t') {
				k++
			}
			e := k
			for e < n && isIdentChar(rs[e]) {
				e++
			}
			add(j, e, KindPreproc)
			name := string(rs[k:e])
			i = e
			// The header of #include <...> is a string
			if name == "include" || name == "include_next" || name == "import" || name == "embed" {
				h := i
				for h < n && (rs[h] == ' ' || rs[h] == '\t') {
					h++
				}
				if h < n && rs[h] == '<' {
					c := h + 1
					for c < n && rs[c] != '>' {
						c++
					}
					if c < n {
						c++
					}
					add(h, c, KindString)
					i = c
				}
			}
		}
	}

	for i < n {
		r := rs[i]
		switch {
		case r == '/' && i+1 < n && rs[i+1] == '/':
			add(i, n, KindComment)
			if cont {
				return toks, StateLineComment
			}
			return toks, StateNormal
		case r == '/' && i+1 < n && rs[i+1] == '*':
			j := closeComment(rs, i+2)
			if j < 0 {
				add(i, n, KindComment)
				return toks, StateBlockComment
			}
			add(i, j, KindComment)
			i = j
		case r == '"' || r == '\'':
			// A prefix of the literal (L, u, U, u8) is part of it
			s := i
			j, closed := closeQuote(rs, i+1, r)
			kind := KindString
			if r == '\'' {
				kind = KindChar
			}
			add(s, j, kind)
			i = j
			if !closed && r == '"' && cont {
				return toks, StateString
			}
		case isDigit(r) || (r == '.' && i+1 < n && isDigit(rs[i+1])):
			j := i + 1
			for j < n {
				c := rs[j]
				if isIdentChar(c) || c == '.' || (c == '\'' && j+1 < n && isIdentChar(rs[j+1])) {
					j++
					continue
				}
				// The sign of an exponent: 1e+5, 0x1p-3
				if (c == '+' || c == '-') && (rs[j-1] == 'e' || rs[j-1] == 'E' || rs[j-1] == 'p' || rs[j-1] == 'P') {
					j++
					continue
				}
				break
			}
			add(i, j, KindNumber)
			i = j
		case isIdentStart(r):
			j := i + 1
			for j < n && isIdentChar(rs[j]) {
				j++
			}
			// String and character prefixes
			if j < n && (rs[j] == '"' || rs[j] == '\'') {
				p := string(rs[i:j])
				if p == "L" || p == "u" || p == "U" || p == "u8" {
					q := rs[j]
					e, closed := closeQuote(rs, j+1, q)
					kind := KindString
					if q == '\'' {
						kind = KindChar
					}
					add(i, e, kind)
					i = e
					if !closed && q == '"' && cont {
						return toks, StateString
					}
					continue
				}
			}
			w := string(rs[i:j])
			switch {
			case keywords[w]:
				add(i, j, KindKeyword)
			case types[w]:
				add(i, j, KindType)
			case preproc && w == "defined":
				add(i, j, KindPreproc)
			}
			i = j
		default:
			i++
		}
	}
	if preproc && cont {
		return toks, StatePreproc
	}
	return toks, StateNormal
}

// closeComment returns the index after "*/" from i, -1 if the comment goes on
func closeComment(rs []rune, i int) int {
	for ; i+1 < len(rs); i++ {
		if rs[i] == '*' && rs[i+1] == '/' {
			return i + 2
		}
	}
	return -1
}

// closeQuote returns the index after the closing quote q from i, or the
// end of the line when it is not closed
func closeQuote(rs []rune, i int, q rune) (int, bool) {
	for i < len(rs) {
		switch rs[i] {
		case '\\':
			i += 2
			continue
		case q:
			return i + 1, true
		}
		i++
	}
	return len(rs), false
}
