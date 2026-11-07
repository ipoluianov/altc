/* quoted-printable after Go's mime/quotedprintable reader (BSD-3-Clause,
   see LICENSE-Go) */
#include <alt/mime.h>
#include <alt/strconv.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int alt_qp_reader_init(alt_qp_reader *q, alt_reader r) {
	memset(q, 0, sizeof *q);
	return alt_bufreader_init(&q->br, r, 4096);
}

void alt_qp_reader_free(alt_qp_reader *q) {
	alt_bufreader_free(&q->br);
	alt_buf_free(&q->line);
}

const char *alt_qp_error(const alt_qp_reader *q) { return q->err; }

static int qerr(alt_qp_reader *q, const char *msg) {
	snprintf(q->err, sizeof q->err, "%s", msg);
	return -ALT_EMIME;
}

static int fromhex(char c) {
	return c >= '0' && c <= '9' ? c - '0' : c >= 'A' && c <= 'F' ? c - 'A' + 10 : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}

static bool discard_ws(char c) { return c == '\n' || c == '\r' || c == ' ' || c == '\t'; }

ssize_t alt_qp_read(alt_qp_reader *q, void *buf, size_t n) {
	uint8_t *p = buf;
	size_t got = 0;
	while (got < n) {
		if (q->off >= q->line.n) {
			if (q->rerr)
				return got ? (ssize_t)got : q->rerr == -ALT_EEOF ? 0 : q->rerr;
			alt_buf_reset(&q->line);
			q->off = 0;
			alt_str s;
			int rc = alt_bufreader_slice(&q->br, '\n', &s);
			if (rc < 0)
				q->rerr = rc;
			const char *w = s.p;
			size_t wn = s.n;
			bool has_lf = wn >= 1 && w[wn - 1] == '\n', has_cr = wn >= 2 && w[wn - 2] == '\r' && has_lf;
			size_t ln = wn;
			while (ln && discard_ws(w[ln - 1]))
				ln--;
			alt_buf_append(&q->line, w, ln);
			if (ln && w[ln - 1] == '=') {
				/* a soft line break */
				const char *rs = w + ln;
				size_t rn = wn - ln;
				while (rn && (*rs == ' ' || *rs == '\t'))
					rs++, rn--;
				alt_buf_truncate(&q->line, ln - 1);
				bool lf = rn >= 1 && rs[0] == '\n', crlf = rn >= 2 && rs[0] == '\r' && rs[1] == '\n';
				if (!lf && !crlf && !(rn == 0 && q->line.n > 0 && q->rerr == -ALT_EEOF)) {
					alt_buf m = ALT_BUF_INIT, qs = ALT_BUF_INIT;
					alt_quote(&qs, alt_str_make(rs, rn));
					alt_buf_printf(&m, "quotedprintable: invalid bytes after =: %s", alt_buf_cstr(&qs));
					q->rerr = qerr(q, alt_buf_cstr(&m));
					alt_buf_free(&m), alt_buf_free(&qs);
				}
			} else if (has_lf)
				alt_buf_append(&q->line, has_cr ? "\r\n" : "\n", has_cr ? 2 : 1);
			continue;
		}
		const char *l = q->line.p + q->off;
		size_t ln = q->line.n - q->off;
		uint8_t b = (uint8_t)l[0];
		if (b == '=') {
			int hi = ln >= 3 ? fromhex(l[1]) : -1, lo = ln >= 3 ? fromhex(l[2]) : -1;
			if (ln < 3 || hi < 0 || lo < 0) {
				if (ln >= 2 && l[1] != '\r' && l[1] != '\n') {
					/* a lone = stays (as Go) */
					p[got++] = '=';
					q->off++;
					continue;
				}
				if (got)
					return (ssize_t)got;
				if (ln < 3)
					return -ALT_EUNEXPECTED_EOF;
				char m[64];
				snprintf(m, sizeof m, "quotedprintable: invalid hex byte 0x%02x", (unsigned char)(hi < 0 ? l[1] : l[2]));
				return qerr(q, m);
			}
			p[got++] = (uint8_t)(hi << 4 | lo);
			q->off += 3;
			continue;
		}
		if (!(b == '\t' || b == '\r' || b == '\n' || b >= 0x80) && (b < ' ' || b > '~')) {
			if (got)
				return (ssize_t)got;
			char m[64];
			snprintf(m, sizeof m, "quotedprintable: invalid unescaped byte 0x%02x in body", b);
			return qerr(q, m);
		}
		p[got++] = b;
		q->off++;
	}
	return (ssize_t)got;
}

static ssize_t qp_read(void *obj, void *buf, size_t n) { return alt_qp_read(obj, buf, n); }
alt_reader alt_qp_reader_of(alt_qp_reader *q) { return (alt_reader){qp_read, q}; }
