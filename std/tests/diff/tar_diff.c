/* W entry... -> the archive written: "<errors>|<hex>". An entry is
     flag,name,link,size,mode,uid,gid,uname,gname,mtime,atime,ctime,devmajor,devminor,format,data,pax
   (strings, data and the PAX keys and values in hex; times "-" for none
   or sec:nsec; pax k:v;k:v); errors are h<i>:<text>; for WriteHeader,
   d<i>:<text>; for the data, c:<text>; for Close.
   R hex -> every entry of the archive: its fields, the length and FNV-1a
   hash of its data (at most 1 MiB is read) and the error of reading it,
   then E:<error of Next>. The name of a global header with a bad
   record is "?". */
#include "common.h"
#include <alt/archive/tar.h>
#include <alt/hash/fnv.h>
#include <inttypes.h>

static char *split(char **s, char sep) {
	char *p = *s, *q = strchr(p, sep);
	if (q) {
		*q = 0;
		*s = q + 1;
	} else
		*s = p + strlen(p);
	return p;
}

static alt_str hexs(char *s) { return alt_str_make(s, diff_unhex(s)); }

static alt_tar_time tm(char *s) {
	if (!strcmp(s, "-"))
		return (alt_tar_time){0, 0, false};
	long long sec, nsec;
	sscanf(s, "%lld:%lld", &sec, &nsec);
	return alt_tar_unix(sec, nsec);
}

static void write_archive(char **f, int n) {
	alt_buf out = ALT_BUF_INIT, errs = ALT_BUF_INIT;
	alt_tar_writer *w;
	alt_tar_writer_new(&w, alt_buf_writer(&out));
	for (int i = 1; i < n; i++) {
		char *s = f[i];
		alt_tar_header h = {0};
		alt_tar_record recs[64];
		h.typeflag = atoi(split(&s, ','));
		h.name = hexs(split(&s, ','));
		h.linkname = hexs(split(&s, ','));
		h.size = strtoll(split(&s, ','), 0, 10);
		h.mode = strtoll(split(&s, ','), 0, 10);
		h.uid = strtoll(split(&s, ','), 0, 10);
		h.gid = strtoll(split(&s, ','), 0, 10);
		h.uname = hexs(split(&s, ','));
		h.gname = hexs(split(&s, ','));
		h.mtime = tm(split(&s, ','));
		h.atime = tm(split(&s, ','));
		h.ctime = tm(split(&s, ','));
		h.devmajor = strtoll(split(&s, ','), 0, 10);
		h.devminor = strtoll(split(&s, ','), 0, 10);
		h.format = atoi(split(&s, ','));
		alt_str data = hexs(split(&s, ','));
		char *pax = split(&s, ',');
		while (*pax && h.npax < 64) {
			char *kv = split(&pax, ';');
			char *k = split(&kv, ':');
			recs[h.npax].key = hexs(k);
			recs[h.npax].value = hexs(kv);
			h.npax++;
		}
		h.pax = recs;
		if (alt_tar_write_header(w, &h) < 0)
			alt_buf_printf(&errs, "h%d:%s;", i - 1, alt_tar_writer_error(w));
		size_t off = 0;
		while (off < data.n) {
			ssize_t k = alt_tar_write(w, data.p + off, data.n - off);
			if (k < 0) {
				alt_buf_printf(&errs, "d%d:%s;", i - 1, alt_tar_writer_error(w));
				break;
			}
			off += (size_t)k;
		}
	}
	if (alt_tar_writer_close(w) < 0)
		alt_buf_printf(&errs, "c:%s;", alt_tar_writer_error(w));
	printf("%s|", alt_buf_cstr(&errs));
	diff_puthex(out.p, out.n);
	alt_tar_writer_free(w);
	alt_buf_free(&out);
	alt_buf_free(&errs);
}

static void puttime(alt_tar_time t) {
	if (alt_tar_time_is_zero(t))
		printf(" z");
	else
		printf(" %" PRId64 ".%d", t.sec, (int)t.nsec);
}

static void putstr(alt_str s) {
	putchar(' ');
	diff_puthex(s.p, s.n);
}

static void read_archive(char *hex) {
	alt_str a = hexs(hex);
	alt_str_reader sr;
	alt_tar_reader *r;
	alt_tar_reader_new(&r, alt_str_reader_init(&sr, a));
	static char buf[4096];
	for (int count = 0; count < 2000; count++) {
		alt_tar_header *h;
		int rc = alt_tar_next(r, &h);
		if (rc < 0) {
			printf("E:%s", alt_tar_reader_error(r));
			break;
		}
		printf("{%d", h->typeflag);
		if (h->typeflag == 'g' && !h->npax)
			printf(" ?"); /* Go's name depends on its map order here */
		else
			putstr(h->name);
		putstr(h->linkname);
		printf(" %" PRId64 " %" PRId64 " %" PRId64 " %" PRId64, h->size, h->mode, h->uid, h->gid);
		putstr(h->uname);
		putstr(h->gname);
		puttime(h->mtime);
		puttime(h->atime);
		puttime(h->ctime);
		printf(" %" PRId64 " %" PRId64 " %d [", h->devmajor, h->devminor, h->format);
		for (size_t i = 0; i < h->npax; i++) {
			diff_puthex(h->pax[i].key.p, h->pax[i].key.n);
			putchar('=');
			diff_puthex(h->pax[i].value.p, h->pax[i].value.n);
			putchar(';');
		}
		printf("]}");
		uint64_t hash = ALT_FNV64_OFFSET;
		size_t total = 0, limit = 1 << 20;
		const char *err = "";
		while (total < limit) {
			size_t want = limit - total < sizeof buf ? limit - total : sizeof buf;
			ssize_t k = alt_tar_read(r, buf, want);
			if (k < 0) {
				err = alt_tar_reader_error(r);
				break;
			}
			if (k == 0)
				break;
			hash = alt_fnv1a_64(hash, buf, (size_t)k);
			total += (size_t)k;
		}
		printf("d%zu:%016" PRIx64 ":%s;", total, hash, err);
	}
	alt_tar_reader_free(r);
}

int main(void) {
	char *line, *f[256];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 256);
		if (!strcmp(f[0], "W"))
			write_archive(f, n);
		else if (!strcmp(f[0], "R"))
			read_archive(n > 1 ? f[1] : (char[]){0});
		printf("\n");
		fflush(stdout);
	}
	return 0;
}
