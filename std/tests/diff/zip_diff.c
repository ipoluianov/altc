/* R hex -> the dump of the archive read by alt_zip_reader (fields below)
   S size spans -> the same of a sparse archive (spans "off:hex;..."),
      without reading files larger than 16 MiB
   W offset comment entries... -> "ok <hex of the archive>" or "err <e>"
      (comment "!" for none; entries as zipEntry in tests/stdzip_test.go)
   C hex -> the archive with every file of hex copied (Copy), as W
   M n -> n empty stored files written, then read back: "<sha256> <nfile>"
   The dump: "ok|comment|nfile|insecure" and per file "|" and the
   header fields joined by ',', or "err <e>". */
#include "common.h"
#include <alt/archive/zip.h>
#include <alt/crypto/hash.h>
#include <inttypes.h>

static alt_str arg(char *s) { return alt_str_make(s, diff_unhex(s)); }

static const char *tok(int e) {
	static char b[32];
	switch (-e) {
	case ALT_EZIP_FORMAT: return "format";
	case ALT_EZIP_ALGORITHM: return "algorithm";
	case ALT_ECHECKSUM: return "checksum";
	case ALT_EEOF: return "eof";
	case ALT_EUNEXPECTED_EOF: return "unexpected-eof";
	case ALT_EFLATE_CORRUPT: return "corrupt";
	case EINVAL: return "inval";
	}
	snprintf(b, sizeof b, "code%d", e);
	return b;
}

static void puthex(alt_buf *o, const void *p, size_t n) {
	static const char hx[] = "0123456789abcdef";
	alt_buf_reserve(o, 2 * n);
	for (size_t i = 0; i < n; i++) {
		unsigned c = ((const unsigned char *)p)[i];
		alt_buf_append_byte(o, hx[c >> 4]);
		alt_buf_append_byte(o, hx[c & 15]);
	}
}

/* reads everything: "<len>:<sha256 prefix>:ok" or ":<e>"; the length and
   hash only with no error or a checksum error */
static void read_all(alt_buf *o, alt_zip_file_reader *fr) {
	alt_hash h;
	alt_hash_init(&h, ALT_SHA256);
	static char buf[65536];
	uint64_t n = 0;
	ssize_t k;
	size_t sizes[] = {1, 7, 512, 4096, 65536};
	int i = 0;
	while ((k = alt_zip_file_read(fr, buf, sizes[i++ % 5])) > 0) {
		alt_hash_write(&h, buf, (size_t)k);
		n += (uint64_t)k;
	}
	if (k == 0 || k == -ALT_ECHECKSUM) {
		uint8_t sum[32];
		alt_hash_sum(&h, sum);
		alt_buf_printf(o, "%" PRIu64 ":", n);
		puthex(o, sum, 8);
		alt_buf_printf(o, ":%s", k ? tok((int)k) : "ok");
	} else
		alt_buf_printf(o, "-:-:%s", tok((int)k));
}

static void dump(alt_buf *o, alt_zip_reader_at ra, int64_t size, bool sparse) {
	alt_zip_reader *r;
	int rc = alt_zip_reader_new(&r, ra, size);
	if (rc < 0) {
		alt_buf_printf(o, "err %s", tok(rc));
		return;
	}
	alt_buf_append_cstr(o, "ok|");
	puthex(o, r->comment.p, r->comment.n);
	alt_buf_printf(o, "|%zu|%d", r->nfile, alt_zip_reader_check_paths(r) < 0);
	for (size_t i = 0; i < r->nfile; i++) {
		alt_zip_file *f = &r->file[i];
		alt_zip_header *h = &f->hdr;
		alt_buf_append_byte(o, '|');
		puthex(o, h->name.p, h->name.n);
		alt_buf_append_byte(o, ',');
		puthex(o, h->comment.p, h->comment.n);
		alt_buf_printf(o, ",%d,%u,%u,%u,%u,%" PRId64 ",", h->non_utf8, h->creator_version, h->reader_version, h->flags, h->method,
			h->modified);
		if (h->modified_has_zone)
			alt_buf_printf(o, "%d", h->modified_offset);
		else
			alt_buf_append_byte(o, 'U');
		alt_buf_printf(o, ",%u,%u,%u,%u,%u,%" PRIu64 ",%" PRIu64 ",", h->modified_time, h->modified_date, h->crc32, h->compressed_size,
			h->uncompressed_size, h->compressed_size64, h->uncompressed_size64);
		puthex(o, h->extra.p, h->extra.n);
		alt_buf_printf(o, ",%u,%o,", h->external_attrs, alt_zip_header_mode(h));
		int64_t off;
		if ((rc = alt_zip_file_data_offset(f, &off)) < 0)
			alt_buf_printf(o, "e:%s", tok(rc));
		else
			alt_buf_printf(o, "%" PRId64, off);
		alt_buf_append_byte(o, ',');
		alt_zip_file_reader *fr;
		bool big = sparse && (h->uncompressed_size64 > 1 << 24 || h->compressed_size64 > 1 << 24);
		if ((rc = alt_zip_file_open(f, &fr)) < 0)
			alt_buf_printf(o, "e:%s", tok(rc));
		else {
			if (big)
				alt_buf_append_cstr(o, "skip");
			else
				read_all(o, fr);
			alt_zip_file_close(fr);
		}
		alt_buf_append_byte(o, ',');
		if ((rc = alt_zip_file_open_raw(f, &fr)) < 0)
			alt_buf_printf(o, "e:%s", tok(rc));
		else {
			if (big)
				alt_buf_append_cstr(o, "skip");
			else
				read_all(o, fr);
			alt_zip_file_close(fr);
		}
	}
	alt_zip_reader_free(r);
}

