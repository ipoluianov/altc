/* AltC standard library: JSON (Go's encoding/json, RFC 8259).

   Parsing builds a tree of alt_json values in an arena. Strings are
   decoded (invalid UTF-8 and lone surrogates become U+FFFD, as in Go);
   numbers keep their text besides their double value, so integers of any
   size can be read exactly; objects keep their members in order
   (duplicates included: alt_json_get finds the last, as Go's decoding
   into a map does). Nesting is limited to 10000 levels.

   Writing follows Go's Marshal: compact or indented output, strings
   escaped with < > & as < > & (unless escape_html is
   off), numbers in the shortest form that reads back. */
#ifndef ALT_ENCODING_JSON_H
#define ALT_ENCODING_JSON_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ALT_JSON_NULL,
	ALT_JSON_BOOL,
	ALT_JSON_NUMBER,
	ALT_JSON_STRING,
	ALT_JSON_ARRAY,
	ALT_JSON_OBJECT,
} alt_json_type;

typedef struct alt_json alt_json;

typedef struct {
	alt_str key;
	alt_json *value;
} alt_json_member;

struct alt_json {
	alt_json_type type;
	bool b;
	double num;
	alt_str str;               /* a string's value, a number's text */
	size_t n;                  /* items of an array, members of an object */
	union {
		alt_json **items;
		alt_json_member *members;
	};
	size_t cap;
	alt_arena *arena;          /* where to add to arrays and objects */
};

/* parses text: 0, or -ALT_EJSON_SYNTAX / -ALT_EJSON_DEPTH with the byte
   offset of the error in *err_offset (when not NULL) */
int alt_json_parse(alt_arena *a, alt_str text, alt_json **out, size_t *err_offset);
/* whether text is valid JSON */
bool alt_json_valid(alt_str text);

/* the member named key of an object (the last one), NULL if none */
alt_json *alt_json_get(const alt_json *obj, const char *key);
alt_json *alt_json_get_str(const alt_json *obj, alt_str key);
/* the item i of an array, NULL out of range */
alt_json *alt_json_at(const alt_json *arr, size_t i);
/* "a.b[2].c": NULL when anything along the way is missing */
alt_json *alt_json_path(const alt_json *v, const char *path);
/* typed reads: false when v is NULL or of another type */
bool alt_json_get_bool(const alt_json *v, bool *out);
bool alt_json_get_number(const alt_json *v, double *out);
/* integers written as such ("12", "-3", "1e3" is not one): exact */
bool alt_json_get_int(const alt_json *v, long long *out);
bool alt_json_get_string(const alt_json *v, alt_str *out);

/* building */
alt_json *alt_json_new_null(alt_arena *a);
alt_json *alt_json_new_bool(alt_arena *a, bool b);
alt_json *alt_json_new_number(alt_arena *a, double d);
alt_json *alt_json_new_int(alt_arena *a, long long v);
alt_json *alt_json_new_string(alt_arena *a, alt_str s); /* copied */
alt_json *alt_json_new_array(alt_arena *a);
alt_json *alt_json_new_object(alt_arena *a);
int alt_json_append(alt_json *arr, alt_json *item);
/* adds a member (the key is copied); a member with the key is replaced */
int alt_json_set(alt_json *obj, const char *key, alt_json *value);

typedef struct {
	const char *indent;   /* NULL: compact; "  ": two spaces per level */
	const char *prefix;   /* before each line when indenting */
	bool sort_keys;       /* sorted, duplicates dropped (the last kept), as Go's maps */
	bool no_escape_html;  /* keep < > & as they are */
} alt_json_opts;

/* appends v: 0 or -EINVAL for NaN and infinities */
int alt_json_write(alt_buf *b, const alt_json *v, const alt_json_opts *opts);
/* a JSON string for s */
int alt_json_quote(alt_buf *b, alt_str s, bool escape_html);

/* a streaming writer: values are separated and keys placed as needed */
typedef struct {
	alt_buf *b;
	int depth;
	bool first[64];       /* per level: no value written yet */
	bool in_object[64];
	bool after_key;
	bool escape_html;
	int err;
} alt_json_enc;

void alt_json_enc_init(alt_json_enc *e, alt_buf *b);
void alt_json_begin_object(alt_json_enc *e);
void alt_json_end_object(alt_json_enc *e);
void alt_json_begin_array(alt_json_enc *e);
void alt_json_end_array(alt_json_enc *e);
void alt_json_key(alt_json_enc *e, const char *key);
void alt_json_string(alt_json_enc *e, alt_str s);
void alt_json_cstring(alt_json_enc *e, const char *s);
void alt_json_number(alt_json_enc *e, double d);
void alt_json_int(alt_json_enc *e, long long v);
void alt_json_bool(alt_json_enc *e, bool b);
void alt_json_null(alt_json_enc *e);
/* 0, or the first error (unbalanced calls, NaN) */
int alt_json_enc_err(const alt_json_enc *e);

#ifdef __cplusplus
}
#endif

#endif
