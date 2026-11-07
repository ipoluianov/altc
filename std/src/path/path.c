/* Paths after Go's path and path/filepath (BSD-3-Clause, see LICENSE-Go) */
#define _GNU_SOURCE
#include <alt/path.h>
#include <alt/sort.h>
#include <alt/unicode/utf8.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int alt_path_clean(alt_buf *out, alt_str p) {
	if (!p.n)
		return alt_buf_append_byte(out, '.');
	bool rooted = p.p[0] == '/';
	size_t n = p.n, r = 0, dotdot = 0;
	alt_buf b = ALT_BUF_INIT;
	if (rooted) {
		alt_buf_append_byte(&b, '/');
		r = 1, dotdot = 1;
	}
	while (r < n) {
		if (p.p[r] == '/')
			r++;
		else if (p.p[r] == '.' && (r + 1 == n || p.p[r + 1] == '/'))
			r++;
		else if (p.p[r] == '.' && p.p[r + 1] == '.' && (r + 2 == n || p.p[r + 2] == '/')) {
			r += 2;
			if (b.n > dotdot) {
				/* back to the previous slash */
				size_t w = b.n - 1;
				while (w > dotdot && b.p[w] != '/')
					w--;
				alt_buf_truncate(&b, w);
			} else if (!rooted) {
				if (b.n > 0)
					alt_buf_append_byte(&b, '/');
				alt_buf_append_cstr(&b, "..");
				dotdot = b.n;
			}
		} else {
			if ((rooted && b.n != 1) || (!rooted && b.n != 0))
				alt_buf_append_byte(&b, '/');
			for (; r < n && p.p[r] != '/'; r++)
				alt_buf_append_byte(&b, p.p[r]);
		}
	}
	if (b.n == 0)
		alt_buf_append_byte(&b, '.');
	int rc = alt_buf_append(out, b.p, b.n);
	alt_buf_free(&b);
	return rc;
}

int alt_path_join(alt_buf *out, const alt_str *e, size_t n) {
	alt_buf b = ALT_BUF_INIT;
	bool any = false;
	for (size_t i = 0; i < n; i++) {
		if (!e[i].n)
			continue;
		if (any)
			alt_buf_append_byte(&b, '/');
		alt_buf_append_str(&b, e[i]);
		any = true;
	}
	int rc = any ? alt_path_clean(out, alt_buf_str(&b)) : 0;
	alt_buf_free(&b);
	return rc;
}

void alt_path_split(alt_str p, alt_str *dir, alt_str *file) {
	ssize_t i = alt_str_last_index_byte(p, '/');
	*dir = alt_str_slice(p, 0, (size_t)(i + 1));
	*file = alt_str_slice(p, (size_t)(i + 1), p.n);
}

alt_str alt_path_base(alt_str p) {
	if (!p.n)
		return ALT_STR(".");
	while (p.n && p.p[p.n - 1] == '/')
		p.n--;
	ssize_t i = alt_str_last_index_byte(p, '/');
	if (i >= 0)
		p = alt_str_slice(p, (size_t)i + 1, p.n);
	if (!p.n)
		return ALT_STR("/");
	return p;
}

int alt_path_dir(alt_buf *b, alt_str p) {
	alt_str d, f;
	alt_path_split(p, &d, &f);
	return alt_path_clean(b, d);
}

alt_str alt_path_ext(alt_str p) {
	for (size_t i = p.n; i-- > 0 && p.p[i] != '/';)
		if (p.p[i] == '.')
			return alt_str_slice(p, i, p.n);
	return ALT_STR("");
}

bool alt_path_is_abs(alt_str p) { return p.n && p.p[0] == '/'; }

