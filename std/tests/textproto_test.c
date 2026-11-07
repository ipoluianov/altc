#include "check.h"
#include <alt/net/textproto.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
	alt_str_reader sr;
	alt_bufreader br;
	alt_textproto_reader r;
} rd;

static alt_textproto_reader *rd_open(rd *x, const char *data) {
	alt_bufreader_init(&x->br, alt_str_reader_init(&x->sr, alt_str_cstr(data)), 16);
	alt_textproto_reader_init(&x->r, &x->br);
	return &x->r;
}

static void rd_close(rd *x) {
	alt_textproto_reader_free(&x->r);
	alt_bufreader_free(&x->br);
}

static void lines(void) {
	rd x;
	/* a 16-byte buffer makes lines longer than it and \r\n across refills */
	alt_textproto_reader *r = rd_open(&x, "line 1\r\nthe second line is long\r\nx\r\r\n\nlast");
	alt_buf l = ALT_BUF_INIT;
	CHECK_OK(alt_textproto_read_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "line 1");
	CHECK_OK(alt_textproto_read_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "the second line is long");
	CHECK_OK(alt_textproto_read_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "x\r");
	CHECK_OK(alt_textproto_read_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "");
	CHECK_OK(alt_textproto_read_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "last");
	CHECK_ERR(alt_textproto_read_line(r, &l), -ALT_EEOF);
	CHECK_STR(alt_textproto_error(r), "EOF");
	rd_close(&x);

	r = rd_open(&x, "Line 1\r\n  continued...\r\n\tand more \r\nLine 2\r\n\r\n  not continued\r\n");
	CHECK_OK(alt_textproto_read_continued_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "Line 1 continued... and more");
	CHECK_OK(alt_textproto_read_continued_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "Line 2");
	CHECK_OK(alt_textproto_read_continued_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "");
	CHECK_OK(alt_textproto_read_continued_line(r, &l));
	CHECK_STR(alt_buf_str(&l), "not continued");
	rd_close(&x);
	alt_buf_free(&l);
	CHECK_STR(alt_textproto_trim(ALT_STR(" \t a b \r\n")), "a b");
}

static void codes(void) {
	rd x;
	alt_buf m = ALT_BUF_INIT;
	int code;
	alt_textproto_reader *r = rd_open(&x, "220 ready\r\n250-first\r\n250-second\r\n250 last\r\n550 5.1.1 no such user\r\n21\r\n"
					      "230-a\r\nplain text\r\n231 other\r\n230 end\r\n");
	CHECK_OK(alt_textproto_read_code_line(r, 2, &code, &m));
	CHECK_INT(code, 220);
	CHECK_STR(alt_buf_str(&m), "ready");
	CHECK_OK(alt_textproto_read_response(r, 25, &code, &m));
	CHECK_INT(code, 250);
	CHECK_STR(alt_buf_str(&m), "first\nsecond\nlast");
	CHECK_ERR(alt_textproto_read_response(r, 250, &code, &m), -EREMOTEIO);
	CHECK_INT(code, 550);
	CHECK_INT(r->code, 550);
	CHECK_STR(alt_textproto_error(r), "550 \"5.1.1 no such user\"");
	CHECK_ERR(alt_textproto_read_response(r, 0, &code, &m), -EPROTO);
	CHECK_STR(alt_textproto_error(r), "short response: \"21\"");
	/* RFC 959 continuation lines and lines of another code join the message */
	CHECK_OK(alt_textproto_read_response(r, 230, &code, &m));
	CHECK_STR(alt_buf_str(&m), "a\nplain text\n231 other\nend");
	CHECK_ERR(alt_textproto_read_response(r, 0, &code, &m), -ALT_EEOF);
	rd_close(&x);

	r = rd_open(&x, "250-multi\r\n250 end\r\n");
	CHECK_ERR(alt_textproto_read_code_line(r, 250, &code, &m), -EPROTO);
	CHECK_STR(alt_textproto_error(r), "unexpected multi-line response: \"multi\"");
	rd_close(&x);
	alt_buf_free(&m);
}

