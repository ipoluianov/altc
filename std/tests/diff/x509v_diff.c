/* now dns eku roots(hex,hex..) inters(hex,..) leafhex -> the chains
   ("chain:CN>CN>CN;...") or "error <message>" */
#include "common.h"
#include <alt/crypto/x509.h>
#include <inttypes.h>

static void add_list(alt_x509_pool *p, char *list) {
	for (char *save = 0, *t = strtok_r(list, ",", &save); t; t = strtok_r(0, ",", &save)) {
		size_t n = diff_unhex(t);
		alt_x509_pool_add_der(p, t, n);
	}
}

int main(void) {
	char *line, *f[8];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 8);
		if (n < 6) {
			printf("bad request\n");
			continue;
		}
		alt_x509_pool *roots = alt_x509_pool_new(), *inters = alt_x509_pool_new();
		add_list(roots, f[3]);
		add_list(inters, f[4]);
		size_t ln = diff_unhex(f[5]);
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_x509_cert *leaf;
		const char *why = "";
		if (alt_x509_parse(&a, f[5], ln, &leaf, &why) < 0) {
			printf("parse %s\n", why);
			goto next;
		}
		int ekus[4];
		size_t neku = 0;
		for (char *save = 0, *t = strtok_r(f[2], ",", &save); t; t = strtok_r(0, ",", &save))
			if (neku < 4)
				ekus[neku++] = atoi(t);
		alt_x509_verify_opts o = {f[1], roots, inters, strtoll(f[0], 0, 10), ekus, neku};
		if (!f[1][0])
			o.dns_name = 0;
		alt_x509_chain *chains;
		size_t nchains;
		if (alt_x509_verify(leaf, &o, &a, &chains, &nchains, &why) < 0) {
			printf("error %s\n", why);
			goto next;
		}
		printf("chains:");
		for (size_t i = 0; i < nchains; i++) {
			if (i)
				printf(";");
			for (size_t k = 0; k < chains[i].n; k++) {
				alt_str cn = alt_x509_name_get(&chains[i].certs[k]->subject, &ALT_OID(2, 5, 4, 3));
				printf("%s%.*s", k ? ">" : "", (int)cn.n, cn.p);
			}
		}
		printf("\n");
	next:
		alt_arena_free(&a);
		alt_x509_pool_free(roots);
		alt_x509_pool_free(inters);
	}
	return 0;
}
