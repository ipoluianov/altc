/* AltC standard library: sorting and searching (Go's sort and slices)

   The comparison returns < 0, 0, > 0 as a before, equal to, after b.
   alt_sort is not stable (introsort: O(n log n) in the worst case);
   alt_sort_stable keeps equal elements in order without allocating
   (Go's insertion blocks and SymMerge). */
#ifndef ALT_SORT_H
#define ALT_SORT_H

#include <alt/str.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*alt_cmp_fn)(const void *a, const void *b, void *arg);

void alt_sort(void *base, size_t n, size_t size, alt_cmp_fn cmp, void *arg);
void alt_sort_stable(void *base, size_t n, size_t size, alt_cmp_fn cmp, void *arg);
bool alt_is_sorted(const void *base, size_t n, size_t size, alt_cmp_fn cmp, void *arg);
/* the smallest i in [0, n) with pred(i) true, n if none (sort.Search;
   pred false then true) */
size_t alt_search(size_t n, bool (*pred)(size_t i, void *arg), void *arg);
/* where key is or would be in a sorted array (slices.BinarySearchFunc) */
size_t alt_binary_search(const void *base, size_t n, size_t size, const void *key, alt_cmp_fn cmp, void *arg, bool *found);

void alt_sort_int64s(int64_t *a, size_t n);
void alt_sort_uint64s(uint64_t *a, size_t n);
/* NaNs first, as Go's sort.Float64s */
void alt_sort_doubles(double *a, size_t n);
/* bytewise, as Go compares strings */
void alt_sort_strs(alt_str *a, size_t n);

int alt_cmp_int64(const void *a, const void *b, void *arg);
int alt_cmp_str(const void *a, const void *b, void *arg);

#ifdef __cplusplus
}
#endif

#endif
