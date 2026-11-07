/* SMTP clients after Go's net/smtp (BSD-3-Clause, see LICENSE-Go): the
   Client with Go's order of commands and its error texts, PLAIN and
   CRAM-MD5 authentication, SendMail. */
#include <alt/arena.h>
#include <alt/crypto/hash.h>
#include <alt/encoding/base64.h>
#include <alt/net/smtp.h>
#include <alt/strconv.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int alt__mime_base64_decode(alt_buf *b, alt_str src, size_t *bad);

typedef struct {
	alt_str k, v;
} ext_entry;

struct alt_smtp_client {
	alt_textproto_conn *text;
	alt_conn *conn;          /* the connection (owned by tlsc after STARTTLS) */
	alt_tls_conn *tlsc;
	bool tls;
	bool closed;
	char *server_name;
	char *local_name;
	bool did_hello;
	int hello_rc;
	alt_buf hello_err;
	/* the extensions of the last EHLO (has_ext false after HELO) */
	bool has_ext;
	alt_arena ext_arena;
	ext_entry *ext;
	size_t next;
	/* the AUTH mechanisms (kept when a later EHLO has none, as Go) */
	alt_arena auth_arena;
	alt_str *auth;
	size_t nauth;
	alt_buf err;
	int code;
};

static int set_err(alt_smtp_client *c, int rc, const char *msg) {
	alt_buf_reset(&c->err);
	alt_buf_append_cstr(&c->err, msg);
	return rc;
}

/* the text of a code returned by the connection or the textproto layer */
static int conn_err(alt_smtp_client *c, int rc) {
	if (c->text && (rc == -EREMOTEIO || rc == -EPROTO || rc == -ALT_EEOF || rc == -ALT_EUNEXPECTED_EOF || rc == -ALT_ETOOLARGE)) {
		if (rc == -EREMOTEIO)
			c->code = c->text->r.code;
		return set_err(c, rc, alt_textproto_error(&c->text->r));
	}
	if (rc == -ALT_ECLOSED || c->closed)
		return set_err(c, -ALT_ECLOSED, "use of closed network connection");
	if (c->tlsc && *alt_tls_error(c->tlsc))
		return set_err(c, rc, alt_tls_error(c->tlsc));
	return set_err(c, rc, alt_strerror(rc));
}

/* a system error of a read or write: Go's os.SyscallError text ("write: broken pipe") */
static int sys_err(alt_smtp_client *c, int rc, const char *op) {
	if (rc >= 0 || rc <= -ALT_EBASE || rc == -EREMOTEIO || rc == -EPROTO || rc == -ENOMEM || (c->tlsc && *alt_tls_error(c->tlsc)) ||
		c->closed)
		return conn_err(c, rc);
	alt_buf_reset(&c->err);
	alt_buf_printf(&c->err, "%s: %s", op, alt_strerror(rc));
	for (size_t i = strlen(op) + 2; i < c->err.n; i++)
		if (c->err.p[i] >= 'A' && c->err.p[i] <= 'Z')
			c->err.p[i] += 32;
	return rc;
}

static int new_text(alt_smtp_client *c, alt_reader r, alt_writer w) {
	alt_textproto_conn *t;
	int rc = alt_textproto_conn_new(&t, r, w);
	if (rc < 0)
		return rc;
	if (c->text)
		alt_textproto_close(c->text);
	c->text = t;
	return 0;
}

/* cmd: sends a line, reads the reply; msg may be NULL */
static int vcmd(alt_smtp_client *c, int expect, int *code, alt_buf *msg, const char *fmt, va_list ap) {
	alt_buf m = ALT_BUF_INIT;
	int cd = 0;
	if (c->closed) {
		if (code)
			*code = 0;
		return set_err(c, -ALT_ECLOSED, "use of closed network connection");
	}
	unsigned id;
	int rc = alt_textproto_vcmd(c->text, &id, fmt, ap);
	if (rc < 0) {
		if (code)
			*code = 0;
		return sys_err(c, rc, "write");
	}
	alt_textproto_start_response(&c->text->p, id);
	rc = alt_textproto_read_response(&c->text->r, expect, &cd, &m);
	alt_textproto_end_response(&c->text->p, id);
	if (code)
		*code = cd;
	if (msg) {
		alt_buf_reset(msg);
		alt_buf_append_str(msg, alt_buf_str(&m));
	}
	alt_buf_free(&m);
	return rc < 0 ? sys_err(c, rc, "read") : 0;
}

