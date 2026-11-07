/* The regexp parser after Go's regexp/syntax parse.go (BSD-3-Clause, see
   LICENSE-Go): the same syntax, limits, character class construction and
   error texts. */
#include "regexp_internal.h"
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_HEIGHT = 1000, MAX_SIZE = (128 << 20) / 40, MAX_RUNES = (128 << 20) / 4, MIN_FOLD = 0x41, MAX_FOLD = 0x1e943 };

typedef struct {
	rx_arena *a;
	uint16_t flags;
	node **stack;
	size_t n, cap;
	int numcap;
	alt_str whole;
	int64_t numrunes;
	/* the error */
	const char *code;
	alt_str expr;
} parser;

static int fail(parser *p, const char *code, alt_str expr) {
	if (!p->code) {
		p->code = code;
		p->expr = expr;
	}
	return -1;
}

static const char E_CLASS[] = "invalid character class", E_RANGE[] = "invalid character class range",
		  E_ESCAPE[] = "invalid escape sequence", E_NAMED[] = "invalid named capture", E_PERL[] = "invalid or unsupported Perl syntax",
		  E_REPEAT_OP[] = "invalid nested repetition operator", E_REPEAT_SIZE[] = "invalid repeat count",
		  E_UTF8[] = "invalid UTF-8", E_BRACKET[] = "missing closing ]", E_PAREN[] = "missing closing )",
		  E_REPEAT_ARG[] = "missing argument to repetition operator", E_BACKSLASH[] = "trailing backslash at end of expression",
		  E_UNEXPECTED[] = "unexpected )", E_DEPTH[] = "expression nests too deeply", E_LARGE[] = "expression too large";

/* ---- nodes ---- */

static node *new_node(parser *p, int op) {
	node *n = rx_alloc(p->a, sizeof *n);
	if (!n)
		return 0;
	memset(n, 0, sizeof *n);
	n->op = op;
	n->height = 1;
	n->size = 1;
	return n;
}

/* Go's calcSize and calcHeight for a node made from its subs */
static void measure(node *re) {
	int h = 1;
	for (size_t i = 0; i < re->nsub; i++)
		if (h < 1 + re->sub[i]->height)
			h = 1 + re->sub[i]->height;
	re->height = h;
	int64_t size = 0, sub = re->nsub ? re->sub[0]->size : 0;
	switch (re->op) {
	case OP_LITERAL: size = (int64_t)re->nrunes; break;
	case OP_CAPTURE: case OP_STAR: size = 2 + sub; break;
	case OP_PLUS: case OP_QUEST: size = 1 + sub; break;
	case OP_CONCAT:
		for (size_t i = 0; i < re->nsub; i++)
			size += re->sub[i]->size;
		break;
	case OP_ALTERNATE:
		for (size_t i = 0; i < re->nsub; i++)
			size += re->sub[i]->size;
		if (re->nsub > 1)
			size += (int64_t)re->nsub - 1;
		break;
	case OP_REPEAT:
		if (re->max == -1)
			size = re->min == 0 ? 2 + sub : 1 + (int64_t)re->min * sub;
		else
			size = (int64_t)re->max * sub + (re->max - re->min);
		break;
	default: break;
	}
	re->size = size < 1 ? 1 : size;
}

static int check_limits(parser *p, node *re) {
	if (p->numrunes > MAX_RUNES || re->size > MAX_SIZE)
		return fail(p, E_LARGE, p->whole);
	if (re->height > MAX_HEIGHT)
		return fail(p, E_DEPTH, p->whole);
	return 0;
}

static int push_stack(parser *p, node *re) {
	if (p->n == p->cap) {
		size_t cap = p->cap ? p->cap * 2 : 32;
		node **ns = rx_realloc(p->a, p->stack, p->cap * sizeof *ns, cap * sizeof *ns);
		if (!ns)
			return fail(p, E_LARGE, p->whole);
		p->stack = ns;
		p->cap = cap;
	}
	p->stack[p->n++] = re;
	return 0;
}

/* runes */
static int add_runes(parser *p, node *re, const int32_t *r, size_t n) {
	if (re->nrunes + n > re->caprunes) {
		size_t cap = re->caprunes ? re->caprunes * 2 : 8;
		while (cap < re->nrunes + n)
			cap *= 2;
		int32_t *nr = rx_realloc(p->a, re->runes, re->caprunes * sizeof *nr, cap * sizeof *nr);
		if (!nr)
			return fail(p, E_LARGE, p->whole);
		re->runes = nr;
		re->caprunes = cap;
	}
	memcpy(re->runes + re->nrunes, r, n * sizeof *r);
	re->nrunes += n;
	return 0;
}

static int32_t simple_fold(int32_t r) { return alt_unicode_simple_fold(r); }

static bool maybe_concat(parser *p, int32_t r, uint16_t flags) {
	if (p->n < 2)
		return false;
	node *re1 = p->stack[p->n - 1], *re2 = p->stack[p->n - 2];
	if (re1->op != OP_LITERAL || re2->op != OP_LITERAL || (re1->flags & F_FOLD) != (re2->flags & F_FOLD))
		return false;
	add_runes(p, re2, re1->runes, re1->nrunes);
	measure(re2);
	if (r >= 0) {
		re1->nrunes = 0;
		add_runes(p, re1, &r, 1);
		re1->flags = flags;
		measure(re1);
		return true;
	}
	p->n--;
	return false;
}

