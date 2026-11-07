/* The ZIP reader after Go's archive/zip reader.go (BSD-3-Clause, see
   LICENSE-Go): the same search for the end of the central directory,
   Zip64 records, base offset, extra fields and errors, and the same
   checks when a file's data ends. */
#include "zip_internal.h"
#include <alt/compress/flate.h>
#include <alt/hash/crc32.h>
#include <alt/path.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* int64 arithmetic that wraps as Go's does */
static int64_t add64(int64_t a, int64_t b) { return (int64_t)((uint64_t)a + (uint64_t)b); }
static int64_t sub64(int64_t a, int64_t b) { return (int64_t)((uint64_t)a - (uint64_t)b); }

/* ---- io.SectionReader ---- */

typedef struct {
	alt_zip_reader_at ra;
	int64_t off, limit;
} section;

static void sec_init(section *s, alt_zip_reader_at ra, int64_t off, int64_t n) {
	s->ra = ra;
	s->off = off;
	if (off <= sub64(INT64_MAX, n))
		s->limit = add64(off, n);
	else
		s->limit = INT64_MAX;
}

static ssize_t sec_read(section *s, void *buf, size_t n) {
	if (s->off >= s->limit)
		return 0;
	int64_t max = sub64(s->limit, s->off);
	if (max < 0)
		return -EINVAL; /* Go panics slicing here */
	if ((uint64_t)n > (uint64_t)max)
		n = (size_t)max;
	ssize_t k = alt_zip_read_at(s->ra, buf, n, s->off);
	if (k > 0)
		s->off += k;
	return k;
}

/* io.ReadFull: 0, -ALT_EEOF when nothing was read, -ALT_EUNEXPECTED_EOF */
static int sec_read_full(section *s, void *buf, size_t n) {
	size_t got = 0;
	while (got < n) {
		ssize_t k = sec_read(s, (char *)buf + got, n - got);
		if (k < 0)
			return (int)k;
		if (k == 0)
			return got ? -ALT_EUNEXPECTED_EOF : -ALT_EEOF;
		got += (size_t)k;
	}
	return 0;
}

/* a buffered section (bufio.Reader over it) for the central directory */
typedef struct {
	section s;
	unsigned char buf[4096];
	size_t r, w;
	int err;
} dstream;

static int ds_read_full(dstream *d, void *p, size_t n) {
	size_t got = 0;
	while (got < n) {
		if (d->r == d->w) {
			if (d->err == -ALT_EEOF)
				return got ? -ALT_EUNEXPECTED_EOF : -ALT_EEOF;
			if (d->err)
				return d->err;
			ssize_t k = sec_read(&d->s, d->buf, sizeof d->buf);
			if (k <= 0) {
				d->err = k < 0 ? (int)k : -ALT_EEOF;
				continue;
			}
			d->r = 0;
			d->w = (size_t)k;
		}
		size_t k = d->w - d->r;
		if (k > n - got)
			k = n - got;
		memcpy((char *)p + got, d->buf + d->r, k);
		d->r += k;
		got += k;
	}
	return 0;
}

/* ---- directory ---- */

/* seconds and nanoseconds as alt_wall, clamped */
static alt_wall wall_clamp(int64_t sec, int64_t nsec) {
	if (sec > 9223372036 || (sec == 9223372036 && nsec > 854775807))
		return INT64_MAX;
	if (sec < -9223372037 || (sec == -9223372037 && nsec < 145224192))
		return INT64_MIN;
	return sec * ALT_SEC + nsec;
}

/* Go's timeZone(msdos.Sub(modified)) in seconds */
static int zone_offset(int64_t dsec, int64_t dnsec) {
	if (dsec > 20 * 3600 || dsec < -20 * 3600)
		return 0; /* also when Sub saturates */
	const int64_t m = 15 * 60 * ALT_SEC;
	int64_t d = dsec * ALT_SEC + dnsec, r = d % m;
	if (d < 0) {
		r = -r;
		d = (uint64_t)r + (uint64_t)r < (uint64_t)m ? d + r : d - m + r;
	} else
		d = (uint64_t)r + (uint64_t)r < (uint64_t)m ? d - r : d + m - r;
	if (d < -12 * 3600 * ALT_SEC || d > 14 * 3600 * ALT_SEC)
		return 0;
	return (int)(d / ALT_SEC);
}

