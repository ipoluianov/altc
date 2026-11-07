/* regexp internals: the syntax tree (Go's regexp/syntax Regexp) and the
   program (Prog) */
#ifndef ALT_REGEXP_INTERNAL_H
#define ALT_REGEXP_INTERNAL_H

#include <alt/arena.h>
#include <alt/regexp.h>
#include <stdint.h>

enum {
	OP_NO_MATCH = 1, OP_EMPTY_MATCH, OP_LITERAL, OP_CHAR_CLASS, OP_ANY_CHAR_NOT_NL, OP_ANY_CHAR, OP_BEGIN_LINE, OP_END_LINE,
	OP_BEGIN_TEXT, OP_END_TEXT, OP_WORD_BOUNDARY, OP_NO_WORD_BOUNDARY, OP_CAPTURE, OP_STAR, OP_PLUS, OP_QUEST, OP_REPEAT,
	OP_CONCAT, OP_ALTERNATE, OP_PSEUDO = 128, OP_LEFT_PAREN, OP_VERTICAL_BAR,
};

enum {
	F_FOLD = 1, F_LITERAL = 2, F_CLASSNL = 4, F_DOTNL = 8, F_ONELINE = 16, F_NONGREEDY = 32, F_PERLX = 64, F_UNICODE = 128,
	F_WASDOLLAR = 256, F_PERL = F_CLASSNL | F_ONELINE | F_PERLX | F_UNICODE, F_POSIX = 0,
};

typedef alt_arena rx_arena;
#define rx_alloc alt_arena_alloc
#define rx_realloc alt_arena_realloc

typedef struct node {
	int op;
	uint16_t flags;
	struct node **sub;
	size_t nsub;
	int32_t *runes;
	size_t nrunes, caprunes;
	int min, max, cap;
	alt_str name;
	int height;
	int64_t size;
} node;

#define rx_parse alt__rx_parse
#define rx_clean_class alt__rx_clean_class
node *rx_parse(rx_arena *a, alt_str s, uint16_t flags, int *ncap, const char **code, alt_str *expr);
void rx_clean_class(node *re);

#endif
