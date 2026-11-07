/* varints after Go's encoding/binary (BSD-3-Clause, see LICENSE-Go) */
#include <alt/encoding/binary.h>

size_t alt_put_uvarint(uint8_t *buf, uint64_t v) {
	size_t i = 0;
	while (v >= 0x80) {
		buf[i++] = (uint8_t)v | 0x80;
		v >>= 7;
	}
	buf[i++] = (uint8_t)v;
	return i;
}

static uint64_t zigzag(int64_t v) { return (uint64_t)v << 1 ^ (uint64_t)(v >> 63); }

size_t alt_put_varint(uint8_t *buf, int64_t v) { return alt_put_uvarint(buf, zigzag(v)); }

int alt_append_uvarint(alt_buf *b, uint64_t v) {
	uint8_t t[ALT_MAX_VARINT_LEN64];
	return alt_buf_append(b, t, alt_put_uvarint(t, v));
}

int alt_append_varint(alt_buf *b, int64_t v) { return alt_append_uvarint(b, zigzag(v)); }

int alt_uvarint(const void *buf, size_t n, uint64_t *v) {
	const uint8_t *p = buf;
	uint64_t x = 0;
	unsigned s = 0;
	*v = 0;
	for (size_t i = 0; i < n; i++) {
		if (i == ALT_MAX_VARINT_LEN64)
			return -(int)(i + 1);
		uint8_t c = p[i];
		if (c < 0x80) {
			if (i == ALT_MAX_VARINT_LEN64 - 1 && c > 1)
				return -(int)(i + 1);
			*v = x | (uint64_t)c << s;
			return (int)(i + 1);
		}
		x |= (uint64_t)(c & 0x7f) << s;
		s += 7;
	}
	return 0;
}

static int64_t unzigzag(uint64_t u) {
	int64_t x = (int64_t)(u >> 1);
	return u & 1 ? ~x : x;
}

int alt_varint(const void *buf, size_t n, int64_t *v) {
	uint64_t u;
	int k = alt_uvarint(buf, n, &u);
	*v = unzigzag(u);
	return k;
}

/* as Go, the bits gathered so far come with the errors */
int alt_read_uvarint(alt_bufreader *r, uint64_t *v) {
	uint64_t x = 0;
	unsigned s = 0;
	for (int i = 0; i < ALT_MAX_VARINT_LEN64; i++) {
		int c = alt_bufreader_byte(r);
		if (c < 0) {
			*v = x;
			if (i > 0 && c == -ALT_EEOF)
				return -ALT_EUNEXPECTED_EOF;
			return c;
		}
		if (c < 0x80) {
			if (i == ALT_MAX_VARINT_LEN64 - 1 && c > 1) {
				*v = x;
				return -ALT_EVARINT;
			}
			*v = x | (uint64_t)c << s;
			return 0;
		}
		x |= (uint64_t)(c & 0x7f) << s;
		s += 7;
	}
	*v = x;
	return -ALT_EVARINT;
}

int alt_read_varint(alt_bufreader *r, int64_t *v) {
	uint64_t u;
	int rc = alt_read_uvarint(r, &u);
	*v = unzigzag(u);
	return rc;
}
