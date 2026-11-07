/* The parser of templates, after Go's text/template/parse/parse.go and
   node.go (BSD-3-Clause, see LICENSE-Go). */
#include "template_internal.h"
#include <alt/strconv.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <math.h>
#include <string.h>

/* the C stack does not grow as Go's: nesting is limited (Go: 10000
   parenthesized pipelines, control structures without limit) */
#define MAX_DEPTH 1000

typedef struct tparser {
	ttree *t;
	tlexer *lex;
	titem token[3];
	int peek_count;
	alt_str *vars;
	size_t nvars, vcap;
	int action_line;
	int range_depth;
	int *depth;
	ttreeset *set;
	const tfuncs *funcs;
	alt_arena *a;
	jmp_buf *jb;
	alt_buf *err;
} tparser;

alt_str tstr_dup(alt_arena *a, alt_str s) {
	char *p = alt_arena_alloc(a, s.n + 1);
	if (s.n)
		memcpy(p, s.p, s.n);
	p[s.n] = 0;
	return alt_str_make(p, s.n);
}

/* ---- strconv: Quote, UnquoteChar, Unquote ---- */

static const char lowerhex[] = "0123456789abcdef";

static void escaped_rune(alt_buf *b, int32_t r, char quote, bool ascii) {
	if (r == quote || r == '\\') {
		alt_buf_append_byte(b, '\\');
		alt_buf_append_byte(b, (int)r);
		return;
	}
	if (ascii ? (r < 0x80 && alt_unicode_is_print(r)) : alt_unicode_is_print(r)) {
		char tmp[4];
		alt_buf_append(b, tmp, alt_utf8_encode(tmp, r));
		return;
	}
	switch (r) {
	case '\a': alt_buf_append_cstr(b, "\\a"); return;
	case '\b': alt_buf_append_cstr(b, "\\b"); return;
	case '\f': alt_buf_append_cstr(b, "\\f"); return;
	case '\n': alt_buf_append_cstr(b, "\\n"); return;
	case '\r': alt_buf_append_cstr(b, "\\r"); return;
	case '\t': alt_buf_append_cstr(b, "\\t"); return;
	case '\v': alt_buf_append_cstr(b, "\\v"); return;
	}
	if (r < ' ' || r == 0x7f) {
		alt_buf_append_cstr(b, "\\x");
		alt_buf_append_byte(b, lowerhex[(r >> 4) & 15]);
		alt_buf_append_byte(b, lowerhex[r & 15]);
	} else if (!alt_utf8_valid_rune(r)) {
		alt_buf_append_cstr(b, "\\ufffd");
	} else if (r < 0x10000) {
		alt_buf_append_cstr(b, "\\u");
		for (int s = 12; s >= 0; s -= 4)
			alt_buf_append_byte(b, lowerhex[(r >> s) & 15]);
	} else {
		alt_buf_append_cstr(b, "\\U");
		for (int s = 28; s >= 0; s -= 4)
			alt_buf_append_byte(b, lowerhex[(r >> s) & 15]);
	}
}

/* Go's appendQuotedWith */
void tquote_with(alt_buf *b, alt_str s, char quote, bool ascii) {
	alt_buf_append_byte(b, quote);
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (w == 1 && r == 0xfffd) {
			alt_buf_append_cstr(b, "\\x");
			alt_buf_append_byte(b, lowerhex[(unsigned char)s.p[i] >> 4]);
			alt_buf_append_byte(b, lowerhex[s.p[i] & 15]);
			i++;
			continue;
		}
		escaped_rune(b, r, quote, ascii);
		i += w;
	}
	alt_buf_append_byte(b, quote);
}

void tquote(alt_buf *b, alt_str s) { tquote_with(b, s, '"', false); }

/* Go's QuoteRune (ascii: QuoteRuneToASCII) */
void tquote_rune(alt_buf *b, int32_t r, bool ascii) {
	if (!alt_utf8_valid_rune(r))
		r = 0xfffd;
	alt_buf_append_byte(b, '\'');
	escaped_rune(b, r, '\'', ascii);
	alt_buf_append_byte(b, '\'');
}

static int unhex(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/* Go's UnquoteChar: 0 and the rune, or -1 (ErrSyntax) */
static int unquote_char(alt_str *s, char quote, int32_t *value, bool *multibyte) {
	*multibyte = false;
	if (!s->n)
		return -1;
	unsigned char c = (unsigned char)s->p[0];
	if (c == (unsigned char)quote && (quote == '\'' || quote == '"'))
		return -1;
	if (c >= 0x80) {
		size_t w = alt_utf8_decode(s->p, s->n, value);
		*s = alt_str_from(*s, w);
		*multibyte = true;
		return 0;
	}
	if (c != '\\') {
		*value = c;
		*s = alt_str_from(*s, 1);
		return 0;
	}
	if (s->n <= 1)
		return -1;
	c = (unsigned char)s->p[1];
	*s = alt_str_from(*s, 2);
	switch (c) {
	case 'a': *value = '\a'; break;
	case 'b': *value = '\b'; break;
	case 'f': *value = '\f'; break;
	case 'n': *value = '\n'; break;
	case 'r': *value = '\r'; break;
	case 't': *value = '\t'; break;
	case 'v': *value = '\v'; break;
	case 'x':
	case 'u':
	case 'U': {
		size_t n = c == 'x' ? 2 : c == 'u' ? 4 : 8;
		if (s->n < n)
			return -1;
		int32_t v = 0;
		for (size_t j = 0; j < n; j++) {
			int x = unhex(s->p[j]);
			if (x < 0)
				return -1;
			v = (int32_t)((uint32_t)v << 4 | (uint32_t)x);
		}
		*s = alt_str_from(*s, n);
		if (c == 'x') {
			*value = v;
			break;
		}
		if (!alt_utf8_valid_rune(v))
			return -1;
		*value = v;
		*multibyte = true;
		break;
	}
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
		int32_t v = c - '0';
		if (s->n < 2)
			return -1;
		for (int j = 0; j < 2; j++) {
			int x = s->p[j] - '0';
			if (x < 0 || x > 7)
				return -1;
			v = v << 3 | x;
		}
		*s = alt_str_from(*s, 2);
		if (v > 255)
			return -1;
		*value = v;
		break;
	}
	case '\\':
		*value = '\\';
		break;
	case '\'':
	case '"':
		if (c != (unsigned char)quote)
			return -1;
		*value = c;
		break;
	default:
		return -1;
	}
	return 0;
}

