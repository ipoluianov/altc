/* The ZIP writer after Go's archive/zip writer.go (BSD-3-Clause, see
   LICENSE-Go): the same local headers, data descriptors, central
   directory and Zip64 records, so stored archives are byte for byte
   Go's. */
#include "zip_internal.h"
#include <alt/compress/flate.h>
#include <alt/hash/crc32.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	alt_zip_header h; /* strings in the writer's arena */
	uint64_t offset;
	bool raw;
} entry;

typedef struct {
	alt_zip_writer *zw;
	size_t idx; /* the entry */
	bool raw, closed;
	uint32_t crc;
	uint64_t raw_count, comp_count;
	alt_deflate *def;
} fwriter;

struct alt_zip_writer {
	alt_bufwriter bw;
	int64_t count;
	entry *dir;
	size_t ndir, cap;
	fwriter *last;
	bool closed;
	alt_str comment;
	alt_arena arena;
};

int alt_zip_writer_new(alt_zip_writer **out, alt_writer w) {
	alt_zip_writer *zw = calloc(1, sizeof *zw);
	if (!zw)
		return -ENOMEM;
	int rc = alt_bufwriter_init(&zw->bw, w, 4096);
	if (rc < 0) {
		free(zw);
		return rc;
	}
	alt_arena_init(&zw->arena, 0);
	*out = zw;
	return 0;
}

void alt_zip_writer_free(alt_zip_writer *zw) {
	if (!zw)
		return;
	if (zw->last && zw->last->def)
		alt_deflate_free(zw->last->def);
	alt_bufwriter_free(&zw->bw);
	free(zw->dir);
	alt_arena_free(&zw->arena);
	free(zw);
}

int alt_zip_writer_set_offset(alt_zip_writer *zw, int64_t n) {
	if (zw->count != 0)
		return -EINVAL; /* Go panics */
	zw->count = n;
	return 0;
}

int alt_zip_writer_set_comment(alt_zip_writer *zw, alt_str c) {
	if (c.n > ZIP_UINT16MAX)
		return -EINVAL; /* "zip: Writer.Comment too long" */
	zw->comment = alt_arena_str(&zw->arena, c);
	return c.n && !zw->comment.p ? -ENOMEM : 0;
}

int alt_zip_writer_flush(alt_zip_writer *zw) { return alt_bufwriter_flush(&zw->bw); }

static int cw_write(alt_zip_writer *zw, const void *p, size_t n) {
	if (!n)
		return 0;
	ssize_t k = alt_bufwriter_write(&zw->bw, p, n);
	if (k < 0)
		return (int)k;
	zw->count += (int64_t)n;
	return 0;
}

static ssize_t comp_write(void *obj, const void *p, size_t n) {
	fwriter *fw = obj;
	int rc = cw_write(fw->zw, p, n);
	if (rc < 0)
		return rc;
	fw->comp_count += n;
	return (ssize_t)n;
}

static ssize_t fw_write(void *obj, const void *p, size_t n) {
	fwriter *fw = obj;
	if (fw->closed)
		return -EINVAL; /* "zip: write to closed file" */
	if (fw->raw) {
		int rc = cw_write(fw->zw, p, n);
		return rc < 0 ? rc : (ssize_t)n;
	}
	fw->crc = alt_crc32_ieee_update(fw->crc, p, n);
	ssize_t k = fw->def ? alt_deflate_write(fw->def, p, n) : comp_write(fw, p, n);
	if (k > 0)
		fw->raw_count += (uint64_t)k;
	return k;
}

static ssize_t dir_write(void *obj, const void *p, size_t n) {
	(void)obj, (void)p;
	return n ? -EINVAL : 0; /* "zip: write to directory" */
}

