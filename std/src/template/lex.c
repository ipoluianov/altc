/* The lexer of templates, after Go's text/template/parse/lex.go
   (BSD-3-Clause, see LICENSE-Go). */
#include "template_internal.h"
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <string.h>

#define EOF_RUNE (-1)


enum { S_DONE, S_TEXT, S_LEFT_DELIM, S_COMMENT, S_RIGHT_DELIM, S_INSIDE_ACTION, S_SPACE, S_IDENTIFIER, S_FIELD, S_VARIABLE, S_CHAR, S_NUMBER, S_QUOTE, S_RAW_QUOTE };

static int32_t lnext(tlexer *l) {
	if ((size_t)l->pos >= l->input.n) {
		l->at_eof = true;
		return EOF_RUNE;
	}
	int32_t r;
	size_t w = alt_utf8_decode(l->input.p + l->pos, l->input.n - (size_t)l->pos, &r);
	l->pos += (int)w;
	if (r == '\n')
		l->line++;
	return r;
}

static void lbackup(tlexer *l) {
	if (!l->at_eof && l->pos > 0) {
		int32_t r;
		size_t w = alt_utf8_decode_last(l->input.p, (size_t)l->pos, &r);
		l->pos -= (int)w;
		if (r == '\n')
			l->line--;
	}
}

static int32_t lpeek(tlexer *l) {
	int32_t r = lnext(l);
	lbackup(l);
	return r;
}

static alt_str lrest(tlexer *l, int at) { return alt_str_from(l->input, (size_t)at); }

static int count_nl(alt_str s) {
	int n = 0;
	for (size_t i = 0; i < s.n; i++)
		n += s.p[i] == '\n';
	return n;
}

static titem this_item(tlexer *l, int t) {
	titem i = {t, l->start, alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos), l->start_line};
	l->start = l->pos;
	l->start_line = l->line;
	return i;
}

static int emit_item(tlexer *l, titem i) {
	l->item = i;
	return 0;
}

static int emit(tlexer *l, int t) { return emit_item(l, this_item(l, t)); }

static void ignore(tlexer *l) {
	l->line += count_nl(alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos));
	l->start = l->pos;
	l->start_line = l->line;
}

static bool contains_rune(const char *valid, int32_t r) { return r > 0 && r < 128 && strchr(valid, (int)r); }

static bool accept(tlexer *l, const char *valid) {
	if (contains_rune(valid, lnext(l)))
		return true;
	lbackup(l);
	return false;
}

static void accept_run(tlexer *l, const char *valid) {
	while (contains_rune(valid, lnext(l)))
		;
	lbackup(l);
}

static int errorf(tlexer *l, char *msg) {
	l->item = (titem){IT_ERROR, l->start, alt_str_cstr(msg), l->start_line};
	l->start = 0;
	l->pos = 0;
	l->input.n = 0;
	return 0;
}

static bool is_space(int32_t r) { return r == ' ' || r == '\t' || r == '\r' || r == '\n'; }
static bool is_alnum(int32_t r) { return r == '_' || (r >= 0 && (alt_unicode_is_letter(r) || alt_unicode_is_digit(r))); }

static bool has_left_trim(alt_str s) { return s.n >= 2 && s.p[0] == '-' && is_space((unsigned char)s.p[1]); }
static bool has_right_trim(alt_str s) { return s.n >= 2 && is_space((unsigned char)s.p[0]) && s.p[1] == '-'; }

static int right_trim_len(alt_str s) {
	size_t n = s.n;
	while (n > 0 && is_space((unsigned char)s.p[n - 1]))
		n--;
	return (int)(s.n - n);
}

static int left_trim_len(alt_str s) {
	size_t i = 0;
	while (i < s.n && is_space((unsigned char)s.p[i]))
		i++;
	return (int)i;
}

/* %#U of r */
static char *unicode_text(tlexer *l, const char *prefix, int32_t r) {
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, prefix);
	tfmt_unicode(&b, r, true);
	char *s = alt_arena_strdup(l->arena, alt_buf_cstr(&b));
	alt_buf_free(&b);
	return s;
}

