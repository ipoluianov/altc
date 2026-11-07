/* AltC standard library: text protocols (Go's net/textproto).

   A reader reads lines, continued lines, numeric replies ("250-...",
   "250 ...") and dot-encoded blocks from a buffered reader; a writer
   writes lines and dot-encoded blocks to a buffered writer; a connection
   joins both over a stream, with the request/response pipeline of Go.

   Errors are negative codes; the reader keeps Go's text of the last one
   (alt_textproto_error):
     -ALT_EEOF               the data ended ("EOF")
     -ALT_EUNEXPECTED_EOF    the data ended inside a unit ("unexpected EOF")
     -EPROTO                 a malformed line (Go's ProtocolError: "short
                             response: \"21\"", "malformed MIME header
                             line: \"a b\""...)
     -EREMOTEIO              a reply whose code is not the expected one
                             (Go's *Error: "550 \"no such user\""); the code
                             is in r->code, the message in the call's msg
     -ALT_ETOOLARGE          a limit was exceeded ("message too large")
   and the errors of the underlying reader. */
#ifndef ALT_NET_TEXTPROTO_H
#define ALT_NET_TEXTPROTO_H

#include <alt/arena.h>
#include <alt/context.h>
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/net.h>
#include <alt/str.h>
#include <alt/sync.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- MIME headers (textproto.MIMEHeader) ---- */

typedef struct {
	alt_str key;   /* canonical ("Content-Type"), or as it was when it has spaces or invalid bytes */
	alt_str value;
} alt_textproto_field;

/* the fields in the order they were added (the values of one key keep
   their order); keys, values and the array live in the arena */
typedef struct {
	alt_textproto_field *f;
	size_t n, cap;
	alt_arena *arena;
} alt_textproto_header;

void alt_textproto_header_init(alt_textproto_header *h, alt_arena *a);
/* Add, Set (replaces every value), Del: the key is canonicalized */
int alt_textproto_header_add(alt_textproto_header *h, alt_str key, alt_str value);
int alt_textproto_header_set(alt_textproto_header *h, alt_str key, alt_str value);
void alt_textproto_header_del(alt_textproto_header *h, alt_str key);
/* Get: the first value of the (canonicalized) key, "" when absent */
alt_str alt_textproto_header_get(const alt_textproto_header *h, alt_str key);
/* whether the key has a value (the first in *value) */
bool alt_textproto_header_lookup(const alt_textproto_header *h, alt_str key, alt_str *value);
/* Values: the number of values of key, the first max of them in out */
size_t alt_textproto_header_values(const alt_textproto_header *h, alt_str key, alt_str *out, size_t max);

/* CanonicalMIMEHeaderKey: appends "Accept-Encoding" for "accept-encoding";
   keys with spaces or bytes that are not token characters stay as they are */
int alt_textproto_canonical_key(alt_buf *b, alt_str key);
/* TrimString: without leading and trailing space, \t, \n, \r */
alt_str alt_textproto_trim(alt_str s);

/* ---- reader ---- */

typedef struct {
	alt_bufreader *br; /* not owned */
	bool dot;          /* a dot reader is open */
	int dot_state;
	int dot_err;       /* an error the dot reader returns after the bytes before it */
	int code;          /* the code of the last -EREMOTEIO */
	alt_buf err;       /* Go's text of the last error */
	alt_buf tmp;
} alt_textproto_reader;

void alt_textproto_reader_init(alt_textproto_reader *r, alt_bufreader *br);
void alt_textproto_reader_free(alt_textproto_reader *r);
/* the text of the last error (Go's Error()) */
const char *alt_textproto_error(const alt_textproto_reader *r);

/* ReadLine: line gets the next line without its \n or \r\n: 0 or an
   error (-ALT_EEOF at the end; a last line without \n is a line) */
int alt_textproto_read_line(alt_textproto_reader *r, alt_buf *line);
/* ReadContinuedLine: lines starting with a space or tab continue the
   line, joined by one space; leading and trailing spaces and tabs go */
int alt_textproto_read_continued_line(alt_textproto_reader *r, alt_buf *line);
/* ReadCodeLine: "code message"; expect 0 accepts any code, 1..9 the first
   digit, 10..99 the first two, 100..999 the code. *code and msg are set
   for -EREMOTEIO too */
