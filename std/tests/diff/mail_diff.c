/* addr flag s -> "ok name|address|String()|reparsed" or "err <msg>"
   list flag s -> "ok " name|address|String(); ... or "err <msg>"
   date s -> "ok <unix sec> <offset> <zone>" or "err <msg>"
   fmt name address -> String()
   msg m -> the header (keys sorted), the body, Date(), AddressList("From"), AddressList("to")
   flag 1: an address parser with a charset reader ("x-upper", "x-fail")
   (strings in hex) */
#include "common.h"
#include <alt/net/mail.h>
#include <ctype.h>
#include <errno.h>

static alt_str arg(char *s) { return alt_str_make(s, diff_unhex(s)); }
static void hex(alt_str s) { diff_puthex(s.p, s.n); }

static int reader(void *arg, alt_str charset, alt_str content, alt_buf *out, alt_buf *err) {
	(void)arg;
	if (alt_str_eq_cstr(charset, "x-upper")) {
		for (size_t i = 0; i < content.n; i++)
			alt_buf_append_byte(out, toupper((unsigned char)content.p[i]));
		return 0;
	}
	alt_buf_reset(err);
	if (alt_str_eq_cstr(charset, "x-fail"))
		alt_buf_append_cstr(err, "boom");
	else
		alt_buf_printf(err, "unknown charset %.*s", (int)charset.n, charset.p);
	return -EINVAL;
}

static const alt_mime_word_decoder dec = {reader, 0};
static const alt_mail_address_parser with_dec = {&dec};

static void put_addr(const alt_mail_address *a) {
	alt_buf s = ALT_BUF_INIT;
	hex(a->name);
	printf("|");
	hex(a->address);
	printf("|");
	alt_mail_address_format(&s, a);
	hex(alt_buf_str(&s));
	alt_buf_free(&s);
}

static void put_list(int rc, alt_mail_address *v, size_t n, alt_buf *err) {
	if (rc < 0) {
		printf("err %s", alt_buf_cstr(err));
		return;
	}
	printf("ok");
	for (size_t i = 0; i < n; i++) {
		printf(i ? ";" : " ");
		put_addr(&v[i]);
	}
}

static void put_date(int rc, alt_wall t, const alt_zone *z, alt_buf *err) {
	if (rc < 0)
		printf("err %s", alt_buf_cstr(err));
	else
		printf("ok %lld %lld %d %s", (long long)alt_wall_unix(t), (long long)(t - alt_wall_unix(t) * ALT_SEC), z->offset, z->name);
}

static void sort_fields(alt_textproto_field *f, size_t n) {
	for (size_t i = 1; i < n; i++)
		for (size_t j = i; j > 0 && alt_str_cmp(f[j - 1].key, f[j].key) > 0; j--) {
			alt_textproto_field t = f[j];
			f[j] = f[j - 1];
			f[j - 1] = t;
		}
}

int main(void) {
	char *line, *f[8];
	while ((line = diff_line())) {
		diff_fields(line, f, 8);
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_buf err = ALT_BUF_INIT;
		if (!strcmp(f[0], "addr")) {
			const alt_mail_address_parser *p = f[1][0] == '1' ? &with_dec : 0;
			alt_mail_address ad;
			if (alt_mail_parser_parse(p, &a, arg(f[2]), &ad, &err) < 0)
				printf("err %s", alt_buf_cstr(&err));
			else {
				printf("ok ");
				put_addr(&ad);
				alt_buf s = ALT_BUF_INIT;
				alt_mail_address_format(&s, &ad);
				alt_mail_address back;
				if (alt_mail_parser_parse(p, &a, alt_buf_str(&s), &back, &err) < 0)
					printf("|err %s", alt_buf_cstr(&err));
				else {
					printf("|ok ");
					hex(back.name);
					printf("|");
					hex(back.address);
				}
				alt_buf_free(&s);
			}
		} else if (!strcmp(f[0], "list")) {
			const alt_mail_address_parser *p = f[1][0] == '1' ? &with_dec : 0;
			alt_mail_address *v;
			size_t n;
			int rc = alt_mail_parser_parse_list(p, &a, arg(f[2]), &v, &n, &err);
			put_list(rc, v, n, &err);
		} else if (!strcmp(f[0], "date")) {
			alt_wall t = 0;
			alt_zone z = {0};
			int rc = alt_mail_parse_date(arg(f[1]), &t, &z, &err);
			put_date(rc, t, &z, &err);
		} else if (!strcmp(f[0], "fmt")) {
			alt_mail_address ad = {arg(f[1]), arg(f[2])};
			alt_buf s = ALT_BUF_INIT;
			alt_mail_address_format(&s, &ad);
			hex(alt_buf_str(&s));
			alt_buf_free(&s);
		} else if (!strcmp(f[0], "msg")) {
			alt_str_reader sr;
			alt_mail_message m;
			if (alt_mail_read_message(&m, &a, alt_str_reader_init(&sr, arg(f[1])), &err) < 0)
				printf("err %s", alt_buf_cstr(&err));
			else {
				printf("ok ");
				alt_textproto_header h = m.header;
				alt_textproto_field *copy = alt_arena_alloc(&a, (h.n + 1) * sizeof *copy);
				memcpy(copy, h.f, h.n * sizeof *copy);
				sort_fields(copy, h.n);
				for (size_t i = 0; i < h.n; i++) {
					if (i == 0 || !alt_str_eq(copy[i].key, copy[i - 1].key)) {
						if (i)
							printf(" ");
						hex(copy[i].key);
						printf("=");
					} else
						printf(",");
					hex(copy[i].value);
				}
				alt_buf body = ALT_BUF_INIT;
				alt_read_all(m.body, &body, 0);
				printf("|");
				hex(alt_buf_str(&body));
				alt_buf_free(&body);
				alt_wall t = 0;
				alt_zone z = {0};
				printf("|date:");
				put_date(alt_mail_header_date(&m.header, &t, &z, &err), t, &z, &err);
				alt_mail_address *v;
				size_t n;
				printf("|from:");
				put_list(alt_mail_header_address_list(&m.header, &a, ALT_STR("From"), &v, &n, &err), v, n, &err);
				printf("|to:");
				put_list(alt_mail_header_address_list(&m.header, &a, ALT_STR("to"), &v, &n, &err), v, n, &err);
				alt_mail_message_free(&m);
			}
		}
		printf("\n");
		alt_buf_free(&err);
		alt_arena_free(&a);
		fflush(stdout);
	}
	return 0;
}
