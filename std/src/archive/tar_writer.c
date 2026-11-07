/* The tar writer, following Go's archive/tar writer.go and
   Header.allowedFormats in common.go (BSD-3-Clause, see LICENSE-Go):
   the same format choice and the same bytes. */
#include "tar_internal.h"
#include <alt/path.h>
#include <stdio.h>

struct alt_tar_writer {
	alt_writer dst;
	int64_t pad;      /* padding owed after the current entry */
	int64_t nb;       /* bytes of the current entry still to write */
	uint8_t blk[TAR_BLOCK];
	int err;          /* sticky (-ALT_ECLOSED after close) */
	alt_buf err_msg;  /* the text of err */
	alt_buf why;      /* the text of the error last returned */
	alt_arena a;      /* per header */
	alt_buf tmp;
};

int alt_tar_writer_new(alt_tar_writer **out, alt_writer dst) {
	alt_tar_writer *w = calloc(1, sizeof *w);
	*out = w;
	if (!w)
		return -ENOMEM;
	w->dst = dst;
	alt_arena_init(&w->a, 0);
	return 0;
}

void alt_tar_writer_free(alt_tar_writer *w) {
	if (!w)
		return;
	alt_buf_free(&w->err_msg);
	alt_buf_free(&w->why);
	alt_buf_free(&w->tmp);
	alt_arena_free(&w->a);
	free(w);
}

const char *alt_tar_writer_error(const alt_tar_writer *w) { return alt_buf_cstr(&w->why); }

static const char *code_text(int code) {
	switch (code) {
	case -ALT_ECLOSED: return "archive/tar: write after close";
	case -ALT_EUNEXPECTED_EOF: return "unexpected EOF";
	}
	return alt_strerror(code);
}

/* returns the sticky error, or sets it */
static int sticky(alt_tar_writer *w) {
	alt_buf_reset(&w->why);
	alt_buf_append_str(&w->why, alt_buf_str(&w->err_msg));
	return w->err;
}

static int set_sticky(alt_tar_writer *w, int code, const char *msg) {
	w->err = code;
	alt_buf_reset(&w->err_msg);
	if (code)
		alt_buf_append_cstr(&w->err_msg, msg ? msg : code_text(code));
	return sticky(w);
}

static int fail(alt_tar_writer *w, int code, const char *msg) {
	alt_buf_reset(&w->why);
	alt_buf_append_cstr(&w->why, msg ? msg : code_text(code));
	return code;
}

static int write_dst(alt_tar_writer *w, const void *p, size_t n) {
	ssize_t k = n ? alt_write_all(w->dst, p, n) : 0;
	return k < 0 ? (int)k : 0;
}

static const uint8_t zero_block[TAR_BLOCK];

int alt_tar_writer_flush(alt_tar_writer *w) {
	if (w->err)
		return sticky(w);
	if (w->nb > 0) {
		char msg[64];
		snprintf(msg, sizeof msg, "archive/tar: missed writing %lld bytes", (long long)w->nb);
		return fail(w, -EINVAL, msg);
	}
	int rc = write_dst(w, zero_block, (size_t)w->pad);
	if (rc < 0)
		return set_sticky(w, rc, 0);
	w->pad = 0;
	alt_buf_reset(&w->why);
	return 0;
}

static bool header_only(int flag) { return flag >= ALT_TAR_LINK && flag <= ALT_TAR_FIFO; }

/* ---- allowedFormats ---- */

typedef struct {
	alt_tar_writer *w;
	const alt_tar_header *h;
	alt_tar_record *user; /* h's records sorted, the last of a key kept */
	size_t nuser;
	alt_tar_record *pax;  /* the records to write (unsorted, unique keys) */
	size_t npax, cap;
	int format;
	bool prefer_pax;
	alt_buf no_ustar, no_pax, no_gnu, only_pax, only_gnu;
	int err;
} check;

static alt_str dup_str(check *c, alt_str s) {
	alt_str d = alt_arena_str(&c->w->a, s);
	if (!d.p)
		c->err = -ENOMEM;
	return d;
}

