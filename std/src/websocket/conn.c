/* WebSocket framing (RFC 6455 5-7) and per-message compression (RFC
   7692 6-7): reading and writing messages, control frames, the close
   handshake. Written from the RFCs; the API and the threading model
   follow gorilla/websocket. */
#include "ws_internal.h"
#include <alt/crypto/hash.h>
#include <alt/unicode/utf8.h>
#include <stdlib.h>
#include <string.h>

enum { FBUF = 16384, DIRECT = 8192 };

static const uint8_t TAIL[9] = {0x00, 0x00, 0xff, 0xff, 0x01, 0x00, 0x00, 0xff, 0xff};

bool alt_ws_valid_close_code(int code) {
	return (code >= 1000 && code <= 1003) || (code >= 1007 && code <= 1014) || (code >= 3000 && code <= 4999);
}

/* ---- masking ---- */

static void mask(uint8_t *p, size_t n, const uint8_t key[4], unsigned *pos) {
	unsigned k = *pos & 3;
	size_t i = 0;
	while (i < n && (k & 3)) {
		p[i++] ^= key[k & 3];
		k++;
	}
	if (n - i >= 8) {
		uint8_t kk[8];
		for (int j = 0; j < 8; j++)
			kk[j] = key[j & 3];
		uint64_t m;
		memcpy(&m, kk, 8);
		for (; n - i >= 8; i += 8) {
			uint64_t v;
			memcpy(&v, p + i, 8);
			v ^= m;
			memcpy(p + i, &v, 8);
		}
	}
	for (; i < n; i++, k++)
		p[i] ^= key[k & 3];
	*pos = k & 3;
}

/* ---- UTF-8 across pieces ---- */

static bool utf8_feed(alt_ws_conn *c, const uint8_t *p, size_t n) {
	uint8_t need = c->u_need, lo = c->u_lo, hi = c->u_hi;
	size_t i = 0;
	while (i < n) {
		if (need == 0) {
			/* ASCII 8 bytes at a time */
			while (n - i >= 8) {
				uint64_t v;
				memcpy(&v, p + i, 8);
				if (v & 0x8080808080808080ull)
					break;
				i += 8;
			}
			if (i == n)
				break;
			uint8_t b = p[i++];
			if (b < 0x80)
				continue;
			lo = 0x80, hi = 0xbf;
			if (b >= 0xc2 && b <= 0xdf)
				need = 1;
			else if (b == 0xe0)
				need = 2, lo = 0xa0;
			else if ((b >= 0xe1 && b <= 0xec) || b == 0xee || b == 0xef)
				need = 2;
			else if (b == 0xed)
				need = 2, hi = 0x9f;
			else if (b == 0xf0)
				need = 3, lo = 0x90;
			else if (b >= 0xf1 && b <= 0xf3)
				need = 3;
			else if (b == 0xf4)
				need = 3, hi = 0x8f;
			else
				return false;
		} else {
			uint8_t b = p[i++];
			if (b < lo || b > hi)
				return false;
			need--;
			lo = 0x80, hi = 0xbf;
		}
	}
	c->u_need = need, c->u_lo = lo, c->u_hi = hi;
	return true;
}

/* ---- writing frames ---- */

static int lock_w(alt_ws_conn *c, alt_time dl) {
	if (dl == ALT_FOREVER) {
		alt_mutex_lock(&c->wmu);
		return 0;
	}
	return alt_mutex_lock_until(&c->wmu, dl);
}

static int put(alt_ws_conn *c, const void *p, size_t n) {
	ssize_t k = alt_write_all(c->wr, p, n);
	return k < 0 ? (int)k : 0;
}