/* ---- sparse files ---- */

typedef struct {
	int64_t off;
	alt_str data;
} span;

typedef struct {
	int64_t size;
	span *s;
	size_t n;
} sparse;

static ssize_t sparse_read_at(void *obj, void *buf, size_t n, int64_t off) {
	sparse *sp = obj;
	if (off < 0)
		return -EINVAL;
	if (off >= sp->size)
		return 0;
	int64_t end = off + (int64_t)n < sp->size ? off + (int64_t)n : sp->size;
	memset(buf, 0, (size_t)(end - off));
	for (size_t i = 0; i < sp->n; i++) {
		int64_t s0 = sp->s[i].off, s1 = s0 + (int64_t)sp->s[i].data.n;
		if (s1 <= off || s0 >= end)
			continue;
		int64_t from = s0 > off ? s0 : off, to = s1 < end ? s1 : end;
		memcpy((char *)buf + (from - off), sp->s[i].data.p + (from - s0), (size_t)(to - from));
	}
	return (ssize_t)(end - off);
}

/* ---- writing ---- */

/* splits s at sep into at most max fields */
static int split(char *s, char **f, int max, int sep) {
	int n = 0;
	while (n < max) {
		f[n++] = s;
		char *t = strchr(s, sep);
		if (!t)
			break;
		*t = 0;
		s = t + 1;
	}
	return n;
}

static uint64_t num(const char *s) { return strtoull(s, 0, 10); }

static int write_entries(alt_zip_writer *zw, char **f, int n) {
	int rc;
	for (int i = 0; i < n; i++) {
		char *e[24];
		int m = split(f[i], e, 24, ',');
		if (m < 20)
			return -1000;
		alt_zip_header h = {0};
		h.name = arg(e[1]);
		h.comment = arg(e[2]);
		h.non_utf8 = num(e[3]);
		h.creator_version = (uint16_t)num(e[4]);
		h.reader_version = (uint16_t)num(e[5]);
		h.flags = (uint16_t)num(e[6]);
		h.method = (uint16_t)num(e[7]);
		h.modified_set = num(e[8]);
		h.modified = strtoll(e[9], 0, 10);
		h.modified_offset = (int)strtol(e[10], 0, 10);
		h.modified_time = (uint16_t)num(e[11]);
		h.modified_date = (uint16_t)num(e[12]);
		h.crc32 = (uint32_t)num(e[13]);
		h.compressed_size64 = num(e[14]);
		h.uncompressed_size64 = num(e[15]);
		h.compressed_size = (uint32_t)h.compressed_size64;
		h.uncompressed_size = (uint32_t)h.uncompressed_size64;
		h.extra = arg(e[16]);
		h.external_attrs = (uint32_t)num(e[17]);
		alt_str data = arg(e[18]);
		size_t chunk = num(e[19]);
		alt_writer fw;
		if (e[0][0] == 'C')
			rc = alt_zip_create(zw, h.name, &fw);
		else if (e[0][0] == 'R')
			rc = alt_zip_create_raw(zw, &h, &fw);
		else
			rc = alt_zip_create_header(zw, &h, &fw);
		if (rc < 0)
			return rc;
		for (size_t off = 0; off < data.n; off += chunk) {
			size_t k = data.n - off < chunk ? data.n - off : chunk;
			ssize_t w = alt_write_all(fw, data.p + off, k);
			if (w < 0)
				return (int)w;
		}
	}
	return 0;
}

