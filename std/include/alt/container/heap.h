/* AltC standard library: binary heaps on an alt_vec (Go's container/heap:
   the same moves, so the same orders). cmp(a, b) < 0 means a goes first. */
#ifndef ALT_CONTAINER_HEAP_H
#define ALT_CONTAINER_HEAP_H

#include <alt/sort.h>
#include <alt/vec.h>

#ifdef __cplusplus
extern "C" {
#endif

void alt_heap_init(alt_vec *h, alt_cmp_fn cmp, void *arg);
int alt_heap_push(alt_vec *h, const void *x, alt_cmp_fn cmp, void *arg);
/* the first element into out: false when empty */
bool alt_heap_pop(alt_vec *h, void *out, alt_cmp_fn cmp, void *arg);
void alt_heap_remove(alt_vec *h, size_t i, void *out, alt_cmp_fn cmp, void *arg);
/* after element i changed */
void alt_heap_fix(alt_vec *h, size_t i, alt_cmp_fn cmp, void *arg);

#ifdef __cplusplus
}
#endif

#endif
