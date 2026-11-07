/* Doubly linked lists after Go's container/list (BSD-3-Clause, see LICENSE-Go) */
#include <alt/container/list.h>
#include <stdlib.h>

void alt_list_init(alt_list *l) {
	l->root.next = l->root.prev = &l->root;
	l->root.list = l;
	l->len = 0;
}

void alt_list_free(alt_list *l) {
	alt_list_elem *e = l->root.next;
	while (e && e != &l->root) {
		alt_list_elem *n = e->next;
		free(e);
		e = n;
	}
	alt_list_init(l);
}

size_t alt_list_len(const alt_list *l) { return l->len; }
alt_list_elem *alt_list_front(const alt_list *l) { return l->len ? l->root.next : 0; }
alt_list_elem *alt_list_back(const alt_list *l) { return l->len ? l->root.prev : 0; }

alt_list_elem *alt_list_next(const alt_list_elem *e) {
	alt_list_elem *p = e->next;
	return e->list && p != &e->list->root ? p : 0;
}

alt_list_elem *alt_list_prev(const alt_list_elem *e) {
	alt_list_elem *p = e->prev;
	return e->list && p != &e->list->root ? p : 0;
}

static alt_list_elem *insert(alt_list *l, alt_list_elem *e, alt_list_elem *at) {
	e->prev = at;
	e->next = at->next;
	e->prev->next = e;
	e->next->prev = e;
	e->list = l;
	l->len++;
	return e;
}

static alt_list_elem *insert_value(alt_list *l, void *v, alt_list_elem *at) {
	alt_list_elem *e = malloc(sizeof *e);
	if (!e)
		return 0;
	e->value = v;
	return insert(l, e, at);
}

static void unlink_elem(alt_list *l, alt_list_elem *e) {
	e->prev->next = e->next;
	e->next->prev = e->prev;
	e->next = e->prev = 0;
	e->list = 0;
	l->len--;
}

static void move(alt_list *l, alt_list_elem *e, alt_list_elem *at) {
	if (e == at)
		return;
	e->prev->next = e->next;
	e->next->prev = e->prev;
	e->prev = at;
	e->next = at->next;
	e->prev->next = e;
	e->next->prev = e;
	(void)l;
}

alt_list_elem *alt_list_push_front(alt_list *l, void *v) { return insert_value(l, v, &l->root); }
alt_list_elem *alt_list_push_back(alt_list *l, void *v) { return insert_value(l, v, l->root.prev); }

alt_list_elem *alt_list_insert_before(alt_list *l, void *v, alt_list_elem *mark) {
	return mark->list == l ? insert_value(l, v, mark->prev) : 0;
}

alt_list_elem *alt_list_insert_after(alt_list *l, void *v, alt_list_elem *mark) { return mark->list == l ? insert_value(l, v, mark) : 0; }

void *alt_list_remove(alt_list *l, alt_list_elem *e) {
	void *v = e->value;
	if (e->list == l) {
		unlink_elem(l, e);
		free(e);
	}
	return v;
}

void alt_list_move_to_front(alt_list *l, alt_list_elem *e) {
	if (e->list != l || l->root.next == e)
		return;
	move(l, e, &l->root);
}

void alt_list_move_to_back(alt_list *l, alt_list_elem *e) {
	if (e->list != l || l->root.prev == e)
		return;
	move(l, e, l->root.prev);
}

void alt_list_move_before(alt_list *l, alt_list_elem *e, alt_list_elem *mark) {
	if (e->list != l || e == mark || mark->list != l)
		return;
	move(l, e, mark->prev);
}

void alt_list_move_after(alt_list *l, alt_list_elem *e, alt_list_elem *mark) {
	if (e->list != l || e == mark || mark->list != l)
		return;
	move(l, e, mark);
}
