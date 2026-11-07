/* enc b|q charset s -> the encoded word (hex)
   dec flag word -> "ok <hex>" or "err <msg>" (Decode); hdr flag header -> DecodeHeader
   flag 1: a charset reader for "x-upper" (ASCII upper case) and "x-fail" (fails)
   (strings in hex) */
#include "common.h"
#include <alt/mime.h>
#include <ctype.h>
#include <errno.h>

static alt_str arg(char *s) { return alt_str_make(s, diff_unhex(s)); }

static int reader(void *arg, alt_str charset, alt_str content, alt_buf *out, alt_buf *err) {
	(void)arg;
	if (alt_str_eq_cstr(charset, "x-upper")) {
		for (size_t i = 0; i < content.n; i++)
			alt_buf_append_byte(out, toupper((unsigned char)content.p[i]));
		return 0;
	}
	alt_buf_reset(err);
	if (alt_str_eq_cstr(charset, "x-fail"))
		alt_buf_append_cstr(err, "boom");
	else
		alt_buf_printf(err, "unknown charset %.*s", (int)charset.n, charset.p);
	return -EINVAL;
}

int main(void) {
	char *line, *f[8];
	alt_mime_word_decoder dec = {reader, 0};
	while ((line = diff_line())) {
		diff_fields(line, f, 8);
		alt_buf b = ALT_BUF_INIT, err = ALT_BUF_INIT;
		if (!strcmp(f[0], "enc")) {
			alt_str cs = arg(f[2]), s = arg(f[3]);
			alt_mime_word_encode(&b, f[1][0], cs, s);
			diff_puthex(b.p, b.n);
		} else {
			const alt_mime_word_decoder *d = f[1][0] == '1' ? &dec : 0;
			alt_str s = arg(f[2]);
			int rc = !strcmp(f[0], "dec") ? alt_mime_word_decode(d, &b, s, &err) : alt_mime_decode_header(d, &b, s, &err);
			if (rc < 0)
				printf("err %s", alt_buf_cstr(&err));
			else {
				printf("ok ");
				diff_puthex(b.p, b.n);
			}
		}
		printf("\n");
		alt_buf_free(&b);
		alt_buf_free(&err);
		fflush(stdout);
	}
	return 0;
}
