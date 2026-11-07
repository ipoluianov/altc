/* Growable arrays */
#include <alt/vec.h>
#include <stdbool.h>

int alt_vec_reserve(alt_vec *v, size_t extra) {
	if (v->n + extra <= v->cap)
		return 0;
	size_t cap = v->cap ? v->cap * 2 : 8;
	while (cap < v->n + extra)
		cap *= 2;
	void *p = realloc(v->p, cap * v->size);
	if (!p)
		return -ENOMEM;
	v->p = p;
	v->cap = cap;
	return 0;
}

int alt_vec_push(alt_vec *v, const void *elem) { return alt_vec_insert(v, v->n, elem, 1); }

int alt_vec_insert(alt_vec *v, size_t i, const void *elems, size_t n) {
	if (alt_vec_reserve(v, n) < 0)
		return -ENOMEM;
	char *p = v->p;
	memmove(p + (i + n) * v->size, p + i * v->size, (v->n - i) * v->size);
	if (n)
		memcpy(p + i * v->size, elems, n * v->size);
	v->n += n;
	return 0;
}

void alt_vec_delete(alt_vec *v, size_t i, size_t j) {
	char *p = v->p;
	memmove(p + i * v->size, p + j * v->size, (v->n - j) * v->size);
	v->n -= j - i;
}

bool alt_vec_pop(alt_vec *v, void *out) {
	if (!v->n)
		return false;
	v->n--;
	if (out)
		memcpy(out, (char *)v->p + v->n * v->size, v->size);
	return true;
}

void alt_vec_free(alt_vec *v) {
	free(v->p);
	v->p = 0;
	v->n = v->cap = 0;
}
