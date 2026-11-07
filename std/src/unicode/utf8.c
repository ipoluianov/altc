/* Decoding follows the Unicode standard's table of well-formed byte
   sequences (Table 3-7), as Go's unicode/utf8 does. */
#include <alt/unicode/utf8.h>

size_t alt_utf8_decode(const char *s, size_t n, int32_t *r) {
	const unsigned char *p = (const unsigned char *)s;
	if (n == 0) {
		*r = ALT_UTF8_RUNE_ERROR;
		return 0;
	}
	unsigned c = p[0];
	if (c < 0x80) {
		*r = (int32_t)c;
		return 1;
	}
	size_t len;
	unsigned lo = 0x80, hi = 0xBF;
	int32_t v;
	if (c >= 0xC2 && c <= 0xDF)
		len = 2, v = c & 0x1F;
	else if (c >= 0xE0 && c <= 0xEF) {
		len = 3, v = c & 0x0F;
		if (c == 0xE0)
			lo = 0xA0;
		else if (c == 0xED)
			hi = 0x9F; /* no surrogates */
	} else if (c >= 0xF0 && c <= 0xF4) {
		len = 4, v = c & 0x07;
		if (c == 0xF0)
			lo = 0x90;
		else if (c == 0xF4)
			hi = 0x8F;
	} else {
		*r = ALT_UTF8_RUNE_ERROR;
		return 1;
	}
	if (n < len) {
		*r = ALT_UTF8_RUNE_ERROR;
		return 1;
	}
	for (size_t i = 1; i < len; i++) {
		unsigned b = p[i];
		if (b < lo || b > hi) {
			*r = ALT_UTF8_RUNE_ERROR;
			return 1;
		}
		lo = 0x80, hi = 0xBF;
		v = (v << 6) | (int32_t)(b & 0x3F);
	}
	*r = v;
	return len;
}

size_t alt_utf8_decode_last(const char *s, size_t n, int32_t *r) {
	if (n == 0) {
		*r = ALT_UTF8_RUNE_ERROR;
		return 0;
	}
	const unsigned char *p = (const unsigned char *)s;
	if (p[n - 1] < 0x80) {
		*r = p[n - 1];
		return 1;
	}
	/* back to a possible start byte, at most 4 bytes */
	size_t lim = n > 4 ? n - 4 : 0, start = n - 1;
	while (start > lim && (p[start] & 0xC0) == 0x80)
		start--;
	size_t k = alt_utf8_decode(s + start, n - start, r);
	if (start + k != n) {
		*r = ALT_UTF8_RUNE_ERROR;
		return 1;
	}
	return k;
}

bool alt_utf8_valid_rune(int32_t r) { return (r >= 0 && r < 0xD800) || (r > 0xDFFF && r <= ALT_UTF8_MAX_RUNE); }

size_t alt_utf8_rune_len(int32_t r) {
	if (r < 0)
		return 0;
	if (r < 0x80)
		return 1;
	if (r < 0x800)
		return 2;
	if (r >= 0xD800 && r <= 0xDFFF)
		return 0;
	if (r < 0x10000)
		return 3;
	if (r <= ALT_UTF8_MAX_RUNE)
		return 4;
	return 0;
}

size_t alt_utf8_encode(char *buf, int32_t r) {
	unsigned char *p = (unsigned char *)buf;
	if (!alt_utf8_valid_rune(r))
		r = ALT_UTF8_RUNE_ERROR;
	uint32_t v = (uint32_t)r;
	if (v < 0x80) {
		p[0] = (unsigned char)v;
		return 1;
	}
	if (v < 0x800) {
		p[0] = (unsigned char)(0xC0 | v >> 6);
		p[1] = (unsigned char)(0x80 | (v & 0x3F));
		return 2;
	}
	if (v < 0x10000) {
		p[0] = (unsigned char)(0xE0 | v >> 12);
		p[1] = (unsigned char)(0x80 | ((v >> 6) & 0x3F));
		p[2] = (unsigned char)(0x80 | (v & 0x3F));
		return 3;
	}
	p[0] = (unsigned char)(0xF0 | v >> 18);
	p[1] = (unsigned char)(0x80 | ((v >> 12) & 0x3F));
	p[2] = (unsigned char)(0x80 | ((v >> 6) & 0x3F));
	p[3] = (unsigned char)(0x80 | (v & 0x3F));
	return 4;
}

bool alt_utf8_valid(const char *s, size_t n) {
	const unsigned char *p = (const unsigned char *)s;
	size_t i = 0;
	while (i < n) {
		/* runs of ASCII, eight bytes at a time */
		while (i + 8 <= n) {
			uint64_t w;
			__builtin_memcpy(&w, p + i, 8);
			if (w & 0x8080808080808080ULL)
				break;
			i += 8;
		}
		if (i >= n)
			break;
		if (p[i] < 0x80) {
			i++;
			continue;
		}
		int32_t r;
		size_t k = alt_utf8_decode(s + i, n - i, &r);
		if (r == ALT_UTF8_RUNE_ERROR && k == 1)
			return false;
		i += k;
	}
	return true;
}

size_t alt_utf8_rune_count(const char *s, size_t n) {
	size_t count = 0, i = 0;
	while (i < n) {
		int32_t r;
		size_t k = (unsigned char)s[i] < 0x80 ? 1 : alt_utf8_decode(s + i, n - i, &r);
		i += k;
		count++;
	}
	return count;
}

bool alt_utf8_full_rune(const char *s, size_t n) {
	if (n == 0)
		return false;
	const unsigned char *p = (const unsigned char *)s;
	unsigned c = p[0];
	size_t need;
	unsigned lo = 0x80, hi = 0xBF;
	if (c >= 0xC2 && c <= 0xDF)
		need = 2;
	else if (c >= 0xE0 && c <= 0xEF) {
		need = 3;
		if (c == 0xE0)
			lo = 0xA0;
		else if (c == 0xED)
			hi = 0x9F;
	} else if (c >= 0xF0 && c <= 0xF4) {
		need = 4;
		if (c == 0xF0)
			lo = 0x90;
		else if (c == 0xF4)
			hi = 0x8F;
	} else
		return true; /* ASCII or a byte that decodes alone (to an error) */
	if (n >= need)
		return true;
	/* a prefix that cannot be completed is an error rune of length 1 */
	for (size_t i = 1; i < n; i++) {
		if (p[i] < lo || p[i] > hi)
			return true;
		lo = 0x80, hi = 0xBF;
	}
	return false;
}
