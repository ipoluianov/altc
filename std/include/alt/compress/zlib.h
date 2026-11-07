/* AltC standard library: zlib streams (RFC 1950; Go's compress/zlib):
   DEFLATE data in a two-byte header and an Adler-32 trailer, optionally
   with a preset dictionary. */
#ifndef ALT_COMPRESS_ZLIB_H
#define ALT_COMPRESS_ZLIB_H

#include <alt/compress/flate.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_zlib_reader alt_zlib_reader;

/* reads the header: 0, -ALT_EGZIP_HEADER, -ALT_EZLIB_DICT (a dictionary
   is needed and dict is not it), -ALT_EUNEXPECTED_EOF */
int alt_zlib_reader_new(alt_zlib_reader **out, alt_reader src, const void *dict, size_t ndict);
int alt_zlib_reader_new_buf(alt_zlib_reader **out, alt_bufreader *src, const void *dict, size_t ndict);
void alt_zlib_reader_free(alt_zlib_reader *z);
/* a count, 0 at the end (after the checksum), -ALT_ECHECKSUM, ... */
ssize_t alt_zlib_read(alt_zlib_reader *z, void *buf, size_t n);
alt_reader alt_zlib_reader_reader(alt_zlib_reader *z);

typedef struct alt_zlib_writer alt_zlib_writer;

int alt_zlib_writer_new(alt_zlib_writer **out, alt_writer dst, int level, const void *dict, size_t ndict);
void alt_zlib_writer_free(alt_zlib_writer *z);
ssize_t alt_zlib_write(alt_zlib_writer *z, const void *p, size_t n);
int alt_zlib_flush(alt_zlib_writer *z);
int alt_zlib_close(alt_zlib_writer *z);
alt_writer alt_zlib_writer_writer(alt_zlib_writer *z);

int alt_zlib_data(alt_buf *out, const void *p, size_t n, int level);
int alt_unzlib_data(alt_buf *out, const void *p, size_t n, size_t limit);

#ifdef __cplusplus
}
#endif

#endif