static node *push(parser *p, node *re) {
	p->numrunes += (int64_t)re->nrunes;
	int32_t *r = re->runes;
	if (re->op == OP_CHAR_CLASS && re->nrunes == 2 && r[0] == r[1]) {
		if (maybe_concat(p, r[0], p->flags & ~F_FOLD))
			return 0;
		re->op = OP_LITERAL;
		re->nrunes = 1;
		re->flags = p->flags & ~F_FOLD;
	} else if ((re->op == OP_CHAR_CLASS && re->nrunes == 4 && r[0] == r[1] && r[2] == r[3] && simple_fold(r[0]) == r[2] &&
		       simple_fold(r[2]) == r[0]) ||
		   (re->op == OP_CHAR_CLASS && re->nrunes == 2 && r[0] + 1 == r[1] && simple_fold(r[0]) == r[1] && simple_fold(r[1]) == r[0])) {
		if (maybe_concat(p, r[0], p->flags | F_FOLD))
			return 0;
		re->op = OP_LITERAL;
		re->nrunes = 1;
		re->flags = p->flags | F_FOLD;
	} else
		maybe_concat(p, -1, 0);
	measure(re);
	if (push_stack(p, re) < 0 || check_limits(p, re) < 0)
		return 0;
	return re;
}

static int32_t min_fold_rune(int32_t r) {
	if (r < MIN_FOLD || r > MAX_FOLD)
		return r;
	int32_t m = r, r0 = r;
	for (r = simple_fold(r); r != r0; r = simple_fold(r))
		if (r < m)
			m = r;
	return m;
}

static int literal(parser *p, int32_t r) {
	node *re = new_node(p, OP_LITERAL);
	if (!re)
		return fail(p, E_LARGE, p->whole);
	re->flags = p->flags;
	if (p->flags & F_FOLD)
		r = min_fold_rune(r);
	add_runes(p, re, &r, 1);
	push(p, re);
	return p->code ? -1 : 0;
}

static node *op(parser *p, int o) {
	node *re = new_node(p, o);
	if (!re) {
		fail(p, E_LARGE, p->whole);
		return 0;
	}
	re->flags = p->flags;
	return push(p, re);
}

static bool repeat_valid(const node *re, int n) {
	if (re->op == OP_REPEAT) {
		int m = re->max;
		if (m == 0)
			return true;
		if (m < 0)
			m = re->min;
		if (m > n)
			return false;
		if (m > 0)
			n /= m;
	}
	for (size_t i = 0; i < re->nsub; i++)
		if (!repeat_valid(re->sub[i], n))
			return false;
	return true;
}

static alt_str span(alt_str before, alt_str after) { return alt_str_make(before.p, before.n - after.n); }

static int repeat(parser *p, int o, int min, int max, alt_str before, alt_str *after, alt_str last_repeat) {
	uint16_t flags = p->flags;
	if (p->flags & F_PERLX) {
		if (after->n && after->p[0] == '?') {
			*after = alt_str_slice(*after, 1, after->n);
			flags ^= F_NONGREEDY;
		}
		if (last_repeat.p)
			return fail(p, E_REPEAT_OP, span(last_repeat, *after));
	}
	if (p->n == 0 || p->stack[p->n - 1]->op >= OP_PSEUDO)
		return fail(p, E_REPEAT_ARG, span(before, *after));
	node *sub = p->stack[p->n - 1];
	node *re = new_node(p, o);
	node **subs = rx_alloc(p->a, sizeof *subs);
	if (!re || !subs)
		return fail(p, E_LARGE, p->whole);
	re->min = min;
	re->max = max;
	re->flags = flags;
	subs[0] = sub;
	re->sub = subs;
	re->nsub = 1;
	measure(re);
	p->stack[p->n - 1] = re;
	if (check_limits(p, re) < 0)
		return -1;
	if (o == OP_REPEAT && (min >= 2 || max >= 2) && !repeat_valid(re, 1000))
		return fail(p, E_REPEAT_SIZE, span(before, *after));
	return 0;
}

/* ---- classes ---- */

static int append_range(parser *p, node *re, int32_t lo, int32_t hi) {
	int32_t *r = re->runes;
	size_t n = re->nrunes;
	for (size_t i = 2; i <= 4; i += 2)
		if (n >= i) {
			int32_t rlo = r[n - i], rhi = r[n - i + 1];
			if (lo <= rhi + 1 && rlo <= hi + 1) {
				if (lo < rlo)
					r[n - i] = lo;
				if (hi > rhi)
					r[n - i + 1] = hi;
				return 0;
			}
		}
	int32_t x[2] = {lo, hi};
	return add_runes(p, re, x, 2);
}

static int append_folded_range(parser *p, node *re, int32_t lo, int32_t hi) {
	if ((lo <= MIN_FOLD && hi >= MAX_FOLD) || hi < MIN_FOLD || lo > MAX_FOLD)
		return append_range(p, re, lo, hi);
	if (lo < MIN_FOLD) {
		append_range(p, re, lo, MIN_FOLD - 1);
		lo = MIN_FOLD;
	}
	if (hi > MAX_FOLD) {
		append_range(p, re, MAX_FOLD + 1, hi);
		hi = MAX_FOLD;
	}
	for (int32_t c = lo; c <= hi; c++) {
		append_range(p, re, c, c);
		for (int32_t f = simple_fold(c); f != c; f = simple_fold(f))
			append_range(p, re, f, f);
	}
	return p->code ? -1 : 0;
}

static void append_class(parser *p, node *re, const int32_t *x, size_t n) {
	for (size_t i = 0; i < n; i += 2)
		append_range(p, re, x[i], x[i + 1]);
}

static void append_negated_class(parser *p, node *re, const int32_t *x, size_t n) {
	int32_t next = 0;
	for (size_t i = 0; i < n; i += 2) {
		if (next <= x[i] - 1)
			append_range(p, re, next, x[i] - 1);
		next = x[i + 1] + 1;
	}
	if (next <= ALT_UNICODE_MAX_RUNE)
		append_range(p, re, next, ALT_UNICODE_MAX_RUNE);
}

