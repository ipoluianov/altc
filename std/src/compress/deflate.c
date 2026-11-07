/* DEFLATE compression (RFC 1951): LZ77 over a 32 KiB window with hash
   chains (greedy for levels 1-3, lazy matching for 4-9, with zlib's
   tuning parameters), each block written in the smallest of the stored,
   fixed-Huffman and dynamic-Huffman forms. */
#include <alt/compress/flate.h>
#include <stdlib.h>
#include <string.h>

enum {
	W = 32768,             /* window */
	WMASK = W - 1,
	MIN_MATCH = 3,
	MAX_MATCH = 258,
	MIN_LOOKAHEAD = MAX_MATCH + MIN_MATCH + 1,
	MAX_DIST = W - MIN_LOOKAHEAD,
	HASH_BITS = 15,
	HASH_SIZE = 1 << HASH_BITS,
	MAX_TOK = 16384,       /* tokens per block */
	TOO_FAR = 4096,        /* length-3 matches farther are not worth it */
	LIT_CODES = 286,
	DIST_CODES = 30,
	CL_CODES = 19,
	MAX_STORED = 65535,
};

typedef struct {
	int good, lazy, nice, chain;
	bool slow;
} config;

/* zlib's table: reduce the chain beyond good, lazy matching up to lazy
   (the insertion limit for the fast levels), stop at nice */
static const config configs[10] = {
	{0, 0, 0, 0, false},
	{4, 4, 8, 4, false},
	{4, 5, 16, 8, false},
	{4, 6, 32, 32, false},
	{4, 4, 16, 16, true},
	{8, 16, 32, 32, true},
	{8, 16, 128, 128, true},
	{8, 32, 128, 256, true},
	{32, 128, 258, 1024, true},
	{32, 258, 258, 4096, true},
};

struct alt_deflate {
	alt_writer dst;
	int level;
	int err;
	bool closed;
	config cfg;
	unsigned char win[2 * W];
	int strstart, lookahead;
	int block_start, tok_end;   /* the raw bytes of the pending block */
	int32_t head[HASH_SIZE];    /* position + 1 of the newest string per hash, 0 none */
	int32_t prev[W];            /* the previous position + 1 of the same hash */
	/* lazy matching state */
	int match_length, match_start, prev_length, prev_match;
	bool match_available;
	/* the pending block */
	uint32_t tok[MAX_TOK];
	int ntok;
	uint32_t lit_freq[LIT_CODES], dist_freq[DIST_CODES];
	/* output */
	uint64_t bits;
	int nbits;
	alt_buf out;
	unsigned char *dict;
	size_t ndict;
};

/* ---- codes of lengths and distances ---- */

static const int len_base[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115,
	131, 163, 195, 227, 258};
static const int len_extra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const int dist_base[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537,
	2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static const int dist_extra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12,
	13, 13};
static unsigned char len_code[256]; /* length - 3 -> code - 257 */
static bool tables_ready;

static void init_tables(void) {
	if (tables_ready)
		return;
	for (int c = 0; c < 29; c++) {
		int n = c == 28 ? 1 : 1 << len_extra[c];
		for (int k = 0; k < n && len_base[c] - 3 + k < 256; k++)
			len_code[len_base[c] - 3 + k] = (unsigned char)c;
	}
	len_code[255] = 28;
	tables_ready = true;
}

static int dist_code(int d) { /* d = distance - 1 */
	if (d < 4)
		return d;
	int nb = 31 - __builtin_clz((unsigned)d); /* the top bit */
	return 2 * nb + ((d >> (nb - 1)) & 1);
}

/* ---- Huffman code lengths ---- */

typedef struct {
	uint32_t freq;
	int sym;
} item;

static int item_cmp(const void *a, const void *b) {
	const item *x = a, *y = b;
	if (x->freq != y->freq)
		return x->freq < y->freq ? -1 : 1;
	return x->sym - y->sym;
}

/* code lengths of at most maxbits for the frequencies; at least two codes
   (complete codes for every decoder) */
