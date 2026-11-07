/* gen bits -> PKCS #1 and PKCS #8 DER (hex) of a new key
   oenc key hash label msg / penc key msg -> the ciphertext
   odec key hash label ct / pdec key ct -> "ok <msg>" or "error"
   (keys: PKCS #1 DER in hex; all data in hex) */
#include "common.h"
#include <alt/arena.h>
#include <alt/crypto/rsa.h>
#include <alt/crypto/x509.h>

static alt_rsa_private key_of(alt_arena *a, char *hex, alt_x509_private_key *pk) {
	size_t n = diff_unhex(hex);
	alt_rsa_private k = {0};
	if (alt_x509_parse_private_key(a, hex, n, pk, 0) < 0) {
		printf("[bad key]");
		return k;
	}
	k = (alt_rsa_private){{pk->rsa.n.p, pk->rsa.n.n, pk->rsa.e}, pk->rsa.d.p, pk->rsa.p.p, pk->rsa.q.p, pk->rsa.dp.p, pk->rsa.dq.p,
		pk->rsa.qinv.p, pk->rsa.d.n, pk->rsa.p.n, pk->rsa.q.n, pk->rsa.dp.n, pk->rsa.dq.n, pk->rsa.qinv.n};
	return k;
}

int main(void) {
	char *line, *f[8];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 8);
		(void)n;
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_x509_private_key pk;
		uint8_t out[ALT_RSA_MAX_BYTES];
		size_t no;
		if (!strcmp(f[0], "gen")) {
			static alt_rsa_key k;
			int rc = alt_rsa_generate(&k, atoi(f[1]));
			if (rc < 0)
				printf("error %d", rc);
			else {
				alt_buf b1 = ALT_BUF_INIT, b8 = ALT_BUF_INIT;
				alt_rsa_marshal_pkcs1(&k.k, &b1);
				alt_rsa_marshal_pkcs8(&k.k, &b8);
				diff_puthex(b1.p, b1.n);
				printf(" ");
				diff_puthex(b8.p, b8.n);
				alt_buf_free(&b1), alt_buf_free(&b8);
			}
		} else if (!strcmp(f[0], "oenc") || !strcmp(f[0], "odec")) {
			alt_rsa_private k = key_of(&a, f[1], &pk);
			alt_hash_kind h = (alt_hash_kind)atoi(f[2]);
			size_t nl = diff_unhex(f[3]), nm = diff_unhex(f[4]);
			if (f[0][1] == 'e') {
				int rc = alt_rsa_encrypt_oaep(&k.pub, h, f[4], nm, f[3], nl, out);
				if (rc < 0)
					printf("error %s", rc == -EMSGSIZE ? "too long" : "other");
				else
					diff_puthex((char *)out, k.pub.nlen);
			} else if (alt_rsa_decrypt_oaep(&k, h, (uint8_t *)f[4], nm, f[3], nl, out, &no) < 0)
				printf("error");
			else {
				printf("ok ");
				diff_puthex((char *)out, no);
			}
		} else {
			alt_rsa_private k = key_of(&a, f[1], &pk);
			size_t nm = diff_unhex(f[2]);
			if (f[0][1] == 'e') {
				int rc = alt_rsa_encrypt_pkcs1v15(&k.pub, f[2], nm, out);
				if (rc < 0)
					printf("error %s", rc == -EMSGSIZE ? "too long" : "other");
				else
					diff_puthex((char *)out, k.pub.nlen);
			} else if (alt_rsa_decrypt_pkcs1v15(&k, (uint8_t *)f[2], nm, out, &no) < 0)
				printf("error");
			else {
				printf("ok ");
				diff_puthex((char *)out, no);
			}
		}
		printf("\n");
		fflush(stdout);
		alt_arena_free(&a);
	}
	return 0;
}
