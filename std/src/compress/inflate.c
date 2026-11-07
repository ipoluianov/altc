/* DEFLATE decompression, translated from Go's compress/flate
   (inflate.go, dict_decoder.go; BSD-3-Clause, see LICENSE-Go): the same
   checks of the input, the same offsets in errors, the same bytes read. */
#include <alt/compress/flate.h>
#include <alt/sync.h>
#include <stdlib.h>
#include <string.h>

enum {
	MAX_CODE_LEN = 16,
	MAX_NUM_LIT = 286,
	MAX_NUM_DIST = 30,
	NUM_CODES = 19,
	CHUNK_BITS = 9,
	NUM_CHUNKS = 1 << CHUNK_BITS,
	COUNT_MASK = 15,
	VALUE_SHIFT = 4,
	HIST = 32768,
	END_BLOCK = 256,
};

/* chunk & 15 is the number of bits, chunk >> 4 the value or the link */
typedef struct {
	int min;
	uint32_t chunks[NUM_CHUNKS];
	uint32_t *links;      /* nlinks tables of link_mask + 1 entries */
	size_t links_cap;
	uint32_t link_mask;
} huff;

static uint16_t rev16(uint16_t v) {
	v = (uint16_t)((v >> 1 & 0x5555) | (v & 0x5555) << 1);
	v = (uint16_t)((v >> 2 & 0x3333) | (v & 0x3333) << 2);
	v = (uint16_t)((v >> 4 & 0x0f0f) | (v & 0x0f0f) << 4);
	return (uint16_t)(v >> 8 | v << 8);
}

/* Go's huffmanDecoder.init: false unless the code is complete (or a
   single code of length 1); an empty code is accepted */
static bool huff_init(huff *h, const int *lengths, int n) {
	h->min = 0;
	h->link_mask = 0;
	memset(h->chunks, 0, sizeof h->chunks);
	int count[MAX_CODE_LEN] = {0}, min = 0, max = 0;
	for (int i = 0; i < n; i++) {
		int l = lengths[i];
		if (l == 0)
			continue;
		if (min == 0 || l < min)
			min = l;
		if (l > max)
			max = l;
		count[l]++;
	}
	if (max == 0)
		return true;
	int code = 0, nextcode[MAX_CODE_LEN] = {0};
	for (int i = min; i <= max; i++) {
		code <<= 1;
		nextcode[i] = code;
		code += count[i];
	}
	if (code != 1 << max && !(code == 1 && max == 1))
		return false;
	h->min = min;
	int numlinks = 0, link = 0;
	if (max > CHUNK_BITS) {
		numlinks = 1 << (max - CHUNK_BITS);
		h->link_mask = (uint32_t)(numlinks - 1);
		link = nextcode[CHUNK_BITS + 1] >> 1;
		size_t need = (size_t)(NUM_CHUNKS - link) * (size_t)numlinks;
		if (need > h->links_cap) {
			uint32_t *p = realloc(h->links, need * sizeof *p);
			if (!p)
				return false;
			h->links = p;
			h->links_cap = need;
		}
		memset(h->links, 0, need * sizeof *h->links);
		for (int j = link; j < NUM_CHUNKS; j++) {
			int reverse = rev16((uint16_t)j) >> (16 - CHUNK_BITS);
			int off = j - link;
			h->chunks[reverse] = (uint32_t)(off << VALUE_SHIFT | (CHUNK_BITS + 1));
		}
	}
	for (int i = 0; i < n; i++) {
		int l = lengths[i];
		if (l == 0)
			continue;
		int c = nextcode[l]++;
		uint32_t chunk = (uint32_t)(i << VALUE_SHIFT | l);
		int reverse = rev16((uint16_t)c) >> (16 - l);
		if (l <= CHUNK_BITS) {
			for (int off = reverse; off < NUM_CHUNKS; off += 1 << l)
				h->chunks[off] = chunk;
		} else {
			int j = reverse & (NUM_CHUNKS - 1);
			uint32_t *tab = h->links + (size_t)(h->chunks[j] >> VALUE_SHIFT) * (size_t)numlinks;
			reverse >>= CHUNK_BITS;
			for (int off = reverse; off < numlinks; off += 1 << (l - CHUNK_BITS))
				tab[off] = chunk;
		}
	}
	return true;
}

static huff fixed;
static alt_once fixed_once = ALT_ONCE_INIT;