/* readDirectoryHeader: 0, -ALT_EZIP_FORMAT, -ALT_EEOF,
   -ALT_EUNEXPECTED_EOF, or an error of the source */
static int read_dir_header(alt_zip_file *f, dstream *d, alt_arena *a) {
	unsigned char b[ZIP_DIR_HEADER_LEN];
	int rc = ds_read_full(d, b, sizeof b);
	if (rc < 0)
		return rc;
	if (zip_le32(b) != ZIP_DIR_HEADER_SIG)
		return -ALT_EZIP_FORMAT;
	alt_zip_header *h = &f->hdr;
	memset(h, 0, sizeof *h);
	h->creator_version = zip_le16(b + 4);
	h->reader_version = zip_le16(b + 6);
	h->flags = zip_le16(b + 8);
	h->method = zip_le16(b + 10);
	h->modified_time = zip_le16(b + 12);
	h->modified_date = zip_le16(b + 14);
	h->crc32 = zip_le32(b + 16);
	h->compressed_size = zip_le32(b + 20);
	h->uncompressed_size = zip_le32(b + 24);
	h->compressed_size64 = h->compressed_size;
	h->uncompressed_size64 = h->uncompressed_size;
	size_t nl = zip_le16(b + 28), el = zip_le16(b + 30), cl = zip_le16(b + 32);
	h->external_attrs = zip_le32(b + 38);
	f->header_offset = zip_le32(b + 42);
	char *v = alt_arena_alloc(a, nl + el + cl + 2);
	if (!v)
		return -ENOMEM;
	if (nl + el + cl && (rc = ds_read_full(d, v, nl + el + cl)) < 0)
		return rc;
	memmove(v + nl + 1, v + nl, el + cl);
	v[nl] = 0;
	v[nl + 1 + el + cl] = 0;
	h->name = alt_str_make(v, nl);
	h->extra = alt_str_make(v + nl + 1, el);
	h->comment = alt_str_make(v + nl + 1 + el, cl);

	bool valid1, req1, valid2, req2;
	alt__zip_detect_utf8(h->name, &valid1, &req1);
	alt__zip_detect_utf8(h->comment, &valid2, &req2);
	if (!valid1 || !valid2)
		h->non_utf8 = true;
	else if (!req1 && !req2)
		h->non_utf8 = false;
	else
		h->non_utf8 = !(h->flags & ALT_ZIP_FLAG_UTF8);

	bool have = false; /* modified (Go: not the zero time) */
	int64_t msec = 0, mnsec = 0;
	const unsigned char *e = (const unsigned char *)h->extra.p;
	size_t en = h->extra.n;
	while (en >= 4) {
		unsigned tag = zip_le16(e), size = zip_le16(e + 2);
		e += 4, en -= 4;
		if (en < size)
			break;
		const unsigned char *fb = e;
		size_t fn = size;
		e += size, en -= size;
		switch (tag) {
		case ZIP_ZIP64_EXTRA:
			if (h->uncompressed_size == 0xffffffffu) {
				if (fn < 8)
					return -ALT_EZIP_FORMAT;
				h->uncompressed_size64 = zip_le64(fb);
				fb += 8, fn -= 8;
			}
			if (h->compressed_size == 0xffffffffu) {
				if (fn < 8)
					return -ALT_EZIP_FORMAT;
				h->compressed_size64 = zip_le64(fb);
				fb += 8, fn -= 8;
			}
			if (f->header_offset == 0xffffffffll) {
				if (fn < 8)
					return -ALT_EZIP_FORMAT;
				f->header_offset = (int64_t)zip_le64(fb);
			}
			break;
		case ZIP_NTFS_EXTRA:
			if (fn < 4)
				break;
			fb += 4, fn -= 4;
			while (fn >= 4) {
				unsigned atag = zip_le16(fb), asize = zip_le16(fb + 2);
				fb += 4, fn -= 4;
				if (fn < asize)
					break;
				const unsigned char *ab = fb;
				fb += asize, fn -= asize;
				if (atag != 1 || asize != 24)
					continue;
				int64_t ts = (int64_t)zip_le64(ab);
				int64_t secs = ts / 10000000, nsecs = 100 * (ts % 10000000);
				msec = -11644473600 + secs;
				mnsec = nsecs;
				if (mnsec < 0)
					mnsec += ALT_SEC, msec--;
				have = !(msec == -62135596800 && mnsec == 0);
			}
			break;
		case ZIP_UNIX_EXTRA:
		case ZIP_INFOZIP_UNIX_EXTRA:
			if (fn < 8)
				break;
			msec = zip_le32(fb + 4);
			mnsec = 0;
			have = true;
			break;
		case ZIP_EXTTIME_EXTRA:
			if (fn < 5 || !(fb[0] & 1))
				break;
			msec = zip_le32(fb + 1);
			mnsec = 0;
			have = true;
			break;
		}
	}

	int64_t dos = alt__zip_dos_unix(h->modified_date, h->modified_time);
	h->modified_set = true;
	if (!have) {
		h->modified = dos * ALT_SEC;
		return 0;
	}
	h->modified = wall_clamp(msec, mnsec);
	if (h->modified_time || h->modified_date) {
		h->modified_has_zone = true;
		h->modified_offset = zone_offset(dos - msec, -mnsec);
	}
	return 0;
}

