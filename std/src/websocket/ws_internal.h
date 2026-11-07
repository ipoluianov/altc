/* The websocket package's internal interface */
#ifndef ALT_WS_INTERNAL_H
#define ALT_WS_INTERNAL_H

#include <alt/compress/flate.h>
#include <alt/crypto/tls.h>
#include <alt/net/websocket.h>
#include <alt/sync.h>
#include <stdatomic.h>

enum { WS_HIST = 32768 };

struct alt_ws_conn {
	bool server;
	alt_conn *nc;               /* NULL for streams without one */
	alt_tls_conn *tls;
	alt_bufreader *br;          /* owned */
	alt_writer wr;
	char *subprotocol, *extensions, *server_name;
	bool compress;              /* permessage-deflate */
	bool wr_reset, rd_reset;    /* no context takeover of our compressor, of the peer's */
	int level;
	size_t frame_size;

	/* the reader */
	size_t read_limit;
	int rerr;                   /* sticky */
	int rop;                    /* the message being read (TEXT, BINARY), 0 before the first frame */
	bool rcomp, rfin, rdone, rmasked;
	uint64_t rleft;             /* payload bytes left in the frame */
	uint64_t rlen;              /* bytes of the message given out */
	uint64_t rdecl;             /* frame lengths of an uncompressed message */
	uint8_t rkey[4];
	unsigned rkpos;
	uint8_t u_need, u_lo, u_hi; /* UTF-8 validation across reads */
	alt_inflate *inf;
	alt_bufreader ibr;          /* the inflater's source: payload, then the tail */
	bool ibr_init;
	int tail_pos;               /* -1 while the payload goes */
	uint8_t *hist;              /* the last WS_HIST bytes of the peer's messages (context takeover) */
	size_t nhist;
	alt_ws_control_fn ping_fn, pong_fn;
	void *ping_arg, *pong_arg;
	alt_ws_close_fn close_fn;
	void *close_arg;
	int close_code;             /* the peer's */
	char close_reason[128];
	size_t nclose_reason;

	/* frames out */
	alt_mutex wmu;
	_Atomic int werr;           /* sticky */
	_Atomic bool close_sent;
	_Atomic int sent_code;
	_Atomic alt_time wdl;       /* the write deadline of data */
	uint8_t rnd[256];           /* random bytes for masking keys (under wmu) */
	unsigned nrnd;
	uint8_t *fbuf;              /* a frame being built (under wmu) */

	/* the message writer */
	int wtype;                  /* the message open, 0 none */
	bool wfirst, wcomp, wcomp_on, def_fresh;
	alt_buf wbuf;               /* payload not sent yet (compressed or not) */
	alt_deflate *def;
	int def_level;
};

/* a connection over a stream: the fields of the handshake are set by the caller */
int alt__ws_new(alt_ws_conn **out, bool server, alt_bufreader *br, alt_writer w, alt_conn *nc, alt_tls_conn *tls);

/* Sec-WebSocket-Extensions (RFC 6455 9.1) */
typedef struct {
	alt_str name, value;
	bool has_value;
} alt__ws_param;
typedef struct {
	alt_str name;
	alt__ws_param params[8];
	int nparams;
} alt__ws_ext;
/* all values of the header: the number of extensions (at most max), -1
   when malformed */
int alt__ws_parse_extensions(const alt_http_header *h, alt_arena *a, alt__ws_ext *out, int max);
/* a window bits value: 8..15 without leading zeros, else -1 */
int alt__ws_window_bits(alt_str v);

#endif
