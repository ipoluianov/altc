/* Serving files, after Go's net/http fs.go (BSD-3-Clause, see
   LICENSE-Go): FileServer, ServeFile, conditional requests, ranges */
#define _GNU_SOURCE
#include "http_internal.h"
#include <alt/errors.h>
#include <alt/mime.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void local_redirect(alt_http_rw *w, alt_http_request *r, const char *target) {
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, target);
	if (r->url.raw_query.n) {
		alt_buf_append_byte(&b, '?');
		alt_buf_append_str(&b, r->url.raw_query);
	}
	alt_http_header_set(alt_http_rw_header(w), ALT_STR("Location"), alt_buf_str(&b));
	alt_http_rw_write_header(w, 301);
	alt_buf_free(&b);
}

static void http_error_for(alt_http_rw *w, int err) {
	if (err == ENOENT || err == ENOTDIR)
		alt_http_error(w, "404 page not found", 404);
	else if (err == EACCES || err == EPERM)
		alt_http_error(w, "403 Forbidden", 403);
	else
		alt_http_error(w, "500 Internal Server Error", 500);
}

static void html_escape(alt_buf *b, const char *s) {
	for (; *s; s++) {
		switch (*s) {
		case '&': alt_buf_append_cstr(b, "&amp;"); break;
		case '<': alt_buf_append_cstr(b, "&lt;"); break;
		case '>': alt_buf_append_cstr(b, "&gt;"); break;
		case '"': alt_buf_append_cstr(b, "&#34;"); break;
		case '\'': alt_buf_append_cstr(b, "&#39;"); break;
		default: alt_buf_append_byte(b, *s);
		}
	}
}

static int cmp_names(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

static void dir_list(alt_http_rw *w, int dirfd) {
	DIR *d = fdopendir(dirfd);
	if (!d) {
		close(dirfd);
		alt_http_error(w, "Error reading directory", 500);
		return;
	}
	char **names = 0;
	size_t n = 0, cap = 0;
	struct dirent *e;
	while ((e = readdir(d))) {
		if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
			continue;
		if (n == cap) {
			cap = cap ? cap * 2 : 64;
			char **nn = realloc(names, cap * sizeof *nn);
			if (!nn)
				break;
			names = nn;
		}
		bool dir = e->d_type == DT_DIR;
		if (e->d_type == DT_UNKNOWN || e->d_type == DT_LNK) {
			struct stat st;
			dir = fstatat(dirfd, e->d_name, &st, 0) == 0 && S_ISDIR(st.st_mode);
		}
		size_t l = strlen(e->d_name);
		char *s = malloc(l + 2);
		if (!s)
			break;
		memcpy(s, e->d_name, l);
		s[l] = dir ? '/' : 0;
		s[l + 1] = 0;
		names[n++] = s;
	}
	closedir(d);
	qsort(names, n, sizeof *names, cmp_names);
	alt_http_header_set_cstr(alt_http_rw_header(w), "Content-Type", "text/html; charset=utf-8");
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, "<!doctype html>\n<meta name=\"viewport\" content=\"width=device-width\">\n<pre>\n");
	for (size_t i = 0; i < n; i++) {
		alt_url u = {0};
		u.path = alt_str_cstr(names[i]);
		alt_buf href = ALT_BUF_INIT;
		alt_url_string(&u, &href);
		alt_buf_append_cstr(&b, "<a href=\"");
		html_escape(&b, alt_buf_cstr(&href));
		alt_buf_append_cstr(&b, "\">");
		html_escape(&b, names[i]);
		alt_buf_append_cstr(&b, "</a>\n");
		alt_buf_free(&href);
		free(names[i]);
	}
	free(names);
	alt_buf_append_cstr(&b, "</pre>\n");
	alt_http_rw_write(w, b.p, b.n);
	alt_buf_free(&b);
}

/* a single range "bytes=a-b" of size: 1 and the range, 0 for none or
   ignored (several ranges, a malformed header), -1 unsatisfiable */
static int parse_range(alt_str h, int64_t size, int64_t *start, int64_t *len) {
	if (!alt_str_has_prefix(h, ALT_STR("bytes=")))
		return 0;
	h = alt_str_trim_space(alt_str_from(h, 6));
	if (alt_str_index_byte(h, ',') >= 0)
		return 0;
	alt_str a, b;
	if (!alt_str_cut_byte(h, '-', &a, &b))
		return 0;
	a = alt_str_trim_space(a);
	b = alt_str_trim_space(b);
	long long x, y;
	if (a.n == 0) {
		/* the last b bytes */
		if (alt_str_to_int(b, 10, &y) < 0 || y < 0)
			return 0;
		if (y == 0)
			return -1;
		if (y > size)
			y = size;
		*start = size - y;
		*len = y;
		return 1;
	}
	if (alt_str_to_int(a, 10, &x) < 0 || x < 0)
		return 0;
	if (x >= size)
		return -1;
	if (b.n == 0)
		y = size - 1;
	else if (alt_str_to_int(b, 10, &y) < 0 || y < x)
		return 0;
	if (y >= size)
		y = size - 1;
	*start = x;
	*len = y - x + 1;
	return 1;
}