/* Go's strconv.Unquote: 0, or -1 for "invalid syntax" */
static int unquote(alt_arena *a, alt_str in, alt_str *out) {
	if (in.n < 2)
		return -1;
	char quote = in.p[0];
	ssize_t e = alt_str_index_byte(alt_str_from(in, 1), quote);
	if (e < 0)
		return -1;
	size_t end = (size_t)e + 2;
	if (quote == '`') {
		if (end != in.n)
			return -1;
		alt_buf b = ALT_BUF_INIT;
		for (size_t i = 1; i + 1 < end; i++)
			if (in.p[i] != '\r')
				alt_buf_append_byte(&b, in.p[i]);
		*out = tstr_dup(a, alt_buf_str(&b));
		alt_buf_free(&b);
		return 0;
	}
	if (quote != '"' && quote != '\'')
		return -1;
	alt_str body = alt_str_slice(in, 1, end - 1);
	if (alt_str_index_byte(body, '\\') < 0 && alt_str_index_byte(body, '\n') < 0) {
		bool valid;
		if (quote == '"')
			valid = alt_utf8_valid(body.p, body.n);
		else {
			int32_t r;
			size_t n = body.n ? alt_utf8_decode(body.p, body.n, &r) : 0;
			valid = body.n && 2 + n == end && (r != 0xfffd || n != 1);
		}
		if (valid) {
			if (end != in.n)
				return -1;
			*out = tstr_dup(a, body);
			return 0;
		}
	}
	alt_buf b = ALT_BUF_INIT;
	alt_str s = alt_str_from(in, 1);
	while (s.n > 0 && s.p[0] != quote) {
		int32_t r;
		bool mb;
		char c0 = s.p[0];
		if (unquote_char(&s, quote, &r, &mb) < 0 || c0 == '\n') {
			alt_buf_free(&b);
			return -1;
		}
		if (r < 0x80 || !mb)
			alt_buf_append_byte(&b, (int)(r & 0xff));
		else {
			char tmp[4];
			alt_buf_append(&b, tmp, alt_utf8_encode(tmp, r));
		}
		if (quote == '\'')
			break;
	}
	if (!(s.n > 0 && s.p[0] == quote) || s.n != 1) {
		alt_buf_free(&b);
		return -1;
	}
	*out = tstr_dup(a, alt_buf_str(&b));
	alt_buf_free(&b);
	return 0;
}

/* ---- nodes ---- */

tnode *tnode_new(alt_arena *a, int type, int pos, ttree *tr) {
	tnode *n = alt_arena_zalloc(a, sizeof *n);
	n->type = type;
	n->pos = pos;
	n->tr = tr;
	return n;
}

void tnode_append(alt_arena *a, tnode *list, tnode *n) {
	if (list->nn == list->ncap) {
		size_t nc = list->ncap ? 2 * list->ncap : 4;
		list->nodes = alt_arena_realloc(a, list->nodes, list->ncap * sizeof(tnode *), nc * sizeof(tnode *));
		list->ncap = nc;
	}
	list->nodes[list->nn++] = n;
}

void tnode_insert(alt_arena *a, tnode *list, size_t at, tnode *n) {
	tnode_append(a, list, n);
	memmove(list->nodes + at + 1, list->nodes + at, (list->nn - 1 - at) * sizeof(tnode *));
	list->nodes[at] = n;
}

static alt_str *split_dots(alt_arena *a, alt_str s, size_t *n) {
	size_t k = alt_str_count(s, ALT_STR(".")) + 1;
	alt_str *v = alt_arena_alloc(a, k * sizeof *v);
	alt_str f;
	*n = 0;
	while (alt_str_split_next(&s, ALT_STR("."), &f))
		v[(*n)++] = f;
	return v;
}

static void write_to(alt_buf *b, const tnode *n);

static void write_list(alt_buf *b, const tnode *l) {
	for (size_t i = 0; l && i < l->nn; i++)
		write_to(b, l->nodes[i]);
}

static void write_pipe(alt_buf *b, const tnode *p) {
	if (p->ndecl) {
		for (size_t i = 0; i < p->ndecl; i++) {
			if (i)
				alt_buf_append_cstr(b, ", ");
			write_to(b, p->decl[i]);
		}
		alt_buf_append_cstr(b, p->is_assign ? " = " : " := ");
	}
	for (size_t i = 0; i < p->nn; i++) {
		if (i)
			alt_buf_append_cstr(b, " | ");
		write_to(b, p->nodes[i]);
	}
}