bool alt_path_is_local(alt_str p) {
	/* unix's isLocal: not absolute, not empty, no ".." element after cleaning */
	if (alt_path_is_abs(p) || !p.n)
		return false;
	bool has_dots = false;
	for (alt_str rest = p; rest.n;) {
		alt_str part;
		ssize_t i = alt_str_index_byte(rest, '/');
		if (i < 0)
			part = rest, rest = alt_str_slice(rest, rest.n, rest.n);
		else
			part = alt_str_slice(rest, 0, (size_t)i), rest = alt_str_slice(rest, (size_t)i + 1, rest.n);
		if (alt_str_eq_cstr(part, ".") || alt_str_eq_cstr(part, "..")) {
			has_dots = true;
			break;
		}
	}
	if (has_dots) {
		alt_buf c = ALT_BUF_INIT;
		alt_path_clean(&c, p);
		alt_str s = alt_buf_str(&c);
		bool bad = alt_str_eq_cstr(s, "..") || alt_str_has_prefix(s, ALT_STR("../"));
		alt_buf_free(&c);
		if (bad)
			return false;
	}
	return true;
}

/* ---- Match ---- */

static void scan_chunk(alt_str pat, bool *star, alt_str *chunk, alt_str *rest) {
	*star = false;
	while (pat.n && pat.p[0] == '*') {
		pat = alt_str_slice(pat, 1, pat.n);
		*star = true;
	}
	bool inrange = false;
	for (size_t i = 0; i < pat.n; i++) {
		switch (pat.p[i]) {
		case '\\':
			if (i + 1 < pat.n)
				i++;
			break;
		case '[': inrange = true; break;
		case ']': inrange = false; break;
		case '*':
			if (!inrange) {
				*chunk = alt_str_slice(pat, 0, i);
				*rest = alt_str_slice(pat, i, pat.n);
				return;
			}
		}
	}
	*chunk = pat;
	*rest = alt_str_slice(pat, pat.n, pat.n);
}

static int get_esc(alt_str chunk, int32_t *r, alt_str *next) {
	if (!chunk.n || chunk.p[0] == '-' || chunk.p[0] == ']')
		return -EINVAL;
	if (chunk.p[0] == '\\') {
		chunk = alt_str_slice(chunk, 1, chunk.n);
		if (!chunk.n)
			return -EINVAL;
	}
	size_t k = alt_utf8_decode(chunk.p, chunk.n, r);
	int rc = 0;
	if (*r == 0xfffd && k == 1)
		rc = -EINVAL;
	*next = alt_str_slice(chunk, k, chunk.n);
	if (!next->n)
		rc = -EINVAL;
	return rc;
}

/* Go's matchChunk: 1 matched (rest set), 0 not, -EINVAL */
static int match_chunk(alt_str chunk, alt_str s, alt_str *rest) {
	bool failed = false;
	while (chunk.n) {
		failed = failed || s.n == 0;
		switch (chunk.p[0]) {
		case '[': {
			int32_t r = 0;
			if (!failed) {
				size_t k = alt_utf8_decode(s.p, s.n, &r);
				s = alt_str_slice(s, k, s.n);
			}
			chunk = alt_str_slice(chunk, 1, chunk.n);
			bool negated = false;
			if (chunk.n && chunk.p[0] == '^') {
				negated = true;
				chunk = alt_str_slice(chunk, 1, chunk.n);
			}
			bool match = false;
			int nrange = 0;
			for (;;) {
				if (chunk.n && chunk.p[0] == ']' && nrange > 0) {
					chunk = alt_str_slice(chunk, 1, chunk.n);
					break;
				}
				int32_t lo, hi;
				if (get_esc(chunk, &lo, &chunk) < 0)
					return -EINVAL;
				hi = lo;
				if (chunk.p[0] == '-' && get_esc(alt_str_slice(chunk, 1, chunk.n), &hi, &chunk) < 0)
					return -EINVAL;
				match = match || (lo <= r && r <= hi);
				nrange++;
			}
			failed = failed || match == negated;
			break;
		}
		case '?':
			if (!failed) {
				failed = s.p[0] == '/';
				int32_t r;
				size_t k = alt_utf8_decode(s.p, s.n, &r);
				s = alt_str_slice(s, k, s.n);
			}
			chunk = alt_str_slice(chunk, 1, chunk.n);
			break;
		case '\\':
			chunk = alt_str_slice(chunk, 1, chunk.n);
			if (!chunk.n)
				return -EINVAL;
			/* fallthrough */
		default:
			if (!failed) {
				failed = chunk.p[0] != s.p[0];
				s = alt_str_slice(s, 1, s.n);
			}
			chunk = alt_str_slice(chunk, 1, chunk.n);
		}
	}
	if (failed)
		return 0;
	*rest = s;
	return 1;
}