static int find_signature(const unsigned char *b, size_t n) {
	for (ssize_t i = (ssize_t)n - ZIP_DIR_END_LEN; i >= 0; i--)
		if (b[i] == 'P' && b[i + 1] == 'K' && b[i + 2] == 5 && b[i + 3] == 6) {
			size_t c = zip_le16(b + i + ZIP_DIR_END_LEN - 2);
			if (c + ZIP_DIR_END_LEN + (size_t)i > n)
				return -1;
			return (int)i;
		}
	return -1;
}

typedef struct {
	uint64_t records, size, offset;
	alt_str comment;
} dir_end;

/* Go's ReadAt: a short read is io.EOF */
static int read_at_full(alt_zip_reader_at ra, void *buf, size_t n, int64_t off) {
	ssize_t k = alt_zip_read_at(ra, buf, n, off);
	if (k < 0)
		return (int)k;
	return (size_t)k < n ? -ALT_EEOF : 0;
}

static int read_dir_end(alt_zip_reader *r, dir_end *d, int64_t *base) {
	alt_zip_reader_at ra = r->ra;
	int64_t size = r->size, end_off = 0;
	unsigned char *buf = 0, *p = 0;
	size_t lens[2] = {1024, 65 * 1024}, n = 0;
	for (int i = 0; i < 2; i++) {
		int64_t blen = (int64_t)lens[i];
		if (blen > size)
			blen = size;
		unsigned char *nb = realloc(buf, (size_t)blen + 1);
		if (!nb) {
			free(buf);
			return -ENOMEM;
		}
		buf = nb;
		memset(buf, 0, (size_t)blen);
		ssize_t k = alt_zip_read_at(ra, buf, (size_t)blen, size - blen);
		if (k < 0) {
			free(buf);
			return (int)k;
		}
		int at = find_signature(buf, (size_t)blen);
		if (at >= 0) {
			p = buf + at;
			n = (size_t)blen - (size_t)at;
			end_off = size - blen + at;
			break;
		}
		if (i == 1 || blen == size) {
			free(buf);
			return -ALT_EZIP_FORMAT;
		}
	}
	d->records = zip_le16(p + 10);
	d->size = zip_le32(p + 12);
	d->offset = zip_le32(p + 16);
	size_t cl = zip_le16(p + 20);
	if (cl > n - ZIP_DIR_END_LEN) {
		free(buf);
		return -EINVAL; /* "zip: invalid comment length" */
	}
	char *c = alt_arena_alloc(&r->arena, cl + 1);
	if (!c) {
		free(buf);
		return -ENOMEM;
	}
	memcpy(c, p + ZIP_DIR_END_LEN, cl);
	c[cl] = 0;
	d->comment = alt_str_make(c, cl);
	free(buf);

	if (d->records == 0xffff || d->size == 0xffffffffu || d->offset == 0xffffffffu) {
		/* findDirectory64End */
		int64_t loc = end_off - ZIP_DIR64_LOC_LEN;
		if (loc >= 0) {
			unsigned char lb[ZIP_DIR64_LOC_LEN];
			int rc = read_at_full(ra, lb, sizeof lb, loc);
			if (rc < 0)
				return rc;
			int64_t p64 = (int64_t)zip_le64(lb + 8);
			if (zip_le32(lb) == ZIP_DIR64_LOC_SIG && zip_le32(lb + 4) == 0 && zip_le32(lb + 16) == 1 && p64 >= 0) {
				end_off = p64;
				unsigned char eb[ZIP_DIR64_END_LEN];
				if ((rc = read_at_full(ra, eb, sizeof eb, p64)) < 0)
					return rc;
				if (zip_le32(eb) != ZIP_DIR64_END_SIG)
					return -ALT_EZIP_FORMAT;
				d->records = zip_le64(eb + 32);
				d->size = zip_le64(eb + 40);
				d->offset = zip_le64(eb + 48);
			}
		}
	}
	if (d->size > (uint64_t)INT64_MAX || d->offset > (uint64_t)INT64_MAX)
		return -ALT_EZIP_FORMAT;
	int64_t b = sub64(sub64(end_off, (int64_t)d->size), (int64_t)d->offset);
	int64_t o = add64(b, (int64_t)d->offset);
	if (o < 0 || o >= size)
		return -ALT_EZIP_FORMAT;
	if (b > 0) {
		/* a directory header where the offset points without the base:
		   the base offset is wrong */
		int64_t off = (int64_t)d->offset;
		dstream *ds = calloc(1, sizeof *ds);
		if (!ds)
			return -ENOMEM;
		sec_init(&ds->s, ra, off, sub64(size, off));
		alt_zip_file tmp;
		if (read_dir_header(&tmp, ds, &r->arena) == 0)
			b = 0;
		free(ds);
	}
	*base = b;
	return 0;
}

