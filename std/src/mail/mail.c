/* Mail messages after Go's net/mail (BSD-3-Clause, see LICENSE-Go):
   ReadMessage, ParseDate, the address parser with Go's quirks (groups,
   comments, quoted strings, domain literals, RFC 2047 display names) and
   Address.String. */
#include <alt/net.h>
#include <alt/net/mail.h>
#include <alt/strconv.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	alt_str s;
	const alt_mime_word_decoder *dec;
	alt_arena *a;
	alt_buf *err;
} parser;

static int fail(parser *p, const char *msg) {
	if (p->err) {
		alt_buf_reset(p->err);
		alt_buf_append_cstr(p->err, msg);
	}
	return -ALT_ESYNTAX;
}

/* msg followed by Go's %q of s */
static int failq(parser *p, const char *msg, alt_str s) {
	if (p->err) {
		alt_buf_reset(p->err);
		alt_buf_append_cstr(p->err, msg);
		alt_quote(p->err, s);
	}
	return -ALT_ESYNTAX;
}

/* msg followed by Go's %q of a rune (strconv.QuoteRune) */
static int failr(parser *p, const char *msg, int32_t r) {
	if (p->err) {
		alt_buf_reset(p->err);
		alt_buf_append_cstr(p->err, msg);
		if (r == '\'')
			alt_buf_append_cstr(p->err, "'\\''");
		else if (r == '"')
			alt_buf_append_cstr(p->err, "'\"'");
		else {
			char u[4];
			alt_buf q = ALT_BUF_INIT;
			alt_quote(&q, alt_str_make(u, alt_utf8_encode(u, r)));
			alt_buf_append_byte(p->err, '\'');
			if (q.n >= 2)
				alt_buf_append(p->err, q.p + 1, q.n - 2);
			alt_buf_append_byte(p->err, '\'');
			alt_buf_free(&q);
		}
	}
	return -ALT_ESYNTAX;
}

static bool is_multibyte(int32_t r) { return r >= 0x80; }
static bool is_vchar(int32_t r) { return ('!' <= r && r <= '~') || is_multibyte(r); }
static bool is_wsp(int32_t r) { return r == ' ' || r == '\t'; }
static bool is_atext(int32_t r, bool dot) {
	if (r == '.')
		return dot;
	if (r < 0x80 && r && strchr("()<>[]:;@\\,\"", (int)r))
		return false;
	return is_vchar(r);
}
static bool is_qtext(int32_t r) { return r != '\\' && r != '"' && is_vchar(r); }
static bool is_dtext(int32_t r) { return r != '[' && r != ']' && r != '\\' && is_vchar(r); }

static bool empty(parser *p) { return p->s.n == 0; }
static int peek(parser *p) { return (unsigned char)p->s.p[0]; }
static bool consume(parser *p, int c) {
	if (empty(p) || peek(p) != c)
		return false;
	p->s = alt_str_from(p->s, 1);
	return true;
}
static void skip_space(parser *p) { p->s = alt_str_trim_left(p->s, " \t"); }

/* consumeComment ('(' already consumed): the text into comment (may be NULL) */
static bool consume_comment(parser *p, alt_buf *comment) {
	int depth = 1;
	while (!empty(p) && depth) {
		if (peek(p) == '\\' && p->s.n > 1)
			p->s = alt_str_from(p->s, 1);
		else if (peek(p) == '(')
			depth++;
		else if (peek(p) == ')')
			depth--;
		if (depth > 0 && comment)
			alt_buf_append_byte(comment, p->s.p[0]);
		p->s = alt_str_from(p->s, 1);
	}
	return depth == 0;
}

static bool skip_cfws(parser *p) {
	skip_space(p);
	while (consume(p, '(')) {
		if (!consume_comment(p, 0))
			return false;
		skip_space(p);
	}
	return true;
}

static alt_str arena_str(parser *p, alt_str s) { return alt_arena_str(p->a, s); }

