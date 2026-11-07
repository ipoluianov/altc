/* Arbitrary-precision integers, after Go's math/big (BSD-3-Clause, see
   LICENSE-Go) in their semantics: schoolbook and Karatsuba
   multiplication, Knuth's division, Montgomery exponentiation for odd
   moduli, the extended Euclidean algorithm (the cofactors Go's Lehmer
   GCD gives), Miller-Rabin and the extra strong Lucas test. */
#include <alt/crypto/hash.h>
#include <alt/math/big.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned __int128 u128;
typedef uint64_t word;

/* ---- storage ---- */

void alt_big_init(alt_big *z) { *z = (alt_big)ALT_BIG_INIT; }

void alt_big_free(alt_big *z) {
	free(z->w);
	*z = (alt_big)ALT_BIG_INIT;
}

static int reserve(alt_big *z, size_t n) {
	if (n <= z->cap)
		return 0;
	size_t cap = z->cap ? z->cap : 4;
	while (cap < n)
		cap *= 2;
	word *w = realloc(z->w, cap * sizeof *w);
	if (!w)
		return -ENOMEM;
	z->w = w;
	z->cap = cap;
	return 0;
}

static void norm(alt_big *z) {
	while (z->n && z->w[z->n - 1] == 0)
		z->n--;
	if (!z->n)
		z->neg = false;
}

/* moves t into z (z's old memory goes) */
static void take(alt_big *z, alt_big *t) {
	if (z == t)
		return;
	free(z->w);
	*z = *t;
	*t = (alt_big)ALT_BIG_INIT;
}

int alt_big_set(alt_big *z, const alt_big *x) {
	if (z == x)
		return 0;
	if (reserve(z, x->n) < 0)
		return -ENOMEM;
	if (x->n)
		memcpy(z->w, x->w, x->n * sizeof(word));
	z->n = x->n;
	z->neg = x->neg;
	return 0;
}

int alt_big_set_uint64(alt_big *z, uint64_t v) {
	if (reserve(z, 1) < 0)
		return -ENOMEM;
	z->w[0] = v;
	z->n = v != 0;
	z->neg = false;
	return 0;
}

int alt_big_set_int64(alt_big *z, int64_t v) {
	uint64_t m = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
	int rc = alt_big_set_uint64(z, m);
	z->neg = v < 0;
	return rc;
}

int alt_big_set_bytes(alt_big *z, const void *bytes, size_t n) {
	const uint8_t *b = bytes;
	size_t nw = (n + 7) / 8;
	if (reserve(z, nw) < 0)
		return -ENOMEM;
	memset(z->w, 0, nw * sizeof(word));
	for (size_t i = 0; i < n; i++)
		z->w[i / 8] |= (word)b[n - 1 - i] << (8 * (i % 8));
	z->n = nw;
	z->neg = false;
	norm(z);
	return 0;
}

size_t alt_big_bit_len(const alt_big *x) {
	if (!x->n)
		return 0;
	return (x->n - 1) * 64 + (size_t)(64 - __builtin_clzll(x->w[x->n - 1]));
}

size_t alt_big_bytes_len(const alt_big *x) { return (alt_big_bit_len(x) + 7) / 8; }

void alt_big_bytes(const alt_big *x, uint8_t *out) {
	size_t n = alt_big_bytes_len(x);
	for (size_t i = 0; i < n; i++)
		out[n - 1 - i] = (uint8_t)(x->w[i / 8] >> (8 * (i % 8)));
}

int alt_big_fill_bytes(const alt_big *x, uint8_t *out, size_t n) {
	size_t k = alt_big_bytes_len(x);
	if (k > n)
		return -ERANGE;
	memset(out, 0, n - k);
	alt_big_bytes(x, out + n - k);
	return 0;
}

int alt_big_sign(const alt_big *x) { return x->n == 0 ? 0 : x->neg ? -1 : 1; }

static int nat_cmp(const word *a, size_t na, const word *b, size_t nb) {
	if (na != nb)
		return na < nb ? -1 : 1;
	for (size_t i = na; i-- > 0;)
		if (a[i] != b[i])
			return a[i] < b[i] ? -1 : 1;
	return 0;
}

int alt_big_cmp_abs(const alt_big *x, const alt_big *y) { return nat_cmp(x->w, x->n, y->w, y->n); }

int alt_big_cmp(const alt_big *x, const alt_big *y) {
	if (x->neg != y->neg)
		return x->neg ? -1 : 1;
	int c = alt_big_cmp_abs(x, y);
	return x->neg ? -c : c;
}

static uint64_t low64(const alt_big *x) { return x->n ? x->w[0] : 0; }

bool alt_big_is_int64(const alt_big *x) {
	if (x->n > 1)
		return false;
	int64_t w = (int64_t)low64(x);
	return w >= 0 || (x->neg && w == -w);
}

bool alt_big_is_uint64(const alt_big *x) { return !x->neg && x->n <= 1; }

int64_t alt_big_int64(const alt_big *x) {
	uint64_t v = low64(x);
	return (int64_t)(x->neg ? (uint64_t)0 - v : v);
}

uint64_t alt_big_uint64(const alt_big *x) { return low64(x); }

size_t alt_big_trailing_zero_bits(const alt_big *x) {
	for (size_t i = 0; i < x->n; i++)
		if (x->w[i])
			return i * 64 + (size_t)__builtin_ctzll(x->w[i]);
	return 0;
}

/* ---- the magnitudes ---- */

/* z = |x| + |y| */
static int mag_add(alt_big *z, const alt_big *x, const alt_big *y) {
	if (x->n < y->n) {
		const alt_big *t = x;
		x = y, y = t;
	}
	alt_big r = ALT_BIG_INIT;
	if (reserve(&r, x->n + 1) < 0)
		return -ENOMEM;
	word c = 0;
	for (size_t i = 0; i < x->n; i++) {
		u128 s = (u128)x->w[i] + (i < y->n ? y->w[i] : 0) + c;
		r.w[i] = (word)s;
		c = (word)(s >> 64);
	}
	r.w[x->n] = c;
	r.n = x->n + 1;
	norm(&r);
	take(z, &r);
	return 0;
}

/* z = |x| - |y|, |x| >= |y| */
static int mag_sub(alt_big *z, const alt_big *x, const alt_big *y) {
	alt_big r = ALT_BIG_INIT;
	if (reserve(&r, x->n ? x->n : 1) < 0)
		return -ENOMEM;
	word b = 0;
	for (size_t i = 0; i < x->n; i++) {
		word yi = i < y->n ? y->w[i] : 0;
		u128 d = (u128)x->w[i] - yi - b;
		r.w[i] = (word)d;
		b = (word)(d >> 64) & 1;
	}
	r.n = x->n;
	norm(&r);
	take(z, &r);
	return 0;
}

