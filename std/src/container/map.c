/* Hash maps: open addressing with Robin Hood probing and backward-shift
   deletion; the load stays under 7/8 */
#include <alt/crypto/hash.h>
#include <alt/map.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned __int128 u128;

static uint64_t mix(uint64_t a, uint64_t b) {
	u128 r = (u128)a * b;
	return (uint64_t)r ^ (uint64_t)(r >> 64);
}

static const uint64_t P0 = 0xa0761d6478bd642full, P1 = 0xe7037ed1a0b428dbull, P2 = 0x8ebc6af09c88c6e3ull;

uint64_t alt_hash_bytes(const void *data, size_t n, uint64_t seed) {
	const uint8_t *p = data;
	uint64_t h = seed ^ P0, len = n;
	while (n >= 16) {
		uint64_t a, b;
		memcpy(&a, p, 8);
		memcpy(&b, p + 8, 8);
		h = mix(a ^ P1, b ^ h);
		p += 16, n -= 16;
	}
	uint64_t a = 0, b = 0;
	if (n >= 8) {
		memcpy(&a, p, 8);
		memcpy(&b, p + n - 8, 8);
	} else if (n >= 4) {
		uint32_t x, y;
		memcpy(&x, p, 4);
		memcpy(&y, p + n - 4, 4);
		a = x, b = y;
	} else if (n > 0)
		a = (uint64_t)p[0] << 16 | (uint64_t)p[n / 2] << 8 | p[n - 1];
	return mix(mix(a ^ P1, b ^ h) ^ len, P2);
}

uint64_t alt_hash_strp(const void *key, uint64_t seed) {
	const alt_str *s = key;
	return alt_hash_bytes(s->p, s->n, seed);
}

bool alt_eq_strp(const void *a, const void *b) { return alt_str_eq(*(const alt_str *)a, *(const alt_str *)b); }
uint64_t alt_hash_u64p(const void *key, uint64_t seed) { return mix(*(const uint64_t *)key ^ P0 ^ seed, P1); }
bool alt_eq_u64p(const void *a, const void *b) { return *(const uint64_t *)a == *(const uint64_t *)b; }

static size_t align8(size_t n) { return (n + 7) & ~(size_t)7; }

int alt_map_init(alt_map *m, size_t key_size, size_t value_size, alt_hash_fn hash, alt_eq_fn eq) {
	memset(m, 0, sizeof *m);
	m->key_size = key_size;
	m->value_size = value_size;
	m->key_off = 8;
	m->value_off = 8 + align8(key_size);
	m->slot_size = m->value_off + align8(value_size);
	m->hash = hash;
	m->eq = eq;
	return alt_crypto_rand(&m->seed, sizeof m->seed) < 0 ? -EIO : 0;
}

void alt_map_free(alt_map *m) {
	free(m->slots);
	free(m->hashes);
	m->slots = 0;
	m->hashes = 0;
	m->cap = m->n = 0;
}

size_t alt_map_len(const alt_map *m) { return m->n; }

static uint64_t *dist(const alt_map *m, size_t i) { return (uint64_t *)(m->slots + i * m->slot_size); }
static void *key_at(const alt_map *m, size_t i) { return m->slots + i * m->slot_size + m->key_off; }
static void *value_at(const alt_map *m, size_t i) { return m->slots + i * m->slot_size + m->value_off; }

/* the slot of key, or SIZE_MAX */
static size_t find(const alt_map *m, const void *key, uint64_t h) {
	if (!m->cap)
		return SIZE_MAX;
	size_t mask = m->cap - 1, i = h & mask;
	uint32_t hs = (uint32_t)(h >> 32);
	for (uint64_t d = 1;; d++, i = (i + 1) & mask) {
		uint64_t di = *dist(m, i);
		if (di < d)
			return SIZE_MAX; /* an empty slot or a richer one: key would be before it */
		if (m->hashes[i] == hs && m->eq(key_at(m, i), key))
			return i;
	}
}

void *alt_map_get(const alt_map *m, const void *key) {
	size_t i = find(m, key, m->cap ? m->hash(key, m->seed) : 0);
	return i == SIZE_MAX ? 0 : value_at(m, i);
}

