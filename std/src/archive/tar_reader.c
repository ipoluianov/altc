/* The tar reader, following Go's archive/tar reader.go (BSD-3-Clause,
   see LICENSE-Go): the same checks in the same order, so the same
   headers, data and errors. */
#include "tar_internal.h"
#include <alt/path.h>

/* errors of Go's that have no code of their own: reported as
   -ALT_ETAR_HEADER or -ALT_ETAR_FIELD with their own text */
enum {
	E_EOF = -ALT_EEOF,
	E_MISS_DATA = -(ALT_ELAST + 1),  /* sparse file references non-existent data */
	E_UNREF_DATA = -(ALT_ELAST + 2), /* sparse file contains unreferenced data */
	E_SPARSE_LONG = -(ALT_ELAST + 3), /* sparse map too long */
};

typedef struct {
	int64_t off, len;
} spent;

struct alt_tar_reader {
	alt_reader src;
	int64_t pad;     /* padding after the current entry */
	int64_t nb;      /* bytes of the entry left in the archive */
	bool sparse;     /* the entry is read through its holes */
	spent *sp;       /* holes sp[sp0..nsp) (the last one always stays) */
	size_t sp0, nsp, spcap;
	int64_t pos;     /* position in the sparse file */
	uint8_t blk[TAR_BLOCK];
	int err;         /* sticky */
	int last;        /* the error last returned */
	bool check_paths;
	alt_arena a;     /* the strings of the current header */
	alt_tar_header hdr;
	alt_buf tmp;
};

static int public_code(int e) {
	switch (e) {
	case E_MISS_DATA:
	case E_UNREF_DATA: return -ALT_ETAR_HEADER;
	case E_SPARSE_LONG: return -ALT_ETAR_FIELD;
	}
	return e;
}

const char *alt_tar_reader_error(const alt_tar_reader *r) {
	switch (r->last) {
	case 0: return "";
	case E_EOF: return "EOF";
	case -ALT_EUNEXPECTED_EOF: return "unexpected EOF";
	case E_MISS_DATA: return "archive/tar: sparse file references non-existent data";
	case E_UNREF_DATA: return "archive/tar: sparse file contains unreferenced data";
	case E_SPARSE_LONG: return "archive/tar: sparse map too long";
	}
	return alt_strerror(r->last);
}

int alt_tar_reader_new(alt_tar_reader **out, alt_reader src) {
	alt_tar_reader *r = calloc(1, sizeof *r);
	*out = r;
	if (!r)
		return -ENOMEM;
	r->src = src;
	alt_arena_init(&r->a, 0);
	return 0;
}

void alt_tar_reader_free(alt_tar_reader *r) {
	if (!r)
		return;
	alt_arena_free(&r->a);
	alt_buf_free(&r->tmp);
	free(r->sp);
	free(r);
}

void alt_tar_reader_check_paths(alt_tar_reader *r, bool on) { r->check_paths = on; }

/* ---- Go-style reads: a count and an error (E_EOF at the end) ---- */

typedef ssize_t (*readfn)(alt_tar_reader *r, void *b, size_t n, int *err);

static ssize_t src_read(alt_tar_reader *r, void *b, size_t n, int *err) {
	ssize_t k = alt_read(r->src, b, n);
	*err = k < 0 ? (int)k : k == 0 ? E_EOF : 0;
	return k < 0 ? 0 : k;
}

/* the entry's bytes in the archive (regFileReader) */
static ssize_t reg_read(alt_tar_reader *r, void *b, size_t n, int *err) {
	if ((uint64_t)n > (uint64_t)r->nb)
		n = (size_t)r->nb;
	ssize_t k = 0;
	*err = 0;
	if (n > 0) {
		k = src_read(r, b, n, err);
		r->nb -= k;
	}
	if (*err == E_EOF && r->nb > 0)
		*err = -ALT_EUNEXPECTED_EOF;
	else if (*err == 0 && r->nb == 0)
		*err = E_EOF;
	return k;
}

