/* hex-encoded byte strings -> valid count; then decoded runes with sizes
   (forward), the last rune, full-rune of each prefix length 0..4
   E rune -> encoding (hex) and rune length */
#include "common.h"
#include <alt/unicode/utf8.h>

int main(void) {
	char *line;
	while ((line = diff_line())) {
		if (line[0] == 'E') {
			long r = strtol(line + 2, 0, 10);
			char buf[4];
			size_t k = alt_utf8_encode(buf, (int32_t)r);
			diff_puthex(buf, k);
			printf(" %zu %d\n", alt_utf8_rune_len((int32_t)r), alt_utf8_valid_rune((int32_t)r));
			continue;
		}
		size_t n = diff_unhex(line + 2);
		const char *p = line + 2;
		printf("%d %zu", alt_utf8_valid(p, n), alt_utf8_rune_count(p, n));
		for (size_t i = 0; i < n;) {
			int32_t r;
			size_t k = alt_utf8_decode(p + i, n - i, &r);
			printf(" %d:%zu", (int)r, k);
			i += k;
		}
		int32_t r;
		size_t k = alt_utf8_decode_last(p, n, &r);
		printf(" L%d:%zu F", (int)r, k);
		for (size_t m = 0; m <= 4 && m <= n; m++)
			printf("%d", alt_utf8_full_rune(p, m));
		printf("\n");
	}
	return 0;
}