/* one frame; the caller holds wmu */
static int put_frame(alt_ws_conn *c, int b0, const void *payload, size_t n, alt_time dl) {
	int rc = atomic_load(&c->werr);
	if (rc)
		return rc;
	if (atomic_load(&c->close_sent))
		return -ALT_ECLOSED;
	if (c->nc)
		alt_conn_set_write_deadline(c->nc, dl);
	if (!c->fbuf && !(c->fbuf = malloc(FBUF + 14)))
		return -ENOMEM;
	uint8_t *h = c->fbuf;
	size_t hn = 2;
	h[0] = (uint8_t)b0;
	if (n < 126)
		h[1] = (uint8_t)n;
	else if (n <= 0xffff) {
		h[1] = 126;
		h[2] = (uint8_t)(n >> 8), h[3] = (uint8_t)n;
		hn = 4;
	} else {
		h[1] = 127;
		for (int i = 0; i < 8; i++)
			h[2 + i] = (uint8_t)((uint64_t)n >> (56 - 8 * i));
		hn = 10;
	}
	const uint8_t *p = payload;
	if (c->server) {
		if (n > DIRECT) {
			if ((rc = put(c, h, hn)) == 0)
				rc = put(c, p, n);
		} else {
			if (n)
				memcpy(h + hn, p, n);
			rc = put(c, h, hn + n);
		}
	} else {
		h[1] |= 0x80;
		if (c->nrnd < 4) {
			if ((rc = alt_crypto_rand(c->rnd, sizeof c->rnd)) < 0) {
				atomic_store(&c->werr, rc);
				return rc;
			}
			c->nrnd = sizeof c->rnd;
		}
		c->nrnd -= 4;
		uint8_t key[4];
		memcpy(key, c->rnd + c->nrnd, 4);
		memcpy(h + hn, key, 4);
		hn += 4;
		unsigned pos = 0;
		size_t off = 0, room = FBUF + 14 - hn;
		do {
			size_t k = n - off < room ? n - off : room;
			uint8_t *dst = h + hn;
			if (k)
				memcpy(dst, p + off, k);
			mask(dst, k, key, &pos);
			rc = put(c, h, hn + k);
			off += k;
			hn = 0;
			room = FBUF + 14;
		} while (rc == 0 && off < n);
	}
	if (rc)
		atomic_store(&c->werr, rc);
	else if ((b0 & 0x0f) == ALT_WS_CLOSE)
		atomic_store(&c->close_sent, true);
	return rc;
}

static int valid_close_payload(const uint8_t *p, size_t n) {
	if (n == 0)
		return 1005;
	if (n == 1)
		return 0;
	int code = p[0] << 8 | p[1];
	if (!alt_ws_valid_close_code(code) || !alt_utf8_valid((const char *)p + 2, n - 2))
		return 0;
	return code;
}

int alt_ws_write_control(alt_ws_conn *c, int type, const void *p, size_t n, alt_time deadline) {
	if ((type != ALT_WS_CLOSE && type != ALT_WS_PING && type != ALT_WS_PONG) || n > 125 || (n && !p))
		return -EINVAL;
	int code = 0;
	if (type == ALT_WS_CLOSE && !(code = valid_close_payload(p, n)))
		return -EINVAL;
	int rc = lock_w(c, deadline);
	if (rc)
		return rc;
	rc = put_frame(c, 0x80 | type, p, n, deadline);
	if (rc == 0 && type == ALT_WS_CLOSE)
		atomic_store(&c->sent_code, code);
	alt_mutex_unlock(&c->wmu);
	return rc;
}

int alt_ws_write_close(alt_ws_conn *c, int code, const char *reason, alt_time deadline) {
	uint8_t p[125];
	size_t n = 0;
	size_t rl = reason ? strlen(reason) : 0;
	if (code) {
		if (!alt_ws_valid_close_code(code) || rl > 123 || !alt_utf8_valid(reason ? reason : "", rl))
			return -EINVAL;
		p[0] = (uint8_t)(code >> 8), p[1] = (uint8_t)code;
		if (rl)
			memcpy(p + 2, reason, rl);
		n = 2 + rl;
	} else if (rl)
		return -EINVAL;
	return alt_ws_write_control(c, ALT_WS_CLOSE, p, n, deadline);
}

/* ---- compression of messages written ---- */

static ssize_t sink_write(void *obj, const void *p, size_t n) {
	alt_ws_conn *c = obj;
	return alt_buf_append(&c->wbuf, p, n) < 0 ? -ENOMEM : (ssize_t)n;
}

