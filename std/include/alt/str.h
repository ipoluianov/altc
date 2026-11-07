/* AltC standard library: byte strings and buffers (Go's strings, bytes).

   alt_str is a pointer and a length: a slice of bytes that need not be
   NUL-terminated and may contain NUL bytes. Slicing costs nothing; the
   bytes belong to whoever owns the memory it points into.

   alt_buf is a growable byte buffer (strings.Builder, bytes.Buffer). Its
   bytes are always followed by a NUL that is not counted, so they can be
   used as a C string. Appending functions return 0 or -ENOMEM; a failed
   append also sets the sticky error b->err, so a series of appends can
   be checked once. */
#ifndef ALT_STR_H
#define ALT_STR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *p;
	size_t n;
} alt_str;

/* ALT_STR("literal") */
#define ALT_STR(lit) ((alt_str){(lit), sizeof(lit) - 1})
#define ALT_STR_NULL ((alt_str){0, 0})

static inline alt_str alt_str_make(const void *p, size_t n) { return (alt_str){(const char *)p, n}; }
static inline alt_str alt_str_cstr(const char *s) { return (alt_str){s, s ? strlen(s) : 0}; }

/* alt_str_slice is s[i:j], clamped to s */
alt_str alt_str_slice(alt_str s, size_t i, size_t j);
static inline alt_str alt_str_from(alt_str s, size_t i) { return alt_str_slice(s, i, s.n); }

bool alt_str_eq(alt_str a, alt_str b);
bool alt_str_eq_cstr(alt_str a, const char *b);
/* ASCII letters compare without case (HTTP header names, schemes) */
bool alt_str_eq_fold(alt_str a, alt_str b);
int alt_str_cmp(alt_str a, alt_str b);
bool alt_str_has_prefix(alt_str s, alt_str prefix);
bool alt_str_has_suffix(alt_str s, alt_str suffix);
bool alt_str_has_prefix_fold(alt_str s, alt_str prefix);

/* the index of the first (last) occurrence, -1 when absent */
ssize_t alt_str_index(alt_str s, alt_str sub);
ssize_t alt_str_index_byte(alt_str s, int c);
ssize_t alt_str_last_index(alt_str s, alt_str sub);
ssize_t alt_str_last_index_byte(alt_str s, int c);
/* the index of the first byte that is in chars, -1 when none is */
ssize_t alt_str_index_any(alt_str s, const char *chars);
bool alt_str_contains(alt_str s, alt_str sub);
size_t alt_str_count(alt_str s, alt_str sub);

/* ASCII white space (space, \t, \n, \v, \f, \r) */
alt_str alt_str_trim_space(alt_str s);
/* the bytes of cutset removed from both ends (left, right) */
alt_str alt_str_trim(alt_str s, const char *cutset);
alt_str alt_str_trim_left(alt_str s, const char *cutset);
alt_str alt_str_trim_right(alt_str s, const char *cutset);
alt_str alt_str_trim_prefix(alt_str s, alt_str prefix);
alt_str alt_str_trim_suffix(alt_str s, alt_str suffix);

/* alt_str_cut splits s around the first sep: true and the parts when sep
   occurs, false and s, "" otherwise (strings.Cut) */
bool alt_str_cut(alt_str s, alt_str sep, alt_str *before, alt_str *after);
bool alt_str_cut_byte(alt_str s, int sep, alt_str *before, alt_str *after);

/* alt_str_split_next takes the next field of *rest up to sep: a loop
   while (alt_str_split_next(&rest, sep, &field)) visits every field, as
   strings.Split does (an empty s gives one empty field) */
bool alt_str_split_next(alt_str *rest, alt_str sep, alt_str *field);
/* alt_str_fields_next skips runs of white space (strings.Fields) */
bool alt_str_fields_next(alt_str *rest, alt_str *field);

/* integers: base 2..36, or 0 for a 0x/0o/0b/0 prefix; the whole string must be
   the number; -ALT_ESYNTAX or -ALT_ERANGE */
int alt_str_to_int(alt_str s, int base, long long *out);
int alt_str_to_uint(alt_str s, int base, unsigned long long *out);

/* a NUL-terminated copy (malloc), NULL when out of memory */
char *alt_str_dup(alt_str s);
/* lower and upper case of ASCII letters, in place */
void alt_ascii_lower(char *p, size_t n);
void alt_ascii_upper(char *p, size_t n);

typedef struct {
	char *p;     /* the bytes, followed by a NUL; NULL while empty */
	size_t n;    /* length */
	size_t cap;  /* allocated, the NUL included */
	int err;     /* sticky: -ENOMEM after a failed append, else 0 */
} alt_buf;

#define ALT_BUF_INIT {0, 0, 0, 0}

static inline void alt_buf_init(alt_buf *b) { *b = (alt_buf)ALT_BUF_INIT; }
void alt_buf_free(alt_buf *b);
/* empties the buffer, keeping its memory */
static inline void alt_buf_reset(alt_buf *b) {
	b->n = 0;
	if (b->p)
		b->p[0] = 0;
}
/* makes room for extra more bytes */
int alt_buf_reserve(alt_buf *b, size_t extra);
int alt_buf_append(alt_buf *b, const void *p, size_t n);
static inline int alt_buf_append_str(alt_buf *b, alt_str s) { return alt_buf_append(b, s.p, s.n); }
static inline int alt_buf_append_cstr(alt_buf *b, const char *s) { return alt_buf_append(b, s, strlen(s)); }
int alt_buf_append_byte(alt_buf *b, int c);
int alt_buf_printf(alt_buf *b, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int alt_buf_vprintf(alt_buf *b, const char *fmt, va_list ap);
/* decimal integers without printf */
int alt_buf_append_int(alt_buf *b, long long v);
int alt_buf_append_uint(alt_buf *b, unsigned long long v);
/* sets the length (shorter, or longer with unspecified bytes after a reserve) */
void alt_buf_truncate(alt_buf *b, size_t n);
/* removes the first n bytes */
void alt_buf_consume(alt_buf *b, size_t n);
static inline alt_str alt_buf_str(const alt_buf *b) { return (alt_str){b->p ? b->p : "", b->n}; }
static inline const char *alt_buf_cstr(const alt_buf *b) { return b->p ? b->p : ""; }
/* the caller takes the memory (free it); the buffer is left empty */
char *alt_buf_detach(alt_buf *b, size_t *n);

#ifdef __cplusplus
}
#endif

#endif
