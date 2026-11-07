/* AltC standard library: HTTP/1.1 servers and clients (Go's net/http).

   A server reads requests on its connections and calls a handler for
   each: void handler(alt_http_rw *w, alt_http_request *r, void *arg).
   Each connection is served by a task; the tasks run on a few schedulers
   (one per processor by default), so a handler may block (read a body,
   wait on a channel, call other services) without stopping other
   connections. Everything a request needs is allocated in its arena
   (r->arena), freed when the handler returns.

   The response writer buffers the first 2 KiB of a body: a handler that
   writes less gets a Content-Length, more is sent chunked (HTTP/1.1) or
   until the connection closes (HTTP/1.0). Content-Type is detected from
   the body when the handler does not set it (Go's DetectContentType).

   The multiplexer takes Go 1.22 patterns: "[METHOD ][HOST]/path" with
   wildcards "{name}", "{name...}" (the rest of the path) and "{$}" (the
   end of the path); the most specific pattern wins, and registering
   patterns that conflict fails.

   The client keeps idle connections for reuse, follows redirects and
   works with "http" URLs (TLS comes with the crypto packages). */
#ifndef ALT_NET_HTTP_H
#define ALT_NET_HTTP_H

#include <alt/errors.h>
#include <alt/arena.h>
#include <alt/context.h>
#include <alt/io.h>
#include <alt/net.h>
#include <alt/net/url.h>
#include <alt/str.h>
#include <alt/time.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- status codes ---- */

enum {
	ALT_HTTP_CONTINUE = 100,
	ALT_HTTP_SWITCHING_PROTOCOLS = 101,
	ALT_HTTP_OK = 200,
	ALT_HTTP_CREATED = 201,
	ALT_HTTP_ACCEPTED = 202,
	ALT_HTTP_NO_CONTENT = 204,
	ALT_HTTP_PARTIAL_CONTENT = 206,
	ALT_HTTP_MOVED_PERMANENTLY = 301,
	ALT_HTTP_FOUND = 302,
	ALT_HTTP_SEE_OTHER = 303,
	ALT_HTTP_NOT_MODIFIED = 304,
	ALT_HTTP_TEMPORARY_REDIRECT = 307,
	ALT_HTTP_PERMANENT_REDIRECT = 308,
	ALT_HTTP_BAD_REQUEST = 400,
	ALT_HTTP_UNAUTHORIZED = 401,
	ALT_HTTP_FORBIDDEN = 403,
	ALT_HTTP_NOT_FOUND = 404,
	ALT_HTTP_METHOD_NOT_ALLOWED = 405,
	ALT_HTTP_REQUEST_TIMEOUT = 408,
	ALT_HTTP_CONFLICT = 409,
	ALT_HTTP_LENGTH_REQUIRED = 411,
	ALT_HTTP_PRECONDITION_FAILED = 412,
	ALT_HTTP_CONTENT_TOO_LARGE = 413,
	ALT_HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
	ALT_HTTP_RANGE_NOT_SATISFIABLE = 416,
	ALT_HTTP_EXPECTATION_FAILED = 417,
	ALT_HTTP_TOO_MANY_REQUESTS = 429,
	ALT_HTTP_HEADER_FIELDS_TOO_LARGE = 431,
	ALT_HTTP_INTERNAL_SERVER_ERROR = 500,
	ALT_HTTP_NOT_IMPLEMENTED = 501,
	ALT_HTTP_BAD_GATEWAY = 502,
	ALT_HTTP_SERVICE_UNAVAILABLE = 503,
	ALT_HTTP_GATEWAY_TIMEOUT = 504,
	ALT_HTTP_VERSION_NOT_SUPPORTED = 505,
};

/* "Not Found"; "" for unknown codes */
const char *alt_http_status_text(int code);

/* ---- headers ---- */

typedef struct {
	alt_str key;   /* canonical ("Content-Type") */
	alt_str value;
} alt_http_field;

typedef struct {
	alt_http_field *f;
	size_t n, cap;
	alt_arena *arena;  /* where keys, values and the array live */
} alt_http_header;

