/* archive/tar: writing and reading back, the errors of the API (also run
   under the sanitizers; tests/stdtar_test.go compares with Go) */
#include "check.h"
#include <alt/archive/tar.h>

static alt_tar_reader *open_buf(alt_str_reader *sr, const alt_buf *b) {
	alt_tar_reader *r;
	CHECK_OK(alt_tar_reader_new(&r, alt_str_reader_init(sr, alt_buf_str(b))));
	return r;
}

static void roundtrip(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_tar_writer *w;
	CHECK_OK(alt_tar_writer_new(&w, alt_buf_writer(&out)));
	char longname[300];
	memset(longname, 'n', sizeof longname);
	alt_tar_record recs[] = {{ALT_STR("GOLANG.pkg"), ALT_STR("tar")}, {ALT_STR("SCHILY.xattr.user.k"), ALT_STR("v")}};
	alt_tar_header hs[] = {
		{.typeflag = ALT_TAR_REG, .name = ALT_STR("a.txt"), .size = 5, .mode = 0644, .uname = ALT_STR("root"),
			.mtime = alt_tar_unix(1700000000, 0)},
		{.typeflag = ALT_TAR_DIR, .name = ALT_STR("dir/"), .mode = 0755, .mtime = alt_tar_unix(-5, 0)},
		{.typeflag = ALT_TAR_REG, .name = alt_str_make(longname, sizeof longname), .size = 3, .uid = 1 << 22,
			.mtime = alt_tar_unix(1, 250000000), .atime = alt_tar_unix(2, 0), .format = ALT_TAR_FORMAT_PAX, .pax = recs, .npax = 2},
		{.typeflag = ALT_TAR_SYMLINK, .name = ALT_STR("gnu"), .linkname = alt_str_make(longname, 200),
			.mtime = alt_tar_unix(1, 0), .ctime = alt_tar_unix(77, 0), .format = ALT_TAR_FORMAT_GNU},
		{.typeflag = ALT_TAR_XGLOBAL, .pax = recs, .npax = 1},
	};
	const char *data[] = {"hello", "", "xyz", "", ""};
	for (size_t i = 0; i < sizeof hs / sizeof *hs; i++) {
		CHECK_OK(alt_tar_write_header(w, &hs[i]));
		CHECK_INT(alt_write_all(alt_tar_writer_writer(w), data[i], strlen(data[i])), (long long)strlen(data[i]));
	}
	CHECK_OK(alt_tar_writer_close(w));
	CHECK_OK(alt_tar_writer_close(w));
	CHECK_ERR(alt_tar_write_header(w, &hs[0]), -ALT_ECLOSED);
	CHECK_STR(alt_tar_writer_error(w), "archive/tar: write after close");
	alt_tar_writer_free(w);
	CHECK_INT(out.n % 512, 0);

	alt_str_reader sr;
	alt_tar_reader *r = open_buf(&sr, &out);
	alt_tar_header *h;
	CHECK_OK(alt_tar_next(r, &h));
	CHECK_STR(h->name, "a.txt");
	CHECK_STR(h->uname, "root");
	CHECK_INT(h->format, ALT_TAR_FORMAT_USTAR);
	CHECK_INT(h->mtime.sec, 1700000000);
	alt_buf d = ALT_BUF_INIT;
	CHECK_INT(alt_read_all(alt_tar_reader_reader(r), &d, 0), 5);
	CHECK_STR(alt_buf_cstr(&d), "hello");
	CHECK_OK(alt_tar_next(r, &h));
	CHECK_INT(h->typeflag, ALT_TAR_DIR);
	CHECK_INT(h->mtime.sec, -5);
	CHECK_INT(h->format, ALT_TAR_FORMAT_PAX); /* a negative mtime needs a record */
	CHECK_OK(alt_tar_next(r, &h));
	CHECK_INT(h->name.n, sizeof longname);
	CHECK_INT(h->uid, 1 << 22);
	CHECK_INT(h->mtime.nsec, 250000000);
	CHECK_INT(h->atime.sec, 2);
	alt_str v;
	CHECK(alt_tar_header_pax(h, ALT_STR("GOLANG.pkg"), &v) && alt_str_eq_cstr(v, "tar"));
	CHECK(alt_tar_header_pax(h, ALT_STR("mtime"), &v) && alt_str_eq_cstr(v, "1.25"));
	CHECK_OK(alt_tar_next(r, &h)); /* skips the data not read */
	CHECK_INT(h->typeflag, ALT_TAR_SYMLINK);
	CHECK_INT(h->linkname.n, 200);
	CHECK_INT(h->format, ALT_TAR_FORMAT_GNU);
	CHECK_INT(h->ctime.sec, 77);
	CHECK_OK(alt_tar_next(r, &h));
	CHECK_INT(h->typeflag, ALT_TAR_XGLOBAL);
	CHECK_INT(h->npax, 1);
	CHECK_ERR(alt_tar_next(r, &h), -ALT_EEOF);
	CHECK_STR(alt_tar_reader_error(r), "EOF");
	CHECK_ERR(alt_tar_next(r, &h), -ALT_EEOF);
	alt_tar_reader_free(r);
	alt_buf_free(&d);
	alt_buf_free(&out);
}

