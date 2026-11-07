#include "check.h"
#include <alt/compress/gzip.h>
#include <alt/compress/zlib.h>
#include <alt/hash/crc32.h>
#include <pthread.h>

static uint64_t rng = 88172645463325252ull;
static uint32_t next(void) {
	rng ^= rng << 13, rng ^= rng >> 7, rng ^= rng << 17;
	return (uint32_t)rng;
}

/* data of several kinds: random, runs, text-like, mixed */
static void make_data(alt_buf *b, size_t n, int kind) {
	alt_buf_reset(b);
	static const char *words[] = {"alpha ", "beta ", "gamma ", "delta\n", "epsilon ", "zeta, "};
	while (b->n < n) {
		switch (kind) {
		case 0: alt_buf_append_byte(b, (int)(next() & 0xff)); break;
		case 1: {
			int c = (int)(next() & 3), k = (int)(next() % 300);
			for (int i = 0; i < k; i++)
				alt_buf_append_byte(b, 'a' + c);
			break;
		}
		case 2: alt_buf_append_cstr(b, words[next() % 6]); break;
		default:
			if (next() % 2)
				alt_buf_append_cstr(b, words[next() % 6]);
			else
				alt_buf_append_byte(b, (int)(next() & 0xff));
		}
	}
	alt_buf_truncate(b, n);
}

static void roundtrips(void) {
	alt_buf data = ALT_BUF_INIT, comp = ALT_BUF_INIT, back = ALT_BUF_INIT;
	size_t sizes[] = {0, 1, 100, 32768, 32769, 65536, 100000, 1500000};
	int levels[] = {-2, -1, 0, 1, 3, 4, 6, 9};
	for (size_t si = 0; si < sizeof sizes / sizeof *sizes; si++)
		for (int kind = 0; kind < 4; kind++)
			for (size_t li = 0; li < 8; li++) {
				if (sizes[si] > 200000 && li % 3)
					continue;
				make_data(&data, sizes[si], kind);
				alt_buf_reset(&comp);
				alt_deflate *z;
				CHECK_OK(alt_deflate_new(&z, alt_buf_writer(&comp), levels[li]));
				/* writes of random sizes, with flushes */
				for (size_t off = 0; off < data.n;) {
					size_t k = next() % 70000 + 1;
					if (k > data.n - off)
						k = data.n - off;
					CHECK_INT(alt_deflate_write(z, data.p + off, k), k);
					off += k;
					if (next() % 8 == 0)
						CHECK_OK(alt_deflate_flush(z));
				}
				CHECK_OK(alt_deflate_close(z));
				CHECK_INT(alt_deflate_write(z, "x", 1), -ALT_ECLOSED);
				alt_deflate_free(z);
				alt_buf_reset(&back);
				CHECK_OK(alt_inflate_data(&back, comp.p, comp.n, 0));
				if (back.n != data.n || (data.n && memcmp(back.p, data.p, data.n) != 0))
					FAILED("size %zu kind %d level %d: round trip differs", sizes[si], kind, levels[li]);
				if (kind == 1 && levels[li] > 0 && data.n > 1000 && comp.n * 20 > data.n)
					FAILED("runs compress badly: %zu -> %zu", data.n, comp.n);
			}
	/* the limit of alt_inflate_data */
	make_data(&data, 100000, 2);
	alt_buf_reset(&comp);
	CHECK_OK(alt_deflate_data(&comp, data.p, data.n, 6));
	alt_buf_reset(&back);
	CHECK_INT(alt_inflate_data(&back, comp.p, comp.n, 1000), -ALT_ETOOLARGE);
	CHECK_INT(back.n, 1000);
	CHECK_INT(alt_deflate_new(&(alt_deflate *){0}, alt_discard, 10), -EINVAL);
	alt_buf_free(&data);
	alt_buf_free(&comp);
	alt_buf_free(&back);
}

static void dictionaries(void) {
	const char *dict = "the common words of the messages: hello world, goodbye world";
	const char *msg = "hello world, hello world, goodbye world";
	alt_buf comp = ALT_BUF_INIT, plain = ALT_BUF_INIT, back = ALT_BUF_INIT;
	alt_deflate *z;
	CHECK_OK(alt_deflate_new_dict(&z, alt_buf_writer(&comp), 9, dict, strlen(dict)));
	alt_deflate_write(z, msg, strlen(msg));
	CHECK_OK(alt_deflate_close(z));
	CHECK_OK(alt_deflate_data(&plain, msg, strlen(msg), 9));
	CHECK(comp.n < plain.n);
	/* reset keeps the dictionary */
	alt_buf second = ALT_BUF_INIT;
	alt_deflate_reset(z, alt_buf_writer(&second));
	alt_deflate_write(z, msg, strlen(msg));
	CHECK_OK(alt_deflate_close(z));
	CHECK(second.n == comp.n && memcmp(second.p, comp.p, comp.n) == 0);
	alt_deflate_free(z);
	alt_str_reader sr;
	alt_inflate *f;
	CHECK_OK(alt_inflate_new(&f, alt_str_reader_init(&sr, alt_buf_str(&comp))));
	alt_inflate_set_dict(f, dict, strlen(dict));
	CHECK_OK(alt_read_all(alt_inflate_reader(f), &back, 0));
	CHECK_STR(alt_buf_str(&back), msg);
	alt_inflate_free(f);

	/* zlib with a dictionary: the reader must be given it */
	alt_buf_reset(&comp);
	alt_zlib_writer *zw;
	CHECK_OK(alt_zlib_writer_new(&zw, alt_buf_writer(&comp), 6, dict, strlen(dict)));
	alt_zlib_write(zw, msg, strlen(msg));
	CHECK_OK(alt_zlib_close(zw));
	alt_zlib_writer_free(zw);
	alt_zlib_reader *zr;
	CHECK_INT(alt_zlib_reader_new(&zr, alt_str_reader_init(&sr, alt_buf_str(&comp)), 0, 0), -ALT_EZLIB_DICT);
	CHECK_INT(alt_zlib_reader_new(&zr, alt_str_reader_init(&sr, alt_buf_str(&comp)), "x", 1), -ALT_EZLIB_DICT);
	CHECK_OK(alt_zlib_reader_new(&zr, alt_str_reader_init(&sr, alt_buf_str(&comp)), dict, strlen(dict)));
	alt_buf_reset(&back);
	CHECK_OK(alt_read_all(alt_zlib_reader_reader(zr), &back, 0));
	CHECK_STR(alt_buf_str(&back), msg);
	alt_zlib_reader_free(zr);
	alt_buf_free(&comp);
	alt_buf_free(&plain);
	alt_buf_free(&back);
	alt_buf_free(&second);
}

