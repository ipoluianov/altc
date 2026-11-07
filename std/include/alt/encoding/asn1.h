/* AltC standard library: ASN.1 DER reading and writing, after Go's
   golang.org/x/crypto/cryptobyte (used by crypto/x509 and crypto/tls).

   Reading works on views (alt_der: bytes and their count) and consumes
   from the front; every reader returns false on malformed input
   (non-minimal lengths, high tag numbers, indefinite lengths are
   rejected, as DER requires) without consuming. Writing appends to an
   alt_buf; alt_der_begin/alt_der_end bracket a constructed element whose
   length is filled in at the end. */
#ifndef ALT_ENCODING_ASN1_H
#define ALT_ENCODING_ASN1_H

#include <alt/str.h>
#include <alt/time.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ALT_ASN1_BOOLEAN = 1,
	ALT_ASN1_INTEGER = 2,
	ALT_ASN1_BIT_STRING = 3,
	ALT_ASN1_OCTET_STRING = 4,
	ALT_ASN1_NULL = 5,
	ALT_ASN1_OID = 6,
	ALT_ASN1_ENUM = 10,
	ALT_ASN1_UTF8_STRING = 12,
	ALT_ASN1_PRINTABLE_STRING = 19,
	ALT_ASN1_T61_STRING = 20,
	ALT_ASN1_IA5_STRING = 22,
	ALT_ASN1_UTC_TIME = 23,
	ALT_ASN1_GENERALIZED_TIME = 24,
	ALT_ASN1_UNIVERSAL_STRING = 28,
	ALT_ASN1_BMP_STRING = 30,
	ALT_ASN1_CONSTRUCTED = 0x20,
	ALT_ASN1_CONTEXT = 0x80,
	ALT_ASN1_SEQUENCE = 0x10 | 0x20,
	ALT_ASN1_SET = 0x11 | 0x20,
};

typedef struct {
	const uint8_t *p;
	size_t n;
} alt_der;

static inline alt_der alt_der_make(const void *p, size_t n) { return (alt_der){(const uint8_t *)p, n}; }
static inline bool alt_der_empty(alt_der s) { return s.n == 0; }

/* an object identifier: up to 32 arcs */
typedef struct {
	int n;
	uint32_t arc[32];
} alt_oid;

bool alt_oid_eq(const alt_oid *a, const alt_oid *b);
/* "1.2.840.113549" */
int alt_oid_format(alt_buf *b, const alt_oid *o);
/* a constant: ALT_OID(1, 2, 840) */
#define ALT_OID(...) ((alt_oid){sizeof((uint32_t[]){__VA_ARGS__}) / sizeof(uint32_t), {__VA_ARGS__}})

/* the contents of the next element with the given tag (header removed) */
bool alt_der_read(alt_der *s, alt_der *out, unsigned tag);
/* the whole next element with the given tag (header included) */
bool alt_der_read_element(alt_der *s, alt_der *out, unsigned tag);
/* the next element of any tag: its tag and contents */
bool alt_der_read_any(alt_der *s, alt_der *out, unsigned *tag);
bool alt_der_read_any_element(alt_der *s, alt_der *out, unsigned *tag);
bool alt_der_peek(alt_der s, unsigned tag);
bool alt_der_skip(alt_der *s, unsigned tag);
/* an element that may be absent: *present tells; out its contents */
bool alt_der_read_optional(alt_der *s, alt_der *out, bool *present, unsigned tag);
bool alt_der_skip_optional(alt_der *s, unsigned tag);

bool alt_der_read_bool(alt_der *s, bool *out);
bool alt_der_read_int64(alt_der *s, int64_t *out);
bool alt_der_read_uint64(alt_der *s, uint64_t *out);
/* a non-negative INTEGER's magnitude, leading zeros removed (at least one byte) */
bool alt_der_read_uint_bytes(alt_der *s, alt_der *out);
/* any INTEGER's minimal two's complement bytes (as written) */
bool alt_der_read_int_bytes(alt_der *s, alt_der *out);
bool alt_der_read_enum(alt_der *s, int64_t *out);
/* fails also for OIDs of more than 32 arcs (valid DER): read those with
   alt_der_read_oid_bytes */
bool alt_der_read_oid(alt_der *s, alt_oid *out);
/* a valid OBJECT IDENTIFIER's contents, of any number of arcs */
bool alt_der_read_oid_bytes(alt_der *s, alt_der *out);
/* the arcs of valid contents: false when they do not fit an alt_oid */
bool alt_oid_from_bytes(alt_der b, alt_oid *out);
/* the bytes and the number of bits */
bool alt_der_read_bit_string(alt_der *s, alt_der *bytes, size_t *bits);
/* a BIT STRING of whole bytes */
bool alt_der_read_bit_string_bytes(alt_der *s, alt_der *out);
/* times as Unix seconds (certificates go up to the year 9999), with
   the formats and checks of cryptobyte: YYMMDDHHMM[SS] and
   YYYYMMDDHHMMSS, then Z or a non-zero +hhmm/-hhmm offset; UTCTime
   years 50-99 are 1950-1999 */
bool alt_der_read_utc_time(alt_der *s, int64_t *unix_sec);
bool alt_der_read_generalized_time(alt_der *s, int64_t *unix_sec);
/* an integer with an explicit or implicit context tag (cryptobyte's ReadOptionalASN1Integer) */
bool alt_der_read_optional_int64(alt_der *s, int64_t *out, unsigned tag, int64_t def);

/* ---- writing ---- */

typedef struct {
	alt_buf *b;
	size_t open[32];   /* where the contents of the open elements start */
	int depth;
	int err;
} alt_der_builder;

void alt_der_builder_init(alt_der_builder *d, alt_buf *b);
void alt_der_begin(alt_der_builder *d, unsigned tag);
void alt_der_end(alt_der_builder *d);
void alt_der_add(alt_der_builder *d, unsigned tag, const void *p, size_t n);
void alt_der_add_int64(alt_der_builder *d, int64_t v);
/* a non-negative integer from its big-endian magnitude */
void alt_der_add_uint_bytes(alt_der_builder *d, const void *p, size_t n);
void alt_der_add_oid(alt_der_builder *d, const alt_oid *o);
void alt_der_add_bool(alt_der_builder *d, bool v);
void alt_der_add_null(alt_der_builder *d);
void alt_der_add_bit_string(alt_der_builder *d, const void *p, size_t n);
/* Unix seconds as UTCTime for 1950-2049, else GeneralizedTime (as RFC 5280 asks) */
void alt_der_add_time(alt_der_builder *d, int64_t unix_sec);
/* 0, or the first error (-ENOMEM, -EINVAL for unbalanced brackets) */
int alt_der_builder_err(const alt_der_builder *d);

#ifdef __cplusplus
}
#endif

#endif
