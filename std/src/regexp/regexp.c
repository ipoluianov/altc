/* Regular expressions after Go's regexp and regexp/syntax (BSD-3-Clause,
   see LICENSE-Go): Simplify, Compile, and the NFA (Pike VM) with the
   same thread priorities, so the same matches and submatches. */
#include "regexp_internal.h"
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* ---- Simplify ---- */

static node *mk(rx_arena *a, int op, uint16_t flags, node *sub) {
	node *n = alt_arena_zalloc(a, sizeof *n);
	if (!n)
		return 0;
	n->op = op;
	n->flags = flags;
	if (sub) {
		n->sub = alt_arena_alloc(a, sizeof *n->sub);
		if (!n->sub)
			return 0;
		n->sub[0] = sub;
		n->nsub = 1;
	}
	return n;
}

static node *simplify1(rx_arena *a, int op, uint16_t flags, node *sub, node *re) {
	if (sub->op == OP_EMPTY_MATCH)
		return sub;
	if (op == sub->op && (flags & F_NONGREEDY) == (sub->flags & F_NONGREEDY))
		return sub;
	if (re && re->op == op && (re->flags & F_NONGREEDY) == (flags & F_NONGREEDY) && sub == re->sub[0])
		return re;
	return mk(a, op, flags, sub);
}

static node *concat2(rx_arena *a, node **subs, size_t n) {
	node *c = mk(a, OP_CONCAT, 0, 0);
	if (!c)
		return 0;
	c->sub = alt_arena_alloc(a, (n ? n : 1) * sizeof *c->sub);
	if (!c->sub)
		return 0;
	memcpy(c->sub, subs, n * sizeof *subs);
	c->nsub = n;
	return c;
}

static node *simplify(rx_arena *a, node *re) {
	switch (re->op) {
	case OP_CAPTURE: case OP_CONCAT: case OP_ALTERNATE: {
		node *nre = re;
		for (size_t i = 0; i < re->nsub; i++) {
			node *ns = simplify(a, re->sub[i]);
			if (!ns)
				return 0;
			if (nre == re && ns != re->sub[i]) {
				nre = alt_arena_alloc(a, sizeof *nre);
				if (!nre)
					return 0;
				*nre = *re;
				nre->sub = alt_arena_alloc(a, re->nsub * sizeof *nre->sub);
				if (!nre->sub)
					return 0;
				memcpy(nre->sub, re->sub, re->nsub * sizeof *nre->sub);
			}
			if (nre != re)
				nre->sub[i] = ns;
		}
		return nre;
	}
	case OP_STAR: case OP_PLUS: case OP_QUEST: {
		node *sub = simplify(a, re->sub[0]);
		return sub ? simplify1(a, re->op, re->flags, sub, re) : 0;
	}
	case OP_REPEAT: {
		if (re->min == 0 && re->max == 0)
			return mk(a, OP_EMPTY_MATCH, 0, 0);
		node *sub = simplify(a, re->sub[0]);
		if (!sub)
			return 0;
		if (re->max == -1) {
			if (re->min == 0)
				return simplify1(a, OP_STAR, re->flags, sub, 0);
			if (re->min == 1)
				return simplify1(a, OP_PLUS, re->flags, sub, 0);
			node **subs = alt_arena_alloc(a, (size_t)re->min * sizeof *subs);
			if (!subs)
				return 0;
			for (int i = 0; i < re->min - 1; i++)
				subs[i] = sub;
			subs[re->min - 1] = simplify1(a, OP_PLUS, re->flags, sub, 0);
			return concat2(a, subs, (size_t)re->min);
		}
		if (re->min == 1 && re->max == 1)
			return sub;
		node *prefix = 0;
		node **pre = 0;
		size_t npre = 0;
		if (re->min > 0) {
			pre = alt_arena_alloc(a, ((size_t)re->min + 1) * sizeof *pre);
			if (!pre)
				return 0;
			for (int i = 0; i < re->min; i++)
				pre[npre++] = sub;
		}
		if (re->max > re->min) {
			node *suffix = simplify1(a, OP_QUEST, re->flags, sub, 0);
			for (int i = re->min + 1; i < re->max && suffix; i++) {
				node *pair[2] = {sub, suffix};
				node *c = concat2(a, pair, 2);
				suffix = c ? simplify1(a, OP_QUEST, re->flags, c, 0) : 0;
			}
			if (!suffix)
				return 0;
			if (!pre)
				return suffix;
			pre[npre++] = suffix;
		}
		if (pre) {
			prefix = concat2(a, pre, npre);
			return prefix;
		}
		return mk(a, OP_NO_MATCH, 0, 0);
	}
	}
	return re;
}

/* ---- the program ---- */

