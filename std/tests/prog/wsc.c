/* A scripted WebSocket client of the library for
   tests/stdwebsocket_test.go: one command per line of stdin (fields
   separated by tabs), one or more lines of output each.
	dial URL [proto=a,b] [compress] [nct] [hdr=K:V] [roots=F] [limit=N]
	         [frame=N] [level=N] [timeout=MS]
	                        "dial ok status=101 proto=P ext=E x-ws=V" or
	                        "dial err NAME status=N body=B"
	echo TYPE SIZE SEED [chunk=N]
	                        sends a message (streamed in chunks of N bytes
	                        with chunk=), reads one: "echo ok" or why not
	echos [MAX]             echo of every size of a list (up to MAX), both
	                        types, whole and streamed: "echos ok N"
	send TYPE HEX           a message or control frame (text, binary, ping,
	                        pong, close): "sent" or "send err NAME"
	recv                    "msg TYPE LEN HEX-PREFIX FNV64" or "err ..."
	readall                 recv until an error
	ping PAYLOAD            a ping, then an echo of "sync": "pong PAYLOAD..."
	writeclose CODE REASON  "writeclose NAME"
	close CODE REASON       the close handshake: "close NAME peer=CODE REASON"
	deadline MS             the read deadline from now
	free                    drops the connection
   errors: "err NAME code=PEER-CODE reason=R sent=SENT-CODE" */
#include <alt/crypto/tls.h>
#include <alt/crypto/x509.h>
#include <alt/net/http.h>
#include <alt/net/websocket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *errname(int rc) {
	static char buf[32];
	switch (rc) {
	case 0: return "ok";
	case -EPROTO: return "EPROTO";
	case -EILSEQ: return "EILSEQ";
	case -EMSGSIZE: return "EMSGSIZE";
	case -ETIMEDOUT: return "ETIMEDOUT";
	case -EINVAL: return "EINVAL";
	case -ECONNREFUSED: return "ECONNREFUSED";
	case -ECONNRESET: return "ECONNRESET";
	case -EPIPE: return "EPIPE";
	case -ALT_ECLOSED: return "ECLOSED";
	case -ALT_EUNEXPECTED_EOF: return "EUNEXPECTED_EOF";
	case -ALT_EFLATE_CORRUPT: return "ECORRUPT";
	case -ALT_EHTTP_BADRESPONSE: return "EBADRESPONSE";
	case -ALT_EHTTP_SCHEME: return "ESCHEME";
	case -ALT_ETLS_CERT: return "ETLSCERT";
	case -ALT_ETLS_HANDSHAKE: return "ETLSHANDSHAKE";
	}
	snprintf(buf, sizeof buf, "E%d", rc);
	return buf;
}

static alt_ws_conn *c;
static char pongs[1024];

static void err_line(int rc) {
	alt_str reason = ALT_STR_NULL;
	int code = c ? alt_ws_close_code(c, &reason) : 0;
	printf("err %s code=%d reason=%.*s sent=%d\n", errname(rc), code, (int)reason.n, reason.p, c ? alt_ws_sent_close_code(c) : 0);
}

static int on_pong(alt_ws_conn *cc, alt_str p, void *arg) {
	(void)cc, (void)arg;
	size_t n = strlen(pongs);
	snprintf(pongs + n, sizeof pongs - n, "%s%.*s", n ? "," : "", (int)p.n, p.p);
	return 0;
}

static uint64_t fnv(const char *p, size_t n) {
	uint64_t h = 14695981039346656037ull;
	for (size_t i = 0; i < n; i++)
		h = (h ^ (uint8_t)p[i]) * 1099511628211ull;
	return h;
}