/* io.ReadFull that keeps EOF when the data ends early (tryReadFull) */
static ssize_t try_read_full(alt_tar_reader *r, readfn fn, void *b, size_t n, int *err) {
	size_t got = 0;
	*err = 0;
	while (n > got && *err == 0)
		got += (size_t)fn(r, (char *)b + got, n - got, err);
	if (n == got && *err == E_EOF)
		*err = 0;
	return (ssize_t)got;
}

static int must_read_full(alt_tar_reader *r, readfn fn, void *b, size_t n) {
	int err;
	try_read_full(r, fn, b, n, &err);
	return err == E_EOF ? -ALT_EUNEXPECTED_EOF : err;
}

static int64_t logical_remaining(const alt_tar_reader *r) {
	if (!r->sparse)
		return r->nb;
	const spent *e = &r->sp[r->nsp - 1];
	return e->off + e->len - r->pos;
}

/* the expanded data of a sparse file (sparseFileReader) */
static ssize_t sparse_read(alt_tar_reader *r, void *buf, size_t n, int *err) {
	bool finished = (uint64_t)n >= (uint64_t)logical_remaining(r);
	if (finished)
		n = (size_t)logical_remaining(r);
	char *b = buf;
	size_t left = n;
	int64_t end = r->pos + (int64_t)n;
	*err = 0;
	while (end > r->pos && *err == 0) {
		int64_t hs = r->sp[r->sp0].off, he = hs + r->sp[r->sp0].len;
		ssize_t nf;
		if (r->pos < hs) {
			size_t m = (uint64_t)left < (uint64_t)(hs - r->pos) ? left : (size_t)(hs - r->pos);
			nf = try_read_full(r, reg_read, b, m, err);
		} else {
			size_t m = (uint64_t)left < (uint64_t)(he - r->pos) ? left : (size_t)(he - r->pos);
			memset(b, 0, m);
			nf = (ssize_t)m;
		}
		b += nf;
		left -= (size_t)nf;
		r->pos += nf;
		if (r->pos >= he && r->nsp - r->sp0 > 1)
			r->sp0++;
	}
	ssize_t k = (ssize_t)(n - left);
	if (*err == E_EOF)
		*err = E_MISS_DATA;
	else if (*err == 0) {
		if (logical_remaining(r) == 0 && r->nb > 0)
			*err = E_UNREF_DATA;
		else if (finished)
			*err = E_EOF;
	}
	return k;
}

static ssize_t curr_read(alt_tar_reader *r, void *b, size_t n, int *err) {
	return r->sparse ? sparse_read(r, b, n, err) : reg_read(r, b, n, err);
}

/* Reader.Read: errors other than EOF stay */
static ssize_t tr_read(alt_tar_reader *r, void *b, size_t n, int *err) {
	if (r->err) {
		*err = r->err;
		return 0;
	}
	ssize_t k = curr_read(r, b, n, err);
	if (*err && *err != E_EOF)
		r->err = *err;
	return k;
}

static int discard(alt_tar_reader *r, int64_t n) {
	char buf[8192];
	int64_t got = 0;
	while (got < n) {
		size_t m = (uint64_t)(n - got) < sizeof buf ? (size_t)(n - got) : sizeof buf;
		int err;
		got += src_read(r, buf, m, &err);
		if (err == E_EOF)
			return got < n ? -ALT_EUNEXPECTED_EOF : 0;
		if (err)
			return err;
	}
	return 0;
}

/* the data of a PAX or GNU long name entry (readSpecialFile) into r->tmp */
static int read_special(alt_tar_reader *r) {
	alt_buf_reset(&r->tmp);
	size_t limit = TAR_MAX_SPECIAL + 1;
	int err = 0;
	while (r->tmp.n < limit) {
		size_t want = limit - r->tmp.n < 32768 ? limit - r->tmp.n : 32768;
		if (alt_buf_reserve(&r->tmp, want) < 0)
			return -ENOMEM;
		ssize_t k = tr_read(r, r->tmp.p + r->tmp.n, want, &err);
		r->tmp.n += (size_t)k;
		r->tmp.p[r->tmp.n] = 0;
		if (err)
			break;
	}
	if (r->tmp.n > TAR_MAX_SPECIAL)
		return -ALT_ETAR_FIELD;
	return err == E_EOF ? 0 : err;
}