__attribute__((format(printf, 5, 6))) static int cmd(alt_smtp_client *c, int expect, int *code, alt_buf *msg, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int rc = vcmd(c, expect, code, msg, fmt, ap);
	va_end(ap);
	return rc;
}

static int validate_line(alt_smtp_client *c, const char *line, alt_buf *err) {
	if (strpbrk(line, "\r\n")) {
		static const char msg[] = "smtp: A line must not contain CR or LF";
		if (c)
			set_err(c, -EINVAL, msg);
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_cstr(err, msg);
		}
		return -EINVAL;
	}
	return 0;
}

static int helo(alt_smtp_client *c) {
	c->has_ext = false;
	return cmd(c, 250, 0, 0, "HELO %s", c->local_name);
}

static int ehlo(alt_smtp_client *c) {
	alt_buf msg = ALT_BUF_INIT;
	int rc = cmd(c, 250, 0, &msg, "EHLO %s", c->local_name);
	if (rc < 0) {
		alt_buf_free(&msg);
		return rc;
	}
	alt_arena_reset(&c->ext_arena);
	c->ext = 0;
	c->next = 0;
	alt_str rest = alt_buf_str(&msg), line;
	size_t nlines = alt_str_count(rest, ALT_STR("\n")) + 1;
	if (nlines > 1) {
		c->ext = alt_arena_alloc(&c->ext_arena, (nlines - 1) * sizeof *c->ext);
		if (!c->ext) {
			alt_buf_free(&msg);
			return set_err(c, -ENOMEM, alt_strerror(-ENOMEM));
		}
		bool first = true;
		while (alt_str_split_next(&rest, ALT_STR("\n"), &line)) {
			if (first) {
				first = false;
				continue;
			}
			alt_str k, v;
			alt_str_cut_byte(line, ' ', &k, &v);
			c->ext[c->next++] = (ext_entry){alt_arena_str(&c->ext_arena, k), alt_arena_str(&c->ext_arena, v)};
		}
	}
	/* AUTH: the last one wins, as in Go's map */
	for (size_t i = c->next; i-- > 0;)
		if (alt_str_eq_cstr(c->ext[i].k, "AUTH")) {
			alt_arena_reset(&c->auth_arena);
			alt_str mechs = c->ext[i].v, f;
			size_t n = alt_str_count(mechs, ALT_STR(" ")) + 1;
			c->auth = alt_arena_alloc(&c->auth_arena, n * sizeof *c->auth);
			c->nauth = 0;
			while (c->auth && alt_str_split_next(&mechs, ALT_STR(" "), &f))
				c->auth[c->nauth++] = alt_arena_str(&c->auth_arena, f);
			break;
		}
	c->has_ext = true;
	alt_buf_free(&msg);
	return 0;
}

static int hello(alt_smtp_client *c) {
	if (!c->did_hello) {
		c->did_hello = true;
		int rc = ehlo(c);
		if (rc < 0) {
			c->hello_rc = helo(c);
			alt_buf_reset(&c->hello_err);
			if (c->hello_rc < 0)
				alt_buf_append_str(&c->hello_err, alt_buf_str(&c->err));
		}
	}
	if (c->hello_rc < 0)
		set_err(c, c->hello_rc, alt_buf_cstr(&c->hello_err));
	return c->hello_rc;
}

