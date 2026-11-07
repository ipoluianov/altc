/* AltC standard library: WebSocket (RFC 6455) with per-message
   compression (permessage-deflate, RFC 7692); the design follows
   gorilla/websocket.

   A server upgrades a request inside an HTTP handler:

	void handler(alt_http_rw *w, alt_http_request *r, void *arg) {
		static const alt_ws_upgrader u = {.enable_compression = true};
		alt_ws_conn *c;
		if (alt_ws_upgrade(&u, w, r, NULL, &c) < 0)
			return;  // the error response was sent
		int type;
		alt_buf msg = ALT_BUF_INIT;
		while (alt_ws_read_message(c, &type, &msg) == 0)
			alt_ws_write_message(c, type, msg.p, msg.n);
		alt_buf_free(&msg);
		alt_ws_close(c, ALT_WS_CLOSE_NORMAL, "", ALT_SEC);
	}

   A client dials a "ws://" or "wss://" URL with alt_ws_dial.

   Concurrency: a connection allows one reader and one writer at a time
   (they may be two different tasks or threads): reading calls
   (alt_ws_read_message, alt_ws_next_reader and its reader) from one,
   writing calls (alt_ws_write_message, alt_ws_next_writer, its writer,
   alt_ws_close_writer) from one. Control frames (alt_ws_write_control,
   alt_ws_write_close, alt_ws_ping) and the deadline setters may be
   called from any thread at any time, also while a message is being
   written: they go out between its frames. The handlers run in the
   reader. alt_ws_close and alt_ws_free must not run concurrently with
   other calls; to wake a blocked reader, set its read deadline (or send
   a close frame: the reader returns -ALT_ECLOSED when the peer answers).

   The connection fails (a close frame with the RFC's code is sent and
   reads return the error from then on) when the peer breaks the
   protocol: -EPROTO (1002: masking, reserved bits, opcodes, control
   frames, fragmentation, close codes, length encodings), -EILSEQ (1007:
   invalid UTF-8 in a text message or a close reason), -EMSGSIZE (1009:
   a message beyond the read limit), -ALT_EFLATE_CORRUPT (1007: bad
   compressed data). A close frame of the peer makes reads return
   -ALT_ECLOSED (alt_ws_close_code tells its code and reason); a
   connection that ends without one, -ALT_EUNEXPECTED_EOF. Errors of
   reads and writes are sticky: timeouts too (the frame being read or
   written may be cut).

   Compression: permessage-deflate with or without context takeover in
   either direction; the window is always 32 KiB (15 bits), so an offer
   asking the server to use a smaller one (server_max_window_bits < 15)
   is declined, and the client never offers client_max_window_bits.
   Not supported: other extensions, HTTP/2 (RFC 8441), proxies,
   credentials in the URL. */
#ifndef ALT_NET_WEBSOCKET_H
#define ALT_NET_WEBSOCKET_H

#include <alt/errors.h>
#include <alt/context.h>
#include <alt/crypto/tls.h>
#include <alt/io.h>
#include <alt/net.h>
#include <alt/net/http.h>
#include <alt/str.h>
#include <alt/time.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* message and frame types (opcodes) */
enum {
	ALT_WS_TEXT = 1,
	ALT_WS_BINARY = 2,
	ALT_WS_CLOSE = 8,
	ALT_WS_PING = 9,
	ALT_WS_PONG = 10,
};