enum { I_ALT, I_ALT_MATCH, I_CAPTURE, I_EMPTY, I_MATCH, I_FAIL, I_NOP, I_RUNE, I_RUNE1, I_RUNE_ANY, I_RUNE_ANY_NOT_NL };
enum { E_BEGIN_LINE = 1, E_END_LINE = 2, E_BEGIN_TEXT = 4, E_END_TEXT = 8, E_WORD = 16, E_NO_WORD = 32 };

typedef struct {
	int op;
	uint32_t out, arg;
	const int32_t *runes;
	size_t nrunes;
} inst;

typedef struct {
	inst *p;
	size_t n, cap;
	int numcap;
	bool oom;
} prog;

typedef struct {
	uint32_t head, tail;
} plist;

typedef struct {
	uint32_t i;
	plist out;
	bool nullable;
} frag;

static frag ins(prog *p, int op) {
	if (p->n == p->cap) {
		size_t cap = p->cap ? p->cap * 2 : 64;
		inst *ni = realloc(p->p, cap * sizeof *ni);
		if (!ni) {
			p->oom = true;
			return (frag){0, {0, 0}, true};
		}
		p->p = ni;
		p->cap = cap;
	}
	frag f = {(uint32_t)p->n, {0, 0}, true};
	p->p[p->n++] = (inst){op, 0, 0, 0, 0};
	return f;
}

static plist mkpl(uint32_t n) { return (plist){n, n}; }

static void patch(prog *p, plist l, uint32_t val) {
	uint32_t head = l.head;
	while (head) {
		inst *i = &p->p[head >> 1];
		if (head & 1) {
			head = i->arg;
			i->arg = val;
		} else {
			head = i->out;
			i->out = val;
		}
	}
}

static plist pl_append(prog *p, plist l1, plist l2) {
	if (!l1.head)
		return l2;
	if (!l2.head)
		return l1;
	inst *i = &p->p[l1.tail >> 1];
	if (l1.tail & 1)
		i->arg = l2.head;
	else
		i->out = l2.head;
	return (plist){l1.head, l2.tail};
}

static frag nop(prog *p) {
	frag f = ins(p, I_NOP);
	f.out = mkpl(f.i << 1);
	return f;
}

static frag cap_ins(prog *p, uint32_t arg) {
	frag f = ins(p, I_CAPTURE);
	f.out = mkpl(f.i << 1);
	if (!p->oom)
		p->p[f.i].arg = arg;
	if (p->numcap < (int)arg + 1)
		p->numcap = (int)arg + 1;
	return f;
}

static frag cat(prog *p, frag f1, frag f2) {
	if (!f1.i || !f2.i)
		return (frag){0, {0, 0}, false};
	patch(p, f1.out, f2.i);
	return (frag){f1.i, f2.out, f1.nullable && f2.nullable};
}

static frag alt(prog *p, frag f1, frag f2) {
	if (!f1.i)
		return f2;
	if (!f2.i)
		return f1;
	frag f = ins(p, I_ALT);
	if (p->oom)
		return f;
	p->p[f.i].out = f1.i;
	p->p[f.i].arg = f2.i;
	f.out = pl_append(p, f1.out, f2.out);
	f.nullable = f1.nullable || f2.nullable;
	return f;
}

static frag quest(prog *p, frag f1, bool ng) {
	frag f = ins(p, I_ALT);
	if (p->oom)
		return f;
	if (ng) {
		p->p[f.i].arg = f1.i;
		f.out = mkpl(f.i << 1);
	} else {
		p->p[f.i].out = f1.i;
		f.out = mkpl(f.i << 1 | 1);
	}
	f.out = pl_append(p, f.out, f1.out);
	return f;
}

static frag loop(prog *p, frag f1, bool ng) {
	frag f = ins(p, I_ALT);
	if (p->oom)
		return f;
	if (ng) {
		p->p[f.i].arg = f1.i;
		f.out = mkpl(f.i << 1);
	} else {
		p->p[f.i].out = f1.i;
		f.out = mkpl(f.i << 1 | 1);
	}
	patch(p, f1.out, f.i);
	return f;
}

static frag plus(prog *p, frag f1, bool ng) { return (frag){f1.i, loop(p, f1, ng).out, f1.nullable}; }

static frag star(prog *p, frag f1, bool ng) {
	if (f1.nullable)
		return quest(p, plus(p, f1, ng), ng); /* (f1+)? for the priorities (Go issue 46123) */
	return loop(p, f1, ng);
}

static frag empty(prog *p, int op) {
	frag f = ins(p, I_EMPTY);
	if (!p->oom)
		p->p[f.i].arg = (uint32_t)op;
	f.out = mkpl(f.i << 1);
	return f;
}

