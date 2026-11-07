/* AltC standard library: HPACK, the header compression of HTTP/2 (RFC
   7541; Go's golang.org/x/net/http2/hpack).

   An encoder and a decoder each keep a dynamic table; one of each is
   used per direction of an HTTP/2 connection. The encoder makes the same
   bytes as Go's: exact matches are indexed (the static table first, then
   the newest dynamic entry), other fields are added to the table when
   they fit and are not sensitive, strings are Huffman-coded when that is
   shorter.

	alt_hpack_encoder e;
	alt_hpack_encoder_init(&e);
	alt_hpack_field f = {ALT_STR(":status"), ALT_STR("200")};
	alt_hpack_encode(&e, &block, &f);

   Decoding errors carry Go's texts (alt_hpack_error): "decoding error:
   invalid indexed representation index 70", "hpack: invalid
   Huffman-encoded data", "decoding error: truncated headers"... */
#ifndef ALT_NET_HPACK_H
#define ALT_NET_HPACK_H

#include <alt/arena.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	alt_str name, value;
	bool sensitive;            /* never indexed (credentials) */
} alt_hpack_field;

/* the size of a field in a table: name and value lengths plus 32 */
static inline uint32_t alt_hpack_field_size(const alt_hpack_field *f) { return (uint32_t)(f->name.n + f->value.n + 32); }

typedef struct {
	struct alt__hpack_entry *e; /* a ring, the oldest at head */
	size_t n, cap, head;
	uint32_t size, max_size, allowed_max;
} alt_hpack_table;

typedef struct {
	alt_hpack_table t;
	uint32_t min_size, max_size_limit;
	bool table_size_update;
} alt_hpack_encoder;

void alt_hpack_encoder_init(alt_hpack_encoder *e);
void alt_hpack_encoder_free(alt_hpack_encoder *e);
/* the table size to use (bounded by the limit); the next field announces it */
void alt_hpack_encoder_set_max_table_size(alt_hpack_encoder *e, uint32_t v);
/* the most the peer accepts (its SETTINGS_HEADER_TABLE_SIZE; 4096 at first) */
void alt_hpack_encoder_set_max_table_size_limit(alt_hpack_encoder *e, uint32_t v);
uint32_t alt_hpack_encoder_max_table_size(const alt_hpack_encoder *e);
/* appends the representation of f (and a pending table size update) */
int alt_hpack_encode(alt_hpack_encoder *e, alt_buf *out, const alt_hpack_field *f);

typedef struct {
	alt_hpack_table t;
	size_t max_string;         /* 0: no limit ("hpack: string too long") */
	char err[96];
} alt_hpack_decoder;

/* both the table size and the most an update may ask for */
void alt_hpack_decoder_init(alt_hpack_decoder *d, uint32_t max_table_size);
void alt_hpack_decoder_free(alt_hpack_decoder *d);
void alt_hpack_decoder_set_max_table_size(alt_hpack_decoder *d, uint32_t v);
void alt_hpack_decoder_set_allowed_max_table_size(alt_hpack_decoder *d, uint32_t v);

/* called for each field: < 0 stops the decoding with that result */
typedef int (*alt_hpack_emit_fn)(void *arg, const alt_hpack_field *f);

/* decodes a whole header block; the strings of the fields live in a:
   0 or -ALT_EHPACK (alt_hpack_error says why) */
int alt_hpack_decode(alt_hpack_decoder *d, const void *block, size_t n, alt_arena *a, alt_hpack_emit_fn emit, void *arg);
const char *alt_hpack_error(const alt_hpack_decoder *d);

/* the Huffman code */
size_t alt_hpack_huffman_encode_len(alt_str s);
int alt_hpack_huffman_append(alt_buf *b, alt_str s);
/* decodes into b (at most max bytes, 0: no limit): 0, -ALT_EHPACK, or
   -ALT_ETOOLARGE beyond max */
int alt_hpack_huffman_decode(alt_buf *b, const void *p, size_t n, size_t max);

#ifdef __cplusplus
}
#endif

#endif