static void write_to(alt_buf *b, const tnode *n) {
	switch (n->type) {
	case N_LIST:
		write_list(b, n);
		break;
	case N_TEXT:
		alt_buf_append_str(b, n->text);
		break;
	case N_COMMENT:
		alt_buf_append_str(b, n->tr->ldelim);
		alt_buf_append_str(b, n->text);
		alt_buf_append_str(b, n->tr->rdelim);
		break;
	case N_PIPE:
		write_pipe(b, n);
		break;
	case N_ACTION:
		alt_buf_append_str(b, n->tr->ldelim);
		write_pipe(b, n->node);
		alt_buf_append_str(b, n->tr->rdelim);
		break;
	case N_COMMAND:
		for (size_t i = 0; i < n->nn; i++) {
			if (i)
				alt_buf_append_byte(b, ' ');
			if (n->nodes[i]->type == N_PIPE) {
				alt_buf_append_byte(b, '(');
				write_pipe(b, n->nodes[i]);
				alt_buf_append_byte(b, ')');
				continue;
			}
			write_to(b, n->nodes[i]);
		}
		break;
	case N_IDENTIFIER:
	case N_NUMBER:
		alt_buf_append_str(b, n->text);
		break;
	case N_STRING:
		alt_buf_append_str(b, n->quoted);
		break;
	case N_VARIABLE:
		for (size_t i = 0; i < n->nident; i++) {
			if (i)
				alt_buf_append_byte(b, '.');
			alt_buf_append_str(b, n->ident[i]);
		}
		break;
	case N_FIELD:
		for (size_t i = 0; i < n->nident; i++) {
			alt_buf_append_byte(b, '.');
			alt_buf_append_str(b, n->ident[i]);
		}
		break;
	case N_CHAIN:
		if (n->node->type == N_PIPE) {
			alt_buf_append_byte(b, '(');
			write_to(b, n->node);
			alt_buf_append_byte(b, ')');
		} else
			write_to(b, n->node);
		for (size_t i = 0; i < n->nident; i++) {
			alt_buf_append_byte(b, '.');
			alt_buf_append_str(b, n->ident[i]);
		}
		break;
	case N_DOT:
		alt_buf_append_byte(b, '.');
		break;
	case N_NIL:
		alt_buf_append_cstr(b, "nil");
		break;
	case N_BOOL:
		alt_buf_append_cstr(b, n->b ? "true" : "false");
		break;
	case N_END:
	case N_ELSE:
	case N_BREAK:
	case N_CONTINUE:
		alt_buf_append_str(b, n->tr->ldelim);
		alt_buf_append_cstr(b, n->type == N_END ? "end" : n->type == N_ELSE ? "else" : n->type == N_BREAK ? "break" : "continue");
		alt_buf_append_str(b, n->tr->rdelim);
		break;
	case N_IF:
	case N_RANGE:
	case N_WITH:
		alt_buf_append_str(b, n->tr->ldelim);
		alt_buf_append_cstr(b, n->type == N_IF ? "if " : n->type == N_RANGE ? "range " : "with ");
		write_pipe(b, n->node);
		alt_buf_append_str(b, n->tr->rdelim);
		write_list(b, n->list);
		if (n->else_list) {
			alt_buf_append_str(b, n->tr->ldelim);
			alt_buf_append_cstr(b, "else");
			alt_buf_append_str(b, n->tr->rdelim);
			write_list(b, n->else_list);
		}
		alt_buf_append_str(b, n->tr->ldelim);
		alt_buf_append_cstr(b, "end");
		alt_buf_append_str(b, n->tr->rdelim);
		break;
	case N_TEMPLATE:
		alt_buf_append_str(b, n->tr->ldelim);
		alt_buf_append_cstr(b, "template ");
		tquote(b, n->text);
		if (n->node) {
			alt_buf_append_byte(b, ' ');
			write_pipe(b, n->node);
		}
		alt_buf_append_str(b, n->tr->rdelim);
		break;
	}
}

void tnode_string(alt_buf *b, const tnode *n) { write_to(b, n); }

tnode *tnode_copy(alt_arena *a, const tnode *n) {
	if (!n)
		return 0;
	tnode *c = alt_arena_alloc(a, sizeof *c);
	*c = *n;
	if (n->nn) {
		c->nodes = alt_arena_alloc(a, n->nn * sizeof(tnode *));
		c->ncap = n->nn;
		for (size_t i = 0; i < n->nn; i++)
			c->nodes[i] = tnode_copy(a, n->nodes[i]);
	} else {
		c->nodes = 0;
		c->ncap = 0;
	}
	if (n->ndecl) {
		c->decl = alt_arena_alloc(a, n->ndecl * sizeof(tnode *));
		for (size_t i = 0; i < n->ndecl; i++)
			c->decl[i] = tnode_copy(a, n->decl[i]);
	}
	if (n->nident) {
		c->ident = alt_arena_alloc(a, n->nident * sizeof(alt_str));
		memcpy(c->ident, n->ident, n->nident * sizeof(alt_str));
	}
	/* a chain's node is shared, as in Go */
	if (n->type != N_CHAIN)
		c->node = tnode_copy(a, n->node);
	c->list = tnode_copy(a, n->list);
	c->else_list = tnode_copy(a, n->else_list);
	return c;
}

bool tree_is_empty(const tnode *n) {
	if (!n)
		return true;
	switch (n->type) {
	case N_COMMENT:
		return true;
	case N_LIST:
		for (size_t i = 0; i < n->nn; i++)
			if (!tree_is_empty(n->nodes[i]))
				return false;
		return true;
	case N_TEXT:
		return alt_str_trim_space(n->text).n == 0;
	}
	return false;
}

/* ---- the parser ---- */