static void basic_mul(word *z, const word *x, size_t nx, const word *y, size_t ny) {
	memset(z, 0, (nx + ny) * sizeof(word));
	for (size_t i = 0; i < nx; i++) {
		word c = 0, xi = x[i];
		if (!xi)
			continue;
		for (size_t j = 0; j < ny; j++) {
			u128 t = (u128)xi * y[j] + z[i + j] + c;
			z[i + j] = (word)t;
			c = (word)(t >> 64);
		}
		z[i + ny] = c;
	}
}

/* z[0..n) += x[0..m), returns the carry (m <= n) */
static word add_to(word *z, size_t n, const word *x, size_t m) {
	word c = 0;
	for (size_t i = 0; i < n; i++) {
		if (i >= m && !c)
			break;
		u128 s = (u128)z[i] + (i < m ? x[i] : 0) + c;
		z[i] = (word)s;
		c = (word)(s >> 64);
	}
	return c;
}

static word sub_from(word *z, size_t n, const word *x, size_t m) {
	word b = 0;
	for (size_t i = 0; i < n; i++) {
		if (i >= m && !b)
			break;
		u128 d = (u128)z[i] - (i < m ? x[i] : 0) - b;
		z[i] = (word)d;
		b = (word)(d >> 64) & 1;
	}
	return b;
}

enum { KARATSUBA = 40 };

/* z (2n words) = x * y (n words each); tmp of 4n words */
static void karatsuba(word *z, const word *x, const word *y, size_t n, word *tmp) {
	if (n < KARATSUBA || n % 2) {
		basic_mul(z, x, n, y, n);
		return;
	}
	size_t h = n / 2;
	const word *x0 = x, *x1 = x + h, *y0 = y, *y1 = y + h;
	/* z0 = x0*y0, z2 = x1*y1 */
	karatsuba(z, x0, y0, h, tmp);
	karatsuba(z + n, x1, y1, h, tmp);
	/* |x1 - x0| and |y1 - y0| with signs */
	word *dx = tmp, *dy = tmp + h, *m = tmp + n, *rest = tmp + 2 * n;
	bool nx = nat_cmp(x1, h, x0, h) < 0, ny = nat_cmp(y1, h, y0, h) < 0;
	memcpy(dx, nx ? x0 : x1, h * sizeof(word));
	sub_from(dx, h, nx ? x1 : x0, h);
	memcpy(dy, ny ? y0 : y1, h * sizeof(word));
	sub_from(dy, h, ny ? y1 : y0, h);
	/* (x1-x0)(y1-y0) = z2 + z0 - z1 */
	word *mid = malloc(2 * n * sizeof(word));
	if (!mid) {
		basic_mul(z, x, n, y, n);
		return;
	}
	karatsuba(m, dx, dy, h, rest);
	/* mid = z0 + z2 */
	memcpy(mid, z, n * sizeof(word));
	memset(mid + n, 0, n * sizeof(word));
	word c = add_to(mid, n + 1, z + n, n);
	(void)c;
	/* z1 = mid - (x1-x0)(y1-y0) when the signs agree, else + */
	if (nx == ny)
		sub_from(mid, n + 1, m, n);
	else
		add_to(mid, n + 1, m, n);
	add_to(z + h, n + h, mid, n + 1);
	free(mid);
}

static int mag_mul(alt_big *z, const alt_big *x, const alt_big *y) {
	if (!x->n || !y->n) {
		z->n = 0;
		z->neg = false;
		return 0;
	}
	alt_big r = ALT_BIG_INIT;
	size_t n = x->n + y->n;
	if (reserve(&r, n) < 0)
		return -ENOMEM;
	if (x->n == y->n && x->n >= KARATSUBA && x->n % 2 == 0) {
		word *tmp = malloc(4 * x->n * sizeof(word));
		if (tmp) {
			karatsuba(r.w, x->w, y->w, x->n, tmp);
			free(tmp);
		} else
			basic_mul(r.w, x->w, x->n, y->w, y->n);
	} else if (x->n >= 2 * KARATSUBA && y->n >= 2 * KARATSUBA) {
		/* unequal sizes: multiply in pieces of the shorter length */
		const alt_big *a = x->n >= y->n ? x : y, *b = x->n >= y->n ? y : x;
		size_t k = b->n % 2 ? b->n + 1 : b->n;
		word *tmp = malloc(4 * k * sizeof(word)), *part = malloc(2 * k * sizeof(word)), *pa = calloc(k, sizeof(word)),
		     *pb = calloc(k, sizeof(word));
		if (tmp && part && pa && pb) {
			memset(r.w, 0, n * sizeof(word));
			memcpy(pb, b->w, b->n * sizeof(word));
			for (size_t off = 0; off < a->n; off += k) {
				size_t m = a->n - off < k ? a->n - off : k;
				memset(pa, 0, k * sizeof(word));
				memcpy(pa, a->w + off, m * sizeof(word));
				karatsuba(part, pa, pb, k, tmp);
				size_t room = n - off;
				add_to(r.w + off, room, part, 2 * k < room ? 2 * k : room);
			}
		} else
			basic_mul(r.w, x->w, x->n, y->w, y->n);
		free(tmp), free(part), free(pa), free(pb);
	} else
		basic_mul(r.w, x->w, x->n, y->w, y->n);
	r.n = n;
	norm(&r);
	take(z, &r);
	return 0;
}

