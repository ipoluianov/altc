/* Binary heaps after Go's container/heap (BSD-3-Clause, see LICENSE-Go) */
#include <alt/container/heap.h>
#include <stdint.h>

static void swap(alt_vec *h, size_t i, size_t j) {
	char *a = alt_vec_at(h, i), *b = alt_vec_at(h, j);
	for (size_t k = 0; k < h->size; k++) {
		char t = a[k];
		a[k] = b[k];
		b[k] = t;
	}
}

static bool less(alt_vec *h, size_t i, size_t j, alt_cmp_fn cmp, void *arg) { return cmp(alt_vec_at(h, i), alt_vec_at(h, j), arg) < 0; }

static void up(alt_vec *h, size_t j, alt_cmp_fn cmp, void *arg) {
	while (j > 0) {
		size_t i = (j - 1) / 2;
		if (!less(h, j, i, cmp, arg))
			break;
		swap(h, i, j);
		j = i;
	}
}

static bool down(alt_vec *h, size_t i0, size_t n, alt_cmp_fn cmp, void *arg) {
	size_t i = i0;
	for (;;) {
		size_t j1 = 2 * i + 1;
		if (j1 >= n)
			break;
		size_t j = j1;
		if (j1 + 1 < n && less(h, j1 + 1, j1, cmp, arg))
			j = j1 + 1;
		if (!less(h, j, i, cmp, arg))
			break;
		swap(h, i, j);
		i = j;
	}
	return i > i0;
}

void alt_heap_init(alt_vec *h, alt_cmp_fn cmp, void *arg) {
	size_t n = h->n;
	for (size_t i = n / 2; i-- > 0;)
		down(h, i, n, cmp, arg);
}

int alt_heap_push(alt_vec *h, const void *x, alt_cmp_fn cmp, void *arg) {
	int rc = alt_vec_push(h, x);
	if (rc == 0)
		up(h, h->n - 1, cmp, arg);
	return rc;
}

bool alt_heap_pop(alt_vec *h, void *out, alt_cmp_fn cmp, void *arg) {
	if (!h->n)
		return false;
	size_t n = h->n - 1;
	swap(h, 0, n);
	down(h, 0, n, cmp, arg);
	alt_vec_pop(h, out);
	return true;
}

void alt_heap_remove(alt_vec *h, size_t i, void *out, alt_cmp_fn cmp, void *arg) {
	size_t n = h->n - 1;
	if (n != i) {
		swap(h, i, n);
		if (!down(h, i, n, cmp, arg))
			up(h, i, cmp, arg);
	}
	alt_vec_pop(h, out);
}

void alt_heap_fix(alt_vec *h, size_t i, alt_cmp_fn cmp, void *arg) {
	if (!down(h, i, h->n, cmp, arg))
		up(h, i, cmp, arg);
}
