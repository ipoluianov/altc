package cc

import (
	"fmt"
	"math/big"
	"strconv"
	"strings"
	"unicode/utf8"
)

// numLit is a parsed numeric constant
type numLit struct {
	isFloat  bool
	f        float64
	ld       *big.Float // exact value for long double constants
	fsuffix  byte       // 'f', 'l' or 0 for floats
	imag     bool       // GNU imaginary constant (suffix i or j)
	i        uint64
	unsigned bool // u suffix
	longs    int  // number of l suffixes
	sizeT    bool // z suffix (C23)
	decimal  bool // decimal integer (affects the type selection)
	overflow bool
}

// parseNumber parses a C integer or floating constant
func parseNumber(text string) (numLit, error) {
	var n numLit
	s := strings.ReplaceAll(text, "'", "")
	// imaginary suffix (GNU): 2.0i, 3if, 1.5Li (i and j never occur otherwise)
	if k := strings.IndexAny(s, "iIjJ"); k > 0 {
		r, err := parseNumber(s[:k] + s[k+1:])
		r.imag = true
		return r, err
	}
	low := strings.ToLower(s)
	isHex := strings.HasPrefix(low, "0x")
	isBin := strings.HasPrefix(low, "0b")
	if !isBin && (strings.ContainsAny(low, ".") || (!isHex && strings.ContainsAny(low, "e")) || (isHex && strings.ContainsAny(low, "p"))) {
		return parseFloatLit(s)
	}
	// integer: split off the suffix
	end := len(s)
	for end > 0 {
		c := low[end-1]
		if c == 'u' || c == 'l' || c == 'z' || (c == 'b' && end > 1 && low[end-2] == 'w') || c == 'w' {
			end--
			continue
		}
		break
	}
	digits, suffix := s[:end], low[end:]
	if isHex && end < 2 {
		return n, fmt.Errorf("invalid integer constant \"%s\"", text)
	}
	switch suffix {
	case "", "u", "l", "ul", "lu", "ll", "ull", "llu", "z", "uz", "zu", "wb", "uwb", "wbu":
	default:
		return n, fmt.Errorf("invalid suffix \"%s\" on integer constant", text[end:])
	}
	n.unsigned = strings.Contains(suffix, "u")
	n.longs = strings.Count(suffix, "l")
	n.sizeT = strings.Contains(suffix, "z")
	base := 10
	d := digits
	switch {
	case isHex:
		base, d = 16, digits[2:]
	case isBin:
		base, d = 2, digits[2:]
	case len(digits) > 1 && digits[0] == '0':
		base, d = 8, digits[1:]
	}
	n.decimal = base == 10
	if d == "" {
		if base == 8 {
			d = "0"
		} else {
			return n, fmt.Errorf("invalid integer constant \"%s\"", text)
		}
	}
	v, err := strconv.ParseUint(d, base, 64)
	if err != nil {
		if ne, ok := err.(*strconv.NumError); ok && ne.Err == strconv.ErrRange {
			n.overflow = true
			v = ^uint64(0)
		} else {
			return n, fmt.Errorf("invalid digit in constant \"%s\"", text)
		}
	}
	n.i = v
	return n, nil
}

func parseFloatLit(s string) (numLit, error) {
	n := numLit{isFloat: true}
	low := strings.ToLower(s)
	isHex := strings.HasPrefix(low, "0x")
	switch {
	case strings.HasSuffix(low, "f16") || strings.HasSuffix(low, "f32") || strings.HasSuffix(low, "f64"):
		if strings.HasSuffix(low, "f64") {
			s = s[:len(s)-3]
		} else {
			n.fsuffix = 'f'
			s = s[:len(s)-3]
		}
	case strings.HasSuffix(low, "f128") || strings.HasSuffix(low, "f64x"):
		n.fsuffix = 'l'
		s = s[:len(s)-4]
	case strings.HasSuffix(low, "df") || strings.HasSuffix(low, "dd") || strings.HasSuffix(low, "dl"):
		return n, fmt.Errorf("decimal floating types are not supported")
	case strings.HasSuffix(low, "f") && !isHex:
		n.fsuffix = 'f'
		s = s[:len(s)-1]
	case strings.HasSuffix(low, "f") && isHex && strings.Contains(low, "p"):
		n.fsuffix = 'f'
		s = s[:len(s)-1]
	case strings.HasSuffix(low, "l"):
		n.fsuffix = 'l'
		s = s[:len(s)-1]
	}
	if isHex && !strings.ContainsAny(s, "pP") {
		return n, fmt.Errorf("hexadecimal floating constants require an exponent")
	}
	f, err := strconv.ParseFloat(s, 64)
	if err != nil {
		if ne, ok := err.(*strconv.NumError); !ok || ne.Err != strconv.ErrRange {
			return n, fmt.Errorf("invalid floating constant \"%s\"", s)
		}
	}
	n.f = f
	if n.fsuffix == 'l' {
		bf, _, err := big.ParseFloat(s, 0, 64, big.ToNearestEven)
		if err == nil {
			n.ld = bf
		}
	}
	return n, nil
}