/* q = u / v, r = u % v (magnitudes; v != 0); Knuth's algorithm D */
static int mag_divmod(alt_big *q, alt_big *r, const alt_big *u, const alt_big *v) {
	alt_big Q = ALT_BIG_INIT, R = ALT_BIG_INIT;
	int rc = 0;
	if (nat_cmp(u->w, u->n, v->w, v->n) < 0) {
		if ((rc = alt_big_set(&R, u)) < 0)
			return rc;
		R.neg = false;
		goto done;
	}
	if (v->n == 1) {
		word d = v->w[0], rem = 0;
		if (reserve(&Q, u->n) < 0)
			return -ENOMEM;
		for (size_t i = u->n; i-- > 0;) {
			u128 cur = (u128)rem << 64 | u->w[i];
			Q.w[i] = (word)(cur / d);
			rem = (word)(cur % d);
		}
		Q.n = u->n;
		norm(&Q);
		if ((rc = alt_big_set_uint64(&R, rem)) < 0)
			goto fail;
		goto done;
	}
	{
		size_t n = v->n, m = u->n - v->n;
		int s = __builtin_clzll(v->w[n - 1]);
		word *vn = malloc(n * sizeof(word)), *un = malloc((u->n + 1) * sizeof(word));
		if (!vn || !un || reserve(&Q, m + 1) < 0 || reserve(&R, n) < 0) {
			free(vn), free(un);
			rc = -ENOMEM;
			goto fail;
		}
		for (size_t i = n; i-- > 0;)
			vn[i] = (v->w[i] << s) | (s && i ? v->w[i - 1] >> (64 - s) : 0);
		un[u->n] = s ? u->w[u->n - 1] >> (64 - s) : 0;
		for (size_t i = u->n; i-- > 0;)
			un[i] = (u->w[i] << s) | (s && i ? u->w[i - 1] >> (64 - s) : 0);
		for (size_t j = m + 1; j-- > 0;) {
			u128 num = (u128)un[j + n] << 64 | un[j + n - 1];
			u128 qhat = num / vn[n - 1], rhat = num % vn[n - 1];
			while (qhat >> 64 || qhat * vn[n - 2] > (rhat << 64 | un[j + n - 2])) {
				qhat--;
				rhat += vn[n - 1];
				if (rhat >> 64)
					break;
			}
			/* un[j..j+n] -= qhat * vn */
			word borrow = 0, carry = 0;
			for (size_t i = 0; i < n; i++) {
				u128 p = qhat * vn[i] + carry;
				carry = (word)(p >> 64);
				u128 t = (u128)un[i + j] - (word)p - borrow;
				un[i + j] = (word)t;
				borrow = (word)(t >> 64) & 1;
			}
			u128 t = (u128)un[j + n] - carry - borrow;
			un[j + n] = (word)t;
			if ((word)(t >> 64) & 1) {
				/* added back once */
				qhat--;
				word c = 0;
				for (size_t i = 0; i < n; i++) {
					u128 s2 = (u128)un[i + j] + vn[i] + c;
					un[i + j] = (word)s2;
					c = (word)(s2 >> 64);
				}
				un[j + n] += c;
			}
			Q.w[j] = (word)qhat;
		}
		Q.n = m + 1;
		norm(&Q);
		for (size_t i = 0; i < n; i++)
			R.w[i] = (un[i] >> s) | (s ? un[i + 1] << (64 - s) : 0);
		R.n = n;
		norm(&R);
		free(vn), free(un);
	}
done:
	if (q)
		take(q, &Q);
	if (r)
		take(r, &R);
	alt_big_free(&Q);
	alt_big_free(&R);
	return 0;
fail:
	alt_big_free(&Q);
	alt_big_free(&R);
	return rc;
}

/* ---- signed arithmetic ---- */

int alt_big_neg(alt_big *z, const alt_big *x) {
	int rc = alt_big_set(z, x);
	z->neg = z->n && !x->neg;
	return rc;
}

int alt_big_abs(alt_big *z, const alt_big *x) {
	int rc = alt_big_set(z, x);
	z->neg = false;
	return rc;
}

static int addsub(alt_big *z, const alt_big *x, const alt_big *y, bool yneg) {
	bool xneg = x->neg;
	int rc;
	if (xneg == yneg) {
		rc = mag_add(z, x, y);
		z->neg = z->n && xneg;
	} else if (alt_big_cmp_abs(x, y) >= 0) {
		rc = mag_sub(z, x, y);
		z->neg = z->n && xneg;
	} else {
		rc = mag_sub(z, y, x);
		z->neg = z->n && yneg;
	}
	return rc;
}

int alt_big_add(alt_big *z, const alt_big *x, const alt_big *y) { return addsub(z, x, y, y->neg); }
int alt_big_sub(alt_big *z, const alt_big *x, const alt_big *y) { return addsub(z, x, y, y->n && !y->neg); }

int alt_big_mul(alt_big *z, const alt_big *x, const alt_big *y) {
	bool neg = x->neg != y->neg;
	int rc = mag_mul(z, x, y);
	z->neg = z->n && neg;
	return rc;
}

int alt_big_quo_rem(alt_big *q, alt_big *r, const alt_big *x, const alt_big *y) {
	if (!y->n)
		return -EDOM;
	bool xneg = x->neg, yneg = y->neg;
	int rc = mag_divmod(q, r, x, y);
	if (rc < 0)
		return rc;
	if (q)
		q->neg = q->n && xneg != yneg;
	if (r)
		r->neg = r->n && xneg;
	return 0;
}

int alt_big_quo(alt_big *z, const alt_big *x, const alt_big *y) { return alt_big_quo_rem(z, 0, x, y); }
int alt_big_rem(alt_big *z, const alt_big *x, const alt_big *y) { return alt_big_quo_rem(0, z, x, y); }

int alt_big_div_mod(alt_big *q, alt_big *m, const alt_big *x, const alt_big *y) {
	alt_big yy = ALT_BIG_INIT, Q = ALT_BIG_INIT, R = ALT_BIG_INIT, one = ALT_BIG_INIT;
	int rc = alt_big_set(&yy, y);
	if (rc == 0)
		rc = alt_big_quo_rem(&Q, &R, x, &yy);
	if (rc == 0 && R.neg) {
		alt_big_set_int64(&one, 1);
		if (yy.neg) {
			rc = alt_big_add(&Q, &Q, &one);
			if (rc == 0)
				rc = alt_big_sub(&R, &R, &yy);
		} else {
			rc = alt_big_sub(&Q, &Q, &one);
			if (rc == 0)
				rc = alt_big_add(&R, &R, &yy);
		}
	}
	if (rc == 0) {
		if (q)
			take(q, &Q);
		if (m)
			take(m, &R);
	}
	alt_big_free(&yy), alt_big_free(&Q), alt_big_free(&R), alt_big_free(&one);
	return rc;
}

int alt_big_div(alt_big *z, const alt_big *x, const alt_big *y) { return alt_big_div_mod(z, 0, x, y); }
int alt_big_mod(alt_big *z, const alt_big *x, const alt_big *y) { return alt_big_div_mod(0, z, x, y); }

/* ---- shifts and bits ---- */

static int mag_lsh(alt_big *z, const alt_big *x, size_t n) {
	if (!x->n) {
		z->n = 0;
		z->neg = false;
		return 0;
	}
	size_t ws = n / 64, bs = n % 64;
	alt_big r = ALT_BIG_INIT;
	if (reserve(&r, x->n + ws + 1) < 0)
		return -ENOMEM;
	memset(r.w, 0, (x->n + ws + 1) * sizeof(word));
	for (size_t i = 0; i < x->n; i++) {
		r.w[i + ws] |= x->w[i] << bs;
		if (bs)
			r.w[i + ws + 1] |= x->w[i] >> (64 - bs);
	}
	r.n = x->n + ws + 1;
	norm(&r);
	take(z, &r);
	return 0;
}