static void build_lengths(const uint32_t *freq, int n, int maxbits, unsigned char *len) {
	item it[LIT_CODES];
	int m = 0;
	memset(len, 0, (size_t)n);
	for (int i = 0; i < n; i++)
		if (freq[i])
			it[m++] = (item){freq[i], i};
	if (m < 2) {
		int a = m ? it[0].sym : 0, b = a == 0 ? 1 : 0;
		len[a] = len[b] = 1;
		return;
	}
	qsort(it, (size_t)m, sizeof *it, item_cmp);
	/* the Huffman tree by two queues: leaves sorted, internal nodes made
	   in increasing weight order */
	uint64_t w[2 * LIT_CODES];
	int parent[2 * LIT_CODES];
	for (int i = 0; i < m; i++)
		w[i] = it[i].freq;
	int leaf = 0, node = m, next = m;
	for (int k = 0; k < m - 1; k++) {
		int pick[2];
		for (int j = 0; j < 2; j++) {
			if (leaf < m && (node >= next || w[leaf] <= w[node]))
				pick[j] = leaf++;
			else
				pick[j] = node++;
		}
		w[next] = w[pick[0]] + w[pick[1]];
		parent[pick[0]] = parent[pick[1]] = next;
		next++;
	}
	/* depths: the root is next - 1 */
	int depth[2 * LIT_CODES];
	depth[next - 1] = 0;
	for (int i = next - 2; i >= 0; i--)
		depth[i] = depth[parent[i]] + 1;
	int count[64] = {0};
	for (int i = 0; i < m; i++)
		count[depth[i] < maxbits ? depth[i] : maxbits]++;
	/* limit the lengths, keeping the Kraft sum at one (as miniz does) */
	uint32_t total = 0;
	for (int i = maxbits; i > 0; i--)
		total += (uint32_t)count[i] << (maxbits - i);
	while (total != 1u << maxbits) {
		count[maxbits]--;
		for (int i = maxbits - 1; i > 0; i--)
			if (count[i]) {
				count[i]--;
				count[i + 1] += 2;
				break;
			}
		total--;
	}
	/* the most frequent symbols get the shortest codes */
	int k = m - 1;
	for (int l = 1; l <= maxbits; l++)
		for (int c = count[l]; c > 0; c--)
			len[it[k--].sym] = (unsigned char)l;
}

/* canonical codes, bit-reversed for writing least significant bit first */
static void make_codes(const unsigned char *len, int n, uint16_t *code) {
	int count[16] = {0}, next[16];
	for (int i = 0; i < n; i++)
		count[len[i]]++;
	count[0] = 0;
	int c = 0;
	for (int l = 1; l < 16; l++) {
		c = (c + count[l - 1]) << 1;
		next[l] = c;
	}
	for (int i = 0; i < n; i++) {
		int l = len[i];
		if (!l) {
			code[i] = 0;
			continue;
		}
		unsigned v = (unsigned)next[l]++, r = 0;
		for (int j = 0; j < l; j++)
			r = r << 1 | (v >> j & 1);
		code[i] = (uint16_t)r;
	}
}

/* ---- bit output ---- */

static void put_bits(alt_deflate *z, uint32_t v, int n) {
	z->bits |= (uint64_t)v << z->nbits;
	z->nbits += n;
	if (z->nbits >= 32) {
		unsigned char b[4] = {(unsigned char)z->bits, (unsigned char)(z->bits >> 8), (unsigned char)(z->bits >> 16),
			(unsigned char)(z->bits >> 24)};
		alt_buf_append(&z->out, b, 4);
		z->bits >>= 32;
		z->nbits -= 32;
	}
}

static void align_bits(alt_deflate *z) {
	while (z->nbits > 0) {
		alt_buf_append_byte(&z->out, (int)(z->bits & 0xff));
		z->bits >>= 8;
		z->nbits = z->nbits > 8 ? z->nbits - 8 : 0;
	}
	z->bits = 0;
}

static int flush_out(alt_deflate *z) {
	if (z->err)
		return z->err;
	if (z->out.err)
		return z->err = z->out.err;
	if (z->out.n) {
		ssize_t w = alt_write_all(z->dst, z->out.p, z->out.n);
		alt_buf_reset(&z->out);
		if (w < 0)
			return z->err = (int)w;
	}
	return 0;
}

/* ---- blocks ---- */

static const unsigned char cl_order[CL_CODES] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/* all 288 literal/length codes count for the canonical fixed code */
static void fixed_lengths(unsigned char *lit, unsigned char *dist) {
	for (int i = 0; i < 288; i++)
		lit[i] = i < 144 ? 8 : i < 256 ? 9 : i < 280 ? 7 : 8;
	for (int i = 0; i < DIST_CODES; i++)
		dist[i] = 5;
}

