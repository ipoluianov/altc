/* AltC standard library: streams (Go's io).

   alt_reader and alt_writer are interfaces: a function and the object it
   works on, passed by value. A read returns the number of bytes read, 0
   at the end of the data, or a negative error code; a write returns the
   number of bytes written or an error (a short write is an error only
   for alt_write_all). */
#ifndef ALT_IO_H
#define ALT_IO_H

#include <alt/errors.h>
#include <alt/str.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ssize_t (*read)(void *obj, void *buf, size_t n);
	void *obj;
} alt_reader;

typedef struct {
	ssize_t (*write)(void *obj, const void *buf, size_t n);
	void *obj;
} alt_writer;

static inline ssize_t alt_read(alt_reader r, void *buf, size_t n) { return r.read(r.obj, buf, n); }
static inline ssize_t alt_write(alt_writer w, const void *buf, size_t n) { return w.write(w.obj, buf, n); }

/* reads exactly n bytes: n, 0 at the end of the data before any byte,
   -ALT_EUNEXPECTED_EOF when the data ends in between */
ssize_t alt_read_full(alt_reader r, void *buf, size_t n);
/* writes all n bytes (looping over short writes): n or an error */
ssize_t alt_write_all(alt_writer w, const void *buf, size_t n);
static inline ssize_t alt_write_str(alt_writer w, alt_str s) { return alt_write_all(w, s.p, s.n); }
/* copies until the end of src: the number of bytes, or an error */
ssize_t alt_copy(alt_writer dst, alt_reader src);
/* copies n bytes: n, or -ALT_EUNEXPECTED_EOF, or an error */
ssize_t alt_copy_n(alt_writer dst, alt_reader src, size_t n);
/* appends everything src has to b: the number of bytes, -ALT_ETOOLARGE
   when there is more than limit (0: no limit) */
ssize_t alt_read_all(alt_reader src, alt_buf *b, size_t limit);

/* a reader of the bytes of s (strings.Reader) */
typedef struct {
	alt_str s;
	size_t off;
} alt_str_reader;
alt_reader alt_str_reader_init(alt_str_reader *r, alt_str s);

/* a writer appending to b */
alt_writer alt_buf_writer(alt_buf *b);
/* a writer that accepts and drops everything */
extern const alt_writer alt_discard;

/* at most n bytes of r, then the end of data (io.LimitReader) */
typedef struct {
	alt_reader r;
	size_t left;
} alt_limit_reader;
alt_reader alt_limit_reader_init(alt_limit_reader *l, alt_reader r, size_t n);

/* reads of a file descriptor, writes to it (waiting with the poller
   when the descriptor is non-blocking and the caller is a task) */
alt_reader alt_fd_reader(int *fd);
alt_writer alt_fd_writer(int *fd);

/* ---- buffered streams (Go's bufio) ---- */

typedef struct {
	alt_reader rd;
	char *buf;
	size_t size, r, w; /* buf[r:w] are buffered */
	int err;           /* sticky error of rd (0 at none; ALT_EEOF after its end) */
} alt_bufreader;

/* size 0: 4096 */
int alt_bufreader_init(alt_bufreader *b, alt_reader rd, size_t size);
void alt_bufreader_free(alt_bufreader *b);
/* drops the buffered data, keeps reading from rd */
void alt_bufreader_reset(alt_bufreader *b, alt_reader rd);
ssize_t alt_bufreader_read(alt_bufreader *b, void *p, size_t n);
/* a byte (0..255), or -ALT_EEOF, or an error */
int alt_bufreader_byte(alt_bufreader *b);
/* the next n bytes without consuming them (valid until the next call):
   0, -ALT_EEOF/-ALT_EUNEXPECTED_EOF when fewer exist (*s gets those),
   -ALT_EBUFFULL when n exceeds the buffer */
int alt_bufreader_peek(alt_bufreader *b, size_t n, alt_str *s);
/* the bytes up to and including delim, pointing into the buffer (valid
   until the next call): 0; -ALT_EBUFFULL with *s the full buffer when no
   delim fits; -ALT_EEOF with *s the rest when the data ends first */
int alt_bufreader_slice(alt_bufreader *b, int delim, alt_str *s);
/* reads a line, appending it to line without its \n or \r\n: 0;
   -ALT_ETOOLARGE for lines longer than max (0: no limit); -ALT_EEOF at
   the end of the data; -ALT_EUNEXPECTED_EOF (with the partial line
   appended) when the data ends in a line */
int alt_bufreader_line(alt_bufreader *b, alt_buf *line, size_t max);
ssize_t alt_bufreader_discard(alt_bufreader *b, size_t n);
static inline size_t alt_bufreader_buffered(const alt_bufreader *b) { return b->w - b->r; }
alt_reader alt_bufreader_reader(alt_bufreader *b);

typedef struct {
	alt_writer wr;
	char *buf;
	size_t size, n;
	int err;           /* sticky error of wr */
} alt_bufwriter;

int alt_bufwriter_init(alt_bufwriter *b, alt_writer wr, size_t size);
void alt_bufwriter_free(alt_bufwriter *b);
void alt_bufwriter_reset(alt_bufwriter *b, alt_writer wr);
/* n or the sticky error */
ssize_t alt_bufwriter_write(alt_bufwriter *b, const void *p, size_t n);
static inline ssize_t alt_bufwriter_str(alt_bufwriter *b, alt_str s) { return alt_bufwriter_write(b, s.p, s.n); }
static inline ssize_t alt_bufwriter_cstr(alt_bufwriter *b, const char *s) { return alt_bufwriter_write(b, s, strlen(s)); }
int alt_bufwriter_byte(alt_bufwriter *b, int c);
int alt_bufwriter_printf(alt_bufwriter *b, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int alt_bufwriter_flush(alt_bufwriter *b);
static inline size_t alt_bufwriter_buffered(const alt_bufwriter *b) { return b->n; }
static inline size_t alt_bufwriter_available(const alt_bufwriter *b) { return b->size - b->n; }
alt_writer alt_bufwriter_writer(alt_bufwriter *b);

#ifdef __cplusplus
}
#endif

#endif