static void pax_set(check *c, alt_str k, alt_str v) {
	for (size_t i = 0; i < c->npax; i++)
		if (alt_str_eq(c->pax[i].key, k)) {
			c->pax[i].value = v;
			return;
		}
	if (c->npax == c->cap) {
		size_t cap = c->cap ? c->cap * 2 : 16;
		alt_tar_record *p = alt_arena_realloc(&c->w->a, c->pax, c->cap * sizeof *p, cap * sizeof *p);
		if (!p) {
			c->err = -ENOMEM;
			return;
		}
		c->pax = p;
		c->cap = cap;
	}
	c->pax[c->npax++] = (alt_tar_record){k, v};
}

static bool pax_has(const check *c, alt_str k) {
	for (size_t i = 0; i < c->npax; i++)
		if (alt_str_eq(c->pax[i].key, k))
			return true;
	return false;
}

static const alt_tar_record *user_rec(const check *c, const char *key) {
	return alt__tar_find_record(c->user, c->nuser, alt_str_cstr(key));
}

static void why(alt_buf *b, const char *fmt, const char *name) {
	alt_buf_reset(b);
	alt_buf_printf(b, fmt, name);
}

static void verify_string(check *c, alt_str s, size_t size, const char *name, const char *key) {
	bool too_long = s.n > size;
	bool long_gnu = !strcmp(key, "path") || !strcmp(key, "linkpath");
	if (alt__tar_has_nul(s) || (too_long && !long_gnu)) {
		why(&c->no_gnu, "GNU cannot encode %s=", name);
		alt_quote(&c->no_gnu, s);
		c->format &= ~ALT_TAR_FORMAT_GNU;
	}
	if (!alt__tar_is_ascii(s) || too_long) {
		bool can_split = !strcmp(key, "path");
		bool split_ok = false;
		if (can_split && s.n > TAR_NAME_SIZE && alt__tar_is_ascii(s)) {
			/* splitUSTARPath */
			size_t length = s.n;
			if (length > TAR_PREFIX_SIZE + 1)
				length = TAR_PREFIX_SIZE + 1;
			else if (s.p[length - 1] == '/')
				length--;
			ssize_t i = alt_str_last_index_byte(alt_str_make(s.p, length), '/');
			size_t nlen = s.n - (size_t)i - 1;
			split_ok = !(i <= 0 || nlen > TAR_NAME_SIZE || nlen == 0 || (size_t)i > TAR_PREFIX_SIZE);
		}
		if (!split_ok) {
			why(&c->no_ustar, "USTAR cannot encode %s=", name);
			alt_quote(&c->no_ustar, s);
			c->format &= ~ALT_TAR_FORMAT_USTAR;
		}
		pax_set(c, alt_str_cstr(key), s);
	}
	const alt_tar_record *r = user_rec(c, key);
	if (r && alt_str_eq(r->value, s))
		pax_set(c, alt_str_cstr(key), r->value);
}

static alt_str fmt_int(check *c, int64_t n) {
	char d[24];
	snprintf(d, sizeof d, "%lld", (long long)n);
	return dup_str(c, alt_str_cstr(d));
}

static void verify_numeric(check *c, int64_t n, size_t size, const char *name, const char *key) {
	if (!alt__tar_fits_base256(size, n)) {
		why(&c->no_gnu, "GNU cannot encode %s=", name);
		alt_buf_append_int(&c->no_gnu, n);
		c->format &= ~ALT_TAR_FORMAT_GNU;
	}
	if (!alt__tar_fits_octal(size, n)) {
		why(&c->no_ustar, "USTAR cannot encode %s=", name);
		alt_buf_append_int(&c->no_ustar, n);
		c->format &= ~ALT_TAR_FORMAT_USTAR;
		if (!*key) {
			why(&c->no_pax, "PAX cannot encode %s=", name);
			alt_buf_append_int(&c->no_pax, n);
			c->format &= ~ALT_TAR_FORMAT_PAX;
		} else
			pax_set(c, alt_str_cstr(key), fmt_int(c, n));
	}
	/* (Go looks the key up even when it is "", for Mode and the devices) */
	const alt_tar_record *r = user_rec(c, key);
	if (r) {
		char d[24];
		snprintf(d, sizeof d, "%lld", (long long)n);
		if (alt_str_eq_cstr(r->value, d))
			pax_set(c, alt_str_cstr(key), r->value);
	}
}

static alt_str fmt_pax_time(check *c, alt_tar_time t) {
	alt_buf b = ALT_BUF_INIT;
	alt__tar_format_pax_time(&b, t);
	alt_str s = dup_str(c, alt_buf_str(&b));
	alt_buf_free(&b);
	return s;
}

