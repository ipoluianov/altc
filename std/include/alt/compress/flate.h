/* AltC standard library: DEFLATE compressed data (RFC 1951; Go's
   compress/flate).

   Decompression is a reader over the compressed stream; it reads no
   byte past the end of the stream when given an alt_bufreader (which
   gzip and zlib rely on). Compression is a writer: data written is
   compressed into dst, alt_deflate_flush makes everything written so far
   decodable (a sync flush, as Go's Flush), alt_deflate_close ends the
   stream. */
#ifndef ALT_COMPRESS_FLATE_H
#define ALT_COMPRESS_FLATE_H

#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ALT_FLATE_NO_COMPRESSION = 0,
	ALT_FLATE_BEST_SPEED = 1,
	ALT_FLATE_BEST_COMPRESSION = 9,
	ALT_FLATE_DEFAULT = -1,       /* level 6 */
	ALT_FLATE_HUFFMAN_ONLY = -2,  /* Huffman coding of literals, no matches */
};

typedef struct alt_inflate alt_inflate;

/* a decompressor of src (buffered by the decompressor itself) */
int alt_inflate_new(alt_inflate **out, alt_reader src);
/* reads the bytes of the stream from src and no more */
int alt_inflate_new_buf(alt_inflate **out, alt_bufreader *src);
/* as if the data started with dict (preset dictionary): call before reading */
void alt_inflate_set_dict(alt_inflate *z, const void *dict, size_t n);
/* starts over on a new stream (src NULL: the same source) */
void alt_inflate_reset(alt_inflate *z, alt_bufreader *src);
void alt_inflate_free(alt_inflate *z);
/* the decompressed bytes: a count, 0 after the final block, or
   -ALT_EFLATE_CORRUPT, -ALT_EUNEXPECTED_EOF, an error of the source.
   The data decoded before an error is returned first. */
ssize_t alt_inflate_read(alt_inflate *z, void *buf, size_t n);
alt_reader alt_inflate_reader(alt_inflate *z);
/* the input offset of a corrupt-input error (as Go's CorruptInputError) */
int64_t alt_inflate_err_offset(const alt_inflate *z);

typedef struct alt_deflate alt_deflate;

/* level: 0..9, ALT_FLATE_DEFAULT or ALT_FLATE_HUFFMAN_ONLY (else -EINVAL) */
int alt_deflate_new(alt_deflate **out, alt_writer dst, int level);
/* with a preset dictionary (the decompressor must be given the same) */
int alt_deflate_new_dict(alt_deflate **out, alt_writer dst, int level, const void *dict, size_t n);
/* starts a new stream into dst with the same level (and dictionary) */
void alt_deflate_reset(alt_deflate *z, alt_writer dst);
void alt_deflate_free(alt_deflate *z);
ssize_t alt_deflate_write(alt_deflate *z, const void *p, size_t n);
int alt_deflate_flush(alt_deflate *z);
int alt_deflate_close(alt_deflate *z);
alt_writer alt_deflate_writer(alt_deflate *z);

/* one-shot helpers: append to out; inflate stops with -ALT_ETOOLARGE
   beyond limit bytes (0: no limit) */
int alt_deflate_data(alt_buf *out, const void *p, size_t n, int level);
int alt_inflate_data(alt_buf *out, const void *p, size_t n, size_t limit);

#ifdef __cplusplus
}
#endif

#endif