static void dots(void) {
	rd x;
	alt_buf b = ALT_BUF_INIT;
	alt_textproto_reader *r = rd_open(&x, "dotlines\r\n..foo\r\n.bar\n...baz\nquux\r\n\r\n.\r\nanot.her\r\n");
	CHECK_OK(alt_textproto_read_dot_bytes(r, &b));
	CHECK_STR(alt_buf_str(&b), "dotlines\n.foo\nbar\n..baz\nquux\n\n");
	alt_buf_reset(&b);
	CHECK_OK(alt_textproto_read_line(r, &b));
	CHECK_STR(alt_buf_str(&b), "anot.her");
	rd_close(&x);

	/* a dot reader read in part is drained by the next call */
	r = rd_open(&x, "abc\r\ndef\r\n.\r\nnext\r\n");
	alt_reader d = alt_textproto_dot_reader(r);
	char buf[2];
	CHECK_INT(alt_read(d, buf, 2), 2);
	CHECK_OK(alt_textproto_read_line(r, &b));
	CHECK_STR(alt_buf_str(&b), "next");
	rd_close(&x);

	r = rd_open(&x, "abc\r\n..x\r\n");
	alt_buf_reset(&b);
	CHECK_ERR(alt_textproto_read_dot_bytes(r, &b), -ALT_EUNEXPECTED_EOF);
	CHECK_STR(alt_buf_str(&b), "abc\n.x\n");
	CHECK_STR(alt_textproto_error(r), "unexpected EOF");
	rd_close(&x);

	alt_arena a;
	alt_arena_init(&a, 0);
	alt_str *v;
	size_t n;
	r = rd_open(&x, "dotlines\r\n.foo\r\n..bar\n...baz\nquux\r\n\r\n.\r\nanother\n");
	CHECK_OK(alt_textproto_read_dot_lines(r, &a, &v, &n));
	CHECK_INT(n, 6);
	CHECK_STR(v[1], "foo");
	CHECK_STR(v[2], ".bar");
	CHECK_STR(v[5], "");
	rd_close(&x);
	alt_arena_free(&a);
	alt_buf_free(&b);
}

static void headers(void) {
	rd x;
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_textproto_header h;
	alt_textproto_header_init(&h, &a);
	alt_textproto_reader *r = rd_open(&x, "my-key: Value 1  \r\nLong-key: Even \n Longer Value\r\nmy-Key: Value 2\r\n"
					      "Foo bar: baz\r\n\r\nbody");
	CHECK_OK(alt_textproto_read_mime_header(r, &h));
	CHECK_INT(h.n, 4);
	CHECK_STR(alt_textproto_header_get(&h, ALT_STR("MY-KEY")), "Value 1");
	alt_str vals[4];
	CHECK_INT(alt_textproto_header_values(&h, ALT_STR("my-key"), vals, 4), 2);
	CHECK_STR(vals[1], "Value 2");
	CHECK_STR(alt_textproto_header_get(&h, ALT_STR("long-key")), "Even Longer Value");
	CHECK_STR(alt_textproto_header_get(&h, ALT_STR("Foo bar")), "baz"); /* a space: kept as it is */
	alt_buf b = ALT_BUF_INIT;
	CHECK_OK(alt_textproto_read_line(r, &b));
	CHECK_STR(alt_buf_str(&b), "body");
	rd_close(&x);

	CHECK_OK(alt_textproto_header_set(&h, ALT_STR("my-key"), ALT_STR("only")));
	CHECK_INT(alt_textproto_header_values(&h, ALT_STR("My-Key"), vals, 4), 1);
	CHECK_OK(alt_textproto_header_add(&h, ALT_STR("x-new"), ALT_STR("1")));
	CHECK(alt_textproto_header_lookup(&h, ALT_STR("X-New"), 0));
	alt_textproto_header_del(&h, ALT_STR("x-NEW"));
	CHECK(!alt_textproto_header_lookup(&h, ALT_STR("X-New"), 0));
	CHECK_STR(alt_textproto_header_get(&h, ALT_STR("absent")), "");

	const char *bad[][2] = {
		{" First: line\r\n\r\n", "malformed MIME header initial line: \" First: line\""},
		{"Foo: bar\r\nno colon\r\n\r\n", "malformed MIME header: missing colon: \"no colon\""},
		{"Fo\x01o: bar\r\n\r\n", "malformed MIME header line: \"Fo\\x01o: bar\""},
		{"foo: b\x01r\r\n\r\n", "malformed MIME header line: \"Foo: b\\x01r\""},
		{": empty\r\n\r\n", "malformed MIME header line: \": empty\""},
		{"Foo: bar\r\n", "EOF"},
	};
	for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
		alt_textproto_header_init(&h, &a);
		r = rd_open(&x, bad[i][0]);
		CHECK(alt_textproto_read_mime_header(r, &h) < 0);
		CHECK_STR(alt_textproto_error(r), bad[i][1]);
		rd_close(&x);
	}
	/* limits */
	alt_textproto_header_init(&h, &a);
	r = rd_open(&x, "A: 1\r\nB: 2\r\nC: 3\r\n\r\n");
	CHECK_ERR(alt_textproto_read_mime_header_limit(r, &h, 0, 2), -ALT_ETOOLARGE);
	CHECK_STR(alt_textproto_error(r), "message too large");
	rd_close(&x);
	alt_textproto_header_init(&h, &a);
	r = rd_open(&x, "A: 1\r\nB: 2\r\n\r\n");
	CHECK_ERR(alt_textproto_read_mime_header_limit(r, &h, 600, 0), -ALT_ETOOLARGE);
	rd_close(&x);

	const char *canon[][2] = {{"a-b-c", "A-B-C"}, {"a", "A"}, {"ABC", "Abc"}, {"user-agent", "User-Agent"}, {"foo bar", "foo bar"},
		{"foo-", "Foo-"}, {"é-x", "é-x"}, {"", ""}, {"c\x01d", "c\x01d"}, {"x-Forwarded-for", "X-Forwarded-For"}};
	for (size_t i = 0; i < sizeof canon / sizeof *canon; i++) {
		alt_buf_reset(&b);
		alt_textproto_canonical_key(&b, alt_str_cstr(canon[i][0]));
		CHECK_STR(alt_buf_str(&b), canon[i][1]);
	}
	alt_buf_free(&b);
	alt_arena_free(&a);
}

