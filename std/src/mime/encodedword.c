/* RFC 2047 encoded-words after Go's mime/encodedword.go (BSD-3-Clause,
   see LICENSE-Go): WordEncoder (B and Q, split into words of at most 75
   characters for UTF-8) and WordDecoder (Decode, DecodeHeader). */
#include <alt/encoding/base64.h>
#include <alt/mime.h>
#include <alt/strconv.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static const char upperhex[] = "0123456789ABCDEF";
#define MAX_ENCODED_WORD_LEN 75
#define MAX_CONTENT_LEN (MAX_ENCODED_WORD_LEN - 10 - 2) /* len("=?UTF-8?q?"), len("?=") */
#define MAX_BASE64_LEN (MAX_CONTENT_LEN / 4 * 3)        /* StdEncoding.DecodedLen(maxContentLen) */

static const char err_invalid_word[] = "mime: invalid RFC 2047 encoded-word";

static bool needs_encoding(alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		unsigned char b = (unsigned char)s.p[i];
		if ((b < ' ' || b > '~') && b != '\t')
			return true;
	}
	return false;
}

static bool is_utf8(alt_str charset) { return alt_str_eq_fold(charset, ALT_STR("UTF-8")); }

static void open_word(alt_buf *b, int e, alt_str charset) {
	alt_buf_append_cstr(b, "=?");
	alt_buf_append_str(b, charset);
	alt_buf_append_byte(b, '?');
	alt_buf_append_byte(b, e);
	alt_buf_append_byte(b, '?');
}

static void split_word(alt_buf *b, int e, alt_str charset) {
	alt_buf_append_cstr(b, "?= ");
	open_word(b, e, charset);
}

static void write_q(alt_buf *b, const char *p, size_t n) {
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)p[i];
		if (c == ' ')
			alt_buf_append_byte(b, '_');
		else if (c >= '!' && c <= '~' && c != '=' && c != '?' && c != '_')
			alt_buf_append_byte(b, c);
		else {
			alt_buf_append_byte(b, '=');
			alt_buf_append_byte(b, upperhex[c >> 4]);
			alt_buf_append_byte(b, upperhex[c & 15]);
		}
	}
}

static void b_encode(alt_buf *b, alt_str charset, alt_str s) {
	if (!is_utf8(charset) || alt_base64_encoded_len(&alt_base64_std, s.n) <= MAX_CONTENT_LEN) {
		alt_base64_encode(&alt_base64_std, b, s.p, s.n);
		return;
	}
	size_t cur = 0, last = 0, rl;
	for (size_t i = 0; i < s.n; i += rl) {
		int32_t r;
		rl = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (cur + rl <= MAX_BASE64_LEN)
			cur += rl;
		else {
			alt_base64_encode(&alt_base64_std, b, s.p + last, i - last);
			split_word(b, 'b', charset);
			last = i;
			cur = rl;
		}
	}
	alt_base64_encode(&alt_base64_std, b, s.p + last, s.n - last);
}

static void q_encode(alt_buf *b, alt_str charset, alt_str s) {
	if (!is_utf8(charset)) {
		write_q(b, s.p, s.n);
		return;
	}
	size_t cur = 0, rl, el;
	for (size_t i = 0; i < s.n; i += rl) {
		unsigned char c = (unsigned char)s.p[i];
		if (c >= ' ' && c <= '~' && c != '=' && c != '?' && c != '_')
			rl = el = 1;
		else {
			int32_t r;
			rl = alt_utf8_decode(s.p + i, s.n - i, &r);
			el = 3 * rl;
		}
		if (cur + el > MAX_CONTENT_LEN) {
			split_word(b, 'q', charset);
			cur = 0;
		}
		write_q(b, s.p + i, rl);
		cur += el;
	}
}

int alt_mime_word_encode(alt_buf *b, int encoding, alt_str charset, alt_str s) {
	if (!needs_encoding(s))
		return alt_buf_append_str(b, s);
	int e = encoding == ALT_MIME_BENCODING ? 'b' : 'q';
	open_word(b, e, charset);
	if (e == 'b')
		b_encode(b, charset, s);
	else
		q_encode(b, charset, s);
	alt_buf_append_cstr(b, "?=");
	return b->err;
}

/* ---- decoding ---- */

static int set_err(alt_buf *err, const char *msg) {
	if (err) {
		alt_buf_reset(err);
		alt_buf_append_cstr(err, msg);
	}
	return -ALT_EMIME;
}

