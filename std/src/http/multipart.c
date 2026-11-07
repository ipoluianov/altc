/* multipart bodies after Go's mime/multipart (BSD-3-Clause, see LICENSE-Go) */
#include "http_internal.h"
#include <alt/crypto/hash.h>
#include <alt/mime/multipart.h>
#include <alt/sort.h>
#include <alt/strconv.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	struct alt_multipart_reader *mr;
	size_t n;          /* bytes of the part known to be ready */
	int err;           /* after them: 1 the end of the part, < 0 an error */
	int read_err;
	uint64_t total;
} part_state;

struct alt_multipart_reader {
	alt_bufreader br;
	alt_buf b;         /* "\r\n--" boundary "--" */
	size_t nl;         /* the line end: 2 ("\r\n") or 1 ("\n") */
	int parts_read;
	bool have_part;
	alt_multipart_part part;
	part_state ps;
	alt_qp_reader qp;
	bool qp_on;
	alt_arena arena;   /* the current part's header */
	alt_buf err;
};

static alt_str dash_boundary(const alt_multipart_reader *r) { return alt_str_make(r->b.p + 2, r->b.n - 4); }
static alt_str dash_boundary_dash(const alt_multipart_reader *r) { return alt_str_make(r->b.p + 2, r->b.n - 2); }
static alt_str nl_dash_boundary(const alt_multipart_reader *r) { return alt_str_make(r->b.p + 2 - r->nl, r->b.n - 4 + r->nl); }
static alt_str nl(const alt_multipart_reader *r) { return alt_str_make(r->b.p + 2 - r->nl, r->nl); }

const char *alt_multipart_error(const alt_multipart_reader *r) { return alt_buf_cstr(&r->err); }

