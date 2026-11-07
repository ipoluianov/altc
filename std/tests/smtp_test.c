#include "check.h"
#include <alt/net/smtp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

/* A scripted server in a thread: it greets, records every line it gets
   and answers each with the next reply of the script (the data block
   after a 354 counts as one line). */
typedef struct {
	int lfd;
	const char *const *replies;
	alt_buf log;
} server;

static int read_line(int fd, alt_buf *line) {
	alt_buf_reset(line);
	char c;
	for (;;) {
		ssize_t k = read(fd, &c, 1);
		if (k <= 0)
			return -1;
		alt_buf_append_byte(line, c);
		if (c == '\n')
			return 0;
	}
}

static void *serve(void *arg) {
	server *s = arg;
	int fd = accept(s->lfd, 0, 0);
	if (fd < 0)
		return 0;
	(void)!write(fd, "220 mx ready\r\n", 14);
	alt_buf line = ALT_BUF_INIT;
	bool data = false;
	for (size_t i = 0; s->replies[i]; i++) {
		if (data) {
			/* the block up to the "." line */
			for (;;) {
				if (read_line(fd, &line) < 0)
					goto out;
				alt_buf_append_str(&s->log, alt_buf_str(&line));
				if (alt_str_eq_cstr(alt_buf_str(&line), ".\r\n"))
					break;
			}
		} else {
			if (read_line(fd, &line) < 0)
				break;
			alt_buf_append_str(&s->log, alt_buf_str(&line));
		}
		const char *r = s->replies[i];
		(void)!write(fd, r, strlen(r));
		data = !strncmp(r, "354", 3);
	}
out:
	read_line(fd, &line); /* until the client closes */
	alt_buf_free(&line);
	close(fd);
	return 0;
}

static pthread_t start(server *s, const char *const *replies, char *addr) {
	s->replies = replies;
	alt_buf_init(&s->log);
	s->lfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
	bind(s->lfd, (struct sockaddr *)&sa, sizeof sa);
	listen(s->lfd, 1);
	socklen_t len = sizeof sa;
	getsockname(s->lfd, (struct sockaddr *)&sa, &len);
	snprintf(addr, 32, "127.0.0.1:%d", ntohs(sa.sin_port));
	pthread_t th;
	pthread_create(&th, 0, serve, s);
	return th;
}

static void finish(server *s, pthread_t th) {
	pthread_join(th, 0);
	close(s->lfd);
}

static void session(void) {
	static const char *const replies[] = {
		"250-mx greets you\r\n250-8BITMIME\r\n250-SIZE 1000\r\n250 AUTH PLAIN CRAM-MD5\r\n",
		"334 PDE4OTYuNjk3MTcwOTUyQHBvc3RvZmZpY2UucmVzdG9uLm1jaS5uZXQ+\r\n", "235 ok\r\n", "250 sender ok\r\n",
		"550 5.1.1 unknown\r\n", "251 forwarded\r\n", "354 go\r\n", "250 queued\r\n", "221 bye\r\n", 0};
	server s;
	char addr[32];
	pthread_t th = start(&s, replies, addr);
	alt_smtp_client *c;
	alt_buf err = ALT_BUF_INIT;
	CHECK_OK(alt_smtp_dial(&c, addr, 0, &err));
	alt_str param;
	CHECK(alt_smtp_extension(c, "size", &param));
	CHECK_STR(param, "1000");
	CHECK(!alt_smtp_extension(c, "STARTTLS", 0));
	CHECK_ERR(alt_smtp_hello(c, "late"), -EINVAL);
	CHECK_STR(alt_smtp_error(c), "smtp: Hello called after other methods");
	alt_smtp_auth a = alt_smtp_cram_md5_auth("tim", "tanstaaftanstaaf");
	CHECK_OK(alt_smtp_authenticate(c, &a));
	CHECK_OK(alt_smtp_mail(c, "tim@example.com"));
	CHECK_ERR(alt_smtp_rcpt(c, "nobody@example.com"), -EREMOTEIO);
	CHECK_INT(alt_smtp_code(c), 550);
	CHECK_STR(alt_smtp_error(c), "550 \"5.1.1 unknown\"");
	CHECK_ERR(alt_smtp_rcpt(c, "a\r\nb"), -EINVAL);
	CHECK_STR(alt_smtp_error(c), "smtp: A line must not contain CR or LF");
	CHECK_OK(alt_smtp_rcpt(c, "ann@example.com"));
	alt_writer w;
	CHECK_OK(alt_smtp_data(c, &w));
	CHECK_INT(alt_write_str(w, ALT_STR("Subject: hi\n\n.dot\nend")), 21);
	CHECK_OK(alt_smtp_data_close(c));
	CHECK_OK(alt_smtp_quit(c));
	CHECK_ERR(alt_smtp_noop(c), -ALT_ECLOSED);
	CHECK_ERR(alt_smtp_close(c), -ALT_ECLOSED);
	finish(&s, th);
	CHECK_STR(alt_buf_str(&s.log), "EHLO localhost\r\n"
				      "AUTH CRAM-MD5\r\n"
				      "dGltIGI5MTNhNjAyYzdlZGE3YTQ5NWI0ZTZlNzMzNGQzODkw\r\n" /* "tim b913a602c7eda7a495b4e6e7334d3890" (RFC 2195) */
				      "MAIL FROM:<tim@example.com> BODY=8BITMIME\r\n"
				      "RCPT TO:<nobody@example.com>\r\n"
				      "RCPT TO:<ann@example.com>\r\n"
				      "DATA\r\n"
				      "Subject: hi\r\n\r\n..dot\r\nend\r\n.\r\n"
				      "QUIT\r\n");
	alt_buf_free(&s.log);
	alt_buf_free(&err);
}