static const int32_t any_not_nl[] = {0, '\n' - 1, '\n' + 1, 0x10ffff}, any_rune[] = {0, 0x10ffff};

static frag rune_ins(prog *p, const int32_t *r, size_t n, uint16_t flags) {
	frag f = ins(p, I_RUNE);
	f.nullable = false;
	if (p->oom)
		return f;
	inst *i = &p->p[f.i];
	i->runes = r;
	i->nrunes = n;
	flags &= F_FOLD;
	if (n != 1 || alt_unicode_simple_fold(r[0]) == r[0])
		flags &= (uint16_t)~F_FOLD;
	i->arg = flags;
	f.out = mkpl(f.i << 1);
	if (!(flags & F_FOLD) && (n == 1 || (n == 2 && r[0] == r[1])))
		i->op = I_RUNE1;
	else if (n == 2 && r[0] == 0 && r[1] == 0x10ffff)
		i->op = I_RUNE_ANY;
	else if (n == 4 && r[0] == 0 && r[1] == '\n' - 1 && r[2] == '\n' + 1 && r[3] == 0x10ffff)
		i->op = I_RUNE_ANY_NOT_NL;
	return f;
}

static frag compile(prog *p, const node *re) {
	frag f = {0, {0, 0}, false};
	switch (re->op) {
	case OP_NO_MATCH: return f;
	case OP_EMPTY_MATCH: return nop(p);
	case OP_LITERAL:
		if (!re->nrunes)
			return nop(p);
		for (size_t j = 0; j < re->nrunes; j++) {
			frag f1 = rune_ins(p, re->runes + j, 1, re->flags);
			f = j == 0 ? f1 : cat(p, f, f1);
		}
		return f;
	case OP_CHAR_CLASS: return rune_ins(p, re->runes, re->nrunes, re->flags);
	case OP_ANY_CHAR_NOT_NL: return rune_ins(p, any_not_nl, 4, 0);
	case OP_ANY_CHAR: return rune_ins(p, any_rune, 2, 0);
	case OP_BEGIN_LINE: return empty(p, E_BEGIN_LINE);
	case OP_END_LINE: return empty(p, E_END_LINE);
	case OP_BEGIN_TEXT: return empty(p, E_BEGIN_TEXT);
	case OP_END_TEXT: return empty(p, E_END_TEXT);
	case OP_WORD_BOUNDARY: return empty(p, E_WORD);
	case OP_NO_WORD_BOUNDARY: return empty(p, E_NO_WORD);
	case OP_CAPTURE: {
		frag bra = cap_ins(p, (uint32_t)re->cap << 1);
		frag sub = compile(p, re->sub[0]);
		frag ket = cap_ins(p, (uint32_t)re->cap << 1 | 1);
		return cat(p, cat(p, bra, sub), ket);
	}
	case OP_STAR: return star(p, compile(p, re->sub[0]), re->flags & F_NONGREEDY);
	case OP_PLUS: return plus(p, compile(p, re->sub[0]), re->flags & F_NONGREEDY);
	case OP_QUEST: return quest(p, compile(p, re->sub[0]), re->flags & F_NONGREEDY);
	case OP_CONCAT:
		if (!re->nsub)
			return nop(p);
		for (size_t i = 0; i < re->nsub; i++)
			f = i == 0 ? compile(p, re->sub[i]) : cat(p, f, compile(p, re->sub[i]));
		return f;
	case OP_ALTERNATE:
		for (size_t i = 0; i < re->nsub; i++)
			f = alt(p, f, compile(p, re->sub[i]));
		return f;
	}
	return f;
}

struct alt_regexp {
	char *expr;
	size_t nexpr;
	inst *p;
	size_t ninst;
	uint32_t start;
	int numcap;        /* 2 * (subexpressions + 1) */
	size_t nsub;
	alt_str *names;
	bool longest;
	alt_arena arena;   /* the tree's runes the program points to */
};

static void cap_names(const node *re, alt_str *names) {
	if (re->op == OP_CAPTURE)
		names[re->cap] = re->name;
	for (size_t i = 0; i < re->nsub; i++)
		cap_names(re->sub[i], names);
}