void alt_http_header_init(alt_http_header *h, alt_arena *a);
/* key and value are copied; the key is canonicalized */
int alt_http_header_add(alt_http_header *h, alt_str key, alt_str value);
static inline int alt_http_header_add_cstr(alt_http_header *h, const char *key, const char *value) {
	return alt_http_header_add(h, alt_str_cstr(key), alt_str_cstr(value));
}
/* replaces all values of key */
int alt_http_header_set(alt_http_header *h, alt_str key, alt_str value);
static inline int alt_http_header_set_cstr(alt_http_header *h, const char *key, const char *value) {
	return alt_http_header_set(h, alt_str_cstr(key), alt_str_cstr(value));
}
void alt_http_header_del(alt_http_header *h, alt_str key);
/* the first value of key (case-insensitive); false when absent */
bool alt_http_header_get(const alt_http_header *h, alt_str key, alt_str *value);
/* the first value or "" */
alt_str alt_http_header_value(const alt_http_header *h, const char *key);
/* whether a comma-separated header contains token (case-insensitive) */
bool alt_http_header_has_token(const alt_http_header *h, const char *key, const char *token);
/* "content-type" -> "Content-Type" (keys with invalid bytes stay as they are) */
alt_str alt_http_canonical_key(alt_arena *a, alt_str key);
/* writes "Key: value\r\n" lines sorted by key */
int alt_http_header_write(const alt_http_header *h, alt_buf *b);
bool alt_http_valid_field_name(alt_str s);
bool alt_http_valid_field_value(alt_str s);

/* ---- requests ---- */

typedef struct alt_http_request {
	alt_str method;          /* "GET" */
	alt_str request_uri;     /* the target as received ("/a?b") */
	alt_url url;
	alt_str proto;           /* "HTTP/1.1" */
	int proto_major, proto_minor;
	alt_http_header header;
	alt_str host;            /* from the URL or the Host header */
	int64_t content_length;  /* -1 when unknown (chunked) */
	bool chunked;
	bool close;              /* the connection closes after this request */
	alt_reader body;         /* reads the body (0 at its end); also for requests without one */
	alt_http_header trailer; /* trailers of a chunked body, once read */
	alt_str remote_addr;     /* "ip:port" of the client */
	struct alt_tls_conn *tls; /* the TLS connection of an HTTPS request, NULL otherwise */
	alt_ctx *ctx;            /* cancelled when the client goes away or the server shuts down */
	alt_arena *arena;        /* lives until the handler returns */
	alt_str pattern;         /* the multiplexer pattern that matched */
	struct {
		alt_str name, value;
	} path_values[16];
	int npath_values;
	void *internal;
} alt_http_request;

/* the value of a wildcard of the matched pattern ("" when absent) */
alt_str alt_http_path_value(const alt_http_request *r, const char *name);
int alt_http_set_path_value(alt_http_request *r, const char *name, alt_str value);
/* the query parameters (parsed on first use) */
const alt_url_values *alt_http_query(alt_http_request *r);
/* a query parameter or "" */
alt_str alt_http_query_value(alt_http_request *r, const char *key);
/* the form of a POST/PUT/PATCH body of type application/x-www-form-urlencoded
   and of the query, as Go's ParseForm (at most max_body bytes, 0: 10 MiB) */
int alt_http_parse_form(alt_http_request *r, size_t max_body, const alt_url_values **form);
/* the whole body in the request's arena (at most max bytes; 0: 10 MiB) */
int alt_http_read_body(alt_http_request *r, size_t max, alt_str *body);
/* a cookie of the request: true and its value when present */
bool alt_http_cookie(const alt_http_request *r, const char *name, alt_str *value);

/* ---- responses of the server ---- */

typedef struct alt_http_rw alt_http_rw;

alt_http_header *alt_http_rw_header(alt_http_rw *w);
/* sends the status and the header (once; later calls are ignored except
   for 1xx informational responses) */