static uint64_t data_bits(alt_deflate *z, const unsigned char *lit, const unsigned char *dist) {
	uint64_t b = 0;
	for (int i = 0; i < LIT_CODES; i++)
		b += (uint64_t)z->lit_freq[i] * (lit[i] + (i > 256 ? len_extra[i - 257] : 0));
	for (int i = 0; i < DIST_CODES; i++)
		b += (uint64_t)z->dist_freq[i] * (dist[i] + dist_extra[i]);
	return b;
}

static void write_tokens(alt_deflate *z, const unsigned char *ll, const uint16_t *lc, const unsigned char *dl,
	const uint16_t *dc) {
	for (int i = 0; i < z->ntok; i++) {
		uint32_t t = z->tok[i];
		if (!(t >> 31)) {
			put_bits(z, lc[t], ll[t]);
			continue;
		}
		int len = (int)(t >> 16 & 0xff), d = (int)(t & 0xffff);
		int c = len_code[len];
		put_bits(z, lc[257 + c], ll[257 + c]);
		if (len_extra[c])
			put_bits(z, (uint32_t)(len + 3 - len_base[c]), len_extra[c]);
		int dcode = dist_code(d);
		put_bits(z, dc[dcode], dl[dcode]);
		if (dist_extra[dcode])
			put_bits(z, (uint32_t)(d + 1 - dist_base[dcode]), dist_extra[dcode]);
	}
	put_bits(z, lc[256], ll[256]);
}

static void write_stored(alt_deflate *z, const unsigned char *p, size_t n, bool final) {
	do {
		size_t k = n < MAX_STORED ? n : MAX_STORED;
		bool last = final && k == n;
		put_bits(z, last ? 1 : 0, 3);
		align_bits(z);
		unsigned char h[4] = {(unsigned char)k, (unsigned char)(k >> 8), (unsigned char)~k, (unsigned char)(~k >> 8)};
		alt_buf_append(&z->out, h, 4);
		alt_buf_append(&z->out, p, k);
		p += k, n -= k;
	} while (n);
}

