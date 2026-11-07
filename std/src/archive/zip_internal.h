/* Shared parts of the ZIP reader and writer, following Go's archive/zip
   (BSD-3-Clause, see LICENSE-Go). */
#ifndef ALT_ARCHIVE_ZIP_INTERNAL_H
#define ALT_ARCHIVE_ZIP_INTERNAL_H

#include <alt/archive/zip.h>
#include <stdint.h>

enum {
	ZIP_FILE_HEADER_SIG = 0x04034b50,
	ZIP_DIR_HEADER_SIG = 0x02014b50,
	ZIP_DIR_END_SIG = 0x06054b50,
	ZIP_DIR64_LOC_SIG = 0x07064b50,
	ZIP_DIR64_END_SIG = 0x06064b50,
	ZIP_DATA_DESC_SIG = 0x08074b50,
	ZIP_FILE_HEADER_LEN = 30,
	ZIP_DIR_HEADER_LEN = 46,
	ZIP_DIR_END_LEN = 22,
	ZIP_DATA_DESC_LEN = 16,
	ZIP_DATA_DESC64_LEN = 24,
	ZIP_DIR64_LOC_LEN = 20,
	ZIP_DIR64_END_LEN = 56,
	ZIP_VERSION20 = 20,
	ZIP_VERSION45 = 45,
	ZIP_UINT16MAX = 0xffff,
	ZIP_ZIP64_EXTRA = 0x0001,
	ZIP_NTFS_EXTRA = 0x000a,
	ZIP_UNIX_EXTRA = 0x000d,
	ZIP_EXTTIME_EXTRA = 0x5455,
	ZIP_INFOZIP_UNIX_EXTRA = 0x5855,
};
#define ZIP_UINT32MAX 0xffffffffull

static inline uint16_t zip_le16(const unsigned char *p) { return (uint16_t)(p[0] | p[1] << 8); }
static inline uint32_t zip_le32(const unsigned char *p) {
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static inline uint64_t zip_le64(const unsigned char *p) { return zip_le32(p) | (uint64_t)zip_le32(p + 4) << 32; }
static inline unsigned char *zip_put16(unsigned char *p, uint16_t v) {
	p[0] = (unsigned char)v, p[1] = (unsigned char)(v >> 8);
	return p + 2;
}
static inline unsigned char *zip_put32(unsigned char *p, uint32_t v) {
	zip_put16(p, (uint16_t)v);
	return zip_put16(p + 2, (uint16_t)(v >> 16));
}
static inline unsigned char *zip_put64(unsigned char *p, uint64_t v) {
	zip_put32(p, (uint32_t)v);
	return zip_put32(p + 4, (uint32_t)(v >> 32));
}

/* Go's detectUTF8: whether s is valid UTF-8, and whether it needs to be
   (has bytes outside the CP-437-compatible 0x20..0x7d without 0x5c) */
void alt__zip_detect_utf8(alt_str s, bool *valid, bool *require);
/* Go's msDosTimeToTime as Unix seconds (normalized as time.Date) */
int64_t alt__zip_dos_unix(uint16_t date, uint16_t time);
/* Go's timeToMsDosTime of t in the zone offset seconds east of UTC */
void alt__zip_dos_time(alt_wall t, int offset, uint16_t *date, uint16_t *time);

#endif