static void fixed_init(void *arg) {
	(void)arg;
	int bits[288];
	for (int i = 0; i < 144; i++)
		bits[i] = 8;
	for (int i = 144; i < 256; i++)
		bits[i] = 9;
	for (int i = 256; i < 280; i++)
		bits[i] = 7;
	for (int i = 280; i < 288; i++)
		bits[i] = 8;
	huff_init(&fixed, bits, 288);
}

enum { STEP_NEXT_BLOCK, STEP_HUFFMAN, STEP_COPY_DATA };
enum { STATE_INIT, STATE_DICT };

struct alt_inflate {
	alt_bufreader *r;
	alt_bufreader own;     /* when made from an alt_reader */
	bool owns;
	int64_t roffset;
	uint32_t b;
	unsigned nb;
	huff h1, h2;
	int bits[MAX_NUM_LIT + MAX_NUM_DIST];
	int codebits[NUM_CODES];
	/* the window (dictDecoder) */
	unsigned char hist[HIST];
	int wr, rd;
	bool full;
	/* the pending output hist[to_read, to_read + nto_read) */
	int to_read, nto_read;
	int step, step_state;
	bool final;
	int err;               /* 0, 1 for the end (io.EOF), or an error code */
	int64_t err_offset;
	huff *hl, *hd;
	int copy_len, copy_dist;
};

/* ---- window ---- */

static int hist_size(alt_inflate *f) { return f->full ? HIST : f->wr; }
static int avail_write(alt_inflate *f) { return HIST - f->wr; }

static void read_flush(alt_inflate *f) {
	f->to_read = f->rd;
	f->nto_read = f->wr - f->rd;
	f->rd = f->wr;
	if (f->wr == HIST) {
		f->wr = f->rd = 0;
		f->full = true;
	}
}

static int write_copy(alt_inflate *f, int dist, int length) {
	int base = f->wr, dst = base, src = dst - dist;
	int end = dst + length < HIST ? dst + length : HIST;
	if (src < 0) {
		src += HIST;
		int k = end - dst < HIST - src ? end - dst : HIST - src;
		memmove(f->hist + dst, f->hist + src, (size_t)k);
		dst += k;
		src = 0;
	}
	while (dst < end) {
		int k = end - dst < dst - src ? end - dst : dst - src;
		memmove(f->hist + dst, f->hist + src, (size_t)k);
		dst += k;
	}
	f->wr = dst;
	return dst - base;
}

static int try_write_copy(alt_inflate *f, int dist, int length) {
	int dst = f->wr, end = dst + length;
	if (dst < dist || end > HIST)
		return 0;
	int base = dst, src = dst - dist;
	while (dst < end) {
		int k = end - dst < dst - src ? end - dst : dst - src;
		memmove(f->hist + dst, f->hist + src, (size_t)k);
		dst += k;
	}
	f->wr = dst;
	return dst - base;
}

/* ---- input ---- */

static int corrupt(alt_inflate *f) {
	f->err_offset = f->roffset;
	return -ALT_EFLATE_CORRUPT;
}

static int no_eof(int rc) { return rc == -ALT_EEOF ? -ALT_EUNEXPECTED_EOF : rc; }

static int more_bits(alt_inflate *f) {
	int c = alt_bufreader_byte(f->r);
	if (c < 0)
		return no_eof(c);
	f->roffset++;
	f->b |= (uint32_t)c << f->nb;
	f->nb += 8;
	return 0;
}

static int need(alt_inflate *f, unsigned n) {
	while (f->nb < n) {
		int rc = more_bits(f);
		if (rc < 0)
			return rc;
	}
	return 0;
}

/* Go's huffSym: a symbol, or an error (empty and degenerate codes give
   chunks of 0 bits for the invalid sequences) */
static int huff_sym(alt_inflate *f, const huff *h) {
	unsigned n = (unsigned)h->min, nb = f->nb;
	uint32_t b = f->b;
	for (;;) {
		while (nb < n) {
			int c = alt_bufreader_byte(f->r);
			if (c < 0) {
				f->b = b, f->nb = nb;
				return no_eof(c);
			}
			f->roffset++;
			b |= (uint32_t)c << (nb & 31);
			nb += 8;
		}
		uint32_t chunk = h->chunks[b & (NUM_CHUNKS - 1)];
		n = chunk & COUNT_MASK;
		if (n > CHUNK_BITS) {
			chunk = h->links[(size_t)(chunk >> VALUE_SHIFT) * (h->link_mask + 1) + ((b >> CHUNK_BITS) & h->link_mask)];
			n = chunk & COUNT_MASK;
		}
		if (n <= nb) {
			if (n == 0) {
				f->b = b, f->nb = nb;
				return corrupt(f);
			}
			f->b = b >> (n & 31);
			f->nb = nb - n;
			return (int)(chunk >> VALUE_SHIFT);
		}
	}
}