static alt_str keep(alt_tar_reader *r, alt_str s, int *err) {
	alt_str c = alt_arena_str(&r->a, s);
	if (!c.p)
		*err = -ENOMEM;
	return c;
}

/* ---- headers ---- */

typedef struct {
	alt_tar_record *r;
	size_t n, cap;
} recs;

static int recs_add(alt_tar_reader *r, recs *v, alt_str k, alt_str val) {
	if (v->n == v->cap) {
		size_t cap = v->cap ? v->cap * 2 : 16;
		alt_tar_record *p = alt_arena_realloc(&r->a, v->r, v->cap * sizeof *p, cap * sizeof *p);
		if (!p)
			return -ENOMEM;
		v->r = p;
		v->cap = cap;
	}
	v->r[v->n++] = (alt_tar_record){k, val};
	return 0;
}

/* parsePAX: the records of an 'x' or 'g' entry, sorted, unique */
static int parse_pax(alt_tar_reader *r, recs *out) {
	int rc = read_special(r);
	if (rc < 0)
		return rc;
	alt_str s = keep(r, alt_buf_str(&r->tmp), &rc);
	if (rc < 0)
		return rc;
	recs v = {0, 0, 0};
	alt_buf smap = ALT_BUF_INIT;
	size_t nsmap = 0;
	while (s.n > 0) {
		alt_str k, val;
		if (alt__tar_parse_pax_record(s, &k, &val, &s) < 0) {
			alt_buf_free(&smap);
			return -ALT_ETAR_HEADER;
		}
		bool off = alt_str_eq_cstr(k, "GNU.sparse.offset"), nbytes = alt_str_eq_cstr(k, "GNU.sparse.numbytes");
		if (off || nbytes) {
			/* the sparse map of format 0.0: pairs of these records */
			if ((nsmap % 2 == 0 && !off) || (nsmap % 2 == 1 && !nbytes) || alt_str_index_byte(val, ',') >= 0) {
				alt_buf_free(&smap);
				return -ALT_ETAR_HEADER;
			}
			if (nsmap++)
				alt_buf_append_byte(&smap, ',');
			alt_buf_append_str(&smap, val);
		} else if ((rc = recs_add(r, &v, k, val)) < 0) {
			alt_buf_free(&smap);
			return rc;
		}
	}
	if (nsmap) {
		/* after the other records so that it replaces an explicit one */
		alt_str m = keep(r, alt_buf_str(&smap), &rc);
		if (rc == 0)
			rc = recs_add(r, &v, ALT_STR("GNU.sparse.map"), m);
	}
	alt_buf_free(&smap);
	if (rc < 0)
		return rc;
	v.n = alt__tar_sort_records(v.r, v.n);
	*out = v;
	return 0;
}

static alt_str rec_get(const recs *v, const char *key) {
	const alt_tar_record *p = v ? alt__tar_find_record(v->r, v->n, alt_str_cstr(key)) : 0;
	return p ? p->value : ALT_STR("");
}