static int cmp_range(const void *a, const void *b) {
	const int32_t *x = a, *y = b;
	if (x[0] != y[0])
		return x[0] < y[0] ? -1 : 1;
	return x[1] > y[1] ? -1 : x[1] < y[1] ? 1 : 0;
}

void rx_clean_class(node *re) {
	if (re->nrunes < 2)
		return;
	qsort(re->runes, re->nrunes / 2, 2 * sizeof(int32_t), cmp_range);
	int32_t *r = re->runes;
	size_t w = 2;
	for (size_t i = 2; i < re->nrunes; i += 2) {
		int32_t lo = r[i], hi = r[i + 1];
		if (lo <= r[w - 1] + 1) {
			if (hi > r[w - 1])
				r[w - 1] = hi;
			continue;
		}
		r[w] = lo, r[w + 1] = hi;
		w += 2;
	}
	re->nrunes = w;
}

static void negate_class(parser *p, node *re) {
	int32_t next = 0, *r = re->runes;
	size_t w = 0, n = re->nrunes;
	for (size_t i = 0; i < n; i += 2) {
		int32_t lo = r[i], hi = r[i + 1];
		if (next <= lo - 1) {
			r[w] = next, r[w + 1] = lo - 1;
			w += 2;
		}
		next = hi + 1;
	}
	re->nrunes = w;
	if (next <= ALT_UNICODE_MAX_RUNE) {
		int32_t x[2] = {next, ALT_UNICODE_MAX_RUNE};
		add_runes(p, re, x, 2);
	}
}

/* a temporary class node */
static node *tmp_class(parser *p) { return new_node(p, OP_CHAR_CLASS); }

typedef struct {
	int sign;
	const int32_t *r;
	size_t n;
} group;

static const int32_t g_digit[] = {'0', '9'}, g_space[] = {'\t', '\n', '\f', '\r', ' ', ' '},
		     g_word[] = {'0', '9', 'A', 'Z', '_', '_', 'a', 'z'};
static const int32_t g_alnum[] = {'0', '9', 'A', 'Z', 'a', 'z'}, g_alpha[] = {'A', 'Z', 'a', 'z'}, g_ascii[] = {0, 0x7f},
		     g_blank[] = {'\t', '\t', ' ', ' '}, g_cntrl[] = {0, 0x1f, 0x7f, 0x7f}, g_graph[] = {'!', '~'},
		     g_lower[] = {'a', 'z'}, g_print[] = {' ', '~'}, g_punct[] = {'!', '/', ':', '@', '[', '`', '{', '~'},
		     g_pspace[] = {'\t', '\r', ' ', ' '}, g_upper[] = {'A', 'Z'}, g_xdigit[] = {'0', '9', 'A', 'F', 'a', 'f'};

static bool perl_group(alt_str s, group *g) {
	if (s.n < 2 || s.p[0] != '\\')
		return false;
	switch (s.p[1]) {
	case 'd': *g = (group){1, g_digit, 2}; return true;
	case 'D': *g = (group){-1, g_digit, 2}; return true;
	case 's': *g = (group){1, g_space, 6}; return true;
	case 'S': *g = (group){-1, g_space, 6}; return true;
	case 'w': *g = (group){1, g_word, 8}; return true;
	case 'W': *g = (group){-1, g_word, 8}; return true;
	}
	return false;
}

static bool posix_group(alt_str name, group *g) {
	static const struct {
		const char *n;
		const int32_t *r;
		size_t len;
	} t[] = {{"alnum", g_alnum, 6}, {"alpha", g_alpha, 4}, {"ascii", g_ascii, 2}, {"blank", g_blank, 4}, {"cntrl", g_cntrl, 4},
		{"digit", g_digit, 2}, {"graph", g_graph, 2}, {"lower", g_lower, 2}, {"print", g_print, 2}, {"punct", g_punct, 8},
		{"space", g_pspace, 4}, {"upper", g_upper, 2}, {"word", g_word, 8}, {"xdigit", g_xdigit, 6}};
	/* name is "[:alpha:]" or "[:^alpha:]" */
	alt_str inner = alt_str_slice(name, 2, name.n - 2);
	int sign = 1;
	if (inner.n && inner.p[0] == '^')
		sign = -1, inner = alt_str_slice(inner, 1, inner.n);
	for (size_t i = 0; i < sizeof t / sizeof *t; i++)
		if (alt_str_eq_cstr(inner, t[i].n)) {
			*g = (group){sign, t[i].r, t[i].len};
			return true;
		}
	return false;
}

static void append_group(parser *p, node *re, const group *g) {
	if (!(p->flags & F_FOLD)) {
		if (g->sign < 0)
			append_negated_class(p, re, g->r, g->n);
		else
			append_class(p, re, g->r, g->n);
		return;
	}
	node *tmp = tmp_class(p);
	for (size_t i = 0; i < g->n; i += 2)
		append_folded_range(p, tmp, g->r[i], g->r[i + 1]);
	rx_clean_class(tmp);
	if (g->sign < 0)
		append_negated_class(p, re, tmp->runes, tmp->nrunes);
	else
		append_class(p, re, tmp->runes, tmp->nrunes);
}

/* utf8.DecodeRuneInString with Go's error for invalid UTF-8 */
static int next_rune(parser *p, alt_str s, int32_t *c, alt_str *rest) {
	size_t k = alt_utf8_decode(s.p, s.n, c);
	if (*c == 0xfffd && k == 1 && !(s.n >= 3 && (unsigned char)s.p[0] == 0xef && (unsigned char)s.p[1] == 0xbf &&
						 (unsigned char)s.p[2] == 0xbd))
		return fail(p, E_UTF8, s);
	*rest = alt_str_slice(s, k, s.n);
	return 0;
}

