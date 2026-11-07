#include "check.h"
#include <alt/archive/zip.h>
#include <alt/hash/crc32.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

static uint64_t rng = 0x9e3779b97f4a7c15ull;
static uint32_t next(void) {
	rng ^= rng << 13, rng ^= rng >> 7, rng ^= rng << 17;
	return (uint32_t)rng;
}

static void make_data(alt_buf *b, size_t n) {
	alt_buf_reset(b);
	static const char *words[] = {"zip ", "archive ", "central ", "directory\n"};
	while (b->n < n) {
		if (next() % 3)
			alt_buf_append_cstr(b, words[next() % 4]);
		else
			alt_buf_append_byte(b, (int)(next() & 0xff));
	}
	alt_buf_truncate(b, n);
}

/* reads a file of the archive fully */
static int read_file(const alt_zip_file *f, alt_buf *out) {
	alt_zip_file_reader *fr;
	int rc = alt_zip_file_open(f, &fr);
	if (rc < 0)
		return rc;
	alt_buf_reset(out);
	ssize_t k = alt_read_all(alt_zip_file_stream(fr), out, 0);
	alt_zip_file_close(fr);
	return k < 0 ? (int)k : 0;
}

/* archives of files of many sizes, stored and deflated, read back */
static void roundtrip(void) {
	size_t sizes[] = {0, 1, 100, 32768, 65537, 300000, 2000000};
	enum { N = sizeof sizes / sizeof *sizes };
	alt_buf data[N], arc = ALT_BUF_INIT, back = ALT_BUF_INIT;
	alt_zip_writer *zw;
	CHECK_OK(alt_zip_writer_new(&zw, alt_buf_writer(&arc)));
	CHECK_OK(alt_zip_writer_set_comment(zw, ALT_STR("the comment")));
	for (size_t i = 0; i < N; i++) {
		alt_buf_init(&data[i]);
		make_data(&data[i], sizes[i]);
		char name[32];
		snprintf(name, sizeof name, "dir/file%zu.bin", i);
		alt_zip_header h = {0};
		h.name = alt_str_cstr(name);
		h.method = i % 2 ? ALT_ZIP_STORE : ALT_ZIP_DEFLATE;
		alt_zip_header_set_mod_time(&h, (alt_wall)1700000000 * ALT_SEC + 123);
		alt_zip_header_set_mode(&h, 0100644);
		alt_writer fw;
		CHECK_OK(alt_zip_create_header(zw, &h, &fw));
		for (size_t off = 0; off < data[i].n;) {
			size_t k = next() % 50000 + 1;
			if (k > data[i].n - off)
				k = data[i].n - off;
			CHECK_INT(alt_write_all(fw, data[i].p + off, k), k);
			off += k;
		}
	}
	alt_writer dw;
	CHECK_OK(alt_zip_create(zw, ALT_STR("dir/"), &dw));
	CHECK_INT(alt_write(dw, "", 0), 0);
	CHECK_ERR(alt_write(dw, "x", 1), -EINVAL);
	CHECK_OK(alt_zip_writer_close(zw));
	CHECK_ERR(alt_zip_writer_close(zw), -EINVAL);
	alt_zip_writer_free(zw);

	alt_zip_reader *zr;
	CHECK_OK(alt_zip_reader_new_mem(&zr, alt_buf_str(&arc)));
	CHECK_INT(zr->nfile, N + 1);
	CHECK_STR(zr->comment, "the comment");
	for (size_t i = 0; i < N && i < zr->nfile; i++) {
		alt_zip_file *f = &zr->file[i];
		CHECK_INT(f->hdr.uncompressed_size64, sizes[i]);
		CHECK_INT(f->hdr.crc32, alt_crc32_ieee(data[i].p, data[i].n));
		CHECK_INT(f->hdr.modified, (alt_wall)1700000000 * ALT_SEC);
		CHECK_INT(alt_zip_header_mode(&f->hdr), 0100644);
		CHECK_OK(read_file(f, &back));
		CHECK(alt_str_eq(alt_buf_str(&back), alt_buf_str(&data[i])));
	}
	CHECK_INT(alt_zip_header_mode(&zr->file[N].hdr), 040666); /* FAT creator: 0666 */
	CHECK_OK(read_file(&zr->file[N], &back));
	CHECK_INT(back.n, 0);
	CHECK_OK(alt_zip_reader_check_paths(zr));
	alt_zip_reader_free(zr);

	/* a corrupted byte of stored data: the CRC-32 no longer matches */
	alt_buf bad = ALT_BUF_INIT;
	alt_buf_append(&bad, arc.p, arc.n);
	CHECK_OK(alt_zip_reader_new_mem(&zr, alt_buf_str(&bad)));
	int64_t off;
	CHECK_OK(alt_zip_file_data_offset(&zr->file[3], &off));
	bad.p[off + 10] ^= 1;
	CHECK_ERR(read_file(&zr->file[3], &back), -ALT_ECHECKSUM);
	CHECK_INT(back.n, sizes[3]); /* the data comes before the error */
	alt_zip_reader_free(zr);

	/* a truncated archive */
	CHECK_ERR(alt_zip_reader_new_mem(&zr, alt_str_make(arc.p, arc.n - 30)), -ALT_EZIP_FORMAT);
	CHECK_ERR(alt_zip_reader_new_mem(&zr, ALT_STR("")), -ALT_EZIP_FORMAT);

	for (size_t i = 0; i < N; i++)
		alt_buf_free(&data[i]);
	alt_buf_free(&bad);
	alt_buf_free(&arc);
	alt_buf_free(&back);
}

