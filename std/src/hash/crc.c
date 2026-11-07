/* CRC-32 and CRC-64 with slicing-by-8 tables, Adler-32 (RFC 1950) */
#include <alt/hash/adler32.h>
#include <alt/hash/crc32.h>
#include <alt/hash/crc64.h>
#include <alt/sync.h>
#include <stdlib.h>
#include <string.h>

struct alt_crc32_table {
	uint32_t poly;
	uint32_t t[8][256];
	struct alt_crc32_table *next;
};

struct alt_crc64_table {
	uint64_t poly;
	uint64_t t[8][256];
	struct alt_crc64_table *next;
};

static alt_mutex tables_mu = ALT_MUTEX_INIT;
static struct alt_crc32_table *tables32;
static struct alt_crc64_table *tables64;
static const alt_crc32_table *_Atomic ieee;

const alt_crc32_table *alt_crc32_make_table(uint32_t poly) {
	alt_mutex_lock(&tables_mu);
	struct alt_crc32_table *t;
	for (t = tables32; t; t = t->next)
		if (t->poly == poly)
			break;
	if (!t && (t = malloc(sizeof *t))) {
		t->poly = poly;
		for (int i = 0; i < 256; i++) {
			uint32_t c = (uint32_t)i;
			for (int k = 0; k < 8; k++)
				c = c & 1 ? (c >> 1) ^ poly : c >> 1;
			t->t[0][i] = c;
		}
		for (int i = 0; i < 256; i++)
			for (int k = 1; k < 8; k++)
				t->t[k][i] = (t->t[k - 1][i] >> 8) ^ t->t[0][t->t[k - 1][i] & 0xff];
		t->next = tables32;
		tables32 = t;
	}
	alt_mutex_unlock(&tables_mu);
	return t;
}

uint32_t alt_crc32_update(uint32_t crc, const alt_crc32_table *tab, const void *p, size_t n) {
	const unsigned char *s = p;
	uint32_t c = ~crc;
	const uint32_t(*t)[256] = tab->t;
	while (n >= 8) {
		uint32_t lo = c ^ ((uint32_t)s[0] | (uint32_t)s[1] << 8 | (uint32_t)s[2] << 16 | (uint32_t)s[3] << 24);
		c = t[7][lo & 0xff] ^ t[6][(lo >> 8) & 0xff] ^ t[5][(lo >> 16) & 0xff] ^ t[4][lo >> 24] ^ t[3][s[4]] ^ t[2][s[5]] ^
		    t[1][s[6]] ^ t[0][s[7]];
		s += 8, n -= 8;
	}
	while (n--)
		c = t[0][(c ^ *s++) & 0xff] ^ (c >> 8);
	return ~c;
}

uint32_t alt_crc32_checksum(const void *p, size_t n, const alt_crc32_table *tab) { return alt_crc32_update(0, tab, p, n); }

uint32_t alt_crc32_ieee_update(uint32_t crc, const void *p, size_t n) {
	const alt_crc32_table *t = atomic_load(&ieee);
	if (!t) {
		t = alt_crc32_make_table(ALT_CRC32_IEEE);
		if (!t)
			abort();
		atomic_store(&ieee, t);
	}
	return alt_crc32_update(crc, t, p, n);
}

uint32_t alt_crc32_ieee(const void *p, size_t n) { return alt_crc32_ieee_update(0, p, n); }

const alt_crc64_table *alt_crc64_make_table(uint64_t poly) {
	alt_mutex_lock(&tables_mu);
	struct alt_crc64_table *t;
	for (t = tables64; t; t = t->next)
		if (t->poly == poly)
			break;
	if (!t && (t = malloc(sizeof *t))) {
		t->poly = poly;
		for (int i = 0; i < 256; i++) {
			uint64_t c = (uint64_t)i;
			for (int k = 0; k < 8; k++)
				c = c & 1 ? (c >> 1) ^ poly : c >> 1;
			t->t[0][i] = c;
		}
		for (int i = 0; i < 256; i++)
			for (int k = 1; k < 8; k++)
				t->t[k][i] = (t->t[k - 1][i] >> 8) ^ t->t[0][t->t[k - 1][i] & 0xff];
		t->next = tables64;
		tables64 = t;
	}
	alt_mutex_unlock(&tables_mu);
	return t;
}

uint64_t alt_crc64_update(uint64_t crc, const alt_crc64_table *tab, const void *p, size_t n) {
	const unsigned char *s = p;
	uint64_t c = ~crc;
	const uint64_t(*t)[256] = tab->t;
	while (n >= 8) {
		uint64_t v;
		memcpy(&v, s, 8); /* little endian */
		c ^= v;
		c = t[7][c & 0xff] ^ t[6][(c >> 8) & 0xff] ^ t[5][(c >> 16) & 0xff] ^ t[4][(c >> 24) & 0xff] ^
		    t[3][(c >> 32) & 0xff] ^ t[2][(c >> 40) & 0xff] ^ t[1][(c >> 48) & 0xff] ^ t[0][c >> 56];
		s += 8, n -= 8;
	}
	while (n--)
		c = t[0][(c ^ *s++) & 0xff] ^ (c >> 8);
	return ~c;
}

uint64_t alt_crc64_checksum(const void *p, size_t n, const alt_crc64_table *tab) { return alt_crc64_update(0, tab, p, n); }

/* Adler-32: sums modulo 65521, reduced every 5552 bytes (the most that
   cannot overflow 32 bits) */
uint32_t alt_adler32_update(uint32_t adler, const void *p, size_t n) {
	enum { MOD = 65521, NMAX = 5552 };
	const unsigned char *s = p;
	uint32_t a = adler & 0xffff, b = adler >> 16;
	while (n) {
		size_t k = n < NMAX ? n : NMAX;
		n -= k;
		while (k--) {
			a += *s++;
			b += a;
		}
		a %= MOD;
		b %= MOD;
	}
	return b << 16 | a;
}