static int check_utf8(parser *p, alt_str s) {
	while (s.n) {
		int32_t c;
		if (next_rune(p, s, &c, &s) < 0)
			return -1;
	}
	return 0;
}

static void append_table(parser *p, node *re, const alt_unicode_table *t) {
	for (int i = 0; i < t->n; i++) {
		int32_t lo = (int32_t)t->r[i][0], hi = (int32_t)t->r[i][1], stride = (int32_t)t->r[i][2];
		if (stride == 1) {
			append_range(p, re, lo, hi);
			continue;
		}
		for (int32_t c = lo; c <= hi; c += stride)
			append_range(p, re, c, c);
	}
}

static void append_negated_table(parser *p, node *re, const alt_unicode_table *t) {
	int32_t next = 0;
	for (int i = 0; i < t->n; i++) {
		int32_t lo = (int32_t)t->r[i][0], hi = (int32_t)t->r[i][1], stride = (int32_t)t->r[i][2];
		if (stride == 1) {
			if (next <= lo - 1)
				append_range(p, re, next, lo - 1);
			next = hi + 1;
			continue;
		}
		for (int32_t c = lo; c <= hi; c += stride) {
			if (next <= c - 1)
				append_range(p, re, next, c - 1);
			next = c + 1;
		}
	}
	if (next <= ALT_UNICODE_MAX_RUNE)
		append_range(p, re, next, ALT_UNICODE_MAX_RUNE);
}

static const uint32_t any_r[][3] = {{0, 0x10ffff, 1}}, ascii_r[][3] = {{0, 0x7f, 1}},
		      ascii_fold_r[][3] = {{0, 0x7f, 1}, {0x17f, 0x17f, 1}, {0x212a, 0x212a, 1}};
static const alt_unicode_table any_t = {any_r, 1, 1}, ascii_t = {ascii_r, 1, 1}, ascii_fold_t = {ascii_fold_r, 3, 1};

/* Go's canonicalName: _, - and spaces dropped, the first letter upper, the rest lower */
static void canonical_name(alt_str name, char *out, size_t cap) {
	size_t k = 0;
	bool first = true;
	for (size_t i = 0; i < name.n && k + 1 < cap; i++) {
		char c = name.p[i];
		if (c == '_' || c == '-' || c == ' ')
			continue;
		if (first) {
			if (c >= 'a' && c <= 'z')
				c -= 'a' - 'A';
			first = false;
		} else if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		out[k++] = c;
	}
	out[k] = 0;
}

static bool unicode_table(alt_str raw, const alt_unicode_table **tab, const alt_unicode_table **fold, int *sign) {
	char name[128];
	canonical_name(raw, name, sizeof name);
	*sign = 1;
	if (!strcmp(name, "Any")) {
		*tab = *fold = &any_t;
		return true;
	}
	if (!strcmp(name, "Assigned")) {
		*tab = *fold = alt_unicode_group_find(alt__unicode_categories, alt__unicode_ncategories, "Cn");
		*sign = -1;
		return true;
	}
	if (!strcmp(name, "Ascii")) {
		*tab = &ascii_t, *fold = &ascii_fold_t;
		return true;
	}
	const char *cat = !strcmp(name, "Lc") ? "LC" : name;
	if ((*tab = alt_unicode_group_find(alt__unicode_categories, alt__unicode_ncategories, cat))) {
		*fold = alt_unicode_group_find(alt__unicode_fold_categories, alt__unicode_nfold_categories, cat);
		return true;
	}
	if ((*tab = alt_unicode_group_find(alt__unicode_scripts, alt__unicode_nscripts, name))) {
		*fold = alt_unicode_group_find(alt__unicode_fold_scripts, alt__unicode_nfold_scripts, name);
		return true;
	}
	/* aliases by canonical name */
	char c2[128];
	for (size_t i = 0; i < alt__unicode_ncategory_aliases; i++) {
		canonical_name(alt_str_cstr(alt__unicode_category_aliases[i][0]), c2, sizeof c2);
		if (!strcmp(c2, name)) {
			const char *actual = alt__unicode_category_aliases[i][1];
			*tab = alt_unicode_group_find(alt__unicode_categories, alt__unicode_ncategories, actual);
			*fold = alt_unicode_group_find(alt__unicode_fold_categories, alt__unicode_nfold_categories, actual);
			return *tab != 0;
		}
	}
	for (size_t i = 0; i < alt__unicode_nscripts; i++) {
		canonical_name(alt_str_cstr(alt__unicode_scripts[i].name), c2, sizeof c2);
		if (!strcmp(c2, name)) {
			*tab = alt__unicode_scripts[i].table;
			*fold = alt_unicode_group_find(alt__unicode_fold_scripts, alt__unicode_nfold_scripts, alt__unicode_scripts[i].name);
			return true;
		}
	}
	return false;
}

