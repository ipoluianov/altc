/* maps, vectors and sorting against references (also under the sanitizers) */
#include "check.h"
#include <alt/map.h>
#include <alt/sort.h>
#include <alt/time.h>
#include <alt/vec.h>

ALT_VEC(ints, int64_t)
ALT_MAP(imap, uint64_t, int64_t, alt_hash_u64p, alt_eq_u64p)
ALT_MAP(smap, alt_str, int, alt_hash_strp, alt_eq_strp)

static uint64_t seed = 0x9e3779b97f4a7c15ull;
static uint64_t rnd(void) {
	seed ^= seed << 13, seed ^= seed >> 7, seed ^= seed << 17;
	return seed;
}

static void maps(void) {
	enum { KEYS = 5000 };
	static int64_t ref[KEYS];
	static bool has[KEYS];
	imap m;
	CHECK_OK(imap_init(&m));
	size_t count = 0;
	for (int i = 0; i < 200000; i++) {
		uint64_t k = rnd() % KEYS;
		switch (rnd() % 4) {
		case 0:
		case 1: {
			int64_t v = (int64_t)rnd();
			CHECK_OK(imap_put(&m, k, v));
			if (!has[k])
				count++;
			has[k] = true, ref[k] = v;
			break;
		}
		case 2: {
			bool d = imap_delete(&m, k);
			CHECK(d == has[k]);
			if (has[k])
				count--;
			has[k] = false;
			break;
		}
		default: {
			int64_t *v = imap_get(&m, k);
			CHECK((v != 0) == has[k]);
			if (v && has[k])
				CHECK(*v == ref[k]);
		}
		}
		if (i % 10000 == 0)
			CHECK_INT(imap_len(&m), count);
	}
	/* iteration sees every key once */
	size_t it = 0, seen = 0;
	uint64_t *k;
	int64_t *v;
	while (imap_next(&m, &it, &k, &v)) {
		CHECK(*k < KEYS && has[*k] && *v == ref[*k]);
		seen++;
	}
	CHECK_INT(seen, count);
	imap_free(&m);
	/* string keys, upsert */
	smap s;
	smap_init(&s);
	const char *words[] = {"go", "c", "go", "rust", "c", "go", ""};
	for (size_t i = 0; i < 7; i++) {
		bool ins;
		int *c = smap_upsert(&s, alt_str_cstr(words[i]), &ins);
		CHECK(c != 0);
		++*c;
	}
	CHECK_INT(smap_len(&s), 4);
	CHECK_INT(*smap_get(&s, ALT_STR("go")), 3);
	CHECK_INT(*smap_get(&s, ALT_STR("")), 1);
	CHECK(smap_get(&s, ALT_STR("zig")) == 0);
	smap_free(&s);
}

static void vectors(void) {
	ints v = {0};
	for (int64_t i = 0; i < 1000; i++)
		CHECK_OK(ints_push(&v, i));
	ints_insert(&v, 0, -1);
	ints_delete(&v, 10, 20);
	CHECK_INT(v.n, 991);
	CHECK(v.p[0] == -1 && v.p[10] == 19);
	int64_t last;
	CHECK(ints_pop(&v, &last) && last == 999);
	ints_free(&v);
}

static void sorting(void) {
	enum { N = 200000 };
	int64_t *a = malloc(N * sizeof *a);
	for (int pattern = 0; pattern < 6; pattern++) {
		for (int64_t i = 0; i < N; i++)
			a[i] = pattern == 0 ? (int64_t)(rnd() % 1000000) : pattern == 1 ? i : pattern == 2 ? N - i : pattern == 3 ? 5
				: pattern == 4 ? (i < N / 2 ? i : N - i) : (int64_t)(rnd() % 3);
		alt_time t0 = alt_now();
		alt_sort_int64s(a, N);
		CHECK(alt_since(t0) < 2 * ALT_SEC);
		CHECK(alt_is_sorted(a, N, sizeof *a, alt_cmp_int64, 0));
		for (int64_t i = 0; i < N; i++)
			a[i] = pattern == 0 ? (int64_t)(rnd() % 1000) : N - i;
		t0 = alt_now();
		alt_sort_stable(a, N, sizeof *a, alt_cmp_int64, 0);
		CHECK(alt_since(t0) < 4 * ALT_SEC);
		CHECK(alt_is_sorted(a, N, sizeof *a, alt_cmp_int64, 0));
	}
	free(a);
	double d[] = {3, 0.0 / 0.0, -1, 2, 0.0 / 0.0};
	alt_sort_doubles(d, 5);
	CHECK(d[0] != d[0] && d[1] != d[1] && d[2] == -1 && d[4] == 3);
}

int main(void) {
	RUN(maps);
	RUN(vectors);
	RUN(sorting);
	return check_done();
}
