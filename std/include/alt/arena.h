/* AltC standard library: arenas.

   An arena gives many allocations one lifetime: everything allocated from
   it is freed at once by alt_arena_free (or alt_arena_reset, which keeps
   the first block for reuse). An HTTP request and what is allocated for
   it live in one arena that is freed when the handler returns. Cleanup
   functions registered with alt_arena_defer run (last registered first)
   when the arena is reset or freed. An arena is not thread-safe. */
#ifndef ALT_ARENA_H
#define ALT_ARENA_H

#include <alt/errors.h>
#include <alt/str.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct alt_arena_block;
struct alt_arena_defer;

typedef struct alt_arena {
	struct alt_arena_block *blocks; /* the current block first */
	struct alt_arena_defer *defers;
	size_t block_size;              /* size of new blocks */
	size_t used;                    /* bytes handed out */
} alt_arena;

/* block_size 0 means the default (8 KiB) */
void alt_arena_init(alt_arena *a, size_t block_size);
void alt_arena_free(alt_arena *a);
void alt_arena_reset(alt_arena *a);

/* memory aligned for any type (16 bytes); NULL when out of memory */
void *alt_arena_alloc(alt_arena *a, size_t n);
void *alt_arena_zalloc(alt_arena *a, size_t n);
/* grows the last allocation in place when possible, else copies */
void *alt_arena_realloc(alt_arena *a, void *p, size_t old, size_t n);
char *alt_arena_strdup(alt_arena *a, const char *s);
/* a NUL-terminated copy */
alt_str alt_arena_str(alt_arena *a, alt_str s);
char *alt_arena_printf(alt_arena *a, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

/* fn(arg) runs when the arena is reset or freed; -ENOMEM on failure */
int alt_arena_defer(alt_arena *a, void (*fn)(void *), void *arg);

#ifdef __cplusplus
}
#endif

#endif
