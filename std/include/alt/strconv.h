/* AltC standard library: conversions of numbers (Go's strconv) */
#ifndef ALT_STRCONV_H
#define ALT_STRCONV_H

#include <alt/errors.h>
#include <alt/str.h>

#ifdef __cplusplus
extern "C" {
#endif

/* appends f as Go's strconv.FormatFloat(f, fmt, prec, 64): fmt 'e', 'E',
   'f', 'g', 'G'; prec -1 gives the shortest digits that read back as f */
int alt_format_float(alt_buf *b, double f, char fmt, int prec);
/* the shortest decimal digits of finite f > 0: digits in buf (no dot),
   their count, and the decimal exponent: f = 0.d1d2... * 10^exp */
int alt_shortest_digits(double f, char buf[32], int *exp);
/* parses a floating-point number as Go's ParseFloat (64 bits): 0,
   -ALT_ESYNTAX, -ALT_ERANGE (with +-Inf or 0 as the value) */
int alt_parse_float(alt_str s, double *out);
/* a Go string literal of s, as strconv.Quote: printable runes as they
   are, the others escaped ("a\"b\n\u200b") */
int alt_quote(alt_buf *b, alt_str s);
/* only ASCII inside ("a\"b\n\u00e9"), as strconv.QuoteToASCII */
int alt_quote_ascii(alt_buf *b, alt_str s);
/* printable and graphic runes as they are, as strconv.QuoteToGraphic */
int alt_quote_graphic(alt_buf *b, alt_str s);
/* the inverse of alt_quote ("...", `...` and '.' forms) */
int alt_unquote(alt_buf *b, alt_str s);
int alt_format_int(alt_buf *b, long long v, int base);
int alt_format_uint(alt_buf *b, unsigned long long v, int base);
/* parse integers as Go's ParseInt and ParseUint: base 2..36, or 0 for
   Go's prefixes (0x, 0o, 0b, 0) with underscores; bits 0 is 64. 0,
   -ALT_ESYNTAX (value 0), -ALT_ERANGE (value clamped), -EINVAL (base or
   bits) */
int alt_parse_int(alt_str s, int base, int bits, long long *out);
int alt_parse_uint(alt_str s, int base, int bits, unsigned long long *out);
/* "1", "t", "T", "TRUE", "true", "True" and the false forms */
int alt_parse_bool(alt_str s, bool *out);

#ifdef __cplusplus
}
#endif

#endif