/* mergePAX: the records override the header's fields */
static int merge_pax(alt_tar_header *h, const recs *v) {
	if (!v) {
		h->pax = 0;
		h->npax = 0;
		return 0;
	}
	for (size_t i = 0; i < v->n; i++) {
		alt_str k = v->r[i].key, val = v->r[i].value;
		if (val.n == 0)
			continue; /* keep the USTAR value */
		int rc = 0;
		if (alt_str_eq_cstr(k, "path"))
			h->name = val;
		else if (alt_str_eq_cstr(k, "linkpath"))
			h->linkname = val;
		else if (alt_str_eq_cstr(k, "uname"))
			h->uname = val;
		else if (alt_str_eq_cstr(k, "gname"))
			h->gname = val;
		else if (alt_str_eq_cstr(k, "uid"))
			rc = alt__tar_atoi(val, &h->uid);
		else if (alt_str_eq_cstr(k, "gid"))
			rc = alt__tar_atoi(val, &h->gid);
		else if (alt_str_eq_cstr(k, "atime"))
			rc = alt__tar_parse_pax_time(val, &h->atime);
		else if (alt_str_eq_cstr(k, "mtime"))
			rc = alt__tar_parse_pax_time(val, &h->mtime);
		else if (alt_str_eq_cstr(k, "ctime"))
			rc = alt__tar_parse_pax_time(val, &h->ctime);
		else if (alt_str_eq_cstr(k, "size"))
			rc = alt__tar_atoi(val, &h->size);
		if (rc < 0)
			return -ALT_ETAR_HEADER;
	}
	h->pax = v->r;
	h->npax = v->n;
	return 0;
}

static bool header_only(int flag) { return flag >= ALT_TAR_LINK && flag <= ALT_TAR_FIFO; }

static bool block_zero(const uint8_t *b) {
	for (int i = 0; i < TAR_BLOCK; i++)
		if (b[i])
			return false;
	return true;
}

static int read_block(alt_tar_reader *r) {
	ssize_t k = alt_read_full(r->src, r->blk, TAR_BLOCK);
	return k == 0 ? E_EOF : k < 0 ? (int)k : 0;
}

static alt_tar_time unix_time(int64_t sec) { return (alt_tar_time){sec, 0, true}; }

/* readHeader: the next header block into h */
static int read_header(alt_tar_reader *r, alt_tar_header *h) {
	int rc = read_block(r);
	if (rc < 0)
		return rc;
	if (block_zero(r->blk)) {
		if ((rc = read_block(r)) < 0)
			return rc;
		return block_zero(r->blk) ? E_EOF : -ALT_ETAR_HEADER;
	}
	int format = alt__tar_get_format(r->blk);
	if (format == ALT_TAR_FORMAT_UNKNOWN)
		return -ALT_ETAR_HEADER;
	const uint8_t *b = r->blk;
	int perr = 0, kerr = 0;
	memset(h, 0, sizeof *h);
	h->typeflag = b[156];
	h->name = keep(r, alt__tar_parse_string(FLD(b, F_NAME)), &kerr);
	h->linkname = keep(r, alt__tar_parse_string(FLD(b, F_LINK)), &kerr);
	h->size = alt__tar_parse_numeric(FLD(b, F_SIZE), &perr);
	h->mode = alt__tar_parse_numeric(FLD(b, F_MODE), &perr);
	h->uid = alt__tar_parse_numeric(FLD(b, F_UID), &perr);
	h->gid = alt__tar_parse_numeric(FLD(b, F_GID), &perr);
	h->mtime = unix_time(alt__tar_parse_numeric(FLD(b, F_MTIME), &perr));
	if (format > ALT_TAR_FORMAT_V7) {
		h->uname = keep(r, alt__tar_parse_string(FLD(b, F_UNAME)), &kerr);
		h->gname = keep(r, alt__tar_parse_string(FLD(b, F_GNAME)), &kerr);
		h->devmajor = alt__tar_parse_numeric(FLD(b, F_DEVMAJOR), &perr);
		h->devminor = alt__tar_parse_numeric(FLD(b, F_DEVMINOR), &perr);
		alt_str prefix = ALT_STR("");
		if (format & (ALT_TAR_FORMAT_USTAR | ALT_TAR_FORMAT_PAX)) {
			h->format = format;
			prefix = alt__tar_parse_string(FLD(b, F_PREFIX));
			for (int i = 0; i < TAR_BLOCK; i++)
				if (b[i] >= 0x80) {
					h->format = ALT_TAR_FORMAT_UNKNOWN; /* non-ASCII characters in the block */
					break;
				}
			static const int nuls[] = {124 + 11, 100 + 7, 108 + 7, 116 + 7, 136 + 11, 329 + 7, 337 + 7};
			for (int i = 0; i < 7; i++)
				if (b[nuls[i]] != 0)
					h->format = ALT_TAR_FORMAT_UNKNOWN; /* numeric fields must end in NUL */
		} else if (format & ALT_TAR_FORMAT_STAR) {
			prefix = alt__tar_parse_string(FLD(b, F_STAR_PREFIX));
			h->atime = unix_time(alt__tar_parse_numeric(FLD(b, F_STAR_ATIME), &perr));
			h->ctime = unix_time(alt__tar_parse_numeric(FLD(b, F_STAR_CTIME), &perr));
		} else if (format & ALT_TAR_FORMAT_GNU) {
			h->format = format;
			int p2 = 0;
			if (b[345])
				h->atime = unix_time(alt__tar_parse_numeric(FLD(b, F_GNU_ATIME), &p2));
			if (b[357])
				h->ctime = unix_time(alt__tar_parse_numeric(FLD(b, F_GNU_CTIME), &p2));
			if (p2) {
				/* old GNU tar wrote a USTAR prefix here: not GNU */
				h->atime = h->ctime = (alt_tar_time){0, 0, false};
				alt_str s = alt__tar_parse_string(FLD(b, F_PREFIX));
				if (alt__tar_is_ascii(s))
					prefix = s;
				h->format = ALT_TAR_FORMAT_UNKNOWN;
			}
		}
		if (prefix.n > 0) {
			alt_str full = alt_str_make(alt_arena_printf(&r->a, "%.*s/%.*s", (int)prefix.n, prefix.p, (int)h->name.n, h->name.p),
				prefix.n + 1 + h->name.n);
			if (!full.p)
				kerr = -ENOMEM;
			h->name = full;
		}
	}
	return kerr ? kerr : perr;
}