static void gzip_members(void) {
	alt_buf comp = ALT_BUF_INIT, back = ALT_BUF_INIT;
	alt_gzip_header h = {.name = "caf\xc3\xa9.txt", .comment = "note", .extra = "\x01\x02", .nextra = 2, .mtime = 1700000000,
		.os = 3};
	alt_gzip_writer *w;
	CHECK_OK(alt_gzip_writer_new(&w, alt_buf_writer(&comp), 6, &h));
	CHECK_INT(alt_gzip_write(w, "one ", 4), 4);
	CHECK_OK(alt_gzip_flush(w));
	CHECK_INT(alt_gzip_write(w, "two ", 4), 4);
	CHECK_OK(alt_gzip_close(w));
	alt_gzip_writer_free(w);
	size_t first = comp.n;
	CHECK_OK(alt_gzip_data(&comp, "three", 5, 1));
	/* header strings must be Latin-1 */
	alt_gzip_header bad = {.name = "\xe2\x82\xac"};
	CHECK_INT(alt_gzip_writer_new(&w, alt_discard, 6, &bad), -EINVAL);

	alt_str_reader sr;
	alt_gzip_reader *r;
	CHECK_OK(alt_gzip_reader_new(&r, alt_str_reader_init(&sr, alt_buf_str(&comp))));
	const alt_gzip_header *got = alt_gzip_reader_header(r);
	CHECK_STR(got->name, "caf\xc3\xa9.txt");
	CHECK_STR(got->comment, "note");
	CHECK_INT(got->nextra, 2);
	CHECK_INT(got->mtime, 1700000000);
	CHECK_INT(got->os, 3);
	CHECK_OK(alt_read_all(alt_gzip_reader_reader(r), &back, 0));
	CHECK_STR(alt_buf_str(&back), "one two three");
	alt_gzip_reader_free(r);
	/* one member only */
	CHECK_OK(alt_gzip_reader_new(&r, alt_str_reader_init(&sr, alt_buf_str(&comp))));
	alt_gzip_reader_multistream(r, false);
	alt_buf_reset(&back);
	CHECK_OK(alt_read_all(alt_gzip_reader_reader(r), &back, 0));
	CHECK_STR(alt_buf_str(&back), "one two ");
	alt_gzip_reader_free(r);
	/* a damaged checksum */
	comp.p[first - 6] ^= 1;
	alt_buf_reset(&back);
	CHECK_INT(alt_gunzip_data(&back, comp.p, comp.n, 0), -ALT_ECHECKSUM);
	CHECK_INT(alt_gunzip_data(&back, "", 0, 0), -ALT_EEOF);
	CHECK_INT(alt_gunzip_data(&back, "\x1f\x8b\x09", 3, 0), -ALT_EUNEXPECTED_EOF);
	alt_buf_free(&comp);
	alt_buf_free(&back);
}

/* compressing and decompressing in many threads at once */
static void *worker(void *arg) {
	alt_buf data = ALT_BUF_INIT, comp = ALT_BUF_INIT, back = ALT_BUF_INIT;
	for (int i = 0; i < 20; i++) {
		alt_buf_reset(&data);
		for (int k = 0; k < 2000 + i * 100; k++)
			alt_buf_printf(&data, "%d:%d ", (int)(intptr_t)arg, k % 97);
		alt_buf_reset(&comp);
		alt_buf_reset(&back);
		if (alt_gzip_data(&comp, data.p, data.n, i % 10) < 0 || alt_gunzip_data(&back, comp.p, comp.n, 0) < 0 ||
			back.n != data.n || memcmp(back.p, data.p, data.n))
			FAILED("thread %d round %d", (int)(intptr_t)arg, i);
	}
	alt_buf_free(&data);
	alt_buf_free(&comp);
	alt_buf_free(&back);
	return 0;
}

static void threads(void) {
	pthread_t th[6];
	for (intptr_t i = 0; i < 6; i++)
		pthread_create(&th[i], 0, worker, (void *)i);
	for (int i = 0; i < 6; i++)
		pthread_join(th[i], 0);
}

int main(void) {
	RUN(roundtrips);
	RUN(dictionaries);
	RUN(gzip_members);
	RUN(threads);
	return check_done();
}
