/* text/template and html/template: the API, sets of templates,
   functions, options, errors, concurrent executions and the limits (also
   run under the sanitizers; tests/stdtemplate_test.go compares the
   output with Go's on many templates) */
#include "check.h"
#include <alt/encoding/json.h>
#include <alt/html/template.h>
#include <alt/text/template.h>
#include <pthread.h>

static alt_json *parse_json(alt_arena *a, const char *s) {
	alt_json *j = 0;
	CHECK_OK(alt_json_parse(a, alt_str_cstr(s), &j, 0));
	return j;
}

/* executes t on data; the output, or "ERR:" and the error */
static char *run(alt_template *t, const char *data) {
	static char res[4096];
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_buf out = ALT_BUF_INIT, err = ALT_BUF_INIT;
	if (alt_template_execute(t, &out, data ? parse_json(&a, data) : 0, &err) < 0)
		snprintf(res, sizeof res, "ERR:%s", alt_buf_cstr(&err));
	else
		snprintf(res, sizeof res, "%s", alt_buf_cstr(&out));
	alt_buf_free(&out);
	alt_buf_free(&err);
	alt_arena_free(&a);
	return res;
}

static char *run_text(const char *text, const char *data) {
	static char res[4096];
	alt_template *t = alt_template_new(ALT_STR("t"));
	alt_buf err = ALT_BUF_INIT;
	if (alt_template_parse(t, alt_str_cstr(text), &err) < 0)
		snprintf(res, sizeof res, "PARSE:%s", alt_buf_cstr(&err));
	else
		snprintf(res, sizeof res, "%s", run(t, data));
	alt_buf_free(&err);
	alt_template_free(t);
	return res;
}

static char *run_html(const char *text, const char *data) {
	static char res[4096];
	alt_template *t = alt_html_template_new(ALT_STR("t"));
	alt_buf err = ALT_BUF_INIT;
	if (alt_template_parse(t, alt_str_cstr(text), &err) < 0)
		snprintf(res, sizeof res, "PARSE:%s", alt_buf_cstr(&err));
	else
		snprintf(res, sizeof res, "%s", run(t, data));
	alt_buf_free(&err);
	alt_template_free(t);
	return res;
}

static void basics(void) {
	CHECK_STR(run_text("hello {{.name}}!", "{\"name\":\"world\"}"), "hello world!");
	CHECK_STR(run_text("{{range .}}{{.}},{{end}}", "[1, 2.5, 1e6, 1e21, \"x\", true, null]"), "1,2.5,1e+06,1e+21,x,true,<no value>,");
	CHECK_STR(run_text("{{range $k, $v := .}}{{$k}}={{$v}} {{end}}", "{\"b\":2,\"a\":1,\"c\":{\"d\":[]}}"), "a=1 b=2 c=map[d:[]] ");
	CHECK_STR(run_text("{{.missing}} {{.a.b}}", "{\"a\":{\"b\":\"deep\"}}"), "<no value> deep");
	CHECK_STR(run_text("{{printf \"%5.2f|%-4d|%q|%x|%v\" .f 7 \"q\" \"hi\" .m}}", "{\"f\":3.14159,\"m\":{\"k\":[1,\"s\"]}}"),
		" 3.14|7   |\"q\"|6869|map[k:[1 s]]");
	CHECK_STR(run_text("{{printf \"%d\" .s}} {{printf \"%d %d\" 1}} {{printf \"%d\" 1 2}}", "{\"s\":\"x\"}"),
		"%!d(string=x) 1 %!d(MISSING) 1%!(EXTRA int=2)");
	CHECK_STR(run_text("{{if and .a (not .b)}}yes{{else}}no{{end}} {{or .b .c \"d\"}} {{len .l}} {{index .l 1}} {{slice .l 1}}",
		"{\"a\":1,\"b\":0,\"c\":\"\",\"l\":[1,2,3]}"), "yes d 3 2 [2 3]");
	CHECK_STR(run_text("{{eq .a 1.0}} {{lt .a 2.0}} {{eq .s \"x\" \"y\"}} {{ne 1 2}}", "{\"a\":1,\"s\":\"y\"}"), "true true true true");
	CHECK_STR(run_text("{{html .h}} {{js .h}} {{urlquery .h}}", "{\"h\":\"<a href='x'>&\"}"),
		"&lt;a href=&#39;x&#39;&gt;&amp; \\u003Ca href\\u003D\\'x\\'\\u003E\\u0026 %3Ca+href%3D%27x%27%3E%26");
	CHECK_STR(run_text("{{range .}}{{if eq . 3.0}}{{break}}{{end}}{{if eq . 1.0}}{{continue}}{{end}}{{.}}{{end}}", "[1,2,3,4]"), "2");
	CHECK_STR(run_text("{{define \"item\"}}<{{.}}>{{end}}{{range .}}{{template \"item\" .}}{{end}}", "[\"a\",\"b\"]"), "<a><b>");
	CHECK_STR(run_text("a  {{- 1 -}}  b {{/* no */}}c", 0), "a1b c");
	CHECK_STR(run_text("{{.}}", 0), "<no value>");
	CHECK_STR(run_text("{{3}} {{1.5}} {{'a'}} {{0x10}} {{1e3}} {{1+2i}}", 0), "3 1.5 97 16 1000 (1+2i)");
	CHECK(alt_template_is_true(0) == false);
	alt_arena a;
	alt_arena_init(&a, 0);
	CHECK(alt_template_is_true(parse_json(&a, "[0]")));
	CHECK(!alt_template_is_true(parse_json(&a, "{}")));
	CHECK(!alt_template_is_true(parse_json(&a, "\"\"")));
	CHECK(alt_template_is_true(parse_json(&a, "-1")));
	alt_arena_free(&a);
}

