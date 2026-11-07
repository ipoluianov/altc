/* AltC standard library: command-line flags (Go's flag package).

   Flags are defined on a flag set with the variable that receives the
   value, then the arguments are parsed with Go's rules: -name or --name,
   -name=value, -name value (not for booleans, which take -name=false),
   "--" ends the flags, and so does the first non-flag argument.

	long long port;
	bool verbose;
	alt_flag_int(NULL, &port, "port", 8080, "listen on `port`");
	alt_flag_bool(NULL, &verbose, "v", false, "verbose output");
	alt_flag_parse(argc, argv);

   A NULL flag set means the command line's (alt_flag_command_line). */
#ifndef ALT_FLAG_H
#define ALT_FLAG_H

#include <alt/errors.h>
#include <alt/io.h>
#include <alt/time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* what parsing does on an error, as Go's ErrorHandling */
enum { ALT_FLAG_CONTINUE, ALT_FLAG_EXIT, ALT_FLAG_ABORT };
enum { ALT_FLAGK_BOOL, ALT_FLAGK_INT, ALT_FLAGK_UINT, ALT_FLAGK_FLOAT, ALT_FLAGK_STRING, ALT_FLAGK_DUR, ALT_FLAGK_FUNC,
	ALT_FLAGK_BOOL_FUNC };

typedef struct alt_flagset alt_flagset;

typedef struct {
	const char *name;
	const char *usage;
	const char *def_value; /* the default as text (as Go's DefValue) */
	int kind;
	void *p;               /* the variable */
} alt_flag;

alt_flagset *alt_flagset_new(const char *name, int on_error);
void alt_flagset_free(alt_flagset *fs);
/* where usage and error messages go (stderr by default) */
void alt_flagset_set_output(alt_flagset *fs, alt_writer w);
alt_writer alt_flagset_output(alt_flagset *fs);
/* replaces the usage message (by default "Usage of name:" and the defaults) */
void alt_flagset_set_usage(alt_flagset *fs, void (*usage)(alt_flagset *fs, void *arg), void *arg);
const char *alt_flagset_name(const alt_flagset *fs);

/* definitions: 0, -EEXIST for a name already defined, -EINVAL for a
   name starting with '-' or holding '=' (Go panics in both cases) */
int alt_flag_bool(alt_flagset *fs, bool *p, const char *name, bool def, const char *usage);
int alt_flag_int(alt_flagset *fs, long long *p, const char *name, long long def, const char *usage);
int alt_flag_uint(alt_flagset *fs, unsigned long long *p, const char *name, unsigned long long def, const char *usage);
int alt_flag_float(alt_flagset *fs, double *p, const char *name, double def, const char *usage);
/* the string stays valid while the flag set lives */
int alt_flag_string(alt_flagset *fs, const char **p, const char *name, const char *def, const char *usage);
int alt_flag_dur(alt_flagset *fs, alt_dur *p, const char *name, alt_dur def, const char *usage);
/* set is called with each value; a negative result is the error */
int alt_flag_func(alt_flagset *fs, const char *name, const char *usage, int (*set)(void *arg, const char *value), void *arg);
/* the same, but the flag takes no value (as BoolFunc: set gets "true" or the -name=value text) */
int alt_flag_bool_func(alt_flagset *fs, const char *name, const char *usage, int (*set)(void *arg, const char *value),
	void *arg);

/* parses args (without the program name): 0, -ALT_EFLAG_HELP or
   -ALT_EFLAG_PARSE after writing the message and the usage (with
   ALT_FLAG_CONTINUE; the others exit or abort) */
int alt_flagset_parse(alt_flagset *fs, int argc, char **argv);
bool alt_flagset_parsed(const alt_flagset *fs);
/* the message of the last parse error ("flag provided but not defined: -x") */
const char *alt_flagset_error(const alt_flagset *fs);
/* the arguments after the flags */
int alt_flagset_nargs(const alt_flagset *fs);
const char *alt_flagset_arg(const alt_flagset *fs, int i); /* NULL out of range */
char **alt_flagset_args(const alt_flagset *fs);
/* the number of flags that were set */
int alt_flagset_nflag(const alt_flagset *fs);
/* sets a flag as if from the command line: 0, -ENOENT, or the value's error */
int alt_flagset_set(alt_flagset *fs, const char *name, const char *value);
const alt_flag *alt_flagset_lookup(const alt_flagset *fs, const char *name);
/* the flags in name order: all, or those that were set */
void alt_flagset_visit_all(const alt_flagset *fs, void (*fn)(const alt_flag *f, void *arg), void *arg);
void alt_flagset_visit(const alt_flagset *fs, void (*fn)(const alt_flag *f, void *arg), void *arg);
/* writes the defaults to the output, as Go's PrintDefaults */
void alt_flagset_print_defaults(alt_flagset *fs);
/* Go's UnquoteUsage: the value name (from `quotes` in the usage, or the
   type) and the usage without the quotes, in memory of the caller */
void alt_flag_unquote_usage(const alt_flag *f, alt_buf *name, alt_buf *usage);

/* the command line's flag set (named after argv[0], ALT_FLAG_EXIT) */
alt_flagset *alt_flag_command_line(void);
/* parses argv[1:] with the command line's flags */
void alt_flag_parse(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