/* writes the pending tokens (raw bytes win[block_start, tok_end)) as one block */
static void write_block(alt_deflate *z, bool final) {
	size_t raw = (size_t)(z->tok_end - z->block_start);
	const unsigned char *rp = z->win + z->block_start;
	if (z->level == 0) {
		write_stored(z, rp, raw, final);
		goto done;
	}
	z->lit_freq[256] = 1;
	unsigned char ll[LIT_CODES], dl[DIST_CODES], fl[288], fd[DIST_CODES];
	build_lengths(z->lit_freq, LIT_CODES, 15, ll);
	build_lengths(z->dist_freq, DIST_CODES, 15, dl);
	int nlit = LIT_CODES, ndist = DIST_CODES;
	while (nlit > 257 && !ll[nlit - 1])
		nlit--;
	while (ndist > 1 && !dl[ndist - 1])
		ndist--;
	/* the code lengths, run-length encoded with 16, 17 and 18 */
	unsigned char lens[LIT_CODES + DIST_CODES], rl[LIT_CODES + DIST_CODES], rx[LIT_CODES + DIST_CODES];
	int nl = 0, nr = 0;
	for (int i = 0; i < nlit; i++)
		lens[nl++] = ll[i];
	for (int i = 0; i < ndist; i++)
		lens[nl++] = dl[i];
	uint32_t cl_freq[CL_CODES] = {0};
	for (int i = 0; i < nl;) {
		int l = lens[i], run = 1;
		while (i + run < nl && lens[i + run] == l)
			run++;
		if (l == 0 && run >= 3) {
			int k = run > 138 ? 138 : run;
			if (k >= 11)
				rl[nr] = 18, rx[nr++] = (unsigned char)(k - 11);
			else
				rl[nr] = 17, rx[nr++] = (unsigned char)(k - 3);
			i += k;
		} else if (l != 0 && run >= 4) {
			rl[nr] = (unsigned char)l, rx[nr++] = 0;
			int k = run - 1 > 6 ? 6 : run - 1;
			rl[nr] = 16, rx[nr++] = (unsigned char)(k - 3);
			i += 1 + k;
		} else {
			rl[nr] = (unsigned char)l, rx[nr++] = 0;
			i++;
		}
	}
	for (int i = 0; i < nr; i++)
		cl_freq[rl[i]]++;
	unsigned char cl[CL_CODES];
	build_lengths(cl_freq, CL_CODES, 7, cl);
	int nclen = CL_CODES;
	while (nclen > 4 && !cl[cl_order[nclen - 1]])
		nclen--;
	uint64_t dyn = 3 + 14 + 3 * (uint64_t)nclen + data_bits(z, ll, dl);
	for (int i = 0; i < CL_CODES; i++)
		dyn += (uint64_t)cl_freq[i] * (cl[i] + (i == 16 ? 2 : i == 17 ? 3 : i == 18 ? 7 : 0));
	fixed_lengths(fl, fd);
	uint64_t fix = 3 + data_bits(z, fl, fd);
	uint64_t stored = (raw + 5 * (raw / MAX_STORED + 1)) * 8 + 8;
	if (stored <= dyn && stored <= fix) {
		write_stored(z, rp, raw, final);
	} else if (fix <= dyn) {
		uint16_t lc[288], dc[DIST_CODES];
		make_codes(fl, 288, lc);
		make_codes(fd, DIST_CODES, dc);
		put_bits(z, (final ? 1 : 0) | 1 << 1, 3);
		write_tokens(z, fl, lc, fd, dc);
	} else {
		uint16_t lc[LIT_CODES], dc[DIST_CODES], cc[CL_CODES];
		make_codes(ll, LIT_CODES, lc);
		make_codes(dl, DIST_CODES, dc);
		make_codes(cl, CL_CODES, cc);
		put_bits(z, (final ? 1 : 0) | 2 << 1, 3);
		put_bits(z, (uint32_t)(nlit - 257), 5);
		put_bits(z, (uint32_t)(ndist - 1), 5);
		put_bits(z, (uint32_t)(nclen - 4), 4);
		for (int i = 0; i < nclen; i++)
			put_bits(z, cl[cl_order[i]], 3);
		for (int i = 0; i < nr; i++) {
			put_bits(z, cc[rl[i]], cl[rl[i]]);
			if (rl[i] >= 16)
				put_bits(z, rx[i], rl[i] == 16 ? 2 : rl[i] == 17 ? 3 : 7);
		}
		write_tokens(z, ll, lc, dl, dc);
	}
done:
	z->ntok = 0;
	memset(z->lit_freq, 0, sizeof z->lit_freq);
	memset(z->dist_freq, 0, sizeof z->dist_freq);
	z->block_start = z->tok_end;
}

static void emit_literal(alt_deflate *z, int pos) {
	unsigned c = z->win[pos];
	z->tok[z->ntok++] = c;
	z->lit_freq[c]++;
	z->tok_end = pos + 1;
	if (z->ntok == MAX_TOK)
		write_block(z, false);
}

static void emit_match(alt_deflate *z, int pos, int dist, int len) {
	z->tok[z->ntok++] = 1u << 31 | (uint32_t)(len - 3) << 16 | (uint32_t)(dist - 1);
	z->lit_freq[257 + len_code[len - 3]]++;
	z->dist_freq[dist_code(dist - 1)]++;
	z->tok_end = pos + len;
	if (z->ntok == MAX_TOK)
		write_block(z, false);
}

/* ---- matching ---- */