static void verify_time(check *c, alt_tar_time t, size_t size, const char *name, const char *key) {
	if (alt_tar_time_is_zero(t))
		return;
	if (!alt__tar_fits_base256(size, t.sec)) {
		why(&c->no_gnu, "GNU cannot encode %s=", name);
		alt__tar_format_go_time(&c->no_gnu, t);
		c->format &= ~ALT_TAR_FORMAT_GNU;
	}
	bool is_mtime = !strcmp(key, "mtime");
	bool fits = alt__tar_fits_octal(size, t.sec);
	if (!is_mtime || !fits) {
		why(&c->no_ustar, "USTAR cannot encode %s=", name);
		alt__tar_format_go_time(&c->no_ustar, t);
		c->format &= ~ALT_TAR_FORMAT_USTAR;
	}
	if (!is_mtime || !fits || t.nsec != 0) {
		c->prefer_pax = true; /* USTAR may truncate sub-second measurements */
		pax_set(c, alt_str_cstr(key), fmt_pax_time(c, t));
	}
	const alt_tar_record *r = user_rec(c, key);
	if (r) {
		alt_str f = fmt_pax_time(c, t);
		if (alt_str_eq(r->value, f))
			pax_set(c, alt_str_cstr(key), r->value);
	}
}

static bool basic_key(alt_str k) {
	static const char *keys[] = {"path", "linkpath", "size", "uid", "gid", "uname", "gname", "mtime", "atime", "ctime"};
	for (size_t i = 0; i < sizeof keys / sizeof *keys; i++)
		if (alt_str_eq_cstr(k, keys[i]))
			return true;
	return false;
}

/* headerError: "archive/tar: cannot encode header: a; and b" */
static void header_error(alt_buf *b, const alt_buf *parts, size_t n) {
	alt_buf_reset(b);
	alt_buf_append_cstr(b, "archive/tar: cannot encode header");
	bool first = true;
	for (size_t i = 0; i < n; i++) {
		if (!parts[i].n)
			continue;
		alt_buf_append_cstr(b, first ? ": " : "; and ");
		alt_buf_append_str(b, alt_buf_str(&parts[i]));
		first = false;
	}
}

static int header_error1(alt_tar_writer *w, const char *msg) {
	alt_buf b = ALT_BUF_INIT, m = ALT_BUF_INIT;
	alt_buf_append_cstr(&m, msg);
	header_error(&b, &m, 1);
	int rc = fail(w, -ALT_ETAR_FIELD, alt_buf_cstr(&b));
	alt_buf_free(&b);
	alt_buf_free(&m);
	return rc;
}

static void check_free(check *c) {
	alt_buf_free(&c->no_ustar);
	alt_buf_free(&c->no_pax);
	alt_buf_free(&c->no_gnu);
	alt_buf_free(&c->only_pax);
	alt_buf_free(&c->only_gnu);
}

static int cmp_rec(const void *a, const void *b) {
	return alt_str_cmp(((const alt_tar_record *)a)->key, ((const alt_tar_record *)b)->key);
}