static int lex_text(tlexer *l);
static int lex_left_delim(tlexer *l);
static int lex_comment(tlexer *l);
static int lex_right_delim(tlexer *l);
static int lex_inside_action(tlexer *l);
static int lex_space(tlexer *l);
static int lex_identifier(tlexer *l);
static int lex_field(tlexer *l);
static int lex_variable(tlexer *l);
static int lex_char(tlexer *l);
static int lex_number(tlexer *l);
static int lex_quote(tlexer *l);
static int lex_raw_quote(tlexer *l);

void tlex_init(tlexer *l, alt_arena *a, alt_str input, alt_str left, alt_str right) {
	memset(l, 0, sizeof *l);
	l->arena = a;
	l->input = input;
	l->ldelim = left.n ? left : ALT_STR("{{");
	l->rdelim = right.n ? right : ALT_STR("}}");
	l->line = 1;
	l->start_line = 1;
}

titem tlex_next(tlexer *l) {
	l->item = (titem){IT_EOF, l->pos, ALT_STR("EOF"), l->start_line};
	static int (*const fns[])(tlexer *) = {0, lex_text, lex_left_delim, lex_comment, lex_right_delim, lex_inside_action, lex_space, lex_identifier, lex_field, lex_variable, lex_char, lex_number, lex_quote, lex_raw_quote};
	int state = l->inside_action ? S_INSIDE_ACTION : S_TEXT;
	while (state != S_DONE)
		state = fns[state](l);
	return l->item;
}

static int lex_text(tlexer *l) {
	ssize_t x = alt_str_index(lrest(l, l->pos), l->ldelim);
	if (x >= 0) {
		if (x > 0) {
			l->pos += (int)x;
			int trim = 0;
			int delim_end = l->pos + (int)l->ldelim.n;
			if (has_left_trim(lrest(l, delim_end)))
				trim = right_trim_len(alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos));
			l->pos -= trim;
			l->line += count_nl(alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos));
			titem i = this_item(l, IT_TEXT);
			l->pos += trim;
			ignore(l);
			if (i.val.n > 0)
				return emit_item(l, i);
		}
		return S_LEFT_DELIM;
	}
	l->pos = (int)l->input.n;
	if (l->pos > l->start) {
		l->line += count_nl(alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos));
		return emit(l, IT_TEXT);
	}
	return emit(l, IT_EOF);
}

static bool at_right_delim(tlexer *l, bool *trim) {
	if (has_right_trim(lrest(l, l->pos)) && alt_str_has_prefix(lrest(l, l->pos + 2), l->rdelim)) {
		*trim = true;
		return true;
	}
	*trim = false;
	return alt_str_has_prefix(lrest(l, l->pos), l->rdelim);
}

static int lex_left_delim(tlexer *l) {
	l->pos += (int)l->ldelim.n;
	bool trim = has_left_trim(lrest(l, l->pos));
	int after = trim ? 2 : 0;
	if (alt_str_has_prefix(lrest(l, l->pos + after), ALT_STR("/*"))) {
		l->pos += after;
		ignore(l);
		return S_COMMENT;
	}
	titem i = this_item(l, IT_LEFT_DELIM);
	l->inside_action = true;
	l->pos += after;
	ignore(l);
	l->paren_depth = 0;
	return emit_item(l, i);
}

static int lex_comment(tlexer *l) {
	l->pos += 2;
	ssize_t x = alt_str_index(lrest(l, l->pos), ALT_STR("*/"));
	if (x < 0)
		return errorf(l, "unclosed comment");
	l->pos += (int)x + 2;
	bool trim;
	if (!at_right_delim(l, &trim))
		return errorf(l, "comment ends before closing delimiter");
	l->line += count_nl(alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos));
	titem i = this_item(l, IT_COMMENT);
	if (trim)
		l->pos += 2;
	l->pos += (int)l->rdelim.n;
	if (trim)
		l->pos += left_trim_len(lrest(l, l->pos));
	ignore(l);
	if (l->emit_comment)
		return emit_item(l, i);
	return S_TEXT;
}