static int mag_rsh(alt_big *z, const alt_big *x, size_t n) {
	size_t ws = n / 64, bs = n % 64;
	if (ws >= x->n) {
		z->n = 0;
		z->neg = false;
		return 0;
	}
	alt_big r = ALT_BIG_INIT;
	size_t m = x->n - ws;
	if (reserve(&r, m) < 0)
		return -ENOMEM;
	for (size_t i = 0; i < m; i++) {
		r.w[i] = x->w[i + ws] >> bs;
		if (bs && i + ws + 1 < x->n)
			r.w[i] |= x->w[i + ws + 1] << (64 - bs);
	}
	r.n = m;
	norm(&r);
	take(z, &r);
	return 0;
}

int alt_big_lsh(alt_big *z, const alt_big *x, size_t n) {
	bool neg = x->neg;
	int rc = mag_lsh(z, x, n);
	z->neg = z->n && neg;
	return rc;
}

static const alt_big one_ = {(uint64_t[]){1}, 1, 1, false};

int alt_big_rsh(alt_big *z, const alt_big *x, size_t n) {
	if (x->neg) {
		/* (-x) >> s == -(((x-1) >> s) + 1) */
		alt_big t = ALT_BIG_INIT;
		int rc = mag_sub(&t, x, &one_);
		if (rc == 0)
			rc = mag_rsh(&t, &t, n);
		if (rc == 0)
			rc = mag_add(&t, &t, &one_);
		if (rc == 0) {
			take(z, &t);
			z->neg = true;
		}
		alt_big_free(&t);
		return rc;
	}
	int rc = mag_rsh(z, x, n);
	z->neg = false;
	return rc;
}

unsigned alt_big_bit(const alt_big *x, size_t i) {
	if (x->neg) {
		/* the bits of x-1, inverted */
		alt_big t = ALT_BIG_INIT;
		if (mag_sub(&t, x, &one_) < 0)
			return 0;
		unsigned b = i / 64 < t.n ? (unsigned)(t.w[i / 64] >> (i % 64)) & 1 : 0;
		alt_big_free(&t);
		return b ^ 1;
	}
	return i / 64 < x->n ? (unsigned)(x->w[i / 64] >> (i % 64)) & 1 : 0;
}

static int mag_set_bit(alt_big *z, const alt_big *x, size_t i, unsigned b) {
	int rc = alt_big_set(z, x);
	if (rc < 0)
		return rc;
	size_t w = i / 64;
	if (b) {
		if (w >= z->n) {
			if (reserve(z, w + 1) < 0)
				return -ENOMEM;
			memset(z->w + z->n, 0, (w + 1 - z->n) * sizeof(word));
			z->n = w + 1;
		}
		z->w[w] |= (word)1 << (i % 64);
	} else if (w < z->n) {
		z->w[w] &= ~((word)1 << (i % 64));
		bool neg = z->neg;
		norm(z);
		z->neg = z->n && neg;
	}
	return 0;
}

int alt_big_set_bit(alt_big *z, const alt_big *x, size_t i, unsigned b) {
	if (x->neg) {
		alt_big t = ALT_BIG_INIT;
		int rc = mag_sub(&t, x, &one_);
		if (rc == 0)
			rc = mag_set_bit(&t, &t, i, (b & 1) ^ 1);
		if (rc == 0)
			rc = mag_add(&t, &t, &one_);
		if (rc == 0) {
			take(z, &t);
			z->neg = z->n > 0;
		}
		alt_big_free(&t);
		return rc;
	}
	int rc = mag_set_bit(z, x, i, b & 1);
	z->neg = false;
	return rc;
}

enum { OP_AND, OP_OR, OP_XOR, OP_ANDNOT };

/* the bitwise operation on magnitudes */
static int mag_bits(alt_big *z, const alt_big *x, const alt_big *y, int op) {
	size_t n = x->n > y->n ? x->n : y->n;
	alt_big r = ALT_BIG_INIT;
	if (reserve(&r, n ? n : 1) < 0)
		return -ENOMEM;
	for (size_t i = 0; i < n; i++) {
		word a = i < x->n ? x->w[i] : 0, b = i < y->n ? y->w[i] : 0;
		r.w[i] = op == OP_AND ? a & b : op == OP_OR ? a | b : op == OP_XOR ? a ^ b : a & ~b;
	}
	r.n = n;
	norm(&r);
	take(z, &r);
	return 0;
}

/* Go's two's complement identities, with x-1 and y-1 for negative operands */
static int bitop(alt_big *z, const alt_big *x, const alt_big *y, int op) {
	alt_big x1 = ALT_BIG_INIT, y1 = ALT_BIG_INIT, t = ALT_BIG_INIT;
	int rc = 0;
	if (x->neg)
		rc = mag_sub(&x1, x, &one_);
	if (rc == 0 && y->neg)
		rc = mag_sub(&y1, y, &one_);
	if (rc < 0)
		goto out;
	const alt_big *a = x->neg ? &x1 : x, *b = y->neg ? &y1 : y;
	bool neg = false, inc = false;
	switch (op) {
	case OP_AND:
		if (x->neg && y->neg)
			rc = mag_bits(&t, a, b, OP_OR), neg = inc = true; /* -(((x-1) | (y-1)) + 1) */
		else if (!x->neg && !y->neg)
			rc = mag_bits(&t, a, b, OP_AND);
		else if (x->neg)
			rc = mag_bits(&t, b, a, OP_ANDNOT); /* y &^ (x-1) */
		else
			rc = mag_bits(&t, a, b, OP_ANDNOT);
		break;
	case OP_OR:
		if (x->neg && y->neg)
			rc = mag_bits(&t, a, b, OP_AND), neg = inc = true;
		else if (!x->neg && !y->neg)
			rc = mag_bits(&t, a, b, OP_OR);
		else if (x->neg)
			rc = mag_bits(&t, a, b, OP_ANDNOT), neg = inc = true; /* -(((x-1) &^ y) + 1) */
		else
			rc = mag_bits(&t, b, a, OP_ANDNOT), neg = inc = true;
		break;
	case OP_XOR:
		if (x->neg == y->neg)
			rc = mag_bits(&t, a, b, OP_XOR);
		else
			rc = mag_bits(&t, a, b, OP_XOR), neg = inc = true;
		break;
	default: /* ANDNOT */
		if (x->neg && y->neg)
			rc = mag_bits(&t, b, a, OP_ANDNOT); /* (y-1) &^ (x-1) */
		else if (!x->neg && !y->neg)
			rc = mag_bits(&t, a, b, OP_ANDNOT);
		else if (x->neg)
			rc = mag_bits(&t, a, b, OP_OR), neg = inc = true; /* -(((x-1) | y) + 1) */
		else
			rc = mag_bits(&t, a, b, OP_AND); /* x & (y-1) */
		break;
	}
	if (rc == 0 && inc)
		rc = mag_add(&t, &t, &one_);
	if (rc == 0) {
		take(z, &t);
		z->neg = z->n && neg;
	}
out:
	alt_big_free(&x1), alt_big_free(&y1), alt_big_free(&t);
	return rc;
}

