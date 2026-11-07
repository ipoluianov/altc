/* Command-line flags, after Go's flag package (BSD-3-Clause, see
   LICENSE-Go): the same parsing rules, messages and usage format. */
#include <alt/flag.h>
#include <alt/strconv.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	alt_flag pub;
	int (*fn)(void *arg, const char *value);
	void *fn_arg;
	bool set;
} entry;

struct alt_flagset {
	char *name;
	int on_error;
	alt_writer out;
	bool has_out;
	void (*usage)(alt_flagset *fs, void *arg);
	void *usage_arg;
	entry *flags;           /* sorted by name */
	int n, cap;
	char **strings;         /* the copies the flag set owns */
	int nstrings, capstrings;
	char **args;
	int nargs;
	bool parsed;
	char *err;
};

static int stderr_fd = 2;

alt_flagset *alt_flagset_new(const char *name, int on_error) {
	alt_flagset *fs = calloc(1, sizeof *fs);
	if (!fs)
		return 0;
	fs->name = strdup(name ? name : "");
	fs->on_error = on_error;
	if (!fs->name) {
		free(fs);
		return 0;
	}
	return fs;
}

void alt_flagset_free(alt_flagset *fs) {
	if (!fs)
		return;
	for (int i = 0; i < fs->n; i++)
		free((char *)fs->flags[i].pub.def_value);
	for (int i = 0; i < fs->nstrings; i++)
		free(fs->strings[i]);
	free(fs->strings);
	free(fs->flags);
	free(fs->name);
	free(fs->err);
	free(fs);
}

void alt_flagset_set_output(alt_flagset *fs, alt_writer w) {
	fs->out = w;
	fs->has_out = true;
}

alt_writer alt_flagset_output(alt_flagset *fs) { return fs->has_out ? fs->out : alt_fd_writer(&stderr_fd); }

void alt_flagset_set_usage(alt_flagset *fs, void (*usage)(alt_flagset *, void *), void *arg) {
	fs->usage = usage;
	fs->usage_arg = arg;
}

const char *alt_flagset_name(const alt_flagset *fs) { return fs->name; }

static alt_flagset *command_line;

alt_flagset *alt_flag_command_line(void) {
	if (!command_line)
		command_line = alt_flagset_new("", ALT_FLAG_EXIT);
	return command_line;
}

static alt_flagset *set_of(alt_flagset *fs) { return fs ? fs : alt_flag_command_line(); }

static char *own(alt_flagset *fs, const char *s) {
	if (fs->nstrings == fs->capstrings) {
		int c = fs->capstrings ? 2 * fs->capstrings : 8;
		char **p = realloc(fs->strings, (size_t)c * sizeof *p);
		if (!p)
			return 0;
		fs->strings = p;
		fs->capstrings = c;
	}
	char *d = strdup(s);
	if (d)
		fs->strings[fs->nstrings++] = d;
	return d;
}

static int find(const alt_flagset *fs, const char *name, bool *found) {
	int lo = 0, hi = fs->n;
	while (lo < hi) {
		int m = lo + (hi - lo) / 2, c = strcmp(name, fs->flags[m].pub.name);
		if (c == 0) {
			*found = true;
			return m;
		}
		if (c < 0)
			hi = m;
		else
			lo = m + 1;
	}
	*found = false;
	return lo;
}

static void out_str(alt_flagset *fs, alt_str s) { alt_write_all(alt_flagset_output(fs), s.p, s.n); }

static int define(alt_flagset *fs, int kind, void *p, const char *name, const char *usage, alt_buf *def,
	int (*fn)(void *, const char *), void *fn_arg) {
	int rc = 0;
	if (!name || name[0] == '-' || strchr(name, '='))
		rc = -EINVAL;
	bool found;
	int at = rc ? 0 : find(fs, name, &found);
	if (!rc && found)
		rc = -EEXIST;
	if (!rc && fs->n == fs->cap) {
		int c = fs->cap ? 2 * fs->cap : 16;
		entry *e = realloc(fs->flags, (size_t)c * sizeof *e);
		if (!e)
			rc = -ENOMEM;
		else
			fs->flags = e, fs->cap = c;
	}
	char *dv = rc ? 0 : strdup(alt_buf_cstr(def));
	const char *nm = rc ? 0 : own(fs, name), *us = rc ? 0 : own(fs, usage ? usage : "");
	if (!rc && (!dv || !nm || !us))
		rc = -ENOMEM;
	if (rc) {
		free(dv);
		alt_buf_free(def);
		return rc;
	}
	alt_buf_free(def);
	memmove(&fs->flags[at + 1], &fs->flags[at], (size_t)(fs->n - at) * sizeof *fs->flags);
	fs->flags[at] = (entry){{nm, us, dv, kind, p}, fn, fn_arg, false};
	fs->n++;
	return 0;
}