static void helo_fallback(void) {
	static const char *const replies[] = {"502 no EHLO\r\n", "250 hi\r\n", "250 ok\r\n", "221 bye\r\n", 0};
	server s;
	char addr[32];
	pthread_t th = start(&s, replies, addr);
	alt_smtp_client *c;
	CHECK_OK(alt_smtp_dial(&c, addr, 0, 0));
	CHECK_OK(alt_smtp_hello(c, "client.example"));
	CHECK_OK(alt_smtp_mail(c, "a@b")); /* no BODY=8BITMIME without EHLO */
	CHECK_OK(alt_smtp_quit(c));
	alt_smtp_close(c);
	finish(&s, th);
	CHECK_STR(alt_buf_str(&s.log), "EHLO client.example\r\nHELO client.example\r\nMAIL FROM:<a@b>\r\nQUIT\r\n");
	alt_buf_free(&s.log);
}

static void plain_rules(void) {
	static const char *const replies[] = {"250-mx\r\n250 AUTH PLAIN\r\n", "221 bye\r\n", 0};
	server s;
	char addr[32];
	pthread_t th = start(&s, replies, addr);
	alt_conn *conn;
	CHECK_OK(alt_dial(&conn, "tcp", addr, 0));
	alt_smtp_client *c;
	CHECK_OK(alt_smtp_new_client(&c, conn, "mail.example.com", 0));
	alt_smtp_auth a = alt_smtp_plain_auth("", "user", "pass", "mail.example.com");
	CHECK_ERR(alt_smtp_authenticate(c, &a), -EACCES);
	CHECK_STR(alt_smtp_error(c), "unencrypted connection");
	alt_smtp_close(c);
	finish(&s, th);
	CHECK_STR(alt_buf_str(&s.log), "EHLO localhost\r\nQUIT\r\n");
	alt_buf_free(&s.log);
}

static void send_mail(void) {
	static const char *const replies[] = {"250-mx\r\n250 AUTH PLAIN\r\n", "235 ok\r\n", "250 ok\r\n", "250 ok\r\n", "250 ok\r\n", "354 go\r\n",
		"250 queued\r\n", "221 bye\r\n", 0};
	server s;
	char addr[32];
	pthread_t th = start(&s, replies, addr);
	alt_smtp_auth a = alt_smtp_plain_auth("", "user", "pass", "127.0.0.1");
	const char *to[] = {"b@example.com", "c@example.com"};
	alt_buf err = ALT_BUF_INIT;
	CHECK_OK(alt_smtp_send_mail(addr, &a, "a@example.com", to, 2, ALT_STR("Subject: x\r\n\r\nbody\r\n"), 0, &err));
	finish(&s, th);
	CHECK_STR(alt_buf_str(&s.log), "EHLO localhost\r\nAUTH PLAIN AHVzZXIAcGFzcw==\r\nMAIL FROM:<a@example.com>\r\n"
				      "RCPT TO:<b@example.com>\r\nRCPT TO:<c@example.com>\r\nDATA\r\nSubject: x\r\n\r\nbody\r\n.\r\nQUIT\r\n");
	alt_buf_free(&s.log);
	const char *bad[] = {"x\ny"};
	CHECK_ERR(alt_smtp_send_mail(addr, 0, "a@b", bad, 1, ALT_STR(""), 0, &err), -EINVAL);
	CHECK_STR(alt_buf_str(&err), "smtp: A line must not contain CR or LF");
	alt_buf_free(&err);
}

int main(void) {
	RUN(session);
	RUN(helo_fallback);
	RUN(plain_rules);
	RUN(send_mail);
	return check_done();
}