int alt_big_and(alt_big *z, const alt_big *x, const alt_big *y) { return bitop(z, x, y, OP_AND); }
int alt_big_or(alt_big *z, const alt_big *x, const alt_big *y) { return bitop(z, x, y, OP_OR); }
int alt_big_xor(alt_big *z, const alt_big *x, const alt_big *y) { return bitop(z, x, y, OP_XOR); }
int alt_big_and_not(alt_big *z, const alt_big *x, const alt_big *y) { return bitop(z, x, y, OP_ANDNOT); }

int alt_big_not(alt_big *z, const alt_big *x) {
	if (x->neg) {
		int rc = mag_sub(z, x, &one_);
		z->neg = false;
		return rc;
	}
	int rc = mag_add(z, x, &one_);
	z->neg = true;
	return rc;
}

/* ---- exponentiation ---- */

/* Montgomery arithmetic modulo an odd m of n words */
typedef struct {
	const word *m;
	size_t n;
	word inv; /* -m^-1 mod 2^64 */
} mont;

static void mont_mul(const mont *M, word *r, const word *a, const word *b, word *t) {
	size_t n = M->n;
	memset(t, 0, (n + 2) * sizeof(word));
	for (size_t i = 0; i < n; i++) {
		word c = 0;
		for (size_t j = 0; j < n; j++) {
			u128 p = (u128)a[j] * b[i] + t[j] + c;
			t[j] = (word)p;
			c = (word)(p >> 64);
		}
		u128 s = (u128)t[n] + c;
		t[n] = (word)s;
		t[n + 1] = (word)(s >> 64);
		word q = t[0] * M->inv;
		u128 p = (u128)q * M->m[0] + t[0];
		c = (word)(p >> 64);
		for (size_t j = 1; j < n; j++) {
			p = (u128)q * M->m[j] + t[j] + c;
			t[j - 1] = (word)p;
			c = (word)(p >> 64);
		}
		s = (u128)t[n] + c;
		t[n - 1] = (word)s;
		t[n] = t[n + 1] + (word)(s >> 64);
	}
	/* t < 2m: subtract m once if needed */
	bool ge = t[n] != 0 || nat_cmp(t, n, M->m, n) >= 0;
	if (ge)
		sub_from(t, n + 1, M->m, n);
	memcpy(r, t, n * sizeof(word));
}

/* z = x**y mod m for odd m (x < m) */
static int exp_mont(alt_big *z, const alt_big *x, const alt_big *y, const alt_big *m) {
	size_t n = m->n;
	mont M = {m->w, n, 0};
	word inv = 1;
	for (int i = 0; i < 6; i++)
		inv *= 2 - m->w[0] * inv;
	M.inv = (word)0 - inv;
	/* R mod m and x*R mod m by division */
	alt_big R = ALT_BIG_INIT, xr = ALT_BIG_INIT;
	int rc = mag_lsh(&R, &one_, 64 * n);
	if (rc == 0)
		rc = mag_divmod(0, &R, &R, m);
	if (rc == 0)
		rc = mag_lsh(&xr, x, 64 * n);
	if (rc == 0)
		rc = mag_divmod(0, &xr, &xr, m);
	word *tab = calloc(16 * n, sizeof(word)), *acc = calloc(n, sizeof(word)), *t = calloc(n + 2, sizeof(word));
	if (rc < 0 || !tab || !acc || !t) {
		free(tab), free(acc), free(t);
		alt_big_free(&R), alt_big_free(&xr);
		return rc < 0 ? rc : -ENOMEM;
	}
	/* tab[i] = x^i * R */
	memcpy(tab, R.w, R.n * sizeof(word));
	memcpy(tab + n, xr.w, xr.n * sizeof(word));
	for (int i = 2; i < 16; i++)
		mont_mul(&M, tab + i * n, tab + (i - 1) * n, tab + n, t);
	memcpy(acc, tab, n * sizeof(word));
	size_t bits = alt_big_bit_len(y);
	size_t nib = (bits + 3) / 4;
	for (size_t k = nib; k-- > 0;) {
		for (int s = 0; s < 4; s++)
			mont_mul(&M, acc, acc, acc, t);
		unsigned d = (unsigned)(y->w[(4 * k) / 64] >> ((4 * k) % 64)) & 15;
		if (d)
			mont_mul(&M, acc, acc, tab + d * n, t);
	}
	/* out of the Montgomery form: times 1 */
	word *onew = calloc(n, sizeof(word));
	if (!onew) {
		rc = -ENOMEM;
	} else {
		onew[0] = 1;
		mont_mul(&M, acc, acc, onew, t);
		alt_big r = ALT_BIG_INIT;
		if (reserve(&r, n) < 0)
			rc = -ENOMEM;
		else {
			memcpy(r.w, acc, n * sizeof(word));
			r.n = n;
			norm(&r);
			take(z, &r);
		}
	}
	free(onew), free(tab), free(acc), free(t);
	alt_big_free(&R), alt_big_free(&xr);
	return rc;
}

/* z = x**y mod m (magnitudes; m may be zero: no reduction) */
static int exp_nn(alt_big *z, const alt_big *x, const alt_big *y, const alt_big *m) {
	if (m && m->n == 1 && m->w[0] == 1) {
		z->n = 0;
		z->neg = false;
		return 0;
	}
	if (!y->n)
		return alt_big_set_uint64(z, 1);
	if (!x->n) {
		z->n = 0;
		z->neg = false;
		return 0;
	}
	alt_big base = ALT_BIG_INIT, acc = ALT_BIG_INIT;
	int rc = alt_big_abs(&base, x);
	if (rc == 0 && m && m->n)
		rc = mag_divmod(0, &base, &base, m);
	if (rc == 0 && m && m->n && (m->w[0] & 1) && m->n > 1) {
		rc = exp_mont(&acc, &base, y, m);
	} else if (rc == 0) {
		rc = alt_big_set_uint64(&acc, 1);
		for (size_t i = alt_big_bit_len(y); rc == 0 && i-- > 0;) {
			rc = mag_mul(&acc, &acc, &acc);
			if (rc == 0 && m && m->n)
				rc = mag_divmod(0, &acc, &acc, m);
			if (rc == 0 && ((y->w[i / 64] >> (i % 64)) & 1)) {
				rc = mag_mul(&acc, &acc, &base);
				if (rc == 0 && m && m->n)
					rc = mag_divmod(0, &acc, &acc, m);
			}
		}
	}
	if (rc == 0)
		take(z, &acc);
	alt_big_free(&base), alt_big_free(&acc);
	return rc;
}

