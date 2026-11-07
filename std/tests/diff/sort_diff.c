/* stable k... -> the original indices in stable order by key
   sort k... -> the sorted keys;  bsearch key k... (sorted) -> "index found"
   heap op... -> popped values then the array (ops: i v,v,v | p v | o | r i | f i v)
   list op... -> the values front to back (ops: pf v | pb v | ib i v | ia i v | rm i | mf i | mb i | mbf i j | maf i j) */
#include "common.h"
#include <alt/container/heap.h>
#include <alt/container/list.h>
#include <alt/sort.h>
#include <stdint.h>

typedef struct {
	int64_t k;
	int64_t i;
} pair;

static int cmp_pair(const void *a, const void *b, void *arg) {
	(void)arg;
	int64_t x = ((const pair *)a)->k, y = ((const pair *)b)->k;
	return (x > y) - (x < y);
}

static alt_list_elem *nth(alt_list *l, long i) {
	alt_list_elem *e = alt_list_front(l);
	while (e && i-- > 0)
		e = alt_list_next(e);
	return e;
}

int main(void) {
	char *line, *f[20000];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 20000);
		if (!strcmp(f[0], "stable") || !strcmp(f[0], "sort")) {
			pair *p = malloc(sizeof *p * (size_t)(n + 1));
			int64_t *k = malloc(sizeof *k * (size_t)(n + 1));
			for (int i = 1; i < n; i++)
				p[i - 1] = (pair){atoll(f[i]), i - 1}, k[i - 1] = atoll(f[i]);
			if (f[0][1] == 't') {
				alt_sort_stable(p, (size_t)(n - 1), sizeof *p, cmp_pair, 0);
				for (int i = 0; i < n - 1; i++)
					printf("%s%lld", i ? " " : "", (long long)p[i].i);
			} else {
				alt_sort_int64s(k, (size_t)(n - 1));
				for (int i = 0; i < n - 1; i++)
					printf("%s%lld", i ? " " : "", (long long)k[i]);
			}
			free(p), free(k);
		} else if (!strcmp(f[0], "bsearch")) {
			int64_t key = atoll(f[1]), *k = malloc(sizeof *k * (size_t)n);
			for (int i = 2; i < n; i++)
				k[i - 2] = atoll(f[i]);
			bool found;
			size_t at = alt_binary_search(k, (size_t)(n - 2), sizeof *k, &key, alt_cmp_int64, 0, &found);
			printf("%zu %d", at, found);
			free(k);
		} else if (!strcmp(f[0], "heap")) {
			alt_vec h = ALT_VEC_INIT(int64_t);
			for (int i = 1; i < n; i++) {
				char *op = f[i];
				int64_t v;
				if (op[0] == 'i') {
					for (char *s = op + 2; *s;) {
						v = strtoll(s, &s, 10);
						alt_vec_push(&h, &v);
						if (*s == ',')
							s++;
					}
					alt_heap_init(&h, alt_cmp_int64, 0);
				} else if (op[0] == 'p') {
					v = atoll(op + 2);
					alt_heap_push(&h, &v, alt_cmp_int64, 0);
				} else if (op[0] == 'o') {
					if (alt_heap_pop(&h, &v, alt_cmp_int64, 0))
						printf("%lld ", (long long)v);
				} else if (op[0] == 'r') {
					size_t at = strtoul(op + 2, 0, 10);
					if (at < h.n) {
						alt_heap_remove(&h, at, &v, alt_cmp_int64, 0);
						printf("%lld ", (long long)v);
					}
				} else if (op[0] == 'f') {
					char *s;
					size_t at = strtoul(op + 2, &s, 10);
					if (at < h.n) {
						*(int64_t *)alt_vec_at(&h, at) = atoll(s + 1);
						alt_heap_fix(&h, at, alt_cmp_int64, 0);
					}
				}
			}
			printf("|");
			for (size_t i = 0; i < h.n; i++)
				printf(" %lld", (long long)*(int64_t *)alt_vec_at(&h, i));
			alt_vec_free(&h);
		} else if (!strcmp(f[0], "list")) {
			alt_list l;
			alt_list_init(&l);
			for (int i = 1; i < n; i++) {
				char *op = f[i], *s;
				long a = 0, b = 0;
				char *sp = strchr(op, ' ');
				if (sp) {
					a = strtol(sp + 1, &s, 10);
					if (*s == ' ')
						b = strtol(s + 1, 0, 10);
				}
				alt_list_elem *ea = nth(&l, a), *eb = nth(&l, b);
				intptr_t v = a;
				if (!strncmp(op, "pf ", 3))
					alt_list_push_front(&l, (void *)v);
				else if (!strncmp(op, "pb ", 3))
					alt_list_push_back(&l, (void *)v);
				else if (!strncmp(op, "ib ", 3) && ea)
					alt_list_insert_before(&l, (void *)(intptr_t)b, ea);
				else if (!strncmp(op, "ia ", 3) && ea)
					alt_list_insert_after(&l, (void *)(intptr_t)b, ea);
				else if (!strncmp(op, "rm ", 3) && ea)
					alt_list_remove(&l, ea);
				else if (!strncmp(op, "mf ", 3) && ea)
					alt_list_move_to_front(&l, ea);
				else if (!strncmp(op, "mb ", 3) && ea)
					alt_list_move_to_back(&l, ea);
				else if (!strncmp(op, "mbf ", 4) && ea && eb)
					alt_list_move_before(&l, ea, eb);
				else if (!strncmp(op, "maf ", 4) && ea && eb)
					alt_list_move_after(&l, ea, eb);
			}
			printf("%zu:", alt_list_len(&l));
			for (alt_list_elem *e = alt_list_front(&l); e; e = alt_list_next(e))
				printf(" %ld", (long)(intptr_t)e->value);
			printf(" <");
			for (alt_list_elem *e = alt_list_back(&l); e; e = alt_list_prev(e))
				printf(" %ld", (long)(intptr_t)e->value);
			alt_list_free(&l);
		}
		printf("\n");
		fflush(stdout);
	}
	return 0;
}
