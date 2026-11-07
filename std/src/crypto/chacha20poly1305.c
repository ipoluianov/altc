/* ChaCha20, Poly1305 and ChaCha20-Poly1305 (RFC 8439), XChaCha20
   (draft-irtf-cfrg-xchacha). Poly1305 with 44-bit limbs (radix 2^44,
   2^44, 2^42), constant time. */
#include <alt/crypto/chacha20poly1305.h>
#include <string.h>

static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void put_le32(uint8_t *p, uint32_t v) {
	p[0] = (uint8_t)v, p[1] = (uint8_t)(v >> 8), p[2] = (uint8_t)(v >> 16), p[3] = (uint8_t)(v >> 24);
}
static uint64_t le64(const uint8_t *p) { return (uint64_t)le32(p) | (uint64_t)le32(p + 4) << 32; }

#define ROTL(x, n) ((x) << (n) | (x) >> (32 - (n)))
#define QR(a, b, c, d) \
	(a += b, d ^= a, d = ROTL(d, 16), c += d, b ^= c, b = ROTL(b, 12), a += b, d ^= a, d = ROTL(d, 8), c += d, b ^= c, \
		b = ROTL(b, 7))

static void rounds(uint32_t x[16]) {
	for (int i = 0; i < 10; i++) {
		QR(x[0], x[4], x[8], x[12]);
		QR(x[1], x[5], x[9], x[13]);
		QR(x[2], x[6], x[10], x[14]);
		QR(x[3], x[7], x[11], x[15]);
		QR(x[0], x[5], x[10], x[15]);
		QR(x[1], x[6], x[11], x[12]);
		QR(x[2], x[7], x[8], x[13]);
		QR(x[3], x[4], x[9], x[14]);
	}
}

static void setup(uint32_t s[16], const uint8_t key[32]) {
	s[0] = 0x61707865, s[1] = 0x3320646e, s[2] = 0x79622d32, s[3] = 0x6b206574;
	for (int i = 0; i < 8; i++)
		s[4 + i] = le32(key + 4 * i);
}

void alt_chacha20_init(alt_chacha20 *c, const uint8_t key[32], const uint8_t nonce[12], uint32_t counter) {
	setup(c->s, key);
	c->s[12] = counter;
	for (int i = 0; i < 3; i++)
		c->s[13 + i] = le32(nonce + 4 * i);
	c->used = 64;
}

static void block(alt_chacha20 *c) {
	uint32_t x[16];
	memcpy(x, c->s, sizeof x);
	rounds(x);
	for (int i = 0; i < 16; i++)
		put_le32(c->ks + 4 * i, x[i] + c->s[i]);
	c->s[12]++;
	c->used = 0;
}

void alt_chacha20_xor(alt_chacha20 *c, void *dst, const void *src, size_t n) {
	uint8_t *d = dst;
	const uint8_t *p = src;
	while (n) {
		if (c->used == 64)
			block(c);
		size_t k = 64 - c->used < n ? 64 - c->used : n;
		for (size_t i = 0; i < k; i++)
			d[i] = p[i] ^ c->ks[c->used + i];
		c->used += k;
		d += k, p += k, n -= k;
	}
}

void alt_hchacha20(uint8_t out[32], const uint8_t key[32], const uint8_t nonce[16]) {
	uint32_t x[16];
	setup(x, key);
	for (int i = 0; i < 4; i++)
		x[12 + i] = le32(nonce + 4 * i);
	rounds(x);
	for (int i = 0; i < 4; i++) {
		put_le32(out + 4 * i, x[i]);
		put_le32(out + 16 + 4 * i, x[12 + i]);
	}
}

/* ---- Poly1305 ---- */

#define M44 ((1ull << 44) - 1)
#define M42 ((1ull << 42) - 1)

void alt_poly1305_init(alt_poly1305 *p, const uint8_t key[32]) {
	uint64_t t0 = le64(key) & 0x0ffffffc0fffffffull, t1 = le64(key + 8) & 0x0ffffffc0ffffffcull;
	p->r[0] = t0 & M44;
	p->r[1] = (t0 >> 44 | t1 << 20) & M44;
	p->r[2] = (t1 >> 24) & M42;
	p->h[0] = p->h[1] = p->h[2] = 0;
	p->pad[0] = le64(key + 16);
	p->pad[1] = le64(key + 24);
	p->nbuf = 0;
}