/* close codes (RFC 6455 7.4.1 and the IANA registry) */
enum {
	ALT_WS_CLOSE_NORMAL = 1000,
	ALT_WS_CLOSE_GOING_AWAY = 1001,
	ALT_WS_CLOSE_PROTOCOL_ERROR = 1002,
	ALT_WS_CLOSE_UNSUPPORTED_DATA = 1003,
	ALT_WS_CLOSE_NO_STATUS = 1005,      /* never sent: a close frame without a code */
	ALT_WS_CLOSE_ABNORMAL = 1006,       /* never sent: the connection ended without a close frame */
	ALT_WS_CLOSE_INVALID_PAYLOAD = 1007,
	ALT_WS_CLOSE_POLICY_VIOLATION = 1008,
	ALT_WS_CLOSE_MESSAGE_TOO_BIG = 1009,
	ALT_WS_CLOSE_MANDATORY_EXTENSION = 1010,
	ALT_WS_CLOSE_INTERNAL_ERROR = 1011,
	ALT_WS_CLOSE_SERVICE_RESTART = 1012,
	ALT_WS_CLOSE_TRY_AGAIN_LATER = 1013,
	ALT_WS_CLOSE_BAD_GATEWAY = 1014,
	ALT_WS_CLOSE_TLS_HANDSHAKE = 1015,  /* never sent */
};

/* whether a close frame may carry the code: 1000-1003, 1007-1014,
   3000-4999 */
bool alt_ws_valid_close_code(int code);

typedef struct alt_ws_conn alt_ws_conn;

/* ---- the server ---- */

typedef struct {
	/* the subprotocols of the server in its order of preference: the
	   first one the client asks for too is chosen. NULL: the
	   Sec-WebSocket-Protocol of the response header, if any */
	const char *const *subprotocols;
	size_t nsubprotocols;
	/* whether the request's Origin is acceptable; NULL: same origin
	   (no Origin header, or one whose host equals the Host header,
	   ignoring case; as gorilla's) */
	bool (*check_origin)(const alt_http_request *r, void *arg);
	void *check_origin_arg;
	/* accept permessage-deflate when the client offers it */
	bool enable_compression;
	/* compress every message on its own, even when the client allows
	   context takeover (saves 300 KiB per connection between messages) */
	bool server_no_context_takeover;
	/* ask the client to compress every message on its own (our side then
	   keeps no 32 KiB history) */
	bool client_no_context_takeover;
	int compression_level;      /* 1-9; 0: 1 (best speed, as gorilla) */
	size_t read_limit;          /* the largest message received (after decompression); 0: none */
	size_t write_buffer_size;   /* the frame payload of streamed messages; 0: 4096 */
	alt_dur handshake_timeout;  /* for writing the response; 0: none */
	/* writes the response to a bad handshake; NULL: alt_http_error with
	   the status text and a Sec-WebSocket-Version: 13 header */
	void (*error)(alt_http_rw *w, alt_http_request *r, int status, const char *reason, void *arg);
	void *error_arg;
} alt_ws_upgrader;

/* upgrades the connection of the request (u NULL: the defaults). The
   checks and the answers to bad handshakes: no "upgrade" token in
   Connection or no "websocket" in Upgrade (400), a method other than GET
   (405), HTTP/1.0 (400), Sec-WebSocket-Version without 13 (426 with
   Sec-WebSocket-Version: 13), Sec-WebSocket-Extensions in response_header
   (500), an origin refused (403), a Sec-WebSocket-Key that is not 16
   bytes in base64 (400), a connection that cannot be taken over (HTTP/2:
   500). Then -ALT_EHTTP_BADREQUEST is returned (the response was sent);
   an error writing the 101 response is returned as it is (the
   connection is closed). response_header (may be NULL) goes into the 101
   response (cookies...).
   Afterwards the handler may return: the connection is the caller's
   until alt_ws_close or alt_ws_free; the request is not needed */
int alt_ws_upgrade(const alt_ws_upgrader *u, alt_http_rw *w, alt_http_request *r, const alt_http_header *response_header,
	alt_ws_conn **out);
/* whether the request asks for a WebSocket upgrade (Connection: upgrade
   and Upgrade: websocket) */
bool alt_ws_is_upgrade(const alt_http_request *r);
/* the subprotocols the client asks for (Sec-WebSocket-Protocol), at most
   max: their number */
size_t alt_ws_request_subprotocols(const alt_http_request *r, alt_str *out, size_t max);
/* Sec-WebSocket-Accept for a Sec-WebSocket-Key: 28 characters and a NUL */
void alt_ws_accept_key(alt_str key, char out[29]);