/* text: valid UTF-8 of exactly n bytes */
static void gen(alt_buf *b, size_t n, bool text, uint64_t seed) {
	static const char *const words[] = {"web", "socket ", "frame ", "π ", "deflate ", "日本 ", "context ", "\n", "€"};
	uint64_t x = seed * 2654435761u + 1;
	alt_buf_reset(b);
	alt_buf_reserve(b, n);
	while (b->n < n) {
		x ^= x << 13, x ^= x >> 7, x ^= x << 17;
		if (!text) {
			alt_buf_append_byte(b, (int)(x & 0xff));
			continue;
		}
		const char *w = words[x % 9];
		size_t k = strlen(w);
		if (b->n + k > n) {
			while (b->n < n)
				alt_buf_append_byte(b, 'x');
			break;
		}
		alt_buf_append(b, w, k);
	}
}

static int type_of(const char *s) {
	if (!strcmp(s, "text"))
		return ALT_WS_TEXT;
	if (!strcmp(s, "binary"))
		return ALT_WS_BINARY;
	if (!strcmp(s, "ping"))
		return ALT_WS_PING;
	if (!strcmp(s, "pong"))
		return ALT_WS_PONG;
	if (!strcmp(s, "close"))
		return ALT_WS_CLOSE;
	return 0;
}

static void print_msg(int type, const alt_buf *m) {
	printf("msg %s %zu ", type == ALT_WS_TEXT ? "text" : "binary", m->n);
	for (size_t i = 0; i < m->n && i < 32; i++)
		printf("%02x", (uint8_t)m->p[i]);
	printf(" %016llx\n", (unsigned long long)fnv(m->p, m->n));
}

/* sends a generated message, reads the echo: 0 when it matches */
static int echo_one(int type, size_t size, uint64_t seed, size_t chunk, bool stream_read) {
	alt_buf out = ALT_BUF_INIT, in = ALT_BUF_INIT;
	gen(&out, size, type == ALT_WS_TEXT, seed);
	int rc;
	if (chunk) {
		alt_writer w;
		rc = alt_ws_next_writer(c, type, &w);
		for (size_t off = 0; rc == 0 && off < out.n; off += chunk) {
			ssize_t k = alt_write(w, out.p + off, out.n - off < chunk ? out.n - off : chunk);
			if (k < 0)
				rc = (int)k;
		}
		if (rc == 0)
			rc = alt_ws_close_writer(c);
	} else
		rc = alt_ws_write_message(c, type, out.p, out.n);
	int got = 0;
	if (rc == 0) {
		if (stream_read) {
			alt_reader r;
			if ((rc = alt_ws_next_reader(c, &got, &r)) == 0) {
				char buf[777];
				ssize_t k;
				while ((k = alt_read(r, buf, sizeof buf)) > 0)
					alt_buf_append(&in, buf, (size_t)k);
				rc = (int)k;
			}
		} else
			rc = alt_ws_read_message(c, &got, &in);
	}
	if (rc < 0)
		err_line(rc);
	else if (got != type || in.n != out.n || (out.n && memcmp(in.p, out.p, out.n))) {
		printf("echo bad type %d/%d len %zu/%zu\n", got, type, in.n, out.n);
		rc = -1;
	}
	alt_buf_free(&out);
	alt_buf_free(&in);
	return rc;
}