static void blocks(alt_poly1305 *p, const uint8_t *m, size_t n, uint64_t hibit) {
	uint64_t r0 = p->r[0], r1 = p->r[1], r2 = p->r[2];
	uint64_t s1 = r1 * (5 << 2), s2 = r2 * (5 << 2);
	uint64_t h0 = p->h[0], h1 = p->h[1], h2 = p->h[2];
	for (; n >= 16; m += 16, n -= 16) {
		uint64_t t0 = le64(m), t1 = le64(m + 8);
		h0 += t0 & M44;
		h1 += (t0 >> 44 | t1 << 20) & M44;
		h2 += ((t1 >> 24) & M42) | hibit << 40;
		unsigned __int128 d0 = (unsigned __int128)h0 * r0 + (unsigned __int128)h1 * s2 + (unsigned __int128)h2 * s1;
		unsigned __int128 d1 = (unsigned __int128)h0 * r1 + (unsigned __int128)h1 * r0 + (unsigned __int128)h2 * s2;
		unsigned __int128 d2 = (unsigned __int128)h0 * r2 + (unsigned __int128)h1 * r1 + (unsigned __int128)h2 * r0;
		uint64_t c = (uint64_t)(d0 >> 44);
		h0 = (uint64_t)d0 & M44;
		d1 += c;
		c = (uint64_t)(d1 >> 44);
		h1 = (uint64_t)d1 & M44;
		d2 += c;
		c = (uint64_t)(d2 >> 42);
		h2 = (uint64_t)d2 & M42;
		h0 += c * 5;
		c = h0 >> 44;
		h0 &= M44;
		h1 += c;
	}
	p->h[0] = h0, p->h[1] = h1, p->h[2] = h2;
}

void alt_poly1305_write(alt_poly1305 *p, const void *msg, size_t n) {
	const uint8_t *m = msg;
	if (p->nbuf) {
		size_t k = 16 - p->nbuf < n ? 16 - p->nbuf : n;
		memcpy(p->buf + p->nbuf, m, k);
		p->nbuf += k, m += k, n -= k;
		if (p->nbuf < 16)
			return;
		blocks(p, p->buf, 16, 1);
		p->nbuf = 0;
	}
	size_t full = n & ~(size_t)15;
	blocks(p, m, full, 1);
	m += full, n -= full;
	memcpy(p->buf, m, n);
	p->nbuf = n;
}

void alt_poly1305_sum(alt_poly1305 *p, uint8_t tag[16]) {
	if (p->nbuf) {
		p->buf[p->nbuf] = 1;
		memset(p->buf + p->nbuf + 1, 0, 16 - p->nbuf - 1);
		blocks(p, p->buf, 16, 0);
	}
	uint64_t h0 = p->h[0], h1 = p->h[1], h2 = p->h[2], c;
	/* full carry */
	c = h1 >> 44, h1 &= M44, h2 += c;
	c = h2 >> 42, h2 &= M42, h0 += c * 5;
	c = h0 >> 44, h0 &= M44, h1 += c;
	c = h1 >> 44, h1 &= M44, h2 += c;
	c = h2 >> 42, h2 &= M42, h0 += c * 5;
	c = h0 >> 44, h0 &= M44, h1 += c;
	/* h - p = h + 5 - 2^130: take it when it does not borrow */
	uint64_t g0 = h0 + 5;
	c = g0 >> 44, g0 &= M44;
	uint64_t g1 = h1 + c;
	c = g1 >> 44, g1 &= M44;
	uint64_t g2 = h2 + c - (1ull << 42);
	uint64_t mask = (g2 >> 63) - 1; /* all ones when g2 did not go negative */
	h0 = (h0 & ~mask) | (g0 & mask);
	h1 = (h1 & ~mask) | (g1 & mask);
	h2 = (h2 & ~mask) | (g2 & mask);
	/* h + pad mod 2^128 */
	uint64_t lo = h0 | h1 << 44, hi = h1 >> 20 | h2 << 24;
	unsigned __int128 t = (unsigned __int128)lo + p->pad[0];
	lo = (uint64_t)t;
	hi = hi + p->pad[1] + (uint64_t)(t >> 64);
	for (int i = 0; i < 8; i++) {
		tag[i] = (uint8_t)(lo >> 8 * i);
		tag[8 + i] = (uint8_t)(hi >> 8 * i);
	}
	memset(p, 0, sizeof *p);
}

