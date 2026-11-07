/* der kind hex -> the value read and the bytes left ("rest=N"), or "fail":
   the answers of tests/goref (cryptobyte) */
#include "common.h"
#include <alt/encoding/asn1.h>
#include <inttypes.h>

int main(void) {
	char *line, *f[3];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 3);
		size_t len = n > 2 ? diff_unhex(f[2]) : 0;
		alt_der s = alt_der_make(f[2], len);
		const char *k = f[1];
		bool ok = false;
		alt_buf r = ALT_BUF_INIT;
		if (!strcmp(k, "elem")) {
			alt_der c;
			unsigned tag;
			if ((ok = alt_der_read_any(&s, &c, &tag))) {
				alt_buf_printf(&r, "%u ", tag);
				for (size_t i = 0; i < c.n; i++)
					alt_buf_printf(&r, "%02x", c.p[i]);
			}
		} else if (!strcmp(k, "bool")) {
			bool v;
			if ((ok = alt_der_read_bool(&s, &v)))
				alt_buf_append_cstr(&r, v ? "true" : "false");
		} else if (!strcmp(k, "int64")) {
			int64_t v;
			if ((ok = alt_der_read_int64(&s, &v)))
				alt_buf_printf(&r, "%" PRId64, v);
		} else if (!strcmp(k, "uint64")) {
			uint64_t v;
			if ((ok = alt_der_read_uint64(&s, &v)))
				alt_buf_printf(&r, "%" PRIu64, v);
		} else if (!strcmp(k, "uintbytes")) {
			alt_der v;
			if ((ok = alt_der_read_uint_bytes(&s, &v)))
				for (size_t i = 0; i < v.n; i++)
					alt_buf_printf(&r, "%02x", v.p[i]);
		} else if (!strcmp(k, "enum")) {
			int64_t v;
			if ((ok = alt_der_read_enum(&s, &v)))
				alt_buf_printf(&r, "%" PRId64, v);
		} else if (!strcmp(k, "oid")) {
			alt_oid o;
			if ((ok = alt_der_read_oid(&s, &o)))
				alt_oid_format(&r, &o);
		} else if (!strcmp(k, "bits")) {
			alt_der b;
			size_t bits;
			if ((ok = alt_der_read_bit_string(&s, &b, &bits))) {
				alt_buf_printf(&r, "%zu ", bits);
				for (size_t i = 0; i < b.n; i++)
					alt_buf_printf(&r, "%02x", b.p[i]);
			}
		} else if (!strcmp(k, "utc") || !strcmp(k, "gen")) {
			int64_t t;
			if ((ok = k[0] == 'u' ? alt_der_read_utc_time(&s, &t) : alt_der_read_generalized_time(&s, &t)))
				alt_buf_printf(&r, "%" PRId64, (int64_t)t);
		}
		if (ok)
			printf("%s rest=%zu\n", alt_buf_cstr(&r), s.n);
		else
			printf("fail\n");
		alt_buf_free(&r);
	}
	return 0;
}
