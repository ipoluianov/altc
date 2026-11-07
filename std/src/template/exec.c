/* The execution of templates, after Go's text/template/exec.go and
   funcs.go (BSD-3-Clause, see LICENSE-Go), on the values of JSON. */
#include "template_internal.h"
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

/* the C stack does not grow: Go allows 100000 */
#define MAX_EXEC_DEPTH 1000

enum { W_NONE, W_BREAK, W_CONTINUE };

typedef struct {
	alt_str name;
	tval value;
} tvar;

typedef struct {
	jmp_buf jb;
	alt_buf *err;
	alt_arena arena;
	alt_buf *wr;
} texec_ctx;

typedef struct {
	alt_template *tmpl;
	const tnode *node;
	tvar *vars;
	size_t nvars, vcap;
	int depth;
	texec_ctx *x;
} tstate;

static tval zero_val(void) {
	tval v;
	memset(&v, 0, sizeof v);
	return v;
}

static tval missing_val(void) {
	tval v = zero_val();
	v.k = V_MISSING;
	return v;
}

static bool is_missing(const tval *v) { return v->k == V_MISSING; }
static bool is_valid(const tval *v) { return v->k != V_INVALID; }

static tval int_val(long long i) {
	tval v = zero_val();
	v.k = V_INT;
	v.i = i;
	return v;
}

static tval bool_val(bool b) {
	tval v = zero_val();
	v.k = V_BOOL;
	v.b = b;
	return v;
}

static tval str_val(alt_str s) {
	tval v = zero_val();
	v.k = V_STRING;
	v.s = s;
	return v;
}

/* reflect.Value.Elem of an interface (invalid for nil) */
static tval indirect_interface(tval v) {
	if (!v.iface)
		return v;
	if (v.k == V_NIL)
		return zero_val();
	v.iface = 0;
	return v;
}

/* indirect: through the interface; is_nil for a nil one (v unchanged) */
static tval indirect(tval v, bool *is_nil) {
	*is_nil = false;
	if (v.iface) {
		if (v.k == V_NIL) {
			*is_nil = true;
			return v;
		}
		v.iface = 0;
	}
	return v;
}

static size_t map_len(alt_arena *a, const alt_json *j) {
	const alt_json_member **ms;
	return tmap_sorted(a, j, &ms);
}

/* isTrue on a non-interface value */
static bool is_true(alt_arena *a, const tval *v) {
	switch (v->k) {
	case V_INVALID:
	case V_MISSING:
		return false;
	case V_NIL:
		return false;
	case V_BOOL:
		return v->b;
	case V_INT:
	case V_UINT8:
		return v->i != 0;
	case V_FLOAT:
		return v->f != 0;
	case V_COMPLEX:
		return v->f != 0 || v->im != 0;
	case V_STRING:
		return v->s.n > 0;
	case V_MAP:
		return v->j->n > 0;
	case V_SLICE:
		return v->len > 0;
	}
	return false;
}

static bool truth(alt_arena *a, tval v) {
	v = indirect_interface(v);
	return is_true(a, &v);
}

/* ---- errors ---- */

static alt_str node_str(tstate *s, const tnode *n) {
	alt_buf b = ALT_BUF_INIT;
	tnode_string(&b, n);
	alt_str r = tstr_dup(&s->x->arena, alt_buf_str(&b));
	alt_buf_free(&b);
	return r;
}

