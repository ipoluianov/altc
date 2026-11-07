/* AltC standard library: URLs (Go's net/url).

   A parsed URL keeps its parts as alt_str: the decoded ones (path,
   fragment, user name and password) are allocated in the arena given to
   the parser when decoding changes them, the others point into the
   string parsed, which must stay alive as long as the URL is used. */
#ifndef ALT_NET_URL_H
#define ALT_NET_URL_H

#include <alt/errors.h>
#include <alt/arena.h>
#include <alt/str.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* [scheme:][//[userinfo@]host][/]path[?query][#fragment], or
   scheme:opaque[?query][#fragment] */
typedef struct {
	alt_str scheme;      /* lower case */
	alt_str opaque;      /* encoded opaque data */
	bool has_user, has_password;
	alt_str username, password;
	alt_str host;        /* "host", "host:port", "[ipv6]:port" */
	alt_str path;        /* decoded */
	alt_str raw_path;    /* the encoded path when it differs from the default encoding */
	alt_str raw_query;   /* without '?' */
	alt_str fragment;    /* decoded, without '#' */
	alt_str raw_fragment;
	bool force_query;    /* a '?' without a query */
	bool omit_host;      /* an empty authority */
} alt_url;

/* parses an absolute or relative URL: 0 or -ALT_EURL_PARSE,
   -ALT_EURL_ESCAPE, -ALT_EURL_HOST, -ALT_EURL_PORT */
int alt_url_parse(alt_arena *a, alt_str raw, alt_url *u);
/* parses the target of an HTTP request (absolute URL or absolute path,
   no fragment) */
int alt_url_parse_request_uri(alt_arena *a, alt_str raw, alt_url *u);
/* reassembles the URL */
int alt_url_string(const alt_url *u, alt_buf *b);
int alt_url_escaped_path(const alt_url *u, alt_buf *b);
int alt_url_escaped_fragment(const alt_url *u, alt_buf *b);
/* the path and query of an HTTP request ("/" at least) */
int alt_url_request_uri(const alt_url *u, alt_buf *b);
/* the host without port and brackets, the port */
alt_str alt_url_hostname(const alt_url *u);
alt_str alt_url_port(const alt_url *u);
bool alt_url_is_abs(const alt_url *u);
/* ref resolved against base (RFC 3986 section 5.2) */
int alt_url_resolve(alt_arena *a, const alt_url *base, const alt_url *ref, alt_url *out);
/* sets the path (decoded), keeping the raw form consistent */
int alt_url_set_path(alt_arena *a, alt_url *u, alt_str path);

/* escaping */
int alt_url_query_escape(alt_buf *b, alt_str s);
int alt_url_path_escape(alt_buf *b, alt_str s);
/* 0 or -ALT_EURL_ESCAPE; *out may point into s when nothing changes */
int alt_url_query_unescape(alt_arena *a, alt_str s, alt_str *out);
int alt_url_path_unescape(alt_arena *a, alt_str s, alt_str *out);

/* query values: pairs in order of appearance */
typedef struct {
	alt_str key, value;
} alt_url_kv;

typedef struct {
	alt_url_kv *kv;
	size_t n, cap;
	alt_arena *arena; /* the pairs' memory */
} alt_url_values;

void alt_url_values_init(alt_url_values *v, alt_arena *a);
/* parses "a=1&b=2": the pairs that parse are added even when others do
   not (the first error is returned, as Go's ParseQuery does); a ';' in a
   pair is an error, at most 10000 pairs */
int alt_url_parse_query(alt_url_values *v, alt_str query);
/* the first value of key; false when absent */
bool alt_url_values_get(const alt_url_values *v, alt_str key, alt_str *value);
bool alt_url_values_has(const alt_url_values *v, alt_str key);
int alt_url_values_add(alt_url_values *v, alt_str key, alt_str value);
/* replaces all values of key */
int alt_url_values_set(alt_url_values *v, alt_str key, alt_str value);
void alt_url_values_del(alt_url_values *v, alt_str key);
/* "a=1&b=2" sorted by key (the values of a key in their order) */
int alt_url_values_encode(const alt_url_values *v, alt_buf *b);

#ifdef __cplusplus
}
#endif

#endif
