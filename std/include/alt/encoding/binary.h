/* AltC standard library: fixed-size numbers and varints (Go's
   encoding/binary) */
#ifndef ALT_ENCODING_BINARY_H
#define ALT_ENCODING_BINARY_H

#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t alt_be16(const void *p) {
	const uint8_t *b = p;
	return (uint16_t)(b[0] << 8 | b[1]);
}
static inline uint32_t alt_be32(const void *p) {
	const uint8_t *b = p;
	return (uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | b[3];
}
static inline uint64_t alt_be64(const void *p) { return (uint64_t)alt_be32(p) << 32 | alt_be32((const uint8_t *)p + 4); }
static inline uint16_t alt_le16(const void *p) {
	const uint8_t *b = p;
	return (uint16_t)(b[1] << 8 | b[0]);
}
static inline uint32_t alt_le32(const void *p) {
	const uint8_t *b = p;
	return (uint32_t)b[3] << 24 | (uint32_t)b[2] << 16 | (uint32_t)b[1] << 8 | b[0];
}
static inline uint64_t alt_le64(const void *p) { return (uint64_t)alt_le32((const uint8_t *)p + 4) << 32 | alt_le32(p); }
static inline void alt_put_be16(void *p, uint16_t v) {
	uint8_t *b = p;
	b[0] = (uint8_t)(v >> 8), b[1] = (uint8_t)v;
}
static inline void alt_put_be32(void *p, uint32_t v) {
	uint8_t *b = p;
	b[0] = (uint8_t)(v >> 24), b[1] = (uint8_t)(v >> 16), b[2] = (uint8_t)(v >> 8), b[3] = (uint8_t)v;
}
static inline void alt_put_be64(void *p, uint64_t v) {
	alt_put_be32(p, (uint32_t)(v >> 32));
	alt_put_be32((uint8_t *)p + 4, (uint32_t)v);
}
static inline void alt_put_le16(void *p, uint16_t v) {
	uint8_t *b = p;
	b[0] = (uint8_t)v, b[1] = (uint8_t)(v >> 8);
}
static inline void alt_put_le32(void *p, uint32_t v) {
	uint8_t *b = p;
	b[0] = (uint8_t)v, b[1] = (uint8_t)(v >> 8), b[2] = (uint8_t)(v >> 16), b[3] = (uint8_t)(v >> 24);
}
static inline void alt_put_le64(void *p, uint64_t v) {
	alt_put_le32(p, (uint32_t)v);
	alt_put_le32((uint8_t *)p + 4, (uint32_t)(v >> 32));
}

enum { ALT_MAX_VARINT_LEN64 = 10 };

/* the bytes written (at most ALT_MAX_VARINT_LEN64) */
size_t alt_put_uvarint(uint8_t *buf, uint64_t v);
size_t alt_put_varint(uint8_t *buf, int64_t v); /* zigzag */
int alt_append_uvarint(alt_buf *b, uint64_t v);
int alt_append_varint(alt_buf *b, int64_t v);
/* Go's Uvarint/Varint: the bytes read (> 0), 0 when buf is too short,
   -(bytes) on overflow */
int alt_uvarint(const void *buf, size_t n, uint64_t *v);
int alt_varint(const void *buf, size_t n, int64_t *v);
/* from a stream (ReadUvarint): 0; -ALT_EEOF before the first byte,
   -ALT_EUNEXPECTED_EOF after it, -ALT_EVARINT on overflow (*v has the
   bits gathered so far then, as Go) */
int alt_read_uvarint(alt_bufreader *r, uint64_t *v);
int alt_read_varint(alt_bufreader *r, int64_t *v);

#ifdef __cplusplus
}
#endif

#endif