static int b64val(unsigned char c) {
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if (c >= '0' && c <= '9')
		return c - '0' + 52;
	return c == '+' ? 62 : c == '/' ? 63 : -1;
}

/* base64.StdEncoding.DecodeString with Go's CorruptInputError offsets:
   0, or -ALT_EENC_BASE64 with *bad the offset */
int alt__mime_base64_decode(alt_buf *b, alt_str src, size_t *bad) {
	size_t si = 0;
	for (;;) {
		unsigned dbuf[4] = {0, 0, 0, 0};
		int dlen = 4, j;
		bool trailing = false;
		size_t trail_at = 0;
		for (j = 0; j < 4; j++) {
			if (si == src.n) {
				if (j == 0)
					return b->err;
				*bad = si - (size_t)j;
				return -ALT_EENC_BASE64; /* j == 1, or padding expected */
			}
			unsigned char in = (unsigned char)src.p[si++];
			int v6 = b64val(in);
			if (v6 >= 0) {
				dbuf[j] = (unsigned)v6;
				continue;
			}
			if (in == '\n' || in == '\r') {
				j--;
				continue;
			}
			if (in != '=' || j < 2) {
				*bad = si - 1;
				return -ALT_EENC_BASE64;
			}
			if (j == 2) {
				while (si < src.n && (src.p[si] == '\n' || src.p[si] == '\r'))
					si++;
				if (si == src.n) {
					*bad = src.n;
					return -ALT_EENC_BASE64;
				}
				if (src.p[si] != '=') {
					*bad = si - 1;
					return -ALT_EENC_BASE64;
				}
				si++;
			}
			while (si < src.n && (src.p[si] == '\n' || src.p[si] == '\r'))
				si++;
			if (si < src.n) {
				trailing = true;
				trail_at = si;
			}
			dlen = j;
			break;
		}
		unsigned v = dbuf[0] << 18 | dbuf[1] << 12 | dbuf[2] << 6 | dbuf[3];
		if (dlen >= 2)
			alt_buf_append_byte(b, (int)(v >> 16 & 255));
		if (dlen >= 3)
			alt_buf_append_byte(b, (int)(v >> 8 & 255));
		if (dlen >= 4)
			alt_buf_append_byte(b, (int)(v & 255));
		if (trailing) {
			*bad = trail_at;
			return -ALT_EENC_BASE64;
		}
		if (dlen < 4)
			return b->err;
	}
}

static int from_hex(unsigned char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

static int q_decode(alt_buf *out, alt_str s, alt_buf *err) {
	for (size_t i = 0; i < s.n; i++) {
		unsigned char c = (unsigned char)s.p[i];
		if (c == '_')
			alt_buf_append_byte(out, ' ');
		else if (c == '=') {
			if (i + 2 >= s.n)
				return set_err(err, err_invalid_word);
			int h = from_hex((unsigned char)s.p[i + 1]), l;
			if (h < 0 || (l = from_hex((unsigned char)s.p[i + 2])) < 0) {
				char msg[48];
				snprintf(msg, sizeof msg, "mime: invalid hex byte 0x%02x", (unsigned char)s.p[h < 0 ? i + 1 : i + 2]);
				return set_err(err, msg);
			}
			alt_buf_append_byte(out, h << 4 | l);
			i += 2;
		} else if ((c <= '~' && c >= ' ') || c == '\n' || c == '\r' || c == '\t')
			alt_buf_append_byte(out, c);
		else
			return set_err(err, err_invalid_word);
	}
	return out->err;
}

static int decode_text(int enc, alt_str text, alt_buf *out, alt_buf *err) {
	switch (enc) {
	case 'B':
	case 'b': {
		size_t bad = 0;
		int rc = alt__mime_base64_decode(out, text, &bad);
		if (rc == -ALT_EENC_BASE64) {
			char msg[64];
			snprintf(msg, sizeof msg, "illegal base64 data at input byte %zu", bad);
			set_err(err, msg);
			return -ALT_EMIME;
		}
		return rc;
	}
	case 'Q':
	case 'q':
		return q_decode(out, text, err);
	}
	return set_err(err, err_invalid_word);
}

static int convert(const alt_mime_word_decoder *d, alt_buf *b, alt_str charset, alt_str content, alt_buf *err) {
	if (alt_str_eq_fold(charset, ALT_STR("utf-8")))
		return alt_buf_append_str(b, content);
	if (alt_str_eq_fold(charset, ALT_STR("iso-8859-1"))) {
		for (size_t i = 0; i < content.n; i++) {
			char u[4];
			alt_buf_append(b, u, alt_utf8_encode(u, (unsigned char)content.p[i]));
		}
		return b->err;
	}
	if (alt_str_eq_fold(charset, ALT_STR("us-ascii"))) {
		for (size_t i = 0; i < content.n; i++) {
			if ((unsigned char)content.p[i] >= 0x80)
				alt_buf_append_cstr(b, "\xef\xbf\xbd");
			else
				alt_buf_append_byte(b, content.p[i]);
		}
		return b->err;
	}
	if (!d || !d->charset_reader) {
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_cstr(err, "mime: unhandled charset ");
			alt_quote(err, charset);
		}
		return -ALT_EMIME;
	}
	alt_buf lower = ALT_BUF_INIT;
	alt_buf_append_str(&lower, charset);
	if (lower.p)
		alt_ascii_lower(lower.p, lower.n);
	if (err)
		alt_buf_reset(err);
	int rc = lower.err ? lower.err : d->charset_reader(d->arg, alt_buf_str(&lower), content, b, err);
	alt_buf_free(&lower);
	return rc < 0 ? rc : b->err;
}

