/* AltC standard library: SMTP clients (Go's net/smtp, RFC 5321).

	alt_smtp_client *c;
	alt_smtp_dial(&c, "mail.example.com:25", ctx, &err);
	alt_smtp_starttls(c, &(alt_tls_config){.server_name = "mail.example.com"});
	alt_smtp_auth a = alt_smtp_plain_auth("", "user", "secret", "mail.example.com");
	alt_smtp_authenticate(c, &a);
	alt_smtp_mail(c, "me@example.com");
	alt_smtp_rcpt(c, "you@example.com");
	alt_writer w;
	alt_smtp_data(c, &w);
	alt_write_all(w, msg, n);
	alt_smtp_data_close(c);
	alt_smtp_quit(c);
	alt_smtp_close(c);

   Every call but Rcpt and Data says EHLO first if that has not been done
   (HELO when EHLO fails), as Go does. Errors are negative codes with
   Go's text in alt_smtp_error(c):
     -EREMOTEIO       an unexpected reply ("550 \"5.1.1 no such user\"",
                      the code in alt_smtp_code(c))
     -EPROTO          a malformed reply
     -ALT_EEOF        the server closed the connection ("EOF")
     -EINVAL          "smtp: A line must not contain CR or LF",
                      "smtp: Hello called after other methods"
     -EOPNOTSUPP      "smtp: server doesn't support AUTH" (SendMail)
     -EACCES          an authentication mechanism refused to go on
                      ("unencrypted connection", "wrong host name"...)
     -ALT_EENC_BASE64 a malformed challenge ("illegal base64 data at input byte 4")
     -ALT_ECLOSED     the connection was closed (by Quit)
   and the errors of the connection and of TLS (with alt_tls_error's text). */
#ifndef ALT_NET_SMTP_H
#define ALT_NET_SMTP_H

#include <alt/context.h>
#include <alt/crypto/tls.h>
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/net.h>
#include <alt/net/textproto.h>
#include <alt/str.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ServerInfo */
typedef struct {
	const char *name;          /* the server name */
	bool tls;                  /* the connection uses TLS */
	const alt_str *auth;       /* the mechanisms advertised */
	size_t nauth;
} alt_smtp_server_info;

/* an authentication mechanism (smtp.Auth) */
typedef struct alt_smtp_auth {
	/* Start: the mechanism name into proto and the initial response into
	   resp (may stay empty): 0, or a negative code with Go's text in err */
	int (*start)(struct alt_smtp_auth *a, const alt_smtp_server_info *info, alt_buf *proto, alt_buf *resp, alt_buf *err);
	/* Next: after a reply (more: a 334 challenge, decoded; else the 235
	   text): 1 with resp to send (possibly empty), 0 when done, or a
	   negative code with Go's text in err */
	int (*next)(struct alt_smtp_auth *a, alt_str from_server, bool more, alt_buf *resp, alt_buf *err);
	const char *s[4];          /* the mechanism's strings */
	void *data;                /* for other mechanisms */
} alt_smtp_auth;

/* PlainAuth (RFC 4616): only over TLS or to localhost, 127.0.0.1, ::1,
   and only to host; the strings must outlive the use */
alt_smtp_auth alt_smtp_plain_auth(const char *identity, const char *username, const char *password, const char *host);
/* CRAMMD5Auth (RFC 2195) */
alt_smtp_auth alt_smtp_cram_md5_auth(const char *username, const char *secret);

typedef struct alt_smtp_client alt_smtp_client;

/* Dial: connects to addr ("host:port") and reads the greeting; err (may
   be NULL) gets the text of a failure */
int alt_smtp_dial(alt_smtp_client **out, const char *addr, alt_ctx *ctx, alt_buf *err);
/* NewClient over a connection (owned from now on, closed on failure too);
   host is the server name for authentication */
int alt_smtp_new_client(alt_smtp_client **out, alt_conn *conn, const char *host, alt_buf *err);
/* over a TLS connection (implicit TLS, port 465) */
int alt_smtp_new_client_tls(alt_smtp_client **out, alt_tls_conn *t, const char *host, alt_buf *err);

/* the text of the last error, and the code of the last unexpected reply */
const char *alt_smtp_error(const alt_smtp_client *c);
int alt_smtp_code(const alt_smtp_client *c);

/* Hello: the name sent in EHLO/HELO (default "localhost"), before any other call */
int alt_smtp_hello(alt_smtp_client *c, const char *local_name);
/* StartTLS: STARTTLS, then the handshake and EHLO again over TLS (cfg
   and its strings must stay valid while the client is used) */
int alt_smtp_starttls(alt_smtp_client *c, const alt_tls_config *cfg);
/* TLSConnectionState: false without TLS */
bool alt_smtp_tls_state(const alt_smtp_client *c, alt_tls_state *s);
/* Extension: whether the server supports ext (any case), its parameter in *param */
bool alt_smtp_extension(alt_smtp_client *c, const char *ext, alt_str *param);
/* Auth */
int alt_smtp_authenticate(alt_smtp_client *c, alt_smtp_auth *a);
/* Verify: VRFY */
int alt_smtp_verify(alt_smtp_client *c, const char *addr);
/* Mail: MAIL FROM:<from>, with BODY=8BITMIME and SMTPUTF8 when supported */
int alt_smtp_mail(alt_smtp_client *c, const char *from);
/* Rcpt: RCPT TO:<to> (25x replies accepted) */
int alt_smtp_rcpt(alt_smtp_client *c, const char *to);
/* Data: DATA; *w takes the message (dot-encoded on the way), and
   alt_smtp_data_close ends it and reads the reply */
int alt_smtp_data(alt_smtp_client *c, alt_writer *w);
int alt_smtp_data_close(alt_smtp_client *c);
int alt_smtp_reset(alt_smtp_client *c);
int alt_smtp_noop(alt_smtp_client *c);
/* Quit: QUIT, then closes the connection */
int alt_smtp_quit(alt_smtp_client *c);
/* closes the connection (-ALT_ECLOSED when Quit did) and frees c */
int alt_smtp_close(alt_smtp_client *c);

/* SendMail: connects to addr, uses STARTTLS when offered (tls: the
   configuration, NULL for the defaults; its server_name defaults to the
   host of addr), authenticates with a (may be NULL) when the server
   supports AUTH, and sends msg from from to the recipients */
int alt_smtp_send_mail(const char *addr, alt_smtp_auth *a, const char *from, const char *const *to, size_t nto, alt_str msg,
	const alt_tls_config *tls, alt_buf *err);

#ifdef __cplusplus
}
#endif

#endif