static const int code_order[NUM_CODES] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static int read_huffman(alt_inflate *f) {
	int rc = need(f, 5 + 5 + 4);
	if (rc < 0)
		return rc;
	int nlit = (int)(f->b & 0x1f) + 257;
	if (nlit > MAX_NUM_LIT)
		return corrupt(f);
	f->b >>= 5;
	int ndist = (int)(f->b & 0x1f) + 1;
	if (ndist > MAX_NUM_DIST)
		return corrupt(f);
	f->b >>= 5;
	int nclen = (int)(f->b & 0xf) + 4;
	f->b >>= 4;
	f->nb -= 5 + 5 + 4;
	for (int i = 0; i < nclen; i++) {
		if ((rc = need(f, 3)) < 0)
			return rc;
		f->codebits[code_order[i]] = (int)(f->b & 7);
		f->b >>= 3;
		f->nb -= 3;
	}
	for (int i = nclen; i < NUM_CODES; i++)
		f->codebits[code_order[i]] = 0;
	if (!huff_init(&f->h1, f->codebits, NUM_CODES))
		return corrupt(f);
	for (int i = 0, n = nlit + ndist; i < n;) {
		int x = huff_sym(f, &f->h1);
		if (x < 0)
			return x;
		if (x < 16) {
			f->bits[i++] = x;
			continue;
		}
		int rep, b;
		unsigned nb;
		switch (x) {
		case 16:
			rep = 3, nb = 2;
			if (i == 0)
				return corrupt(f);
			b = f->bits[i - 1];
			break;
		case 17: rep = 3, nb = 3, b = 0; break;
		default: rep = 11, nb = 7, b = 0; break;
		}
		if ((rc = need(f, nb)) < 0)
			return rc;
		rep += (int)(f->b & ((1u << nb) - 1));
		f->b >>= nb;
		f->nb -= nb;
		if (i + rep > n)
			return corrupt(f);
		for (int j = 0; j < rep; j++)
			f->bits[i++] = b;
	}
	if (!huff_init(&f->h1, f->bits, nlit) || !huff_init(&f->h2, f->bits + nlit, ndist))
		return corrupt(f);
	/* every block ends with the end-of-block code: never reading fewer
	   bits keeps the decoder from reading past the stream */
	if (f->h1.min < f->bits[END_BLOCK])
		f->h1.min = f->bits[END_BLOCK];
	return 0;
}

static void finish_block(alt_inflate *f) {
	if (f->final) {
		if (f->wr - f->rd > 0)
			read_flush(f);
		f->err = 1;
	}
	f->step = STEP_NEXT_BLOCK;
}

static void copy_data(alt_inflate *f) {
	int k = HIST - f->wr;
	if (k > f->copy_len)
		k = f->copy_len;
	ssize_t cnt = 0;
	int rc = 0;
	while (cnt < k) {
		ssize_t m = alt_bufreader_read(f->r, f->hist + f->wr + cnt, (size_t)(k - cnt));
		if (m <= 0) {
			rc = m == 0 ? -ALT_EUNEXPECTED_EOF : no_eof((int)m);
			break;
		}
		cnt += m;
	}
	f->roffset += cnt;
	f->copy_len -= (int)cnt;
	f->wr += (int)cnt;
	if (rc < 0) {
		f->err = rc;
		return;
	}
	if (avail_write(f) == 0 || f->copy_len > 0) {
		read_flush(f);
		f->step = STEP_COPY_DATA;
		return;
	}
	finish_block(f);
}

