#include "check.h"
#include <alt/chan.h>
#include <alt/net.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

static void addr_str(const alt_addr *a, char *out, size_t n) {
	alt_buf b = ALT_BUF_INIT;
	alt_addr_format(a, &b);
	snprintf(out, n, "%s", alt_buf_cstr(&b));
	alt_buf_free(&b);
}

static void ips(void) {
	alt_ip ip;
	alt_buf b = ALT_BUF_INIT;
	struct {
		const char *in, *out;
	} cases[] = {
		{"1.2.3.4", "1.2.3.4"}, {"0.0.0.0", "0.0.0.0"}, {"255.255.255.255", "255.255.255.255"}, {"::", "::"}, {"::1", "::1"},
		{"2001:db8::1", "2001:db8::1"}, {"2001:0db8:0000:0000:0000:0000:0000:0001", "2001:db8::1"},
		{"2001:db8:0:0:1:0:0:1", "2001:db8::1:0:0:1"}, {"::ffff:1.2.3.4", "::ffff:1.2.3.4"}, {"fe80::1%eth0", "fe80::1%eth0"},
		{"1:2:3:4:5:6:7:8", "1:2:3:4:5:6:7:8"}, {"1::", "1::"}, {"::2:0:0:0:3", "::2:0:0:0:3"}, {"1:0:0:2:0:0:0:3", "1:0:0:2::3"}, {"ABCD::EF", "abcd::ef"},
	};
	for (size_t i = 0; i < sizeof cases / sizeof *cases; i++) {
		CHECK_OK(alt_ip_parse(alt_str_cstr(cases[i].in), &ip));
		alt_buf_reset(&b);
		alt_ip_format(&ip, &b);
		CHECK_STR(alt_buf_str(&b), cases[i].out);
	}
	const char *bad[] = {"", "1.2.3", "1.2.3.4.5", "256.1.1.1", "01.2.3.4", "1.2.3.4%x", ":::", "1:2:3:4:5:6:7:8:9", "::1::",
		"12345::", "1.2.3.-4", "x", "fe80::1%", "1:2:3:4:5:6:7::8"};
	for (size_t i = 0; i < sizeof bad / sizeof *bad; i++)
		CHECK_ERR(alt_ip_parse(alt_str_cstr(bad[i]), &ip), -ALT_ENET_ADDR);
	alt_ip_parse(ALT_STR("127.0.0.53"), &ip);
	CHECK(alt_ip_is_loopback(&ip));
	alt_ip_parse(ALT_STR("192.168.1.1"), &ip);
	CHECK(alt_ip_is_private(&ip));
	alt_ip_parse(ALT_STR("::"), &ip);
	CHECK(alt_ip_is_unspecified(&ip));
	alt_buf_free(&b);

	alt_str h, p;
	CHECK_OK(alt_split_host_port(ALT_STR("example.com:80"), &h, &p));
	CHECK_STR(h, "example.com");
	CHECK_STR(p, "80");
	CHECK_OK(alt_split_host_port(ALT_STR("[::1]:443"), &h, &p));
	CHECK_STR(h, "::1");
	CHECK_OK(alt_split_host_port(ALT_STR(":8080"), &h, &p));
	CHECK_STR(h, "");
	CHECK_ERR(alt_split_host_port(ALT_STR("::1:80"), &h, &p), -ALT_ENET_ADDR);
	CHECK_ERR(alt_split_host_port(ALT_STR("host"), &h, &p), -ALT_ENET_ADDR);
	uint16_t port;
	CHECK_OK(alt_lookup_port("tcp", ALT_STR("https"), &port));
	CHECK_INT(port, 443);
	CHECK_OK(alt_lookup_port("tcp", ALT_STR("8080"), &port));
	CHECK_INT(port, 8080);
	CHECK_ERR(alt_lookup_port("tcp", ALT_STR("70000"), &port), -ALT_ENET_ADDR);
	CHECK_ERR(alt_lookup_port("tcp", ALT_STR("no-such-service"), &port), -ALT_ENET_ADDR);
}

/* ---- an echo server in tasks ---- */

static void echo_conn(void *arg) {
	alt_conn *c = arg;
	char buf[8192];
	for (;;) {
		ssize_t n = alt_conn_read(c, buf, sizeof buf);
		if (n <= 0)
			break;
		if (alt_conn_write(c, buf, (size_t)n) < 0)
			break;
	}
	alt_conn_close(c);
}

typedef struct {
	alt_listener *l;
	alt_sched **workers;
	int nworkers;
	_Atomic int accepted;
} server;