int alt_flag_bool(alt_flagset *fs, bool *p, const char *name, bool def, const char *usage) {
	alt_buf d = ALT_BUF_INIT;
	alt_buf_append_cstr(&d, def ? "true" : "false");
	int rc = define(set_of(fs), ALT_FLAGK_BOOL, p, name, usage, &d, 0, 0);
	if (rc == 0)
		*p = def;
	return rc;
}

int alt_flag_int(alt_flagset *fs, long long *p, const char *name, long long def, const char *usage) {
	alt_buf d = ALT_BUF_INIT;
	alt_buf_append_int(&d, def);
	int rc = define(set_of(fs), ALT_FLAGK_INT, p, name, usage, &d, 0, 0);
	if (rc == 0)
		*p = def;
	return rc;
}

int alt_flag_uint(alt_flagset *fs, unsigned long long *p, const char *name, unsigned long long def, const char *usage) {
	alt_buf d = ALT_BUF_INIT;
	alt_buf_append_uint(&d, def);
	int rc = define(set_of(fs), ALT_FLAGK_UINT, p, name, usage, &d, 0, 0);
	if (rc == 0)
		*p = def;
	return rc;
}

int alt_flag_float(alt_flagset *fs, double *p, const char *name, double def, const char *usage) {
	alt_buf d = ALT_BUF_INIT;
	alt_format_float(&d, def, 'g', -1);
	int rc = define(set_of(fs), ALT_FLAGK_FLOAT, p, name, usage, &d, 0, 0);
	if (rc == 0)
		*p = def;
	return rc;
}

int alt_flag_string(alt_flagset *fs, const char **p, const char *name, const char *def, const char *usage) {
	fs = set_of(fs);
	alt_buf d = ALT_BUF_INIT;
	alt_buf_append_cstr(&d, def ? def : "");
	const char *v = own(fs, def ? def : "");
	if (!v) {
		alt_buf_free(&d);
		return -ENOMEM;
	}
	int rc = define(fs, ALT_FLAGK_STRING, p, name, usage, &d, 0, 0);
	if (rc == 0)
		*p = v;
	return rc;
}

int alt_flag_dur(alt_flagset *fs, alt_dur *p, const char *name, alt_dur def, const char *usage) {
	alt_buf d = ALT_BUF_INIT;
	alt_dur_format(&d, def);
	int rc = define(set_of(fs), ALT_FLAGK_DUR, p, name, usage, &d, 0, 0);
	if (rc == 0)
		*p = def;
	return rc;
}

int alt_flag_func(alt_flagset *fs, const char *name, const char *usage, int (*set)(void *, const char *), void *arg) {
	alt_buf d = ALT_BUF_INIT;
	return define(set_of(fs), ALT_FLAGK_FUNC, 0, name, usage, &d, set, arg);
}

int alt_flag_bool_func(alt_flagset *fs, const char *name, const char *usage, int (*set)(void *, const char *), void *arg) {
	alt_buf d = ALT_BUF_INIT;
	return define(set_of(fs), ALT_FLAGK_BOOL_FUNC, 0, name, usage, &d, set, arg);
}

/* the Value.Set of each kind; *why is Go's error text */
static int set_value(alt_flagset *fs, entry *e, const char *value, const char **why) {
	alt_str s = alt_str_cstr(value);
	int rc = 0;
	*why = 0;
	switch (e->pub.kind) {
	case ALT_FLAGK_BOOL: {
		bool b = false;
		if (alt_parse_bool(s, &b) < 0)
			rc = -ALT_ESYNTAX, *why = "parse error";
		*(bool *)e->pub.p = b;
		return rc;
	}
	case ALT_FLAGK_INT: rc = alt_parse_int(s, 0, 64, (long long *)e->pub.p); break;
	case ALT_FLAGK_UINT: rc = alt_parse_uint(s, 0, 64, (unsigned long long *)e->pub.p); break;
	case ALT_FLAGK_FLOAT: rc = alt_parse_float(s, (double *)e->pub.p); break;
	case ALT_FLAGK_STRING: {
		const char *v = own(fs, value);
		if (!v)
			return -ENOMEM;
		*(const char **)e->pub.p = v;
		return 0;
	}
	case ALT_FLAGK_DUR: {
		alt_dur d = 0;
		if (alt_dur_parse(s, &d) < 0)
			rc = -ALT_ESYNTAX, *why = "parse error", d = 0;
		*(alt_dur *)e->pub.p = d;
		return rc;
	}
	default:
		rc = e->fn(e->fn_arg, value);
		if (rc < 0)
			*why = alt_strerror(rc);
		return rc < 0 ? rc : 0;
	}
	/* numError */
	if (rc == -ALT_ESYNTAX)
		*why = "parse error";
	else if (rc == -ALT_ERANGE)
		*why = "value out of range";
	else if (rc < 0)
		*why = alt_strerror(rc);
	return rc;
}

