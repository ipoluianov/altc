/* MD5 (RFC 1321), SHA-1 and the SHA-2 family (FIPS 180-4), HMAC (RFC 2104) */
#include <alt/crypto/hash.h>
#include <errno.h>
#include <string.h>
#include <sys/random.h>

static uint32_t rol32(uint32_t x, int n) { return x << n | x >> (32 - n); }
static uint32_t ror32(uint32_t x, int n) { return x >> n | x << (32 - n); }
static uint64_t ror64(uint64_t x, int n) { return x >> n | x << (64 - n); }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint32_t le32(const uint8_t *p) { return (uint32_t)p[3] << 24 | (uint32_t)p[2] << 16 | (uint32_t)p[1] << 8 | p[0]; }
static uint64_t be64(const uint8_t *p) { return (uint64_t)be32(p) << 32 | be32(p + 4); }

/* ---- MD5 ---- */

static const uint32_t md5_k[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
	0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
	0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
	0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};
static const int md5_s[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	5, 9, 14, 20, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10,
	15, 21};

static void md5_block(uint32_t *h, const uint8_t *p) {
	uint32_t m[16];
	for (int i = 0; i < 16; i++)
		m[i] = le32(p + 4 * i);
	uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
	for (int i = 0; i < 64; i++) {
		uint32_t f;
		int g;
		if (i < 16)
			f = (b & c) | (~b & d), g = i;
		else if (i < 32)
			f = (d & b) | (~d & c), g = (5 * i + 1) % 16;
		else if (i < 48)
			f = b ^ c ^ d, g = (3 * i + 5) % 16;
		else
			f = c ^ (b | ~d), g = (7 * i) % 16;
		uint32_t t = d;
		d = c;
		c = b;
		b = b + rol32(a + f + md5_k[i] + m[g], md5_s[i]);
		a = t;
	}
	h[0] += a, h[1] += b, h[2] += c, h[3] += d;
}

/* ---- SHA-1 ---- */

static void sha1_block(uint32_t *h, const uint8_t *p) {
	uint32_t w[80];
	for (int i = 0; i < 16; i++)
		w[i] = be32(p + 4 * i);
	for (int i = 16; i < 80; i++)
		w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
	for (int i = 0; i < 80; i++) {
		uint32_t f, k;
		if (i < 20)
			f = (b & c) | (~b & d), k = 0x5A827999;
		else if (i < 40)
			f = b ^ c ^ d, k = 0x6ED9EBA1;
		else if (i < 60)
			f = (b & c) | (b & d) | (c & d), k = 0x8F1BBCDC;
		else
			f = b ^ c ^ d, k = 0xCA62C1D6;
		uint32_t t = rol32(a, 5) + f + e + k + w[i];
		e = d;
		d = c;
		c = rol32(b, 30);
		b = a;
		a = t;
	}
	h[0] += a, h[1] += b, h[2] += c, h[3] += d, h[4] += e;
}

/* ---- SHA-256 ---- */

