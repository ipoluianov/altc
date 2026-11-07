/* A key data -> AES encryption of the blocks (AES-NI and tables must agree, decryption inverts)
   C key iv data -> CTR;  B key iv data -> CBC encryption (decryption inverts)
   G key nonce ad pt -> GCM seal (open inverts);  O key nonce ad ct -> "ok pt" or "auth"
   K kind salt ikm info len -> HKDF;  P kind pass salt iter len -> PBKDF2
   X key nonce ad pt -> (X)ChaCha20-Poly1305 seal;  Y key nonce ad ct -> open
   E seed msg -> Ed25519 public key and signature;  V pub msg sig -> ok/bad
   W scalar point -> X25519 or "low";  w scalar -> X25519 public key */
#include "common.h"
#include <alt/crypto/aes.h>
#include <alt/crypto/chacha20poly1305.h>
#include <alt/crypto/ecdsa.h>
#include <alt/crypto/ed25519.h>
#include <alt/crypto/kdf.h>
#include <alt/crypto/rsa.h>

extern bool alt__aes_generic;

static void put(const uint8_t *p, size_t n) { diff_puthex((const char *)p, n); }

int main(void) {
	char *line, *f[16];
	uint8_t *out = malloc(1 << 20), *back = malloc(1 << 20), *first = malloc(1 << 20);
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 16);
		size_t len[16] = {0};
		if (strchr("rpst", line[0]) && f[0][1] == 0) {
			int kind = atoi(f[1]);
			for (int i = 2; i < n; i++)
				if (i != 3 && !((line[0] == 'p' || line[0] == 't') && i == n - 1))
					len[i] = diff_unhex(f[i]);
			alt_rsa_public pub = {(uint8_t *)f[2], len[2], strtoll(f[3], 0, 10)};
			int rc;
			if (line[0] == 'r' || line[0] == 'p') {
				rc = line[0] == 'r' ? alt_rsa_verify_pkcs1v15(&pub, kind, f[4], len[4], (uint8_t *)f[5], len[5])
						    : alt_rsa_verify_pss(&pub, kind, f[4], len[4], (uint8_t *)f[5], len[5], atoi(f[6]));
				printf("%s\n", rc == 0 ? "ok" : rc == -ALT_EVERIFY ? "bad" : "key");
				continue;
			}
			alt_rsa_private k = {pub, (uint8_t *)f[4], (uint8_t *)f[5], (uint8_t *)f[6], (uint8_t *)f[7], (uint8_t *)f[8],
				(uint8_t *)f[9], len[4], len[5], len[6], len[7], len[8], len[9]};
			uint8_t sig[ALT_RSA_MAX_BYTES];
			rc = line[0] == 's' ? alt_rsa_sign_pkcs1v15(&k, kind, f[10], len[10], sig)
					    : alt_rsa_sign_pss(&k, kind, f[10], len[10], sig, atoi(f[11]));
			if (rc < 0)
				printf("error %d\n", rc);
			else {
				put(sig, pub.nlen);
				printf("\n");
			}
			continue;
		}
		if (line[0] == 'K' || line[0] == 'P') {
			uint8_t o[1024];
			int kind = atoi(line + 2);
			size_t ln = 0;
			int nh = line[0] == 'K' ? 5 : 4; /* the hex fields end before */
			for (int i = 2; i < nh && i < n; i++)
				len[i] = diff_unhex(f[i]);
			ln = (size_t)atoi(f[5]);
			int rc = line[0] == 'K' ? alt_hkdf(kind, f[2], len[2], f[3], len[3], f[4], len[4], o, ln)
						: alt_pbkdf2(kind, f[2], len[2], f[3], len[3], (unsigned)atoi(f[4]), o, ln);
			if (rc < 0)
				printf("error\n");
			else {
				put(o, ln);
				printf("\n");
			}
			continue;
		}
		if (line[0] == 'U' || line[0] == 'S' || line[0] == 'Q' || line[0] == 'D') {
			alt_ec_curve cv = (alt_ec_curve)atoi(f[1]);
			for (int i = 2; i < n; i++)
				len[i] = diff_unhex(f[i]);
			uint8_t o[256];
			int rc;
			switch (line[0]) {
			case 'U':
				rc = alt_ec_public(cv, (uint8_t *)f[2], o);
				if (rc == 0)
					put(o, alt_ec_point_size(cv));
				break;
			case 'S':
				rc = alt_ecdsa_sign(cv, (uint8_t *)f[2], f[3], len[3], o);
				if (rc > 0) {
					uint8_t pub[ALT_EC_MAX_POINT];
					alt_ec_public(cv, (uint8_t *)f[2], pub);
					put(o, (size_t)rc);
					if (alt_ecdsa_verify(cv, pub, alt_ec_point_size(cv), f[3], len[3], o, (size_t)rc))
						printf(" selfcheck-failed");
					rc = 0;
				}
				break;
			case 'Q':
				rc = alt_ecdsa_verify(cv, (uint8_t *)f[2], len[2], f[3], len[3], (uint8_t *)f[4], len[4]);
				printf("%s", rc == 0 ? "ok" : rc == -ALT_EVERIFY ? "bad" : "key");
				rc = 0;
				break;
			default:
				rc = alt_ecdh(cv, (uint8_t *)f[2], (uint8_t *)f[3], len[3], o);
				if (rc == 0)
					put(o, alt_ec_scalar_size(cv));
			}
			printf("%s\n", rc < 0 ? "key" : "");
			continue;
		}
		for (int i = 1; i < n; i++)
			len[i] = diff_unhex(f[i]);
		if (line[0] == 'E') {
			uint8_t pub[32], priv[64], sig[64];
			alt_ed25519_keygen(pub, priv, (uint8_t *)f[1]);
			alt_ed25519_sign(sig, priv, f[2], len[2]);
			put(pub, 32);
			printf(" ");
			put(sig, 64);
			printf("%s\n", alt_ed25519_verify(pub, f[2], len[2], sig) ? " selfcheck-failed" : "");
			continue;
		}
		if (line[0] == 'V') {
			printf("%s\n", len[1] == 32 && len[3] == 64 && alt_ed25519_verify((uint8_t *)f[1], f[2], len[2], (uint8_t *)f[3]) == 0
					      ? "ok"
					      : "bad");
			continue;
		}
		if (line[0] == 'W' || line[0] == 'w') {
			uint8_t o[32];
			if (line[0] == 'w')
				alt_x25519_base(o, (uint8_t *)f[1]);
			else if (alt_x25519(o, (uint8_t *)f[1], (uint8_t *)f[2]) < 0) {
				printf("low\n");
				continue;
			}
			put(o, 32);
			printf("\n");
			continue;
		}
		if (line[0] == 'X' || line[0] == 'Y') {
			const uint8_t *k = (uint8_t *)f[1], *nonce = (uint8_t *)f[2];
			if (line[0] == 'X') {
				if (len[2] == 24)
					alt_xchacha20poly1305_seal(k, nonce, out, f[4], len[4], f[3], len[3]);
				else
					alt_chacha20poly1305_seal(k, nonce, out, f[4], len[4], f[3], len[3]);
				put(out, len[4] + 16);
				printf("\n");
			} else {
				int rc = len[2] == 24 ? alt_xchacha20poly1305_open(k, nonce, out, f[4], len[4], f[3], len[3])
						      : alt_chacha20poly1305_open(k, nonce, out, f[4], len[4], f[3], len[3]);
				if (rc < 0)
					printf("auth\n");
				else {
					printf("ok ");
					put(out, len[4] - 16);
					printf("\n");
				}
			}
			continue;
		}
		const uint8_t *key = (uint8_t *)f[1];
		bool fail = false;
		for (int pass = 0; pass < 2; pass++) {
			alt__aes_generic = pass == 1;
			alt_aes c;
			alt_gcm g;
			if (alt_aes_init(&c, key, len[1]) < 0) {
				printf("badkey\n");
				break;
			}
			alt_gcm_init(&g, key, len[1]);
			size_t on = 0;
			const char *res = "";
			switch (line[0]) {
			case 'A':
				for (size_t i = 0; i + 16 <= len[2]; i += 16) {
					alt_aes_encrypt(&c, out + i, f[2] + i);
					alt_aes_decrypt(&c, back + i, out + i);
					if (memcmp(back + i, f[2] + i, 16))
						fail = true;
				}
				on = len[2] / 16 * 16;
				break;
			case 'C': {
				alt_aes_ctr s;
				alt_aes_ctr_init(&s, &c, (uint8_t *)f[2]);
				size_t off = 0, step = 1;
				while (off < len[3]) {
					size_t k = step < len[3] - off ? step : len[3] - off;
					alt_aes_ctr_xor(&s, out + off, f[3] + off, k);
					off += k;
					step = step * 3 % 37 + 1;
				}
				on = len[3];
				break;
			}
			case 'B': {
				uint8_t iv[16], iv2[16];
				memcpy(iv, f[2], 16);
				memcpy(iv2, f[2], 16);
				size_t m = len[3] / 16 * 16;
				alt_aes_cbc_encrypt(&c, iv, out, f[3], m);
				alt_aes_cbc_decrypt(&c, iv2, back, out, m);
				if (memcmp(back, f[3], m) || memcmp(iv, iv2, 16))
					fail = true;
				on = m;
				break;
			}
			case 'G':
				alt_gcm_seal(&g, out, f[2], len[2], f[4], len[4], f[3], len[3]);
				on = len[4] + 16;
				if (alt_gcm_open(&g, back, f[2], len[2], out, on, f[3], len[3]) < 0 || memcmp(back, f[4], len[4]))
					fail = true;
				break;
			case 'O':
				if (alt_gcm_open(&g, out, f[2], len[2], f[4], len[4], f[3], len[3]) < 0)
					res = "auth";
				else
					res = "ok ", on = len[4] - 16;
				break;
			}
			if (pass == 0) {
				printf("%s", res);
				put(out, on);
				printf("%s\n", fail ? " roundtrip-failed" : "");
				memcpy(first, out, on);
			} else if (memcmp(first, out, on))
				printf("generic differs\n"); /* an extra line: the test fails */
		}
	}
	return 0;
}