static void set_reg(alt_tar_reader *r, int64_t nb) {
	r->nb = nb;
	r->sparse = false;
}

static int handle_regular(alt_tar_reader *r, const alt_tar_header *h) {
	int64_t nb = header_only(h->typeflag) ? 0 : h->size;
	if (nb < 0)
		return -ALT_ETAR_HEADER;
	r->pad = tar_block_padding(nb);
	set_reg(r, nb);
	return 0;
}

/* the sparse data map (offset/length pairs), built in r->sp */
static int sp_add(alt_tar_reader *r, int64_t off, int64_t len) {
	if (r->nsp >= TAR_MAX_SPARSE)
		return E_SPARSE_LONG;
	if (r->nsp == r->spcap) {
		size_t cap = r->spcap ? r->spcap * 2 : 16;
		spent *p = realloc(r->sp, cap * sizeof *p);
		if (!p)
			return -ENOMEM;
		r->sp = p;
		r->spcap = cap;
	}
	r->sp[r->nsp++] = (spent){off, len};
	return 0;
}

static int read_old_gnu_sparse(alt_tar_reader *r, alt_tar_header *h) {
	if (alt__tar_get_format(r->blk) != ALT_TAR_FORMAT_GNU)
		return -ALT_ETAR_HEADER;
	int perr = 0;
	h->size = alt__tar_parse_numeric(FLD(r->blk, F_GNU_REALSIZE), &perr);
	if (perr)
		return perr;
	const uint8_t *s = r->blk + 386;
	size_t entries = 4, total = 97;
	while (total < TAR_MAX_SPECIAL) {
		for (size_t i = 0; i < entries; i++) {
			const uint8_t *e = s + i * 24;
			if (e[0] == 0)
				break; /* the extended headers still follow */
			int64_t off = alt__tar_parse_numeric(e, 12, &perr);
			int64_t len = alt__tar_parse_numeric(e + 12, 12, &perr);
			if (perr)
				return perr;
			int rc = sp_add(r, off, len);
			if (rc < 0)
				return rc;
		}
		if (s[24 * entries] > 0) {
			int rc = must_read_full(r, src_read, r->blk, TAR_BLOCK);
			if (rc < 0)
				return rc;
			s = r->blk;
			entries = 21;
			total += TAR_BLOCK;
			continue;
		}
		return 0;
	}
	return E_SPARSE_LONG;
}