static const uint32_t k256[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
	0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
	0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static void sha256_block(uint32_t *h, const uint8_t *p) {
	uint32_t w[64];
	for (int i = 0; i < 16; i++)
		w[i] = be32(p + 4 * i);
	for (int i = 16; i < 64; i++) {
		uint32_t s0 = ror32(w[i - 15], 7) ^ ror32(w[i - 15], 18) ^ (w[i - 15] >> 3);
		uint32_t s1 = ror32(w[i - 2], 17) ^ ror32(w[i - 2], 19) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}
	uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
	for (int i = 0; i < 64; i++) {
		uint32_t S1 = ror32(e, 6) ^ ror32(e, 11) ^ ror32(e, 25);
		uint32_t ch = (e & f) ^ (~e & g);
		uint32_t t1 = hh + S1 + ch + k256[i] + w[i];
		uint32_t S0 = ror32(a, 2) ^ ror32(a, 13) ^ ror32(a, 22);
		uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
		uint32_t t2 = S0 + maj;
		hh = g, g = f, f = e, e = d + t1, d = c, c = b, b = a, a = t1 + t2;
	}
	h[0] += a, h[1] += b, h[2] += c, h[3] += d, h[4] += e, h[5] += f, h[6] += g, h[7] += hh;
}

/* ---- SHA-512 ---- */

static const uint64_t k512[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL,
	0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
	0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL, 0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
	0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL, 0x983e5152ee66dfabULL,
	0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL,
	0x53380d139d95b3dfULL, 0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
	0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL, 0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
	0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL,
	0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL, 0xca273eceea26619cULL,
	0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
	0x113f9804bef90daeULL, 0x1b710b35131c471bULL, 0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL,
	0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL,
};

static void sha512_block(uint64_t *h, const uint8_t *p) {
	uint64_t w[80];
	for (int i = 0; i < 16; i++)
		w[i] = be64(p + 8 * i);
	for (int i = 16; i < 80; i++) {
		uint64_t s0 = ror64(w[i - 15], 1) ^ ror64(w[i - 15], 8) ^ (w[i - 15] >> 7);
		uint64_t s1 = ror64(w[i - 2], 19) ^ ror64(w[i - 2], 61) ^ (w[i - 2] >> 6);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}
	uint64_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
	for (int i = 0; i < 80; i++) {
		uint64_t S1 = ror64(e, 14) ^ ror64(e, 18) ^ ror64(e, 41);
		uint64_t ch = (e & f) ^ (~e & g);
		uint64_t t1 = hh + S1 + ch + k512[i] + w[i];
		uint64_t S0 = ror64(a, 28) ^ ror64(a, 34) ^ ror64(a, 39);
		uint64_t maj = (a & b) ^ (a & c) ^ (b & c);
		hh = g, g = f, f = e, e = d + t1, d = c, c = b, b = a, a = t1 + S0 + maj;
	}
	for (int i = 0; i < 8; i++)
		h[i] += (i == 0 ? a : i == 1 ? b : i == 2 ? c : i == 3 ? d : i == 4 ? e : i == 5 ? f : i == 6 ? g : hh);
}

/* ---- the generic interface ---- */

size_t alt_hash_size(alt_hash_kind k) {
	static const size_t s[] = {16, 20, 28, 32, 48, 64, 32};
	return s[k];
}

size_t alt_hash_block_size(alt_hash_kind k) { return k >= ALT_SHA384 ? 128 : 64; }

void alt_hash_init(alt_hash *h, alt_hash_kind kind) {
	memset(h, 0, sizeof *h);
	h->kind = kind;
	static const uint32_t md5[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
	static const uint32_t sha1[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
	static const uint32_t s224[8] = {0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4};
	static const uint32_t s256[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	static const uint64_t s384[8] = {0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
		0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL, 0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL};
	static const uint64_t s512[8] = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
		0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
	static const uint64_t s512_256[8] = {0x22312194FC2BF72CULL, 0x9F555FA3C84C64C2ULL, 0x2393B86B6F53B151ULL, 0x963877195940EABDULL,
		0x96283EE2A88EFFE3ULL, 0xBE5E1E2553863992ULL, 0x2B0199FC2C85B8AAULL, 0x0EB72DDC81C52CA2ULL};
	switch (kind) {
	case ALT_MD5: memcpy(h->h32, md5, sizeof md5); break;
	case ALT_SHA1: memcpy(h->h32, sha1, sizeof sha1); break;
	case ALT_SHA224: memcpy(h->h32, s224, sizeof s224); break;
	case ALT_SHA256: memcpy(h->h32, s256, sizeof s256); break;
	case ALT_SHA384: memcpy(h->h64, s384, sizeof s384); break;
	case ALT_SHA512: memcpy(h->h64, s512, sizeof s512); break;
	case ALT_SHA512_256: memcpy(h->h64, s512_256, sizeof s512_256); break;
	}
}

static void block(alt_hash *h, const uint8_t *p) {
	switch (h->kind) {
	case ALT_MD5: md5_block(h->h32, p); break;
	case ALT_SHA1: sha1_block(h->h32, p); break;
	case ALT_SHA224:
	case ALT_SHA256: sha256_block(h->h32, p); break;
	default: sha512_block(h->h64, p); break;
	}
}

void alt_hash_write(alt_hash *h, const void *data, size_t n) {
	const uint8_t *p = data;
	size_t bs = alt_hash_block_size(h->kind);
	h->len += n;
	if (h->nbuf) {
		size_t k = bs - h->nbuf < n ? bs - h->nbuf : n;
		memcpy(h->buf + h->nbuf, p, k);
		h->nbuf += k;
		p += k, n -= k;
		if (h->nbuf == bs) {
			block(h, h->buf);
			h->nbuf = 0;
		}
	}
	for (; n >= bs; p += bs, n -= bs)
		block(h, p);
	if (n) {
		memcpy(h->buf, p, n);
		h->nbuf = n;
	}
}

void alt_hash_sum(const alt_hash *h0, uint8_t *out) {
	alt_hash h = *h0;
	size_t bs = alt_hash_block_size(h.kind), lenbytes = bs == 128 ? 16 : 8;
	uint64_t bits = h.len * 8;
	uint8_t pad[256] = {0x80};
	size_t padlen = bs - (h.nbuf + lenbytes) % bs;
	if (padlen == 0)
		padlen = bs;
	/* the length: little-endian for MD5, big-endian for the others */
	uint8_t len[16] = {0};
	for (int i = 0; i < 8; i++)
		len[lenbytes - 1 - (size_t)i] = (uint8_t)(bits >> (8 * i));
	if (h.kind == ALT_MD5)
		for (int i = 0; i < 8; i++)
			len[i] = (uint8_t)(bits >> (8 * i));
	alt_hash_write(&h, pad, padlen);
	alt_hash_write(&h, len, lenbytes);
	size_t size = alt_hash_size(h.kind);
	if (h.kind == ALT_MD5) {
		for (int i = 0; i < 4; i++)
			for (int k = 0; k < 4; k++)
				out[4 * i + k] = (uint8_t)(h.h32[i] >> (8 * k));
	} else if (h.kind <= ALT_SHA256) {
		for (size_t i = 0; i < size / 4; i++)
			for (int k = 0; k < 4; k++)
				out[4 * i + (size_t)k] = (uint8_t)(h.h32[i] >> (24 - 8 * k));
	} else {
		uint8_t full[64];
		for (int i = 0; i < 8; i++)
			for (int k = 0; k < 8; k++)
				full[8 * i + k] = (uint8_t)(h.h64[i] >> (56 - 8 * k));
		memcpy(out, full, size);
	}
}

void alt_hash_data(alt_hash_kind kind, const void *p, size_t n, uint8_t *out) {
	alt_hash h;
	alt_hash_init(&h, kind);
	alt_hash_write(&h, p, n);
	alt_hash_sum(&h, out);
}

/* ---- HMAC ---- */

void alt_hmac_init(alt_hmac *m, alt_hash_kind kind, const void *key, size_t keylen) {
	size_t bs = alt_hash_block_size(kind);
	uint8_t k[ALT_HASH_MAX_BLOCK] = {0};
	if (keylen > bs)
		alt_hash_data(kind, key, keylen, k);
	else if (keylen)
		memcpy(k, key, keylen);
	uint8_t ipad[ALT_HASH_MAX_BLOCK], opad[ALT_HASH_MAX_BLOCK];
	for (size_t i = 0; i < bs; i++) {
		ipad[i] = k[i] ^ 0x36;
		opad[i] = k[i] ^ 0x5c;
	}
	alt_hash_init(&m->inner, kind);
	alt_hash_write(&m->inner, ipad, bs);
	alt_hash_init(&m->outer, kind);
	alt_hash_write(&m->outer, opad, bs);
}

void alt_hmac_write(alt_hmac *m, const void *p, size_t n) { alt_hash_write(&m->inner, p, n); }

void alt_hmac_sum(const alt_hmac *m, uint8_t *out) {
	uint8_t in[ALT_HASH_MAX_SIZE];
	alt_hash_sum(&m->inner, in);
	alt_hash o = m->outer;
	alt_hash_write(&o, in, alt_hash_size(o.kind));
	alt_hash_sum(&o, out);
}

void alt_hmac_data(alt_hash_kind kind, const void *key, size_t keylen, const void *p, size_t n, uint8_t *out) {
	alt_hmac m;
	alt_hmac_init(&m, kind, key, keylen);
	alt_hmac_write(&m, p, n);
	alt_hmac_sum(&m, out);
}

/* ---- randomness, comparison ---- */

int alt_crypto_rand(void *buf, size_t n) {
	uint8_t *p = buf;
	while (n) {
		ssize_t k = getrandom(p, n, 0);
		if (k < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}
		p += k;
		n -= (size_t)k;
	}
	return 0;
}

int alt_ct_eq(const void *a, const void *b, size_t n) {
	const volatile uint8_t *x = a, *y = b;
	uint8_t v = 0;
	for (size_t i = 0; i < n; i++)
		v |= x[i] ^ y[i];
	return ((uint32_t)v - 1) >> 31;
}