/* consumeQuotedString: the unquoted text appended to out */
static int consume_quoted_string(parser *p, alt_buf *out) {
	size_t i = 1;
	bool escaped = false;
	for (;;) {
		int32_t r;
		size_t size = alt_utf8_decode(p->s.p + i, p->s.n - i, &r);
		if (size == 0)
			return fail(p, "mail: unclosed quoted-string");
		if (size == 1 && r == ALT_UTF8_RUNE_ERROR)
			return failq(p, "mail: invalid utf-8 in quoted-string: ", p->s);
		if (escaped) {
			if (!is_vchar(r) && !is_wsp(r))
				return failr(p, "mail: bad character in quoted-string: ", r);
			alt_buf_append(out, p->s.p + i, size);
			escaped = false;
		} else if (is_qtext(r) || is_wsp(r))
			alt_buf_append(out, p->s.p + i, size);
		else if (r == '"')
			break;
		else if (r == '\\')
			escaped = true;
		else
			return failr(p, "mail: bad character in quoted-string: ", r);
		i += size;
	}
	p->s = alt_str_from(p->s, i + 1);
	return out->err;
}

/* consumeAtom: *atom points into the input */
static int consume_atom(parser *p, bool dot, bool permissive, alt_str *atom) {
	size_t i = 0;
	for (;;) {
		int32_t r;
		size_t size = alt_utf8_decode(p->s.p + i, p->s.n - i, &r);
		if (size == 1 && r == ALT_UTF8_RUNE_ERROR)
			return failq(p, "mail: invalid utf-8 in address: ", p->s);
		if (size == 0 || !is_atext(r, dot))
			break;
		i += size;
	}
	if (i == 0)
		return fail(p, "mail: invalid string");
	*atom = alt_str_slice(p->s, 0, i);
	p->s = alt_str_from(p->s, i);
	if (!permissive) {
		if (atom->p[0] == '.')
			return fail(p, "mail: leading dot in atom");
		if (alt_str_contains(*atom, ALT_STR("..")))
			return fail(p, "mail: double dot in atom");
		if (atom->p[atom->n - 1] == '.')
			return fail(p, "mail: trailing dot in atom");
	}
	return 0;
}

/* net.ParseIP: valid, without a zone; *v4 when To4 is not nil */
static bool parse_ip(alt_str s, bool *v4) {
	alt_ip ip;
	if (alt_ip_parse(s, &ip) < 0 || ip.zone[0])
		return false;
	static const uint8_t prefix[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff};
	*v4 = ip.is4 || !memcmp(ip.b, prefix, 12);
	return true;
}

static int consume_domain_literal(parser *p, alt_buf *out) {
	if (!consume(p, '['))
		return fail(p, "mail: missing \"[\" in domain-literal");
	alt_str dtext = p->s;
	size_t len = 0;
	for (;;) {
		if (empty(p))
			return fail(p, "mail: unclosed domain-literal");
		if (peek(p) == ']')
			break;
		int32_t r;
		size_t size = alt_utf8_decode(p->s.p, p->s.n, &r);
		if (size == 1 && r == ALT_UTF8_RUNE_ERROR)
			return failq(p, "mail: invalid utf-8 in domain-literal: ", p->s);
		if (!is_dtext(r))
			return failr(p, "mail: bad character in domain-literal: ", r);
		len += size;
		p->s = alt_str_from(p->s, size);
	}
	dtext = alt_str_slice(dtext, 0, len);
	if (!consume(p, ']'))
		return fail(p, "mail: unclosed domain-literal");
	bool v4;
	if (alt_str_has_prefix(dtext, ALT_STR("IPv6:"))) {
		if (!parse_ip(alt_str_from(dtext, 5), &v4))
			return failq(p, "mail: invalid IPv6 address in domain-literal: ", dtext);
	} else if (!parse_ip(dtext, &v4) || !v4)
		return failq(p, "mail: invalid IP address in domain-literal: ", dtext);
	alt_buf_append_byte(out, '[');
	alt_buf_append_str(out, dtext);
	alt_buf_append_byte(out, ']');
	return out->err;
}

/* consumeAddrSpec: "local@domain" into *spec (in the arena); p is left
   as it was on errors */