static int reader_init(alt_zip_reader *r) {
	dir_end d;
	int64_t base;
	int rc = read_dir_end(r, &d, &base);
	if (rc < 0)
		return rc;
	r->base_offset = base;
	r->comment = d.comment;
	dstream *ds = calloc(1, sizeof *ds);
	if (!ds)
		return -ENOMEM;
	sec_init(&ds->s, r->ra, 0, r->size);
	ds->s.off = base + (int64_t)d.offset;
	size_t cap = 0;
	for (;;) {
		if (r->nfile == cap) {
			size_t nc = cap ? 2 * cap : 16;
			alt_zip_file *nf = realloc(r->file, nc * sizeof *nf);
			if (!nf) {
				rc = -ENOMEM;
				break;
			}
			r->file = nf, cap = nc;
		}
		alt_zip_file *f = &r->file[r->nfile];
		rc = read_dir_header(f, ds, &r->arena);
		if (rc == -ENOMEM)
			break;
		if (rc == -ALT_EZIP_FORMAT || rc == -ALT_EUNEXPECTED_EOF)
			break;
		if (rc < 0) {
			free(ds);
			return rc;
		}
		f->zip = r;
		f->header_offset = add64(f->header_offset, base);
		r->nfile++;
	}
	free(ds);
	if (rc == -ENOMEM)
		return rc;
	if ((uint16_t)r->nfile != (uint16_t)d.records)
		return rc;
	return 0;
}