// decodeLiteral decodes the body of a string or character literal (without
// quotes and prefix). For narrow literals it returns bytes; for wide ones,
// code points (or UTF-16 units when utf16 is set).
func decodeLiteral(body string, wide, utf16 bool) ([]uint32, error) {
	var out []uint32
	emitRune := func(r rune) {
		switch {
		case !wide:
			var buf [4]byte
			n := utf8.EncodeRune(buf[:], r)
			for _, b := range buf[:n] {
				out = append(out, uint32(b))
			}
		case utf16 && r > 0xFFFF:
			r -= 0x10000
			out = append(out, uint32(0xD800+(r>>10)), uint32(0xDC00+(r&0x3FF)))
		default:
			out = append(out, uint32(r))
		}
	}
	for i := 0; i < len(body); {
		c := body[i]
		if c != '\\' {
			if wide && c >= 0x80 {
				r, n := utf8.DecodeRuneInString(body[i:])
				emitRune(r)
				i += n
				continue
			}
			out = append(out, uint32(c))
			i++
			continue
		}
		i++
		if i >= len(body) {
			return out, fmt.Errorf("incomplete escape sequence")
		}
		c = body[i]
		i++
		switch c {
		case 'n':
			out = append(out, '\n')
		case 't':
			out = append(out, '\t')
		case 'r':
			out = append(out, '\r')
		case 'a':
			out = append(out, 7)
		case 'b':
			out = append(out, 8)
		case 'f':
			out = append(out, 12)
		case 'v':
			out = append(out, 11)
		case 'e', 'E':
			out = append(out, 27)
		case '\\', '\'', '"', '?':
			out = append(out, uint32(c))
		case 'x':
			v := uint64(0)
			n := 0
			for i < len(body) && isHexDigit(body[i]) {
				v = v*16 + uint64(hexVal(body[i]))
				i++
				n++
			}
			if n == 0 {
				return out, fmt.Errorf("\\x used with no following hex digits")
			}
			out = append(out, uint32(v))
		case 'u', 'U':
			n := 4
			if c == 'U' {
				n = 8
			}
			if i+n > len(body) {
				return out, fmt.Errorf("incomplete universal character name")
			}
			v, err := strconv.ParseUint(body[i:i+n], 16, 32)
			if err != nil {
				return out, fmt.Errorf("invalid universal character name")
			}
			i += n
			emitRune(rune(v))
		default:
			if c >= '0' && c <= '7' {
				v := uint32(c - '0')
				for k := 0; k < 2 && i < len(body) && body[i] >= '0' && body[i] <= '7'; k++ {
					v = v*8 + uint32(body[i]-'0')
					i++
				}
				out = append(out, v)
			} else {
				out = append(out, uint32(c)) // unknown escape: GCC warns and keeps the char
			}
		}
	}
	return out, nil
}

func isHexDigit(c byte) bool {
	return c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F'
}

func hexVal(c byte) int {
	switch {
	case c >= '0' && c <= '9':
		return int(c - '0')
	case c >= 'a' && c <= 'f':
		return int(c-'a') + 10
	}
	return int(c-'A') + 10
}

// literalPrefix splits "L'x'" into ("L", "x")
func literalPrefix(text string) (string, string) {
	q := strings.IndexAny(text, "\"'")
	if q < 0 || len(text) < q+2 {
		return "", ""
	}
	return text[:q], text[q+1 : len(text)-1]
}

// charValue evaluates a character constant: its value and prefix
func charValue(text string) (int64, string, error) {
	prefix, body := literalPrefix(text)
	wide := prefix != "" && prefix != "u8"
	vals, err := decodeLiteral(body, wide, prefix == "u")
	if err != nil {
		return 0, prefix, err
	}
	if len(vals) == 0 {
		return 0, prefix, fmt.Errorf("empty character constant")
	}
	switch prefix {
	case "":
		if len(vals) == 1 {
			return int64(int8(vals[0])), prefix, nil
		}
		// multi-character constant (GCC: big-endian packing into int)
		v := int64(0)
		for _, x := range vals {
			v = v<<8 | int64(x&0xFF)
		}
		return int64(int32(v)), prefix, nil
	case "u8":
		return int64(uint8(vals[0])), prefix, nil
	case "u":
		return int64(uint16(vals[0])), prefix, nil
	case "U":
		return int64(vals[0]), prefix, nil
	}
	return int64(int32(vals[len(vals)-1])), prefix, nil // L
}