/* format 1.0: the map is at the start of the data, in decimal lines */
static int read_sparse_map_1x0(alt_tar_reader *r) {
	alt_buf *buf = &r->tmp;
	alt_buf_reset(buf);
	size_t rd = 0;
	int64_t newlines = 0;
	size_t total = 0;
	uint8_t blk[TAR_BLOCK];
#define FEED(want) \
	do { \
		while (newlines < (want)) { \
			total += TAR_BLOCK; \
			if (total > TAR_MAX_SPECIAL) \
				return E_SPARSE_LONG; \
			int rc_ = must_read_full(r, reg_read, blk, TAR_BLOCK); \
			if (rc_ < 0) \
				return rc_; \
			if (alt_buf_append(buf, blk, TAR_BLOCK) < 0) \
				return -ENOMEM; \
			for (int i_ = 0; i_ < TAR_BLOCK; i_++) \
				newlines += blk[i_] == '\n'; \
		} \
	} while (0)
	FEED(1);
	alt_str tok;
#define NEXT_TOKEN() \
	do { \
		newlines--; \
		alt_str rest_ = alt_str_from(alt_buf_str(buf), rd); \
		ssize_t nl_ = alt_str_index_byte(rest_, '\n'); \
		size_t len_ = nl_ < 0 ? rest_.n : (size_t)nl_; \
		tok = alt_str_make(rest_.p, len_); \
		rd += nl_ < 0 ? len_ : len_ + 1; \
	} while (0)
	NEXT_TOKEN();
	int64_t n;
	if (alt__tar_atoi(tok, &n) < 0 || n < 0 || (int64_t)((uint64_t)n * 2) < n)
		return -ALT_ETAR_HEADER;
	FEED(2 * n);
	for (int64_t i = 0; i < n; i++) {
		int64_t off, len;
		NEXT_TOKEN();
		int e1 = alt__tar_atoi(tok, &off);
		NEXT_TOKEN();
		int e2 = alt__tar_atoi(tok, &len);
		if (e1 < 0 || e2 < 0)
			return -ALT_ETAR_HEADER;
		int rc = sp_add(r, off, len);
		if (rc < 0)
			return rc;
	}
	return 0;
#undef FEED
#undef NEXT_TOKEN
}

/* formats 0.0 and 0.1: the map is in the records */
static int read_sparse_map_0x1(alt_tar_reader *r, const recs *v) {
	int64_t n;
	if (alt__tar_atoi(rec_get(v, "GNU.sparse.numblocks"), &n) < 0 || n < 0 || (int64_t)((uint64_t)n * 2) < n)
		return -ALT_ETAR_HEADER;
	alt_str m = rec_get(v, "GNU.sparse.map");
	int64_t count = m.n ? (int64_t)alt_str_count(m, ALT_STR(",")) + 1 : 0;
	if (count != 2 * n)
		return -ALT_ETAR_HEADER;
	while (m.n) {
		ssize_t c = alt_str_index_byte(m, ',');
		alt_str a = alt_str_make(m.p, (size_t)c);
		m = alt_str_from(m, (size_t)c + 1);
		c = alt_str_index_byte(m, ',');
		alt_str b = c < 0 ? m : alt_str_make(m.p, (size_t)c);
		m = c < 0 ? ALT_STR("") : alt_str_from(m, (size_t)c + 1);
		int64_t off, len;
		int e1 = alt__tar_atoi(a, &off), e2 = alt__tar_atoi(b, &len);
		if (e1 < 0 || e2 < 0)
			return -ALT_ETAR_HEADER;
		int rc = sp_add(r, off, len);
		if (rc < 0)
			return rc;
	}
	return 0;
}