/* the formats that can encode h, and the PAX records it needs (sorted) */
static int allowed_formats(alt_tar_writer *w, const alt_tar_header *h, check *c) {
	memset(c, 0, sizeof *c);
	c->w = w;
	c->h = h;
	c->format = TAR_FORMAT_ANY;
	if (h->npax) {
		c->user = alt_arena_alloc(&w->a, h->npax * sizeof *c->user);
		if (!c->user)
			return -ENOMEM;
		memcpy(c->user, h->pax, h->npax * sizeof *c->user);
		c->nuser = alt__tar_sort_records(c->user, h->npax);
	}
	verify_string(c, h->name, 100, "Name", "path");
	verify_string(c, h->linkname, 100, "Linkname", "linkpath");
	verify_string(c, h->uname, 32, "Uname", "uname");
	verify_string(c, h->gname, 32, "Gname", "gname");
	verify_numeric(c, h->mode, 8, "Mode", "");
	verify_numeric(c, h->uid, 8, "Uid", "uid");
	verify_numeric(c, h->gid, 8, "Gid", "gid");
	verify_numeric(c, h->size, 12, "Size", "size");
	verify_numeric(c, h->devmajor, 8, "Devmajor", "");
	verify_numeric(c, h->devminor, 8, "Devminor", "");
	verify_time(c, h->mtime, 12, "ModTime", "mtime");
	verify_time(c, h->atime, 12, "AccessTime", "atime");
	verify_time(c, h->ctime, 12, "ChangeTime", "ctime");
	if (c->err)
		return c->err;

	switch (h->typeflag) {
	case ALT_TAR_REG:
	case ALT_TAR_CHAR:
	case ALT_TAR_BLOCK:
	case ALT_TAR_FIFO:
	case ALT_TAR_GNU_SPARSE:
		if (alt_str_has_suffix(h->name, ALT_STR("/")))
			return header_error1(w, "filename may not have trailing slash");
		break;
	case ALT_TAR_XHEADER:
	case ALT_TAR_GNU_LONGNAME:
	case ALT_TAR_GNU_LONGLINK:
		return header_error1(w, "cannot manually encode TypeXHeader, TypeGNULongName, or TypeGNULongLink headers");
	case ALT_TAR_XGLOBAL:
		if (h->linkname.n || h->size || h->mode || h->uid || h->gid || h->uname.n || h->gname.n || h->mtime.set ||
			h->atime.set || h->ctime.set || h->devmajor || h->devminor)
			return header_error1(w, "only PAXRecords should be set for TypeXGlobalHeader");
		alt_buf_reset(&c->only_pax);
		alt_buf_append_cstr(&c->only_pax, "only PAX supports TypeXGlobalHeader");
		c->format &= ALT_TAR_FORMAT_PAX;
		break;
	}
	if (!header_only(h->typeflag) && h->size < 0)
		return header_error1(w, "negative size on header-only type");

	if (h->npax) {
		size_t nbasic = c->npax;
		for (size_t i = 0; i < c->nuser; i++) {
			alt_str k = c->user[i].key;
			bool exists = false;
			for (size_t j = 0; j < nbasic; j++)
				if (alt_str_eq(c->pax[j].key, k))
					exists = true;
			if (exists)
				continue;
			if (h->typeflag == ALT_TAR_XGLOBAL || (!basic_key(k) && !alt_str_has_prefix(k, ALT_STR("GNU.sparse."))))
				pax_set(c, k, c->user[i].value);
		}
		alt_buf_reset(&c->only_pax);
		alt_buf_append_cstr(&c->only_pax, "only PAX supports PAXRecords");
		c->format &= ALT_TAR_FORMAT_PAX;
	}
	if (c->err)
		return c->err;
	if (c->npax > 1)
		qsort(c->pax, c->npax, sizeof *c->pax, cmp_rec);
	for (size_t i = 0; i < c->npax; i++)
		if (!alt__tar_valid_pax_record(c->pax[i].key, c->pax[i].value)) {
			/* (Go reports one of the invalid records at random) */
			alt_buf b = ALT_BUF_INIT, q = ALT_BUF_INIT;
			alt_buf_append_str(&b, c->pax[i].key);
			alt_buf_append_cstr(&b, " = ");
			alt_buf_append_str(&b, c->pax[i].value);
			alt_buf_append_cstr(&q, "invalid PAX record: ");
			alt_quote(&q, alt_buf_str(&b));
			int rc = header_error1(w, alt_buf_cstr(&q));
			alt_buf_free(&b);
			alt_buf_free(&q);
			return rc;
		}

	if (h->format != ALT_TAR_FORMAT_UNKNOWN) {
		int want = h->format;
		if ((want & ALT_TAR_FORMAT_PAX) && !c->prefer_pax)
			want |= ALT_TAR_FORMAT_USTAR; /* PAX implies USTAR allowed too */
		c->format &= want;
	}
	if (c->format == ALT_TAR_FORMAT_UNKNOWN) {
		alt_buf parts[5], msg = ALT_BUF_INIT;
		memset(parts, 0, sizeof parts);
		size_t n = 0;
#define PART(s) alt_buf_append_str(&parts[n++], (s))
		switch (h->format) {
		case ALT_TAR_FORMAT_USTAR:
			PART(ALT_STR("Format specifies USTAR"));
			PART(alt_buf_str(&c->no_ustar));
			PART(alt_buf_str(&c->only_pax));
			PART(alt_buf_str(&c->only_gnu));
			break;
		case ALT_TAR_FORMAT_PAX:
			PART(ALT_STR("Format specifies PAX"));
			PART(alt_buf_str(&c->no_pax));
			PART(alt_buf_str(&c->only_gnu));
			break;
		case ALT_TAR_FORMAT_GNU:
			PART(ALT_STR("Format specifies GNU"));
			PART(alt_buf_str(&c->no_gnu));
			PART(alt_buf_str(&c->only_pax));
			break;
		default:
			PART(alt_buf_str(&c->no_ustar));
			PART(alt_buf_str(&c->no_pax));
			PART(alt_buf_str(&c->no_gnu));
			PART(alt_buf_str(&c->only_pax));
			PART(alt_buf_str(&c->only_gnu));
		}
#undef PART
		header_error(&msg, parts, n);
		int rc = fail(w, -ALT_ETAR_FIELD, alt_buf_cstr(&msg));
		for (size_t i = 0; i < n; i++)
			alt_buf_free(&parts[i]);
		alt_buf_free(&msg);
		return rc;
	}
	return 0;
}