static unsigned hash3(const unsigned char *p) {
	uint32_t v = (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16;
	return (v * 2654435761u) >> (32 - HASH_BITS);
}

/* inserts the string at pos: the previous head (position + 1), 0 if none */
static int insert(alt_deflate *z, int pos) {
	if (pos + MIN_MATCH > z->strstart + z->lookahead)
		return 0;
	unsigned h = hash3(z->win + pos);
	int head = z->head[h];
	z->prev[pos & WMASK] = head;
	z->head[h] = pos + 1;
	return head;
}

/* the longest match at strstart longer than best, searching from
   cur (position + 1); sets match_start */
static int longest_match(alt_deflate *z, int cur, int best) {
	int chain = z->cfg.chain, nice = z->cfg.nice;
	if (best >= z->cfg.good)
		chain >>= 2;
	if (nice > z->lookahead)
		nice = z->lookahead;
	int limit = z->strstart > MAX_DIST ? z->strstart - MAX_DIST : 0;
	const unsigned char *s = z->win + z->strstart;
	int maxlen = z->lookahead < MAX_MATCH ? z->lookahead : MAX_MATCH;
	if (best >= maxlen)
		return best;
	for (int m = cur - 1; m >= limit && m < z->strstart && chain-- > 0; m = z->prev[m & WMASK] - 1) {
		const unsigned char *p = z->win + m;
		if (p[best] != s[best] || p[0] != s[0] || p[1] != s[1])
			continue;
		int l = 2;
		while (l < maxlen && p[l] == s[l])
			l++;
		if (l > best) {
			z->match_start = m;
			best = l;
			if (l >= nice)
				break;
		}
		if (best >= maxlen)
			break;
	}
	return best;
}

static void compress_fast(alt_deflate *z, bool flush) {
	while (z->lookahead >= MIN_LOOKAHEAD || (flush && z->lookahead > 0)) {
		int head = insert(z, z->strstart), len = 0;
		if (head && z->strstart - (head - 1) <= MAX_DIST && z->level > 0)
			len = longest_match(z, head, MIN_MATCH - 1);
		if (len >= MIN_MATCH) {
			emit_match(z, z->strstart, z->strstart - z->match_start, len);
			if (len <= z->cfg.lazy) {
				for (int k = 1; k < len; k++)
					insert(z, z->strstart + k);
			}
			z->lookahead -= len;
			z->strstart += len;
		} else {
			emit_literal(z, z->strstart);
			z->strstart++;
			z->lookahead--;
		}
	}
}

static void compress_slow(alt_deflate *z, bool flush) {
	while (z->lookahead >= MIN_LOOKAHEAD || (flush && z->lookahead > 0)) {
		int head = insert(z, z->strstart);
		z->prev_length = z->match_length;
		z->prev_match = z->match_start;
		z->match_length = MIN_MATCH - 1;
		if (head && z->prev_length < z->cfg.lazy && z->strstart - (head - 1) <= MAX_DIST) {
			z->match_length = longest_match(z, head, z->prev_length);
			if (z->match_length < MIN_MATCH || (z->match_length == MIN_MATCH && z->strstart - z->match_start > TOO_FAR)) {
				z->match_length = MIN_MATCH - 1;
				z->match_start = z->prev_match;
			}
		}
		if (z->prev_length >= MIN_MATCH && z->match_length <= z->prev_length) {
			int at = z->strstart - 1, len = z->prev_length;
			emit_match(z, at, at - z->prev_match, len);
			/* the strings inside the match go into the hash table */
			for (int k = 1; k < len - 1; k++)
				insert(z, z->strstart + k);
			z->lookahead -= len - 1;
			z->strstart += len - 1;
			z->match_available = false;
			z->match_length = MIN_MATCH - 1;
		} else if (z->match_available) {
			emit_literal(z, z->strstart - 1);
			z->strstart++;
			z->lookahead--;
		} else {
			z->match_available = true;
			z->strstart++;
			z->lookahead--;
		}
	}
	if (flush && z->match_available) {
		emit_literal(z, z->strstart - 1);
		z->match_available = false;
		z->match_length = MIN_MATCH - 1; /* its match is not pending any more */
	}
}

static void compress(alt_deflate *z, bool flush) {
	if (z->level == ALT_FLATE_HUFFMAN_ONLY || z->level == 0) {
		while (z->lookahead > 0 && (flush || z->lookahead >= MIN_LOOKAHEAD)) {
			if (z->level == 0) {
				/* stored: no tokens, the raw bytes are the block */
				z->tok_end = z->strstart + 1;
				if (z->tok_end - z->block_start >= MAX_STORED)
					write_block(z, false);
			} else
				emit_literal(z, z->strstart);
			z->strstart++;
			z->lookahead--;
		}
		return;
	}
	if (z->cfg.slow)
		compress_slow(z, flush);
	else
		compress_fast(z, flush);
}

/* moves the upper half of the window down, after writing the pending block
   (whose raw bytes would be lost) */
static void slide(alt_deflate *z) {
	if (z->tok_end > z->block_start || z->ntok)
		write_block(z, false);
	memmove(z->win, z->win + W, W);
	z->strstart -= W;
	z->block_start -= W;
	z->tok_end -= W;
	z->match_start -= W;
	z->prev_match -= W;
	for (int i = 0; i < HASH_SIZE; i++)
		z->head[i] = z->head[i] > W ? z->head[i] - W : 0;
	for (int i = 0; i < W; i++)
		z->prev[i] = z->prev[i] > W ? z->prev[i] - W : 0;
}

ssize_t alt_deflate_write(alt_deflate *z, const void *p, size_t n) {
	if (z->closed)
		return -ALT_ECLOSED;
	if (z->err)
		return z->err;
	const unsigned char *s = p;
	size_t done = 0;
	while (done < n) {
		int end = z->strstart + z->lookahead;
		if (end == 2 * W) {
			compress(z, false);
			slide(z);
			if (flush_out(z) < 0)
				return z->err;
			end = z->strstart + z->lookahead;
		}
		size_t k = (size_t)(2 * W - end);
		if (k > n - done)
			k = n - done;
		memcpy(z->win + end, s + done, k);
		z->lookahead += (int)k;
		done += k;
		compress(z, false);
	}
	if (z->out.n >= 65536 && flush_out(z) < 0)
		return z->err;
	return (ssize_t)n;
}

int alt_deflate_flush(alt_deflate *z) {
	if (z->closed)
		return -ALT_ECLOSED;
	if (z->err)
		return z->err;
	compress(z, true);
	if (z->ntok || z->tok_end > z->block_start)
		write_block(z, false);
	/* an empty stored block: byte aligned, 00 00 ff ff */
	put_bits(z, 0, 3);
	align_bits(z);
	alt_buf_append(&z->out, "\0\0\xff\xff", 4);
	return flush_out(z);
}

int alt_deflate_close(alt_deflate *z) {
	if (z->closed)
		return z->err;
	if (z->err)
		return z->err;
	compress(z, true);
	write_block(z, true);
	align_bits(z);
	z->closed = true;
	return flush_out(z);
}

static ssize_t writer_write(void *obj, const void *p, size_t n) { return alt_deflate_write(obj, p, n); }
alt_writer alt_deflate_writer(alt_deflate *z) { return (alt_writer){writer_write, z}; }

void alt_deflate_reset(alt_deflate *z, alt_writer dst) {
	z->dst = dst;
	z->err = 0;
	z->closed = false;
	z->strstart = z->lookahead = z->block_start = z->tok_end = 0;
	memset(z->head, 0, sizeof z->head);
	memset(z->prev, 0, sizeof z->prev);
	z->match_length = z->prev_length = MIN_MATCH - 1;
	z->match_start = z->prev_match = 0;
	z->match_available = false;
	z->ntok = 0;
	memset(z->lit_freq, 0, sizeof z->lit_freq);
	memset(z->dist_freq, 0, sizeof z->dist_freq);
	z->bits = 0;
	z->nbits = 0;
	alt_buf_reset(&z->out);
	if (z->ndict) {
		size_t n = z->ndict > W ? W : z->ndict;
		memcpy(z->win, z->dict + z->ndict - n, n);
		z->lookahead = (int)n; /* for insert's bounds */
		for (int i = 0; i + MIN_MATCH <= (int)n; i++)
			insert(z, i);
		z->lookahead = 0;
		z->strstart = z->block_start = z->tok_end = (int)n;
	}
}

int alt_deflate_new_dict(alt_deflate **out, alt_writer dst, int level, const void *dict, size_t n) {
	if (level == ALT_FLATE_DEFAULT)
		level = 6;
	if (level < ALT_FLATE_HUFFMAN_ONLY || level > 9)
		return -EINVAL;
	init_tables();
	alt_deflate *z = calloc(1, sizeof *z);
	if (!z)
		return -ENOMEM;
	z->level = level;
	z->cfg = configs[level > 0 ? level : 0];
	if (n) {
		if (!(z->dict = malloc(n))) {
			free(z);
			return -ENOMEM;
		}
		memcpy(z->dict, dict, n);
		z->ndict = n;
	}
	alt_deflate_reset(z, dst);
	*out = z;
	return 0;
}

int alt_deflate_new(alt_deflate **out, alt_writer dst, int level) { return alt_deflate_new_dict(out, dst, level, 0, 0); }

void alt_deflate_free(alt_deflate *z) {
	if (!z)
		return;
	alt_buf_free(&z->out);
	free(z->dict);
	free(z);
}

int alt_deflate_data(alt_buf *out, const void *p, size_t n, int level) {
	alt_deflate *z;
	int rc = alt_deflate_new(&z, alt_buf_writer(out), level);
	if (rc < 0)
		return rc;
	ssize_t w = alt_deflate_write(z, p, n);
	rc = w < 0 ? (int)w : alt_deflate_close(z);
	alt_deflate_free(z);
	return rc;
}
