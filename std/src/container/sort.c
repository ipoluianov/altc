/* Sorting: introsort for alt_sort; Go's stable algorithm (BSD-3-Clause,
   see LICENSE-Go) for alt_sort_stable */
#include <alt/sort.h>
#include <math.h>
#include <string.h>

static void swap(char *a, char *b, size_t size) {
	if (a == b)
		return;
	while (size >= 8) {
		uint64_t t;
		memcpy(&t, a, 8);
		memcpy(a, b, 8);
		memcpy(b, &t, 8);
		a += 8, b += 8, size -= 8;
	}
	while (size--) {
		char t = *a;
		*a++ = *b;
		*b++ = t;
	}
}

#define AT(i) (b + (i) * size)

static void insertion(char *b, size_t lo, size_t hi, size_t size, alt_cmp_fn cmp, void *arg) {
	for (size_t i = lo + 1; i < hi; i++)
		for (size_t j = i; j > lo && cmp(AT(j), AT(j - 1), arg) < 0; j--)
			swap(AT(j), AT(j - 1), size);
}

static void sift(char *b, size_t root, size_t n, size_t first, size_t size, alt_cmp_fn cmp, void *arg) {
	for (;;) {
		size_t child = 2 * root + 1;
		if (child >= n)
			return;
		if (child + 1 < n && cmp(AT(first + child), AT(first + child + 1), arg) < 0)
			child++;
		if (cmp(AT(first + root), AT(first + child), arg) >= 0)
			return;
		swap(AT(first + root), AT(first + child), size);
		root = child;
	}
}

static void heapsort(char *b, size_t lo, size_t hi, size_t size, alt_cmp_fn cmp, void *arg) {
	size_t n = hi - lo;
	for (size_t i = n / 2; i-- > 0;)
		sift(b, i, n, lo, size, cmp, arg);
	for (size_t i = n; i-- > 1;) {
		swap(AT(lo), AT(lo + i), size);
		sift(b, 0, i, lo, size, cmp, arg);
	}
}

static void intro(char *b, size_t lo, size_t hi, int depth, size_t size, alt_cmp_fn cmp, void *arg) {
	while (hi - lo > 16) {
		if (depth-- == 0) {
			heapsort(b, lo, hi, size, cmp, arg);
			return;
		}
		/* the median of three to lo */
		size_t m = lo + (hi - lo) / 2, l = lo, r = hi - 1;
		if (cmp(AT(m), AT(l), arg) < 0)
			swap(AT(m), AT(l), size);
		if (cmp(AT(r), AT(m), arg) < 0) {
			swap(AT(r), AT(m), size);
			if (cmp(AT(m), AT(l), arg) < 0)
				swap(AT(m), AT(l), size);
		}
		swap(AT(lo), AT(m), size);
		/* Hoare partition around the pivot at lo */
		size_t i = lo, j = hi;
		for (;;) {
			do
				i++;
			while (i < hi && cmp(AT(i), AT(lo), arg) < 0);
			do
				j--;
			while (cmp(AT(lo), AT(j), arg) < 0);
			if (i >= j)
				break;
			swap(AT(i), AT(j), size);
		}
		swap(AT(lo), AT(j), size);
		/* the smaller part by recursion, the larger by the loop */
		if (j - lo < hi - j - 1) {
			intro(b, lo, j, depth, size, cmp, arg);
			lo = j + 1;
		} else {
			intro(b, j + 1, hi, depth, size, cmp, arg);
			hi = j;
		}
	}
	insertion(b, lo, hi, size, cmp, arg);
}

void alt_sort(void *base, size_t n, size_t size, alt_cmp_fn cmp, void *arg) {
	int depth = 0;
	for (size_t k = n; k > 0; k >>= 1)
		depth += 2;
	intro(base, 0, n, depth, size, cmp, arg);
}

/* ---- stable: Go's stable() with insertionSort and symMerge ---- */

static void swap_range(char *b, size_t a, size_t c, size_t n, size_t size) {
	for (size_t i = 0; i < n; i++)
		swap(AT(a + i), AT(c + i), size);
}

static void rotate(char *b, size_t a, size_t m, size_t e, size_t size) {
	size_t i = m - a, j = e - m;
	while (i != j) {
		if (i > j) {
			swap_range(b, m - i, m, j, size);
			i -= j;
		} else {
			swap_range(b, m - i, m + j - i, i, size);
			j -= i;
		}
	}
	swap_range(b, m - i, m, i, size);
}

