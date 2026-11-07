/* J hex -> compact JSON (sorted keys, HTML escaped) as hex | err
   I hex -> indented ("  ") | err
   E kind hex -> base64 of the bytes (kind: std url rawstd rawurl)
   D kind hex -> decoded bytes as hex | err
   H hex -> hex encoding;  X hex -> hex decoding as hex | err */
#include "common.h"
#include <alt/encoding/base64.h>
#include <alt/encoding/hex.h>
#include <alt/encoding/json.h>
#include <math.h>

static bool finite_tree(const alt_json *v) {
	if (v->type == ALT_JSON_NUMBER)
		return !isinf(v->num);
	if (v->type == ALT_JSON_ARRAY)
		for (size_t i = 0; i < v->n; i++)
			if (!finite_tree(v->items[i]))
				return false;
	if (v->type == ALT_JSON_OBJECT)
		for (size_t i = 0; i < v->n; i++)
			if (!finite_tree(v->members[i].value))
				return false;
	return true;
}

static const alt_base64 *enc(const char *k) {
	if (!strcmp(k, "url"))
		return &alt_base64_url;
	if (!strcmp(k, "rawstd"))
		return &alt_base64_raw_std;
	if (!strcmp(k, "rawurl"))
		return &alt_base64_raw_url;
	return &alt_base64_std;
}

int main(void) {
	char *line, *f[3];
	alt_buf b = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 3);
		alt_buf_reset(&b);
		char op = line[0];
		if (op == 'J' || op == 'I') {
			size_t k = diff_unhex(f[1]);
			alt_arena a;
			alt_arena_init(&a, 0);
			alt_json *v;
			alt_json_opts o = {op == 'I' ? "  " : 0, 0, true, false};
			if (alt_json_parse(&a, alt_str_make(f[1], k), &v, 0) < 0 || !finite_tree(v) || alt_json_write(&b, v, &o) < 0)
				printf("err\n");
			else {
				diff_puthex(b.p, b.n);
				printf("\n");
			}
			alt_arena_free(&a);
		} else if (op == 'E' || op == 'D') {
			size_t k = n > 2 ? diff_unhex(f[2]) : 0;
			if (op == 'E') {
				alt_base64_encode(enc(f[1]), &b, f[2], k);
				printf("%s\n", alt_buf_cstr(&b));
			} else if (alt_base64_decode(enc(f[1]), &b, alt_str_make(f[2], k)) < 0)
				printf("err\n");
			else {
				diff_puthex(b.p, b.n);
				printf("\n");
			}
		} else if (op == 'H') {
			size_t k = diff_unhex(f[1]);
			alt_hex_encode(&b, f[1], k);
			printf("%s\n", alt_buf_cstr(&b));
		} else if (op == 'X') {
			size_t k = diff_unhex(f[1]);
			if (alt_hex_decode(&b, alt_str_make(f[1], k)) < 0)
				printf("err\n");
			else {
				diff_puthex(b.p, b.n);
				printf("\n");
			}
		}
	}
	alt_buf_free(&b);
	return 0;
}
