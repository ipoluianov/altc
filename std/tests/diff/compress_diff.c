/* I hex -> inflate;  G hex -> gunzip;  Z hex -> zlib decode:
	"ok <data hex>" or "<error> <data hex decoded before it>"
   D level cut hex -> deflate (with a flush after cut bytes);
   g level cut hex, z level cut hex -> gzip, zlib: the stream hex */
#include "common.h"
#include <alt/compress/gzip.h>
#include <alt/compress/zlib.h>

static const char *kind(int rc, alt_inflate *f) {
	static char buf[64];
	switch (rc) {
	case -ALT_EFLATE_CORRUPT:
		if (f)
			snprintf(buf, sizeof buf, "corrupt %lld", (long long)alt_inflate_err_offset(f));
		else
			snprintf(buf, sizeof buf, "corrupt");
		return buf;
	case -ALT_EUNEXPECTED_EOF: return "eof";
	case -ALT_ECHECKSUM: return "checksum";
	case -ALT_EGZIP_HEADER: return "header";
	case -ALT_EZLIB_DICT: return "dict";
	case -ALT_EEOF: return "empty";
	}
	snprintf(buf, sizeof buf, "other %d", rc);
	return buf;
}

/* reads r in pieces of varying size, as a consumer would */
static int drain(alt_reader r, alt_buf *out) {
	static const size_t sizes[] = {1, 7, 4096, 100, 65536, 3};
	for (int i = 0;; i = (i + 1) % 6) {
		alt_buf_reserve(out, sizes[i] + 1);
		ssize_t k = alt_read(r, out->p + out->n, sizes[i]);
		if (k <= 0)
			return (int)k;
		out->n += (size_t)k;
	}
}

int main(void) {
	char *line, *f[4];
	alt_buf out = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 4);
		alt_buf_reset(&out);
		char op = line[0];
		if (op == 'I' || op == 'G' || op == 'Z') {
			size_t len = n > 1 ? diff_unhex(f[1]) : 0;
			alt_str_reader sr;
			alt_reader src = alt_str_reader_init(&sr, alt_str_make(f[1], len));
			int rc;
			alt_inflate *fl = 0;
			if (op == 'I') {
				rc = alt_inflate_new(&fl, src);
				if (!rc)
					rc = drain(alt_inflate_reader(fl), &out);
			} else if (op == 'G') {
				alt_gzip_reader *z;
				rc = alt_gzip_reader_new(&z, src);
				if (!rc) {
					rc = drain(alt_gzip_reader_reader(z), &out);
					alt_gzip_reader_free(z);
				}
			} else {
				alt_zlib_reader *z;
				rc = alt_zlib_reader_new(&z, src, 0, 0);
				if (!rc) {
					rc = drain(alt_zlib_reader_reader(z), &out);
					alt_zlib_reader_free(z);
				}
			}
			printf("%s ", rc ? kind(rc, fl) : "ok");
			alt_inflate_free(fl);
			diff_puthex(out.p, out.n);
			printf("\n");
			continue;
		}
		int level = atoi(f[1]);
		size_t cut = (size_t)atoi(f[2]);
		size_t len = n > 3 ? diff_unhex(f[3]) : 0;
		if (cut > len)
			cut = len;
		int rc;
		alt_writer w = alt_buf_writer(&out);
		if (op == 'D') {
			alt_deflate *z;
			rc = alt_deflate_new(&z, w, level);
			if (!rc) {
				alt_deflate_write(z, f[3], cut);
				alt_deflate_flush(z);
				alt_deflate_write(z, f[3] + cut, len - cut);
				rc = alt_deflate_close(z);
				alt_deflate_free(z);
			}
		} else if (op == 'g') {
			alt_gzip_writer *z;
			alt_gzip_header h = {.name = "name\xc3\xa9.txt", .comment = "c", .extra = "xy", .nextra = 2, .mtime = 1234567890,
				.os = 3};
			rc = alt_gzip_writer_new(&z, w, level, &h);
			if (!rc) {
				alt_gzip_write(z, f[3], cut);
				alt_gzip_flush(z);
				alt_gzip_write(z, f[3] + cut, len - cut);
				rc = alt_gzip_close(z);
				alt_gzip_writer_free(z);
			}
		} else {
			alt_zlib_writer *z;
			rc = alt_zlib_writer_new(&z, w, level, 0, 0);
			if (!rc) {
				alt_zlib_write(z, f[3], cut);
				alt_zlib_flush(z);
				alt_zlib_write(z, f[3] + cut, len - cut);
				rc = alt_zlib_close(z);
				alt_zlib_writer_free(z);
			}
		}
		if (rc)
			printf("error %d\n", rc);
		else {
			diff_puthex(out.p, out.n);
			printf("\n");
		}
	}
	alt_buf_free(&out);
	return 0;
}
