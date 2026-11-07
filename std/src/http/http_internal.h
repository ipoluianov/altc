/* The http package's internal interface */
#ifndef ALT_HTTP_INTERNAL_H
#define ALT_HTTP_INTERNAL_H

#include <alt/net/http.h>

enum { ALT__BODY_NONE, ALT__BODY_LENGTH, ALT__BODY_CHUNKED, ALT__BODY_CLOSE };

/* the body of a received message */
typedef struct {
	alt_bufreader *br;
	int kind;
	uint64_t left;            /* LENGTH: bytes left; CHUNKED: bytes left in the chunk */
	uint64_t read;            /* bytes of body read */
	bool check_end;           /* CHUNKED: a CRLF after the chunk data is due */
	bool done;                /* the end was reached */
	int err;                  /* sticky */
	int64_t excess;           /* CHUNKED: non-data bytes beyond the allowance */
	alt_http_header *trailer;
	size_t max_trailer;
	void (*before_read)(void *); /* called once before the first read */
	void *before_arg;
} alt__http_body;

void alt__http_body_init(alt__http_body *b, alt_bufreader *br, int kind, uint64_t length, alt_http_header *trailer);
alt_reader alt__http_body_reader(alt__http_body *b);

/* what a parsed request keeps besides the public fields */
typedef struct {
	alt__http_body body;
	alt_url_values query;
	bool query_parsed;
	alt_url_values form;
	bool form_parsed;
	struct alt__http_conn *conn; /* the server's connection */
} alt__http_request_state;

/* reads header lines up to the empty line; budget counts the bytes */
int alt__http_read_header(alt_bufreader *br, alt_arena *a, size_t *budget, alt_http_header *h, bool strict);

enum { PRE_BUFFER = 2048, MAX_POST_HANDLER_READ = 256 << 10 };
enum { CONN_NEW, CONN_ACTIVE, CONN_IDLE, CONN_DONE };

#include <alt/task.h>
#include <alt/sync.h>
#include <stdatomic.h>

typedef struct alt_tls_conn alt_tls_conn;
struct alt__h2_stream;

typedef struct alt__http_conn {
	struct alt_http_server *srv;
	alt_conn *c;
	alt_tls_conn *tls;          /* HTTPS */
	alt_bufreader br;
	alt_bufwriter bw;
	alt_arena arena;
	_Atomic int state;
	struct alt__http_conn *prev, *next;
	char remote[80];
	bool hijacked;
	struct alt__h2_conn *h2;    /* serving HTTP/2 */
	bool werr; /* writing failed: the connection is done */
} conn;

struct alt_http_server {
	alt_http_server_config cfg;
	alt_sched **workers;
	int nworkers;
	_Atomic unsigned rr;
	alt_mutex mu;
	alt_listener *listeners[8];
	int nlisteners;
	conn *conns;
	_Atomic bool shutting_down, closed;
	alt_waitgroup active;
	alt_ctx *base;
	alt_addr addr;
	bool has_addr;
};

struct alt_http_rw {
	conn *hc;
	struct alt__h2_stream *h2;  /* the HTTP/2 stream of the response, NULL for HTTP/1 */
	alt_http_request *req;
	alt_http_header header;
	int status;
	bool wrote_header, sent, chunked, close_after, hijacked, finished;
	bool head, body_allowed;
	char *pre;
	size_t npre;
	int64_t declared;   /* Content-Length set by the handler, -1 none */
	uint64_t written;
	int err;
};

/* HTTP/2 (h2.c) */
struct alt__h2_conn;
void alt__h2_serve(conn *hc);
int alt__h2_send_headers(struct alt__h2_stream *s, int status, const alt_http_header *h, bool end);
int alt__h2_send_data(struct alt__h2_stream *s, const void *p, size_t n, bool end);
bool alt__h2_stream_ended(const struct alt__h2_stream *s);
void alt__http_rw_finish(alt_http_rw *w);
int alt__h2_client_new(struct alt__h2_conn **out, alt_conn *c, alt_tls_conn *tls, alt_sched *sched, const char *key);
const char *alt__h2_client_key(const struct alt__h2_conn *c);
bool alt__h2_client_usable(struct alt__h2_conn *c);
void alt__h2_client_release(struct alt__h2_conn *c);
int alt__h2_round_trip(struct alt__h2_conn *c, const char *method, const alt_url *u, const alt_http_header *extra, alt_reader *body,
	int64_t length, bool gzip, const char *ua, alt_arena *a, alt_http_response *resp, alt_ctx *ctx, struct alt__h2_stream **out,
	bool *retry);
void alt__h2_stream_done(struct alt__h2_stream *s);
bool alt__h2_stream_complete(const struct alt__h2_stream *s);
alt_time alt__h2_client_idle_since(struct alt__h2_conn *c, bool *idle);

/* the request's state */
static inline alt__http_request_state *alt__http_state(const alt_http_request *r) { return (alt__http_request_state *)r->internal; }

#endif
