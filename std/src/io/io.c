/* Streams and buffered streams (after Go's io and bufio) */
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/task.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

ssize_t alt_read_full(alt_reader r, void *buf, size_t n) {
	size_t got = 0;
	while (got < n) {
		ssize_t k = alt_read(r, (char *)buf + got, n - got);
		if (k < 0)
			return k;
		if (k == 0)
			return got ? -ALT_EUNEXPECTED_EOF : 0;
		got += (size_t)k;
	}
	return (ssize_t)n;
}

ssize_t alt_write_all(alt_writer w, const void *buf, size_t n) {
	size_t done = 0;
	while (done < n) {
		ssize_t k = alt_write(w, (const char *)buf + done, n - done);
		if (k < 0)
			return k;
		if (k == 0)
			return -EIO; /* a writer that makes no progress */
		done += (size_t)k;
	}
	return (ssize_t)n;
}

ssize_t alt_copy(alt_writer dst, alt_reader src) {
	char buf[32 * 1024];
	size_t total = 0;
	for (;;) {
		ssize_t k = alt_read(src, buf, sizeof buf);
		if (k < 0)
			return k;
		if (k == 0)
			return (ssize_t)total;
		ssize_t w = alt_write_all(dst, buf, (size_t)k);
		if (w < 0)
			return w;
		total += (size_t)k;
	}
}

ssize_t alt_copy_n(alt_writer dst, alt_reader src, size_t n) {
	char buf[32 * 1024];
	size_t total = 0;
	while (total < n) {
		size_t want = n - total < sizeof buf ? n - total : sizeof buf;
		ssize_t k = alt_read(src, buf, want);
		if (k < 0)
			return k;
		if (k == 0)
			return -ALT_EUNEXPECTED_EOF;
		ssize_t w = alt_write_all(dst, buf, (size_t)k);
		if (w < 0)
			return w;
		total += (size_t)k;
	}
	return (ssize_t)total;
}

ssize_t alt_read_all(alt_reader src, alt_buf *b, size_t limit) {
	size_t start = b->n;
	for (;;) {
		int rc = alt_buf_reserve(b, 4096);
		if (rc)
			return rc;
		size_t room = b->cap - b->n - 1;
		if (limit) {
			size_t used = b->n - start;
			/* one byte more than the limit tells whether there is more */
			if (room > limit + 1 - used)
				room = limit + 1 - used;
		}
		ssize_t k = alt_read(src, b->p + b->n, room);
		if (k < 0)
			return k;
		if (k == 0)
			break;
		b->n += (size_t)k;
		b->p[b->n] = 0;
		if (limit && b->n - start > limit) {
			alt_buf_truncate(b, start + limit);
			return -ALT_ETOOLARGE;
		}
	}
	return (ssize_t)(b->n - start);
}

static ssize_t str_read(void *obj, void *buf, size_t n) {
	alt_str_reader *r = obj;
	size_t left = r->s.n - r->off;
	if (n > left)
		n = left;
	if (n)
		memcpy(buf, r->s.p + r->off, n);
	r->off += n;
	return (ssize_t)n;
}

alt_reader alt_str_reader_init(alt_str_reader *r, alt_str s) {
	r->s = s;
	r->off = 0;
	return (alt_reader){str_read, r};
}

static ssize_t buf_write(void *obj, const void *p, size_t n) {
	int rc = alt_buf_append(obj, p, n);
	return rc ? rc : (ssize_t)n;
}

alt_writer alt_buf_writer(alt_buf *b) { return (alt_writer){buf_write, b}; }

static ssize_t discard_write(void *obj, const void *p, size_t n) {
	(void)obj, (void)p;
	return (ssize_t)n;
}

const alt_writer alt_discard = {discard_write, 0};

static ssize_t limit_read(void *obj, void *buf, size_t n) {
	alt_limit_reader *l = obj;
	if (l->left == 0)
		return 0;
	if (n > l->left)
		n = l->left;
	ssize_t k = alt_read(l->r, buf, n);
	if (k > 0)
		l->left -= (size_t)k;
	return k;
}

alt_reader alt_limit_reader_init(alt_limit_reader *l, alt_reader r, size_t n) {
	l->r = r;
	l->left = n;
	return (alt_reader){limit_read, l};
}

static ssize_t fd_read(void *obj, void *buf, size_t n) {
	int fd = *(int *)obj;
	for (;;) {
		ssize_t k = read(fd, buf, n);
		if (k >= 0)
			return k;
		if (errno == EINTR)
			continue;
		if (errno == EAGAIN) {
			int rc = alt_wait_fd(fd, ALT_POLL_IN, ALT_FOREVER);
			if (rc < 0)
				return rc;
			continue;
		}
		return -errno;
	}
}

