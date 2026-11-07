#include "check.h"
#include <alt/net.h>
#include <alt/net/http.h>
#include <alt/net/websocket.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

static uint64_t rng = 88172645463325252ull;
static uint32_t rnd(void) {
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (uint32_t)rng;
}

/* text that compresses (words) or random bytes */
static void fill(alt_buf *b, size_t n, bool text) {
	static const char *const words[] = {"web", "socket ", "frame ", "π ", "deflate ", "日本 ", "context ", "\n"};
	alt_buf_reset(b);
	while (b->n < n) {
		if (text) {
			const char *w = words[rnd() % 8];
			size_t k = strlen(w);
			if (b->n + k > n) {
				while (b->n < n)
					alt_buf_append_byte(b, 'x');
				break;
			}
			alt_buf_append(b, w, k);
		} else
			alt_buf_append_byte(b, (int)(rnd() & 0xff));
	}
}

static void accept_key(void) {
	char out[29];
	alt_ws_accept_key(ALT_STR("dGhlIHNhbXBsZSBub25jZQ=="), out);
	CHECK_STR(out, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
	int valid[] = {1000, 1001, 1002, 1003, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 3000, 3999, 4000, 4999};
	int invalid[] = {0, 999, 1004, 1005, 1006, 1015, 1016, 1100, 2000, 2999, 5000, 65535};
	for (size_t i = 0; i < sizeof valid / sizeof *valid; i++)
		CHECK(alt_ws_valid_close_code(valid[i]));
	for (size_t i = 0; i < sizeof invalid / sizeof *invalid; i++)
		CHECK(!alt_ws_valid_close_code(invalid[i]));
}

/* ---- a pair of connections over a socketpair ---- */

typedef struct {
	alt_ws_conn *srv, *cli;
	int sfd; /* the raw server end when raw */
} pair;

static void make_pair(pair *p, const alt_ws_stream_config *scfg, const alt_ws_stream_config *ccfg) {
	int fds[2];
	CHECK_OK(socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
	alt_conn *a, *b;
	CHECK_OK(alt_conn_from_fd(&a, fds[0]));
	CHECK_OK(alt_conn_from_fd(&b, fds[1]));
	alt_ws_stream_config s = *scfg, c = *ccfg;
	s.server = true;
	c.server = false;
	CHECK_OK(alt_ws_new_stream(&p->srv, alt_conn_reader(a), alt_conn_writer(a), a, &s));
	CHECK_OK(alt_ws_new_stream(&p->cli, alt_conn_reader(b), alt_conn_writer(b), b, &c));
}

static void *echo_thread(void *arg) {
	alt_ws_conn *c = arg;
	alt_buf m = ALT_BUF_INIT;
	int type, rc;
	while ((rc = alt_ws_read_message(c, &type, &m)) == 0)
		if (alt_ws_write_message(c, type, m.p, m.n) < 0)
			break;
	alt_buf_free(&m);
	return (void *)(intptr_t)rc;
}

/* echo of messages of many sizes, as whole messages and streamed */
static void echo_with(const alt_ws_stream_config *cfg) {
	pair p;
	make_pair(&p, cfg, cfg);
	pthread_t th;
	pthread_create(&th, 0, echo_thread, p.srv);
	size_t sizes[] = {0, 1, 125, 126, 127, 4095, 4096, 4097, 65535, 65536, 65537, 100000, 1 << 20, 3000000};
	alt_buf out = ALT_BUF_INIT, in = ALT_BUF_INIT;
	for (size_t i = 0; i < sizeof sizes / sizeof *sizes; i++) {
		for (int mode = 0; mode < (sizes[i] > 100000 ? 1 : 3); mode++) {
			bool text = (i + mode) % 2 == 0;
			fill(&out, sizes[i], text);
			int type = text ? ALT_WS_TEXT : ALT_WS_BINARY;
			if (mode == 0)
				CHECK_OK(alt_ws_write_message(p.cli, type, out.p, out.n));
			else {
				alt_writer w;
				CHECK_OK(alt_ws_next_writer(p.cli, type, &w));
				size_t chunk = mode == 1 ? 1000 : 7, off = 0;
				if (sizes[i] > 100000)
					chunk = 70000;
				while (off < out.n) {
					size_t k = out.n - off < chunk ? out.n - off : chunk;
					CHECK_INT(alt_write(w, out.p + off, k), k);
					off += k;
				}
				CHECK_OK(alt_ws_close_writer(p.cli));
			}
			int got;
			if (mode == 2) {
				alt_reader r;
				CHECK_OK(alt_ws_next_reader(p.cli, &got, &r));
				alt_buf_reset(&in);
				char buf[333];
				ssize_t k;
				while ((k = alt_read(r, buf, sizeof buf)) > 0)
					alt_buf_append(&in, buf, (size_t)k);
				CHECK_INT(k, 0);
			} else
				CHECK_OK(alt_ws_read_message(p.cli, &got, &in));
			CHECK_INT(got, type);
			CHECK_INT(in.n, out.n);
			CHECK(in.n == out.n && (out.n == 0 || memcmp(in.p, out.p, out.n) == 0));
		}
	}
	/* the close handshake started by the client */
	CHECK_OK(alt_ws_close(p.cli, 4321, "bye", ALT_SEC));
	void *res;
	pthread_join(th, &res);
	CHECK_ERR((int)(intptr_t)res, -ALT_ECLOSED);
	alt_str reason;
	CHECK_INT(alt_ws_close_code(p.srv, &reason), 4321);
	CHECK_STR(reason, "bye");
	CHECK_INT(alt_ws_sent_close_code(p.srv), 4321);
	CHECK_OK(alt_ws_close(p.srv, 1000, "", ALT_SEC));
	alt_buf_free(&out);
	alt_buf_free(&in);
}

static void echo_plain(void) {
	alt_ws_stream_config c = {.write_buffer_size = 1000};
	echo_with(&c);
}

static void echo_compressed(void) {
	for (int m = 0; m < 4; m++) {
		alt_ws_stream_config c = {.compress = true, .server_no_context_takeover = m & 1, .client_no_context_takeover = m & 2,
			.write_buffer_size = m == 3 ? 100 : 0, .compression_level = m == 2 ? 6 : 0};
		echo_with(&c);
	}
}

/* ---- raw frames against a server connection ---- */

typedef struct {
	int fd;
	alt_ws_conn *srv;
} rawpair;

static void raw_pair(rawpair *p, const alt_ws_stream_config *cfg) {
	int fds[2];
	CHECK_OK(socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
	alt_conn *a;
	CHECK_OK(alt_conn_from_fd(&a, fds[0]));
	alt_ws_stream_config s = cfg ? *cfg : (alt_ws_stream_config){0};
	s.server = true;
	CHECK_OK(alt_ws_new_stream(&p->srv, alt_conn_reader(a), alt_conn_writer(a), a, &s));
	p->fd = fds[1];
}

/* a masked frame */
static size_t frame(uint8_t *out, int b0, const void *payload, size_t n) {
	size_t h = 0;
	out[h++] = (uint8_t)b0;
	if (n < 126)
		out[h++] = (uint8_t)(0x80 | n);
	else if (n < 65536) {
		out[h++] = 0x80 | 126;
		out[h++] = (uint8_t)(n >> 8);
		out[h++] = (uint8_t)n;
	} else {
		out[h++] = 0x80 | 127;
		for (int i = 0; i < 8; i++)
			out[h++] = (uint8_t)((uint64_t)n >> (56 - 8 * i));
	}
	uint8_t key[4] = {0x12, 0x34, 0x56, 0x78};
	memcpy(out + h, key, 4);
	h += 4;
	for (size_t i = 0; i < n; i++)
		out[h + i] = ((const uint8_t *)payload)[i] ^ key[i & 3];
	return h + n;
}

static void put_all(int fd, const void *p, size_t n) {
	while (n) {
		ssize_t k = write(fd, p, n);
		if (k <= 0)
			break;
		p = (const char *)p + k;
		n -= (size_t)k;
	}
}

/* reads one unmasked frame from the server: opcode, payload */
static int get_frame(int fd, uint8_t *payload, size_t *n) {
	uint8_t h[10];
	if (read(fd, h, 2) != 2)
		return -1;
	size_t len = h[1] & 0x7f;
	if (h[1] & 0x80)
		return -2;
	if (len == 126) {
		if (read(fd, h + 2, 2) != 2)
			return -1;
		len = (size_t)h[2] << 8 | h[3];
	}
	size_t got = 0;
	while (got < len) {
		ssize_t k = read(fd, payload + got, len - got);
		if (k <= 0)
			return -1;
		got += (size_t)k;
	}
	*n = len;
	return h[0];
}

static void raw_frames(void) {
	struct {
		const char *name;
		uint8_t bytes[64];
		size_t n;
		int want_err, want_code;
	} cases[] = {
		{"rsv2", {0x81 | 0x20, 0x80, 0, 0, 0, 0}, 6, -EPROTO, 1002},
		{"opcode 3", {0x83, 0x80, 0, 0, 0, 0}, 6, -EPROTO, 1002},
		{"unmasked", {0x81, 0x00}, 2, -EPROTO, 1002},
		{"rsv1 without extension", {0xc1, 0x80, 0, 0, 0, 0}, 6, -EPROTO, 1002},
		{"continuation first", {0x80, 0x80, 0, 0, 0, 0}, 6, -EPROTO, 1002},
		{"fragmented ping", {0x09, 0x80, 0, 0, 0, 0}, 6, -EPROTO, 1002},
		{"long ping", {0x89, 0x80 | 126, 0, 126, 0, 0, 0, 0}, 8, -EPROTO, 1002},
		{"short 16-bit length", {0x82, 0x80 | 126, 0, 5, 0, 0, 0, 0}, 8, -EPROTO, 1002},
		{"64-bit msb", {0x82, 0x80 | 127, 0x80, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, 14, -EPROTO, 1002},
		{"close of 1 byte", {0x88, 0x81, 0, 0, 0, 0, 3}, 7, -EPROTO, 1002},
		{"close 1005", {0x88, 0x82, 0, 0, 0, 0, 0x03, 0xed}, 8, -EPROTO, 1002},
		{"bad utf-8 text", {0x81, 0x82, 0, 0, 0, 0, 0xc3, 0x28}, 8, -EILSEQ, 1007},
	};
	for (size_t i = 0; i < sizeof cases / sizeof *cases; i++) {
		rawpair p;
		raw_pair(&p, 0);
		put_all(p.fd, cases[i].bytes, cases[i].n);
		int type;
		alt_buf m = ALT_BUF_INIT;
		int rc = alt_ws_read_message(p.srv, &type, &m);
		if (rc != cases[i].want_err)
			FAILED("%s: %s", cases[i].name, alt_strerror(rc));
		uint8_t pl[200];
		size_t n = 0;
		int op = get_frame(p.fd, pl, &n);
		CHECK_INT(op, 0x88);
		if (op == 0x88 && n >= 2 && (pl[0] << 8 | pl[1]) != cases[i].want_code)
			FAILED("%s: close code %d", cases[i].name, pl[0] << 8 | pl[1]);
		alt_buf_free(&m);
		alt_ws_free(p.srv);
		close(p.fd);
	}
	/* a ping between fragments is answered; the message comes whole */
	rawpair p;
	raw_pair(&p, 0);
	uint8_t buf[256];
	size_t n = frame(buf, 0x01, "Hel", 3);
	n += frame(buf + n, 0x89, "pp", 2);
	n += frame(buf + n, 0x00, "lo", 2);
	n += frame(buf + n, 0x80, "", 0);
	put_all(p.fd, buf, n);
	int type;
	alt_buf m = ALT_BUF_INIT;
	CHECK_OK(alt_ws_read_message(p.srv, &type, &m));
	CHECK_STR(alt_buf_str(&m), "Hello");
	uint8_t pl[200];
	CHECK_INT(get_frame(p.fd, pl, &n), 0x8a);
	CHECK(n == 2 && memcmp(pl, "pp", 2) == 0);
	/* a read limit */
	alt_ws_set_read_limit(p.srv, 10);
	n = frame(buf, 0x82, "0123456789a", 11);
	put_all(p.fd, buf, n);
	CHECK_ERR(alt_ws_read_message(p.srv, &type, &m), -EMSGSIZE);
	CHECK_INT(get_frame(p.fd, pl, &n), 0x88);
	CHECK_INT(pl[0] << 8 | pl[1], 1009);
	/* sticky */
	CHECK_ERR(alt_ws_read_message(p.srv, &type, &m), -EMSGSIZE);
	alt_buf_free(&m);
	alt_ws_free(p.srv);
	close(p.fd);
	/* the peer goes away without a close frame */
	raw_pair(&p, 0);
	close(p.fd);
	CHECK_ERR(alt_ws_read_message(p.srv, &type, &m), -ALT_EUNEXPECTED_EOF);
	CHECK_INT(alt_ws_close_code(p.srv, 0), 1006);
	alt_ws_free(p.srv);
	alt_buf_free(&m);
}

/* ---- concurrency: writers in threads while the main thread reads ---- */

static _Atomic int pongs;
static int on_pong(alt_ws_conn *c, alt_str payload, void *arg) {
	(void)c, (void)arg;
	if (payload.n == 4 && memcmp(payload.p, "ping", 4) == 0)
		atomic_fetch_add(&pongs, 1);
	return 0;
}

enum { NMSG = 300 };
static _Atomic bool writer_done;

static void *data_writer(void *arg) {
	alt_ws_conn *c = arg;
	alt_buf b = ALT_BUF_INIT;
	for (int i = 0; i < NMSG; i++) {
		fill(&b, (size_t)(i * 97 % 20000), true);
		alt_buf_printf(&b, "#%d", i);
		if (i % 3 == 0) {
			alt_writer w;
			if (alt_ws_next_writer(c, ALT_WS_TEXT, &w) < 0)
				break;
			for (size_t off = 0; off < b.n; off += 500)
				alt_write(w, b.p + off, b.n - off < 500 ? b.n - off : 500);
			if (alt_ws_close_writer(c) < 0)
				break;
		} else if (alt_ws_write_message(c, ALT_WS_TEXT, b.p, b.n) < 0)
			break;
	}
	alt_buf_free(&b);
	atomic_store(&writer_done, true);
	return 0;
}

static void *pinger(void *arg) {
	alt_ws_conn *c = arg;
	int sent = 0;
	while (!atomic_load(&writer_done)) {
		if (alt_ws_ping(c, "ping", 4, alt_deadline(ALT_SEC)) == 0)
			sent++;
		usleep(200);
	}
	/* the close comes from this thread too, while the main one reads */
	usleep(20000);
	alt_ws_write_close(c, 1001, "going", alt_deadline(ALT_SEC));
	return (void *)(intptr_t)sent;
}

static void concurrent(void) {
	for (int comp = 0; comp < 2; comp++) {
		alt_ws_stream_config cfg = {.compress = comp, .write_buffer_size = 700};
		pair p;
		make_pair(&p, &cfg, &cfg);
		pthread_t echo, wr, pg;
		pthread_create(&echo, 0, echo_thread, p.srv);
		atomic_store(&pongs, 0);
		atomic_store(&writer_done, false);
		alt_ws_set_pong_handler(p.cli, on_pong, 0);
		pthread_create(&wr, 0, data_writer, p.cli);
		pthread_create(&pg, 0, pinger, p.cli);
		int type, n = 0, rc;
		alt_buf m = ALT_BUF_INIT;
		while ((rc = alt_ws_read_message(p.cli, &type, &m)) == 0) {
			char want[16];
			snprintf(want, sizeof want, "#%d", n);
			CHECK(alt_str_has_suffix(alt_buf_str(&m), alt_str_cstr(want)));
			n++;
		}
		CHECK_ERR(rc, -ALT_ECLOSED);
		CHECK_INT(n, NMSG);
		CHECK_INT(alt_ws_close_code(p.cli, 0), 1001);
		void *sent;
		pthread_join(wr, 0);
		pthread_join(pg, &sent);
		CHECK((intptr_t)sent > 0);
		CHECK_INT(atomic_load(&pongs), (intptr_t)sent);
		void *res;
		pthread_join(echo, &res);
		CHECK_ERR((int)(intptr_t)res, -ALT_ECLOSED);
		CHECK_INT(alt_ws_close_code(p.srv, 0), 1001);
		alt_ws_free(p.srv);
		CHECK_OK(alt_ws_close(p.cli, 1000, "", ALT_SEC));
		alt_buf_free(&m);
	}
}

/* a deadline set from another thread wakes the reader */
static void *waker(void *arg) {
	usleep(50000);
	alt_ws_set_read_deadline(arg, ALT_NOWAIT);
	return 0;
}

static void deadlines(void) {
	alt_ws_stream_config cfg = {0};
	pair p;
	make_pair(&p, &cfg, &cfg);
	pthread_t th;
	pthread_create(&th, 0, waker, p.cli);
	int type;
	alt_buf m = ALT_BUF_INIT;
	CHECK_ERR(alt_ws_read_message(p.cli, &type, &m), -ETIMEDOUT);
	pthread_join(th, 0);
	/* a read deadline */
	alt_ws_set_read_deadline(p.srv, alt_deadline(30 * ALT_MS));
	CHECK_ERR(alt_ws_read_message(p.srv, &type, &m), -ETIMEDOUT);
	/* writes of control frames fail with a passed deadline only when they must wait */
	CHECK_OK(alt_ws_ping(p.cli, "x", 1, alt_deadline(ALT_SEC)));
	CHECK_ERR(alt_ws_write_control(p.cli, ALT_WS_PING, "x", 126, ALT_FOREVER), -EINVAL);
	CHECK_ERR(alt_ws_write_close(p.cli, 1005, "", ALT_FOREVER), -EINVAL);
	CHECK_ERR(alt_ws_write_message(p.cli, ALT_WS_TEXT, "\xff", 1), -EILSEQ);
	CHECK_OK(alt_ws_write_close(p.cli, 1000, "done", ALT_FOREVER));
	CHECK_ERR(alt_ws_write_message(p.cli, ALT_WS_TEXT, "a", 1), -ALT_ECLOSED);
	alt_buf_free(&m);
	alt_ws_free(p.cli);
	alt_ws_free(p.srv);
}

/* ---- over HTTP: the upgrade and the dial ---- */

static void ws_handler(alt_http_rw *w, alt_http_request *r, void *arg) {
	(void)arg;
	static const char *const protos[] = {"two", "one"};
	alt_ws_upgrader u = {.subprotocols = protos, .nsubprotocols = 2, .enable_compression = true};
	alt_ws_conn *c;
	if (alt_ws_upgrade(&u, w, r, 0, &c) < 0)
		return;
	alt_buf m = ALT_BUF_INIT;
	int type;
	while (alt_ws_read_message(c, &type, &m) == 0)
		if (alt_ws_write_message(c, type, m.p, m.n) < 0)
			break;
	alt_buf_free(&m);
	alt_ws_close(c, 1000, "", ALT_SEC);
}

struct sv {
	alt_http_server *s;
	alt_listener *l;
};

static void *serve_thread(void *arg) {
	struct sv *sv = arg;
	alt_http_server_serve(sv->s, sv->l);
	return 0;
}

static void http_upgrade(void) {
	alt_http_server *s;
	alt_http_server_config cfg = {.handler = {ws_handler, 0}, .threads = 2};
	CHECK_OK(alt_http_server_new(&s, &cfg));
	alt_listener *ln;
	CHECK_OK(alt_listen(&ln, "tcp", "127.0.0.1:0"));
	alt_buf url = ALT_BUF_INIT;
	alt_buf_append_cstr(&url, "ws://");
	alt_addr_format(alt_listener_addr(ln), &url);
	alt_buf_append_cstr(&url, "/ws?x=1");
	pthread_t th;
	struct sv sv = {s, ln};
	pthread_create(&th, 0, serve_thread, &sv);
	for (int comp = 0; comp < 2; comp++) {
		static const char *const offer[] = {"one", "two", "three"};
		alt_ws_dialer d = {.subprotocols = offer, .nsubprotocols = 3, .enable_compression = comp, .no_context_takeover = comp};
		alt_ws_conn *c;
		alt_http_response *resp;
		CHECK_OK(alt_ws_dial(&d, url.p, 0, &c, &resp));
		if (!c)
			break;
		CHECK_INT(resp->status, 101);
		CHECK_STR(alt_ws_subprotocol(c), "two");
		CHECK_INT(alt_ws_compression_negotiated(c), comp);
		if (comp)
			CHECK_STR(alt_ws_extensions(c), "permessage-deflate; server_no_context_takeover; client_no_context_takeover");
		alt_ws_response_free(resp);
		alt_buf b = ALT_BUF_INIT, in = ALT_BUF_INIT;
		for (int i = 0; i < 20; i++) {
			fill(&b, rnd() % 70000, i % 2);
			CHECK_OK(alt_ws_write_message(c, i % 2 ? ALT_WS_TEXT : ALT_WS_BINARY, b.p, b.n));
			int type;
			CHECK_OK(alt_ws_read_message(c, &type, &in));
			CHECK(in.n == b.n && (b.n == 0 || memcmp(in.p, b.p, b.n) == 0));
		}
		CHECK_OK(alt_ws_close(c, 1000, "", ALT_SEC));
		alt_buf_free(&b);
		alt_buf_free(&in);
	}
	/* a plain GET is refused */
	alt_http_client *hc;
	alt_http_client_config ccfg = {0};
	CHECK_OK(alt_http_client_new(&hc, &ccfg));
	alt_buf hu = ALT_BUF_INIT;
	alt_buf_printf(&hu, "http://%s", url.p + 5);
	alt_http_response *hr;
	CHECK_OK(alt_http_get(hc, hu.p, &hr, 0));
	CHECK_INT(hr->status, 400);
	alt_http_response_free(hr);
	alt_http_client_free(hc);
	alt_buf_free(&hu);
	/* a dial with a wrong scheme, a refused handshake */
	alt_ws_conn *c;
	CHECK_ERR(alt_ws_dial(0, "http://127.0.0.1:1/", 0, &c, 0), -ALT_EHTTP_SCHEME);
	alt_http_server_close(s);
	pthread_join(th, 0);
	alt_http_server_free(s);
	alt_buf_free(&url);
}

int main(void) {
	RUN(accept_key);
	RUN(echo_plain);
	RUN(echo_compressed);
	RUN(raw_frames);
	RUN(concurrent);
	RUN(deadlines);
	RUN(http_upgrade);
	return check_done();
}