static void dial(char **f, int nf) {
	alt_ws_dialer d = {0};
	static const char *protos[8];
	static char protobuf[256];
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_http_header h;
	alt_http_header_init(&h, &a);
	alt_tls_config tc = {0};
	alt_x509_pool *pool = 0;
	for (int i = 2; i < nf; i++) {
		char *o = f[i];
		if (!strncmp(o, "proto=", 6)) {
			snprintf(protobuf, sizeof protobuf, "%s", o + 6);
			char *save, *tok;
			d.nsubprotocols = 0;
			for (tok = strtok_r(protobuf, ",", &save); tok && d.nsubprotocols < 8; tok = strtok_r(0, ",", &save))
				protos[d.nsubprotocols++] = tok;
			d.subprotocols = protos;
		} else if (!strcmp(o, "compress"))
			d.enable_compression = true;
		else if (!strcmp(o, "nct"))
			d.no_context_takeover = true;
		else if (!strncmp(o, "hdr=", 4)) {
			char *colon = strchr(o + 4, ':');
			if (colon) {
				*colon = 0;
				alt_http_header_add_cstr(&h, o + 4, colon + 1);
				d.header = &h;
			}
		} else if (!strncmp(o, "roots=", 6)) {
			alt_buf pem = ALT_BUF_INIT;
			FILE *fp = fopen(o + 6, "rb");
			char buf[4096];
			size_t k;
			while (fp && (k = fread(buf, 1, sizeof buf, fp)) > 0)
				alt_buf_append(&pem, buf, k);
			if (fp)
				fclose(fp);
			pool = alt_x509_pool_new();
			alt_x509_pool_add_pem(pool, pem.p, pem.n);
			alt_buf_free(&pem);
			tc.roots = pool;
			d.tls = &tc;
		} else if (!strncmp(o, "limit=", 6))
			d.read_limit = (size_t)atol(o + 6);
		else if (!strncmp(o, "frame=", 6))
			d.write_buffer_size = (size_t)atol(o + 6);
		else if (!strncmp(o, "level=", 6))
			d.compression_level = atoi(o + 6);
		else if (!strncmp(o, "timeout=", 8))
			d.handshake_timeout = atol(o + 8) * ALT_MS;
	}
	alt_http_response *resp = 0;
	int rc = alt_ws_dial(&d, f[1], 0, &c, &resp);
	if (rc == 0) {
		printf("dial ok status=%d proto=%s ext=%s x-ws=%.*s\n", resp->status, alt_ws_subprotocol(c), alt_ws_extensions(c),
			(int)alt_http_header_value(&resp->header, "X-Ws").n, alt_http_header_value(&resp->header, "X-Ws").p);
		alt_ws_set_pong_handler(c, on_pong, 0);
	} else {
		alt_buf body = ALT_BUF_INIT;
		if (resp) {
			alt_read_all(resp->body, &body, 0);
			for (size_t i = 0; i < body.n; i++)
				if (body.p[i] == '\n' || body.p[i] == '\r')
					body.p[i] = '|';
		}
		printf("dial err %s status=%d body=%s\n", errname(rc), resp ? resp->status : 0, alt_buf_cstr(&body));
		alt_buf_free(&body);
		c = 0;
	}
	alt_ws_response_free(resp);
	alt_arena_free(&a);
	/* the pool lives as long as the process: TLS keeps a pointer to it */
}

static int unhex(const char *s, alt_buf *b) {
	alt_buf_reset(b);
	for (size_t i = 0; s[i] && s[i + 1]; i += 2) {
		unsigned v;
		if (sscanf(s + i, "%2x", &v) != 1)
			return -1;
		alt_buf_append_byte(b, (int)v);
	}
	return 0;
}