int alt_textproto_read_code_line(alt_textproto_reader *r, int expect, int *code, alt_buf *msg);
/* ReadResponse: a reply of one or more lines ("code-text" lines, then
   "code text"), msg the texts joined by \n */
int alt_textproto_read_response(alt_textproto_reader *r, int expect, int *code, alt_buf *msg);

/* DotReader: the decoded dot-encoded block (\r\n -> \n, leading dots
   unescaped) up to the "." line: counts, 0 at its end,
   -ALT_EUNEXPECTED_EOF when the data ends first. The next call on r
   reads (and drops) the rest of the block first. */
alt_reader alt_textproto_dot_reader(alt_textproto_reader *r);
/* ReadDotBytes: the whole decoded block appended to b */
int alt_textproto_read_dot_bytes(alt_textproto_reader *r, alt_buf *b);
/* ReadDotLines: the lines of the block (in a), even on errors */
int alt_textproto_read_dot_lines(alt_textproto_reader *r, alt_arena *a, alt_str **lines, size_t *n);

/* ReadMIMEHeader: "Key: value" lines (possibly continued) up to an empty
   line, with Go's validation; h gets the fields read even on errors
   (-ALT_EEOF when the data ends before the empty line) */
int alt_textproto_read_mime_header(alt_textproto_reader *r, alt_textproto_header *h);
/* with Go's internal limits (net/http, multipart): max_memory bytes
   (estimated as Go does), max_headers fields; <= 0 means no limit */
int alt_textproto_read_mime_header_limit(alt_textproto_reader *r, alt_textproto_header *h, int64_t max_memory, int64_t max_headers);

/* ---- writer ---- */

typedef struct {
	alt_bufwriter *bw; /* not owned */
	bool dot;
	int dot_state;
} alt_textproto_writer;

void alt_textproto_writer_init(alt_textproto_writer *w, alt_bufwriter *bw);
/* PrintfLine: the line, \r\n, and a flush */
int alt_textproto_printf_line(alt_textproto_writer *w, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int alt_textproto_vprintf_line(alt_textproto_writer *w, const char *fmt, va_list ap);
/* DotWriter: what is written gets dot-encoded (\n -> \r\n, leading dots
   doubled); alt_textproto_dot_close ends the block (".\r\n") and flushes.
   The next line written closes the block first. */
alt_writer alt_textproto_dot_writer(alt_textproto_writer *w);
int alt_textproto_dot_close(alt_textproto_writer *w);

/* ---- pipeline (textproto.Pipeline) ---- */

typedef struct {
	alt_mutex mu;
	alt_cond cond;
	unsigned id, req, resp;
} alt_textproto_pipeline;

/* Next: the id of the next request; StartRequest waits until it is the
   turn of id to send, EndRequest passes the turn on (likewise responses) */
unsigned alt_textproto_next(alt_textproto_pipeline *p);
void alt_textproto_start_request(alt_textproto_pipeline *p, unsigned id);
void alt_textproto_end_request(alt_textproto_pipeline *p, unsigned id);
void alt_textproto_start_response(alt_textproto_pipeline *p, unsigned id);
void alt_textproto_end_response(alt_textproto_pipeline *p, unsigned id);

/* ---- connections (textproto.Conn) ---- */

typedef struct alt_textproto_conn {
	alt_textproto_reader r;
	alt_textproto_writer w;
	alt_textproto_pipeline p;
	alt_bufreader br;
	alt_bufwriter bw;
	alt_conn *conn;    /* closed by alt_textproto_close (set by alt_textproto_dial) */
} alt_textproto_conn;

/* NewConn over a stream (not owned); free with alt_textproto_close */
int alt_textproto_conn_new(alt_textproto_conn **out, alt_reader rd, alt_writer wr);
/* Dial: connects; the connection is closed with the textproto one */
int alt_textproto_dial(alt_textproto_conn **out, const char *network, const char *address, alt_ctx *ctx);
/* closes the connection (when owned) and frees c */
int alt_textproto_close(alt_textproto_conn *c);
/* Cmd: waits for its turn, sends the line, gives the id to wait for the
   response with (alt_textproto_start_response(&c->p, id)) */
int alt_textproto_cmd(alt_textproto_conn *c, unsigned *id, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
int alt_textproto_vcmd(alt_textproto_conn *c, unsigned *id, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