int alt_zip_reader_new(alt_zip_reader **out, alt_zip_reader_at ra, int64_t size) {
	*out = 0;
	if (size < 0)
		return -EINVAL; /* "zip: size cannot be negative" */
	alt_zip_reader *r = calloc(1, sizeof *r);
	if (!r)
		return -ENOMEM;
	r->fd = -1;
	r->ra = ra;
	r->size = size;
	alt_arena_init(&r->arena, 0);
	int rc = reader_init(r);
	if (rc < 0) {
		alt_zip_reader_free(r);
		return rc;
	}
	*out = r;
	return 0;
}

int alt_zip_reader_new_mem(alt_zip_reader **out, alt_str data) {
	*out = 0;
	alt_zip_reader *r = calloc(1, sizeof *r);
	if (!r)
		return -ENOMEM;
	r->fd = -1;
	r->ra = alt_zip_mem_reader_at(&r->mem, data);
	r->size = (int64_t)data.n;
	alt_arena_init(&r->arena, 0);
	int rc = reader_init(r);
	if (rc < 0) {
		alt_zip_reader_free(r);
		return rc;
	}
	*out = r;
	return 0;
}

int alt_zip_open(alt_zip_reader **out, const char *path) {
	*out = 0;
	int fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return -errno;
	struct stat st;
	if (fstat(fd, &st) < 0) {
		int e = -errno;
		close(fd);
		return e;
	}
	alt_zip_reader *r = calloc(1, sizeof *r);
	if (!r) {
		close(fd);
		return -ENOMEM;
	}
	r->fd = fd;
	r->ra = alt_zip_fd_reader_at(&r->fd);
	r->size = st.st_size;
	alt_arena_init(&r->arena, 0);
	int rc = reader_init(r);
	if (rc < 0) {
		alt_zip_reader_free(r);
		return rc;
	}
	*out = r;
	return 0;
}

void alt_zip_reader_free(alt_zip_reader *r) {
	if (!r)
		return;
	if (r->fd >= 0)
		close(r->fd);
	free(r->file);
	alt_arena_free(&r->arena);
	free(r);
}

int alt_zip_reader_check_paths(const alt_zip_reader *r) {
	for (size_t i = 0; i < r->nfile; i++) {
		alt_str n = r->file[i].hdr.name;
		if (!n.n)
			continue;
		if (!alt_path_is_local(n) || memchr(n.p, '\\', n.n))
			return -ALT_EZIP_INSECURE_PATH;
	}
	return 0;
}

/* ---- file contents ---- */

static int find_body_offset(const alt_zip_file *f, int64_t *body) {
	unsigned char b[ZIP_FILE_HEADER_LEN];
	int rc = read_at_full(f->zip->ra, b, sizeof b, f->header_offset);
	if (rc < 0)
		return rc;
	if (zip_le32(b) != ZIP_FILE_HEADER_SIG)
		return -ALT_EZIP_FORMAT;
	*body = ZIP_FILE_HEADER_LEN + (int64_t)zip_le16(b + 26) + zip_le16(b + 28);
	return 0;
}

int alt_zip_file_data_offset(const alt_zip_file *f, int64_t *off) {
	int64_t body;
	int rc = find_body_offset(f, &body);
	if (rc < 0)
		return rc;
	*off = add64(f->header_offset, body);
	return 0;
}

enum { FR_DIR, FR_STORE, FR_DEFLATE, FR_RAW };

struct alt_zip_file_reader {
	const alt_zip_file *f;
	int kind;
	int err;  /* sticky: an error, or 1 at the end */
	section data, desc;
	bool has_desc;
	alt_inflate *inf;
	uint32_t crc;
	uint64_t nread;
};

static ssize_t data_read(void *obj, void *buf, size_t n) { return sec_read(obj, buf, n); }

