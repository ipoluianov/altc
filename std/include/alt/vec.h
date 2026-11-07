/* AltC standard library: growable arrays (Go's slices with append)

	ALT_VEC(ints, int64_t);
	ints v = {0};
	ints_push(&v, 42);
	for (size_t i = 0; i < v.n; i++) ... v.p[i] ...
	ints_free(&v);

   alt_vec is the same for elements of a size known at run time. */
#ifndef ALT_VEC_H
#define ALT_VEC_H

#include <alt/errors.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *p;
	size_t n, cap, size;   /* size: of an element */
} alt_vec;

#define ALT_VEC_INIT(type) {0, 0, 0, sizeof(type)}

int alt_vec_reserve(alt_vec *v, size_t extra);
int alt_vec_push(alt_vec *v, const void *elem);
/* inserts n elements at i (slices.Insert) */
int alt_vec_insert(alt_vec *v, size_t i, const void *elems, size_t n);
/* removes [i, j) (slices.Delete) */
void alt_vec_delete(alt_vec *v, size_t i, size_t j);
/* the last element into out (when not NULL): false when empty */
bool alt_vec_pop(alt_vec *v, void *out);
static inline void *alt_vec_at(const alt_vec *v, size_t i) { return (char *)v->p + i * v->size; }
void alt_vec_free(alt_vec *v);

/* a typed vector: name##_push, _pop, _insert, _delete, _reserve, _free */
#define ALT_VEC(name, T)                                                                                                     \
	typedef struct {                                                                                                     \
		T *p;                                                                                                        \
		size_t n, cap;                                                                                               \
	} name;                                                                                                              \
	static inline int name##_reserve(name *v, size_t extra) {                                                            \
		if (v->n + extra <= v->cap)                                                                                  \
			return 0;                                                                                            \
		size_t cap = v->cap ? v->cap * 2 : 8;                                                                        \
		while (cap < v->n + extra)                                                                                   \
			cap *= 2;                                                                                            \
		T *p = (T *)realloc(v->p, cap * sizeof(T));                                                                  \
		if (!p)                                                                                                      \
			return -ENOMEM;                                                                                      \
		v->p = p;                                                                                                    \
		v->cap = cap;                                                                                                \
		return 0;                                                                                                    \
	}                                                                                                                    \
	static inline int name##_push(name *v, T x) {                                                                        \
		if (name##_reserve(v, 1) < 0)                                                                                \
			return -ENOMEM;                                                                                      \
		v->p[v->n++] = x;                                                                                            \
		return 0;                                                                                                    \
	}                                                                                                                    \
	static inline bool name##_pop(name *v, T *out) {                                                                     \
		if (!v->n)                                                                                                   \
			return false;                                                                                        \
		v->n--;                                                                                                      \
		if (out)                                                                                                     \
			*out = v->p[v->n];                                                                                   \
		return true;                                                                                                 \
	}                                                                                                                    \
	static inline int name##_insert(name *v, size_t i, T x) {                                                            \
		if (name##_reserve(v, 1) < 0)                                                                                \
			return -ENOMEM;                                                                                      \
		memmove(v->p + i + 1, v->p + i, (v->n - i) * sizeof(T));                                                     \
		v->p[i] = x;                                                                                                 \
		v->n++;                                                                                                      \
		return 0;                                                                                                    \
	}                                                                                                                    \
	static inline void name##_delete(name *v, size_t i, size_t j) {                                                      \
		memmove(v->p + i, v->p + j, (v->n - j) * sizeof(T));                                                         \
		v->n -= j - i;                                                                                               \
	}                                                                                                                    \
	static inline void name##_free(name *v) {                                                                            \
		free(v->p);                                                                                                  \
		v->p = 0;                                                                                                    \
		v->n = v->cap = 0;                                                                                           \
	}

#ifdef __cplusplus
}
#endif

#endif