static int consume_addr_spec(parser *p, alt_str *spec) {
	alt_str orig = p->s;
	alt_buf b = ALT_BUF_INIT;
	int rc;
	skip_space(p);
	if (empty(p)) {
		rc = fail(p, "mail: no addr-spec");
		goto out;
	}
	if (peek(p) == '"') {
		rc = consume_quoted_string(p, &b);
		if (rc < 0 || b.n == 0) /* Go's localPart is "" on errors too */
			rc = fail(p, "mail: empty quoted string in addr-spec");
	} else {
		alt_str atom;
		rc = consume_atom(p, true, false, &atom);
		if (rc == 0)
			alt_buf_append_str(&b, atom);
	}
	if (rc < 0)
		goto out;
	if (!consume(p, '@')) {
		rc = fail(p, "mail: missing @ in addr-spec");
		goto out;
	}
	alt_buf_append_byte(&b, '@');
	skip_space(p);
	if (empty(p)) {
		rc = fail(p, "mail: no domain in addr-spec");
		goto out;
	}
	if (peek(p) == '[')
		rc = consume_domain_literal(p, &b);
	else {
		alt_str atom;
		rc = consume_atom(p, true, false, &atom);
		if (rc == 0)
			alt_buf_append_str(&b, atom);
	}
	if (rc == 0) {
		*spec = arena_str(p, alt_buf_str(&b));
		rc = spec->p ? b.err : -ENOMEM;
	}
out:
	alt_buf_free(&b);
	if (rc < 0)
		p->s = orig;
	return rc;
}

/* the charset reader of decodeRFC2047Word: tells charset errors apart */
typedef struct {
	const alt_mime_word_decoder *user;
	bool charset_error;
} wrap;

static int wrap_reader(void *arg, alt_str charset, alt_str content, alt_buf *out, alt_buf *err) {
	wrap *w = arg;
	if (!w->user || !w->user->charset_reader) {
		w->charset_error = true;
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_cstr(err, "charset not supported: ");
			alt_quote(err, charset);
		}
		return -ALT_ESYNTAX;
	}
	int rc = w->user->charset_reader(w->user->arg, charset, content, out, err);
	if (rc < 0)
		w->charset_error = true;
	return rc;
}

/* decodeRFC2047Word: 1 and the decoded word appended to out when s is an
   encoded-word, 0 (s appended) when it is not, <0 for charset errors */
static int decode_word(parser *p, alt_str s, alt_buf *out) {
	wrap w = {p->dec, false};
	alt_mime_word_decoder d = {wrap_reader, &w};
	alt_buf e = ALT_BUF_INIT;
	int rc = alt_mime_word_decode(&d, out, s, &e);
	if (rc == 0) {
		alt_buf_free(&e);
		return 1;
	}
	if (w.charset_error) {
		if (p->err) {
			alt_buf_reset(p->err);
			alt_buf_append_str(p->err, alt_buf_str(&e));
		}
		alt_buf_free(&e);
		return rc;
	}
	alt_buf_free(&e);
	alt_buf_append_str(out, s);
	return 0;
}

/* consumePhrase: the display name into *phrase */
static int consume_phrase(parser *p, alt_str *phrase) {
	alt_buf words = ALT_BUF_INIT, sb = ALT_BUF_INIT, word = ALT_BUF_INIT;
	size_t nwords = 0;
	int rc = 0;
	for (;;) {
		if (nwords > 0 && !skip_cfws(p)) {
			rc = fail(p, "mail: misformatted parenthetical comment");
			goto out;
		}
		skip_space(p);
		if (empty(p))
			break;
		bool encoded = false;
		alt_buf_reset(&word);
		if (peek(p) == '"')
			rc = consume_quoted_string(p, &word);
		else {
			alt_str atom;
			rc = consume_atom(p, true, true, &atom);
			if (rc == 0) {
				rc = decode_word(p, atom, &word);
				if (rc >= 0) {
					encoded = rc == 1;
					rc = 0;
				}
			}
		}
		if (rc < 0)
			break;
		if (encoded)
			alt_buf_append_str(&sb, alt_buf_str(&word));
		else {
			if (sb.n) {
				if (nwords++)
					alt_buf_append_byte(&words, ' ');
				alt_buf_append_str(&words, alt_buf_str(&sb));
				alt_buf_reset(&sb);
			}
			if (nwords++)
				alt_buf_append_byte(&words, ' ');
			alt_buf_append_str(&words, alt_buf_str(&word));
		}
	}
	if (sb.n) {
		if (nwords++)
			alt_buf_append_byte(&words, ' ');
		alt_buf_append_str(&words, alt_buf_str(&sb));
	}
	if (rc < 0 && nwords == 0) {
		if (p->err) {
			alt_buf msg = ALT_BUF_INIT;
			alt_buf_append_cstr(&msg, "mail: missing word in phrase: ");
			alt_buf_append_str(&msg, alt_buf_str(p->err));
			alt_buf_reset(p->err);
			alt_buf_append_str(p->err, alt_buf_str(&msg));
			alt_buf_free(&msg);
		}
		goto out;
	}
	rc = 0;
	*phrase = arena_str(p, alt_buf_str(&words));
	if (!phrase->p)
		rc = -ENOMEM;
out:
	alt_buf_free(&words);
	alt_buf_free(&sb);
	alt_buf_free(&word);
	return rc;
}

