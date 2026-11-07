/* AltC standard library: regular expressions (Go's regexp, RE2 syntax)

   Matching takes time linear in the input (a Pike VM: no backtracking),
   with Go's leftmost-first semantics (or leftmost-longest after
   alt_regexp_longest, and for alt_regexp_compile_posix):

	alt_regexp *re;
	alt_buf err = ALT_BUF_INIT;
	if (alt_regexp_compile(&re, ALT_STR("(\\w+)@(\\w+)\\.com"), &err) < 0)
		... alt_buf_cstr(&err): "error parsing regexp: missing closing ): `(a`" ...
	ssize_t m[6];
	if (alt_regexp_find_submatch(re, s, m))
		... s[m[2]:m[3]] is the first group ...

   Positions are byte offsets (-1 for groups that did not take part).
   Compiled expressions are immutable and safe for concurrent use. The
   syntax, error texts and replacement templates ($1, ${name}) are Go's. */
#ifndef ALT_REGEXP_H
#define ALT_REGEXP_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_regexp alt_regexp;

/* 0, or -EINVAL with Go's message appended to err (when not NULL) */
int alt_regexp_compile(alt_regexp **out, alt_str expr, alt_buf *err);
/* POSIX ERE syntax, leftmost-longest (Go's CompilePOSIX) */
int alt_regexp_compile_posix(alt_regexp **out, alt_str expr, alt_buf *err);
void alt_regexp_free(alt_regexp *re);
/* leftmost-longest matching from now on (call before sharing re) */
void alt_regexp_longest(alt_regexp *re);
alt_str alt_regexp_string(const alt_regexp *re);
/* the number of parenthesized subexpressions */
size_t alt_regexp_num_subexp(const alt_regexp *re);
/* the name of subexpression i (0 is the whole match): "" if unnamed */
alt_str alt_regexp_subexp_name(const alt_regexp *re, size_t i);
/* the index of the first subexpression of that name, -1 if none */
int alt_regexp_subexp_index(const alt_regexp *re, alt_str name);

bool alt_regexp_match(const alt_regexp *re, alt_str s);
/* the leftmost match: m[0..1] (m holds 2 ints); false if none */
bool alt_regexp_find(const alt_regexp *re, alt_str s, ssize_t *m);
/* with the subexpressions: m holds 2 * (num_subexp + 1) ints */
bool alt_regexp_find_submatch(const alt_regexp *re, alt_str s, ssize_t *m);
/* the leftmost match starting at or after pos (the text before pos
   counts for ^ and \b) */
bool alt_regexp_find_at(const alt_regexp *re, alt_str s, size_t pos, ssize_t *m, size_t nm);
/* successive non-overlapping matches (at most n, n < 0: all): fn gets
   each one's positions (2 * (num_subexp + 1)); the count */
size_t alt_regexp_find_all(const alt_regexp *re, alt_str s, ssize_t n, void (*fn)(void *arg, const ssize_t *m), void *arg);

/* appends src with the matches replaced by repl after $ expansion ($1,
   ${1}, $name, ${name}, $$) */
int alt_regexp_replace_all(const alt_regexp *re, alt_buf *b, alt_str src, alt_str repl);
int alt_regexp_replace_all_literal(const alt_regexp *re, alt_buf *b, alt_str src, alt_str repl);
/* the replacement by a function, given each match's positions */
int alt_regexp_replace_all_func(const alt_regexp *re, alt_buf *b, alt_str src,
	void (*fn)(void *arg, alt_buf *b, alt_str src, const ssize_t *m), void *arg);
/* Go's Expand: the template with the match's groups */
int alt_regexp_expand(const alt_regexp *re, alt_buf *b, alt_str templ, alt_str src, const ssize_t *m);
/* Go's Split: the pieces in a (at most n, n < 0: all) */
int alt_regexp_split(const alt_regexp *re, alt_arena *a, alt_str s, ssize_t n, alt_str **parts, size_t *count);

/* escapes the metacharacters of s (QuoteMeta) */
int alt_regexp_quote_meta(alt_buf *b, alt_str s);
/* Go's MatchString: -EINVAL for a bad pattern, else 0/1 */
int alt_regexp_match_string(alt_str pattern, alt_str s);

#ifdef __cplusplus
}
#endif

#endif
