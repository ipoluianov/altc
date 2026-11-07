/* AltC standard library: doubly linked lists (Go's container/list) */
#ifndef ALT_CONTAINER_LIST_H
#define ALT_CONTAINER_LIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_list_elem {
	struct alt_list_elem *next, *prev;
	struct alt_list *list;
	void *value;
} alt_list_elem;

typedef struct alt_list {
	alt_list_elem root; /* a sentinel */
	size_t len;
} alt_list;

void alt_list_init(alt_list *l);
/* frees the elements (not the values) */
void alt_list_free(alt_list *l);
size_t alt_list_len(const alt_list *l);
alt_list_elem *alt_list_front(const alt_list *l);
alt_list_elem *alt_list_back(const alt_list *l);
alt_list_elem *alt_list_next(const alt_list_elem *e);
alt_list_elem *alt_list_prev(const alt_list_elem *e);
/* NULL on ENOMEM */
alt_list_elem *alt_list_push_front(alt_list *l, void *v);
alt_list_elem *alt_list_push_back(alt_list *l, void *v);
alt_list_elem *alt_list_insert_before(alt_list *l, void *v, alt_list_elem *mark);
alt_list_elem *alt_list_insert_after(alt_list *l, void *v, alt_list_elem *mark);
/* removes and frees e (when it is in l): its value */
void *alt_list_remove(alt_list *l, alt_list_elem *e);
void alt_list_move_to_front(alt_list *l, alt_list_elem *e);
void alt_list_move_to_back(alt_list *l, alt_list_elem *e);
void alt_list_move_before(alt_list *l, alt_list_elem *e, alt_list_elem *mark);
void alt_list_move_after(alt_list *l, alt_list_elem *e, alt_list_elem *mark);

#ifdef __cplusplus
}
#endif

#endif