static int lex_right_delim(tlexer *l) {
	bool trim;
	at_right_delim(l, &trim);
	if (trim) {
		l->pos += 2;
		ignore(l);
	}
	l->pos += (int)l->rdelim.n;
	titem i = this_item(l, IT_RIGHT_DELIM);
	if (trim) {
		l->pos += left_trim_len(lrest(l, l->pos));
		ignore(l);
	}
	l->inside_action = false;
	return emit_item(l, i);
}

static int lex_inside_action(tlexer *l) {
	bool trim;
	if (at_right_delim(l, &trim)) {
		if (l->paren_depth == 0)
			return S_RIGHT_DELIM;
		return errorf(l, "unclosed left paren");
	}
	int32_t r = lnext(l);
	if (r == EOF_RUNE)
		return errorf(l, "unclosed action");
	if (is_space(r)) {
		lbackup(l);
		return S_SPACE;
	}
	switch (r) {
	case '=':
		return emit(l, IT_ASSIGN);
	case ':':
		if (lnext(l) != '=')
			return errorf(l, "expected :=");
		return emit(l, IT_DECLARE);
	case '|':
		return emit(l, IT_PIPE);
	case '"':
		return S_QUOTE;
	case '`':
		return S_RAW_QUOTE;
	case '$':
		return S_VARIABLE;
	case '\'':
		return S_CHAR;
	case '.':
		if ((size_t)l->pos < l->input.n) {
			char c = l->input.p[l->pos];
			if (c < '0' || '9' < c)
				return S_FIELD;
		}
		lbackup(l);
		return S_NUMBER;
	case '(':
		l->paren_depth++;
		return emit(l, IT_LEFT_PAREN);
	case ')':
		l->paren_depth--;
		if (l->paren_depth < 0)
			return errorf(l, "unexpected right paren");
		return emit(l, IT_RIGHT_PAREN);
	}
	if (r == '+' || r == '-' || ('0' <= r && r <= '9')) {
		lbackup(l);
		return S_NUMBER;
	}
	if (is_alnum(r)) {
		lbackup(l);
		return S_IDENTIFIER;
	}
	if (r >= 0x20 && r < 0x7f)
		return emit(l, IT_CHAR);
	return errorf(l, unicode_text(l, "unrecognized character in action: ", r));
}

static int lex_space(tlexer *l) {
	int num = 0;
	for (;;) {
		int32_t r = lpeek(l);
		if (!is_space(r))
			break;
		lnext(l);
		num++;
	}
	if (has_right_trim(lrest(l, l->pos - 1)) && alt_str_has_prefix(lrest(l, l->pos - 1 + 2), l->rdelim)) {
		lbackup(l);
		if (num == 1)
			return S_RIGHT_DELIM;
	}
	return emit(l, IT_SPACE);
}

static bool at_terminator(tlexer *l) {
	int32_t r = lpeek(l);
	if (is_space(r))
		return true;
	switch (r) {
	case EOF_RUNE:
	case '.':
	case ',':
	case '|':
	case ':':
	case ')':
	case '(':
		return true;
	}
	return alt_str_has_prefix(lrest(l, l->pos), l->rdelim);
}

static int keyword(alt_str w) {
	static const struct {
		const char *s;
		int t;
	} keys[] = {{".", IT_DOT}, {"block", IT_BLOCK}, {"break", IT_BREAK}, {"continue", IT_CONTINUE}, {"define", IT_DEFINE},
		{"else", IT_ELSE}, {"end", IT_END}, {"if", IT_IF}, {"range", IT_RANGE}, {"nil", IT_NIL}, {"template", IT_TEMPLATE},
		{"with", IT_WITH}};
	for (size_t i = 0; i < sizeof keys / sizeof keys[0]; i++)
		if (alt_str_eq_cstr(w, keys[i].s))
			return keys[i].t;
	return 0;
}