int main(void) {
	char *line = 0;
	size_t cap = 0;
	while (getline(&line, &cap, stdin) > 0) {
		line[strcspn(line, "\r\n")] = 0;
		char *f[16];
		int nf = 0;
		char *save, *tok;
		for (tok = strtok_r(line, "\t", &save); tok && nf < 16; tok = strtok_r(0, "\t", &save))
			f[nf++] = tok;
		if (nf == 0)
			continue;
		const char *cmd = f[0];
		if (!strcmp(cmd, "dial") && nf >= 2) {
			dial(f, nf);
		} else if (!c) {
			printf("no connection\n");
		} else if (!strcmp(cmd, "echo") && nf >= 4) {
			size_t chunk = nf >= 5 && !strncmp(f[4], "chunk=", 6) ? (size_t)atol(f[4] + 6) : 0;
			if (echo_one(type_of(f[1]), (size_t)atol(f[2]), (uint64_t)atoll(f[3]), chunk, false) == 0)
				printf("echo ok\n");
		} else if (!strcmp(cmd, "echos")) {
			static const size_t sizes[] = {0, 1, 125, 126, 127, 65535, 65536, 65537, 1 << 20, 5 << 20};
			int n = 0, rc = 0;
			size_t max = nf >= 2 ? (size_t)atol(f[1]) : SIZE_MAX;
			for (size_t i = 0; rc == 0 && i < sizeof sizes / sizeof *sizes && sizes[i] <= max; i++)
				for (int mode = 0; rc == 0 && mode < 4; mode++) {
					int type = mode % 2 ? ALT_WS_BINARY : ALT_WS_TEXT;
					rc = echo_one(type, sizes[i], i * 4 + (size_t)mode, mode >= 2 ? 3000 : 0, mode == 3);
					n++;
				}
			if (rc == 0)
				printf("echos ok %d\n", n);
		} else if (!strcmp(cmd, "send") && nf >= 2) {
			alt_buf p = ALT_BUF_INIT;
			unhex(nf >= 3 ? f[2] : "", &p);
			int rc = alt_ws_write_message(c, type_of(f[1]), p.p, p.n);
			if (rc == 0)
				printf("sent\n");
			else
				printf("send err %s\n", errname(rc));
			alt_buf_free(&p);
		} else if (!strcmp(cmd, "recv") || !strcmp(cmd, "readall")) {
			alt_buf m = ALT_BUF_INIT;
			int type, rc;
			do {
				if ((rc = alt_ws_read_message(c, &type, &m)) == 0)
					print_msg(type, &m);
				else
					err_line(rc);
			} while (rc == 0 && !strcmp(cmd, "readall"));
			alt_buf_free(&m);
		} else if (!strcmp(cmd, "ping")) {
			const char *p = nf >= 2 ? f[1] : "";
			pongs[0] = 0;
			int rc = alt_ws_ping(c, p, strlen(p), alt_deadline(ALT_SEC));
			if (rc == 0) {
				alt_buf m = ALT_BUF_INIT;
				int type;
				if ((rc = alt_ws_write_message(c, ALT_WS_TEXT, "sync", 4)) == 0 && (rc = alt_ws_read_message(c, &type, &m)) == 0)
					printf("pong %s\n", pongs);
				alt_buf_free(&m);
			}
			if (rc)
				err_line(rc);
		} else if (!strcmp(cmd, "pongs")) {
			printf("pongs %s\n", pongs);
		} else if (!strcmp(cmd, "writeclose") && nf >= 2) {
			printf("writeclose %s\n", errname(alt_ws_write_close(c, atoi(f[1]), nf >= 3 ? f[2] : "", alt_deadline(ALT_SEC))));
		} else if (!strcmp(cmd, "close") && nf >= 2) {
			int code = atoi(f[1]);
			const char *reason = nf >= 3 ? f[2] : "";
			static int peer;
			static char peer_reason[128];
			peer = 0;
			peer_reason[0] = 0;
			alt_ws_conn *cc = c;
			c = 0;
			int rc = alt_ws_write_close(cc, code, reason, alt_deadline(ALT_SEC));
			if (rc == 0) {
				/* read up to the peer's close frame ourselves to see its code, then the handshake ends in alt_ws_close */
				alt_buf m = ALT_BUF_INIT;
				int type;
				alt_ws_set_read_deadline(cc, alt_deadline(2 * ALT_SEC));
				while ((rc = alt_ws_read_message(cc, &type, &m)) == 0) {
				}
				alt_buf_free(&m);
				alt_str r;
				peer = alt_ws_close_code(cc, &r);
				snprintf(peer_reason, sizeof peer_reason, "%.*s", (int)r.n, r.p);
			}
			int rc2 = alt_ws_close(cc, code, reason, ALT_SEC);
			printf("close %s peer=%d %s\n", errname(rc == -ALT_ECLOSED ? rc2 : rc), peer, peer_reason);
		} else if (!strcmp(cmd, "deadline") && nf >= 2) {
			alt_ws_set_read_deadline(c, alt_deadline(atol(f[1]) * ALT_MS));
		} else if (!strcmp(cmd, "free")) {
			alt_ws_free(c);
			c = 0;
		} else
			printf("unknown %s\n", cmd);
		fflush(stdout);
	}
	if (c)
		alt_ws_free(c);
	free(line);
	return 0;
}
