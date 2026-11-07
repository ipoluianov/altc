/* CSV after Go's encoding/csv (BSD-3-Clause, see LICENSE-Go): the same
   record parsing, positions and errors */
#include <alt/encoding/csv.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool valid_delim(int32_t r) { return r != 0 && r != '"' && r != '\r' && r != '\n' && alt_utf8_valid_rune(r) && r != 0xfffd; }

int alt_csv_reader_init(alt_csv_reader *r, alt_reader rd) {
	memset(r, 0, sizeof *r);
	r->comma = ',';
	return alt_bufreader_init(&r->br, rd, 4096);
}

void alt_csv_reader_free(alt_csv_reader *r) {
	alt_bufreader_free(&r->br);
	alt_buf_free(&r->line);
	alt_buf_free(&r->rec);
	free(r->ends);
	free(r->fields);
}

const char *alt_csv_error(const alt_csv_reader *r) { return r->err; }

/* Go's readLine: the next line with \r\n made \n; 0, -ALT_EEOF (line empty), errors */
static int read_line(alt_csv_reader *r) {
	alt_buf_reset(&r->line);
	int rc;
	for (;;) {
		alt_str s;
		rc = alt_bufreader_slice(&r->br, '\n', &s);
		alt_buf_append(&r->line, s.p, s.n);
		if (rc != -ALT_EBUFFULL)
			break;
	}
	if (rc == -ALT_EEOF && r->line.n > 0) {
		rc = 0;
		if (r->line.p[r->line.n - 1] == '\r')
			alt_buf_truncate(&r->line, r->line.n - 1); /* dropped before the end, as Go */
	}
	r->num_line++;
	size_t n = r->line.n;
	if (n >= 2 && r->line.p[n - 2] == '\r' && r->line.p[n - 1] == '\n') {
		r->line.p[n - 2] = '\n';
		alt_buf_truncate(&r->line, n - 1);
	}
	return rc;
}

static size_t length_nl(const char *p, size_t n) { return n > 0 && p[n - 1] == '\n' ? 1 : 0; }

static int32_t next_rune(const char *p, size_t n) {
	if (!n)
		return -1;
	int32_t c;
	alt_utf8_decode(p, n, &c);
	return c;
}

/* the first occurrence of rune c, or -1 */
static ssize_t index_rune(const char *p, size_t n, int32_t c) {
	char e[4];
	size_t k = alt_utf8_encode(e, c);
	for (size_t i = 0; i + k <= n; i++)
		if (memcmp(p + i, e, k) == 0)
			return (ssize_t)i;
	return -1;
}

static int add_end(alt_csv_reader *r) {
	if (r->nends == r->cap) {
		size_t cap = r->cap ? r->cap * 2 : 16;
		size_t *e = realloc(r->ends, cap * sizeof *e);
		if (!e)
			return -ENOMEM;
		r->ends = e;
		r->cap = cap;
	}
	r->ends[r->nends++] = r->rec.n;
	return 0;
}

static int perr(alt_csv_reader *r, int start, int line, int col, const char *what) {
	if (start != line)
		snprintf(r->err, sizeof r->err, "record on line %d; parse error on line %d, column %d: %s", start, line, col, what);
	else
		snprintf(r->err, sizeof r->err, "parse error on line %d, column %d: %s", line, col, what);
	return -ALT_ECSV;
}

static const char bare[] = "bare \" in non-quoted-field", quote_err[] = "extraneous or missing \" in quoted-field";