/* a compressor ready for a message */
static int def_start(alt_ws_conn *c) {
	alt_writer sink = {sink_write, c};
	if (!c->def || c->def_level != c->level) {
		alt_deflate_free(c->def);
		c->def = 0;
		int rc = alt_deflate_new(&c->def, sink, c->level);
		if (rc < 0)
			return rc;
		c->def_level = c->level;
		c->def_fresh = true;
	}
	if (c->wr_reset && !c->def_fresh)
		alt_deflate_reset(c->def, sink);
	c->def_fresh = false;
	return 0;
}

/* ends the compressed data of a message: wbuf ends with 00 00 ff ff */
static int def_end(alt_ws_conn *c) {
	int rc = alt_deflate_flush(c->def);
	if (rc < 0)
		return rc;
	if (c->wbuf.err)
		return c->wbuf.err;
	if (c->wbuf.n < 4 || memcmp(c->wbuf.p + c->wbuf.n - 4, TAIL, 4))
		return -EIO;
	return 0;
}

static int data_frame(alt_ws_conn *c, int b0, const void *p, size_t n) {
	alt_time dl = atomic_load(&c->wdl);
	int rc = lock_w(c, dl);
	if (rc)
		return rc;
	rc = put_frame(c, b0, p, n, dl);
	alt_mutex_unlock(&c->wmu);
	return rc;
}

int alt_ws_write_message(alt_ws_conn *c, int type, const void *p, size_t n) {
	if (type == ALT_WS_CLOSE || type == ALT_WS_PING || type == ALT_WS_PONG)
		return alt_ws_write_control(c, type, p, n, atomic_load(&c->wdl));
	if ((type != ALT_WS_TEXT && type != ALT_WS_BINARY) || (n && !p))
		return -EINVAL;
	if (type == ALT_WS_TEXT && !alt_utf8_valid(p, n))
		return -EILSEQ;
	int rc;
	if (c->wtype && (rc = alt_ws_close_writer(c)) < 0)
		return rc;
	if ((rc = atomic_load(&c->werr)))
		return rc;
	if (atomic_load(&c->close_sent))
		return -ALT_ECLOSED;
	if (!c->compress || !c->wcomp_on)
		return data_frame(c, 0x80 | type, p, n);
	alt_buf_reset(&c->wbuf);
	if ((rc = def_start(c)) < 0)
		return rc;
	if (n && alt_deflate_write(c->def, p, n) < 0)
		return -ENOMEM;
	if ((rc = def_end(c)) < 0)
		return rc;
	rc = data_frame(c, 0x80 | 0x40 | type, c->wbuf.p, c->wbuf.n - 4);
	alt_buf_reset(&c->wbuf);
	return rc;
}

/* ---- the message writer ---- */

static int send_part(alt_ws_conn *c, const void *p, size_t n, bool fin) {
	int b0 = fin ? 0x80 : 0;
	if (c->wfirst)
		b0 |= c->wtype | (c->wcomp ? 0x40 : 0);
	c->wfirst = false;
	return data_frame(c, b0, p, n);
}

static ssize_t mw_write(void *obj, const void *p, size_t n) {
	alt_ws_conn *c = obj;
	if (!c->wtype)
		return -EINVAL;
	int rc = atomic_load(&c->werr);
	if (rc)
		return rc;
	if (n == 0)
		return 0;
	size_t fs = c->frame_size;
	if (c->wcomp) {
		if (alt_deflate_write(c->def, p, n) < 0 || c->wbuf.err)
			return -ENOMEM;
		/* the last 4 bytes stay: the flush at the end makes them the tail */
		size_t off = 0;
		while (c->wbuf.n - off >= fs + 4) {
			if ((rc = send_part(c, c->wbuf.p + off, fs, false)) < 0)
				return rc;
			off += fs;
		}
		alt_buf_consume(&c->wbuf, off);
		return (ssize_t)n;
	}
	const char *s = p;
	size_t left = n;
	while (left) {
		if (c->wbuf.n == fs) {
			if ((rc = send_part(c, c->wbuf.p, fs, false)) < 0)
				return rc;
			alt_buf_reset(&c->wbuf);
		}
		if (c->wbuf.n == 0 && left > fs) {
			/* whole frames straight from the caller's bytes */
			if ((rc = send_part(c, s, fs, false)) < 0)
				return rc;
			s += fs, left -= fs;
			continue;
		}
		size_t k = fs - c->wbuf.n < left ? fs - c->wbuf.n : left;
		if (alt_buf_append(&c->wbuf, s, k) < 0)
			return -ENOMEM;
		s += k, left -= k;
	}
	return (ssize_t)n;
}

