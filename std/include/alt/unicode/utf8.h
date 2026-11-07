/* AltC standard library: UTF-8 (Go's unicode/utf8) */
#ifndef ALT_UNICODE_UTF8_H
#define ALT_UNICODE_UTF8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ALT_UTF8_RUNE_ERROR = 0xFFFD, /* what invalid input decodes to */
	ALT_UTF8_MAX_RUNE = 0x10FFFF,
	ALT_UTF8_UTF_MAX = 4,         /* bytes of the longest encoding */
};

/* decodes the first rune of p[0:n]: its value in *r and its length;
   invalid or truncated input gives ALT_UTF8_RUNE_ERROR and length 1, an
   empty input length 0 */
size_t alt_utf8_decode(const char *p, size_t n, int32_t *r);
/* decodes the last rune */
size_t alt_utf8_decode_last(const char *p, size_t n, int32_t *r);
/* writes the encoding of r (ALT_UTF8_RUNE_ERROR for invalid runes) into
   buf, which has room for 4 bytes; returns the length */
size_t alt_utf8_encode(char *buf, int32_t r);
/* the length of the encoding of r, 0 if r cannot be encoded */
size_t alt_utf8_rune_len(int32_t r);
bool alt_utf8_valid(const char *p, size_t n);
bool alt_utf8_valid_rune(int32_t r);
/* the number of runes (invalid bytes count one each) */
size_t alt_utf8_rune_count(const char *p, size_t n);
/* whether p[0:n] begins with a complete (possibly invalid) encoding */
bool alt_utf8_full_rune(const char *p, size_t n);

#ifdef __cplusplus
}
#endif

#endif