static void accept_loop(void *arg) {
	server *s = arg;
	for (int i = 0;; i++) {
		alt_conn *c;
		int rc = alt_accept(s->l, &c, 0);
		if (rc < 0) {
			CHECK_ERR(rc, -ALT_ECLOSED);
			return;
		}
		atomic_fetch_add(&s->accepted, 1);
		if (s->nworkers)
			alt_go_on(s->workers[i % s->nworkers], echo_conn, c, 0);
		else
			alt_go(echo_conn, c);
	}
}

static char server_addr[128];
static alt_waitgroup clients = ALT_WAITGROUP_INIT;

static void echo_client(void *arg) {
	int id = (int)(intptr_t)arg;
	alt_conn *c;
	int rc = alt_dial(&c, "tcp", server_addr, 0);
	CHECK_OK(rc);
	if (rc == 0) {
		char msg[64], back[64];
		for (int k = 0; k < 20; k++) {
			int n = snprintf(msg, sizeof msg, "client %d message %d", id, k);
			CHECK_INT(alt_conn_write(c, msg, (size_t)n), n);
			CHECK_INT(alt_read_full(alt_conn_reader(c), back, (size_t)n), n);
			CHECK(memcmp(msg, back, (size_t)n) == 0);
		}
		alt_conn_close(c);
	}
	alt_waitgroup_done(&clients);
}

static void *thread_client(void *arg) {
	echo_client(arg);
	return 0;
}

static void echo_main(void *arg) {
	server *s = arg;
	for (int i = 0; i < 100; i++)
		alt_go(echo_client, (void *)(intptr_t)i);
	alt_waitgroup_wait(&clients);
	alt_listener_close(s->l); /* wakes the accept loop */
}

static void echo(void) {
	server s = {0};
	CHECK_OK(alt_listen(&s.l, "tcp", "127.0.0.1:0"));
	addr_str(alt_listener_addr(s.l), server_addr, sizeof server_addr);
	CHECK(strncmp(server_addr, "127.0.0.1:", 10) == 0);
	alt_sched *w[3];
	for (int i = 0; i < 3; i++)
		alt_sched_start(&w[i]);
	s.workers = w;
	s.nworkers = 3;
	alt_sched *acc;
	alt_sched_start(&acc);
	alt_go_on(acc, accept_loop, &s, 0);
	/* clients: 100 tasks and 8 plain threads */
	alt_waitgroup_add(&clients, 100 + 8);
	pthread_t th[8];
	for (int i = 0; i < 8; i++)
		pthread_create(&th[i], 0, thread_client, (void *)(intptr_t)(1000 + i));
	CHECK_OK(alt_run(echo_main, &s));
	for (int i = 0; i < 8; i++)
		pthread_join(th[i], 0);
	CHECK_INT(atomic_load(&s.accepted), 108);
	alt_sched_stop(acc);
	for (int i = 0; i < 3; i++)
		alt_sched_stop(w[i]);
}

/* ---- a large transfer: partial writes, both directions at once ---- */

enum { BIG = 16 << 20 };

static void big_writer(void *arg) {
	alt_conn *c = arg;
	char *data = malloc(BIG);
	for (int i = 0; i < BIG; i++)
		data[i] = (char)(i * 7 + (i >> 12));
	CHECK_INT(alt_conn_write(c, data, BIG), BIG);
	alt_conn_close_write(c);
	free(data);
}

static void big_main(void *arg) {
	(void)arg;
	alt_listener *l;
	CHECK_OK(alt_listen(&l, "tcp", "[::1]:0"));
	char a[128];
	addr_str(alt_listener_addr(l), a, sizeof a);
	CHECK(strncmp(a, "[::1]:", 6) == 0);
	alt_conn *client, *srv;
	CHECK_OK(alt_dial(&client, "tcp6", a, 0));
	CHECK_OK(alt_accept(l, &srv, 0));
	alt_go(big_writer, client);
	long got = 0, bad = 0;
	char buf[65536];
	for (;;) {
		ssize_t n = alt_conn_read(srv, buf, sizeof buf);
		CHECK(n >= 0);
		if (n <= 0)
			break;
		for (ssize_t i = 0; i < n; i++, got++)
			bad += buf[i] != (char)(got * 7 + (got >> 12));
	}
	CHECK_INT(got, BIG);
	CHECK_INT(bad, 0);
	alt_yield();
	alt_conn_close(srv);
	alt_conn_close(client);
	alt_listener_close(l);
}

static void transfer(void) { CHECK_OK(alt_run(big_main, 0)); }

/* ---- deadlines and closing ---- */

typedef struct {
	alt_conn *c;
	int rc;
	alt_dur took;
} blocked;