/* ---- blocks ---- */

static int write_raw_header(alt_tar_writer *w, int64_t size, int flag) {
	int rc = alt_tar_writer_flush(w);
	if (rc < 0)
		return rc;
	if ((rc = write_dst(w, w->blk, TAR_BLOCK)) < 0)
		return set_sticky(w, rc, 0);
	if (header_only(flag))
		size = 0;
	w->nb = size;
	w->pad = tar_block_padding(size);
	return 0;
}

typedef void (*str_fmt)(alt_tar_writer *w, uint8_t *b, size_t n, alt_str s, int *err);
typedef void (*num_fmt)(uint8_t *b, size_t n, int64_t x, int *err);

static void plain_string(alt_tar_writer *w, uint8_t *b, size_t n, alt_str s, int *err) {
	(void)w;
	alt__tar_format_string(b, n, s, err);
}

static void ascii_string(alt_tar_writer *w, uint8_t *b, size_t n, alt_str s, int *err) {
	alt__tar_format_string(b, n, alt__tar_to_ascii(s, &w->tmp), err);
}

static void template_v7_plus(alt_tar_writer *w, const alt_tar_header *h, str_fmt fs, num_fmt fn, int *err) {
	uint8_t *b = w->blk;
	memset(b, 0, TAR_BLOCK);
	int64_t mtime = alt_tar_time_is_zero(h->mtime) ? 0 : h->mtime.sec;
	b[156] = (uint8_t)h->typeflag;
	fs(w, FLD(b, F_NAME), h->name, err);
	fs(w, FLD(b, F_LINK), h->linkname, err);
	fn(FLD(b, F_MODE), h->mode, err);
	fn(FLD(b, F_UID), h->uid, err);
	fn(FLD(b, F_GID), h->gid, err);
	fn(FLD(b, F_SIZE), h->size, err);
	fn(FLD(b, F_MTIME), mtime, err);
	fs(w, FLD(b, F_UNAME), h->uname, err);
	fs(w, FLD(b, F_GNAME), h->gname, err);
	fn(FLD(b, F_DEVMAJOR), h->devmajor, err);
	fn(FLD(b, F_DEVMINOR), h->devminor, err);
}

static ssize_t write_data(alt_tar_writer *w, const void *p, size_t n);

/* a header of its own and its data (PAX records, GNU long names) */
static int write_raw_file(alt_tar_writer *w, alt_str name, alt_str data, int flag, int format) {
	uint8_t *b = w->blk;
	memset(b, 0, TAR_BLOCK);
	alt_buf nb = ALT_BUF_INIT;
	name = alt__tar_to_ascii(name, &nb);
	if (name.n > TAR_NAME_SIZE)
		name.n = TAR_NAME_SIZE;
	while (name.n && name.p[name.n - 1] == '/')
		name.n--;
	int err = 0;
	b[156] = (uint8_t)flag;
	alt__tar_format_string(FLD(b, F_NAME), name, &err);
	alt_buf_free(&nb);
	alt__tar_format_octal(FLD(b, F_MODE), 0, &err);
	alt__tar_format_octal(FLD(b, F_UID), 0, &err);
	alt__tar_format_octal(FLD(b, F_GID), 0, &err);
	alt__tar_format_octal(FLD(b, F_SIZE), (int64_t)data.n, &err);
	alt__tar_format_octal(FLD(b, F_MTIME), 0, &err);
	alt__tar_set_format(b, format);
	if (err)
		return fail(w, err, "archive/tar: header field too long");
	int rc = write_raw_header(w, (int64_t)data.n, flag);
	if (rc < 0)
		return rc;
	ssize_t k = write_data(w, data.p, data.n);
	return k < 0 ? (int)k : 0;
}