/* the same archive read through a file and copied with alt_zip_copy */
static void files_and_copy(void) {
	alt_buf arc = ALT_BUF_INIT, data = ALT_BUF_INIT, back = ALT_BUF_INIT;
	alt_zip_writer *zw;
	CHECK_OK(alt_zip_writer_new(&zw, alt_buf_writer(&arc)));
	make_data(&data, 100000);
	for (int i = 0; i < 20; i++) {
		char name[32];
		snprintf(name, sizeof name, "f%02d", i);
		alt_writer fw;
		CHECK_OK(alt_zip_create(zw, alt_str_cstr(name), &fw));
		CHECK_INT(alt_write_all(fw, data.p, (size_t)i * 5000), i * 5000);
	}
	CHECK_OK(alt_zip_writer_close(zw));
	alt_zip_writer_free(zw);

	char path[] = "/tmp/alt_zip_testXXXXXX";
	int fd = mkstemp(path);
	CHECK(fd >= 0);
	CHECK_INT(write(fd, arc.p, arc.n), arc.n);
	close(fd);
	alt_zip_reader *zr;
	CHECK_OK(alt_zip_open(&zr, path));
	unlink(path);
	alt_zip_reader *none;
	CHECK_ERR(alt_zip_open(&none, "/nonexistent/x.zip"), -ENOENT);
	CHECK_INT(zr->nfile, 20);

	/* all files open at once, read in turns */
	alt_zip_file_reader *fr[20];
	alt_buf parts[20];
	for (int i = 0; i < 20; i++) {
		alt_buf_init(&parts[i]);
		CHECK_OK(alt_zip_file_open(&zr->file[i], &fr[i]));
	}
	for (bool more = true; more;) {
		more = false;
		for (int i = 0; i < 20; i++) {
			char buf[777];
			ssize_t k = alt_zip_file_read(fr[i], buf, sizeof buf);
			CHECK(k >= 0);
			if (k > 0) {
				alt_buf_append(&parts[i], buf, (size_t)k);
				more = true;
			}
		}
	}
	for (int i = 0; i < 20; i++) {
		CHECK(alt_str_eq(alt_buf_str(&parts[i]), alt_str_make(data.p, (size_t)i * 5000)));
		alt_zip_file_close(fr[i]);
		alt_buf_free(&parts[i]);
	}

	alt_buf copy = ALT_BUF_INIT;
	CHECK_OK(alt_zip_writer_new(&zw, alt_buf_writer(&copy)));
	for (size_t i = 0; i < zr->nfile; i++)
		CHECK_OK(alt_zip_copy(zw, &zr->file[i]));
	CHECK_OK(alt_zip_writer_close(zw));
	alt_zip_writer_free(zw);
	alt_zip_reader_free(zr);
	/* copied raw: the same bytes */
	CHECK(copy.n == arc.n && !memcmp(copy.p, arc.p, arc.n));
	alt_buf_free(&copy);
	alt_buf_free(&arc);
	alt_buf_free(&data);
	alt_buf_free(&back);
}