static void default_usage(alt_flagset *fs) {
	alt_buf b = ALT_BUF_INIT;
	if (fs->name[0])
		alt_buf_printf(&b, "Usage of %s:\n", fs->name);
	else
		alt_buf_append_cstr(&b, "Usage:\n");
	out_str(fs, alt_buf_str(&b));
	alt_buf_free(&b);
	alt_flagset_print_defaults(fs);
}

static void usage(alt_flagset *fs) {
	if (fs->usage)
		fs->usage(fs, fs->usage_arg);
	else
		default_usage(fs);
}

static int failf(alt_flagset *fs, alt_buf *msg) {
	free(fs->err);
	fs->err = strdup(alt_buf_cstr(msg));
	alt_buf_append_byte(msg, '\n');
	out_str(fs, alt_buf_str(msg));
	alt_buf_free(msg);
	usage(fs);
	return -ALT_EFLAG_PARSE;
}

static void quoted(alt_buf *b, const char *s) { alt_quote(b, alt_str_cstr(s)); }

/* Go's parseOne: 1 when a flag was parsed, 0 at the end of the flags */
static int parse_one(alt_flagset *fs) {
	if (fs->nargs == 0)
		return 0;
	const char *s = fs->args[0];
	size_t len = strlen(s);
	if (len < 2 || s[0] != '-')
		return 0;
	int minuses = 1;
	if (s[1] == '-') {
		minuses++;
		if (len == 2) {
			fs->args++, fs->nargs--;
			return 0;
		}
	}
	const char *name = s + minuses;
	alt_buf msg = ALT_BUF_INIT;
	if (!name[0] || name[0] == '-' || name[0] == '=') {
		alt_buf_printf(&msg, "bad flag syntax: %s", s);
		return failf(fs, &msg);
	}
	fs->args++, fs->nargs--;
	const char *eq = strchr(name + 1, '=');
	bool has_value = eq != 0;
	const char *value = eq ? eq + 1 : "";
	alt_buf nb = ALT_BUF_INIT;
	alt_buf_append(&nb, name, eq ? (size_t)(eq - name) : strlen(name));
	const char *nm = alt_buf_cstr(&nb);
	bool found;
	int at = find(fs, nm, &found);
	if (!found) {
		if (!strcmp(nm, "help") || !strcmp(nm, "h")) {
			alt_buf_free(&nb);
			usage(fs);
			free(fs->err);
			fs->err = strdup("flag: help requested");
			return -ALT_EFLAG_HELP;
		}
		alt_buf_printf(&msg, "flag provided but not defined: -%s", nm);
		alt_buf_free(&nb);
		return failf(fs, &msg);
	}
	entry *e = &fs->flags[at];
	const char *why;
	if (e->pub.kind == ALT_FLAGK_BOOL || e->pub.kind == ALT_FLAGK_BOOL_FUNC) {
		if (has_value) {
			if (set_value(fs, e, value, &why) < 0) {
				alt_buf_append_cstr(&msg, "invalid boolean value ");
				quoted(&msg, value);
				alt_buf_printf(&msg, " for -%s: %s", nm, why);
				alt_buf_free(&nb);
				return failf(fs, &msg);
			}
		} else if (set_value(fs, e, "true", &why) < 0) {
			alt_buf_printf(&msg, "invalid boolean flag %s: %s", nm, why);
			alt_buf_free(&nb);
			return failf(fs, &msg);
		}
	} else {
		if (!has_value && fs->nargs > 0) {
			has_value = true;
			value = fs->args[0];
			fs->args++, fs->nargs--;
		}
		if (!has_value) {
			alt_buf_printf(&msg, "flag needs an argument: -%s", nm);
			alt_buf_free(&nb);
			return failf(fs, &msg);
		}
		if (set_value(fs, e, value, &why) < 0) {
			alt_buf_append_cstr(&msg, "invalid value ");
			quoted(&msg, value);
			alt_buf_printf(&msg, " for flag -%s: %s", nm, why ? why : "error");
			alt_buf_free(&nb);
			return failf(fs, &msg);
		}
	}
	e->set = true;
	alt_buf_free(&nb);
	return 1;
}

int alt_flagset_parse(alt_flagset *fs, int argc, char **argv) {
	fs = set_of(fs);
	fs->parsed = true;
	fs->args = argv;
	fs->nargs = argc;
	int rc;
	while ((rc = parse_one(fs)) > 0) {
	}
	if (rc == 0)
		return 0;
	if (fs->on_error == ALT_FLAG_EXIT)
		exit(rc == -ALT_EFLAG_HELP ? 0 : 2);
	if (fs->on_error == ALT_FLAG_ABORT)
		abort();
	return rc;
}

