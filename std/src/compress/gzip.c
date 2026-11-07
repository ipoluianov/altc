/* gzip (RFC 1952) and zlib (RFC 1950) streams, after Go's compress/gzip
   and compress/zlib (BSD-3-Clause, see LICENSE-Go) */
#include <alt/compress/gzip.h>
#include <alt/compress/zlib.h>
#include <alt/hash/adler32.h>
#include <alt/hash/crc32.h>
#include <alt/unicode/utf8.h>
#include <stdlib.h>
#include <string.h>

enum { FLAG_TEXT = 1, FLAG_HDRCRC = 2, FLAG_EXTRA = 4, FLAG_NAME = 8, FLAG_COMMENT = 16 };

static int no_eof(int rc) { return rc == -ALT_EEOF ? -ALT_EUNEXPECTED_EOF : rc; }

/* io.ReadFull: 0, -ALT_EEOF before any byte, -ALT_EUNEXPECTED_EOF after some */
static int read_full(alt_bufreader *r, void *buf, size_t n) {
	size_t got = 0;
	while (got < n) {
		ssize_t m = alt_bufreader_read(r, (char *)buf + got, n - got);
		if (m < 0)
			return (int)m;
		if (m == 0)
			return got ? -ALT_EUNEXPECTED_EOF : -ALT_EEOF;
		got += (size_t)m;
	}
	return 0;
}