static int lex_identifier(tlexer *l) {
	for (;;) {
		int32_t r = lnext(l);
		if (is_alnum(r))
			continue;
		lbackup(l);
		alt_str word = alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos);
		if (!at_terminator(l))
			return errorf(l, unicode_text(l, "bad character ", r));
		int k = keyword(word);
		if (k > IT_KEYWORD) {
			if ((k == IT_BREAK && !l->break_ok) || (k == IT_CONTINUE && !l->continue_ok))
				return emit(l, IT_IDENTIFIER);
			return emit(l, k);
		}
		if (word.p[0] == '.')
			return emit(l, IT_FIELD);
		if (alt_str_eq_cstr(word, "true") || alt_str_eq_cstr(word, "false"))
			return emit(l, IT_BOOL);
		return emit(l, IT_IDENTIFIER);
	}
}

static int lex_field_or_variable(tlexer *l, int typ) {
	if (at_terminator(l))
		return emit(l, typ == IT_VARIABLE ? IT_VARIABLE : IT_DOT);
	int32_t r;
	for (;;) {
		r = lnext(l);
		if (!is_alnum(r)) {
			lbackup(l);
			break;
		}
	}
	if (!at_terminator(l))
		return errorf(l, unicode_text(l, "bad character ", r));
	return emit(l, typ);
}

static int lex_field(tlexer *l) { return lex_field_or_variable(l, IT_FIELD); }

static int lex_variable(tlexer *l) {
	if (at_terminator(l))
		return emit(l, IT_VARIABLE);
	return lex_field_or_variable(l, IT_VARIABLE);
}

static int lex_char(tlexer *l) {
	for (;;) {
		int32_t r = lnext(l);
		if (r == '\\') {
			int32_t r2 = lnext(l);
			if (r2 != EOF_RUNE && r2 != '\n')
				continue;
			return errorf(l, "unterminated character constant");
		}
		if (r == EOF_RUNE || r == '\n')
			return errorf(l, "unterminated character constant");
		if (r == '\'')
			break;
	}
	return emit(l, IT_CHAR_CONSTANT);
}

static bool scan_number(tlexer *l) {
	accept(l, "+-");
	const char *digits = "0123456789_";
	if (accept(l, "0")) {
		if (accept(l, "xX"))
			digits = "0123456789abcdefABCDEF_";
		else if (accept(l, "oO"))
			digits = "01234567_";
		else if (accept(l, "bB"))
			digits = "01_";
	}
	accept_run(l, digits);
	if (accept(l, "."))
		accept_run(l, digits);
	if (strlen(digits) == 11 && accept(l, "eE")) {
		accept(l, "+-");
		accept_run(l, "0123456789_");
	}
	if (strlen(digits) == 23 && accept(l, "pP")) {
		accept(l, "+-");
		accept_run(l, "0123456789_");
	}
	accept(l, "i");
	if (is_alnum(lpeek(l))) {
		lnext(l);
		return false;
	}
	return true;
}

static int bad_number(tlexer *l) {
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, "bad number syntax: ");
	tquote(&b, alt_str_slice(l->input, (size_t)l->start, (size_t)l->pos));
	char *s = alt_arena_strdup(l->arena, alt_buf_cstr(&b));
	alt_buf_free(&b);
	return errorf(l, s);
}

static int lex_number(tlexer *l) {
	if (!scan_number(l))
		return bad_number(l);
	int32_t sign = lpeek(l);
	if (sign == '+' || sign == '-') {
		if (!scan_number(l) || l->input.p[l->pos - 1] != 'i')
			return bad_number(l);
		return emit(l, IT_COMPLEX);
	}
	return emit(l, IT_NUMBER);
}

static int lex_quote(tlexer *l) {
	for (;;) {
		int32_t r = lnext(l);
		if (r == '\\') {
			int32_t r2 = lnext(l);
			if (r2 != EOF_RUNE && r2 != '\n')
				continue;
			return errorf(l, "unterminated quoted string");
		}
		if (r == EOF_RUNE || r == '\n')
			return errorf(l, "unterminated quoted string");
		if (r == '"')
			break;
	}
	return emit(l, IT_STRING);
}

static int lex_raw_quote(tlexer *l) {
	for (;;) {
		int32_t r = lnext(l);
		if (r == EOF_RUNE)
			return errorf(l, "unterminated raw quoted string");
		if (r == '`')
			break;
	}
	return emit(l, IT_RAW_STRING);
}
