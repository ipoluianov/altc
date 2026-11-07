/* AltC standard library: MIME types (Go's mime) */
#ifndef ALT_MIME_H
#define ALT_MIME_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the type of a file extension (".html", case-insensitive): Go's
   built-in table, then /etc/mime.types; NULL when unknown */
const char *alt_mime_type_by_extension(alt_str ext);

typedef struct {
	alt_str key, value;
} alt_mime_param;

/* ParseMediaType: the type (lowercase) and the parameters (keys
   lowercase, RFC 2231 continuations and encodings decoded) sorted by key,
   in a: 0 or -ALT_EMIME with *why Go's message ("mime: no media type",
   "mime: invalid media parameter"...); with that last error the type is
   still set, as Go */
int alt_mime_parse_media_type(alt_arena *a, alt_str v, alt_str *type, alt_mime_param **params, size_t *n, const char **why);
/* FormatMediaType: nothing appended (and -EINVAL) for invalid input */
int alt_mime_format_media_type(alt_buf *b, alt_str type, const alt_mime_param *params, size_t n);
/* a parameter's value, "" when absent */
alt_str alt_mime_param_get(const alt_mime_param *p, size_t n, const char *key);

/* quoted-printable (Go's mime/quotedprintable Reader) */
typedef struct {
	alt_bufreader br;
	alt_buf line;
	size_t off;
	int rerr;
	char err[128];
} alt_qp_reader;

int alt_qp_reader_init(alt_qp_reader *q, alt_reader r);
void alt_qp_reader_free(alt_qp_reader *q);
/* the decoded bytes: a count, 0 at the end, -ALT_EMIME (alt_qp_error) or a read error */
ssize_t alt_qp_read(alt_qp_reader *q, void *buf, size_t n);
const char *alt_qp_error(const alt_qp_reader *q);
alt_reader alt_qp_reader_of(alt_qp_reader *q);

/* ---- RFC 2047 encoded-words (Go's mime.WordEncoder, WordDecoder) ---- */

enum { ALT_MIME_BENCODING = 'b', ALT_MIME_QENCODING = 'q' };

/* WordEncoder.Encode: appends the encoded-word form of s
   ("=?UTF-8?q?caf=C3=A9?="), or s itself when it is ASCII without special
   characters; for UTF-8 (any case) long text is split into words of at
   most 75 characters separated by a space, never inside a character */
int alt_mime_word_encode(alt_buf *b, int encoding, alt_str charset, alt_str s);

typedef struct {
	/* CharsetReader: converts content in charset (lower case) to UTF-8,
	   appending it to out: 0, or a negative code with Go's error text in err
	   (which may be NULL). NULL: charsets other than utf-8, iso-8859-1 and
	   us-ascii fail with "mime: unhandled charset \"x\"" */
	int (*charset_reader)(void *arg, alt_str charset, alt_str content, alt_buf *out, alt_buf *err);
	void *arg;
} alt_mime_word_decoder;

/* WordDecoder.Decode of one encoded-word (d may be NULL): appends the
   decoded text; -ALT_EMIME with Go's text in err ("mime: invalid RFC 2047
   encoded-word", "illegal base64 data at input byte 4", "mime: invalid hex
   byte 0x67"...) or the charset reader's code; nothing is appended then */
int alt_mime_word_decode(const alt_mime_word_decoder *d, alt_buf *b, alt_str word, alt_buf *err);
/* WordDecoder.DecodeHeader: appends header with its encoded-words decoded
   (malformed words stay as they are); fails only when the charset is not
   handled (or the charset reader fails), appending nothing then */
int alt_mime_decode_header(const alt_mime_word_decoder *d, alt_buf *b, alt_str header, alt_buf *err);

#ifdef __cplusplus
}
#endif

#endif