static void errors(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_tar_writer *w;
	CHECK_OK(alt_tar_writer_new(&w, alt_buf_writer(&out)));
	alt_tar_header h = {.typeflag = ALT_TAR_REG, .name = ALT_STR("f"), .size = 3};
	CHECK_OK(alt_tar_write_header(w, &h));
	CHECK_INT(alt_tar_write(w, "abcdef", 6), 3);
	CHECK_ERR(alt_tar_write(w, "def", 3), -ALT_ETAR_WRITE_TOO_LONG);
	CHECK_STR(alt_tar_writer_error(w), "archive/tar: write too long");
	h.name = ALT_STR("dir/");
	CHECK_ERR(alt_tar_write_header(w, &h), -ALT_ETAR_FIELD);
	CHECK_STR(alt_tar_writer_error(w), "archive/tar: cannot encode header: filename may not have trailing slash");
	h = (alt_tar_header){.typeflag = ALT_TAR_REG, .name = ALT_STR("big"), .uid = -1, .format = ALT_TAR_FORMAT_USTAR};
	CHECK_ERR(alt_tar_write_header(w, &h), -ALT_ETAR_FIELD);
	CHECK_STR(alt_tar_writer_error(w), "archive/tar: cannot encode header: Format specifies USTAR; and USTAR cannot encode Uid=-1");
	h = (alt_tar_header){.typeflag = ALT_TAR_REG, .name = ALT_STR("t"), .mtime = alt_tar_unix(-1, 0), .format = ALT_TAR_FORMAT_USTAR};
	CHECK_ERR(alt_tar_write_header(w, &h), -ALT_ETAR_FIELD);
	CHECK_STR(alt_tar_writer_error(w),
		"archive/tar: cannot encode header: Format specifies USTAR; and USTAR cannot encode ModTime=1969-12-31 23:59:59 +0000 UTC");
	h = (alt_tar_header){.typeflag = ALT_TAR_REG, .name = ALT_STR("short"), .size = 10};
	CHECK_OK(alt_tar_write_header(w, &h));
	CHECK_INT(alt_tar_write(w, "x", 1), 1);
	CHECK_ERR(alt_tar_writer_close(w), -EINVAL);
	CHECK_STR(alt_tar_writer_error(w), "archive/tar: missed writing 9 bytes");
	alt_tar_writer_free(w);

	/* a truncated archive: the data ends early, and the error stays */
	alt_str_reader sr;
	alt_tar_reader *r = open_buf(&sr, &out);
	alt_tar_header *hp;
	CHECK_OK(alt_tar_next(r, &hp));
	CHECK_OK(alt_tar_next(r, &hp));
	CHECK_STR(hp->name, "short");
	char buf[64];
	CHECK_INT(alt_tar_read(r, buf, sizeof buf), 1);
	CHECK_ERR(alt_tar_read(r, buf, sizeof buf), -ALT_EUNEXPECTED_EOF);
	CHECK_STR(alt_tar_reader_error(r), "unexpected EOF");
	CHECK_ERR(alt_tar_next(r, &hp), -ALT_EUNEXPECTED_EOF);
	alt_tar_reader_free(r);

	/* garbage */
	alt_buf_reset(&out);
	for (int i = 0; i < 512; i++)
		alt_buf_append_byte(&out, 'z');
	r = open_buf(&sr, &out);
	CHECK_ERR(alt_tar_next(r, &hp), -ALT_ETAR_HEADER);
	CHECK_STR(alt_tar_reader_error(r), "archive/tar: invalid tar header");
	alt_tar_reader_free(r);
	alt_buf_free(&out);
}