static void serve_content(alt_http_rw *w, alt_http_request *r, const char *name, int fd, const struct stat *st) {
	alt_http_header *h = alt_http_rw_header(w);
	alt_wall mod = alt_wall_from_unix(st->st_mtim.tv_sec, 0);
	bool get_or_head = alt_str_eq_cstr(r->method, "GET") || alt_str_eq_cstr(r->method, "HEAD");
	/* preconditions (RFC 9110 section 13.2.2, second-granular dates) */
	alt_str ims, ius;
	alt_wall t;
	if (alt_http_header_get(&r->header, ALT_STR("If-Unmodified-Since"), &ius) && alt_http_parse_date(ius, &t) == 0 && mod > t) {
		alt_http_rw_write_header(w, 412);
		return;
	}
	if (get_or_head && alt_http_header_get(&r->header, ALT_STR("If-Modified-Since"), &ims) &&
		!alt_http_header_get(&r->header, ALT_STR("If-None-Match"), 0) && alt_http_parse_date(ims, &t) == 0 && mod <= t) {
		alt_http_header_del(h, ALT_STR("Content-Type"));
		alt_http_header_del(h, ALT_STR("Content-Length"));
		alt_buf lm = ALT_BUF_INIT;
		alt_http_format_date(&lm, mod);
		alt_http_header_set(h, ALT_STR("Last-Modified"), alt_buf_str(&lm));
		alt_buf_free(&lm);
		alt_http_rw_write_header(w, 304);
		return;
	}
	alt_buf lm = ALT_BUF_INIT;
	alt_http_format_date(&lm, mod);
	alt_http_header_set(h, ALT_STR("Last-Modified"), alt_buf_str(&lm));
	alt_buf_free(&lm);
	/* the type: the extension, else the content */
	char head[512];
	ssize_t nh = -1;
	if (!alt_http_header_get(h, ALT_STR("Content-Type"), 0)) {
		const char *dot = strrchr(name, '.');
		const char *ct = dot && !strchr(dot, '/') ? alt_mime_type_by_extension(alt_str_cstr(dot)) : 0;
		if (!ct) {
			nh = pread(fd, head, sizeof head, 0);
			ct = alt_http_detect_content_type(head, nh > 0 ? (size_t)nh : 0);
		}
		alt_http_header_set_cstr(h, "Content-Type", ct);
	}
	int64_t size = st->st_size, start = 0, len = size;
	int status = 200;
	alt_str range;
	if (alt_http_header_get(&r->header, ALT_STR("Range"), &range) && get_or_head) {
		alt_str ifr;
		bool ok_ifr = true;
		if (alt_http_header_get(&r->header, ALT_STR("If-Range"), &ifr))
			ok_ifr = alt_http_parse_date(ifr, &t) == 0 && t == mod;
		int pr = ok_ifr ? parse_range(range, size, &start, &len) : 0;
		if (pr < 0) {
			char cr[64];
			snprintf(cr, sizeof cr, "bytes */%lld", (long long)size);
			alt_http_header_set_cstr(h, "Content-Range", cr);
			alt_http_error(w, "invalid range: failed to overlap", 416);
			return;
		}
		if (pr > 0) {
			char cr[96];
			snprintf(cr, sizeof cr, "bytes %lld-%lld/%lld", (long long)start, (long long)(start + len - 1), (long long)size);
			alt_http_header_set_cstr(h, "Content-Range", cr);
			status = 206;
		}
	}
	alt_http_header_set_cstr(h, "Accept-Ranges", "bytes");
	char cl[24];
	snprintf(cl, sizeof cl, "%lld", (long long)len);
	alt_http_header_set_cstr(h, "Content-Length", cl);
	alt_http_rw_write_header(w, status);
	if (alt_str_eq_cstr(r->method, "HEAD"))
		return;
	char buf[32 * 1024];
	int64_t off = start, left = len;
	while (left > 0) {
		ssize_t k = pread(fd, buf, left < (int64_t)sizeof buf ? (size_t)left : sizeof buf, off);
		if (k <= 0)
			break;
		if (alt_http_rw_write(w, buf, (size_t)k) < 0)
			break;
		off += k;
		left -= k;
	}
}

