/* b32e kind hex -> encoded;  b32d kind text-hex -> "ok <hex>" or "err <offset> <hex>"
   uv hex -> Uvarint and Varint results;  pv number -> PutUvarint/PutVarint of it (hex)
   csv comma comment fpr lazy trim input-hex -> the records ("|"-separated hex fields) and errors
   csvw comma crlf field-hex... -> the written record (hex)
   (kinds: std hex rawstd rawhex) */
#include "common.h"
#include <alt/encoding/base32.h>
#include <alt/encoding/binary.h>
#include <alt/encoding/csv.h>

static const alt_base32 *kind(const char *k) {
	return !strcmp(k, "std") ? &alt_base32_std : !strcmp(k, "hex") ? &alt_base32_hex : !strcmp(k, "rawstd") ? &alt_base32_raw_std
														 : &alt_base32_raw_hex;
}

int main(void) {
	char *line, *f[64];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 64);
		alt_buf b = ALT_BUF_INIT;
		if (!strcmp(f[0], "b32e")) {
			size_t k = diff_unhex(f[2]);
			alt_base32_encode(kind(f[1]), &b, f[2], k);
			printf("%.*s", (int)b.n, b.p ? b.p : "");
		} else if (!strcmp(f[0], "b32d")) {
			size_t k = diff_unhex(f[2]), bad = 0;
			int rc = alt_base32_decode(kind(f[1]), &b, alt_str_make(f[2], k), &bad);
			if (rc < 0)
				printf("err %zu ", bad);
			else
				printf("ok ");
			diff_puthex(b.p ? b.p : "", b.n);
		} else if (!strcmp(f[0], "uv")) {
			size_t k = diff_unhex(f[1]);
			uint64_t u;
			int64_t s;
			int a = alt_uvarint(f[1], k, &u), c = alt_varint(f[1], k, &s);
			printf("%llu %d %lld %d", (unsigned long long)u, a, (long long)s, c);
			alt_str_reader sr;
			alt_bufreader br;
			alt_bufreader_init(&br, alt_str_reader_init(&sr, alt_str_make(f[1], k)), 64);
			int rc = alt_read_uvarint(&br, &u);
			printf(" %s %llu", rc == 0 ? "ok" : rc == -ALT_EEOF ? "EOF" : rc == -ALT_EUNEXPECTED_EOF ? "unexpected" : "overflow",
				(unsigned long long)u);
			alt_bufreader_free(&br);
		} else if (!strcmp(f[0], "pv")) {
			uint8_t t[16];
			unsigned long long v = strtoull(f[1], 0, 10);
			size_t k = alt_put_uvarint(t, v);
			diff_puthex((char *)t, k);
			k = alt_put_varint(t, (int64_t)v);
			printf(" ");
			diff_puthex((char *)t, k);
		} else if (!strcmp(f[0], "csv")) {
			size_t k = diff_unhex(f[6]);
			alt_str_reader sr;
			alt_csv_reader r;
			alt_csv_reader_init(&r, alt_str_reader_init(&sr, alt_str_make(f[6], k)));
			r.comma = (int32_t)strtol(f[1], 0, 10);
			r.comment = (int32_t)strtol(f[2], 0, 10);
			r.fields_per_record = atoi(f[3]);
			r.lazy_quotes = f[4][0] == '1';
			r.trim_leading_space = f[5][0] == '1';
			for (int it = 0; it < 1000; it++) {
				alt_str *fl;
				size_t nf;
				int rc = alt_csv_read(&r, &fl, &nf);
				if (rc == -ALT_EEOF)
					break;
				if (rc < 0)
					printf("{err %s}", alt_csv_error(&r));
				if (rc == 0 || nf) {
					printf("[");
					for (size_t i = 0; i < nf; i++) {
						if (i)
							printf("|");
						diff_puthex(fl[i].p, fl[i].n);
					}
					printf("]");
				}
				if (rc == -EINVAL)
					break;
			}
			alt_csv_reader_free(&r);
		} else if (!strcmp(f[0], "csvw")) {
			alt_csv_writer w = {(int32_t)strtol(f[1], 0, 10), f[2][0] == '1'};
			alt_str fs[64];
			int nf = 0;
			for (int i = 3; i < n; i++) {
				size_t k = diff_unhex(f[i]);
				fs[nf++] = alt_str_make(f[i], k);
			}
			if (alt_csv_write(&w, &b, fs, (size_t)nf) < 0)
				printf("invalid");
			else
				diff_puthex(b.p ? b.p : "", b.n);
		}
		printf("\n");
		alt_buf_free(&b);
		fflush(stdout);
	}
	return 0;
}
