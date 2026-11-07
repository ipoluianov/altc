#include <alt/arena.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct alt_arena_block {
	struct alt_arena_block *next;
	size_t size, off; /* capacity and use of data */
	size_t last;      /* offset of the last allocation (for realloc) */
	_Alignas(16) unsigned char data[];
};

struct alt_arena_defer {
	struct alt_arena_defer *next;
	void (*fn)(void *);
	void *arg;
};

enum { DEFAULT_BLOCK = 8192, ALIGN = 16 };

void alt_arena_init(alt_arena *a, size_t block_size) {
	a->blocks = 0;
	a->defers = 0;
	a->block_size = block_size ? block_size : DEFAULT_BLOCK;
	a->used = 0;
}

static void run_defers(alt_arena *a) {
	while (a->defers) {
		struct alt_arena_defer *d = a->defers;
		a->defers = d->next;
		d->fn(d->arg); /* d itself is arena memory: freed with the blocks */
	}
}

void alt_arena_free(alt_arena *a) {
	run_defers(a);
	while (a->blocks) {
		struct alt_arena_block *b = a->blocks;
		a->blocks = b->next;
		free(b);
	}
	a->used = 0;
}

void alt_arena_reset(alt_arena *a) {
	run_defers(a);
	/* keep the oldest block (the last in the list) when it has the
	   regular size: the next use starts without malloc */
	struct alt_arena_block *keep = 0;
	while (a->blocks) {
		struct alt_arena_block *b = a->blocks;
		a->blocks = b->next;
		if (!a->blocks && b->size == a->block_size)
			keep = b;
		else
			free(b);
	}
	if (keep) {
		keep->off = 0;
		keep->last = 0;
		keep->next = 0;
	}
	a->blocks = keep;
	a->used = 0;
}

static struct alt_arena_block *new_block(alt_arena *a, size_t n) {
	size_t size = a->block_size;
	if (n > size / 4) {
		/* a large allocation gets a block of its own, behind the current
		   one, which stays the place for small allocations */
		if (n > SIZE_MAX - sizeof(struct alt_arena_block))
			return 0;
		struct alt_arena_block *b = malloc(sizeof *b + n);
		if (!b)
			return 0;
		b->size = n;
		b->off = 0;
		b->last = 0;
		if (a->blocks) {
			b->next = a->blocks->next;
			a->blocks->next = b;
		} else {
			b->next = 0;
			a->blocks = b;
		}
		return b;
	}
	struct alt_arena_block *b = malloc(sizeof *b + size);
	if (!b)
		return 0;
	b->size = size;
	b->off = 0;
	b->last = 0;
	b->next = a->blocks;
	a->blocks = b;
	return b;
}

void *alt_arena_alloc(alt_arena *a, size_t n) {
	if (!a->block_size)
		a->block_size = DEFAULT_BLOCK;
	if (n == 0)
		n = 1;
	if (n > SIZE_MAX - ALIGN)
		return 0;
	size_t need = (n + ALIGN - 1) & ~(size_t)(ALIGN - 1);
	struct alt_arena_block *b = a->blocks;
	if (!b || b->size - b->off < need) {
		b = new_block(a, need);
		if (!b)
			return 0;
	}
	void *p = b->data + b->off;
	b->last = b->off;
	b->off += need;
	a->used += need;
	return p;
}

void *alt_arena_zalloc(alt_arena *a, size_t n) {
	void *p = alt_arena_alloc(a, n);
	if (p)
		memset(p, 0, n);
	return p;
}

void *alt_arena_realloc(alt_arena *a, void *p, size_t old, size_t n) {
	if (!p)
		return alt_arena_alloc(a, n);
	struct alt_arena_block *b = a->blocks;
	if (b && (unsigned char *)p == b->data + b->last) {
		size_t need = (n + ALIGN - 1) & ~(size_t)(ALIGN - 1);
		if (need <= b->size - b->last) {
			a->used += need - (b->off - b->last);
			b->off = b->last + need;
			return p;
		}
	}
	if (n <= old)
		return p;
	void *q = alt_arena_alloc(a, n);
	if (q)
		memcpy(q, p, old);
	return q;
}

char *alt_arena_strdup(alt_arena *a, const char *s) {
	size_t n = strlen(s);
	char *p = alt_arena_alloc(a, n + 1);
	if (p)
		memcpy(p, s, n + 1);
	return p;
}

alt_str alt_arena_str(alt_arena *a, alt_str s) {
	char *p = alt_arena_alloc(a, s.n + 1);
	if (!p)
		return ALT_STR_NULL;
	if (s.n)
		memcpy(p, s.p, s.n);
	p[s.n] = 0;
	return (alt_str){p, s.n};
}

char *alt_arena_printf(alt_arena *a, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char small[256];
	va_list ap2;
	va_copy(ap2, ap);
	int k = vsnprintf(small, sizeof small, fmt, ap2);
	va_end(ap2);
	char *p = 0;
	if (k >= 0) {
		p = alt_arena_alloc(a, (size_t)k + 1);
		if (p) {
			if ((size_t)k < sizeof small)
				memcpy(p, small, (size_t)k + 1);
			else
				vsnprintf(p, (size_t)k + 1, fmt, ap);
		}
	}
	va_end(ap);
	return p;
}

int alt_arena_defer(alt_arena *a, void (*fn)(void *), void *arg) {
	struct alt_arena_defer *d = alt_arena_alloc(a, sizeof *d);
	if (!d)
		return -ENOMEM;
	d->fn = fn;
	d->arg = arg;
	d->next = a->defers;
	a->defers = d;
	return 0;
}