int alt_path_match(alt_str pattern, alt_str name) {
	while (pattern.n) {
		bool star;
		alt_str chunk, t;
		scan_chunk(pattern, &star, &chunk, &pattern);
		if (star && !chunk.n)
			return alt_str_index_byte(name, '/') < 0;
		int ok = match_chunk(chunk, name, &t);
		if (ok == 1 && (!t.n || pattern.n)) {
			name = t;
			continue;
		}
		if (ok < 0)
			return ok;
		bool again = false;
		if (star)
			for (size_t i = 0; i < name.n && name.p[i] != '/'; i++) {
				ok = match_chunk(chunk, alt_str_slice(name, i + 1, name.n), &t);
				if (ok == 1) {
					if (!pattern.n && t.n)
						continue;
					name = t;
					again = true;
					break;
				}
				if (ok < 0)
					return ok;
			}
		if (again)
			continue;
		/* the rest of the pattern must be well formed */
		while (pattern.n) {
			scan_chunk(pattern, &star, &chunk, &pattern);
			if (match_chunk(chunk, ALT_STR(""), &t) < 0)
				return -EINVAL;
		}
		return 0;
	}
	return name.n == 0;
}

/* ---- Rel, Abs ---- */

int alt_path_rel(alt_buf *out, alt_str basep, alt_str targp, const char **why) {
	alt_buf bb = ALT_BUF_INIT, tb = ALT_BUF_INIT, r = ALT_BUF_INIT;
	alt_path_clean(&bb, basep);
	alt_path_clean(&tb, targp);
	alt_str base = alt_buf_str(&bb), targ = alt_buf_str(&tb);
	int rc = 0;
	if (alt_str_eq(base, targ)) {
		alt_buf_append_byte(out, '.');
		goto done;
	}
	if (alt_str_eq_cstr(base, "."))
		base = ALT_STR("");
	bool bs = base.n && base.p[0] == '/', ts = targ.n && targ.p[0] == '/';
	if (bs != ts) {
		rc = -EINVAL;
		goto done;
	}
	size_t bl = base.n, tl = targ.n, b0 = 0, bi = 0, t0 = 0, ti = 0;
	for (;;) {
		while (bi < bl && base.p[bi] != '/')
			bi++;
		while (ti < tl && targ.p[ti] != '/')
			ti++;
		if (!alt_str_eq(alt_str_slice(targ, t0, ti), alt_str_slice(base, b0, bi)))
			break;
		if (bi < bl)
			bi++;
		if (ti < tl)
			ti++;
		b0 = bi, t0 = ti;
	}
	if (alt_str_eq_cstr(alt_str_slice(base, b0, bi), "..")) {
		rc = -EINVAL;
		goto done;
	}
	if (b0 != bl) {
		size_t seps = 0;
		for (size_t i = b0; i < bl; i++)
			seps += base.p[i] == '/';
		alt_buf_append_cstr(&r, "..");
		for (size_t i = 0; i < seps; i++)
			alt_buf_append_cstr(&r, "/..");
		if (t0 != tl) {
			alt_buf_append_byte(&r, '/');
			alt_buf_append_str(&r, alt_str_slice(targ, t0, tl));
		}
		alt_path_clean(out, alt_buf_str(&r));
	} else
		alt_buf_append_str(out, alt_str_slice(targ, t0, tl));
done:
	if (rc < 0 && why) {
		static __thread char msg[512];
		size_t k = (size_t)snprintf(msg, sizeof msg, "Rel: can't make %.*s relative to %.*s", (int)targp.n, targp.p, (int)basep.n,
			basep.p);
		(void)k;
		*why = msg;
	}
	alt_buf_free(&bb), alt_buf_free(&tb), alt_buf_free(&r);
	return rc ? rc : out->err;
}