bool alt_flagset_parsed(const alt_flagset *fs) { return fs->parsed; }
const char *alt_flagset_error(const alt_flagset *fs) { return fs->err ? fs->err : ""; }
int alt_flagset_nargs(const alt_flagset *fs) { return fs->nargs; }
const char *alt_flagset_arg(const alt_flagset *fs, int i) { return i >= 0 && i < fs->nargs ? fs->args[i] : 0; }
char **alt_flagset_args(const alt_flagset *fs) { return fs->args; }

int alt_flagset_nflag(const alt_flagset *fs) {
	int n = 0;
	for (int i = 0; i < fs->n; i++)
		n += fs->flags[i].set;
	return n;
}

int alt_flagset_set(alt_flagset *fs, const char *name, const char *value) {
	fs = set_of(fs);
	bool found;
	int at = find(fs, name, &found);
	if (!found)
		return -ENOENT;
	const char *why;
	int rc = set_value(fs, &fs->flags[at], value, &why);
	if (rc < 0)
		return rc;
	fs->flags[at].set = true;
	return 0;
}

const alt_flag *alt_flagset_lookup(const alt_flagset *fs, const char *name) {
	if (!fs)
		fs = alt_flag_command_line();
	bool found;
	int at = find(fs, name, &found);
	return found ? &fs->flags[at].pub : 0;
}

void alt_flagset_visit_all(const alt_flagset *fs, void (*fn)(const alt_flag *, void *), void *arg) {
	for (int i = 0; i < fs->n; i++)
		fn(&fs->flags[i].pub, arg);
}

void alt_flagset_visit(const alt_flagset *fs, void (*fn)(const alt_flag *, void *), void *arg) {
	for (int i = 0; i < fs->n; i++)
		if (fs->flags[i].set)
			fn(&fs->flags[i].pub, arg);
}

void alt_flag_unquote_usage(const alt_flag *f, alt_buf *name, alt_buf *usage) {
	const char *u = f->usage, *q = strchr(u, '`');
	if (q) {
		const char *q2 = strchr(q + 1, '`');
		if (q2) {
			alt_buf_append(name, q + 1, (size_t)(q2 - q - 1));
			alt_buf_append(usage, u, (size_t)(q - u));
			alt_buf_append(usage, q + 1, (size_t)(q2 - q - 1));
			alt_buf_append_cstr(usage, q2 + 1);
			return;
		}
	}
	static const char *const names[] = {"", "int", "uint", "float", "string", "duration", "value", ""};
	alt_buf_append_cstr(name, names[f->kind]);
	alt_buf_append_cstr(usage, u);
}

static const char *zero_text(int kind) {
	switch (kind) {
	case ALT_FLAGK_BOOL: return "false";
	case ALT_FLAGK_INT: case ALT_FLAGK_UINT: case ALT_FLAGK_FLOAT: return "0";
	case ALT_FLAGK_DUR: return "0s";
	default: return "";
	}
}

void alt_flagset_print_defaults(alt_flagset *fs) {
	fs = set_of(fs);
	for (int i = 0; i < fs->n; i++) {
		const alt_flag *f = &fs->flags[i].pub;
		alt_buf b = ALT_BUF_INIT, name = ALT_BUF_INIT, us = ALT_BUF_INIT;
		alt_buf_printf(&b, "  -%s", f->name);
		alt_flag_unquote_usage(f, &name, &us);
		if (name.n) {
			alt_buf_append_byte(&b, ' ');
			alt_buf_append_str(&b, alt_buf_str(&name));
		}
		alt_buf_append_cstr(&b, b.n <= 4 ? "\t" : "\n    \t");
		for (size_t k = 0; k < us.n; k++) {
			if (us.p[k] == '\n')
				alt_buf_append_cstr(&b, "\n    \t");
			else
				alt_buf_append_byte(&b, us.p[k]);
		}
		if (strcmp(f->def_value, zero_text(f->kind)) != 0) {
			alt_buf_append_cstr(&b, " (default ");
			if (f->kind == ALT_FLAGK_STRING)
				quoted(&b, f->def_value);
			else
				alt_buf_append_cstr(&b, f->def_value);
			alt_buf_append_byte(&b, ')');
		}
		alt_buf_append_byte(&b, '\n');
		out_str(fs, alt_buf_str(&b));
		alt_buf_free(&b);
		alt_buf_free(&name);
		alt_buf_free(&us);
	}
}

void alt_flag_parse(int argc, char **argv) {
	alt_flagset *fs = alt_flag_command_line();
	if (argc > 0 && argv[0]) {
		char *n = strdup(argv[0]);
		if (n) {
			free(fs->name);
			fs->name = n;
		}
	}
	alt_flagset_parse(fs, argc > 0 ? argc - 1 : 0, argc > 0 ? argv + 1 : argv);
}