static int write_ustar(alt_tar_writer *w, alt_tar_header *h) {
	alt_str prefix = ALT_STR("");
	alt_str name = h->name;
	if (name.n > TAR_NAME_SIZE && alt__tar_is_ascii(name)) {
		size_t length = name.n;
		if (length > TAR_PREFIX_SIZE + 1)
			length = TAR_PREFIX_SIZE + 1;
		else if (name.p[length - 1] == '/')
			length--;
		ssize_t i = alt_str_last_index_byte(alt_str_make(name.p, length), '/');
		size_t nlen = name.n - (size_t)i - 1;
		if (!(i <= 0 || nlen > TAR_NAME_SIZE || nlen == 0 || (size_t)i > TAR_PREFIX_SIZE)) {
			prefix = alt_str_make(name.p, (size_t)i);
			h->name = alt_str_from(name, (size_t)i + 1);
		}
	}
	int err = 0;
	template_v7_plus(w, h, plain_string, alt__tar_format_octal, &err);
	alt__tar_format_string(FLD(w->blk, F_PREFIX), prefix, &err);
	alt__tar_set_format(w->blk, ALT_TAR_FORMAT_USTAR);
	if (err)
		return fail(w, err, "archive/tar: header field too long");
	return write_raw_header(w, h->size, h->typeflag);
}

static int write_pax(alt_tar_writer *w, alt_tar_header *h, const check *c) {
	alt_str real_name = h->name;
	bool global = h->typeflag == ALT_TAR_XGLOBAL;
	if (c->npax || global) {
		alt_buf data = ALT_BUF_INIT, name = ALT_BUF_INIT;
		for (size_t i = 0; i < c->npax; i++)
			alt__tar_format_pax_record(&data, c->pax[i].key, c->pax[i].value);
		int flag;
		if (global) {
			alt_buf_append_str(&name, real_name.n ? real_name : ALT_STR("GlobalHead.0.0"));
			flag = ALT_TAR_XGLOBAL;
		} else {
			alt_str dir, file;
			alt_path_split(real_name, &dir, &file);
			alt_str parts[3] = {dir, ALT_STR("PaxHeaders.0"), file};
			alt_path_join(&name, parts, 3);
			flag = ALT_TAR_XHEADER;
		}
		int rc;
		if (data.err || name.err)
			rc = -ENOMEM;
		else if (data.n > TAR_MAX_SPECIAL)
			rc = fail(w, -ALT_ETAR_FIELD, "archive/tar: header field too long");
		else
			rc = write_raw_file(w, alt_buf_str(&name), alt_buf_str(&data), flag, ALT_TAR_FORMAT_PAX);
		alt_buf_free(&data);
		alt_buf_free(&name);
		if (rc < 0 || global)
			return rc;
	}
	int err = 0; /* the fields that do not fit are expected to be wrong */
	template_v7_plus(w, h, ascii_string, alt__tar_format_octal, &err);
	alt__tar_set_format(w->blk, ALT_TAR_FORMAT_PAX);
	return write_raw_header(w, h->size, h->typeflag);
}

