/* AltC standard library: slash-separated paths (Go's path and
   path/filepath on Unix)

   The lexical functions append to an alt_buf or return pieces of their
   input; the others look at the file system. */
#ifndef ALT_PATH_H
#define ALT_PATH_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the shortest equivalent path: "a/./b/../c/" -> "a/c" (Clean) */
int alt_path_clean(alt_buf *b, alt_str p);
/* the non-empty elements joined and cleaned (Join) */
int alt_path_join(alt_buf *b, const alt_str *elems, size_t n);
/* dir ends with the last slash, file is the rest (Split) */
void alt_path_split(alt_str p, alt_str *dir, alt_str *file);
/* the last element (Base): "." for "", "/" for slashes only */
alt_str alt_path_base(alt_str p);
/* all but the last element, cleaned (Dir) */
int alt_path_dir(alt_buf *b, alt_str p);
/* the extension from the last dot of the last element (Ext) */
alt_str alt_path_ext(alt_str p);
bool alt_path_is_abs(alt_str p);
/* filepath.IsLocal: within the directory it is evaluated in */
bool alt_path_is_local(alt_str p);
/* shell patterns (* ? [a-z] [^x] \\): 1 match, 0 not, -EINVAL a bad pattern */
int alt_path_match(alt_str pattern, alt_str name);
/* filepath.Rel: -EINVAL ("Rel: can't make t relative to b" in *why) */
int alt_path_rel(alt_buf *b, alt_str base, alt_str targ, const char **why);
/* filepath.Abs: with the working directory */
int alt_path_abs(alt_buf *b, alt_str p);
/* filepath.Glob: the matching names (sorted) in a; -EINVAL for a bad pattern */
int alt_path_glob(alt_arena *a, alt_str pattern, alt_str **names, size_t *n);

/* filepath.WalkDir: fn for root and everything below it, in lexical
   order; err is the error reading the entry (then fn decides). fn
   returns 0 to go on, ALT_PATH_SKIP_DIR to skip the directory (or the
   rest of the entry's directory), ALT_PATH_SKIP_ALL to stop, or an
   error to stop with it. */
enum { ALT_PATH_SKIP_DIR = 1, ALT_PATH_SKIP_ALL = 2 };
typedef struct {
	alt_str name;   /* the base name */
	bool is_dir;
	unsigned type;  /* the file type bits of st_mode (S_IFREG...) */
} alt_dir_entry;
typedef int (*alt_walk_fn)(void *arg, alt_str path, const alt_dir_entry *d, int err);
int alt_path_walk(alt_str root, alt_walk_fn fn, void *arg);

#ifdef __cplusplus
}
#endif

#endif