static void data_block(alt_inflate *f) {
	f->nb = 0;
	f->b = 0;
	unsigned char buf[4];
	ssize_t got = 0;
	int rc = 0;
	while (got < 4) {
		ssize_t m = alt_bufreader_read(f->r, buf + got, (size_t)(4 - got));
		if (m <= 0) {
			/* io.ReadFull: EOF before any byte, else ErrUnexpectedEOF;
			   noEOF makes both unexpected */
			rc = m == 0 ? -ALT_EUNEXPECTED_EOF : no_eof((int)m);
			break;
		}
		got += m;
	}
	f->roffset += got;
	if (rc < 0) {
		f->err = rc;
		return;
	}
	int n = buf[0] | buf[1] << 8, nn = buf[2] | buf[3] << 8;
	if ((uint16_t)nn != (uint16_t)~n) {
		f->err = corrupt(f);
		return;
	}
	if (n == 0) {
		read_flush(f);
		finish_block(f);
		return;
	}
	f->copy_len = n;
	copy_data(f);
}

static void huffman_block(alt_inflate *f) {
	if (f->step_state == STATE_DICT)
		goto copy_history;
read_literal: {
	int v = huff_sym(f, f->hl);
	if (v < 0) {
		f->err = v;
		return;
	}
	unsigned n;
	int length;
	if (v < 256) {
		f->hist[f->wr++] = (unsigned char)v;
		if (avail_write(f) == 0) {
			read_flush(f);
			f->step = STEP_HUFFMAN;
			f->step_state = STATE_INIT;
			return;
		}
		goto read_literal;
	} else if (v == 256) {
		finish_block(f);
		return;
	} else if (v < 265)
		length = v - (257 - 3), n = 0;
	else if (v < 269)
		length = v * 2 - (265 * 2 - 11), n = 1;
	else if (v < 273)
		length = v * 4 - (269 * 4 - 19), n = 2;
	else if (v < 277)
		length = v * 8 - (273 * 8 - 35), n = 3;
	else if (v < 281)
		length = v * 16 - (277 * 16 - 67), n = 4;
	else if (v < 285)
		length = v * 32 - (281 * 32 - 131), n = 5;
	else if (v < MAX_NUM_LIT)
		length = 258, n = 0;
	else {
		f->err = corrupt(f);
		return;
	}
	int rc;
	if (n > 0) {
		if ((rc = need(f, n)) < 0) {
			f->err = rc;
			return;
		}
		length += (int)(f->b & ((1u << n) - 1));
		f->b >>= n;
		f->nb -= n;
	}
	int dist;
	if (!f->hd) {
		if ((rc = need(f, 5)) < 0) {
			f->err = rc;
			return;
		}
		/* five bits, most significant first */
		uint32_t x = f->b & 0x1f;
		dist = (int)((x & 1) << 4 | (x & 2) << 2 | (x & 4) | (x & 8) >> 2 | (x & 16) >> 4);
		f->b >>= 5;
		f->nb -= 5;
	} else if ((dist = huff_sym(f, f->hd)) < 0) {
		f->err = dist;
		return;
	}
	if (dist < 4)
		dist++;
	else if (dist < MAX_NUM_DIST) {
		unsigned nb = (unsigned)(dist - 2) >> 1;
		int extra = (dist & 1) << nb;
		if ((rc = need(f, nb)) < 0) {
			f->err = rc;
			return;
		}
		extra |= (int)(f->b & ((1u << nb) - 1));
		f->b >>= nb;
		f->nb -= nb;
		dist = (1 << (nb + 1)) + 1 + extra;
	} else {
		f->err = corrupt(f);
		return;
	}
	if (dist > hist_size(f)) {
		f->err = corrupt(f);
		return;
	}
	f->copy_len = length, f->copy_dist = dist;
}
copy_history: {
	int cnt = try_write_copy(f, f->copy_dist, f->copy_len);
	if (cnt == 0)
		cnt = write_copy(f, f->copy_dist, f->copy_len);
	f->copy_len -= cnt;
	if (avail_write(f) == 0 || f->copy_len > 0) {
		read_flush(f);
		f->step = STEP_HUFFMAN;
		f->step_state = STATE_DICT;
		return;
	}
	goto read_literal;
}
}

static void next_block(alt_inflate *f) {
	int rc = need(f, 1 + 2);
	if (rc < 0) {
		f->err = rc;
		return;
	}
	f->final = f->b & 1;
	f->b >>= 1;
	uint32_t typ = f->b & 3;
	f->b >>= 2;
	f->nb -= 1 + 2;
	f->step_state = STATE_INIT;
	switch (typ) {
	case 0: data_block(f); break;
	case 1:
		f->hl = &fixed;
		f->hd = 0;
		huffman_block(f);
		break;
	case 2:
		if ((rc = read_huffman(f)) < 0) {
			f->err = rc;
			break;
		}
		f->hl = &f->h1;
		f->hd = &f->h2;
		huffman_block(f);
		break;
	default: f->err = corrupt(f);
	}
}