static int write_gnu(alt_tar_writer *w, alt_tar_header *h) {
	static const alt_str long_name = {"././@LongLink", 13};
	if (h->name.n > TAR_NAME_SIZE) {
		alt_buf d = ALT_BUF_INIT;
		alt_buf_append_str(&d, h->name);
		alt_buf_append_byte(&d, 0);
		int rc = d.err ? -ENOMEM : write_raw_file(w, long_name, alt_buf_str(&d), ALT_TAR_GNU_LONGNAME, ALT_TAR_FORMAT_GNU);
		alt_buf_free(&d);
		if (rc < 0)
			return rc;
	}
	if (h->linkname.n > TAR_NAME_SIZE) {
		alt_buf d = ALT_BUF_INIT;
		alt_buf_append_str(&d, h->linkname);
		alt_buf_append_byte(&d, 0);
		int rc = d.err ? -ENOMEM : write_raw_file(w, long_name, alt_buf_str(&d), ALT_TAR_GNU_LONGLINK, ALT_TAR_FORMAT_GNU);
		alt_buf_free(&d);
		if (rc < 0)
			return rc;
	}
	int err = 0;
	template_v7_plus(w, h, plain_string, alt__tar_format_numeric, &err);
	if (!alt_tar_time_is_zero(h->atime))
		alt__tar_format_numeric(FLD(w->blk, F_GNU_ATIME), h->atime.sec, &err);
	if (!alt_tar_time_is_zero(h->ctime))
		alt__tar_format_numeric(FLD(w->blk, F_GNU_CTIME), h->ctime.sec, &err);
	alt__tar_set_format(w->blk, ALT_TAR_FORMAT_GNU);
	return write_raw_header(w, h->size, h->typeflag);
}

int alt_tar_write_header(alt_tar_writer *w, const alt_tar_header *hdr) {
	int rc = alt_tar_writer_flush(w);
	if (rc < 0)
		return rc;
	alt_arena_reset(&w->a);
	alt_tar_header h = *hdr;
	if (h.typeflag == ALT_TAR_REGA)
		h.typeflag = alt_str_has_suffix(h.name, ALT_STR("/")) ? ALT_TAR_DIR : ALT_TAR_REG;
	if (h.format == ALT_TAR_FORMAT_UNKNOWN) {
		h.mtime = alt__tar_round_second(h.mtime);
		h.atime = h.ctime = (alt_tar_time){0, 0, false};
	}
	check c;
	rc = allowed_formats(w, &h, &c);
	if (rc == 0) {
		if (c.format & ALT_TAR_FORMAT_USTAR)
			rc = write_ustar(w, &h);
		else if (c.format & ALT_TAR_FORMAT_PAX)
			rc = write_pax(w, &h, &c);
		else
			rc = write_gnu(w, &h);
		/* Go keeps the error of writing a header for good */
		if (rc < 0 && !w->err) {
			alt_buf msg = ALT_BUF_INIT;
			alt_buf_append_cstr(&msg, alt_buf_cstr(&w->why));
			set_sticky(w, rc, alt_buf_cstr(&msg));
			alt_buf_free(&msg);
		}
	}
	check_free(&c);
	if (rc == 0)
		alt_buf_reset(&w->why);
	return rc;
}

static ssize_t write_data(alt_tar_writer *w, const void *p, size_t n) {
	if (w->err)
		return sticky(w);
	if (n == 0)
		return 0;
	if (w->nb == 0)
		return fail(w, -ALT_ETAR_WRITE_TOO_LONG, "archive/tar: write too long");
	size_t m = (uint64_t)n > (uint64_t)w->nb ? (size_t)w->nb : n;
	int rc = write_dst(w, p, m);
	if (rc < 0)
		return set_sticky(w, rc, 0);
	w->nb -= (int64_t)m;
	alt_buf_reset(&w->why);
	return (ssize_t)m;
}

ssize_t alt_tar_write(alt_tar_writer *w, const void *p, size_t n) { return write_data(w, p, n); }

static ssize_t writer_write(void *obj, const void *p, size_t n) { return alt_tar_write(obj, p, n); }

alt_writer alt_tar_writer_writer(alt_tar_writer *w) { return (alt_writer){writer_write, w}; }

int alt_tar_writer_close(alt_tar_writer *w) {
	if (w->err == -ALT_ECLOSED) {
		alt_buf_reset(&w->why);
		return 0;
	}
	if (w->err)
		return sticky(w);
	int rc = alt_tar_writer_flush(w);
	for (int i = 0; i < 2 && rc == 0; i++)
		if ((rc = write_dst(w, zero_block, TAR_BLOCK)) < 0)
			fail(w, rc, 0);
	/* the error (an I/O error or data missing) is reported once */
	alt_buf msg = ALT_BUF_INIT;
	alt_buf_append_str(&msg, alt_buf_str(&w->why));
	set_sticky(w, -ALT_ECLOSED, 0);
	alt_buf_reset(&w->why);
	if (rc < 0)
		alt_buf_append_str(&w->why, alt_buf_str(&msg));
	alt_buf_free(&msg);
	return rc;
}
