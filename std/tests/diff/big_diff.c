/* op args... -> the result in base 10 (Go's math/big Int); see
   tests/stdbig_test.go for the operations */
#include "common.h"
#include <alt/math/big.h>

static alt_big num(const char *s) {
	alt_big x = ALT_BIG_INIT;
	if (alt_big_set_string(&x, alt_str_cstr(s), 10) < 0)
		printf("[bad input %s]", s);
	return x;
}

static void put(const alt_big *x) {
	alt_buf b = ALT_BUF_INIT;
	alt_big_append_text(&b, x, 10);
	printf("%s", alt_buf_cstr(&b));
	alt_buf_free(&b);
}

int main(void) {
	char *line, *f[8];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 8);
		(void)n;
		const char *op = f[0];
		alt_big a = strcmp(op, "parse") ? num(f[1]) : (alt_big)ALT_BIG_INIT, b = ALT_BIG_INIT, c = ALT_BIG_INIT, z = ALT_BIG_INIT, x = ALT_BIG_INIT, y = ALT_BIG_INIT;
		if (!strcmp(op, "parse") || !strcmp(op, "text") || !strcmp(op, "lsh") || !strcmp(op, "rsh") || !strcmp(op, "bit") ||
			!strcmp(op, "setbit") || !strcmp(op, "prime"))
			;
		else if (n > 2 && strcmp(f[2], "nil"))
			b = num(f[2]);
		int (*bin)(alt_big *, const alt_big *, const alt_big *) = 0;
		if (!strcmp(op, "add")) bin = alt_big_add;
		else if (!strcmp(op, "sub")) bin = alt_big_sub;
		else if (!strcmp(op, "mul")) bin = alt_big_mul;
		else if (!strcmp(op, "quo")) bin = alt_big_quo;
		else if (!strcmp(op, "rem")) bin = alt_big_rem;
		else if (!strcmp(op, "div")) bin = alt_big_div;
		else if (!strcmp(op, "mod")) bin = alt_big_mod;
		else if (!strcmp(op, "and")) bin = alt_big_and;
		else if (!strcmp(op, "or")) bin = alt_big_or;
		else if (!strcmp(op, "xor")) bin = alt_big_xor;
		else if (!strcmp(op, "andnot")) bin = alt_big_and_not;
		if (bin) {
			/* also in place: z aliasing the first operand */
			bin(&z, &a, &b);
			alt_big_set(&x, &a);
			bin(&x, &x, &b);
			put(&z);
			if (alt_big_cmp(&z, &x) != 0)
				printf(" alias-differs");
		} else if (!strcmp(op, "lsh") || !strcmp(op, "rsh")) {
			size_t k = strtoul(f[2], 0, 10);
			if (op[0] == 'l')
				alt_big_lsh(&z, &a, k);
			else
				alt_big_rsh(&z, &a, k);
			put(&z);
		} else if (!strcmp(op, "not")) {
			alt_big_not(&z, &a);
			put(&z);
		} else if (!strcmp(op, "sqrt")) {
			alt_big_sqrt(&z, &a);
			put(&z);
		} else if (!strcmp(op, "bit")) {
			printf("%u", alt_big_bit(&a, strtoul(f[2], 0, 10)));
		} else if (!strcmp(op, "setbit")) {
			alt_big_set_bit(&z, &a, strtoul(f[2], 0, 10), (unsigned)atoi(f[3]));
			put(&z);
		} else if (!strcmp(op, "exp")) {
			if (strcmp(f[3], "nil"))
				c = num(f[3]);
			if (!strcmp(f[3], "nil") ? alt_big_exp(&z, &a, &b, 0) < 0 : alt_big_exp(&z, &a, &b, &c) < 0)
				printf("nil");
			else
				put(&z);
		} else if (!strcmp(op, "gcd")) {
			alt_big_gcd(&z, &x, &y, &a, &b);
			put(&z), printf(" "), put(&x), printf(" "), put(&y);
			alt_big z2 = ALT_BIG_INIT;
			alt_big_gcd(&z2, 0, 0, &a, &b);
			if (alt_big_cmp(&z, &z2))
				printf(" plain-differs");
			alt_big_free(&z2);
		} else if (!strcmp(op, "inv")) {
			if (alt_big_mod_inverse(&z, &a, &b) < 0)
				printf("nil");
			else
				put(&z);
		} else if (!strcmp(op, "prime")) {
			printf("%s", alt_big_probably_prime(&a, atoi(f[2])) ? "true" : "false");
		} else if (!strcmp(op, "jacobi")) {
			printf("%d", alt_big_jacobi(&a, &b));
		} else if (!strcmp(op, "text")) {
			alt_buf t = ALT_BUF_INIT;
			alt_big_append_text(&t, &a, atoi(f[2]));
			printf("%s", alt_buf_cstr(&t));
			alt_buf_free(&t);
		} else if (!strcmp(op, "parse")) {
			alt_big_free(&a);
			char *s = f[1];
			size_t sl = diff_unhex(s);
			if (alt_big_set_string(&z, alt_str_make(s, sl), atoi(f[2])) < 0)
				printf("fail");
			else
				put(&z);
		} else if (!strcmp(op, "ints")) {
			printf("%lld %d %llu %d %zu %zu %d", (long long)alt_big_int64(&a), alt_big_is_int64(&a),
				(unsigned long long)alt_big_uint64(&a), alt_big_is_uint64(&a), alt_big_bit_len(&a),
				alt_big_trailing_zero_bits(&a), alt_big_sign(&a));
		} else if (!strcmp(op, "bytes")) {
			size_t k = alt_big_bytes_len(&a);
			uint8_t *buf = malloc(k + 1);
			alt_big_bytes(&a, buf);
			diff_puthex((char *)buf, k);
			alt_big_set_bytes(&z, buf, k);
			printf(" ");
			put(&z);
			free(buf);
		} else if (!strcmp(op, "cmp")) {
			printf("%d %d", alt_big_cmp(&a, &b), alt_big_cmp_abs(&a, &b));
		} else
			printf("unknown");
		printf("\n");
		alt_big_free(&a), alt_big_free(&b), alt_big_free(&c), alt_big_free(&z), alt_big_free(&x), alt_big_free(&y);
		fflush(stdout);
	}
	return 0;
}
