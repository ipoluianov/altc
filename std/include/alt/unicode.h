/* AltC standard library: Unicode character classes and case mapping
   (Go's unicode package, with its tables).

   Categories are tables (alt_unicode_Lu, alt_unicode_Nd...) to test with
   alt_unicode_is; the common questions have functions of their own,
   answering as Go's unicode.IsLetter, IsPrint, ToUpper... do. */
#ifndef ALT_UNICODE_H
#define ALT_UNICODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_UNICODE_MAX_RUNE = 0x10FFFF };

typedef struct {
	const uint32_t (*r)[3]; /* lo, hi, stride: sorted ranges */
	int n;
	int latin_offset;       /* the ranges below U+0100 */
} alt_unicode_table;

typedef struct {
	uint32_t lo, hi;
	int32_t delta[3];       /* to upper, lower, title */
} alt_unicode_case_range;

#define ALT__UNICODE_TABLES(X) \
	X(L) X(Lu) X(Ll) X(Lt) X(Lm) X(Lo) X(M) X(Mn) X(Mc) X(Me) X(N) X(Nd) X(Nl) X(No) \
	X(P) X(Pc) X(Pd) X(Ps) X(Pe) X(Pi) X(Pf) X(Po) X(S) X(Sm) X(Sc) X(Sk) X(So) \
	X(Z) X(Zs) X(Zl) X(Zp) X(C) X(Cc) X(Cf) X(Co) X(Cs) X(White_Space)
#define ALT__UNICODE_DECL(t) extern const alt_unicode_table alt_unicode_##t;
ALT__UNICODE_TABLES(ALT__UNICODE_DECL)
#undef ALT__UNICODE_DECL

bool alt_unicode_is(const alt_unicode_table *t, int32_t r);
/* every category ("Lu", "LC", "Cn"...), script ("Greek"...) and
   case-folding table by name (Go's Categories, Scripts, FoldCategory,
   FoldScript), and the category aliases ("Uppercase_Letter" -> "Lu") */
typedef struct {
	const char *name;
	const alt_unicode_table *table;
} alt_unicode_group;
extern const alt_unicode_group alt__unicode_categories[], alt__unicode_scripts[], alt__unicode_fold_categories[],
	alt__unicode_fold_scripts[];
extern const size_t alt__unicode_ncategories, alt__unicode_nscripts, alt__unicode_nfold_categories, alt__unicode_nfold_scripts;
extern const char *const alt__unicode_category_aliases[][2];
extern const size_t alt__unicode_ncategory_aliases;
/* a table of one of those lists by exact name, NULL if absent */
const alt_unicode_table *alt_unicode_group_find(const alt_unicode_group *g, size_t n, const char *name);

/* the table of a category or property by name ("Lu", "White_Space"), NULL if unknown */
const alt_unicode_table *alt_unicode_category(const char *name);

bool alt_unicode_is_letter(int32_t r);
bool alt_unicode_is_digit(int32_t r);   /* Nd */
bool alt_unicode_is_number(int32_t r);  /* N */
bool alt_unicode_is_upper(int32_t r);
bool alt_unicode_is_lower(int32_t r);
bool alt_unicode_is_title(int32_t r);
bool alt_unicode_is_mark(int32_t r);
bool alt_unicode_is_punct(int32_t r);
bool alt_unicode_is_symbol(int32_t r);
bool alt_unicode_is_control(int32_t r); /* C0, DEL, C1 */
bool alt_unicode_is_space(int32_t r);   /* White_Space */
/* L, M, N, P, S and U+0020 (Go's IsPrint) */
bool alt_unicode_is_print(int32_t r);
/* L, M, N, P, S, Zs */
bool alt_unicode_is_graphic(int32_t r);

int32_t alt_unicode_to_upper(int32_t r);
int32_t alt_unicode_to_lower(int32_t r);
int32_t alt_unicode_to_title(int32_t r);
/* the next rune of r's case-folding orbit (Go's SimpleFold) */
int32_t alt_unicode_simple_fold(int32_t r);

#ifdef __cplusplus
}
#endif

#endif
