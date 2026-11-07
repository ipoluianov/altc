/* Sets of associated templates, after Go's text/template/template.go
   and option.go (BSD-3-Clause, see LICENSE-Go). */
#include "template_internal.h"
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static tcommon *common_new(void) {
	tcommon *c = calloc(1, sizeof *c);
	if (!c)
		return 0;
	alt_arena_init(&c->arena, 0);
	c->mu = (alt_mutex)ALT_MUTEX_INIT;
	return c;
}

alt_template *ttemplate_new_in(tcommon *c, alt_str name, alt_str ldelim, alt_str rdelim) {
	alt_template *t = alt_arena_zalloc(&c->arena, sizeof *t);
	if (!t)
		return 0;
	t->name = tstr_dup(&c->arena, name);
	t->common = c;
	t->ldelim = ldelim;
	t->rdelim = rdelim;
	t->next_all = c->all;
	c->all = t;
	return t;
}

hentry *thtml_entry(tcommon *c, alt_str name, bool add) {
	for (size_t i = 0; i < c->nh; i++)
		if (alt_str_eq(c->hset[i].name, name))
			return &c->hset[i];
	if (!add)
		return 0;
	if (c->nh == c->ch) {
		size_t nc = c->ch ? 2 * c->ch : 8;
		hentry *h = realloc(c->hset, nc * sizeof *h);
		if (!h)
			return 0;
		c->hset = h;
		c->ch = nc;
	}
	hentry *h = &c->hset[c->nh++];
	memset(h, 0, sizeof *h);
	h->name = tstr_dup(&c->arena, name);
	return h;
}

alt_template *alt_template_new(alt_str name) {
	tcommon *c = common_new();
	if (!c)
		return 0;
	return ttemplate_new_in(c, name, ALT_STR_NULL, ALT_STR_NULL);
}

alt_template *alt_html_template_new(alt_str name) {
	tcommon *c = common_new();
	if (!c)
		return 0;
	c->html = true;
	thtml_entry(c, name, true);
	return ttemplate_new_in(c, name, ALT_STR_NULL, ALT_STR_NULL);
}

alt_template *alt_template_new_assoc(alt_template *t, alt_str name) {
	tcommon *c = t->common;
	if (c->html) {
		/* Go's html new: an existing entry becomes an empty template */
		hentry *h = thtml_entry(c, name, true);
		h->state = 0;
		alt_buf_free(&h->err);
	}
	return ttemplate_new_in(c, name, t->ldelim, t->rdelim);
}

void alt_template_free(alt_template *t) {
	if (!t)
		return;
	tcommon *c = t->common;
	for (size_t i = 0; i < c->nh; i++)
		alt_buf_free(&c->hset[i].err);
	free(c->hset);
	free(c->funcs.f);
	alt_arena_free(&c->arena);
	free(c);
}

alt_str alt_template_name(const alt_template *t) { return t->name; }

void alt_template_delims(alt_template *t, alt_str left, alt_str right) {
	t->ldelim = tstr_dup(&t->common->arena, left);
	t->rdelim = tstr_dup(&t->common->arena, right);
}

int alt_template_option(alt_template *t, alt_str opt, alt_buf *err) {
	if (!opt.n) {
		if (err)
			alt_buf_append_cstr(err, "empty option string");
		return -EINVAL;
	}
	alt_str key, value;
	if (alt_str_cut_byte(opt, '=', &key, &value) && alt_str_eq_cstr(key, "missingkey")) {
		if (alt_str_eq_cstr(value, "invalid") || alt_str_eq_cstr(value, "default")) {
			t->common->missingkey = 0;
			return 0;
		}
		if (alt_str_eq_cstr(value, "zero")) {
			t->common->missingkey = 1;
			return 0;
		}
		if (alt_str_eq_cstr(value, "error")) {
			t->common->missingkey = 2;
			return 0;
		}
	}
	if (err)
		alt_buf_printf(err, "unrecognized option: %.*s", (int)opt.n, opt.p);
	return -EINVAL;
}

static bool good_name(alt_str name) {
	if (!name.n)
		return false;
	for (size_t i = 0; i < name.n;) {
		int32_t r;
		size_t w = alt_utf8_decode(name.p + i, name.n - i, &r);
		if (r != '_') {
			if (i == 0 && !alt_unicode_is_letter(r))
				return false;
			if (!alt_unicode_is_letter(r) && !alt_unicode_is_digit(r))
				return false;
		}
		i += w;
	}
	return true;
}

int alt_template_funcs(alt_template *t, const alt_template_func *funcs, size_t n, alt_buf *err) {
	tcommon *c = t->common;
	for (size_t i = 0; i < n; i++) {
		alt_str name = alt_str_cstr(funcs[i].name);
		if (!good_name(name)) {
			if (err) {
				alt_buf_append_cstr(err, "function name ");
				tquote(err, name);
				alt_buf_append_cstr(err, " is not a valid identifier");
			}
			return -EINVAL;
		}
		if (!funcs[i].fn) {
			if (err)
				alt_buf_printf(err, "value for %s not a function", funcs[i].name);
			return -EINVAL;
		}
	}
	for (size_t i = 0; i < n; i++) {
		if (c->funcs.n == c->funcs.cap) {
			size_t nc = c->funcs.cap ? 2 * c->funcs.cap : 8;
			tfunc *f = realloc(c->funcs.f, nc * sizeof *f);
			if (!f)
				return -ENOMEM;
			c->funcs.f = f;
			c->funcs.cap = nc;
		}
		tfunc *f = &c->funcs.f[c->funcs.n++];
		memset(f, 0, sizeof *f);
		f->name = tstr_dup(&c->arena, alt_str_cstr(funcs[i].name));
		f->kind = F_USER;
		f->fn = funcs[i].fn;
		f->ctx = funcs[i].ctx;
		f->nargs = funcs[i].nargs;
	}
	return 0;
}

