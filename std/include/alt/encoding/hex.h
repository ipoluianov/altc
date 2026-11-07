/* AltC standard library: hexadecimal (Go's encoding/hex) */
#ifndef ALT_ENCODING_HEX_H
#define ALT_ENCODING_HEX_H

#include <alt/errors.h>
#include <alt/str.h>

#ifdef __cplusplus
extern "C" {
#endif

/* appends the lower-case hexadecimal form of data */
int alt_hex_encode(alt_buf *b, const void *data, size_t n);
/* appends the bytes of s: 0 or -ALT_EENC_HEX (odd length, a bad digit) */
int alt_hex_decode(alt_buf *b, alt_str s);

#ifdef __cplusplus
}
#endif

#endif