static void client_free(alt_smtp_client *c) {
	if (c->text)
		alt_textproto_close(c->text);
	if (!c->closed) {
		if (c->tlsc)
			alt_tls_close(c->tlsc);
		else if (c->conn)
			alt_conn_close(c->conn);
	}
	free(c->server_name);
	free(c->local_name);
	alt_buf_free(&c->hello_err);
	alt_buf_free(&c->err);
	alt_arena_free(&c->ext_arena);
	alt_arena_free(&c->auth_arena);
	free(c);
}

static int new_client(alt_smtp_client **out, alt_conn *conn, alt_tls_conn *t, const char *host, alt_buf *err) {
	alt_smtp_client *c = calloc(1, sizeof *c);
	if (!c || !(c->server_name = strdup(host ? host : "")) || !(c->local_name = strdup("localhost"))) {
		if (c) {
			free(c->server_name);
			free(c);
		}
		if (t)
			alt_tls_close(t);
		else
			alt_conn_close(conn);
		return -ENOMEM;
	}
	c->conn = conn;
	c->tlsc = t;
	c->tls = t != 0;
	alt_arena_init(&c->ext_arena, 0);
	alt_arena_init(&c->auth_arena, 0);
	int rc = t ? new_text(c, alt_tls_reader(t), alt_tls_writer(t)) : new_text(c, alt_conn_reader(conn), alt_conn_writer(conn));
	if (rc == 0) {
		int code;
		alt_buf msg = ALT_BUF_INIT;
		rc = alt_textproto_read_response(&c->text->r, 220, &code, &msg);
		alt_buf_free(&msg);
		if (rc < 0)
			conn_err(c, rc);
	} else
		set_err(c, rc, alt_strerror(rc));
	if (rc < 0) {
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_str(err, alt_buf_str(&c->err));
		}
		client_free(c);
		return rc;
	}
	*out = c;
	return 0;
}

int alt_smtp_new_client(alt_smtp_client **out, alt_conn *conn, const char *host, alt_buf *err) {
	return new_client(out, conn, 0, host, err);
}

int alt_smtp_new_client_tls(alt_smtp_client **out, alt_tls_conn *t, const char *host, alt_buf *err) {
	return new_client(out, alt_tls_net_conn(t), t, host, err);
}

int alt_smtp_dial(alt_smtp_client **out, const char *addr, alt_ctx *ctx, alt_buf *err) {
	alt_conn *conn;
	int rc = alt_dial(&conn, "tcp", addr, ctx);
	if (rc < 0) {
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_cstr(err, alt_strerror(rc));
		}
		return rc;
	}
	alt_str host = ALT_STR(""), port;
	if (alt_split_host_port(alt_str_cstr(addr), &host, &port) < 0)
		host = ALT_STR("");
	char *h = alt_str_dup(host);
	if (!h) {
		alt_conn_close(conn);
		return -ENOMEM;
	}
	rc = new_client(out, conn, 0, h, err);
	free(h);
	return rc;
}

const char *alt_smtp_error(const alt_smtp_client *c) { return alt_buf_cstr(&c->err); }
int alt_smtp_code(const alt_smtp_client *c) { return c->code; }

int alt_smtp_hello(alt_smtp_client *c, const char *local_name) {
	int rc = validate_line(c, local_name, 0);
	if (rc < 0)
		return rc;
	if (c->did_hello)
		return set_err(c, -EINVAL, "smtp: Hello called after other methods");
	char *n = strdup(local_name);
	if (!n)
		return set_err(c, -ENOMEM, alt_strerror(-ENOMEM));
	free(c->local_name);
	c->local_name = n;
	return hello(c);
}

int alt_smtp_starttls(alt_smtp_client *c, const alt_tls_config *cfg) {
	int rc = hello(c);
	if (rc < 0)
		return rc;
	rc = cmd(c, 220, 0, 0, "STARTTLS");
	if (rc < 0)
		return rc;
	alt_tls_conn *t;
	rc = alt_tls_client(&t, c->conn, cfg);
	if (rc < 0)
		return set_err(c, rc, alt_strerror(rc));
	c->tlsc = t;
	rc = new_text(c, alt_tls_reader(t), alt_tls_writer(t));
	if (rc < 0)
		return set_err(c, rc, alt_strerror(rc));
	c->tls = true;
	return ehlo(c);
}

