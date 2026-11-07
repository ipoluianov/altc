/* mt hex -> "type|k=v|..." (hex values) or "err <message>"
   fmt type k v ... (hex) -> the formatted value (hex) or "invalid"
   qp hex -> the decoded bytes (hex) then " err <message>" if any
   mp boundary hex -> per part "[header|name|filename|body hex]", then "end" or "err <message>"
   mpw boundary (f name value | F field file content | P k v...)... -> the body (hex) */
#include "common.h"
#include <alt/mime.h>
#include <alt/mime/multipart.h>

static void hexs(alt_str s) { diff_puthex(s.p ? s.p : "", s.n); }

int main(void) {
	char *line, *f[256];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 256);
		alt_arena a;
		alt_arena_init(&a, 0);
		if (!strcmp(f[0], "mt")) {
			size_t k = diff_unhex(f[1]);
			alt_str t;
			alt_mime_param *p;
			size_t np;
			const char *why = "";
			int rc = alt_mime_parse_media_type(&a, alt_str_make(f[1], k), &t, &p, &np, &why);
			if (rc < 0)
				printf("err %s [%.*s]", why, (int)t.n, t.p);
			else {
				printf("%.*s", (int)t.n, t.p);
				for (size_t i = 0; i < np; i++) {
					printf("|%.*s=", (int)p[i].key.n, p[i].key.p);
					hexs(p[i].value);
				}
			}
		} else if (!strcmp(f[0], "fmt")) {
			alt_mime_param p[64];
			size_t np = 0;
			for (int i = 2; i + 1 < n; i += 2) {
				size_t kk = diff_unhex(f[i]), kv = diff_unhex(f[i + 1]);
				alt_mime_param np2 = {alt_str_make(f[i], kk), alt_str_make(f[i + 1], kv)};
				/* a Go map: the last value of a key */
				size_t j = 0;
				while (j < np && !alt_str_eq(p[j].key, np2.key))
					j++;
				p[j] = np2;
				if (j == np)
					np++;
			}
			size_t tn = diff_unhex(f[1]);
			alt_buf b = ALT_BUF_INIT;
			if (alt_mime_format_media_type(&b, alt_str_make(f[1], tn), p, np) < 0)
				printf("invalid");
			else
				hexs(alt_buf_str(&b));
			alt_buf_free(&b);
		} else if (!strcmp(f[0], "qp")) {
			size_t k = diff_unhex(f[1]);
			alt_str_reader sr;
			alt_qp_reader q;
			alt_qp_reader_init(&q, alt_str_reader_init(&sr, alt_str_make(f[1], k)));
			alt_buf b = ALT_BUF_INIT;
			char buf[7];
			ssize_t r;
			while ((r = alt_qp_read(&q, buf, sizeof buf)) > 0)
				alt_buf_append(&b, buf, (size_t)r);
			hexs(alt_buf_str(&b));
			if (r < 0)
				printf(" err %s", r == -ALT_EMIME ? alt_qp_error(&q) : r == -ALT_EUNEXPECTED_EOF ? "unexpected EOF" : alt_strerror((int)r));
			alt_buf_free(&b);
			alt_qp_reader_free(&q);
		} else if (!strcmp(f[0], "mp")) {
			size_t bk = diff_unhex(f[1]), k = diff_unhex(f[2]);
			alt_str_reader sr;
			alt_multipart_reader *mr;
			alt_multipart_reader_new(&mr, alt_str_reader_init(&sr, alt_str_make(f[2], k)), alt_str_make(f[1], bk));
			for (int it = 0; it < 100; it++) {
				alt_multipart_part *p;
				int rc = alt_multipart_next_part(mr, &p);
				if (rc == -ALT_EEOF) {
					printf("end");
					break;
				}
				if (rc < 0) {
					printf("err %s", alt_multipart_error(mr)[0] ? alt_multipart_error(mr) : alt_strerror(rc));
					break;
				}
				printf("[");
				for (size_t i = 0; i < p->header.n; i++)
					printf("%s%.*s=%.*s", i ? "," : "", (int)p->header.f[i].key.n, p->header.f[i].key.p, (int)p->header.f[i].value.n,
						p->header.f[i].value.p);
				printf("|%.*s|%.*s|", (int)p->form_name.n, p->form_name.p, (int)p->file_name.n, p->file_name.p);
				alt_buf b = ALT_BUF_INIT;
				char buf[13];
				ssize_t r;
				while ((r = alt_read(p->body, buf, sizeof buf)) > 0)
					alt_buf_append(&b, buf, (size_t)r);
				hexs(alt_buf_str(&b));
				if (r < 0)
					printf(" readerr");
				alt_buf_free(&b);
				printf("]");
			}
			alt_multipart_reader_free(mr);
		} else if (!strcmp(f[0], "mpw")) {
			alt_buf out = ALT_BUF_INIT;
			alt_multipart_writer w;
			alt_multipart_writer_init(&w, alt_buf_writer(&out));
			if (alt_multipart_writer_set_boundary(&w, alt_str_cstr(f[1])) < 0) {
				printf("badboundary\n"); /* a random boundary then */
				alt_buf_free(&out);
				alt_arena_free(&a);
				continue;
			}
			for (int i = 2; i < n; i++) {
				if (f[i][0] == 'f' && i + 2 < n) {
					size_t a1 = diff_unhex(f[i + 1]), a2 = diff_unhex(f[i + 2]);
					alt_multipart_write_field(&w, alt_str_make(f[i + 1], a1), alt_str_make(f[i + 2], a2));
					i += 2;
				} else if (f[i][0] == 'F' && i + 3 < n) {
					size_t a1 = diff_unhex(f[i + 1]), a2 = diff_unhex(f[i + 2]), a3 = diff_unhex(f[i + 3]);
					alt_multipart_create_form_file(&w, alt_str_make(f[i + 1], a1), alt_str_make(f[i + 2], a2));
					alt_write_all(w.w, f[i + 3], a3);
					i += 3;
				}
			}
			alt_multipart_writer_close(&w);
			alt_buf ct = ALT_BUF_INIT;
			alt_multipart_form_data_content_type(&w, &ct);
			printf("%s ", alt_buf_cstr(&ct));
			hexs(alt_buf_str(&out));
			alt_buf_free(&out);
			alt_buf_free(&ct);
		}
		printf("\n");
		fflush(stdout);
		alt_arena_free(&a);
	}
	return 0;
}