static void headers(void) {
	alt_buf arc = ALT_BUF_INIT;
	alt_zip_writer *zw;
	CHECK_OK(alt_zip_writer_new(&zw, alt_buf_writer(&arc)));
	alt_writer fw;
	CHECK_OK(alt_zip_create(zw, ALT_STR("a"), &fw));
	CHECK_ERR(alt_zip_writer_set_offset(zw, 10), -EINVAL);
	alt_zip_header h = {0};
	h.name = ALT_STR("x");
	h.method = 12;
	CHECK_ERR(alt_zip_create_header(zw, &h, &fw), -ALT_EZIP_ALGORITHM);
	char big[70000];
	memset(big, 'c', sizeof big);
	CHECK_ERR(alt_zip_writer_set_comment(zw, alt_str_make(big, sizeof big)), -EINVAL);
	/* a name in UTF-8 gets the flag, a modification time in a zone */
	h = (alt_zip_header){0};
	h.name = ALT_STR("../日本.txt");
	h.modified_set = true;
	h.modified = (alt_wall)1700000000 * ALT_SEC;
	h.modified_offset = 3600;
	CHECK_OK(alt_zip_create_header(zw, &h, &fw));
	CHECK_INT(alt_write_all(fw, "abc", 3), 3);
	/* a stored raw file without data descriptor */
	h = (alt_zip_header){0};
	h.name = ALT_STR("raw");
	h.crc32 = alt_crc32_ieee("hello", 5);
	h.compressed_size64 = h.uncompressed_size64 = 5;
	CHECK_OK(alt_zip_create_raw(zw, &h, &fw));
	CHECK_INT(alt_write_all(fw, "hello", 5), 5);
	CHECK_OK(alt_zip_writer_close(zw));
	alt_zip_writer_free(zw);

	alt_zip_reader *zr;
	CHECK_OK(alt_zip_reader_new_mem(&zr, alt_buf_str(&arc)));
	CHECK_INT(zr->nfile, 3);
	if (zr->nfile == 3) {
		alt_zip_header *g = &zr->file[1].hdr;
		CHECK(g->flags & ALT_ZIP_FLAG_UTF8);
		CHECK(!g->non_utf8);
		CHECK(g->modified_has_zone);
		CHECK_INT(g->modified_offset, 3600);
		CHECK_INT(g->modified, (alt_wall)1700000000 * ALT_SEC);
		CHECK_INT(zr->file[2].hdr.flags, 0);
		alt_buf back = ALT_BUF_INIT;
		CHECK_OK(read_file(&zr->file[2], &back));
		CHECK_STR(alt_buf_str(&back), "hello");
		alt_buf_free(&back);
	}
	CHECK_ERR(alt_zip_reader_check_paths(zr), -ALT_EZIP_INSECURE_PATH);
	alt_zip_reader_free(zr);
	alt_buf_free(&arc);

	/* modes and MS-DOS times */
	h = (alt_zip_header){0};
	h.name = ALT_STR("l");
	alt_zip_header_set_mode(&h, 0120777);
	CHECK_INT(h.creator_version >> 8, ALT_ZIP_CREATOR_UNIX);
	CHECK_INT(alt_zip_header_mode(&h), 0120777);
	alt_zip_header_set_mode(&h, 040555);
	CHECK_INT(h.external_attrs & 0x11, 0x11);
	h.creator_version = 0;
	h.external_attrs = 0x10;
	CHECK_INT(alt_zip_header_mode(&h), 040777);
	h.external_attrs = 0x01;
	CHECK_INT(alt_zip_header_mode(&h), 0100444);
	alt_zip_header_set_mod_time(&h, (alt_wall)1700000001 * ALT_SEC);
	CHECK_INT(alt_zip_header_mod_time(&h), (alt_wall)1700000000 * ALT_SEC);
}

static void *worker(void *arg) {
	alt_str arc = *(alt_str *)arg;
	alt_buf back = ALT_BUF_INIT;
	for (int round = 0; round < 20; round++) {
		alt_zip_reader *zr;
		if (alt_zip_reader_new_mem(&zr, arc) < 0) {
			FAILED("reader");
			break;
		}
		for (size_t i = 0; i < zr->nfile; i++)
			if (read_file(&zr->file[i], &back) < 0 || back.n != zr->file[i].hdr.uncompressed_size64)
				FAILED("file %zu", i);
		alt_zip_reader_free(zr);
	}
	alt_buf_free(&back);
	return 0;
}

/* readers of one archive in several threads */
static void threads(void) {
	alt_buf arc = ALT_BUF_INIT, data = ALT_BUF_INIT;
	alt_zip_writer *zw;
	CHECK_OK(alt_zip_writer_new(&zw, alt_buf_writer(&arc)));
	make_data(&data, 50000);
	for (int i = 0; i < 8; i++) {
		char name[16];
		snprintf(name, sizeof name, "t%d", i);
		alt_writer fw;
		CHECK_OK(alt_zip_create(zw, alt_str_cstr(name), &fw));
		CHECK_INT(alt_write_all(fw, data.p, data.n - (size_t)i * 1000), data.n - (size_t)i * 1000);
	}
	CHECK_OK(alt_zip_writer_close(zw));
	alt_zip_writer_free(zw);
	alt_str s = alt_buf_str(&arc);
	pthread_t th[4];
	for (int i = 0; i < 4; i++)
		pthread_create(&th[i], 0, worker, &s);
	for (int i = 0; i < 4; i++)
		pthread_join(th[i], 0);
	alt_buf_free(&arc);
	alt_buf_free(&data);
}

int main(void) {
	RUN(roundtrip);
	RUN(files_and_copy);
	RUN(headers);
	RUN(threads);
	return check_done();
}