static int write_desc(fwriter *fw) {
	alt_zip_header *h = &fw->zw->dir[fw->idx].h;
	if (!(h->flags & ALT_ZIP_FLAG_DATA_DESCRIPTOR))
		return 0;
	unsigned char b[ZIP_DATA_DESC64_LEN], *p = b;
	p = zip_put32(p, ZIP_DATA_DESC_SIG);
	p = zip_put32(p, h->crc32);
	if (h->compressed_size64 > ZIP_UINT32MAX || h->uncompressed_size64 > ZIP_UINT32MAX) {
		p = zip_put64(p, h->compressed_size64);
		p = zip_put64(p, h->uncompressed_size64);
	} else {
		p = zip_put32(p, h->compressed_size);
		p = zip_put32(p, h->uncompressed_size);
	}
	return cw_write(fw->zw, b, (size_t)(p - b));
}

static int fw_close(fwriter *fw) {
	if (fw->closed)
		return -EINVAL; /* "zip: file closed twice" */
	fw->closed = true;
	if (fw->raw)
		return write_desc(fw);
	if (fw->def) {
		int rc = alt_deflate_close(fw->def);
		alt_deflate_free(fw->def);
		fw->def = 0;
		if (rc < 0)
			return rc;
	}
	alt_zip_header *h = &fw->zw->dir[fw->idx].h;
	h->crc32 = fw->crc;
	h->compressed_size64 = fw->comp_count;
	h->uncompressed_size64 = fw->raw_count;
	if (h->compressed_size64 > ZIP_UINT32MAX || h->uncompressed_size64 > ZIP_UINT32MAX) {
		h->compressed_size = 0xffffffffu;
		h->uncompressed_size = 0xffffffffu;
		h->reader_version = ZIP_VERSION45;
	} else {
		h->compressed_size = (uint32_t)h->compressed_size64;
		h->uncompressed_size = (uint32_t)h->uncompressed_size64;
	}
	return write_desc(fw);
}

/* Go's prepare: finishes the previous file */
static int prepare(alt_zip_writer *zw) {
	if (zw->last && !zw->last->closed) {
		int rc = fw_close(zw->last);
		if (rc < 0)
			return rc;
	}
	return 0;
}

static int append_extra(alt_zip_writer *zw, alt_zip_header *h, const void *p, size_t n) {
	char *e = alt_arena_alloc(&zw->arena, h->extra.n + n + 1);
	if (!e)
		return -ENOMEM;
	if (h->extra.n)
		memcpy(e, h->extra.p, h->extra.n);
	memcpy(e + h->extra.n, p, n);
	h->extra = alt_str_make(e, h->extra.n + n);
	return 0;
}

/* a new entry with a copy of *src */
static entry *add_entry(alt_zip_writer *zw, const alt_zip_header *src) {
	if (zw->ndir == zw->cap) {
		size_t nc = zw->cap ? 2 * zw->cap : 16;
		entry *nd = realloc(zw->dir, nc * sizeof *nd);
		if (!nd)
			return 0;
		zw->dir = nd, zw->cap = nc;
	}
	entry *e = &zw->dir[zw->ndir];
	memset(e, 0, sizeof *e);
	e->h = *src;
	e->h.name = alt_arena_str(&zw->arena, src->name);
	e->h.comment = alt_arena_str(&zw->arena, src->comment);
	e->h.extra = alt_arena_str(&zw->arena, src->extra);
	if (!e->h.name.p || !e->h.comment.p || !e->h.extra.p)
		return 0;
	e->offset = (uint64_t)zw->count;
	return e;
}