static int consume_display_name_comment(parser *p, alt_str *name) {
	if (!consume(p, '('))
		return fail(p, "mail: comment does not start with (");
	alt_buf comment = ALT_BUF_INIT, out = ALT_BUF_INIT;
	int rc = 0;
	if (!consume_comment(p, &comment)) {
		rc = fail(p, "mail: misformatted parenthetical comment");
		goto done;
	}
	alt_str rest = alt_buf_str(&comment);
	size_t nw = 0;
	while (rest.n) {
		size_t i = 0;
		while (i < rest.n && (rest.p[i] == ' ' || rest.p[i] == '\t'))
			i++;
		size_t j = i;
		while (j < rest.n && rest.p[j] != ' ' && rest.p[j] != '\t')
			j++;
		if (j > i) {
			if (nw++)
				alt_buf_append_byte(&out, ' ');
			rc = decode_word(p, alt_str_slice(rest, i, j), &out);
			if (rc < 0)
				goto done;
		}
		rest = alt_str_from(rest, j);
	}
	rc = 0;
	*name = arena_str(p, alt_buf_str(&out));
	if (!name->p)
		rc = -ENOMEM;
done:
	alt_buf_free(&comment);
	alt_buf_free(&out);
	return rc;
}

/* the addresses found, growing in the arena */
typedef struct {
	alt_mail_address *v;
	size_t n, cap;
} list;

static int push(parser *p, list *l, alt_str name, alt_str addr) {
	if (l->n == l->cap) {
		size_t cap = l->cap ? 2 * l->cap : 4;
		alt_mail_address *v = alt_arena_realloc(p->a, l->v, l->cap * sizeof *v, cap * sizeof *v);
		if (!v)
			return -ENOMEM;
		l->v = v;
		l->cap = cap;
	}
	l->v[l->n++] = (alt_mail_address){name, addr};
	return 0;
}

static int parse_address(parser *p, bool handle_group, list *out);

static int consume_group_list(parser *p, list *out) {
	skip_space(p);
	if (consume(p, ';')) {
		if (!skip_cfws(p))
			return fail(p, "mail: misformatted parenthetical comment");
		return 0;
	}
	for (;;) {
		skip_space(p);
		int rc = parse_address(p, false, out);
		if (rc < 0)
			return rc;
		if (!skip_cfws(p))
			return fail(p, "mail: misformatted parenthetical comment");
		if (consume(p, ';')) {
			if (!skip_cfws(p))
				return fail(p, "mail: misformatted parenthetical comment");
			return 0;
		}
		if (!consume(p, ','))
			return fail(p, "mail: expected comma");
	}
}