/* places a slot's contents (dist, hash, key, value) by Robin Hood: the index where the new one went */
static size_t place(alt_map *m, uint8_t *slot, uint32_t hs, uint64_t h, size_t want_index_of_first) {
	(void)want_index_of_first;
	size_t mask = m->cap - 1, i = h & mask, placed = SIZE_MAX;
	uint8_t *tmp = slot + m->slot_size; /* scratch after the carried slot */
	uint64_t d = 1;
	memcpy(slot, &d, 8);
	for (;; i = (i + 1) & mask) {
		uint64_t di = *dist(m, i), cur;
		memcpy(&cur, slot, 8);
		if (di == 0) {
			memcpy(m->slots + i * m->slot_size, slot, m->slot_size);
			m->hashes[i] = hs;
			return placed == SIZE_MAX ? i : placed;
		}
		if (di < cur) {
			/* swap: the carried one stays here, the poorer one moves on */
			memcpy(tmp, m->slots + i * m->slot_size, m->slot_size);
			memcpy(m->slots + i * m->slot_size, slot, m->slot_size);
			memcpy(slot, tmp, m->slot_size);
			uint32_t th = m->hashes[i];
			m->hashes[i] = hs;
			hs = th;
			if (placed == SIZE_MAX)
				placed = i;
			cur = di;
		}
		cur++;
		memcpy(slot, &cur, 8);
	}
}

static int grow(alt_map *m) {
	size_t ncap = m->cap ? m->cap * 2 : 8;
	uint8_t *old = m->slots;
	uint32_t *oldh = m->hashes;
	size_t ocap = m->cap;
	uint8_t *ns = calloc(ncap, m->slot_size);
	uint32_t *nh = calloc(ncap, sizeof *nh);
	uint8_t *carry = malloc(2 * m->slot_size);
	if (!ns || !nh || !carry) {
		free(ns), free(nh), free(carry);
		return -ENOMEM;
	}
	m->slots = ns;
	m->hashes = nh;
	m->cap = ncap;
	for (size_t i = 0; i < ocap; i++) {
		uint8_t *s = old + i * m->slot_size;
		uint64_t di;
		memcpy(&di, s, 8);
		if (!di)
			continue;
		memcpy(carry, s, m->slot_size);
		uint64_t h = m->hash(carry + m->key_off, m->seed);
		place(m, carry, (uint32_t)(h >> 32), h, 0);
	}
	free(carry);
	free(old);
	free(oldh);
	return 0;
}

void *alt_map_upsert(alt_map *m, const void *key, bool *inserted) {
	uint64_t h = m->hash(key, m->seed);
	size_t i = find(m, key, h);
	if (i != SIZE_MAX) {
		if (inserted)
			*inserted = false;
		return value_at(m, i);
	}
	if ((m->n + 1) * 8 > m->cap * 7) {
		if (grow(m) < 0)
			return 0;
	}
	uint8_t *carry = calloc(2, m->slot_size);
	if (!carry)
		return 0;
	memcpy(carry + m->key_off, key, m->key_size);
	i = place(m, carry, (uint32_t)(h >> 32), h, 0);
	free(carry);
	m->n++;
	if (inserted)
		*inserted = true;
	return value_at(m, i);
}

int alt_map_put(alt_map *m, const void *key, const void *value) {
	void *v = alt_map_upsert(m, key, 0);
	if (!v)
		return -ENOMEM;
	memcpy(v, value, m->value_size);
	return 0;
}

bool alt_map_delete(alt_map *m, const void *key) {
	if (!m->cap)
		return false;
	size_t i = find(m, key, m->hash(key, m->seed));
	if (i == SIZE_MAX)
		return false;
	/* backward shift: the following displaced slots move one closer to home */
	size_t mask = m->cap - 1;
	for (;;) {
		size_t j = (i + 1) & mask;
		uint64_t dj = *dist(m, j);
		if (dj <= 1) {
			memset(m->slots + i * m->slot_size, 0, m->slot_size);
			m->hashes[i] = 0;
			break;
		}
		memcpy(m->slots + i * m->slot_size, m->slots + j * m->slot_size, m->slot_size);
		*dist(m, i) = dj - 1;
		m->hashes[i] = m->hashes[j];
		i = j;
	}
	m->n--;
	return true;
}

void alt_map_clear(alt_map *m) {
	if (m->cap) {
		memset(m->slots, 0, m->cap * m->slot_size);
		memset(m->hashes, 0, m->cap * sizeof *m->hashes);
	}
	m->n = 0;
}

bool alt_map_next(const alt_map *m, size_t *it, void **key, void **value) {
	for (size_t i = *it; i < m->cap; i++)
		if (*dist(m, i)) {
			if (key)
				*key = key_at(m, i);
			if (value)
				*value = value_at(m, i);
			*it = i + 1;
			return true;
		}
	*it = m->cap;
	return false;
}
