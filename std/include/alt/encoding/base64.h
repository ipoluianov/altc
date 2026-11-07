/* AltC standard library: base64 (Go's encoding/base64, RFC 4648) */
#ifndef ALT_ENCODING_BASE64_H
#define ALT_ENCODING_BASE64_H

#include <alt/errors.h>
#include <alt/str.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char alphabet[64];
	char pad;         /* '=' or 0 for none */
} alt_base64;

extern const alt_base64 alt_base64_std;     /* StdEncoding */
extern const alt_base64 alt_base64_url;     /* URLEncoding */
extern const alt_base64 alt_base64_raw_std; /* RawStdEncoding: no padding */
extern const alt_base64 alt_base64_raw_url; /* RawURLEncoding */

int alt_base64_encode(const alt_base64 *e, alt_buf *b, const void *data, size_t n);
/* appends the decoded bytes: 0 or -ALT_EENC_BASE64; \r and \n are
   ignored, as Go does */
int alt_base64_decode(const alt_base64 *e, alt_buf *b, alt_str s);
size_t alt_base64_encoded_len(const alt_base64 *e, size_t n);

#ifdef __cplusplus
}
#endif

#endif