static int build(alt_regexp **out, alt_str expr, uint16_t flags, bool longest, alt_buf *err) {
	alt_regexp *re = calloc(1, sizeof *re);
	if (!re)
		return -ENOMEM;
	alt_arena_init(&re->arena, 4096);
	re->expr = malloc(expr.n + 1);
	if (!re->expr) {
		free(re);
		return -ENOMEM;
	}
	memcpy(re->expr, expr.p, expr.n);
	re->expr[expr.n] = 0;
	re->nexpr = expr.n;
	alt_str e = alt_str_make(re->expr, expr.n);
	int ncap;
	const char *code;
	alt_str bad;
	node *tree = rx_parse(&re->arena, e, flags, &ncap, &code, &bad);
	if (!tree) {
		if (err)
			alt_buf_printf(err, "error parsing regexp: %s: `%.*s`", code, (int)bad.n, bad.p);
		alt_regexp_free(re);
		return -EINVAL;
	}
	re->nsub = (size_t)ncap;
	re->names = alt_arena_zalloc(&re->arena, ((size_t)ncap + 1) * sizeof *re->names);
	if (!re->names) {
		alt_regexp_free(re);
		return -ENOMEM;
	}
	for (int i = 0; i <= ncap; i++)
		re->names[i] = ALT_STR("");
	cap_names(tree, re->names);
	node *s = simplify(&re->arena, tree);
	prog p = {0};
	p.numcap = 2;
	ins(&p, I_FAIL);
	frag f = s ? compile(&p, s) : (frag){0, {0, 0}, false};
	frag m = ins(&p, I_MATCH);
	patch(&p, f.out, m.i);
	if (!s || p.oom) {
		free(p.p);
		alt_regexp_free(re);
		return -ENOMEM;
	}
	re->p = p.p;
	re->ninst = p.n;
	re->start = f.i;
	re->numcap = 2 * (ncap + 1);
	re->longest = longest;
	*out = re;
	return 0;
}

int alt_regexp_compile(alt_regexp **out, alt_str expr, alt_buf *err) { return build(out, expr, F_PERL, false, err); }
int alt_regexp_compile_posix(alt_regexp **out, alt_str expr, alt_buf *err) { return build(out, expr, F_POSIX, true, err); }

void alt_regexp_free(alt_regexp *re) {
	if (!re)
		return;
	free(re->expr);
	free(re->p);
	alt_arena_free(&re->arena);
	free(re);
}

void alt_regexp_longest(alt_regexp *re) { re->longest = true; }
alt_str alt_regexp_string(const alt_regexp *re) { return alt_str_make(re->expr, re->nexpr); }
size_t alt_regexp_num_subexp(const alt_regexp *re) { return re->nsub; }
alt_str alt_regexp_subexp_name(const alt_regexp *re, size_t i) { return i <= re->nsub ? re->names[i] : ALT_STR(""); }

int alt_regexp_subexp_index(const alt_regexp *re, alt_str name) {
	if (name.n)
		for (size_t i = 1; i <= re->nsub; i++)
			if (alt_str_eq(re->names[i], name))
				return (int)i;
	return -1;
}

/* ---- the NFA ---- */

typedef struct {
	uint32_t *sparse;
	uint32_t *pc;         /* dense: the pcs */
	ssize_t **caps;       /* dense: the thread's captures (NULL: no thread) */
	size_t n;
} queue;

typedef struct {
	const alt_regexp *re;
	const char *s;
	size_t len;
	int ncap;             /* captures tracked (0: only whether it matches) */
	queue q0, q1;
	ssize_t *pool;        /* capture arrays */
	ssize_t **free_caps;
	size_t nfree;
	ssize_t *matchcap;
	bool matched;
	/* the explicit stack of add */
	struct frame {
		uint32_t pc;
		int restore;      /* >= 0: restore cap[restore] = old */
		ssize_t old;
	} *stack;
	size_t nstack, capstack;
} machine;