static uint32_t le32(const unsigned char *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static uint32_t be32(const unsigned char *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

/* ---- gzip reader ---- */

struct alt_gzip_reader {
	alt_bufreader *r;
	alt_bufreader own;
	bool owns;
	alt_inflate *dec;
	uint32_t digest, size;
	int err;               /* 0, 1 at the end, or an error */
	bool multistream;
	alt_gzip_header hdr;
	alt_buf name, comment, extra;
};

/* a zero-terminated Latin-1 string of at most 512 bytes, as UTF-8 */
static int read_string(alt_gzip_reader *z, alt_buf *out) {
	alt_buf_reset(out);
	for (int i = 0;; i++) {
		if (i >= 512)
			return -ALT_EGZIP_HEADER;
		int c = alt_bufreader_byte(z->r);
		if (c < 0)
			return c;
		unsigned char b = (unsigned char)c;
		z->digest = alt_crc32_ieee_update(z->digest, &b, 1);
		if (c == 0)
			return out->err;
		char u[4];
		alt_buf_append(out, u, alt_utf8_encode(u, c));
	}
}

static int read_header(alt_gzip_reader *z) {
	unsigned char buf[10];
	int rc = read_full(z->r, buf, 10);
	if (rc < 0)
		return rc;
	if (buf[0] != 0x1f || buf[1] != 0x8b || buf[2] != 8)
		return -ALT_EGZIP_HEADER;
	int flg = buf[3];
	alt_buf_reset(&z->name);
	alt_buf_reset(&z->comment);
	alt_buf_reset(&z->extra);
	z->hdr = (alt_gzip_header){0};
	int64_t t = le32(buf + 4);
	z->hdr.mtime = t > 0 ? t : 0;
	z->hdr.os = buf[9];
	z->digest = alt_crc32_ieee(buf, 10);
	if (flg & FLAG_EXTRA) {
		if ((rc = read_full(z->r, buf, 2)) < 0)
			return no_eof(rc);
		z->digest = alt_crc32_ieee_update(z->digest, buf, 2);
		size_t n = (size_t)(buf[0] | buf[1] << 8);
		if ((rc = alt_buf_reserve(&z->extra, n + 1)) < 0)
			return rc;
		if (n && (rc = read_full(z->r, z->extra.p, n)) < 0)
			return no_eof(rc);
		z->extra.n = n;
		z->digest = alt_crc32_ieee_update(z->digest, z->extra.p, n);
		z->hdr.extra = z->extra.p ? z->extra.p : "";
		z->hdr.nextra = n;
	}
	if (flg & FLAG_NAME) {
		if ((rc = read_string(z, &z->name)) < 0)
			return no_eof(rc);
		z->hdr.name = alt_buf_cstr(&z->name);
	}
	if (flg & FLAG_COMMENT) {
		if ((rc = read_string(z, &z->comment)) < 0)
			return no_eof(rc);
		z->hdr.comment = alt_buf_cstr(&z->comment);
	}
	if (flg & FLAG_HDRCRC) {
		if ((rc = read_full(z->r, buf, 2)) < 0)
			return no_eof(rc);
		if ((uint16_t)(buf[0] | buf[1] << 8) != (uint16_t)z->digest)
			return -ALT_EGZIP_HEADER;
	}
	z->digest = 0;
	if (!z->dec)
		return alt_inflate_new_buf(&z->dec, z->r);
	alt_inflate_reset(z->dec, z->r);
	return 0;
}

static int gzip_reader_init(alt_gzip_reader **out, alt_gzip_reader *z) {
	z->multistream = true;
	int rc = read_header(z);
	if (rc < 0) {
		alt_gzip_reader_free(z);
		return rc;
	}
	*out = z;
	return 0;
}

int alt_gzip_reader_new_buf(alt_gzip_reader **out, alt_bufreader *src) {
	alt_gzip_reader *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	z->r = src;
	return gzip_reader_init(out, z);
}

int alt_gzip_reader_new(alt_gzip_reader **out, alt_reader src) {
	alt_gzip_reader *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	int rc = alt_bufreader_init(&z->own, src, 0);
	if (rc < 0) {
		free(z);
		return rc;
	}
	z->owns = true;
	z->r = &z->own;
	return gzip_reader_init(out, z);
}

void alt_gzip_reader_free(alt_gzip_reader *z) {
	if (!z)
		return;
	alt_inflate_free(z->dec);
	if (z->owns)
		alt_bufreader_free(&z->own);
	alt_buf_free(&z->name);
	alt_buf_free(&z->comment);
	alt_buf_free(&z->extra);
	free(z);
}

void alt_gzip_reader_multistream(alt_gzip_reader *z, bool on) { z->multistream = on; }
const alt_gzip_header *alt_gzip_reader_header(const alt_gzip_reader *z) { return &z->hdr; }

ssize_t alt_gzip_read(alt_gzip_reader *z, void *buf, size_t n) {
	for (;;) {
		if (z->err)
			return z->err == 1 ? 0 : z->err;
		ssize_t k = alt_inflate_read(z->dec, buf, n);
		if (k > 0) {
			z->digest = alt_crc32_ieee_update(z->digest, buf, (size_t)k);
			z->size += (uint32_t)k;
			return k;
		}
		if (k < 0)
			return z->err = (int)k;
		if (n == 0)
			return 0;
		/* the end of a member: its trailer */
		unsigned char t[8];
		int rc = read_full(z->r, t, 8);
		if (rc < 0)
			return z->err = no_eof(rc);
		if (le32(t) != z->digest || le32(t + 4) != z->size)
			return z->err = -ALT_ECHECKSUM;
		z->digest = z->size = 0;
		if (!z->multistream) {
			z->err = 1;
			continue;
		}
		rc = read_header(z);
		if (rc == -ALT_EEOF)
			z->err = 1;
		else if (rc < 0)
			z->err = rc;
	}
}

static ssize_t gz_reader_read(void *obj, void *buf, size_t n) { return alt_gzip_read(obj, buf, n); }
alt_reader alt_gzip_reader_reader(alt_gzip_reader *z) { return (alt_reader){gz_reader_read, z}; }

/* ---- gzip writer ---- */

struct alt_gzip_writer {
	alt_writer dst;
	int level;
	bool wrote_header, closed;
	alt_buf head;          /* the header, made at creation */
	alt_deflate *comp;
	uint32_t digest, size;
	int err;
};

/* UTF-8 to Latin-1 (as Go's writeString: no NUL, nothing beyond U+00FF) */
static int latin1(alt_buf *b, const char *s) {
	size_t n = strlen(s);
	for (size_t i = 0; i < n;) {
		int32_t r;
		size_t k = alt_utf8_decode(s + i, n - i, &r);
		if (r == 0 || r > 0xff)
			return -EINVAL;
		alt_buf_append_byte(b, (int)r);
		i += k;
	}
	return alt_buf_append_byte(b, 0);
}

int alt_gzip_writer_new(alt_gzip_writer **out, alt_writer dst, int level, const alt_gzip_header *hdr) {
	if (level < ALT_FLATE_HUFFMAN_ONLY || level > ALT_FLATE_BEST_COMPRESSION)
		return -EINVAL;
	alt_gzip_writer *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	z->dst = dst;
	z->level = level;
	alt_gzip_header h = {.os = 255};
	if (hdr)
		h = *hdr;
	unsigned char b[10] = {0x1f, 0x8b, 8};
	if (h.extra)
		b[3] |= FLAG_EXTRA;
	if (h.name && h.name[0])
		b[3] |= FLAG_NAME;
	if (h.comment && h.comment[0])
		b[3] |= FLAG_COMMENT;
	if (h.mtime > 0) {
		uint32_t t = (uint32_t)h.mtime;
		b[4] = (unsigned char)t, b[5] = (unsigned char)(t >> 8), b[6] = (unsigned char)(t >> 16), b[7] = (unsigned char)(t >> 24);
	}
	b[8] = level == ALT_FLATE_BEST_COMPRESSION ? 2 : level == ALT_FLATE_BEST_SPEED ? 4 : 0;
	b[9] = (unsigned char)h.os;
	alt_buf_append(&z->head, b, 10);
	int rc = 0;
	if (h.extra) {
		if (h.nextra > 0xffff)
			rc = -EINVAL;
		unsigned char l[2] = {(unsigned char)h.nextra, (unsigned char)(h.nextra >> 8)};
		alt_buf_append(&z->head, l, 2);
		alt_buf_append(&z->head, h.extra, h.nextra);
	}
	if (!rc && (b[3] & FLAG_NAME))
		rc = latin1(&z->head, h.name);
	if (!rc && (b[3] & FLAG_COMMENT))
		rc = latin1(&z->head, h.comment);
	if (!rc)
		rc = z->head.err;
	if (!rc)
		rc = alt_deflate_new(&z->comp, dst, level);
	if (rc < 0) {
		alt_gzip_writer_free(z);
		return rc;
	}
	*out = z;
	return 0;
}

void alt_gzip_writer_free(alt_gzip_writer *z) {
	if (!z)
		return;
	alt_deflate_free(z->comp);
	alt_buf_free(&z->head);
	free(z);
}

static int write_header(alt_gzip_writer *z) {
	if (z->wrote_header)
		return 0;
	z->wrote_header = true;
	ssize_t w = alt_write_all(z->dst, z->head.p, z->head.n);
	return w < 0 ? (z->err = (int)w) : 0;
}

ssize_t alt_gzip_write(alt_gzip_writer *z, const void *p, size_t n) {
	if (z->err)
		return z->err;
	if (z->closed)
		return -ALT_ECLOSED;
	if (write_header(z) < 0)
		return z->err;
	z->size += (uint32_t)n;
	z->digest = alt_crc32_ieee_update(z->digest, p, n);
	ssize_t w = alt_deflate_write(z->comp, p, n);
	if (w < 0)
		z->err = (int)w;
	return w;
}

int alt_gzip_flush(alt_gzip_writer *z) {
	if (z->err)
		return z->err;
	if (z->closed)
		return 0;
	if (write_header(z) < 0)
		return z->err;
	return z->err = alt_deflate_flush(z->comp);
}

int alt_gzip_close(alt_gzip_writer *z) {
	if (z->err)
		return z->err;
	if (z->closed)
		return 0;
	z->closed = true;
	if (write_header(z) < 0)
		return z->err;
	if ((z->err = alt_deflate_close(z->comp)) < 0)
		return z->err;
	unsigned char t[8];
	for (int i = 0; i < 4; i++) {
		t[i] = (unsigned char)(z->digest >> (8 * i));
		t[4 + i] = (unsigned char)(z->size >> (8 * i));
	}
	ssize_t w = alt_write_all(z->dst, t, 8);
	return w < 0 ? (z->err = (int)w) : 0;
}

static ssize_t gz_writer_write(void *obj, const void *p, size_t n) { return alt_gzip_write(obj, p, n); }
alt_writer alt_gzip_writer_writer(alt_gzip_writer *z) { return (alt_writer){gz_writer_write, z}; }

int alt_gzip_data(alt_buf *out, const void *p, size_t n, int level) {
	alt_gzip_writer *z;
	int rc = alt_gzip_writer_new(&z, alt_buf_writer(out), level, 0);
	if (rc < 0)
		return rc;
	ssize_t w = alt_gzip_write(z, p, n);
	rc = w < 0 ? (int)w : alt_gzip_close(z);
	alt_gzip_writer_free(z);
	return rc;
}

int alt_gunzip_data(alt_buf *out, const void *p, size_t n, size_t limit) {
	alt_str_reader sr;
	alt_gzip_reader *z;
	int rc = alt_gzip_reader_new(&z, alt_str_reader_init(&sr, alt_str_make(p, n)));
	if (rc < 0)
		return rc;
	ssize_t got = alt_read_all(alt_gzip_reader_reader(z), out, limit);
	alt_gzip_reader_free(z);
	return got < 0 ? (int)got : 0;
}

/* ---- zlib ---- */

struct alt_zlib_reader {
	alt_bufreader *r;
	alt_bufreader own;
	bool owns;
	alt_inflate *dec;
	uint32_t digest;
	int err;
};

static int zlib_reader_init(alt_zlib_reader *z, const void *dict, size_t ndict) {
	unsigned char h[4];
	int rc = read_full(z->r, h, 2);
	if (rc < 0)
		return no_eof(rc);
	if ((h[0] & 0x0f) != 8 || h[0] >> 4 > 7 || ((unsigned)h[0] << 8 | h[1]) % 31 != 0)
		return -ALT_EGZIP_HEADER;
	bool have_dict = h[1] & 0x20;
	if (have_dict) {
		if ((rc = read_full(z->r, h, 4)) < 0)
			return no_eof(rc);
		if (be32(h) != alt_adler32_checksum(dict, dict ? ndict : 0))
			return -ALT_EZLIB_DICT;
	}
	if ((rc = alt_inflate_new_buf(&z->dec, z->r)) < 0)
		return rc;
	if (have_dict)
		alt_inflate_set_dict(z->dec, dict, ndict);
	z->digest = 1;
	return 0;
}

int alt_zlib_reader_new_buf(alt_zlib_reader **out, alt_bufreader *src, const void *dict, size_t ndict) {
	alt_zlib_reader *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	z->r = src;
	int rc = zlib_reader_init(z, dict, ndict);
	if (rc < 0) {
		alt_zlib_reader_free(z);
		return rc;
	}
	*out = z;
	return 0;
}

int alt_zlib_reader_new(alt_zlib_reader **out, alt_reader src, const void *dict, size_t ndict) {
	alt_zlib_reader *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	int rc = alt_bufreader_init(&z->own, src, 0);
	if (rc < 0) {
		free(z);
		return rc;
	}
	z->owns = true;
	z->r = &z->own;
	if ((rc = zlib_reader_init(z, dict, ndict)) < 0) {
		alt_zlib_reader_free(z);
		return rc;
	}
	*out = z;
	return 0;
}

void alt_zlib_reader_free(alt_zlib_reader *z) {
	if (!z)
		return;
	alt_inflate_free(z->dec);
	if (z->owns)
		alt_bufreader_free(&z->own);
	free(z);
}

ssize_t alt_zlib_read(alt_zlib_reader *z, void *buf, size_t n) {
	if (z->err)
		return z->err == 1 ? 0 : z->err;
	ssize_t k = alt_inflate_read(z->dec, buf, n);
	if (k > 0) {
		z->digest = alt_adler32_update(z->digest, buf, (size_t)k);
		return k;
	}
	if (k < 0 || n == 0)
		return k < 0 ? (z->err = (int)k) : 0;
	unsigned char t[4];
	int rc = read_full(z->r, t, 4);
	if (rc < 0)
		return z->err = no_eof(rc);
	if (be32(t) != z->digest)
		return z->err = -ALT_ECHECKSUM;
	z->err = 1;
	return 0;
}

static ssize_t zl_reader_read(void *obj, void *buf, size_t n) { return alt_zlib_read(obj, buf, n); }
alt_reader alt_zlib_reader_reader(alt_zlib_reader *z) { return (alt_reader){zl_reader_read, z}; }

struct alt_zlib_writer {
	alt_writer dst;
	int level;
	bool wrote_header, closed;
	alt_deflate *comp;
	uint32_t digest, dict_sum;
	bool have_dict;
	int err;
};

int alt_zlib_writer_new(alt_zlib_writer **out, alt_writer dst, int level, const void *dict, size_t ndict) {
	if (level < ALT_FLATE_HUFFMAN_ONLY || level > ALT_FLATE_BEST_COMPRESSION)
		return -EINVAL;
	alt_zlib_writer *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	z->dst = dst;
	z->level = level;
	z->digest = 1;
	z->have_dict = dict != 0;
	z->dict_sum = alt_adler32_checksum(dict, dict ? ndict : 0);
	int rc = alt_deflate_new_dict(&z->comp, dst, level, dict, dict ? ndict : 0);
	if (rc < 0) {
		free(z);
		return rc;
	}
	*out = z;
	return 0;
}

void alt_zlib_writer_free(alt_zlib_writer *z) {
	if (!z)
		return;
	alt_deflate_free(z->comp);
	free(z);
}

static int zlib_header(alt_zlib_writer *z) {
	if (z->wrote_header)
		return 0;
	z->wrote_header = true;
	unsigned char h[6] = {0x78};
	int l = z->level;
	h[1] = (unsigned char)((l == -2 || l == 0 || l == 1 ? 0 : l >= 2 && l <= 5 ? 1 : l == 6 || l == -1 ? 2 : 3) << 6);
	if (z->have_dict)
		h[1] |= 1 << 5;
	h[1] = (unsigned char)(h[1] + (31 - ((unsigned)h[0] << 8 | h[1]) % 31));
	size_t n = 2;
	if (z->have_dict) {
		for (int i = 0; i < 4; i++)
			h[2 + i] = (unsigned char)(z->dict_sum >> (24 - 8 * i));
		n = 6;
	}
	ssize_t w = alt_write_all(z->dst, h, n);
	return w < 0 ? (z->err = (int)w) : 0;
}

ssize_t alt_zlib_write(alt_zlib_writer *z, const void *p, size_t n) {
	if (z->err)
		return z->err;
	if (z->closed)
		return -ALT_ECLOSED;
	if (zlib_header(z) < 0)
		return z->err;
	ssize_t w = alt_deflate_write(z->comp, p, n);
	if (w < 0)
		return z->err = (int)w;
	z->digest = alt_adler32_update(z->digest, p, n);
	return w;
}

int alt_zlib_flush(alt_zlib_writer *z) {
	if (z->err)
		return z->err;
	if (z->closed)
		return 0;
	if (zlib_header(z) < 0)
		return z->err;
	return z->err = alt_deflate_flush(z->comp);
}

int alt_zlib_close(alt_zlib_writer *z) {
	if (z->err)
		return z->err;
	if (z->closed)
		return 0;
	z->closed = true;
	if (zlib_header(z) < 0)
		return z->err;
	if ((z->err = alt_deflate_close(z->comp)) < 0)
		return z->err;
	unsigned char t[4];
	for (int i = 0; i < 4; i++)
		t[i] = (unsigned char)(z->digest >> (24 - 8 * i));
	ssize_t w = alt_write_all(z->dst, t, 4);
	return w < 0 ? (z->err = (int)w) : 0;
}

static ssize_t zl_writer_write(void *obj, const void *p, size_t n) { return alt_zlib_write(obj, p, n); }
alt_writer alt_zlib_writer_writer(alt_zlib_writer *z) { return (alt_writer){zl_writer_write, z}; }

int alt_zlib_data(alt_buf *out, const void *p, size_t n, int level) {
	alt_zlib_writer *z;
	int rc = alt_zlib_writer_new(&z, alt_buf_writer(out), level, 0, 0);
	if (rc < 0)
		return rc;
	ssize_t w = alt_zlib_write(z, p, n);
	rc = w < 0 ? (int)w : alt_zlib_close(z);
	alt_zlib_writer_free(z);
	return rc;
}

int alt_unzlib_data(alt_buf *out, const void *p, size_t n, size_t limit) {
	alt_str_reader sr;
	alt_zlib_reader *z;
	int rc = alt_zlib_reader_new(&z, alt_str_reader_init(&sr, alt_str_make(p, n)), 0, 0);
	if (rc < 0)
		return rc;
	ssize_t got = alt_read_all(alt_zlib_reader_reader(z), out, limit);
	alt_zlib_reader_free(z);
	return got < 0 ? (int)got : 0;
}
