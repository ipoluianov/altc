/* Internals of text/template and html/template, after Go's text/template,
   text/template/parse and html/template (BSD-3-Clause, see LICENSE-Go). */
#ifndef ALT_TEMPLATE_INTERNAL_H
#define ALT_TEMPLATE_INTERNAL_H

#include <alt/arena.h>
#include <alt/encoding/json.h>
#include <alt/html/template.h>
#include <alt/str.h>
#include <alt/sync.h>
#include <alt/text/template.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>

/* ---- lexer ---- */

enum {
	IT_ERROR,
	IT_BOOL,
	IT_CHAR,
	IT_CHAR_CONSTANT,
	IT_COMMENT,
	IT_COMPLEX,
	IT_ASSIGN,
	IT_DECLARE,
	IT_EOF,
	IT_FIELD,
	IT_IDENTIFIER,
	IT_LEFT_DELIM,
	IT_LEFT_PAREN,
	IT_NUMBER,
	IT_PIPE,
	IT_RAW_STRING,
	IT_RIGHT_DELIM,
	IT_RIGHT_PAREN,
	IT_SPACE,
	IT_STRING,
	IT_TEXT,
	IT_VARIABLE,
	IT_KEYWORD,
	IT_BLOCK,
	IT_BREAK,
	IT_CONTINUE,
	IT_DOT,
	IT_DEFINE,
	IT_ELSE,
	IT_END,
	IT_IF,
	IT_NIL,
	IT_RANGE,
	IT_TEMPLATE,
	IT_WITH,
};

typedef struct {
	int typ;
	int pos;
	alt_str val;
	int line;
} titem;

typedef struct tlexer {
	alt_str input;
	alt_str ldelim, rdelim;
	int pos, start;
	bool at_eof;
	int paren_depth;
	int line, start_line;
	titem item;
	bool inside_action;
	bool emit_comment, break_ok, continue_ok;
	alt_arena *arena; /* for error texts */
} tlexer;

void tlex_init(tlexer *l, alt_arena *a, alt_str input, alt_str left, alt_str right);
titem tlex_next(tlexer *l);

/* ---- nodes ---- */

enum {
	N_TEXT,
	N_ACTION,
	N_BOOL,
	N_CHAIN,
	N_COMMAND,
	N_DOT,
	N_ELSE,
	N_END,
	N_FIELD,
	N_IDENTIFIER,
	N_IF,
	N_LIST,
	N_NIL,
	N_NUMBER,
	N_PIPE,
	N_RANGE,
	N_STRING,
	N_TEMPLATE,
	N_VARIABLE,
	N_WITH,
	N_COMMENT,
	N_BREAK,
	N_CONTINUE,
};

typedef struct ttree ttree;
typedef struct tnode tnode;

struct tnode {
	int type;
	int pos;
	int line;
	ttree *tr;
	/* LIST: the nodes; COMMAND: the arguments; PIPE: the commands */
	tnode **nodes;
	size_t nn, ncap;
	/* PIPE: the declared variables */
	tnode **decl;
	size_t ndecl;
	bool is_assign;
	/* TEXT, COMMENT: the text; STRING: the unquoted text (quoted in
	   quoted); IDENTIFIER: the name; NUMBER: the text; TEMPLATE: the name */
	alt_str text, quoted;
	/* FIELD, VARIABLE: the identifiers; CHAIN: the fields */
	alt_str *ident;
	size_t nident;
	/* CHAIN: the node; ACTION, IF, RANGE, WITH, TEMPLATE: the pipe */
	tnode *node;
	tnode *list, *else_list;
	bool b;
	/* NUMBER */
	bool is_int, is_uint, is_float, is_complex;
	long long i64;
	unsigned long long u64;
	double f64, re, im;
};

struct ttree {
	alt_str name, parse_name;
	tnode *root;
	alt_str text;
	alt_str ldelim, rdelim;
};

/* the parse: the trees by name, or -EINVAL with Go's message in err */
typedef struct {
	ttree **trees;
	size_t n, cap;
} ttreeset;

struct tfuncs;
int tparse(alt_arena *a, alt_str name, alt_str text, alt_str left, alt_str right, const struct tfuncs *funcs, ttreeset *set,
	alt_buf *err);
bool tree_is_empty(const tnode *n);
void tnode_string(alt_buf *b, const tnode *n);
tnode *tnode_copy(alt_arena *a, const tnode *n);
tnode *tnode_new(alt_arena *a, int type, int pos, ttree *tr);
void tnode_append(alt_arena *a, tnode *list, tnode *n);
void tnode_insert(alt_arena *a, tnode *list, size_t at, tnode *n);
alt_str tstr_dup(alt_arena *a, alt_str s);
/* Go's strconv.Quote */
void tquote(alt_buf *b, alt_str s);

/* ---- values ---- */

/* the kinds of the values of a run: what json.Unmarshal into any gives
   (with V_NIL only as the content of an interface), the constants of
   templates and the results of the builtins */
enum {
	V_INVALID, /* the zero reflect.Value */
	V_MISSING, /* exec's missingVal: no final argument */
	V_NIL,     /* a nil interface (iface is set) */
	V_BOOL,
	V_INT,
	V_UINT8,
	V_FLOAT,
	V_COMPLEX,
	V_STRING,
	V_MAP,
	V_SLICE,
};

typedef struct {
	uint8_t k;
	uint8_t iface;   /* a value of static type interface{} (from a map, a slice or a function) */
	uint8_t content; /* V_STRING: an html/template type (ALT_HTML_*), 0 for string */
	bool b;
	long long i;
	double f, im;
	alt_str s;
	const alt_json *j; /* V_MAP, V_SLICE */
	size_t off, len, cap; /* V_SLICE: the window into j's items */
} tval;

