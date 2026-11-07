/* tar internals shared by the field codecs (tar.c), the reader
   (tar_reader.c) and the writer (tar_writer.c). They follow Go's
   archive/tar (BSD-3-Clause, see LICENSE-Go). */
#ifndef ALT_TAR_INTERNAL_H
#define ALT_TAR_INTERNAL_H

#include <alt/archive/tar.h>
#include <alt/strconv.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
	TAR_BLOCK = 512,
	TAR_NAME_SIZE = 100,
	TAR_PREFIX_SIZE = 155,
	TAR_MAX_SPECIAL = 1 << 20,     /* the largest PAX or GNU long name data */
	TAR_MAX_SPARSE = 1 << 20,      /* the most sparse map entries */
	TAR_FORMAT_ANY = ALT_TAR_FORMAT_USTAR | ALT_TAR_FORMAT_PAX | ALT_TAR_FORMAT_GNU,
};

/* Unix seconds of Go's zero time (January 1, year 1) */
#define TAR_ZERO_SEC (-62135596800LL)

/* header field offsets and sizes */
#define F_NAME 0, 100
#define F_MODE 100, 8
#define F_UID 108, 8
#define F_GID 116, 8
#define F_SIZE 124, 12
#define F_MTIME 136, 12
#define F_CHKSUM 148, 8
#define F_TYPE 156, 1
#define F_LINK 157, 100
#define F_MAGIC 257, 6
#define F_VERSION 263, 2
#define F_UNAME 265, 32
#define F_GNAME 297, 32
#define F_DEVMAJOR 329, 8
#define F_DEVMINOR 337, 8
#define F_PREFIX 345, 155          /* USTAR */
#define F_GNU_ATIME 345, 12
#define F_GNU_CTIME 357, 12
#define F_GNU_SPARSE 386, 97       /* 4 entries of 24 bytes and the extended flag */
#define F_GNU_REALSIZE 483, 12
#define F_STAR_PREFIX 345, 131
#define F_STAR_ATIME 476, 12
#define F_STAR_CTIME 488, 12
#define F_STAR_TRAILER 508, 4

/* a field of block b: FLD(b, F_SIZE) gives the pointer and the size */
#define FLD_(b, off, n) ((b) + (off)), (size_t)(n)
#define FLD(b, f) FLD_(b, f)

static inline int64_t tar_block_padding(int64_t off) { return (int64_t)(-(uint64_t)off & (TAR_BLOCK - 1)); }

/* numbers and strings of header fields: errors set *err (0: none) */
alt_str alt__tar_parse_string(const uint8_t *b, size_t n);
int64_t alt__tar_parse_numeric(const uint8_t *b, size_t n, int *err);
int64_t alt__tar_parse_octal(const uint8_t *b, size_t n, int *err);
void alt__tar_format_string(uint8_t *b, size_t n, alt_str s, int *err);
void alt__tar_format_octal(uint8_t *b, size_t n, int64_t x, int *err);
void alt__tar_format_numeric(uint8_t *b, size_t n, int64_t x, int *err);
bool alt__tar_fits_base256(size_t n, int64_t x);
bool alt__tar_fits_octal(size_t n, int64_t x);
bool alt__tar_has_nul(alt_str s);
bool alt__tar_is_ascii(alt_str s);
/* the ASCII characters of s except NUL, into b (or s itself when all are) */
alt_str alt__tar_to_ascii(alt_str s, alt_buf *b);

/* blocks */
int alt__tar_get_format(const uint8_t *blk);
void alt__tar_set_format(uint8_t *blk, int format);

/* PAX records: parse one from s (0 or -ALT_ETAR_HEADER), format one */
bool alt__tar_valid_pax_record(alt_str k, alt_str v);
int alt__tar_parse_pax_record(alt_str s, alt_str *k, alt_str *v, alt_str *rest);
int alt__tar_format_pax_record(alt_buf *b, alt_str k, alt_str v);
int alt__tar_parse_pax_time(alt_str s, alt_tar_time *t);
void alt__tar_format_pax_time(alt_buf *b, alt_tar_time t);
/* Go's Time.String in UTC */
void alt__tar_format_go_time(alt_buf *b, alt_tar_time t);
/* Go's Time.Round(time.Second) */
alt_tar_time alt__tar_round_second(alt_tar_time t);
/* Go's strconv.ParseInt(s, 10, 64): 0 or an error */
int alt__tar_atoi(alt_str s, int64_t *v);

/* records sorted by key, the last of equal keys kept: the new count */
size_t alt__tar_sort_records(alt_tar_record *r, size_t n);
/* the record of key in sorted records, NULL if none */
const alt_tar_record *alt__tar_find_record(const alt_tar_record *r, size_t n, alt_str key);

#endif
