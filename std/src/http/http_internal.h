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

/* the request's state */
static inline alt__http_request_state *alt__http_state(const alt_http_request *r) { return (alt__http_request_state *)r->internal; }

#endif