/* ---- functions ---- */

enum {
	F_USER,
	F_AND,
	F_CALL,
	F_HTML,
	F_INDEX,
	F_SLICE,
	F_JS,
	F_LEN,
	F_NOT,
	F_OR,
	F_PRINT,
	F_PRINTF,
	F_PRINTLN,
	F_URLQUERY,
	F_EQ,
	F_GE,
	F_GT,
	F_LE,
	F_LT,
	F_NE,
	F_ESCAPER, /* html/template's escapers: variadic, string result */
};

typedef struct tfunc {
	alt_str name;
	int kind;
	alt_template_fn fn; /* F_USER */
	void *ctx;
	int nargs;              /* F_USER: >= 0 fixed; < 0: at least -nargs-1, variadic */
	int esc;                /* F_ESCAPER: which */
} tfunc;

typedef struct tfuncs {
	tfunc *f;
	size_t n, cap;
} tfuncs;

const tfunc *tfuncs_find(const tfuncs *fs, alt_str name);
const tfunc *tbuiltin_find(alt_str name);

/* ---- templates ---- */

typedef struct tcommon {
	alt_arena arena;
	alt_template **tmpl;
	size_t n, cap;
	int missingkey; /* 0 invalid, 1 zero, 2 error */
	tfuncs funcs;
	alt_template *all; /* every template made (to free them) */
	/* html/template: the names of the set and their escaping */
	bool html;
	bool escaped; /* executed: no more parsing */
	alt_mutex mu;
	struct hentry *hset;
	size_t nh, ch;
} tcommon;

typedef struct hentry {
	alt_str name;
	int state; /* 0 not escaped, 1 escaped, -1 failed (err) */
	alt_buf err;
} hentry;

hentry *thtml_entry(tcommon *c, alt_str name, bool add);

struct alt_template {
	alt_str name;
	ttree *tree;
	tcommon *common;
	alt_str ldelim, rdelim;
	alt_template *next_all;
};

alt_template *ttemplate_lookup(const alt_template *t, alt_str name);
alt_template *ttemplate_new_in(tcommon *c, alt_str name, alt_str ldelim, alt_str rdelim);
/* AddParseTree */
alt_template *ttemplate_add_tree(alt_template *t, alt_str name, ttree *tree);

/* executes t (whose escaping, for html/template, is done) */
int texec(alt_template *t, alt_buf *out, const alt_json *data, alt_buf *err);

/* ---- fmt ---- */

void tfmt_sprint(alt_buf *b, const tval *a, size_t n);
void tfmt_sprintln(alt_buf *b, const tval *a, size_t n);
void tfmt_sprintf(alt_buf *b, alt_str format, const tval *a, size_t n);
/* the dynamic type's name (reflect.TypeOf(x).String()) */
const char *tval_type_name(const tval *v);
/* the static type's name (reflect.Value.Type().String()) */
const char *tval_static_type_name(const tval *v);
/* the items of maps (sorted, the last of duplicate keys) and slices */
size_t tmap_sorted(alt_arena *a, const alt_json *obj, const alt_json_member ***out);
tval tslice_at(const tval *v, size_t i);
tval tjson_val(const alt_json *j, bool iface);
/* the fmt %U form with # (U+0041 'A') */
void tfmt_unicode(alt_buf *b, int32_t r, bool sharp);

/* ---- escapers shared with text/template's builtins ---- */

void thtml_escape(alt_buf *b, alt_str s);
void tjs_escape(alt_buf *b, alt_str s);
void turl_query_escape(alt_buf *b, alt_str s);
/* evalArgs: the single string argument, or fmt.Sprint of the arguments */
alt_str teval_args(alt_arena *a, const tval *args, size_t n);

/* ---- html/template ---- */

int thtml_escape_template(alt_template *t, alt_buf *err);
/* runs escaper esc on the arguments */
alt_str thtml_run_escaper(alt_arena *a, int esc, const tval *args, size_t n);
const char *thtml_escaper_name(int esc);
int thtml_escaper_by_name(alt_str name);
const tfunc *thtml_escaper_func(alt_str name);
int thtml_execute_name(alt_template *t, alt_buf *out, alt_str name, const alt_json *data, alt_buf *err);

enum {
	ESC_ATTR,
	ESC_COMMENT,
	ESC_CSS,
	ESC_CSSVALUE,
	ESC_HTMLNAME,
	ESC_HTML,
	ESC_JSREGEXP,
	ESC_JSSTR,
	ESC_JSTMPLLIT,
	ESC_JSVAL,
	ESC_NOSPACE,
	ESC_RCDATA,
	ESC_SRCSET,
	ESC_URL,
	ESC_URLFILTER,
	ESC_URLNORM,
	ESC_EVALARGS,
	ESC_COUNT,
};

typedef struct {
	const char *name;
	uint32_t r1, r2;
} thtml_entity;

extern const thtml_entity thtml_entities[];
extern const size_t thtml_nentities;
/* html.UnescapeString */
alt_str thtml_unescape(alt_arena *a, alt_str s);
/* attrType: a content type (8 for contentTypeUnsafe) */
int thtml_attr_type(alt_str name);
bool thtml_is_js_type(alt_str mime);
/* the html/template escaping helpers the transitions need */
bool thtml_ends_with_css_keyword(alt_str b, const char *kw);
alt_str thtml_decode_css(alt_arena *a, alt_str s);
int thtml_next_js_ctx(alt_str s, int preceding);

#endif
