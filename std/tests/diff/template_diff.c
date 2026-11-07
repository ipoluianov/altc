/* text <opts> <texts> <data> [name] -> the result of text/template
   html <opts> <texts> <data> [name] -> the same with html/template
   opts: ";"-separated settings (mk=error, ld=[[, rd=]]); texts: the
   texts to parse one after the other, separated by NUL bytes; data: JSON
   ("-" for nil); name: the template to execute (the first by default).
   All fields but the kind are in hex. The answer is "P:" and the parse
   error, or "O:" and the output followed by " E:" and the error if
   execution failed, with \ and newlines escaped. */
#include "common.h"
#include <alt/encoding/json.h>
#include <alt/html/template.h>
#include <alt/text/template.h>
#include <errno.h>
#include <stdint.h>

static void put_esc(alt_str s) {
	for (size_t i = 0; i < s.n; i++) {
		char c = s.p[i];
		if (c == '\\')
			fputs("\\\\", stdout);
		else if (c == '\n')
			fputs("\\n", stdout);
		else if (c == '\r')
			fputs("\\r", stdout);
		else
			putchar(c);
	}
}

static double num(const alt_json *j, bool *ok) {
	if (j->type != ALT_JSON_NUMBER) {
		*ok = false;
		return 0;
	}
	return j->num;
}

static int f_add(alt_template_call *c) {
	bool ok = true;
	double s = num(c->argv[0], &ok) + num(c->argv[1], &ok);
	if (!ok) {
		alt_buf_append_cstr(c->err, "add: bad arg");
		return -EINVAL;
	}
	c->result = alt_json_new_number(c->arena, s);
	return 0;
}

static int f_echo(alt_template_call *c) {
	c->result = c->argv[0];
	return 0;
}

static int f_first(alt_template_call *c) {
	c->result = c->argv[0];
	return 0;
}

static int f_fail(alt_template_call *c) {
	alt_buf_append_cstr(c->err, "boom");
	return -EINVAL;
}

static int f_nilf(alt_template_call *c) {
	c->result = 0;
	return 0;
}

static int f_join(alt_template_call *c) {
	alt_buf b = ALT_BUF_INIT;
	for (size_t i = 0; i < c->argc; i++) {
		if (c->argv[i]->type != ALT_JSON_STRING) {
			alt_buf_free(&b);
			alt_buf_append_cstr(c->err, "join: not a string");
			return -EINVAL;
		}
		if (i)
			alt_buf_append_byte(&b, ',');
		alt_buf_append_str(&b, c->argv[i]->str);
	}
	c->result = alt_json_new_string(c->arena, alt_buf_str(&b));
	alt_buf_free(&b);
	return 0;
}

static int f_list(alt_template_call *c) {
	alt_json *a = alt_json_new_array(c->arena);
	for (size_t i = 0; i < c->argc; i++)
		alt_json_append(a, c->argv[i]);
	c->result = a;
	return 0;
}

static int f_dict(alt_template_call *c) {
	alt_json *o = alt_json_new_object(c->arena);
	for (size_t i = 0; i + 1 < c->argc; i += 2) {
		if (c->argv[i]->type != ALT_JSON_STRING) {
			alt_buf_append_cstr(c->err, "dict: key not a string");
			return -EINVAL;
		}
		char *k = alt_arena_printf(c->arena, "%.*s", (int)c->argv[i]->str.n, c->argv[i]->str.p);
		alt_json_set(o, k, c->argv[i + 1]);
	}
	c->result = o;
	return 0;
}

static int f_safe(alt_template_call *c) {
	if (c->argv[0]->type != ALT_JSON_STRING) {
		alt_buf_append_cstr(c->err, "safe: not a string");
		return -EINVAL;
	}
	c->result = c->argv[0];
	c->content = (int)(intptr_t)c->ctx;
	return 0;
}

static const alt_template_func funcs[] = {
	{"add", f_add, 0, 2},
	{"echo", f_echo, 0, 1},
	{"first", f_first, 0, ALT_TEMPLATE_VARIADIC(1)},
	{"fail", f_fail, 0, ALT_TEMPLATE_VARIADIC(0)},
	{"nilf", f_nilf, 0, 0},
	{"join", f_join, 0, ALT_TEMPLATE_VARIADIC(0)},
	{"list", f_list, 0, ALT_TEMPLATE_VARIADIC(0)},
	{"dict", f_dict, 0, ALT_TEMPLATE_VARIADIC(0)},
	{"safeCSS", f_safe, (void *)ALT_HTML_CSS, 1},
	{"safeHTML", f_safe, (void *)ALT_HTML_HTML, 1},
	{"safeAttr", f_safe, (void *)ALT_HTML_HTML_ATTR, 1},
	{"safeJS", f_safe, (void *)ALT_HTML_JS, 1},
	{"safeJSStr", f_safe, (void *)ALT_HTML_JS_STR, 1},
	{"safeURL", f_safe, (void *)ALT_HTML_URL, 1},
	{"safeSrcset", f_safe, (void *)ALT_HTML_SRCSET, 1},
};

int main(void) {
	char *line, *f[8];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 8);
		bool html = !strcmp(f[0], "html");
		alt_str opts = alt_str_make(f[1], diff_unhex(f[1]));
		alt_str texts = alt_str_make(f[2], diff_unhex(f[2]));
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_json *data = 0;
		if (strcmp(f[3], "-")) {
			alt_str d = alt_str_make(f[3], diff_unhex(f[3]));
			if (alt_json_parse(&a, d, &data, 0) < 0)
				data = 0;
		}
		alt_template *t = html ? alt_html_template_new(ALT_STR("t")) : alt_template_new(ALT_STR("t"));
		alt_buf err = ALT_BUF_INIT, out = ALT_BUF_INIT;
		alt_template_funcs(t, funcs, sizeof funcs / sizeof funcs[0], 0);
		alt_str ld = ALT_STR_NULL, rd = ALT_STR_NULL, opt;
		while (opts.n && alt_str_split_next(&opts, ALT_STR(";"), &opt)) {
			if (alt_str_has_prefix(opt, ALT_STR("mk=")))
				alt_template_option(t, alt_str_cstr(alt_arena_printf(&a, "missingkey=%.*s", (int)opt.n - 3, opt.p + 3)), 0);
			else if (alt_str_has_prefix(opt, ALT_STR("ld=")))
				ld = alt_str_from(opt, 3);
			else if (alt_str_has_prefix(opt, ALT_STR("rd=")))
				rd = alt_str_from(opt, 3);
		}
		if (ld.n || rd.n)
			alt_template_delims(t, ld, rd);
		alt_str text;
		int rc = 0;
		while (alt_str_split_next(&texts, alt_str_make("", 1), &text)) {
			rc = alt_template_parse(t, text, &err);
			if (rc < 0)
				break;
		}
		if (rc < 0) {
			fputs("P:", stdout);
			put_esc(alt_buf_str(&err));
		} else {
			if (n > 4 && f[4][0]) {
				alt_str name = alt_str_make(f[4], diff_unhex(f[4]));
				rc = alt_template_execute_name(t, &out, name, data, &err);
			} else
				rc = alt_template_execute(t, &out, data, &err);
			fputs("O:", stdout);
			put_esc(alt_buf_str(&out));
			if (rc < 0) {
				fputs(" E:", stdout);
				put_esc(alt_buf_str(&err));
			}
		}
		putchar('\n');
		alt_template_free(t);
		alt_buf_free(&err);
		alt_buf_free(&out);
		alt_arena_free(&a);
		fflush(stdout);
	}
	return 0;
}