/* ---- the client ---- */

typedef struct {
	alt_http_header *header;    /* extra request headers (Origin, Cookie...; Host replaces the URL's); may be NULL */
	const char *const *subprotocols;
	size_t nsubprotocols;
	bool enable_compression;    /* offer permessage-deflate */
	/* offer it with server_no_context_takeover and
	   client_no_context_takeover (as gorilla's dialer does) */
	bool no_context_takeover;
	int compression_level;      /* 1-9; 0: 1 */
	/* wss: NULL is the system's roots; a configuration without
	   server_name verifies the URL's host */
	const alt_tls_config *tls;
	alt_dur handshake_timeout;  /* the connection, TLS and HTTP handshakes; 0: 45 s */
	size_t read_limit;          /* 0: none */
	size_t write_buffer_size;   /* 0: 4096 */
	size_t max_header_bytes;    /* of the response; 0: 1 MiB */
} alt_ws_dialer;

/* connects to a "ws://" or "wss://" URL (d NULL: the defaults) within
   ctx (may be NULL). *resp (when resp is not NULL) gets the handshake
   response, also when it is refused (with at most 1 KiB of its body):
   free it with alt_ws_response_free. Errors: those of the connection
   and TLS, -ETIMEDOUT after handshake_timeout, -ALT_EHTTP_SCHEME,
   -ALT_EURL_* for the URL, -EINVAL for request headers the dialer sets
   itself (Upgrade, Connection, Sec-WebSocket-*) or that are malformed,
   -ALT_EHTTP_BADRESPONSE when the server refuses or
   answers wrongly (a status other than 101, Upgrade, Connection,
   Sec-WebSocket-Accept, a subprotocol not offered, an extension not
   offered or with parameters not offered) */
int alt_ws_dial(const alt_ws_dialer *d, const char *url, alt_ctx *ctx, alt_ws_conn **out, alt_http_response **resp);
void alt_ws_response_free(alt_http_response *resp);

/* ---- messages ---- */

/* reads the next data message whole, reassembling its fragments: its
   type (ALT_WS_TEXT, ALT_WS_BINARY) in *type and its payload in msg
   (which is reset first). Control frames that come meanwhile go to the
   handlers */
int alt_ws_read_message(alt_ws_conn *c, int *type, alt_buf *msg);
/* starts reading the next data message (what is left of the previous
   one is skipped): *r reads its payload, 0 at its end; valid until the
   next call that reads */
int alt_ws_next_reader(alt_ws_conn *c, int *type, alt_reader *r);

/* sends a message in one frame (compressed when negotiated and
   enabled); a text message must be valid UTF-8 (else -EILSEQ). Types
   ALT_WS_PING, ALT_WS_PONG and ALT_WS_CLOSE go as alt_ws_write_control
   with the write deadline */
int alt_ws_write_message(alt_ws_conn *c, int type, const void *p, size_t n);
/* starts a message written in pieces through *w: sent in frames of
   write_buffer_size bytes, the last one by alt_ws_close_writer (a
   message still open is ended first). Text is not checked */
int alt_ws_next_writer(alt_ws_conn *c, int type, alt_writer *w);
/* ends the message of alt_ws_next_writer */
int alt_ws_close_writer(alt_ws_conn *c);

/* ---- control frames ---- */

/* sends a ping, pong or close frame (payload at most 125 bytes), waiting
   for the connection's writer no later than deadline (ALT_FOREVER:
   none); safe to call from any thread at any time */
int alt_ws_write_control(alt_ws_conn *c, int type, const void *p, size_t n, alt_time deadline);
static inline int alt_ws_ping(alt_ws_conn *c, const void *p, size_t n, alt_time deadline) {
	return alt_ws_write_control(c, ALT_WS_PING, p, n, deadline);
}
/* sends a close frame (code 0: without a code; else a valid code and a
   reason of at most 123 bytes of UTF-8, else -EINVAL); writes of data
   fail with -ALT_ECLOSED afterwards. Any thread */
