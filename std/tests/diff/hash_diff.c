/* H kind hex -> digest hex;  M kind keyhex hex -> HMAC hex
   S kind hex -> digest written in pieces of 1, 63, 64, 65... bytes
   C kind hex -> checksums: crc32 IEEE, Castagnoli, Koopman, adler32,
   crc64 ISO, ECMA, fnv1/fnv1a 32/64 (all hex, fed in two parts at kind) */
#include "common.h"
#include <alt/crypto/hash.h>
#include <alt/hash/adler32.h>
#include <alt/hash/crc32.h>
#include <alt/hash/crc64.h>
#include <alt/hash/fnv.h>
#include <inttypes.h>

int main(void) {
	char *line, *f[4];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 4);
		alt_hash_kind k = (alt_hash_kind)atoi(f[1]);
		uint8_t out[64];
		if (line[0] == 'C') {
			size_t len = n > 2 ? diff_unhex(f[2]) : 0, cut = (size_t)atoi(f[1]);
			if (cut > len)
				cut = len;
			const char *a = f[2], *b = f[2] + cut;
			size_t bl = len - cut;
			uint32_t polys[] = {ALT_CRC32_IEEE, ALT_CRC32_CASTAGNOLI, ALT_CRC32_KOOPMAN};
			for (int i = 0; i < 3; i++) {
				const alt_crc32_table *t = alt_crc32_make_table(polys[i]);
				printf("%08x ", alt_crc32_update(alt_crc32_update(0, t, a, cut), t, b, bl));
			}
			printf("%08x ", alt_adler32_update(alt_adler32_update(1, a, cut), b, bl));
			uint64_t p64[] = {ALT_CRC64_ISO, ALT_CRC64_ECMA};
			for (int i = 0; i < 2; i++) {
				const alt_crc64_table *t = alt_crc64_make_table(p64[i]);
				printf("%016" PRIx64 " ", alt_crc64_update(alt_crc64_update(0, t, a, cut), t, b, bl));
			}
			printf("%08x %08x %016" PRIx64 " %016" PRIx64 "\n", alt_fnv1_32(alt_fnv1_32(ALT_FNV32_OFFSET, a, cut), b, bl),
				alt_fnv1a_32(alt_fnv1a_32(ALT_FNV32_OFFSET, a, cut), b, bl),
				alt_fnv1_64(alt_fnv1_64(ALT_FNV64_OFFSET, a, cut), b, bl),
				alt_fnv1a_64(alt_fnv1a_64(ALT_FNV64_OFFSET, a, cut), b, bl));
			continue;
		}
		if (line[0] == 'H' || line[0] == 'S') {
			size_t len = n > 2 ? diff_unhex(f[2]) : 0;
			if (line[0] == 'H')
				alt_hash_data(k, f[2], len, out);
			else {
				alt_hash h;
				alt_hash_init(&h, k);
				size_t steps[] = {1, 63, 64, 65, 127, 128, 129, 7};
				size_t off = 0;
				for (int i = 0; off < len; i = (i + 1) % 8) {
					size_t s = steps[i] < len - off ? steps[i] : len - off;
					alt_hash_write(&h, f[2] + off, s);
					off += s;
					uint8_t mid[64];
					alt_hash_sum(&h, mid); /* does not disturb the state */
				}
				alt_hash_sum(&h, out);
			}
		} else {
			size_t kl = diff_unhex(f[2]), ml = n > 3 ? diff_unhex(f[3]) : 0;
			alt_hmac_data(k, f[2], kl, f[3], ml, out);
		}
		diff_puthex((char *)out, alt_hash_size(k));
		printf("\n");
	}
	return 0;
}