static int write_header(alt_zip_writer *zw, const entry *e) {
	const alt_zip_header *h = &e->h;
	if (h->name.n > ZIP_UINT16MAX)
		return -EINVAL; /* "zip: FileHeader.Name too long" */
	if (h->extra.n > ZIP_UINT16MAX)
		return -EINVAL; /* "zip: FileHeader.Extra too long" */
	unsigned char z64[20];
	size_t z64n = 0;
	uint16_t rv = h->reader_version;
	bool no_desc = e->raw && !(h->flags & ALT_ZIP_FLAG_DATA_DESCRIPTOR);
	if (no_desc && (h->compressed_size64 > ZIP_UINT32MAX || h->uncompressed_size64 > ZIP_UINT32MAX)) {
		if (rv < ZIP_VERSION45)
			rv = ZIP_VERSION45;
		unsigned char *p = zip_put16(z64, ZIP_ZIP64_EXTRA);
		p = zip_put16(p, 16);
		p = zip_put64(p, h->uncompressed_size64);
		zip_put64(p, h->compressed_size64);
		z64n = 20;
	}
	unsigned char b[ZIP_FILE_HEADER_LEN], *p = b;
	p = zip_put32(p, ZIP_FILE_HEADER_SIG);
	p = zip_put16(p, rv);
	p = zip_put16(p, h->flags);
	p = zip_put16(p, h->method);
	p = zip_put16(p, h->modified_time);
	p = zip_put16(p, h->modified_date);
	if (no_desc) {
		p = zip_put32(p, h->crc32);
		if (z64n) {
			p = zip_put32(p, 0xffffffffu);
			p = zip_put32(p, 0xffffffffu);
		} else {
			p = zip_put32(p, (uint32_t)h->compressed_size64);
			p = zip_put32(p, (uint32_t)h->uncompressed_size64);
		}
	} else {
		p = zip_put32(p, 0);
		p = zip_put32(p, 0);
		p = zip_put32(p, 0);
	}
	p = zip_put16(p, (uint16_t)h->name.n);
	zip_put16(p, (uint16_t)(h->extra.n + z64n));
	int rc;
	if ((rc = cw_write(zw, b, sizeof b)) < 0 || (rc = cw_write(zw, h->name.p, h->name.n)) < 0 ||
		(rc = cw_write(zw, h->extra.p, h->extra.n)) < 0 || (rc = cw_write(zw, z64, z64n)) < 0)
		return rc;
	return 0;
}

static bool is_dir(alt_str name) { return name.n && name.p[name.n - 1] == '/'; }

int alt_zip_create_header(alt_zip_writer *zw, const alt_zip_header *fh, alt_writer *out) {
	*out = (alt_writer){dir_write, 0};
	int rc = prepare(zw);
	if (rc < 0)
		return rc;
	alt_zip_header h = *fh;
	bool valid1, req1, valid2, req2;
	alt__zip_detect_utf8(h.name, &valid1, &req1);
	alt__zip_detect_utf8(h.comment, &valid2, &req2);
	if (h.non_utf8)
		h.flags &= (uint16_t)~ALT_ZIP_FLAG_UTF8;
	else if ((req1 || req2) && valid1 && valid2)
		h.flags |= ALT_ZIP_FLAG_UTF8;
	h.creator_version = (uint16_t)((h.creator_version & 0xff00) | ZIP_VERSION20);
	h.reader_version = ZIP_VERSION20;

	bool dir = is_dir(h.name);
	if (dir) {
		h.method = ALT_ZIP_STORE;
		h.flags &= (uint16_t)~ALT_ZIP_FLAG_DATA_DESCRIPTOR;
		h.compressed_size = h.uncompressed_size = 0;
		h.compressed_size64 = h.uncompressed_size64 = 0;
	} else {
		h.flags |= ALT_ZIP_FLAG_DATA_DESCRIPTOR;
		if (h.method != ALT_ZIP_STORE && h.method != ALT_ZIP_DEFLATE)
			return -ALT_EZIP_ALGORITHM;
	}
	entry *e = add_entry(zw, &h);
	if (!e)
		return -ENOMEM;
	if (h.modified_set) {
		alt__zip_dos_time(h.modified, h.modified_offset, &e->h.modified_date, &e->h.modified_time);
		unsigned char m[9], *p = zip_put16(m, ZIP_EXTTIME_EXTRA);
		p = zip_put16(p, 5);
		*p++ = 1;
		zip_put32(p, (uint32_t)alt_wall_unix(h.modified));
		if ((rc = append_extra(zw, &e->h, m, sizeof m)) < 0)
			return rc;
	}
	fwriter *fw = 0;
	if (!dir) {
		fw = alt_arena_zalloc(&zw->arena, sizeof *fw);
		if (!fw)
			return -ENOMEM;
		fw->zw = zw;
		fw->idx = zw->ndir;
		if (h.method == ALT_ZIP_DEFLATE && (rc = alt_deflate_new(&fw->def, (alt_writer){comp_write, fw}, 5)) < 0)
			return rc;
	}
	zw->ndir++;
	zw->last = fw;
	if ((rc = write_header(zw, e)) < 0)
		return rc;
	if (fw)
		*out = (alt_writer){fw_write, fw};
	return 0;
}