bool alt_smtp_tls_state(const alt_smtp_client *c, alt_tls_state *s) {
	if (!c->tlsc)
		return false;
	alt_tls_connection_state(c->tlsc, s);
	return true;
}

bool alt_smtp_extension(alt_smtp_client *c, const char *ext, alt_str *param) {
	if (param)
		*param = ALT_STR("");
	if (hello(c) < 0 || !c->has_ext)
		return false;
	for (size_t i = c->next; i-- > 0;) {
		alt_str k = c->ext[i].k;
		bool eq = k.n == strlen(ext);
		for (size_t j = 0; eq && j < k.n; j++) {
			char e = ext[j];
			if (e >= 'a' && e <= 'z')
				e -= 32;
			eq = k.p[j] == e;
		}
		if (eq) {
			if (param)
				*param = c->ext[i].v;
			return true;
		}
	}
	return false;
}

int alt_smtp_verify(alt_smtp_client *c, const char *addr) {
	int rc = validate_line(c, addr, 0);
	if (rc < 0 || (rc = hello(c)) < 0)
		return rc;
	return cmd(c, 250, 0, 0, "VRFY %s", addr);
}

int alt_smtp_authenticate(alt_smtp_client *c, alt_smtp_auth *a) {
	int rc = hello(c);
	if (rc < 0)
		return rc;
	alt_buf proto = ALT_BUF_INIT, resp = ALT_BUF_INIT, err = ALT_BUF_INIT, line = ALT_BUF_INIT, msg64 = ALT_BUF_INIT, msg = ALT_BUF_INIT;
	alt_smtp_server_info info = {c->server_name, c->tls, c->auth, c->nauth};
	rc = a->start(a, &info, &proto, &resp, &err);
	if (rc < 0) {
		alt_smtp_quit(c);
		set_err(c, rc, alt_buf_cstr(&err));
		goto out;
	}
	alt_buf_printf(&line, "AUTH %s ", alt_buf_cstr(&proto));
	alt_base64_encode(&alt_base64_std, &line, resp.p, resp.n);
	int code;
	alt_str first = alt_str_trim_space(alt_buf_str(&line));
	rc = cmd(c, 0, &code, &msg64, "%.*s", (int)first.n, first.p);
	while (rc == 0) {
		alt_buf_reset(&msg);
		alt_buf_reset(&err);
		if (code == 334) {
			size_t bad = 0;
			if (alt__mime_base64_decode(&msg, alt_buf_str(&msg64), &bad) < 0) {
				rc = -ALT_EENC_BASE64;
				alt_buf_printf(&err, "illegal base64 data at input byte %zu", bad);
			}
		} else if (code == 235)
			alt_buf_append_str(&msg, alt_buf_str(&msg64));
		else {
			rc = -EREMOTEIO;
			c->code = code;
			alt_buf_printf(&err, "%03d ", code); /* textproto.Error */
			alt_quote(&err, alt_buf_str(&msg64));
		}
		int has = 0;
		if (rc == 0) {
			alt_buf_reset(&resp);
			has = a->next(a, alt_buf_str(&msg), code == 334, &resp, &err);
			if (has < 0)
				rc = has;
		}
		if (rc < 0) {
			cmd(c, 501, 0, 0, "*");
			alt_smtp_quit(c);
			set_err(c, rc, alt_buf_cstr(&err));
			if (rc == -EREMOTEIO)
				c->code = code;
			break;
		}
		if (!has)
			break;
		alt_buf_reset(&line);
		alt_base64_encode(&alt_base64_std, &line, resp.p, resp.n);
		rc = cmd(c, 0, &code, &msg64, "%s", alt_buf_cstr(&line));
	}
out:
	alt_buf_free(&proto);
	alt_buf_free(&resp);
	alt_buf_free(&err);
	alt_buf_free(&line);
	alt_buf_free(&msg64);
	alt_buf_free(&msg);
	return rc;
}

