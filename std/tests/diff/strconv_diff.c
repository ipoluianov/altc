/* F bits fmt prec -> FormatFloat
   P text -> ParseFloat bits | err code
   Q hex mode -> Quote (mode 0), QuoteToASCII (1), QuoteToGraphic (2)
   U hex -> Unquote hex | err
   I text base bits -> ParseInt value code;  N ... -> ParseUint */
#include "common.h"
#include <alt/strconv.h>
#include <inttypes.h>

int main(void) {
	char *line, *f[4];
	alt_buf b = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 4);
		alt_buf_reset(&b);
		switch (line[0]) {
		case 'F': {
			uint64_t bits = strtoull(f[1], 0, 16);
			double d;
			memcpy(&d, &bits, 8);
			alt_format_float(&b, d, f[2][0], atoi(f[3]));
			printf("%s\n", alt_buf_cstr(&b));
			break;
		}
		case 'P': {
			double d;
			int rc = alt_parse_float(alt_str_cstr(n > 1 ? f[1] : ""), &d);
			uint64_t bits;
			memcpy(&bits, &d, 8);
			if (rc == -ALT_ESYNTAX)
				printf("syntax\n");
			else
				printf("%016" PRIx64 " %s\n", d != d ? 0x7ff8000000000001ULL : bits, rc ? "range" : "ok");
			break;
		}
		case 'Q': {
			size_t k = diff_unhex(f[1]);
			alt_str q = alt_str_make(f[1], k);
			int mode = n > 2 ? atoi(f[2]) : 1;
			if (mode == 0)
				alt_quote(&b, q);
			else if (mode == 1)
				alt_quote_ascii(&b, q);
			else
				alt_quote_graphic(&b, q);
			printf("%s\n", alt_buf_cstr(&b));
			break;
		}
		case 'I':
		case 'N': {
			alt_str t = alt_str_cstr(n > 1 ? f[1] : "");
			int base = atoi(f[2]), bits = atoi(f[3]), rc;
			if (line[0] == 'I') {
				long long v;
				rc = alt_parse_int(t, base, bits, &v);
				printf("%lld", v);
			} else {
				unsigned long long v;
				rc = alt_parse_uint(t, base, bits, &v);
				printf("%llu", v);
			}
			printf(" %s\n", rc == 0 ? "ok" : rc == -ALT_ESYNTAX ? "syntax" : rc == -ALT_ERANGE ? "range" : "inval");
			break;
		}
		case 'U': {
			size_t k = diff_unhex(f[1]);
			if (alt_unquote(&b, alt_str_make(f[1], k)) < 0)
				printf("err\n");
			else {
				diff_puthex(b.p, b.n);
				printf("\n");
			}
			break;
		}
		}
	}
	alt_buf_free(&b);
	return 0;
}