static int parse_address(parser *p, bool handle_group, list *out) {
	skip_space(p);
	if (empty(p))
		return fail(p, "mail: no address");
	alt_str spec, name = ALT_STR("");
	if (consume_addr_spec(p, &spec) == 0) {
		skip_space(p);
		if (!empty(p) && peek(p) == '(') {
			int rc = consume_display_name_comment(p, &name);
			if (rc < 0)
				return rc;
		}
		return push(p, out, name, spec);
	}
	if (peek(p) != '<') {
		int rc = consume_phrase(p, &name);
		if (rc < 0)
			return rc;
	}
	skip_space(p);
	if (handle_group && consume(p, ':'))
		return consume_group_list(p, out);
	if (!consume(p, '<')) {
		bool atext = true;
		for (size_t i = 0; i < name.n;) {
			int32_t r;
			i += alt_utf8_decode(name.p + i, name.n - i, &r);
			if (!is_atext(r, true)) {
				atext = false;
				break;
			}
		}
		if (atext)
			return fail(p, "mail: missing '@' or angle-addr");
		return fail(p, "mail: no angle-addr");
	}
	int rc = consume_addr_spec(p, &spec);
	if (rc < 0)
		return rc;
	if (!consume(p, '>'))
		return fail(p, "mail: unclosed angle-addr");
	return push(p, out, name, spec);
}

static int parse_list(parser *p, list *l) {
	for (;;) {
		skip_space(p);
		if (consume(p, ','))
			continue;
		int rc = parse_address(p, true, l);
		if (rc < 0)
			return rc;
		if (!skip_cfws(p))
			return fail(p, "mail: misformatted parenthetical comment");
		if (empty(p))
			return 0;
		if (peek(p) != ',')
			return fail(p, "mail: expected comma");
		while (consume(p, ','))
			skip_space(p);
		if (empty(p))
			return 0;
	}
}

static int parse_single(parser *p, alt_mail_address *out) {
	list l = {0};
	int rc = parse_address(p, true, &l);
	if (rc < 0)
		return rc;
	if (!skip_cfws(p))
		return fail(p, "mail: misformatted parenthetical comment");
	if (!empty(p))
		return failq(p, "mail: expected single address, got ", p->s);
	if (l.n == 0)
		return fail(p, "mail: empty group");
	if (l.n > 1)
		return fail(p, "mail: group with multiple addresses");
	*out = l.v[0];
	return 0;
}

int alt_mail_parser_parse(const alt_mail_address_parser *ap, alt_arena *a, alt_str s, alt_mail_address *out, alt_buf *err) {
	parser p = {s, ap ? ap->word_decoder : 0, a, err};
	return parse_single(&p, out);
}

int alt_mail_parser_parse_list(const alt_mail_address_parser *ap, alt_arena *a, alt_str s, alt_mail_address **out, size_t *n,
	alt_buf *err) {
	parser p = {s, ap ? ap->word_decoder : 0, a, err};
	list l = {0};
	int rc = parse_list(&p, &l);
	*out = rc < 0 ? 0 : l.v;
	*n = rc < 0 ? 0 : l.n;
	return rc;
}

int alt_mail_parse_address(alt_arena *a, alt_str s, alt_mail_address *out, alt_buf *err) {
	return alt_mail_parser_parse(0, a, s, out, err);
}

int alt_mail_parse_address_list(alt_arena *a, alt_str s, alt_mail_address **out, size_t *n, alt_buf *err) {
	return alt_mail_parser_parse_list(0, a, s, out, n, err);
}

/* ---- Address.String ---- */

static void quote_string(alt_buf *b, alt_str s) {
	alt_buf_append_byte(b, '"');
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		i += alt_utf8_decode(s.p + i, s.n - i, &r);
		char u[4];
		size_t k = alt_utf8_encode(u, r);
		if (is_qtext(r) || is_wsp(r))
			alt_buf_append(b, u, k);
		else if (is_vchar(r)) {
			alt_buf_append_byte(b, '\\');
			alt_buf_append(b, u, k);
		}
	}
	alt_buf_append_byte(b, '"');
}