int alt_ws_write_close(alt_ws_conn *c, int code, const char *reason, alt_time deadline);

/* the handlers, called by the reader (set them before reading). A
   nonzero result ends the read with that error (sticky, as all read
   errors). The defaults: a ping is answered with a pong of its payload,
   a pong is ignored, a close frame is answered with a close frame of
   its code. */
typedef int (*alt_ws_control_fn)(alt_ws_conn *c, alt_str payload, void *arg);
typedef int (*alt_ws_close_fn)(alt_ws_conn *c, int code, alt_str reason, void *arg);
void alt_ws_set_ping_handler(alt_ws_conn *c, alt_ws_control_fn fn, void *arg);
void alt_ws_set_pong_handler(alt_ws_conn *c, alt_ws_control_fn fn, void *arg);
void alt_ws_set_close_handler(alt_ws_conn *c, alt_ws_close_fn fn, void *arg);

/* the close code the peer sent (and its reason, valid while c is): 0
   while it sent none, ALT_WS_CLOSE_NO_STATUS for a close frame without
   a code, ALT_WS_CLOSE_ABNORMAL when the connection ended without one */
int alt_ws_close_code(const alt_ws_conn *c, alt_str *reason);
/* the code of the close frame sent (0: none sent; 1005 for one without
   a code) */
int alt_ws_sent_close_code(const alt_ws_conn *c);

/* the close handshake: sends a close frame (unless one was sent), reads
   (and drops data) until the peer's close frame comes, at most timeout
   (0: 5 s), a client then waits for the server to close the
   connection; then closes the connection and frees c. 0 when the
   handshake completed, else the error that ended it (c is freed
   anyway) */
int alt_ws_close(alt_ws_conn *c, int code, const char *reason, alt_dur timeout);
/* closes the connection at once and frees c */
void alt_ws_free(alt_ws_conn *c);

/* ---- settings and state ---- */

/* deadlines of the reads and writes (ALT_FOREVER: none); they affect
   operations already waiting */
int alt_ws_set_read_deadline(alt_ws_conn *c, alt_time t);
int alt_ws_set_write_deadline(alt_ws_conn *c, alt_time t);
/* the largest message to read (0: no limit) */
void alt_ws_set_read_limit(alt_ws_conn *c, size_t n);
/* whether messages written are compressed (when negotiated; on by
   default), and how much */
void alt_ws_enable_write_compression(alt_ws_conn *c, bool on);
int alt_ws_set_compression_level(alt_ws_conn *c, int level);

/* the subprotocol chosen ("" when none) */
const char *alt_ws_subprotocol(const alt_ws_conn *c);
/* the Sec-WebSocket-Extensions agreed ("" when none) */
const char *alt_ws_extensions(const alt_ws_conn *c);
bool alt_ws_compression_negotiated(const alt_ws_conn *c);
bool alt_ws_is_server(const alt_ws_conn *c);
alt_conn *alt_ws_net_conn(alt_ws_conn *c);
/* the TLS connection of wss:// (NULL otherwise) */
struct alt_tls_conn *alt_ws_tls_conn(alt_ws_conn *c);
const alt_addr *alt_ws_remote_addr(const alt_ws_conn *c);
const alt_addr *alt_ws_local_addr(const alt_ws_conn *c);

/* ---- for tests: a connection over any stream ---- */

typedef struct {
	bool server;
	bool compress;              /* permessage-deflate was negotiated */
	bool server_no_context_takeover, client_no_context_takeover;
	int compression_level;
	size_t read_limit, write_buffer_size;
} alt_ws_stream_config;

/* a WebSocket connection over r and w, as if the handshake had been
   made with cfg (NULL: a client without extensions); nc (may be NULL)
   gets the deadlines and is closed with c */
int alt_ws_new_stream(alt_ws_conn **out, alt_reader r, alt_writer w, alt_conn *nc, const alt_ws_stream_config *cfg);

#ifdef __cplusplus
}
#endif

#endif
