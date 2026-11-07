#include "check.h"
#include <alt/mime.h>
#include <alt/net/mail.h>
#include <ctype.h>

static void encode_words(void) {
	alt_buf b = ALT_BUF_INIT;
	alt_mime_word_encode(&b, ALT_MIME_QENCODING, ALT_STR("utf-8"), ALT_STR("¡Hola, señor!"));
	CHECK_STR(alt_buf_str(&b), "=?utf-8?q?=C2=A1Hola,_se=C3=B1or!?=");
	alt_buf_reset(&b);
	alt_mime_word_encode(&b, ALT_MIME_BENCODING, ALT_STR("UTF-8"), ALT_STR("¡Hola, señor!"));
	CHECK_STR(alt_buf_str(&b), "=?UTF-8?b?wqFIb2xhLCBzZcOxb3Ih?=");
	alt_buf_reset(&b);
	alt_mime_word_encode(&b, ALT_MIME_QENCODING, ALT_STR("utf-8"), ALT_STR("plain ASCII"));
	CHECK_STR(alt_buf_str(&b), "plain ASCII");
	/* long text: words of at most 75 characters */
	alt_buf_reset(&b);
	alt_mime_word_encode(&b, ALT_MIME_QENCODING, ALT_STR("UTF-8"),
		ALT_STR("Gaston Lagaffe à la caisse d'épargne avec Mademoiselle Jeanne et Prunelle, encore et toujours"));
	alt_str rest = alt_buf_str(&b), w;
	int words = 0;
	while (alt_str_split_next(&rest, ALT_STR(" "), &w)) {
		CHECK(w.n <= 75);
		words++;
	}
	CHECK(words > 1);
	alt_buf_free(&b);
}

static int upper_reader(void *arg, alt_str charset, alt_str content, alt_buf *out, alt_buf *err) {
	(void)arg;
	if (!alt_str_eq_cstr(charset, "x-upper")) {
		alt_buf_reset(err);
		alt_buf_append_cstr(err, "no such charset");
		return -EINVAL;
	}
	for (size_t i = 0; i < content.n; i++)
		alt_buf_append_byte(out, toupper((unsigned char)content.p[i]));
	return 0;
}

static void decode_words(void) {
	const char *ok[][2] = {
		{"=?UTF-8?Q?=C2=A1Hola,_se=C3=B1or!?=", "¡Hola, señor!"},
		{"=?UTF-8?b?wqFIb2xhLCBzZcOxb3Ih?=", "¡Hola, señor!"},
		{"=?ISO-8859-1?Q?Caf=E9?=", "Café"},
		{"=?us-ascii?q?a=FFb?=", "a\xef\xbf\xbd" "b"},
		{"=?UTF-8?Q??=", ""},
	};
	alt_buf b = ALT_BUF_INIT, err = ALT_BUF_INIT;
	for (size_t i = 0; i < sizeof ok / sizeof *ok; i++) {
		alt_buf_reset(&b);
		CHECK_OK(alt_mime_word_decode(0, &b, alt_str_cstr(ok[i][0]), &err));
		CHECK_STR(alt_buf_str(&b), ok[i][1]);
	}
	const char *bad[][2] = {
		{"=?UTF-8?Q?a?", "mime: invalid RFC 2047 encoded-word"},
		{"=?UTF-8?X?a?=", "mime: invalid RFC 2047 encoded-word"},
		{"=?UTF-8?Q?=G0?=", "mime: invalid hex byte 0x47"},
		{"=?UTF-8?B?w6=?=", "illegal base64 data at input byte 3"},
		{"=?koi8-r?Q?a?=", "mime: unhandled charset \"koi8-r\""},
	};
	for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
		alt_buf_reset(&b);
		CHECK_ERR(alt_mime_word_decode(0, &b, alt_str_cstr(bad[i][0]), &err), -ALT_EMIME);
		CHECK_STR(alt_buf_str(&err), bad[i][1]);
		CHECK_INT(b.n, 0);
	}
	alt_buf_reset(&b);
	CHECK_OK(alt_mime_decode_header(0, &b, ALT_STR("=?UTF-8?Q?a?= =?UTF-8?Q?b?= c =?bad?= =?UTF-8?Q?d?="), &err));
	CHECK_STR(alt_buf_str(&b), "ab c =?bad?= =?UTF-8?Q?d?="); /* "=?bad?= =" stops the scan, as in Go */
	alt_mime_word_decoder d = {upper_reader, 0};
	alt_buf_reset(&b);
	CHECK_OK(alt_mime_word_decode(&d, &b, ALT_STR("=?X-Upper?q?abc?="), &err));
	CHECK_STR(alt_buf_str(&b), "ABC");
	alt_buf_reset(&b);
	CHECK(alt_mime_decode_header(&d, &b, ALT_STR("x =?other?q?abc?="), &err) < 0);
	CHECK_STR(alt_buf_str(&err), "no such charset");
	alt_buf_free(&b);
	alt_buf_free(&err);
}