int alt_ws_next_writer(alt_ws_conn *c, int type, alt_writer *w) {
	if (type != ALT_WS_TEXT && type != ALT_WS_BINARY)
		return -EINVAL;
	int rc;
	if (c->wtype && (rc = alt_ws_close_writer(c)) < 0)
		return rc;
	if ((rc = atomic_load(&c->werr)))
		return rc;
	if (atomic_load(&c->close_sent))
		return -ALT_ECLOSED;
	alt_buf_reset(&c->wbuf);
	c->wcomp = c->compress && c->wcomp_on;
	if (c->wcomp && (rc = def_start(c)) < 0)
		return rc;
	c->wtype = type;
	c->wfirst = true;
	*w = (alt_writer){mw_write, c};
	return 0;
}

int alt_ws_close_writer(alt_ws_conn *c) {
	if (!c->wtype)
		return 0;
	int rc = 0;
	if (c->wcomp && (rc = def_end(c)) == 0)
		rc = send_part(c, c->wbuf.p, c->wbuf.n - 4, true);
	else if (!c->wcomp)
		rc = send_part(c, c->wbuf.p, c->wbuf.n, true);
	c->wtype = 0;
	alt_buf_reset(&c->wbuf);
	return rc;
}

/* ---- reading frames ---- */

static int rd_err(alt_ws_conn *c, int rc) {
	if (c->rerr)
		return c->rerr;
	if (rc == -ALT_EEOF || rc == 0)
		rc = -ALT_EUNEXPECTED_EOF;
	if (rc == -ALT_EUNEXPECTED_EOF && !c->close_code)
		c->close_code = ALT_WS_CLOSE_ABNORMAL;
	return c->rerr = rc;
}

/* fails the connection: a close frame with the code goes out */
static int fail(alt_ws_conn *c, int code, int err, const char *why) {
	if (c->rerr)
		return c->rerr;
	c->rerr = err;
	if (!atomic_load(&c->close_sent))
		alt_ws_write_close(c, code, why, alt_deadline(ALT_SEC));
	return err;
}

static int read_full(alt_ws_conn *c, void *p, size_t n) {
	size_t got = 0;
	while (got < n) {
		ssize_t k = alt_bufreader_read(c->br, (char *)p + got, n - got);
		if (k <= 0)
			return rd_err(c, (int)k);
		got += (size_t)k;
	}
	return 0;
}

static int control(alt_ws_conn *c, int op, uint8_t *p, size_t n) {
	alt_str s = alt_str_make(p, n);
	if (op == ALT_WS_PING || op == ALT_WS_PONG) {
		int rc = 0;
		if (op == ALT_WS_PING && c->ping_fn)
			rc = c->ping_fn(c, s, c->ping_arg);
		else if (op == ALT_WS_PING)
			alt_ws_write_control(c, ALT_WS_PONG, p, n, alt_deadline(5 * ALT_SEC)); /* a failing writer shows itself to the writer */
		else if (c->pong_fn)
			rc = c->pong_fn(c, s, c->pong_arg);
		if (rc && !c->rerr)
			c->rerr = rc; /* sticky: the inflater of a message may have seen it */
		return rc;
	}
	/* close */
	int code = ALT_WS_CLOSE_NO_STATUS;
	alt_str reason = ALT_STR_NULL;
	if (n == 1)
		return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "close frame of one byte");
	if (n >= 2) {
		code = p[0] << 8 | p[1];
		if (!alt_ws_valid_close_code(code))
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "invalid close code");
		if (!alt_utf8_valid((const char *)p + 2, n - 2))
			return fail(c, ALT_WS_CLOSE_INVALID_PAYLOAD, -EILSEQ, "invalid UTF-8 in the close reason");
		memcpy(c->close_reason, p + 2, n - 2);
		c->nclose_reason = n - 2;
		reason = alt_str_make(c->close_reason, n - 2);
	}
	c->close_code = code;
	c->rerr = -ALT_ECLOSED;
	int rc = 0;
	if (c->close_fn)
		rc = c->close_fn(c, code, reason, c->close_arg);
	else if (!atomic_load(&c->close_sent))
		alt_ws_write_close(c, code == ALT_WS_CLOSE_NO_STATUS ? 0 : code, "", alt_deadline(5 * ALT_SEC));
	if (rc)
		c->rerr = rc;
	return c->rerr;
}

