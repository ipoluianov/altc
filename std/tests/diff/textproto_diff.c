/* Requests (data in hex), answers as tests/stdtextproto_test.go prints Go's:
   rl d; rcl d; code expect d; resp expect d; dot d; dotl d; dotp n d; mh d;
   canon s; trim s; pl s; dw chunk... [L] */
#include "common.h"
#include <alt/net/textproto.h>

static alt_str arg(char *s) { return alt_str_make(s, diff_unhex(s)); }

typedef struct {
	alt_str_reader sr;
	alt_bufreader br;
	alt_textproto_reader r;
} rd;

static void rd_init(rd *x, alt_str data) {
	alt_bufreader_init(&x->br, alt_str_reader_init(&x->sr, data), 4096);
	alt_textproto_reader_init(&x->r, &x->br);
}

static void rd_free(rd *x) {
	alt_textproto_reader_free(&x->r);
	alt_bufreader_free(&x->br);
}

static void hex(alt_str s) { diff_puthex(s.p, s.n); }

/* the lines left: "hex;" each, then "E:<err>" */
static void rest(rd *x, bool continued) {
	alt_buf line = ALT_BUF_INIT;
	for (int i = 0; i < 50; i++) {
		int rc = continued ? alt_textproto_read_continued_line(&x->r, &line) : alt_textproto_read_line(&x->r, &line);
		if (rc < 0) {
			printf("E:%s", alt_textproto_error(&x->r));
			break;
		}
		hex(alt_buf_str(&line));
		printf(";");
	}
	alt_buf_free(&line);
}

/* a stable sort by key (Go's map, keys sorted, values in order) */
static void sort_fields(alt_textproto_field *f, size_t n) {
	for (size_t i = 1; i < n; i++)
		for (size_t j = i; j > 0 && alt_str_cmp(f[j - 1].key, f[j].key) > 0; j--) {
			alt_textproto_field t = f[j];
			f[j] = f[j - 1];
			f[j - 1] = t;
		}
}

int main(void) {
	char *line, *f[64];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 64);
		rd x;
		alt_buf b = ALT_BUF_INIT;
		const char *op = f[0];
		if (!strcmp(op, "rl") || !strcmp(op, "rcl")) {
			rd_init(&x, arg(f[1]));
			rest(&x, op[1] == 'c');
			rd_free(&x);
		} else if (!strcmp(op, "code") || !strcmp(op, "resp")) {
			int expect = atoi(f[1]);
			rd_init(&x, arg(f[2]));
			for (int i = 0; i < 20; i++) {
				int code;
				int rc = op[0] == 'c' ? alt_textproto_read_code_line(&x.r, expect, &code, &b)
						      : alt_textproto_read_response(&x.r, expect, &code, &b);
				printf("%d|", code);
				hex(alt_buf_str(&b));
				printf("|%s;", rc < 0 ? alt_textproto_error(&x.r) : "");
				if (rc == -ALT_EEOF || rc == -ALT_EUNEXPECTED_EOF)
					break;
			}
			rd_free(&x);
		} else if (!strcmp(op, "dot")) {
			rd_init(&x, arg(f[1]));
			int rc = alt_textproto_read_dot_bytes(&x.r, &b);
			hex(alt_buf_str(&b));
			printf("|%s|", rc < 0 ? alt_textproto_error(&x.r) : "");
			rest(&x, false);
			rd_free(&x);
		} else if (!strcmp(op, "dotl")) {
			rd_init(&x, arg(f[1]));
			alt_arena a;
			alt_arena_init(&a, 0);
			alt_str *lines;
			size_t nl;
			int rc = alt_textproto_read_dot_lines(&x.r, &a, &lines, &nl);
			for (size_t i = 0; i < nl; i++) {
				hex(lines[i]);
				printf(",");
			}
			printf("|%s|", rc < 0 ? alt_textproto_error(&x.r) : "");
			rest(&x, false);
			alt_arena_free(&a);
			rd_free(&x);
		} else if (!strcmp(op, "dotp")) {
			size_t want = (size_t)atoi(f[1]);
			rd_init(&x, arg(f[2]));
			alt_reader d = alt_textproto_dot_reader(&x.r);
			char buf[3];
			while (b.n < want) {
				size_t k = want - b.n < 3 ? want - b.n : 3;
				ssize_t got = alt_read(d, buf, k);
				if (got <= 0)
					break;
				alt_buf_append(&b, buf, (size_t)got);
			}
			hex(alt_buf_str(&b));
			printf("|");
			rest(&x, false);
			rd_free(&x);
		} else if (!strcmp(op, "mh")) {
			rd_init(&x, arg(f[1]));
			alt_arena a;
			alt_arena_init(&a, 0);
			alt_textproto_header h;
			alt_textproto_header_init(&h, &a);
			int rc = alt_textproto_read_mime_header(&x.r, &h);
			sort_fields(h.f, h.n);
			for (size_t i = 0; i < h.n; i++) {
				if (i == 0 || !alt_str_eq(h.f[i].key, h.f[i - 1].key)) {
					if (i)
						printf(" ");
					hex(h.f[i].key);
					printf("=");
				} else
					printf(",");
				hex(h.f[i].value);
			}
			/* Get through the canonicalizing lookup */
			if (h.n) {
				/* Get of the first key in lower case */
				alt_buf low = ALT_BUF_INIT;
				alt_buf_append_str(&low, h.f[0].key);
				alt_ascii_lower(low.p, low.n);
				printf("|g:");
				hex(alt_textproto_header_get(&h, alt_buf_str(&low)));
				alt_buf_free(&low);
			}
			printf("|%s|", rc < 0 ? alt_textproto_error(&x.r) : "");
			rest(&x, false);
			alt_arena_free(&a);
			rd_free(&x);
		} else if (!strcmp(op, "canon")) {
			alt_textproto_canonical_key(&b, arg(f[1]));
			hex(alt_buf_str(&b));
		} else if (!strcmp(op, "trim")) {
			hex(alt_textproto_trim(arg(f[1])));
		} else if (!strcmp(op, "pl") || !strcmp(op, "dw")) {
			alt_bufwriter bw;
			alt_bufwriter_init(&bw, alt_buf_writer(&b), 4096);
			alt_textproto_writer w;
			alt_textproto_writer_init(&w, &bw);
			if (op[0] == 'p') {
				alt_str s = arg(f[1]);
				alt_textproto_printf_line(&w, "%.*s", (int)s.n, s.p);
			} else {
				alt_writer d = alt_textproto_dot_writer(&w);
				bool line_after = false;
				for (int i = 1; i < n; i++) {
					if (!strcmp(f[i], "L")) {
						line_after = true;
						continue;
					}
					alt_str s = arg(f[i]);
					alt_write(d, s.p, s.n);
				}
				if (line_after)
					alt_textproto_printf_line(&w, "x");
				else
					alt_textproto_dot_close(&w);
			}
			alt_bufwriter_free(&bw);
			hex(alt_buf_str(&b));
		}
		printf("\n");
		alt_buf_free(&b);
		fflush(stdout);
	}
	return 0;
}