static void blocked_reader(void *arg) {
	blocked *b = arg;
	char buf[8];
	alt_time t0 = alt_now();
	b->rc = (int)alt_conn_read(b->c, buf, sizeof buf);
	b->took = alt_since(t0);
}

static void deadline_main(void *arg) {
	(void)arg;
	alt_listener *l;
	CHECK_OK(alt_listen(&l, "tcp", "127.0.0.1:0"));
	char a[128];
	addr_str(alt_listener_addr(l), a, sizeof a);
	alt_conn *c, *s;
	CHECK_OK(alt_dial(&c, "tcp", a, 0));
	CHECK_OK(alt_accept(l, &s, 0));
	char buf[8];
	/* a read deadline */
	alt_conn_set_read_deadline(c, alt_deadline(20 * ALT_MS));
	alt_time t0 = alt_now();
	CHECK_ERR(alt_conn_read(c, buf, sizeof buf), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 20 * ALT_MS);
	/* a passed deadline fails at once, even with data */
	alt_conn_write(s, "x", 1);
	alt_sleep(5 * ALT_MS);
	CHECK_ERR(alt_conn_read(c, buf, sizeof buf), -ETIMEDOUT);
	alt_conn_set_read_deadline(c, ALT_FOREVER);
	CHECK_INT(alt_conn_read(c, buf, sizeof buf), 1);
	/* extending the deadline of a waiting read */
	blocked b = {c, 0, 0};
	alt_conn_set_read_deadline(c, alt_deadline(10 * ALT_MS));
	alt_go(blocked_reader, &b);
	alt_yield();
	alt_conn_set_read_deadline(c, alt_deadline(60 * ALT_MS));
	alt_sleep(100 * ALT_MS);
	CHECK_ERR(b.rc, -ETIMEDOUT);
	CHECK(b.took >= 55 * ALT_MS);
	/* closing wakes a waiting read */
	alt_conn_set_read_deadline(c, ALT_FOREVER);
	blocked b2 = {c, 0, 0};
	alt_go(blocked_reader, &b2);
	alt_sleep(10 * ALT_MS);
	CHECK_OK(alt_conn_close(c)); /* waits for the read to return */
	CHECK_ERR(b2.rc, -ALT_ECLOSED);
	/* the peer sees the end of the stream */
	CHECK_INT(alt_conn_read(s, buf, sizeof buf), 0);
	alt_conn_close(s);
	/* accept with a cancelled context */
	alt_ctx *ctx;
	alt_ctx_with_timeout(&ctx, 0, 15 * ALT_MS);
	t0 = alt_now();
	CHECK_ERR(alt_accept(l, &s, ctx), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 15 * ALT_MS);
	alt_ctx_free(ctx);
	alt_ctx_new(&ctx, 0);
	alt_ctx_cancel(ctx);
	CHECK_ERR(alt_accept(l, &s, ctx), -ECANCELED);
	alt_ctx_free(ctx);
	alt_listener_close(l);
	/* refused */
	CHECK_ERR(alt_dial(&c, "tcp", a, 0), -ECONNREFUSED);
}

static void deadlines(void) { CHECK_OK(alt_run(deadline_main, 0)); }

/* the same from a plain thread */
static void *thread_deadline(void *arg) {
	(void)arg;
	alt_listener *l;
	CHECK_OK(alt_listen(&l, "tcp4", ":0"));
	char a[128];
	addr_str(alt_listener_addr(l), a, sizeof a);
	CHECK(strncmp(a, "0.0.0.0:", 8) == 0);
	alt_conn *c, *s;
	CHECK_OK(alt_dial(&c, "tcp", a + 7 /* ":port" */, 0));
	CHECK_OK(alt_accept(l, &s, 0));
	char buf[8];
	alt_conn_set_read_deadline(c, alt_deadline(15 * ALT_MS));
	alt_time t0 = alt_now();
	CHECK_ERR(alt_conn_read(c, buf, sizeof buf), -ETIMEDOUT);
	CHECK(alt_since(t0) >= 15 * ALT_MS);
	CHECK_INT(alt_conn_write(s, "hello", 5), 5);
	alt_conn_set_read_deadline(c, alt_deadline(ALT_SEC));
	CHECK_INT(alt_read_full(alt_conn_reader(c), buf, 5), 5);
	CHECK(memcmp(buf, "hello", 5) == 0);
	alt_conn_close(c);
	alt_conn_close(s);
	alt_listener_close(l);
	return 0;
}

static void threads(void) {
	pthread_t th;
	pthread_create(&th, 0, thread_deadline, 0);
	pthread_join(th, 0);
}

/* ---- dual stack, UDP, Unix sockets ---- */