/* reads frames up to the next data frame's payload, handling control
   frames on the way: a message's first frame when none is being read,
   else its next one */
static int next_frame(alt_ws_conn *c) {
	for (;;) {
		uint8_t h[8];
		int rc = read_full(c, h, 2);
		if (rc)
			return rc;
		bool fin = h[0] & 0x80, masked = h[1] & 0x80;
		int rsv = h[0] & 0x70, op = h[0] & 0x0f;
		uint64_t len = h[1] & 0x7f;
		bool ctl = op & 8;
		if (rsv & 0x30)
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "reserved bits set");
		if (op != 0 && op != 1 && op != 2 && op != 8 && op != 9 && op != 10)
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "unknown opcode");
		if ((rsv & 0x40) && (!c->compress || ctl || op == 0))
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "RSV1 set");
		if (ctl) {
			if (!fin)
				return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "fragmented control frame");
			if (len > 125)
				return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "control frame too long");
		} else if (op == 0 && !c->rop)
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "continuation frame without a message");
		else if (op != 0 && c->rop)
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "new message within a fragmented one");
		if (masked != c->server)
			return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, c->server ? "frame not masked" : "masked frame");
		if (len == 126) {
			if ((rc = read_full(c, h, 2)))
				return rc;
			len = (uint64_t)h[0] << 8 | h[1];
			if (len < 126)
				return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "length not in its shortest form");
		} else if (len == 127) {
			if ((rc = read_full(c, h, 8)))
				return rc;
			len = 0;
			for (int i = 0; i < 8; i++)
				len = len << 8 | h[i];
			if (len >> 63)
				return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "length with the most significant bit set");
			if (len <= 0xffff)
				return fail(c, ALT_WS_CLOSE_PROTOCOL_ERROR, -EPROTO, "length not in its shortest form");
		}
		uint8_t key[4] = {0};
		if (masked && (rc = read_full(c, key, 4)))
			return rc;
		if (ctl) {
			uint8_t p[125];
			if ((rc = read_full(c, p, (size_t)len)))
				return rc;
			unsigned pos = 0;
			if (masked)
				mask(p, (size_t)len, key, &pos);
			if ((rc = control(c, op, p, (size_t)len)))
				return rc;
			continue;
		}
		if (op != 0) {
			c->rop = op;
			c->rcomp = rsv & 0x40;
		}
		if (!c->rcomp && c->read_limit) {
			if (len > c->read_limit || c->rdecl + len > c->read_limit)
				return fail(c, ALT_WS_CLOSE_MESSAGE_TOO_BIG, -EMSGSIZE, "message too big");
			c->rdecl += len;
		}
		c->rleft = len;
		c->rfin = fin;
		c->rmasked = masked;
		memcpy(c->rkey, key, 4);
		c->rkpos = 0;
		return 0;
	}
}

/* the payload of the message as it is on the wire; 0 at its end */
static ssize_t raw_read(alt_ws_conn *c, void *buf, size_t n) {
	while (c->rleft == 0) {
		if (c->rfin)
			return 0;
		int rc = next_frame(c);
		if (rc)
			return rc;
	}
	if (n > c->rleft)
		n = (size_t)c->rleft;
	ssize_t k = alt_bufreader_read(c->br, buf, n);
	if (k <= 0)
		return rd_err(c, (int)k);
	if (c->rmasked)
		mask(buf, (size_t)k, c->rkey, &c->rkpos);
	c->rleft -= (uint64_t)k;
	return k;
}