int alt_mime_word_decode(const alt_mime_word_decoder *d, alt_buf *b, alt_str word, alt_buf *err) {
	if (word.n < 8 || !alt_str_has_prefix(word, ALT_STR("=?")) || !alt_str_has_suffix(word, ALT_STR("?=")) ||
		alt_str_count(word, ALT_STR("?")) != 4)
		return set_err(err, err_invalid_word);
	word = alt_str_slice(word, 2, word.n - 2);
	alt_str charset, text, encoding;
	alt_str_cut_byte(word, '?', &charset, &text);
	if (!charset.n)
		return set_err(err, err_invalid_word);
	alt_str_cut_byte(text, '?', &encoding, &text);
	if (encoding.n != 1)
		return set_err(err, err_invalid_word);
	alt_buf content = ALT_BUF_INIT;
	int rc = decode_text((unsigned char)encoding.p[0], text, &content, err);
	size_t start = b->n;
	if (rc == 0)
		rc = convert(d, b, charset, alt_buf_str(&content), err);
	alt_buf_free(&content);
	if (rc < 0)
		alt_buf_truncate(b, start);
	return rc;
}

static bool has_non_whitespace(alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		if (s.p[i] != ' ' && s.p[i] != '\t' && s.p[i] != '\n' && s.p[i] != '\r')
			return true;
	return false;
}

int alt_mime_decode_header(const alt_mime_word_decoder *d, alt_buf *b, alt_str header, alt_buf *err) {
	ssize_t i = alt_str_index(header, ALT_STR("=?"));
	if (i < 0)
		return alt_buf_append_str(b, header);
	size_t start0 = b->n;
	alt_buf_append(b, header.p, (size_t)i);
	header = alt_str_from(header, (size_t)i);
	bool between = false;
	alt_buf content = ALT_BUF_INIT;
	int rc = 0;
	for (;;) {
		ssize_t start = alt_str_index(header, ALT_STR("=?"));
		if (start < 0)
			break;
		size_t cur = (size_t)start + 2;
		ssize_t q = alt_str_index_byte(alt_str_from(header, cur), '?');
		if (q < 0)
			break;
		alt_str charset = alt_str_slice(header, cur, cur + (size_t)q);
		cur += (size_t)q + 1;
		if (header.n < cur + 4)
			break;
		int enc = (unsigned char)header.p[cur++];
		if (header.p[cur] != '?')
			break;
		cur++;
		ssize_t j = alt_str_index(alt_str_from(header, cur), ALT_STR("?="));
		if (j < 0)
			break;
		alt_str text = alt_str_slice(header, cur, cur + (size_t)j);
		size_t end = cur + (size_t)j + 2;
		alt_buf_reset(&content);
		if (decode_text(enc, text, &content, 0) < 0) {
			between = false;
			alt_buf_append(b, header.p, end);
			header = alt_str_from(header, end);
			continue;
		}
		if (start > 0 && (!between || has_non_whitespace(alt_str_slice(header, 0, (size_t)start))))
			alt_buf_append(b, header.p, (size_t)start);
		rc = convert(d, b, charset, alt_buf_str(&content), err);
		if (rc < 0)
			break;
		header = alt_str_from(header, end);
		between = true;
	}
	alt_buf_free(&content);
	if (rc < 0) {
		alt_buf_truncate(b, start0);
		return rc;
	}
	alt_buf_append_str(b, header);
	return b->err;
}
