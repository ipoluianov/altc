/* AltC standard library: hash maps (Go's map)

	ALT_MAP(counts, alt_str, int64_t, alt_hash_strp, alt_eq_strp);
	counts m;
	counts_init(&m);
	int64_t *v = counts_upsert(&m, ALT_STR("go"), 0);
	++*v;
	int64_t *x = counts_get(&m, ALT_STR("go"));
	counts_free(&m);

   Keys and values are stored by value (the map does not own what they
   point to: an alt_str key's bytes must outlive it). Open addressing
   with Robin Hood probing; each map has a random hash seed, so the
   iteration order is unspecified (as Go's). */
#ifndef ALT_MAP_H
#define ALT_MAP_H

#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t (*alt_hash_fn)(const void *key, uint64_t seed);
typedef bool (*alt_eq_fn)(const void *a, const void *b);

typedef struct {
	uint8_t *slots;           /* per slot: the distance + 1 (0: empty), then key, value */
	size_t cap, n;
	size_t key_size, value_size, slot_size, key_off, value_off;
	uint32_t *hashes;         /* the low bits of the hash of each slot */
	uint64_t seed;
	alt_hash_fn hash;
	alt_eq_fn eq;
} alt_map;

int alt_map_init(alt_map *m, size_t key_size, size_t value_size, alt_hash_fn hash, alt_eq_fn eq);
void alt_map_free(alt_map *m);
size_t alt_map_len(const alt_map *m);
/* the value of key, NULL when absent (valid until the map changes) */
void *alt_map_get(const alt_map *m, const void *key);
/* the value slot of key, inserted (zeroed) when absent: NULL on ENOMEM */
void *alt_map_upsert(alt_map *m, const void *key, bool *inserted);
int alt_map_put(alt_map *m, const void *key, const void *value);
bool alt_map_delete(alt_map *m, const void *key);
void alt_map_clear(alt_map *m);
/* iteration: it starts at 0; false at the end (the map unchanged meanwhile) */
bool alt_map_next(const alt_map *m, size_t *it, void **key, void **value);

/* hashes for common keys (keys passed by pointer) */
uint64_t alt_hash_bytes(const void *p, size_t n, uint64_t seed);
uint64_t alt_hash_strp(const void *key, uint64_t seed);  /* alt_str */
bool alt_eq_strp(const void *a, const void *b);
uint64_t alt_hash_u64p(const void *key, uint64_t seed);  /* uint64_t, int64_t, pointers as uintptr_t */
bool alt_eq_u64p(const void *a, const void *b);

/* a typed map: name##_init, _get, _upsert, _put, _delete, _len, _next, _free */
#define ALT_MAP(name, K, V, hashfn, eqfn)                                                                                    \
	typedef struct {                                                                                                     \
		alt_map m;                                                                                                   \
	} name;                                                                                                              \
	static inline int name##_init(name *x) { return alt_map_init(&x->m, sizeof(K), sizeof(V), hashfn, eqfn); }           \
	static inline void name##_free(name *x) { alt_map_free(&x->m); }                                                     \
	static inline size_t name##_len(const name *x) { return alt_map_len(&x->m); }                                        \
	static inline V *name##_get(const name *x, K k) { return (V *)alt_map_get(&x->m, &k); }                              \
	static inline V *name##_upsert(name *x, K k, bool *inserted) { return (V *)alt_map_upsert(&x->m, &k, inserted); }     \
	static inline int name##_put(name *x, K k, V v) { return alt_map_put(&x->m, &k, &v); }                               \
	static inline bool name##_delete(name *x, K k) { return alt_map_delete(&x->m, &k); }                                 \
	static inline bool name##_next(const name *x, size_t *it, K **k, V **v) {                                            \
		void *kk, *vv;                                                                                               \
		if (!alt_map_next(&x->m, it, &kk, &vv))                                                                      \
			return false;                                                                                        \
		if (k)                                                                                                       \
			*k = (K *)kk;                                                                                        \
		if (v)                                                                                                       \
			*v = (V *)vv;                                                                                        \
		return true;                                                                                                 \
	}

#ifdef __cplusplus
}
#endif

#endif