int alt_zip_create(alt_zip_writer *zw, alt_str name, alt_writer *fw) {
	alt_zip_header h = {0};
	h.name = name;
	h.method = ALT_ZIP_DEFLATE;
	return alt_zip_create_header(zw, &h, fw);
}

int alt_zip_create_raw(alt_zip_writer *zw, const alt_zip_header *fh, alt_writer *out) {
	*out = (alt_writer){dir_write, 0};
	int rc = prepare(zw);
	if (rc < 0)
		return rc;
	alt_zip_header h = *fh;
	h.compressed_size = (uint32_t)(h.compressed_size64 < ZIP_UINT32MAX ? h.compressed_size64 : ZIP_UINT32MAX);
	h.uncompressed_size = (uint32_t)(h.uncompressed_size64 < ZIP_UINT32MAX ? h.uncompressed_size64 : ZIP_UINT32MAX);
	entry *e = add_entry(zw, &h);
	if (!e)
		return -ENOMEM;
	e->raw = true;
	zw->ndir++;
	if ((rc = write_header(zw, e)) < 0)
		return rc;
	if (is_dir(h.name)) {
		zw->last = 0;
		return 0;
	}
	fwriter *fw = alt_arena_zalloc(&zw->arena, sizeof *fw);
	if (!fw)
		return -ENOMEM;
	fw->zw = zw;
	fw->idx = zw->ndir - 1;
	fw->raw = true;
	zw->last = fw;
	*out = (alt_writer){fw_write, fw};
	return 0;
}

int alt_zip_copy(alt_zip_writer *zw, const alt_zip_file *f) {
	alt_zip_file_reader *fr;
	int rc = alt_zip_file_open_raw(f, &fr);
	if (rc < 0)
		return rc;
	alt_writer fw;
	if ((rc = alt_zip_create_raw(zw, &f->hdr, &fw)) < 0) {
		alt_zip_file_close(fr);
		return rc;
	}
	ssize_t k = alt_copy(fw, alt_zip_file_stream(fr));
	alt_zip_file_close(fr);
	return k < 0 ? (int)k : 0;
}