/* ---- the AEAD ---- */

static const uint8_t zeros[16];

static void aead_tag(alt_chacha20 *c, const uint8_t *ad, size_t nad, const uint8_t *ct, size_t n, uint8_t tag[16]) {
	uint8_t pk[64] = {0};
	alt_chacha20_xor(c, pk, pk, 64); /* block 0: the Poly1305 key */
	alt_poly1305 p;
	alt_poly1305_init(&p, pk);
	alt_poly1305_write(&p, ad, nad);
	alt_poly1305_write(&p, zeros, (16 - nad % 16) % 16);
	alt_poly1305_write(&p, ct, n);
	alt_poly1305_write(&p, zeros, (16 - n % 16) % 16);
	uint8_t lens[16];
	for (int i = 0; i < 8; i++) {
		lens[i] = (uint8_t)((uint64_t)nad >> 8 * i);
		lens[8 + i] = (uint8_t)((uint64_t)n >> 8 * i);
	}
	alt_poly1305_write(&p, lens, 16);
	alt_poly1305_sum(&p, tag);
	memset(pk, 0, sizeof pk);
}

void alt_chacha20poly1305_seal(const uint8_t key[32], const uint8_t *nonce, void *dst, const void *pt, size_t n, const void *ad,
	size_t nad) {
	alt_chacha20 c, k;
	alt_chacha20_init(&c, key, nonce, 1);
	alt_chacha20_init(&k, key, nonce, 0);
	uint8_t *d = dst;
	alt_chacha20_xor(&c, d, pt, n);
	aead_tag(&k, ad, nad, d, n, d + n);
	memset(&c, 0, sizeof c);
}

int alt_chacha20poly1305_open(const uint8_t key[32], const uint8_t *nonce, void *dst, const void *ct, size_t n, const void *ad,
	size_t nad) {
	if (n < 16)
		return -ALT_EAUTH;
	n -= 16;
	alt_chacha20 c, k;
	alt_chacha20_init(&k, key, nonce, 0);
	const uint8_t *s = ct;
	uint8_t t[16], diff = 0;
	aead_tag(&k, ad, nad, s, n, t);
	for (int i = 0; i < 16; i++)
		diff |= t[i] ^ s[n + i];
	if (diff) {
		memset(dst, 0, n);
		return -ALT_EAUTH;
	}
	alt_chacha20_init(&c, key, nonce, 1);
	alt_chacha20_xor(&c, dst, s, n);
	memset(&c, 0, sizeof c);
	return 0;
}

static void xkey(const uint8_t key[32], const uint8_t nonce[24], uint8_t sub[32], uint8_t n12[12]) {
	alt_hchacha20(sub, key, nonce);
	memset(n12, 0, 4);
	memcpy(n12 + 4, nonce + 16, 8);
}

void alt_xchacha20poly1305_seal(const uint8_t key[32], const uint8_t nonce[24], void *dst, const void *pt, size_t n,
	const void *ad, size_t nad) {
	uint8_t sub[32], n12[12];
	xkey(key, nonce, sub, n12);
	alt_chacha20poly1305_seal(sub, n12, dst, pt, n, ad, nad);
	memset(sub, 0, sizeof sub);
}

int alt_xchacha20poly1305_open(const uint8_t key[32], const uint8_t nonce[24], void *dst, const void *ct, size_t n,
	const void *ad, size_t nad) {
	uint8_t sub[32], n12[12];
	xkey(key, nonce, sub, n12);
	int rc = alt_chacha20poly1305_open(sub, n12, dst, ct, n, ad, nad);
	memset(sub, 0, sizeof sub);
	return rc;
}