static void __attribute__((noreturn)) perrorf(tparser *p, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void perrorf(tparser *p, const char *fmt, ...) {
	p->t->root = 0;
	alt_buf_printf(p->err, "template: %.*s:%d: ", (int)p->t->parse_name.n, p->t->parse_name.p, p->token[0].line);
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(p->err, fmt, ap);
	va_end(ap);
	longjmp(*p->jb, 1);
}

static titem pnext(tparser *p) {
	if (p->peek_count > 0)
		p->peek_count--;
	else
		p->token[0] = tlex_next(p->lex);
	return p->token[p->peek_count];
}

static void pbackup(tparser *p) { p->peek_count++; }

static void pbackup2(tparser *p, titem t1) {
	p->token[1] = t1;
	p->peek_count = 2;
}

static void pbackup3(tparser *p, titem t2, titem t1) {
	p->token[1] = t1;
	p->token[2] = t2;
	p->peek_count = 3;
}

static titem ppeek(tparser *p) {
	if (p->peek_count > 0)
		return p->token[p->peek_count - 1];
	p->peek_count = 1;
	p->token[0] = tlex_next(p->lex);
	return p->token[0];
}

static titem next_non_space(tparser *p) {
	titem t;
	do
		t = pnext(p);
	while (t.typ == IT_SPACE);
	return t;
}

static titem peek_non_space(tparser *p) {
	titem t = next_non_space(p);
	pbackup(p);
	return t;
}

/* item.String() */
static void item_string(alt_buf *b, titem i) {
	if (i.typ == IT_EOF)
		alt_buf_append_cstr(b, "EOF");
	else if (i.typ == IT_ERROR)
		alt_buf_append_str(b, i.val);
	else if (i.typ > IT_KEYWORD) {
		alt_buf_append_byte(b, '<');
		alt_buf_append_str(b, i.val);
		alt_buf_append_byte(b, '>');
	} else if (i.val.n > 10) {
		/* %.10q... */
		size_t k = 0, runes = 0;
		while (k < i.val.n && runes < 10) {
			int32_t r;
			k += alt_utf8_decode(i.val.p + k, i.val.n - k, &r);
			runes++;
		}
		tquote(b, alt_str_slice(i.val, 0, k));
		alt_buf_append_cstr(b, "...");
	} else
		tquote(b, i.val);
}

static void __attribute__((noreturn)) unexpected(tparser *p, titem tok, const char *context) {
	alt_buf s = ALT_BUF_INIT;
	item_string(&s, tok);
	if (tok.typ == IT_ERROR) {
		alt_buf extra = ALT_BUF_INIT;
		if (p->action_line != 0 && p->action_line != tok.line) {
			alt_buf_printf(&extra, " in action started at %.*s:%d", (int)p->t->parse_name.n, p->t->parse_name.p, p->action_line);
			if (alt_str_has_suffix(tok.val, ALT_STR(" action")))
				alt_buf_consume(&extra, strlen(" in action"));
		}
		alt_buf_append_str(&s, alt_buf_str(&extra));
		alt_buf_free(&extra);
		char *m = alt_arena_strdup(p->a, alt_buf_cstr(&s));
		alt_buf_free(&s);
		perrorf(p, "%s", m);
	}
	char *m = alt_arena_strdup(p->a, alt_buf_cstr(&s));
	alt_buf_free(&s);
	perrorf(p, "unexpected %s in %s", m, context);
}

static titem expect(tparser *p, int typ, const char *context) {
	titem t = next_non_space(p);
	if (t.typ != typ)
		unexpected(p, t, context);
	return t;
}

static titem expect_one_of(tparser *p, int t1, int t2, const char *context) {
	titem t = next_non_space(p);
	if (t.typ != t1 && t.typ != t2)
		unexpected(p, t, context);
	return t;
}

static bool has_function(const tparser *p, alt_str name) { return tfuncs_find(p->funcs, name) || tbuiltin_find(name); }

static void push_var(tparser *p, alt_str name) {
	if (p->nvars == p->vcap) {
		size_t nc = p->vcap ? 2 * p->vcap : 8;
		p->vars = alt_arena_realloc(p->a, p->vars, p->vcap * sizeof(alt_str), nc * sizeof(alt_str));
		p->vcap = nc;
	}
	p->vars[p->nvars++] = name;
}

static void start_parse(tparser *p, ttree *t, tlexer *lex, tparser *parent) {
	memset(p, 0, sizeof *p);
	p->t = t;
	p->lex = lex;
	p->a = parent->a;
	p->jb = parent->jb;
	p->err = parent->err;
	p->funcs = parent->funcs;
	p->set = parent->set;
	p->depth = parent->depth;
	push_var(p, ALT_STR("$"));
}

static ttree *new_tree(tparser *p, alt_str name) {
	ttree *t = alt_arena_zalloc(p->a, sizeof *t);
	t->name = name;
	t->text = p->t->text;
	t->ldelim = p->t->ldelim;
	t->rdelim = p->t->rdelim;
	t->parse_name = p->t->parse_name;
	return t;
}

static tnode *new_node(tparser *p, int type, int pos) { return tnode_new(p->a, type, pos, p->t); }

static void add_tree(tparser *p) {
	ttreeset *set = p->set;
	for (size_t i = 0; i < set->n; i++) {
		if (alt_str_eq(set->trees[i]->name, p->t->name)) {
			if (tree_is_empty(set->trees[i]->root)) {
				set->trees[i] = p->t;
				return;
			}
			if (!tree_is_empty(p->t->root)) {
				alt_buf q = ALT_BUF_INIT;
				tquote(&q, p->t->name);
				char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
				alt_buf_free(&q);
				perrorf(p, "template: multiple definition of template %s", m);
			}
			return;
		}
	}
	if (set->n == set->cap) {
		size_t nc = set->cap ? 2 * set->cap : 4;
		set->trees = alt_arena_realloc(p->a, set->trees, set->cap * sizeof(ttree *), nc * sizeof(ttree *));
		set->cap = nc;
	}
	set->trees[set->n++] = p->t;
}

static tnode *text_or_action(tparser *p);
static tnode *pipeline(tparser *p, const char *context, int end);
static tnode *item_list(tparser *p, tnode **next);

static void enter(tparser *p) {
	if (++*p->depth > MAX_DEPTH)
		perrorf(p, "max expression depth exceeded");
}

static void leave(tparser *p) { --*p->depth; }

static void parse_definition(tparser *p) {
	const char *context = "define clause";
	titem name = expect_one_of(p, IT_STRING, IT_RAW_STRING, context);
	if (unquote(p->a, name.val, &p->t->name) < 0)
		perrorf(p, "invalid syntax");
	expect(p, IT_RIGHT_DELIM, context);
	tnode *end;
	p->t->root = item_list(p, &end);
	if (end->type != N_END) {
		alt_buf s = ALT_BUF_INIT;
		tnode_string(&s, end);
		char *m = alt_arena_strdup(p->a, alt_buf_cstr(&s));
		alt_buf_free(&s);
		perrorf(p, "unexpected %s in %s", m, context);
	}
	add_tree(p);
}

static void parse_top(tparser *p) {
	p->t->root = new_node(p, N_LIST, ppeek(p).pos);
	while (ppeek(p).typ != IT_EOF) {
		if (ppeek(p).typ == IT_LEFT_DELIM) {
			titem delim = pnext(p);
			if (next_non_space(p).typ == IT_DEFINE) {
				tparser sub;
				start_parse(&sub, new_tree(p, ALT_STR("definition")), p->lex, p);
				parse_definition(&sub);
				continue;
			}
			pbackup2(p, delim);
		}
		tnode *n = text_or_action(p);
		if (n->type == N_END || n->type == N_ELSE) {
			alt_buf s = ALT_BUF_INIT;
			tnode_string(&s, n);
			char *m = alt_arena_strdup(p->a, alt_buf_cstr(&s));
			alt_buf_free(&s);
			perrorf(p, "unexpected %s", m);
		}
		tnode_append(p->a, p->t->root, n);
	}
}

static tnode *item_list(tparser *p, tnode **next) {
	tnode *list = new_node(p, N_LIST, peek_non_space(p).pos);
	while (peek_non_space(p).typ != IT_EOF) {
		tnode *n = text_or_action(p);
		if (n->type == N_END || n->type == N_ELSE) {
			*next = n;
			return list;
		}
		tnode_append(p->a, list, n);
	}
	perrorf(p, "unexpected EOF");
}

static tnode *action(tparser *p);

static tnode *text_or_action(tparser *p) {
	titem tok = next_non_space(p);
	switch (tok.typ) {
	case IT_TEXT: {
		tnode *n = new_node(p, N_TEXT, tok.pos);
		n->text = tok.val;
		return n;
	}
	case IT_LEFT_DELIM: {
		p->action_line = tok.line;
		tnode *n = action(p);
		p->action_line = 0;
		return n;
	}
	case IT_COMMENT: {
		tnode *n = new_node(p, N_COMMENT, tok.pos);
		n->text = tok.val;
		return n;
	}
	}
	unexpected(p, tok, "input");
}

static tnode *new_pipeline(tparser *p, int pos, int line) {
	tnode *n = new_node(p, N_PIPE, pos);
	n->line = line;
	return n;
}

static tnode *new_variable(tparser *p, int pos, alt_str ident) {
	tnode *n = new_node(p, N_VARIABLE, pos);
	n->ident = split_dots(p->a, ident, &n->nident);
	return n;
}

static void add_decl(tparser *p, tnode *pipe, tnode *v) {
	pipe->decl = alt_arena_realloc(p->a, pipe->decl, pipe->ndecl * sizeof(tnode *), (pipe->ndecl + 1) * sizeof(tnode *));
	pipe->decl[pipe->ndecl++] = v;
}

static tnode *command(tparser *p);

static void check_pipeline(tparser *p, tnode *pipe, const char *context) {
	if (pipe->nn == 0)
		perrorf(p, "missing value for %s", context);
	for (size_t i = 1; i < pipe->nn; i++) {
		switch (pipe->nodes[i]->nodes[0]->type) {
		case N_BOOL:
		case N_DOT:
		case N_NIL:
		case N_NUMBER:
		case N_STRING:
			perrorf(p, "non executable command in pipeline stage %zu", i + 1);
		}
	}
}

static tnode *pipeline(tparser *p, const char *context, int end) {
	titem tok = peek_non_space(p);
	tnode *pipe = new_pipeline(p, tok.pos, tok.line);
decls:;
	titem v = peek_non_space(p);
	if (v.typ == IT_VARIABLE) {
		pnext(p);
		titem after = ppeek(p);
		titem next = peek_non_space(p);
		if (next.typ == IT_ASSIGN || next.typ == IT_DECLARE) {
			pipe->is_assign = next.typ == IT_ASSIGN;
			next_non_space(p);
			add_decl(p, pipe, new_variable(p, v.pos, v.val));
			push_var(p, v.val);
		} else if (next.typ == IT_CHAR && alt_str_eq_cstr(next.val, ",")) {
			next_non_space(p);
			add_decl(p, pipe, new_variable(p, v.pos, v.val));
			push_var(p, v.val);
			if (!strcmp(context, "range") && pipe->ndecl < 2) {
				switch (peek_non_space(p).typ) {
				case IT_VARIABLE:
				case IT_RIGHT_DELIM:
				case IT_RIGHT_PAREN:
					goto decls;
				default:
					perrorf(p, "range can only initialize variables");
				}
			}
			perrorf(p, "too many declarations in %s", context);
		} else if (after.typ == IT_SPACE)
			pbackup3(p, v, after);
		else
			pbackup2(p, v);
	}
	for (;;) {
		titem t = next_non_space(p);
		if (t.typ == end) {
			check_pipeline(p, pipe, context);
			return pipe;
		}
		switch (t.typ) {
		case IT_BOOL:
		case IT_CHAR_CONSTANT:
		case IT_COMPLEX:
		case IT_DOT:
		case IT_FIELD:
		case IT_IDENTIFIER:
		case IT_NUMBER:
		case IT_NIL:
		case IT_RAW_STRING:
		case IT_STRING:
		case IT_VARIABLE:
		case IT_LEFT_PAREN:
			pbackup(p);
			tnode_append(p->a, pipe, command(p));
			break;
		default:
			unexpected(p, t, context);
		}
	}
}

static tnode *if_control(tparser *p);
static tnode *with_control(tparser *p);

static tnode *parse_control(tparser *p, const char *context, int type) {
	size_t mark = p->nvars;
	enter(p);
	tnode *pipe = pipeline(p, context, IT_RIGHT_DELIM);
	bool is_range = !strcmp(context, "range");
	if (is_range)
		p->range_depth++;
	tnode *next;
	tnode *list = item_list(p, &next), *else_list = 0;
	if (is_range)
		p->range_depth--;
	if (next->type == N_ELSE) {
		if (!strcmp(context, "if") && ppeek(p).typ == IT_IF) {
			pnext(p);
			else_list = new_node(p, N_LIST, next->pos);
			tnode_append(p->a, else_list, if_control(p));
		} else if (!strcmp(context, "with") && ppeek(p).typ == IT_WITH) {
			pnext(p);
			else_list = new_node(p, N_LIST, next->pos);
			tnode_append(p->a, else_list, with_control(p));
		} else {
			else_list = item_list(p, &next);
			if (next->type != N_END) {
				alt_buf s = ALT_BUF_INIT;
				tnode_string(&s, next);
				char *m = alt_arena_strdup(p->a, alt_buf_cstr(&s));
				alt_buf_free(&s);
				perrorf(p, "expected end; found %s", m);
			}
		}
	}
	leave(p);
	p->nvars = mark;
	tnode *n = new_node(p, type, pipe->pos);
	n->line = pipe->line;
	n->node = pipe;
	n->list = list;
	n->else_list = else_list;
	return n;
}

static tnode *if_control(tparser *p) { return parse_control(p, "if", N_IF); }
static tnode *with_control(tparser *p) { return parse_control(p, "with", N_WITH); }

static alt_str template_name(tparser *p, titem tok, const char *context) {
	alt_str s;
	if (tok.typ == IT_STRING || tok.typ == IT_RAW_STRING) {
		if (unquote(p->a, tok.val, &s) < 0)
			perrorf(p, "invalid syntax");
		return s;
	}
	unexpected(p, tok, context);
}

static tnode *new_template(tparser *p, titem tok, alt_str name, tnode *pipe) {
	tnode *n = new_node(p, N_TEMPLATE, tok.pos);
	n->line = tok.line;
	n->text = name;
	n->node = pipe;
	return n;
}

static tnode *block_control(tparser *p) {
	const char *context = "block clause";
	titem tok = next_non_space(p);
	alt_str name = template_name(p, tok, context);
	tnode *pipe = pipeline(p, context, IT_RIGHT_DELIM);
	tparser sub;
	start_parse(&sub, new_tree(p, name), p->lex, p);
	tnode *end;
	enter(p);
	sub.t->root = item_list(&sub, &end);
	leave(p);
	if (end->type != N_END) {
		alt_buf s = ALT_BUF_INIT;
		tnode_string(&s, end);
		char *m = alt_arena_strdup(p->a, alt_buf_cstr(&s));
		alt_buf_free(&s);
		perrorf(p, "unexpected %s in %s", m, context);
	}
	add_tree(&sub);
	return new_template(p, tok, name, pipe);
}

static tnode *template_control(tparser *p) {
	const char *context = "template clause";
	titem tok = next_non_space(p);
	alt_str name = template_name(p, tok, context);
	tnode *pipe = 0;
	if (next_non_space(p).typ != IT_RIGHT_DELIM) {
		pbackup(p);
		pipe = pipeline(p, context, IT_RIGHT_DELIM);
	}
	return new_template(p, tok, name, pipe);
}

static tnode *action(tparser *p) {
	titem tok = next_non_space(p);
	switch (tok.typ) {
	case IT_BLOCK:
		return block_control(p);
	case IT_BREAK:
	case IT_CONTINUE: {
		bool brk = tok.typ == IT_BREAK;
		titem t = next_non_space(p);
		if (t.typ != IT_RIGHT_DELIM)
			unexpected(p, t, brk ? "{{break}}" : "{{continue}}");
		if (p->range_depth == 0)
			perrorf(p, brk ? "{{break}} outside {{range}}" : "{{continue}} outside {{range}}");
		tnode *n = new_node(p, brk ? N_BREAK : N_CONTINUE, tok.pos);
		n->line = tok.line;
		return n;
	}
	case IT_ELSE: {
		titem pk = peek_non_space(p);
		if (pk.typ == IT_IF || pk.typ == IT_WITH) {
			tnode *n = new_node(p, N_ELSE, pk.pos);
			n->line = pk.line;
			return n;
		}
		titem t = expect(p, IT_RIGHT_DELIM, "else");
		tnode *n = new_node(p, N_ELSE, t.pos);
		n->line = t.line;
		return n;
	}
	case IT_END:
		return new_node(p, N_END, expect(p, IT_RIGHT_DELIM, "end").pos);
	case IT_IF:
		return if_control(p);
	case IT_RANGE:
		return parse_control(p, "range", N_RANGE);
	case IT_TEMPLATE:
		return template_control(p);
	case IT_WITH:
		return with_control(p);
	}
	pbackup(p);
	titem t = ppeek(p);
	tnode *n = new_node(p, N_ACTION, t.pos);
	n->line = t.line;
	n->node = pipeline(p, "command", IT_RIGHT_DELIM);
	return n;
}

/* ---- numbers ---- */

static bool f_is_int64(double f) { return f >= -9223372036854775808.0 && f < 9223372036854775808.0 && (double)(long long)f == f; }
static bool f_is_uint64(double f) { return f >= 0 && f < 18446744073709551616.0 && (double)(unsigned long long)f == f; }

static void simplify_complex(tnode *n) {
	n->is_float = n->im == 0;
	if (n->is_float) {
		n->f64 = n->re;
		n->is_int = f_is_int64(n->f64);
		if (n->is_int)
			n->i64 = (long long)n->f64;
		n->is_uint = f_is_uint64(n->f64);
		if (n->is_uint)
			n->u64 = (unsigned long long)n->f64;
	}
}

static bool contains_any(alt_str s, const char *chars) { return alt_str_index_any(s, chars) >= 0; }

/* fmt's floatToken: the length of the float at the start of s */
static size_t float_token(alt_str s) {
	size_t i = 0;
#define ACCEPT(set) (i < s.n && strchr(set, s.p[i]) ? (i++, true) : false)
	if (ACCEPT("nN") && ACCEPT("aA") && ACCEPT("nN"))
		return i;
	ACCEPT("+-");
	if (ACCEPT("iI") && ACCEPT("nN") && ACCEPT("fF"))
		return i;
	const char *digits = "0123456789_", *exp = "eEpP";
	if (ACCEPT("0") && ACCEPT("xX")) {
		digits = "0123456789aAbBcCdDeEfF_";
		exp = "pP";
	}
	while (ACCEPT(digits))
		;
	if (ACCEPT("."))
		while (ACCEPT(digits))
			;
	if (ACCEPT(exp)) {
		ACCEPT("+-");
		while (ACCEPT("0123456789_"))
			;
	}
#undef ACCEPT
	return i;
}

/* fmt's convertFloat (64 bits) */
static int convert_float(tparser *p, alt_str str, double *out) {
	int rc;
	if (alt_str_index_byte(str, 'p') >= 0 && alt_str_index_any(str, "xX") < 0) {
		ssize_t k = alt_str_index_byte(str, 'p');
		double f;
		rc = alt_parse_float(alt_str_slice(str, 0, (size_t)k), &f);
		if (rc == 0) {
			long long m;
			if (alt_parse_int(alt_str_from(str, (size_t)k + 1), 10, 0, &m) < 0) {
				alt_buf q = ALT_BUF_INIT;
				tquote(&q, str);
				char *msg = alt_arena_strdup(p->a, alt_buf_cstr(&q));
				alt_buf_free(&q);
				perrorf(p, "strconv.Atoi: parsing %s: invalid syntax", msg);
			}
			*out = ldexp(f, (int)(m > 100000 ? 100000 : m < -100000 ? -100000 : m));
			return 0;
		}
	} else
		rc = alt_parse_float(str, out);
	if (rc < 0) {
		alt_buf q = ALT_BUF_INIT;
		tquote(&q, str);
		char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
		alt_buf_free(&q);
		perrorf(p, "strconv.ParseFloat: parsing %s: %s", m, rc == -ALT_ERANGE ? "value out of range" : "invalid syntax");
	}
	return 0;
}

/* fmt.Sscan of a complex number */
static int scan_complex(tparser *p, alt_str text, double *re, double *im) {
	size_t k = float_token(text);
	alt_str sre = alt_str_slice(text, 0, k);
	if (k >= text.n || (text.p[k] != '+' && text.p[k] != '-'))
		perrorf(p, "syntax error scanning complex number");
	size_t k2 = k + 1 + float_token(alt_str_from(text, k + 1));
	alt_str sim = alt_str_slice(text, k, k2);
	if (k2 >= text.n || text.p[k2] != 'i')
		perrorf(p, "syntax error scanning complex number");
	convert_float(p, sre, re);
	convert_float(p, sim, im);
	return 0;
}

static tnode *new_number(tparser *p, int pos, alt_str text, int typ) {
	tnode *n = new_node(p, N_NUMBER, pos);
	n->text = text;
	if (typ == IT_CHAR_CONSTANT) {
		alt_str s = alt_str_from(text, 1);
		int32_t r;
		bool mb;
		if (unquote_char(&s, text.p[0], &r, &mb) < 0)
			perrorf(p, "invalid syntax");
		if (!alt_str_eq_cstr(s, "'"))
			perrorf(p, "malformed character constant: %.*s", (int)text.n, text.p);
		n->i64 = r;
		n->is_int = true;
		n->u64 = (unsigned long long)r;
		n->is_uint = true;
		n->f64 = r;
		n->is_float = true;
		return n;
	}
	if (typ == IT_COMPLEX) {
		scan_complex(p, text, &n->re, &n->im);
		n->is_complex = true;
		simplify_complex(n);
		return n;
	}
	if (text.n > 0 && text.p[text.n - 1] == 'i') {
		double f;
		if (alt_parse_float(alt_str_slice(text, 0, text.n - 1), &f) == 0) {
			n->is_complex = true;
			n->re = 0;
			n->im = f;
			simplify_complex(n);
			return n;
		}
	}
	unsigned long long u;
	if (alt_parse_uint(text, 0, 64, &u) == 0) {
		n->is_uint = true;
		n->u64 = u;
	}
	long long i;
	if (alt_parse_int(text, 0, 64, &i) == 0) {
		n->is_int = true;
		n->i64 = i;
		if (i == 0) {
			n->is_uint = true;
			n->u64 = u;
		}
	}
	if (n->is_int) {
		n->is_float = true;
		n->f64 = (double)n->i64;
	} else if (n->is_uint) {
		n->is_float = true;
		n->f64 = (double)n->u64;
	} else {
		double f;
		if (alt_parse_float(text, &f) == 0) {
			if (!contains_any(text, ".eEpP")) {
				alt_buf q = ALT_BUF_INIT;
				tquote(&q, text);
				char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
				alt_buf_free(&q);
				perrorf(p, "integer overflow: %s", m);
			}
			n->is_float = true;
			n->f64 = f;
			if (!n->is_int && f_is_int64(f)) {
				n->is_int = true;
				n->i64 = (long long)f;
			}
			if (!n->is_uint && f_is_uint64(f)) {
				n->is_uint = true;
				n->u64 = (unsigned long long)f;
			}
		}
	}
	if (!n->is_int && !n->is_uint && !n->is_float) {
		alt_buf q = ALT_BUF_INIT;
		tquote(&q, text);
		char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
		alt_buf_free(&q);
		perrorf(p, "illegal number syntax: %s", m);
	}
	return n;
}

/* ---- commands and operands ---- */

static tnode *use_var(tparser *p, int pos, alt_str name) {
	tnode *v = new_variable(p, pos, name);
	for (size_t i = 0; i < p->nvars; i++)
		if (alt_str_eq(p->vars[i], v->ident[0]))
			return v;
	alt_buf q = ALT_BUF_INIT;
	tquote(&q, v->ident[0]);
	char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
	alt_buf_free(&q);
	perrorf(p, "undefined variable %s", m);
}

static tnode *term(tparser *p) {
	titem tok = next_non_space(p);
	switch (tok.typ) {
	case IT_IDENTIFIER: {
		if (!has_function(p, tok.val)) {
			alt_buf q = ALT_BUF_INIT;
			tquote(&q, tok.val);
			char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
			alt_buf_free(&q);
			perrorf(p, "function %s not defined", m);
		}
		tnode *n = new_node(p, N_IDENTIFIER, tok.pos);
		n->text = tok.val;
		return n;
	}
	case IT_DOT:
		return new_node(p, N_DOT, tok.pos);
	case IT_NIL:
		return new_node(p, N_NIL, tok.pos);
	case IT_VARIABLE:
		return use_var(p, tok.pos, tok.val);
	case IT_FIELD: {
		tnode *n = new_node(p, N_FIELD, tok.pos);
		n->ident = split_dots(p->a, alt_str_from(tok.val, 1), &n->nident);
		return n;
	}
	case IT_BOOL: {
		tnode *n = new_node(p, N_BOOL, tok.pos);
		n->b = alt_str_eq_cstr(tok.val, "true");
		return n;
	}
	case IT_CHAR_CONSTANT:
	case IT_COMPLEX:
	case IT_NUMBER:
		return new_number(p, tok.pos, tok.val, tok.typ);
	case IT_LEFT_PAREN: {
		enter(p);
		tnode *n = pipeline(p, "parenthesized pipeline", IT_RIGHT_PAREN);
		leave(p);
		return n;
	}
	case IT_STRING:
	case IT_RAW_STRING: {
		alt_str s;
		if (unquote(p->a, tok.val, &s) < 0)
			perrorf(p, "invalid syntax");
		tnode *n = new_node(p, N_STRING, tok.pos);
		n->quoted = tok.val;
		n->text = s;
		return n;
	}
	}
	pbackup(p);
	return 0;
}

static tnode *operand(tparser *p) {
	tnode *node = term(p);
	if (!node)
		return 0;
	if (ppeek(p).typ == IT_FIELD) {
		tnode *chain = new_node(p, N_CHAIN, ppeek(p).pos);
		chain->node = node;
		while (ppeek(p).typ == IT_FIELD) {
			alt_str f = alt_str_from(pnext(p).val, 1);
			chain->ident = alt_arena_realloc(p->a, chain->ident, chain->nident * sizeof(alt_str), (chain->nident + 1) * sizeof(alt_str));
			chain->ident[chain->nident++] = f;
		}
		switch (node->type) {
		case N_FIELD:
		case N_VARIABLE: {
			alt_buf s = ALT_BUF_INIT;
			tnode_string(&s, chain);
			alt_str str = tstr_dup(p->a, alt_buf_str(&s));
			alt_buf_free(&s);
			if (node->type == N_FIELD) {
				tnode *n = new_node(p, N_FIELD, chain->pos);
				n->ident = split_dots(p->a, alt_str_from(str, 1), &n->nident);
				return n;
			}
			return new_variable(p, chain->pos, str);
		}
		case N_BOOL:
		case N_STRING:
		case N_NUMBER:
		case N_NIL:
		case N_DOT: {
			alt_buf s = ALT_BUF_INIT, q = ALT_BUF_INIT;
			tnode_string(&s, node);
			tquote(&q, alt_buf_str(&s));
			char *m = alt_arena_strdup(p->a, alt_buf_cstr(&q));
			alt_buf_free(&s);
			alt_buf_free(&q);
			perrorf(p, "unexpected . after term %s", m);
		}
		}
		return chain;
	}
	return node;
}

static tnode *command(tparser *p) {
	tnode *cmd = new_node(p, N_COMMAND, peek_non_space(p).pos);
	for (;;) {
		peek_non_space(p);
		tnode *op = operand(p);
		if (op)
			tnode_append(p->a, cmd, op);
		titem t = pnext(p);
		if (t.typ == IT_SPACE)
			continue;
		if (t.typ == IT_RIGHT_DELIM || t.typ == IT_RIGHT_PAREN)
			pbackup(p);
		else if (t.typ != IT_PIPE)
			unexpected(p, t, "operand");
		break;
	}
	if (cmd->nn == 0)
		perrorf(p, "empty command");
	return cmd;
}

int tparse(alt_arena *a, alt_str name, alt_str text, alt_str left, alt_str right, const tfuncs *funcs, ttreeset *set, alt_buf *err) {
	jmp_buf jb;
	tlexer lex;
	int depth = 0;
	tparser root;
	memset(&root, 0, sizeof root);
	ttree *t = alt_arena_zalloc(a, sizeof *t);
	t->name = name;
	t->parse_name = name;
	t->text = text;
	t->ldelim = left.n ? left : ALT_STR("{{");
	t->rdelim = right.n ? right : ALT_STR("}}");
	root.a = a;
	root.jb = &jb;
	root.err = err;
	root.funcs = funcs;
	root.set = set;
	root.depth = &depth;
	tparser p;
	start_parse(&p, t, &lex, &root);
	tlex_init(&lex, a, text, t->ldelim, t->rdelim);
	lex.break_ok = !has_function(&p, ALT_STR("break"));
	lex.continue_ok = !has_function(&p, ALT_STR("continue"));
	if (setjmp(jb))
		return -EINVAL;
	parse_top(&p);
	add_tree(&p);
	return 0;
}