static ssize_t fd_write(void *obj, const void *buf, size_t n) {
	int fd = *(int *)obj;
	for (;;) {
		ssize_t k = write(fd, buf, n);
		if (k >= 0)
			return k;
		if (errno == EINTR)
			continue;
		if (errno == EAGAIN) {
			int rc = alt_wait_fd(fd, ALT_POLL_OUT, ALT_FOREVER);
			if (rc < 0)
				return rc;
			continue;
		}
		return -errno;
	}
}

alt_reader alt_fd_reader(int *fd) { return (alt_reader){fd_read, fd}; }
alt_writer alt_fd_writer(int *fd) { return (alt_writer){fd_write, fd}; }

/* ---- bufio.Reader ---- */

int alt_bufreader_init(alt_bufreader *b, alt_reader rd, size_t size) {
	if (size == 0)
		size = 4096;
	if (size < 16)
		size = 16;
	b->buf = malloc(size);
	if (!b->buf)
		return -ENOMEM;
	b->size = size;
	b->rd = rd;
	b->r = b->w = 0;
	b->err = 0;
	return 0;
}

void alt_bufreader_free(alt_bufreader *b) {
	free(b->buf);
	b->buf = 0;
}

void alt_bufreader_reset(alt_bufreader *b, alt_reader rd) {
	b->rd = rd;
	b->r = b->w = 0;
	b->err = 0;
}

/* reads more into the buffer: >0 bytes, 0 when the buffer is full, <0
   the sticky error (-ALT_EEOF at the end) */
static int fill(alt_bufreader *b) {
	if (b->err)
		return b->err;
	if (b->r > 0) {
		memmove(b->buf, b->buf + b->r, b->w - b->r);
		b->w -= b->r;
		b->r = 0;
	}
	if (b->w == b->size)
		return 0;
	/* a reader returning nothing (and no error) a hundred times is broken */
	for (int i = 0; i < 100; i++) {
		ssize_t k = alt_read(b->rd, b->buf + b->w, b->size - b->w);
		if (k < 0)
			return b->err = (int)k;
		if (k == 0)
			return b->err = -ALT_EEOF;
		b->w += (size_t)k;
		return (int)k;
	}
	return b->err = -EIO;
}

ssize_t alt_bufreader_read(alt_bufreader *b, void *p, size_t n) {
	if (n == 0)
		return 0;
	if (b->r == b->w) {
		if (b->err)
			return b->err == -ALT_EEOF ? 0 : b->err;
		if (n >= b->size) {
			/* large reads go directly into p */
			ssize_t k = alt_read(b->rd, p, n);
			if (k < 0)
				b->err = (int)k;
			else if (k == 0)
				b->err = -ALT_EEOF;
			return k;
		}
		b->r = b->w = 0;
		int rc = fill(b);
		if (rc < 0)
			return rc == -ALT_EEOF ? 0 : rc;
	}
	size_t k = b->w - b->r;
	if (k > n)
		k = n;
	memcpy(p, b->buf + b->r, k);
	b->r += k;
	return (ssize_t)k;
}

int alt_bufreader_byte(alt_bufreader *b) {
	while (b->r == b->w) {
		int rc = fill(b);
		if (rc < 0)
			return rc;
	}
	return (unsigned char)b->buf[b->r++];
}

int alt_bufreader_peek(alt_bufreader *b, size_t n, alt_str *s) {
	if (n > b->size) {
		*s = alt_str_make(b->buf + b->r, b->w - b->r);
		return -ALT_EBUFFULL;
	}
	while (b->w - b->r < n) {
		int rc = fill(b);
		if (rc < 0) {
			*s = alt_str_make(b->buf + b->r, b->w - b->r);
			return rc == -ALT_EEOF && s->n ? -ALT_EUNEXPECTED_EOF : rc;
		}
	}
	*s = alt_str_make(b->buf + b->r, n);
	return 0;
}

int alt_bufreader_slice(alt_bufreader *b, int delim, alt_str *s) {
	size_t scanned = 0;
	for (;;) {
		const char *q = memchr(b->buf + b->r + scanned, delim, b->w - b->r - scanned);
		if (q) {
			size_t len = (size_t)(q - (b->buf + b->r)) + 1;
			*s = alt_str_make(b->buf + b->r, len);
			b->r += len;
			return 0;
		}
		scanned = b->w - b->r;
		if (b->w - b->r == b->size) {
			*s = alt_str_make(b->buf + b->r, b->size);
			b->r = b->w;
			return -ALT_EBUFFULL;
		}
		int rc = fill(b);
		if (rc < 0) {
			*s = alt_str_make(b->buf + b->r, b->w - b->r);
			b->r = b->w;
			return rc;
		}
	}
}