static int open_file(const alt_zip_file *f, alt_zip_file_reader **out, bool raw) {
	*out = 0;
	int64_t body;
	int rc = find_body_offset(f, &body);
	if (rc < 0)
		return rc;
	const alt_zip_header *h = &f->hdr;
	int kind = FR_RAW;
	if (!raw) {
		if (alt_str_has_suffix(h->name, ALT_STR("/")))
			kind = FR_DIR;
		else if (h->method == ALT_ZIP_STORE)
			kind = FR_STORE;
		else if (h->method == ALT_ZIP_DEFLATE)
			kind = FR_DEFLATE;
		else
			return -ALT_EZIP_ALGORITHM;
	}
	alt_zip_file_reader *fr = calloc(1, sizeof *fr);
	if (!fr)
		return -ENOMEM;
	fr->f = f;
	fr->kind = kind;
	if (kind == FR_DIR) {
		fr->err = h->uncompressed_size64 ? -ALT_EZIP_FORMAT : 1;
		*out = fr;
		return 0;
	}
	int64_t start = add64(f->header_offset, body), size = (int64_t)h->compressed_size64;
	sec_init(&fr->data, f->zip->ra, start, size);
	if (kind == FR_DEFLATE && (rc = alt_inflate_new(&fr->inf, (alt_reader){data_read, &fr->data})) < 0) {
		free(fr);
		return rc;
	}
	if (kind != FR_RAW && (h->flags & ALT_ZIP_FLAG_DATA_DESCRIPTOR)) {
		fr->has_desc = true;
		sec_init(&fr->desc, f->zip->ra, add64(start, size), ZIP_DATA_DESC_LEN);
	}
	*out = fr;
	return 0;
}

int alt_zip_file_open(const alt_zip_file *f, alt_zip_file_reader **out) { return open_file(f, out, false); }
int alt_zip_file_open_raw(const alt_zip_file *f, alt_zip_file_reader **out) { return open_file(f, out, true); }

/* readDataDescriptor */
static int read_desc(alt_zip_file_reader *fr) {
	unsigned char b[ZIP_DATA_DESC_LEN];
	int rc = sec_read_full(&fr->desc, b, 4);
	if (rc < 0)
		return rc;
	size_t off = zip_le32(b) != ZIP_DATA_DESC_SIG ? 4 : 0;
	if ((rc = sec_read_full(&fr->desc, b + off, 12 - off)) < 0)
		return rc;
	if (zip_le32(b) != fr->f->hdr.crc32)
		return -ALT_ECHECKSUM;
	return 0;
}

ssize_t alt_zip_file_read(alt_zip_file_reader *fr, void *buf, size_t n) {
	if (fr->err)
		return fr->err > 0 ? 0 : fr->err;
	if (fr->kind == FR_RAW) {
		ssize_t k = sec_read(&fr->data, buf, n);
		if (k <= 0)
			fr->err = k < 0 ? (int)k : 1;
		return k;
	}
	if (!n)
		return 0;
	const alt_zip_header *h = &fr->f->hdr;
	ssize_t k = fr->kind == FR_STORE ? sec_read(&fr->data, buf, n) : alt_inflate_read(fr->inf, buf, n);
	if (k > 0) {
		fr->crc = alt_crc32_ieee_update(fr->crc, buf, (size_t)k);
		fr->nread += (uint64_t)k;
		if (fr->nread > h->uncompressed_size64)
			return fr->err = -ALT_EZIP_FORMAT;
		return k;
	}
	int err = (int)k;
	if (k == 0) {
		err = 1;
		if (fr->nread != h->uncompressed_size64)
			err = -ALT_EUNEXPECTED_EOF;
		else if (fr->has_desc) {
			int e = read_desc(fr);
			if (e == -ALT_EEOF)
				err = -ALT_EUNEXPECTED_EOF;
			else if (e < 0)
				err = e;
			else if (fr->crc != h->crc32)
				err = -ALT_ECHECKSUM;
		} else if (h->crc32 && fr->crc != h->crc32)
			err = -ALT_ECHECKSUM;
	}
	fr->err = err;
	return err > 0 ? 0 : err;
}

static ssize_t stream_read(void *obj, void *buf, size_t n) { return alt_zip_file_read(obj, buf, n); }

alt_reader alt_zip_file_stream(alt_zip_file_reader *fr) { return (alt_reader){stream_read, fr}; }

void alt_zip_file_close(alt_zip_file_reader *fr) {
	if (!fr)
		return;
	if (fr->inf)
		alt_inflate_free(fr->inf);
	free(fr);
}
