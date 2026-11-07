/* AltC standard library: gzip files (RFC 1952; Go's compress/gzip).

   The reader reads concatenated members as one stream (unless
   multistream is turned off) and checks each member's CRC-32 and size;
   the header of the current member is in alt_gzip_reader_header. The
   writer writes one member, its header taken from the header given at
   creation (Latin-1 strings, as the format requires). */
#ifndef ALT_COMPRESS_GZIP_H
#define ALT_COMPRESS_GZIP_H

#include <alt/compress/flate.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *name;      /* file name (UTF-8), NULL or "" for none */
	const char *comment;   /* NULL or "" for none */
	const void *extra;     /* extra field, NULL for none */
	size_t nextra;
	int64_t mtime;         /* Unix seconds, 0 for none */
	int os;                /* 255: unknown (the writer's default when 0 is not meant: set it) */
} alt_gzip_header;

typedef struct alt_gzip_reader alt_gzip_reader;

/* reads the first header: 0, -ALT_EGZIP_HEADER, -ALT_EUNEXPECTED_EOF, -ALT_EEOF (no data) */
int alt_gzip_reader_new(alt_gzip_reader **out, alt_reader src);
/* reads src exactly up to the end of the stream (members) */
int alt_gzip_reader_new_buf(alt_gzip_reader **out, alt_bufreader *src);
void alt_gzip_reader_free(alt_gzip_reader *z);
/* false: stop at the end of the current member (Go's Multistream) */
void alt_gzip_reader_multistream(alt_gzip_reader *z, bool on);
/* the header of the current member (valid while z lives) */
const alt_gzip_header *alt_gzip_reader_header(const alt_gzip_reader *z);
/* the data: a count, 0 at the end, -ALT_ECHECKSUM, -ALT_EGZIP_HEADER,
   -ALT_EFLATE_CORRUPT, -ALT_EUNEXPECTED_EOF, an error of the source */
ssize_t alt_gzip_read(alt_gzip_reader *z, void *buf, size_t n);
alt_reader alt_gzip_reader_reader(alt_gzip_reader *z);

typedef struct alt_gzip_writer alt_gzip_writer;

/* hdr may be NULL (no name, OS unknown); its strings are copied. -EINVAL
   for a bad level, or strings not in Latin-1 or with NUL, or extra
   beyond 65535 bytes */
int alt_gzip_writer_new(alt_gzip_writer **out, alt_writer dst, int level, const alt_gzip_header *hdr);
void alt_gzip_writer_free(alt_gzip_writer *z);
ssize_t alt_gzip_write(alt_gzip_writer *z, const void *p, size_t n);
int alt_gzip_flush(alt_gzip_writer *z);
/* ends the member (trailer): does not close dst */
int alt_gzip_close(alt_gzip_writer *z);
alt_writer alt_gzip_writer_writer(alt_gzip_writer *z);

int alt_gzip_data(alt_buf *out, const void *p, size_t n, int level);
int alt_gunzip_data(alt_buf *out, const void *p, size_t n, size_t limit);

#ifdef __cplusplus
}
#endif

#endif
