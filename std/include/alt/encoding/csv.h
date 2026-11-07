/* AltC standard library: CSV (Go's encoding/csv, RFC 4180)

	alt_csv_reader r;
	alt_csv_reader_init(&r, rd);
	alt_str *f;
	size_t n;
	while (alt_csv_read(&r, &f, &n) == 0)
		...

   Errors carry Go's texts (alt_csv_error): "parse error on line 2,
   column 5: bare \" in non-quoted-field", "record on line 3: wrong
   number of fields"... */
#ifndef ALT_ENCODING_CSV_H
#define ALT_ENCODING_CSV_H

#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	/* the options, set after init (Go's fields) */
	int32_t comma;             /* ',' */
	int32_t comment;           /* 0: none */
	int fields_per_record;     /* > 0: required; 0: set by the first record; < 0: any */
	bool lazy_quotes, trim_leading_space;
	/* the state */
	alt_bufreader br;
	alt_buf line, rec;
	size_t *ends;
	alt_str *fields;
	size_t nends, cap;
	int num_line;
	char err[160];
} alt_csv_reader;

int alt_csv_reader_init(alt_csv_reader *r, alt_reader rd);
void alt_csv_reader_free(alt_csv_reader *r);
/* the next record (fields valid until the next call): 0; -ALT_EEOF at the
   end; -ALT_ECSV (the record is set for a wrong field count, as Go);
   other errors of the reader */
int alt_csv_read(alt_csv_reader *r, alt_str **fields, size_t *n);
const char *alt_csv_error(const alt_csv_reader *r);

typedef struct {
	int32_t comma;             /* ',' */
	bool use_crlf;
} alt_csv_writer;

/* appends a record (Go's Writer.Write): -EINVAL for an invalid comma */
int alt_csv_write(const alt_csv_writer *w, alt_buf *b, const alt_str *fields, size_t n);

#ifdef __cplusplus
}
#endif

#endif