static void dual_main(void *arg) {
	(void)arg;
	alt_listener *l;
	CHECK_OK(alt_listen(&l, "tcp", ":0"));
	uint16_t port;
	alt_ip lip;
	CHECK(alt_addr_ip(alt_listener_addr(l), &lip, &port));
	char a4[64], a6[64];
	snprintf(a4, sizeof a4, "127.0.0.1:%u", port);
	snprintf(a6, sizeof a6, "[::1]:%u", port);
	const char *addrs[] = {a4, a6};
	for (int i = 0; i < 2; i++) {
		alt_conn *c, *s;
		CHECK_OK(alt_dial(&c, "tcp", addrs[i], 0));
		CHECK_OK(alt_accept(l, &s, 0));
		char ra[64];
		addr_str(alt_conn_remote_addr(s), ra, sizeof ra);
		CHECK(strncmp(ra, i == 0 ? "127.0.0.1:" : "[::1]:", i == 0 ? 10 : 6) == 0);
		alt_conn_close(c);
		alt_conn_close(s);
	}
	/* "localhost" */
	char al[64];
	snprintf(al, sizeof al, "localhost:%u", port);
	alt_conn *c, *s;
	CHECK_OK(alt_dial(&c, "tcp", al, 0));
	CHECK_OK(alt_accept(l, &s, 0));
	alt_conn_close(c);
	alt_conn_close(s);
	alt_listener_close(l);
}

static void udp_main(void *arg) {
	(void)arg;
	alt_conn *srv;
	CHECK_OK(alt_listen_packet(&srv, "udp", "127.0.0.1:0"));
	char a[64];
	addr_str(alt_conn_local_addr(srv), a, sizeof a);
	alt_conn *c;
	CHECK_OK(alt_dial(&c, "udp", a, 0));
	CHECK_INT(alt_conn_write(c, "ping", 4), 4);
	char buf[16];
	alt_addr from;
	CHECK_INT(alt_conn_read_from(srv, buf, sizeof buf, &from), 4);
	CHECK(memcmp(buf, "ping", 4) == 0);
	CHECK_INT(alt_conn_write_to(srv, "pong!", 5, &from), 5);
	CHECK_INT(alt_conn_read(c, buf, sizeof buf), 5);
	CHECK(memcmp(buf, "pong!", 5) == 0);
	alt_conn_set_read_deadline(srv, alt_deadline(10 * ALT_MS));
	CHECK_ERR(alt_conn_read_from(srv, buf, sizeof buf, &from), -ETIMEDOUT);
	alt_conn_close(c);
	alt_conn_close(srv);
}

static void unix_main(void *arg) {
	(void)arg;
	char path[64];
	snprintf(path, sizeof path, "/tmp/alt-net-test-%d.sock", getpid());
	const char *names[] = {path, "@alt-net-test-abstract"};
	for (int i = 0; i < 2; i++) {
		alt_listener *l;
		CHECK_OK(alt_listen(&l, "unix", names[i]));
		char a[128];
		addr_str(alt_listener_addr(l), a, sizeof a);
		CHECK_STR(a, names[i]);
		alt_conn *c, *s;
		CHECK_OK(alt_dial(&c, "unix", names[i], 0));
		CHECK_OK(alt_accept(l, &s, 0));
		CHECK_INT(alt_conn_write(c, "unix", 4), 4);
		char buf[8];
		CHECK_INT(alt_read_full(alt_conn_reader(s), buf, 4), 4);
		alt_conn_close(c);
		alt_conn_close(s);
		alt_listener_close(l);
	}
	CHECK(access(path, F_OK) != 0); /* removed on close */
}

static void sockets(void) {
	CHECK_OK(alt_run(dual_main, 0));
	CHECK_OK(alt_run(udp_main, 0));
	CHECK_OK(alt_run(unix_main, 0));
	alt_conn *c;
	CHECK_ERR(alt_dial(&c, "sctp", "x:1", 0), -ALT_ENET_NETWORK);
	CHECK_ERR(alt_dial(&c, "tcp", "no-port", 0), -ALT_ENET_ADDR);
	alt_ip out[4];
	CHECK_INT(alt_lookup_host(0, "localhost", out, 4) >= 1, 1);
	CHECK(alt_ip_is_loopback(&out[0]));
	CHECK_INT(alt_lookup_host(0, "1.2.3.4", out, 4), 1);
	CHECK_ERR(alt_lookup_host(0, "bad..name", out, 4), -ALT_ENET_NOHOST);
}

int main(void) {
	RUN(ips);
	RUN(echo);
	RUN(transfer);
	RUN(deadlines);
	RUN(threads);
	RUN(sockets);
	return check_done();
}
