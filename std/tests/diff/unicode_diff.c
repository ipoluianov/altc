/* rune -> class letters (LdnulTmpsc_PgZ) then upper lower title fold */
#include "common.h"
#include <alt/unicode.h>

int main(void) {
	char *line;
	while ((line = diff_line())) {
		int32_t r = (int32_t)strtol(line, 0, 10);
		bool (*fn[])(int32_t) = {alt_unicode_is_letter, alt_unicode_is_digit, alt_unicode_is_number, alt_unicode_is_upper,
			alt_unicode_is_lower, alt_unicode_is_title, alt_unicode_is_mark, alt_unicode_is_punct, alt_unicode_is_symbol,
			alt_unicode_is_control, alt_unicode_is_space, alt_unicode_is_print, alt_unicode_is_graphic};
		const char *names = "LdnulTmpsc_PG";
		char out[16];
		int k = 0;
		for (int i = 0; i < 13; i++)
			if (fn[i](r))
				out[k++] = names[i];
		out[k] = 0;
		printf("%s %d %d %d %d %d\n", out, alt_unicode_to_upper(r), alt_unicode_to_lower(r), alt_unicode_to_title(r),
			alt_unicode_simple_fold(r), alt_unicode_is(&alt_unicode_Zs, r));
	}
	return 0;
}