/* the inflater's source: the payload, then the sync flush tail and an
   empty final block (so that the stream ends) */
static ssize_t comp_src(void *obj, void *buf, size_t n) {
	alt_ws_conn *c = obj;
	if (c->tail_pos < 0) {
		ssize_t k = raw_read(c, buf, n);
		if (k != 0)
			return k;
		c->tail_pos = 0;
	}
	size_t k = sizeof TAIL - (size_t)c->tail_pos;
	if (k > n)
		k = n;
	memcpy(buf, TAIL + c->tail_pos, k);
	c->tail_pos += (int)k;
	return (ssize_t)k;
}

static void hist_add(alt_ws_conn *c, const uint8_t *p, size_t n) {
	if (n >= WS_HIST) {
		memcpy(c->hist, p + n - WS_HIST, WS_HIST);
		c->nhist = WS_HIST;
		return;
	}
	if (c->nhist + n > WS_HIST) {
		size_t drop = c->nhist + n - WS_HIST;
		memmove(c->hist, c->hist + drop, c->nhist - drop);
		c->nhist -= drop;
	}
	memcpy(c->hist + c->nhist, p, n);
	c->nhist += n;
}

static ssize_t msg_read(alt_ws_conn *c, void *buf, size_t n) {
	if (c->rerr)
		return c->rerr;
	if (!c->rop)
		return -EINVAL;
	if (c->rdone || n == 0)
		return 0;
	ssize_t k;
	if (c->rcomp) {
		if (c->read_limit && n > c->read_limit - c->rlen + 1)
			n = (size_t)(c->read_limit - c->rlen + 1);
		k = alt_inflate_read(c->inf, buf, n);
		if (k < 0)
			return c->rerr ? c->rerr : fail(c, ALT_WS_CLOSE_INVALID_PAYLOAD, -ALT_EFLATE_CORRUPT, "invalid compressed data");
		if (k == 0 && c->tail_pos < 0) {
			/* the data ended the stream itself: skip what follows */
			uint8_t tmp[512];
			ssize_t d;
			while ((d = raw_read(c, tmp, sizeof tmp)) > 0) {
			}
			if (d < 0)
				return d;
		}
	} else if ((k = raw_read(c, buf, n)) < 0)
		return k;
	if (k > 0) {
		c->rlen += (uint64_t)k;
		if (c->read_limit && c->rlen > c->read_limit)
			return fail(c, ALT_WS_CLOSE_MESSAGE_TOO_BIG, -EMSGSIZE, "message too big");
		if (c->rop == ALT_WS_TEXT && !utf8_feed(c, buf, (size_t)k))
			return fail(c, ALT_WS_CLOSE_INVALID_PAYLOAD, -EILSEQ, "invalid UTF-8 in a text message");
		if (c->rcomp && !c->rd_reset)
			hist_add(c, buf, (size_t)k);
		return k;
	}
	if (c->rop == ALT_WS_TEXT && c->u_need)
		return fail(c, ALT_WS_CLOSE_INVALID_PAYLOAD, -EILSEQ, "invalid UTF-8 in a text message");
	c->rdone = true;
	return 0;
}

static ssize_t reader_read(void *obj, void *buf, size_t n) { return msg_read(obj, buf, n); }

static int begin(alt_ws_conn *c, int *type) {
	if (c->rerr)
		return c->rerr;
	if (c->rop && !c->rdone) {
		uint8_t tmp[4096];
		ssize_t k;
		while ((k = msg_read(c, tmp, sizeof tmp)) > 0) {
		}
		if (k < 0)
			return (int)k;
	}
	c->rop = 0;
	c->rcomp = c->rdone = false;
	c->rlen = c->rdecl = 0;
	c->u_need = 0;
	int rc = next_frame(c);
	if (rc)
		return rc;
	if (c->rcomp) {
		if (!c->ibr_init) {
			if ((rc = alt_bufreader_init(&c->ibr, (alt_reader){comp_src, c}, 4096)) < 0)
				return c->rerr = rc;
			c->ibr_init = true;
		} else
			alt_bufreader_reset(&c->ibr, (alt_reader){comp_src, c});
		if (!c->inf && (rc = alt_inflate_new_buf(&c->inf, &c->ibr)) < 0)
			return c->rerr = rc;
		alt_inflate_reset(c->inf, &c->ibr);
		if (!c->rd_reset) {
			if (!c->hist && !(c->hist = malloc(WS_HIST)))
				return c->rerr = -ENOMEM;
			if (c->nhist)
				alt_inflate_set_dict(c->inf, c->hist, c->nhist);
		}
		c->tail_pos = -1;
	}
	*type = c->rop;
	return 0;
}