int alt_mail_address_format(alt_buf *b, const alt_mail_address *addr) {
	alt_str local = addr->address, domain = ALT_STR("");
	ssize_t at = alt_str_last_index_byte(addr->address, '@');
	if (at >= 0) {
		local = alt_str_slice(addr->address, 0, (size_t)at);
		domain = alt_str_from(addr->address, (size_t)at + 1);
	}
	bool quote_local = false;
	for (size_t i = 0; i < local.n;) {
		int32_t r;
		size_t k = alt_utf8_decode(local.p + i, local.n - i, &r);
		if (!is_atext(r, false)) {
			if (!(r == '.' && i > 0 && local.p[i - 1] != '.' && i < local.n - 1)) {
				quote_local = true;
				break;
			}
		}
		i += k;
	}
	alt_buf s = ALT_BUF_INIT;
	alt_buf_append_byte(&s, '<');
	if (quote_local)
		quote_string(&s, local);
	else
		alt_buf_append_str(&s, local);
	alt_buf_append_byte(&s, '@');
	alt_buf_append_str(&s, domain);
	alt_buf_append_byte(&s, '>');
	alt_str name = addr->name;
	if (name.n) {
		bool printable = true;
		for (size_t i = 0; i < name.n;) {
			int32_t r;
			i += alt_utf8_decode(name.p + i, name.n - i, &r);
			if ((!is_vchar(r) && !is_wsp(r)) || is_multibyte(r)) {
				printable = false;
				break;
			}
		}
		if (printable)
			quote_string(b, name);
		else
			alt_mime_word_encode(b, alt_str_index_any(name, "\"#$%&'(),.:;<>@[]^`{|}~") >= 0 ? ALT_MIME_BENCODING : ALT_MIME_QENCODING,
				ALT_STR("utf-8"), name);
		alt_buf_append_byte(b, ' ');
	}
	alt_buf_append_str(b, alt_buf_str(&s));
	alt_buf_free(&s);
	return b->err;
}

/* ---- dates ---- */

int alt_mail_parse_date(alt_str in, alt_wall *t, alt_zone *z, alt_buf *err) {
	parser p = {ALT_STR(""), 0, 0, err};
	alt_buf d = ALT_BUF_INIT;
	int rc = -ALT_ESYNTAX;
	for (size_t i = 0; i < in.n; i++) {
		if (in.p[i] == '\r' && i + 1 < in.n && in.p[i + 1] == '\n') {
			i++;
			continue;
		}
		alt_buf_append_byte(&d, in.p[i]);
	}
	if (d.err) {
		alt_buf_free(&d);
		return -ENOMEM;
	}
	alt_str date = alt_buf_str(&d);
	if (alt_str_index_byte(date, '\r') >= 0) {
		fail(&p, "mail: header has a CR without LF");
		goto out;
	}
	p.s = date;
	skip_space(&p);
	ssize_t ind = alt_str_index_any(p.s, "+-");
	if (ind >= 0 && p.s.n >= (size_t)ind + 5) {
		date = alt_str_slice(p.s, 0, (size_t)ind + 5);
		p.s = alt_str_from(p.s, (size_t)ind + 5);
	} else {
		ind = alt_str_index_byte(p.s, 'T');
		if (ind == 0) {
			ind = alt_str_index_byte(alt_str_from(p.s, 1), 'T');
			if (ind >= 0)
				ind++;
		}
		if (ind >= 0 && p.s.n >= (size_t)ind + 5) {
			date = alt_str_slice(p.s, 0, (size_t)ind + 1);
			p.s = alt_str_from(p.s, (size_t)ind + 1);
		}
	}
	if (!skip_cfws(&p)) {
		fail(&p, "mail: misformatted parenthetical comment");
		goto out;
	}
	static const char *const dows[] = {"", "Mon, "}, *const days[] = {"2", "02"}, *const years[] = {"2006", "06"},
				 *const secs[] = {":05", ""}, *const zones[] = {"-0700", "MST", "UT"};
	for (int a = 0; a < 2; a++)
		for (int b = 0; b < 2; b++)
			for (int c = 0; c < 2; c++)
				for (int e = 0; e < 2; e++)
					for (int f = 0; f < 3; f++) {
						char layout[64];
						snprintf(layout, sizeof layout, "%s%s Jan %s 15:04%s %s", dows[a], days[b], years[c], secs[e], zones[f]);
						alt_wall w;
						alt_zone zz;
						if (alt_wall_parse(date, layout, &w, &zz) == 0) {
							*t = w;
							if (z)
								*z = zz;
							rc = 0;
							goto out;
						}
					}
	fail(&p, "mail: header could not be parsed");
out:
	alt_buf_free(&d);
	return rc;
}

/* ---- messages ---- */