static void seterr(alt_multipart_reader *r, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void seterr(alt_multipart_reader *r, const char *fmt, ...) {
	alt_buf_reset(&r->err);
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(&r->err, fmt, ap);
	va_end(ap);
}

int alt_multipart_reader_new(alt_multipart_reader **out, alt_reader rd, alt_str boundary) {
	alt_multipart_reader *r = calloc(1, sizeof *r);
	if (!r)
		return -ENOMEM;
	alt_buf_append_cstr(&r->b, "\r\n--");
	alt_buf_append_str(&r->b, boundary);
	alt_buf_append_cstr(&r->b, "--");
	r->nl = 2;
	alt_arena_init(&r->arena, 2048);
	if (r->b.err || alt_bufreader_init(&r->br, rd, 4096) < 0) {
		alt_buf_free(&r->b);
		free(r);
		return -ENOMEM;
	}
	*out = r;
	return 0;
}

void alt_multipart_reader_free(alt_multipart_reader *r) {
	if (!r)
		return;
	if (r->qp_on)
		alt_qp_reader_free(&r->qp);
	alt_bufreader_free(&r->br);
	alt_buf_free(&r->b);
	alt_arena_free(&r->arena);
	alt_buf_free(&r->err);
	free(r);
}

static bool has_prefix(alt_str s, alt_str p) { return s.n >= p.n && memcmp(s.p, p.p, p.n) == 0; }

/* Go's matchAfterPrefix: -1 not a boundary, 0 need more, +1 a boundary */
static int match_after(alt_str buf, alt_str prefix, int read_err) {
	if (buf.n == prefix.n)
		return read_err ? 1 : 0;
	char c = buf.p[prefix.n];
	if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
		return 1;
	if (c == '-') {
		if (buf.n == prefix.n + 1)
			return read_err ? -1 : 0;
		if (buf.p[prefix.n + 1] == '-')
			return 1;
	}
	return -1;
}

/* Go's scanUntilBoundary: the bytes ready, *err 1 (the part ends after them), a read error or 0 */
static size_t scan(alt_str buf, alt_str db, alt_str ndb, uint64_t total, int read_err, int *err) {
	*err = 0;
	if (total == 0) {
		if (has_prefix(buf, db)) {
			switch (match_after(buf, db, read_err)) {
			case -1: return db.n;
			case 0: return 0;
			default: *err = 1; return 0;
			}
		}
		if (has_prefix(db, buf)) {
			*err = read_err;
			return 0;
		}
	}
	ssize_t i = -1;
	for (size_t k = 0; k + ndb.n <= buf.n; k++)
		if (memcmp(buf.p + k, ndb.p, ndb.n) == 0) {
			i = (ssize_t)k;
			break;
		}
	if (i >= 0) {
		switch (match_after(alt_str_slice(buf, (size_t)i, buf.n), ndb, read_err)) {
		case -1: return (size_t)i + ndb.n;
		case 0: return (size_t)i;
		default: *err = 1; return (size_t)i;
		}
	}
	if (has_prefix(ndb, buf)) {
		*err = read_err;
		return 0;
	}
	ssize_t li = alt_str_last_index_byte(buf, ndb.p[0]);
	if (li >= 0 && has_prefix(ndb, alt_str_slice(buf, (size_t)li, buf.n)))
		return (size_t)li;
	*err = read_err;
	return buf.n;
}

static ssize_t part_read(void *obj, void *d, size_t n) {
	part_state *p = obj;
	alt_multipart_reader *mr = p->mr;
	while (p->n == 0 && p->err == 0) {
		size_t have = alt_bufreader_buffered(&mr->br);
		alt_str peek;
		alt_bufreader_peek(&mr->br, have, &peek);
		p->n = scan(peek, dash_boundary(mr), nl_dash_boundary(mr), p->total, p->read_err, &p->err);
		if (p->n == 0 && p->err == 0) {
			int rc = alt_bufreader_peek(&mr->br, have + 1, &peek);
			if (rc == -ALT_EEOF || rc == -ALT_EUNEXPECTED_EOF)
				p->read_err = -ALT_EUNEXPECTED_EOF;
			else if (rc < 0)
				p->read_err = rc;
		}
	}
	if (p->n == 0)
		return p->err == 1 ? 0 : p->err;
	size_t k = n < p->n ? n : p->n;
	ssize_t got = alt_bufreader_read(&mr->br, d, k);
	if (got < 0)
		return got;
	p->total += (uint64_t)got;
	p->n -= (size_t)got;
	return got;
}

static alt_str skip_lwsp(alt_str s) {
	while (s.n && (s.p[0] == ' ' || s.p[0] == '\t'))
		s.p++, s.n--;
	return s;
}

static bool final_boundary(const alt_multipart_reader *r, alt_str line) {
	alt_str dbd = dash_boundary_dash(r);
	if (!has_prefix(line, dbd))
		return false;
	alt_str rest = skip_lwsp(alt_str_slice(line, dbd.n, line.n));
	return rest.n == 0 || alt_str_eq(rest, nl(r));
}

static bool delimiter_line(alt_multipart_reader *r, alt_str line) {
	alt_str db = dash_boundary(r);
	if (!has_prefix(line, db))
		return false;
	alt_str rest = skip_lwsp(alt_str_slice(line, db.n, line.n));
	if (r->parts_read == 0 && rest.n == 1 && rest.p[0] == '\n')
		r->nl = 1;
	return alt_str_eq(rest, nl(r));
}

static int merr(alt_multipart_reader *r, const char *what, alt_str line) {
	alt_buf q = ALT_BUF_INIT;
	alt_quote(&q, line);
	seterr(r, "multipart: %s; got line %s", what, alt_buf_cstr(&q));
	alt_buf_free(&q);
	return -ALT_EMIME;
}

/* filepath.Base */
static alt_str base_name(alt_str s) {
	while (s.n > 1 && s.p[s.n - 1] == '/')
		s.n--;
	if (s.n == 0)
		return ALT_STR(".");
	ssize_t i = alt_str_last_index_byte(s, '/');
	if (i >= 0 && s.n > 1)
		s = alt_str_slice(s, (size_t)i + 1, s.n);
	return s;
}

/* a line without its \r\n or \n (bufio's ReadLine): 0, -ALT_EEOF with nothing left */
static int read_line(alt_multipart_reader *r, alt_buf *line) {
	alt_buf_reset(line);
	int rc;
	for (;;) {
		alt_str s;
		rc = alt_bufreader_slice(&r->br, '\n', &s);
		alt_buf_append(line, s.p, s.n);
		if (rc != -ALT_EBUFFULL)
			break;
	}
	if (rc == -ALT_EEOF && line->n == 0)
		return -ALT_EEOF;
	if (rc < 0 && rc != -ALT_EEOF)
		return rc;
	if (line->n && line->p[line->n - 1] == '\n') {
		alt_buf_truncate(line, line->n - 1);
		if (line->n && line->p[line->n - 1] == '\r')
			alt_buf_truncate(line, line->n - 1);
	}
	return 0;
}

static bool value_byte(unsigned char c) { return c >= 0x80 || (c >= 0x20 && c != 0x7f) || c == '\t'; }

/* textproto's readMIMEHeader: 0 at the empty line; -ALT_EEOF when the data
   ends (NextPart then ends, as Go's); -ALT_EMIME for malformed lines */
static int read_mime_header(alt_multipart_reader *r, alt_http_header *h) {
	alt_buf line = ALT_BUF_INIT, kv = ALT_BUF_INIT;
	alt_str peek;
	int rc = 0;
	if (alt_bufreader_peek(&r->br, 1, &peek) == 0 && (peek.p[0] == ' ' || peek.p[0] == '\t')) {
		rc = read_line(r, &line);
		if (rc == 0) {
			seterr(r, "malformed MIME header initial line: %.*s", (int)(line.n < 80 ? line.n : 80), line.p);
			rc = -ALT_EMIME;
		}
		goto out;
	}
	for (int count = 0;; count++) {
		if ((rc = read_line(r, &line)) < 0)
			goto out;
		if (line.n == 0)
			break;
		if (!memchr(line.p, ':', line.n)) {
			alt_buf q = ALT_BUF_INIT;
			alt_quote(&q, alt_buf_str(&line));
			seterr(r, "malformed MIME header: missing colon: %s", alt_buf_cstr(&q));
			alt_buf_free(&q);
			rc = -ALT_EMIME;
			goto out;
		}
		alt_buf_reset(&kv);
		alt_buf_append_str(&kv, alt_str_trim(alt_buf_str(&line), " \t"));
		/* continuation lines */
		while (alt_bufreader_peek(&r->br, 1, &peek) == 0 && (peek.p[0] == ' ' || peek.p[0] == '\t')) {
			if (read_line(r, &line) < 0)
				break;
			alt_buf_append_byte(&kv, ' ');
			alt_buf_append_str(&kv, alt_str_trim(alt_buf_str(&line), " \t"));
		}
		alt_str k, v;
		alt_str_cut_byte(alt_buf_str(&kv), ':', &k, &v);
		bool ok = k.n > 0;
		for (size_t i = 0; i < k.n && ok; i++)
			ok = alt_http_valid_field_name(alt_str_make(k.p + i, 1)) || k.p[i] == ' ';
		for (size_t i = 0; i < v.n && ok; i++)
			ok = value_byte((unsigned char)v.p[i]);
		if (!ok) {
			seterr(r, "malformed MIME header line: %.*s", (int)kv.n, kv.p);
			rc = -ALT_EMIME;
			goto out;
		}
		if (count >= 10000) {
			seterr(r, "multipart: message too large");
			rc = -ALT_EMIME;
			goto out;
		}
		if ((rc = alt_http_header_add(h, k, alt_str_trim_left(v, " \t"))) < 0)
			goto out;
	}
out:
	alt_buf_free(&line);
	alt_buf_free(&kv);
	return rc;
}

int alt_multipart_next_part(alt_multipart_reader *r, alt_multipart_part **out) {
	alt_buf_reset(&r->err);
	if (r->have_part) {
		/* the rest of the current part */
		char tmp[4096];
		while (alt_read(r->part.body, tmp, sizeof tmp) > 0) {
		}
		r->have_part = false;
	}
	if (r->qp_on) {
		alt_qp_reader_free(&r->qp);
		r->qp_on = false;
	}
	if (r->b.n == 6) {
		seterr(r, "multipart: boundary is empty");
		return -ALT_EMIME;
	}
	bool expect_new = false;
	for (;;) {
		alt_str line;
		int rc = alt_bufreader_slice(&r->br, '\n', &line);
		if (rc == -ALT_EEOF && final_boundary(r, line))
			return -ALT_EEOF;
		if (rc < 0) {
			seterr(r, "multipart: NextPart: %s",
				rc == -ALT_EEOF ? "EOF" : rc == -ALT_EBUFFULL ? "bufio: buffer full" : alt_strerror(rc));
			return rc == -ALT_EEOF ? -ALT_EMIME : rc == -ALT_EBUFFULL ? -ALT_EMIME : rc;
		}
		if (delimiter_line(r, line)) {
			r->parts_read++;
			alt_arena_reset(&r->arena);
			memset(&r->part, 0, sizeof r->part);
			alt_http_header_init(&r->part.header, &r->arena);
			if ((rc = read_mime_header(r, &r->part.header)) < 0)
				return rc;
			r->ps = (part_state){.mr = r};
			r->part.body = (alt_reader){part_read, &r->ps};
			if (alt_str_eq_fold(alt_http_header_value(&r->part.header, "Content-Transfer-Encoding"), ALT_STR("quoted-printable"))) {
				alt_http_header_del(&r->part.header, ALT_STR("Content-Transfer-Encoding"));
				alt_qp_reader_init(&r->qp, r->part.body);
				r->qp_on = true;
				r->part.body = alt_qp_reader_of(&r->qp);
			}
			/* Content-Disposition */
			alt_str disp;
			alt_mime_param *ps;
			size_t np;
			if (alt_mime_parse_media_type(&r->arena, alt_http_header_value(&r->part.header, "Content-Disposition"), &disp, &ps, &np,
				    0) == 0) {
				if (alt_str_eq_cstr(disp, "form-data"))
					r->part.form_name = alt_mime_param_get(ps, np, "name");
				alt_str fn = alt_mime_param_get(ps, np, "filename");
				if (fn.n)
					r->part.file_name = base_name(fn);
			}
			r->have_part = true;
			*out = &r->part;
			return 0;
		}
		if (final_boundary(r, line))
			return -ALT_EEOF;
		if (expect_new)
			return merr(r, "expecting a new Part", line);
		if (r->parts_read == 0)
			continue;
		if (alt_str_eq(line, nl(r))) {
			expect_new = true;
			continue;
		}
		alt_buf q = ALT_BUF_INIT;
		alt_quote(&q, line);
		seterr(r, "multipart: unexpected line in Next(): %s", alt_buf_cstr(&q));
		alt_buf_free(&q);
		return -ALT_EMIME;
	}
}

/* ---- forms ---- */

int alt_multipart_read_form(alt_multipart_reader *r, alt_arena *a, size_t max, alt_multipart_form *f) {
	memset(f, 0, sizeof *f);
	f->arena = a;
	alt_url_values_init(&f->values, a);
	if (!max)
		max = 32 << 20;
	size_t total = 0, cap = 0;
	for (;;) {
		alt_multipart_part *p;
		int rc = alt_multipart_next_part(r, &p);
		if (rc == -ALT_EEOF)
			return 0;
		if (rc < 0)
			return rc;
		if (!p->form_name.n)
			continue;
		alt_buf data = ALT_BUF_INIT;
		ssize_t k = alt_read_all(p->body, &data, max - total + 1);
		if (k < 0) {
			alt_buf_free(&data);
			return k == -ALT_ETOOLARGE ? -ALT_EHTTP_BODYTOOLARGE : (int)k;
		}
		total += data.n;
		if (total > max) {
			alt_buf_free(&data);
			return -ALT_EHTTP_BODYTOOLARGE;
		}
		alt_str v = alt_arena_str(a, alt_buf_str(&data));
		alt_buf_free(&data);
		alt_str name = alt_arena_str(a, p->form_name);
		if (!p->file_name.n) {
			alt_url_values_add(&f->values, name, v);
			continue;
		}
		if (f->nfiles == cap) {
			size_t nc = cap ? cap * 2 : 4;
			alt_multipart_file *nf = alt_arena_realloc(a, f->files, cap * sizeof *nf, nc * sizeof *nf);
			if (!nf)
				return -ENOMEM;
			f->files = nf;
			cap = nc;
		}
		alt_multipart_file *mf = &f->files[f->nfiles++];
		memset(mf, 0, sizeof *mf);
		mf->field = name;
		mf->file_name = alt_arena_str(a, p->file_name);
		mf->data = v;
		alt_http_header_init(&mf->header, a);
		for (size_t i = 0; i < p->header.n; i++)
			alt_http_header_add(&mf->header, p->header.f[i].key, p->header.f[i].value);
		mf->content_type = alt_http_header_value(&mf->header, "Content-Type");
	}
}

const alt_multipart_file *alt_multipart_form_file(const alt_multipart_form *f, const char *field) {
	for (size_t i = 0; i < f->nfiles; i++)
		if (alt_str_eq_cstr(f->files[i].field, field))
			return &f->files[i];
	return 0;
}

static void form_free(void *arg) { alt_multipart_reader_free(arg); }

int alt_http_parse_multipart_form(alt_http_request *r, size_t max, const alt_multipart_form **form) {
	alt_str ct = alt_http_header_value(&r->header, "Content-Type"), mt;
	alt_mime_param *ps;
	size_t np;
	if (alt_mime_parse_media_type(r->arena, ct, &mt, &ps, &np, 0) < 0 ||
		(!alt_str_eq_cstr(mt, "multipart/form-data") && !alt_str_eq_cstr(mt, "multipart/mixed")))
		return -ALT_EMIME; /* Go's ErrNotMultipart */
	alt_str boundary = alt_mime_param_get(ps, np, "boundary");
	if (!boundary.n)
		return -ALT_EMIME; /* ErrMissingBoundary */
	alt_multipart_reader *mr;
	int rc = alt_multipart_reader_new(&mr, r->body, boundary);
	if (rc < 0)
		return rc;
	alt_arena_defer(r->arena, form_free, mr);
	alt_multipart_form *f = alt_arena_zalloc(r->arena, sizeof *f);
	if (!f)
		return -ENOMEM;
	rc = alt_multipart_read_form(mr, r->arena, max, f);
	if (rc == 0)
		*form = f;
	return rc;
}

/* ---- the writer ---- */

int alt_multipart_writer_init(alt_multipart_writer *m, alt_writer w) {
	memset(m, 0, sizeof *m);
	m->w = w;
	uint8_t r[30];
	int rc = alt_crypto_rand(r, 30);
	if (rc < 0)
		return rc;
	for (int i = 0; i < 30; i++)
		snprintf(m->boundary + 2 * i, 3, "%02x", r[i]);
	return 0;
}

int alt_multipart_writer_set_boundary(alt_multipart_writer *m, alt_str b) {
	if (m->started || b.n < 1 || b.n > 70)
		return -EINVAL;
	for (size_t i = 0; i < b.n; i++) {
		char c = b.p[i];
		bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || strchr("'()+_,-./:=?", c) ||
			  (c == ' ' && i + 1 < b.n);
		if (!ok || !c)
			return -EINVAL;
	}
	memcpy(m->boundary, b.p, b.n);
	m->boundary[b.n] = 0;
	return 0;
}

int alt_multipart_form_data_content_type(const alt_multipart_writer *m, alt_buf *b) {
	/* quoted when the boundary has tspecials (Go's FormDataContentType) */
	bool q = strpbrk(m->boundary, "()<>@,;:\\\"/[]?= ") != 0;
	return alt_buf_printf(b, q ? "multipart/form-data; boundary=\"%s\"" : "multipart/form-data; boundary=%s", m->boundary);
}

static int put(alt_multipart_writer *m, const void *p, size_t n) {
	if (m->err)
		return m->err;
	ssize_t k = alt_write_all(m->w, p, n);
	if (k < 0)
		m->err = (int)k;
	return k < 0 ? (int)k : 0;
}

static int cmp_field(const void *a, const void *b, void *arg) {
	(void)arg;
	return alt_str_cmp(((const alt_http_field *)a)->key, ((const alt_http_field *)b)->key);
}

int alt_multipart_create_part(alt_multipart_writer *m, const alt_http_header *h) {
	alt_buf b = ALT_BUF_INIT;
	alt_buf_printf(&b, m->started ? "\r\n--%s\r\n" : "--%s\r\n", m->boundary);
	m->started = true;
	if (h && h->n) {
		alt_http_field *f = malloc(h->n * sizeof *f);
		if (!f) {
			alt_buf_free(&b);
			return -ENOMEM;
		}
		memcpy(f, h->f, h->n * sizeof *f);
		alt_sort_stable(f, h->n, sizeof *f, cmp_field, 0);
		for (size_t i = 0; i < h->n; i++)
			alt_buf_printf(&b, "%.*s: %.*s\r\n", (int)f[i].key.n, f[i].key.p, (int)f[i].value.n, f[i].value.p);
		free(f);
	}
	alt_buf_append_cstr(&b, "\r\n");
	int rc = b.err ? b.err : put(m, b.p, b.n);
	alt_buf_free(&b);
	return rc;
}

static void escape_quotes(alt_buf *b, alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		switch (s.p[i]) {
		case '\\': alt_buf_append_cstr(b, "\\\\"); break;
		case '"': alt_buf_append_cstr(b, "\\\""); break;
		case '\r': alt_buf_append_cstr(b, "%0D"); break;
		case '\n': alt_buf_append_cstr(b, "%0A"); break;
		default: alt_buf_append_byte(b, s.p[i]);
		}
}

int alt_multipart_create_form_field(alt_multipart_writer *m, alt_str name) {
	alt_arena a;
	alt_arena_init(&a, 512);
	alt_http_header h;
	alt_http_header_init(&h, &a);
	alt_buf v = ALT_BUF_INIT;
	alt_buf_append_cstr(&v, "form-data; name=\"");
	escape_quotes(&v, name);
	alt_buf_append_byte(&v, '"');
	alt_http_header_set(&h, ALT_STR("Content-Disposition"), alt_buf_str(&v));
	int rc = alt_multipart_create_part(m, &h);
	alt_buf_free(&v);
	alt_arena_free(&a);
	return rc;
}

int alt_multipart_create_form_file(alt_multipart_writer *m, alt_str field, alt_str file_name) {
	alt_arena a;
	alt_arena_init(&a, 512);
	alt_http_header h;
	alt_http_header_init(&h, &a);
	alt_buf v = ALT_BUF_INIT;
	alt_buf_append_cstr(&v, "form-data; name=\"");
	escape_quotes(&v, field);
	alt_buf_append_cstr(&v, "\"; filename=\"");
	escape_quotes(&v, file_name);
	alt_buf_append_byte(&v, '"');
	alt_http_header_set(&h, ALT_STR("Content-Disposition"), alt_buf_str(&v));
	alt_http_header_set(&h, ALT_STR("Content-Type"), ALT_STR("application/octet-stream"));
	int rc = alt_multipart_create_part(m, &h);
	alt_buf_free(&v);
	alt_arena_free(&a);
	return rc;
}

int alt_multipart_write_field(alt_multipart_writer *m, alt_str name, alt_str value) {
	int rc = alt_multipart_create_form_field(m, name);
	return rc ? rc : put(m, value.p, value.n);
}

int alt_multipart_writer_close(alt_multipart_writer *m) {
	char b[96];
	int n = snprintf(b, sizeof b, "\r\n--%s--\r\n", m->boundary);
	return put(m, b, (size_t)n);
}
