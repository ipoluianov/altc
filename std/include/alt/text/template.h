/* AltC standard library: data-driven templates (Go's text/template)

   The template language, its parse and execution errors and its output
   are Go's. C has no reflection, so the data is a JSON tree: executing
   on an alt_json is what Go does on the value json.Unmarshal gives into
   an any (map[string]any, []any, float64, string, bool, nil):

	alt_template *t = alt_template_new(ALT_STR("page"));
	alt_buf err = ALT_BUF_INIT, out = ALT_BUF_INIT;
	if (alt_template_parse(t, ALT_STR("{{range .items}}{{.name}}: {{printf \"%.2f\" .price}}\n{{end}}"), &err) < 0)
		... alt_buf_cstr(&err): "template: page:1: unexpected EOF" ...
	if (alt_template_execute(t, &out, data, &err) < 0)
		... "template: page:1:9: executing \"page\" at <.name>: ..." ...
	alt_template_free(t);

   So .a.b reads members of objects, numbers print as Go prints float64
   (1e+06, 0.5), maps range in the order of their sorted keys, a missing
   member is "<no value>" (see the missingkey option), and the builtins
   (and, or, not, len, index, slice, eq, ne, lt, le, gt, ge, print,
   printf, println, html, js, urlquery, call) behave as Go's on such
   values, printf with Go's verbs, flags and %!-errors.

   A template belongs to a set of associated templates (those of define
   and block, and alt_template_new_assoc); alt_template_free on any one
   frees the set. Parsing and setting options and functions must happen
   before executions; executions may run concurrently.

   The differences from Go: templates may nest (with template, and
   parentheses and control structures in the text) only to depth 1000
   (Go: 100000 for template, 10000 for parentheses, no limit for control
   structures), since the C stack does not grow; data has no methods and
   no function values (call always fails, as on Go's JSON data);
   functions take and return JSON values (so an int argument arrives as
   a number, and a result is an interface{} holding float64, string,
   ...); printf's %p shows C addresses; the capacity of slices from JSON
   arrays, which slice's third index can reach, is Go's only up to 512
   items. There is no Clone, AddParseTree, Templates, ParseFiles, nor
   access to the parse trees. */
#ifndef ALT_TEXT_TEMPLATE_H
#define ALT_TEXT_TEMPLATE_H

#include <alt/arena.h>
#include <alt/encoding/json.h>
#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_template alt_template;

/* what a function is called with, and returns */
typedef struct {
	alt_json **argv;   /* the arguments: never NULL (nil is a JSON null); ints are numbers, complex numbers null */
	size_t argc;
	alt_arena *arena;  /* for the result: freed when the execution ends */
	void *ctx;         /* as registered */
	alt_json *result;  /* set by the function; NULL is nil */
	int content;       /* html/template: an ALT_HTML_* type for a string result, 0 for a plain one */
	alt_buf *err;      /* the message when the function fails */
} alt_template_call;

/* returns 0, or a negative code with a message appended to c->err (the
   execution then fails with "error calling name: message") */
typedef int (*alt_template_fn)(alt_template_call *c);

/* an entry of a function map: nargs >= 0 takes exactly that many
   arguments (func(a, b any) (any, error) in Go); ALT_TEMPLATE_VARIADIC(k)
   at least k (func(a any, rest ...any) with k = 1) */
typedef struct {
	const char *name;
	alt_template_fn fn;
	void *ctx;
	int nargs;
} alt_template_func;

#define ALT_TEMPLATE_VARIADIC(k) (-(k)-1)

/* a new template (and set); NULL when out of memory */
alt_template *alt_template_new(alt_str name);
/* a new template associated with t (Go's t.New(name)): same set,
   delimiters, functions and options */
alt_template *alt_template_new_assoc(alt_template *t, alt_str name);
/* frees the whole set t belongs to */
void alt_template_free(alt_template *t);
alt_str alt_template_name(const alt_template *t);

/* the action delimiters for the next parses ("" for the default {{ }}) */
void alt_template_delims(alt_template *t, alt_str left, alt_str right);
/* "missingkey=default", "missingkey=invalid", "missingkey=zero",
   "missingkey=error": 0, or -EINVAL with Go's panic text in err
   ("unrecognized option: x") */
int alt_template_option(alt_template *t, alt_str opt, alt_buf *err);
/* adds functions to the set (names must be identifiers): 0 or -EINVAL
   with Go's panic text in err */
int alt_template_funcs(alt_template *t, const alt_template_func *funcs, size_t n, alt_buf *err);

/* parses text as the body of t (its define and block clauses become
   associated templates): 0, or -EINVAL with Go's message in err (nothing
   is added then) */
int alt_template_parse(alt_template *t, alt_str text, alt_buf *err);
/* the associated template of that name, NULL if none */
alt_template *alt_template_lookup(const alt_template *t, alt_str name);
/* Go's DefinedTemplates: "; defined templates are: \"a\", \"b\"" (sorted
   here) or "" */
void alt_template_defined(const alt_template *t, alt_buf *b);

/* appends the output of t on data (NULL is nil) to out: 0, or -EINVAL
   with Go's message in err (out then holds the output up to the error) */
int alt_template_execute(alt_template *t, alt_buf *out, const alt_json *data, alt_buf *err);
/* runs the associated template of that name */
int alt_template_execute_name(alt_template *t, alt_buf *out, alt_str name, const alt_json *data, alt_buf *err);

/* the escapers of the html, js and urlquery builtins
   (HTMLEscapeString, JSEscapeString, url.QueryEscape) */
int alt_template_html_escape(alt_buf *b, alt_str s);
int alt_template_js_escape(alt_buf *b, alt_str s);
/* Go's template.IsTrue on a JSON value (NULL: nil) */
bool alt_template_is_true(const alt_json *v);

#ifdef __cplusplus
}
#endif

#endif