static int set_err(alt_buf *err, const char *msg, alt_str q) {
	if (err) {
		alt_buf_reset(err);
		alt_buf_append_cstr(err, msg);
		alt_quote(err, q);
	}
	return -ALT_ESYNTAX;
}

/* readHeader: like ReadMIMEHeader without its validation */
static int read_header(alt_textproto_reader *r, alt_textproto_header *h, alt_buf *err) {
	alt_str pk;
	alt_buf kv = ALT_BUF_INIT;
	int rc = 0;
	if (alt_bufreader_peek(r->br, 1, &pk) == 0 && (pk.p[0] == ' ' || pk.p[0] == '\t')) {
		rc = alt_textproto_read_line(r, &kv);
		if (rc == 0)
			rc = set_err(err, "malformed initial line: ", alt_buf_str(&kv));
		goto out;
	}
	for (;;) {
		rc = alt_textproto_read_continued_line(r, &kv);
		if (!kv.n)
			break;
		alt_str k, v;
		if (!alt_str_cut_byte(alt_buf_str(&kv), ':', &k, &v)) {
			rc = set_err(err, "malformed header line: ", alt_buf_str(&kv));
			goto out;
		}
		if (!k.n)
			continue;
		alt_buf key = ALT_BUF_INIT;
		alt_textproto_canonical_key(&key, k);
		alt_str ks = alt_arena_str(h->arena, alt_buf_str(&key)), vs = alt_arena_str(h->arena, alt_str_trim_left(v, " \t"));
		alt_buf_free(&key);
		if (!ks.p || !vs.p) {
			rc = -ENOMEM;
			goto out;
		}
		/* append without canonicalizing again */
		if (h->n == h->cap) {
			size_t cap = h->cap ? 2 * h->cap : 8;
			void *f = alt_arena_realloc(h->arena, h->f, h->cap * sizeof *h->f, cap * sizeof *h->f);
			if (!f) {
				rc = -ENOMEM;
				goto out;
			}
			h->f = f;
			h->cap = cap;
		}
		h->f[h->n++] = (alt_textproto_field){ks, vs};
		if (rc < 0)
			break;
	}
out:
	if (rc < 0 && rc != -ALT_ESYNTAX && err) {
		alt_buf_reset(err);
		alt_buf_append_cstr(err, rc == -ENOMEM ? alt_strerror(rc) : alt_textproto_error(r));
	}
	alt_buf_free(&kv);
	return rc;
}

int alt_mail_read_message(alt_mail_message *m, alt_arena *a, alt_reader rd, alt_buf *err) {
	memset(m, 0, sizeof *m);
	int rc = alt_bufreader_init(&m->br, rd, 4096);
	if (rc < 0)
		return rc;
	alt_textproto_header_init(&m->header, a);
	alt_textproto_reader tr;
	alt_textproto_reader_init(&tr, &m->br);
	rc = read_header(&tr, &m->header, err);
	alt_textproto_reader_free(&tr);
	if (rc < 0 && (rc != -ALT_EEOF || m->header.n == 0)) {
		alt_bufreader_free(&m->br);
		return rc;
	}
	if (err)
		alt_buf_reset(err);
	m->body = alt_bufreader_reader(&m->br);
	return 0;
}

void alt_mail_message_free(alt_mail_message *m) { alt_bufreader_free(&m->br); }

int alt_mail_header_date(const alt_textproto_header *h, alt_wall *t, alt_zone *z, alt_buf *err) {
	alt_str v = alt_textproto_header_get(h, ALT_STR("Date"));
	if (!v.n) {
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_cstr(err, "mail: header not in message");
		}
		return -ENOENT;
	}
	return alt_mail_parse_date(v, t, z, err);
}

int alt_mail_header_address_list(const alt_textproto_header *h, alt_arena *a, alt_str key, alt_mail_address **out, size_t *n,
	alt_buf *err) {
	alt_str v = alt_textproto_header_get(h, key);
	if (!v.n) {
		*out = 0;
		*n = 0;
		if (err) {
			alt_buf_reset(err);
			alt_buf_append_cstr(err, "mail: header not in message");
		}
		return -ENOENT;
	}
	return alt_mail_parse_address_list(a, v, out, n, err);
}
