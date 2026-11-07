#include <alt/encoding/hex.h>

int alt_hex_encode(alt_buf *b, const void *data, size_t n) {
	static const char d[] = "0123456789abcdef";
	const unsigned char *p = data;
	int rc = alt_buf_reserve(b, 2 * n);
	if (rc)
		return rc;
	for (size_t i = 0; i < n; i++) {
		b->p[b->n++] = d[p[i] >> 4];
		b->p[b->n++] = d[p[i] & 15];
	}
	b->p[b->n] = 0;
	return 0;
}

static int val(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int alt_hex_decode(alt_buf *b, alt_str s) {
	/* Go checks the digits pair by pair, then the length */
	size_t i = 0;
	for (; i + 1 < s.n; i += 2) {
		int x = val(s.p[i]), y = val(s.p[i + 1]);
		if (x < 0 || y < 0)
			return -ALT_EENC_HEX;
		alt_buf_append_byte(b, x << 4 | y);
	}
	if (s.n % 2) {
		if (val(s.p[s.n - 1]) < 0)
			return -ALT_EENC_HEX;
		return -ALT_EENC_HEX; /* odd length */
	}
	return b->err;
}