alt_template *ttemplate_lookup(const alt_template *t, alt_str name) {
	tcommon *c = t->common;
	for (size_t i = 0; i < c->n; i++)
		if (alt_str_eq(c->tmpl[i]->name, name))
			return c->tmpl[i];
	return 0;
}

alt_template *alt_template_lookup(const alt_template *t, alt_str name) { return ttemplate_lookup(t, name); }

/* associate: false when an empty tree would replace a defined one */
static bool associate(alt_template *t, alt_template *nt, ttree *tree) {
	tcommon *c = t->common;
	for (size_t i = 0; i < c->n; i++) {
		if (alt_str_eq(c->tmpl[i]->name, nt->name)) {
			if (tree_is_empty(tree->root) && c->tmpl[i]->tree)
				return false;
			c->tmpl[i] = nt;
			return true;
		}
	}
	if (c->n == c->cap) {
		size_t nc = c->cap ? 2 * c->cap : 8;
		c->tmpl = alt_arena_realloc(&c->arena, c->tmpl, c->cap * sizeof(alt_template *), nc * sizeof(alt_template *));
		c->cap = nc;
	}
	c->tmpl[c->n++] = nt;
	return true;
}

alt_template *ttemplate_add_tree(alt_template *t, alt_str name, ttree *tree) {
	alt_template *nt = t;
	if (!alt_str_eq(name, t->name))
		nt = alt_template_new_assoc(t, name);
	if (associate(t, nt, tree) || !nt->tree)
		nt->tree = tree;
	return nt;
}

int alt_template_parse(alt_template *t, alt_str text, alt_buf *err) {
	tcommon *c = t->common;
	alt_buf tmp = ALT_BUF_INIT;
	if (!err)
		err = &tmp;
	if (c->html && c->escaped) {
		alt_buf_printf(err, "html/template: cannot Parse after Execute");
		alt_buf_free(&tmp);
		return -EINVAL;
	}
	text = tstr_dup(&c->arena, text);
	ttreeset set = {0, 0, 0};
	int rc = tparse(&c->arena, t->name, text, t->ldelim, t->rdelim, &c->funcs, &set, err);
	alt_buf_free(&tmp);
	if (rc < 0)
		return rc;
	for (size_t i = 0; i < set.n; i++)
		ttemplate_add_tree(t, set.trees[i]->name, set.trees[i]);
	if (c->html)
		for (size_t i = 0; i < c->n; i++)
			thtml_entry(c, c->tmpl[i]->name, true);
	return 0;
}

static int name_cmp(const void *a, const void *b) {
	const alt_template *x = *(alt_template *const *)a, *y = *(alt_template *const *)b;
	return alt_str_cmp(x->name, y->name);
}

void alt_template_defined(const alt_template *t, alt_buf *b) {
	tcommon *c = t->common;
	if (!c->n)
		return;
	alt_template **v = malloc(c->n * sizeof *v);
	if (!v)
		return;
	memcpy(v, c->tmpl, c->n * sizeof *v);
	qsort(v, c->n, sizeof *v, name_cmp);
	bool first = true;
	for (size_t i = 0; i < c->n; i++) {
		if (!v[i]->tree || !v[i]->tree->root)
			continue;
		alt_buf_append_cstr(b, first ? "; defined templates are: " : ", ");
		first = false;
		tquote(b, v[i]->name);
	}
	free(v);
}

int alt_template_execute(alt_template *t, alt_buf *out, const alt_json *data, alt_buf *err) {
	alt_buf tmp = ALT_BUF_INIT;
	if (!err)
		err = &tmp;
	int rc = 0;
	if (t->common->html)
		rc = thtml_escape_template(t, err);
	if (rc == 0)
		rc = texec(t, out, data, err);
	alt_buf_free(&tmp);
	return rc;
}

int alt_template_execute_name(alt_template *t, alt_buf *out, alt_str name, const alt_json *data, alt_buf *err) {
	alt_buf tmp = ALT_BUF_INIT;
	if (!err)
		err = &tmp;
	if (t->common->html) {
		int rc = thtml_execute_name(t, out, name, data, err);
		alt_buf_free(&tmp);
		return rc;
	}
	alt_template *nt = ttemplate_lookup(t, name);
	int rc;
	if (!nt) {
		{
			alt_buf_append_cstr(err, "template: no template ");
			tquote(err, name);
			alt_buf_append_cstr(err, " associated with template ");
			tquote(err, t->name);
		}
		rc = -EINVAL;
	} else
		rc = alt_template_execute(nt, out, data, err);
	alt_buf_free(&tmp);
	return rc;
}