static void addresses(void) {
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_buf err = ALT_BUF_INIT, s = ALT_BUF_INIT;
	alt_mail_address ad;
	struct {
		const char *in, *name, *addr, *str;
	} ok[] = {
		{"jdoe@machine.example", "", "jdoe@machine.example", "<jdoe@machine.example>"},
		{"John Doe <jdoe@machine.example>", "John Doe", "jdoe@machine.example", "\"John Doe\" <jdoe@machine.example>"},
		{"\"Joe Q. Public\" <john.q.public@example.com>", "Joe Q. Public", "john.q.public@example.com",
			"\"Joe Q. Public\" <john.q.public@example.com>"},
		{"=?iso-8859-1?q?J=F6rg_Doe?= <joerg@example.com>", "Jörg Doe", "joerg@example.com", "=?utf-8?q?J=C3=B6rg_Doe?= <joerg@example.com>"},
		{"jdoe@[192.168.0.1]", "", "jdoe@[192.168.0.1]", "<jdoe@[192.168.0.1]>"},
		{"\"john doe\"@example.com (Comment)", "Comment", "john doe@example.com", "\"Comment\" <\"john doe\"@example.com>"},
		{"Group: jdoe@example.com;", "", "jdoe@example.com", "<jdoe@example.com>"},
	};
	for (size_t i = 0; i < sizeof ok / sizeof *ok; i++) {
		CHECK_OK(alt_mail_parse_address(&a, alt_str_cstr(ok[i].in), &ad, &err));
		CHECK_STR(ad.name, ok[i].name);
		CHECK_STR(ad.address, ok[i].addr);
		alt_buf_reset(&s);
		alt_mail_address_format(&s, &ad);
		CHECK_STR(alt_buf_str(&s), ok[i].str);
	}
	const char *bad[][2] = {
		{"", "mail: no address"},
		{"Full Name", "mail: no angle-addr"},
		{"foo.bar", "mail: missing '@' or angle-addr"},
		{"John <jdoe@example.com", "mail: unclosed angle-addr"},
		{"a@b, c@d", "mail: expected single address, got \", c@d\""},
		{"g: a@b, c@d;", "mail: group with multiple addresses"},
		{"g:;", "mail: empty group"},
		{"a@[1.2.3]", "mail: missing '@' or angle-addr"}, /* the addr-spec's error is not the one reported */
		{"\"a\x01\"@b", "mail: missing word in phrase: mail: bad character in quoted-string: '\\x01'"},
		{"a@b (unclosed", "mail: misformatted parenthetical comment"},
		{"=?x-unknown?q?a?= <a@b>", "mail: missing word in phrase: charset not supported: \"x-unknown\""},
	};
	for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
		CHECK_ERR(alt_mail_parse_address(&a, alt_str_cstr(bad[i][0]), &ad, &err), -ALT_ESYNTAX);
		CHECK_STR(alt_buf_str(&err), bad[i][1]);
	}
	alt_mail_address *v;
	size_t n;
	CHECK_OK(alt_mail_parse_address_list(&a, ALT_STR("a@b, , Team: c@d, \"E F\" <e@f>; (c), g@h"), &v, &n, &err));
	CHECK_INT(n, 4);
	CHECK_STR(v[2].name, "E F");
	CHECK_STR(v[3].address, "g@h");
	CHECK_ERR(alt_mail_parse_address_list(&a, ALT_STR("a@b c@d"), &v, &n, &err), -ALT_ESYNTAX);
	CHECK_STR(alt_buf_str(&err), "mail: expected comma");
	/* a parser with a charset reader */
	alt_mime_word_decoder d = {upper_reader, 0};
	alt_mail_address_parser p = {&d};
	CHECK_OK(alt_mail_parser_parse(&p, &a, ALT_STR("=?x-upper?q?bob?= <b@x>"), &ad, &err));
	CHECK_STR(ad.name, "BOB");
	/* quoting of the local part and names */
	alt_mail_address q = {ALT_STR("a\"b"), ALT_STR("x y@example.com")};
	alt_buf_reset(&s);
	alt_mail_address_format(&s, &q);
	CHECK_STR(alt_buf_str(&s), "\"a\\\"b\" <\"x y\"@example.com>");
	q = (alt_mail_address){ALT_STR("Café, Ltd."), ALT_STR("x@y")};
	alt_buf_reset(&s);
	alt_mail_address_format(&s, &q);
	CHECK_STR(alt_buf_str(&s), "=?utf-8?b?Q2Fmw6ksIEx0ZC4=?= <x@y>");
	alt_buf_free(&err);
	alt_buf_free(&s);
	alt_arena_free(&a);
}