static void errors(void) {
	CHECK_STR(run_text("{{if .x}}", 0), "PARSE:template: t:1: unexpected EOF");
	CHECK_STR(run_text("a\n{{.x y}}", 0), "PARSE:template: t:2: function \"y\" not defined");
	CHECK_STR(run_text("{{$x}}", 0), "PARSE:template: t:1: undefined variable \"$x\"");
	CHECK_STR(run_text("{{.a | 1}}", 0), "PARSE:template: t:1: non executable command in pipeline stage 2");
	CHECK_STR(run_text("{{\"unterminated}}", 0), "PARSE:template: t:1: unterminated quoted string");
	CHECK_STR(run_text("{{.a#}}", 0), "PARSE:template: t:1: bad character U+0023 '#'");
	CHECK_STR(run_text("{{break}}", 0), "PARSE:template: t:1: {{break}} outside {{range}}");
	CHECK_STR(run_text("x\n{{.a.b.c}}", "{\"a\":5}"), "ERR:template: t:2:4: executing \"t\" at <.a.b.c>: can't evaluate field b in type interface {}");
	CHECK_STR(run_text("{{index .l 5}}", "{\"l\":[1]}"), "ERR:template: t:1:2: executing \"t\" at <index .l 5>: error calling index: index out of range: 5");
	CHECK_STR(run_text("{{lt .a .b}}", "{\"a\":1,\"b\":\"x\"}"),
		"ERR:template: t:1:2: executing \"t\" at <lt .a .b>: error calling lt: incompatible types for comparison: float64 and string");
	CHECK_STR(run_text("{{template \"nope\"}}", 0), "ERR:template: t:1:11: executing \"t\" at <{{template \"nope\"}}>: template \"nope\" not defined");
	/* the output before the error stays */
	CHECK_STR(run_text("ab{{.x.y}}", "{\"x\":null}"), "ERR:template: t:1:6: executing \"t\" at <.x.y>: nil pointer evaluating interface {}.y");
	alt_template *t = alt_template_new(ALT_STR("t"));
	alt_buf out = ALT_BUF_INIT, err = ALT_BUF_INIT;
	CHECK_OK(alt_template_parse(t, ALT_STR("ab{{len 3}}"), 0));
	CHECK_ERR(alt_template_execute(t, &out, 0, &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&out), "ab");
	CHECK_STR(alt_buf_cstr(&err), "template: t:1:4: executing \"t\" at <len 3>: error calling len: len of type int");
	alt_template_free(t);
	alt_buf_free(&out);
	alt_buf_free(&err);
	/* an unparsed template */
	t = alt_template_new(ALT_STR("empty"));
	CHECK_STR(run(t, 0), "ERR:template: empty: \"empty\" is an incomplete or empty template");
	alt_template_free(t);
}

