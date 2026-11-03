#include <stdio.h>
#include <stdint.h>
#include <string.h>

static uint32_t crc32(const uint8_t *d, size_t n) {
    static uint32_t table[256]; static int init;
    if (!init) { for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1; table[i] = c; } init = 1; }
    uint32_t c = 0xffffffffu; while (n--) c = table[(c ^ *d++) & 0xff] ^ (c >> 8); return c ^ 0xffffffffu;
}
#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
static void sha256(const uint8_t *msg, size_t len, uint8_t out[32]) {
    static const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    uint8_t block[128]; size_t full = len / 64 * 64;
    for (size_t off = 0; off <= len; off += 64) {
        size_t nblocks = 1; const uint8_t *p;
        if (off < full) p = msg + off;
        else { size_t rem = len - full; memset(block, 0, sizeof block); memcpy(block, msg + full, rem); block[rem] = 0x80;
            nblocks = rem + 9 > 64 ? 2 : 1; uint64_t bits = (uint64_t)len * 8; for (int i = 0; i < 8; i++) block[nblocks * 64 - 1 - i] = (uint8_t)(bits >> (8 * i)); p = block; }
        for (size_t b = 0; b < nblocks; b++, p += 64) {
            uint32_t w[64];
            for (int i = 0; i < 16; i++) w[i] = (uint32_t)p[4 * i] << 24 | (uint32_t)p[4 * i + 1] << 16 | (uint32_t)p[4 * i + 2] << 8 | p[4 * i + 3];
            for (int i = 16; i < 64; i++) { uint32_t s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3), s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10); w[i] = w[i - 16] + s0 + w[i - 7] + s1; }
            uint32_t a = h[0], bb = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int i = 0; i < 64; i++) {
                uint32_t S1 = ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25), ch = (e & f) ^ (~e & g), t1 = hh + S1 + ch + k[i] + w[i];
                uint32_t S0 = ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22), maj = (a & bb) ^ (a & c) ^ (bb & c), t2 = S0 + maj;
                hh = g; g = f; f = e; e = d + t1; d = c; c = bb; bb = a; a = t1 + t2;
            }
            h[0] += a; h[1] += bb; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
        }
        if (off >= full) break;
    }
    for (int i = 0; i < 8; i++) for (int j = 0; j < 4; j++) out[4 * i + j] = (uint8_t)(h[i] >> (24 - 8 * j));
}
static uint64_t fnv64(const char *s) { uint64_t h = 0xcbf29ce484222325ULL; while (*s) { h ^= (uint8_t)*s++; h *= 0x100000001b3ULL; } return h; }
static uint64_t xorshift(uint64_t *s) { uint64_t x = *s; x ^= x << 13; x ^= x >> 7; x ^= x << 17; return *s = x; }
static uint32_t murmur_mix(uint32_t h) { h ^= h >> 16; h *= 0x85ebca6b; h ^= h >> 13; h *= 0xc2b2ae35; h ^= h >> 16; return h; }
static uint16_t adler_like(const char *s) { uint16_t a = 1, b = 0; while (*s) { a = (a + (uint8_t)*s++) % 251; b = (b + a) % 251; } return (uint16_t)(b << 8 | a); }

int main(void) {
    const char *msgs[] = {"", "abc", "The quick brown fox jumps over the lazy dog", "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                          "0123456789012345678901234567890123456789012345678901234567890123456789"};
    for (int m = 0; m < 5; m++) {
        uint8_t dg[32]; sha256((const uint8_t *)msgs[m], strlen(msgs[m]), dg);
        printf("crc=%08x fnv=%016llx adl=%04x sha=", crc32((const uint8_t *)msgs[m], strlen(msgs[m])), (unsigned long long)fnv64(msgs[m]), adler_like(msgs[m]));
        for (int i = 0; i < 32; i++) printf("%02x", dg[i]); printf("\n");
    }
    uint64_t s = 88172645463325252ULL; for (int i = 0; i < 5; i++) printf("%016llx ", (unsigned long long)xorshift(&s)); printf("\n");
    for (uint32_t i = 0; i < 5; i++) printf("%08x ", murmur_mix(i * 0x9e3779b9u)); printf("\n");
    uint64_t big = 0; for (int i = 0; i < 64; i += 3) big |= 1ULL << i; printf("%016llx %d\n", (unsigned long long)big, __builtin_popcountll(big));
    uint32_t rot = 0x80000001u; for (int i = 0; i < 4; i++) { rot = ROR(rot, 1); printf("%08x ", rot); } printf("\n");
    return 0;
}