int alt_http_rw_write_header(alt_http_rw *w, int status);
ssize_t alt_http_rw_write(alt_http_rw *w, const void *p, size_t n);
static inline ssize_t alt_http_rw_write_str(alt_http_rw *w, alt_str s) { return alt_http_rw_write(w, s.p, s.n); }
static inline ssize_t alt_http_rw_write_cstr(alt_http_rw *w, const char *s) { return alt_http_rw_write(w, s, strlen(s)); }
int alt_http_rw_printf(alt_http_rw *w, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
/* sends what is buffered (the header first) */
int alt_http_rw_flush(alt_http_rw *w);
alt_writer alt_http_rw_writer(alt_http_rw *w);
/* the status written (200 by default) */
int alt_http_rw_status(const alt_http_rw *w);
/* takes the connection over (WebSocket upgrades...): the handler owns
   the connection from then on and closes it itself; *br has the bytes
   already read from it (alt_bufreader_free it too) */
int alt_http_rw_hijack(alt_http_rw *w, alt_conn **c, alt_bufreader **br);

/* "Content-Type: text/plain; charset=utf-8" and the message, as http.Error */
void alt_http_error(alt_http_rw *w, const char *msg, int code);
/* a redirect to url (relative to the request's path) */
void alt_http_redirect(alt_http_rw *w, alt_http_request *r, const char *url, int code);
/* 404 page not found */
void alt_http_not_found(alt_http_rw *w, alt_http_request *r);

/* sets a cookie (Set-Cookie); max_age < 0 deletes, 0 is a session cookie */
typedef struct {
	const char *name, *value, *path, *domain;
	int max_age;
	alt_wall expires;       /* 0: none */
	bool secure, http_only;
	const char *same_site;  /* "Lax", "Strict", "None" or NULL */
} alt_http_cookie_spec;
int alt_http_set_cookie(alt_http_rw *w, const alt_http_cookie_spec *c);

/* ---- handlers ---- */

typedef void (*alt_http_handler_fn)(alt_http_rw *w, alt_http_request *r, void *arg);

typedef struct {
	alt_http_handler_fn fn;
	void *arg;
} alt_http_handler;

static inline void alt_http_serve(alt_http_handler h, alt_http_rw *w, alt_http_request *r) { h.fn(w, r, h.arg); }

/* ---- the multiplexer (ServeMux) ---- */

typedef struct alt_http_mux alt_http_mux;

int alt_http_mux_new(alt_http_mux **out);
void alt_http_mux_free(alt_http_mux *m);
/* -ALT_EHTTP_PATTERN for malformed patterns, -ALT_EHTTP_CONFLICT */
int alt_http_mux_handle(alt_http_mux *m, const char *pattern, alt_http_handler h);
int alt_http_mux_handle_func(alt_http_mux *m, const char *pattern, alt_http_handler_fn fn, void *arg);
alt_http_handler alt_http_mux_handler(alt_http_mux *m);

/* a handler that removes prefix from the path before calling h */
typedef struct {
	const char *prefix;
	alt_http_handler h;
} alt_http_strip_prefix;
alt_http_handler alt_http_strip_prefix_handler(alt_http_strip_prefix *s);

/* files of a directory: index.html, directory listings, ranges,
   If-Modified-Since, Content-Type by extension (http.FileServer) */
typedef struct {
	const char *root;
	bool no_listing;
} alt_http_file_server;
alt_http_handler alt_http_file_server_handler(alt_http_file_server *fs);
/* one file (http.ServeFile) */
void alt_http_serve_file(alt_http_rw *w, alt_http_request *r, const char *path);
/* the MIME type of content (Go's DetectContentType, at most the first 512 bytes) */
const char *alt_http_detect_content_type(const void *data, size_t n);

/* ---- the server ---- */

typedef struct {
	const char *addr;              /* ":8080" (default ":http") */
	alt_http_handler handler;
	alt_dur read_header_timeout;   /* 0: read_timeout */
	alt_dur read_timeout;          /* the whole request, 0: none */
	alt_dur write_timeout;         /* the response, 0: none */
	alt_dur idle_timeout;          /* between requests, 0: read_timeout, else none */
	size_t max_header_bytes;       /* 0: 1 MiB */
	int threads;                   /* schedulers for connections, 0: one per processor */
	/* errors of connections (malformed requests are answered, not logged) */
	void (*log)(void *arg, const char *msg);
	void *log_arg;
	/* HTTPS: connections are TLS with this configuration ("http/1.1" is
	   offered by ALPN when it names no protocols) */
	const struct alt_tls_config_s *tls;
} alt_http_server_config;

typedef struct alt_http_server alt_http_server;

int alt_http_server_new(alt_http_server **out, const alt_http_server_config *cfg);
/* serves connections of l until shutdown or close: -ALT_ECLOSED then
   (Go's ErrServerClosed); the server owns l */
int alt_http_server_serve(alt_http_server *s, alt_listener *l);
int alt_http_server_listen_and_serve(alt_http_server *s);
/* stops accepting, waits for active requests to finish (or ctx to be
   done: then -ETIMEDOUT/-ECANCELED, connections are closed) */
int alt_http_server_shutdown(alt_http_server *s, alt_ctx *ctx);
/* closes listeners and connections at once */
int alt_http_server_close(alt_http_server *s);
void alt_http_server_free(alt_http_server *s);
/* the address listened on (after serve started; NULL before) */
const alt_addr *alt_http_server_addr(alt_http_server *s);

/* listens on addr and serves with h until the process ends */
int alt_http_listen_and_serve(const char *addr, alt_http_handler h);
/* the same over HTTPS with a certificate chain and key from PEM files */
int alt_http_listen_and_serve_tls(const char *addr, const char *cert_file, const char *key_file, alt_http_handler h);

/* ---- the client ---- */

typedef struct {
	alt_dur timeout;              /* the whole exchange including the body, 0: none */
	int max_redirects;            /* 0: 10; < 0: none followed */
	int max_idle_per_host;        /* 0: 2 */
	alt_dur idle_timeout;         /* 0: 90 s */
	alt_dur dial_timeout;         /* 0: 30 s */
	size_t max_header_bytes;      /* 0: 1 MiB */
	const char *user_agent;       /* NULL: "AltC-http-client/1.0" */
	/* as Go's Transport, requests without Accept-Encoding and Range ask
	   for gzip, and gzip responses are decompressed (unless this is set) */
	bool disable_compression;
	/* HTTPS: the roots (NULL: the system's), no verification (tests only) */
	const struct alt_x509_pool *tls_roots;
	bool tls_insecure_skip_verify;
} alt_http_client_config;

typedef struct alt_http_client alt_http_client;

typedef struct alt_http_response {
	int status;                   /* 200 */
	alt_str status_text;          /* "200 OK" */
	alt_str proto;
	int proto_major, proto_minor;
	alt_http_header header;
	int64_t content_length;       /* -1 when unknown */
	bool chunked, close;
	alt_reader body;              /* read it, then alt_http_response_free */
	alt_http_header trailer;
	alt_url url;                  /* of the final request (after redirects) */
	bool uncompressed;            /* body decompressed (Content-Encoding and -Length removed) */
	alt_arena *arena;
	void *internal;
} alt_http_response;

/* a request to send: method NULL is GET; body may be NULL; content_length
   -1 sends the body chunked */
typedef struct {
	const char *method;
	const char *url;
	alt_http_header *header;      /* extra headers, may be NULL */
	alt_reader *body;
	int64_t content_length;
} alt_http_client_request;

int alt_http_client_new(alt_http_client **out, const alt_http_client_config *cfg);
void alt_http_client_free(alt_http_client *c);
int alt_http_client_do(alt_http_client *c, const alt_http_client_request *req, alt_http_response **resp, alt_ctx *ctx);
int alt_http_get(alt_http_client *c, const char *url, alt_http_response **resp, alt_ctx *ctx);
int alt_http_post(alt_http_client *c, const char *url, const char *content_type, alt_str body, alt_http_response **resp, alt_ctx *ctx);
/* the whole body in the response's arena (at most max; 0: 10 MiB) */
int alt_http_response_body(alt_http_response *r, size_t max, alt_str *body);
/* releases the response; its connection is reused when the body was read
   to its end */
void alt_http_response_free(alt_http_response *r);
/* closes idle connections */
void alt_http_client_close_idle(alt_http_client *c);

/* ---- for tests and proxies: the message parsers ---- */

/* reads a request from br (header at most max_header bytes): 0,
   -ALT_EEOF when the stream ends before the request,
   -ALT_EHTTP_BADREQUEST, -ALT_EHTTP_HEADERTOOLARGE... */
int alt_http_read_request(alt_bufreader *br, alt_arena *a, size_t max_header, alt_http_request *r);
int alt_http_read_response(alt_bufreader *br, alt_arena *a, size_t max_header, const char *method, alt_http_response *r);
/* an HTTP date: "Mon, 02 Jan 2006 15:04:05 GMT" (and the RFC 850 and
   asctime forms when parsing) */
int alt_http_format_date(alt_buf *b, alt_wall t);
int alt_http_parse_date(alt_str s, alt_wall *t);

#ifdef __cplusplus
}
#endif

#endif