int alt_big_exp(alt_big *z, const alt_big *x, const alt_big *y, const alt_big *m) {
	alt_big inv = ALT_BIG_INIT, mm = ALT_BIG_INIT, yy = ALT_BIG_INIT;
	const alt_big *xw = x;
	bool xneg = x->neg;
	int rc = 0;
	if (y->neg) {
		if (!m || !m->n) {
			return alt_big_set_int64(z, 1);
		}
		if ((rc = alt_big_mod_inverse(&inv, x, m)) < 0)
			return rc;
		xw = &inv;
	}
	if (m && (rc = alt_big_abs(&mm, m)) < 0)
		goto out;
	if ((rc = alt_big_abs(&yy, y)) < 0)
		goto out;
	alt_big r = ALT_BIG_INIT;
	rc = exp_nn(&r, xw, &yy, m ? &mm : 0);
	if (rc == 0) {
		bool neg = r.n && xneg && yy.n && (yy.w[0] & 1);
		if (neg && mm.n) {
			rc = mag_sub(&r, &mm, &r); /* the result made positive */
			neg = false;
		}
		r.neg = r.n && neg;
		take(z, &r);
	}
	alt_big_free(&r);
out:
	alt_big_free(&inv), alt_big_free(&mm), alt_big_free(&yy);
	return rc;
}

/* ---- GCD, inverse, square root, Jacobi ---- */

int alt_big_gcd(alt_big *z, alt_big *x, alt_big *y, const alt_big *a, const alt_big *b) {
	if (!a->n || !b->n) {
		bool zeroA = !a->n, negA = a->neg, negB = b->neg, zeroB = !b->n;
		int rc = alt_big_set(z, zeroA ? b : a);
		z->neg = false;
		if (rc == 0 && x) {
			rc = alt_big_set_uint64(x, zeroA ? 0 : 1);
			x->neg = !zeroA && negA;
		}
		if (rc == 0 && y) {
			rc = alt_big_set_uint64(y, zeroB ? 0 : 1);
			y->neg = !zeroB && negB;
		}
		return rc;
	}
	/* the extended Euclidean algorithm: the quotients (so the cofactors) of Go's Lehmer GCD */
	alt_big A = ALT_BIG_INIT, B = ALT_BIG_INIT, Ua = ALT_BIG_INIT, Ub = ALT_BIG_INIT, q = ALT_BIG_INIT, r = ALT_BIG_INIT,
		t = ALT_BIG_INIT, aa = ALT_BIG_INIT, bb = ALT_BIG_INIT;
	bool ext = x || y;
	bool negA = a->neg;
	int rc = alt_big_abs(&A, a);
	if (rc == 0)
		rc = alt_big_abs(&B, b);
	if (rc == 0)
		rc = alt_big_set(&aa, a);
	if (rc == 0)
		rc = alt_big_set(&bb, b);
	if (rc == 0 && ext) {
		rc = alt_big_set_int64(&Ua, 1);
		if (rc == 0)
			rc = alt_big_set_int64(&Ub, 0);
	}
	if (rc == 0 && alt_big_cmp_abs(&A, &B) < 0) {
		alt_big s = A;
		A = B, B = s;
		s = Ua, Ua = Ub, Ub = s;
	}
	while (rc == 0 && B.n) {
		rc = alt_big_quo_rem(&q, &r, &A, &B);
		if (rc == 0 && ext) {
			/* Ua, Ub = Ub, Ua - q*Ub */
			rc = alt_big_mul(&t, &q, &Ub);
			if (rc == 0)
				rc = alt_big_sub(&t, &Ua, &t);
			if (rc == 0) {
				alt_big s = Ua;
				Ua = Ub, Ub = t, t = s;
			}
		}
		if (rc == 0) {
			alt_big s = A;
			A = B, B = r, r = s;
		}
	}
	if (rc == 0 && y) {
		/* y = (z - a*x)/b */
		rc = alt_big_mul(&t, &aa, &Ua);
		if (rc == 0) {
			if (negA)
				t.neg = t.n && !t.neg;
			rc = alt_big_sub(&t, &A, &t);
		}
		if (rc == 0)
			rc = alt_big_div(&t, &t, &bb);
		if (rc == 0)
			take(y, &t);
	}
	if (rc == 0 && x) {
		take(x, &Ua);
		if (negA)
			x->neg = x->n && !x->neg;
	}
	if (rc == 0)
		take(z, &A);
	alt_big_free(&A), alt_big_free(&B), alt_big_free(&Ua), alt_big_free(&Ub), alt_big_free(&q), alt_big_free(&r);
	alt_big_free(&t), alt_big_free(&aa), alt_big_free(&bb);
	return rc;
}

int alt_big_mod_inverse(alt_big *z, const alt_big *g, const alt_big *n) {
	alt_big nn = ALT_BIG_INIT, gg = ALT_BIG_INIT, d = ALT_BIG_INIT, x = ALT_BIG_INIT;
	int rc = alt_big_abs(&nn, n);
	if (rc == 0)
		rc = alt_big_set(&gg, g);
	if (rc == 0 && gg.neg)
		rc = nn.n ? alt_big_mod(&gg, &gg, &nn) : -EDOM;
	if (rc == 0)
		rc = alt_big_gcd(&d, &x, 0, &gg, &nn);
	if (rc == 0 && !(d.n == 1 && d.w[0] == 1 && !d.neg))
		rc = -EDOM;
	if (rc == 0) {
		if (x.neg)
			rc = alt_big_add(&x, &x, &nn);
		if (rc == 0)
			take(z, &x);
	}
	alt_big_free(&nn), alt_big_free(&gg), alt_big_free(&d), alt_big_free(&x);
	return rc;
}

int alt_big_sqrt(alt_big *z, const alt_big *x) {
	if (x->neg)
		return -EDOM;
	if (x->n == 0 || (x->n == 1 && x->w[0] < 2))
		return alt_big_set(z, x);
	/* Newton from above: z1 = 2^(bitlen/2 + 1) */
	alt_big z1 = ALT_BIG_INIT, z2 = ALT_BIG_INIT;
	int rc = mag_lsh(&z1, &one_, alt_big_bit_len(x) / 2 + 1);
	while (rc == 0) {
		rc = mag_divmod(&z2, 0, x, &z1);
		if (rc == 0)
			rc = mag_add(&z2, &z2, &z1);
		if (rc == 0)
			rc = mag_rsh(&z2, &z2, 1);
		if (rc < 0 || alt_big_cmp_abs(&z2, &z1) >= 0)
			break;
		alt_big s = z1;
		z1 = z2, z2 = s;
	}
	if (rc == 0)
		take(z, &z1);
	alt_big_free(&z1), alt_big_free(&z2);
	return rc;
}