int alt_csv_read(alt_csv_reader *r, alt_str **out, size_t *nout) {
	r->err[0] = 0;
	*nout = 0;
	if (r->comma == r->comment || !valid_delim(r->comma) || (r->comment && !valid_delim(r->comment))) {
		snprintf(r->err, sizeof r->err, "csv: invalid field or comment delimiter");
		return -EINVAL;
	}
	int er = 0;
	for (;;) {
		er = read_line(r);
		if (r->comment && next_rune(r->line.p, r->line.n) == r->comment) {
			alt_buf_reset(&r->line);
			if (er)
				break;
			continue;
		}
		if (er == 0 && r->line.n == length_nl(r->line.p, r->line.n)) {
			alt_buf_reset(&r->line);
			continue;
		}
		break;
	}
	if (er == -ALT_EEOF)
		return -ALT_EEOF;
	char cb[4];
	size_t comma_len = alt_utf8_encode(cb, r->comma);
	int rec_line = r->num_line, rc = 0;
	alt_buf_reset(&r->rec);
	r->nends = 0;
	const char *line = r->line.p ? r->line.p : "";
	size_t n = r->line.n;
	int col = 1, pline = r->num_line;
	for (;;) {
		if (r->trim_leading_space) {
			size_t i = 0;
			bool found = false;
			while (i < n) {
				int32_t c;
				size_t k = alt_utf8_decode(line + i, n - i, &c);
				if (!alt_unicode_is_space(c)) {
					found = true;
					break;
				}
				i += k;
			}
			if (!found) {
				i = n;
				col -= (int)length_nl(line, n);
			}
			line += i, n -= i;
			col += (int)i;
		}
		if (n == 0 || line[0] != '"') {
			/* a field without quotes */
			ssize_t i = index_rune(line, n, r->comma);
			size_t fl = i >= 0 ? (size_t)i : n - length_nl(line, n);
			if (!r->lazy_quotes) {
				const char *q = memchr(line, '"', fl);
				if (q) {
					rc = perr(r, rec_line, r->num_line, col + (int)(q - line), bare);
					break;
				}
			}
			alt_buf_append(&r->rec, line, fl);
			if ((rc = add_end(r)) < 0)
				break;
			if (i >= 0) {
				line += (size_t)i + comma_len, n -= (size_t)i + comma_len;
				col += (int)i + (int)comma_len;
				continue;
			}
			break;
		}
		/* a quoted field (its lines continue) */
		line++, n--;
		col++;
		bool done = false;
		for (;;) {
			const char *q = memchr(line, '"', n);
			if (q) {
				size_t i = (size_t)(q - line);
				alt_buf_append(&r->rec, line, i);
				line += i + 1, n -= i + 1;
				col += (int)i + 1;
				int32_t rn = next_rune(line, n);
				if (rn == '"') {
					alt_buf_append_byte(&r->rec, '"');
					line++, n--;
					col++;
				} else if (rn == r->comma) {
					line += comma_len, n -= comma_len;
					col += (int)comma_len;
					rc = add_end(r);
					break; /* the next field */
				} else if (length_nl(line, n) == n) {
					rc = add_end(r);
					done = true;
					break;
				} else if (r->lazy_quotes) {
					alt_buf_append_byte(&r->rec, '"');
				} else {
					rc = perr(r, rec_line, r->num_line, col - 1, quote_err);
					done = true;
					break;
				}
			} else if (n > 0) {
				/* the end of the line: the field goes on */
				alt_buf_append(&r->rec, line, n);
				if (er) {
					done = true;
					break;
				}
				col += (int)n;
				er = read_line(r);
				line = r->line.p ? r->line.p : "";
				n = r->line.n;
				if (n > 0) {
					pline++;
					col = 1;
				}
				if (er == -ALT_EEOF)
					er = 0;
			} else {
				/* the data ended (or an error) */
				if (!r->lazy_quotes && er == 0) {
					rc = perr(r, rec_line, pline, col, quote_err);
					done = true;
					break;
				}
				rc = add_end(r);
				done = true;
				break;
			}
		}
		if (done || rc < 0)
			break;
	}
	if (rc == 0 && er < 0)
		rc = er;
	/* the fields */
	alt_str *f = realloc(r->fields, (r->nends ? r->nends : 1) * sizeof *f);
	if (!f)
		return -ENOMEM;
	r->fields = f;
	size_t prev = 0;
	for (size_t i = 0; i < r->nends; i++) {
		f[i] = (alt_str){r->rec.p ? r->rec.p + prev : "", r->ends[i] - prev};
		prev = r->ends[i];
	}
	*out = f;
	*nout = r->nends;
	if (r->fields_per_record > 0) {
		if ((int)r->nends != r->fields_per_record && rc == 0) {
			snprintf(r->err, sizeof r->err, "record on line %d: wrong number of fields", rec_line);
			rc = -ALT_ECSV;
		}
	} else if (r->fields_per_record == 0)
		r->fields_per_record = (int)r->nends;
	return rc;
}

static bool needs_quotes(const alt_csv_writer *w, int32_t comma, alt_str f) {
	if (f.n == 0)
		return false;
	if (f.n == 2 && f.p[0] == '\\' && f.p[1] == '.')
		return true;
	if (comma < 0x80) {
		for (size_t i = 0; i < f.n; i++) {
			char c = f.p[i];
			if (c == '\n' || c == '\r' || c == '"' || c == (char)comma)
				return true;
		}
	} else {
		if (index_rune(f.p, f.n, comma) >= 0)
			return true;
		for (size_t i = 0; i < f.n; i++)
			if (f.p[i] == '"' || f.p[i] == '\r' || f.p[i] == '\n')
				return true;
	}
	(void)w;
	int32_t c;
	alt_utf8_decode(f.p, f.n, &c);
	return alt_unicode_is_space(c);
}

int alt_csv_write(const alt_csv_writer *w, alt_buf *b, const alt_str *fields, size_t n) {
	int32_t comma = w->comma ? w->comma : ',';
	if (!valid_delim(comma))
		return -EINVAL;
	char cb[4];
	size_t cl = alt_utf8_encode(cb, comma);
	for (size_t k = 0; k < n; k++) {
		if (k)
			alt_buf_append(b, cb, cl);
		alt_str f = fields[k];
		if (!needs_quotes(w, comma, f)) {
			alt_buf_append(b, f.p, f.n);
			continue;
		}
		alt_buf_append_byte(b, '"');
		for (size_t i = 0; i < f.n; i++) {
			char c = f.p[i];
			if (c == '"')
				alt_buf_append(b, "\"\"", 2);
			else if (c == '\r') {
				if (!w->use_crlf)
					alt_buf_append_byte(b, '\r');
			} else if (c == '\n') {
				if (w->use_crlf)
					alt_buf_append(b, "\r\n", 2);
				else
					alt_buf_append_byte(b, '\n');
			} else
				alt_buf_append_byte(b, c);
		}
		alt_buf_append_byte(b, '"');
	}
	if (w->use_crlf)
		alt_buf_append(b, "\r\n", 2);
	else
		alt_buf_append_byte(b, '\n');
	return b->err;
}