int alt_ws_next_reader(alt_ws_conn *c, int *type, alt_reader *r) {
	int rc = begin(c, type);
	if (rc)
		return rc;
	*r = (alt_reader){reader_read, c};
	return 0;
}

int alt_ws_read_message(alt_ws_conn *c, int *type, alt_buf *msg) {
	alt_buf_reset(msg);
	int rc = begin(c, type);
	if (rc)
		return rc;
	for (;;) {
		size_t want = 4096;
		if (!c->rcomp && c->rleft > want)
			want = c->rleft < (1u << 20) ? (size_t)c->rleft : (1u << 20);
		else if (msg->n > want)
			want = msg->n;
		if (alt_buf_reserve(msg, want) < 0)
			return -ENOMEM;
		ssize_t k = msg_read(c, msg->p + msg->n, msg->cap - msg->n - 1);
		if (k < 0)
			return (int)k;
		if (k == 0)
			break;
		msg->n += (size_t)k;
		msg->p[msg->n] = 0;
	}
	return 0;
}

/* ---- handlers, state ---- */

void alt_ws_set_ping_handler(alt_ws_conn *c, alt_ws_control_fn fn, void *arg) { c->ping_fn = fn, c->ping_arg = arg; }
void alt_ws_set_pong_handler(alt_ws_conn *c, alt_ws_control_fn fn, void *arg) { c->pong_fn = fn, c->pong_arg = arg; }
void alt_ws_set_close_handler(alt_ws_conn *c, alt_ws_close_fn fn, void *arg) { c->close_fn = fn, c->close_arg = arg; }

int alt_ws_close_code(const alt_ws_conn *c, alt_str *reason) {
	if (reason)
		*reason = alt_str_make(c->close_reason, c->nclose_reason);
	return c->close_code;
}

int alt_ws_sent_close_code(const alt_ws_conn *c) { return atomic_load(&((alt_ws_conn *)c)->sent_code); }

int alt_ws_set_read_deadline(alt_ws_conn *c, alt_time t) { return c->nc ? alt_conn_set_read_deadline(c->nc, t) : 0; }

int alt_ws_set_write_deadline(alt_ws_conn *c, alt_time t) {
	atomic_store(&c->wdl, t);
	return c->nc ? alt_conn_set_write_deadline(c->nc, t) : 0;
}

void alt_ws_set_read_limit(alt_ws_conn *c, size_t n) { c->read_limit = n; }
void alt_ws_enable_write_compression(alt_ws_conn *c, bool on) { c->wcomp_on = on; }

int alt_ws_set_compression_level(alt_ws_conn *c, int level) {
	if (level < ALT_FLATE_HUFFMAN_ONLY || level > ALT_FLATE_BEST_COMPRESSION)
		return -EINVAL;
	c->level = level;
	return 0;
}

const char *alt_ws_subprotocol(const alt_ws_conn *c) { return c->subprotocol ? c->subprotocol : ""; }
const char *alt_ws_extensions(const alt_ws_conn *c) { return c->extensions ? c->extensions : ""; }
bool alt_ws_compression_negotiated(const alt_ws_conn *c) { return c->compress; }
bool alt_ws_is_server(const alt_ws_conn *c) { return c->server; }
alt_conn *alt_ws_net_conn(alt_ws_conn *c) { return c->nc; }
struct alt_tls_conn *alt_ws_tls_conn(alt_ws_conn *c) { return c->tls; }
const alt_addr *alt_ws_remote_addr(const alt_ws_conn *c) { return c->nc ? alt_conn_remote_addr(c->nc) : 0; }
const alt_addr *alt_ws_local_addr(const alt_ws_conn *c) { return c->nc ? alt_conn_local_addr(c->nc) : 0; }