int alt_path_abs(alt_buf *b, alt_str p) {
	if (alt_path_is_abs(p))
		return alt_path_clean(b, p);
	char cwd[4096];
	if (!getcwd(cwd, sizeof cwd))
		return -errno;
	alt_str parts[2] = {alt_str_cstr(cwd), p};
	return alt_path_join(b, parts, 2);
}

/* ---- the file system ---- */

typedef struct {
	char *name;
	unsigned type;
} dent;

static int cmp_dent(const void *a, const void *b, void *arg) {
	(void)arg;
	return strcmp(((const dent *)a)->name, ((const dent *)b)->name);
}

/* the sorted entries of a directory */
static int read_dir(const char *path, dent **out, size_t *n) {
	*out = 0;
	*n = 0;
	DIR *d = opendir(path);
	if (!d)
		return -errno;
	size_t cap = 0;
	struct dirent *e;
	int rc = 0;
	while ((e = readdir(d))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		if (*n == cap) {
			cap = cap ? cap * 2 : 16;
			dent *nd = realloc(*out, cap * sizeof *nd);
			if (!nd) {
				rc = -ENOMEM;
				break;
			}
			*out = nd;
		}
		unsigned type = 0;
		switch (e->d_type) {
		case DT_DIR: type = S_IFDIR; break;
		case DT_REG: type = S_IFREG; break;
		case DT_LNK: type = S_IFLNK; break;
		case DT_FIFO: type = S_IFIFO; break;
		case DT_SOCK: type = S_IFSOCK; break;
		case DT_CHR: type = S_IFCHR; break;
		case DT_BLK: type = S_IFBLK; break;
		default: {
			struct stat st;
			char full[4096];
			snprintf(full, sizeof full, "%s/%s", path, e->d_name);
			if (lstat(full, &st) == 0)
				type = st.st_mode & S_IFMT;
		}
		}
		(*out)[*n].name = strdup(e->d_name);
		(*out)[(*n)++].type = type;
	}
	closedir(d);
	if (*n)
		alt_sort(*out, *n, sizeof **out, cmp_dent, 0);
	return rc;
}

static void free_dents(dent *d, size_t n) {
	for (size_t i = 0; i < n; i++)
		free(d[i].name);
	free(d);
}

static bool has_meta(alt_str s) {
	for (size_t i = 0; i < s.n; i++)
		if (strchr("*?[\\", s.p[i]) && s.p[i])
			return true;
	return false;
}

typedef struct {
	alt_arena *a;
	alt_str *v;
	size_t n, cap;
} strs;

static int add_str(strs *s, alt_str x) {
	if (s->n == s->cap) {
		size_t cap = s->cap ? s->cap * 2 : 16;
		alt_str *nv = alt_arena_realloc(s->a, s->v, s->cap * sizeof *nv, cap * sizeof *nv);
		if (!nv)
			return -ENOMEM;
		s->v = nv;
		s->cap = cap;
	}
	s->v[s->n++] = alt_arena_str(s->a, x);
	return 0;
}

/* Go's glob: the names in dir matching pattern */
static void glob_dir(strs *out, alt_str dir, alt_str pattern) {
	char d[4096];
	snprintf(d, sizeof d, "%.*s", (int)dir.n, dir.p);
	struct stat st;
	if (stat(d, &st) != 0 || !S_ISDIR(st.st_mode))
		return;
	dent *e;
	size_t n;
	read_dir(d, &e, &n);
	for (size_t i = 0; i < n; i++) {
		alt_str name = alt_str_cstr(e[i].name);
		if (alt_path_match(pattern, name) == 1) {
			alt_buf b = ALT_BUF_INIT;
			alt_str parts[2] = {dir, name};
			alt_path_join(&b, parts, 2);
			add_str(out, alt_buf_str(&b));
			alt_buf_free(&b);
		}
	}
	free_dents(e, n);
}