int alt_zip_writer_close(alt_zip_writer *zw) {
	if (zw->last && !zw->last->closed) {
		int rc = fw_close(zw->last);
		if (rc < 0)
			return rc;
		zw->last = 0;
	}
	if (zw->closed)
		return -EINVAL; /* "zip: writer closed twice" */
	zw->closed = true;

	int64_t start = zw->count;
	bool used64 = false;
	int rc;
	for (size_t i = 0; i < zw->ndir; i++) {
		entry *e = &zw->dir[i];
		alt_zip_header *h = &e->h;
		uint16_t rv = h->reader_version;
		if (h->compressed_size64 >= ZIP_UINT32MAX || h->uncompressed_size64 >= ZIP_UINT32MAX || e->offset >= ZIP_UINT32MAX) {
			used64 = true;
			if (rv < ZIP_VERSION45)
				rv = ZIP_VERSION45;
			unsigned char b[28], *p = b + 4;
			if (h->uncompressed_size64 >= ZIP_UINT32MAX)
				p = zip_put64(p, h->uncompressed_size64);
			if (h->compressed_size64 >= ZIP_UINT32MAX)
				p = zip_put64(p, h->compressed_size64);
			if (e->offset >= ZIP_UINT32MAX)
				p = zip_put64(p, e->offset);
			zip_put16(b, ZIP_ZIP64_EXTRA);
			zip_put16(b + 2, (uint16_t)(p - b - 4));
			if ((rc = append_extra(zw, h, b, (size_t)(p - b))) < 0)
				return rc;
		}
		unsigned char b[ZIP_DIR_HEADER_LEN], *p = b;
		p = zip_put32(p, ZIP_DIR_HEADER_SIG);
		p = zip_put16(p, h->creator_version);
		p = zip_put16(p, rv);
		p = zip_put16(p, h->flags);
		p = zip_put16(p, h->method);
		p = zip_put16(p, h->modified_time);
		p = zip_put16(p, h->modified_date);
		p = zip_put32(p, h->crc32);
		p = zip_put32(p, (uint32_t)(h->compressed_size64 < ZIP_UINT32MAX ? h->compressed_size64 : ZIP_UINT32MAX));
		p = zip_put32(p, (uint32_t)(h->uncompressed_size64 < ZIP_UINT32MAX ? h->uncompressed_size64 : ZIP_UINT32MAX));
		p = zip_put16(p, (uint16_t)h->name.n);
		p = zip_put16(p, (uint16_t)h->extra.n);
		p = zip_put16(p, (uint16_t)h->comment.n);
		p = zip_put32(p, 0); /* disk number start, internal attributes */
		p = zip_put32(p, h->external_attrs);
		zip_put32(p, (uint32_t)(e->offset < ZIP_UINT32MAX ? e->offset : ZIP_UINT32MAX));
		if ((rc = cw_write(zw, b, sizeof b)) < 0 || (rc = cw_write(zw, h->name.p, h->name.n)) < 0 ||
			(rc = cw_write(zw, h->extra.p, h->extra.n)) < 0 || (rc = cw_write(zw, h->comment.p, h->comment.n)) < 0)
			return rc;
	}
	int64_t end = zw->count;
	uint64_t records = zw->ndir, size = (uint64_t)(end - start), offset = (uint64_t)start;
	if (used64 || records >= ZIP_UINT16MAX || size >= ZIP_UINT32MAX || offset >= ZIP_UINT32MAX) {
		unsigned char b[ZIP_DIR64_END_LEN + ZIP_DIR64_LOC_LEN], *p = b;
		p = zip_put32(p, ZIP_DIR64_END_SIG);
		p = zip_put64(p, ZIP_DIR64_END_LEN - 12);
		p = zip_put16(p, ZIP_VERSION45);
		p = zip_put16(p, ZIP_VERSION45);
		p = zip_put32(p, 0);
		p = zip_put32(p, 0);
		p = zip_put64(p, records);
		p = zip_put64(p, records);
		p = zip_put64(p, size);
		p = zip_put64(p, offset);
		p = zip_put32(p, ZIP_DIR64_LOC_SIG);
		p = zip_put32(p, 0);
		p = zip_put64(p, (uint64_t)end);
		zip_put32(p, 1);
		if ((rc = cw_write(zw, b, sizeof b)) < 0)
			return rc;
	}
	unsigned char b[ZIP_DIR_END_LEN], *p = b;
	p = zip_put32(p, ZIP_DIR_END_SIG);
	p = zip_put32(p, 0);
	p = zip_put16(p, (uint16_t)(records < ZIP_UINT16MAX ? records : ZIP_UINT16MAX));
	p = zip_put16(p, (uint16_t)(records < ZIP_UINT16MAX ? records : ZIP_UINT16MAX));
	p = zip_put32(p, (uint32_t)(size < ZIP_UINT32MAX ? size : ZIP_UINT32MAX));
	p = zip_put32(p, (uint32_t)(offset < ZIP_UINT32MAX ? offset : ZIP_UINT32MAX));
	zip_put16(p, (uint16_t)zw->comment.n);
	if ((rc = cw_write(zw, b, sizeof b)) < 0 || (rc = cw_write(zw, zw->comment.p, zw->comment.n)) < 0)
		return rc;
	return alt_bufwriter_flush(&zw->bw);
}
