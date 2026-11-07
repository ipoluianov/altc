/* AltC standard library: client cookies (Go's http.ParseSetCookie and
   net/http/cookiejar without a public suffix list)

	alt_cookie_jar *jar;
	alt_cookie_jar_new(&jar);
	alt_http_client_config cfg = {.jar = jar};

   The client then sends the jar's cookies with each request and keeps
   the Set-Cookie of each response (redirects included), as Go's Client
   with a Jar. */
#ifndef ALT_NET_COOKIEJAR_H
#define ALT_NET_COOKIEJAR_H

#include <alt/arena.h>
#include <alt/net/url.h>
#include <alt/str.h>
#include <alt/time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_SAMESITE_UNSET, ALT_SAMESITE_DEFAULT, ALT_SAMESITE_LAX, ALT_SAMESITE_STRICT, ALT_SAMESITE_NONE };

typedef struct {
	alt_str name, value;
	bool quoted;
	alt_str path, domain, raw_expires;
	alt_wall expires;      /* 0: none */
	int max_age;           /* 0: unset; < 0: delete now */
	bool secure, http_only, partitioned;
	int same_site;
} alt_set_cookie;

/* Go's ParseSetCookie: 0, or -EINVAL with *why ("http: blank cookie",
   "http: '=' not found in cookie", "http: invalid cookie name"...) */
int alt_http_parse_set_cookie(alt_arena *a, alt_str line, alt_set_cookie *c, const char **why);

typedef struct alt_cookie_jar alt_cookie_jar;

int alt_cookie_jar_new(alt_cookie_jar **out);
void alt_cookie_jar_free(alt_cookie_jar *j);
/* safe for concurrent use; now in Unix seconds (0: the current time) */
void alt_cookie_jar_set(alt_cookie_jar *j, const alt_url *u, const alt_set_cookie *c, size_t n, int64_t now);
/* the Cookie header for u ("a=1; b=2", as Go's Request.AddCookie for each): "" when none */
int alt_cookie_jar_header(alt_cookie_jar *j, const alt_url *u, alt_buf *b, int64_t now);

#ifdef __cplusplus
}
#endif

#endif