static void finish(alt_buf *o, alt_zip_writer *zw, alt_buf *out, int rc) {
	if (rc >= 0)
		rc = alt_zip_writer_close(zw);
	if (rc < 0)
		alt_buf_printf(o, "err %s", tok(rc));
	else {
		alt_buf_append_cstr(o, "ok ");
		puthex(o, out->p, out->n);
	}
	alt_zip_writer_free(zw);
}

int main(void) {
	char *line, **f = malloc(sizeof *f * 100000);
	alt_buf o = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 100000);
		alt_buf_reset(&o);
		if (!strcmp(f[0], "R")) {
			alt_str data = arg(f[1]);
			alt_zip_mem m;
			dump(&o, alt_zip_mem_reader_at(&m, data), (int64_t)data.n, false);
		} else if (!strcmp(f[0], "S")) {
			sparse sp = {strtoll(f[1], 0, 10), 0, 0};
			sp.s = malloc(sizeof *sp.s * (strlen(f[2]) / 2 + 1));
			alt_str rest = alt_str_cstr(f[2]), item;
			while (rest.n && alt_str_split_next(&rest, ALT_STR(";"), &item)) {
				if (!item.n)
					continue;
				char *it = (char *)item.p;
				it[item.n] = 0;
				char *colon = strchr(it, ':');
				*colon = 0;
				sp.s[sp.n].off = strtoll(it, 0, 10);
				sp.s[sp.n++].data = arg(colon + 1);
			}
			dump(&o, (alt_zip_reader_at){sparse_read_at, &sp}, sp.size, true);
			free(sp.s);
		} else if (!strcmp(f[0], "W") || !strcmp(f[0], "C")) {
			alt_buf out = ALT_BUF_INIT;
			alt_zip_writer *zw;
			alt_zip_writer_new(&zw, alt_buf_writer(&out));
			int rc = 0;
			if (f[0][0] == 'W') {
				rc = alt_zip_writer_set_offset(zw, strtoll(f[1], 0, 10));
				if (rc >= 0 && strcmp(f[2], "!"))
					rc = alt_zip_writer_set_comment(zw, arg(f[2]));
				if (rc >= 0)
					rc = write_entries(zw, f + 3, n - 3);
			} else {
				alt_str src = arg(f[1]);
				alt_zip_reader *r;
				rc = alt_zip_reader_new_mem(&r, src);
				for (size_t i = 0; rc >= 0 && i < r->nfile; i++)
					rc = alt_zip_copy(zw, &r->file[i]);
				if (r)
					alt_zip_reader_free(r);
			}
			finish(&o, zw, &out, rc);
			alt_buf_free(&out);
		} else if (!strcmp(f[0], "M")) {
			long cnt = strtol(f[1], 0, 10);
			alt_buf out = ALT_BUF_INIT;
			alt_zip_writer *zw;
			alt_zip_writer_new(&zw, alt_buf_writer(&out));
			int rc = 0;
			for (long i = 0; rc >= 0 && i < cnt; i++) {
				char name[32];
				snprintf(name, sizeof name, "f%ld", i);
				alt_zip_header h = {0};
				h.name = alt_str_cstr(name);
				alt_writer fw;
				rc = alt_zip_create_header(zw, &h, &fw);
			}
			if (rc >= 0)
				rc = alt_zip_writer_close(zw);
			alt_zip_writer_free(zw);
			alt_zip_reader *r;
			if (rc >= 0)
				rc = alt_zip_reader_new_mem(&r, alt_buf_str(&out));
			if (rc < 0)
				alt_buf_printf(&o, "err %s", tok(rc));
			else {
				uint8_t sum[32];
				alt_hash_data(ALT_SHA256, out.p, out.n, sum);
				puthex(&o, sum, 32);
				alt_buf_printf(&o, " %zu", r->nfile);
				alt_zip_reader_free(r);
			}
			alt_buf_free(&out);
		} else
			alt_buf_append_cstr(&o, "unknown");
		fwrite(o.p ? o.p : "", 1, o.n, stdout);
		putchar('\n');
	}
	alt_buf_free(&o);
	free(f);
	return 0;
}