/* \p{...}: 1 when parsed (rest set), 0 when not a class, -1 on error */
static int parse_unicode_class(parser *p, alt_str s, node *re, alt_str *rest) {
	if (!(p->flags & F_UNICODE) || s.n < 2 || s.p[0] != '\\' || (s.p[1] != 'p' && s.p[1] != 'P'))
		return 0;
	int sign = s.p[1] == 'P' ? -1 : 1;
	alt_str t = alt_str_slice(s, 2, s.n), seq, name;
	int32_t c;
	if (next_rune(p, t, &c, &t) < 0) {
		/* Go returns (nil, "", err) with err set: but nextRune's error only for invalid UTF-8 */
		return -1;
	}
	if (c != '{') {
		seq = alt_str_make(s.p, s.n - t.n);
		name = alt_str_slice(seq, 2, seq.n);
	} else {
		ssize_t end = alt_str_index_byte(s, '}');
		if (end < 0) {
			if (check_utf8(p, s) < 0)
				return -1;
			return fail(p, E_RANGE, s);
		}
		seq = alt_str_slice(s, 0, (size_t)end + 1);
		t = alt_str_slice(s, (size_t)end + 1, s.n);
		name = alt_str_slice(s, 3, (size_t)end);
		if (check_utf8(p, name) < 0)
			return -1;
	}
	if (name.n && name.p[0] == '^') {
		sign = -sign;
		name = alt_str_slice(name, 1, name.n);
	}
	const alt_unicode_table *tab, *fold;
	int tsign;
	if (!unicode_table(name, &tab, &fold, &tsign) || !tab)
		return fail(p, E_RANGE, seq);
	if (tsign < 0)
		sign = -sign;
	if (!(p->flags & F_FOLD) || !fold) {
		if (sign > 0)
			append_table(p, re, tab);
		else
			append_negated_table(p, re, tab);
	} else {
		node *tmp = tmp_class(p);
		append_table(p, tmp, tab);
		append_table(p, tmp, fold);
		rx_clean_class(tmp);
		if (sign > 0)
			append_class(p, re, tmp->runes, tmp->nrunes);
		else
			append_negated_class(p, re, tmp->runes, tmp->nrunes);
	}
	*rest = t;
	return 1;
}