/* readGNUSparsePAXHeaders: 1 when the entry is a PAX sparse file */
static int read_pax_sparse(alt_tar_reader *r, alt_tar_header *h, const recs *v) {
	alt_str major = rec_get(v, "GNU.sparse.major"), minor = rec_get(v, "GNU.sparse.minor");
	bool is1x0;
	if (alt_str_eq_cstr(major, "0") && (alt_str_eq_cstr(minor, "0") || alt_str_eq_cstr(minor, "1")))
		is1x0 = false;
	else if (alt_str_eq_cstr(major, "1") && alt_str_eq_cstr(minor, "0"))
		is1x0 = true;
	else if (major.n || minor.n)
		return 0; /* an unknown version */
	else if (rec_get(v, "GNU.sparse.map").n)
		is1x0 = false; /* 0.0 and 0.1 had no version records */
	else
		return 0;
	alt_str name = rec_get(v, "GNU.sparse.name");
	if (name.n)
		h->name = name;
	alt_str size = rec_get(v, "GNU.sparse.size");
	if (!size.n)
		size = rec_get(v, "GNU.sparse.realsize");
	if (size.n && alt__tar_atoi(size, &h->size) < 0)
		return -ALT_ETAR_HEADER;
	int rc = is1x0 ? read_sparse_map_1x0(r) : read_sparse_map_0x1(r, v);
	return rc < 0 ? rc : 1;
}

static bool valid_sparse(const spent *sp, size_t n, int64_t size) {
	if (size < 0)
		return false;
	spent pre = {0, 0};
	for (size_t i = 0; i < n; i++) {
		spent c = sp[i];
		if (c.off < 0 || c.len < 0)
			return false;
		if (c.off > INT64_MAX - c.len)
			return false;
		if (c.off + c.len > size)
			return false;
		if (pre.off + pre.len > c.off)
			return false;
		pre = c;
	}
	return true;
}

/* the data fragments in place become the holes (invertSparseEntries) */
static void invert_sparse(alt_tar_reader *r, int64_t size) {
	size_t m = 0;
	spent pre = {0, 0};
	for (size_t i = 0; i < r->nsp; i++) {
		spent c = r->sp[i];
		if (c.len == 0)
			continue;
		pre.len = c.off - pre.off;
		if (pre.len > 0)
			r->sp[m++] = pre;
		pre.off = c.off + c.len;
	}
	pre.len = size - pre.off;
	r->sp[m++] = pre; /* there is room: one more than the fragments */
	r->nsp = m;
}

static int handle_sparse(alt_tar_reader *r, alt_tar_header *h, const recs *v) {
	r->nsp = 0;
	r->sp0 = 0;
	r->pos = 0;
	int rc;
	if (h->typeflag == ALT_TAR_GNU_SPARSE)
		rc = (rc = read_old_gnu_sparse(r, h)) < 0 ? rc : 1;
	else
		rc = read_pax_sparse(r, h, v);
	if (rc <= 0)
		return rc;
	if (header_only(h->typeflag) || !valid_sparse(r->sp, r->nsp, h->size))
		return -ALT_ETAR_HEADER;
	if (r->nsp == r->spcap) {
		spent *p = realloc(r->sp, (r->spcap + 1) * sizeof *p);
		if (!p)
			return -ENOMEM;
		r->sp = p;
		r->spcap++;
	}
	invert_sparse(r, h->size);
	r->sparse = true;
	return 0;
}