static void sets(void) {
	alt_template *t = alt_template_new(ALT_STR("root"));
	alt_buf err = ALT_BUF_INIT, b = ALT_BUF_INIT;
	CHECK_OK(alt_template_parse(t, ALT_STR("{{define \"a\"}}A{{.}}{{end}}{{define \"b\"}}B{{template \"a\" .}}{{end}}root"), &err));
	CHECK_STR(alt_template_name(t), "root");
	CHECK(alt_template_lookup(t, ALT_STR("a")) != 0);
	CHECK(alt_template_lookup(t, ALT_STR("zz")) == 0);
	alt_template_defined(t, &b);
	CHECK_STR(alt_buf_cstr(&b), "; defined templates are: \"a\", \"b\", \"root\"");
	alt_buf_reset(&b);
	CHECK_OK(alt_template_execute_name(t, &b, ALT_STR("b"), 0, &err));
	CHECK_STR(alt_buf_cstr(&b), "BA<no value>");
	alt_buf_reset(&b);
	CHECK_ERR(alt_template_execute_name(t, &b, ALT_STR("zz"), 0, &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "template: no template \"zz\" associated with template \"root\"");
	alt_buf_reset(&err);
	/* redefinition by a later parse, and an empty body that does not replace */
	CHECK_OK(alt_template_parse(t, ALT_STR("{{define \"a\"}}new{{end}}"), &err));
	CHECK_STR(run(t, 0), "root");
	CHECK_OK(alt_template_execute_name(t, &b, ALT_STR("b"), 0, &err));
	CHECK_STR(alt_buf_cstr(&b), "Bnew");
	alt_buf_reset(&b);
	/* a template associated by name, other delimiters */
	alt_template *u = alt_template_new_assoc(t, ALT_STR("u"));
	alt_template_delims(u, ALT_STR("[["), ALT_STR("]]"));
	CHECK_OK(alt_template_parse(u, ALT_STR("[[template \"a\"]]{{x}}"), &err));
	CHECK_STR(run(u, 0), "new{{x}}");
	CHECK(alt_template_lookup(t, ALT_STR("u")) == u);
	/* a failed parse adds nothing */
	CHECK_ERR(alt_template_parse(t, ALT_STR("{{define \"c\"}}C{{end}}{{if}}"), &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "template: root:1: missing value for if");
	CHECK(alt_template_lookup(t, ALT_STR("c")) == 0);
	alt_buf_reset(&err);
	CHECK_ERR(alt_template_parse(t, ALT_STR("{{define \"c\"}}1{{end}}{{define \"c\"}}2{{end}}"), &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "template: root:1: template: multiple definition of template \"c\"");
	alt_template_free(t);
	alt_buf_free(&err);
	alt_buf_free(&b);
}

static void options(void) {
	alt_template *t = alt_template_new(ALT_STR("t"));
	alt_buf err = ALT_BUF_INIT;
	CHECK_OK(alt_template_parse(t, ALT_STR("{{.a}} {{.b.c}}"), 0));
	CHECK_STR(run(t, "{\"b\":{}}"), "<no value> <no value>");
	CHECK_OK(alt_template_option(t, ALT_STR("missingkey=zero"), &err));
	CHECK_STR(run(t, "{\"b\":{}}"), "<no value> <no value>");
	CHECK_OK(alt_template_option(t, ALT_STR("missingkey=error"), &err));
	CHECK_STR(run(t, "{\"b\":{}}"), "ERR:template: t:1:2: executing \"t\" at <.a>: map has no entry for key \"a\"");
	CHECK_STR(run(t, 0), "ERR:template: t:1:2: executing \"t\" at <.a>: nil data; no entry for key \"a\"");
	CHECK_ERR(alt_template_option(t, ALT_STR("missingkey=nope"), &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "unrecognized option: missingkey=nope");
	alt_buf_reset(&err);
	CHECK_ERR(alt_template_option(t, ALT_STR(""), &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "empty option string");
	alt_template_free(t);
	alt_buf_free(&err);
}

static int f_concat(alt_template_call *c) {
	alt_buf b = ALT_BUF_INIT;
	for (size_t i = 0; i < c->argc; i++) {
		if (c->argv[i]->type == ALT_JSON_STRING)
			alt_buf_append_str(&b, c->argv[i]->str);
		else if (c->argv[i]->type == ALT_JSON_NUMBER)
			alt_buf_printf(&b, "%g", c->argv[i]->num);
		else if (c->argv[i]->type == ALT_JSON_NULL)
			alt_buf_append_cstr(&b, "null");
		else
			alt_buf_append_cstr(&b, "?");
	}
	c->result = alt_json_new_string(c->arena, alt_buf_str(&b));
	alt_buf_free(&b);
	return 0;
}

static int f_pair(alt_template_call *c) {
	alt_json *o = alt_json_new_object(c->arena);
	alt_json_set(o, "first", c->argv[0]);
	alt_json_set(o, "second", c->argv[1]);
	c->result = o;
	return 0;
}

static int f_fail(alt_template_call *c) {
	alt_buf_printf(c->err, "failed with %s", (const char *)c->ctx);
	return -EINVAL;
}

static int f_bold(alt_template_call *c) {
	c->result = alt_json_new_string(c->arena, ALT_STR("<b>bold</b>"));
	c->content = ALT_HTML_HTML;
	return 0;
}

static const alt_template_func funcs[] = {
	{"concat", f_concat, 0, ALT_TEMPLATE_VARIADIC(0)},
	{"pair", f_pair, 0, 2},
	{"fail", f_fail, "ctx", 0},
	{"bold", f_bold, 0, 0},
};

static void functions(void) {
	alt_template *t = alt_template_new(ALT_STR("t"));
	alt_buf err = ALT_BUF_INIT;
	CHECK_OK(alt_template_funcs(t, funcs, sizeof funcs / sizeof *funcs, &err));
	CHECK_OK(alt_template_parse(t, ALT_STR("{{concat \"a\" 1 2.5 nil .x}} {{(pair 1 \"b\").second}} {{.x | concat \"<\"}} {{bold}}"), &err));
	CHECK_STR(run(t, "{\"x\":\"y\"}"), "a12.5nully b <y <b>bold</b>");
	alt_template_free(t);
	t = alt_template_new(ALT_STR("t"));
	CHECK_OK(alt_template_funcs(t, funcs, sizeof funcs / sizeof *funcs, &err));
	CHECK_OK(alt_template_parse(t, ALT_STR("{{pair 1}}"), &err));
	CHECK_STR(run(t, 0), "ERR:template: t:1:2: executing \"t\" at <pair>: wrong number of args for pair: want 2 got 1");
	alt_template_free(t);
	t = alt_template_new(ALT_STR("t"));
	CHECK_OK(alt_template_funcs(t, funcs, sizeof funcs / sizeof *funcs, &err));
	CHECK_OK(alt_template_parse(t, ALT_STR("{{fail}}"), &err));
	CHECK_STR(run(t, 0), "ERR:template: t:1:2: executing \"t\" at <fail>: error calling fail: failed with ctx");
	static const alt_template_func bad[] = {{"no-dash", f_fail, 0, 0}};
	CHECK_ERR(alt_template_funcs(t, bad, 1, &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "function name \"no-dash\" is not a valid identifier");
	alt_template_free(t);
	alt_buf_free(&err);
	/* a function named like a keyword makes it an identifier */
	static const alt_template_func brk[] = {{"break", f_bold, 0, 0}};
	t = alt_template_new(ALT_STR("t"));
	CHECK_OK(alt_template_funcs(t, brk, 1, 0));
	CHECK_OK(alt_template_parse(t, ALT_STR("{{break}}"), 0));
	CHECK_STR(run(t, 0), "<b>bold</b>");
	alt_template_free(t);
}

static void limits(void) {
	/* recursion stops at depth 1000 (Go: 100000) */
	CHECK_STR(run_text("{{define \"r\"}}{{template \"r\" .}}{{end}}{{template \"r\"}}", 0),
		"ERR:template: t:1:25: executing \"r\" at <{{template \"r\" .}}>: exceeded maximum template depth (1000)");
	/* nesting in the text is limited too */
	alt_buf b = ALT_BUF_INIT;
	alt_buf_append_cstr(&b, "{{");
	for (int i = 0; i < 2000; i++)
		alt_buf_append_byte(&b, '(');
	alt_buf_append_byte(&b, '1');
	for (int i = 0; i < 2000; i++)
		alt_buf_append_byte(&b, ')');
	alt_buf_append_cstr(&b, "}}");
	CHECK_STR(run_text(alt_buf_cstr(&b), 0), "PARSE:template: t:1: max expression depth exceeded");
	alt_buf_reset(&b);
	for (int i = 0; i < 1500; i++)
		alt_buf_append_cstr(&b, "{{if 1}}");
	CHECK_STR(run_text(alt_buf_cstr(&b), 0), "PARSE:template: t:1: max expression depth exceeded");
	alt_buf_reset(&b);
	for (int i = 0; i < 500; i++)
		alt_buf_append_cstr(&b, "{{with 1}}(");
	for (int i = 0; i < 500; i++)
		alt_buf_append_cstr(&b, "){{end}}");
	alt_buf_append_cstr(&b, "{{range 3}}{{.}}{{end}}");
	CHECK(strlen(run_text(alt_buf_cstr(&b), 0)) == 1003);
	alt_buf_free(&b);
}

static void html(void) {
	CHECK_STR(run_html("<a href=\"/q?x={{.}}\" onclick=\"f({{.}})\">{{.}}</a>", "\"O'Reilly & <co>\""),
		"<a href=\"/q?x=O%27Reilly%20%26%20%3cco%3e\" onclick=\"f(&#34;O&#39;Reilly \\u0026 \\u003cco\\u003e&#34;)\">O&#39;Reilly &amp; &lt;co&gt;</a>");
	CHECK_STR(run_html("<a href=\"{{.}}\">", "\"javascript:alert(1)\""), "<a href=\"#ZgotmplZ\">");
	CHECK_STR(run_html("<script>var x = {{.}};</script>", "{\"a\":[1,\"</script>\"]}"),
		"<script>var x = {\"a\":[1,\"\\u003c/script\\u003e\"]};</script>");
	CHECK_STR(run_html("<p style=\"color: {{.}}\">", "\"expression(x)\""), "<p style=\"color: ZgotmplZ\">");
	CHECK_STR(run_html("{{.missing}}|{{.}}", "{}"), "|map[]");
	CHECK_STR(run_html("<a href=\"{{if .}}x{{end}}", "1"), "ERR:html/template:t: ends in a non-text context: {stateURL delimDoubleQuote urlPartUnknown jsCtxRegexp [] attrURL elementNone <nil>}");
	CHECK_STR(run_html("<a href=\"{{if .}}?{{end}}{{.}}\">", "1"), "ERR:html/template:t:1:27: {{.}} appears in an ambiguous context within a URL");
	CHECK_STR(run_html("{{.X | html | print}}", "{}"), "ERR:html/template:t:1:2: predefined escaper \"html\" disallowed in template");
	/* typed content from a function */
	alt_template *t = alt_html_template_new(ALT_STR("t"));
	CHECK_OK(alt_template_funcs(t, funcs, sizeof funcs / sizeof *funcs, 0));
	CHECK_OK(alt_template_parse(t, ALT_STR("<p>{{bold}}</p><p title=\"{{bold}}\">{{concat \"<i>\"}}</p>"), 0));
	CHECK_STR(run(t, 0), "<p><b>bold</b></p><p title=\"bold\">&lt;i&gt;</p>");
	/* executed once: no more parsing; errors stick */
	alt_buf err = ALT_BUF_INIT;
	CHECK_ERR(alt_template_parse(t, ALT_STR("x"), &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "html/template: cannot Parse after Execute");
	alt_template_free(t);
	alt_buf_reset(&err);
	t = alt_html_template_new(ALT_STR("t"));
	CHECK_OK(alt_template_parse(t, ALT_STR("<a href=\"{{.}}"), 0));
	CHECK_ERR(alt_html_template_escape(t, &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "html/template:t: ends in a non-text context: {stateURL delimDoubleQuote urlPartNone jsCtxRegexp [] attrURL elementNone <nil>}");
	CHECK_STR(run(t, 0), "ERR:html/template:t: ends in a non-text context: {stateURL delimDoubleQuote urlPartNone jsCtxRegexp [] attrURL elementNone <nil>}");
	alt_template_free(t);
	alt_buf_reset(&err);
	/* ExecuteTemplate escapes the named template in its own right */
	t = alt_html_template_new(ALT_STR("t"));
	CHECK_OK(alt_template_parse(t, ALT_STR("{{define \"a\"}}<b>{{.}}</b>{{end}}{{template \"a\" .}}"), 0));
	alt_buf out = ALT_BUF_INIT;
	alt_arena a;
	alt_arena_init(&a, 0);
	CHECK_OK(alt_template_execute_name(t, &out, ALT_STR("a"), parse_json(&a, "\"<>\""), &err));
	CHECK_STR(alt_buf_cstr(&out), "<b>&lt;&gt;</b>");
	alt_buf_reset(&out);
	CHECK_ERR(alt_template_execute_name(t, &out, ALT_STR("zz"), 0, &err), -EINVAL);
	CHECK_STR(alt_buf_cstr(&err), "html/template: \"zz\" is undefined");
	alt_arena_free(&a);
	alt_buf_free(&out);
	alt_buf_free(&err);
	alt_template_free(t);
	alt_buf b = ALT_BUF_INIT;
	alt_html_escape_string(&b, ALT_STR("<'\"&>"));
	CHECK_STR(alt_buf_cstr(&b), "&lt;&#39;&#34;&amp;&gt;");
	alt_buf_reset(&b);
	alt_html_js_escape_string(&b, ALT_STR("<'\\\n"));
	CHECK_STR(alt_buf_cstr(&b), "\\u003C\\'\\\\\\u000A");
	alt_buf_free(&b);
}

typedef struct {
	alt_template *t;
	alt_json *data;
	const char *want;
	int bad;
} job;

static void *worker(void *arg) {
	job *j = arg;
	for (int i = 0; i < 200; i++) {
		alt_buf out = ALT_BUF_INIT, err = ALT_BUF_INIT;
		if (alt_template_execute(j->t, &out, j->data, &err) < 0 || strcmp(alt_buf_cstr(&out), j->want))
			j->bad++;
		alt_buf_free(&out);
		alt_buf_free(&err);
	}
	return 0;
}

/* one template, many threads; for html/template the first executions
   race to escape it */
static void concurrent(void) {
	for (int html = 0; html < 2; html++) {
		alt_template *t = html ? alt_html_template_new(ALT_STR("t")) : alt_template_new(ALT_STR("t"));
		CHECK_OK(alt_template_parse(t, ALT_STR("{{define \"row\"}}<td>{{.}}</td>{{end}}{{range .}}{{template \"row\" .}}{{end}}"), 0));
		alt_arena a;
		alt_arena_init(&a, 0);
		job j[8];
		pthread_t th[8];
		for (int i = 0; i < 8; i++) {
			j[i] = (job){t, parse_json(&a, "[\"<a>\", 2]"), html ? "<td>&lt;a&gt;</td><td>2</td>" : "<td><a></td><td>2</td>", 0};
			pthread_create(&th[i], 0, worker, &j[i]);
		}
		for (int i = 0; i < 8; i++) {
			pthread_join(th[i], 0);
			CHECK_INT(j[i].bad, 0);
		}
		alt_arena_free(&a);
		alt_template_free(t);
	}
}

int main(void) {
	RUN(basics);
	RUN(errors);
	RUN(sets);
	RUN(options);
	RUN(functions);
	RUN(limits);
	RUN(html);
	RUN(concurrent);
	return check_done();
}