static int unhex(int32_t c) {
	return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

static bool isalnum_(int32_t c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

static int parse_escape(parser *p, alt_str s, int32_t *r, alt_str *rest) {
	alt_str t = alt_str_slice(s, 1, s.n);
	if (!t.n)
		return fail(p, E_BACKSLASH, ALT_STR(""));
	int32_t c;
	if (next_rune(p, t, &c, &t) < 0)
		return -1;
	switch (c) {
	default:
		if (c < 0x80 && !isalnum_(c)) {
			*r = c, *rest = t;
			return 0;
		}
		break;
	case '1': case '2': case '3': case '4': case '5': case '6': case '7':
		if (!t.n || t.p[0] < '0' || t.p[0] > '7')
			break;
		/* fallthrough */
	case '0': {
		int32_t v = c - '0';
		for (int i = 1; i < 3; i++) {
			if (!t.n || t.p[0] < '0' || t.p[0] > '7')
				break;
			v = v * 8 + t.p[0] - '0';
			t = alt_str_slice(t, 1, t.n);
		}
		*r = v, *rest = t;
		return 0;
	}
	case 'x': {
		if (!t.n)
			break;
		if (next_rune(p, t, &c, &t) < 0)
			return -1;
		if (c == '{') {
			int nhex = 0;
			int32_t v = 0;
			for (;;) {
				if (!t.n)
					goto bad;
				if (next_rune(p, t, &c, &t) < 0)
					return -1;
				if (c == '}')
					break;
				int d = unhex(c);
				if (d < 0)
					goto bad;
				v = v * 16 + d;
				if (v > ALT_UNICODE_MAX_RUNE)
					goto bad;
				nhex++;
			}
			if (nhex == 0)
				goto bad;
			*r = v, *rest = t;
			return 0;
		}
		int x = unhex(c);
		if (next_rune(p, t, &c, &t) < 0)
			return -1;
		int y = unhex(c);
		if (x < 0 || y < 0)
			break;
		*r = x * 16 + y, *rest = t;
		return 0;
	}
	case 'a': *r = '\a', *rest = t; return 0;
	case 'f': *r = '\f', *rest = t; return 0;
	case 'n': *r = '\n', *rest = t; return 0;
	case 'r': *r = '\r', *rest = t; return 0;
	case 't': *r = '\t', *rest = t; return 0;
	case 'v': *r = '\v', *rest = t; return 0;
	}
bad:
	return fail(p, E_ESCAPE, span(s, t));
}

static int parse_class_char(parser *p, alt_str s, alt_str whole, int32_t *r, alt_str *rest) {
	if (!s.n)
		return fail(p, E_BRACKET, whole);
	if (s.p[0] == '\\')
		return parse_escape(p, s, r, rest);
	return next_rune(p, s, r, rest);
}

static int parse_class(parser *p, alt_str s, alt_str *rest) {
	alt_str t = alt_str_slice(s, 1, s.n);
	node *re = new_node(p, OP_CHAR_CLASS);
	if (!re)
		return fail(p, E_LARGE, p->whole);
	re->flags = p->flags;
	int sign = 1;
	if (t.n && t.p[0] == '^') {
		sign = -1;
		t = alt_str_slice(t, 1, t.n);
		if (!(p->flags & F_CLASSNL)) {
			int32_t nl[2] = {'\n', '\n'};
			add_runes(p, re, nl, 2);
		}
	}
	bool first = true;
	while (!t.n || t.p[0] != ']' || first) {
		if (t.n && t.p[0] == '-' && !(p->flags & F_PERLX) && !first && (t.n == 1 || t.p[1] != ']')) {
			int32_t c;
			size_t sz = alt_utf8_decode(t.p + 1, t.n - 1, &c);
			return fail(p, E_RANGE, alt_str_slice(t, 0, 1 + sz));
		}
		first = false;
		if (t.n > 2 && t.p[0] == '[' && t.p[1] == ':') {
			ssize_t i = alt_str_index(alt_str_slice(t, 2, t.n), ALT_STR(":]"));
			if (i >= 0) {
				alt_str name = alt_str_slice(t, 0, (size_t)i + 4);
				group g;
				if (!posix_group(name, &g))
					return fail(p, E_RANGE, name);
				append_group(p, re, &g);
				t = alt_str_slice(t, (size_t)i + 4, t.n);
				continue;
			}
		}
		int u = parse_unicode_class(p, t, re, &t);
		if (u < 0)
			return -1;
		if (u > 0)
			continue;
		group g;
		if ((p->flags & F_PERLX) && perl_group(t, &g)) {
			append_group(p, re, &g);
			t = alt_str_slice(t, 2, t.n);
			continue;
		}
		alt_str rng = t;
		int32_t lo, hi;
		if (parse_class_char(p, t, s, &lo, &t) < 0)
			return -1;
		hi = lo;
		if (t.n >= 2 && t.p[0] == '-' && t.p[1] != ']') {
			t = alt_str_slice(t, 1, t.n);
			if (parse_class_char(p, t, s, &hi, &t) < 0)
				return -1;
			if (hi < lo)
				return fail(p, E_RANGE, span(rng, t));
		}
		if (p->flags & F_FOLD)
			append_folded_range(p, re, lo, hi);
		else
			append_range(p, re, lo, hi);
		if (p->code)
			return -1;
	}
	t = alt_str_slice(t, 1, t.n);
	rx_clean_class(re);
	if (sign < 0)
		negate_class(p, re);
	push(p, re);
	*rest = t;
	return p->code ? -1 : 0;
}

/* ---- concatenation and alternation ---- */

static node *collapse(parser *p, node **subs, size_t n, int o) {
	if (n == 1)
		return subs[0];
	node *re = new_node(p, o);
	if (!re)
		return 0;
	size_t total = 0;
	for (size_t i = 0; i < n; i++)
		total += subs[i]->op == o ? subs[i]->nsub : 1;
	re->sub = rx_alloc(p->a, total * sizeof *re->sub);
	if (!re->sub)
		return 0;
	for (size_t i = 0; i < n; i++)
		if (subs[i]->op == o)
			for (size_t k = 0; k < subs[i]->nsub; k++)
				re->sub[re->nsub++] = subs[i]->sub[k];
		else
			re->sub[re->nsub++] = subs[i];
	measure(re);
	return re;
}

static node *concat(parser *p) {
	maybe_concat(p, -1, 0);
	size_t i = p->n;
	while (i > 0 && p->stack[i - 1]->op < OP_PSEUDO)
		i--;
	size_t n = p->n - i;
	p->n = i;
	if (n == 0)
		return push(p, new_node(p, OP_EMPTY_MATCH));
	node *c = collapse(p, p->stack + i, n, OP_CONCAT);
	return c ? push(p, c) : 0;
}

static bool any_class(const node *re, bool not_nl) {
	if (not_nl)
		return re->nrunes == 4 && re->runes[0] == 0 && re->runes[1] == '\n' - 1 && re->runes[2] == '\n' + 1 &&
		       re->runes[3] == ALT_UNICODE_MAX_RUNE;
	return re->nrunes == 2 && re->runes[0] == 0 && re->runes[1] == ALT_UNICODE_MAX_RUNE;
}

static void clean_alt(node *re) {
	if (re->op == OP_CHAR_CLASS) {
		rx_clean_class(re);
		if (any_class(re, false)) {
			re->nrunes = 0;
			re->op = OP_ANY_CHAR;
		} else if (any_class(re, true)) {
			re->nrunes = 0;
			re->op = OP_ANY_CHAR_NOT_NL;
		}
	}
}

static node *alternate(parser *p) {
	size_t i = p->n;
	while (i > 0 && p->stack[i - 1]->op < OP_PSEUDO)
		i--;
	size_t n = p->n - i;
	p->n = i;
	if (n > 0)
		clean_alt(p->stack[i + n - 1]);
	if (n == 0)
		return push(p, new_node(p, OP_NO_MATCH));
	node *c = collapse(p, p->stack + i, n, OP_ALTERNATE);
	return c ? push(p, c) : 0;
}

static bool is_char_class(const node *re) {
	return (re->op == OP_LITERAL && re->nrunes == 1) || re->op == OP_CHAR_CLASS || re->op == OP_ANY_CHAR_NOT_NL ||
	       re->op == OP_ANY_CHAR;
}

static bool match_rune(const node *re, int32_t r) {
	switch (re->op) {
	case OP_LITERAL: return re->nrunes == 1 && re->runes[0] == r;
	case OP_CHAR_CLASS:
		for (size_t i = 0; i < re->nrunes; i += 2)
			if (re->runes[i] <= r && r <= re->runes[i + 1])
				return true;
		return false;
	case OP_ANY_CHAR_NOT_NL: return r != '\n';
	case OP_ANY_CHAR: return true;
	}
	return false;
}

static void append_literal(parser *p, node *re, int32_t x, uint16_t flags) {
	if (flags & F_FOLD)
		append_folded_range(p, re, x, x);
	else
		append_range(p, re, x, x);
}

static void merge_char_class(parser *p, node *dst, node *src) {
	switch (dst->op) {
	case OP_ANY_CHAR: break;
	case OP_ANY_CHAR_NOT_NL:
		if (match_rune(src, '\n'))
			dst->op = OP_ANY_CHAR;
		break;
	case OP_CHAR_CLASS:
		if (src->op == OP_LITERAL)
			append_literal(p, dst, src->runes[0], src->flags);
		else
			append_class(p, dst, src->runes, src->nrunes);
		break;
	case OP_LITERAL: {
		if (src->runes[0] == dst->runes[0] && src->flags == dst->flags)
			break;
		int32_t r0 = dst->runes[0];
		uint16_t f0 = dst->flags;
		dst->op = OP_CHAR_CLASS;
		dst->nrunes = 0;
		append_literal(p, dst, r0, f0);
		append_literal(p, dst, src->runes[0], src->flags);
		break;
	}
	}
}

static bool swap_vertical_bar(parser *p) {
	size_t n = p->n;
	if (n >= 3 && p->stack[n - 2]->op == OP_VERTICAL_BAR && is_char_class(p->stack[n - 1]) && is_char_class(p->stack[n - 3])) {
		node *re1 = p->stack[n - 1], *re3 = p->stack[n - 3];
		if (re1->op > re3->op) {
			node *t = re1;
			re1 = re3, re3 = t;
			p->stack[n - 3] = re3;
		}
		merge_char_class(p, re3, re1);
		p->n--;
		return true;
	}
	if (n >= 2) {
		node *re1 = p->stack[n - 1], *re2 = p->stack[n - 2];
		if (re2->op == OP_VERTICAL_BAR) {
			if (n >= 3)
				clean_alt(p->stack[n - 3]);
			p->stack[n - 2] = re1;
			p->stack[n - 1] = re2;
			return true;
		}
	}
	return false;
}

static void parse_vertical_bar(parser *p) {
	concat(p);
	if (!swap_vertical_bar(p))
		op(p, OP_VERTICAL_BAR);
}

static int parse_right_paren(parser *p) {
	concat(p);
	if (swap_vertical_bar(p))
		p->n--;
	alternate(p);
	if (p->code)
		return -1;
	if (p->n < 2)
		return fail(p, E_UNEXPECTED, p->whole);
	node *re1 = p->stack[p->n - 1], *re2 = p->stack[p->n - 2];
	p->n -= 2;
	if (re2->op != OP_LEFT_PAREN)
		return fail(p, E_UNEXPECTED, p->whole);
	p->flags = re2->flags;
	if (re2->cap == 0)
		push(p, re1);
	else {
		re2->op = OP_CAPTURE;
		re2->sub = rx_alloc(p->a, sizeof *re2->sub);
		if (!re2->sub)
			return fail(p, E_LARGE, p->whole);
		re2->sub[0] = re1;
		re2->nsub = 1;
		push(p, re2);
	}
	return p->code ? -1 : 0;
}

static bool valid_capture_name(parser *p, alt_str name) {
	if (!name.n)
		return false;
	while (name.n) {
		int32_t c;
		alt_str rest;
		if (next_rune(p, name, &c, &rest) < 0)
			return false;
		if (c != '_' && !isalnum_(c))
			return false;
		name = rest;
	}
	return true;
}

static int parse_perl_flags(parser *p, alt_str s, alt_str *rest) {
	alt_str t = s;
	bool starts_p = t.n > 4 && t.p[2] == 'P' && t.p[3] == '<', starts_name = t.n > 3 && t.p[2] == '<';
	if (starts_p || starts_name) {
		size_t start = starts_name ? 3 : 4;
		ssize_t end = alt_str_index_byte(t, '>');
		if (end < 0) {
			if (check_utf8(p, t) < 0)
				return -1;
			return fail(p, E_NAMED, s);
		}
		alt_str capture = alt_str_slice(t, 0, (size_t)end + 1), name = alt_str_slice(t, start, (size_t)end);
		if (check_utf8(p, name) < 0)
			return -1;
		if (!valid_capture_name(p, name))
			return fail(p, E_NAMED, capture);
		p->numcap++;
		node *re = op(p, OP_LEFT_PAREN);
		if (!re)
			return -1;
		re->cap = p->numcap;
		re->name = name;
		*rest = alt_str_slice(t, (size_t)end + 1, t.n);
		return 0;
	}
	t = alt_str_slice(t, 2, t.n);
	uint16_t flags = p->flags;
	int sign = 1;
	bool saw = false;
	while (t.n) {
		int32_t c;
		if (next_rune(p, t, &c, &t) < 0)
			return -1;
		switch (c) {
		default: goto out;
		case 'i': flags |= F_FOLD, saw = true; break;
		case 'm': flags &= (uint16_t)~F_ONELINE, saw = true; break;
		case 's': flags |= F_DOTNL, saw = true; break;
		case 'U': flags |= F_NONGREEDY, saw = true; break;
		case '-':
			if (sign < 0)
				goto out;
			sign = -1;
			flags = (uint16_t)~flags;
			saw = false;
			break;
		case ':': case ')':
			if (sign < 0) {
				if (!saw)
					goto out;
				flags = (uint16_t)~flags;
			}
			if (c == ':' && !op(p, OP_LEFT_PAREN))
				return -1;
			p->flags = flags;
			*rest = t;
			return 0;
		}
	}
out:
	return fail(p, E_PERL, span(s, t));
}

static bool parse_int(alt_str s, int *n, alt_str *rest) {
	if (!s.n || s.p[0] < '0' || s.p[0] > '9')
		return false;
	if (s.n >= 2 && s.p[0] == '0' && s.p[1] >= '0' && s.p[1] <= '9')
		return false;
	size_t i = 0;
	while (i < s.n && s.p[i] >= '0' && s.p[i] <= '9')
		i++;
	*n = 0;
	for (size_t k = 0; k < i; k++) {
		if (*n >= 100000000) {
			*n = -1;
			break;
		}
		*n = *n * 10 + s.p[k] - '0';
	}
	*rest = alt_str_slice(s, i, s.n);
	return true;
}

static bool parse_repeat(alt_str s, int *min, int *max, alt_str *rest) {
	if (!s.n || s.p[0] != '{')
		return false;
	s = alt_str_slice(s, 1, s.n);
	if (!parse_int(s, min, &s) || !s.n)
		return false;
	if (s.p[0] != ',')
		*max = *min;
	else {
		s = alt_str_slice(s, 1, s.n);
		if (!s.n)
			return false;
		if (s.p[0] == '}')
			*max = -1;
		else if (!parse_int(s, max, &s))
			return false;
		else if (*max < 0)
			*min = -1;
	}
	if (!s.n || s.p[0] != '}')
		return false;
	*rest = alt_str_slice(s, 1, s.n);
	return true;
}

node *rx_parse(rx_arena *a, alt_str s, uint16_t flags, int *ncap, const char **code, alt_str *expr) {
	parser p = {.a = a, .flags = flags, .whole = s};
	alt_str t = s, last_repeat = {0, 0};
	int32_t c;
	if (flags & F_LITERAL) {
		if (check_utf8(&p, s) < 0)
			goto err;
		node *re = new_node(&p, OP_LITERAL);
		re->flags = flags;
		while (t.n) {
			next_rune(&p, t, &c, &t);
			add_runes(&p, re, &c, 1);
		}
		measure(re);
		*ncap = 0;
		return re;
	}
	while (t.n) {
		alt_str repeat_at = {0, 0};
		switch (t.p[0]) {
		default:
			if (next_rune(&p, t, &c, &t) < 0 || literal(&p, c) < 0)
				goto err;
			break;
		case '(':
			if ((p.flags & F_PERLX) && t.n >= 2 && t.p[1] == '?') {
				if (parse_perl_flags(&p, t, &t) < 0)
					goto err;
				break;
			}
			p.numcap++;
			{
				node *re = op(&p, OP_LEFT_PAREN);
				if (!re)
					goto err;
				re->cap = p.numcap;
			}
			t = alt_str_slice(t, 1, t.n);
			break;
		case '|':
			parse_vertical_bar(&p);
			t = alt_str_slice(t, 1, t.n);
			break;
		case ')':
			if (parse_right_paren(&p) < 0)
				goto err;
			t = alt_str_slice(t, 1, t.n);
			break;
		case '^':
			op(&p, (p.flags & F_ONELINE) ? OP_BEGIN_TEXT : OP_BEGIN_LINE);
			t = alt_str_slice(t, 1, t.n);
			break;
		case '$':
			if (p.flags & F_ONELINE) {
				node *re = op(&p, OP_END_TEXT);
				if (re)
					re->flags |= F_WASDOLLAR;
			} else
				op(&p, OP_END_LINE);
			t = alt_str_slice(t, 1, t.n);
			break;
		case '.':
			op(&p, (p.flags & F_DOTNL) ? OP_ANY_CHAR : OP_ANY_CHAR_NOT_NL);
			t = alt_str_slice(t, 1, t.n);
			break;
		case '[':
			if (parse_class(&p, t, &t) < 0)
				goto err;
			break;
		case '*': case '+': case '?': {
			alt_str before = t, after = alt_str_slice(t, 1, t.n);
			int o = t.p[0] == '*' ? OP_STAR : t.p[0] == '+' ? OP_PLUS : OP_QUEST;
			if (repeat(&p, o, 0, 0, before, &after, last_repeat) < 0)
				goto err;
			repeat_at = before;
			t = after;
			break;
		}
		case '{': {
			alt_str before = t, after;
			int min, max;
			if (!parse_repeat(t, &min, &max, &after)) {
				if (literal(&p, '{') < 0)
					goto err;
				t = alt_str_slice(t, 1, t.n);
				break;
			}
			if (min < 0 || min > 1000 || max > 1000 || (max >= 0 && min > max)) {
				fail(&p, E_REPEAT_SIZE, span(before, after));
				goto err;
			}
			if (repeat(&p, OP_REPEAT, min, max, before, &after, last_repeat) < 0)
				goto err;
			repeat_at = before;
			t = after;
			break;
		}
		case '\\': {
			if ((p.flags & F_PERLX) && t.n >= 2) {
				int simple = 0;
				switch (t.p[1]) {
				case 'A': simple = OP_BEGIN_TEXT; break;
				case 'b': simple = OP_WORD_BOUNDARY; break;
				case 'B': simple = OP_NO_WORD_BOUNDARY; break;
				case 'z': simple = OP_END_TEXT; break;
				case 'C':
					fail(&p, E_ESCAPE, alt_str_slice(t, 0, 2));
					goto err;
				case 'Q': {
					alt_str lit = alt_str_slice(t, 2, t.n), after;
					ssize_t e = alt_str_index(lit, ALT_STR("\\E"));
					if (e >= 0) {
						after = alt_str_slice(lit, (size_t)e + 2, lit.n);
						lit = alt_str_slice(lit, 0, (size_t)e);
					} else
						after = alt_str_slice(lit, lit.n, lit.n);
					while (lit.n) {
						if (next_rune(&p, lit, &c, &lit) < 0 || literal(&p, c) < 0)
							goto err;
					}
					t = after;
					last_repeat = (alt_str){0, 0};
					goto next;
				}
				}
				if (simple) {
					op(&p, simple);
					t = alt_str_slice(t, 2, t.n);
					break;
				}
			}
			node *re = new_node(&p, OP_CHAR_CLASS);
			if (!re)
				goto err;
			re->flags = p.flags;
			if (t.n >= 2 && (t.p[1] == 'p' || t.p[1] == 'P')) {
				int u = parse_unicode_class(&p, t, re, &t);
				if (u < 0)
					goto err;
				if (u > 0) {
					push(&p, re);
					break;
				}
			}
			group g;
			if ((p.flags & F_PERLX) && perl_group(t, &g)) {
				append_group(&p, re, &g);
				t = alt_str_slice(t, 2, t.n);
				push(&p, re);
				break;
			}
			if (parse_escape(&p, t, &c, &t) < 0 || literal(&p, c) < 0)
				goto err;
			break;
		}
		}
		last_repeat = repeat_at;
	next:
		if (p.code)
			goto err;
	}
	concat(&p);
	if (swap_vertical_bar(&p))
		p.n--;
	alternate(&p);
	if (p.code)
		goto err;
	if (p.n != 1) {
		fail(&p, E_PAREN, s);
		goto err;
	}
	*ncap = p.numcap;
	return p.stack[0];
err:
	*code = p.code ? p.code : "regexp/syntax: internal error";
	*expr = p.expr;
	return 0;
}