int alt_big_jacobi(const alt_big *x, const alt_big *y) {
	/* Go's Jacobi (y odd) */
	alt_big a = ALT_BIG_INIT, b = ALT_BIG_INIT, c = ALT_BIG_INIT;
	alt_big_set(&a, x);
	alt_big_set(&b, y);
	int j = 1;
	if (b.neg) {
		if (a.neg)
			j = -1;
		b.neg = false;
	}
	for (;;) {
		if (b.n == 1 && b.w[0] == 1)
			break;
		if (!a.n) {
			j = 0;
			break;
		}
		alt_big_mod(&a, &a, &b);
		if (!a.n) {
			j = 0;
			break;
		}
		size_t s = alt_big_trailing_zero_bits(&a);
		if (s & 1) {
			word bmod8 = b.w[0] & 7;
			if (bmod8 == 3 || bmod8 == 5)
				j = -j;
		}
		mag_rsh(&c, &a, s);
		if ((b.w[0] & 3) == 3 && (c.w[0] & 3) == 3)
			j = -j;
		alt_big_set(&a, &b);
		alt_big_set(&b, &c);
	}
	alt_big_free(&a), alt_big_free(&b), alt_big_free(&c);
	return j;
}

/* ---- primes ---- */

static int u64_mod(const alt_big *x, word d) {
	word r = 0;
	for (size_t i = x->n; i-- > 0;)
		r = (word)(((u128)r << 64 | x->w[i]) % d);
	return (int)r;
}

/* Miller-Rabin with base 2 and reps-1 random bases */
static bool miller_rabin(const alt_big *n, int reps) {
	alt_big nm1 = ALT_BIG_INIT, q = ALT_BIG_INIT, x = ALT_BIG_INIT, y = ALT_BIG_INIT, nm3 = ALT_BIG_INIT, two = ALT_BIG_INIT;
	bool prime = true;
	mag_sub(&nm1, n, &one_);
	size_t k = alt_big_trailing_zero_bits(&nm1);
	mag_rsh(&q, &nm1, k);
	alt_big_set_int64(&two, 3);
	mag_sub(&nm3, n, &two);
	alt_big_set_int64(&two, 2);
	for (int i = 0; i < reps && prime; i++) {
		if (i == 0)
			alt_big_set_int64(&x, 2);
		else {
			/* a random base in [2, n-2] */
			size_t nb = alt_big_bytes_len(n);
			uint8_t buf[1024];
			if (nb > sizeof buf)
				nb = sizeof buf;
			alt_crypto_rand(buf, nb);
			alt_big_set_bytes(&x, buf, nb);
			mag_divmod(0, &x, &x, &nm3);
			mag_add(&x, &x, &two);
		}
		exp_nn(&y, &x, &q, n);
		if ((y.n == 1 && y.w[0] == 1) || alt_big_cmp_abs(&y, &nm1) == 0)
			continue;
		bool ok = false;
		for (size_t j = 1; j < k; j++) {
			mag_mul(&y, &y, &y);
			mag_divmod(0, &y, &y, n);
			if (alt_big_cmp_abs(&y, &nm1) == 0) {
				ok = true;
				break;
			}
			if (y.n == 1 && y.w[0] == 1)
				break;
		}
		prime = ok;
	}
	alt_big_free(&nm1), alt_big_free(&q), alt_big_free(&x), alt_big_free(&y), alt_big_free(&nm3), alt_big_free(&two);
	return prime;
}

/* Go's probablyPrimeLucas: the extra strong Lucas test */
static bool lucas(const alt_big *n) {
	if (!n->n || (n->n == 1 && n->w[0] == 1))
		return false;
	if (!(n->w[0] & 1))
		return n->n == 1 && n->w[0] == 2;
	word p = 3;
	alt_big d = ALT_BIG_INIT, t1 = ALT_BIG_INIT, t2 = ALT_BIG_INIT;
	bool result = false;
	for (;; p++) {
		alt_big_set_uint64(&d, p * p - 4);
		int j = alt_big_jacobi(&d, n);
		if (j == -1)
			break;
		if (j == 0) {
			result = n->n == 1 && n->w[0] == p + 2;
			goto out;
		}
		if (p == 40) {
			alt_big_sqrt(&t1, n);
			mag_mul(&t1, &t1, &t1);
			if (alt_big_cmp_abs(&t1, n) == 0)
				goto out; /* a square never has (D/n) = -1 */
		}
		if (p > 10000)
			goto out;
	}
	{
		alt_big s = ALT_BIG_INIT, nm2 = ALT_BIG_INIT, P = ALT_BIG_INIT, vk = ALT_BIG_INIT, vk1 = ALT_BIG_INIT, two = ALT_BIG_INIT;
		mag_add(&s, n, &one_);
		size_t r = alt_big_trailing_zero_bits(&s);
		mag_rsh(&s, &s, r);
		alt_big_set_int64(&two, 2);
		mag_sub(&nm2, n, &two);
		alt_big_set_uint64(&P, p);
		alt_big_set_int64(&vk, 2);
		alt_big_set_uint64(&vk1, p);
		for (size_t i = alt_big_bit_len(&s) + 1; i-- > 0;) {
			bool bit = i / 64 < s.n && ((s.w[i / 64] >> (i % 64)) & 1);
			alt_big *a = bit ? &vk : &vk1, *b = bit ? &vk1 : &vk;
			/* a = vk*vk1 + n - P; b = b^2 + n - 2 (mod n) */
			mag_mul(&t1, &vk, &vk1);
			mag_add(&t1, &t1, n);
			mag_sub(&t1, &t1, &P);
			mag_divmod(0, &t2, &t1, n);
			mag_mul(&t1, b, b);
			mag_add(&t1, &t1, &nm2);
			mag_divmod(0, b, &t1, n);
			alt_big_set(a, &t2);
		}
		if (alt_big_cmp_abs(&vk, &two) == 0 || alt_big_cmp_abs(&vk, &nm2) == 0) {
			mag_mul(&t1, &vk, &P);
			mag_lsh(&t2, &vk1, 1);
			if (alt_big_cmp_abs(&t1, &t2) < 0) {
				alt_big sw = t1;
				t1 = t2, t2 = sw;
			}
			mag_sub(&t1, &t1, &t2);
			mag_divmod(0, &t2, &t1, n);
			if (!t2.n) {
				result = true;
				goto done;
			}
		}
		for (size_t t = 0; t + 1 < r; t++) {
			if (!vk.n) {
				result = true;
				goto done;
			}
			if (vk.n == 1 && vk.w[0] == 2)
				goto done;
			mag_mul(&t1, &vk, &vk);
			mag_sub(&t1, &t1, &two);
			mag_divmod(0, &vk, &t1, n);
		}
	done:
		alt_big_free(&s), alt_big_free(&nm2), alt_big_free(&P), alt_big_free(&vk), alt_big_free(&vk1), alt_big_free(&two);
	}
out:
	alt_big_free(&d), alt_big_free(&t1), alt_big_free(&t2);
	return result;
}