static int glob(strs *out, alt_str pattern, int depth) {
	if (depth > 10000)
		return -EINVAL;
	if (!has_meta(pattern)) {
		char p[4096];
		snprintf(p, sizeof p, "%.*s", (int)pattern.n, pattern.p);
		struct stat st;
		if (lstat(p, &st) == 0)
			add_str(out, pattern);
		return 0;
	}
	alt_str dir, file;
	alt_path_split(pattern, &dir, &file);
	/* cleanGlobPath */
	if (!dir.n)
		dir = ALT_STR(".");
	else if (!alt_str_eq_cstr(dir, "/"))
		dir.n--;
	if (!has_meta(dir)) {
		glob_dir(out, dir, file);
		return 0;
	}
	if (alt_str_eq(dir, pattern))
		return -EINVAL; /* the same pattern again: ErrBadPattern */
	strs dirs = {out->a, 0, 0, 0};
	int rc = glob(&dirs, dir, depth + 1);
	if (rc < 0)
		return rc;
	for (size_t i = 0; i < dirs.n; i++)
		glob_dir(out, dirs.v[i], file);
	return 0;
}

int alt_path_glob(alt_arena *a, alt_str pattern, alt_str **names, size_t *n) {
	*names = 0;
	*n = 0;
	if (alt_path_match(pattern, ALT_STR("")) < 0)
		return -EINVAL;
	strs out = {a, 0, 0, 0};
	int rc = glob(&out, pattern, 0);
	if (rc < 0)
		return rc;
	*names = out.v;
	*n = out.n;
	return 0;
}

static int walk(alt_str path, const alt_dir_entry *d, alt_walk_fn fn, void *arg) {
	int rc = fn(arg, path, d, 0);
	if (rc != 0 || !d->is_dir)
		return rc == ALT_PATH_SKIP_DIR && d->is_dir ? 0 : rc;
	char p[4096];
	snprintf(p, sizeof p, "%.*s", (int)path.n, path.p);
	dent *e;
	size_t n;
	int err = read_dir(p, &e, &n);
	if (err < 0) {
		/* a second call reports the error reading the directory */
		rc = fn(arg, path, d, err);
		if (rc != 0) {
			free_dents(e, n);
			return rc == ALT_PATH_SKIP_DIR && d->is_dir ? 0 : rc;
		}
	}
	for (size_t i = 0; i < n; i++) {
		alt_buf b = ALT_BUF_INIT;
		alt_str parts[2] = {path, alt_str_cstr(e[i].name)};
		alt_path_join(&b, parts, 2);
		alt_dir_entry de = {alt_str_cstr(e[i].name), e[i].type == S_IFDIR, e[i].type};
		rc = walk(alt_buf_str(&b), &de, fn, arg);
		alt_buf_free(&b);
		if (rc != 0) {
			if (rc == ALT_PATH_SKIP_DIR)
				rc = 0; /* the rest of this directory is skipped */
			free_dents(e, n);
			return rc;
		}
	}
	free_dents(e, n);
	return 0;
}

int alt_path_walk(alt_str root, alt_walk_fn fn, void *arg) {
	char p[4096];
	snprintf(p, sizeof p, "%.*s", (int)root.n, root.p);
	struct stat st;
	int rc;
	alt_dir_entry d = {alt_path_base(root), false, 0};
	if (lstat(p, &st) != 0)
		rc = fn(arg, root, 0, -errno);
	else {
		d.is_dir = S_ISDIR(st.st_mode);
		d.type = st.st_mode & S_IFMT;
		rc = walk(root, &d, fn, arg);
	}
	if (rc == ALT_PATH_SKIP_DIR || rc == ALT_PATH_SKIP_ALL)
		rc = 0;
	return rc;
}
