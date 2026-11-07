/* AltC standard library: multipart bodies (Go's mime/multipart, RFC
   2046, RFC 7578 forms)

	alt_multipart_reader *mr;
	alt_multipart_reader_new(&mr, body, boundary);
	alt_multipart_part *p;
	while (alt_multipart_next_part(mr, &p) == 0)
		... p->header, p->form_name, p->file_name, alt_read(p->body, ...) ...
	alt_multipart_reader_free(mr);

   Parts with Content-Transfer-Encoding: quoted-printable are decoded (the
   header removed), as Go's NextPart. Errors carry Go's texts
   (alt_multipart_error). */
#ifndef ALT_MIME_MULTIPART_H
#define ALT_MIME_MULTIPART_H

#include <alt/mime.h>
#include <alt/net/http.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	alt_http_header header;
	alt_str form_name;   /* the name of a form-data part, "" otherwise */
	alt_str file_name;   /* the base name of the filename parameter, "" if none */
	alt_reader body;
} alt_multipart_part;

typedef struct alt_multipart_reader alt_multipart_reader;

int alt_multipart_reader_new(alt_multipart_reader **out, alt_reader r, alt_str boundary);
void alt_multipart_reader_free(alt_multipart_reader *r);
/* the next part (valid until the next call; the rest of the previous one
   is skipped): 0, -ALT_EEOF after the last, -ALT_EMIME... */
int alt_multipart_next_part(alt_multipart_reader *r, alt_multipart_part **part);
const char *alt_multipart_error(const alt_multipart_reader *r);

/* a parsed form (Go's ReadForm): values and files kept in memory */
typedef struct {
	alt_str field, file_name, content_type;
	alt_http_header header;
	alt_str data;
} alt_multipart_file;

typedef struct {
	alt_url_values values;
	alt_multipart_file *files;
	size_t nfiles;
	alt_arena *arena;
} alt_multipart_form;

/* reads the whole body into f (in a): at most max_bytes of parts in
   total (0: 32 MiB), else -ALT_EHTTP_BODYTOOLARGE */
int alt_multipart_read_form(alt_multipart_reader *r, alt_arena *a, size_t max_bytes, alt_multipart_form *f);
const alt_multipart_file *alt_multipart_form_file(const alt_multipart_form *f, const char *field);

/* the writer: parts appended to a buffer or written to a stream */
typedef struct {
	alt_writer w;
	char boundary[71];
	bool started;
	int err;
} alt_multipart_writer;

/* a random boundary (30 bytes in hex, as Go) */
int alt_multipart_writer_init(alt_multipart_writer *m, alt_writer w);
/* -EINVAL for a boundary Go's SetBoundary refuses */
int alt_multipart_writer_set_boundary(alt_multipart_writer *m, alt_str boundary);
/* "multipart/form-data; boundary=..." */
int alt_multipart_form_data_content_type(const alt_multipart_writer *m, alt_buf *b);
/* starts a part with the header (keys sorted, as Go): then write its body to m->w */
int alt_multipart_create_part(alt_multipart_writer *m, const alt_http_header *h);
int alt_multipart_create_form_field(alt_multipart_writer *m, alt_str name);
int alt_multipart_create_form_file(alt_multipart_writer *m, alt_str field, alt_str file_name);
int alt_multipart_write_field(alt_multipart_writer *m, alt_str name, alt_str value);
int alt_multipart_writer_close(alt_multipart_writer *m);

/* for servers: the multipart/form-data body of r (Go's ParseMultipartForm) */
int alt_http_parse_multipart_form(alt_http_request *r, size_t max_bytes, const alt_multipart_form **form);

#ifdef __cplusplus
}
#endif

#endif