static void writer(void) {
	alt_buf out = ALT_BUF_INIT;
	alt_bufwriter bw;
	alt_bufwriter_init(&bw, alt_buf_writer(&out), 0);
	alt_textproto_writer w;
	alt_textproto_writer_init(&w, &bw);
	CHECK_OK(alt_textproto_printf_line(&w, "foo %d", 123));
	CHECK_STR(alt_buf_str(&out), "foo 123\r\n");
	alt_buf_reset(&out);
	alt_writer d = alt_textproto_dot_writer(&w);
	CHECK_INT(alt_write_str(d, ALT_STR("abc\n.def\n..ghi\n.jkl\n.")), 21);
	CHECK_INT(alt_write_str(d, ALT_STR("\r\n")), 2);
	CHECK_OK(alt_textproto_dot_close(&w));
	CHECK_STR(alt_buf_str(&out), "abc\r\n..def\r\n...ghi\r\n..jkl\r\n..\r\n.\r\n");
	alt_buf_reset(&out);
	alt_textproto_dot_writer(&w);
	CHECK_OK(alt_textproto_dot_close(&w));
	CHECK_STR(alt_buf_str(&out), "\r\n.\r\n"); /* Go's quirk for an empty block */
	alt_bufwriter_free(&bw);
	alt_buf_free(&out);
}

/* pipelined requests from several threads: requests and responses keep their order */
typedef struct {
	alt_textproto_pipeline *p;
	int i;
	int *order;
	_Atomic int *n;
} pipe_arg;

static alt_textproto_pipeline pl;
static _Atomic unsigned ids[8];

static void *pipe_worker(void *arg) {
	pipe_arg *a = arg;
	unsigned id = ids[a->i];
	alt_textproto_start_response(a->p, id);
	a->order[(*a->n)++] = (int)id;
	alt_textproto_end_response(a->p, id);
	return 0;
}

static void pipeline(void) {
	int order[8];
	_Atomic int n = 0;
	pthread_t th[8];
	pipe_arg args[8];
	for (int i = 0; i < 8; i++) {
		ids[i] = alt_textproto_next(&pl);
		alt_textproto_start_request(&pl, ids[i]);
		alt_textproto_end_request(&pl, ids[i]);
	}
	for (int i = 7; i >= 0; i--) {
		args[i] = (pipe_arg){&pl, i, order, &n};
		pthread_create(&th[i], 0, pipe_worker, &args[i]);
	}
	for (int i = 0; i < 8; i++)
		pthread_join(th[i], 0);
	for (int i = 0; i < 8; i++)
		CHECK_INT(order[i], i);
}

/* a Conn over a socket pair: Cmd and the reply */
static void *echo_server(void *arg) {
	int fd = *(int *)arg;
	char buf[256];
	ssize_t k = read(fd, buf, sizeof buf);
	if (k > 0)
		(void)!write(fd, "250-hello\r\n250 there\r\n", 22);
	close(fd);
	return 0;
}

static void conn(void) {
	int sv[2];
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
	pthread_t th;
	pthread_create(&th, 0, echo_server, &sv[1]);
	alt_conn *c;
	CHECK_OK(alt_conn_from_fd(&c, sv[0]));
	alt_textproto_conn *tc;
	CHECK_OK(alt_textproto_conn_new(&tc, alt_conn_reader(c), alt_conn_writer(c)));
	unsigned id;
	CHECK_OK(alt_textproto_cmd(tc, &id, "HELO %s", "x"));
	CHECK_INT(id, 0);
	alt_textproto_start_response(&tc->p, id);
	int code;
	alt_buf m = ALT_BUF_INIT;
	CHECK_OK(alt_textproto_read_response(&tc->r, 250, &code, &m));
	alt_textproto_end_response(&tc->p, id);
	CHECK_STR(alt_buf_str(&m), "hello\nthere");
	alt_buf_free(&m);
	pthread_join(th, 0);
	alt_textproto_close(tc);
	alt_conn_close(c);
}

int main(void) {
	RUN(lines);
	RUN(codes);
	RUN(dots);
	RUN(headers);
	RUN(writer);
	RUN(pipeline);
	RUN(conn);
	return check_done();
}