static void serve_path(alt_http_rw *w, alt_http_request *r, const char *root, alt_str name, bool redirect) {
	if (redirect && alt_str_has_suffix(r->url.path, ALT_STR("/index.html"))) {
		local_redirect(w, r, "./");
		return;
	}
	alt_buf full = ALT_BUF_INIT;
	alt_buf_append_cstr(&full, root);
	alt_buf_append_str(&full, name);
	int fd = open(alt_buf_cstr(&full), O_RDONLY | O_CLOEXEC);
	if (fd < 0 || alt_str_index_byte(name, 0) >= 0) {
		http_error_for(w, fd < 0 ? errno : ENOENT);
		if (fd >= 0)
			close(fd);
		alt_buf_free(&full);
		return;
	}
	struct stat st;
	if (fstat(fd, &st) < 0) {
		http_error_for(w, errno);
		close(fd);
		alt_buf_free(&full);
		return;
	}
	alt_str url = r->url.path;
	if (redirect) {
		if (S_ISDIR(st.st_mode)) {
			if (!url.n || url.p[url.n - 1] != '/') {
				ssize_t i = alt_str_last_index_byte(url, '/');
				char *t = alt_arena_printf(r->arena, "%.*s/", (int)(url.n - (size_t)(i + 1)), url.p + i + 1);
				local_redirect(w, r, t);
				close(fd);
				alt_buf_free(&full);
				return;
			}
		} else if (url.n && url.p[url.n - 1] == '/') {
			alt_str base = alt_str_slice(url, 0, url.n - 1);
			ssize_t i = alt_str_last_index_byte(base, '/');
			char *t = alt_arena_printf(r->arena, "../%.*s", (int)(base.n - (size_t)(i + 1)), base.p + i + 1);
			local_redirect(w, r, t);
			close(fd);
			alt_buf_free(&full);
			return;
		}
	}
	if (S_ISDIR(st.st_mode)) {
		int idx = openat(fd, "index.html", O_RDONLY | O_CLOEXEC);
		struct stat ist;
		if (idx >= 0 && fstat(idx, &ist) == 0 && S_ISREG(ist.st_mode)) {
			close(fd);
			serve_content(w, r, "index.html", idx, &ist);
			close(idx);
			alt_buf_free(&full);
			return;
		}
		if (idx >= 0)
			close(idx);
		dir_list(w, fd); /* closes fd */
		alt_buf_free(&full);
		return;
	}
	if (!S_ISREG(st.st_mode)) {
		http_error_for(w, EACCES);
		close(fd);
		alt_buf_free(&full);
		return;
	}
	serve_content(w, r, alt_buf_cstr(&full), fd, &st);
	close(fd);
	alt_buf_free(&full);
}

/* path.Clean of a path starting with '/' */
static alt_str clean_abs(alt_arena *a, alt_str p) {
	alt_url base = {0}, ref = {0}, out;
	base.path = ALT_STR("/");
	ref.path = p;
	if (alt_url_resolve(a, &base, &ref, &out) < 0)
		return p;
	/* resolve keeps a trailing slash; path.Clean drops it */
	alt_str s = out.path;
	if (s.n > 1 && s.p[s.n - 1] == '/')
		s.n--;
	return s;
}

static void fs_serve(alt_http_rw *w, alt_http_request *r, void *arg) {
	alt_http_file_server *fs = arg;
	alt_str up = r->url.path;
	if (!alt_str_has_prefix(up, ALT_STR("/"))) {
		char *p = alt_arena_printf(r->arena, "/%.*s", (int)up.n, up.p);
		up = alt_str_cstr(p);
		r->url.path = up;
	}
	alt_str name = clean_abs(r->arena, up);
	if (fs->no_listing) {
		/* a directory without index.html is not listed */
		alt_buf full = ALT_BUF_INIT;
		alt_buf_append_cstr(&full, fs->root);
		alt_buf_append_str(&full, name);
		struct stat st;
		bool dir = stat(alt_buf_cstr(&full), &st) == 0 && S_ISDIR(st.st_mode);
		if (dir) {
			alt_buf_append_cstr(&full, "/index.html");
			bool idx = stat(alt_buf_cstr(&full), &st) == 0;
			if (!idx && alt_str_has_suffix(up, ALT_STR("/"))) {
				alt_buf_free(&full);
				alt_http_error(w, "404 page not found", 404);
				return;
			}
		}
		alt_buf_free(&full);
	}
	serve_path(w, r, fs->root, name, true);
}

alt_http_handler alt_http_file_server_handler(alt_http_file_server *fs) { return (alt_http_handler){fs_serve, fs}; }

void alt_http_serve_file(alt_http_rw *w, alt_http_request *r, const char *path) {
	/* a path with ".." is refused, as Go */
	alt_str p = alt_str_cstr(path), part;
	alt_str rest = r->url.path;
	while (alt_str_split_next(&rest, ALT_STR("/"), &part))
		if (alt_str_eq_cstr(part, "..")) {
			alt_http_error(w, "invalid URL path", 400);
			return;
		}
	ssize_t slash = alt_str_last_index_byte(p, '/');
	char *dir = alt_arena_printf(r->arena, "%.*s", (int)(slash > 0 ? slash : 0), p.p);
	alt_str name = slash >= 0 ? alt_str_from(p, (size_t)slash) : alt_str_cstr(alt_arena_printf(r->arena, "/%s", path));
	if (slash < 0)
		dir = ".";
	else if (slash == 0)
		dir = "";
	serve_path(w, r, dir, name, false);
}
