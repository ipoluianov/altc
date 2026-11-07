/* AltC standard library: PEM (RFC 1421; Go's encoding/pem) */
#ifndef ALT_ENCODING_PEM_H
#define ALT_ENCODING_PEM_H

#include <alt/arena.h>
#include <alt/str.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	alt_str key, value;
} alt_pem_header;

typedef struct {
	alt_str type;          /* "CERTIFICATE" */
	alt_pem_header *headers;
	size_t nheaders;
	alt_str bytes;         /* the decoded contents (in the arena) */
} alt_pem_block;

/* the next block of *rest (Go's pem.Decode): true and the block, *rest
   moved after it; false when there is none (*rest unchanged) */
bool alt_pem_decode(alt_arena *a, alt_str *rest, alt_pem_block *out);
/* appends a block: 64-column base64, headers as given ("Proc-Type" first, as Go) */
int alt_pem_encode(alt_buf *b, const char *type, const alt_pem_header *headers, size_t nheaders, const void *data, size_t n);

#ifdef __cplusplus
}
#endif

#endif