int alt_smtp_mail(alt_smtp_client *c, const char *from) {
	int rc = validate_line(c, from, 0);
	if (rc < 0 || (rc = hello(c)) < 0)
		return rc;
	bool eight = false, utf8 = false;
	if (c->has_ext) {
		eight = alt_smtp_extension(c, "8BITMIME", 0);
		utf8 = alt_smtp_extension(c, "SMTPUTF8", 0);
	}
	return cmd(c, 250, 0, 0, "MAIL FROM:<%s>%s%s", from, eight ? " BODY=8BITMIME" : "", utf8 ? " SMTPUTF8" : "");
}

int alt_smtp_rcpt(alt_smtp_client *c, const char *to) {
	int rc = validate_line(c, to, 0);
	if (rc < 0)
		return rc;
	return cmd(c, 25, 0, 0, "RCPT TO:<%s>", to);
}

int alt_smtp_data(alt_smtp_client *c, alt_writer *w) {
	int rc = cmd(c, 354, 0, 0, "DATA");
	if (rc < 0)
		return rc;
	*w = alt_textproto_dot_writer(&c->text->w);
	return 0;
}

int alt_smtp_data_close(alt_smtp_client *c) {
	if (c->closed)
		return set_err(c, -ALT_ECLOSED, "use of closed network connection");
	alt_textproto_dot_close(&c->text->w);
	int code;
	alt_buf msg = ALT_BUF_INIT;
	int rc = alt_textproto_read_response(&c->text->r, 250, &code, &msg);
	alt_buf_free(&msg);
	return rc < 0 ? sys_err(c, rc, "read") : 0;
}

int alt_smtp_reset(alt_smtp_client *c) {
	int rc = hello(c);
	if (rc < 0)
		return rc;
	return cmd(c, 250, 0, 0, "RSET");
}

int alt_smtp_noop(alt_smtp_client *c) {
	int rc = hello(c);
	if (rc < 0)
		return rc;
	return cmd(c, 250, 0, 0, "NOOP");
}

/* Text.Close: closes the connection, keeps the client */
static int close_conn(alt_smtp_client *c) {
	if (c->closed)
		return set_err(c, -ALT_ECLOSED, "use of closed network connection");
	c->closed = true;
	int rc = c->tlsc ? alt_tls_close(c->tlsc) : alt_conn_close(c->conn);
	c->tlsc = 0;
	c->conn = 0;
	return rc;
}

int alt_smtp_quit(alt_smtp_client *c) {
	hello(c);
	int rc = cmd(c, 221, 0, 0, "QUIT");
	if (rc < 0)
		return rc;
	return close_conn(c);
}

int alt_smtp_close(alt_smtp_client *c) {
	if (!c)
		return 0;
	int rc = c->closed ? -ALT_ECLOSED : close_conn(c);
	client_free(c);
	return rc;
}

/* ---- mechanisms ---- */

static bool is_localhost(const char *name) { return !strcmp(name, "localhost") || !strcmp(name, "127.0.0.1") || !strcmp(name, "::1"); }

static int err_text(alt_buf *err, const char *msg) {
	if (err) {
		alt_buf_reset(err);
		alt_buf_append_cstr(err, msg);
	}
	return -EACCES;
}

static int plain_start(alt_smtp_auth *a, const alt_smtp_server_info *info, alt_buf *proto, alt_buf *resp, alt_buf *err) {
	if (!info->tls && !is_localhost(info->name))
		return err_text(err, "unencrypted connection");
	if (strcmp(info->name, a->s[3]))
		return err_text(err, "wrong host name");
	alt_buf_append_cstr(proto, "PLAIN");
	alt_buf_append_cstr(resp, a->s[0]);
	alt_buf_append_byte(resp, 0);
	alt_buf_append_cstr(resp, a->s[1]);
	alt_buf_append_byte(resp, 0);
	alt_buf_append_cstr(resp, a->s[2]);
	return resp->err;
}