int alt_bufreader_line(alt_bufreader *b, alt_buf *line, size_t max) {
	size_t start = line->n;
	for (;;) {
		alt_str s;
		int rc = alt_bufreader_slice(b, '\n', &s);
		if (rc < 0 && rc != -ALT_EBUFFULL) {
			if (rc == -ALT_EEOF && (s.n || line->n > start)) {
				alt_buf_append_str(line, s);
				return -ALT_EUNEXPECTED_EOF; /* a line without its end */
			}
			return rc;
		}
		int e = alt_buf_append_str(line, s);
		if (e)
			return e;
		if (max && line->n - start > max + 2) /* 2: the line ending */
			return -ALT_ETOOLARGE;
		if (rc == -ALT_EBUFFULL)
			continue;
		line->n--; /* \n, and a \r before it */
		if (line->n > start && line->p[line->n - 1] == '\r')
			line->n--;
		line->p[line->n] = 0;
		if (max && line->n - start > max)
			return -ALT_ETOOLARGE;
		return 0;
	}
}

ssize_t alt_bufreader_discard(alt_bufreader *b, size_t n) {
	size_t done = 0;
	while (done < n) {
		if (b->r == b->w) {
			int rc = fill(b);
			if (rc < 0)
				return rc == -ALT_EEOF ? -ALT_EUNEXPECTED_EOF : rc;
		}
		size_t k = b->w - b->r;
		if (k > n - done)
			k = n - done;
		b->r += k;
		done += k;
	}
	return (ssize_t)n;
}

static ssize_t bufreader_read(void *obj, void *p, size_t n) { return alt_bufreader_read(obj, p, n); }
alt_reader alt_bufreader_reader(alt_bufreader *b) { return (alt_reader){bufreader_read, b}; }

/* ---- bufio.Writer ---- */

int alt_bufwriter_init(alt_bufwriter *b, alt_writer wr, size_t size) {
	if (size == 0)
		size = 4096;
	b->buf = malloc(size);
	if (!b->buf)
		return -ENOMEM;
	b->size = size;
	b->n = 0;
	b->wr = wr;
	b->err = 0;
	return 0;
}

void alt_bufwriter_free(alt_bufwriter *b) {
	free(b->buf);
	b->buf = 0;
}

void alt_bufwriter_reset(alt_bufwriter *b, alt_writer wr) {
	b->wr = wr;
	b->n = 0;
	b->err = 0;
}

int alt_bufwriter_flush(alt_bufwriter *b) {
	if (b->err)
		return b->err;
	if (b->n == 0)
		return 0;
	ssize_t k = alt_write_all(b->wr, b->buf, b->n);
	if (k < 0)
		return b->err = (int)k;
	b->n = 0;
	return 0;
}

ssize_t alt_bufwriter_write(alt_bufwriter *b, const void *p, size_t n) {
	if (b->err)
		return b->err;
	const char *s = p;
	size_t left = n;
	while (left > b->size - b->n) {
		if (b->n == 0) {
			/* large writes go directly to wr */
			ssize_t k = alt_write_all(b->wr, s, left);
			if (k < 0)
				return b->err = (int)k;
			return (ssize_t)n;
		}
		size_t k = b->size - b->n;
		memcpy(b->buf + b->n, s, k);
		b->n += k;
		s += k;
		left -= k;
		int rc = alt_bufwriter_flush(b);
		if (rc)
			return rc;
	}
	memcpy(b->buf + b->n, s, left);
	b->n += left;
	return (ssize_t)n;
}

int alt_bufwriter_byte(alt_bufwriter *b, int c) {
	char ch = (char)c;
	ssize_t k = alt_bufwriter_write(b, &ch, 1);
	return k < 0 ? (int)k : 0;
}

int alt_bufwriter_printf(alt_bufwriter *b, const char *fmt, ...) {
	char small[512];
	va_list ap;
	va_start(ap, fmt);
	va_list ap2;
	va_copy(ap2, ap);
	int k = vsnprintf(small, sizeof small, fmt, ap2);
	va_end(ap2);
	int rc;
	if (k < 0)
		rc = -EINVAL;
	else if ((size_t)k < sizeof small)
		rc = (int)alt_bufwriter_write(b, small, (size_t)k);
	else {
		char *big = malloc((size_t)k + 1);
		if (!big)
			rc = -ENOMEM;
		else {
			vsnprintf(big, (size_t)k + 1, fmt, ap);
			rc = (int)alt_bufwriter_write(b, big, (size_t)k);
			free(big);
		}
	}
	va_end(ap);
	return rc < 0 ? rc : 0;
}

static ssize_t bufwriter_write(void *obj, const void *p, size_t n) { return alt_bufwriter_write(obj, p, n); }
alt_writer alt_bufwriter_writer(alt_bufwriter *b) { return (alt_writer){bufwriter_write, b}; }