ssize_t alt_inflate_read(alt_inflate *f, void *buf, size_t n) {
	for (;;) {
		if (f->nto_read > 0) {
			size_t k = (size_t)f->nto_read < n ? (size_t)f->nto_read : n;
			memcpy(buf, f->hist + f->to_read, k);
			f->to_read += (int)k;
			f->nto_read -= (int)k;
			return (ssize_t)k;
		}
		if (f->err)
			return f->err == 1 ? 0 : f->err;
		if (n == 0)
			return 0;
		switch (f->step) {
		case STEP_NEXT_BLOCK: next_block(f); break;
		case STEP_HUFFMAN: huffman_block(f); break;
		case STEP_COPY_DATA: copy_data(f); break;
		}
		if (f->err && f->nto_read == 0)
			read_flush(f); /* what was decoded before the error */
	}
}

static ssize_t reader_read(void *obj, void *buf, size_t n) { return alt_inflate_read(obj, buf, n); }
alt_reader alt_inflate_reader(alt_inflate *z) { return (alt_reader){reader_read, z}; }
int64_t alt_inflate_err_offset(const alt_inflate *z) { return z->err_offset; }

void alt_inflate_reset(alt_inflate *f, alt_bufreader *src) {
	if (src) {
		if (f->owns && src != &f->own) {
			alt_bufreader_free(&f->own);
			f->owns = false;
		}
		f->r = src;
	}
	f->roffset = 0;
	f->b = f->nb = 0;
	f->h1.min = f->h2.min = 0;
	f->wr = f->rd = 0;
	f->full = false;
	f->to_read = f->nto_read = 0;
	f->step = STEP_NEXT_BLOCK;
	f->step_state = STATE_INIT;
	f->final = false;
	f->err = 0;
	f->err_offset = 0;
	f->hl = f->hd = 0;
	f->copy_len = f->copy_dist = 0;
}

void alt_inflate_set_dict(alt_inflate *f, const void *dict, size_t n) {
	const unsigned char *d = dict;
	if (n > HIST) {
		d += n - HIST;
		n = HIST;
	}
	memcpy(f->hist, d, n);
	f->wr = (int)n;
	f->full = false;
	if (f->wr == HIST) {
		f->wr = 0;
		f->full = true;
	}
	f->rd = f->wr;
}

int alt_inflate_new_buf(alt_inflate **out, alt_bufreader *src) {
	alt_once_do(&fixed_once, fixed_init, 0);
	alt_inflate *f = calloc(1, sizeof *f);
	if (!f)
		return -ENOMEM;
	f->r = src;
	alt_inflate_reset(f, 0);
	*out = f;
	return 0;
}

int alt_inflate_new(alt_inflate **out, alt_reader src) {
	alt_inflate *f;
	int rc = alt_inflate_new_buf(&f, 0);
	if (rc < 0)
		return rc;
	if ((rc = alt_bufreader_init(&f->own, src, 0)) < 0) {
		free(f);
		return rc;
	}
	f->owns = true;
	f->r = &f->own;
	*out = f;
	return 0;
}

void alt_inflate_free(alt_inflate *f) {
	if (!f)
		return;
	if (f->owns)
		alt_bufreader_free(&f->own);
	free(f->h1.links);
	free(f->h2.links);
	free(f);
}

int alt_inflate_data(alt_buf *out, const void *p, size_t n, size_t limit) {
	alt_str_reader sr;
	alt_bufreader br;
	int rc = alt_bufreader_init(&br, alt_str_reader_init(&sr, alt_str_make(p, n)), 0);
	if (rc < 0)
		return rc;
	alt_inflate *f;
	if ((rc = alt_inflate_new_buf(&f, &br)) < 0) {
		alt_bufreader_free(&br);
		return rc;
	}
	ssize_t got = alt_read_all(alt_inflate_reader(f), out, limit);
	alt_inflate_free(f);
	alt_bufreader_free(&br);
	return got < 0 ? (int)got : 0;
}