static void dates(void) {
	alt_buf err = ALT_BUF_INIT;
	alt_wall t;
	alt_zone z;
	CHECK_OK(alt_mail_parse_date(ALT_STR("Fri, 21 Nov 1997 09:55:06 -0600"), &t, &z, &err));
	CHECK_INT(alt_wall_unix(t), 880127706);
	CHECK_INT(z.offset, -6 * 3600);
	CHECK_OK(alt_mail_parse_date(ALT_STR("Thu, 20 Nov 1997 09:55:06 MDT (MDT)"), &t, &z, &err));
	CHECK_STR(z.name, "MDT");
	CHECK_OK(alt_mail_parse_date(ALT_STR("21 Nov 97 09:55 +0100 (comment)"), &t, &z, &err));
	CHECK_OK(alt_mail_parse_date(ALT_STR("Fri, 21 Nov 1997 09:55:06\r\n -0600"), &t, &z, &err));
	CHECK_ERR(alt_mail_parse_date(ALT_STR("Fri, 21 Nov 1997 09:55:06\r -0600"), &t, &z, &err), -ALT_ESYNTAX);
	CHECK_STR(alt_buf_str(&err), "mail: header has a CR without LF");
	CHECK_ERR(alt_mail_parse_date(ALT_STR("Fri, 21 Nov 1997 09:55:06 -0600 (unclosed"), &t, &z, &err), -ALT_ESYNTAX);
	CHECK_STR(alt_buf_str(&err), "mail: misformatted parenthetical comment");
	CHECK_ERR(alt_mail_parse_date(ALT_STR("yesterday"), &t, &z, &err), -ALT_ESYNTAX);
	CHECK_STR(alt_buf_str(&err), "mail: header could not be parsed");
	alt_buf_free(&err);
}

static void messages(void) {
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_buf err = ALT_BUF_INIT, body = ALT_BUF_INIT;
	const char *text = "From: John Doe <jdoe@machine.example>\r\n"
			   "To: Mary Smith <mary@example.net>, ann@example.net\r\n"
			   "Subject: Saying\r\n Hello\r\n"
			   "Date: Fri, 21 Nov 1997 09:55:06 -0600\r\n"
			   "Message-ID: <1234@local.machine.example>\r\n"
			   "\r\n"
			   "This is a message just to say hello.\r\nSo, \"Hello\".\r\n";
	alt_str_reader sr;
	alt_mail_message m;
	CHECK_OK(alt_mail_read_message(&m, &a, alt_str_reader_init(&sr, alt_str_cstr(text)), &err));
	CHECK_STR(alt_textproto_header_get(&m.header, ALT_STR("subject")), "Saying Hello");
	CHECK_STR(alt_textproto_header_get(&m.header, ALT_STR("Message-Id")), "<1234@local.machine.example>");
	alt_wall t;
	alt_zone z;
	CHECK_OK(alt_mail_header_date(&m.header, &t, &z, &err));
	CHECK_INT(alt_wall_unix(t), 880127706);
	alt_mail_address *v;
	size_t n;
	CHECK_OK(alt_mail_header_address_list(&m.header, &a, ALT_STR("to"), &v, &n, &err));
	CHECK_INT(n, 2);
	CHECK_STR(v[1].address, "ann@example.net");
	CHECK_ERR(alt_mail_header_address_list(&m.header, &a, ALT_STR("Cc"), &v, &n, &err), -ENOENT);
	CHECK_STR(alt_buf_str(&err), "mail: header not in message");
	CHECK(alt_read_all(m.body, &body, 0) >= 0);
	CHECK_STR(alt_buf_str(&body), "This is a message just to say hello.\r\nSo, \"Hello\".\r\n");
	alt_mail_message_free(&m);

	/* the header may end with the data; errors */
	CHECK_OK(alt_mail_read_message(&m, &a, alt_str_reader_init(&sr, ALT_STR("A: b")), &err));
	CHECK_STR(alt_textproto_header_get(&m.header, ALT_STR("a")), "b");
	alt_mail_message_free(&m);
	CHECK_ERR(alt_mail_read_message(&m, &a, alt_str_reader_init(&sr, ALT_STR("")), &err), -ALT_EEOF);
	CHECK_STR(alt_buf_str(&err), "EOF");
	CHECK_ERR(alt_mail_read_message(&m, &a, alt_str_reader_init(&sr, ALT_STR(" x: y\r\n\r\n")), &err), -ALT_ESYNTAX);
	CHECK_STR(alt_buf_str(&err), "malformed initial line: \" x: y\"");
	CHECK_ERR(alt_mail_read_message(&m, &a, alt_str_reader_init(&sr, ALT_STR("A: b\r\nnocolon\r\n\r\n")), &err), -ALT_ESYNTAX);
	CHECK_STR(alt_buf_str(&err), "malformed header line: \"nocolon\"");
	alt_buf_free(&err);
	alt_buf_free(&body);
	alt_arena_free(&a);
}

int main(void) {
	RUN(encode_words);
	RUN(decode_words);
	RUN(addresses);
	RUN(dates);
	RUN(messages);
	return check_done();
}