static int plain_next(alt_smtp_auth *a, alt_str from_server, bool more, alt_buf *resp, alt_buf *err) {
	if (more)
		return err_text(err, "unexpected server challenge");
	return 0;
}

alt_smtp_auth alt_smtp_plain_auth(const char *identity, const char *username, const char *password, const char *host) {
	return (alt_smtp_auth){plain_start, plain_next, {identity, username, password, host}, 0};
}

static int cram_start(alt_smtp_auth *a, const alt_smtp_server_info *info, alt_buf *proto, alt_buf *resp, alt_buf *err) {
	return alt_buf_append_cstr(proto, "CRAM-MD5");
}

static int cram_next(alt_smtp_auth *a, alt_str from_server, bool more, alt_buf *resp, alt_buf *err) {
	if (!more)
		return 0;
	uint8_t sum[16];
	alt_hmac_data(ALT_MD5, a->s[1], strlen(a->s[1]), from_server.p, from_server.n, sum);
	alt_buf_append_cstr(resp, a->s[0]);
	alt_buf_append_byte(resp, ' ');
	for (int i = 0; i < 16; i++)
		alt_buf_printf(resp, "%02x", sum[i]);
	return resp->err ? resp->err : 1;
}

alt_smtp_auth alt_smtp_cram_md5_auth(const char *username, const char *secret) {
	return (alt_smtp_auth){cram_start, cram_next, {username, secret, 0, 0}, 0};
}

/* ---- SendMail ---- */

int alt_smtp_send_mail(const char *addr, alt_smtp_auth *a, const char *from, const char *const *to, size_t nto, alt_str msg,
	const alt_tls_config *tls, alt_buf *err) {
	int rc = validate_line(0, from, err);
	for (size_t i = 0; rc == 0 && i < nto; i++)
		rc = validate_line(0, to[i], err);
	if (rc < 0)
		return rc;
	alt_smtp_client *c;
	rc = alt_smtp_dial(&c, addr, 0, err);
	if (rc < 0)
		return rc;
	if ((rc = hello(c)) < 0)
		goto out;
	if (alt_smtp_extension(c, "STARTTLS", 0)) {
		alt_tls_config cfg = tls ? *tls : (alt_tls_config){0};
		if (!cfg.server_name)
			cfg.server_name = c->server_name;
		if ((rc = alt_smtp_starttls(c, &cfg)) < 0)
			goto out;
	}
	if (a && c->has_ext) {
		bool has_auth = false;
		for (size_t i = 0; i < c->next; i++)
			has_auth |= alt_str_eq_cstr(c->ext[i].k, "AUTH");
		if (!has_auth) {
			rc = set_err(c, -EOPNOTSUPP, "smtp: server doesn't support AUTH");
			goto out;
		}
		if ((rc = alt_smtp_authenticate(c, a)) < 0)
			goto out;
	}
	if ((rc = alt_smtp_mail(c, from)) < 0)
		goto out;
	for (size_t i = 0; i < nto; i++)
		if ((rc = alt_smtp_rcpt(c, to[i])) < 0)
			goto out;
	alt_writer w;
	if ((rc = alt_smtp_data(c, &w)) < 0)
		goto out;
	ssize_t k = alt_write_all(w, msg.p, msg.n);
	if (k < 0) {
		rc = conn_err(c, (int)k);
		goto out;
	}
	if ((rc = alt_smtp_data_close(c)) < 0)
		goto out;
	rc = alt_smtp_quit(c);
out:
	if (rc < 0 && err) {
		alt_buf_reset(err);
		alt_buf_append_str(err, alt_buf_str(&c->err));
	}
	alt_smtp_close(c);
	return rc;
}