bool alt_big_probably_prime(const alt_big *x, int n) {
	if (n < 0 || x->neg || !x->n)
		return false;
	static const uint64_t mask = 1ull << 2 | 1ull << 3 | 1ull << 5 | 1ull << 7 | 1ull << 11 | 1ull << 13 | 1ull << 17 | 1ull << 19 |
				    1ull << 23 | 1ull << 29 | 1ull << 31 | 1ull << 37 | 1ull << 41 | 1ull << 43 | 1ull << 47 | 1ull << 53 |
				    1ull << 59 | 1ull << 61;
	word w = x->w[0];
	if (x->n == 1 && w < 64)
		return (mask >> w) & 1;
	if (!(w & 1))
		return false;
	static const int small[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53};
	for (size_t i = 0; i < sizeof small / sizeof *small; i++)
		if (u64_mod(x, (word)small[i]) == 0)
			return false;
	return miller_rabin(x, n + 1) && lucas(x);
}

int alt_big_random_prime(alt_big *z, size_t bits) {
	if (bits < 2)
		return -EINVAL;
	size_t nb = (bits + 7) / 8;
	uint8_t *b = malloc(nb);
	if (!b)
		return -ENOMEM;
	int rc;
	for (;;) {
		if ((rc = alt_crypto_rand(b, nb)) < 0)
			break;
		/* the excess bits off, the top two bits on (as crypto/rand.Prime), odd */
		unsigned top = (unsigned)(bits % 8 ? bits % 8 : 8);
		b[0] &= (uint8_t)((1u << top) - 1);
		if (top >= 2)
			b[0] |= (uint8_t)(3u << (top - 2));
		else {
			b[0] |= 1;
			if (nb > 1)
				b[1] |= 0x80;
		}
		b[nb - 1] |= 1;
		if ((rc = alt_big_set_bytes(z, b, nb)) < 0)
			break;
		if (alt_big_probably_prime(z, 20)) {
			rc = 0;
			break;
		}
	}
	memset(b, 0, nb);
	free(b);
	return rc;
}

/* ---- strings ---- */

static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int alt_big_append_text(alt_buf *b, const alt_big *x, int base) {
	if (base < 2 || base > 62)
		return -EINVAL;
	if (!x->n)
		return alt_buf_append_byte(b, '0');
	/* the largest power of base in a word: digits come out in chunks */
	word bb = (word)base, pw = bb;
	int per = 1;
	while ((u128)pw * bb <= UINT64_MAX)
		pw *= bb, per++;
	alt_big t = ALT_BIG_INIT;
	if (alt_big_abs(&t, x) < 0)
		return -ENOMEM;
	char *s = malloc(x->n * 64 + 2);
	if (!s) {
		alt_big_free(&t);
		return -ENOMEM;
	}
	size_t k = 0;
	while (t.n) {
		word rem = 0;
		for (size_t i = t.n; i-- > 0;) {
			u128 cur = (u128)rem << 64 | t.w[i];
			t.w[i] = (word)(cur / pw);
			rem = (word)(cur % pw);
		}
		norm(&t);
		for (int i = 0; i < per && (t.n || rem); i++) {
			s[k++] = digits[rem % bb];
			rem /= bb;
		}
	}
	if (x->neg)
		alt_buf_append_byte(b, '-');
	for (size_t i = k; i-- > 0;)
		alt_buf_append_byte(b, s[i]);
	free(s);
	alt_big_free(&t);
	return b->err;
}

/* Go's nat.scan and Int.SetString: the whole string */
int alt_big_set_string(alt_big *z, alt_str s, int base) {
	if (base != 0 && (base < 2 || base > 62))
		return -EINVAL;
	size_t i = 0;
	bool neg = false;
	if (i < s.n && (s.p[i] == '+' || s.p[i] == '-'))
		neg = s.p[i++] == '-';
	int b = base, prefix = 0, count = 0;
	char prev = '.';
	bool inval = false;
	if (base == 0) {
		b = 10;
		if (i < s.n && s.p[i] == '0') {
			prev = '0';
			count = 1;
			i++;
			if (i < s.n) {
				switch (s.p[i]) {
				case 'b': case 'B': b = 2, prefix = 'b'; break;
				case 'o': case 'O': b = 8, prefix = 'o'; break;
				case 'x': case 'X': b = 16, prefix = 'x'; break;
				default: b = 8, prefix = '0';
				}
				count = 0;
				if (prefix != '0')
					i++;
			}
		}
	}
	alt_big r = ALT_BIG_INIT, mul = ALT_BIG_INIT, dig = ALT_BIG_INIT;
	int rc = alt_big_set_uint64(&r, 0);
	/* digits in chunks of a word */
	word bw = (word)b, chunk = 0, cpow = 1;
	for (; rc == 0 && i < s.n; i++) {
		char ch = s.p[i];
		if (ch == '_' && base == 0) {
			if (prev != '0')
				inval = true;
			prev = '_';
			continue;
		}
		word d;
		if (ch >= '0' && ch <= '9')
			d = (word)(ch - '0');
		else if (ch >= 'a' && ch <= 'z')
			d = (word)(ch - 'a' + 10);
		else if (ch >= 'A' && ch <= 'Z')
			d = b <= 36 ? (word)(ch - 'A' + 10) : (word)(ch - 'A' + 36);
		else
			d = 63;
		if (d >= bw)
			break; /* not part of the number: SetString fails below */
		prev = '0';
		count++;
		if ((u128)cpow * bw > UINT64_MAX) {
			alt_big_set_uint64(&mul, cpow);
			alt_big_set_uint64(&dig, chunk);
			rc = mag_mul(&r, &r, &mul);
			if (rc == 0)
				rc = mag_add(&r, &r, &dig);
			chunk = 0, cpow = 1;
		}
		chunk = chunk * bw + d;
		cpow *= bw;
	}
	if (rc == 0 && cpow > 1) {
		alt_big_set_uint64(&mul, cpow);
		alt_big_set_uint64(&dig, chunk);
		rc = mag_mul(&r, &r, &mul);
		if (rc == 0)
			rc = mag_add(&r, &r, &dig);
	}
	bool ok = rc == 0 && i == s.n && !inval && prev != '_' && (count > 0 || prefix == '0');
	if (ok) {
		r.neg = r.n && neg;
		take(z, &r);
	}
	alt_big_free(&r), alt_big_free(&mul), alt_big_free(&dig);
	return rc < 0 ? rc : ok ? 0 : -EINVAL;
}