static int next(alt_tar_reader *r, alt_tar_header **out) {
	recs pax = {0, 0, 0};
	bool have_pax = false;
	alt_str long_name = ALT_STR(""), long_link = ALT_STR("");
	int format = TAR_FORMAT_ANY, rc, err;
	alt_tar_header *h = &r->hdr;
	for (;;) {
		if ((rc = discard(r, r->nb)) < 0)
			return rc;
		try_read_full(r, src_read, r->blk, (size_t)r->pad, &err);
		if (err)
			return err;
		r->pad = 0;
		if ((rc = read_header(r, h)) < 0)
			return rc;
		if ((rc = handle_regular(r, h)) < 0)
			return rc;
		format &= h->format;
		switch (h->typeflag) {
		case ALT_TAR_XHEADER:
		case ALT_TAR_XGLOBAL:
			format &= ALT_TAR_FORMAT_PAX;
			if ((rc = parse_pax(r, &pax)) < 0)
				return rc;
			have_pax = true;
			if (h->typeflag == ALT_TAR_XGLOBAL) {
				/* Go ignores the errors of the records here */
				bool ok = merge_pax(h, &pax) == 0;
				alt_tar_header g = {0};
				g.typeflag = h->typeflag;
				g.name = h->name;
				if (ok) {
					g.pax = h->pax;
					g.npax = h->npax;
				}
				g.format = format;
				*h = g;
				*out = h;
				return 0;
			}
			continue; /* the records apply to the next header */
		case ALT_TAR_GNU_LONGNAME:
		case ALT_TAR_GNU_LONGLINK: {
			format &= ALT_TAR_FORMAT_GNU;
			if ((rc = read_special(r)) < 0)
				return rc;
			int kerr = 0;
			alt_str s = keep(r, alt__tar_parse_string((const uint8_t *)r->tmp.p, r->tmp.n), &kerr);
			if (kerr)
				return kerr;
			if (h->typeflag == ALT_TAR_GNU_LONGNAME)
				long_name = s;
			else
				long_link = s;
			continue;
		}
		default:
			if ((rc = merge_pax(h, have_pax ? &pax : 0)) < 0)
				return rc;
			if (long_name.n)
				h->name = long_name;
			if (long_link.n)
				h->linkname = long_link;
			if (h->typeflag == ALT_TAR_REGA)
				h->typeflag = alt_str_has_suffix(h->name, ALT_STR("/")) ? ALT_TAR_DIR : ALT_TAR_REG;
			if ((rc = handle_regular(r, h)) < 0)
				return rc;
			if ((rc = handle_sparse(r, h, have_pax ? &pax : 0)) < 0)
				return rc;
			if ((format & ALT_TAR_FORMAT_USTAR) && (format & ALT_TAR_FORMAT_PAX))
				format &= ALT_TAR_FORMAT_USTAR;
			h->format = format;
			*out = h;
			return 0;
		}
	}
}

int alt_tar_next(alt_tar_reader *r, alt_tar_header **h) {
	*h = 0;
	if (r->err) {
		r->last = r->err;
		return public_code(r->err);
	}
	alt_arena_reset(&r->a);
	alt_tar_header *hp = 0;
	int rc = next(r, &hp);
	r->err = rc;
	r->last = rc;
	if (rc < 0)
		return public_code(rc);
	*h = hp;
	if (r->check_paths && !alt_path_is_local(hp->name)) {
		r->last = -ALT_ETAR_INSECURE_PATH;
		return -ALT_ETAR_INSECURE_PATH;
	}
	return 0;
}

ssize_t alt_tar_read(alt_tar_reader *r, void *buf, size_t n) {
	if (r->err) {
		r->last = r->err;
		return r->err == E_EOF ? 0 : public_code(r->err);
	}
	if (n == 0)
		return 0;
	int err;
	ssize_t k = curr_read(r, buf, n, &err);
	if (err && err != E_EOF)
		r->err = err;
	if (k > 0 || err == 0 || err == E_EOF)
		return k;
	r->last = err;
	return public_code(err);
}

static ssize_t reader_read(void *obj, void *buf, size_t n) { return alt_tar_read(obj, buf, n); }

alt_reader alt_tar_reader_reader(alt_tar_reader *r) { return (alt_reader){reader_read, r}; }