static void insecure(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_tar_writer *w;
	CHECK_OK(alt_tar_writer_new(&w, alt_buf_writer(&out)));
	alt_tar_header h = {.typeflag = ALT_TAR_REG, .name = ALT_STR("../etc/passwd")};
	CHECK_OK(alt_tar_write_header(w, &h));
	h.name = ALT_STR("ok");
	CHECK_OK(alt_tar_write_header(w, &h));
	CHECK_OK(alt_tar_writer_close(w));
	alt_tar_writer_free(w);
	alt_str_reader sr;
	alt_tar_reader *r = open_buf(&sr, &out);
	alt_tar_reader_check_paths(r, true);
	alt_tar_header *hp;
	CHECK_ERR(alt_tar_next(r, &hp), -ALT_ETAR_INSECURE_PATH);
	CHECK(hp && alt_str_eq_cstr(hp->name, "../etc/passwd"));
	CHECK_OK(alt_tar_next(r, &hp)); /* not sticky */
	CHECK_STR(hp->name, "ok");
	alt_tar_reader_free(r);
	alt_buf_free(&out);
}

/* a PAX 1.0 sparse file: "ab" at 0 and "cd" at 10 of 14 bytes */
static void sparse(void) {
	static const char recs[] = "22 GNU.sparse.major=1\n22 GNU.sparse.minor=0\n22 GNU.sparse.name=sp\n26 GNU.sparse.realsize=14\n";
	alt_buf data = ALT_BUF_INIT, out = ALT_BUF_INIT;
	alt_buf_append_cstr(&data, "2\n0\n2\n10\n2\n");
	while (data.n % 512)
		alt_buf_append_byte(&data, 0);
	alt_buf_append_cstr(&data, "abcd");
	/* written as plain entries, read back as a sparse file */
	alt_tar_writer *w;
	CHECK_OK(alt_tar_writer_new(&w, alt_buf_writer(&out)));
	alt_tar_header h = {.typeflag = ALT_TAR_REG, .name = ALT_STR("PaxHeaders.0/sp"), .size = sizeof recs - 1};
	CHECK_OK(alt_tar_write_header(w, &h));
	CHECK_INT(alt_tar_write(w, recs, sizeof recs - 1), sizeof recs - 1);
	h = (alt_tar_header){.typeflag = ALT_TAR_REG, .name = ALT_STR("GNUSparseFile.0/sp"), .size = (int64_t)data.n};
	CHECK_OK(alt_tar_write_header(w, &h));
	CHECK_INT(alt_tar_write(w, data.p, data.n), data.n);
	CHECK_OK(alt_tar_writer_close(w));
	alt_tar_writer_free(w);
	out.p[156] = 'x'; /* the first entry becomes the PAX header */
	memcpy(out.p + 148, "        ", 8);
	unsigned sum = 0;
	for (int i = 0; i < 512; i++)
		sum += (unsigned char)out.p[i];
	snprintf(out.p + 148, 8, "%06o", sum);
	alt_str_reader sr;
	alt_tar_reader *r = open_buf(&sr, &out);
	alt_tar_header *hp;
	CHECK_OK(alt_tar_next(r, &hp));
	CHECK_STR(hp->name, "sp");
	CHECK_INT(hp->size, 14);
	alt_buf got = ALT_BUF_INIT;
	CHECK_INT(alt_read_all(alt_tar_reader_reader(r), &got, 0), 14);
	CHECK(got.n == 14 && memcmp(got.p, "ab\0\0\0\0\0\0\0\0cd\0\0", 14) == 0);
	CHECK_ERR(alt_tar_next(r, &hp), -ALT_EEOF);
	alt_tar_reader_free(r);
	alt_buf_free(&got);
	alt_buf_free(&data);
	alt_buf_free(&out);
}

int main(void) {
	RUN(roundtrip);
	RUN(errors);
	RUN(insecure);
	RUN(sparse);
	return check_done();
}
