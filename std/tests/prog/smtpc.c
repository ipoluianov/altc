/* An SMTP client of the library for tests/stdsmtp_test.go:
	smtpc addr ca.pem < ops
   runs the operations of stdin (one per line, fields separated by tabs,
   strings in hex) and prints one line each, as the Go side does with
   net/smtp: "<op>: ok", "<op>: err <text>", "ext: <bool> <param>"...
   Operations: dial; connect host; hello name; ext name; starttls name;
   tlsstate; plain identity user password host; cram user secret;
   mail from; rcpt to; data body; rset; noop; vrfy addr; quit; close;
   sendmail auth from to,to,... msg (auth: none, plain:id:user:pass:host
   or cram:user:secret) */
#include "../diff/common.h"
#include <alt/net/smtp.h>

static alt_x509_pool *roots;

static char *str(char *s) {
	s[diff_unhex(s)] = 0;
	return s;
}

static void result(const char *op, int rc, alt_smtp_client *c) {
	if (rc < 0)
		printf("%s: err %s\n", op, alt_smtp_error(c));
	else
		printf("%s: ok\n", op);
}

/* "plain:id:user:pass:host", "cram:user:secret" (hex parts) */
static bool parse_auth(char *spec, alt_smtp_auth *a) {
	char *parts[6];
	int n = 0;
	for (char *save = 0, *t = strtok_r(spec, ":", &save); t && n < 6; t = strtok_r(0, ":", &save))
		parts[n++] = t;
	/* empty parts vanish in strtok: they are written as "-" */
	for (int i = 1; i < n; i++)
		if (!strcmp(parts[i], "-"))
			parts[i][0] = 0;
		else
			str(parts[i]);
	if (n == 5 && !strcmp(parts[0], "plain"))
		*a = alt_smtp_plain_auth(parts[1], parts[2], parts[3], parts[4]);
	else if (n == 3 && !strcmp(parts[0], "cram"))
		*a = alt_smtp_cram_md5_auth(parts[1], parts[2]);
	else
		return false;
	return true;
}

int main(int argc, char **argv) {
	const char *addr = argv[1];
	roots = alt_x509_pool_new();
	FILE *f = fopen(argv[2], "rb");
	static char pem[65536];
	size_t pn = f ? fread(pem, 1, sizeof pem, f) : 0;
	if (f)
		fclose(f);
	alt_x509_pool_add_pem(roots, pem, pn);
	alt_smtp_client *c = 0;
	char *line, *fl[16];
	static char strs[4][4096];
	while ((line = diff_line())) {
		int n = diff_fields(line, fl, 16);
		const char *op = fl[0];
		alt_buf err = ALT_BUF_INIT;
		int rc = 0;
		if (!strcmp(op, "dial") || !strcmp(op, "connect")) {
			if (op[0] == 'd')
				rc = alt_smtp_dial(&c, addr, 0, &err);
			else {
				alt_conn *conn;
				rc = alt_dial(&conn, "tcp", addr, 0);
				if (rc < 0)
					alt_buf_append_cstr(&err, alt_strerror(rc));
				else
					rc = alt_smtp_new_client(&c, conn, str(fl[1]), &err);
			}
			if (rc < 0) {
				c = 0;
				printf("%s: err %s\n", op, alt_buf_cstr(&err));
			} else
				printf("%s: ok\n", op);
		} else if (!strcmp(op, "sendmail")) {
			alt_smtp_auth a, *ap = 0;
			if (strcmp(fl[1], "none")) {
				snprintf(strs[0], sizeof strs[0], "%s", fl[1]);
				if (parse_auth(strs[0], &a))
					ap = &a;
			}
			const char *to[16];
			size_t nto = 0;
			for (char *save = 0, *t = strtok_r(fl[3], ",", &save); t && nto < 16; t = strtok_r(0, ",", &save))
				to[nto++] = !strcmp(t, "-") ? "" : str(t);
			char *msg = fl[4];
			size_t mn = diff_unhex(msg);
			rc = alt_smtp_send_mail(addr, ap, str(fl[2]), to, nto, alt_str_make(msg, mn), 0, &err);
			if (rc < 0)
				printf("sendmail: err %s\n", alt_buf_cstr(&err));
			else
				printf("sendmail: ok\n");
		} else if (!c) {
			printf("%s: no client\n", op);
		} else if (!strcmp(op, "hello"))
			result(op, alt_smtp_hello(c, str(fl[1])), c);
		else if (!strcmp(op, "ext")) {
			alt_str param;
			bool ok = alt_smtp_extension(c, str(fl[1]), &param);
			printf("ext: %s %.*s\n", ok ? "true" : "false", (int)param.n, param.p);
		} else if (!strcmp(op, "starttls")) {
			/* the configuration's strings must outlive the connection */
			alt_tls_config cfg = {.server_name = strdup(str(fl[1])), .roots = roots};
			result(op, alt_smtp_starttls(c, &cfg), c);
		} else if (!strcmp(op, "tlsstate")) {
			alt_tls_state st;
			bool ok = alt_smtp_tls_state(c, &st);
			printf("tlsstate: %s %s\n", ok ? "true" : "false", ok ? st.server_name : "");
		} else if (!strcmp(op, "plain") || !strcmp(op, "cram")) {
			for (int i = 1; i < n && i <= 4; i++)
				snprintf(strs[i - 1], sizeof strs[0], "%s", str(fl[i]));
			alt_smtp_auth a =
				op[0] == 'p' ? alt_smtp_plain_auth(strs[0], strs[1], strs[2], strs[3]) : alt_smtp_cram_md5_auth(strs[0], strs[1]);
			result(op, alt_smtp_authenticate(c, &a), c);
		} else if (!strcmp(op, "mail"))
			result(op, alt_smtp_mail(c, str(fl[1])), c);
		else if (!strcmp(op, "rcpt"))
			result(op, alt_smtp_rcpt(c, str(fl[1])), c);
		else if (!strcmp(op, "vrfy"))
			result(op, alt_smtp_verify(c, str(fl[1])), c);
		else if (!strcmp(op, "data")) {
			alt_writer w;
			rc = alt_smtp_data(c, &w);
			result("data", rc, c);
			if (rc == 0) {
				size_t bn = diff_unhex(fl[1]);
				ssize_t k = alt_write_all(w, fl[1], bn);
				if (k < 0)
					printf("write: err %s\n", alt_strerror((int)k));
				result("dataclose", alt_smtp_data_close(c), c);
			}
		} else if (!strcmp(op, "rset"))
			result(op, alt_smtp_reset(c), c);
		else if (!strcmp(op, "noop"))
			result(op, alt_smtp_noop(c), c);
		else if (!strcmp(op, "quit"))
			result(op, alt_smtp_quit(c), c);
		else if (!strcmp(op, "close")) {
			rc = alt_smtp_close(c);
			c = 0;
			printf("close: %s\n", rc < 0 ? "err" : "ok");
		} else
			printf("%s: unknown\n", op);
		alt_buf_free(&err);
		fflush(stdout);
	}
	if (c)
		alt_smtp_close(c);
	return 0;
}