static void __attribute__((noreturn)) serrorf(tstate *s, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void serrorf(tstate *s, const char *fmt, ...) {
	alt_buf *e = s->x->err;
	alt_str name = s->tmpl->name;
	if (!s->node) {
		alt_buf_printf(e, "template: %.*s: ", (int)name.n, name.p);
	} else {
		const tnode *n = s->node;
		ttree *tree = n->tr;
		alt_str text = alt_str_slice(tree->text, 0, (size_t)n->pos);
		ssize_t nl = alt_str_last_index_byte(text, '\n');
		int byte_num = nl < 0 ? n->pos : n->pos - (int)(nl + 1);
		int line = 1 + (int)alt_str_count(text, ALT_STR("\n"));
		alt_buf_printf(e, "template: %.*s:%d:%d: executing ", (int)tree->parse_name.n, tree->parse_name.p, line, byte_num);
		tquote(e, name);
		alt_buf_append_cstr(e, " at <");
		tnode_string(e, n);
		alt_buf_append_cstr(e, ">: ");
	}
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(e, fmt, ap);
	va_end(ap);
	longjmp(s->x->jb, 1);
}

static const char *quoted(tstate *s, alt_str str) {
	alt_buf b = ALT_BUF_INIT;
	tquote(&b, str);
	const char *r = tstr_dup(&s->x->arena, alt_buf_str(&b)).p;
	alt_buf_free(&b);
	return r;
}

/* fmt's %v (or another verb) of a value, for messages */
static const char *sprint_verb(alt_arena *a, const tval *v, const char *verb) {
	alt_buf b = ALT_BUF_INIT;
	tval x = *v;
	x.iface = 0;
	if (x.k == V_NIL)
		x.k = V_INVALID;
	if (!strcmp(verb, "%v") && x.k == V_INVALID)
		alt_buf_append_cstr(&b, "<invalid reflect.Value>");
	else
		tfmt_sprintf(&b, alt_str_cstr(verb), &x, 1);
	const char *r = tstr_dup(a, alt_buf_str(&b)).p;
	alt_buf_free(&b);
	return r;
}

/* ---- variables ---- */

static void push(tstate *s, alt_str name, tval v) {
	if (s->nvars == s->vcap) {
		size_t nc = s->vcap ? 2 * s->vcap : 8;
		s->vars = alt_arena_realloc(&s->x->arena, s->vars, s->vcap * sizeof(tvar), nc * sizeof(tvar));
		s->vcap = nc;
	}
	s->vars[s->nvars++] = (tvar){name, v};
}

static void set_var(tstate *s, alt_str name, tval v) {
	for (size_t i = s->nvars; i-- > 0;)
		if (alt_str_eq(s->vars[i].name, name)) {
			s->vars[i].value = v;
			return;
		}
	serrorf(s, "undefined variable: %.*s", (int)name.n, name.p);
}

static tval var_value(tstate *s, alt_str name) {
	for (size_t i = s->nvars; i-- > 0;)
		if (alt_str_eq(s->vars[i].name, name))
			return s->vars[i].value;
	serrorf(s, "undefined variable: %.*s", (int)name.n, name.p);
}

/* ---- evaluation ---- */

enum { AT_NONE, AT_VALUE, AT_ANY, AT_STRING };

static const char *at_name(int t) { return t == AT_VALUE ? "reflect.Value" : t == AT_ANY ? "interface {}" : "string"; }

typedef struct {
	int num_in;
	bool variadic;
	uint8_t in[8]; /* the types, the last one the variadic element's */
} fsig;

static tval eval_pipeline(tstate *s, tval dot, const tnode *pipe);
static tval eval_arg(tstate *s, tval dot, int typ, const tnode *n);
static tval eval_field_chain(tstate *s, tval dot, tval receiver, const tnode *node, const alt_str *ident, size_t nident,
	const tnode *const *args, size_t nargs, tval final);
static tval eval_call(tstate *s, tval dot, const tfunc *fn, const tnode *node, alt_str name, const tnode *const *args, size_t nargs,
	tval final);
static int walk(tstate *s, tval dot, const tnode *node);

static void not_a_function(tstate *s, const tnode *const *args, size_t nargs, tval final) {
	if (nargs > 1 || !is_missing(&final))
		serrorf(s, "can't give argument to non-function %s", node_str(s, args[0]).p);
}

static bool is_hex_int(alt_str t) {
	return t.n > 2 && t.p[0] == '0' && (t.p[1] == 'x' || t.p[1] == 'X') && alt_str_index_any(t, "pP") < 0;
}

static tval ideal_constant(tstate *s, const tnode *c) {
	s->node = c;
	if (c->is_complex) {
		tval v = zero_val();
		v.k = V_COMPLEX;
		v.f = c->re;
		v.im = c->im;
		return v;
	}
	if (c->is_float && !is_hex_int(c->text) && !(c->text.n && c->text.p[0] == '\'') && alt_str_index_any(c->text, ".eEpP") >= 0) {
		tval v = zero_val();
		v.k = V_FLOAT;
		v.f = c->f64;
		return v;
	}
	if (c->is_int)
		return int_val(c->i64);
	if (c->is_uint)
		serrorf(s, "%.*s overflows int", (int)c->text.n, c->text.p);
	return zero_val();
}

static tval eval_field(tstate *s, tval dot, alt_str field, const tnode *node, const tnode *const *args, size_t nargs, tval final,
	tval receiver) {
	(void)dot;
	(void)node;
	if (!is_valid(&receiver)) {
		if (s->tmpl->common->missingkey == 2)
			serrorf(s, "nil data; no entry for key %s", quoted(s, field));
		return zero_val();
	}
	const char *typ = tval_static_type_name(&receiver);
	bool is_nil;
	receiver = indirect(receiver, &is_nil);
	if (is_nil)
		serrorf(s, "nil pointer evaluating %s.%.*s", typ, (int)field.n, field.p);
	bool has_args = nargs > 1 || !is_missing(&final);
	if (receiver.k == V_MAP) {
		if (has_args)
			serrorf(s, "%.*s is not a method but has arguments", (int)field.n, field.p);
		alt_json *r = alt_json_get_str(receiver.j, field);
		if (!r) {
			switch (s->tmpl->common->missingkey) {
			case 1: {
				tval v = zero_val();
				v.k = V_NIL;
				v.iface = 1;
				return v;
			}
			case 2:
				serrorf(s, "map has no entry for key %s", quoted(s, field));
			}
			return zero_val();
		}
		return tjson_val(r, true);
	}
	serrorf(s, "can't evaluate field %.*s in type %s", (int)field.n, field.p, typ);
}

static tval eval_field_chain(tstate *s, tval dot, tval receiver, const tnode *node, const alt_str *ident, size_t nident,
	const tnode *const *args, size_t nargs, tval final) {
	for (size_t i = 0; i + 1 < nident; i++)
		receiver = eval_field(s, dot, ident[i], node, 0, 0, missing_val(), receiver);
	return eval_field(s, dot, ident[nident - 1], node, args, nargs, final, receiver);
}

static tval eval_field_node(tstate *s, tval dot, const tnode *field, const tnode *const *args, size_t nargs, tval final) {
	s->node = field;
	return eval_field_chain(s, dot, dot, field, field->ident, field->nident, args, nargs, final);
}

static tval eval_chain_node(tstate *s, tval dot, const tnode *chain, const tnode *const *args, size_t nargs, tval final) {
	s->node = chain;
	if (chain->nident == 0)
		serrorf(s, "internal error: no fields in evalChainNode");
	if (chain->node->type == N_NIL)
		serrorf(s, "indirection through explicit nil in %s", node_str(s, chain).p);
	tval pipe = eval_arg(s, dot, AT_NONE, chain->node);
	return eval_field_chain(s, dot, pipe, chain, chain->ident, chain->nident, args, nargs, final);
}

static tval eval_variable_node(tstate *s, tval dot, const tnode *var, const tnode *const *args, size_t nargs, tval final) {
	s->node = var;
	tval value = var_value(s, var->ident[0]);
	if (var->nident == 1) {
		not_a_function(s, args, nargs, final);
		return value;
	}
	return eval_field_chain(s, dot, value, var, var->ident + 1, var->nident - 1, args, nargs, final);
}

static const tfunc *find_function(tstate *s, alt_str name) {
	const tfunc *f = 0;
	if (s->tmpl->common->html)
		f = thtml_escaper_func(name);
	if (!f)
		f = tfuncs_find(&s->tmpl->common->funcs, name);
	return f ? f : tbuiltin_find(name);
}

static tval eval_function(tstate *s, tval dot, const tnode *node, const tnode *cmd, const tnode *const *args, size_t nargs,
	tval final) {
	s->node = node;
	alt_str name = node->text;
	const tfunc *f = find_function(s, name);
	if (!f)
		serrorf(s, "%s is not a defined function", quoted(s, name));
	return eval_call(s, dot, f, cmd, name, args, nargs, final);
}

static tval eval_command(tstate *s, tval dot, const tnode *cmd, tval final) {
	const tnode *first = cmd->nodes[0];
	const tnode *const *args = (const tnode *const *)cmd->nodes;
	switch (first->type) {
	case N_FIELD:
		return eval_field_node(s, dot, first, args, cmd->nn, final);
	case N_CHAIN:
		return eval_chain_node(s, dot, first, args, cmd->nn, final);
	case N_IDENTIFIER:
		return eval_function(s, dot, first, cmd, args, cmd->nn, final);
	case N_PIPE:
		not_a_function(s, args, cmd->nn, final);
		return eval_pipeline(s, dot, first);
	case N_VARIABLE:
		return eval_variable_node(s, dot, first, args, cmd->nn, final);
	}
	s->node = first;
	not_a_function(s, args, cmd->nn, final);
	switch (first->type) {
	case N_BOOL:
		return bool_val(first->b);
	case N_DOT:
		return dot;
	case N_NIL:
		serrorf(s, "nil is not a command");
	case N_NUMBER:
		return ideal_constant(s, first);
	case N_STRING:
		return str_val(first->text);
	}
	serrorf(s, "can't evaluate command %s", quoted(s, node_str(s, first)));
}

static tval eval_pipeline(tstate *s, tval dot, const tnode *pipe) {
	if (!pipe)
		return zero_val();
	s->node = pipe;
	tval value = missing_val();
	for (size_t i = 0; i < pipe->nn; i++) {
		value = eval_command(s, dot, pipe->nodes[i], value);
		if (value.iface)
			value = indirect_interface(value);
	}
	for (size_t i = 0; i < pipe->ndecl; i++) {
		if (pipe->is_assign)
			set_var(s, pipe->decl[i]->ident[0], value);
		else
			push(s, pipe->decl[i]->ident[0], value);
	}
	return value;
}

static bool assignable(const tval *v, int typ) {
	if (typ == AT_ANY)
		return true;
	return !v->iface && v->k == V_STRING && v->content == 0;
}

static tval validate_type(tstate *s, tval value, int typ) {
	if (!is_valid(&value)) {
		if (typ == AT_NONE)
			return value;
		if (typ == AT_VALUE)
			return zero_val();
		if (typ == AT_ANY) {
			tval v = zero_val();
			v.k = V_NIL;
			v.iface = 1;
			return v;
		}
		serrorf(s, "invalid value; expected %s", at_name(typ));
	}
	if (typ == AT_VALUE || typ == AT_NONE)
		return value;
	if (!assignable(&value, typ)) {
		if (value.iface && value.k != V_NIL) {
			value.iface = 0;
			if (assignable(&value, typ))
				return value;
		}
		serrorf(s, "wrong type for value; expected %s; got %s", at_name(typ), tval_static_type_name(&value));
	}
	return value;
}

static tval eval_empty_interface(tstate *s, tval dot, const tnode *n) {
	s->node = n;
	switch (n->type) {
	case N_BOOL:
		return bool_val(n->b);
	case N_DOT:
		return dot;
	case N_FIELD:
		return eval_field_node(s, dot, n, 0, 0, missing_val());
	case N_IDENTIFIER:
		return eval_function(s, dot, n, n, 0, 0, missing_val());
	case N_NIL:
		serrorf(s, "evalEmptyInterface: nil (can't happen)");
	case N_NUMBER:
		return ideal_constant(s, n);
	case N_STRING:
		return str_val(n->text);
	case N_VARIABLE:
		return eval_variable_node(s, dot, n, 0, 0, missing_val());
	case N_PIPE:
		return eval_pipeline(s, dot, n);
	}
	serrorf(s, "can't handle assignment of %s to empty interface argument", node_str(s, n).p);
}

static tval eval_arg(tstate *s, tval dot, int typ, const tnode *n) {
	s->node = n;
	switch (n->type) {
	case N_DOT:
		return validate_type(s, dot, typ);
	case N_NIL:
		if (typ == AT_VALUE)
			return zero_val();
		if (typ == AT_ANY) {
			tval v = zero_val();
			v.k = V_NIL;
			v.iface = 1;
			return v;
		}
		serrorf(s, "cannot assign nil to %s", at_name(typ));
	case N_FIELD: {
		const tnode *one[1] = {n};
		return validate_type(s, eval_field_node(s, dot, n, one, 1, missing_val()), typ);
	}
	case N_VARIABLE:
		return validate_type(s, eval_variable_node(s, dot, n, 0, 0, missing_val()), typ);
	case N_PIPE:
		return validate_type(s, eval_pipeline(s, dot, n), typ);
	case N_IDENTIFIER:
		return validate_type(s, eval_function(s, dot, n, n, 0, 0, missing_val()), typ);
	case N_CHAIN:
		return validate_type(s, eval_chain_node(s, dot, n, 0, 0, missing_val()), typ);
	}
	switch (typ) {
	case AT_STRING:
		s->node = n;
		if (n->type == N_STRING)
			return str_val(n->text);
		serrorf(s, "expected string; found %s", node_str(s, n).p);
	case AT_ANY:
	case AT_VALUE:
		return eval_empty_interface(s, dot, n);
	}
	serrorf(s, "can't handle %s for arg of type %s", node_str(s, n).p, "<nil>");
}

/* ---- the builtins ---- */

typedef struct {
	alt_arena *a;
	alt_buf *err;
} bctx;

static int berr(bctx *c, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static int berr(bctx *c, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(c->err, fmt, ap);
	va_end(ap);
	return -1;
}

static int index_arg(bctx *c, tval index, size_t cap, size_t *out) {
	long long x;
	if (index.iface) /* slice does not look into interfaces, as index does */
		return berr(c, "cannot index slice/array with type interface {}");
	switch (index.k) {
	case V_INT:
	case V_UINT8:
		x = index.i;
		break;
	case V_INVALID:
		return berr(c, "cannot index slice/array with nil");
	default:
		return berr(c, "cannot index slice/array with type %s", tval_type_name(&index));
	}
	if (x < 0 || (unsigned long long)x > cap)
		return berr(c, "index out of range: %lld", x);
	*out = (size_t)x;
	return 0;
}

static int b_index(bctx *c, const tval *argv, size_t n, tval *out) {
	tval item = indirect_interface(argv[0]);
	if (!is_valid(&item))
		return berr(c, "index of untyped nil");
	for (size_t i = 1; i < n; i++) {
		tval index = indirect_interface(argv[i]);
		bool is_nil;
		item = indirect(item, &is_nil);
		if (is_nil)
			return berr(c, "index of nil pointer");
		switch (item.k) {
		case V_SLICE:
		case V_STRING: {
			size_t len = item.k == V_SLICE ? item.len : item.s.n, x;
			if (index_arg(c, index, len, &x) < 0)
				return -1;
			if (x == len)
				return berr(c, item.k == V_SLICE ? "reflect: slice index out of range" : "reflect: string index out of range");
			if (item.k == V_SLICE)
				item = tslice_at(&item, x);
			else {
				tval v = zero_val();
				v.k = V_UINT8;
				v.i = (unsigned char)item.s.p[x];
				item = v;
			}
			break;
		}
		case V_MAP: {
			if (!is_valid(&index))
				return berr(c, "value is nil; should be of type string");
			if (index.k != V_STRING || index.content)
				return berr(c, "value has type %s; should be string", tval_type_name(&index));
			alt_json *r = alt_json_get_str(item.j, index.s);
			item = tjson_val(r, true);
			break;
		}
		default:
			return berr(c, "can't index item of type %s", tval_type_name(&item));
		}
	}
	*out = item;
	return 0;
}

static int b_slice(bctx *c, const tval *argv, size_t n, tval *out) {
	tval item = indirect_interface(argv[0]);
	if (!is_valid(&item))
		return berr(c, "slice of untyped nil");
	bool is_nil;
	item = indirect(item, &is_nil);
	if (is_nil)
		return berr(c, "slice of nil pointer");
	size_t nidx = n - 1;
	if (nidx > 3)
		return berr(c, "too many slice indexes: %zu", nidx);
	size_t cap, len;
	switch (item.k) {
	case V_STRING:
		if (nidx == 3)
			return berr(c, "cannot 3-index slice a string");
		cap = len = item.s.n;
		break;
	case V_SLICE:
		cap = item.cap;
		len = item.len;
		break;
	default:
		return berr(c, "can't slice item of type %s", tval_type_name(&item));
	}
	size_t idx[3] = {0, len, 0};
	for (size_t i = 0; i < nidx; i++)
		if (index_arg(c, argv[1 + i], cap, &idx[i]) < 0)
			return -1;
	if (idx[0] > idx[1])
		return berr(c, "invalid slice index: %zu > %zu", idx[0], idx[1]);
	if (nidx < 3) {
		if (item.k == V_STRING)
			item.s = alt_str_slice(item.s, idx[0], idx[1]);
		else {
			item.off += idx[0];
			item.len = idx[1] - idx[0];
			item.cap -= idx[0];
		}
		*out = item;
		return 0;
	}
	if (idx[1] > idx[2])
		return berr(c, "invalid slice index: %zu > %zu", idx[1], idx[2]);
	item.off += idx[0];
	item.len = idx[1] - idx[0];
	item.cap = idx[2] - idx[0];
	*out = item;
	return 0;
}

static int b_len(bctx *c, tval item, tval *out) {
	bool is_nil;
	item = indirect(item, &is_nil);
	if (is_nil)
		return berr(c, "len of nil pointer");
	switch (item.k) {
	case V_MAP:
		*out = int_val((long long)map_len(c->a, item.j));
		return 0;
	case V_SLICE:
		*out = int_val((long long)item.len);
		return 0;
	case V_STRING:
		*out = int_val((long long)item.s.n);
		return 0;
	case V_INVALID:
		return berr(c, "reflect: call of reflect.Value.Type on zero Value");
	}
	return berr(c, "len of type %s", tval_type_name(&item));
}

enum { K_INVALID, K_BOOL, K_COMPLEX, K_INT, K_FLOAT, K_STRING, K_UINT };

static int basic_kind(const tval *v) {
	switch (v->k) {
	case V_BOOL: return K_BOOL;
	case V_INT: return K_INT;
	case V_UINT8: return K_UINT;
	case V_FLOAT: return K_FLOAT;
	case V_COMPLEX: return K_COMPLEX;
	case V_STRING: return K_STRING;
	}
	return K_INVALID;
}

static int b_eq(bctx *c, const tval *argv, size_t n, bool *out) {
	tval a1 = indirect_interface(argv[0]);
	if (n < 2)
		return berr(c, "missing argument for comparison");
	int k1 = basic_kind(&a1);
	for (size_t i = 1; i < n; i++) {
		tval arg = indirect_interface(argv[i]);
		int k2 = basic_kind(&arg);
		bool t = false;
		if (k1 != k2) {
			if (k1 == K_INT && k2 == K_UINT)
				t = a1.i >= 0 && a1.i == arg.i;
			else if (k1 == K_UINT && k2 == K_INT)
				t = arg.i >= 0 && a1.i == arg.i;
			else if (is_valid(&a1) && is_valid(&arg))
				return berr(c, "incompatible types for comparison: %s and %s", tval_type_name(&a1), tval_type_name(&arg));
		} else {
			switch (k1) {
			case K_BOOL: t = a1.b == arg.b; break;
			case K_COMPLEX: t = a1.f == arg.f && a1.im == arg.im; break;
			case K_FLOAT: t = a1.f == arg.f; break;
			case K_INT:
			case K_UINT: t = a1.i == arg.i; break;
			case K_STRING: t = alt_str_eq(a1.s, arg.s); break;
			default:
				if (a1.k != arg.k && is_valid(&a1) && is_valid(&arg))
					return berr(c, "non-comparable types %s: %s, %s: %s", sprint_verb(c->a, &a1, "%s"), tval_type_name(&a1),
						tval_type_name(&arg), sprint_verb(c->a, &arg, "%v"));
				if (!is_valid(&a1) || !is_valid(&arg))
					t = is_valid(&a1) == is_valid(&arg);
				else
					return berr(c, "non-comparable type %s: %s", sprint_verb(c->a, &arg, "%s"), tval_type_name(&arg));
			}
		}
		if (t) {
			*out = true;
			return 0;
		}
	}
	*out = false;
	return 0;
}

static int b_lt(bctx *c, tval a1, tval a2, bool *out) {
	a1 = indirect_interface(a1);
	int k1 = basic_kind(&a1);
	if (k1 == K_INVALID)
		return berr(c, "invalid type for comparison");
	a2 = indirect_interface(a2);
	int k2 = basic_kind(&a2);
	if (k2 == K_INVALID)
		return berr(c, "invalid type for comparison");
	bool t = false;
	if (k1 != k2) {
		if (k1 == K_INT && k2 == K_UINT)
			t = a1.i < 0 || a1.i < a2.i;
		else if (k1 == K_UINT && k2 == K_INT)
			t = a2.i >= 0 && a1.i < a2.i;
		else
			return berr(c, "incompatible types for comparison: %s and %s", tval_type_name(&a1), tval_type_name(&a2));
	} else {
		switch (k1) {
		case K_BOOL:
		case K_COMPLEX:
			return berr(c, "invalid type for comparison");
		case K_FLOAT: t = a1.f < a2.f; break;
		case K_INT:
		case K_UINT: t = a1.i < a2.i; break;
		case K_STRING: t = alt_str_cmp(a1.s, a2.s) < 0; break;
		}
	}
	*out = t;
	return 0;
}

static int b_compare(bctx *c, int kind, const tval *argv, size_t n, bool *out) {
	bool lt, eq;
	switch (kind) {
	case F_EQ:
		return b_eq(c, argv, n, out);
	case F_NE:
		if (b_eq(c, argv, n, &eq) < 0)
			return -1;
		*out = !eq;
		return 0;
	case F_LT:
		return b_lt(c, argv[0], argv[1], out);
	case F_LE:
	case F_GT:
		if (b_lt(c, argv[0], argv[1], &lt) < 0)
			return -1;
		if (lt) {
			*out = kind == F_LE;
			return 0;
		}
		if (b_eq(c, argv, n, &eq) < 0)
			return -1;
		*out = kind == F_LE ? eq : !eq;
		return 0;
	case F_GE:
		if (b_lt(c, argv[0], argv[1], &lt) < 0)
			return -1;
		*out = !lt;
		return 0;
	}
	return -1;
}

/* the any arguments of fmt: the dynamic values */
static tval *dynamic_args(alt_arena *a, const tval *argv, size_t n) {
	tval *r = alt_arena_alloc(a, (n ? n : 1) * sizeof *r);
	for (size_t i = 0; i < n; i++) {
		r[i] = argv[i];
		r[i].iface = 0;
		if (r[i].k == V_NIL)
			r[i].k = V_INVALID;
	}
	return r;
}

static alt_str buf_to_arena(alt_arena *a, alt_buf *b) {
	alt_str r = tstr_dup(a, alt_buf_str(b));
	alt_buf_free(b);
	return r;
}

/* ---- functions of the user ---- */

static alt_json *to_json(alt_arena *a, const tval *v) {
	switch (v->k) {
	case V_BOOL:
		return alt_json_new_bool(a, v->b);
	case V_INT:
	case V_UINT8:
		return alt_json_new_int(a, v->i);
	case V_FLOAT:
		return alt_json_new_number(a, v->f);
	case V_STRING:
		return alt_json_new_string(a, v->s);
	case V_MAP:
		return (alt_json *)v->j;
	case V_SLICE: {
		if (v->off == 0 && v->len == v->j->n)
			return (alt_json *)v->j;
		alt_json *arr = alt_json_new_array(a);
		for (size_t i = 0; i < v->len; i++) {
			tval e = tslice_at(v, i);
			e = indirect_interface(e);
			alt_json_append(arr, to_json(a, &e));
		}
		return arr;
	}
	}
	return alt_json_new_null(a);
}

static int call_user(bctx *c, const tfunc *f, const tval *argv, size_t n, tval *out) {
	alt_json **jv = alt_arena_alloc(c->a, (n ? n : 1) * sizeof *jv);
	for (size_t i = 0; i < n; i++) {
		tval v = indirect_interface(argv[i]);
		jv[i] = to_json(c->a, &v);
	}
	alt_template_call call = {jv, n, c->a, f->ctx, 0, 0, c->err};
	size_t before = c->err->n;
	int rc = f->fn(&call);
	if (rc < 0) {
		if (c->err->n == before)
			alt_buf_printf(c->err, "%s", alt_strerror(rc));
		return -1;
	}
	*out = tjson_val(call.result, true);
	if (out->k == V_STRING && call.content > 0 && call.content <= ALT_HTML_SRCSET)
		out->content = (uint8_t)call.content;
	return 0;
}

/* ---- calls ---- */

static fsig func_sig(const tfunc *f) {
	fsig g;
	memset(&g, 0, sizeof g);
	switch (f->kind) {
	case F_AND:
	case F_OR:
	case F_CALL:
	case F_INDEX:
	case F_SLICE:
	case F_EQ:
		g = (fsig){2, true, {AT_VALUE, AT_VALUE}};
		break;
	case F_HTML:
	case F_JS:
	case F_URLQUERY:
	case F_PRINT:
	case F_PRINTLN:
	case F_ESCAPER:
		g = (fsig){1, true, {AT_ANY}};
		break;
	case F_PRINTF:
		g = (fsig){2, true, {AT_STRING, AT_ANY}};
		break;
	case F_LEN:
	case F_NOT:
		g = (fsig){1, false, {AT_VALUE}};
		break;
	case F_NE:
	case F_GE:
	case F_GT:
	case F_LE:
	case F_LT:
		g = (fsig){2, false, {AT_VALUE, AT_VALUE}};
		break;
	case F_USER:
		if (f->nargs >= 0) {
			g.num_in = f->nargs;
			g.variadic = false;
		} else {
			g.num_in = -f->nargs - 1 + 1;
			g.variadic = true;
		}
		for (size_t i = 0; i < 8; i++)
			g.in[i] = AT_ANY;
		break;
	}
	return g;
}

static int sig_in(const fsig *g, int i) { return g->in[i < 8 ? i : 7]; }

static tval eval_call(tstate *s, tval dot, const tfunc *fn, const tnode *node, alt_str name, const tnode *const *args, size_t nargs,
	tval final) {
	if (args) {
		args++;
		nargs--;
	}
	fsig g = func_sig(fn);
	size_t num_in = nargs + (is_missing(&final) ? 0 : 1);
	size_t num_fixed = nargs;
	if (g.variadic) {
		num_fixed = (size_t)g.num_in - 1;
		if (num_in < num_fixed)
			serrorf(s, "wrong number of args for %.*s: want at least %d got %zu", (int)name.n, name.p, g.num_in - 1, nargs);
	} else if (num_in != (size_t)g.num_in)
		serrorf(s, "wrong number of args for %.*s: want %d got %zu", (int)name.n, name.p, g.num_in, num_in);
	alt_arena *a = &s->x->arena;
	if (fn->kind == F_AND || fn->kind == F_OR) {
		tval v = zero_val();
		for (size_t i = 0; i < nargs; i++) {
			v = eval_arg(s, dot, AT_VALUE, args[i]);
			if (truth(a, v) == (fn->kind == F_OR))
				return v;
		}
		if (!is_missing(&final))
			v = validate_type(s, final, AT_VALUE);
		return v;
	}
	tval *argv = alt_arena_alloc(a, (num_in ? num_in : 1) * sizeof *argv);
	size_t i = 0;
	for (; i < num_fixed && i < nargs; i++)
		argv[i] = eval_arg(s, dot, sig_in(&g, (int)i), args[i]);
	if (g.variadic) {
		int t = g.in[g.num_in - 1];
		for (; i < nargs; i++)
			argv[i] = eval_arg(s, dot, t, args[i]);
	}
	if (!is_missing(&final)) {
		int t = sig_in(&g, g.num_in - 1);
		if (g.variadic && num_in - 1 < num_fixed)
			t = sig_in(&g, (int)num_in - 1);
		argv[i] = validate_type(s, final, t);
	}
	alt_buf err = ALT_BUF_INIT;
	bctx c = {a, &err};
	tval out = zero_val();
	int rc = 0;
	bool bv;
	switch (fn->kind) {
	case F_CALL: {
		alt_str callee;
		if (nargs == 0)
			callee = final.k == V_STRING ? final.s : alt_str_cstr(alt_arena_printf(a, "<%s Value>", tval_static_type_name(&final)));
		else
			callee = node_str(s, args[0]);
		tval f0 = indirect_interface(argv[0]);
		if (!is_valid(&f0))
			rc = berr(&c, "call of nil");
		else
			rc = berr(&c, "non-function %.*s of type %s", (int)callee.n, callee.p, tval_type_name(&f0));
		break;
	}
	case F_INDEX:
		rc = b_index(&c, argv, num_in, &out);
		break;
	case F_SLICE:
		rc = b_slice(&c, argv, num_in, &out);
		break;
	case F_LEN:
		rc = b_len(&c, argv[0], &out);
		break;
	case F_NOT:
		out = bool_val(!truth(a, argv[0]));
		break;
	case F_EQ:
	case F_NE:
	case F_LT:
	case F_LE:
	case F_GT:
	case F_GE:
		rc = b_compare(&c, fn->kind, argv, num_in, &bv);
		if (rc == 0)
			out = bool_val(bv);
		break;
	case F_PRINT:
	case F_PRINTLN:
	case F_PRINTF: {
		alt_buf b = ALT_BUF_INIT;
		if (fn->kind == F_PRINT)
			tfmt_sprint(&b, dynamic_args(a, argv, num_in), num_in);
		else if (fn->kind == F_PRINTLN)
			tfmt_sprintln(&b, dynamic_args(a, argv, num_in), num_in);
		else
			tfmt_sprintf(&b, argv[0].s, dynamic_args(a, argv + 1, num_in - 1), num_in - 1);
		out = str_val(buf_to_arena(a, &b));
		break;
	}
	case F_HTML:
	case F_JS:
	case F_URLQUERY: {
		alt_str str = teval_args(a, argv, num_in);
		alt_buf b = ALT_BUF_INIT;
		if (fn->kind == F_HTML)
			thtml_escape(&b, str);
		else if (fn->kind == F_JS)
			tjs_escape(&b, str);
		else
			turl_query_escape(&b, str);
		out = str_val(buf_to_arena(a, &b));
		break;
	}
	case F_ESCAPER:
		out = str_val(thtml_run_escaper(a, fn->esc, argv, num_in));
		break;
	case F_USER:
		rc = call_user(&c, fn, argv, num_in, &out);
		break;
	}
	if (rc < 0) {
		s->node = node;
		alt_str m = buf_to_arena(a, &err);
		serrorf(s, "error calling %.*s: %.*s", (int)name.n, name.p, (int)m.n, m.p);
	}
	alt_buf_free(&err);
	return out;
}

/* ---- walking ---- */

static void print_value(tstate *s, const tnode *n, tval v) {
	s->node = n;
	if (!is_valid(&v) || v.k == V_MISSING) {
		alt_buf_append_cstr(s->x->wr, "<no value>");
		return;
	}
	tval x = v;
	x.iface = 0;
	if (x.k == V_NIL)
		x.k = V_INVALID;
	tfmt_sprint(s->x->wr, &x, 1);
}

static int walk_if_or_with(tstate *s, int type, tval dot, const tnode *n) {
	size_t mark = s->nvars;
	tval val = eval_pipeline(s, dot, n->node);
	bool t = truth(&s->x->arena, val);
	int rc = W_NONE;
	if (t)
		rc = walk(s, type == N_WITH ? val : dot, n->list);
	else if (n->else_list)
		rc = walk(s, dot, n->else_list);
	s->nvars = mark;
	return rc;
}

/* one iteration: W_BREAK ends the loop */
static int one_iteration(tstate *s, const tnode *r, size_t mark, tval index, tval elem) {
	const tnode *pipe = r->node;
	if (pipe->ndecl > 0) {
		if (pipe->is_assign) {
			if (pipe->ndecl > 1)
				set_var(s, pipe->decl[0]->ident[0], index);
			else
				set_var(s, pipe->decl[0]->ident[0], elem);
		} else
			s->vars[s->nvars - 1].value = elem;
	}
	if (pipe->ndecl > 1) {
		if (pipe->is_assign)
			set_var(s, pipe->decl[1]->ident[0], elem);
		else
			s->vars[s->nvars - 2].value = index;
	}
	int rc = walk(s, elem, r->list);
	s->nvars = mark;
	return rc == W_BREAK ? W_BREAK : W_NONE;
}

static void walk_range(tstate *s, tval dot, const tnode *r) {
	s->node = r;
	size_t outer = s->nvars;
	bool is_nil;
	tval val = indirect(eval_pipeline(s, dot, r->node), &is_nil);
	size_t mark = s->nvars;
	alt_arena *a = &s->x->arena;
	bool ran = false;
	switch (val.k) {
	case V_INT:
	case V_UINT8:
		if (r->node->ndecl > 1)
			serrorf(s, "can't use %s to iterate over more than one variable", sprint_verb(a, &val, "%v"));
		for (long long i = 0; i < val.i; i++) {
			ran = true;
			tval e = val.k == V_INT ? int_val(i) : val;
			e.i = i;
			if (one_iteration(s, r, mark, zero_val(), e) == W_BREAK)
				break;
		}
		break;
	case V_SLICE:
		for (size_t i = 0; i < val.len; i++) {
			ran = true;
			if (one_iteration(s, r, mark, int_val((long long)i), tslice_at(&val, i)) == W_BREAK)
				break;
		}
		break;
	case V_MAP: {
		const alt_json_member **ms;
		size_t n = tmap_sorted(a, val.j, &ms);
		for (size_t i = 0; i < n; i++) {
			ran = true;
			if (one_iteration(s, r, mark, str_val(ms[i]->key), tjson_val(ms[i]->value, true)) == W_BREAK)
				break;
		}
		break;
	}
	case V_INVALID:
		break;
	default:
		serrorf(s, "range can't iterate over %s", sprint_verb(a, &val, "%v"));
	}
	if (!ran && r->else_list)
		walk(s, dot, r->else_list);
	s->nvars = outer;
}

static void walk_template(tstate *s, tval dot, const tnode *t) {
	s->node = t;
	alt_template *tmpl = ttemplate_lookup(s->tmpl, t->text);
	if (!tmpl || !tmpl->tree)
		serrorf(s, "template %s not defined", quoted(s, t->text));
	if (s->depth == MAX_EXEC_DEPTH)
		serrorf(s, "exceeded maximum template depth (%d)", MAX_EXEC_DEPTH);
	dot = eval_pipeline(s, dot, t->node);
	tstate ns = *s;
	ns.depth++;
	ns.tmpl = tmpl;
	ns.vars = 0;
	ns.nvars = ns.vcap = 0;
	push(&ns, ALT_STR("$"), dot);
	walk(&ns, dot, tmpl->tree->root);
}

static int walk(tstate *s, tval dot, const tnode *node) {
	s->node = node;
	switch (node->type) {
	case N_ACTION: {
		tval val = eval_pipeline(s, dot, node->node);
		if (node->node->ndecl == 0)
			print_value(s, node, val);
		return W_NONE;
	}
	case N_BREAK:
		return W_BREAK;
	case N_CONTINUE:
		return W_CONTINUE;
	case N_COMMENT:
		return W_NONE;
	case N_IF:
	case N_WITH:
		return walk_if_or_with(s, node->type, dot, node);
	case N_LIST:
		for (size_t i = 0; i < node->nn; i++) {
			int rc = walk(s, dot, node->nodes[i]);
			if (rc)
				return rc;
		}
		return W_NONE;
	case N_RANGE:
		walk_range(s, dot, node);
		return W_NONE;
	case N_TEMPLATE:
		walk_template(s, dot, node);
		return W_NONE;
	case N_TEXT:
		alt_buf_append_str(s->x->wr, node->text);
		return W_NONE;
	}
	serrorf(s, "unknown node: %s", node_str(s, node).p);
}

int texec(alt_template *t, alt_buf *out, const alt_json *data, alt_buf *err) {
	texec_ctx x;
	x.err = err;
	x.wr = out;
	alt_arena_init(&x.arena, 0);
	tstate s;
	memset(&s, 0, sizeof s);
	s.tmpl = t;
	s.x = &x;
	tval value = tjson_val(data, false);
	if (setjmp(x.jb)) {
		alt_arena_free(&x.arena);
		return -EINVAL;
	}
	push(&s, ALT_STR("$"), value);
	if (!t->tree || !t->tree->root)
		serrorf(&s, "%s is an incomplete or empty template", quoted(&s, t->name));
	walk(&s, value, t->tree->root);
	alt_arena_free(&x.arena);
	return 0;
}

bool alt_template_is_true(const alt_json *v) {
	alt_arena a;
	alt_arena_init(&a, 0);
	tval x = tjson_val(v, false);
	bool t = is_true(&a, &x);
	alt_arena_free(&a);
	return t;
}