static bool word_char(int32_t r) { return (r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z') || (r >= '0' && r <= '9') || r == '_'; }

static bool cond_match(int op, int32_t before, int32_t after) {
	if (op == 0)
		return true;
	if (op & E_BEGIN_LINE) {
		if (before != '\n' && before >= 0)
			return false;
		op &= ~E_BEGIN_LINE;
	}
	if (op & E_BEGIN_TEXT) {
		if (before >= 0)
			return false;
		op &= ~E_BEGIN_TEXT;
	}
	if (op == 0)
		return true;
	if (op & E_END_LINE) {
		if (after != '\n' && after >= 0)
			return false;
		op &= ~E_END_LINE;
	}
	if (op & E_END_TEXT) {
		if (after >= 0)
			return false;
		op &= ~E_END_TEXT;
	}
	if (op == 0)
		return true;
	if (word_char(before) != word_char(after))
		op &= ~E_WORD;
	else
		op &= ~E_NO_WORD;
	return op == 0;
}

static bool match_rune(const inst *i, int32_t r) {
	const int32_t *rs = i->runes;
	switch (i->nrunes) {
	case 0: return false;
	case 1:
		if (r == rs[0])
			return true;
		if (i->arg & F_FOLD)
			for (int32_t r1 = alt_unicode_simple_fold(rs[0]); r1 != rs[0]; r1 = alt_unicode_simple_fold(r1))
				if (r == r1)
					return true;
		return false;
	case 2: return r >= rs[0] && r <= rs[1];
	}
	size_t lo = 0, hi = i->nrunes / 2;
	while (lo < hi) {
		size_t m = lo + (hi - lo) / 2;
		if (r < rs[2 * m])
			hi = m;
		else if (r > rs[2 * m + 1])
			lo = m + 1;
		else
			return true;
	}
	return false;
}

static ssize_t *get_caps(machine *m) { return m->nfree ? m->free_caps[--m->nfree] : 0; }
static void put_caps(machine *m, ssize_t *c) {
	if (c)
		m->free_caps[m->nfree++] = c;
}

static bool push_frame(machine *m, uint32_t pc, int restore, ssize_t old) {
	if (m->nstack == m->capstack) {
		size_t cap = m->capstack ? m->capstack * 2 : 64;
		struct frame *ns = realloc(m->stack, cap * sizeof *ns);
		if (!ns)
			return false;
		m->stack = ns;
		m->capstack = cap;
	}
	m->stack[m->nstack++] = (struct frame){pc, restore, old};
	return true;
}

/* Go's machine.add, with an explicit stack in the same order */
static void add(machine *m, queue *q, uint32_t pc0, size_t pos, ssize_t *cap, int32_t before, int32_t after) {
	size_t base = m->nstack;
	push_frame(m, pc0, -1, 0);
	while (m->nstack > base) {
		struct frame f = m->stack[--m->nstack];
		if (f.restore >= 0) {
			cap[f.restore] = f.old;
			continue;
		}
		uint32_t pc = f.pc;
		if (pc == 0)
			continue;
		uint32_t j = q->sparse[pc];
		if (j < q->n && q->pc[j] == pc)
			continue;
		j = (uint32_t)q->n++;
		q->sparse[pc] = j;
		q->pc[j] = pc;
		q->caps[j] = 0;
		const inst *i = &m->re->p[pc];
		switch (i->op) {
		case I_FAIL: break;
		case I_ALT: case I_ALT_MATCH:
			push_frame(m, i->arg, -1, 0);
			push_frame(m, i->out, -1, 0);
			break;
		case I_EMPTY:
			if (cond_match((int)i->arg, before, after))
				push_frame(m, i->out, -1, 0);
			break;
		case I_NOP: push_frame(m, i->out, -1, 0); break;
		case I_CAPTURE:
			if ((int)i->arg < m->ncap) {
				push_frame(m, 0, (int)i->arg, cap[i->arg]);
				cap[i->arg] = (ssize_t)pos;
			}
			push_frame(m, i->out, -1, 0);
			break;
		default: {
			ssize_t *t = get_caps(m);
			if (m->ncap)
				memcpy(t, cap, (size_t)m->ncap * sizeof *t);
			q->caps[j] = t ? t : (ssize_t *)1; /* a thread even without captures */
		}
		}
	}
}

static void clear(machine *m, queue *q) {
	for (size_t j = 0; j < q->n; j++)
		if (q->caps[j] && q->caps[j] != (ssize_t *)1)
			put_caps(m, q->caps[j]);
	q->n = 0;
}

/* the rune at pos (Go's inputString.step): -1 at the end */
static int32_t step_at(const machine *m, size_t pos, size_t *w) {
	if (pos >= m->len) {
		*w = 0;
		return -1;
	}
	unsigned char c = (unsigned char)m->s[pos];
	if (c < 0x80) {
		*w = 1;
		return c;
	}
	int32_t r;
	*w = alt_utf8_decode(m->s + pos, m->len - pos, &r);
	return r;
}

static int32_t rune_before(const machine *m, size_t pos) {
	if (pos == 0)
		return -1;
	int32_t r;
	alt_utf8_decode_last(m->s, pos, &r);
	return r;
}

static void step(machine *m, queue *run, queue *next, size_t pos, size_t next_pos, int32_t c, int32_t nb, int32_t na) {
	bool longest = m->re->longest;
	for (size_t j = 0; j < run->n; j++) {
		ssize_t *t = run->caps[j];
		if (!t)
			continue;
		ssize_t *tc = t == (ssize_t *)1 ? 0 : t;
		if (longest && m->matched && m->ncap > 0 && m->matchcap[0] < tc[0]) {
			put_caps(m, tc);
			run->caps[j] = 0;
			continue;
		}
		const inst *i = &m->re->p[run->pc[j]];
		bool go = false;
		switch (i->op) {
		case I_MATCH:
			if (m->ncap > 0 && (!longest || !m->matched || m->matchcap[1] < (ssize_t)pos)) {
				tc[1] = (ssize_t)pos;
				memcpy(m->matchcap, tc, (size_t)m->ncap * sizeof *tc);
			}
			if (!longest) {
				/* first-match mode: the lower-priority threads go */
				for (size_t k = j + 1; k < run->n; k++)
					if (run->caps[k] && run->caps[k] != (ssize_t *)1) {
						put_caps(m, run->caps[k]);
						run->caps[k] = 0;
					}
				run->n = j + 1;
			}
			m->matched = true;
			break;
		case I_RUNE: go = match_rune(i, c); break;
		case I_RUNE1: go = c == i->runes[0]; break;
		case I_RUNE_ANY: go = true; break;
		case I_RUNE_ANY_NOT_NL: go = c != '\n'; break;
		}
		if (go)
			add(m, next, i->out, next_pos, tc, nb, na);
		if (tc)
			put_caps(m, tc);
		run->caps[j] = 0;
	}
	run->n = 0;
}

/* Go's machine.match from pos: whether there is a match (caps in m->matchcap) */
static bool run_match(machine *m, size_t pos) {
	m->matched = false;
	for (int i = 0; i < m->ncap; i++)
		m->matchcap[i] = -1;
	queue *runq = &m->q0, *nextq = &m->q1;
	size_t w, w1;
	int32_t r = step_at(m, pos, &w), r1 = -1;
	if (r != -1)
		r1 = step_at(m, pos + w, &w1);
	else
		w1 = 0;
	int32_t before = rune_before(m, pos), after = r;
	/* an anchored program: only from the start of the text */
	const inst *st = &m->re->p[m->re->start];
	bool anchored = st->op == I_EMPTY && (st->arg & E_BEGIN_TEXT);
	ssize_t *cap = calloc((size_t)(m->ncap ? m->ncap : 1), sizeof *cap);
	if (!cap)
		return false;
	for (;;) {
		if (runq->n == 0) {
			if (anchored && pos != 0)
				break;
			if (m->matched)
				break;
		}
		if (!m->matched && (pos == 0 || !anchored)) {
			for (int i = 0; i < m->ncap; i++)
				cap[i] = m->matchcap[i];
			if (m->ncap > 0)
				cap[0] = (ssize_t)pos;
			add(m, runq, m->re->start, pos, cap, before, after);
		}
		/* the context after this rune */
		int32_t nb = r, na = r1;
		step(m, runq, nextq, pos, pos + w, r, nb, na);
		if (w == 0)
			break;
		if (m->ncap == 0 && m->matched)
			break;
		pos += w;
		before = r, after = r1;
		r = r1, w = w1;
		if (r != -1)
			r1 = step_at(m, pos + w, &w1);
		else
			r1 = -1, w1 = 0;
		queue *t = runq;
		runq = nextq, nextq = t;
	}
	clear(m, nextq);
	clear(m, runq);
	free(cap);
	return m->matched;
}

static bool machine_init(machine *m, const alt_regexp *re, alt_str s, int ncap) {
	memset(m, 0, sizeof *m);
	m->re = re;
	m->s = s.p ? s.p : "";
	m->len = s.n;
	m->ncap = ncap;
	size_t n = re->ninst;
	size_t threads = 2 * n + 2;
	m->q0.sparse = calloc(n, sizeof(uint32_t));
	m->q1.sparse = calloc(n, sizeof(uint32_t));
	m->q0.pc = malloc(n * sizeof(uint32_t));
	m->q1.pc = malloc(n * sizeof(uint32_t));
	m->q0.caps = malloc(n * sizeof(ssize_t *));
	m->q1.caps = malloc(n * sizeof(ssize_t *));
	m->matchcap = calloc((size_t)(ncap ? ncap : 1), sizeof(ssize_t));
	m->free_caps = malloc(threads * sizeof(ssize_t *));
	if (ncap)
		m->pool = malloc(threads * (size_t)ncap * sizeof(ssize_t));
	if (!m->q0.sparse || !m->q1.sparse || !m->q0.pc || !m->q1.pc || !m->q0.caps || !m->q1.caps || !m->matchcap || !m->free_caps ||
		(ncap && !m->pool))
		return false;
	if (ncap)
		for (size_t i = 0; i < threads; i++)
			m->free_caps[m->nfree++] = m->pool + i * (size_t)ncap;
	return true;
}

static void machine_free(machine *m) {
	free(m->q0.sparse), free(m->q1.sparse), free(m->q0.pc), free(m->q1.pc), free(m->q0.caps), free(m->q1.caps);
	free(m->matchcap), free(m->free_caps), free(m->pool), free(m->stack);
}

/* the match at or after pos with ncap positions */
static bool execute(const alt_regexp *re, alt_str s, size_t pos, ssize_t *caps, int ncap) {
	machine m;
	bool ok = false;
	if (machine_init(&m, re, s, ncap) && pos <= s.n) {
		ok = run_match(&m, pos);
		if (ok && caps)
			memcpy(caps, m.matchcap, (size_t)ncap * sizeof *caps);
	}
	machine_free(&m);
	return ok;
}

bool alt_regexp_match(const alt_regexp *re, alt_str s) { return execute(re, s, 0, 0, 0); }
bool alt_regexp_find(const alt_regexp *re, alt_str s, ssize_t *m) { return execute(re, s, 0, m, 2); }
bool alt_regexp_find_submatch(const alt_regexp *re, alt_str s, ssize_t *m) { return execute(re, s, 0, m, re->numcap); }

bool alt_regexp_find_at(const alt_regexp *re, alt_str s, size_t pos, ssize_t *m, size_t nm) {
	int n = (int)nm < re->numcap ? (int)nm : re->numcap;
	return execute(re, s, pos, m, n);
}

/* Go's allMatches */
size_t alt_regexp_find_all(const alt_regexp *re, alt_str s, ssize_t n, void (*fn)(void *arg, const ssize_t *m), void *arg) {
	ssize_t *caps = malloc((size_t)re->numcap * sizeof *caps);
	if (!caps)
		return 0;
	size_t count = 0;
	ssize_t prev_end = -1;
	machine m;
	if (!machine_init(&m, re, s, re->numcap)) {
		machine_free(&m);
		free(caps);
		return 0;
	}
	for (size_t pos = 0; (n < 0 || count < (size_t)n) && pos <= s.n;) {
		if (!run_match(&m, pos))
			break;
		memcpy(caps, m.matchcap, (size_t)re->numcap * sizeof *caps);
		bool accept = true;
		if (caps[1] == (ssize_t)pos) {
			if (caps[0] == prev_end)
				accept = false;
			size_t w;
			step_at(&m, pos, &w);
			pos = w ? pos + w : s.n + 1;
		} else
			pos = (size_t)caps[1];
		prev_end = caps[1];
		if (accept) {
			fn(arg, caps);
			count++;
		}
	}
	machine_free(&m);
	free(caps);
	return count;
}

/* ---- replacement ---- */

static bool name_rune(int32_t r) { return alt_unicode_is_letter(r) || alt_unicode_is_digit(r) || r == '_'; }

int alt_regexp_expand(const alt_regexp *re, alt_buf *b, alt_str t, alt_str src, const ssize_t *m) {
	size_t nm = (size_t)re->numcap;
	while (t.n) {
		ssize_t d = alt_str_index_byte(t, '$');
		if (d < 0)
			break;
		alt_buf_append(b, t.p, (size_t)d);
		t = alt_str_slice(t, (size_t)d + 1, t.n);
		if (t.n && t.p[0] == '$') {
			alt_buf_append_byte(b, '$');
			t = alt_str_slice(t, 1, t.n);
			continue;
		}
		/* Go's extract */
		alt_str str = t;
		bool brace = false, ok = false;
		alt_str name = {0, 0}, rest = t;
		if (str.n) {
			if (str.p[0] == '{') {
				brace = true;
				str = alt_str_slice(str, 1, str.n);
			}
			size_t i = 0;
			while (i < str.n) {
				int32_t r;
				size_t k = alt_utf8_decode(str.p + i, str.n - i, &r);
				if (!name_rune(r))
					break;
				i += k;
			}
			if (i > 0) {
				name = alt_str_slice(str, 0, i);
				if (!brace || (i < str.n && str.p[i] == '}')) {
					if (brace)
						i++;
					rest = alt_str_slice(str, i, str.n);
					ok = true;
				}
			}
		}
		if (!ok) {
			alt_buf_append_byte(b, '$');
			continue;
		}
		t = rest;
		long num = 0;
		for (size_t i = 0; i < name.n; i++) {
			if (name.p[i] < '0' || name.p[i] > '9' || num >= 100000000) {
				num = -1;
				break;
			}
			num = num * 10 + name.p[i] - '0';
		}
		if (name.p[0] == '0' && name.n > 1)
			num = -1;
		if (num >= 0) {
			if ((size_t)(2 * num + 1) < nm && m[2 * num] >= 0)
				alt_buf_append(b, src.p + m[2 * num], (size_t)(m[2 * num + 1] - m[2 * num]));
		} else
			for (size_t i = 0; i <= re->nsub; i++)
				if (alt_str_eq(name, re->names[i]) && 2 * i + 1 < nm && m[2 * i] >= 0) {
					alt_buf_append(b, src.p + m[2 * i], (size_t)(m[2 * i + 1] - m[2 * i]));
					break;
				}
	}
	alt_buf_append(b, t.p, t.n);
	return b->err;
}

typedef struct {
	int kind; /* 0 expand, 1 literal, 2 func */
	alt_str repl;
	void (*fn)(void *, alt_buf *, alt_str, const ssize_t *);
	void *arg;
} replacer;

/* Go's replaceAll */
static int replace_all(const alt_regexp *re, alt_buf *b, alt_str src, const replacer *r) {
	int ncap = r->kind == 1 ? 2 : re->numcap;
	ssize_t *a = malloc((size_t)ncap * sizeof *a);
	machine m;
	if (!a || !machine_init(&m, re, src, ncap)) {
		free(a);
		if (a)
			machine_free(&m);
		return -ENOMEM;
	}
	size_t last_end = 0, search = 0;
	while (search <= src.n) {
		if (!run_match(&m, search))
			break;
		memcpy(a, m.matchcap, (size_t)ncap * sizeof *a);
		alt_buf_append(b, src.p + last_end, (size_t)a[0] - last_end);
		if ((size_t)a[1] > last_end || a[0] == 0) {
			if (r->kind == 1)
				alt_buf_append(b, r->repl.p, r->repl.n);
			else if (r->kind == 0)
				alt_regexp_expand(re, b, r->repl, src, a);
			else
				r->fn(r->arg, b, src, a);
		}
		last_end = (size_t)a[1];
		size_t w;
		step_at(&m, search, &w);
		if (search + w > (size_t)a[1])
			search += w;
		else if (search + 1 > (size_t)a[1])
			search++;
		else
			search = (size_t)a[1];
	}
	alt_buf_append(b, src.p + last_end, src.n - last_end);
	machine_free(&m);
	free(a);
	return b->err;
}

int alt_regexp_replace_all(const alt_regexp *re, alt_buf *b, alt_str src, alt_str repl) {
	replacer r = {0, repl, 0, 0};
	return replace_all(re, b, src, &r);
}

int alt_regexp_replace_all_literal(const alt_regexp *re, alt_buf *b, alt_str src, alt_str repl) {
	replacer r = {1, repl, 0, 0};
	return replace_all(re, b, src, &r);
}

int alt_regexp_replace_all_func(const alt_regexp *re, alt_buf *b, alt_str src,
	void (*fn)(void *arg, alt_buf *b, alt_str src, const ssize_t *m), void *arg) {
	replacer r = {2, {0, 0}, fn, arg};
	return replace_all(re, b, src, &r);
}

typedef struct {
	ssize_t (*m)[2];
	size_t n, cap;
	bool oom;
} spans;

static void collect(void *arg, const ssize_t *m) {
	spans *s = arg;
	if (s->n == s->cap) {
		size_t cap = s->cap ? s->cap * 2 : 16;
		ssize_t(*nm)[2] = realloc(s->m, cap * sizeof *nm);
		if (!nm) {
			s->oom = true;
			return;
		}
		s->m = nm;
		s->cap = cap;
	}
	s->m[s->n][0] = m[0];
	s->m[s->n++][1] = m[1];
}

int alt_regexp_split(const alt_regexp *re, alt_arena *a, alt_str s, ssize_t n, alt_str **parts, size_t *count) {
	*parts = 0;
	*count = 0;
	if (n == 0)
		return 0;
	if (re->nexpr > 0 && s.n == 0) {
		*parts = alt_arena_alloc(a, sizeof **parts);
		if (!*parts)
			return -ENOMEM;
		(*parts)[0] = s;
		*count = 1;
		return 0;
	}
	spans sp = {0};
	alt_regexp_find_all(re, s, n, collect, &sp);
	if (sp.oom) {
		free(sp.m);
		return -ENOMEM;
	}
	alt_str *out = alt_arena_alloc(a, (sp.n + 1) * sizeof *out);
	if (!out) {
		free(sp.m);
		return -ENOMEM;
	}
	size_t k = 0, beg = 0, end = 0;
	for (size_t i = 0; i < sp.n; i++) {
		if (n > 0 && k == (size_t)n - 1)
			break;
		end = (size_t)sp.m[i][0];
		if (sp.m[i][1] != 0)
			out[k++] = alt_str_slice(s, beg, end);
		beg = (size_t)sp.m[i][1];
	}
	if (end != s.n)
		out[k++] = alt_str_slice(s, beg, s.n);
	free(sp.m);
	*parts = out;
	*count = k;
	return 0;
}

int alt_regexp_quote_meta(alt_buf *b, alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		if (strchr("\\.+*?()|[]{}^$", s.p[i]) && s.p[i])
			alt_buf_append_byte(b, '\\');
		alt_buf_append_byte(b, s.p[i]);
	}
	return b->err;
}

int alt_regexp_match_string(alt_str pattern, alt_str s) {
	alt_regexp *re;
	int rc = alt_regexp_compile(&re, pattern, 0);
	if (rc < 0)
		return rc;
	rc = alt_regexp_match(re, s);
	alt_regexp_free(re);
	return rc;
}