/* ---- life ---- */

int alt__ws_new(alt_ws_conn **out, bool server, alt_bufreader *br, alt_writer w, alt_conn *nc, alt_tls_conn *tls) {
	alt_ws_conn *c = calloc(1, sizeof *c);
	if (!c)
		return -ENOMEM;
	c->server = server;
	c->br = br;
	c->wr = w;
	c->nc = nc;
	c->tls = tls;
	c->level = ALT_FLATE_BEST_SPEED;
	c->frame_size = 4096;
	c->wcomp_on = true;
	c->wmu = (alt_mutex)ALT_MUTEX_INIT;
	atomic_init(&c->wdl, ALT_FOREVER);
	*out = c;
	return 0;
}

int alt_ws_new_stream(alt_ws_conn **out, alt_reader r, alt_writer w, alt_conn *nc, const alt_ws_stream_config *cfg) {
	alt_bufreader *br = malloc(sizeof *br);
	if (!br)
		return -ENOMEM;
	int rc = alt_bufreader_init(br, r, 0);
	if (rc < 0) {
		free(br);
		return rc;
	}
	alt_ws_conn *c;
	if ((rc = alt__ws_new(&c, cfg && cfg->server, br, w, nc, 0)) < 0) {
		alt_bufreader_free(br);
		free(br);
		return rc;
	}
	if (cfg) {
		c->compress = cfg->compress;
		c->wr_reset = cfg->server ? cfg->server_no_context_takeover : cfg->client_no_context_takeover;
		c->rd_reset = cfg->server ? cfg->client_no_context_takeover : cfg->server_no_context_takeover;
		if (cfg->compression_level)
			c->level = cfg->compression_level;
		c->read_limit = cfg->read_limit;
		if (cfg->write_buffer_size)
			c->frame_size = cfg->write_buffer_size;
	}
	*out = c;
	return 0;
}

void alt_ws_free(alt_ws_conn *c) {
	if (!c)
		return;
	if (c->tls) {
		if (c->nc)
			alt_conn_set_deadline(c->nc, alt_deadline(ALT_SEC));
		alt_tls_close(c->tls); /* closes nc */
	} else if (c->nc)
		alt_conn_close(c->nc);
	if (c->br) {
		alt_bufreader_free(c->br);
		free(c->br);
	}
	if (c->ibr_init)
		alt_bufreader_free(&c->ibr);
	alt_inflate_free(c->inf);
	alt_deflate_free(c->def);
	alt_buf_free(&c->wbuf);
	free(c->hist);
	free(c->fbuf);
	free(c->subprotocol);
	free(c->extensions);
	free(c->server_name);
	free(c);
}

int alt_ws_close(alt_ws_conn *c, int code, const char *reason, alt_dur timeout) {
	alt_time dl = alt_deadline(timeout > 0 ? timeout : 5 * ALT_SEC);
	int rc = 0;
	if (!atomic_load(&c->close_sent)) {
		rc = alt_ws_write_close(c, code, reason, dl);
		if (rc == -EINVAL)
			rc = alt_ws_write_close(c, ALT_WS_CLOSE_NORMAL, "", dl);
	}
	if (rc == 0 && !c->rerr) {
		alt_ws_set_read_deadline(c, dl);
		int type;
		while ((rc = begin(c, &type)) == 0) {
		}
	}
	bool done = c->rerr == -ALT_ECLOSED || (c->close_code && c->close_code != ALT_WS_CLOSE_ABNORMAL);
	if (done) {
		rc = 0;
		if (!c->server && c->nc) {
			/* the server closes the TCP connection first (RFC 6455 7.1.1) */
			alt_conn_set_read_deadline(c->nc, dl);
			uint8_t tmp[256];
			while (alt_bufreader_read(c->br, tmp, sizeof tmp) > 0) {
			}
		}
	} else if (rc == 0)
		rc = c->rerr ? c->rerr : -ALT_EUNEXPECTED_EOF;
	alt_ws_free(c);
	return rc;
}
