/* base64 after Go's encoding/base64 (BSD-3-Clause, see LICENSE-Go) */
#include <alt/encoding/base64.h>
#include <stdbool.h>
#include <string.h>

const alt_base64 alt_base64_std = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", '='};
const alt_base64 alt_base64_url = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_", '='};
const alt_base64 alt_base64_raw_std = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", 0};
const alt_base64 alt_base64_raw_url = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_", 0};

size_t alt_base64_encoded_len(const alt_base64 *e, size_t n) { return e->pad ? (n + 2) / 3 * 4 : (n * 8 + 5) / 6; }

int alt_base64_encode(const alt_base64 *e, alt_buf *b, const void *data, size_t n) {
	const unsigned char *p = data;
	int rc = alt_buf_reserve(b, alt_base64_encoded_len(e, n));
	if (rc)
		return rc;
	size_t i = 0;
	for (; i + 3 <= n; i += 3) {
		unsigned v = (unsigned)p[i] << 16 | (unsigned)p[i + 1] << 8 | p[i + 2];
		b->p[b->n++] = e->alphabet[v >> 18 & 63];
		b->p[b->n++] = e->alphabet[v >> 12 & 63];
		b->p[b->n++] = e->alphabet[v >> 6 & 63];
		b->p[b->n++] = e->alphabet[v & 63];
	}
	size_t rem = n - i;
	if (rem) {
		unsigned v = (unsigned)p[i] << 16 | (rem == 2 ? (unsigned)p[i + 1] << 8 : 0);
		b->p[b->n++] = e->alphabet[v >> 18 & 63];
		b->p[b->n++] = e->alphabet[v >> 12 & 63];
		if (rem == 2)
			b->p[b->n++] = e->alphabet[v >> 6 & 63];
		else if (e->pad)
			b->p[b->n++] = e->pad;
		if (e->pad)
			b->p[b->n++] = e->pad;
	}
	b->p[b->n] = 0;
	return 0;
}

int alt_base64_decode(const alt_base64 *e, alt_buf *b, alt_str s) {
	signed char map[256];
	memset(map, -1, sizeof map);
	for (int i = 0; i < 64; i++)
		map[(unsigned char)e->alphabet[i]] = (signed char)i;
	/* the characters without line breaks */
	unsigned q[4];
	int nq = 0;
	size_t i = 0;
	bool saw_pad = false;
	for (;;) {
		/* skip line breaks */
		while (i < s.n && (s.p[i] == '\r' || s.p[i] == '\n'))
			i++;
		if (i == s.n)
			break;
		unsigned char c = (unsigned char)s.p[i];
		if (e->pad && c == (unsigned char)e->pad) {
			/* padding: 2 or 3 characters before it in the quantum */
			if (nq < 2)
				return -ALT_EENC_BASE64;
			i++;
			if (nq == 2) {
				while (i < s.n && (s.p[i] == '\r' || s.p[i] == '\n'))
					i++;
				if (i == s.n || s.p[i] != e->pad)
					return -ALT_EENC_BASE64;
				i++;
			}
			while (i < s.n && (s.p[i] == '\r' || s.p[i] == '\n'))
				i++;
			if (i != s.n)
				return -ALT_EENC_BASE64; /* data after the padding */
			saw_pad = true;
			break;
		}
		if (map[c] < 0)
			return -ALT_EENC_BASE64;
		q[nq++] = (unsigned)map[c];
		i++;
		if (nq == 4) {
			unsigned v = q[0] << 18 | q[1] << 12 | q[2] << 6 | q[3];
			alt_buf_append_byte(b, (int)(v >> 16 & 255));
			alt_buf_append_byte(b, (int)(v >> 8 & 255));
			alt_buf_append_byte(b, (int)(v & 255));
			nq = 0;
		}
	}
	if (nq == 1)
		return -ALT_EENC_BASE64;
	if (nq) {
		if (e->pad && !saw_pad)
			return -ALT_EENC_BASE64; /* the padding is missing */
		unsigned v = q[0] << 18 | q[1] << 12 | (nq > 2 ? q[2] << 6 : 0);
		alt_buf_append_byte(b, (int)(v >> 16 & 255));
		if (nq == 3)
			alt_buf_append_byte(b, (int)(v >> 8 & 255));
	}
	return b->err;
}
