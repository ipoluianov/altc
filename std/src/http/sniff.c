/* Content type sniffing (the WHATWG MIME Sniffing standard), translated
   from Go's net/http/internal/sniff.go (BSD-3-Clause, see LICENSE-Go) */
#include <alt/net/http.h>
#include <stdint.h>
#include <string.h>

enum { EXACT, MASKED, HTML, MP4, TEXT };

typedef struct {
	int kind;
	const char *pat, *mask;
	size_t n;     /* pattern length (patterns contain NUL bytes) */
	bool skip_ws;
	const char *ct;
} sig;

#define S(lit) lit, 0, sizeof(lit) - 1
#define HTMLSIG(lit) {HTML, lit, 0, sizeof(lit) - 1, false, "text/html; charset=utf-8"}

static const sig sigs[] = {
	HTMLSIG("<!DOCTYPE HTML"), HTMLSIG("<HTML"), HTMLSIG("<HEAD"), HTMLSIG("<SCRIPT"), HTMLSIG("<IFRAME"), HTMLSIG("<H1"),
	HTMLSIG("<DIV"), HTMLSIG("<FONT"), HTMLSIG("<TABLE"), HTMLSIG("<A"), HTMLSIG("<STYLE"), HTMLSIG("<TITLE"), HTMLSIG("<B"),
	HTMLSIG("<BODY"), HTMLSIG("<BR"), HTMLSIG("<P"), HTMLSIG("<!--"),
	{MASKED, "<?xml", "\xFF\xFF\xFF\xFF\xFF", 5, true, "text/xml; charset=utf-8"},
	{EXACT, S("%PDF-"), false, "application/pdf"},
	{EXACT, S("%!PS-Adobe-"), false, "application/postscript"},
	{MASKED, "\xFE\xFF\x00\x00", "\xFF\xFF\x00\x00", 4, false, "text/plain; charset=utf-16be"},
	{MASKED, "\xFF\xFE\x00\x00", "\xFF\xFF\x00\x00", 4, false, "text/plain; charset=utf-16le"},
	{MASKED, "\xEF\xBB\xBF\x00", "\xFF\xFF\xFF\x00", 4, false, "text/plain; charset=utf-8"},
	{EXACT, "\x00\x00\x01\x00", 0, 4, false, "image/x-icon"},
	{EXACT, "\x00\x00\x02\x00", 0, 4, false, "image/x-icon"},
	{EXACT, S("BM"), false, "image/bmp"},
	{EXACT, S("GIF87a"), false, "image/gif"},
	{EXACT, S("GIF89a"), false, "image/gif"},
	{MASKED, "RIFF\x00\x00\x00\x00WEBPVP", "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF", 14, false, "image/webp"},
	{EXACT, "\x89PNG\x0D\x0A\x1A\x0A", 0, 8, false, "image/png"},
	{EXACT, "\xFF\xD8\xFF", 0, 3, false, "image/jpeg"},
	{MASKED, "FORM\x00\x00\x00\x00AIFF", "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF", 12, false, "audio/aiff"},
	{MASKED, "ID3", "\xFF\xFF\xFF", 3, false, "audio/mpeg"},
	{MASKED, "OggS\x00", "\xFF\xFF\xFF\xFF\xFF", 5, false, "application/ogg"},
	{MASKED, "MThd\x00\x00\x00\x06", "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8, false, "audio/midi"},
	{MASKED, "RIFF\x00\x00\x00\x00AVI ", "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF", 12, false, "video/avi"},
	{MASKED, "RIFF\x00\x00\x00\x00WAVE", "\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF", 12, false, "audio/wave"},
	{MP4, 0, 0, 0, false, "video/mp4"},
	{EXACT, "\x1A\x45\xDF\xA3", 0, 4, false, "video/webm"},
	{MASKED,
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00LP",
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\xFF\xFF",
		36, false, "application/vnd.ms-fontobject"},
	{EXACT, "\x00\x01\x00\x00", 0, 4, false, "font/ttf"},
	{EXACT, S("OTTO"), false, "font/otf"},
	{EXACT, S("ttcf"), false, "font/collection"},
	{EXACT, S("wOFF"), false, "font/woff"},
	{EXACT, S("wOF2"), false, "font/woff2"},
	{EXACT, "\x1F\x8B\x08", 0, 3, false, "application/x-gzip"},
	{EXACT, "PK\x03\x04", 0, 4, false, "application/zip"},
	{EXACT, "Rar!\x1A\x07\x00", 0, 7, false, "application/x-rar-compressed"},
	{EXACT, "Rar!\x1A\x07\x01\x00", 0, 8, false, "application/x-rar-compressed"},
	{EXACT, "\x00\x61\x73\x6D", 0, 4, false, "application/wasm"},
	{TEXT, 0, 0, 0, false, "text/plain; charset=utf-8"},
};

static bool is_ws(unsigned char b) { return b == '\t' || b == '\n' || b == '\x0c' || b == '\r' || b == ' '; }

static const char *match(const sig *s, const unsigned char *d, size_t n, size_t ws) {
	switch (s->kind) {
	case EXACT:
		return n >= s->n && memcmp(d, s->pat, s->n) == 0 ? s->ct : 0;
	case MASKED:
		if (s->skip_ws)
			d += ws, n -= ws;
		if (n < s->n)
			return 0;
		for (size_t i = 0; i < s->n; i++)
			if ((d[i] & (unsigned char)s->mask[i]) != (unsigned char)s->pat[i])
				return 0;
		return s->ct;
	case HTML: {
		d += ws, n -= ws;
		if (n < s->n + 1)
			return 0;
		for (size_t i = 0; i < s->n; i++) {
			unsigned char b = (unsigned char)s->pat[i], db = d[i];
			if (b >= 'A' && b <= 'Z')
				db &= 0xDF;
			if (b != db)
				return 0;
		}
		return d[s->n] == ' ' || d[s->n] == '>' ? s->ct : 0;
	}
	case MP4: {
		if (n < 12)
			return 0;
		uint32_t box = (uint32_t)d[0] << 24 | (uint32_t)d[1] << 16 | (uint32_t)d[2] << 8 | d[3];
		if (n < box || box % 4 != 0 || memcmp(d + 4, "ftyp", 4) != 0)
			return 0;
		for (size_t st = 8; st < box; st += 4) {
			if (st == 12)
				continue;
			if (memcmp(d + st, "mp4", 3) == 0)
				return s->ct;
		}
		return 0;
	}
	}
	for (size_t i = ws; i < n; i++) {
		unsigned char b = d[i];
		if (b <= 0x08 || b == 0x0B || (b >= 0x0E && b <= 0x1A) || (b >= 0x1C && b <= 0x1F))
			return 0;
	}
	return s->ct;
}

const char *alt_http_detect_content_type(const void *data, size_t n) {
	const unsigned char *d = data;
	if (n > 512)
		n = 512;
	size_t ws = 0;
	while (ws < n && is_ws(d[ws]))
		ws++;
	for (size_t i = 0; i < sizeof sigs / sizeof *sigs; i++) {
		const char *ct = match(&sigs[i], d, n, ws);
		if (ct)
			return ct;
	}
	return "application/octet-stream";
}
