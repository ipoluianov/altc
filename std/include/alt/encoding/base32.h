/* AltC standard library: base32 (Go's encoding/base32, RFC 4648) */
#ifndef ALT_ENCODING_BASE32_H
#define ALT_ENCODING_BASE32_H

#include <alt/errors.h>
#include <alt/str.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char alphabet[32];
	char pad;         /* '=' or 0 for none */
} alt_base32;

extern const alt_base32 alt_base32_std;     /* StdEncoding */
extern const alt_base32 alt_base32_hex;     /* HexEncoding ("extended hex") */
extern const alt_base32 alt_base32_raw_std; /* without padding */
extern const alt_base32 alt_base32_raw_hex;

size_t alt_base32_encoded_len(const alt_base32 *e, size_t n);
int alt_base32_encode(const alt_base32 *e, alt_buf *b, const void *data, size_t n);
/* appends the decoded bytes (those before an error too): 0 or
   -ALT_EENC_BASE32 with *bad (when not NULL) the offset Go reports
   (after \r and \n are removed, as Go's DecodeString) */
int alt_base32_decode(const alt_base32 *e, alt_buf *b, alt_str s, size_t *bad);

#ifdef __cplusplus
}
#endif

#endif