static void sym_merge(char *b, size_t a, size_t m, size_t c, size_t size, alt_cmp_fn cmp, void *arg) {
	if (m - a == 1) {
		size_t i = m, j = c;
		while (i < j) {
			size_t h = i + (j - i) / 2;
			if (cmp(AT(h), AT(a), arg) < 0)
				i = h + 1;
			else
				j = h;
		}
		for (size_t k = a; k < i - 1; k++)
			swap(AT(k), AT(k + 1), size);
		return;
	}
	if (c - m == 1) {
		size_t i = a, j = m;
		while (i < j) {
			size_t h = i + (j - i) / 2;
			if (!(cmp(AT(m), AT(h), arg) < 0))
				i = h + 1;
			else
				j = h;
		}
		for (size_t k = m; k > i; k--)
			swap(AT(k), AT(k - 1), size);
		return;
	}
	size_t mid = a + (c - a) / 2, n = mid + m, start, r;
	if (m > mid) {
		start = n - c;
		r = mid;
	} else {
		start = a;
		r = m;
	}
	size_t p = n - 1;
	while (start < r) {
		size_t h = start + (r - start) / 2;
		if (!(cmp(AT(p - h), AT(h), arg) < 0))
			start = h + 1;
		else
			r = h;
	}
	size_t end = n - start;
	if (start < m && m < end)
		rotate(b, start, m, end, size);
	if (a < start && start < mid)
		sym_merge(b, a, start, mid, size, cmp, arg);
	if (mid < end && end < c)
		sym_merge(b, mid, end, c, size, cmp, arg);
}

void alt_sort_stable(void *base, size_t n, size_t size, alt_cmp_fn cmp, void *arg) {
	char *b = base;
	size_t block = 20, a = 0, e = block;
	for (; e <= n; a = e, e += block)
		insertion(b, a, e, size, cmp, arg);
	insertion(b, a, n, size, cmp, arg);
	while (block < n) {
		a = 0, e = 2 * block;
		for (; e <= n; a = e, e += 2 * block)
			sym_merge(b, a, a + block, e, size, cmp, arg);
		if (a + block < n)
			sym_merge(b, a, a + block, n, size, cmp, arg);
		block *= 2;
	}
}

bool alt_is_sorted(const void *base, size_t n, size_t size, alt_cmp_fn cmp, void *arg) {
	const char *b = base;
	for (size_t i = 1; i < n; i++)
		if (cmp(AT(i), AT(i - 1), arg) < 0)
			return false;
	return true;
}

size_t alt_search(size_t n, bool (*pred)(size_t i, void *arg), void *arg) {
	size_t i = 0, j = n;
	while (i < j) {
		size_t h = i + (j - i) / 2;
		if (!pred(h, arg))
			i = h + 1;
		else
			j = h;
	}
	return i;
}

size_t alt_binary_search(const void *base, size_t n, size_t size, const void *key, alt_cmp_fn cmp, void *arg, bool *found) {
	const char *b = base;
	size_t i = 0, j = n;
	while (i < j) {
		size_t h = i + (j - i) / 2;
		if (cmp(AT(h), key, arg) < 0)
			i = h + 1;
		else
			j = h;
	}
	if (found)
		*found = i < n && cmp(AT(i), key, arg) == 0;
	return i;
}

int alt_cmp_int64(const void *a, const void *b, void *arg) {
	(void)arg;
	int64_t x = *(const int64_t *)a, y = *(const int64_t *)b;
	return (x > y) - (x < y);
}

static int cmp_u64(const void *a, const void *b, void *arg) {
	(void)arg;
	uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
	return (x > y) - (x < y);
}

static int cmp_double(const void *a, const void *b, void *arg) {
	(void)arg;
	double x = *(const double *)a, y = *(const double *)b;
	/* NaN sorts before everything (Go's Float64Slice.Less) */
	if (isnan(x))
		return isnan(y) ? 0 : -1;
	if (isnan(y))
		return 1;
	return (x > y) - (x < y);
}

int alt_cmp_str(const void *a, const void *b, void *arg) {
	(void)arg;
	return alt_str_cmp(*(const alt_str *)a, *(const alt_str *)b);
}

void alt_sort_int64s(int64_t *a, size_t n) { alt_sort(a, n, sizeof *a, alt_cmp_int64, 0); }
void alt_sort_uint64s(uint64_t *a, size_t n) { alt_sort(a, n, sizeof *a, cmp_u64, 0); }
void alt_sort_doubles(double *a, size_t n) { alt_sort(a, n, sizeof *a, cmp_double, 0); }
void alt_sort_strs(alt_str *a, size_t n) { alt_sort(a, n, sizeof *a, alt_cmp_str, 0); }
