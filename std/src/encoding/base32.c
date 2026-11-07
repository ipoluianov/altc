/* base32 after Go's encoding/base32 (BSD-3-Clause, see LICENSE-Go) */
#include <alt/encoding/base32.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const alt_base32 alt_base32_std = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567", '='};
const alt_base32 alt_base32_hex = {"0123456789ABCDEFGHIJKLMNOPQRSTUV", '='};
const alt_base32 alt_base32_raw_std = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567", 0};
const alt_base32 alt_base32_raw_hex = {"0123456789ABCDEFGHIJKLMNOPQRSTUV", 0};

size_t alt_base32_encoded_len(const alt_base32 *e, size_t n) { return e->pad ? (n + 4) / 5 * 8 : (n * 8 + 4) / 5; }

int alt_base32_encode(const alt_base32 *e, alt_buf *b, const void *data, size_t n) {
	const uint8_t *s = data;
	const char *a = e->alphabet;
	for (; n >= 5; s += 5, n -= 5) {
		uint64_t v = (uint64_t)s[0] << 32 | (uint64_t)s[1] << 24 | (uint64_t)s[2] << 16 | (uint64_t)s[3] << 8 | s[4];
		char o[8];
		for (int i = 0; i < 8; i++)
			o[i] = a[(v >> (35 - 5 * i)) & 31];
		alt_buf_append(b, o, 8);
	}
	if (n) {
		uint8_t t[5] = {0};
		memcpy(t, s, n);
		uint64_t v = (uint64_t)t[0] << 32 | (uint64_t)t[1] << 24 | (uint64_t)t[2] << 16 | (uint64_t)t[3] << 8 | t[4];
		size_t chars = (n * 8 + 4) / 5;
		for (size_t i = 0; i < chars; i++)
			alt_buf_append_byte(b, a[(v >> (35 - 5 * i)) & 31]);
		if (e->pad)
			for (size_t i = chars; i < 8; i++)
				alt_buf_append_byte(b, e->pad);
	}
	return b->err;
}

int alt_base32_decode(const alt_base32 *e, alt_buf *b, alt_str in, size_t *bad) {
	uint8_t map[256];
	memset(map, 0xff, sizeof map);
	for (int i = 0; i < 32; i++)
		map[(uint8_t)e->alphabet[i]] = (uint8_t)i;
	/* newlines go first (Go's stripNewlines); offsets count without them */
	char *buf = malloc(in.n + 1);
	if (!buf)
		return -ENOMEM;
	size_t olen = 0;
	for (size_t i = 0; i < in.n; i++)
		if (in.p[i] != '\r' && in.p[i] != '\n')
			buf[olen++] = in.p[i];
	const uint8_t *src = (const uint8_t *)buf;
	size_t len = olen;
	int rc = 0;
	size_t badat = 0;
	bool end = false;
	while (len > 0 && !end) {
		uint8_t d[8] = {0};
		size_t dlen = 8;
		for (size_t j = 0; j < 8;) {
			if (len == 0) {
				if (e->pad) {
					rc = -1, badat = olen - len - j;
					goto out;
				}
				dlen = j, end = true;
				break;
			}
			uint8_t c = *src++;
			len--;
			/* without padding Go compares with byte(NoPadding), 0xff */
			uint8_t padc = e->pad ? (uint8_t)e->pad : 0xff;
			if (c == padc && j >= 2 && len < 8) {
				if (len + j < 8 - 1) {
					rc = -1, badat = olen;
					goto out;
				}
				for (size_t k = 0; k < 8 - 1 - j; k++)
					if (len > k && src[k] != padc) {
						rc = -1, badat = olen - len + k - 1;
						goto out;
					}
				dlen = j, end = true;
				if (dlen == 1 || dlen == 3 || dlen == 6) {
					rc = -1, badat = olen - len - 1;
					goto out;
				}
				break;
			}
			d[j] = map[c];
			if (d[j] == 0xff) {
				rc = -1, badat = olen - len - 1;
				goto out;
			}
			j++;
		}
		uint8_t o[5];
		size_t k = 0;
		switch (dlen) {
		case 8: case 7: k = dlen == 8 ? 5 : 4; break;
		case 5: k = 3; break;
		case 4: k = 2; break;
		case 2: k = 1; break;
		default: k = 0;
		}
		o[0] = (uint8_t)(d[0] << 3 | d[1] >> 2);
		o[1] = (uint8_t)(d[1] << 6 | d[2] << 1 | d[3] >> 4);
		o[2] = (uint8_t)(d[3] << 4 | d[4] >> 1);
		o[3] = (uint8_t)(d[4] << 7 | d[5] << 2 | d[6] >> 3);
		o[4] = (uint8_t)(d[6] << 5 | d[7]);
		alt_buf_append(b, o, k);
	}
out:
	free(buf);
	if (rc < 0) {
		if (bad)
			*bad = badat;
		return -ALT_EENC_BASE32;
	}
	return b->err;
}
